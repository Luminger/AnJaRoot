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
import java.io.IOException;
import java.io.InputStream;

import android.app.ProgressDialog;
import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.Toast;

public class Installer {
	private static final String LOGTAG = "AnJaRootInstallerCls";

	private static enum InstallMode {
		SystemInstall, RecoveryInstall, SystemUninstall
	}

	private static enum RebootMode {
		Recovery, System
	}

	private final Context ctx;
	private final String installTemplate;

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

	private class InstallTask extends AsyncTask<InstallMode, Integer, Boolean> {
		private ProgressDialog dialog;

		private String getLibraryLocation() {
			final String basepath = ctx.getApplicationInfo().dataDir + "/lib/";
			return basepath + "libanjarootinstaller.so";
		}

		private String getApkLocation() {
			return ctx.getPackageCodePath();
		}

		private String buildCommand(InstallMode mode) {

			String command = "false";
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
			}

			return installTemplate.replace("%COMMAND%", command);
		}

		@Override
		protected void onPreExecute() {
			dialog = new ProgressDialog(ctx);
			dialog.setTitle(ctx.getText(R.string.installer_working_title));
			dialog.setMessage(ctx.getText(R.string.installer_working_message));
			dialog.setIndeterminate(true);
			dialog.show();
		}

		@Override
		protected Boolean doInBackground(InstallMode... params) {
			String command = buildCommand(params[0]);
			Log.v(LOGTAG, String.format("Running as su: \n%s", command));

			try {
				Process p = Runtime.getRuntime().exec("su");
				p.getOutputStream().write(command.getBytes());
				p.getOutputStream().close();

				int ret = p.waitFor();
				Log.v(LOGTAG, String.format("Command returned '%d'", ret));

				return ret == 0;
			} catch (Exception e) {
				e.printStackTrace();
				Log.e(LOGTAG, "Failed to run command", e);
			}

			return false;
		}

		@Override
		protected void onPostExecute(Boolean result) {
			dialog.dismiss();

			int rid;
			if (result) {
				rid = R.string.installer_toast_success;
			} else {
				rid = R.string.installer_toast_failure;
			}
			Toast.makeText(ctx, rid, Toast.LENGTH_LONG).show();
		}
	}

	private class RebootTask extends AsyncTask<RebootMode, Integer, Boolean> {
		private String buildCommand(RebootMode mode) {
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

			return String.format("su -c '%s %s'", getInstallerLocation(),
					cmdswitch);
		}

		@Override
		protected Boolean doInBackground(RebootMode... params) {
			try {
				String command = buildCommand(params[0]);
				Process p = Runtime.getRuntime().exec(command);
				p.getOutputStream().close();

				int ret = p.waitFor();
				Log.v(LOGTAG, String.format("Command returned '%d'", ret));

				return ret == 0;
			} catch (Exception e) {
				e.printStackTrace();
				Log.e(LOGTAG, "Failed to run command", e);
			}

			return false;
		}

		@Override
		protected void onPostExecute(Boolean result) {
			if (!result) {
				Toast.makeText(ctx, R.string.installer_toast_reboot_failure,
						Toast.LENGTH_LONG).show();
			}
		}
	}

	public void doRecoveryInstall() {
		Log.v(LOGTAG, "Starting recovery install process");
		InstallTask task = new InstallTask();
		task.execute(InstallMode.RecoveryInstall);
	}

	public void doSystemInstall() {
		Log.v(LOGTAG, "Starting system install process");
		InstallTask task = new InstallTask();
		task.execute(InstallMode.SystemInstall);
	}

	public void doSystemUninstall() {
		Log.v(LOGTAG, "Starting system uninstall process");
		InstallTask task = new InstallTask();
		task.execute(InstallMode.SystemUninstall);
	}

	public void doSystemReboot() {
		Log.v(LOGTAG, "Performing system reboot");
		RebootTask task = new RebootTask();
		task.execute(RebootMode.System);
	}

	public void doRecoveryReboot() {
		Log.v(LOGTAG, "Performing recovery reboot");
		RebootTask task = new RebootTask();
		task.execute(RebootMode.Recovery);
	}
}
