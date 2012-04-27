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
import java.util.List;
import java.util.Stack;

import org.apache.axis.message.MessageElement;
import org.apache.commons.logging.Log;
import org.globus.wsrf.NoSuchResourceException;
import org.globus.wsrf.PersistenceCallback;
import org.globus.wsrf.RemoveCallback;
import org.globus.wsrf.Resource;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.ResourceIdentifier;
import org.globus.wsrf.ResourceKey;
import org.globus.wsrf.ResourceProperties;
import org.globus.wsrf.ResourceProperty;
import org.globus.wsrf.ResourcePropertySet;
import org.globus.wsrf.impl.ReflectionResourceProperty;
import org.globus.wsrf.impl.SimpleResourcePropertySet;

/**
 * RNS Resource(directory) status and operations.
 */
public class RNSResource implements Resource, ResourceIdentifier,
		PersistenceCallback, RemoveCallback, ResourceProperties {
	private static final long serialVersionUID = 1L;

	private ResourcePropertySet propSet = null;
	private RNSDB db = null;

	private static Object lockNewID = new Object();

	private static class DestroyThread extends Thread {
		Stack<RNSResource> stack;

		DestroyThread() {
			stack = new Stack<RNSResource>(); /* synchronized */
		}

		Log log = RNSLog.getLog();

		public void doRNSDestroy() {
			RNSResource r = stack.pop();
			synchronized (r.getLockAndStartTransaction()) {
				try {
					r.destroy();
					r.commit();
				} catch (Exception e) {
					e.printStackTrace();
					log.error("DESTORY: " + e.getMessage());
					try {
						r.rollback();
					} catch (Exception e2) {
						e2.printStackTrace();
						log.error("DESTORY: " + e2.getMessage());
					}
				}
			}
		}

		@Override
		public void run() {
			long stime = 1000;
			while (true) {
				if (stack.isEmpty() == false) {
					stime = 10000 / stack.size(); /* automatically */
					if (stime <= 0) {
						stime = 1;
					}
					doRNSDestroy();
				} else {
					if (stime < 60000) {
						stime *= 2;
					}
				}
				try {
					Thread.sleep(stime);
				} catch (InterruptedException e) {
				}
			}
		}
	}

	private static Object destroyThreadLock = new Object();
	private static DestroyThread destroyThread = null;

	private void destroy() throws Exception {
		//System.out.println("[RNS] Destroy : " + this.hashCode());
		db.destroy();
	}

	/* RemoveCallback interface (for DESTROY operation) */
	public void remove() throws ResourceException {
		long s = RNSProfiler.start();
		if (db == null) {
			return;
		}
		if (db.getRootID().equals(db.getID())) {
			throw new ResourceException("Cannot delete Root directory");
		}
		RNSLog.getLog().debug("DESTROY:ID=" + db.getID());
		if (isWritable() == false) {
			throw new ResourceException(
					"You cannot permit to destroy resources");
		}
		try {
			if (db.getListSize() > 0) {
				throw new ResourceException("not empty");
			}
		} catch (Exception e) {
			throw new ResourceException(e);
		}
		if (RNSConfig.delayedDestroy()) {
			/* NOTE: not so effective... */
			synchronized (destroyThreadLock) {
				if (destroyThread == null) {
					destroyThread = new DestroyThread();
					destroyThread.setDaemon(false);
					destroyThread.start();

					Runtime.getRuntime().addShutdownHook(new Thread() {
						@Override
						public void run() {
							while (destroyThread.stack.isEmpty() == false) {
								destroyThread.doRNSDestroy();
							}
						}
					});
				}
			}
			destroyThread.stack.push(this);
			RNSProfiler.stop(RNSProfiler.TYPE.Total_Destroy, s);
			return;
		}

		synchronized (getLockAndStartTransaction()) {
			try {
				db.destroy();
				commit();
			} catch (Exception e) {
				rollback();
				throw new ResourceException(e);
			}
		}
		RNSProfiler.stop(RNSProfiler.TYPE.Total_Destroy, s);
	}

	/* ResourceIdentifier interface */
	public Object getID() {
		if (db == null) {
			return null;
		}
		return db.getID();
	}

	/* PersistenceCallback interface */
	public final void load(ResourceKey key) throws ResourceException {
		String id = (String) key.getValue();
		RNSLog.getLog().debug("load resource:ID=" + id);
		initializeExistingDir(id);
	}

	/* PersistenceCallback interface */
	public final void store() throws ResourceException {
		synchronized (getLockAndStartTransaction()) {
			try {
				commit();
			} catch (Exception e) {
				throw new ResourceException(e);
			}
		}
	}

	private RNSDirectoryProperties getInitializeProperties() {
		RNSDirectoryProperties rnsProps = new RNSDirectoryProperties();
		rnsProps.setCreateTime(Calendar.getInstance());
		rnsProps.setAccessTime(rnsProps.getCreateTime());
		rnsProps.setModificationTime(rnsProps.getCreateTime());
		return rnsProps;
	}

	private void initialize(String id, boolean createRoot)
			throws ResourceException {
		if (db != null) {
			return;
		}

		/* switch DB */
		String type = RNSConfig.getDataBaseType();
		if (RNSDB.TYPE_DERBY.equals(type)) {
			try {
				if (RNSConfig.isDataBaseCache()) {
					db = new RNSDBCache(new RNSDBDerby(), true); /* force cached list */
				} else {
					db = new RNSDBDerby();
				}
			} catch (Exception e) {
				throw new ResourceException(e);
			}
			RNSLog.getLog().debug("dbType=RNSDBDerby");
		} else if (RNSDB.TYPE_FILE.equals(type)) {
			db = new RNSDBFile();
			RNSLog.getLog().debug("dbType=RNSDBFile");
		} else if (RNSDB.TYPE_XML.equals(type)) {
			db = new RNSDBXML();
			RNSLog.getLog().debug("dbType=RNSDBXML");
		} else if (RNSDB.TYPE_MEMORY.equals(type)) {
			db = new RNSDBMemory();
			RNSLog.getLog().debug("dbType=RNSDBMemory");
		} else {
			RNSLog.getLog().debug("dbType=" + type + " is unknown");
			throw new ResourceException("unknown dbType: " + type);
		}

		if (id == null) { /* new Resource */
			synchronized (lockNewID) {
				try {
					db.createAndSetNewID();
					db.setDirectoryProperties(getInitializeProperties());
					db.commit();
				} catch (Exception e) {
					rollback();
					throw new ResourceException(
							"cannot create new Resource ID", e);
				}
			}
		} else {
			if (createRoot) {
				synchronized (lockNewID) {
					try {
						db.createAndSetNewRootID();
						db.setDirectoryProperties(getInitializeProperties());
						ACL acl = new ACL();
						acl.setOwner(ACL.ANONYMOUS, ACL.PERM_ALL);
						acl.setOwnerGroup(ACL.ANONYMOUS, ACL.PERM_ALL);
						acl.setOther(ACL.PERM_ALL);
						db.setACL(acl);
						db.commit();
					} catch (Exception e) {
						rollback();
						throw new ResourceException(
								"cannot create new Resource ID", e);
					}
				}
			} else { /* existing entry */
				if (RNSConfig.getRootID().equals(id)) {
					id = db.getRootID();
				}
				boolean exist;
				try {
					exist = db.setID(id);
				} catch (Exception e) {
					throw new ResourceException("cannot set id=" + id, e);
				}
				/* important */
				if (exist == false) {
					throw new NoSuchResourceException("not exist on DB: id="
							+ id);
				}
			}
		}
	}

	public final void initializeNewRootDir()
			throws ResourceException {
		initialize(null, true);
		return;
	}

	public final void initializeNewDir() throws ResourceException {
		initialize(null, false);
		return;
	}

	public final void initializeExistingDir(String id) throws ResourceException {
		initialize(id, false);
		return;
	}

	private synchronized void initPropertySet() {
		if (propSet != null) {
			return;
		}
		propSet = new SimpleResourcePropertySet(RNSQNames.RESOURCE_PROPERTIES);
		try {
			ResourceProperty rp;
			/* RNS Resource Properties */
			rp = new ReflectionResourceProperty(RNSQNames.RP_ELEMENTCOUNT, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(RNSQNames.RP_CREATETIME, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(RNSQNames.RP_ACCESSTIME, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(RNSQNames.RP_MODIFICATIONTIME,
					this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(RNSQNames.RP_READABLE, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(RNSQNames.RP_WRITABLE, this);
			propSet.add(rp);

			/* WS-Iterator Resource Properties */
			rp = new ReflectionResourceProperty(
					RNSQNames.ITERATOR_RP_ELEMENTCOUNT, this);
			propSet.add(rp);
			rp = new ReflectionResourceProperty(
					RNSQNames.ITERATOR_RP_PREFERREDBLOCKSIZE, this);
			propSet.add(rp);

			/* RNSExtension Resource Properties */
			rp = new ReflectionResourceProperty(RNSQNames.RP_VERSION, this);
			propSet.add(rp);
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException(e);
		}
	}

	/* ResourceProperties interface */
	public ResourcePropertySet getResourcePropertySet() {
		initPropertySet();
		return propSet;
	}

	/* ResourceProperties Getter/Setter s */
	public long getElementCount() throws ResourceException {
		try {
			return db.getListSize();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public final void setElementCount(long value) {
		/* unused */
	}

	public final Calendar getCreateTime() throws ResourceException {
		RNSDirectoryProperties prop;
		try {
			prop = db.getDirectoryProperties();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
		if (prop == null) {
			return Calendar.getInstance();
		}
		return prop.getCreateTime();
	}

	public final void setCreateTime(Calendar t) throws ResourceException {
		try {
			db.setCreateTime(t);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public final Calendar getAccessTime() throws ResourceException {
		RNSDirectoryProperties prop;
		try {
			prop = db.getDirectoryProperties();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
		if (prop == null) {
			return Calendar.getInstance();
		}
		return prop.getAccessTime();
	}

	public final void setAccessTime(Calendar t) throws ResourceException {
		try {
			db.setAccessTime(t);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public final Calendar getModificationTime() throws ResourceException {
		RNSDirectoryProperties prop;
		try {
			prop = db.getDirectoryProperties();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
		if (prop == null) {
			return Calendar.getInstance();
		}
		return prop.getModificationTime();
	}

	public final void setModificationTime(Calendar t) throws ResourceException {
		try {
			db.setModificationTime(t);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public final boolean isReadable() {
		return AccessControlSwitch.canRead(this,
				AccessControlSwitch.getCallerInfomation());
	}

	public final void setReadable(boolean b) {
		/* unused */
		return;
	}

	public final boolean isWritable() {
		return AccessControlSwitch.canWrite(this,
				AccessControlSwitch.getCallerInfomation());
	}

	public final void setWritable(boolean b) {
		/* unused */
		return;
	}

	/* for WS-Iterator */
	public int getPreferredBlockSize() {
		int i = RNSConfig.getIteratorUnit();
		if (i > 0) {
			return i;
		}
		return 5;
	}

	public void setPreferredBlockSize(int size) {
		/* unused */
	}

	/* for RNSExtension */
	public final String getVersion() {
		return RNSVersion.getVersion();
	}

	public final void setVersion(String version) {
		/* unused */
	}

	/* --------------------------------------------- */
	/* RNS Resource special methods */
	public Object getLockAndStartTransaction() {
		return db.getLockAndStartTransaction();
	}

	public void commit() throws ResourceException {
		RNSLog.getLog().debug("update resource:ID=" + db.getID());
		try {
			db.commit();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void rollback() {
		db.rollback();
	}

	public List<String> getList() throws ResourceException {
		try {
			return db.getList();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public RNSEntryData getRNSEntryData(String name) throws ResourceException {
		/* null: noent */
		try {
			return db.getEntryData(name);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void removeRNSEntryData(String name) throws ResourceException {
		try {
			db.removeEntryData(name);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void insertRNSEntryData(String name, RNSEntryData ent)
			throws ResourceException {
		if (name == null) {
			throw new ResourceException("unexpected: name is null");
		}
		try {
			db.insertEntryData(name, ent);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void rename(String from, String to) throws ResourceException {
		try {
			db.rename(from, to);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void replaceMetadata(String name, MessageElement[] xmls)
			throws ResourceException {
		try {
			db.replaceMetadata(name, xmls);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	/* ACL operations */
	public ACL getACL() throws ResourceException {
		try {
			return db.getACL();
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void setACL(ACL acl) throws ResourceException {
		try {
			db.setACL(acl);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}

	public void removeACL(short type, String[] names) throws ResourceException {
		try {
			db.removeACL(type, names);
		} catch (Exception e) {
			throw new ResourceException(e);
		}
	}
}
