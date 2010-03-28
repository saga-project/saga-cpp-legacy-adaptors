/*
 * $RCSfile: HeartbeatInfo.java,v $ $Revision: 1.5 $ $Date: 2008/01/22 07:03:03 $
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
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HEARTBEAT;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.HEARTBEAT_TIMEOUTCOUNT;

/*
 * Information for RemoteMachineInfo
 */
public class HeartbeatInfo {
	private String heartbeat = null;
	private String heartbeat_timeoutCount = null;

	/**
	 * RemoteMachineInfo Constructor
	 */
	public HeartbeatInfo() {
		setDefaultParameter();
	}
	
	/**
	 * @param param
	 */
	public HeartbeatInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.heartbeat = "60";
		this.heartbeat_timeoutCount = "5";
	}


	public int getHeartbeat() {
		return Integer.parseInt(this.heartbeat);
	} 

	public int getHeartbeatTimeoutCount() {
		return Integer.parseInt(this.heartbeat_timeoutCount);
	} 

	/**
	 * @param param
	 */
	public void update(Map<Object, Object> param) {
		for (Map.Entry<Object, Object> ent : param.entrySet()) {
			_update((String) ent.getKey(), (String) ent.getValue());
		}
	}

	public void put(String aKey, String value) {
		_update(aKey, value);
	}

	private final void _update(String aKey, String value) {
		if ( aKey.equals(HEARTBEAT) ) {
			this.heartbeat = value;
			return;
		} else if ( aKey.equals(HEARTBEAT_TIMEOUTCOUNT) ) {
			this.heartbeat_timeoutCount = value;
			return;
		}
		throw new IllegalArgumentException(aKey);
	}

	public static HeartbeatInfo copy(HeartbeatInfo other) {
		HeartbeatInfo result = new HeartbeatInfo();
		result.heartbeat = other.heartbeat;
		result.heartbeat_timeoutCount = other.heartbeat_timeoutCount;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.heartbeat = null;
		this.heartbeat_timeoutCount = null;
	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("heartbeat: " + heartbeat + "\n");
		sb.append("heartbeat_timeoutCount: " + heartbeat_timeoutCount + "\n");
		return sb.toString();
	}

}

