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

#include "zygotechildhandler.h"
#include "hook.h"
#include "shared/util.h"

ZygoteChildHandler::ZygoteChildHandler()
{
}

ZygoteChildHandler::~ZygoteChildHandler()
{
    util::logVerbose("Detaching from zygote children...");
    std::for_each(childs.begin(), childs.end(),
            [] (trace::Tracee::Ptr x) { x->detach(); });
}

bool ZygoteChildHandler::handle(const trace::WaitResult& res)
{
    // first find out if we already know about this child
    auto found = searchChildByPid(res.getPid());
    if(found == childs.end())
    {
        if(res.getStopSignal() == SIGSTOP)
        {
            // TODO: someone stopped we don't know about till now? Maybe we can
            // check who's the parent of the child to decide if we should attach
            // to it or not. We could parse proc, but that's ugly as hell...
            pid_t newpid = res.getPid();

            util::logVerbose("SIGSTOP for untracked child %d received, "
                    "starting trace", newpid);

            trace::Tracee::Ptr child = std::make_shared<trace::Tracee>(newpid);
            child->setupSyscallTrace();
            child->waitForSyscallResume();
            childs.push_back(child);
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

        found->get()->detach();
        childs.erase(found);
        return true;
    }

    if(res.wasSignaled())
    {
        util::logVerbose("Zygote child received termination signal: %d",
                res.getTermSignal());

        found->get()->detach();
        childs.erase(found);
        return true;
    }

    if(res.inSyscall())
    {
        bool detach = hook::performHookActions(*found);
        if(detach)
        {
            found->get()->detach();
            childs.erase(found);
        }
        else
        {
            found->get()->waitForSyscallResume();
        }

        return true;
    }

    if(res.hasStopped())
    {
        util::logVerbose("Zygote child received stop signal %d, deliver it",
                res.getStopSignal());
        found->get()->resume(res.getStopSignal());
        return true;
    }

    util::logError("Bug detected in handleZygoteChild");
    res.logDebugInfo();
    return false;
}

trace::Tracee::Ptr ZygoteChildHandler::getChildByPid(pid_t pid)
{
    auto found = searchChildByPid(pid);
    if(found != childs.end())
    {
        return *found;
    }

    return NULL;
}

void ZygoteChildHandler::removeChildByPid(pid_t pid)
{
    auto found = searchChildByPid(pid);
    if(found != childs.end())
    {
        found->get()->detach();
        childs.erase(found);
    }
}

trace::Tracee::List::iterator ZygoteChildHandler::searchChildByPid(pid_t pid)
{
    auto comperator = [=] (trace::Tracee::Ptr tracee)
    { return tracee->getPid() == pid; };
    return std::find_if(childs.begin(), childs.end(), comperator);
}

