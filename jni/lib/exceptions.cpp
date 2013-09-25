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

#include "exceptions.h"
#include "util.h"

namespace exceptions {

static const char* ClassNotFoundName = "java/lang/NoClassDefFoundError";
static const char* MethodNotFoundName = "java/lang/NoSuchMethodError";
static const char* NativeName = "org/failedprojects/anjaroot/library/exceptions/NativeException";
static const char* NativeConstructorSignatur = "(ILjava/lang/String;)V";
static const char* OutOfBoundsName = "org/failedprojects/anjaroot/library/exceptions/OutOfBoundsException";
static const char* PermissionsName = "org/failedprojects/anjaroot/library/exceptions/PermissionsException";

void throwExceptionSimple(JNIEnv* env, const char* clsname, const char* msg)
{
    jclass cls = env->FindClass(clsname);
    if(cls == NULL)
    {
        throwClassNotFoundException(env, clsname);
    }

    env->ThrowNew(cls, msg);
}

void throwClassNotFoundException(JNIEnv* env, const char* msg)
{
    jclass cls = env->FindClass(ClassNotFoundName);
    if(cls == NULL)
    {
        util::logError("Failed to find %s class, fatal!", ClassNotFoundName);
        abort();
    }

    env->ThrowNew(cls, msg);
}

void throwNoMethodFoundException(JNIEnv* env, const char* method)
{
    throwExceptionSimple(env, MethodNotFoundName, method);
}

void throwOutOfBoundsException(JNIEnv* env, const char* msg)
{
    throwExceptionSimple(env, OutOfBoundsName, msg);
}

void throwPermissionsException(JNIEnv* env)
{
    throwExceptionSimple(env, PermissionsName, "");
}

void throwNativeException(JNIEnv* env, const std::system_error& e)
{
    jclass cls = env->FindClass(NativeName);
    if(cls == NULL)
    {
        throwClassNotFoundException(env, NativeName);
        return;
    }

    jmethodID constructor = env->GetMethodID(cls, "<init>", NativeConstructorSignatur);
    if(constructor == NULL)
    {
        throwNoMethodFoundException(env, "<init>");
        return;
    }

    jstring msg = env->NewStringUTF(e.what());
    if(msg == NULL)
    {
        // this "should" throw on its own...
        return;
    }

    jobject obj = env->NewObject(cls, constructor, e.code().value(), msg);
    if(obj == NULL)
    {
        // same here, "should" throw on its own...
        return;
    }

    env->Throw(static_cast<jthrowable>(obj));
}

}
