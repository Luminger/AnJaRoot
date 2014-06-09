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

#ifndef _ANJAROOTD_ANJAROOTDAEMON_H_
#define _ANJAROOTD_ANJAROOTDAEMON_H_

#include <getopt.h>

class AnJaRootDaemon
{
    public:
        AnJaRootDaemon();
        ~AnJaRootDaemon();

        int run(int argc, char** argv);

    private:
        static const char* shortopts;
        static const option longopts[];
        static bool shouldRun;

        static void signalHandler(int signum);

        void printUsage(const char* progname) const;
        void processArguments(int argc, char** argv);
        void claimLockSocket() const;
        void setupSignalHandling() const;

        bool showVersion;
        bool showUsage;
};

#endif
