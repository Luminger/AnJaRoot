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

#include "version.h"
#include "shared/version.h"

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

const JNINativeMethod methods[] = {
    {"getversion", "()[I", (void *) jni_getversion},
};

extern bool initializeVersion(JNIEnv* env, jclass nativeMethods)
{
    jint ret = env->RegisterNatives(nativeMethods, methods,
            sizeof(methods) / sizeof(methods[0]));

    return ret == true;
}
