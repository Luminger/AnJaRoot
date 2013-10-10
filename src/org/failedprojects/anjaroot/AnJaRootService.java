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

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Process;
import android.os.RemoteException;
import android.util.Log;

public class AnJaRootService extends Service {
	private static final String LOGTAG = "AnJaRootService";
	private static final int timeout = 11 * 1000;
	private static List<RequestResult> requestResultWaitingList = new CopyOnWriteArrayList<AnJaRootService.RequestResult>();

	private class RequestResult {
		private final int uid;
		private boolean granted = false;
		private boolean handled = false;

		public RequestResult(int uid) {
			this.uid = uid;
		}

		public int getUid() {
			return uid;
		}

		public boolean isGranted() {
			return granted;
		}

		public void setGranted(boolean granted) {
			this.granted = granted;
		}

		public synchronized void setHandled() {
			this.handled = true;
		}

		public synchronized boolean isHandled() {
			return this.handled;
		}
	}

	private class ServiceImplementation extends IAnJaRootService.Stub {
		@Override
		public boolean requestAccess() throws RemoteException {
			RequestResult result = new RequestResult(getCallingUid());
			requestResultWaitingList.add(result);

			Intent intent = new Intent(getApplicationContext(),
					RequestActivity.class);
			intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			intent.putExtra("uid", getCallingUid());
			startActivity(intent);

			// TODO polling is bad...
			Log.v(LOGTAG, "Activity started, waiting for answer...");
			long start = System.currentTimeMillis();
			while ((System.currentTimeMillis() - timeout) < start) {
				try {
					Thread.sleep(100);
					if (result.isHandled()) {
						Log.v(LOGTAG, "Request answered");
						break;
					}
				} catch (InterruptedException e) {
					Log.v(LOGTAG, "Wait interrupted", e);
				}
			}

			requestResultWaitingList.remove(result);

			if (!result.isHandled()) {
				Log.v(LOGTAG, "Request wasn't answered within time");
				return false;
			}

			return result.isGranted();
		}

		@Override
		public void answerAccessRequest(int uid, boolean granted)
				throws RemoteException {
			if (Process.myUid() != getCallingUid()) {
				Log.e(LOGTAG,
						"Client is not allowed to call answerAccessRequest()");
				return;
			}

			for (RequestResult current : requestResultWaitingList) {
				if (current.getUid() == uid) {
					current.setGranted(granted);
					current.setHandled();
					Log.v(LOGTAG, String.format(
							"Handled request for %d with %b", uid, granted));
					return;
				}
			}

			Log.e(LOGTAG, String.format("Failed to handle request for %d", uid));
		}
	}

	@Override
	public void onCreate() {
		super.onCreate();
		Log.v(LOGTAG, "Service started");
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		Log.v(LOGTAG, "Sercive destroyed");
	}

	@Override
	public IBinder onBind(Intent intent) {
		return new ServiceImplementation();
	}
}
