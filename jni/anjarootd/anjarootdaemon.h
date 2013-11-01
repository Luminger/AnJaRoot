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

#ifndef _ANJAROOTD_ANJAROOTDAEMON_H_
#define _ANJAROOTD_ANJAROOTDAEMON_H_

#include "trace.h"

class AnJaRootDaemon
{
    public:
        AnJaRootDaemon();
        ~AnJaRootDaemon();

        void run(const bool& shouldRun);

    private:
        pid_t getZygotePid() const;
        trace::Tracee::List::iterator searchTracee(pid_t pid);
        bool handleZygote(const trace::WaitResult& res);
        bool handleZygoteChild(const trace::WaitResult& res);

        trace::Tracee::Ptr zygote;
        trace::Tracee::List zygoteForks;
};

#endif
