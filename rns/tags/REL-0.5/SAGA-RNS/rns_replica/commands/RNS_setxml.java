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
import java.util.*;

import org.apache.axis.message.MessageElement;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
//import org.naregi.rns.RNSUtil;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClient;
//import org.naregi.rns.client.RNSClientUtil;

import javax.xml.stream.*;
import java.io.*;
import javax.xml.stream.events.*;


public class RNS_setxml {
	
	private static MessageElement[] metadata_attr(String key, String value) {
		String xml = "<file><attribute>";
		xml += "<" + key + ">" + value + "</" + key + ">";
		xml += "</attribute></file>";
		try {
//			return RNSClientUtil.convertStringToMessageElements(xml);
//			return RNSUtil.convertStringToMessageElements(xml);
			return RNSUtil.toMessageElements(xml);			
		} catch (Exception e) {
			e.printStackTrace(); // TODO
			return null;
		}
	}
	
	public static boolean RNS_attr_exists(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		
		boolean result_exists = false;

		try {
			List<String> al = home.parseArgs(args, 1, 2);
			RNSClient rnsclient = home.getRNSClient();
			String path = al.get(0);
			String attr = al.get(1);
			
//			MessageElement[] mes = rnsclient.getXML(path);
			MessageElement[] mes = rnsclient.getMetadata(path);
//			System.out.println("mes=" + mes);
			
			if(mes == null) return false;
			
			for (MessageElement me : mes) {
				String me_str = me.getAsString();
				InputStream in = new ByteArrayInputStream(me_str.getBytes());

				XMLInputFactory factory = XMLInputFactory.newInstance();
				XMLEventReader parser = factory.createXMLEventReader(in);
				
				// if both start and end elements exist, the attribute does exist.
				boolean start_chk = false;
				boolean end_chk = false;
				while (parser.hasNext()) {           
					XMLEvent event = parser.nextEvent();
			        if(event.isStartElement()){
			            StartElement start = event.asStartElement();
			            if(start.getName().toString() == "attribute"){
							while (parser.hasNext()) {           
								XMLEvent attr_event = parser.nextEvent();
						        if(attr_event.isStartElement()){
						            StartElement attr_start = attr_event.asStartElement();
							        if(attr.equals(attr_start.getName().toString())){
						            	start_chk = true;
//										System.out.println("start_chk=" + start_chk);
						            }
						        }
						        else if(attr_event.isEndElement()){
						            EndElement attr_end = attr_event.asEndElement();
						            if(attr.equals(attr_end.getName().toString())){
						            	end_chk = true;
//										System.out.println("end_chk=" + end_chk);
						            }
						        }
							}
			            }
			        }
				}

				
				if(start_chk & end_chk) {
					result_exists = true;
					break;
				}
				else{
					start_chk = false;
					end_chk = false;
				}
			}
					
		} catch (Exception e) {
			home.printError(e, System.err);
			result_exists = false;
		}
		
		return result_exists;
	}
	
	public static void RNS_set_attr(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l] RNS_path");
		
		try {
			List<String> al = home.parseArgs(args, 1, 3);
			RNSClient rnsclient = home.getRNSClient();
			String path = al.get(0);
			String attr = al.get(1);
			String value = al.get(2);
			
//			MessageElement[] old_mes = rnsclient.getXML(path);
			MessageElement[] old_mes = rnsclient.getMetadata(path);
			MessageElement[] new_mes = metadata_attr(attr, value);
			
			ArrayList<MessageElement> mes_list = new ArrayList<MessageElement>();

			if(old_mes != null) mes_list.addAll(Arrays.asList(old_mes));
			if(new_mes != null) mes_list.addAll(Arrays.asList(new_mes));
			
			MessageElement[] mes = (MessageElement[])mes_list.toArray(new MessageElement[0]);

//			rnsclient.setXML(path, mes);
			rnsclient.setMetadata(path, mes);
						
		} catch (Exception e) {
			home.printError(e, System.err);
		}
	}

}
