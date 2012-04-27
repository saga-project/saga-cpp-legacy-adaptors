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
package org.naregi.rns.test;

import java.util.ArrayList;
import java.util.List;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSSearchClient;
import org.naregi.rns.client.RNSSearchResult;
import org.naregi.rns.client.RNSSearchResultHandle;
import org.naregi.rns.util.RNSUtil;

/**
 * Stress test and test of multi-threaded client.
 */
public class TestStress {

	private static boolean stop = false;

	private static class MkdirFlat extends Thread {
		RNSClient rnsc;
		RNSSearchClient rnss;
		String dir;
		int num;

		MkdirFlat(RNSClientHome home, String dir, int num) {
			this.rnsc = home.getRNSClient();
			this.rnss = home.getRNSSearchClient();
			this.dir = dir + "/flat";
			this.num = num;
		}

		public void run() {
			try {
				rnsc.mkdir(dir);
			} catch (RNSError re) {
				if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
					re.printStackTrace();
					stop = true;
					return;
				}
			}

			int div = num / 10;
			if (div == 0) {
				div = 1;
			} else if (div > 100) {
				div = 100;
			}
			ArrayList<String> al = new ArrayList<String>();
			for (int i = 0; i < num; i++) {
				if (stop) {
					return;
				}
				String path = dir + "/" + i;
				if (i % div == 0) {
					System.out.println("[progress] mkdir flat: " + path);
				}
				al.add("" + i);
				try {
					rnsc.mkdir(path);
				} catch (RNSError re) {
					if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
						re.printStackTrace();
						stop = true;
						return;
					}
				}
			}

			/* search */
			try {
				RNSSearchResultHandle results = rnss.searchBulk(dir,
						al.toArray(new String[0]), "/");
				int i = 0;
				for (RNSSearchResult res : results) {
					if (stop) {
						return;
					}
					if (i % div == 0) {
						System.out.println("[progress] search bulk: "
								+ res.getPath());
					}
					RNSError error = res.getError();
					if (error != null) {
						throw error;
					}
					i++;
				}
				RNSError error = results.getError();
				if (error != null) {
					throw error;
				}
			} catch (RNSError re) {
				re.printStackTrace();
				stop = true;
				return;
			}
		}
	}

	private static String omitString(String str) {
		int len = str.length();
		if (len > 50) {
			return str.substring(0, 28) + "..." + str.substring(len - 20, len);
		} else {
			return str;
		}
	}

	private static class MkdirDeep extends Thread {
		RNSClient rnsc;
		RNSSearchClient rnss;
		String dir;
		int num;

		MkdirDeep(RNSClientHome home, String dir, int num) {
			this.rnsc = home.getRNSClient();
			this.rnss = home.getRNSSearchClient();
			this.dir = dir + "/deep";

			int depth = num;
			if (depth > 1000) {
				depth = 1000; /* limiter */
			}
			this.num = depth;
		}

		public void run() {
			try {
				rnsc.mkdir(dir);
			} catch (RNSError re) {
				if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
					re.printStackTrace();
					stop = true;
					return;
				}
			}

			int div = num / 10;
			if (div == 0) {
				div = 1;
			} else if (div > 100) {
				div = 100;
			}
			StringBuilder sb = new StringBuilder(dir);
			for (int i = 0; i < num; i++) {
				if (stop) {
					return;
				}
				sb.append("/" + String.valueOf(i));
				String path = sb.toString();
				if (i % div == 0) {
					System.out.println("[progress] mkdir deep: "
							+ omitString(path));
				}
				try {
					rnsc.mkdir(path);
				} catch (RNSError re) {
					if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
						re.printStackTrace();
						stop = true;
						return;
					}
				}
			}

			/* search recursive */
			try {
				RNSSearchResultHandle results = rnss.searchRecursive(dir, "/",
						num);
				int i = 0;
				for (RNSSearchResult res : results) {
					if (stop) {
						return;
					}
					if (i % div == 0) {
						System.out.println("[progress] search recursive: "
								+ omitString(res.getPath()));
					}
					RNSError error = res.getError();
					if (error != null) {
						throw error;
					}
					i++;
				}
				RNSError error = results.getError();
				if (error != null) {
					throw error;
				}
			} catch (RNSError re) {
				re.printStackTrace();
				stop = true;
				return;
			}
		}
	}

	private static String str100KB() {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < 1000; i++) {
			/* 100byte */
			sb.append("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
		}
		return sb.toString();
	}

	private static MessageElement[] generateBigMeatadata(int nKB)
			throws Exception {
		StringBuilder sb = new StringBuilder();
		sb.append("<test>");

		int n = nKB / 100;
		String str = str100KB();

		for (int i = 0; i < n; i++) {
			sb.append("<tag" + i + ">");
			sb.append(str);
			sb.append("</tag" + i + ">");
		}
		sb.append("</test>");
		return RNSUtil.toMessageElements(sb.toString());
	}

	private static class BigMetadata extends Thread {
		RNSClient rnsc;
		String path;
		int num;

		BigMetadata(RNSClient rnsc, String dir, int num) {
			this.rnsc = rnsc;
			this.path = dir + "/bigMetadata";

			num = num / 100;
			if (num > 100) {
				num = 100; /* limiter */
			} else if (num <= 0){
				num = 1;
			}
			this.num = num;
		}

		public void run() {
			try {
				rnsc.remove(path);
			} catch (RNSError re) {
				/* ignore */
			}

			/* generate big metadata */
			MessageElement[] mes;
			try {
				mes = generateBigMeatadata(900); /* 900KB */
			} catch (Exception e) {
				e.printStackTrace();
				stop = true;
				return;
			}

			try {
				EndpointReferenceType epr = RNSUtil.toEPR(new URI(
						"http://www.example.com/"));
				rnsc.addJunction(path, epr, mes);

				int div = num / 10;
				if (div == 0) {
					div = 1;
				} else if (div > 100) {
					div = 100;
				}
				for (int i = 0; i < num; i++) {
					if (stop) {
						return;
					}
					if (i % div == 0) {
						System.out.println("[progress] set big metadata (" + i
								+ "): " + path);
					}
					rnsc.setMetadata(path, mes);
				}
				rnsc.remove(path);
			} catch (RNSError re) {
				re.printStackTrace();
				stop = true;
				return;
			} catch (MalformedURIException e) {
				e.printStackTrace();
				stop = true;
				return;
			}
		}
	}

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		boolean usage = true;
		try {
			home.setCustomUsage("[number [n_loop]]");
			List<String> al = home.parseArgs(args, 0, 2);
			RNSClient rnsc = home.getRNSClient();

			String dir = "/_TEST_STRESS";
			int num = 10000;
			if (al.size() >= 1) {
				num = Integer.parseInt(al.get(0));
			}
			int loop = 1;
			if (al.size() >= 2) {
				loop = Integer.parseInt(al.get(1));
			}

			usage = false;

			try {
				rnsc.mkdir(dir);
			} catch (RNSError re) {
				if (re.getError().equals(RNSError.Errno.EEXIST) == false) {
					throw re;
				}
			}

			for (int i = 1; i <= loop; i++) {
				if (stop) {
					return;
				}
				System.out.println("LOOP " + i);
				BigMetadata bigMetadata = new BigMetadata(rnsc, dir, num);
				bigMetadata.start();

				MkdirFlat mkdirFlat1 = new MkdirFlat(home, dir, num);
				mkdirFlat1.start();
				MkdirFlat mkdirFlat2 = new MkdirFlat(home, dir, num);
				mkdirFlat2.start();

				MkdirDeep mkdirDeep1 = new MkdirDeep(home, dir, num);
				mkdirDeep1.start();
				MkdirDeep mkdirDeep2 = new MkdirDeep(home, dir, num);
				mkdirDeep2.start();

				mkdirFlat1.join();
				mkdirFlat2.join();

				mkdirDeep1.join();
				mkdirDeep2.join();

				bigMetadata.join();
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
