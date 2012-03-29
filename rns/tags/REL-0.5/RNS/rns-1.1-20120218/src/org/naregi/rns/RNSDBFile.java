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
package org.naregi.rns;

import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.axis.message.MessageElement;
import org.apache.commons.logging.Log;
import org.naregi.rns.stubs.ACLEntryType;
import org.naregi.rns.util.RNSUtil;

/**
 * An implementation to store into local files (using ObjectOutput).
 */
public class RNSDBFile implements RNSDB {
	private static final String DATA_SUFFIX = ".rns";
	private static final int MAX_FILENUM = 30000; // 30000dirs * 30000files
	private static final String FILE_SEP = System.getProperty("file.separator");

	private static final String DUMMY = "__dummy__";

	/* children (on memory) */
	protected RNSDirectoryProperties rnsProps = null;
	protected Map<String, RNSEntryData> list;
	protected ACLEntryType[] aclents;
	protected String id = null;
	private Log logger;

	private static File storageDirBase;
	private static File counterFile;

	private int max_dir = MAX_FILENUM;
	private int max_file = MAX_FILENUM;
	private String data_suffix = DATA_SUFFIX;

	private Exception unexpected = new Exception("unexpected condition");

	/* constructor */
	public RNSDBFile() {
		this("file");
	}

	protected RNSDBFile(String name) {
		logger = RNSLog.getLog();
		initializeStorage(name);
	}

	protected void setMaxDir(int max) {
		max_dir = max;
	}

	protected void setMaxFile(int max) {
		max_file = max;
	}

	protected void setDataSuffix(String suffix) {
		data_suffix = suffix;
	}

	private static boolean initializedStorage = false;

	private static void initializeStorage(String name) {
		if (initializedStorage) { // recycle
			return;
		}
		String basePath = RNSConfig.getStorageDir() + FILE_SEP + name;
		File f = new File(basePath);
		if (!f.exists()) {
			if (f.mkdirs() == false) {
				RNSLog.getLog().error(
						basePath + " could not be created for storage.");
				return;
			}
		}
		storageDirBase = f;
		counterFile = new File(storageDirBase, "counter");
		initializedStorage = true;
	}

	protected static File getStorageDirBase() {
		return storageDirBase;
	}

	private static String cacheCounter = null;

	private String getCounterFromFile() {
		if (cacheCounter != null) {
			return cacheCounter;
		}
		if (!counterFile.exists()) {
			return null;
		}
		FileInputStream fis = null;
		String str = null;
		try {
			fis = new FileInputStream(counterFile);
			// max length: "30000_30000" = 11
			byte[] b = new byte[12];
			int l = fis.read(b);
			str = new String(b, 0, l);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			logger.error(e.getMessage());
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (fis != null) {
				try {
					fis.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		cacheCounter = str;
		return str;
	}

	private void setCounterToFile(String id) {
		FileOutputStream fos = null;
		try {
			fos = new FileOutputStream(counterFile);
			fos.write(id.getBytes());
			cacheCounter = id;
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (fos != null) {
				try {
					fos.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	private static String getCounterDirID(String id) {
		String[] s = id.split("_|\r|\n|\r\n");
		if (s == null || s.length == 0) {
			return null;
		}
		return s[0];
	}

	private static String getCounterFileID(String id) {
		String[] s = id.split("_|\r|\n|\r\n");
		if (s == null || s.length == 0) {
			return "NoEntryDummyName";
		}
		return s[1];
	}

	protected File getIdAsFile(String id, String suffix) {
		String dirID = getCounterDirID(id);
		if (dirID == null) {
			logger.debug("cannot create data directory: id=" + id);
			return null;
		}
		File dir = new File(storageDirBase, dirID);
		if (!dir.exists()) {
			if (dir.mkdir() == false) {
				logger.debug("cannot create: " + dir.getAbsolutePath());
				return null;
			}
		}
		return new File(dir, getCounterFileID(id) + suffix);
	}

	public String getID() {
		return id;
	}

	private final static String ROOTID = "0_0";

	public String getRootID() {
		return ROOTID;
	}

	public boolean setID(String id) throws Exception {
		File f = getIdAsFile(id, data_suffix);
		if (f == null) { // directory
			return false;
		}
		this.id = id;
		if (f.exists()) {
			load();
			return true;
		}
		return false;
	}

	public void createAndSetNewRootID() throws Exception {
		id = ROOTID;
		File f = getIdAsFile(id, data_suffix);
		if (f == null) {
			throw new Exception("cannot create root directory");
		}
		try {
			f.createNewFile();
		} catch (IOException e) {
			e.printStackTrace();
			logger.error("cannot create: " + f.getAbsolutePath());
		}
	}

	public void createAndSetNewID() throws Exception {
		File f;
		int dirID = 0;
		int fileID = 0;
		String newid = getCounterFromFile();
		if (newid != null) {
			try {
				dirID = Integer.parseInt(getCounterDirID(newid));
				fileID = Integer.parseInt(getCounterFileID(newid));
			} catch (NumberFormatException e) {
				/* through */
			}
			if (dirID < 0) {
				dirID = 0;
			}
			if (fileID < 0) {
				fileID = 0;
			}
		}
		boolean loop2 = false;
		do {
			fileID++;
			if (fileID >= max_file) {
				dirID++;
				fileID = 0;
			}
			if (dirID >= max_dir) {
				if (loop2) {
					return;
				}
				dirID = 0;
				loop2 = true;
			}
			newid = dirID + "_" + fileID;
			f = getIdAsFile(newid, data_suffix);
			if (f == null) {
				// error
				throw new Exception("invalid id=" + newid);
			}
		} while (f.exists());
		setCounterToFile(newid);
		id = newid;
		try {
			f.createNewFile();
		} catch (IOException e) {
			e.printStackTrace();
			logger.error("cannot create: " + f.getAbsolutePath());
			throw e;
		}
		return;
	}

	protected void load() throws Exception {
		if (id == null) {
			return;
		}
		File file = getIdAsFile(id, data_suffix);
		if (file == null || !file.exists()) {
			throw new Exception("unexpected condition:not exist:id=" + id);
		}
		if (file.length() == 0) {
			return; // need setRNSDirProps(not null)
		}
		ObjectInputStream ois = null;
		RNSDirectoryProperties newRnsProps = new RNSDirectoryProperties();
		try {
			ois = new ObjectInputStream(new FileInputStream(file));
			/* 1 */
			newRnsProps.setCreateTime((Calendar) ois.readObject());
			/* 2 */
			newRnsProps.setAccessTime((Calendar) ois.readObject());
			/* 3 */
			newRnsProps.setModificationTime((Calendar) ois.readObject());
			/* 4 */
			int size = ois.readInt();
			/* create new list */
			Map<String, RNSEntryData> newlist = Collections.synchronizedMap(new HashMap<String, RNSEntryData>());
			for (int i = 0; i < size; i++) {
				/* 5 */
				String keyname = ois.readUTF();
				RNSEntryData ent = new RNSEntryData();
				/* 6 */
				ent.setDirectory(ois.readBoolean());
				/* 7 */
				ent.setLocalID(ois.readUTF());
				if ("".equals(ent.getLocalID())) {
					ent.setLocalID(null);
				}
				/* 8 */
				String eprStr = ois.readUTF();
				if (eprStr != null && eprStr.length() != 0) {
					ent.setEpr(RNSUtil.toEPR(eprStr));
				}
				/* 9 */
				int metalen = ois.readInt();
				if (metalen > 0) {
					MessageElement[] me = new MessageElement[metalen];
					for (int n = 0; n < metalen; n++) {
						/* 10 */
						String xml = ois.readUTF();
						me[n] = RNSUtil.toMessageElement(xml);
					}
					ent.setAny(me);
				}
				newlist.put(keyname, ent);
			}
			ACLEntryType[] newaclents;
			/* ACL */
			int acllen;
			try {
				acllen = ois.readInt(); /* 11 */
			} catch (EOFException eof) {
				acllen = 0;
			}
			if (acllen > 0) {
				newaclents = new ACLEntryType[acllen];
				for (int n = 0; n < acllen; n++) { /* 12 */
					newaclents[n] = new ACLEntryType();
					newaclents[n].setType(ois.readShort());
					String aclname = ois.readUTF();
					if (aclname.equals(DUMMY)) {
						aclname = null;
					}
					newaclents[n].setName(aclname);
					newaclents[n].setPerm(ois.readShort());
				}
			} else {
				newaclents = null;
			}

			rnsProps = newRnsProps;
			list = newlist;
			aclents = newaclents;
			logger.debug("load id=" + id);
		} catch (IOException e) {
			throw new Exception("Failed to load resource: id=" + id, e);
		} finally {
			if (ois != null) {
				try {
					ois.close();
				} catch (Exception e2) {
					logger.warn("cannot close: resource id=" + id + ": "
							+ e2.getMessage());
				}
			}
		}
	}

	protected void store() throws Exception {
		ObjectOutputStream oos = null;
		File tmpFile = null;
		try {
			tmpFile = File.createTempFile("RNS", ".tmp", storageDirBase);
			tmpFile.deleteOnExit();
			oos = new ObjectOutputStream(new FileOutputStream(tmpFile));
			/* 1 */
			oos.writeObject(rnsProps.getCreateTime());
			/* 2 */
			oos.writeObject(rnsProps.getAccessTime());
			/* 3 */
			oos.writeObject(rnsProps.getModificationTime());
			if (list == null || list.size() == 0) {
				/* 4 */
				oos.writeInt(0); // list size
			} else {
				/* 4 */
				oos.writeInt(list.size());
				Iterator<String> i = list.keySet().iterator();
				while (i.hasNext()) {
					String name = i.next();
					/* 5 */
					oos.writeUTF(name);
					RNSEntryData ent = list.get(name);
					/* 6 */
					oos.writeBoolean(ent.isDirectory());
					/* 7 */
					if (ent.getLocalID() == null) {
						oos.writeUTF("");
					} else {
						oos.writeUTF(ent.getLocalID());
					}
					/* 8 */
					String eprStr;
					if (ent.getEpr() == null) {
						eprStr = "";
					} else {
						eprStr = RNSUtil.toXMLString(ent.getEpr());
					}
					oos.writeUTF(eprStr);
					if (ent.getRNSMetadata() != null
							&& ent.getRNSMetadata().get_any() != null) {
						MessageElement[] me = ent.getRNSMetadata().get_any();
						/* 9 */
						oos.writeInt(me.length);
						for (int n = 0; n < me.length; n++) {
							String xml = me[n].getAsString();
							/* 10 */
							oos.writeUTF(xml);
							// System.out.println(getID() + "/" + name +
							// ": store XML:  " + xml);
						}
					} else {
						oos.writeInt(0); /* 9 */
						/* 10: nothing */
					}
				}
			}
			/* ACL */
			if (aclents == null || aclents.length == 0) {
				oos.writeInt(0); /* 11 */
				/* 12: nothing */
			} else {
				oos.writeInt(aclents.length);
				for (ACLEntryType aet : aclents) { /* 12 */
					oos.writeShort(aet.getType());
					String aclname = aet.getName();
					if (aclname == null) {
						aclname = DUMMY;
					}
					oos.writeUTF(aclname);
					oos.writeShort(aet.getPerm());
				}
			}
			logger.debug("store id=" + id);
		} catch (Exception e) {
			if (tmpFile != null) {
				if (tmpFile.delete() == false) {
					logger.error("cannot delete: " + tmpFile.getAbsolutePath());
				}
			}
			logger.error("cannot store resource: id=" + id);
			throw e;
		} finally {
			if (oos != null) {
				try {
					oos.close();
				} catch (Exception e2) {
					logger.warn("cannot write-close: "
							+ tmpFile.getAbsolutePath() + ": "
							+ e2.getMessage());
				}
			}
		}
		File file = getIdAsFile(id, data_suffix);
		if (file == null) {
			logger.error("directory is not exist: id=" + id);
			throw new Exception("Failed to store resource");
		}
		if (file.exists()) {
			if (file.delete() == false) {
				logger.error("cannot delete: " + file.getAbsolutePath());
			}
		}
		if (!tmpFile.renameTo(file)) {
			if (tmpFile.delete() == false) {
				logger.error("cannot rename: " + tmpFile.getAbsolutePath());
			}
			throw new Exception("Failed to store resource");
		}
	}

	public Object getLockAndStartTransaction() {
		return this;
	}

	public void commit() throws Exception {
		store();
	}

	public void rollback() {
		try {
			load();
		} catch (Exception e) {
			e.printStackTrace();
			logger.warn("rollback error: " + e.getMessage());
		}
	}

	public RNSDirectoryProperties getDirectoryProperties() {
		return rnsProps;
	}

	public void setDirectoryProperties(RNSDirectoryProperties props) {
		rnsProps = props;
	}

	public void setAccessTime(Calendar t) {
		rnsProps.setAccessTime(t);
	}

	public void setCreateTime(Calendar t) {
		rnsProps.setCreateTime(t);
	}

	public void setModificationTime(Calendar t) {
		rnsProps.setModificationTime(t);
	}

	public void destroy() throws Exception {
		if (id == null) {
			return;
		}
		File file = getIdAsFile(id, data_suffix);
		if (file != null && file.exists()) {
			if (file.delete() == false) {
				logger.error("DESTROY:cannot delete:" + file.getAbsolutePath());
			}
		}
	}

	public long getListSize() {
		if (list == null) {
			return 0;
		}
		return list.size();
	}

	public List<String> getList() {
		if (this.list == null) {
			this.list = new HashMap<String, RNSEntryData>();
		}
		return new ArrayList<String>(this.list.keySet());
	}

	public RNSEntryData getEntryData(String name) {
		if (list == null) {
			return null;
		}
		return this.list.get(name);
	}

	public void insertEntryData(String name, RNSEntryData ent) {
		if (this.list == null) {
			this.list = new HashMap<String, RNSEntryData>();
		}
		this.list.put(name, ent);
	}

	public void removeEntryData(String name) throws Exception {
		if (list == null) {
			throw unexpected;
		}
		list.remove(name);
	}

	public void rename(String from, String to) throws Exception {
		if (list == null) {
			throw unexpected;
		}
		RNSEntryData ent = list.remove(from);
		if (ent == null) {
			throw unexpected;
		}
		list.put(to, ent);
	}

	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception {
		if (list == null) {
			throw unexpected;
		}
		RNSEntryData oldent = list.get(name);
		if (oldent == null) {
			throw unexpected;
		}
		oldent.setAny(xmls);
		list.put(name, oldent);
	}

	public ACL getACL() throws Exception {
		if (aclents == null) {
			return new ACL();
		}
		return new ACL(aclents); /* copy */
	}

	public void setACL(ACL acl) throws Exception {
		aclents = acl.toACLEntries();
	}

	public void removeACL(short type, String[] names) throws Exception {
		if (aclents == null) {
			return;
		}
		ArrayList<ACLEntryType> al = new ArrayList<ACLEntryType>();
		for (ACLEntryType ent : aclents) {
			if (ent == null) {
				continue;
			} else if (ent.getType() == type) {
				if (names == null) {
					/* remove all of this type */
				} else {
					boolean equal = false;
					for (String name : names) {
						if (name == null) {
							continue;
						} else if (name.equals(ent.getName())) {
							equal = true;
							break;
						}
					}
					if (equal == false) {
						al.add(ent);
					}
				}
			} else {
				al.add(ent);
			}
		}
		aclents = al.toArray(new ACLEntryType[0]);
	}
}
