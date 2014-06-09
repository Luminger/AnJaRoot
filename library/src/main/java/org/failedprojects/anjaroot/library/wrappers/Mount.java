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
package org.failedprojects.anjaroot.library.wrappers;

import org.failedprojects.anjaroot.library.exceptions.NativeException;

public class Mount {
	public class Mountpoint {
		private String fsname;
		private String dir;
		private String type;
		private String opts;
		private int freq;
		private int passno;
	}

	private native static void _mount(String source, String target,
			String filesystemtype, long mountflags, String data)
			throws NativeException;

	private native static void _umount(String target) throws NativeException;

	private native static void _umount2(String target, long flags)
			throws NativeException;

	public static void mount(String source, String target,
			String filesystemtype, long mountflags, String data)
			throws NativeException {
		_mount(source, target, filesystemtype, mountflags, data);
	}

	public static void umount(String target) throws NativeException {
		_umount(target);
	}

	public static void umount2(String target, long flags)
			throws NativeException {
		_umount2(target, flags);
	}
}
