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

#ifndef _ANJAROOTD_PTRACE_H_
#define _ANJAROOTD_PTRACE_H_

#include <memory>
#include <vector>
#include <unistd.h>

namespace trace {
    class Tracee
    {
        public:
            typedef std::shared_ptr<Tracee> Ptr;
            typedef std::vector<Ptr> List;

            Tracee(pid_t pid_);
            ~Tracee();

            pid_t getPid() const;
            bool detach() const;
            void resume(int signal = 0) const;
            void waitForSyscallResume() const;
            void setupSyscallTrace() const;
            void setupChildTrace() const;
            unsigned long getEventMsg() const;
            siginfo_t getSignalInfo() const;

            bool isSyscallBegin() const;
            void setSyscallBegin(bool value);

        private:
            pid_t pid;
            bool syscallBegin;
    };

    class WaitResult
    {
        public:
            WaitResult(pid_t pid_, int status_);
            void logDebugInfo() const;

            pid_t getPid() const;
            int getStatus() const;
            bool hasExited() const;
            int getExitStatus() const;
            bool wasSignaled() const;
            int getTermSignal() const;
            bool wasCoredumped() const;
            bool hasStopped() const;
            int getStopSignal() const;
            int getEvent() const;
            bool inSyscall() const;

        private:
            pid_t pid;
            int status;
    };

    Tracee::Ptr attach(pid_t pid);
    WaitResult waitChilds();
    WaitResult waitChild(pid_t pid);
}

#endif
