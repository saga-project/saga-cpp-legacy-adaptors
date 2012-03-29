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

import org.apache.axis.message.MessageElement;

/**
 * Interface for database implementations.
 */
public interface RNSDB {
	/**
	 * Store into local files (using ObjectOutput).
	 */
	public static final String TYPE_FILE = "file";
	/**
	 * Store into Apache Derby.
	 */
	public static final String TYPE_DERBY = "derby";
	/**
	 * Store into local files as XML (using XMLEncoder).
	 */
	public static final String TYPE_XML = "xml";
	/**
	 * Store into memory. (for debug)
	 */
	public static final String TYPE_MEMORY = "mem";

	/**
	 * Create the new root directory and Set the Root ID to this object.
	 *
	 * @throws Exception
	 */
	public void createAndSetNewRootID() throws Exception;

	/**
	 * Create a new directory and Set the new ID to this object.
	 *
	 * @throws Exception
	 */
	public void createAndSetNewID() throws Exception;

	/**
	 * Initialize this object and Search an existing entry ID from this
	 * database.
	 *
	 * @param id
	 * @return true if the entry exists
	 * @throws Exception
	 */
	public boolean setID(String id) throws Exception;

	/**
	 * Get the setID()ed ID.
	 *
	 * @return ID
	 */
	public String getID();

	/**
	 * Get the root directory ID of this implementation.
	 *
	 * @return the root ID (fixed String)
	 */
	public String getRootID();

	/**
	 * Get a lock object and Start the transaction.
	 *
	 * @return Object for locks and synchronization
	 */
	public Object getLockAndStartTransaction();

	/**
	 * Makes all changes made since the previous commit/rollback permanent.
	 *
	 * @throws Exception
	 */
	public void commit() throws Exception;

	/**
	 * Undoes all changes made in the current transaction.
	 */
	public void rollback();

	/**
	 * Get properties of this directory.
	 *
	 * @return RNSDirectoryProperties (should not be null)
	 * @throws Exception
	 */
	public RNSDirectoryProperties getDirectoryProperties() throws Exception;

	/**
	 * Set properties to this directory.
	 *
	 * @param props
	 * @throws Exception
	 */
	public void setDirectoryProperties(RNSDirectoryProperties props)
			throws Exception;

	/**
	 * Set a time at which the RNS directory is created.
	 *
	 * @param t a time at which the RNS resource is created.
	 * @throws Exception
	 */
	public void setCreateTime(Calendar t) throws Exception;

	/**
	 * Set a time at which the RNS directory is last accessed.
	 *
	 * @param t a time at which the RNS directory is last accessed
	 * @throws Exception
	 */
	public void setAccessTime(Calendar t) throws Exception;

	/**
	 * Set a time at which the RNS directory is last modified.
	 *
	 * @param t a time at which the RNS directory is last modified
	 * @throws Exception
	 */
	public void setModificationTime(Calendar t) throws Exception;

	/**
	 * Get this directory list.
	 *
	 * @return entry name array (should not be null)
	 * @throws Exception
	 */
	public List<String> getList() throws Exception;

	/**
	 * Get the list size of this directory.
	 *
	 * @return the list size
	 * @throws Exception
	 */
	public long getListSize() throws Exception;

	/**
	 * Get an entry data (EPR and Metadata).
	 *
	 * @param name an entry name
	 * @return the entry data (may be null)
	 * @throws Exception
	 */
	public RNSEntryData getEntryData(String name) throws Exception;

	/**
	 * Create a new entry and Insert a new entry data.
	 *
	 * @param name an entry name
	 * @param ent the entry data (should not be null)
	 * @throws Exception
	 */
	public void insertEntryData(String name, RNSEntryData ent) throws Exception;

	/**
	 * Change an entry name.
	 *
	 * @param from old name
	 * @param to new name
	 * @throws Exception
	 */
	public void rename(String from, String to) throws Exception;

	/**
	 * Replace a Metadata of the entry.
	 *
	 * @param name an entry name.
	 * @param xmls Metadata
	 * @throws Exception
	 */
	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception;

	/**
	 * Remove an entry.
	 *
	 * @param name a name to be removed.
	 * @throws Exception
	 */
	public void removeEntryData(String name) throws Exception;

	/**
	 * Delete this directory permanently.
	 *
	 * @throws Exception
	 */
	public void destroy() throws Exception;

	/* ACL operations */

	/**
	 * Get an ACL information of this directory.
	 * @return ACL
	 * @throws Exception
	 */
	public ACL getACL() throws Exception;

	/**
	 * Replace an ACL information to this directory.
	 *
	 * @param acl ACL
	 * @throws Exception
	 */
	public void setACL(ACL acl) throws Exception;

	/**
	 * Remove the ACL of this directory.
	 *
	 * @param type ACL type (see {@link org.naregi.rns.ACL})
	 * @param names if names is null, the entries of type are removed.
	 * @throws Exception if an error occurs
	 */
	public void removeACL(short type, String[] names) throws Exception;
}
