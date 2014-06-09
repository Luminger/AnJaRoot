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

#include <ctime>

#include "shared/util.h"

#include "mark.h"
#include "config.h"
#include "hash.h"
#include "operations.h"

namespace mark {

bool write()
{
    try
    {
        hash::CRC32 crc(config::installedLibraryPath);
        operations::writeFile(config::installMarkPath, crc.toString());
    }
    catch(std::exception& e)
    {
        util::logError("Failed to write mark file: %s", e.what());
        return false;
    }
    return true;
}

bool verify()
{
    try
    {
        std::string content = operations::readFile(config::installMarkPath);
        hash::CRC32 crc(config::installedLibraryPath);
        return crc.toString() == content;
    }
    catch(std::exception& e)
    {
        util::logError("Failed to verify mark file: %s", e.what());
        return false;
    }
    return true;
}

bool exists()
{
    bool exists = operations::access(config::installMarkPath, F_OK);
    if(!exists)
    {
        util::logVerbose("Mark file %s doesn't exist",
                config::installMarkPath.c_str());
        return false;
    }

    struct stat st;
    try
    {
        operations::stat(config::installMarkPath, st);
        const time_t created = st.st_ctime;
        util::logVerbose("Mark exists, created on: %s", ctime(&created));
        return true;
    }
    catch(std::exception& e)
    {
        util::logError("Couldn't stat mark file: %s", e.what());
        return false;
    }
}

}
