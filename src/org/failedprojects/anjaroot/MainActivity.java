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
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

public class MainActivity extends FragmentActivity {
	private static final String LOGTAG = "AnjaRoot";
	private Installer installer;

	void createInstallDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("AnJaRoot");
		builder.setMessage("Do you want to install AnJaRoot?");
		builder.setPositiveButton("Install", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				installer.doSystemInstall();
			}
		});
		builder.setNeutralButton("Recovery", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				installer.doRecoveryInstall();
			}
		});
		builder.setNegativeButton("Cancel", null);
		builder.create().show();
	}

	void createUninstallDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("AnJaRoot");
		builder.setMessage("Do you want to uninstall AnJaRoot?");
		builder.setPositiveButton("Uninstall", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				installer.doSystemUninstall();
			}
		});
		builder.setNegativeButton("Cancel", null);
		builder.create().show();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		installer = new Installer(this);

		Button install = (Button) findViewById(R.id.installBtn);
		install.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				createInstallDialog();
			}
		});

		Button uninstall = (Button) findViewById(R.id.uninstallBtn);
		uninstall.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				createUninstallDialog();
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
