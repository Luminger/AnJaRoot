#include <system_error>
#include <errno.h>
#include <linux/user.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shared/util.h"

bool shouldRun = true;
pid_t initPid = 1;
pid_t zygotePid = 0;

pid_t getZygotePid()
{
    // So... we could iterate through /proc/ to find a process name zygote and
    // read one of the status files where format is not guaranteed for specifig
    // Linux version... Or we could grab the zygote socket in
    // /dev/socket/zygote and ask the socket for the remote pid!
    //
    // I would say that's a clever (portable!) hack
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1)
    {
        util::logError("Failed to create zygote socket: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    struct sockaddr_un addr = {0, };
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/dev/socket/zygote");

    int ret = connect(fd, reinterpret_cast<struct sockaddr *>(&addr),
            sizeof(addr.sun_family) + sizeof(addr.sun_path));
    if(ret == -1)
    {
        util::logError("Failed to connect to zygote socket: %s", strerror(errno));
        close(fd);
        throw std::system_error(errno, std::system_category());
    }

    struct ucred creds = {0, };
    socklen_t len = sizeof(creds);
    ret = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &creds, &len);

    if(ret == -1)
    {
        util::logError("Failed to get socket credentials: %s", strerror(errno));
        close(fd);
        throw std::system_error(errno, std::system_category());
    }

    close(fd);

    return creds.pid;
}

void startTrace(pid_t pid)
{
    util::logVerbose("Attaching...");
    int ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to attach to process: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    util::logVerbose("Waiting for process to stop...");
    wait(NULL);
}

void setupChildTrace(pid_t pid)
{
    int ret = ptrace(PTRACE_SETOPTIONS, pid, NULL,
            reinterpret_cast<void*>(PTRACE_O_TRACEFORK));
    if(ret == -1)
    {
        util::logError("Failed to setup fork tracing: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void endTrace(pid_t pid)
{
    int ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to deattach from %d: %s", pid, strerror(errno));
        return;
    }

    util::logVerbose("Detached from %d", pid);
}

void runMainLoop(pid_t zygote)
{
    int status;
    while(shouldRun)
    {
        util::logVerbose("Waiting for child action...");
        pid_t pid = wait(&status);

        if(pid == -1)
        {
            util::logVerbose("wait failed...");
            continue;
        }

        // handle zygote
        if(pid == initPid)
        {
            util::logVerbose("init stopped, examining signal...");

            siginfo_t sig;
            int ret = ptrace(PTRACE_GETSIGINFO, initPid, NULL, &sig);
            if(ret == -1)
            {
                util::logVerbose("Failed to get siginfo from init: %s",
                        strerror(errno));
                break;
            }

            if(sig.si_pid == zygote)
            {
                util::logVerbose("Init got signaled about zygote, swallowing");
                int ret = ptrace(PTRACE_CONT, initPid, NULL, NULL);
                if(ret == -1)
                {
                    util::logError("Failed to continue init: %s", strerror(errno));
                    throw std::system_error(errno, std::system_category());
                }
            }
            else
            {
                util::logVerbose("Init received something not interresting");
            }
        }
        else if(pid == zygote)
        {
            if(WIFEXITED(status))
            {
                util::logVerbose("Zygote exited...");
                break;
            }
            else
            {
                util::logError("Bug detected, zygote shouldn't signal us, "
                        "status: %d", status);
                break;
            }
        }
        // handle zygotes forks
        else if(status >> 16 == PTRACE_EVENT_FORK)
        {
            util::logVerbose("Zygote forked new child with pid %d", pid);
            endTrace(pid);
        }
        else
        {
            util::logError("Bug detected, something else happened, status %d",
                    status);
            break;
        }
    }
}

void signalHandler(int signum)
{
    if(signum == SIGINT || signum == SIGTERM)
    {
        util::logVerbose("SIGINT/SIGTERM catched, shutting down...");
        shouldRun = false;
        return;
    }

    util::logError("Catched signal '%d' which shouldn't lang here", signum);
}

void setupSignalHandling()
{
    util::logVerbose("Setting up signal handlers...");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;

    int ret = sigaction(SIGINT, &sa, NULL);
    if(ret == -1)
    {
        util::logError("Failed to setup SIGINT handler: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    ret = sigaction(SIGTERM, &sa, NULL);
    if(ret == -1)
    {
        util::logError("Failed to setup SIGTERM handler: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}


int main(int argc, char** argv)
{
    setupSignalHandling();

    util::logVerbose("Start tracing init...");
    startTrace(initPid);

    // TODO handle zygote crash (reconnect on zygote failure)
    try
    {

        zygotePid = getZygotePid();
        util::logVerbose("Zygote pid: %d", zygotePid);

        startTrace(zygotePid);
        util::logVerbose("Zygote forks are now traced");

        setupChildTrace(zygotePid);
        runMainLoop(zygotePid);
    }
    catch(std::exception& e)
    {
        util::logError("Failed: %s", e.what());
        return -1;
    }

    // TODO tracing of child should also be ended here
    endTrace(initPid);
    endTrace(zygotePid);

    return 0;
}
