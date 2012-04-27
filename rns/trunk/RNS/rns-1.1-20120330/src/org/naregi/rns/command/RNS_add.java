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

import java.io.File;
import java.util.List;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.util.RNSUtil;

/**
 * rns-add
 */
public class RNS_add {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("(e EPR_file | u URL | er RNS_EPR_file | ur RNS_URL) RNS_path [Metadata_file(XML)]");
		List<String> al;
		RNSClient rnsclient;
		boolean usage = true;
		try {
			al = home.parseArgs(args, 3, 4);
			rnsclient = home.getRNSClient();

			String opt = al.get(0);
			String val = al.get(1);
			String path = al.get(2);
			MessageElement[] mes = null;
			if (al.size() == 4) {
				String xmlFile = al.get(3);
				mes = RNSUtil.toMessageElements(new File(xmlFile));
			}
			EndpointReferenceType epr;
			boolean isRNS;
			if ("u".equals(opt)) {
				epr = RNSUtil.toEPR(new URI(val));
				isRNS = false;
			} else if ("e".equals(opt)) {
				epr = RNSUtil.toEPR(new File(val));
				isRNS = false;
			} else if ("ur".equals(opt)) {
				epr = RNSUtil.toRNSEPR(new URI(val));
				isRNS = true;
			} else if ("er".equals(opt)) {
				epr = RNSUtil.toEPR(new File(val));
				isRNS = true;
			} else {
				throw new Exception("invalid option: " + opt);
			}
			if ("".equals(val) || "".equals(path)) {
				throw new Exception("invalid argument");
			}
			usage = false;
			if (isRNS) {
				rnsclient.addRNSEPR(path, epr, mes);
			} else {
				rnsclient.addJunction(path, epr, mes);
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
