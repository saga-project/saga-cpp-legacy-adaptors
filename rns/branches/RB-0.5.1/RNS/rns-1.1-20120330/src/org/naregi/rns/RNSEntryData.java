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

import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.stubs.RNSSupportType;
import org.naregi.rns.stubs.SupportsRNSType;

import org.apache.axis.message.MessageElement;
import org.globus.axis.message.addressing.EndpointReferenceType;

/**
 * Structure for a RNS entry data.
 */
public class RNSEntryData {
	private String localID = null; /* entry id in local service */
	private EndpointReferenceType epr = null; /* entry reference to remote service */
	private RNSMetadataType meta;

	public boolean isDirectory() {
		if (meta != null) {
			return RNSSupportType.value1.equals(meta.getSupportsRns()
					.getValue());
		}
		return false;
	}

	public void setDirectory(boolean isDirectory) {
		if (meta == null) {
			meta = new RNSMetadataType();
		}
		SupportsRNSType st = new SupportsRNSType();
		if (isDirectory) {
			st.setValue(RNSSupportType.value1);
		} else {
			st.setValue(RNSSupportType.value2);
		}
		meta.setSupportsRns(st);
	}

	public RNSMetadataType getRNSMetadata() {
		return meta;
	}

	public void setRNSMetadata(RNSMetadataType meta) {
		this.meta = meta;
	}

	public void setAny(MessageElement[] any) {
		if (meta == null) {
			meta = new RNSMetadataType();
		}
		meta.set_any(any);
	}

	/**
	 * Get the local ID.
	 *
	 * @return local ID
	 */
	public String getLocalID() {
		return localID;
	}

	/**
	 * Set a local ID.
	 *
	 * @param localID local ID
	 */
	public void setLocalID(String localID) {
		if (localID != null) {
			this.epr = null;
		}
		this.localID = localID;
	}

	/**
	 * Get the EPR.
	 *
	 * @return epr EPR
	 */
	public EndpointReferenceType getEpr() {
		return epr;
	}

	/**
	 * Set an EPR.
	 *
	 * @param epr EPR
	 */
	public void setEpr(EndpointReferenceType epr) {
		if (epr != null) {
			this.localID = null;
		}
		this.epr = epr;
	}
}
