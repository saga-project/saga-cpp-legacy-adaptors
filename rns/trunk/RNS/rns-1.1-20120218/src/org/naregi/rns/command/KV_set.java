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

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSKeyValue;

/**
 * rns-kv-set
 */
public class KV_set {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("path key value");
		boolean usage = true;
		try {
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					null, null, null, null, "arg", 3, 3);
			usage = false;
			String path = argMap.get("arg0");
			String key = argMap.get("arg1");
			String value = argMap.get("arg2");
			RNSKeyValue kv = new RNSKeyValue(home, path);
			kv.put(key, value);
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
