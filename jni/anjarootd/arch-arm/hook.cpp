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

#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>

#include "shared/util.h"
#include "../hook.h"

long hook::getSyscallNumber(pid_t pid)
{
    long syscallnum = -1;
    struct pt_regs regs;

    int ret = ptrace(PTRACE_GETREGS, pid, NULL, (void *)&regs);
    if(ret == -1)
    {
        util::logError("Failed to get registers, err %d: %s", errno, strerror(errno));
        return -1;
    }

    // we are only interested in syscall entries
    if(regs.ARM_ip == 0)
    {
        if (regs.ARM_cpsr & 0x20)
        {
            // Get the Thumb-mode system call number
            syscallnum = regs.ARM_r7;
        }
        else
        {
            //Get the ARM-mode system call number
            errno = 0;
            syscallnum = ptrace(PTRACE_PEEKTEXT, pid,
                    (void *)(regs.ARM_pc - 4), NULL);
            if(errno)
            {
                util::logError("Failed to get registers, err %d: %s", errno, strerror(errno));
                return -1;
            }

            if(syscallnum == 0xef000000)
            {
                syscallnum = regs.ARM_r7;
            }
            else
            {
                if ((syscallnum & 0x0ff00000) != 0x0f900000) {
                    util::logError("unknown syscall trap 0x%08lx\n", syscallnum);
                    return -1;
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

    // signal syscall exit
    return -2;
}

int hook::changePermittedCapabilities(pid_t pid)
{
    struct pt_regs regs;
    int ret = ptrace(PTRACE_GETREGS, pid, NULL, (void *)&regs);
    if(ret == -1)
    {
        util::logError("Failed to get registers, err %d: %s", errno, strerror(errno));
        return -1;
    }

    long dataaddr = regs.uregs[1];
    long permitted = ptrace(PTRACE_PEEKDATA, pid, (void*)(dataaddr + sizeof(__u32)), NULL);
    ptrace(PTRACE_POKEDATA, pid, (void*)(dataaddr + sizeof(__u32)), (void*)0xFFFFFFFF);

    return -1;
}
