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

import java.net.URI;
import java.util.ArrayList;
import java.util.Date;
import java.util.Map;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI.MalformedURIException;
import org.glite.lfc.LFCException;
import org.glite.lfc.LFCServer;
import org.glite.lfc.internal.FileDesc;
import org.glite.lfc.internal.ReplicaDesc;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSAddHandle;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.util.RNSUtil;

/**
 * lfcj-rns-migrate
 */
public class LFC_RNS_migrate {

	private static void warning(String msg) {
		System.err.println("LFC_RNS_migrate [WARN] " + msg);
	}

	private static MessageElement[] myMetadata(RNSClientHome home,
			long fileSize, Date mtime, ArrayList<String> repUrls, String lfcPath) {
		String xml = "<file><size>" + fileSize + "</size>" + "<mtime>"
				+ mtime.getTime() + "</mtime><replica>";
		int no = 0;
		StringBuilder sb = new StringBuilder(xml);
		for (String suburl : repUrls) {
			sb.append("<url no=\"" + no + "\">" + suburl + "</url>");
			no++;
		}
		sb.append("</replica></file>");
		try {
			return RNSUtil.toMessageElements(sb.toString());
		} catch (Exception e) {
			if (home.isDebugMode()) {
				e.printStackTrace();
			}
			warning("lfcPath=" + lfcPath + ": don't convert Metadata");
			return null;
		}
	}

	private static int countEEXIST = 0;

	private static void createRecursive(LFCServer lfcs, RNSClientHome home,
			String lfcPath, String rnsPath, boolean verbose)
			throws LFCException, RNSError {
		RNSClient rnsc = home.getRNSClient();
		ArrayList<FileDesc> dir = lfcs.listDirectory(lfcPath);

		for (FileDesc fd : dir) {
			String name = fd.getFileName();
			String newPath = rnsPath + "/" + name;
			if (fd.isDirectory()) {
				try {
					rnsc.mkdir(newPath);
					if (verbose) {
						System.out.println("mkdir: " + newPath);
					}
				} catch (RNSError e) {
					if (e.getError().equals(RNSError.Errno.EEXIST)) {
						countEEXIST++;
					} else {
						throw e;
					}
				}
				if (fd.getULink() > 0) {
					createRecursiveRetry(lfcs, home, lfcPath + "/" + name,
							newPath, verbose, false);
				}
			} else if (fd.isFile()) {
				if (countEEXIST > 5 && rnsc.exists(newPath)) {
					/* skip getReplicas() */
					continue;
				}

				String url = null;
				ArrayList<String> repUrls = new ArrayList<String>();
				ArrayList<ReplicaDesc> reps = lfcs.getReplicas(fd.getGuid(),
						true);
				for (ReplicaDesc rd : reps) {
					if (url == null) {
						url = rd.getSfn();
					} else {
						repUrls.add(rd.getSfn());
					}
				}
				if (url == null) {
					warning("lfcPath=" + lfcPath + ": no URL");
				} else {
					EndpointReferenceType epr = null;
					try {
						epr = RNSUtil.toEPR(new org.apache.axis.types.URI(url));
					} catch (MalformedURIException e) {
						if (home.isDebugMode()) {
							e.printStackTrace();
						}
					}
					if (epr == null) {
						warning("lfcPath=" + lfcPath + ": cannot convert EPR");
					} else {
						MessageElement[] mes = myMetadata(home,
								fd.getFileSize(), fd.getMDate(), repUrls,
								lfcPath);
						try {
							if (verbose) {
								System.out.println("addEPR: " + newPath);
							}
							rnsc.addJunction(newPath, epr, mes);
						} catch (RNSError e) {
							if (e.getError().equals(RNSError.Errno.EEXIST)) {
								countEEXIST++;
							} else {
								throw e;
							}
						}
					}
				}
			} else {
				warning("lfcPath=" + lfcPath + ": unsupported type");
			}
		}
	}

	private static void createRecursiveBulk(LFCServer lfcs, RNSClientHome home,
			String lfcPath, String rnsPath, boolean verbose)
			throws LFCException, RNSError {
		RNSClient rnsc = home.getRNSClient();
		ArrayList<FileDesc> dir = lfcs.listDirectory(lfcPath);
		ArrayList<String> mkdirList = new ArrayList<String>();
		RNSAddHandle rah = new RNSAddHandle();

		for (FileDesc fd : dir) {
			String name = fd.getFileName();
			String newPath = rnsPath + "/" + name;
			if (fd.isDirectory()) {
				rah.registerMkdir(name, null);
				if (verbose) {
					System.out.println("mkdir: " + newPath);
				}
				if (fd.getULink() > 0) {
					mkdirList.add(name);
				}
			} else if (fd.isFile()) {
				if (countEEXIST > 5 && rnsc.exists(newPath)) {
					/* skip getReplicas() */
					continue;
				}
				if (verbose) {
					System.out.println("addEPR(prepare): " + newPath);
				}
				String url = null;
				ArrayList<String> repUrls = new ArrayList<String>();
				ArrayList<ReplicaDesc> reps = lfcs.getReplicas(fd.getGuid(),
						true);
				for (ReplicaDesc rd : reps) {
					if (url == null) {
						url = rd.getSfn();
					} else {
						repUrls.add(rd.getSfn());
					}
				}
				if (url == null) {
					warning("lfcPath=" + lfcPath + ": no URL");
				} else {
					EndpointReferenceType epr = null;
					try {
						epr = RNSUtil.toEPR(new org.apache.axis.types.URI(url));
					} catch (MalformedURIException e) {
						if (home.isDebugMode()) {
							e.printStackTrace();
						}
					}
					if (epr == null) {
						warning("lfcPath=" + lfcPath + ": cannot convert EPR");
					} else {
						MessageElement[] mes = myMetadata(home,
								fd.getFileSize(), fd.getMDate(), repUrls,
								lfcPath);
						rah.registerAddEPR(name, epr, mes, false);
					}
				}
			} else {
				warning("lfcPath=" + lfcPath + ": unsupported type");
			}
		}
		RNSError[] errors = rnsc.addBulk(rnsPath, rah);
		if (errors != null && errors.length > 0) {
			for (RNSError e : errors) {
				if (e.getError().equals(RNSError.Errno.EEXIST)) {
					countEEXIST++;
				} else {
					System.err.println(e.toString());
				}
			}
			return;
		}
		for (String childDirName : mkdirList) {
			createRecursiveRetry(lfcs, home, lfcPath + "/" + childDirName,
					rnsPath + "/" + childDirName, verbose, true);
		}
	}

	private static void createRecursiveRetry(LFCServer lfcs,
			RNSClientHome home, String lfcPath, String rnsPath,
			boolean verbose, boolean bulk) throws RNSError {
		for (;;) {
			try {
				if (bulk) {
					createRecursiveBulk(lfcs, home, lfcPath, rnsPath, verbose);
				} else {
					createRecursive(lfcs, home, lfcPath, rnsPath, verbose);
				}
				return;
			} catch (LFCException e) {
				e.printStackTrace();
				boolean check = false;
				for (;;) {
					warning("retry to connect LFC server");
					try {
						lfcs.disconnect();
						lfcs.connect();
						break;
					} catch (LFCException e1) {
						if (!check) {
							e1.printStackTrace();
							check = true;
						}
					}
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e1) {
						e1.printStackTrace();
					}
				}
			}
		}
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		home.setCustomUsage("[+b|++bulk] [+v|++verbose] LFC_URL(lfn://host:port/path) RNS_Path");
		boolean usage = true;
		try {
			String[] shortFlags = { "b", "v" };
			String[] longFlags = { "bulk", "verbose" };
			Map<String, String> argMap = home.parseArgsWithPlusOption(args,
					shortFlags, longFlags, null, null, "arg", 2, 2);
			URI lfnUri = new URI(argMap.get("arg0"));
			String rnsPath = argMap.get("arg1");

			boolean verbose = false;
			if (argMap.containsKey("+v") || argMap.containsKey("++verbose")) {
				verbose = true;
			}

			RNSClient rnsclient = home.getRNSClient();
			usage = false;

			LFCServer lfcs = new LFCServer(lfnUri);
			FileDesc entfd = lfcs.fetchFileDesc(lfnUri.getPath());
			if (entfd.isDirectory()) {
				try {
					rnsclient.stat(rnsPath);
					/* a directory exists */
				} catch (RNSError rnse) {
					if (rnse.getError().equals(RNSError.Errno.ENOENT)) {
						rnsclient.mkdir(rnsPath);
						/* continue */
					} else {
						throw rnse;
					}
				}
				if (argMap.containsKey("+b") || argMap.containsKey("++bulk")) {
					createRecursiveRetry(lfcs, home, lfnUri.getPath(), rnsPath,
							verbose, true);
				} else {
					createRecursiveRetry(lfcs, home, lfnUri.getPath(), rnsPath,
							verbose, false);
				}
			} else {
				/* copy a junction */
				String lfcPath = lfnUri.getPath();
				if (entfd.isFile()) {
					if (verbose) {
						System.out.println("addEPR: " + rnsPath);
					}
					String url = null;
					ArrayList<String> repUrls = new ArrayList<String>();
					ArrayList<ReplicaDesc> reps = lfcs.getReplicas(
							entfd.getGuid(), true);
					for (ReplicaDesc rd : reps) {
						if (url == null) {
							url = rd.getSfn();
						} else {
							repUrls.add(rd.getSfn());
						}
					}
					if (url == null) {
						warning("lfcPath=" + lfcPath + ": do not have URL");
					} else {
						EndpointReferenceType epr = null;
						try {
							epr = RNSUtil.toEPR(new org.apache.axis.types.URI(
									url));
						} catch (MalformedURIException e) {
							if (home.isDebugMode()) {
								e.printStackTrace();
							}
						}
						if (epr == null) {
							warning("lfcPath=" + lfcPath
									+ ": don't convert EPR");
						} else {
							MessageElement[] mes = myMetadata(home,
									entfd.getFileSize(), entfd.getMDate(),
									repUrls, lfcPath);
							rnsclient.addJunction(rnsPath, epr, mes);
						}
					}
				} else {
					warning("lfcPath=" + lfcPath + ": unsupported type");
				}
			}
		} catch (RuntimeException e) {
			e.printStackTrace();
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
