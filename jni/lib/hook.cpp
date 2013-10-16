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
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared/util.h"
#include "shared/version.h"

#include "packages.h"
#include "helper.h"

static const helper::Capabilities rootCapabilities(0xFFFFFFFF, 0xFFFFFFFF, 0);
static const helper::UserIds rootUids(0, 0, 0);
static const helper::GroupIds rootGids(0, 0, 0);
static bool executedHookedFunction = false;

typedef int (*capset_type)(cap_user_header_t, const cap_user_data_t);
static capset_type orig_capset;

 __attribute__((constructor (65535)))
static void constructor()
{
    util::logVerbose("AnJaRoot %s loaded", version::asString().c_str());

    orig_capset = reinterpret_cast<capset_type>(dlsym(RTLD_NEXT, "capset"));
    if(orig_capset != NULL)
    {
        util::logVerbose("Found original capset()");
        return;
    }

    util::logError("Failed to get original capset(), abort!");
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
    util::logVerbose("capset() called");
    if(executedHookedFunction)
    {
        util::logVerbose("unhooked capset() called");
        return orig_capset(hdrp, datap);
    }

    util::logVerbose("hooked capset() called");
    executedHookedFunction = true;

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
            util::logVerbose("Running original capset() to restore defaults");
            return orig_capset(hdrp, datap);
        }

        // restore orig uids/guids (except suid/sgid, we need them to regain
        // access later), permissions are granted
        origGids.sgid = 0;
        origUids.suid = 0;
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
