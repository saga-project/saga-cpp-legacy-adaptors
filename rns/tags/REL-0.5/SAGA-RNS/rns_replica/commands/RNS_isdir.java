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

//import java.io.PrintStream;
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

public class RNS_isdir {

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
	
	
	public static boolean RNS_ent_isdir(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		boolean usage = true;

//		System.out.println("SRA java code!! args[0]=" + args[0]);
		
		boolean result_isdir = false;
		
		try {
			String[] shortFlagNames = { "l" };
			if (args[0] == null)	args[0] = "/";
//			Map<String, String> map = home.parseArgsWithPlusOption(args,
//					shortFlagNames, null, null, null, "arg", 1);
			Map<String, String> map = home.parseArgsWithPlusOption(args,
					shortFlagNames, null, null, null, "arg", 1, 1);
			String path = map.get("arg0");
			RNSClient rnsclient = home.getRNSClient();
			String baseUrl = getHostAddressURL(home.getEPR());
			result_isdir = rnsclient.isDirectory(path);			
			
		} catch (Exception e) {
			System.out.println("rnsclient.isDirectory Error!");
			result_isdir = false;
		}
				
		return result_isdir;
	}
}
