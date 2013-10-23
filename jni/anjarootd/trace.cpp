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
#include <linux/user.h>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#include "trace.h"
#include "shared/util.h"

trace::Tracee::Tracee(pid_t pid_) : pid(pid_)
{
}

trace::Tracee::~Tracee()
{
    detach();
}

pid_t trace::Tracee::getPid() const
{
    return pid;
}

bool trace::Tracee::detach() const
{
    int ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to deattach from %d: %s", pid, strerror(errno));
        return false;
    }

    util::logVerbose("Detached from %d", pid);
    return true;
}

void trace::Tracee::restart() const
{
    int ret = ptrace(PTRACE_CONT, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to continue %d: %s", pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void trace::Tracee::setupChildTrace() const
{
    int ret = ptrace(PTRACE_SETOPTIONS, pid, NULL,
            reinterpret_cast<void*>(PTRACE_O_TRACEFORK));
    if(ret == -1)
    {
        util::logError("Failed to setup fork tracing on %d: %s",
                pid, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

unsigned long trace::Tracee::getEventMsg() const
{
    unsigned long result;
    int ret = ptrace(PTRACE_GETEVENTMSG, pid, NULL, &result);
    if(ret == -1)
    {
        util::logError("Failed to get event msg from %d: %s", pid,
                strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return result;
}

trace::WaitResult::WaitResult(pid_t pid_, int status_) : pid(pid_),
    status(status_)
{
}

void trace::WaitResult::logDebugInfo() const
{
    util::logError("PID: %d STATUS: %d EVENT: %d", pid, status, getEvent());
    util::logError("WIFEXITED: %d WEXITSTATUS: %d WIFSIGNALED: %d",
            hasExited(), getExitStatus(), wasSignaled());
    util::logError("WTERMSIG: %d WCOREDUMP: %d WIFSTOPPED: %d WSTOPSIG: %d",
            getTermSignal(), wasCoredumped(), hasStopped(), getStopSignal());
    WIFEXITED(status), WEXITSTATUS(status), WIFSIGNALED(status), WTERMSIG(status), WCOREDUMP(status), WIFSTOPPED(status), WSTOPSIG(status);
}

pid_t trace::WaitResult::getPid() const
{
    return pid;
}

bool trace::WaitResult::hasExited() const
{
    return WIFEXITED(status);
}

int trace::WaitResult::getExitStatus() const
{
    return WEXITSTATUS(status);
}

bool trace::WaitResult::wasSignaled() const
{
    return WIFSIGNALED(status);
}

int trace::WaitResult::getTermSignal() const
{
    return WTERMSIG(status);
}

bool trace::WaitResult::wasCoredumped() const
{
    return WCOREDUMP(status);
}

bool trace::WaitResult::hasStopped() const
{
    return WIFSTOPPED(status);
}

int trace::WaitResult::getStopSignal() const
{
    return WSTOPSIG(status);
}

int trace::WaitResult::getEvent() const
{
    return status >> 16;
}

trace::Tracee trace::attach(pid_t pid)
{
    util::logVerbose("Attaching to %d...", pid);
    int ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if(ret == -1)
    {
        util::logError("Failed to attach to process: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return Tracee(pid);
}

trace::WaitResult trace::waitChilds()
{
    int status;
    pid_t pid = wait(&status);
    return WaitResult(pid, status);
}
