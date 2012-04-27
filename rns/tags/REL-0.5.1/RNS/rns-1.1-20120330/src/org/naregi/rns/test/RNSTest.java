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
import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import junit.framework.TestCase;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI;
import org.apache.axis.types.URI.MalformedURIException;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.client.RNSAddHandle;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSKeyValue;
import org.naregi.rns.client.RNSSearchClient;
import org.naregi.rns.client.RNSSearchResult;
import org.naregi.rns.client.RNSSearchResultHandle;
import org.naregi.rns.client.RNSStat;
import org.naregi.rns.util.RNSUtil;

/**
 * Test case by JUnit framework
 */
public class RNSTest extends TestCase {
	private static final String TESTDIR_PREFIX = "/_JUNIT_";
	private static String TESTDIR = null;

	private static RNSClientHome home = null;
	private static RNSClient client = null;
	// private static RNSExtensionClient xClient = null;
	private static RNSSearchClient sClient = null;

	private EndpointReferenceType dummyEPR = null;

	private synchronized RNSClientHome getClientHome() {
		if (home != null) {
			return home;
		}
		home = new RNSClientHome();
		try {
			String[] a = new String[0];
			home.parseArgs(a, 0, 0);
			return home;
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
			return null;
		}
	}

	private synchronized RNSClient getClient() {
		if (client != null) {
			return client;
		}
		RNSClientHome home = getClientHome();
		try {
			client = home.getRNSClient();
			if (client == null) {
				fail("home.getRNSClient() is null");
			}
			return client;
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
		return null;
	}

	// private synchronized RNSExtensionClient getExtensionClient() {
	// if (xClient != null) {
	// return xClient;
	// }
	// RNSClientHome home = getClientHome();
	// try {
	// xClient = home.getRNSExtensionClient();
	// if (xClient == null) {
	// fail("home.getRNSExtensionClient() is null");
	// }
	// return xClient;
	// } catch (Exception e) {
	// e.printStackTrace();
	// fail(e.getMessage());
	// }
	// return null;
	// }

	private synchronized RNSSearchClient getRNSSearchClient() {
		if (sClient != null) {
			return sClient;
		}
		RNSClientHome home = getClientHome();
		try {
			sClient = home.getRNSSearchClient();
			if (sClient == null) {
				fail("home.getRNSSearchClient() is null");
			}
			return sClient;
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
		return null;
	}

	public void setUp() {
		if (TESTDIR == null) {
			TESTDIR = TESTDIR_PREFIX + UUID.randomUUID();
			System.out.println("TESTDIR: " + TESTDIR);
		}
		if (dummyEPR == null) {
			try {
				dummyEPR = RNSUtil.toRNSEPR(new URI("http://example.com/"));
			} catch (MalformedURIException e) {
				e.printStackTrace();
				fail(e.getMessage());
			}
		}

		RNSClient client = getClient();
		try {
			client.removeRecursive(TESTDIR);
		} catch (Exception e) {
			// ignore
		}
		try {
			client.mkdir(TESTDIR);
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void tearDown() {
		RNSClient client = getClient();
		try {
			client.removeRecursive(TESTDIR);
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	private void checkRNSError(RNSError.Errno expected, RNSError e) {
		if (expected != e.getError()) {
			e.printStackTrace();
			assertEquals(expected, e.getError());
		}
	}

	private void shouldFail(String name) {
		fail(name + " should be failed");
	}

	public void test_stat() {
		RNSClient client = getClient();
		try {
			RNSStat st = client.stat(TESTDIR);
			assertNotNull(st);
			assertEquals(0, st.getElementCount().longValue());
			String test = TESTDIR + "/a";
			client.mkdir(test);
			st = client.stat(TESTDIR);
			assertNotNull(st);
			assertEquals(1, st.getElementCount().longValue());
			st = client.stat(test);
			assertNotNull(st);
			assertEquals(0, st.getElementCount().longValue());

			// long a1 = st.getAccessTime().getTimeInMillis();
			// Thread.sleep(100);
			// long a2 = Calendar.getInstance().getTimeInMillis();
			// Thread.sleep(100);
			// client.list(test);
			// st = client.stat(test);
			// assertNotNull(st);
			// long a3 = st.getAccessTime().getTimeInMillis();
			// assertTrue("a2(" + a2 + ") >  a1(" + a1 + ")", a2 > a1);
			// assertTrue("a3(" + a3 + ") >  a2(" + a2 + ")", a3 > a2);

			st = client.stat(TESTDIR);
			long m1 = st.getModificationTime().getTimeInMillis();
			Thread.sleep(100);
			long m2 = Calendar.getInstance().getTimeInMillis();
			Thread.sleep(100);
			client.rmdir(test);
			st = client.stat(TESTDIR);
			long m3 = st.getModificationTime().getTimeInMillis();
			assertTrue("m2(" + m2 + ") >  m1(" + m1 + ")", m2 > m1);
			assertTrue("m3(" + m3 + ") >  m2(" + m2 + ")", m3 > m2);

			client.addJunction(test, dummyEPR);
			try {
				client.stat(test);
				shouldFail("stat");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOTDIR, e);
			}
			client.rmJunction(test);
		} catch (InterruptedException e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_path_mkdir_rmdir() {
		RNSClient client = getClient();
		try {
			client.mkdir(TESTDIR + "/a");
			try {
				client.mkdir(TESTDIR + "/a/b/c");
				shouldFail("mkdir: ENOENT");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOENT, e);
			}
			client.mkdir(TESTDIR + "/a/b/");
			client.mkdir(TESTDIR + "/a/b/c");
			client.mkdir(TESTDIR + "/a/b/c/d/");
			try {
				client.mkdir(TESTDIR + "/a/b/c/././d");
				shouldFail("mkdir");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EEXIST, e);
			}
			try {
				client.mkdir(TESTDIR + "/a/b/../c/../../a");
				shouldFail("mkdir");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EEXIST, e);
			}
			try {
				client.rmdir(TESTDIR + "/a");
				shouldFail("rmdir");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOTEMPTY, e);
			}
			RNSDirHandle l1 = client.list(TESTDIR + "/a/b/c", false);
			assertNotNull(l1);
			for (RNSDirent de : l1) {
				assertEquals("d", de.getName());
			}
			assertNull(l1.getError());
			client.rmdir(TESTDIR + "/a/b/c/d");
			client.addJunction(TESTDIR + "/a/b/c/d", dummyEPR);
			try {
				client.rmdir(TESTDIR + "/a/b/c/d");
				shouldFail("rmdir: ENOTDIR");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOTDIR, e);
			}
			client.rmJunction(TESTDIR + "/a/b/c/d");
			try {
				client.rmdir(TESTDIR + "/a/b/c/d");
				shouldFail("rmdir: ENOENT");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOENT, e);
			}
			try {
				client.getEPR(TESTDIR + "/a/b/c/d", true);
				shouldFail("getEPR");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOENT, e);
			}
			RNSDirHandle l2 = client.list(TESTDIR + "/a/b/c", false);
			assertNull(l2);
			if (l2 != null) {
				for (RNSDirent de : l2) {
					assertNotSame("d", de.getName());
					fail("This should have no children: name=" + de.getName());
				}
			}
			client.removeRecursive(TESTDIR + "/a");
			RNSDirHandle l3 = client.list(TESTDIR, false);
			assertNull(l3);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_rename() {
		RNSClient client = getClient();
		try {
			client.mkdir(TESTDIR + "/a");
			client.mkdir(TESTDIR + "/a/b");
			client.mkdir(TESTDIR + "/a/b/c");
			client.rename(TESTDIR + "/a", TESTDIR + "/tmp");
			try {
				client.rename(TESTDIR + "/a", TESTDIR + "/tmp");
				shouldFail("rename: ENOENT: from-entry");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ENOENT, e);
			}
			client.rename(TESTDIR + "/tmp", TESTDIR + "/a");
			client.mkdir(TESTDIR + "/d");
			// check move
			client.rename(TESTDIR + "/d", TESTDIR + "/a/");
			client.rename(TESTDIR + "/a/d", TESTDIR + "/d");
			try {
				client.rename(TESTDIR + "/d", TESTDIR + "/a/b/c");
				shouldFail("rename: EEXIST: to-entry");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EEXIST, e);
			}
			client.rename(TESTDIR + "/d", TESTDIR + "/a/b/c/");
			try {
				client.rename(TESTDIR + "/a", TESTDIR + "/a/b/c/");
				shouldFail("rename: EINVAL: subdirectory of itself");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EINVAL, e);
			}
			client.rmdir(TESTDIR + "/a/b/c/d");
			client.rmdir(TESTDIR + "/a/b/c");
			client.rmdir(TESTDIR + "/a/b");
			client.rmdir(TESTDIR + "/a");
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_copyEPR_removeReference() {
		RNSClient client = getClient();
		try {
			String from = TESTDIR + "/a";
			String to = TESTDIR + "/b";
			client.mkdir(from);
			client.mkdir(from + "/abc");
			client.copyEntry(from, to);
			try {
				client.rmJunction(from);
				shouldFail("rmJunction");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EISDIR, e);
			}
			client.removeReference(from);
			client.list(to, false);
			client.stat(to);
			client.mkdir(to + "/dir");
			client.removeRecursive(to);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_remove() {
		RNSClient client = getClient();
		try {
			String test = TESTDIR + "/a";
			client.mkdir(test);
			client.remove(test);
			client.addJunction(test, dummyEPR);
			client.remove(test);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_maxRecursive() {
		RNSClient client = getClient();
		int recursiveNum = 5;
		int save = client.getDefaultMaxRecursiveDepth();
		try {
			client.setDefaultMaxRecursiveDepth(recursiveNum);
			String name = "/dir";
			String base = TESTDIR + name;

			StringBuilder sbPath = new StringBuilder(base);
			client.mkdir(sbPath.toString());
			for (int i = 0; i < recursiveNum - 1; i++) {
				sbPath.append(name);
				client.mkdir(sbPath.toString());
			}
			client.removeRecursive(base);

			sbPath = new StringBuilder(base);
			client.mkdir(sbPath.toString());
			for (int i = 0; i < recursiveNum; i++) {
				sbPath.append(name);
				client.mkdir(sbPath.toString());
			}
			try {
				client.removeRecursive(base);
				shouldFail("removeRecursive");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.ELOOP, e);
			}
			client.rmdir(sbPath.toString()); /* -1 */
			client.removeRecursive(base); /* success */

			client.setDefaultMaxRecursiveDepth(save);
		} catch (RNSError e) {
			client.setDefaultMaxRecursiveDepth(save);
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_list() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/dir";
			client.mkdir(path);
			int num = 100;
			for (int i = 0; i < num; i++) {
				client.mkdir(path + "/" + i);
			}
			RNSDirHandle dir = client.list(path, false);
			assertNotNull(dir);
			int count = 0;
			ArrayList<RNSDirent> al = new ArrayList<RNSDirent>();
			for (RNSDirent de : dir) {
				al.add(de);
				count++;
			}
			assertNull(dir.getError());
			assertEquals("num == count(" + num + ")", num, count);
			for (RNSDirent rd : al) {
				client.rmdir(path + "/" + rd.getName());
			}
			client.rmdir(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_mkdirBulk_setMetadataBulk() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/dir";
			client.mkdir(path);
			RNSAddHandle addHandle = new RNSAddHandle();
			int num = 100;
			for (int i = 0; i < num; i++) {
				addHandle.registerMkdir(Integer.toString(i), null);
			}
			RNSError[] rnse = client.addBulk(path, addHandle);
			if (rnse != null && rnse.length > 0) {
				fail(rnse[0].getMessage());
			}

			String xmlStr = "<A>test</A>\n----\n<B><C/></B>\n----\n<D test=\"value\" />";
			MessageElement[] mes = RNSUtil.toMessageElements(xmlStr);
			Map<String, MessageElement[]> map = new HashMap<String, MessageElement[]>();
			for (int i = 0; i < num; i++) {
				map.put(Integer.toString(i), mes);
			}
			RNSError[] es = client.setMetadataBulk(path, map);
			assertNull(es);

			RNSDirHandle dir = client.list(path, false);
			assertNotNull(dir);
			int count = 0;
			ArrayList<RNSDirent> al = new ArrayList<RNSDirent>();
			for (RNSDirent rd : dir) {
				MessageElement[] mes2 = rd.getMeta().get_any();
				assertNotNull(mes2);
				rd.setEpr(null);
				rd.setMeta(null);
				al.add(rd);
				count++;
			}
			assertNull(dir.getError());
			assertEquals("num == count(" + num + ")", num, count);

			for (RNSDirent rd : al) {
				client.rmdir(path + "/" + rd.getName());
			}
			client.rmdir(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_addEPRBulk_removeBulk() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/dir";
			client.mkdir(path);
			RNSAddHandle addHandle = new RNSAddHandle();
			int num = 100;
			EndpointReferenceType epr = new EndpointReferenceType();
			epr.setAddress(new Address("http://www.example.com/test"));
			String xmlStr = "<A>test</A>\n----\n<B><C/></B>\n----\n<D test=\"value\" />";
			MessageElement[] mes = RNSUtil.toMessageElements(xmlStr);
			for (int i = 0; i < num; i++) {
				addHandle.registerAddEPR(Integer.toString(i), epr, mes, false);
			}
			RNSError[] rnse = client.addBulk(path, addHandle);
			if (rnse != null && rnse.length > 0) {
				fail(rnse[0].getMessage());
			}
			RNSDirHandle dir = client.list(path, false);
			assertNotNull(dir);
			int count = 0;
			ArrayList<RNSDirent> al = new ArrayList<RNSDirent>();
			for (RNSDirent rd : dir) {
				rd.setEpr(null);
				rd.setMeta(null);
				al.add(rd);
				count++;
			}
			assertNull(dir.getError());
			assertEquals("num == count(" + num + ")", num, count);
			ArrayList<String> list = new ArrayList<String>();
			for (RNSDirent rd : al) {
				String name = rd.getName();
				String child = RNSUtil.joinPath(path, name);
				assertTrue(client.exists(child));
				if (rd.isDirectory() == false) {
					list.add(name);
				}
			}
			RNSError[] es = client.removeBulk(path, list.toArray(new String[0]));
			assertNull(es);
			for (String name : list) {
				String child = RNSUtil.joinPath(path, name);
				assertFalse(client.exists(child));
			}
			client.rmdir(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_renameBulk() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/dir";
			client.mkdir(path);
			RNSAddHandle addHandle = new RNSAddHandle();
			int num = 100;
			EndpointReferenceType epr = new EndpointReferenceType();
			epr.setAddress(new Address("http://www.example.com/test"));
			String xmlStr = "<A>test</A>\n----\n<B><C/></B>\n----\n<D test=\"value\" />";
			MessageElement[] mes = RNSUtil.toMessageElements(xmlStr);
			for (int i = 0; i < num; i++) {
				addHandle.registerAddEPR(Integer.toString(i), epr, mes, false);
			}
			RNSError[] rnse = client.addBulk(path, addHandle);
			if (rnse != null && rnse.length > 0) {
				fail(rnse[0].getMessage());
			}
			Map<String, String> map = new HashMap<String, String>();
			for (int i = 0; i < num; i++) {
				map.put(Integer.toString(i), Integer.toString(i) + ".renamed");
			}
			rnse = client.renameBulk(path, map);
			if (rnse != null && rnse.length > 0) {
				fail(rnse[0].getMessage());
			}
			for (String name : map.keySet()) {
				String child = RNSUtil.joinPath(path, name);
				assertFalse(client.exists(child));
			}
			ArrayList<String> al = new ArrayList<String>();
			for (int i = 0; i < num; i++) {
				al.add(Integer.toString(i) + ".renamed");
			}
			RNSError[] es = client.removeBulk(path, al.toArray(new String[0]));
			assertNull(es);
			client.rmdir(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_recursiveFlatMany() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/dir";
			client.mkdir(path);
			int num = 100;
			for (int i = 0; i < num; i++) {
				client.mkdir(path + "/" + i);
			}
			RNSDirHandle dir = client.list(path, false);
			assertNotNull(dir);
			int count = 0;
			for (RNSDirent rd : dir) {
				RNSError error = rd.getRNSError();
				if (error != null) {
					error.printStackTrace();
					fail(error.getMessage());
				}
				count++;
			}
			assertNull(dir.getError());
			assertEquals(num, count);
			client.removeRecursive(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_getEPR_addRNSEPR() {
		RNSClient client = getClient();
		try {
			client.mkdir(TESTDIR + "/a");
			try {
				client.getEPR(TESTDIR + "/a", false);
				shouldFail("getEPR: EISDIR");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EISDIR, e);
			}
			EndpointReferenceType epr = client.getEPR(TESTDIR + "/a", true);
			client.addRNSEPR(TESTDIR + "/b", epr);
			client.mkdir(TESTDIR + "/b/dir");
			try {
				client.mkdir(TESTDIR + "/a/dir");
				shouldFail("mkdir: EEXIST");
			} catch (RNSError e) {
				checkRNSError(RNSError.Errno.EEXIST, e);
			}
			client.removeRecursive(TESTDIR + "/a");
			client.rmdir(TESTDIR + "/b");
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_metadata() {
		RNSClient client = getClient();
		try {
			String path = TESTDIR + "/a";
			int xmlnum = 3;
			String xmlStr = "<A>test</A>\n----\n<B><C/></B>\n----\n<D />";
			MessageElement[] any1 = RNSUtil.toMessageElements(xmlStr);
			client.mkdir(path, any1);
			MessageElement[] any2 = client.getMetadata(path);
			assertNotNull(any2);
			assertEquals(xmlnum, any2.length);
			String x1 = RNSUtil.toIndentedXML(any2[2]);
			client.setMetadata(path, any2);
			MessageElement[] any3 = client.getMetadata(path);
			assertNotNull(any3);
			assertEquals(xmlnum, any3.length);
			String x2 = RNSUtil.toIndentedXML(any3[2]);
			assertEquals(x1, x2);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_xquery() {
		RNSClient client = getClient();
		RNSSearchClient sClient = getRNSSearchClient();
		try {
			String dir = TESTDIR + "/d";
			String path = TESTDIR + "/j";
			String xml1 = "<A>test</A>";
			String xml2 = "<B>2000</B>";

			String[] xmls = new String[2];
			xmls[0] = xml1;
			xmls[1] = xml2;

			/* directory */
			MessageElement[] any1 = RNSUtil.toMessageElements(xmls);
			client.mkdir(dir, any1);
			String xqA = RNSUtil.generateXQueryForRNSSearch("$meta/A");
			RNSSearchResult resultA = sClient.search(dir, xqA);
			assertNotNull(resultA.getMetadataString());
			assertEquals("test", RNSUtil.localXQuery(
					resultA.getMetadataString(), "string(//A)"));

			String xqB = "declare namespace ns1 = \"http://schemas.ogf.org/rns/2009/12/rns\"; "
					+ "let $ent := /ns1:RNSEntryResponseType "
					+ "let $name := string($ent/@entry-name) "
					+ "let $epr := $ent/ns1:endpoint "
					+ "let $meta := $ent/ns1:metadata "
					+ "let $sptrns := $meta/ns1:supports-rns "
					+ "let $isdir := $sptrns/@value "
					+ "let $val := $meta/B/text() "
					+ "where xs:integer($val) > 1500 "
					+ "return "
					+ "<ns1:RNSEntryResponseType entry-name=\"{$name}\" "
					+ "xmlns:ns1=\"http://schemas.ogf.org/rns/2009/12/rns\" "
					+ "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
					+ "xsi:type=\"ns1:RNSEntryResponseType\">"
					+ "{$epr}"
					+ "<ns1:metadata xsi:type=\"ns1:RNSMetadataType\">"
					+ "{$sptrns}"
					+ "{$meta/B}"
					+ "</ns1:metadata></ns1:RNSEntryResponseType>";

			RNSSearchResult resultB = sClient.search(dir, xqB);
			assertNotNull(resultB.getMetadataString());
			String resultB_Str = RNSUtil.localXQuery(
					resultB.getMetadataString(), "string(//B)");
			assertNotNull(resultB_Str);
			assertEquals("2000", resultB_Str);

			String xqSupp = RNSUtil.generateXQueryForRNSSearch(null);
			RNSSearchResult resultSupp = sClient.search(dir, xqSupp);
			assertEquals(true,
					RNSUtil.isDirectory(resultSupp.getEntryResponseType()
							.getMetadata()));

			/* junction */
			EndpointReferenceType epr = RNSUtil.toEPR(new URI(
					"http://www.example.com/"));
			client.addJunction(path, epr, any1);
			resultSupp = sClient.search(path, xqSupp);
			assertEquals(false,
					RNSUtil.isDirectory(resultSupp.getEntryResponseType()
							.getMetadata()));

			/* bulk */
			String[] names = new String[2];
			names[0] = "d";
			names[1] = "j";
			RNSSearchResultHandle results = sClient.searchBulk(TESTDIR, names,
					xqSupp);
			assertNotNull(results);
			ArrayList<RNSSearchResult> al = new ArrayList<RNSSearchResult>();
			for (RNSSearchResult r : results) {
				al.add(r);
			}
			assertNull(results.getError());
			assertEquals(2, al.size());
			assertEquals(dir, al.get(0).getPath());
			assertEquals(path, al.get(1).getPath());
			assertEquals(
					true,
					RNSUtil.isDirectory(al.get(0)
							.getEntryResponseType()
							.getMetadata()));
			assertEquals(
					false,
					RNSUtil.isDirectory(al.get(1)
							.getEntryResponseType()
							.getMetadata()));

			/* list */
			RNSDirHandle enames = sClient.listBySearch(TESTDIR);
			assertNotNull(enames);
			ArrayList<RNSDirent> dents = new ArrayList<RNSDirent>();
			for (RNSDirent de : enames) {
				dents.add(de);
			}
			assertNull(enames.getError());
			assertEquals(2, dents.size());
			if (dents.get(0).getName().equals("d")) {
				if (!dents.get(1).getName().equals("j")) {
					fail("unexpected child's name: " + dents.get(1).getName());
				}
			} else if (dents.get(0).getName().equals("j")) {
				if (!dents.get(1).getName().equals("d")) {
					fail("unexpected child's name: " + dents.get(1).getName());
				}
			} else {
				fail("unexpected child's name: " + dents.get(0).getName());
			}

			/* recursive */
			int depth = 5;
			StringBuilder subdir = new StringBuilder(dir);
			for (int i = 0; i < depth; i++) {
				subdir.append("/dir");
				client.mkdir(subdir.toString(), any1);
			}
			results = sClient.searchRecursive(dir, xqSupp, depth);
			assertNotNull(results);
			al = new ArrayList<RNSSearchResult>();
			for (RNSSearchResult r : results) {
				al.add(r);
			}
			assertEquals(5, al.size());
			for (RNSSearchResult r : results) {
				assertEquals(true, RNSUtil.isDirectory(r.getEntryResponseType()
						.getMetadata()));
			}

			client.removeRecursive(dir);
			client.rmJunction(path);
		} catch (RNSError e) {
			e.printStackTrace();
			fail(e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}

	public void test_KeyValue() {
		int testlen = 30;
		String KEY = "key<>&'\"";
		String VALUE = "value<>&'\"";
		RNSClientHome home = getClientHome();
		RNSClient client = getClient();
		try {
			String dir = TESTDIR + "/d";
			int xmlNum = 3;

			String xmlStr = "<A>test</A>\n----\n<B><C/></B>\n----\n<D test=\"value\" />";
			MessageElement[] mes = RNSUtil.toMessageElements(xmlStr);
			client.mkdir(dir, mes);

			RNSKeyValue kv = new RNSKeyValue(home, dir);

			assertEquals(kv.size(), 0); /* test keySet() */
			for (int i = 0; i < testlen; i++) {
				kv.put(KEY + i, VALUE + i);
			}
			assertEquals(kv.size(), testlen); /* test keySet() */
			for (int i = 0; i < testlen; i++) {
				assertTrue(kv.containsKey(KEY + i));
			}
			for (int i = 0; i < testlen; i++) {
				String val = kv.get(KEY + i);
				assertNotNull(val);
				assertEquals(val, VALUE + i);
			}
			for (int i = 0; i < testlen; i++) {
				kv.remove(KEY + i);
			}
			for (int i = 0; i < testlen; i++) {
				assertFalse(kv.containsKey(KEY + i));
				String val = kv.get(KEY + i);
				assertNull(val);
			}

			Map<String, String> map = new HashMap<String, String>();
			for (int i = 0; i < testlen; i++) {
				map.put(KEY + i, VALUE + i);
			}
			kv.putAll(map);
			assertEquals(kv.size(), testlen); /* test keySet() */
			for (int i = 0; i < testlen; i++) {
				assertTrue(kv.containsKey(KEY + i));
			}
			for (int i = 0; i < testlen; i++) {
				String val = kv.get(KEY + i);
				assertNotNull(val);
				assertEquals(val, VALUE + i);
			}
			for (int i = 0; i < testlen; i++) {
				kv.remove(KEY + i);
			}
			for (int i = 0; i < testlen; i++) {
				assertFalse(kv.containsKey(KEY + i));
				String val = kv.get(KEY + i);
				assertNull(val);
			}

			MessageElement[] mes2 = client.getMetadata(dir);
			assertEquals(xmlNum, mes2.length);

			client.rmdir(dir);
		} catch (Exception e) {
			e.printStackTrace();
			fail(e.getMessage());
		}
	}
}
