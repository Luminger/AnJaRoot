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

#include "packages.h"
#include "util.h"

#include <stdlib.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <fstream>

namespace packages {

Package::Package() : uid(-1), debugFlag(false)
{
}

PackageList::PackageList()
{
    readPackages();
}

void PackageList::readPackages()
{
    try
    {
        std::ifstream file("/data/system/packages.list");
        std::string line;

        while(std::getline(file, line))
        {
            std::istringstream stream(line);
            std::string token;
            std::vector<std::string> tokens;

            while(std::getline(stream, token, ' '))
            {
                tokens.push_back(token);
            }

            // At least the x86 4.2 emulator doesn't append the seinfo
            // field to the file, therefor I assume this is optional.
            if(tokens.size() < 4)
            {
                util::logError("Read line has less than 4 tokens!");
                packages.clear();
                return;
            }

            // smartptrs would be great, we con't need that many copies...
            Package package;
            package.pkgName = tokens[0];
            package.uid = atoi(tokens[1].c_str()); // yea, I'm lazy...
            package.debugFlag = atoi(tokens[2].c_str()) == 1; // dito...
            package.dataDir = tokens[3];

            if(tokens.size() == 5)
            {
                package.seinfo = tokens[4];
            }

            packages.push_back(package);
        }
    }
    catch(std::exception& e)
    {
        util::logError("Failed to read packages.list: %s", e.what());
    }
}

const Package* PackageList::findByName(const std::string& name) const
{
    for(Packages::const_iterator iter = packages.begin();
            iter != packages.end(); iter++)
    {
        if(iter->pkgName == name)
        {
            return &*iter;
        }
    }

    return NULL;
}

const Package* PackageList::findByUid(uid_t uid) const
{
    for(Packages::const_iterator iter = packages.begin();
            iter != packages.end(); iter++)
    {
        if(iter->uid == uid)
        {
            return &*iter;
        }
    }

    return NULL;
}

GrantedPackageList::GrantedPackageList(const Package& anjaroot)
{
    std::string grantfile = "/data/data/" + anjaroot.pkgName + "/files/granted";
    readPackages(grantfile);
}

void GrantedPackageList::readPackages(const std::string& filename)
{
    try
    {
        std::ifstream file(filename.c_str());
        std::string line;

        while(std::getline(file, line))
        {
            packages.push_back(line);
        }
    }
    catch(std::exception& e)
    {
        util::logError("Failed to read granted file!");
    }
}

bool GrantedPackageList::isGranted(const Package& package) const
{
    for(Packages::const_iterator iter = packages.begin();
            iter != packages.end(); iter++)
    {
        if(*iter == package.pkgName)
        {
            return true;
        }
    }

    return false;
}

}
