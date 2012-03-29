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

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.Map;

import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;

/**
 * rns-bulk-rename
 */
public class RNS_bulk_rename {

	private static class SplitString {
		String first = null;
		String second = null;
	}

	private static char[] skipSpace(char[] cs) {
		int i = 0;
		while (i < cs.length) {
			if (cs[i] != ' ' && cs[i] != '\t') {
				break;
			}
			i++;
		}
		char[] dst = new char[cs.length - i];
		System.arraycopy(cs, i, dst, 0, dst.length);
		return dst;
	}

	private static SplitString split(String str) throws RNSError {
		SplitString ss = new SplitString();
		int start;
		int end;

		boolean haveDQ = false; /* double quotation */
		char[] cs = str.toCharArray();
		cs = skipSpace(cs);
		if (cs.length == 0) {
			return null; /* skip */
		}
		int i = 0;
		if (cs[0] == '"') {
			i++;
			start = i;
			haveDQ = true;
		} else {
			start = 0;
		}
		while (i < cs.length) {
			if (cs[i] == '"') {
				if (haveDQ == true) {
					end = i - 1;
					if (start > end) {
						throw RNSError.createEINVAL(null, "old name is null",
								null);
					}
					ss.first = new String(cs, start, end - start + 1);
					haveDQ = false;
					i++;
					break;
				} else {
					throw RNSError.createEINVAL(null,
							"not found first of double quotation", null);
				}
			} else if (haveDQ == false && cs[i] == ' ') {
				end = i - 1;
				if (start > end) {
					throw RNSError.createEINVAL(null, "old name is null", null);
				}
				ss.first = new String(cs, start, end - start + 1);
				i++;
				break;
			}
			i++;
		}
		if (haveDQ == true) {
			RNSError.createEINVAL(null, "not found end of double quotation",
					null);
		}
		if (ss.first == null) {
			throw RNSError.createEINVAL(null, "no old name", null);
		}
		if (i == cs.length) {
			throw RNSError.createEINVAL(null, "no new name", null);
		}

		char[] cs2 = new char[cs.length - i];
		System.arraycopy(cs, i, cs2, 0, cs2.length);
		cs = skipSpace(cs2);

		i = 0;
		if (cs[0] == '"') {
			i++;
			start = i;
			haveDQ = true;
		} else {
			start = 0;
		}
		while (i < cs.length) {
			if (cs[i] == '"') {
				if (haveDQ == true) {
					end = i - 1;
					if (start > end) {
						throw RNSError.createEINVAL(null, "new name is null",
								null);
					}
					ss.second = new String(cs, start, end - start + 1);
					haveDQ = false;
					break;
				} else {
					throw RNSError.createEINVAL(null,
							"not found first of double quotation", null);
				}
			} else if (haveDQ == false && (cs[i] == ' ' || i == cs.length - 1)) {
				if (i == cs.length - 1) {
					end = i;
				} else {
					end = i - 1;
				}
				if (start > end) {
					throw RNSError.createEINVAL(null, "new name is null", null);
				}
				ss.second = new String(cs, start, end - start + 1);
				break;
			}
			i++;
		}
		if (haveDQ == true) {
			throw RNSError.createEINVAL(null,
					"not found end of double quotation", null);
		}
		if (ss.second == null) {
			throw RNSError.createEINVAL(null, "no new name", null);
		}

		return ss;
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[option] RNS_dir inputFile(\"old_name\" \"new_name\" per line)");
		home.addHelp("+i,++ignore", "Ignore ENOENT/EEXIST error");

		RNSClient rnsclient;
		boolean usage = true;
		try {
			String[] shortFlags = { "i" };
			String[] longFlags = { "ignore" };
			String[] shortKeys = null;
			String[] longKeyss = null;
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					shortFlags, longFlags, shortKeys, longKeyss, "arg", 2, 2);

			String dir = argMap.get("arg0");
			String inputFile = argMap.get("arg1");

			rnsclient = home.getRNSClient();
			usage = false;

			boolean ignore = (argMap.containsKey("+i") || argMap.containsKey("++ignore"));

			BufferedReader br;
			if ("-".equals(inputFile)) {
				br = new BufferedReader(new InputStreamReader(System.in));
			} else {
				br = new BufferedReader(new FileReader(inputFile));
			}

			HashMap<String, String> map = new HashMap<String, String>();
			String line;
			while ((line = br.readLine()) != null) {
				SplitString ss = split(line);
				if (ss != null) {
					map.put(ss.first, ss.second);
				}
			}
			br.close();

			RNSError[] errors = rnsclient.renameBulk(dir, map);
			if (errors != null && errors.length > 0) {
				boolean hasError = false;
				for (RNSError error : errors) {
					if (error.getError().equals(RNSError.Errno.ENOENT)
							|| error.getError().equals(RNSError.Errno.EEXIST)) {
						if (ignore == false) {
							home.printError(error, System.err);
							hasError = true;
						}
					} else {
						home.printError(error, System.err);
						hasError = true;
					}
				}
				if (hasError) {
					System.exit(1);
				}
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
