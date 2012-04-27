/*
 * Copyright (C) 2008-2011 Osaka University.
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Map;

import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;

/**
 * rns-bulk-remove
 */
public class RNS_bulk_remove {

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[option] RNS_dir inputFile(a name per line)");
		home.addHelp("+i,++ignore", "Ignore ENOENT error");

		RNSClient rnsclient;
		boolean usage = true;
		try {
			String[] shortFlags = { "i" };
			String[] longFlags = { "ignore" };
			String[] shortKeys = null;
			String[] longKeyss = null;
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					shortFlags, longFlags, shortKeys, longKeyss, "arg", 2, 2);

			String dir = argMap.get("arg0");
			String inputFile = argMap.get("arg1");

			rnsclient = home.getRNSClient();
			usage = false;

			boolean ignore = (argMap.containsKey("+i") || argMap.containsKey("++ignore"));

			BufferedReader br;
			if ("-".equals(inputFile)) {
				br = new BufferedReader(new InputStreamReader(System.in));
			} else {
				br = new BufferedReader(new FileReader(inputFile));
			}
			ArrayList<String> al = new ArrayList<String>();
			String line;
			while ((line = br.readLine()) != null) {
				if (line == null || line.length() == 0) {
					continue;
				}
				al.add(line);
			}
			br.close();

			RNSError[] errors = rnsclient.removeBulk(dir,
					al.toArray(new String[0]));
			if (errors != null && errors.length > 0) {
				boolean hasError = false;
				for (RNSError error : errors) {
					if (error.getError().equals(RNSError.Errno.ENOENT)) {
						if (ignore == false) {
							home.printError(error, System.err);
							hasError = true;
						}
					} else {
						home.printError(error, System.err);
						hasError = true;
					}
				}
				if (hasError) {
					System.exit(1);
				}
			}
		} catch (Exception e) {
			home.printError(e, System.err);
			if (usage) {
				home.printUsage(System.out);
			}
			System.exit(1);
			return;
		}
	}
}
