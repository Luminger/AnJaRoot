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

#ifndef _ANJAROOTD_ZYGOTEHANDLER_H_
#define _ANJAROOTD_ZYGOTEHANDLER_H_

#include "trace.h"
#include "zygotechildhandler.h"

class ZygoteHandler
{
    public:
        ZygoteHandler(ZygoteChildHandler& childhandler_);
        ~ZygoteHandler();

        pid_t getPid() const;
        bool handle(const trace::WaitResult& res);

    private:
        pid_t getZygotePid() const;

        trace::Tracee::Ptr zygote;
        ZygoteChildHandler& childhandler;
};

#endif
