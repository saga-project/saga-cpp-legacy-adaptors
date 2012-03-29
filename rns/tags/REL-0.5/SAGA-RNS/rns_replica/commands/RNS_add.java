/*
 * Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
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


//package org.naregi.rns.client.command;

import java.io.File;
import java.util.List;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.Map.Entry;
import java.util.Vector;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;

import javax.xml.stream.*;
import java.io.*;
import javax.xml.stream.events.*;



public class RNS_add {
	
	private static MessageElement[] metadata(ArrayList<String> repUrls) {
		String xml = "<file><replica>";
		int no = 0;
		for (String suburl : repUrls) {
			xml += "<url no=\"" + no + "\">" + suburl + "</url>";
			no++;
		}
		xml += "</replica></file>";
		try {
//			return RNSClientUtil.convertStringToMessageElements(xml);
//			return RNSUtil.convertStringToMessageElements(xml);
			return RNSUtil.toMessageElements(xml);
		} catch (Exception e) {
			e.printStackTrace(); // TODO
//			warning("lfcPath=" + lfcPath + ": don't convert Metadata");
			return null;
		}
	}
	
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
//				mes = RNSClientUtil.convertFileToMessageElements(new File(xmlFile));
//				mes = RNSUtil.convertFileToMessageElements(new File(xmlFile));
				mes =  RNSUtil.toMessageElements(new File(xmlFile));
			}
			EndpointReferenceType epr;
			boolean isRNS;
			if ("u".equals(opt)) {
//				epr = new EndpointReferenceType();
//				epr.setAddress(new Address(val));
				epr = RNSUtil.toEPR(new URI(val));
				isRNS = false;
			} else if ("e".equals(opt)) {
//				epr = RNSUtil.getEPRFromFileName(val);
				epr = RNSUtil.toEPR(new File(val));
				isRNS = false;
			} else if ("ur".equals(opt)) {
//				epr = RNSUtil.getRNSEPRFromURL(val);
				epr = RNSUtil.toRNSEPR(new URI(val));
				isRNS = true;
			} else if ("er".equals(opt)) {
//				epr = RNSUtil.getEPRFromFileName(val);
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
//				rnsclient.addEPR(path, epr, mes);
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
	
	public static boolean RNS_ent_add_epr(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		RNSClient rnsclient;
	
		boolean result_add_epr = false;
		
		try {
			String[] shortFlagNames = { "l" };
//			Map<String, String> map = home.parseArgsWithPlusOption(args,
//					shortFlagNames, null, null, null, "arg", 2);
			Map<String, String> map = home.parseArgsWithPlusOption(args,
					shortFlagNames, null, null, null, "arg", 2, 2);
			String path = map.get("arg0");
			String val  = map.get("arg1");
			
			rnsclient = home.getRNSClient();
			MessageElement[] mes = null;
	
			EndpointReferenceType epr;
			epr = new EndpointReferenceType();
			epr.setAddress(new Address(val));
	
//			rnsclient.addEPR(path, epr, mes);
			rnsclient.addJunction(path, epr, mes);
			
			result_add_epr = true;
			
		} catch (Exception e) {
			System.out.println("rnsclient.addEPR Error!");
			result_add_epr = false;
		}
		
		return result_add_epr;
	}

	
	public static boolean RNS_ent_add_url(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		
		boolean result_add_url = false;

		try {
			List<String> al = home.parseArgs(args, 1, 2);
			RNSClient rnsclient = home.getRNSClient();
			String path = al.get(0);
			String uxml = al.get(1);
			
			ArrayList<String> repUrls = new ArrayList<String>();
//			MessageElement[] mes = rnsclient.getXML(path);
			MessageElement[] mes = rnsclient.getMetadata(path);
				
			for (MessageElement me : mes) {
				String me_str = me.getAsString();
				InputStream in = new ByteArrayInputStream(me_str.getBytes());

				XMLInputFactory factory = XMLInputFactory.newInstance();
				XMLEventReader parser = factory.createXMLEventReader(in);
				
				// read & write XML
				// read url lines, add the new url	
				while (parser.hasNext()) {           
					XMLEvent event = parser.nextEvent();
			        if(event.isStartElement()){
			            StartElement start = event.asStartElement();
			            if(start.getName().toString() == "replica"){
							while (parser.hasNext()) {           
								XMLEvent rep_event = parser.nextEvent();
						        if(rep_event.isStartElement()){
						            StartElement rep_start = rep_event.asStartElement();
						            if(rep_start.getName().toString() == "url"){
						            	XMLEvent url_event = parser.nextEvent();
					                    repUrls.add(url_event.asCharacters().getData());
						            }
						        }
							}
			            }
			        }
				}

				repUrls.add(uxml);
				mes = metadata(repUrls);
//				rnsclient.setXML(path, mes);
				rnsclient.setMetadata(path, mes);
			}
			
			
			result_add_url = true;
			
		} catch (Exception e) {
			home.printError(e, System.err);
			result_add_url = false;
		}
		
		return result_add_url;
	}
	
	

//	public static void RNS_ent_open(String[] args) {
//		RNSClientHome home = new RNSClientHome();
//		home.setCustomUsage("[+l] RNS_path");
//		RNSClient rnsclient;
//		try {
//			String[] shortFlagNames = { "l" };
//			Map<String, String> map = home.parseArgsWithPlusOption(args,
//					shortFlagNames, null, null, null, "arg", 1);
//			String path = map.get("arg0");
//			
//			rnsclient = home.getRNSClient();
//
//			String val = "gsiftp://";
//			MessageElement[] mes = null;
//
//			EndpointReferenceType epr;
//			epr = new EndpointReferenceType();
//			epr.setAddress(new Address(val));
//
//			rnsclient.addEPR(path, epr, mes);
//			
//		} catch (Exception e) {
//			home.printError(e, System.err);
//			System.exit(1);
//			return;
//		}
//	}
}
