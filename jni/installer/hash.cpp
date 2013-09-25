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

#include <sstream>

#include "hash.h"

namespace hash {

CRC32::CRC32()
{
    initialize();
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

}
