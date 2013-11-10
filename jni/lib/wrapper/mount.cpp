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

#include <system_error>
#include <errno.h>
#include <sys/mount.h>

#include "mount.h"
#include "shared/util.h"
#include "../exceptions.h"

const char* jni_mount_signature = "(Ljava/lang/String;Ljava/lang/String;"
    "Ljava/lang/String;JLjava/lang/String;)V";
void jni_mount(JNIEnv* env, jclass cls, jstring source, jstring target,
        jstring filesystemtype, jlong mountflags, jstring data)
{
    const char* sourcestr = env->GetStringUTFChars(source, 0);
    const char* targetstr = env->GetStringUTFChars(target, 0);
    const char* filesystemtypestr = env->GetStringUTFChars(filesystemtype, 0);
    const char* datastr = env->GetStringUTFChars(data, 0);

    if(sourcestr && targetstr && filesystemtypestr && datastr)
    {
        int ret = mount(sourcestr, targetstr, filesystemtypestr, mountflags,
                datastr);
        if(ret != 0)
        {
            std::system_error error(errno, std::system_category());
            exceptions::throwNativeException(env, error);
        }
    }

    env->ReleaseStringUTFChars(source, sourcestr);
    env->ReleaseStringUTFChars(target, targetstr);
    env->ReleaseStringUTFChars(filesystemtype, filesystemtypestr);
    env->ReleaseStringUTFChars(data, datastr);
}

const char* jni_umount_signature = "(Ljava/lang/String;)V";
void jni_umount(JNIEnv* env, jclass cls, jstring target)
{
    const char* targetstr = env->GetStringUTFChars(target, 0);
    if(targetstr == nullptr)
    {
        // JVM may have crashed already or just throws an OOM exception
        return;
    }

    int ret = umount(targetstr);
    if(ret != 0)
    {
        std::system_error error(errno, std::system_category());
        exceptions::throwNativeException(env, error);
    }

    env->ReleaseStringUTFChars(target, targetstr);
}

const char* jni_umount2_signature = "(Ljava/lang/String;I)V";
void jni_umount2(JNIEnv* env, jclass cls, jstring target, jlong flags)
{
    const char* targetstr = env->GetStringUTFChars(target, 0);
    if(targetstr == nullptr)
    {
        // JVM may have crashed already or just throws an OOM exception
        return;
    }

    int ret = umount2(targetstr, static_cast<int>(flags));
    if(ret != 0)
    {
        std::system_error error(errno, std::system_category());
        exceptions::throwNativeException(env, error);
    }

    env->ReleaseStringUTFChars(target, targetstr);
}
