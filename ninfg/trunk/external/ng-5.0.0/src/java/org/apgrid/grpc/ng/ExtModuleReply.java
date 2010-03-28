/*
 * $RCSfile: ExtModuleReply.java,v $ $Revision: 1.5 $ $Date: 2008/02/01 06:29:27 $
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

import java.io.LineNumberReader;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

/*
 * Representation of a Reply for External Module Protocol
 */
class ExtModuleReply {

	private final String result; // REPLY result "S", "F", "SM"
	private final String value; // return value of single line reply
	private final List<String> values; // return value of multiple line reply
	private final boolean multiple;

	private static final String CLOSE_TAG = "REPLY_END";

	/*
	 * Constructor for Multiple Reply
	 */
	private ExtModuleReply(String result, List<String> values) {
		this.result = result;
		this.value  = null;
		this.values = values;
		this.multiple = true;
	}

	/*
	 * Constructor for Single Reply
	 */
	private ExtModuleReply(String result, String value) {
		this.result = result;
		this.value  = value;
		this.values = null;
		this.multiple = false;
	}

	/**
	 * Create method
	 * 
	 * @param reader 
	 */
	public static ExtModuleReply issues(LineNumberReader reader)
	throws IOException {
		List<String> values = null;
		String first_line = reader.readLine();
		if ( isMultipleReply(first_line) ) {
			values = getReturnValues(reader);
		}
		return build(first_line, values);
	}

	private static boolean isMultipleReply(String line) {
		if ( line.startsWith("SM") ) { return true; }
		return false;
	}

	private static List<String> getReturnValues(LineNumberReader reader)
	throws IOException {
		List<String> ret = new ArrayList<String>();
		String line = null;
		for (;;) {
			line = reader.readLine();
			if ((line == null) || (line.equals(CLOSE_TAG)) )
				break;
			ret.add(line);
		}
		return ret;
	}

	private static ExtModuleReply build(String line, List<String> values) {
		if ( isMultipleReply(line) )
			return new ExtModuleReply(line, values);

		String res = null, val = "";
		res = line.substring(0, 1); // get first string
		if (line.length() > 2) {
			val = line.substring(2);
		}
		return new ExtModuleReply(res, val);
	}

	/*
	 * @return "S" , "F", or "SM"
	 */
	public String result() {
		return this.result;
	}

	/*
	 * @return return value by single reply
	 */
	public String returnValue() {
		if (multiple)
			throw new UnsupportedOperationException();
		return this.value;
	}

	/*
	 * @return return values by multiple reply
	 */
	public List<String> returnValues() {
		if ( ! multiple)
			throw new UnsupportedOperationException();
		return this.values;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder(result);
		if ( multiple ) {
			sb.append("\n");
			for (String val : values) {
				sb.append(val).append("\n");
			}
			sb.append(CLOSE_TAG);
		} else {
			sb.append(" ").append(value);
		}
		return sb.toString();
	}

}

