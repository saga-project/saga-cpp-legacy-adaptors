/*
 * $RCSfile: ClientCommunicationProxyInfo.java,v $ $Revision: 1.3 $ $Date: 2008/03/28 03:25:55 $
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

package org.apgrid.grpc.ng.info;

import java.util.List;
import java.util.ArrayList;
import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;

public class ClientCommunicationProxyInfo {

	private static final String TYPE = "type";
	private static final String PATH = "path";
	private static final String LOG_FILEPATH = "log_filePath";
	private static final String MAX_JOBS = "max_jobs";
	private static final String BUFFER_SIZE = "buffer_size";
	private static final String OPTION = "option";

	private static final int DEFAULT_BUFF_SIZE = 8192; // 8KB
	private static final int DEFAULT_MAX_JOBS  = 0;    // unlimited

	private String type = null;
	private String path = null;
	private int max_jobs = DEFAULT_MAX_JOBS;
	private int buffer_size = DEFAULT_BUFF_SIZE;
	private String log_filePath = null;
	private List<String> option = new ArrayList<String>();

	/*
	 * Constructor 
	 * 
	 * @param type of Client Communication Proxy
	 */
	public ClientCommunicationProxyInfo(String type) {
		if ((type == null) || (type.length() == 0)) {
			throw new IllegalArgumentException();
		}
		this.type = type;
	}

	/*
	 * Constructor 
	 *
	 * @param
	 */
	public ClientCommunicationProxyInfo(NgConfigSection config) {
		for (NgConfigEntry ent : config) {
			String key   = ent.getKey();
			String value = ent.getValue();
			if (key.equals(TYPE)) {
				this.type = value;
			} else if (key.equals(PATH)) {
				this.path = value;
			} else if (key.equals(LOG_FILEPATH)) {
				this.log_filePath = value;
			} else if (key.equals(MAX_JOBS)) {
				this.max_jobs = Integer.parseInt(value);
			} else if (key.equals(BUFFER_SIZE)) {
				this.buffer_size = Integer.parseInt(value);
			} else if (key.equals(OPTION)) {
				this.option.add(value);
			}
		}
		if (this.type == null) {
			throw new IllegalArgumentException();
		}
	}
	
	public String getType() { return this.type; }
	public String getPath() { return this.path; }
	public int getMaxJobs() { return this.max_jobs; }
	public int getBufferSize() { return this.buffer_size; }
	public String getLogFilePath() { return this.log_filePath; }
	public List<String> getOption() { return this.option; }

}

