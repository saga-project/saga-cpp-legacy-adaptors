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

import javax.xml.namespace.QName;

/**
 * Definitions of QName for RNS service protocol.
 */
public interface RNSQNames {
	public static final String NS_RNS = "http://schemas.ogf.org/rns/2009/12/rns";
	public static final String NS_RNSMS = "http://schemas.ogf.org/rns/2010/11/metadata-search";
	public static final String NS_ITERATOR = "http://schemas.ogf.org/ws-iterator/2008/06/iterator";
	public static final String NS_EXTENSION = "http://schemas.naregi.org/rns/2010/07/extension";
	public static final String NS_IMPL = "http://naregi.org/rns/2010/10/rns";

	/* our specification */
	public static final QName RESOURCE_ID = new QName(NS_IMPL, "RNSID");
	//public static final QName RESOURCE_REFERENCE = new QName(NS_IMPL, "RNSReosurceReference");

	/* RNS */
	public static final QName RESOURCE_PROPERTIES = new QName(NS_RNS, "RNSRP");

	public static final QName RP_ELEMENTCOUNT = new QName(NS_RNS, "elementCount");
	public static final QName RP_CREATETIME = new QName(NS_RNS, "createTime");
	public static final QName RP_ACCESSTIME = new QName(NS_RNS, "accessTime");
	public static final QName RP_MODIFICATIONTIME = new QName(NS_RNS, "modificationTime");
	public static final QName RP_READABLE = new QName(NS_RNS, "readable");
	public static final QName RP_WRITABLE = new QName(NS_RNS, "writable");

	public static final QName TYPE_ENTRY_TYPE = new QName(NS_RNS, "RNSEntryType");
	public static final QName TYPE_ENTRY_RESPONSE_TYPE = new QName(NS_RNS, "RNSEntryResponseType");
	public static final QName TYPE_SUPPORTS_RNS = new QName(NS_RNS, "supports-rns");

	/* Extension */
	public static final QName RP_VERSION = new QName(NS_EXTENSION, "version");

	/* WS-Iterator */
	//public static final QName ITERATOR_RESOURCE_REFERENCE = new QName(NS_ITERATOR, "WSIteratorReosurceReference");
	public static final QName ITERATOR_RESOURCE_PROPERTIES = new QName(NS_ITERATOR, "WSIteratorRP");

	public static final QName ITERATOR_RP_ELEMENTCOUNT = new QName(NS_ITERATOR, "elementCount");
	public static final QName ITERATOR_RP_PREFERREDBLOCKSIZE = new QName(NS_ITERATOR, "preferredBlockSize");
}
