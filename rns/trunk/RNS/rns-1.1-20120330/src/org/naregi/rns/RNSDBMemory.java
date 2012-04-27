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
package org.naregi.rns;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.axis.message.MessageElement;
import org.naregi.rns.stubs.ACLEntryType;

/**
 * An example implementation of using RNSDB interface.
 */
public class RNSDBMemory implements RNSDB {

	private static final int ROOTID = 0;
	private static int count = ROOTID;

	private static class MemEnt {
		RNSDirectoryProperties props = new RNSDirectoryProperties();
		Map<String, RNSEntryData> listmap = new HashMap<String, RNSEntryData>();
		ACL acl;
	}

	private static Object globalLock = new Object();

	private static Map<Integer, MemEnt> map = new HashMap<Integer, RNSDBMemory.MemEnt>();

	private int id;
	private MemEnt mement;

	@Override
	public void createAndSetNewRootID() throws Exception {
		id = ROOTID;
		MemEnt me = map.get(id);
		if (me == null) {
			me = new MemEnt();
			ACL acl = new ACL();
			acl.setOwner("admin", ACL.PERM_ALL);
			acl.setOwnerGroup("admin", ACL.PERM_READ);
			acl.setOther(ACL.PERM_READ);
			me.acl = acl;
			map.put(id, me);
		}
		mement = me;
	}

	@Override
	public void createAndSetNewID() throws Exception {
		count++;
		id = count;
		MemEnt me = new MemEnt();
		map.put(id, me);
		mement = me;
	}

	@Override
	public boolean setID(String id) throws Exception {
		int i = Integer.parseInt(id);
		MemEnt me = map.get(i);
		if (me != null) {
			this.id = i;
			this.mement = me;
			return true;
		} else {
			return false;
		}
	}

	@Override
	public String getID() {
		return String.valueOf(id);
	}

	@Override
	public String getRootID() {
		return String.valueOf(ROOTID);
	}

	@Override
	public Object getLockAndStartTransaction() {
		return globalLock;
	}

	@Override
	public void commit() throws Exception {
		/* not support */
	}

	@Override
	public void rollback() {
		/* not support */
	}

	@Override
	public RNSDirectoryProperties getDirectoryProperties() {
		return mement.props;
	}

	@Override
	public void setDirectoryProperties(RNSDirectoryProperties props)
			throws Exception {
		mement.props = props;
	}

	@Override
	public void setCreateTime(Calendar t) throws Exception {
		mement.props.setCreateTime(t);
	}

	@Override
	public void setAccessTime(Calendar t) throws Exception {
		mement.props.setAccessTime(t);
	}

	@Override
	public void setModificationTime(Calendar t) throws Exception {
		mement.props.setModificationTime(t);
	}

	@Override
	public List<String> getList() throws Exception {
		return new ArrayList<String>(mement.listmap.keySet());
	}

	@Override
	public long getListSize() throws Exception {
		return mement.listmap.size();
	}

	@Override
	public RNSEntryData getEntryData(String name) {
		return mement.listmap.get(name);
	}

	@Override
	public void insertEntryData(String name, RNSEntryData ent) throws Exception {
		mement.listmap.put(name, ent);
	}

	@Override
	public void rename(String from, String to) throws Exception {
		RNSEntryData ent1 = mement.listmap.get(from);
		if (ent1 == null) {
			throw new Exception("rename failed: no from_name");
		}
		RNSEntryData ent2 = mement.listmap.get(to);
		if (ent2 != null) {
			throw new Exception("rename failed: to_name exists");
		}
		mement.listmap.remove(from);
		mement.listmap.put(to, ent1);
	}

	@Override
	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception {
		RNSEntryData ent = mement.listmap.get(name);
		if (ent == null) {
			throw new Exception("replaceMetadata failed: not exists");
		}
		ent.setAny(xmls);
	}

	@Override
	public void removeEntryData(String name) throws Exception {
		mement.listmap.remove(name);
	}

	@Override
	public void destroy() throws Exception {
		map.remove(id);
	}

	@Override
	public ACL getACL() throws Exception {
		return mement.acl;
	}

	@Override
	public void setACL(ACL acl) throws Exception {
		mement.acl = acl;
	}

	@Override
	public void removeACL(short type, String[] names) throws Exception {
		ACLEntryType[] aclents = mement.acl.toACLEntries();
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
		mement.acl = new ACL(al.toArray(new ACLEntryType[0]));
	}

}
