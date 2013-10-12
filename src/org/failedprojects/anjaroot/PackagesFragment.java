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

import java.util.ArrayList;
import java.util.List;

import org.failedprojects.anjaroot.GrantedStorage.OnChangeHandler;
import org.failedprojects.anjaroot.library.AnJaRoot;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.os.Process;
import android.support.v4.app.ListFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

public class PackagesFragment extends ListFragment implements OnChangeHandler {
	private static final String LOGTAG = "AnjaRoot";
	private PackageManager pm;
	private GrantedStorage storage;

	class PackagesAdapter extends ArrayAdapter<String> {
		private class ViewHolder {
			TextView name;
			TextView pkgname;
			ImageView icon;
		}

		public PackagesAdapter(Context context, List<String> objects) {
			super(context, R.layout.packages_list_item, objects);
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			View v = convertView;
			if (v == null) {
				LayoutInflater inflater = (LayoutInflater) getActivity()
						.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
				v = inflater
						.inflate(R.layout.packages_list_item, parent, false);

				ViewHolder holder = new ViewHolder();
				holder.name = (TextView) v.findViewById(R.id.name);
				holder.pkgname = (TextView) v.findViewById(R.id.package_name);
				holder.icon = (ImageView) v.findViewById(R.id.icon);
				v.setTag(holder);
			}

			String pkgname = getItem(position);
			try {
				PackageInfo pi = pm.getPackageInfo(pkgname, 0);

				ViewHolder holder = (ViewHolder) v.getTag();
				holder.name.setText(pi.applicationInfo.loadLabel(pm));
				holder.pkgname.setText(pkgname);
				holder.icon.setImageDrawable(pi.applicationInfo.loadIcon(pm));
			} catch (NameNotFoundException e) {
				Log.e(LOGTAG, "Failed to get PackageInfo", e);
			}

			return v;
		}

	}

	public static PackagesFragment newInstance() {
		return new PackagesFragment();
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setHasOptionsMenu(true);

		storage = new GrantedStorage(getActivity());
		pm = getActivity().getPackageManager();

		setListAdapter(new PackagesAdapter(getActivity(), storage.getPackages()));
	}

	@Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		PackagesAdapter pa = (PackagesAdapter) getListAdapter();
		final String pkg = pa.getItem(position);
		String pkgname = "";

		try {
			PackageInfo pi = pm.getPackageInfo(pkg, 0);
			pkgname = (String) pi.applicationInfo.loadLabel(pm);
		} catch (NameNotFoundException e) {
			Log.e(LOGTAG, "Failed to load label", e);
		}

		AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity());
		dialog.setTitle(R.string.packages_list_delete_single_title);

		dialog.setMessage(String.format(
				getString(R.string.packages_list_delete_single_msg), pkgname));
		dialog.setPositiveButton(R.string.packages_list_delete_single_positive,
				new OnClickListener() {
					@Override
					public void onClick(DialogInterface dialog, int which) {
						// this will trigger the OnChangeHandler
						storage.removePackage(pkg);

						List<String> remove = new ArrayList<String>(1);
						remove.add(pkg);
						killProcesses(new ArrayList<String>(remove));
					}
				});
		dialog.setNegativeButton(R.string.packages_list_delete_single_negative,
				null);
		dialog.create().show();
	}

	@Override
	public void onResume() {
		super.onResume();

		PackagesAdapter pa = (PackagesAdapter) getListAdapter();
		pa.clear();

		storage.forceReload();
		pa.addAll(storage.getPackages());
		storage.startObserving(this);
	}

	@Override
	public void onPause() {
		super.onPause();

		storage.stopObserving();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		return inflater.inflate(R.layout.packages_list, null);
	}

	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
		inflater.inflate(R.menu.packages_list_menu, menu);

		if (isDonateAppPresent()) {
			menu.removeItem(R.id.action_buy_donate);
		}
		super.onCreateOptionsMenu(menu, inflater);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case R.id.action_delete_all:
			AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity());
			dialog.setTitle(R.string.packages_list_delete_all_title);
			dialog.setMessage(R.string.packages_list_delete_all_msg);
			dialog.setPositiveButton(
					R.string.packages_list_delete_all_positive,
					new OnClickListener() {
						@Override
						public void onClick(DialogInterface dialog, int which) {
							List<String> remove = storage.getPackages();
							storage.removeAll();
							killProcesses(remove);
						}
					});
			dialog.setNegativeButton(
					R.string.packages_list_delete_all_negative, null);
			dialog.create().show();
			return true;
		case R.id.action_buy_donate:
			Intent intent = new Intent(Intent.ACTION_VIEW);
			intent.setData(Uri
					.parse("market://details?id=org.failedprojects.anjaroot.donate"));
			startActivity(intent);
			return true;
		default:
			return super.onOptionsItemSelected(item);
		}

	}

	public void killProcesses(List<String> pkgs) {
		boolean success = AnJaRoot.gainAccess();
		if (!success) {
			Log.e(LOGTAG,
					"Failed to gain access, something must be seriously broken...");
			return;
		}

		ActivityManager am = (ActivityManager) getActivity().getSystemService(
				Context.ACTIVITY_SERVICE);
		List<RunningAppProcessInfo> runningapps = am.getRunningAppProcesses();
		for (RunningAppProcessInfo info : runningapps) {
			for (String pkgname : info.pkgList) {
				if (!pkgs.contains(pkgname)) {
					continue;
				}

				Log.v(LOGTAG, String.format(
						"Killing %s with pid %d as access has been revoked",
						pkgname, info.pid));
				Process.killProcess(info.pid);
			}
		}

		success = AnJaRoot.dropAccess();
		if (!success) {
			Log.e(LOGTAG, "Failed to drop access level");
		}
	}

	@Override
	public void onReload(boolean success) {
		getActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				PackagesAdapter pa = (PackagesAdapter) getListAdapter();
				pa.clear();
				pa.addAll(storage.getPackages());
			}
		});
	}

	private boolean isDonateAppPresent() {
		int sigmatch = pm.checkSignatures("org.failedprojects.anjaroot",
				"org.failedprojects.anjaroot.donate");
		if (sigmatch != PackageManager.SIGNATURE_MATCH) {
			return false;
		}

		return true;
	}
}
