/*    Copyright 2013 Simon Brakhane
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.failedprojects.anjaroot.library;

import org.failedprojects.anjaroot.IAnJaRootService;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

/**
 * Request AnJaRoot access.
 * 
 * <p>
 * This class is used to request access to AnJaRoot. Use it in conjunction with
 * the main {@link AnJaRoot} class to gain access in the first place. If your
 * request is granted, this class it not needed anymore (until the user deletes
 * the given granting, of course).
 * </p>
 * 
 * <p>
 * It's a thin wrapper around {@link android.component.Context#bindService()
 * bindService()}, which should aid you in maintaining a healthy connection to
 * the service. You should instantiate it in your Fragments/Activity/Service
 * <code>onCreate()</code> method. The service connection will not be available
 * before at least <code>onCreate()</code> returned. It's good practice to
 * register a {@link ConnectionStatusListener} to get a notification if the
 * connection is ready for use.
 * </p>
 * 
 * @see The <a href="https://github.com/Luminger/AnJaRootTester">AnJaRoot
 *      Tester</a> for a reference implementation.
 */
public class AnJaRootRequester {
	private static final String LOGTAG = "AnJaRootLibraryRequester";
	private final Context context;
	private IAnJaRootService service;
	private ConnectionStatusListener listener;
	private final AnJaRootServiceConnection connection = new AnJaRootServiceConnection();

	private class AnJaRootServiceConnection implements ServiceConnection {

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Log.v(LOGTAG, "Disconnected from AnJaRootService");
			service = null;

			if (listener != null) {
				listener.onConnectionStatusChange(false);
			}
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder binder) {
			Log.v(LOGTAG, "Connected to AnJaRootService");
			service = IAnJaRootService.Stub.asInterface(binder);

			if (listener != null) {
				listener.onConnectionStatusChange(true);
			}
		}
	};

	private class AsyncRequestTask extends AsyncTask<Void, Void, Boolean> {
		private final AsyncRequestHandler handler;

		public AsyncRequestTask(AsyncRequestHandler handler) {
			this.handler = handler;
		}

		@Override
		protected Boolean doInBackground(Void... params) {
			try {
				return service.requestAccess();
			} catch (RemoteException e) {
				Log.e(LOGTAG, "Remote method failed", e);
				return null;
			}
		}

		@Override
		protected void onPostExecute(Boolean result) {
			if (result == null) {
				handler.onServiceUnrechable();
			} else {
				handler.onReturn(result);
			}
		}

	}

	/**
	 * Listener which informs about connection status changes.
	 * 
	 * <p>
	 * A call to {@link #requestAccess()} can only succeed if the connection is
	 * up.
	 * </p>
	 */
	public interface ConnectionStatusListener {
		/**
		 * Called whenever a status change occurred.
		 * 
		 * @param connected
		 *            <code>true</code> if connected, otherwise
		 *            <code>false</code>
		 */
		void onConnectionStatusChange(boolean connected);
	}

	/**
	 * Handler which informs about the returned granting status of an access
	 * request.
	 */
	public interface AsyncRequestHandler {
		/**
		 * Called when a decision about the access request has been made.
		 */
		void onReturn(boolean granted);

		/**
		 * Called when no connection to the AnJaRoot Service could be
		 * established.
		 */
		void onServiceUnrechable();
	}

	/**
	 * Constructor
	 * 
	 * @param context
	 *            A reference to the surrounding Application, Activity, Fragment
	 *            or Service. It will be saved in the resulting object.
	 * @param listener
	 *            A listener which will be called on status changes on the
	 *            connection, may be <code>null</code>
	 */
	public AnJaRootRequester(Context context, ConnectionStatusListener listener) {
		if (context == null) {
			throw new NullPointerException("context can't be null");
		}

		this.context = context;

		setConnectionStatusListener(listener);
		connectToService();
	}

	/**
	 * Set (or unset) the used listener which gets informed about connection
	 * status changes.
	 * 
	 * @param listener
	 *            May be <code>null</code> to unregister
	 */
	public void setConnectionStatusListener(ConnectionStatusListener listener) {
		this.listener = listener;
	}

	private boolean connectToService() {
		boolean ret = true;
		if (service == null) {
			try {
				final Intent intent = new Intent(
						"org.failedprojects.anjaroot.action.REQUEST_ACCESS");
				ret = context.bindService(intent, connection,
						Context.BIND_AUTO_CREATE);
			} catch (SecurityException e) {
				Log.e(LOGTAG,
						"Declare permission usage of android.permission.ACCESS_ANJAROOT in your Manifest");
				Log.e(LOGTAG,
						"Insufficent permissions to access AnJaRoot Service", e);
			}
		}

		if (!ret) {
			Log.e(LOGTAG,
					"Unable to connect to AnJaRoot Service, is AnJaRoot installed?");
		} else {
			Log.v(LOGTAG,
					"Bound to AnJaRoot Service, waiting for connection object");
		}
		return ret;
	}

	/**
	 * Manually issue a reconnect if connection has been lost or was
	 * unavailable.
	 * 
	 * <p>
	 * Normally this is not needed as every other method issues a reconnect if
	 * needed but it may be useful nevertheless.
	 * </p>
	 */
	public void reconnectToService() {
		connectToService();
	}

	/**
	 * Get information about the current connection state.
	 * 
	 * <p>
	 * You only need this method if you aren't using the provided listener
	 * possibility.
	 * </p>
	 * 
	 * @return if <code>true</code>, {@link #requestAccess()} may be used
	 */
	public boolean isConnectedToService() {
		connectToService();
		return service != null;
	}

	/**
	 * Request access to AnJaRoot.
	 * 
	 * <p>
	 * This method will (if the service connection is established) create a new
	 * dialog for the user where he may grant or deny your request. This method
	 * WILL BLOCK, don't call it on your UI thread, you will get ANRs as this
	 * method may block multiple seconds.
	 * </p>
	 * 
	 * <p>
	 * The result may be <code>false</code> if the connection wasn't up or the
	 * user denied your request. If <code>true</code> is returned the user
	 * granted your request and you are allowed to use AnJaRoot. To make this
	 * happen you have to completely restart your application process(es) as
	 * otherwise the granting can take no effect. Use
	 * {@link AnJaRoot#commitSuicideAndRestart(Context, android.app.PendingIntent)}
	 * or {@link AnJaRoot#commitSuicide()} to kill your process.
	 * </p>
	 * 
	 * <p>
	 * Please note that when the user has denied you access to AnJaRoot, by
	 * pressing the corresponding button in the presented ui, your app will be
	 * blocked for several seconds until it's allowed to issue another request.
	 * </p>
	 * 
	 * @return <code>true</code> if the user has granted your request,
	 *         <code>false</code> if the request timed out, couldn't be placed
	 *         or simply timed out.
	 */
	public boolean requestAccess() {
		connectToService();

		try {
			return service.requestAccess();
		} catch (RemoteException e) {
			Log.e(LOGTAG, "Remote method failed", e);
		}

		return false;
	}

	/**
	 * Request access to AnJaRoot.
	 * 
	 * <p>
	 * This method will (if the service connection is established) create a new
	 * dialog for the user where he may grant or deny your request.
	 * </p>
	 * 
	 * <p>
	 * This is the non blocking version of the request interface which should be
	 * preferred by users of this library as it provides a very convenient way
	 * of requesting access in a non blocking manner.
	 * </p>
	 * 
	 * <p>
	 * The {@link AsyncRequestHandler} handler behaves exactly like its
	 * synchronous brother {@link #requestAccess()} in terms of the returned
	 * value (return value is dispatched to
	 * {@link AsyncRequestHandler#onReturn(boolean)}).
	 * </p>
	 * 
	 * @param handler
	 *            a handler used to return the request result
	 */
	public void requestAccessAsync(AsyncRequestHandler handler) {
		if (handler == null) {
			throw new NullPointerException("handler can't be null");
		}

		AsyncRequestTask task = new AsyncRequestTask(handler);
		task.execute();
	}

}
