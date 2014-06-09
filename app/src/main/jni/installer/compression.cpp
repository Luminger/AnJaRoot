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
#include <stdexcept>

#include "compression.h"
#include "shared/util.h"

namespace compression {

Unzip::Unzip(const std::string& file)
{
    zip = unzOpen(file.c_str());
    if(!zip)
    {
        std::string msg = "Failed to open '" + file + "' as zipfile!";
        util::logError(msg.c_str());
        throw std::runtime_error(msg);
    }
}

Unzip::~Unzip()
{
    unzClose(zip);
}

void Unzip::extractFileTo(const std::string& file, const std::string& to)
{
    int ret = unzLocateFile(zip, file.c_str(), 1);
    if(ret != UNZ_OK)
    {
        std::string msg = "Failed to locate '" + file + "'!";
        util::logError(msg.c_str());
        throw std::runtime_error(msg);
    }

    ret = unzOpenCurrentFile(zip);
    if(ret != UNZ_OK)
    {
        std::string msg = "Failed to open '" + file + "'!";
        util::logError(msg.c_str());
        throw std::runtime_error(msg);
    }

    int read = 0;
    char buf[BufferSize];
    try
    {
        std::ofstream stream(to, std::ios::trunc | std::ios::binary);

        while((read = unzReadCurrentFile(zip, buf, BufferSize)))
        {
            stream.write(buf, read);
        }

        stream.close();
        unzCloseCurrentFile(zip);
    }
    catch(std::exception& e)
    {
        unzCloseCurrentFile(zip);
        util::logError("Failed to extract %s to %s: %s", file.c_str(),
                to.c_str(),  e.what());
        throw;
    }
}

}
