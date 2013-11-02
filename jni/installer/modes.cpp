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

#include "modes.h"

#include <errno.h>
#include <fstream>
#include <sstream>
#include <system_error>

#include "shared/util.h"
#include "hash.h"
#include "operations.h"
#include "mark.h"
#include "config.h"
#include "compression.h"

namespace modes {

ReturnCode install(const std::string& libpath, const std::string& daemonpath,
        const std::string& apkpath)
{
    util::logVerbose("Running install mode");

    // check if there is an install mark
    if(mark::exists())
    {
        util::logError("Already installed, will not overwrite");
        return FAIL;
    }

    // this install process will break when libpath is identical to its final
    // place, refuse to install if this happens
    if(libpath == config::installedLibraryPath)
    {
        util::logError("Provided sourcelibpath is identical to the final "
                "place in the system. Move it somewhere else!");
        return FAIL;
    }

    // same for daemonpath...
    if(daemonpath == config::originalDebuggerdPath)
    {
        util::logError("Provided daemonpath is identical to the final "
                "place in the system. Move it somewhere else!");
        return FAIL;
    }

    // ...and the apk
    if(apkpath == config::apkSystemPath)
    {
        util::logError("Provided apkpath is identical to the final "
                "place in the system. Move it somewhere else!");
        return FAIL;
    }

    struct stat origst;
    struct stat libst;

    try
    {
        // stat debuggerd to recreate files with correct rights
        // as they may differ from system (depends on version/vendor)
        operations::stat(config::originalDebuggerdPath, origst);
        // stat libandroid.so to have mode,uid,gid to set on our lib
        operations::stat(config::libandroidPath, libst);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to stat needed files: %s", e.what());
        throw;
    }

    // copy libanjaroot.so
    try
    {
        try
        {
            // We have to unlink the library before we overwrite it. The chance
            // to overwrite it here is very low but when it happens it will
            // break the whole device as everything will crash and we can't
            // recover from that (did that multiple times to my phone)...
            //
            // As described above, this should not happen. But better save than
            // sorry, just try to prevent major pain for the user.
            operations::unlink(config::installedLibraryPath);
        }
        catch(std::exception& e)
        {
            util::logError("Failed to remove previous installed lib: %s",
                    e.what());
        }

        operations::copy(libpath, config::installedLibraryPath);
        operations::chown(config::installedLibraryPath, libst.st_uid,
                libst.st_gid);
        operations::chmod(config::installedLibraryPath, libst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to copy libanjaroot.so into place, reverting");
        uninstall();
        throw;
    }

    // make sure source and destination lib have matching crc32 sums
    bool libEqual = hash::CRC32::compareFiles(libpath,
            config::installedLibraryPath);
    if(!libEqual)
    {
        util::logError("Library CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // move debuggerd away so we can place our daemon
    try
    {
        operations::move(config::originalDebuggerdPath,
                config::newDebuggerdPath);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to move debuggerd away, reverting");
        uninstall();
        throw;
    }

    // copy daemon to /system/bin/
    try
    {
        operations::copy(daemonpath, config::originalDebuggerdPath);
        operations::chown(config::originalDebuggerdPath, origst.st_uid,
                origst.st_gid);
        operations::chmod(config::originalDebuggerdPath, origst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to install anjarootd, reverting");
        uninstall();
        throw;
    }

    // make sure original anjarootd and its copy have matching crc32 sums
    bool daemonEqual = hash::CRC32::compareFiles(daemonpath,
            config::originalDebuggerdPath);
    if(!daemonEqual)
    {
        util::logError("anjarootd CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // copy apk to /system/apk/
    try
    {
        operations::copy(apkpath, config::apkSystemPath);
        operations::chmod(config::apkSystemPath, 0644);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to install AnJaRoot.apk, reverting");
        uninstall();
        throw;
    }

    bool apkEqual = hash::CRC32::compareFiles(apkpath, config::apkSystemPath);
    if(!apkEqual)
    {
        util::logError("CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // copy installer to /system/bin/
    try
    {
        operations::copy("/proc/self/exe", config::installerPath);
        operations::chown(config::installerPath, origst.st_uid, origst.st_gid);
        operations::chmod(config::installerPath, origst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to install anjarootinstaller, reverting");
        uninstall();
        throw;
    }

    bool installerEqual = hash::CRC32::compareFiles(config::installerPath,
            "/proc/self/exe");
    if(!installerEqual)
    {
        util::logError("CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // place the install mark
    bool markPlaced = mark::write();
    if(!markPlaced)
    {
        util::logError("Failed to place install mark, reverting");
        uninstall();
        return FAIL;
    }

    // we are done, sync to disk - just to be sure
    operations::sync();

    return OK;
}

ReturnCode uninstall()
{
    // BEWARE: don't let an exception escape here as there is already one
    //         active if we come from modes::instal! Catch and handle them!

    util::logVerbose("Running uninstall mode");

    try
    {
        operations::unlink(config::installedLibraryPath);
        util::logVerbose("Removed %s", config::installedLibraryPath.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove library: %s", e.what());
    }

    try
    {
        operations::move(config::newDebuggerdPath,
                config::originalDebuggerdPath);
        util::logVerbose("Moved %s back to %s",
                config::newDebuggerdPath.c_str(),
                config::originalDebuggerdPath.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove new binary: %s", e.what());
    }

    try
    {
        operations::unlink(config::apkSystemPath);
        util::logVerbose("Removed %s", config::apkSystemPath.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove system apk: %s", e.what());
    }

    try
    {
        operations::unlink(config::installerPath);
        util::logVerbose("Removed %s", config::installerPath.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove installer: %s", e.what());
    }

    try
    {
        operations::unlink(config::installMarkPath);
        util::logVerbose("Removed %s", config::installMarkPath.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove install mark: %s", e.what());
    }

    // just to be sure everything goes to disk
    operations::sync();

    return OK;
}

ReturnCode check()
{
    // We are just checking for the install mark here. This is more or less a
    // good source for checking if we are in a good state but otherwise we
    // would have to verify every binary and the wrapper script and it may be
    // quite hard to do so, think of version updates etc.

    util::logVerbose("Running check mode");

    if(!mark::verify())
    {
        util::logError("Failed to verify install mark, we may be broken!");
        return FAIL;
    }
    return OK;
}

ReturnCode recoveryInstall(const std::string& apkpath)
{
    util::logVerbose("Running recovery install mode");

    try
    {
        operations::mkdir("/cache/recovery/", 0770);
        operations::chmod("/cache/recovery/", 0770); // dir may already exist
        operations::chown("/cache/recovery/", "system", "cache");
    }
    catch(std::exception& e)
    {
        util::logError("Failed to create or alter '/cache/recovery'");
        return FAIL;
    }

    try
    {
        compression::Unzip zip(apkpath);
        zip.extractFileTo("assets/AnJaRoot.zip", "/cache/AnJaRoot.zip");
        zip.extractFileTo("assets/command", "/cache/recovery/command");

        operations::chmod("/cache/AnJaRoot.zip", 0644);
        operations::chmod("/cache/recovery/command", 0644);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to prepare recovery install: %s", e.what());
        return FAIL;
    }

    operations::sync();

    return OK;
}

ReturnCode rebootIntoRecovery()
{
    util::logVerbose("Booting into recovery");

    int ret = operations::reboot(true) ? FAIL : OK;
    return ret != 0 ? FAIL : OK;
}

ReturnCode rebootSystem()
{
    util::logVerbose("Rebooting system");

    int ret = operations::reboot(false) ? FAIL : OK;
    return ret != 0 ? FAIL : OK;
}

}
