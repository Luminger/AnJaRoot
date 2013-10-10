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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

import android.content.Context;

public class GrantedStorage {
	private final Context context;
	private final String filename;

	public GrantedStorage(Context ctx) {
		context = ctx;
		filename = ctx.getFilesDir() + "/granted";
	}

	public List<String> getPackages() {
		ArrayList<String> ret = new ArrayList<String>();

		try {
			FileInputStream inputstream = new FileInputStream(filename);
			InputStreamReader inputreader = new InputStreamReader(inputstream);
			BufferedReader bufreader = new BufferedReader(inputreader);

			String line;
			do {
				line = bufreader.readLine();
				if (line == null) {
					break;
				}
				ret.add(line);
			} while (true);

			bufreader.close();
			inputreader.close();
			inputstream.close();
		} catch (IOException e) {
		}

		return ret;
	}

	private void write(List<String> granted) {
		try {
			File tmp = File.createTempFile("granted", "tmp",
					context.getFilesDir());
			FileWriter writer = new FileWriter(tmp);
			BufferedWriter bufwriter = new BufferedWriter(writer);

			for (String pkg : granted) {
				bufwriter.write(pkg);
				bufwriter.newLine();
			}

			bufwriter.flush();
			bufwriter.close();
			writer.close();

			tmp.renameTo(new File(filename));
			tmp.setReadOnly();
		} catch (IOException e) {
		}
	}

	public void addPackage(String name) {
		List<String> granted = getPackages();
		if (!granted.contains(name)) {
			granted.add(name);
			write(granted);
		}
	}

	public void removePackage(String name) {
		List<String> granted = getPackages();
		if (granted.contains(name)) {
			granted.remove(name);
			write(granted);
		}
	}

	public boolean isGranted(String pkgname) {
		List<String> granted = getPackages();
		return granted.contains(pkgname);
	}
}