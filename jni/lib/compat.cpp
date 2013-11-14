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

#include "compat.h"
#include "shared/util.h"

bool setCapEnabled = false;

bool isSetCapCompatEnabled()
{
    return setCapEnabled;
}

const char* jni_setcompatmode_signature = "(I)V";
void jni_setcompatmode(JNIEnv*, jclass cls, jint apilvl)
{
    // We are at apilvl 1, nothing to do here =)
    util::logVerbose("Library API level: %d", apilvl);

    // apilvl 2 adds compatibility for unset CAP_SETCAP capability
    if(apilvl < 2)
    {
        util::logVerbose("Enabling CAP_SETCAP compat mode");
        setCapEnabled = true;
    }
}
