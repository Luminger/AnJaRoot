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

#include "shared/util.h"

#include "hash.h"

namespace hash {

CRC32::CRC32()
{
    initialize();
}

CRC32::CRC32(const std::string& in) : CRC32()
{
    add(in);
}

CRC32::CRC32(std::istream& in) : CRC32()
{
    add(in);
}

void CRC32::initialize()
{
    crc = crc32(0, Z_NULL, 0);
}

void CRC32::reset()
{
    initialize();
}

void CRC32::add(const std::string& in)
{
    std::stringstream stream;
    stream << in;
    add(stream);
}

void CRC32::add(std::istream& in)
{
    // reset stream to begin, restore later
    std::streampos oldpos = in.tellg();
    in.seekg(0, in.beg);

    // local buffer which will be feeded to crc32()
    char buf[BufferSize];

    std::streamsize read = 0;
    while((read = in.readsome(buf, sizeof(buf))))
    {
        // Bwah... this cast is stupid... Anyway, a unsigned char (aka Bytef)
        // is not that far away from char...
        crc = crc32(crc, reinterpret_cast<Bytef*>(&buf), read);
    }

    // reset stream position
    in.seekg(oldpos, in.beg);
}

std::string CRC32::toString() const
{
    std::stringstream ss;
    ss << crc;
    return ss.str();
}

bool CRC32::compare(const std::string& left, const std::string& right)
{
    try
    {
        std::ifstream leftStream(left, std::ios::binary);
        std::ifstream rightStream(right, std::ios::binary);

        hash::CRC32 leftHash(leftStream);
        hash::CRC32 rightHash(rightStream);

        if(leftHash != rightHash)
        {
            util::logError("CRC32 sums of %s and %s differ", left.c_str(),
                    right.c_str());
            return false;
        }

        util::logVerbose("CRC32 sums of %s and %s are equal", left.c_str(),
                right.c_str());
        return true;
    }
    catch(std::exception& e)
    {
        util::logError("Couldn't calc CRC32 sums: %s", e.what());
        return false;
    }
}

bool CRC32::operator==(const CRC32& other) const
{
    return crc == other.crc;
}

bool CRC32::operator!=(const CRC32& other) const
{
    return crc != other.crc;
}

}
