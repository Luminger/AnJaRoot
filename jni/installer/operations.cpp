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

#include <fstream>
#include <sstream>
#include <system_error>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "operations.h"
#include "util.h"

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

void sync()
{
    util::logVerbose("Op: sync");
    ::sync();
}

}
