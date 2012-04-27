/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.naregi.rns.command;

import java.io.PrintStream;
import java.util.List;

import org.naregi.rns.client.RNSClientHome;

/**
 * rns-ls-l
 */
public class RNS_ls_long {
	public static void main(String[] args) {
		main(args, System.out, System.err);
	}

	public static void main(String[] args, PrintStream stdout,
			PrintStream stderr) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("RNS_path");
		boolean usage = true;
		try {
			List<String> al = home.parseArgs(args, 0, 1);
			String path;
			if (al.size() == 0) {
				path = null;
			} else {
				path = al.get(0);
			}
			usage = false;
			RNS_ls.printList(home, path, System.getProperty("line.separator"),
					stdout, true, false, false, true, false, false, false);
		} catch (Exception e) {
			home.printError(e, stderr);
			if (usage) {
				home.printUsage(stdout);
			}
			System.exit(1);
			return;
		}
	}
}
