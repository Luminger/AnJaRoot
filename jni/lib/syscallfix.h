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

#ifndef _ANJAROOT_LIB_SYSCALLFIX_H_
#define _ANJAROOT_LIB_SYSCALLFIX_H_

// Hack to suspress ugly compile warning (invalid suffix on literal)
#include <system_error>
#include <unistd.h>

extern "C" {

// getresuid and getresgid are defined with the wrong signatures in some old
// android versions, we have to fix them on our own with stolen code from the
// current bionic, slightly backported to the ndk
//
// Commit info which fixed it for reference
//
// bionic: Fix wrong prototype of system call getresuid/getresgid
// In bionic/libc/SYSCALLS.TXT, the prototypes of system call
// getresuid/getresgid are incorrect.
//
// According to man page, they should be:
//    int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
//    int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
//
// Change-Id: I676098868bb05a9e1fe45419b234cf397626fdad
// Signed-off-by: Jin Wei <wei.a.jin@intel.com>
// Signed-off-by: Jack Ren <jack.ren@intel.com>
// Signed-off-by: Bruce Beare <bruce.j.beare@intel.com>

// used by out asm functions to set the errno
int __local_set_errno(int n);

// defined in asm
int local_getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
int local_getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);

}

#endif
