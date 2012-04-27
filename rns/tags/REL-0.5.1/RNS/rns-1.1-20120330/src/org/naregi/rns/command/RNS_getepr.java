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

import java.util.List;

import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.util.RNSUtil;

/**
 * rns-getepr
 */
public class RNS_getepr {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("RNS_path");
		boolean usage = true;
		try {
			List<String> al = home.parseArgs(args, 1, 1);
			RNSClient rnsclient= home.getRNSClient();
			String path = al.get(0);
			usage = false;
			EndpointReferenceType epr = rnsclient.getEPR(path, true);
			System.out.print(RNSUtil.toXMLString(epr));
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
