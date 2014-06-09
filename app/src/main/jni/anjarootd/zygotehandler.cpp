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

#include <errno.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "zygotehandler.h"
#include "shared/util.h"

ZygoteHandler::ZygoteHandler(ZygoteChildHandler& childhandler_) :
    childhandler(childhandler_)
{
    // both methods will throw if something is wrong
    pid_t zygotePid = getZygotePid();
    zygote = trace::attach(zygotePid);
    util::logVerbose("Attached to zygote (pid: %d)", zygotePid);
}

ZygoteHandler::~ZygoteHandler()
{
    util::logVerbose("Detaching from zygote...");
    zygote->detach();
}

pid_t ZygoteHandler::getZygotePid() const
{
    // So... we could iterate through /proc/ to find a process named zygote and
    // read one of the status files where the format is not guaranteed to stay
    // stable in different kernel releases... Or we could grab the zygote socket
    // from /dev/socket/zygote and ask the socket for the remote pid!
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

pid_t ZygoteHandler::getPid() const
{
    return zygote->getPid();
}

bool ZygoteHandler::handle(const trace::WaitResult& res)
{
    if(res.hasExited())
    {
        util::logVerbose("Zygote exited...");
        zygote->detach();
        return false;
    }

    if(res.wasSignaled())
    {
        util::logVerbose("Zygote received termination signal: %d",
                res.getTermSignal());

        zygote->detach();
        return false;
    }

    if(res.getEvent() == PTRACE_EVENT_FORK)
    {
        pid_t newpid = zygote->getEventMsg();
        util::logVerbose("Zygote has forked a new child: %d", newpid);

        zygote->resume();
        return true;
    }

    if(res.hasStopped())
    {
        if(res.getStopSignal() == SIGCHLD)
        {
            siginfo_t siginfo = zygote->getSignalInfo();
            // well, that's not always correct - SIGCHLDs aren't only send on
            // child death, but for now we assume that - it's easier
            util::logVerbose("Zygote received SIGCHILD for %d", siginfo.si_pid);
            childhandler.removeChildByPid(siginfo.si_pid);
            zygote->resume(res.getStopSignal());
        }
        else if(res.getStopSignal() == SIGSTOP)
        {
            util::logVerbose("Zygote received SIGSTOP, seting up child trace");
            zygote->setupChildTrace();
            zygote->resume();
        }
        else
        {
            util::logVerbose("Zygote resumed with signal %d", res.getStopSignal());
            zygote->resume(res.getStopSignal());
        }

        return true;
    }

    util::logError("Bug detected in handleZygote");
    res.logDebugInfo();
    return false;
}
