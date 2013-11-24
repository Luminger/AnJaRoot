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

#include "library.h"

#include "exceptions.h"
#include "shared/util.h"
#include "shared/version.h"

void jni_setcompatmode(JNIEnv* env, jclass cls, jint apilvl)
{
    util::logVerbose("Library API level: %d", version::Api);
    util::logVerbose("Library API compat level set to: %d", apilvl);

    // Everything below 3 isn't supported anymore
    if(apilvl < 3)
    {
        const char* msg = "API level 1 and 2 are not suppoted anymore, sorry!";
        util::logError(msg);

        std::system_error error(0, std::generic_category(), msg);
        exceptions::throwNativeException(env, error);
    }
}

jintArray jni_getversion(JNIEnv* env, jclass cls)
{
    jintArray retval = env->NewIntArray(4);
    if(retval == nullptr) {
        // OOM exception thrown
        return nullptr;
    }

    jint buf[4] = {
        version::Major,
        version::Minor,
        version::Patch,
        version::Api,
    };

    env->SetIntArrayRegion(retval, 0, 4, buf);
    return retval;
}

static const JNINativeMethod methods[] = {
    {"_getversion", "()[I", (void *) jni_getversion},
    {"_setcompatmode", "(I)V", (void *)jni_setcompatmode},
};

static const char* clsName =
    "org/failedprojects/anjaroot/library/wrappers/Library";

extern bool initializeLibrary(JNIEnv* env)
{
    jclass cls = env->FindClass(clsName);
    if(cls == nullptr)
    {
        util::logError("Failed to get %s class reference", clsName);
        return -1;
    }

    jint ret = env->RegisterNatives(cls, methods,
            sizeof(methods) / sizeof(methods[0]));
    if(ret != 0)
    {
        util::logError("Failed to register natives on class %s", clsName);
    }

    return ret == 0;
}
