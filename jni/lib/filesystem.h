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

#ifndef _ANJAROOT_LIB_FILESYSTEM_H_
#define _ANJAROOT_LIB_FILESYSTEM_H_

#include <jni.h>

namespace Filesystem {
    bool initialize(JNIEnv* env);

    void symlink(JNIEnv* env, jclass cls, jstring oldpath, jstring newpath);
    void link(JNIEnv* env, jclass cls, jstring oldpath, jstring newpath);
}

#endif
