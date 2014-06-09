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
import org.failedprojects.anjaroot.library.exceptions.OutOfBoundsException;

public class Credentials {
	/**
	 * Linux user uids holder class.
	 * 
	 * <p>
	 * This class holds linux uids. It's not more than a helper for the
	 * underlying native methods. If you want to know more about linux uids
	 * consult the manpages.
	 * </p>
	 * 
	 * @see <code>man 7 credentials</code>
	 * @see <code>man 2 getresuid</code>
	 * @see <code>man 2 setresuid</code>
	 */
	public static class UserIds {
		private long real = 0;
		private long effective = 0;
		private long saved = 0;

		public UserIds() {
		}

		public UserIds(long real, long effective, long saved) {
			this.real = real;
			this.effective = effective;
			this.saved = saved;
		}

		public long getReal() {
			return real;
		}

		public void setReal(long real) {
			this.real = real;
		}

		public long getEffective() {
			return effective;
		}

		public void setEffective(long effective) {
			this.effective = effective;
		}

		public long getSaved() {
			return saved;
		}

		public void setSaved(long saved) {
			this.saved = saved;
		}
	}

	/**
	 * Linux user gids holder class.
	 * 
	 * <p>
	 * This class holds linux gids. It's not more than a helper for the
	 * underlying native methods. If you want to know more about linux gids
	 * consult the manpages.
	 * </p>
	 * 
	 * @see <code>man 7 credentials</code>
	 * @see <code>man 2 getresgid</code>
	 * @see <code>man 2 setresgid</code>
	 */
	public static class GroupIds {
		private long real = 0;
		private long effective = 0;
		private long saved = 0;

		public GroupIds() {
		}

		public GroupIds(long real, long effective, long saved) {
			this.real = real;
			this.effective = effective;
			this.saved = saved;
		}

		public long getReal() {
			return real;
		}

		public void setReal(long real) {
			this.real = real;
		}

		public long getEffective() {
			return effective;
		}

		public void setEffective(long effective) {
			this.effective = effective;
		}

		public long getSaved() {
			return saved;
		}

		public void setSaved(long saved) {
			this.saved = saved;
		}
	}

	private native static long[] _getresuid() throws NativeException;

	private native static void _setresuid(long ruid, long euid, long suid)
			throws NativeException, OutOfBoundsException;

	private native static long[] _getresgid() throws NativeException;

	private native static void _setresgid(long rgid, long egid, long sgid)
			throws NativeException, OutOfBoundsException;

	public static UserIds getUserIds() throws NativeException {
		long[] uids = _getresuid();
		return new UserIds(uids[0], uids[1], uids[2]);
	}

	public static void setUserIds(UserIds uids) throws NativeException,
			OutOfBoundsException {
		_setresuid(uids.getReal(), uids.getEffective(), uids.getSaved());
	}

	public static GroupIds getGroupIds() throws NativeException {
		long[] gids = _getresgid();
		return new GroupIds(gids[0], gids[1], gids[2]);
	}

	public static void setGroupIds(GroupIds gids) throws NativeException,
			OutOfBoundsException {
		_setresgid(gids.getReal(), gids.getEffective(), gids.getSaved());
	}
}
