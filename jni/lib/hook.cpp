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

#include "hook.h"

#include <system_error>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared/util.h"

#include "packages.h"
#include "helper.h"

static const helper::Capabilities rootCapabilities(0xFFFFFFFF, 0xFFFFFFFF, 0);
static const helper::UserIds rootUids(0, 0, 0);
static const helper::GroupIds rootGids(0, 0, 0);

typedef int (*capset_type)(cap_user_header_t, const cap_user_data_t);
static capset_type orig_capset;
bool hook::Hooked= false;
bool hook::AlreadyRun = false;


 __attribute__((constructor))
static void constructor()
{
    orig_capset = reinterpret_cast<capset_type>(dlsym(RTLD_NEXT, "capset"));
    if(orig_capset != NULL)
    {
        hook::Hooked = true;
        util::logVerbose("Installed hook for capset()");
        return;
    }

    util::logError("Failed to get original capset() with RTLD_NEXT");
    orig_capset = reinterpret_cast<capset_type>(dlsym(RTLD_DEFAULT, "capset"));
    if(orig_capset != NULL)
    {
        util::logVerbose("Hook not installed");
        return;
    }

    util::logError("Failed to get original capset() with RTLD_DEFAULT");
    abort();
}

bool isGranted(uid_t uid)
{
    packages::PackageList pkgs;
    const packages::Package* anjaroot = pkgs.findByName("org.failedprojects.anjaroot");

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

int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
    if(hook::Hooked && hook::AlreadyRun)
    {
        util::logVerbose("hooked capset() called");

        // Set alreadyRun to false to prefent later abuse of our hook
        hook::AlreadyRun = false;
    } else {
        util::logVerbose("unhooked capset() called");
        return orig_capset(hdrp, datap);
    }

    try
    {
        helper::UserIds origUids = helper::getUserIds();
        helper::GroupIds origGids = helper::getGroupIds();

        // gain caps, otherwise we can't read the packages.list
        helper::setCapabilities(rootCapabilities);
        helper::setUserIds(rootUids);
        helper::setGroupIds(rootGids);

        bool granted = isGranted(origUids.ruid);
        if(!granted)
        {
            util::logVerbose("Process is not a target");
            helper::setGroupIds(origGids);
            helper::setUserIds(origUids);
            return orig_capset(hdrp, datap);
        }

        // restore orig uids/guids, permissions are granted
        helper::setGroupIds(origGids);
        helper::setUserIds(origUids);

        // TODO: Maybe we should add only certain capabilities to a process.
        util::logVerbose("Process is a target, granting capabilities");

        // capset call was successfull, indeed =)
        return 0;
    }
    catch(std::system_error& e)
    {
        // Well, that's... unfortunate...
        util::logError("Failed to run hooked capset: errno=%d, err=%s",
                e.code().value(), e.what());
        util::logError("Running original function to prevent damage");
        return orig_capset(hdrp, datap);
    }
}
