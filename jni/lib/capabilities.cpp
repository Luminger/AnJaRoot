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
#include <errno.h>
#include <limits>
#include <sys/capability.h>
#include <unistd.h>

#include "capabilities.h"

#include "shared/util.h"
#include "exceptions.h"
#include "library.h"

jlongArray jni_capget(JNIEnv* env, jobject obj, jint pid)
{
    __user_cap_header_struct hdr;
    __user_cap_data_struct data;

    memset(&hdr, 0, sizeof(hdr));
    memset(&data, 0, sizeof(data));

    hdr.version = _LINUX_CAPABILITY_VERSION;

    int ret = capget(&hdr, &data);
    if(ret != 0)
    {
        util::logError("capget failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
        return nullptr;
    }

    util::logVerbose("getCapabilities: effective=0x%X, permitted=0x%X, "
            "inheritable=0x%X", data.effective, data.permitted,
            data.inheritable);

    __u32 effective = data.effective;
    __u32 permitted = data.permitted;
    __u32 inheritable = data.inheritable;

    if(isSetCapCompatEnabled())
    {
        int mask = 0xFFFFFFFF & (~(1 << CAP_SETPCAP));
        effective &= mask;
        permitted &= mask;
        inheritable &= mask;
    }

    jlongArray retval = env->NewLongArray(3);
    if(retval == nullptr) {
        // OOM exception thrown
        return nullptr;
    }

    jlong buf[3] = {effective, permitted, inheritable};
    env->SetLongArrayRegion(retval, 0, 3, buf);
    return retval;
}

const char* jni_capset_signature = "(JJJ)V";
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

    util::logVerbose("setCapabilities: effective=0x%X, permitted=0x%X, "
            "inheritable=0x%X", effective, permitted, inheritable);

    if(isSetCapCompatEnabled())
    {
        int mask = 0xFFFFFFFF & (~(1 << CAP_SETPCAP));
        effective &= mask;
        permitted &= mask;
        inheritable &= mask;
    }

    __user_cap_header_struct hdr;
    __user_cap_data_struct data;

    memset(&hdr, 0, sizeof(hdr));
    memset(&data, 0, sizeof(data));

    hdr.version = _LINUX_CAPABILITY_VERSION;
    data.permitted = permitted;
    data.effective = effective;
    data.inheritable = inheritable;

    int ret = capset(&hdr, &data);
    if(ret != 0)
    {
        util::logError("setcap failed: errno=%d, err=%s",
                errno, strerror(errno));
        std::system_error e(errno, std::system_category());
        exceptions::throwNativeException(env, e);
    }
}

static const JNINativeMethod methods[] = {
    {"_capget", "(I)[J", (void *) jni_capget},
    {"_capset", "(JJJ)V", (void *) jni_capset},
};

static const char* clsName =
    "org/failedprojects/anjaroot/library/wrappers/Capabilities";

bool initializeCapabilities(JNIEnv* env)
{
    jclass cls = env->FindClass(clsName);
    if(cls == nullptr)
    {
        util::logError("Failed to get %s class reference", clsName);
        return -1;
    }

    jint ret = env->RegisterNatives(cls, methods,
            sizeof(methods) / sizeof(methods[0]));
    return ret == true;
}
