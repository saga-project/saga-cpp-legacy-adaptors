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

import java.io.FileInputStream;
import java.io.InputStream;
import java.util.Map;

import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSSearchClient;
import org.naregi.rns.client.RNSSearchResult;
import org.naregi.rns.client.RNSSearchResultHandle;
import org.naregi.rns.util.RNSUtil;

/**
 * rns-xquery
 */
public class RNS_xquery {
	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+r|++recursive depth] [+x|++xml] [+a|++all] [+e|++epronly] path [xqueryFile]");

		boolean usage = true;
		try {
			String[] shortFlagNames = { "x", "a", "e" };
			String[] longFlagNames = { "xml", "all", "epronly" };
			String[] shortOptNames = { "r" };
			String[] longOptNames = { "recursive" };
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					shortFlagNames, longFlagNames, shortOptNames, longOptNames,
					"arg", 1, 2);

			usage = false;
			RNSSearchClient client = home.getRNSSearchClient();
			String path = argMap.get("arg0");
			String xqFile = argMap.get("arg1");
			boolean isXML = (argMap.containsKey("++xml") || argMap.containsKey("+x"));
			boolean printAll = (argMap.containsKey("++all") || argMap.containsKey("+a"));
			boolean eprOnly = (argMap.containsKey("++epronly") || argMap.containsKey("+e"));

			InputStream in;
			if (xqFile != null && xqFile.equals("-") == false) {
				in = new FileInputStream(xqFile);
			} else {
				in = System.in;
			}
			String inputStr = RNSUtil.inputStreamToString(in);
			String depthStr = argMap.get("++recursive");
			if (depthStr == null) {
				depthStr = argMap.get("+r");
			}
			boolean isRecursive = false;
			if (depthStr != null) {
				isRecursive = true;
			}
			if (isXML) {
				System.out.println("<XQueryResult>");
			}
			if (isRecursive && path.equals("/")) {
				/* ignore Root directory */
			} else {
				RNSSearchResult result = client.search(path, inputStr);
				if (result == null) {
					print(path, null, isRecursive, isXML);
				} else if (printAll) {
					print(result.getPath(),
							result.getEntryResponseTypeString(),
							isRecursive, isXML);
				} else if (eprOnly) {
					print(result.getPath(),
							result.getEPRString(),
							isRecursive, isXML);
				} else {
					print(result.getPath(),
							result.getMetadataString(),
							isRecursive, isXML);
				}
			}
			if (depthStr != null) {
				int depth = Integer.parseInt(depthStr);
				RNSSearchResultHandle results = client.searchRecursive(path, inputStr,
						depth);
				if (results != null) {
					for (RNSSearchResult result : results) {
						RNSError error = result.getError();
						if (error != null) {
							home.printError(error, System.err);
						} else {
							if (printAll) {
								print(result.getPath(),
										result.getEntryResponseTypeString(),
										isRecursive, isXML);
							} else if (eprOnly) {
								print(result.getPath(),
										result.getEPRString(),
										isRecursive, isXML);
							} else {
								print(result.getPath(),
										result.getMetadataString(),
										isRecursive, isXML);
							}
						}
					}
					RNSError e = results.getError();
					if (e != null) {
						throw e;
					}
				}
			}
			if (isXML) {
				System.out.println("</XQueryResult>");
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

	private static void printEntryXML(String path, String result) {
		System.out.println("  <entry path=\"" + path + "\">");
		System.out.println("    <result>");
		System.out.println(result);
		System.out.println("    </result>");
		System.out.println("  </entry>");
	}

	private static void print(String path, String result, boolean isRecursive,
			boolean isXML) {
		if (isRecursive) {
			if (result != null) {
				if (isXML) {
					printEntryXML(path, result);
				} else {
					System.out.println("Path: " + path);
					System.out.println(result);
				}
			}
			/* else: ignore */
		} else {
			if (result != null) {
				if (isXML) {
					printEntryXML(path, result);
				} else {
					System.out.println(result);
				}
			} else {
				System.err.println("no result output");
				System.exit(2);
				return;
			}
		}
	}
}
