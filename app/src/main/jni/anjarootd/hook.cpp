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

#include <asm/unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "hook.h"
#include "packages.h"
#include "shared/util.h"

// functions which require plarform dependant code (like getSyscallNumber) are
// implemented in the arch-xxx directories and build as needed. Only
// independant stuff is placed here.
//

// TODO make this package name configureable at buildtime, people would want to
// change it to enable custom builds without source changes
const char* hook::GranterPackageName = "org.failedprojects.anjaroot";

uid_t hook::getUidFromPid(pid_t pid)
{
    // Why this works:
    //
    // If we take a look at dalvik_system_Zygote.cpp, within the dalvik
    // repository in vm/native, one can see that the forkAndSpecializeCommon
    // method does call set(res)uid before capset. So the uid already changed,
    // it's save to read it that way. We could also trace the syscall which
    // sets the uid in the new zygote child, but it's not worth all the
    // platform specific code one has to write. So we just read the uid from
    // /proc/<pid> as those files are owned by the process uid.
    char path[32] = {0, };
    snprintf(path, sizeof(path), "/proc/%d", pid);

    struct stat st;
    int ret = stat(path, &st);
    if(ret == -1)
    {
        util::logError("Failed to stat %s: %s", path, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return st.st_uid;
}

bool hook::isUidGranted(uid_t uid)
{
    packages::PackageList pkgs;
    const packages::Package* anjaroot = pkgs.findByName(GranterPackageName);

    if(anjaroot == NULL)
    {
        util::logError("Couldn't get anjaroot package");
        return false;
    }

    const packages::Package* target = pkgs.findByUid(uid);
    if(target == NULL)
    {
        util::logError("Couldn't get target package");
        return false;
    }

    packages::GrantedPackageList granter(*anjaroot);
    return granter.isGranted(*target);
}

// TODO we don't have logmsgs here on purpose, it would result in major
// spam with little information at all. A cmdline switch or environment
// variable would be nice to have for enabling otherwise disabled log lines
//
// we return true if the caller can now detach from the tracee
bool hook::performHookActions(trace::Tracee::Ptr tracee)
{
    long syscallnum = getSyscallNumber(tracee);
    if(syscallnum == -1)
    {
        // TODO create conditional logging for this case (syscall exit)
        return false;
    }

    if(syscallnum == __NR_capset)
    {
        uid_t uid = getUidFromPid(tracee->getPid());
        bool granted = isUidGranted(uid);
        if(granted)
        {
            util::logVerbose("Child with pid %d is a target, "
                    "changing capabilities", tracee->getPid());
            changePermittedCapabilities(tracee);
        }
        else
        {
            util::logVerbose("Child with pid %d is not a target, "
                    "no action performed", tracee->getPid());
        }

        return true;
    }

    return false;
}
