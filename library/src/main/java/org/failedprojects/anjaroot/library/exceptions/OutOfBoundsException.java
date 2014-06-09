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
 * Passed parameter is our of bounds.
 * 
 * This exception is thrown whenever a native method detects that a value beyond
 * the bounds is passed to it. It's needed because the native library deals with
 * unsigned 32bit integers and it's parameters have to be signed 64bit integers.
 * 
 * If you see this exception and you don't use {@link AnJaRootNative}, something
 * is wrong with the provided wrappers and you should fill a bugreport. Users of
 * {@link AnJaRootNative} need to review their code.
 */
public class OutOfBoundsException extends Exception {
	private static final long serialVersionUID = -5890404777017384039L;

	public OutOfBoundsException(String msg) {
		super(msg);
	}
}
