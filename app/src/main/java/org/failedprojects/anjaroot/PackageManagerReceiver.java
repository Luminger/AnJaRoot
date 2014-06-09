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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;

public class PackageManagerReceiver extends BroadcastReceiver {
	private static final String LOGTAG = "AnJaRootReceiver";

	@Override
	public void onReceive(Context context, Intent intent) {
		if (Intent.ACTION_PACKAGE_REMOVED.equals(intent.getAction())) {
			Uri uri = intent.getData();
			if (uri == null) {
				Log.e(LOGTAG, "Received intent had no data attached");
				return;
			}

			String pkg = uri.getSchemeSpecificPart();
			Log.v(LOGTAG, String.format("Received intent for %s", pkg));

			GrantedStorage storage = new GrantedStorage(context);
			storage.removePackage(pkg);
		}

	}
}
