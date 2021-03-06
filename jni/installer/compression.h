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

#ifndef _ANJAROOT_INSTALLER_COMPRESSION_H
#define _ANJAROOT_INSTALLER_COMPRESSION_H

#include <string>
#include "minizip/unzip.h"

namespace compression {
    class Unzip
    {
        public:
            Unzip(const std::string& file);
            ~Unzip();

            void extractFileTo(const std::string& file, const std::string& to);

        private:
            static const size_t BufferSize = 256; // That's a wild guess,
                                                  // but should work for now
            unzFile zip;
    };
}

#endif
