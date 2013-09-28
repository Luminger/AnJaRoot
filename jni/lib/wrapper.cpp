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

#include <cstring>
#include <limits>
#include <system_error>
#include <android/log.h>
#include <jni.h>
#include <string.h>
#include <unistd.h>
#include "exceptions.h"
#include "helper.h"
#include "util.h"
#include "version.h"
#include "hook.h"

static const char* className = "org/failedprojects/anjaroot/library/internal/NativeMethods";

jlongArray jni_capget(JNIEnv* env, jobject obj, jint pid)
{
    jlongArray retval = env->NewLongArray(3);
    if(retval == NULL) {
        // OOM exception thrown
        return NULL;
    }

    try
    {
        helper::Capabilities caps = helper::getCapabilities(pid);
        jlong buf[3] = {caps.effective, caps.permitted, caps.inheritable};
        env->SetLongArrayRegion(retval, 0, 3, buf);
        return retval;
    }
    catch(std::system_error& e)
    {
        if(e.code().value() == EPERM)
        {
            exceptions::throwPermissionsException(env);
        }
        else
        {
            exceptions::throwNativeException(env, e);
        }
        return NULL;
    }
}

void jni_capset(JNIEnv* env, jclass cls, jlong effective, jlong permitted,
        jlong inheritable)
{
    const __u32 minValue = std::numeric_limits<__u32>::min();
    const __u32 maxValue = std::numeric_limits<__u32>::max();

    // otherwise the checks below will report 0xFFFFFFFF (-1) < 0
    effective = static_cast<__u32>(effective);
    permitted = static_cast<__u32>(permitted);
    inheritable = static_cast<__u32>(inheritable);

    if(effective < minValue || effective > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Effective out of bounds");
        return;
    }

    if(permitted < minValue || permitted > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Permitted out of bounds");
        return;
    }

    if(inheritable < minValue || inheritable > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Inheritable out of bounds");
        return;
    }

    try
    {
        helper::Capabilities caps(effective, permitted, inheritable);
        helper::setCapabilities(caps);
    }
    catch(std::system_error& e)
    {
        if(e.code().value() == EPERM)
        {
            exceptions::throwPermissionsException(env);
        }
        else
        {
            exceptions::throwNativeException(env, e);
        }
    }
}

jlongArray jni_getresuid(JNIEnv* env, jclass cls)
{
    jlongArray retval = env->NewLongArray(3);
    if(retval == NULL) {
        // OOM exception thrown
        return NULL;
    }

    try
    {
        helper::UserIds uids = helper::getUserIds();
        jlong buf[3] = {uids.ruid, uids.euid, uids.suid};
        env->SetLongArrayRegion(retval, 0, 3, buf);
        return retval;
    }
    catch(std::system_error& e)
    {
        exceptions::throwNativeException(env, e);
        return NULL;
    }
}

void jni_setresuid(JNIEnv* env, jclass cls, jlong ruid, jlong euid, jlong suid)
{
    const uid_t minValue = std::numeric_limits<uid_t>::min();
    const uid_t maxValue = std::numeric_limits<uid_t>::max();

    // otherwise the checks below will report 0xFFFFFFFF (-1) < 0
    ruid = static_cast<uid_t>(ruid);
    euid = static_cast<uid_t>(euid);
    suid = static_cast<uid_t>(suid);

    if(ruid < minValue || ruid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Ruid out of bounds");
        return;
    }

    if(euid < minValue || euid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Euid out of bounds");
        return;
    }

    if(suid < minValue || suid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Suid out of bounds");
        return;
    }

    try
    {
        helper::UserIds uids(ruid, euid, suid);
        helper::setUserIds(uids);
    }
    catch(std::system_error& e)
    {
        if(e.code().value() == EPERM)
        {
            exceptions::throwPermissionsException(env);
        }
        else
        {
            exceptions::throwNativeException(env, e);
        }
    }
}

jlongArray jni_getresgid(JNIEnv* env, jclass cls)
{
    jlongArray retval = env->NewLongArray(3);
    if(retval == NULL) {
        // OOM exception thrown
        return NULL;
    }

    try
    {
        helper::GroupIds gids = helper::getGroupIds();
        jlong buf[3] = {gids.rgid, gids.egid, gids.sgid};
        env->SetLongArrayRegion(retval, 0, 3, buf);
        return retval;
    }
    catch(std::system_error& e)
    {
        exceptions::throwNativeException(env, e);
        return NULL;
    }
}

void jni_setresgid(JNIEnv* env, jclass cls, jlong rgid, jlong egid, jlong sgid)
{
    const gid_t minValue = std::numeric_limits<gid_t>::min();
    const gid_t maxValue = std::numeric_limits<gid_t>::max();

    // otherwise the checks below will report 0xFFFFFFFF (-1) < 0
    rgid = static_cast<gid_t>(rgid);
    egid = static_cast<gid_t>(egid);
    sgid = static_cast<gid_t>(sgid);

    if(rgid < minValue || rgid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Rgid out of bounds");
        return;
    }

    if(egid < minValue || egid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Egid out of bounds");
        return;
    }

    if(sgid < minValue || sgid > maxValue)
    {
        exceptions::throwOutOfBoundsException(env, "Sgid out of bounds");
        return;
    }

    try
    {
        helper::GroupIds gids(rgid, egid, sgid);
        helper::setGroupIds(gids);
    }
    catch(std::system_error& e)
    {
        if(e.code().value() == EPERM)
        {
            exceptions::throwPermissionsException(env);
        }
        else
        {
            exceptions::throwNativeException(env, e);
        }
    }
}

jintArray jni_getversion(JNIEnv* env, jclass cls)
{
    jintArray retval = env->NewIntArray(3);
    if(retval == NULL) {
        // OOM exception thrown
        return NULL;
    }

    jint buf[3] = {
        version::Major,
        version::Minor,
        version::Patch,
    };

    env->SetIntArrayRegion(retval, 0, 3, buf);
    return retval;
}

jbooleanArray jni_getstatus(JNIEnv* env, jclass cls)
{
    jbooleanArray retval = env->NewBooleanArray(2);
    if(retval == NULL) {
        // OOM exception thrown
        return NULL;
    }

    jboolean buf[2] = {
        hook::Hooked,
        hook::AlreadyRun,
    };

    env->SetBooleanArrayRegion(retval, 0, 2, buf);
    return retval;
}

static JNINativeMethod methods[] = {
    {"capget", "(I)[J", (void *) jni_capget},
    {"capset", "(JJJ)V", (void *) jni_capset},
    {"getresuid", "()[J", (void *) jni_getresuid},
    {"setresuid", "(JJJ)V", (void *) jni_setresuid},
    {"getresgid", "()[J", (void *) jni_getresgid},
    {"setresgid", "(JJJ)V", (void *) jni_setresgid},
    {"getversion", "()[I", (void *) jni_getversion},
    {"getstatus", "()[Z", (void *) jni_getstatus},
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
    if(cls == NULL)
    {
        util::logError("Failed to get %s class reference", className);
        return -1;
    }

    env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(methods[0]));
    return JNI_VERSION_1_6;
}
