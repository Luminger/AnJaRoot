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

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
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
		
		try {
			NativeWrapper.getUserIds();
			Log.v(LOGTAG, String.format("Version: %s", NativeWrapper.getVersion().toString()));
		} catch (NativeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		};
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
