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

#include "shared/util.h"

#include "helper.h"
#include "syscallfix.h"

namespace helper {

UserIds getUserIds()
{
    UserIds uids;

    int ret = local_getresuid(&uids.ruid, &uids.euid, &uids.suid);
    if(ret != 0)
    {
        util::logError("getresuid failed: errno=%d, err=%s",
                errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    util::logVerbose("getUserIds: ruid=%d, euid=%d, suid=%d", uids.ruid,
            uids.euid, uids.suid);

    return uids;
}

void setUserIds(const UserIds& uids)
{
    util::logVerbose("setUserIds: ruid=%d, euid=%d, suid=%d", uids.ruid,
            uids.euid, uids.suid);

    int ret = setresuid(uids.ruid, uids.euid, uids.suid);
    if(ret == EPERM)
    {
        util::logError("setresuid failed: EPERM");
        throw std::system_error(ret, std::system_category());
    }
    else if(ret != 0)
    {
        util::logError("setresuid failed: errno=%d, err=%s",
                errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

GroupIds getGroupIds()
{
    GroupIds gids;

    int ret = local_getresgid(&gids.rgid, &gids.egid, &gids.sgid);
    if(ret != 0)
    {
        util::logError("getresgid failed: errno=%d, err=%s",
                errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    util::logVerbose("getGroupIds: rgid=%d, egid=%d, sgid=%d", gids.rgid,
            gids.egid, gids.sgid);

    return gids;
}

void setGroupIds(const GroupIds& gids)
{
    util::logVerbose("setGroupIds: rgid=%d, egid=%d, sgid=%d", gids.rgid,
            gids.egid, gids.sgid);

    int ret = setresgid(gids.rgid, gids.egid, gids.sgid);
    if(ret != 0)
    {
        util::logError("setresgid failed: errno=%d, err=%s",
                errno, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

}
