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

    long syscallnum = -1;
    struct pt_regs regs;
    long ret = ptrace(PTRACE_GETREGS, tracee->getPid(), NULL, (void *)&regs);
    if(ret == -1)
    {
        util::logError("Failed to get registers, err %d: %s", errno,
                strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    // we are only interested in syscall entries
    if(regs.ARM_ip == 0)
    {
        tracee->setSyscallBegin(true);

        if (regs.ARM_cpsr & 0x20)
        {
            // Get the Thumb-mode system call number
            syscallnum = regs.ARM_r7;
        }
        else
        {
            //Get the ARM-mode system call number
            errno = 0;
            syscallnum = ptrace(PTRACE_PEEKTEXT, tracee->getPid(),
                    (void *)(regs.ARM_pc - 4), NULL);
            if(errno)
            {
                util::logError("Failed to get registers, err %d: %s", errno,
                        strerror(errno));
                throw std::system_error(errno, std::system_category());
            }

            if(syscallnum == 0xef000000)
            {
                syscallnum = regs.ARM_r7;
            }
            else
            {
                if ((syscallnum & 0x0ff00000) != 0x0f900000) {
                    util::logError("unknown syscall trap 0x%08lx\n",
                            syscallnum);
                    throw std::system_error(syscallnum,
                            std::generic_category());
                }

                // Fixup the syscall number
                syscallnum &= 0x000fffff;
            }
        }

        if(syscallnum & 0x0f0000)
        {
            // Handle ARM specific syscall
            syscallnum &= 0x0000ffff;
        }

        return syscallnum;
    }

    // signal syscall exit, we should never end here
    util::logError("Reached end of getSyscallNumber(), shouldn't happen");
    return -1;
}

bool hook::changePermittedCapabilities(trace::Tracee::Ptr tracee)
{
    // First we need to read (again) the registers.
    struct pt_regs regs;
    long ret = ptrace(PTRACE_GETREGS, tracee->getPid(), NULL, (void *)&regs);
    if(ret == -1)
    {
        util::logError("Failed to get registers, err %d: %s", errno,
                strerror(errno));
        return false;
    }

    // res.uregs[0] holds the addr of the cap_user_header_t*, we don't care
    // about it here - we naivly trust that the syscall would succeed.
    // res.uregs[1] holds the addr of the cap_user_data_t*, which is defined
    // as (on every supported arch):
    //
    // typedef struct __user_cap_data_struct {
    //     __u32 effective;
    //     __u32 permitted;
    //     __u32 inheritable;
    // } *cap_user_data_t;
    //
    long dataaddr = regs.uregs[1];
    ret = ptrace(PTRACE_POKEDATA, tracee->getPid(),
            (void*)(dataaddr + sizeof(__u32)), (void*)0xFFFFFEFF);
    if(ret == -1)
    {
        util::logError("Failed to set permitted value, err %d: %s", errno,
                strerror(errno));
        return false;
    }

    return true;
}
