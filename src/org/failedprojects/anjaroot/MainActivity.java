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

import java.io.IOException;

import org.failedprojects.anjaroot.library.exceptions.NativeException;
import org.failedprojects.anjaroot.library.internal.NativeWrapper;

import android.os.Bundle;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Button;

public class MainActivity extends FragmentActivity {
	
	static final String LOGTAG = "AnjaRoot";
	
	ProgressDialog createProgessDialog() {
		ProgressDialog dial = new ProgressDialog(this);
		dial.setTitle("AnJaRoot");
		dial.setMessage("Testing root access...");
		dial.setIndeterminate(true);
		dial.show();
		
		return dial;
	}
	
	void doSystemInstall() {
		final Context ctx = this;
		final ProgressDialog dial = createProgessDialog();
		new Thread() {
			public void run() {

				final String basepath = ctx.getApplicationInfo().dataDir + "/lib/";
				final String library = basepath + "libanjaroot.so";
				final String installer = basepath + "libanjarootinstaller.so";
				final String command = 
						"mount -orw,remount /system\n" +
						String.format("%s -i -s '%s'\n", installer, library) +
						"/system/bin/setprop ctl.stop zygote\n" + 
						"/system/bin/setprop ctl.start zygote\n" + 
						"mount -oro,remount /system\n";
				
				try {
					Process p = Runtime.getRuntime().exec("su");
					p.getOutputStream().write(command.getBytes());
					p.getOutputStream().close();
					
					if(p.waitFor() != 0)
					{
						Log.e(LOGTAG, "Non zero result");
					}
				} catch (Exception e) {
					e.printStackTrace();
					Log.e(LOGTAG, "Failed to install", e);
				}
				
				dial.dismiss();
			}
		}.run();
	}

	void doSystemUninstall() {
		final Context ctx = this;
		final ProgressDialog dial = createProgessDialog();
		new Thread() {
			public void run() {

				final String basepath = ctx.getApplicationInfo().dataDir + "/lib/";
				final String installer = basepath + "libanjarootinstaller.so";
				final String command = 
						"mount -orw,remount /system\n" +
						String.format("%s --uninstall\n", installer) +
						"/system/bin/setprop ctl.stop zygote\n" + 
						"/system/bin/setprop ctl.start zygote\n" + 
						"mount -oro,remount /system\n";
				
				try {
					Process p = Runtime.getRuntime().exec("su");
					p.getOutputStream().write(command.getBytes());
					p.getOutputStream().close();
					
					if(p.waitFor() != 0)
					{
						Log.e(LOGTAG, "Non zero result");
					}
				} catch (Exception e) {
					e.printStackTrace();
					Log.e(LOGTAG, "Failed to install", e);
				}
				
				dial.dismiss();
			}
		}.run();
	}
	
	void doRecoveryInstall() {
		
	}
	
	void createInstallDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("AnJaRoot");
		builder.setMessage("Do you want to install AnJaRoot?");
		builder.setPositiveButton("Install", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				doSystemInstall();
			}
		});
		builder.setNeutralButton("Recovery", new OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				doRecoveryInstall();
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
				doSystemUninstall();
			}
		});
		builder.setNegativeButton("Cancel", null);
		builder.create().show();
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Button install = (Button)findViewById(R.id.installBtn);
		install.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				createInstallDialog();
			}
		});
		
		Button uninstall = (Button)findViewById(R.id.uninstallBtn);
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
