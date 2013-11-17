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

#include <limits>
#include <system_error>
#include <unistd.h>

#include "credentials.h"
#include "shared/util.h"
#include "exceptions.h"
#include "syscallfix.h"

jlongArray jni_getresuid(JNIEnv* env, jclass cls)
{
    uid_t ruid, euid, suid;
    int ret = local_getresuid(&ruid, &euid, &suid);
    if(ret != 0)
    {
        util::logError("getresuid failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
    }

    util::logVerbose("getUserIds: ruid=%d, euid=%d, suid=%d", ruid, euid, suid);

    jlongArray retval = env->NewLongArray(3);
    if(retval == nullptr) {
        // OOM exception thrown
        return nullptr;
    }

    jlong buf[3] = {ruid, euid, suid};
    env->SetLongArrayRegion(retval, 0, 3, buf);
    return retval;
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

    util::logVerbose("setUserIds: ruid=%d, euid=%d, suid=%d", ruid, euid, suid);

    int ret = setresuid(ruid, euid, suid);
    if(ret != 0)
    {
        util::logError("setresuid failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
    }
}

jlongArray jni_getresgid(JNIEnv* env, jclass cls)
{
    gid_t rgid, egid, sgid;
    int ret = local_getresgid(&rgid, &egid, &sgid);
    if(ret != 0)
    {
        util::logError("getresgid failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
    }

    util::logVerbose("getGroupIds: rgid=%d, egid=%d, sgid=%d", rgid, egid,
            sgid);

    jlongArray retval = env->NewLongArray(3);
    if(retval == nullptr) {
        // OOM exception thrown
        return nullptr;
    }

    jlong buf[3] = {rgid, egid, sgid};
    env->SetLongArrayRegion(retval, 0, 3, buf);
    return retval;
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

    util::logVerbose("setGroupIds: rgid=%d, egid=%d, sgid=%d", rgid, egid,
            sgid);

    int ret = setresgid(rgid, egid, sgid);
    if(ret != 0)
    {
        util::logError("setresgid failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
    }
}

const JNINativeMethod methods[] = {
    {"getresuid", "()[J", (void *) jni_getresuid},
    {"setresuid", "(JJJ)V", (void *) jni_setresuid},
    {"getresgid", "()[J", (void *) jni_getresgid},
    {"setresgid", "(JJJ)V", (void *) jni_setresgid},
};

extern bool initializeCredentials(JNIEnv* env, jclass nativeMethods)
{
    jint ret = env->RegisterNatives(nativeMethods, methods,
            sizeof(methods) / sizeof(methods[0]));

    return ret == true;
}
