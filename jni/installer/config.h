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

#ifndef _ANJAROOT_INSTALLER_CONFIG_H_
#define _ANJAROOT_INSTALLER_CONFIG_H_

#include <string>

// well, this is just a better namespace...
struct config {
    static const std::string installMark;
    static const std::string libandroid;
    static const std::string library;
    static const std::string tmpBinary;
    static const std::string newBinary;
    static const std::string backupBinary;
    static const std::string origBinary;
    static const std::string content;
};

#endif
