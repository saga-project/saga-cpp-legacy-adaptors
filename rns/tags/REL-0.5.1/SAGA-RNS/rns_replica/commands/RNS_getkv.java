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

import java.util.Map;

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSKeyValue;
//import org.naregi.rns.client.XQueryResult;

public class RNS_getkv {
	
	public static String RNS_getval(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+r|++recursive depth] [+x|++xml] path key");
		boolean usage = true;
		try {
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					null, null, null, null, "arg", 2, 2);
			usage = false;
			String path = argMap.get("arg0");
			String key = argMap.get("arg1");

			RNSKeyValue kv = new RNSKeyValue(home, path);
			return kv.get(key);
			
		} catch (Exception e) {
			home.printError(e, System.err);
			if (usage) {
				home.printUsage(System.out);
			}
			System.exit(1);
			return null;
		}
	}
		
	
//	
//	public static void main(String[] args) {
//		RNSClientHome home = new RNSClientHome();
//		home.setCustomUsage("[+r|++recursive depth] [+x|++xml] path key");
//		boolean usage = true;
//		try {
//			String[] shortFlagNames = { "x" };
//			String[] longFlagNames = { "xml" };
//			String[] shortOptNames = { "r" };
//			String[] longOptNames = { "recursive" };
//			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
//					shortFlagNames, longFlagNames, shortOptNames, longOptNames,
//					"arg", 2, 2);
//			usage = false;
//			String path = argMap.get("arg0");
//			String key = argMap.get("arg1");
//
//			boolean isXML = false;
//			if (argMap.containsKey("++xml") || argMap.containsKey("+x")) {
//				isXML = true;
//			}
//			String depthStr = argMap.get("++recursive");
//			if (depthStr == null) {
//				depthStr = argMap.get("+r");
//			}
//			boolean isRecursive = false;
//			if (depthStr != null) {
//				isRecursive = true;
//			}
//			RNSKeyValue kv = new RNSKeyValue(home, path);
//			if (isRecursive && path.equals("/")) {
//				/* ignore Root directory */
//				if (isXML) {
//					System.out.println("<KeyValueResult>");
//				}
//			} else {
//				String value = kv.get(key);
//				if (isXML) {
//					System.out.println("<KeyValueResult>");
//				}
//				print(path, key, value, isRecursive, isXML);
//			}
//			if (depthStr != null) {
//				int depth = Integer.parseInt(depthStr);
//				XQueryResult[] results = kv.searchChildren(key, depth);
//				if (results != null) {
//					for (XQueryResult result : results) {
//						RNSError error = result.getError();
//						if (result.getResult() == null) {
//							continue;
//						}
//						if (error != null) {
//							home.printError(error, System.err);
//						} else {
//							print(result.getPath(), key, result.getResult(),
//									isRecursive, isXML);
//						}
//					}
//				}
//			}
//			if (isXML) {
//				System.out.println("</KeyValueResult>");
//			}
//		} catch (Exception e) {
//			home.printError(e, System.err);
//			if (usage) {
//				home.printUsage(System.out);
//			}
//			System.exit(1);
//			return;
//		}
//	}
//
//	private static void printEntryXML(String path, String key, String value) {
//		System.out.println("  <entry path=\"" + path + "\">");
//		System.out.println("    <key>" + key + "</key>");
//		System.out.println("    <value>" + value + "</value>");
//		System.out.println("  </entry>");
//	}
//
//	private static void print(String path, String key, String value,
//			boolean isRecursive, boolean isXML) {
//		if (isRecursive) {
//			if (value != null) {
//				if (isXML) {
//					printEntryXML(path, key, value);
//				} else {
//					System.out.println(path + ": " + value);
//				}
//			}
//			/* else: ignore */
//		} else {
//			if (value != null) {
//				if (isXML) {
//					printEntryXML(path, key, value);
//				} else {
//					System.out.println(value);
//				}
//			} else {
//				System.err.println("no such key: " + key);
//				System.exit(2);
//				return;
//			}
//		}
//	}
}
