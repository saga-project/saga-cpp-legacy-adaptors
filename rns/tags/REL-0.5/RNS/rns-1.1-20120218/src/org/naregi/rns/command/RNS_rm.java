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

import java.util.Map;

import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;

/**
 * rns-rm
 */
public class RNS_rm {
	public static void main(String[] args) {
		RNSClientHome base = new RNSClientHome();
		base.setCustomUsage("[+r|++recursive depth] [+f|++force] RNS_dir...");
		boolean usage = true;
		try {
			String[] shortFlags = { "f" };
			String[] longFlags = { "force" };
			String[] shortKeys = { "r" };
			String[] longKeyss = { "recursive" };
			Map<String, String> argMap = base.parseArgsWithPlusOption(args,
					shortFlags, longFlags, shortKeys, longKeyss, "arg", 1, -1);
			RNSClient rnsclient = base.getRNSClient();
			usage = false;

			boolean force = (argMap.containsKey("+f") || argMap.containsKey("++force"));

			String depthStr = argMap.get("++recursive");
			if (depthStr == null) {
				depthStr = argMap.get("+r");
			}

			int i = 0;
			String path;
			while ((path = argMap.get("arg" + i)) != null) {
				if (depthStr != null) {
					rnsclient.removeRecursive(path, Integer.parseInt(depthStr));
				} else if (force) {
					rnsclient.removeForce(path);
				} else {
					rnsclient.rmJunction(path);
				}
				i++;
			}
		} catch (Exception e) {
			base.printError(e, System.err);
			if (usage) {
				base.printUsage(System.out);
			}
			System.exit(1);
			return;
		}
	}
}
