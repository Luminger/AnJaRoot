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
package org.failedprojects.anjaroot;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;

public class Installer {
	private static final String LOGTAG = "AnJaRootInstallerCls";
	private final Context ctx;
	private final String installTemplate;

	private static enum InstallMode {
		SystemInstall, RecoveryInstall, SystemUninstall
	}

	private static enum RebootMode {
		Recovery, System
	}

	public interface Handler {
		void preExecute();

		void postExecute(boolean result);
	}

	public Installer(Context ctx) {
		this.ctx = ctx;
		this.installTemplate = readFileFromAssets("install-template.sh");
	}

	private String readFileFromAssets(String file) {
		try {
			InputStream in = ctx.getAssets().open(file);
			int size = in.available();

			byte[] buf = new byte[size];
			in.read(buf);
			in.close();

			return new String(buf);

		} catch (IOException e) {
			Log.e(LOGTAG, "Failed to read install-template.sh", e);
		}

		return "";
	}

	private String getInstallerLocation() {
		final String basepath = ctx.getApplicationInfo().dataDir + "/lib/";
		final String installer = basepath + "libanjarootinstaller.so";

		if (new File(installer).exists()) {
			return installer;
		} else {
			return "/system/bin/anjarootinstaller";
		}
	}

	private int runWithSu(String[] command) {
		try {
			Process p = Runtime.getRuntime().exec(command);

			int ret = p.waitFor();
			Log.v(LOGTAG, String.format("Command returned '%d'", ret));

			return ret;
		} catch (Exception e) {
			e.printStackTrace();
			Log.e(LOGTAG, "Failed to run command", e);
		}

		return -1;
	}

	private class InstallTask extends AsyncTask<InstallMode, Integer, Boolean> {
		private final Handler handler;

		public InstallTask(Handler handler) {
			this.handler = handler;
		}

		private String getLibraryLocation() {
			final String basepath = ctx.getApplicationInfo().dataDir + "/lib/";
			return basepath + "libanjarootinstaller.so";
		}

		private String getApkLocation() {
			return ctx.getPackageCodePath();
		}

		private String buildCommand(InstallMode mode) {

			String command;
			switch (mode) {
			case RecoveryInstall:
				command = String.format("%s --recovery --apkpath='%s'\n",
						getInstallerLocation(), getApkLocation());
				break;
			case SystemInstall:
				command = String.format(
						"%s --install --apkpath='%s' --srclibpath='%s'\n",
						getInstallerLocation(), getApkLocation(),
						getLibraryLocation());
				break;
			case SystemUninstall:
				command = String.format("%s --uninstall\n",
						getInstallerLocation());
				break;
			default:
				command = "false";
				break;
			}

			return installTemplate.replace("%COMMAND%", command);
		}

		@Override
		protected void onPreExecute() {
			handler.preExecute();
		}

		@Override
		protected Boolean doInBackground(InstallMode... params) {
			String commandscript = buildCommand(params[0]);
			Log.v(LOGTAG, String.format("Running as su: \n%s", commandscript));

			File script = ctx.getFileStreamPath("installer.sh");
			try {
				FileOutputStream stream = ctx.openFileOutput("installer.sh", 0);
				stream.write(commandscript.getBytes());
				stream.flush();
				stream.close();
			} catch (IOException e) {
				Log.e(LOGTAG, "Failed to write installer.sh", e);
				return false;
			}

			String[] command = new String[] { "su", "-c",
					String.format("sh %s", script.getAbsolutePath()) };
			int ret = runWithSu(command);
			return ret == 0;
		}

		@Override
		protected void onPostExecute(Boolean result) {
			handler.postExecute(result);
		}
	}

	private class RebootTask extends AsyncTask<RebootMode, Integer, Boolean> {
		private final Handler handler;

		public RebootTask(Handler handler) {
			this.handler = handler;
		}

		private String[] buildCommand(RebootMode mode) {
			String cmdswitch;

			switch (mode) {
			case Recovery:
				cmdswitch = "--reboot-recovery";
				break;
			default:
			case System:
				cmdswitch = "--reboot-system";
				break;
			}

			return new String[] { "su", "-c",
					String.format("%s %s", getInstallerLocation(), cmdswitch) };
		}

		@Override
		protected Boolean doInBackground(RebootMode... params) {
			String[] command = buildCommand(params[0]);
			int ret = runWithSu(command);
			return ret == 0;
		}

		@Override
		protected void onPreExecute() {
			handler.preExecute();
		}

		@Override
		protected void onPostExecute(Boolean result) {
			handler.postExecute(result);
		}
	}

	public void doRecoveryInstall(Handler handler) {
		Log.v(LOGTAG, "Starting recovery install process");
		InstallTask task = new InstallTask(handler);
		task.execute(InstallMode.RecoveryInstall);
	}

	public void doSystemInstall(Handler handler) {
		Log.v(LOGTAG, "Starting system install process");
		InstallTask task = new InstallTask(handler);
		task.execute(InstallMode.SystemInstall);
	}

	public void doSystemUninstall(Handler handler) {
		Log.v(LOGTAG, "Starting system uninstall process");
		InstallTask task = new InstallTask(handler);
		task.execute(InstallMode.SystemUninstall);
	}

	public void doSystemReboot(Handler handler) {
		Log.v(LOGTAG, "Performing system reboot");
		RebootTask task = new RebootTask(handler);
		task.execute(RebootMode.System);
	}

	public void doRecoveryReboot(Handler handler) {
		Log.v(LOGTAG, "Performing recovery reboot");
		RebootTask task = new RebootTask(handler);
		task.execute(RebootMode.Recovery);
	}
}
