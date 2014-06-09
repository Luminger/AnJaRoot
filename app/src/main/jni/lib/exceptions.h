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

#ifndef _ANJAROOT_LIB_EXCEPTIONS_H_
#define _ANJAROOT_LIB_EXCEPTIONS_H_

#include <system_error>
#include <stdlib.h>
#include <jni.h>

namespace exceptions {
    void throwClassNotFoundException(JNIEnv* env, const char* msg);
    void throwNoMethodFoundException(JNIEnv* env, const char* method);
    void throwNativeException(JNIEnv* env, const std::system_error& e);
    void throwOutOfBoundsException(JNIEnv* env, const char* msg);
}

#endif
