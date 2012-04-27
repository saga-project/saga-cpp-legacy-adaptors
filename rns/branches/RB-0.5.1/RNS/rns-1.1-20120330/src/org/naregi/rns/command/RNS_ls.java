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

import java.io.PrintStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.TreeSet;

import javax.xml.transform.TransformerException;

import org.apache.axis.message.MessageElement;
import org.globus.axis.message.addressing.AttributedURIType;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.naregi.rns.ACL;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSDirentComparator;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSExtensionClient;
import org.naregi.rns.client.RNSSearchClient;
import org.naregi.rns.client.RNSStat;
import org.naregi.rns.stubs.RNSEntryType;
import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.stubs.RNSSupportType;
import org.naregi.rns.stubs.SupportsRNSType;
import org.naregi.rns.util.RNSUtil;

/**
 * rns-ls
 */
public class RNS_ls {

	private static final String LS = System.getProperty("line.separator");

	private static final char table[] = { '0', '1', '2', '3', '4', '5', '6',
			'7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
			'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
			'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
			'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
			'X', 'Y', 'Z', '!', '@', '#', '$', '%', '&', '*', '-', '=', '+',
			'\\', '/', '?' };

	private static String hashString(String str) {
		return toStringForHashCode(str.hashCode(), 62);
	}

	private static String toStringForHashCode(int i, int radix) {
		if (radix < 2 || radix > table.length) {
			radix = 10;
		}
		long l = ((long) i) + ((long) Integer.MAX_VALUE) + 1;

		char ac[] = new char[32];
		int k = ac.length - 1;
		for (; l >= radix; l /= radix)
			ac[k--] = table[(int) (l % radix)];
		ac[k] = table[(int) l];
		return new String(ac, k, ac.length - k);
	}

	private static void printJunctionEpr(PrintStream ps, String name,
			EndpointReferenceType epr) {
		ps.print(name + " -> " + epr.getAddress().getValue().toString());
	}

	private static void printLine(PrintStream ps, String modeAll,
			int childCount, String owner, String ownerGroup, String modTime,
			String name) {
		if (modeAll != null && owner != null && ownerGroup != null) {
			ps.printf("%11s %5d %9s %9s %16s %s", modeAll, childCount, owner,
					ownerGroup, modTime, name);
		} else {
			ps.printf("%5d %16s %s", childCount, modTime, name);
		}
	}

	private static String checkAndPutNameMap(HashMap<String, String> nameMap,
			String name) {
		if (name.length() <= 9) {
			nameMap.put(name, name);
			return name;
		}
		String hashKey = hashString(name);
		String val = nameMap.get(hashKey);
		if (val != null) {
			if (val.equals(name)) {
				return hashKey;
			} else {
				nameMap.put(name, name);
				return name;
			}
		} else {
			nameMap.put(hashKey, name);
			return hashKey;
		}
	}

	private static ACL getACL(boolean enable, RNSClientHome home,
			RNSExtensionClient extclient, String path) {
		if (enable) {
			try {
				return extclient.getACL(path, true);
			} catch (RNSError e) {
				if (home.isDebugMode()) {
					e.printStackTrace();
				}
				System.err.println("Error: " + path + ": " + e.toString());
			}
		}
		ACL acl = new ACL();
		acl.autoComplete();
		return acl;
	}

	private static String printDirLine(PrintStream ps, String name,
			HashMap<String, String> nameMap, RNSStat stat, SimpleDateFormat df,
			boolean enableACL, RNSClientHome home,
			RNSExtensionClient extclient, String path) {
		int childCount = 0;
		String modTime = df.format(new Date(0));
		if (stat != null) {
			childCount = stat.getElementCount().intValue();
			modTime = df.format(stat.getModificationTime().getTime());
		}

		String modeStr = null;
		if (enableACL) {
			ACL acl = getACL(enableACL, home, extclient, path);
			modeStr = ACL.permToString(acl.getOwnerPerm())
					+ ACL.permToString(acl.getOwnerGroupPerm())
					+ ACL.permToString(acl.getOtherPerm());
			String ext;
			if (acl.hasExtension()) {
				ext = "+";
			} else {
				ext = " ";
			}
			String modeAll = "d" + modeStr + ext;
			String owner = checkAndPutNameMap(nameMap, acl.getOwner());
			String ownerGroup = checkAndPutNameMap(nameMap, acl.getOwnerGroup());
			printLine(ps, modeAll, childCount, owner, ownerGroup, modTime, name
					+ "/");
		} else {
			printLine(ps, null, childCount, null, null, modTime, name + "/");
		}

		return modeStr;
	}

	private static void longList(RNSClientHome home, RNSClient rnsclient,
			String path, Iterable<RNSDirent> itr, String lineSeparator,
			PrintStream ps, boolean optACL) throws RNSError {
		RNSExtensionClient extclient = home.getRNSExtensionClient();
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd hh:mm");
		String defaultModTime = df.format(new Date(0));
		HashMap<String, String> nameMap = new HashMap<String, String>();

		/* print "." */
		RNSStat dotst = rnsclient.stat(path);
		String parentModeStr = printDirLine(ps, ".", nameMap, dotst, df,
				optACL, home, extclient, path);
		ps.print(lineSeparator);
		/* print ".." */
		String dotdot = RNSUtil.getDirname(path);
		RNSStat dotdotst = rnsclient.stat(dotdot);
		printDirLine(ps, "..", nameMap, dotdotst, df, optACL, home, extclient,
				dotdot);
		ps.print(lineSeparator);

		if (itr == null) {
			return;
		}
		for (RNSDirent ent : itr) {
			if (ent == null) {
				continue;
			}
			String name = ent.getName();

			if (ent.getRNSError() != null) {
				if (!ent.getRNSError().getError().equals(RNSError.Errno.ENOENT)) {
					ent.getRNSError().printStackTrace();
				}
			} else if (ent.isDirectory()) {
				String childPath = path + "/" + name;
				printDirLine(ps, name, nameMap, ent.getStat(), df, optACL,
						home, extclient, childPath);
			} else {
				String mode;
				if (optACL) {
					mode = "-" + parentModeStr + " ";
				} else {
					mode = null;
				}
				int childCount = 0;
				String owner;
				String ownerGroup;
				String modTime = defaultModTime;
				owner = ownerGroup = "Junction";
				printLine(ps, mode, childCount, owner, ownerGroup, modTime,
						name);
			}
			ps.print(lineSeparator);
		}
		Set<Entry<String, String>> es = nameMap.entrySet();
		for (Entry<String, String> e : es) {
			ps.printf("%6s = %s", e.getKey(), e.getValue());
			ps.print(lineSeparator);
		}
	}

	private static String getHostAddressURL(EndpointReferenceType epr) {
		AttributedURIType addr = epr.getAddress();
		String hostname;
		try {
			hostname = InetAddress.getByName(addr.getHost())
					.getCanonicalHostName();
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

	private static void urlList(RNSClientHome home, RNSClient rnsclient,
			String path, Iterable<RNSDirent> itr, String lineSeparator,
			PrintStream ps) throws RNSError {
		if (itr == null) {
			return;
		}
		String baseUrl = getHostAddressURL(home.getEPR());
		for (RNSDirent ent : itr) {
			if (ent == null) {
				continue;
			}
			if (ent.getRNSError() != null) {
				if (!ent.getRNSError().getError().equals(RNSError.Errno.ENOENT)) {
					ent.getRNSError().printStackTrace();
				}
			} else if (ent.isDirectory()) {
				ps.print(ent.getName() + "/");
				EndpointReferenceType epr = ent.getEpr();
				String rawUrl = epr.getAddress().toString();
				ReferenceParametersType rpt = epr.getParameters();
				if (rpt != null) {
					MessageElement elm = rpt.get(RNSQNames.RESOURCE_ID);
					if (elm != null) {
						/* support my RNS implement */
						String id = elm.getValue();
						String url = getHostAddressURL(epr);
						if (baseUrl.equals(url)) {
							ps.print(" [id=" + id + "]");
						} else {
							ps.print(" [id=" + id + "] " + rawUrl);
						}
					} else {
						/* other RNS implement */
						MessageElement[] mes = epr.get_any();
						if (mes != null && mes[0] != null) {
							String id = mes[0].getValue();
							ps.print(" [(unknown RNS)id(?)=" + id + "] "
									+ rawUrl);
						} else {
							ps.print(" [(RNS?)] " + rawUrl);
						}
					}
				} else {
					ps.print(" [(RNS?)] " + rawUrl);
				}
			} else {
				printJunctionEpr(ps, ent.getName(), ent.getEpr());
			}
			ps.print(lineSeparator);
		}
	}

	private static void xmlList(RNSClientHome home, RNSClient rnsclient,
			String path, Iterable<RNSDirent> itr, String lineSeparator,
			PrintStream ps) throws RNSError, TransformerException,
			SerializationException {
		if (itr == null) {
			return;
		}
		for (RNSDirent ent : itr) {
			if (ent == null) {
				continue;
			} else if (ent.getRNSError() != null) {
				if (!ent.getRNSError().getError().equals(RNSError.Errno.ENOENT)) {
					ent.getRNSError().printStackTrace();
				}
				continue;
			}

			RNSEntryType rert = new RNSEntryType();
			rert.setEntryName(ent.getName());
			rert.setEndpoint(ent.getEpr());
			rert.setMetadata(ent.getMeta());
			try {
				System.out.print(RNSUtil.toIndentedXML(ObjectSerializer
						.toElement(rert, RNSQNames.TYPE_ENTRY_TYPE)));
			} catch (TransformerException e) {
				throw e;
			} catch (SerializationException e) {
				throw e;
			}
		}
	}

	public static void printList(RNSClientHome home, String path,
			String lineSeparator, PrintStream ps, boolean optLong,
			boolean optURL, boolean optSort, boolean optACL, boolean optXML,
			boolean optBySearch, boolean opt1) throws Exception {
		RNSClient rnsclient = home.getRNSClient();
		if (path == null) {
			path = "/";
		}
		if (rnsclient.isDirectory(path) == false) {
			EndpointReferenceType epr = rnsclient.getEPR(path, false);
			if (optXML) {
				RNSEntryType rert = new RNSEntryType();
				rert.setEntryName(RNSUtil.getBasename(path));
				rert.setEndpoint(epr);
				RNSMetadataType meta = new RNSMetadataType();
				SupportsRNSType st = new SupportsRNSType();
				st.setValue(RNSSupportType.value2); /* junction */
				meta.setSupportsRns(st);
				/* nillable="true" */
				meta.set_any(rnsclient.getMetadata(path));
				rert.setMetadata(meta);
				try {
					System.out.print(RNSUtil.toIndentedXML(ObjectSerializer
							.toElement(rert, RNSQNames.TYPE_ENTRY_TYPE)));
				} catch (TransformerException e) {
					throw e;
				} catch (SerializationException e) {
					throw e;
				}
			} else {
				printJunctionEpr(ps, path, epr);
				ps.print(lineSeparator);
			}
			return;
		}

		RNSDirHandle dir;
		if (optLong) {
			dir = rnsclient.list(path, true); /* with stat() */
		} else if (optBySearch) {
			RNSSearchClient searchClient = home.getRNSSearchClient();
			dir = searchClient.listBySearch(path);
		} else {
			dir = rnsclient.list(path, false);
		}

		Iterable<RNSDirent> itr = null;
		if (dir != null) {
			if (optSort) {
				TreeSet<RNSDirent> ts = null;
				ts = new TreeSet<RNSDirent>(new RNSDirentComparator(
						RNSDirentComparator.MODE.NAME, false));
				for (RNSDirent ent : dir) {
					ts.add(ent);
				}
				itr = ts;
			} else {
				itr = dir;
			}
		}
		/* itr may be null */

		if (optXML) {
			xmlList(home, rnsclient, path, itr, lineSeparator, ps);
		} else if (optLong) {
			longList(home, rnsclient, path, itr, lineSeparator, ps, optACL);
		} else if (optURL) {
			urlList(home, rnsclient, path, itr, lineSeparator, ps);
		} else if (opt1) {
			if (itr != null) {
				for (RNSDirent ent : itr) {
					if (ent.getRNSError() != null) {
						if (!ent.getRNSError().getError()
								.equals(RNSError.Errno.ENOENT)) {
							ent.getRNSError().printStackTrace();
						}
					} else {
						ps.print(ent.getName() + lineSeparator);
					}
				}
			}
		} else {
			if (itr != null) {
				String tmp = "";
				for (RNSDirent ent : itr) {
					String name;
					if (ent.getRNSError() != null) {
						if (!ent.getRNSError().getError()
								.equals(RNSError.Errno.ENOENT)) {
							ent.getRNSError().printStackTrace();
						}
						continue;
					} else if (ent.isDirectory()) {
						name = ent.getName() + "/";
					} else {
						name = ent.getName();
					}
					if (tmp.length() + name.length() >= 80) {
						ps.print(tmp + lineSeparator);
						tmp = "";
					}
					tmp += name + " ";
				}
				if (tmp.length() > 0) {
					ps.print(tmp + lineSeparator);
				}
			}
		}
		if (dir != null) {
			RNSError e = dir.getError();
			if (e != null) {
				throw e;
			}
		}
	}

	public static void main(String[] args) {
		main(args, System.out, System.err);
	}

	public static void main(String[] args, PrintStream stdout,
			PrintStream stderr) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+l|++long] [+u|++url] [+s|++sort] [+a|++acl] [+x|++xml] [+1] RNS_path");
		home.setDescription("List information about the RNS entries.");
		home.addHelp("+l,++long", "Print long format");
		home.addHelp("+u,++url", "Print URL (junction) or ID (directory)");
		home.addHelp("+s,++sort", "Sort list (slow)");
		home.addHelp("+a,++acl", "Print acl (imply +l option)");
		home.addHelp("+x,++xml", "Print XML format");
		home.addHelp("+1", "List one entry per line");

		boolean usage = true;
		try {
			String[] shortFlagNames = { "l", "u", "s", "a", "x", "1" };
			String[] longFlagNames = { "long", "url", "sort", "acl", "xml",
					"bysearch" };
			Map<String, String> map = home.parseArgsWithPlusOption(args,
					shortFlagNames, longFlagNames, null, null, "arg", 0, 1);
			String path = map.get("arg0");
			usage = false;

			boolean optLong = (map.containsKey("+l") || map
					.containsKey("++long"));
			boolean optURL = (map.containsKey("+u") || map.containsKey("++url"));
			boolean optSort = (map.containsKey("+s") || map
					.containsKey("++sort"));
			boolean optACL = (map.containsKey("+a") || map.containsKey("++acl"));
			if (optACL) {
				optLong = true;
			}
			boolean optXML = (map.containsKey("+x") || map.containsKey("++xml"));
			boolean optBySearch = map.containsKey("++bysearch");
			boolean opt1 = (map.containsKey("+1"));

			printList(home, path, LS, stdout, optLong, optURL, optSort, optACL,
					optXML, optBySearch, opt1);
		} catch (Exception e) {
			home.printError(e, stderr);
			if (usage) {
				home.printUsage(stdout);
			}
			System.exit(1);
			return;
		}
	}
}
