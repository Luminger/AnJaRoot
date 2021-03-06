package org.failedprojects.anjaroot;

import org.failedprojects.anjaroot.library.AnJaRoot;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;

public class MainActivity extends ActionBarActivity {
	private ActionBar ab;

	@Override
	protected void onCreate(Bundle arg0) {
		super.onCreate(arg0);

		setContentView(R.layout.activity_main);

		ab = getSupportActionBar();
		ab.setTitle(R.string.anjaroot);

		Fragment fragment;
		boolean installed = AnJaRoot.isInstalled();
		if (installed) {
			fragment = PackagesFragment.newInstance();
		} else {
			fragment = InstallFragment.newInstance();
		}

		getSupportFragmentManager().beginTransaction()
				.replace(R.id.main_activity_layout, fragment).commit();
	}

	public void openInfoFragment() {
		InfoFragment fragment = InfoFragment.newInstance();

		FragmentTransaction trans = getSupportFragmentManager()
				.beginTransaction();
		trans.replace(R.id.main_activity_layout, fragment);
		trans.addToBackStack(null);
		trans.setCustomAnimations(R.anim.abc_fade_in, R.anim.abc_fade_out);
		trans.commit();
	}
}
