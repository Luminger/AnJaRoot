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

#ifndef _ANJAROOT_LIB_HELPER_H_
#define _ANJAROOT_LIB_HELPER_H_

#include <android/log.h>
#include <unistd.h>

namespace helper {
    struct Capabilities {
        Capabilities() : effective(0), permitted(0), inheritable(0) {};
        Capabilities(__u32 effective_, __u32 permitted_, __u32 inheritable_) :
            effective(effective_), permitted(permitted_),
            inheritable(inheritable_) {};

        __u32 effective;
        __u32 permitted;
        __u32 inheritable;
    };

    struct UserIds {
        UserIds() : ruid(0), euid(0), suid(0) {};
        UserIds(uid_t ruid_, uid_t euid_, uid_t suid_) : ruid(ruid_),
            euid(euid_), suid(suid_) {};

        uid_t ruid;
        uid_t euid;
        uid_t suid;
    };

    struct GroupIds {
        GroupIds() : rgid(0), egid(0), sgid(0) {};
        GroupIds(gid_t rgid_, gid_t egid_, gid_t sgid_) : rgid(rgid_),
            egid(egid_), sgid(sgid_) {};

        gid_t rgid;
        gid_t egid;
        gid_t sgid;
    };

    Capabilities getCapabilities(pid_t pid);
    void setCapabilities(const Capabilities& caps);

    UserIds getUserIds();
    void setUserIds(const UserIds& uids);

    GroupIds getGroupIds();
    void setGroupIds(const GroupIds& gids);
}

#endif
