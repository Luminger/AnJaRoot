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

#include "anjarootdaemon.h"
#include "trace.h"
#include "shared/util.h"
#include "shared/version.h"

bool shouldRun = true;

void signalHandler(int signum)
{
    // Zygote doesn't use the android logging methods in its signal handler, so
    // we don't do either. It's just too risky...
    if(signum == SIGINT || signum == SIGTERM)
    {
        shouldRun = false;
        return;
    }
}

void setupSignalHandling()
{
    util::logVerbose("Setting up signal handlers...");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signalHandler;
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

// TODO move to AnJaRootDaemon class
uid_t getUidFromPid(uid_t pid)
{
    // There is no real "official" and/or good way, so we just stat the
    // /proc/<pid> directory which is owned by the process.
    char path[32] = {0, };
    snprintf(path, sizeof(path), "/proc/%d", pid);

    struct stat st;
    int ret = stat(path, &st);
    if(ret == -1)
    {
        util::logError("Failed to stat %s: %s", path, strerror(errno));
        throw std::system_error(errno, std::system_category());
    }

    return st.st_uid;
}

void claimLockSocket()
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

int main(int argc, char** argv)
{
    util::logVerbose("AnJaRootDaemon (version %s) started",
            version::asString().c_str());

    // simple and stupid version - I don't care as long as it's only one arg
    if(argc == 2 && std::string(argv[1]) == "-d")
    {
        int ret = daemon(0, 0);
        if(ret == -1)
        {
            util::logError("Failed to daemonize, error %d: %s", errno,
                    strerror(errno));
            return -3;
        }
    }

    try
    {
        setupSignalHandling();
        claimLockSocket();

        while(shouldRun)
        {
            AnJaRootDaemon().run(shouldRun);

            if(shouldRun)
            {
                util::logVerbose("MainLoop exited, wait 1sec and restart");
                sleep(1);
            }
        }
    }
    catch(std::exception& e)
    {
        util::logError("Failed: %s", e.what());
        return -1;
    }

    return 0;
}
