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

import java.io.FileReader;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.globus.wsrf.encoding.ObjectDeserializer;
import org.naregi.rns.client.RNSAddHandle;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.stubs.RNSEntryType;
import org.naregi.rns.stubs.SupportsRNSType;
import org.xml.sax.InputSource;

/**
 * rns-bulk-add
 */
public class RNS_bulk_add {

	public static void checkValid(RNSDirent rd, String xmlStr) throws RNSError {
		if (rd.getName() == null) {
			throw RNSError.createEINVAL(null,
					"'entry-name' must be specified: " + xmlStr, null);
		}
		if (rd.getMeta() == null) {
			throw RNSError.createEINVAL(null,
					"<metadata> must be specified: " + xmlStr, null);
		}
		SupportsRNSType srt = rd.getMeta().getSupportsRns();
		if (srt == null || srt.getValue() == null
				|| srt.getValue().getValue() == null) {
			throw RNSError.createEINVAL(null,
					"<supports-rns> must be specified: " + xmlStr, null);
		}
		if (rd.getEpr() == null && rd.isDirectory() == false) {
			throw RNSError.createEINVAL(null,
					"<endpoint> is necessary to create a junction: "
							+ xmlStr, null);
		}
		if (rd.getEpr() != null && rd.isDirectory() == true) {
			System.err.println("NOTE: <endpoint> is not used to create a directory: "
					+ rd.getName());
		}
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[option] RNS_dir inputFile(rns-ls +x output format)");
		home.addHelp("+i,++ignore", "Ignore EEXIST error");

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

			Reader reader;
			if ("-".equals(inputFile)) {
				reader = new InputStreamReader(System.in);
			} else {
				reader = new FileReader(inputFile);
			}
			StringBuilder sb = new StringBuilder();
			int ch;
			while ((ch = reader.read()) != -1) {
				sb.append((char) ch);
			}
			reader.close();

			String xml = sb.toString();
			xml = xml.replaceAll(":RNSEntryResponseType", ":RNSEntryType");

			Pattern pat = Pattern.compile(
					"<.+?:RNSEntryType .*?</.+?:RNSEntryType>",
					Pattern.MULTILINE | Pattern.DOTALL);
			Matcher mat = pat.matcher(xml);
			List<RNSDirent> al = new ArrayList<RNSDirent>();
			while (mat.find()) {
				String str = mat.group();
				RNSEntryType ret = (RNSEntryType) ObjectDeserializer.deserialize(
						new InputSource(new StringReader(str)),
						RNSEntryType.class);
				RNSDirent rd = new RNSDirent();
				rd.setName(ret.getEntryName());
				rd.setEpr(ret.getEndpoint());
				rd.setMeta(ret.getMetadata());
				checkValid(rd, str);
				al.add(rd);
			}
			RNSAddHandle rah = new RNSAddHandle();
			for (RNSDirent de : al) {
				if (de.isDirectory()) {
					rah.registerMkdir(de.getName(), de.getMeta().get_any());
				} else {
					rah.registerAddEPR(de.getName(), de.getEpr(), de.getMeta()
							.get_any(), false);
				}
			}
			RNSError[] errors = rnsclient.addBulk(dir, rah);
			if (errors != null && errors.length > 0) {
				boolean hasError = false;
				for (RNSError error : errors) {
					if (error.getError().equals(RNSError.Errno.EEXIST)) {
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
