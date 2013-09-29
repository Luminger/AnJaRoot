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

/* Assumptions:
 *  - The caller takes care of calling "mount -o rw,remount /system"
 *    and its inversion after we have finished here
 *  - The caller takes care of granting us enough rights, we don't
 *    check for root or any elected state, we just do our business.
 */

#include <iostream>
#include <getopt.h>
#include <stdlib.h>

#include "shared/util.h"
#include "shared/version.h"

#include "installer.h"
#include "modes.h"

const char* shortopts = "s:icurvh";
const struct option longopts[] = {
    {"srclibpath",   required_argument, 0, 's'},
    {"install",      no_argument,       0, 'i'},
    {"check",        no_argument,       0, 'c'},
    {"uninstall",    no_argument,       0, 'u'},
    {"version",      no_argument,       0, 'v'},
    {"help",         no_argument,       0, 'h'},
    {0, 0, 0, 0},
};

void printUsage(const char* progname)
{
    std::cerr << "Usage: " << progname << " [OPTIONS] [MODE]" << std::endl;
    std::cerr << std::endl << "Valid Options:" << std::endl;
    std::cerr << "\t-h, --help\t\t\tprint this usage message" << std::endl;
    std::cerr << "\t-v, --version\t\t\tprint version" << std::endl;
    std::cerr << "\t-s, --srclibpath [PATH] \tset source lib path" << std::endl;
    std::cerr << std::endl << "Valid Modes:" << std::endl;
    std::cerr << "\t-i, --install\t\t\tdo install (needs -s to be set)" << std::endl;
    std::cerr << "\t-u, --uninstall\t\t\tdo uninstall" << std::endl;
    std::cerr << "\t-c, --check\t\t\tdo an installation ckeck" << std::endl;
}

ModeSpec processArguments(int argc, char** argv)
{
    std::string sourcelib;
    modes::OperationMode mode = modes::InvalidMode;

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
                mode = modes::InstallMode;
                break;
            case 'c':
                util::logVerbose("Opt: -c");
                mode = modes::CheckMode;
                break;
            case 'u':
                util::logVerbose("Opt: -u");
                mode = modes::UninstallMode;
                break;
            case 'v':
                util::logVerbose("opt: -v");
                mode = modes::VersionMode;
                return std::make_pair("", modes::VersionMode);
            case 'h':
                util::logVerbose("opt: -h");
                return std::make_pair("", modes::HelpMode);
            default:
                return std::make_pair("", modes::InvalidMode);
        }
    }

    return std::make_pair(sourcelib, mode);
}

int main(int argc, char** argv)
{
    util::logVerbose("Installer (version %s) started",
            version::asString().c_str());

    ModeSpec spec = processArguments(argc, argv);

    switch(spec.second)
    {
        case modes::VersionMode:
            std::cout << "Version: " << version::asString() << std::endl;
            return 0;
        case modes::HelpMode:
            printUsage(argv[0]);
            return 0;
        case modes::InvalidMode:
            printUsage(argv[0]);
            util::logError("Called with wrong/insufficient arguments");
            return -1;
        default:
            break;
    }

    modes::ReturnCode ret = modes::FAIL;
    try
    {
        if(spec.second == modes::InstallMode)
        {
            util::logVerbose("Running install mode");
            ret = modes::install(spec.first);
        }
        else if(spec.second == modes::UninstallMode)
        {
            util::logVerbose("Running uninstall mode");
            ret = modes::uninstall();
        }
        else if(spec.second == modes::CheckMode)
        {
            util::logVerbose("Running check mode");
            ret = modes::check();
        }
    }
    catch(std::exception& e)
    {
        // TODO: use google-breakpad to create a dump here
        util::logError("Failed while executing mode: %s", e.what());
        ret = modes::FAIL;
    }

    if(ret == modes::OK)
    {
        const char* msg = "Positive status returned from execution";
        std::cout << msg << std::endl;
        util::logVerbose(msg);
    } else {
        const char* msg = "Negativ status returned from execution";
        std::cerr << msg << std::endl;
        util::logError(msg);
    }

    util::logVerbose("Installer finished");
    return ret == modes::OK ? 0 : 1;
}
