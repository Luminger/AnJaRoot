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
#include <string.h>
#include <sys/ptrace.h>

#include "shared/util.h"
#include "../hook.h"

int hook::getSyscallNumber(trace::Tracee::Ptr tracee)
{
    // This function is a copy from strace (syscall.c), adopted to our needs.
    if(tracee->isSyscallBegin())
    {
        tracee->setSyscallBegin(false);
        return -1;
    }

    errno = 0;
    long syscallnum = ptrace(PTRACE_PEEKUSER, tracee->getPid(),
            (void*)(4*ORIG_EAX), NULL);
    if(errno)
    {
        util::logError("Failed to get eax, err %d: %s", errno,
                strerror(errno));
    }

    tracee->setSyscallBegin(true);
    return syscallnum;
}

bool hook::changePermittedCapabilities(trace::Tracee::Ptr tracee)
{
    // ebx holds the addr of the cap_user_header_t*, we don't care
    // about it here - we naivly trust that the syscall would succeed.
    // ecx holds the addr of the cap_user_data_t*, which is defined
    // as (on every supported arch):
    //
    // typedef struct __user_cap_data_struct {
    //     __u32 effective;
    //     __u32 permitted;
    //     __u32 inheritable;
    // } *cap_user_data_t;
    //
    errno = 0;
    long dataaddr = ptrace(PTRACE_PEEKUSER, tracee->getPid(), (void*)(4*ECX),
            NULL);
    if(errno)
    {
        util::logError("Failed to get ecx, err %d: %s", errno,
                strerror(errno));
        return false;
    }

    long ret = ptrace(PTRACE_POKEDATA, tracee->getPid(),
            (void*)(dataaddr + sizeof(__u32)), (void*)0xFFFFFEFF);
    if(ret == -1)
    {
        util::logError("Failed to set permitted value, err %d: %s", errno,
                strerror(errno));
        return false;
    }

    return true;
}
