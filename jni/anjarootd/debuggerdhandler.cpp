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

#include <unistd.h>

#include "debuggerdhandler.h"
#include "shared/util.h"

const char* DebuggerdHandler::executableName = "debuggerd.orig";

DebuggerdHandler::DebuggerdHandler() : pid(0)
{
    pid = fork();
    if(pid > 0)
    {
        execlp(executableName, executableName, NULL);

        // if we land here, exec failed...
        util::logError("Failed to exec %d: %s", errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
    else if(pid == -1)
    {
        util::logError("Failed to fork %d: %s", errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    util::logVerbose("Spawned debuggerd with pid %d", pid);
}

DebuggerdHandler::~DebuggerdHandler()
{
    if(pid < 1)
    {
        util::logVerbose("Can't kill debuggerd - we have no pid");
        return;
    }

    int ret = kill(pid, SIGTERM);
    if(ret == -1)
    {
        util::logError("Failed to send SIGTERM to debuggerd, pid %d, %d: %s",
                pid, errno, strerror(errno));
    }
}

pid_t DebuggerdHandler::getPid() const
{
    return pid;
}

bool DebuggerdHandler::handle(const trace::WaitResult& res)
{
    util::logError("debuggerd exited");
    res.logDebugInfo();
    return false;
}
