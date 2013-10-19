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

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

public class InstallFragment extends Fragment {

	private Installer installer;
	private ProgressDialog dialog;

	public static InstallFragment newInstance() {
		return new InstallFragment();
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setHasOptionsMenu(true);

		Context ctx = getActivity();
		installer = new Installer(ctx);

		// prepare dialog
		dialog = new ProgressDialog(ctx);
		dialog.setTitle(ctx.getText(R.string.installer_working_title));
		dialog.setMessage(ctx.getText(R.string.installer_working_message));
		dialog.setIndeterminate(true);
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.install_fragment, container, false);
		Button b = (Button) v.findViewById(R.id.install_fragment_install_btn);
		b.setOnClickListener(new View.OnClickListener() {

			@Override
			public void onClick(View v) {
				AlertDialog.Builder builder = new AlertDialog.Builder(
						getActivity());
				builder.setTitle(R.string.install_fragment_install_dialog_title);
				builder.setMessage(R.string.install_fragment_install_dialog_msg);
				builder.setPositiveButton(
						R.string.install_fragment_install_dialog_positive,
						new OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog,
									int which) {
								installer
										.doRecoveryInstall(recoveryInstallHandler);
							}
						});
				builder.setNegativeButton(
						R.string.install_fragment_install_dialog_negative, null);
				builder.create().show();
			}
		});

		Button rb = (Button) v.findViewById(R.id.install_fragment_reboot_btn);
		rb.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				AlertDialog.Builder builder = new AlertDialog.Builder(
						getActivity());
				builder.setTitle(R.string.install_fragment_reboot_title);
				builder.setMessage(R.string.install_fragment_reboot_msg);
				builder.setPositiveButton(
						R.string.install_fragment_reboot_positive,
						new OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog,
									int which) {
								installer
										.doRecoveryReboot(recoveryRebootHandler);
							}
						});
				builder.setNegativeButton(
						R.string.install_fragment_reboot_negative, null);
				builder.create().show();
			}
		});

		return v;
	}

	void dismissProgressDialog() {
		dialog.dismiss();
	}

	void showProgressDialog() {
		dialog.show();
	}

	void showRebootInterface() {
		View v = getView();
		v.findViewById(R.id.install_fragment_install_btn).setVisibility(
				View.GONE);
		v.findViewById(R.id.install_fragment_install_text).setVisibility(
				View.GONE);

		v.findViewById(R.id.install_fragment_reboot_btn).setVisibility(
				View.VISIBLE);
		v.findViewById(R.id.install_fragment_reboot_text).setVisibility(
				View.VISIBLE);
	}

	Installer.Handler recoveryInstallHandler = new Installer.Handler() {

		@Override
		public void preExecute() {
			showProgressDialog();
		}

		@Override
		public void postExecute(boolean result) {
			dismissProgressDialog();

			int rid;
			if (result) {
				showRebootInterface();
				rid = R.string.installer_toast_success;
			} else {
				rid = R.string.installer_toast_failure;
			}
			Toast.makeText(getActivity(), rid, Toast.LENGTH_LONG).show();
		}
	};

	Installer.Handler recoveryRebootHandler = new Installer.Handler() {

		@Override
		public void preExecute() {
			showProgressDialog();
		}

		@Override
		public void postExecute(boolean result) {
			dismissProgressDialog();

			if (!result) {
				Toast.makeText(getActivity(),
						R.string.installer_toast_reboot_failure,
						Toast.LENGTH_LONG).show();
			}
		}
	};
}
