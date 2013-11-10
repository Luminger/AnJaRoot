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
#include <sys/capability.h>

#include "capabilities.h"

#include "shared/util.h"
#include "../helper.h"
#include "../exceptions.h"
#include "compat.h"

const char* jni_capget_signature = "(I)[J";
jlongArray jni_capget(JNIEnv* env, jobject obj, jint pid)
{
    jlongArray retval = env->NewLongArray(3);
    if(retval == nullptr) {
        // OOM exception thrown
        return nullptr;
    }

    try
    {
        helper::Capabilities caps = helper::getCapabilities(pid);

        if(isSetCapCompatEnabled())
        {
            if(caps.effective == 0xFFFFFEFF)
            {
                caps.effective = 0xFFFFFFFF;
            }

            if(caps.permitted == 0xFFFFFEFF)
            {
                caps.permitted = 0xFFFFFFFF;
            }

            if(caps.inheritable == 0xFFFFFEFF)
            {
                caps.inheritable = 0xFFFFFFFF;
            }
        }

        jlong buf[3] = {caps.effective, caps.permitted, caps.inheritable};
        env->SetLongArrayRegion(retval, 0, 3, buf);
        return retval;
    }
    catch(std::system_error& e)
    {
        exceptions::throwNativeException(env, e);
        return nullptr;
    }
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

    if(isSetCapCompatEnabled())
    {
        int mask = 0xFFFFFFFF & (~(1 << CAP_SETPCAP));
        effective &= mask;
        permitted &= mask;
        inheritable &= mask;
    }

    try
    {
        helper::Capabilities caps(effective, permitted, inheritable);
        helper::setCapabilities(caps);
    }
    catch(std::system_error& e)
    {
        exceptions::throwNativeException(env, e);
    }
}
