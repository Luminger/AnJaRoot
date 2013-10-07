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

#ifndef _ANJAROOT_LIB_PACKAGES_H_
#define _ANJAROOT_LIB_PACKAGES_H_

#include <string>
#include <vector>
#include <unistd.h>

namespace packages
{
    /* Format is described in the run-as source which can be found in the
     * platform_system_core repository.
     *
     * Quote from there:
     *
     * expect the following format on each line of the control file:
     *
     *  <pkgName> <uid> <debugFlag> <dataDir> <seinfo>
     *
     * where:
     *  <pkgName>    is the package's name
     *  <uid>        is the application-specific user Id (decimal)
     *  <debugFlag>  is 1 if the package is debuggable, or 0 otherwise
     *  <dataDir>    is the path to the package's data directory (e.g. /data/data/com.example.foo)
     *  <seinfo>     is the seinfo label associated with the package
     *
     * The file is generated in com.android.server.PackageManagerService.Settings.writeLP()
     */
    struct Package
    {
        Package();

        std::string pkgName;
        uid_t uid;
        bool debugFlag;
        std::string dataDir;
        std::string seinfo;
    };

    class PackageList
    {
        public:
            typedef std::vector<Package> Packages;

            PackageList();

            const Package* findByName(const std::string& name) const;
            const Package* findByUid(uid_t uid) const;

        private:
            void readPackages();

            Packages packages;
    };

    class GrantedPackageList
    {
        public:
            typedef std::vector<std::string> Packages;

            GrantedPackageList(const Package& anjaroot);

            bool isGranted(const Package& package) const;

        private:
            void readPackages(const std::string& filename);

            const Package& myself;
            Packages packages;
    };
}

#endif
