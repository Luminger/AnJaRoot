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
#include <system_error>
#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>

#include "anjarootdaemon.h"
#include "debuggerdhandler.h"
#include "zygotehandler.h"
#include "zygotechildhandler.h"
#include "trace.h"
#include "shared/util.h"
#include "shared/version.h"

bool AnJaRootDaemon::shouldRun = true;
const char* AnJaRootDaemon::shortopts = "vh";
const struct option AnJaRootDaemon::longopts[] = {
    {"version",         no_argument,       0, 'v'},
    {"help",            no_argument,       0, 'h'},
    {0, 0, 0, 0},
};

AnJaRootDaemon::AnJaRootDaemon() : showVersion(false), showUsage(false)
{
}

AnJaRootDaemon::~AnJaRootDaemon()
{
}

void AnJaRootDaemon::printUsage(const char* progname) const
{
    std::cerr << "Usage: " << progname << " [OPTIONS]" << std::endl;
    std::cerr << std::endl << "Valid Options:" << std::endl;
    std::cerr << "\t-h, --help\t\t\tprint this usage message" << std::endl;
    std::cerr << "\t-v, --version\t\t\tprint version" << std::endl;
}

void AnJaRootDaemon::processArguments(int argc, char** argv)
{
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
            case 'v':
                util::logVerbose("opt: -v");
                showVersion = true;
                return;
            case 'h':
            default:
                util::logVerbose("opt: -h (or unknown)");
                showUsage = true;
                return;
        }
    }
}

void AnJaRootDaemon::signalHandler(int signum)
{
    // Zygote doesn't use the android logging methods in its signal handler, so
    // we don't do either. It's just too risky...
    if(signum == SIGINT || signum == SIGTERM)
    {
        shouldRun = false;
        return;
    }
}

void AnJaRootDaemon::setupSignalHandling() const
{
    util::logVerbose("Setting up signal handlers...");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = AnJaRootDaemon::signalHandler;
    sa.sa_flags = 0;

    int ret = sigaction(SIGINT, &sa, NULL);
    if(ret == -1)
    {
        util::logError("Failed to setup SIGINT handler: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    ret = sigaction(SIGTERM, &sa, NULL);
    if(ret == -1)
    {
        util::logError("Failed to setup SIGTERM handler: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }
}

void AnJaRootDaemon::claimLockSocket() const
{
    // Android hasn't any good scratch place for pidfile (like /var or /tmp),
    // maybe we could do this in the /cache partition, but I have mixed
    // feelings about it.
    //
    // So we don't write a PidFile to ensure we are alone, we use a Unix Domain
    // Socket for this. We just need to bind to it and leave it open till we
    // die (kernel will cleanup after us). Check for an already running
    // instance is simple: If the bind failes, there is already an instance,
    // otherwise we are the only instance now.
    //
    // We don't care about the socket after the bind call, it may be reused as
    // a communication channel later (if needed).
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd == -1)
    {
        util::logError("Failed to create lock socket: %s", strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    struct sockaddr_un addr = {0, };
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "\0anjarootd");

    int ret = bind(fd, reinterpret_cast<struct sockaddr*>(&addr),
            sizeof(addr.sun_family) + sizeof(addr.sun_path));
    if(ret == -1)
    {
        util::logError("Failed to bind to lock socket: %s", strerror(errno));
        close(fd);
        throw std::system_error(errno, std::system_category());
    }

    // we don't close the socket here on purpose, it's our lock!
}

int AnJaRootDaemon::run(int argc, char** argv)
{
    processArguments(argc, argv);
    if(showUsage)
    {
        printUsage(argv[0]);
        return 0;
    }

    if(showVersion)
    {
        std::cout << "Version: " << version::asString() << std::endl;
        return 0;
    }

    try
    {
        setupSignalHandling();
        claimLockSocket();
    }
    catch(std::exception& e)
    {
        util::logError("Failed to initialize: %s", e.what());
        return 1;
    }

    while(shouldRun)
    {
        try
        {
            ZygoteChildHandler zygoteChilds;
            ZygoteHandler zygote(zygoteChilds);
            DebuggerdHandler debuggerd;

            bool handled = true;
            while(shouldRun && handled)
            {
                trace::WaitResult res = trace::waitChilds();
                if(errno == ECHILD)
                {
                    util::logVerbose("We have no children :(");
                    res.logDebugInfo();
                    handled = false;
                }
                else if(errno == EINTR)
                {
                    util::logVerbose("We got interrupted in wait()");
                    handled = true;
                }
                else if(res.getPid() == zygote.getPid())
                {
                    handled = zygote.handle(res);
                }
                else if(res.getPid() == debuggerd.getPid())
                {
                    handled = debuggerd.handle(res);
                }
                else
                {
                    handled = zygoteChilds.handle(res);
                }
            }
        }
        catch(std::exception& e)
        {
            util::logError("Failed: %s", e.what());
            sleep(1);
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    util::logVerbose("AnJaRootDaemon (version %s) started",
            version::asString().c_str());

    return AnJaRootDaemon().run(argc, argv);
}
