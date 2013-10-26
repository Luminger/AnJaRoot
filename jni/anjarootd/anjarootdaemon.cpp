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

#include <sys/ptrace.h>

#include "anjarootdaemon.h"
#include "shared/util.h"

AnJaRootDaemon::AnJaRootDaemon(trace::Tracee::Ptr zygote_) : zygote(zygote_)
{
}

AnJaRootDaemon::~AnJaRootDaemon()
{
    zygote->detach();
    std::for_each(zygoteForks.begin(), zygoteForks.end(),
            [] (trace::Tracee::Ptr x) { x->detach(); });
}
trace::Tracee::List::iterator AnJaRootDaemon::searchTracee(pid_t pid)
{
    auto comperator = [=] (trace::Tracee::Ptr tracee)
    { return tracee->getPid() == pid; };
    return std::find_if(zygoteForks.begin(), zygoteForks.end(), comperator);
}


void AnJaRootDaemon::run(const bool& shouldRun)
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
            handled = handleZygote(res);
        }
        else
        {
            // handle (possible) zygotes fork stops
            trace::Tracee::List::iterator found = searchTracee(res.getPid());
            if(found != zygoteForks.end())
            {
                handled = handleKnownZygoteChild(res, found);
            }
            else if(res.getStopSignal() == SIGSTOP)
            {
                // someone stopped we don't know about till now
                util::logVerbose("Someone unknown stopped, add to internal list");
                res.logDebugInfo();

                trace::Tracee::Ptr child = trace::Tracee::Ptr(
                        new trace::Tracee(res.getPid()));
                child->setupSyscallTrace();
                zygoteForks.push_back(child);

                handled = true;
            }
            else
            {
                util::logError("Bug detected in mainloop, something else happened");
                res.logDebugInfo();
                handled = false;
            }
        }

        if(handled)
        {
            continue;
        }
        break;
    }
}

bool AnJaRootDaemon::handleZygote(const trace::WaitResult& res)
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

            trace::Tracee::List::iterator found =searchTracee(newpid);
            if(found != zygoteForks.end())
            {
                // We have already received the SIGSTOP, continue the child
                util::logVerbose("Zygote delivered fork event for known child");
                found->get()->resume();
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
        }
        else if(res.getStopSignal() == SIGCHLD)
        {
            siginfo_t siginfo = zygote->getSignalInfo();

            util::logVerbose("SIGCHLD from %d for zygote received, deliver it",
                    siginfo.si_pid);

            // TODO well, is that correct? We should check if the child has
            // really exited, don't we?
            trace::Tracee::List::iterator found = searchTracee(res.getPid());
            if(found != zygoteForks.end())
            {
                found->get()->detach();
                zygoteForks.erase(found);
            }

            zygote->resume(SIGCHLD);
        }
        // TODO deliver all other signals also to the zygote
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

// TODO rename to handleKnownZygoteChild
bool AnJaRootDaemon::handleKnownZygoteChild(const trace::WaitResult& res,
        trace::Tracee::List::iterator tracee)
{
    util::logVerbose("Zygote child %d waited...", res.getPid());

    if(res.hasExited())
    {
        util::logVerbose("Zygote child exited");
        res.logDebugInfo();

        tracee->get()->detach();
        zygoteForks.erase(tracee);
        return true;
    }
    else if(res.inSyscall())
    {
        util::logVerbose("Zygote child signaled syscall");

        tracee->get()->detach();
        zygoteForks.erase(tracee);

        return true;
    }
    else if(res.getStopSignal() == SIGSTOP)
    {
        util::logVerbose("Zygote child was stopped by SIGSTOP");

        // we don't know if we have already setup the syscall tracing here
        tracee->get()->setupSyscallTrace();
        tracee->get()->waitForSyscallResume();
        return true;
    }
    else if(res.wasSignaled() && res.getTermSignal() != 0)
    {
        util::logVerbose("Zygote Child received term signal %d, detaching",
                res.getTermSignal());

        trace::Tracee::List::iterator found = searchTracee(res.getPid());
        if(found != zygoteForks.end())
        {
            found->get()->detach();
            zygoteForks.erase(found);
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

