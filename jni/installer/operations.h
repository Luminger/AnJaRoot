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

#ifndef _ANJAROOT_INSTALLER_OPERATIONS_H
#define _ANJAROOT_INSTALLER_OPERATIONS_H

#include <istream>
#include <string>
#include <sys/types.h>

namespace operations {
    void move(const std::string& src, const std::string& dst);
    void copy(const std::string& src, const std::string& dst);
    void unlink(const std::string& src);
    void stat(const std::string& target, struct stat& out);
    void chown(const std::string& target, uid_t uid, gid_t gid);
    void chmod(const std::string& target, mode_t mode);
    void sync();
}

#endif
