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
#include <system_error>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include "anjarootdaemon.h"
#include "trace.h"
#include "shared/util.h"
#include "shared/version.h"

bool shouldRun = true;

void signalHandler(int signum)
{
    // Zygote doesn't use the android logging methods in its signal handler, so
    // we don't do either. It's just too risky...
    if(signum == SIGINT || signum == SIGTERM)
    {
        shouldRun = false;
        return;
    }
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

// TODO move to AnJaRootDaemon class
uid_t getUidFromPid(uid_t pid)
{
    // There is no real "official" and/or good way, so we just stat the
    // /proc/<pid> directory which is owned by the process.
    char path[32] = {0, };
    snprintf(path, sizeof(path), "/proc/%d", pid);

    struct stat st;
    int ret = stat(path, &st);
    if(ret == -1)
    {
        util::logError("Failed to stat %s: %s", path, strerror(errno));
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

int main(int argc, char** argv)
{
    setupSignalHandling();

    util::logVerbose("AnJaRootDaemon (version %s) started",
            version::asString().c_str());

    // TODO perform a daemonize
    // TODO we only want to run once, ensure that
    // TODO handle zygote crash (reconnect on zygote failure)
    try
    {
        pid_t zygotePid = getZygotePid();
        trace::Tracee::Ptr zygote = trace::attach(zygotePid);

        trace::WaitResult res = trace::waitChild(zygotePid);
        if(!res.hasStopped() || res.getStopSignal() != SIGSTOP)
        {
            util::logError("Failed to wait for zygote");
            res.logDebugInfo();
            return -2;
        }

        zygote->setupChildTrace();
        zygote->resume();

        util::logVerbose("Attached to zygote (pid: %d)", zygotePid);
        AnJaRootDaemon(zygote).run(shouldRun);
    }
    catch(std::exception& e)
    {
        util::logError("Failed: %s", e.what());
        return -1;
    }

    return 0;
}
