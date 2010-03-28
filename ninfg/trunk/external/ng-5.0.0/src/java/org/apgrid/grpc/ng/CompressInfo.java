/*
 * $RCSfile: CompressInfo.java,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:07 $
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

import java.util.Map;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMPRESS;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMPRESS_THRESHOLD;

/*
 * Information for RemoteMachineInfo class
 */
public class CompressInfo {
	private String compress;
	private String compress_threshold;

	public static final String COMPRESS_RAW  = "raw";
	public static final String COMPRESS_ZLIB = "zlib";


	/**
	 * CompressInfo Default Constructor
	 */
	public CompressInfo() {
		initInfo();
		setDefaultParameter();
	}
	
	public CompressInfo(Map<Object, Object> param) {
		this(); // call default constructor
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void initInfo() {
		this.compress = null;
		this.compress_threshold = null;
	}

	private final void setDefaultParameter() {
		this.compress = COMPRESS_RAW;
		this.compress_threshold = "65536";
	}


	public String getCompress() {
		return this.compress;
	} 
	public int getCompressThreshold() {
		return Integer.parseInt(this.compress_threshold);
	}


	/**
	 * @param param
	 */
	public void update(Map<Object, Object> param) {
		for (Map.Entry<Object, Object> ent : param.entrySet()) {
			_update((String)ent.getKey(), (String)ent.getValue());
		}
	}

	public void put(String aKey, String value) {
		_update(aKey, value);
	}

	private final void _update(String aKey, String value) {
		if ( aKey.equals(COMPRESS) ) {
			this.compress = value;
			return;
		} else if ( aKey.equals(COMPRESS_THRESHOLD) ) {
			this.compress_threshold = value;
			return;
		}
		throw new IllegalArgumentException(aKey);
	}

	public static CompressInfo copy(CompressInfo other) {
		CompressInfo result = new CompressInfo();
		result.compress = other.compress;
		result.compress_threshold = other.compress_threshold;
		return result;
	}

	public void reset() {
		initInfo();
		setDefaultParameter();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("compress: " + compress + "\n");
		sb.append("compress_threshold: " + compress_threshold + "\n");
		return sb.toString();
	}

}
