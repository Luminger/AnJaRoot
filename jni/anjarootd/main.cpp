/*
 * Copyright 2013 Simon Brakhane
 *
 * This file is part of AnJaRoot.
 *
 * AnJaRoot is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * AnJaRoot is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * AnJaRoot. If not, see http://www.gnu.org/licenses/.
 */
#include <algorithm>
#include <system_error>

#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "trace.h"
#include "shared/util.h"

bool shouldRun = true;

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

uid_t getUidFromPid(uid_t pid)
{
    // There is no real "official" and/or good way, so we just stat the
    // /proc/<pid> directory which is owned by the process.
    std::string path = "/proc/" + pid;

    struct stat st;
    int ret = stat(path.c_str(), &st);
    if(ret == -1)
    {
        util::logError("Failed to stat %s: %s", path.c_str(), strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return st.st_uid;
}

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
        util::logError("Failed to connect to zygote socket: %s",
                strerror(errno));
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

bool handleZygote(trace::Tracee::List& forks, const trace::WaitResult& res,
        trace::Tracee::Ptr zygote)
{
    if(res.hasExited())
    {
        util::logVerbose("Zygote exited...");
        return false;
    }

    if(res.hasStopped())
    {
        util::logVerbose("Zygote stopped");

        if(res.getEvent() == PTRACE_EVENT_FORK)
        {
            pid_t newpid = zygote->getEventMsg();
            util::logVerbose("Zygote has forked a new child: %d", newpid);
            res.logDebugInfo();

            trace::Tracee::Ptr child = trace::Tracee::Ptr(
                    new trace::Tracee(newpid));
            forks.push_back(child);
            zygote->resume();
        }
        else if(res.getStopSignal() == SIGCHLD)
        {
            siginfo_t siginfo = zygote->getSignalInfo();

            util::logVerbose("SIGCHLD from %d for zygote received, deliver it",
                    siginfo.si_pid);

            auto comperator = [&] (trace::Tracee::Ptr tracee)
            { return tracee->getPid() == res.getPid(); };
            trace::Tracee::List::iterator found = std::find_if(forks.begin(),
                    forks.end(), comperator);

            if(found != forks.end())
            {
                forks.erase(found);
            }

            zygote->resume(SIGCHLD);
        }
        else
        {
            util::logError("Bug detected, zygote shouldn't get stoped without "
                    "child");
            res.logDebugInfo();
            return false;
        }
    }

    return true;
}

bool handleTracee(trace::Tracee::List::iterator tracee,
        trace::Tracee::List& forks, const trace::WaitResult& res)
{
    util::logVerbose("Zygote child %d waited...", res.getPid());

    if(res.hasExited())
    {
        util::logVerbose("Zygote child exited");
        res.logDebugInfo();

        forks.erase(tracee);
        return true;
    }
    else if(res.inSyscall())
    {
        util::logVerbose("Zygote child signaled syscall");

        forks.erase(tracee);

        return true;
    }
    else if(res.getStopSignal() == SIGSTOP)
    {
        util::logVerbose("Zygote child was stopped by SIGSTOP");
        tracee->get()->waitForSyscallResume();
        return true;
    }
    else if(res.wasSignaled() && res.getTermSignal() != 0)
    {
        util::logVerbose("Zygote Child received term signal %d, detaching",
                res.getTermSignal());

        auto comperator = [&] (trace::Tracee::Ptr tracee)
        { return tracee->getPid() == res.getPid(); };
        trace::Tracee::List::iterator found = std::find_if(forks.begin(),
                forks.end(), comperator);

        if(found != forks.end())
        {
            forks.erase(found);
            return true;
        }

        return false;
    }
    else if(res.wasSignaled() && res.getStopSignal() != 0)
    {
        util::logVerbose("Zygote Child received stop signal %d, deliver it",
                res.getStopSignal());
        tracee->get()->resume(res.getStopSignal());
        return true;
    }
    else
    {
        util::logError("Bug detected in handleTracee, something else happened");
        res.logDebugInfo();
        return true;
    }
}

void runMainLoop(trace::Tracee::Ptr zygote, trace::Tracee::List& forks)
{
    while(shouldRun)
    {
        util::logVerbose("Waiting for child action...");
        trace::WaitResult res = trace::waitChilds();

        if(res.getPid() == -1)
        {
            util::logVerbose("wait failed...");
            res.logDebugInfo();
            continue;
        }

        bool handled = true;
        // handle zygote
        if(res.getPid() == zygote->getPid())
        {
            handled = handleZygote(forks, res, zygote);
        }
        else if(res.getStopSignal() == SIGSTOP)
        {
            util::logVerbose("Someone with pid %d stopped, ignore him",
                    res.getPid());
            res.logDebugInfo();
            handled = true;
        }
        else
        {
            // handle (possible) zygotes fork stops
            auto comperator = [&] (trace::Tracee::Ptr tracee)
            { return tracee->getPid() == res.getPid(); };
            trace::Tracee::List::iterator found = std::find_if(forks.begin(),
                    forks.end(), comperator);

            if(found != forks.end())
            {
                handled = handleTracee(found, forks, res);
            }
            else
            {
                util::logError("Bug detected in mainloop, something else happened");
                res.logDebugInfo();
                break;
            }
        }

        if(handled)
        {
            continue;
        }
        break;
    }
}

int main(int argc, char** argv)
{
    setupSignalHandling();

    trace::Tracee::Ptr zygote;
    trace::Tracee::List zygoteForks;

    // TODO handle zygote crash (reconnect on zygote failure)
    try
    {
        pid_t zygotePid = getZygotePid();
        util::logVerbose("Zygote pid: %d", zygotePid);

        zygote = trace::attach(zygotePid);
        trace::WaitResult res = trace::waitChilds();

        if(res.getPid() != zygote->getPid())
        {
            util::logError("Failed to wait for zygote");
            return -1;
        }

        zygote->setupChildTrace();
        util::logVerbose("Zygote forks are now traced");
        zygote->resume();

        runMainLoop(zygote, zygoteForks);
    }
    catch(std::exception& e)
    {
        util::logError("Failed: %s", e.what());
        return -1;
    }

    return 0;
}
