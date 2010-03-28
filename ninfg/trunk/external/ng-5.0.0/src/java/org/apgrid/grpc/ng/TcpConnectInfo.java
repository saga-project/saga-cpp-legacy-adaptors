/*
 * $RCSfile: TcpConnectInfo.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.TCP_CONNECT_RETRY_COUNT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.TCP_CONNECT_RETRY_BASEINTERVAL;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.TCP_CONNECT_RETRY_INCREASERATIO;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.TCP_CONNECT_RETRY_RANDOM;

/*
 * Information for RemoteMachineInfo
 */
public class TcpConnectInfo {
	private String tcp_connect_retryCount;
	private String tcp_connect_retryBaseInterval;
	private String tcp_connect_retryIncreaseRatio;
	private String tcp_connect_retryRandom;

	/**
	 * TcpConnectInfo Constructor
	 */
	public TcpConnectInfo() {
		setDefaultParameter();
	}
	
	/**
	 * @param param
	 */
	public TcpConnectInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.tcp_connect_retryCount = "4";
		this.tcp_connect_retryBaseInterval = "1";
		this.tcp_connect_retryIncreaseRatio = "2.0";
		this.tcp_connect_retryRandom = "true";
	}


	public String getTcpConnectRetryCount() {
    	return this.tcp_connect_retryCount;
	} 
	public String getTcpConnectRetryBaseinterval() {
    	return this.tcp_connect_retryBaseInterval;
	} 
	public String getTcpConnectRetryIncreaseratio() {
    	return this.tcp_connect_retryIncreaseRatio;
	} 
	public String getTcpConnectRetryRandom() {
    	return this.tcp_connect_retryRandom;
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
		if ( aKey.equals(TCP_CONNECT_RETRY_COUNT) ) {
			this.tcp_connect_retryCount = value;
			return;
		} else if ( aKey.equals(TCP_CONNECT_RETRY_BASEINTERVAL) ) {
			this.tcp_connect_retryBaseInterval = value;
			return;
		} else if ( aKey.equals(TCP_CONNECT_RETRY_INCREASERATIO) ) {
			this.tcp_connect_retryIncreaseRatio = value;
			return;
		} else if ( aKey.equals(TCP_CONNECT_RETRY_RANDOM) ) {
			this.tcp_connect_retryRandom = value;
			return;
		}

		throw new IllegalArgumentException(aKey);
	}

	public static TcpConnectInfo copy(TcpConnectInfo other) {
		TcpConnectInfo result = new TcpConnectInfo();
		result.tcp_connect_retryCount = other.tcp_connect_retryCount;
		result.tcp_connect_retryBaseInterval =
			 other.tcp_connect_retryBaseInterval;
		result.tcp_connect_retryIncreaseRatio =
			 other.tcp_connect_retryIncreaseRatio;
		result.tcp_connect_retryRandom = other.tcp_connect_retryRandom;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.tcp_connect_retryCount = null;
		this.tcp_connect_retryBaseInterval = null;
		this.tcp_connect_retryIncreaseRatio = null;
		this.tcp_connect_retryRandom = null;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("tcp_connect_retryCount: " + tcp_connect_retryCount + "\n");
		sb.append("tcp_connect_retryBaseInterval: " 
			+ tcp_connect_retryBaseInterval + "\n");
		sb.append("tcp_connect_retryIncreaseRatio: " 
			+ tcp_connect_retryIncreaseRatio + "\n");
		sb.append("tcp_connect_retryRandom: " + tcp_connect_retryRandom + "\n");
		return sb.toString();
	}

}

