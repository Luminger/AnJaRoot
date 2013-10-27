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

#include <asm/unistd.h>
#include <errno.h>
#include <sys/ptrace.h>

#include "anjarootdaemon.h"
#include "hook.h"
#include "shared/util.h"

AnJaRootDaemon::AnJaRootDaemon(trace::Tracee::Ptr zygote_) : zygote(zygote_)
{
}

AnJaRootDaemon::~AnJaRootDaemon()
{
    std::for_each(zygoteForks.begin(), zygoteForks.end(),
            [] (trace::Tracee::Ptr x) { x->detach(); });
    zygote->detach();
}

trace::Tracee::List::iterator AnJaRootDaemon::searchTracee(pid_t pid)
{
    auto comperator = [=] (trace::Tracee::Ptr tracee)
    { return tracee->getPid() == pid; };
    return std::find_if(zygoteForks.begin(), zygoteForks.end(), comperator);
}

void AnJaRootDaemon::run(const bool& shouldRun)
{
    bool handled = true;
    while(shouldRun && handled)
    {
        trace::WaitResult res = trace::waitChilds();
        if(res.getPid() == -1)
        {
            if(errno != EINTR)
            {
                util::logVerbose("Wait failed with %d: %s", errno,
                        strerror(errno));
                res.logDebugInfo();
            }

            handled = true;
        }
        else if(res.getPid() == zygote->getPid())
        {
            handled = handleZygote(res);
        }
        else
        {
            handled = handleZygoteChild(res);
        }
    }
}

bool AnJaRootDaemon::handleZygote(const trace::WaitResult& res)
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

        trace::Tracee::List::iterator found = searchTracee(newpid);
        if(found != zygoteForks.end())
        {
            // We have already received the SIGSTOP, continue the child
            util::logVerbose("Zygote delivered fork event for known child");
            found->get()->setupSyscallTrace();
            found->get()->waitForSyscallResume();
        }
        else
        {
            // Event received first, child will be continued in its handler
            util::logVerbose("Zygote delivered fork event for a unknown "
                    "child, will wait for its SIGSTOP");
            trace::Tracee::Ptr child = trace::Tracee::Ptr(
                    new trace::Tracee(newpid));
            zygoteForks.push_back(child);
        }

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
            trace::Tracee::List::iterator found = searchTracee(siginfo.si_pid);
            if(found != zygoteForks.end())
            {
                found->get()->detach();
                zygoteForks.erase(found);
            }
        }

        zygote->resume(res.getStopSignal());
        util::logVerbose("Zygote resumed with signal %d", res.getStopSignal());
        return true;
    }

    util::logError("Bug detected in handleZygote");
    res.logDebugInfo();
    return false;
}

bool AnJaRootDaemon::handleZygoteChild(const trace::WaitResult& res)
{
    // first find out if we already know about this child
    trace::Tracee::List::iterator tracee = searchTracee(res.getPid());
    if(tracee == zygoteForks.end())
    {
        if(res.getStopSignal() == SIGSTOP)
        {
            // TODO: someone stopped we don't know about till now? Maybe we can
            // check who's the parent of the child to decide if we should attach
            // to it or not
            util::logVerbose("Someone unknown stopped, add to internal list");

            trace::Tracee::Ptr child = trace::Tracee::Ptr(
                    new trace::Tracee(res.getPid()));
            child->setupSyscallTrace();
            zygoteForks.push_back(child);
            return true;
        }

        util::logError("Bug detected in handleZygoteChild (untracked child)");
        res.logDebugInfo();
        return true;
    }

    if(res.hasExited())
    {
        util::logVerbose("Zygote child exited with status: %d",
                res.getExitStatus());

        tracee->get()->detach();
        zygoteForks.erase(tracee);
        return true;
    }

    if(res.wasSignaled())
    {
        util::logVerbose("Zygote child received termination signal: %d",
                res.getTermSignal());

        tracee->get()->detach();
        zygoteForks.erase(tracee);
        return true;
    }

    if(res.inSyscall())
    {
        long syscallnum = hook::getSyscallNumber(*tracee);
        if(syscallnum == -1)
        {
            // this was a syscall exit, we don't care
            tracee->get()->waitForSyscallResume();
            return true;
        }
        else if(syscallnum == __NR_capset)
        {
            hook::changePermittedCapabilities(*tracee);

            tracee->get()->detach();
            zygoteForks.erase(tracee);
            return true;
        }
        else
        {
            tracee->get()->waitForSyscallResume();
            return true;
        }

        return true;
    }

    if(res.hasStopped())
    {
        if(res.getStopSignal() == SIGSTOP)
        {
            util::logVerbose("Zygote child was stopped by SIGSTOP");

            // we don't know if we have already setup the syscall tracing here
            tracee->get()->setupSyscallTrace();
            tracee->get()->waitForSyscallResume();
            return true;
        }

        util::logVerbose("Zygote child received stop signal %d, deliver it",
                res.getStopSignal());
        tracee->get()->resume(res.getStopSignal());
        return true;
    }

    util::logError("Bug detected in handleZygoteChild");
    res.logDebugInfo();
    return false;
}
