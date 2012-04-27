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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.List;


import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.gsi.gssapi.auth.Authorization;
import org.naregi.rns.client.GlobusURLFixed;
import org.naregi.rns.client.GridFTPUtil;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.util.RNSUtil;

/**
 * rns-gridftp-put
 */
public class RNS_gridftp_put {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("GridFTP_URL(gsiftp://.../path/to/name) new_RNS_path [GridFTP_Authorization(self|host|hostSelf|IdString)]");
		boolean usage = true;
		try {
			List<String> al = home.parseArgs(args, 2, 3);
			if (al == null) {
				System.exit(1);
			}
			RNSClient rnsclient= home.getRNSClient();
			if (rnsclient == null) {
				System.exit(1);
			}
			String toUrl = al.get(0);
			String path = al.get(1);
			String ftpAuthz = null;
			if (al.size() == 3) {
				ftpAuthz = al.get(2);
			}
			usage = false;

			GlobusURLFixed gurl = new GlobusURLFixed(toUrl);
			InputStream is = System.in;
			Authorization authz;
			if (ftpAuthz != null) {
				authz = RNSUtil.getGSSAPIAuthorizationFromName(ftpAuthz);
			} else { // use same Authorization as RNS
				authz = RNSUtil.convertGSSAPIAuthorization(home.getAuthorization());
			}
			OutputStream os = GridFTPUtil.getOutputStream(authz, gurl,
					true, false);
			GridFTPUtil.streamCopy(is, os, 65536);
			os.close();
			try {
				is.close();
			} catch (IOException ioe) {
				home.printError(ioe, System.err);
			}
			EndpointReferenceType epr = new EndpointReferenceType();
			epr.setAddress(new Address(toUrl));
			rnsclient.addJunction(path, epr);
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
