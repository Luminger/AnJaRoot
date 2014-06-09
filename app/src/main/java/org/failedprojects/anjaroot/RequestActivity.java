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

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class RequestActivity extends Activity implements OnClickListener {
	private static final String LOGTAG = "AnJaRootRequestActivity";
	private static final int timeout = 10 * 1000;
	private final Handler handler = new Handler();

	private IAnJaRootService service;
	private final ServiceConnection connection = new ServiceConnection() {
		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.v(LOGTAG, "Service disconnected");
			service = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder binder) {
			service = IAnJaRootService.Stub.asInterface(binder);
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		handler.postDelayed(new Runnable() {
			@Override
			public void run() {
				Log.v(LOGTAG, "Timeout triggered");
				answerRequest(false);
			}
		}, timeout);
		setContentView(R.layout.request_activity);

		Button grant = (Button) findViewById(R.id.grant_btn);
		grant.setOnClickListener(this);
		Button deny = (Button) findViewById(R.id.deny_btn);
		deny.setOnClickListener(this);

		Intent intent = getIntent();
		int uid = intent.getIntExtra("uid", -1);
		PackageManager pm = getPackageManager();
		String[] pkgs = pm.getPackagesForUid(uid);

		if (pkgs.length < 1) {
			Log.e(LOGTAG,
					String.format("Failed to get package for uid %d", uid));
			return;
		}

		if (pkgs.length > 1) {
			Log.e(LOGTAG,
					"There is more than one package with uid %d, displaying only the first");
			Log.e(LOGTAG, "Returned packages are:");
			for (String pkg : pkgs) {
				Log.e(LOGTAG, String.format("  %s", pkg));
			}
		}

		try {
			PackageInfo pi = pm.getPackageInfo(pkgs[0], 0);

			Drawable logo = pi.applicationInfo.loadIcon(pm);
			CharSequence name = pi.applicationInfo.loadLabel(pm);

			ImageView iv = (ImageView) findViewById(R.id.icon);
			iv.setImageDrawable(logo);

			TextView tv = (TextView) findViewById(R.id.icon_name);
			tv.setText(name);

			TextView t = (TextView) findViewById(R.id.title_text);
			t.setText(String.format(
					getString(R.string.request_activity_title_text), name));
		} catch (NameNotFoundException e) {
			Log.e(LOGTAG, "Failed to get PackageInfo", e);
		}
	}

	@Override
	protected void onResume() {
		super.onResume();

		Log.v(LOGTAG, "onResume() called, connecting to service");
		Intent intent = new Intent(
				"org.failedprojects.anjaroot.action.ANSWER_REQUEST");
		bindService(intent, connection, Context.BIND_AUTO_CREATE);
	}

	@Override
	protected void onPause() {
		super.onPause();

		Log.v(LOGTAG, "onPause() called, disconnecting from service");
		unbindService(connection);
	}

	@Override
	public void onClick(View v) {
		boolean granted = false;
		if (v.getId() == R.id.grant_btn) {
			granted = true;
		}

		answerRequest(granted);
	}

	private void answerRequest(boolean granted) {
		handler.removeMessages(0);

		Intent intent = getIntent();
		int uid = intent.getIntExtra("uid", -1);

		try {
			service.answerAccessRequest(uid, granted);
		} catch (RemoteException e) {
			Log.e(LOGTAG, "Failed to answerAccessRequest", e);
		}

		finish();

	}
}
