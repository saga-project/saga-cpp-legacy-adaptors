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

import org.naregi.rns.ACL;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSExtensionClient;

/**
 * rns-chmod
 */
public class ACL_chmod {

	private static String convertAclSpecs(String modeString) throws Exception {
		if (modeString == null) {
			return null;
		}
		StringBuilder retStr = new StringBuilder();

		char[] c = modeString.toCharArray();
		int len = c.length;
		if (len >= 1 && len <= 4 && c[0] >= '0' && c[0] <= '7') {
			/* ex. 7, 55, 755, 0644 */
			retStr.append("o:" + ACL.permToString((short) Short.parseShort(String.valueOf(c[len - 1]))));
			if (len >= 2) {
				retStr.append(",og::" + ACL.permToString((short) Short.parseShort(String.valueOf(c[len - 2]))));
			}
			if (len >= 3) {
				retStr.append(",ou::" + ACL.permToString((short) Short.parseShort(String.valueOf(c[len - 3]))));
			}
			/* len == 4 ... ignore */
			return retStr.toString();
		}
		throw new Exception("not supported: " + modeString);
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("mode RNS_path");
		boolean usage = true;
		try {
			List<String> al = home.parseArgs(args, 2, 2);
			String aclSpecs = convertAclSpecs(al.get(0));
			String path = al.get(1);
			usage = false;
			RNSExtensionClient client = home.getRNSExtensionClient();
			client.setACL(path, aclSpecs);
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
