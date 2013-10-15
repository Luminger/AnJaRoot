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

#include "config.h"

const std::string config::installMarkPath =
        "/system/etc/.anjaroot.install.mark";
const std::string config::libandroidPath = "/system/lib/libandroid.so";
const std::string config::installedLibraryPath = "/system/lib/libanjaroot.so";
const std::string config::temporaryAppProcessPath =
        "/system/bin/app_process.anjaroot";
const std::string config::newAppProcessPath =
        "/system/bin/app_process.anjaroot";
const std::string config::backupAppProcessPath =
        "/system/bin/app_process.backup.anjaroot";
const std::string config::originalAppProcessPath = "/system/bin/app_process";
const std::string config::installerPath = "/system/bin/anjarootinstaller";
const std::string config::apkSystemPath = "/system/app/AnJaRoot.apk";
const std::string config::wrapperScriptContent =
        "#!/system/bin/sh\n\n"
        "LD_PRELOAD=/system/lib/libanjaroot.so /system/bin/app_process_orig $@";
