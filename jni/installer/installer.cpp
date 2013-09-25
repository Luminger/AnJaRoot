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

/* Well, I would normally all of the operations performed by this tool in a
 * simple shell script. But all the Android devices out there may or may not
 * have a certain tool (cp is sometimes not there, people use cat to emulate
 * it, how ugly is that...) it seems to be the safest way to just code this in
 * c++ as the bionic libc provides all the stuff we need (copy, mv, chmod,
 * chown and more)
 *
 * Another good thing is that we can use this installer binary also to perform
 * a recovery install as liblog.so and libc.so are also present there. And even
 * if that's not the case (for liblog.so) we could link it statically, all
 * problems solved at once.
 */

/* How it would look like as a shellscript (without error handling):
 *
 * #!/bin/sh
 *
 * # pre install
 * mount -o rw,remount /system/
 *
 * # install library
 * cp libanjaroothook.so /system/lib/
 * # would be better to get perms and owner from an library like libc.so
 * chown 0:0 /system/lib/libanjaroothook.so
 * chmod 644 /system/lib/libanjaroothook.so
 *
 * # install app_process wrapper
 * cp app_process_wrapper.sh /system/bin/app_process_wrapper
 * # would be better to just copy perms and owner from orig binary
 * chown 0:0 /system/bin/app_process_wrapper
 * chmod 755 /system/bin/app_process_wrapper
 * mv /system/bin/app_process /system/bin/app_process.orig
 * mv /system/bin/app_process_wrapper /system/bin/app_process
 *
 * # cleanup
 * mount -o ro,remount /system/
 * sync
 */

/* Assumptions:
 *  - The caller takes care of calling "mount -o rw,remount /system"
 *    and its inversion after we have finished here
 *  - The caller takes care of granting us enough rights, we don't
 *    check for root or any elected state, we just do our business.
 */

#include <iostream>
#include <getopt.h>
#include <stdlib.h>

#include "installer.h"
#include "modes.h"
#include "util.h"

const char* shortopts = "s:icurh";
const struct option longopts[] = {
    {"srclibpath",   required_argument, 0, 's'},
    {"install",      no_argument,       0, 'i'},
    {"check",        no_argument,       0, 'c'},
    {"uninstall",    no_argument,       0, 'u'},
    {"repair",       no_argument,       0, 'r'},
    {"help",         no_argument,       0, 'h'},
    {0, 0, 0, 0},
};


ModeSpec processArguments(int argc, char** argv)
{
    std::string sourcelib;
    OperationMode mode = InvalidMode;

    int c, option_index = 0;
    while(true)
    {
        c = getopt_long (argc, argv, shortopts, longopts, &option_index);
        if(c == -1)
        {
            break;
        }

        switch(c)
        {
            case 's':
                util::logVerbose("Opt: -s set to '%s'", optarg);
                sourcelib = optarg;
                break;
            case 'i':
                util::logVerbose("Opt: -i");
                mode = InstallMode;
                break;
            case 'c':
                util::logVerbose("Opt: -c");
                mode = CheckMode;
                break;
            case 'u':
                util::logVerbose("Opt: -u");
                mode = UninstallMode;
                break;
            case 'r':
                util::logVerbose("opt: -r");
                mode = RepairMode;
                break;
            case 'h':
            default:
                return std::make_pair("", HelpMode);
        }
    }

    return std::make_pair(sourcelib, mode);
}

void printUsage(const char* progname)
{
    std::cerr << "Usage: " << progname << " [OPTIONS] [MODE]" << std::endl;
    std::cerr << std::endl << "Valid Options:" << std::endl;
    std::cerr << "\t-h, --help\t\t\tprint this usage message" << std::endl;
    std::cerr << "\t-s, --srclibpath [PATH] \tset source lib path" << std::endl;
    std::cerr << std::endl << "Valid Modes:" << std::endl;
    std::cerr << "\t-i, --install\t\t\tdo install (needs -s to be set)" << std::endl;
    std::cerr << "\t-u, --uninstall\t\t\tdo uninstall" << std::endl;
    std::cerr << "\t-r, --repair\t\t\tdo repair" << std::endl;
    std::cerr << "\t-c, --check\t\t\tdo an installation ckeck" << std::endl;
}

int main(int argc, char** argv)
{
    util::logVerbose("Installer started");

    ModeSpec spec = processArguments(argc, argv);
    if(spec.second == HelpMode)
    {
        printUsage(argv[0]);
        util::logError("Called with wrong/insufficient arguments");
        return -1;
    }

    try
    {
        if(spec.second == InstallMode)
        {
            util::logVerbose("Running install mode");
            modes::install(spec.first);
        }
        else if(spec.second == UninstallMode)
        {
            util::logVerbose("Running uninstall mode");
            modes::uninstall();
        }
        else if(spec.second == CheckMode)
        {
            util::logVerbose("Running check mode");
            modes::check();
        }
        else if(spec.second == RepairMode)
        {
            util::logVerbose("Running repair mode");
            modes::repair();
        }
    }
    catch(std::exception& e)
    {
        // TODO log a stacktrace (with backtrace from <execinfo.h>?)
        util::logError("Failed while executing mode: %s", e.what());
        return -1;
    }

    util::logVerbose("Installer finished");
    return 0;
}
