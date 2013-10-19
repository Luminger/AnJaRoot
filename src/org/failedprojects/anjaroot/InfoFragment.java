package org.failedprojects.anjaroot;

import org.failedprojects.anjaroot.library.AnJaRoot;
import org.failedprojects.anjaroot.library.containers.Version;
import org.failedprojects.anjaroot.library.exceptions.LibraryNotLoadedException;

import android.app.Activity;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

public class InfoFragment extends Fragment {

	public static InfoFragment newInstance() {
		return new InfoFragment();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		View v = inflater.inflate(R.layout.info_fragment, container, false);

		TextView tv = (TextView) v.findViewById(R.id.app_version);

		String nativeVersionStr = "N/A";
		try {
			Version nativeVersion = AnJaRoot.getNativeVersion();
			nativeVersionStr = nativeVersion.toString();
		} catch (LibraryNotLoadedException e) {
		}

		String appVersionStr = "N/A";
		try {
			Activity activity = getActivity();
			PackageManager pm = activity.getPackageManager();
			PackageInfo pi;
			pi = pm.getPackageInfo(activity.getPackageName(), 0);
			appVersionStr = pi.versionName;
		} catch (NameNotFoundException e) {
		}

		String text = String.format(getString(R.string.info_version),
				appVersionStr, nativeVersionStr);
		tv.setText(text);

		return v;
	}
}
