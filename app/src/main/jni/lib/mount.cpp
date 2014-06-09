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
#include "exceptions.h"

void Mount::mount(JNIEnv* env, jclass cls, jstring source, jstring target,
        jstring filesystemtype, jlong mountflags, jstring data)
{
    const char* sourcestr = env->GetStringUTFChars(source, 0);
    const char* targetstr = env->GetStringUTFChars(target, 0);
    const char* filesystemtypestr = env->GetStringUTFChars(filesystemtype, 0);
    const char* datastr = env->GetStringUTFChars(data, 0);

    if(sourcestr && targetstr && filesystemtypestr && datastr)
    {
        util::logVerbose("mount: source=%s, target=%s, filesystemtype=%s, "
                "mountflags=0x%X, data=%s", sourcestr, targetstr,
                filesystemtypestr, mountflags, data);

        int ret = ::mount(sourcestr, targetstr, filesystemtypestr,
                static_cast<int>(mountflags), datastr);
        if(ret != 0)
        {
            util::logError("mount failed: errno=%d, err=%s", errno,
                    strerror(errno));
            std::system_error error(errno, std::system_category());
            exceptions::throwNativeException(env, error);
        }
    }

    env->ReleaseStringUTFChars(source, sourcestr);
    env->ReleaseStringUTFChars(target, targetstr);
    env->ReleaseStringUTFChars(filesystemtype, filesystemtypestr);
    env->ReleaseStringUTFChars(data, datastr);
}

void Mount::umount(JNIEnv* env, jclass cls, jstring target)
{
    const char* targetstr = env->GetStringUTFChars(target, 0);
    if(targetstr == nullptr)
    {
        // JVM may have crashed already or just throws an OOM exception
        return;
    }

    util::logVerbose("umount: target=%s", targetstr);

    int ret = ::umount(targetstr);
    if(ret != 0)
    {
        util::logError("umount failed: errno=%d, err=%s", errno,
                strerror(errno));
        std::system_error error(errno, std::system_category());
        exceptions::throwNativeException(env, error);
    }

    env->ReleaseStringUTFChars(target, targetstr);
}

void Mount::umount2(JNIEnv* env, jclass cls, jstring target, jlong flags)
{
    const char* targetstr = env->GetStringUTFChars(target, 0);
    if(targetstr == nullptr)
    {
        // JVM may have crashed already or just throws an OOM exception
        return;
    }

    util::logVerbose("umount2: target=%s, flags=0x%X", target, flags);

    int ret = ::umount2(targetstr, static_cast<int>(flags));
    if(ret != 0)
    {
        util::logError("umount2 failed: errno=%d, err=%s", errno,
                strerror(errno));
        std::system_error error(errno, std::system_category());
        exceptions::throwNativeException(env, error);
    }

    env->ReleaseStringUTFChars(target, targetstr);
}

bool Mount::initialize(JNIEnv* env)
{
    const JNINativeMethod methods[] = {
        {"_mount", "(Ljava/lang/String;Ljava/lang/String;"
            "Ljava/lang/String;JLjava/lang/String;)V", (void *) Mount::mount},
        {"_umount", "(Ljava/lang/String;)V", (void *) Mount::umount},
        {"_umount2", "(Ljava/lang/String;J)V", (void *) Mount::umount2},
    };

    const char* clsName =
        "org/failedprojects/anjaroot/library/wrappers/Mount";

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
