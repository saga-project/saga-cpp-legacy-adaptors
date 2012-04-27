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

import java.beans.XMLDecoder;
import java.beans.XMLEncoder;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;

import javax.xml.parsers.ParserConfigurationException;

import org.apache.axis.message.MessageElement;
import org.apache.commons.logging.Log;
import org.globus.wsrf.NoSuchResourceException;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.encoding.DeserializationException;
import org.naregi.rns.stubs.ACLEntryType;
import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.util.RNSUtil;
import org.xml.sax.SAXException;

/**
 * An implementation to store into local files as XML (using XMLEncoder).
 */
public class RNSDBXML extends RNSDBFile {
	private static String SUFFIX_PROP = ".prop.xml";
	private static String SUFFIX_LIST = ".list.xml";
	private static String SUFFIX_ACL = ".acl.xml";

	private boolean updateProp;
	private boolean updateList;
	private boolean updateACL;

	private Log logger;

	public RNSDBXML() {
		super("xml");
		logger = RNSLog.getLog();
		setMaxDir(30000);
		setMaxFile(15000);
		setDataSuffix(SUFFIX_PROP); // check file existence
		updateProp = true;
		updateList = true;
		updateACL = true;
	}

	private static class DirList {
		private Map<String, EntryData> map;

		public Map<String, EntryData> getMap() {
			return map;
		}

		public void setMap(Map<String, EntryData> map) {
			this.map = map;
		}
	}

	private static class EntryData {
		boolean directory;
		String localID;
		String epr;
		String[] metadata;
	}

	private RNSEntryData decodeEntryData(EntryData in)
			throws DeserializationException, IOException,
			ParserConfigurationException, SAXException {
		RNSEntryData out = new RNSEntryData();
		out.setDirectory(in.directory);
		out.setLocalID(in.localID);
		if (in.epr != null) {
			out.setEpr(RNSUtil.toEPR(in.epr));
		}
		if (in.metadata != null) {
			int len = in.metadata.length;
			MessageElement[] mes = new MessageElement[len];
			for (int i = 0; i < len; i++) {
				mes[i] = RNSUtil.toMessageElement(in.metadata[i]);
			}
			out.setAny(mes);
		}
		return out;
	}

	private EntryData encodeEntryData(RNSEntryData in) throws Exception {
		EntryData out = new EntryData();
		out.directory = in.isDirectory();
		out.localID = in.getLocalID();
		if (in.getEpr() == null) {
			out.epr = null;
		} else {
			out.epr = RNSUtil.toXMLString(in.getEpr());
		}
		RNSMetadataType meta = in.getRNSMetadata();
		if (meta != null) {
			MessageElement[] mes = meta.get_any();
			if (mes != null) {
				out.metadata = new String[mes.length];
				for (int i = 0; i < mes.length; i++) {
					out.metadata[i] = mes[i].getAsString();
				}
			}
		}
		return out;
	}

	private static class ACLData {
		private short type;
		private String name;
		private short perm;

		public void setType(short type) {
			this.type = type;
		}

		public short getType() {
			return type;
		}

		public void setName(String name) {
			this.name = name;
		}

		public String getName() {
			return name;
		}

		public void setPerm(short perm) {
			this.perm = perm;
		}

		public short getPerm() {
			return perm;
		}
	}

	protected void load() throws ResourceException {
		if (id == null) {
			return;
		}
		File propFile = getIdAsFile(id, SUFFIX_PROP);
		if (propFile == null || !propFile.exists()) {
			throw new NoSuchResourceException("id=" + id);
		}
		RNSDirectoryProperties newRnsProps = (RNSDirectoryProperties) loadXML(propFile);

		Map<String, RNSEntryData> newlist = Collections.synchronizedMap(new HashMap<String, RNSEntryData>());
		File listFile = getIdAsFile(id, SUFFIX_LIST);
		if (listFile != null && listFile.exists()) {
			DirList dl = (DirList) loadXML(listFile);
			Map<String, EntryData> map = dl.getMap();
			if (map != null) {
				Set<Entry<String, EntryData>> ents = map.entrySet();
				for (Entry<String, EntryData> ent : ents) {
					String key = ent.getKey();
					try {
						newlist.put(key, decodeEntryData(ent.getValue()));
					} catch (Exception e) {
						throw new ResourceException(
								"Failed to load resource: id=" + id, e);
					}
				}
			}
		}
		File aclFile = getIdAsFile(id, SUFFIX_ACL);
		if (aclFile != null && aclFile.exists()) {
			ACLData[] acldatas = (ACLData[]) loadXML(aclFile);
			ACLEntryType[] acls = new ACLEntryType[acldatas.length];
			for (int i = 0; i < acls.length; i++) {
				acls[i] = new ACLEntryType();
				acls[i].setType(acldatas[i].getType());
				acls[i].setName(acldatas[i].getName());
				acls[i].setPerm(acldatas[i].getPerm());
			}
			aclents = acls;
		}
		rnsProps = newRnsProps;
		list = newlist;

		updateProp = false;
		updateList = false;
		updateACL = false;
	}

	private Object loadXML(File file) throws ResourceException {
		XMLDecoder dec = null;
		BufferedInputStream bis = null;
		try {
			bis = new BufferedInputStream(new FileInputStream(file));
			dec = new XMLDecoder(bis);
			return dec.readObject();
		} catch (Exception e) {
			throw new ResourceException("Failed to load resource: file="
					+ file.getAbsolutePath(), e);
		} finally {
			if (dec != null) {
				dec.close();
				bis = null;
			}
			if (bis != null) {
				try {
					bis.close();
				} catch (IOException e2) {
					logger.warn("cannot close: resource id=" + id + ": "
							+ e2.getMessage());
				}
			}
		}
	}

	protected void store() throws ResourceException {
		if (updateProp) {
			File propFile = getIdAsFile(id, SUFFIX_PROP);
			if (propFile == null) {
				throw new NoSuchResourceException("id=" + id);
			}
			storeXML(propFile, rnsProps);
			updateProp = false;
		}
		if (updateList) {
			updateList = false;
			if (list == null || list.size() == 0) {
				File file = getIdAsFile(id, SUFFIX_LIST);
				if (file != null && file.exists()) {
					if (file.delete() == false) {
						logger.error("list:cannot delete:"
								+ file.getAbsolutePath());
					}
				}
			} else {
				DirList dl = new DirList();
				Map<String, EntryData> map = new HashMap<String, EntryData>();
				Set<String> keys = list.keySet();
				for (String key : keys) {
					try {
						map.put(key, encodeEntryData(list.get(key)));
					} catch (Exception e) {
						throw new ResourceException("Failed to store resource",
								e);
					}
				}
				dl.setMap(map);
				File listFile = getIdAsFile(id, SUFFIX_LIST);
				if (listFile == null) {
					throw new NoSuchResourceException(
							"invalid DB directory: id=" + id);
				}
				storeXML(listFile, dl);
			}
		}
		if (updateACL) {
			File aclFile = getIdAsFile(id, SUFFIX_ACL);
			if (aclFile == null) {
				throw new NoSuchResourceException("id=" + id);
			}
			if (aclents != null) {
				ACLData[] acldatas = new ACLData[aclents.length];
				for (int i = 0; i < aclents.length; i++) {
					ACLData ad = new ACLData();
					ad.setType(aclents[i].getType());
					ad.setName(aclents[i].getName());
					ad.setPerm(aclents[i].getPerm());
					acldatas[i] = ad;
				}
				storeXML(aclFile, acldatas);

			}
			updateACL = false;
		}
	}

	private void storeXML(File file, Object obj) throws ResourceException {
		XMLEncoder enc = null;
		BufferedOutputStream bos = null;
		File tmpFile = null;
		try {
			tmpFile = File.createTempFile("RNS", ".tmp", getStorageDirBase());
			tmpFile.deleteOnExit();
			bos = new BufferedOutputStream(new FileOutputStream(tmpFile));
			enc = new XMLEncoder(bos);
			enc.writeObject(obj);
		} catch (Exception e) {
			if (tmpFile != null) {
				if (tmpFile.delete() == false) {
					logger.error("cannot delete: " + tmpFile.getAbsolutePath());
				}
			}
			logger.error("cannot store resource: id=" + id);
			throw new ResourceException("Failed to store resource", e);
		} finally {
			if (enc != null) {
				enc.close();
				bos = null;
			}
			if (bos != null) {
				try {
					bos.close();
				} catch (IOException e2) {
					logger.warn("cannot write-close: "
							+ tmpFile.getAbsolutePath() + ": "
							+ e2.getMessage());
				}
			}
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
			throw new ResourceException("Failed to store resource");
		}
	}

	public void setDirectoryProperties(RNSDirectoryProperties props) {
		super.setDirectoryProperties(props);
		updateProp = true;
	}

	public void setAccessTime(Calendar t) {
		super.setAccessTime(t);
		updateProp = true;
	}

	public void setCreateTime(Calendar t) {
		super.setCreateTime(t);
		updateProp = true;
	}

	public void setModificationTime(Calendar t) {
		super.setModificationTime(t);
		updateProp = true;
	}

	public void insertEntryData(String name, RNSEntryData ent) {
		super.insertEntryData(name, ent);
		updateList = true;
	}

	public void removeEntryData(String name) throws Exception {
		updateList = true;
		super.removeEntryData(name);
	}

	public void rename(String from, String to) throws Exception {
		updateList = true;
		super.rename(from, to);
	}

	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception {
		updateList = true;
		super.replaceMetadata(name, xmls);
	}

	public void destroy() throws Exception {
		File listFile = getIdAsFile(id, SUFFIX_LIST);
		if (listFile != null && listFile.exists()) {
			if (listFile.delete() == false) {
				logger.error("DESTROY:cannot delete:"
						+ listFile.getAbsolutePath());
			}
		}
		File aclFile = getIdAsFile(id, SUFFIX_ACL);
		if (aclFile != null && aclFile.exists()) {
			if (aclFile.delete() == false) {
				logger.error("DESTROY:cannot delete:"
						+ aclFile.getAbsolutePath());
			}
		}
		super.destroy();
	}

	public void setACL(ACL acl) throws Exception {
		updateACL = true;
		super.setACL(acl);
	}

	public void removeACL(short type, String[] names) throws Exception {
		updateACL = true;
		super.removeACL(type, names);
	}
}
