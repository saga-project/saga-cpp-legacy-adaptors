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

import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.Map;

import org.apache.axis.message.MessageElement;
import org.apache.commons.collections.map.ReferenceMap;
import org.naregi.rns.util.RNSUtil;

/**
 * A caching system for the implementations of RNSDB.
 */
public class RNSDBCache implements RNSDB {
	private RNSDB db;
	private boolean forceCachedList;

	public RNSDBCache(RNSDB db, boolean forceCachedList) {
		this.db = db;
		this.forceCachedList = forceCachedList;
	}

	@Override
	public void createAndSetNewRootID() throws Exception {
		db.createAndSetNewRootID();
	}

	@Override
	public void createAndSetNewID() throws Exception {
		db.createAndSetNewID();
	}

	private String id = null;

	@Override
	public boolean setID(String id) throws Exception {
		this.id = id;
		return db.setID(id);
	}

	@Override
	public String getID() {
		if (id != null) {
			return id;
		}
		id = db.getID();
		return id;
	}

	private String rootId;

	@Override
	public String getRootID() {
		rootId = db.getRootID();
		return rootId;
	}

	@Override
	public Object getLockAndStartTransaction() {
		return db.getLockAndStartTransaction();
	}

	@Override
	public void commit() throws Exception {
		db.commit();
	}

	@Override
	public void rollback() {
		db.rollback();
	}

	private RNSDirectoryProperties dirProp = null;
	private Object dirPropLock = new Object();

	@Override
	public RNSDirectoryProperties getDirectoryProperties() throws Exception {
		synchronized (dirPropLock) {
			if (dirProp != null) {
				return dirProp;
			}
			dirProp = db.getDirectoryProperties();
			return dirProp;
		}
	}

	@Override
	public void setDirectoryProperties(RNSDirectoryProperties props)
			throws Exception {
		db.setDirectoryProperties(props);
		synchronized (dirPropLock) {
			dirProp = props;
		}
	}

	@Override
	public void setCreateTime(Calendar t) throws Exception {
		db.setCreateTime(t);
		synchronized (dirPropLock) {
			dirProp = null;
		}
	}

	@Override
	public void setAccessTime(Calendar t) throws Exception {
		db.setAccessTime(t);
		synchronized (dirPropLock) {
			dirProp = null;
		}
	}

	@Override
	public void setModificationTime(Calendar t) throws Exception {
		db.setModificationTime(t);
		synchronized (dirPropLock) {
			dirProp = null;
		}
	}

	private List<String> list = null;
	private Object listLock = new Object();

	@Override
	public List<String> getList() throws Exception {
		synchronized (listLock) {
			if (list != null) {
				return list;
			}
			List<String> tmp = db.getList();
			if (forceCachedList) {
				list = tmp;
			} else {
				if (RNSUtil.checkMemory(false) == false) {
					list = null;
				} else {
					list = tmp;
				}
			}
			return tmp;
		}
	}

	@Override
	public long getListSize() throws Exception {
		synchronized (listLock) {
			if (list != null) {
				return list.size();
			}
		}
		return db.getListSize();
	}

	@SuppressWarnings({ "unchecked" })
	private Map<String, RNSEntryData> entMap = Collections.synchronizedMap((Map<String, RNSEntryData>) new ReferenceMap(
			ReferenceMap.SOFT, ReferenceMap.SOFT, true));

	@Override
	public RNSEntryData getEntryData(String name) throws Exception {
		synchronized (entMap) {
			RNSEntryData ent = entMap.get(name);
			if (ent != null) {
				return ent;
			}
			ent = db.getEntryData(name);
			if (ent != null) {
				entMap.put(name, ent);
			}
			return ent;
		}
	}

	@Override
	public void insertEntryData(String name, RNSEntryData ent) throws Exception {
		db.insertEntryData(name, ent);
		synchronized (listLock) {
			list = null;
		}
		synchronized (entMap) {
			entMap.put(name, ent);
		}
	}

	@Override
	public void rename(String from, String to) throws Exception {
		db.rename(from, to);
		synchronized (listLock) {
			list = null;
		}
		synchronized (entMap) {
			entMap.remove(from);
		}
	}

	@Override
	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception {
		db.replaceMetadata(name, xmls);
		synchronized (entMap) {
			entMap.remove(name);
		}
	}

	@Override
	public void removeEntryData(String name) throws Exception {
		db.removeEntryData(name);
		synchronized (listLock) {
			list = null;
		}
		synchronized (entMap) {
			entMap.remove(name);
		}
	}

	@Override
	public void destroy() throws Exception {
		db.destroy();
	}

	private ACL acl = null;

	@Override
	public ACL getACL() throws Exception {
		if (acl == null) {
			acl = db.getACL();
		}
		return new ACL(acl.toACLEntries()); /* copy */
	}

	@Override
	public void setACL(ACL acl) throws Exception {
		db.setACL(acl);
		this.acl = null;
	}

	@Override
	public void removeACL(short type, String[] names) throws Exception {
		db.removeACL(type, names);
		acl = null;
	}
}
