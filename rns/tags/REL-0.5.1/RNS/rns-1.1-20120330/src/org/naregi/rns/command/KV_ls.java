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

import java.util.Map;
import java.util.Set;

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSKeyValue;

/**
 * rns-kv-ls
 */
public class KV_ls {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("path");
		boolean usage = true;
		try {
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					null, null, null, null, "arg", 1, 1);
			usage = false;
			String path = argMap.get("arg0");

			RNSKeyValue kv = new RNSKeyValue(home, path);
			Set<String> list = kv.keySet();
			for (String key : list) {
				System.out.println(key);
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
