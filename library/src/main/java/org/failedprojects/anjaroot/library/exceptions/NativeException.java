/*    Copyright 2013 Simon Brakhane
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.failedprojects.anjaroot.library.exceptions;

/**
 * Native code syscall error.
 * 
 * This exception is thrown by the native AnJaRoot part whenever a syscall
 * failed.
 * 
 * If you see this exception and you don't use {@link AnJaRootNative}, something
 * is wrong with the provided wrappers and you should fill a bugreport. Users of
 * {@link AnJaRootNative} need to review their code and pay attention to the
 * provided errno.
 * 
 * @see <code>man errno</code>
 */
public class NativeException extends Exception {
	private static final long serialVersionUID = 5613972860865993387L;

	public NativeException(int errno_, String err) {
		super(err);
		errno = errno_;
	}

	public int getErrno() {
		return errno;
	}

	private final int errno;
}
