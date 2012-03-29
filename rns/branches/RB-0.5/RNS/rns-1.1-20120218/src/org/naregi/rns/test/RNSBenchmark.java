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
package org.naregi.rns.test;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.Map.Entry;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSAddHandle;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSExtensionClient;
import org.naregi.rns.client.RNSSearchClient;
import org.naregi.rns.client.RNSSearchResult;
import org.naregi.rns.client.RNSSearchResultHandle;
import org.naregi.rns.stubs.ProfileType;
import org.naregi.rns.util.RNSUtil;

/**
 * Benchmark test.
 */
public class RNSBenchmark {

	private static long start() {
		return System.nanoTime();
	}

	private static boolean stop = false;

	private static long clientTotal = 0;

	public static String format(String type, long time, int count) {
		return String.format("%7d", (time / 1000000))
				+ "ms ("
				+ String.format("%8.3f", (double) time / (double) count
						/ 1000000) + "ms * " + String.format("%7d", count)
				+ ") : " + type.toString();
	}

	private static void stop(String type, long startTime, int count) {
		long time = System.nanoTime() - startTime;
		System.out.println(format(type, time, count));
		clientTotal += time;
	}

	private static void clearDir(RNSClient rnsc, String basedir, int depth) {
		try {
			rnsc.removeRecursive(basedir, depth);
		} catch (RNSError e) {
		}
	}

	private static void bench_bind_host(RNSClientHome home, int n)
			throws RNSError {
		if (stop) {
			return;
		}
		EndpointReferenceType epr = home.getEPR();
		String host = epr.getAddress().getHost();
		long start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			try {
				InetAddress ia = InetAddress.getByName(host);
				InetAddress ia2 = InetAddress.getByAddress(ia.getAddress());
				ia2.getHostName();
			} catch (UnknownHostException e) {
				e.printStackTrace();
			}
		}
		stop("InetAddress.getByName(" + host + ")", start, n);
	}

	private static void bench_noop(RNSClientHome home, int n) throws RNSError {
		if (stop) {
			return;
		}
		RNSExtensionClient rnse = home.getRNSExtensionClient();
		long start = start();
		for (int i = 0; i < n && stop == false; i++) {
			rnse.noop();
		}
		stop("NO OPERATION (empty communication)", start, n);
	}

	private static void bench_mkdir_depth_N(RNSClientHome home, String basedir,
			int n) throws RNSError {
		if (stop) {
			return;
		}
		RNSClient rnsc = home.getRNSClient();
		RNSSearchClient rnss = home.getRNSSearchClient();
		long start = start();
		StringBuilder sb = new StringBuilder(basedir);
		for (int i = 1; i <= n && stop == false; i++) {
			sb.append("/" + String.valueOf(i));
			rnsc.mkdir(sb.toString());
		}
		stop("MKDIR (deep)", start, n);

		/* search */
		String xq = RNSUtil.generateXQueryForRNSSearch(null);
		start = start();
		RNSSearchResultHandle handle = rnss.searchRecursive(basedir, xq, n);
		if (handle != null) {
			for (RNSSearchResult r : handle) {
				if (stop) {
					return;
				}
				r.getMetadataString();
			}
		}
		stop("SEARCH RECURSIVE (deep)", start, n);

		/* clean */
		start = start();
		for (int i = n; i >= 1 && stop == false; i--) {
			StringBuilder sb2 = new StringBuilder(basedir);
			for (int j = 1; j <= i; j++) {
				sb2.append("/" + String.valueOf(j));
			}
			rnsc.rmdir(sb2.toString());
		}
		stop("RMDIR (deep) (with DESTROY)", start, n);
	}

	private static void bench_rename_N(RNSClientHome home, String basedir, int n)
			throws RNSError {
		if (stop) {
			return;
		}
		RNSClient rnsc = home.getRNSClient();
		String name1 = basedir + "/" + "1";
		String name2 = basedir + "/" + "2";
		rnsc.mkdir(name1);

		n = n / 2;
		if (n == 0) {
			n = 1;
		}
		long start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.rename(name1, name2);
			rnsc.rename(name2, name1);
		}
		stop("RENAME", start, n * 2);

		rnsc.remove(name1);
	}

	private static void bench_mkdir_N(RNSClientHome home, String basedir, int n)
			throws Exception {
		if (stop) {
			return;
		}
		RNSClient rnsc = home.getRNSClient();
		long start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.mkdir(basedir + "/" + String.valueOf(i));
		}
		stop("MKDIR (flat)", start, n);

		String xmlStr = "<A>test</A>";
		MessageElement[] any1 = RNSUtil.toMessageElements(xmlStr);
		start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.setMetadata(basedir + "/" + String.valueOf(i), any1);
		}
		stop("SET METADATA (flat)", start, n);

		start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.rmdir(basedir + "/" + String.valueOf(i));
		}
		stop("RMDIR (flat) (with DESTROY)", start, n);

		EndpointReferenceType epr = RNSUtil.toEPR(new URI(
				"http://www.example.com/"));
		start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.addJunction(basedir + "/" + String.valueOf(i), epr);
		}
		stop("ADD JUNCTION (flat)", start, n);

		start = start();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.rmJunction(basedir + "/" + String.valueOf(i));
		}
		stop("REMOVE JUNCTION (flat)", start, n);
	}

	private static void bench_list_search_N(RNSClientHome home, String basedir,
			int n) throws RNSError {
		if (stop) {
			return;
		}
		RNSClient rnsc = home.getRNSClient();
		RNSSearchClient rnss = home.getRNSSearchClient();
		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.mkdir(basedir + "/" + String.valueOf(i));
		}

		long start = start();
		RNSDirHandle dh = rnsc.list(basedir, false);
		if (dh != null) {
			for (RNSDirent de : dh) {
				if (stop) {
					return;
				}
				de.getName();
			}
			RNSError e = dh.getError();
			if (e != null) {
				throw e;
			}
		}
		stop("LIST", start, n);

		start = start();
		dh = rnsc.list(basedir, true);
		if (dh != null) {
			for (RNSDirent de : dh) {
				if (stop) {
					return;
				}
				de.getName();
			}
			RNSError e = dh.getError();
			if (e != null) {
				throw e;
			}
		}
		stop("LIST with STAT", start, n);

		String xq = RNSUtil.generateXQueryForRNSSearch(null);
		start = start();
		RNSSearchResultHandle handle = rnss.searchBulk(basedir, null, xq);
		if (handle != null) {
			for (RNSSearchResult r : handle) {
				if (stop) {
					return;
				}
				r.getMetadataString();
			}
		}
		stop("SEARCH (flat)", start, n);

		for (int i = 1; i <= n && stop == false; i++) {
			rnsc.rmdir(basedir + "/" + String.valueOf(i));
		}
	}

	private static void bench_mkdirBulk_N(RNSClientHome home, String basedir,
			int n) throws Exception {
		if (stop) {
			return;
		}
		long start = start();
		RNSClient rnsc = home.getRNSClient();
		RNSAddHandle addHandle = new RNSAddHandle();
		for (int i = 1; i <= n && stop == false; i++) {
			addHandle.registerMkdir(String.valueOf(i), null);
		}
		RNSError[] errors = rnsc.addBulk(basedir, addHandle);
		if (errors != null) {
			for (RNSError e : errors) {
				e.printStackTrace();
			}
		}
		stop("BULK MKDIR", start, n);

		String xmlStr = "<A>test</A>";
		MessageElement[] mes = RNSUtil.toMessageElements(xmlStr);
		Map<String, MessageElement[]> map = new HashMap<String, MessageElement[]>();
		for (int i = 1; i <= n && stop == false; i++) {
			map.put(Integer.toString(i), mes);
		}
		start = start();
		errors = rnsc.setMetadataBulk(basedir, map);
		if (errors != null) {
			for (RNSError e : errors) {
				e.printStackTrace();
			}
		}
		stop("BULK SET METADATA", start, n);

		ArrayList<String> al = new ArrayList<String>();
		for (int i = 1; i <= n && stop == false; i++) {
			al.add(String.valueOf(i));
		}
		String[] names = al.toArray(new String[0]);
		start = start();
		errors = rnsc.removeBulk(basedir, names);
		if (errors != null) {
			for (RNSError e : errors) {
				e.printStackTrace();
			}
		}
		stop("BULK RMDIR (with DESTROY)", start, n);
	}

	private static void printServerStatus(RNSExtensionClient rnsx)
			throws RNSError {
		Map<String, String> map = rnsx.getServerStatus();
		for (Entry<String, String> m : map.entrySet()) {
			System.out.println("[SERVER STATUS] "
					+ String.format("%12s", m.getKey()) + " = "
					+ String.format("%10s", m.getValue()));
		}
	}

	private static class ClearThread extends Thread {
		RNSClient rnsc;
		String basedir;

		ClearThread(RNSClient rnsc, String basedir) {
			this.rnsc = rnsc;
			this.basedir = basedir;
		}

		@Override
		public void run() {
			stop = true;
			clearDir(rnsc, basedir, maxDepth);
		}
	}

	private static int maxDepth = 10000; /* limiter */

	public static void main(String[] args) {
		RNSClientHome home = new RNSClientHome();
		boolean usage = true;
		try {
			home.setCustomUsage("[count(default 200) [RNS_path(workdir)]]");
			List<String> l = home.parseArgs(args, 0, 2);
			RNSClient rnsc = home.getRNSClient();
			RNSExtensionClient rnsx = home.getRNSExtensionClient();

			String basedir;
			int testN = 200; /* default */
			if (l.size() == 0) {
				basedir = "__BENCH_" + UUID.randomUUID();
				clearDir(rnsc, basedir, maxDepth);
			} else if (l.size() == 1) {
				testN = Integer.parseInt(l.get(0));
				basedir = "__BENCH_" + UUID.randomUUID();
				clearDir(rnsc, basedir, maxDepth);
			} else {
				testN = Integer.parseInt(l.get(0));
				basedir = l.get(1);
			}
			usage = false;

			Runtime.getRuntime()
					.addShutdownHook(new ClearThread(rnsc, basedir));

			printServerStatus(rnsx);

			rnsx.startProfile();

			rnsc.mkdir(basedir);

			bench_bind_host(home, testN);
			bench_noop(home, testN);

			bench_list_search_N(home, basedir, testN);

			bench_rename_N(home, basedir, testN);

			/* mkdir basedir/{1,2,...,N} */
			bench_mkdir_N(home, basedir, testN);
			bench_mkdirBulk_N(home, basedir, testN);

			/* mkdir basedir/1/2/.../N */
			bench_mkdir_depth_N(home, basedir, testN > maxDepth ? maxDepth
					: testN);
			if (stop) {
				return;
			}

			System.out.println(String.format("%7d", clientTotal / 1000000)
					+ "ms (CLIENT TOTAL)");

			System.out.println();

			ProfileType[] profs = rnsx.stopProfile();
			long serverTotal = 0;
			if (profs != null) {
				for (ProfileType prof : profs) {
					System.out.println(format("[SERVER] " + prof.getName(),
							prof.getTotal().longValue(), prof.getCount()
									.intValue()));
					if (prof.getName().startsWith("Total_")) {
						serverTotal += prof.getTotal().longValue();
					}
				}
			}
			System.out.println(String.format("%7d", serverTotal / 1000000)
					+ "ms (SERVER TOTAL) (sum of Total_*)");

			printServerStatus(rnsx);

			rnsc.rmdir(basedir);
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
