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
#include <stdio.h>

#include "util.h"

static std::ofstream logstream;

namespace util {

void log(android_LogPriority prio, const char* format, va_list vargs)
{
    if(logstream.is_open())
    {
        char buf[1024];
        std::size_t size = vsnprintf(buf, sizeof(buf), format, vargs);
        if(size > 0)
        {
            char timestr[128] = {0, };
            time_t now;
            struct tm* tinfo;

            time(&now);
            tinfo = localtime(&now);

            strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%S%z", tinfo);

            std::stringstream strstream;
            strstream << "[" << timestr << "][" << prio << "] " << buf <<
                std::endl;

            logstream << strstream.str();
        }
    }
    else
    {
        __android_log_vprint(prio, ANJAROOT_LOGTAG, format, vargs);
    }
}

void logError(const char* format, ...)
{
    va_list vargs;
    va_start(vargs, format);

    log(ANDROID_LOG_ERROR, format, vargs);

    va_end(vargs);
}

void logVerbose(const char* format, ...)
{
    va_list vargs;
    va_start(vargs, format);

    log(ANDROID_LOG_VERBOSE, format, vargs);

    va_end(vargs);
}

void setupFileLogging(const char* file)
{
    // TODO a logrotate would be cool, otherwise we have to truncate...
    logstream.open(file, std::ios::trunc);
    // disable buffering, we would loose data, it's static and never closed
    logstream.rdbuf()->pubsetbuf(0, 0);
}

}
