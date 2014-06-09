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
package org.failedprojects.anjaroot.library;

import org.failedprojects.anjaroot.library.exceptions.LibraryNotLoadedException;
import org.failedprojects.anjaroot.library.wrappers.Capabilities;
import org.failedprojects.anjaroot.library.wrappers.Capabilities.Permissions;
import org.failedprojects.anjaroot.library.wrappers.Credentials;
import org.failedprojects.anjaroot.library.wrappers.Credentials.GroupIds;
import org.failedprojects.anjaroot.library.wrappers.Credentials.UserIds;
import org.failedprojects.anjaroot.library.wrappers.Library;
import org.failedprojects.anjaroot.library.wrappers.Library.Version;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Process;
import android.os.SystemClock;

/**
 * Core functionality provided by the library.
 * 
 * <p>
 * This class provides all the functionalities needed to gain root privileges
 * (linux capabilities and uid=0, gid=0). The user has to do the following steps
 * to make use of AnJaRoot:
 * </p>
 * 
 * <p>
 * <ol>
 * <li>Declare to use <code>android.permission.ACCESS_ANJAROOT</code> permission
 * in your AndroidManifest.xml in a <code>uses-permissions</code> tag
 * <li>Use {@link #isInstalled()} to check if AnJaRoot is installed, you may
 * redirect the user via {@link #openPlayStore(Context)}</li>
 * <li>Use {@link #isAccessGranted()} to check for already granted access</li>
 * </ol>
 * <p>
 * 
 * 
 * <p>
 * If {@link #isAccessGranted()} returned false (access is NOT granted):
 * <ol>
 * <li>Use {@link org.failedprojects.anjaroot.library.AnJaRootRequester} to
 * request access
 * <li>If the previous step returned true, use
 * {@link AnJaRoot#commitSuicideAndRestart(Context, PendingIntent)} or
 * {@link #commitSuicide()} to kill your current (linux) process, you won't be
 * able to proceed otherwise.</li>
 * </ol>
 * </p>
 * 
 * <p>
 * If {@link #isAccessGranted()} returned true (access is now granted):
 * <ol>
 * <li>Use {@link #gainAccess()} to raise the privileges of the current
 * process/thread</li>
 * <li>Do actions which require root privileges</li>
 * <li>Use {@link #dropAccess()} to drop access as early as possible.</li>
 * </ol>
 * </p>
 * 
 * <p>
 * The steps described above can be done as often as needed. There is also the
 * {@link org.failedprojects.anjaroot.library.AnJaRootNative} class which
 * provides the low level functions used by this class.
 * </p>
 * 
 * <p>
 * It's also encouraged to keep the privileged phases of an app as short as
 * possible to not disrupt any other part of the (android) system, things may
 * start to randomly fail if your app just stays root.
 * </p>
 * 
 * @see The <a href="https://github.com/Luminger/AnJaRootTester">AnJaRoot
 *      Tester</a> for a reference implementation.
 */
public class AnJaRoot {
	private static final UserIds rootUids = new UserIds(0, 0, 0);
	private static final GroupIds rootGids = new GroupIds(0, 0, 0);
	private static final Permissions rootCaps = new Permissions(0xFFFFFEFF,
			0xFFFFFEFF, 0);

	private static UserIds originalUids = null;
	private static GroupIds originalGids = null;
	private static Permissions originalCaps = null;

	private AnJaRoot() {

	}

	/**
	 * Kill the current (linux) process.
	 * 
	 * <p>
	 * You may prefer to use
	 * {@link #commitSuicideAndRestart(Context, PendingIntent)} as it's more
	 * user friendly.
	 * </p>
	 * 
	 * <p>
	 * Android apps live in their own linux process. To make use of newly
	 * granted AnJaRoot rights an app has to restart completely. This method
	 * invokes {@link android.os.Process#killProcess(int)} to commit suicide and
	 * will therefore not return.
	 * </p>
	 * 
	 * <p>
	 * Calling this method will kill your app immediately! This means no more
	 * code is executed. Therefore the normal activity/fragment flow will NOT
	 * happen, you have to make sure that your app leaves a clean state.
	 * </p>
	 * 
	 */
	public static void commitSuicide() {
		Process.killProcess(Process.myPid());
	}

	/**
	 * Kill the current (linux) process and restart.
	 * 
	 * <p>
	 * This method works exactly like {@link #commitSuicide()} (take also a look
	 * at its documentation) but also accepts a pending intent which will be
	 * fired by the @ AlarmManager} a second after the current process is
	 * killed. This enables you to restart your app in a user friendly way as it
	 * (should) gets soon restarted. It's up to you to provide a valid Intent
	 * here, no further checking is done if it links to an Activity.
	 * </p>
	 * 
	 * @param context
	 *            context used to get a reference to {@link AlarmManager}
	 * @param intent
	 *            intent which will be fired by the {@link AlarmManager} shortly
	 *            after the app committed suicide
	 */
	public static void commitSuicideAndRestart(Context context,
			PendingIntent intent) {
		if (context == null) {
			throw new NullPointerException("context can't be null");
		}

		if (intent == null) {
			throw new NullPointerException("intent can't be null");
		}

		AlarmManager am = (AlarmManager) context
				.getSystemService(Context.ALARM_SERVICE);
		am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP,
				SystemClock.elapsedRealtime() + 1000, intent);
		commitSuicide();
	}

	/**
	 * Check whether AnJaRoot may be used or not.
	 * 
	 * <p>
	 * This method checks if AnJaRoot is installed on this system and if it's
	 * usable. A negative return value may either indicate that AnJaRoot is not
	 * installed at all or the current installation is broken.
	 * </p>
	 * 
	 * @return <code>true</code> if AnJaRoot is ready, <code>false</code> if
	 *         AnJaRoot is not usable on this system (not installed).
	 */
	public static boolean isInstalled() {
		return Library.isLibraryLoaded();
	}

	/**
	 * Check whether access is granted or not.
	 * 
	 * <p>
	 * This method checks if access is already granted for the calling app. The
	 * user may proceed to {@link #gainAccess()} if <code>true</code> is
	 * returned, otherwise you need to utilize {@link AnJaRootRequester} to
	 * request access.
	 * </p>
	 * 
	 * @return <code>true</code> if the user allowed the usage of AnJaRoot for
	 *         this app, <code>false</code> if request must be requested
	 */
	public static boolean isAccessGranted() {
		try {
			Permissions caps = Capabilities.getCapabilities();
			return caps.getPermitted() == 0xFFFFFEFFL;
		} catch (Exception ignored) {
			return false;
		}
	}

	/**
	 * Get the version identifier for AnJaRootLibrary.
	 * 
	 * @return The version identifier for AnJaRootLibrary.
	 */
	public static Version getLibraryVersion() {
		return Library.getLibraryVersion();
	}

	/**
	 * Get the version identifier for the native part of AnJaRoot
	 * 
	 * @return The version identifier for the native parts of AnJaRoot.
	 * @throws LibraryNotLoadedException
	 *             if AnJaRoot isn't installed
	 */
	public static Version getNativeVersion() throws LibraryNotLoadedException {
		return Library.getNativeVersion();
	}

	/**
	 * Check the current access level (active of inactive).
	 * 
	 * <p>
	 * This method may be used to check whether the calling all is currently
	 * able to use root capabilities.
	 * </p>
	 * 
	 * @return <code>true</code> if the privileges for this app are currently
	 *         raised, otherwise <code>false</code>
	 */
	public static boolean isAccessActive() {
		if (!isAccessGranted()) {
			return false;
		}

		try {
			UserIds uids = Credentials.getUserIds();
			return uids.getReal() == 0;
		} catch (Exception e) {
			return false;
		}
	}

	/**
	 * Gain root access
	 * 
	 * <p>
	 * This method raises the calling app from an unprivileged state to a
	 * privileged state where root capabilities are granted and uid and gid are
	 * set both to 0 (root).
	 * </p>
	 * 
	 * @return <code>true</code> if gain was successful, otherwise
	 *         <code>false</code>
	 */
	public static boolean gainAccess() {
		if (!isAccessGranted()) {
			return false;
		}

		if (isAccessActive()) {
			return true;
		}

		try {
			originalUids = Credentials.getUserIds();
			originalGids = Credentials.getGroupIds();
			originalCaps = Capabilities.getCapabilities();
		} catch (Exception e) {
			return false;
		}

		try {
			Capabilities.setCapabilities(rootCaps);
			Credentials.setUserIds(rootUids);
			Credentials.setGroupIds(rootGids);
		} catch (Exception e) {
			emergencyAccessDrop();
			return false;
		}

		return true;
	}

	/**
	 * Drop privileged state after usage.
	 * 
	 * <p>
	 * This method drops the (possibly) acquired privileged state and restores
	 * the unprivileged state back. It's highly encouraged to do so because the
	 * Android system is very sensitive to uid and gid changed of a process
	 * (access is checked, for example, by referring to the callers uid and
	 * gid).
	 * </p>
	 * 
	 * @return <code>true</code> is returned in case privileges have been
	 *         restored, <code>false</code> otherwise.
	 */
	public static boolean dropAccess() {
		if (!isAccessActive()) {
			return true;
		}

		try {
			Credentials.setGroupIds(originalGids);
		} catch (Exception e) {
			emergencyAccessDrop();
			return false;
		}

		try {
			Credentials.setUserIds(originalUids);
		} catch (Exception e) {
			emergencyAccessDrop();
			return false;
		}

		try {
			Capabilities.setCapabilities(originalCaps);
		} catch (Exception e) {
			return false;
		}

		return true;
	}

	/**
	 * Open the AnJaRoot Play Store entry.
	 * 
	 * <p>
	 * You may call this method to offer the user a possibility to install
	 * AnJaRoot. It will open the Play Store via
	 * {@link Context#startActivity(Intent)} immediately.
	 * </p>
	 * 
	 * <p>
	 * This method should be used after you have detected that AnJaRoot is not
	 * installed (via {@link #isInstalled()}).
	 * </p>
	 * 
	 * @param context
	 *            a context used to spawn the Play Store
	 */
	public static void openPlayStore(Context context) {
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri
				.parse("market://details?id=org.failedprojects.anjaroot"));
		context.startActivity(intent);
	}

	private static void emergencyAccessDrop() {
		try {
			Credentials.setGroupIds(originalGids);
		} catch (Exception ignored) {
		}

		try {
			Credentials.setUserIds(originalUids);
		} catch (Exception ignored) {
		}

		try {
			Capabilities.setCapabilities(originalCaps);
		} catch (Exception ignored) {
		}
	}
}
