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

import java.util.List;

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSExtensionClient;
import org.naregi.rns.stubs.ProfileType;
import org.naregi.rns.test.RNSBenchmark;

/**
 * rns-profile
 */
public class RNS_profile {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		boolean usage = true;
		try {
			home.setCustomUsage("on|off");
			List<String> l = home.parseArgs(args, 1, 1);
			RNSExtensionClient rnsx = home.getRNSExtensionClient();
			String onoff = l.get(0);
			usage = false;
			if ("on".equals(onoff)) {
				rnsx.startProfile();
			} else if ("off".equals(onoff)) {
				ProfileType[] profs = rnsx.stopProfile();
				if (profs != null) {
					for (ProfileType prof : profs) {
						System.out.println(RNSBenchmark.format("[SERVER] "
								+ prof.getName(), prof.getTotal().longValue(),
								prof.getCount().intValue()));
					}
				}
			} else {
				usage = true;
				throw RNSError.createEINVAL(null, "on or off", null);
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
