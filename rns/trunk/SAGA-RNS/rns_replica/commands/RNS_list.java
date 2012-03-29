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

import java.io.PrintStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.Map.Entry;
import java.util.Vector;

import org.apache.axis.message.MessageElement;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.naregi.rns.ACL;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSDirentComparator;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSStat;

public class RNS_list {

	private static final String LS = System.getProperty("line.separator");

	public static void rnsList(RNSClientHome home, String path,
			String lineSeparator, boolean longMode, Vector<String> ent_list)
			throws Exception {
		RNSClient rnsclient = home.getRNSClient();
		String baseUrl = getHostAddressURL(home.getEPR());
				
		if (path == null) {
			path = "/";
		}
		try {
			if (rnsclient.isDirectory(path) == false) {
				EndpointReferenceType epr = rnsclient.getEPR(path, false);
				return;
			}

			RNSDirHandle dir;
			if (longMode) {
				dir = rnsclient.list(path, true);
			} else {
				dir = rnsclient.list(path, false);
			}
			TreeSet<RNSDirent> ts = null;
			if (dir != null) {
//				ts = new TreeSet<RNSDirent>(new RNSDirentComparator(
//						RNSDirentComparator.MODES.NAME, false));
				ts = new TreeSet<RNSDirent>(new RNSDirentComparator(
						RNSDirentComparator.MODE.NAME, false));
//				while (dir.hasNext()) {
//					ts.add(dir.next());
//				}
				for (RNSDirent ent : dir) {
					ts.add(ent);
				}
			}

			if (ts == null) return;
			for (RNSDirent ent : ts) {
				ent_list.add(ent.getName());
			}
		} catch (RNSError e) {
			throw e;
		}
		return;
	}

	public static int rnsGetNumEntry(RNSClientHome home, String path,
			String lineSeparator, boolean longMode)
			throws Exception {
		RNSClient rnsclient = home.getRNSClient();
		String baseUrl = getHostAddressURL(home.getEPR());
		int ent_num = 0;
		
		if (path == null) {
			path = "/";
		}
		try {
			if (rnsclient.isDirectory(path) == false) {
				EndpointReferenceType epr = rnsclient.getEPR(path, false);
				return 0;
			}

			RNSDirHandle dir;
			dir = rnsclient.list(path, false);
			if (dir != null) {
//				while (dir.hasNext()) {
//					ent_num++;
//					dir.next();
//				}
				for (RNSDirent ent : dir) {
					ent_num++;
				}
			}
		} catch (RNSError e) {
			throw e;
		}
		
		return ent_num;
	}
	
	private static String getHostAddressURL(EndpointReferenceType epr) {
		AttributedURIType addr = epr.getAddress();
		String hostname;
		try {
			hostname = InetAddress.getByName(addr.getHost()).getCanonicalHostName();
		} catch (UnknownHostException e) {
			hostname = addr.getHost();
		}
		String url = addr.getScheme() + "://" + hostname;
		if (addr.getPort() > 0) {
			url += ":" + addr.getPort();
		}
		url += addr.getPath();
		return url;
	}

	
	public static int RNS_ent_getnum(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		boolean usage = true;

//		System.out.println("SRA java code RNS_ent_getnum!!");
		int ent_num = 0;
		
		try {
			String[] shortFlagNames = { "l" };
//			Map<String, String> map = home.parseArgsWithPlusOption(args,
//					shortFlagNames, null, null, null, "arg", 1);
			Map<String, String> map = home.parseArgsWithPlusOption(args,
					shortFlagNames, null, null, null, "arg", 1, 1);
			String path = map.get("arg0");
			usage = false;
			
			ent_num = rnsGetNumEntry(home, path, LS, map.containsKey("+l"));
			
		} catch (Exception e) {
			System.out.println("rnsGetNumEntry Error!");
			System.exit(1);			
		}
		
		return ent_num;
	}
	
	
	public static String[] RNS_ent_list(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		boolean usage = true;

//		System.out.println("SRA java code RNS_ent_list!!");
		Vector<String> ent_list = new Vector<String>(); 
		
		try {
			String[] shortFlagNames = { "l" };
//			Map<String, String> map = home.parseArgsWithPlusOption(args,
//					shortFlagNames, null, null, null, "arg", 1);
			Map<String, String> map = home.parseArgsWithPlusOption(args,
					shortFlagNames, null, null, null, "arg", 1, 1);
			String path = map.get("arg0");
			usage = false;
			
			rnsList(home, path, LS, map.containsKey("+l"), ent_list);
			
		} catch (Exception e) {
			System.out.println("rnsList Error!");
			System.exit(1);			
		}
		
		String[] result_list = new String[ent_list.size()];
		ent_list.copyInto(result_list);
		
		return result_list;
	}

	
}
