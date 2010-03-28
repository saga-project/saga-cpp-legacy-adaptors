/*
 * $RCSfile: ExtModuleRequest.java,v $ $Revision: 1.6 $ $Date: 2008/02/07 08:17:43 $
 * $AIST_Release: 5.0.0 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
package org.apgrid.grpc.ng;

import java.io.Writer;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

/*
 * Representation of a Request for External Module Protocol
 */
class ExtModuleRequest {

	private final String name;              // request name
	private final String params;            // params that follows request name
	private final boolean multiple;         // multiple request flag
	private List<String> attributes = null; // attribute of request

	private static final String LS = "\r\n"; // Line Separator

	// Common requests on Ninf-G
	public static final ExtModuleRequest QUERY_FEATURES = 
		new ExtModuleRequest("QUERY_FEATURES", false);
	public static final ExtModuleRequest EXIT =
		new ExtModuleRequest("EXIT", false);

	/**
	 * Constructor
	 * @param name Request name
	 * @param multiple true if multiple line request
	 * @param params Parameters that follows request name
	 */
	private ExtModuleRequest(String name, boolean multiple,
	                              String ... params){
		this.name = name;
		this.attributes = new ArrayList<String>();
		this.multiple = multiple;

		StringBuilder sb = new StringBuilder();
		for (String p : params) {
			sb.append(" ").append(p);
		}
		this.params = sb.toString();
	}

	/**
	 * Single Request create method
	 * 
	 * @param name Request name
	 * @param params parameters that follows request name
	 */
	public static ExtModuleRequest issuesSingle(String name,
	                                                 String ... params) {
		return new ExtModuleRequest(name, false, params);
	}

	/*
	 * Multiple Request create method
	 * @param name Request name
	 * @param params parameters that follows request name
	 */
	public static ExtModuleRequest issuesMultiple(String name,
	                                                   String ... params) {
		return new ExtModuleRequest(name, true, params);
	}

	/**
	 * Add attribute. 
	 * This method influence submit()
	 * @param key Attribute name
	 * @param val Attribute value
	 */
	public void addAttribute(String key, String val) {
		if ( ! multiple )
			throw new UnsupportedOperationException("This is single request");

		attributes.add(key + " " + val);
	}

	/**
	 * Add attribute. 
	 * @param attr Attribute.(expected format "name val");
	 */
	public void addAttribute(String attr) {
		if ( ! multiple )
			throw new UnsupportedOperationException("This is single request");
		if ( attr.indexOf(" ") < 0 )
			throw new IllegalArgumentException("Attribute is " + attr);

		attributes.add(attr);
	}

	/*
	 * Submit request to writer. 
	 */
	public void submit(Writer writer) throws IOException {
		if ( multiple ) {
			multipleSubmit(writer);
		} else {
			singleSubmit(writer);
		}
	}

	private void singleSubmit(Writer writer) throws IOException {
		if ( (name == null) || (params == null) )
			throw new IllegalArgumentException();

		String req = name + params + LS;
		writer.write(req);
		writer.flush();
	}

	private void multipleSubmit(Writer writer) throws IOException {
		if ( (name == null) || (params == null) || (attributes == null) )
			throw new IllegalArgumentException();

		StringBuilder req = new StringBuilder(name);
		req.append(params).append(LS);

		for (String attr : attributes) {
			req.append(attr).append(LS);
		}

		req.append(name).append("_END").append(LS);
		writer.write(req.toString());
		writer.flush();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder(this.name);
		sb.append(this.params);
		if ( multiple ) {
			sb.append("\n");
			for (String attr : attributes) {
				sb.append(attr).append("\n");
			}
			sb.append(this.name).append("_END");
		}
		return sb.toString();
	}

}

