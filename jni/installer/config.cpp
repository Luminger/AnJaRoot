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
const std::string config::originalDebuggerdPath = "/system/bin/debuggerd";
const std::string config::newDebuggerdPath = "/system/bin/debuggerd.orig";
const std::string config::installerPath = "/system/bin/anjarootinstaller";
const std::string config::apkSystemPath = "/system/app/AnJaRoot.apk";
