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

#include <jni.h>

#include "shared/util.h"

#include "capabilities.h"
#include "credentials.h"
#include "compat.h"
#include "mount.h"
#include "version.h"

// can't be changed as the library is distributed with that package
static const char* className =
        "org/failedprojects/anjaroot/library/internal/NativeMethods";

static JNINativeMethod methods[] = {
    // capabilities.h
    {"capget", jni_capget_signature, (void *) jni_capget},
    {"capset", jni_capset_signature, (void *) jni_capset},
    // compat.h
    {"setcompatmode", jni_setcompatmode_signature, (void *)jni_setcompatmode},
    // credentials.h
    {"getresuid", jni_getresuid_signature, (void *) jni_getresuid},
    {"setresuid", jni_setresuid_signature, (void *) jni_setresuid},
    {"getresgid", jni_getresgid_signature, (void *) jni_getresgid},
    {"setresgid", jni_setresgid_signature, (void *) jni_setresgid},
    // mount.h
    {"mount", jni_mount_signature, (void *) jni_mount},
    {"umount", jni_umount_signature, (void *) jni_umount},
    {"umount2", jni_umount2_signature, (void *) jni_umount2},
    // version.h
    {"getversion", jni_getversion_signature, (void *) jni_getversion},
};

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env;
    int ret = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if(ret != JNI_OK)
    {
        util::logError("Failed to get JNI Environment");
        return -1;
    }

    jclass cls = env->FindClass(className);
    if(cls == nullptr)
    {
        util::logError("Failed to get %s class reference", className);
        return -1;
    }

    env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_VERSION_1_6;
}
