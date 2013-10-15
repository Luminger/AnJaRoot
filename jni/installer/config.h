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
    static const std::string installMarkPath;
    static const std::string libandroidPath;
    static const std::string installedLibraryPath;
    static const std::string temporaryAppProcessPath;
    static const std::string newAppProcessPath;
    static const std::string backupAppProcessPath;
    static const std::string originalAppProcessPath;
    static const std::string installerPath;
    static const std::string apkSystemPath;
    static const std::string wrapperScriptContent;
};

#endif
