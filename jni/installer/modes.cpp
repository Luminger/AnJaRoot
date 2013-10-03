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

#include <dlfcn.h>
#include <errno.h>
#include <fstream>
#include <system_error>
#include <sys/reboot.h>

#include "shared/util.h"
#include "hash.h"
#include "operations.h"
#include "mark.h"
#include "config.h"
#include "compression.h"

namespace modes {

ReturnCode install(const std::string& libpath)
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
        operations::move(config::backupBinary, config::origBinary);
        util::logVerbose("Moved %s back to %s", config::backupBinary.c_str(),
                config::origBinary.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to restore app_process backup: %s", e.what());
        try
        {
            operations::unlink(config::backupBinary);
            util::logVerbose("Removed %s", config::backupBinary.c_str());
        }
        catch(std::exception& e)
        {
            util::logError("Failed to remove app_process backup: %s", e.what());
        }
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
        operations::unlink(config::newBinary);
        util::logVerbose("Removed %s", config::newBinary.c_str());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to remove new binary: %s", e.what());
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

    // Now the hack: libcutils.so has a function (android_reboot.c) to reboot
    // the device into recovery mode. Unfortunately this is not part of the ndk
    // to we try a manual load here. If that failed (Gingerbread doesn't know
    // this function) we fall back to __reboot() which does what it should BUT
    // I'm lazy so we skip the "wait till every mount is ro" phase. Shouldn't
    // do that much harm as we have done a sync() anyway here.
    void* libcutils = dlopen("libcutils.so", RTLD_LAZY);
    if(!libcutils)
    {
        util::logError("Failed to open libcutils.so via dlopen()!");
        return FAIL;
    }

    typedef int (*android_reboot_type)(int cmd, int flags, char* arg);
    android_reboot_type android_reboot = reinterpret_cast<android_reboot_type>(
            dlsym(libcutils, "android_reboot"));

    int ret;
    char cmd[] = "recovery";
    if(!android_reboot)
    {
        util::logError("Failed to resolve symbol, doing legacy reboot.");
        // Direct copy from JB's libcutils/android_reboot.c
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
                LINUX_REBOOT_CMD_RESTART2, cmd);
    }
    else
    {
        // Value stolen from: include/cutils/android_reboot.h
        int ANDROID_RB_RESTART2 = 0xDEAD0003;
        ret = android_reboot(ANDROID_RB_RESTART2, 0, cmd);

    }

    dlclose(libcutils);
    if(ret)
    {
        util::logError("Failed to reboot into recovery!");
        return FAIL;
    }

    return OK;
}

}
