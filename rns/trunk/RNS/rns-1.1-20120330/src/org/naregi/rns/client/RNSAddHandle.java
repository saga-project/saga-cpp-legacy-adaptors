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

import java.util.ArrayList;
import java.util.List;

import org.apache.axis.message.MessageElement;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.naregi.rns.stubs.RNSEntryType;
import org.naregi.rns.stubs.RNSMetadataType;
import org.naregi.rns.stubs.RNSSupportType;
import org.naregi.rns.stubs.SupportsRNSType;

/**
 * Handle for preparation of bulk Add operation.
 *
 * @see RNSClient#addBulk(String, RNSAddHandle)
 */
public class RNSAddHandle {
	private ArrayList<RNSEntryType> list = new ArrayList<RNSEntryType>();

	/**
	 * Register a request for mkdir operation into the bulk operation array.
	 *
	 * @param name new directory name
	 * @param xmls Meatadata
	 */
	public void registerMkdir(String name, MessageElement[] xmls) {
		list.add(mkdirRequest(name, xmls));
	}

	/**
	 * Register a request for add new junction operation into the bulk operation
	 * array.
	 *
	 * @param name new junction name
	 * @param epr EPR
	 * @param xmls Metadata
	 * @param isRNSDir true when a referenced RNS directory will be registered.
	 */
	public void registerAddEPR(String name, EndpointReferenceType epr,
			MessageElement[] xmls, boolean isRNSDir) {
		list.add(addEPRRequest(name, epr, xmls, isRNSDir));
	}

	/**
	 * Get List of prepared bulk operation.
	 *
	 * @return List of RNSEntryType
	 */
	public List<RNSEntryType> getList() {
		return list;
	}

	private RNSEntryType mkdirRequest(String name, MessageElement[] xmls) {
		RNSEntryType er = new RNSEntryType();
		er.setEntryName(name);
		er.setEndpoint(null); /* == local directory */
		if (xmls != null) { /* nillable="true" */
			RNSMetadataType meta = new RNSMetadataType();
			meta.set_any(xmls);
			SupportsRNSType st = new SupportsRNSType();
			// is Directory
			st.setValue(RNSSupportType.value1);
			meta.setSupportsRns(st);
			er.setMetadata(meta);
		}
		return er;
	}

	private RNSEntryType addEPRRequest(String name, EndpointReferenceType epr,
			MessageElement[] xmls, boolean isRNS) {
		RNSEntryType er = new RNSEntryType();
		er.setEntryName(name);
		er.setEndpoint(epr);
		RNSMetadataType meta = new RNSMetadataType();
		SupportsRNSType st = new SupportsRNSType();
		if (isRNS) {
			st.setValue(RNSSupportType.value1);
		} else {
			st.setValue(RNSSupportType.value2);
		}
		meta.setSupportsRns(st);
		meta.set_any(xmls);
		er.setMetadata(meta);
		return er;
	}
}
