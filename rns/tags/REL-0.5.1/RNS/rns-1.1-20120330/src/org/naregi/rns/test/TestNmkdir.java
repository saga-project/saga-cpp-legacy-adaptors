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
package org.naregi.rns.test;

import java.util.List;

import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;

/**
 * Create many directories.
 */
public class TestNmkdir {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		boolean usage = true;
		try {
			home.setCustomUsage("RNS_dir number");
			List<String> al = home.parseArgs(args, 2, 2);
			RNSClient rnsclient = home.getRNSClient();
			String path = al.get(0);
			int num = Integer.parseInt(al.get(1));
			usage = false;

			try {
				rnsclient.mkdir(path);
			} catch (RNSError re) {
				if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
					throw re;
				}
			}
			for (int i = 0; i < num; i++) {
				if (i % 1000 == 0) {
					System.out.println(((double) i / (double) num) * 100 + "% (now=" + i + ")");
				}
				try {
					rnsclient.mkdir(path + "/" + i, null);
				} catch (RNSError re) {
					if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
						throw re;
					}
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
