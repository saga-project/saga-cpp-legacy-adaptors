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

import javax.xml.transform.TransformerException;

import org.apache.axis.message.MessageElement;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.SerializationException;
import org.naregi.rns.RNSQNames;
import org.naregi.rns.stubs.RNSEntryResponseType;
import org.naregi.rns.util.RNSUtil;

/**
 * A result data structure of RNS search.
 */
public class RNSSearchResult {
	private String path;
	private RNSEntryResponseType entry;
	private RNSError error;

	/**
	 * Set a pathname.
	 *
	 * @param path full pathname
	 */
	public void setPath(String path) {
		this.path = path;
	}

	/**
	 * Get a pathname.
	 *
	 * @return full pathname
	 */
	public String getPath() {
		return path;
	}

	/**
	 * Set an entry response data.
	 *
	 * @param entry an entry response data
	 */
	public void setEntryResponseType(RNSEntryResponseType entry) {
		this.entry = entry;
	}

	/**
	 * Get an entry response data.
	 *
	 * @return an entry response data
	 */
	public RNSEntryResponseType getEntryResponseType() {
		return entry;
	}

	/**
	 * Set a RNS error.
	 *
	 * @param error RNS error
	 */
	public void setError(RNSError error) {
		this.error = error;
	}

	/**
	 * Get a RNS error.
	 *
	 * @return RNS error
	 */
	public RNSError getError() {
		return error;
	}

	/**
	 * Get a String data which is converted from EPR.
	 *
	 * @return EPR String
	 */
	public String getEPRString() {
		if (entry == null || entry.getEndpoint() == null) {
			return null;
		}
		try {
			return RNSUtil.toXMLString(entry.getEndpoint());
		} catch (SerializationException e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		}
	}

	/**
	 * Get a Sring data which is converted from Metadata.
	 *
	 * @return Metadata String
	 */
	public String getMetadataString() {
		if (entry == null || entry.getMetadata() == null
				|| entry.getMetadata().get_any() == null) {
			return null;
		}
		try {
			StringBuilder sb = new StringBuilder();
			MessageElement[] mes = entry.getMetadata().get_any();
			for (MessageElement me : mes) {
				sb.append(RNSUtil.toIndentedXML(me));
			}

			return sb.toString();
		} catch (SerializationException e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		} catch (TransformerException e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		} catch (Exception e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		}
	}

	/**
	 * Get a String data which is converted from all structure of an entry
	 * response data (EntryResponseType) (XML).
	 *
	 * @return EntryResponseType String
	 */
	public String getEntryResponseTypeString() {
		if (entry == null) {
			return null;
		}
		MessageElement[] mes = new MessageElement[1];
		try {
			mes[0] = (MessageElement) ObjectSerializer.toSOAPElement(entry,
					RNSQNames.TYPE_ENTRY_RESPONSE_TYPE);
		} catch (SerializationException e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		}
		try {
			return RNSUtil.toIndentedXML(mes[0]);
		} catch (TransformerException e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		} catch (Exception e) {
			/* unexpected */
			e.printStackTrace();
			return null;
		}
	}
}
