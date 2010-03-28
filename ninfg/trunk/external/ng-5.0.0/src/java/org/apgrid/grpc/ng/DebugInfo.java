/*
 * $RCSfile: DebugInfo.java,v $ $Revision: 1.2 $ $Date: 2007/09/26 04:14:07 $
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
import java.util.Set;

// import the constant (Java 5.0+ feature)
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.DEBUG;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.DEBUG_DISPLAY;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.DEBUG_TERM;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.DEBUG_DEBUGGER;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.DEBUG_BUSYLOOP;

/*
 * Information for RemoteMachineInfo
 */
public class DebugInfo {
	private String debug = null;
	private String debug_display = null;
	private String debug_terminal = null;
	private String debug_debugger = null;
	private String debug_busyLoop = null;

	/**
	 * DebugInfo Constructor
	 */
	public DebugInfo() {
		setDefaultParameter();
	}
	
	/**
	 * @param param
	 */
	public DebugInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.debug = "false";
		this.debug_busyLoop = "false";
	}


	public String getDebug() {
		return this.debug;
	} 
	public String getDebugDisplay() {
		return this.debug_display;
	} 
	public String getDebugTerm() {
    	return this.debug_terminal;
	} 
	public String getDebugDebugger() {
    	return this.debug_debugger;
	} 
	public String getDebugBusyloop() {
    	return this.debug_busyLoop;
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
		if ( aKey.equals(DEBUG) ) {
			this.debug = value;
			return;
		} else if ( aKey.equals(DEBUG_DISPLAY) ) {
			this.debug_display = value;
			return;
		} else if ( aKey.equals(DEBUG_TERM) ) {
			this.debug_terminal = value;
			return;
		} else if ( aKey.equals(DEBUG_DEBUGGER) ) {
			this.debug_debugger = value;
			return;
		} else if ( aKey.equals(DEBUG_BUSYLOOP) ) {
			this.debug_busyLoop = value;
			return;
		}
		throw new IllegalArgumentException(aKey);
	}

	public static DebugInfo copy(DebugInfo other) {
		DebugInfo result = new DebugInfo();
		result.debug = other.debug;
		result.debug_display = other.debug_display;
		result.debug_terminal = other.debug_terminal;
		result.debug_debugger = other.debug_debugger;
		result.debug_busyLoop = other.debug_busyLoop;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.debug = null;
		this.debug_display = null;
		this.debug_terminal = null;
		this.debug_debugger = null;
		this.debug_busyLoop = null;
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("debug: " + debug + "\n");
		sb.append("debug_display: " + debug_display + "\n");
		sb.append("debug_terminal: " + debug_terminal + "\n");
		sb.append("debug_debugger: " + debug_debugger + "\n");
		sb.append("debug_busyLoop: " + debug_busyLoop + "\n");
		return sb.toString();
	}
}

