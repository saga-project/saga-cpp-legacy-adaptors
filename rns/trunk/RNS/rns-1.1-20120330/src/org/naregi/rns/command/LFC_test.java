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
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Random;

import org.glite.lfc.LFCException;
import org.glite.lfc.LFCServer;
import org.glite.lfc.internal.FileDesc;
import org.glite.lfc.internal.ReplicaDesc;

/**
 * lfcj-test
 */
public class LFC_test {

	private static void clear(LFCServer lfcServer, String path) {
		FileDesc fd;
		try {
			System.out.println("clear: path=" + path);
			fd = lfcServer.fetchFileDesc(path);
			lfcServer.deleteFile(fd.getGuid(), path);
		} catch (LFCException e1) {
		}
	}

	private static Random rand = new Random();

	private static String randomString() {
		int i = rand.nextInt();
		if (i < 0) {
			i = -i;
		}
		return Integer.toString(i, Character.MAX_RADIX);
	}

	private static void test(LFCServer lfcServer, String path)
			throws LFCException, URISyntaxException {
		clear(lfcServer, path);

		lfcServer.register(new URI("http://www.example.com/" + randomString()),
				path, 12345);

		FileDesc fd2;
		fd2 = lfcServer.fetchFileDesc(path);
		System.out.println("mtime=" + fd2.getMDate());
		System.out.println("guid=" + fd2.getGuid());
		System.out.println("size=" + fd2.getFileSize());
		lfcServer.addReplica(fd2, new URI("http://www.example.com/"
				+ randomString()));
		lfcServer.addReplica(fd2, new URI("http://www.example.com/"
				+ randomString()));
		ArrayList<ReplicaDesc> al = lfcServer.getReplicasByPath(path);
		for (ReplicaDesc rd : al) {
			System.out.println(rd);
		}

		// String parent = path.substring(0, path.lastIndexOf("/"));
		// System.out.println("lfc-ls -l " + parent);
		// String[] s = { "-l", "-guid", "lfn://" + host + ":" + port + parent
		// };
		// LfcCommand.doLS(s);

		// clear(lfcServer, path);
	}

	private static void clearAll(LFCServer lfcServer, String dir) {
		try {
			ArrayList<FileDesc> l = lfcServer.listDirectory(dir);
			if (l == null) {
				return;
			}
			for (FileDesc fd : l) {
				String name = fd.getFileName();
				String path = dir + "/" + name;
				String perm = fd.getPermissions();
				if (perm == null) {
					clear(lfcServer, path);
				} else if (perm.charAt(0) == 'd') {
					clearAll(lfcServer, path);
					System.out.println("rmdir: path=" + path);
					lfcServer.rmdir(path);
				} else {
					clear(lfcServer, path);
				}
			}
			lfcServer.rmdir(dir);
		} catch (LFCException e) {
			e.printStackTrace();
		}
	}

	public static void main(String args[]) {
		if (args.length <= 3) {
			System.err.println("LFC_Host Port TestDirPath num|clean (debug)");
			return;
		}
		String host = args[0];
		String port = args[1];
		String path = args[2];
		String num = null;
		if (args.length >= 4) {
			num = args[3];
		}
		String debug = null;
		if (args.length >= 5) {
			debug = args[4];
		}

		LFCServer lfcServer;
		try {
			if (debug != null) {
				LFCServer.getLogger().printLog = true;
				LFCServer.getLogger().printIOLog = true;
			}

			lfcServer = new LFCServer(new URI("lfn://" + host + ":" + port
					+ "/"));
		} catch (URISyntaxException e) {
			e.printStackTrace();
			return;
		}

		if (num != null && num.equals("clean")) {
			clearAll(lfcServer, path);
		} else if (num != null) {
			int n = Integer.parseInt(num);
			try {
				// NOTE: mkdir() cannot work.
//				lfcServer.mkdir(path);
				for (int i = 0; i < n; i++) {
//					String dirName = randomString();
//					lfcServer.mkdir(path + "/" + dirName);
//					test(lfcServer, path + "/" + dirName + "/" + randomString());
					test(lfcServer, path + "/" + randomString());
				}
			} catch (LFCException e) {
				e.printStackTrace();
			} catch (URISyntaxException e) {
				e.printStackTrace();
			}
		}
	}
}
