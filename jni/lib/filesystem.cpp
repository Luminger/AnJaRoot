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
#include <unistd.h>

#include "filesystem.h"
#include "shared/util.h"
#include "exceptions.h"

void jni_symlink(JNIEnv* env, jclass cls, jstring oldpath, jstring newpath)
{
    const char* oldpathstr = env->GetStringUTFChars(oldpath, 0);
    const char* newpathstr = env->GetStringUTFChars(newpath, 0);

    if(oldpathstr && newpathstr)
    {
        util::logVerbose("symlink: oldpath=%s, newpath=%s", oldpathstr,
                newpathstr);

        int ret = symlink(oldpathstr, newpathstr);
        if(ret != 0)
        {
            util::logError("symlink failed: errno=%d, err=%s", errno,
                    strerror(errno));
            std::system_error error(errno, std::system_category());
            exceptions::throwNativeException(env, error);
        }
    }

    env->ReleaseStringUTFChars(oldpath, oldpathstr);
    env->ReleaseStringUTFChars(newpath, newpathstr);
}

void jni_link(JNIEnv* env, jclass cls, jstring oldpath, jstring newpath)
{
    const char* oldpathstr = env->GetStringUTFChars(oldpath, 0);
    const char* newpathstr = env->GetStringUTFChars(newpath, 0);

    if(oldpathstr && newpathstr)
    {
        util::logVerbose("link: oldpath=%s, newpath=%s", oldpathstr,
                newpathstr);

        int ret = link(oldpathstr, newpathstr);
        if(ret != 0)
        {
            util::logError("link failed: errno=%d, err=%s", errno,
                    strerror(errno));
            std::system_error error(errno, std::system_category());
            exceptions::throwNativeException(env, error);
        }
    }

    env->ReleaseStringUTFChars(oldpath, oldpathstr);
    env->ReleaseStringUTFChars(newpath, newpathstr);
}

// TODO add readlink, stat family, chmod, chown, chattr
// MAYDO add access, utime
static const JNINativeMethod methods[] = {
    {"_symlink", "(Ljava/lang/String;Ljava/lang/String;)V",
        (void *) jni_symlink},
    {"_link", "(Ljava/lang/String;Ljava/lang/String;)V", (void *) jni_link},
};

static const char* clsName =
    "org/failedprojects/anjaroot/library/wrappers/Filesystem";

extern bool initializeFilesystem(JNIEnv* env)
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
