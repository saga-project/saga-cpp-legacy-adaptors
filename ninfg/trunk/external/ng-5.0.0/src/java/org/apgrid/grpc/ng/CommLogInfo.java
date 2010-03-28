/*
 * $RCSfile: CommLogInfo.java,v $ $Revision: 1.3 $ $Date: 2008/02/07 08:17:43 $
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
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_ENABLE;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_FILEPATH;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_SUFFIX;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_NFILES;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_MAXFILESIZE;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.COMMLOG_OVERWRITEDIR;

/*
 * Information for RemoteMachineInfo class
 */
public class CommLogInfo {
	private String commLog_enable = null;
	private String commLog_filePath = null;
	private String commLog_suffix = null;
	private String commLog_nFiles = null;
	private String commLog_maxFileSize = null;
	private String commLog_overwriteDirectory = null;


	/**
	 * CommLogInfo Constructor
	 */
	public CommLogInfo() {
		setDefaultParameter();
	}
	
	/**
	 * @param prop
	 */
	public CommLogInfo(Map<Object, Object> param) {
		setDefaultParameter();
		update(param);
	}

	/*
	 * initialize the fields
	 */
	private final void setDefaultParameter() {
		this.commLog_enable = "false";
		this.commLog_nFiles = "1";
		this.commLog_overwriteDirectory = "false";

	}

	public String getCommlogEnable() {
    	return this.commLog_enable;
	} 
	public String getCommlogFilepath() {
    	return this.commLog_filePath;
	} 
	public String getCommlogSuffix() {
    	return this.commLog_suffix;
	} 
	public String getCommlogNfiles() {
    	return this.commLog_nFiles;
	} 
	public String getCommlogMaxfilesize() {
    	return this.commLog_maxFileSize;
	} 
	public String getCommlogOverwritedir() {
    	return this.commLog_overwriteDirectory;
	}

	
	/**
	 * @param  param
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
		if ( aKey.equals(COMMLOG_ENABLE) ) {
			this.commLog_enable = value;
			return;
		} else if ( aKey.equals(COMMLOG_FILEPATH) ) {
			this.commLog_filePath = value;
			return;
		} else if ( aKey.equals(COMMLOG_SUFFIX) ) {
			this.commLog_suffix = value;
			return;
		} else if ( aKey.equals(COMMLOG_NFILES) ) {
			this.commLog_nFiles = value;
			return;
		} else if ( aKey.equals(COMMLOG_MAXFILESIZE) ) {
			this.commLog_maxFileSize = value;
			return;
		} else if ( aKey.equals(COMMLOG_OVERWRITEDIR) ) {

			this.commLog_overwriteDirectory = value;
			return;
		}
		throw new IllegalArgumentException(aKey);
	}

	public static CommLogInfo copy(CommLogInfo other) {
		CommLogInfo result = new CommLogInfo();
		result.commLog_enable = other.commLog_enable;
		result.commLog_filePath = other.commLog_filePath;
		result.commLog_suffix = other.commLog_suffix;
		result.commLog_nFiles = other.commLog_nFiles;
		result.commLog_maxFileSize = other.commLog_maxFileSize;
		result.commLog_overwriteDirectory = other.commLog_overwriteDirectory;
		return result;
	}

	public void reset() {
		resetAllFields();
		setDefaultParameter();
	}

	private final void resetAllFields() {
		this.commLog_enable = null;
		this.commLog_filePath = null;
		this.commLog_suffix = null;
		this.commLog_nFiles = null;
		this.commLog_maxFileSize = null;
		this.commLog_overwriteDirectory = null;

	}
	
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("commLog_enable: " + commLog_enable + "\n");
		sb.append("commLog_filePath: " + commLog_filePath + "\n");
		sb.append("commLog_suffix: " + commLog_suffix + "\n");
		sb.append("commLog_nFiles: " + commLog_nFiles + "\n");
		sb.append("commLog_maxFileSize: " + commLog_maxFileSize + "\n");
		sb.append("commLog_overwriteDirectory: " + commLog_overwriteDirectory + "\n");
		return sb.toString();
	}
}

