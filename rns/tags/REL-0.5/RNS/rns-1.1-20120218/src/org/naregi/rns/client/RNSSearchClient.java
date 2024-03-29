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
package org.naregi.rns.client;

/**
 * RNSSearchClient object is generated by
 * {@link RNSClientHome#getRNSSearchClient()}.
 *
 * RNSSearchClient implementation is {@link RNSSearchClientImpl}.
 */
public interface RNSSearchClient {

	/**
	 * Search Metadata (&lt;RNSEntryResponseType&gt; structure) of the entry.
	 *
	 * The return structure also should be &lt;RNSEntryResponseType&gt;.
	 *
	 * The pure &lt;RNSEntryResponseType&gt; structure is searched by "/" as the
	 * XQuery (XPath).
	 *
	 * <pre>
	 * --- Example of the XQuery format ---
	 * declare namespace ns1 = "http://schemas.ogf.org/rns/2009/12/rns";
	 * let $ent := /ns1:RNSEntryResponseType
	 * let $name := string($ent/@entry-name)
	 * let $epr := $ent/ns1:endpoint
	 * let $meta := $ent/ns1:metadata
	 * let $sptrns := $meta/ns1:supports-rns
	 *
	 * let $retv :=
	 * (
	 *   for $m in $meta/*
	 *   let $metatag := local-name($m)
	 *   let $key := string($m/@key)
	 *   let $value := $m/text()
	 *   where $key = "key1"
	 *     and fn:matches($value, "^[0-9]+$")
	 *     and xs:integer($value) &gt;= 1000
	 *   return
	 *   $m
	 * )
	 * where exists($retv)
	 * return
	 * &lt;ns1:RNSEntryResponseType entry-name="{$name}"
	 *   xmlns:ns1="http://schemas.ogf.org/rns/2009/12/rns"
	 *   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	 *   xsi:type="ns1:RNSEntryResponseType"&gt;
	 *   {$epr}"
	 *   &lt;ns1:metadata xsi:type="ns1:RNSMetadataType"&gt;
	 *     {$sptrns}
	 *     {$retv}
	 *   &lt;/ns1:metadata&gt;
	 * &lt;/ns1:RNSEntryResponseType&gt;
	 * </pre>
	 *
	 * @param path a pathname
	 * @param xquery XQuery of the request
	 * @return null if the result does not exist
	 * @throws RNSError if an error occurs
	 */
	public RNSSearchResult search(String path, String xquery) throws RNSError;

	/**
	 * Bulk search operation.
	 *
	 * See {@link RNSSearchClient#search(String, String)}
	 *
	 * @param dirPath a parent directory
	 * @param names names of the children
	 * @param xquery XQuery of the request
	 * @return null if the result does not exist
	 * @throws RNSError if an error occurs
	 */
	public RNSSearchResultHandle searchBulk(String dirPath, String[] names,
			String xquery) throws RNSError;

	/**
	 * Search recursively.
	 *
	 * See {@link RNSSearchClient#search(String, String)}
	 *
	 * @param dirPath a directory pathname. (This directory is not searched)
	 * @param xquery XQuery of the request
	 * @param depth a number of depth to search recursively
	 * @return null or empty result in RNSSearchResultHandle if the result does
	 *         not exist
	 * @throws RNSError if an error occurs
	 */
	public RNSSearchResultHandle searchRecursive(String dirPath, String xquery,
			int depth) throws RNSError;

	/**
	 * List directory entries by Search operation.
	 *
	 * This is the same result as RNSClient#list(dirPath, false).
	 *
	 * See {@link RNSClient#list(String, boolean)}.
	 *
	 * @param dirPath a directory pathname
	 * @return RNSDirHandle. (have no entry if this is null)
	 * @throws RNSError if an error occurs
	 */
	public RNSDirHandle listBySearch(String dirPath) throws RNSError;
}
