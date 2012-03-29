/*
 * Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2010 National Institute of Informatics in Japan.
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

//package org.naregi.rns.client.command;

import java.util.List;

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;


public class RNS_rmdir {
	public static void main(String[] args) {
		RNSClientHome base = new RNSClientHome();
		base.setCustomUsage("RNS_path");
		boolean usage = true;
		
//		System.out.println("SRA java code!!");
		
		try {
			List<String> al = base.parseArgs(args, 1, 1);
			RNSClient rnsclient= base.getRNSClient();
			String path = al.get(0);
			usage = false;
			rnsclient.rmdir(path);
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
