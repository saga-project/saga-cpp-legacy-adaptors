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
import org.naregi.rns.client.RNSClient;

/**
 * rns-mv
 */
public class RNS_rename {
	public static void main(String[] args) {
		RNSClientHome base = new RNSClientHome();
		base.setCustomUsage("RNS_from RNS_to");
		boolean usage = true;
		try {
			List<String> al = base.parseArgs(args, 2, 2);
			RNSClient rnsclient= base.getRNSClient();
			String from = al.get(0);
			String to = al.get(1);
			usage = false;
			rnsclient.rename(from, to);
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
