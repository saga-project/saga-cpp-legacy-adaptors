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
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSStat;

/**
 * rns-stat
 */
public class RNS_stat {
	public static void main(String[] args) {
		RNSClientHome base = new RNSClientHome();
		base.setCustomUsage("RNS_path");
		boolean usage = true;
		try {
			List<String> al = base.parseArgs(args, 1, 1);
			RNSClient rnsclient= base.getRNSClient();
			String path = al.get(0);
			usage = false;

			try {
				RNSStat st = rnsclient.stat(path);
				System.out.println("Directory: " + path);
				System.out.println("Readable: " + st.isReadable());
				System.out.println("Writable: " + st.isWritable());
				System.out.println("ElementCount: " + st.getElementCount());
				System.out.println("Create: " + st.getCreateTime().getTime().toString());
				System.out.println("Access: " + st.getAccessTime().getTime().toString());
				System.out.println("Change: " + st.getModificationTime().getTime().toString());
			} catch (RNSError rnsErr) {
				if (rnsErr.getError().equals(RNSError.Errno.ENOTDIR)) {
					System.out.println("Junction: " + path);
				} else {
					throw rnsErr;
				}
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
