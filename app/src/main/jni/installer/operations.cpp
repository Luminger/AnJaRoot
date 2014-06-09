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

#include "operations.h"

#include <fstream>
#include <sstream>
#include <system_error>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <dlfcn.h>
#include <sys/reboot.h>

#include "shared/util.h"

namespace operations {

std::string readFile(const std::string& target)
{
    util::logVerbose("Op: readFile '%s'", target.c_str());

    try
    {
        std::stringstream out;
        std::ifstream stream(target);
        out << stream.rdbuf();

        return out.str();
    }
    catch(std::exception& e)
    {
        util::logError("Op: readFile failed: %s", e.what());
        throw;
    }
}

void writeFile(const std::string& target, const std::string& content)
{
    util::logVerbose("Op: writeFile '%s'", target.c_str());

    try
    {
        std::ofstream stream(target, std::ios::trunc);
        stream << content;
        stream.close();
    }
    catch(std::exception& e)
    {
        util::logError("Op: writeFile failed: %s", e.what());
        throw;
    }
}

void move(const std::string& src, const std::string& dst)
{
    util::logVerbose("Op: rename '%s' to '%s'", src.c_str(), dst.c_str());

    int ret = ::rename(src.c_str(), dst.c_str());
    if(ret == -1)
    {
        util::logError("Op: rename failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void copy(const std::string& src, const std::string& dst)
{
    util::logVerbose("Op: copy '%s' to '%s'", src.c_str(), dst.c_str());

    try
    {
        std::ifstream srcStream(src, std::ios::binary);
        std::ofstream dstStream(dst, std::ios::binary | std::ios::trunc);

        dstStream << srcStream.rdbuf();
    }
    catch(std::exception& e)
    {
        util::logError("Op: copy failed: %s", e.what());
        throw;
    }
}

void unlink(const std::string& target)
{
    util::logVerbose("Op: unlink '%s'", target.c_str());

    int ret = ::unlink(target.c_str());
    if(ret == -1)
    {
        util::logError("Op: unlink failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void stat(const std::string& target, struct stat& out)
{
    util::logVerbose("Op: stat on '%s'", target.c_str());

    int ret = ::stat(target.c_str(), &out);
    if(ret == -1)
    {
        util::logError("Op: stat failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void chown(const std::string& target, const std::string& user,
        const std::string& group)
{
    util::logVerbose("Op: chown on '%s' with user='%s', group='%s'",
            target.c_str(), user.c_str(), group.c_str());

    struct passwd* pwd = getpwnam(user.c_str());
    if(pwd == NULL)
    {
        util::logError("Op: getpwnam failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    struct group* grp = getgrnam(group.c_str());
    if(grp == NULL)
    {
        util::logError("Op: getgrnam failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    chown(target, pwd->pw_uid,  grp->gr_gid);
}

void chown(const std::string& target, uid_t uid, gid_t gid)
{
    util::logVerbose("Op: chown on '%s' with uid=%d, gid=%d", target.c_str(),
            uid, gid);

    int ret = ::chown(target.c_str(), uid, gid);
    if(ret == -1)
    {
        util::logError("Op: chown failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void chmod(const std::string& target, mode_t mode)
{
    util::logVerbose("Op: chmod on '%s' with mode=%o", target.c_str(), mode);

    int ret = ::chmod(target.c_str(), mode);
    if(ret == -1)
    {
        util::logError("Op: chmod failed: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void mkdir(const std::string& dir, mode_t mode)
{
    util::logVerbose("Op: mkdir '%s' with mode=%o", dir.c_str(), mode);

    int ret = ::mkdir(dir.c_str(), mode);
    if(ret == -1)
    {
        if(errno != EEXIST)
        {
            util::logError("Op: mkdir failed: %s", strerror(errno));
            throw std::system_error(errno, std::system_category());
        }
    }
}

int reboot(bool bootRecovery)
{
    util::logVerbose("Op: reboot with bootRecovery=%d", bootRecovery);

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
        return -1;
    }

    typedef int (*android_reboot_type)(int cmd, int flags, char* arg);
    android_reboot_type android_reboot = reinterpret_cast<android_reboot_type>(
            dlsym(libcutils, "android_reboot"));

    int ret;
    char cmd[] = "recovery";
    if(!android_reboot)
    {
        util::logVerbose("Failed to resolve symbol, doing legacy reboot.");

        // emulate androids reboot toolbox utility a bit
        sync();

        if(bootRecovery)
        {
            // Direct copy from JB's libcutils/android_reboot.c
            ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
                    LINUX_REBOOT_CMD_RESTART2, cmd);
        }
        else
        {
            ret = reboot(RB_AUTOBOOT);
        }
    }
    else
    {
        const int ANDROID_RB_RESTART2 = 0xDEAD0003;
        const int ANDROID_RB_RESTART = 0xDEAD0001;

        util::logVerbose("Using libcutils reboot method");
        if(bootRecovery)
        {
            // Value stolen from: include/cutils/android_reboot.h
            ret = android_reboot(ANDROID_RB_RESTART2, 0, cmd);
        }
        else
        {
           ret = android_reboot(ANDROID_RB_RESTART, 0, NULL);
        }

    }

    dlclose(libcutils);
    if(ret)
    {
        util::logError("Failed to reboot into recovery!");
    }

    return ret;
}

bool access(const std::string& target, int mode)
{
    util::logVerbose("Op: access on %s with mode=%d", target.c_str(), mode);

    int ret = ::access(target.c_str(), mode);

    // this check is way to simple (see manpage) but it's sufficient for our
    // needs (at least currently).
    if(ret != 0)
    {
        util::logError("access() failed with %d on %s: %s", errno,
                target.c_str(), strerror(errno));
    }

    return ret == 0;
}

void sync()
{
    util::logVerbose("Op: sync");
    ::sync();
}

}
