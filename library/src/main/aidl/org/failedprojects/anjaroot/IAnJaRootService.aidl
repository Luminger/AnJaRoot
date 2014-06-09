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
package org.failedprojects.anjaroot;

/**
 * AnJaRootService exposed Interface
 *
 * It's very likely that you will never ever need to mess with this AIDL, it's 
 * here to be utilized by AnJaRootRequester. But you can obviously use it on your
 * own if the provided abstraction doesn't fulfill your requirements.
 *
 * WARNING: Never ever change the order of this functions, neither delete one!
 */
interface IAnJaRootService {
	/**
	 * Request access to AnJaRoot
	 * 
	 * Whenever called this method will either wait until the user decides to
	 * grant the request or denies it or return immediately if access is already
	 * granted. This method may block for a long time (multiple seconds) as the
	 * used timeout may trigger so don't call this method from the ui thread as
	 * it will most likely trigger an ANR.
	 *
	 * @return <code>true</code> if the user granted access, otherwise 
	 *         <code>false</code>
	 */
	boolean requestAccess();
	
	/**
	 * Internal AnJaRoot request answer interface
	 *
	 * Nothing for users, nobody except AnJaRoot itself is allowed to perform
	 * this action. It will just return without doing anything when called from
	 * the outside.
	 *
	 * It's used by AnJaRoot to communicate back the users decision whenever a
	 * request was issued.
	 */
	void answerAccessRequest(int uid, boolean granted);
}