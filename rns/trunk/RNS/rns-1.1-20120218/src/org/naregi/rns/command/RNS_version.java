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

import org.naregi.rns.RNSVersion;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSExtensionClient;

/**
 * rns-version
 */
public class RNS_version {
	public static void main(String[] args) {
		String cver = RNSVersion.getVersion();
		System.out.println("RNS Client: " + cver);

		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("count");
		boolean usage = true;
		try {
			home.parseArgs(args, 0, 0);
			usage = false;
			RNSExtensionClient client = home.getRNSExtensionClient();
			String sver = client.getServerVersion();
			System.out.println("RNS Server: " + sver);
			if (cver.equals(sver) == false) {
				System.exit(255);
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
