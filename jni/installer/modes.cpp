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
#include <system_error>

#include "shared/util.h"
#include "hash.h"
#include "operations.h"
#include "mark.h"
#include "config.h"
#include "compression.h"

namespace modes {

ReturnCode install(const std::string& libpath, const std::string& apkpath)
{
    // BEWARE This operation is damn critical - if we overwrite the library
    //        with the our copy operation it will truncate the file and this
    //        will result in a full system crash as EVERY process has our lib
    //        loaded. We have to make sure the library is still around for the
    //        processes which are using it.

    // check if there is an install mark
    if(mark::exists())
    {
        util::logError("Already installed, will not overwrite");
        return FAIL;
    }

    struct stat origst;
    struct stat libst;

    try
    {
        // stat app_process to recreate files with correct rights
        // as they may differ from system (depends on version/vendor)
        operations::stat(config::origBinary, origst);
        // stat libandroid.so to have mode,uid,gid to set on our lib
        operations::stat(config::libandroid, libst);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to stat needed files: %s", e.what());
        throw;
    }

    // copy our lib
    try
    {
        try
        {
            // We have to unlink the library before we overwrite it. The chance
            // to overwrite it here is very low but when it happens it will
            // break the whole device as everything will crash and we can't
            // recover from that (did that multiple times to my phone)...
            operations::unlink(config::library);
        }
        catch(std::exception& e)
        {
            util::logError("Failed to remove previous installed lib: %s",
                    e.what());
        }

        operations::copy(libpath, config::library);
        operations::chown(config::library, libst.st_uid, libst.st_gid);
        operations::chmod(config::library, libst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to copy libanjaroot.so into place, reverting");
        uninstall();
        throw;
    }

    // make sure source and destination lib have matching crc32 sums
    bool libEqual = hash::CRC32::compare(libpath, config::library);
    if(!libEqual)
    {
        util::logError("Library CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // create a backup of the original app_process;
    try
    {
        operations::copy(config::origBinary, config::backupBinary);
        operations::chown(config::backupBinary, origst.st_uid, origst.st_gid);
        operations::chmod(config::backupBinary, origst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to create app_process backup, reverting");
        uninstall();
        throw;
    }

    // make sure backup and orig binary have the same crc32 sum
    bool binEqual = hash::CRC32::compare(config::origBinary,
            config::backupBinary);
    if(!binEqual)
    {
        util::logError("CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // prepare the wrapper
    try
    {
        operations::writeFile(config::tmpBinary, config::content);
        operations::chown(config::tmpBinary, origst.st_uid, origst.st_gid);
        operations::chmod(config::tmpBinary, origst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to create wrapper script, reverting");
        uninstall();
        throw;
    }

    // move everything into the right place
    try
    {
        operations::move(config::origBinary, config::newBinary);
        operations::move(config::tmpBinary, config::origBinary);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to move wrapper in place, reverting");
        uninstall();
        throw;
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

    bool apkEqual = hash::CRC32::compare(apkpath, config::apkSystemPath);
    if(!apkEqual)
    {
        util::logError("CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // copy installer to /system/bin/
    try
    {
        operations::copy("/proc/self/exe", config::installerBinary);
        operations::chown(config::installerBinary, origst.st_uid,
                origst.st_gid);
        operations::chmod(config::installerBinary, origst.st_mode);
    }
    catch(std::exception& e)
    {
        util::logError("Failed to install anjarootinstaller, reverting");
        uninstall();
        throw;
    }

    bool installerEqual = hash::CRC32::compare(config::installerBinary,
            "/proc/self/exe");
    if(!installerEqual)
    {
        util::logError("CRC32 sums differ, reverting");
        uninstall();
        return FAIL;
    }

    // final CRC32 checks, again we may have failed hard. While that's pretty
    // unlikely, we are fiddeling with the system core here. It's worth to have
    // another safety check to be sure we don't mess the device up.
    // TODO: a CRC32 check for our wrapper script would be nice also
    bool newEqual = hash::CRC32::compare(config::backupBinary, config::newBinary);
    if(!newEqual)
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
    try
    {
        operations::unlink(config::library);
        util::logVerbose("Removed %s", config::library.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove library: %s", e.what());
    }

    try
    {
        operations::unlink(config::tmpBinary);
        util::logVerbose("Removed %s", config::tmpBinary.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove tmp wrapper script: %s", e.what());
    }

    try
    {
        operations::move(config::newBinary, config::origBinary);
        util::logVerbose("Moved %s back to %s", config::newBinary.c_str(),
                config::origBinary.c_str());
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
        operations::unlink(config::installerBinary);
        util::logVerbose("Removed %s", config::installerBinary.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove installer: %s", e.what());
    }

    try
    {
        operations::unlink(config::installMark);
        util::logVerbose("Removed %s", config::installMark.c_str());
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

    if(!mark::verify())
    {
        util::logError("Failed to verify install mark, we may be broken!");
        return FAIL;
    }
    return OK;
}

ReturnCode recoveryInstall(const std::string& apkpath)
{
    try
    {
        operations::mkdir("/cache/recovery/", 0770);
        operations::chmod("/cache/recovery/", 0770); // dir may already be existing
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
    int ret = operations::reboot(true);

    return ret != 0 ? FAIL : OK;
}

}
