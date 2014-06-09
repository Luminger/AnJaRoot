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
 * Native library not found.
 * 
 * Thrown if AnJaRootLibrary couldn't access the underlying native library which
 * is hooked directly into the zygote process. If you see this exception it
 * normally means that AnJaRoot is not installed (correctly) and is therefore
 * non functional.
 * 
 * Make sure that AnJaRoot is installed properly, consider filling a bugreport.
 */
public class LibraryNotLoadedException extends Exception {

	private static final long serialVersionUID = -8618891193291966124L;

	public LibraryNotLoadedException() {
		super();
	}
}
