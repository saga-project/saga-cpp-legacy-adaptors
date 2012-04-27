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
package org.naregi.rns.client;

import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.util.RNSUtil;

import org.globus.axis.message.addressing.EndpointReferenceType;

/**
 * Represent a RNS directory. RNSDirent is taken from RNSDirHandle.
 */
public class RNSDirent {
	/**
	 * Entry types.
	 */
	public enum TYPE {
		DIRECTORY, JUNCTION
	};

	private String name = null;
	private TYPE type = null;
	private EndpointReferenceType epr = null;
	private RNSMetadataType meta = null;
	private RNSStat stat = null;

	private RNSError error;

	/**
	 * Get the entry name.
	 *
	 * @return entry name
	 */
	public String getName() {
		return name;
	}

	/**
	 * Set an entry name
	 *
	 * @param name entry name
	 */
	public void setName(String name) {
		this.name = name;
	}

	/**
	 * Get type of this entry.
	 *
	 * @return DIRECTORY or JUNCTION
	 */
	public TYPE getType() {
		return type;
	}

	/**
	 * Set type of the entry.
	 *
	 * @param type DIRECTORY or JUNCTION
	 */
	public void setType(TYPE type) {
		this.type = type;
	}

	/**
	 * Check whether the entry is a directory.
	 *
	 * @return true if the entry is a directory
	 */
	public boolean isDirectory() {
		if (type != null) {
			if (type.equals(TYPE.DIRECTORY)) {
				return true;
			} else {
				return false;
			}
		}
		if (RNSUtil.isDirectory(meta)) {
			type = TYPE.DIRECTORY;
			return true;
		} else {
			type = TYPE.JUNCTION;
			return false;
		}
	}

	/**
	 * Get the Endpoint Reference of this entry.
	 *
	 * @return EPR
	 */
	public EndpointReferenceType getEpr() {
		return epr;
	}

	/**
	 * Set a Endpoint Reference to this entry.
	 *
	 * @param epr EPR
	 */
	public void setEpr(EndpointReferenceType epr) {
		this.epr = epr;
	}

	/**
	 * Get the Metadata of this entry.
	 *
	 * @return RNSMetadataType
	 */
	public RNSMetadataType getMeta() {
		return meta;
	}

	/**
	 * Set a Metadata to this entry.
	 *
	 * @param meta RNSMetadataType
	 */
	public void setMeta(RNSMetadataType meta) {
		this.meta = meta;
		isDirectory(); /* to set "type" */
	}

	/**
	 * Get the RNS status of this entry.
	 *
	 * @return null if withRNSStat flag was not set as true in list().
	 */
	public RNSStat getStat() {
		return stat;
	}

	/**
	 * Set a RNS status to this entry.
	 *
	 * @param stat RNSStat
	 */
	public void setStat(RNSStat stat) {
		this.stat = stat;
	}

	/**
	 * Set RNSError of this entry.
	 *
	 * @param error null if no error occurred at this entry
	 */
	public void setRNSError(RNSError error) {
		this.error = error;
	}

	public RNSError getRNSError() {
		return error;
	}
}
