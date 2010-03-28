/*
 * $RCSfile: InformationSource.java,v $ $Revision: 1.9 $ $Date: 2008/03/25 05:39:07 $
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

import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.BlockingQueue;
import java.io.File;

import org.apgrid.grpc.util.NgUtil;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;

/*
 * Representation of <INFORMATION_SOURCE>
 */
public class InformationSource {
	private NgInformationManager infoMng = null;
	private String type         = null;
	private String [] sources   = null;
	private String path         = null;
	private String tag          = null;
	private String log_file     = null;
	private List<String> option = null;
	private long timeout;

	private InformationService isvc = null;
	private NgLog logger = null;

        private int requestedIds;

	// copy from InformationService
	private static final String DEFAULT_NAME = "ng_information_service";
	private static final int DEFAULT_TIMEOUT = 0;

	private final String logHead; // Log Header String

	public InformationSource(
		NgInformationManager infoMng, NgConfigSection config, NgLog log)
	throws NgException {
	 	this.infoMng = infoMng;
		this.logger = log;
	 	update(config);

		//Map<String, String> attrs = new HashMap<String, String>();
		//attrs.put("path", path);
		//attrs.put("timeout", String.valueOf(timeout));
		//attrs.put("log_filePath", log_file);

		// create InformationService object
		//this.isvc = new InformationService(context, type, attrs, option, logger);
		//this.isvc.setContainer(this);

		StringBuilder sb = new StringBuilder(type);
		if (tag != null)
			sb.append("/").append(tag);
		sb.append(": ");
		this.logHead = sb.toString();
		sb = null;

		this.logger.logInfo(CAT_NG_INTERNAL,
			logHead + "created InformationSource " + toString());
	}

	protected void update(NgConfigSection config) throws NgException {
		this.type = config.get("type").getValue();

		List<String> tmp = new ArrayList<String>();
		for (NgConfigEntry ent: config) {
			if (ent.getKey().equals("source"))
				tmp.add(ent.getValue());
		}
		this.sources = new String[tmp.size()];
		for (int i = 0; i < sources.length; i++) {
			this.sources[i] = tmp.get(i);
		}

		tmp = null;
		this.path     = getPath(config);
		this.timeout  = getTimeout(config);
		this.log_file = getLogFilePath(config);
		this.tag      = getTag(config);

		tmp = new ArrayList<String>();
		for (NgConfigEntry ent : config) {
			if (ent.getKey().equals("option"))
				tmp.add(ent.getValue());
		}
		this.option = tmp;
		tmp = null;

		Map<String, String> attrs = new HashMap<String, String>();
		attrs.put("path", path);
		attrs.put("timeout", String.valueOf(timeout));
		attrs.put("log_filePath", log_file);

		// create InformationService object
		this.isvc = newInformationService(attrs);

		//this.requestedIds = new int[sources.length];
		this.logger.logInfo(CAT_NG_INTERNAL,
			logHead + "updated InformationSource " + toString());
	}

	protected String getType() {
		return type;
	}

	private InformationService newInformationService(Map<String, String> attrs)
	throws NgException {
		InformationService iSvc = 
			new InformationService(
				this.infoMng, this.type, attrs, this.option, this.logger);
		iSvc.setContainer(this);
		NotifyThreadManager notifyThread = new NotifyThreadManager();
		notifyThread.setExtModule(iSvc);
		notifyThread.setNgLog(this.logger);
		notifyThread.startThread();
		return iSvc;
	}

	private String getPath(NgConfigSection config) {
		NgConfigEntry ent = config.get("path");
		if (ent == null)
			return NgUtil.getDefaultPath(DEFAULT_NAME, this.type);
		return ent.getValue();
	}

	private long getTimeout(NgConfigSection config) {
		NgConfigEntry ent = config.get("timeout");
		if (ent == null)
			return DEFAULT_TIMEOUT; 
		try {
			return Long.parseLong(ent.getValue());
		} catch (NumberFormatException e) {
			return DEFAULT_TIMEOUT; 
		}
	}

	private String getLogFilePath(NgConfigSection config) {
		NgConfigEntry ent = config.get("log_filePath");
		if (ent == null) return null;
		return ent.getValue();
	}

	private String getTag(NgConfigSection config) {
		NgConfigEntry ent = config.get("tag");
		if (ent == null) return null;
		return ent.getValue();
	}

	// getter
	public String getTag() {
		return this.tag;
	}

	protected long getTimeout() {
		return this.timeout;
	}

//// tentative

	// interface 
	protected synchronized void query(String classname, String hostname)
	throws NgException {
		String id;
		logger.logInfo(CAT_NG_INTERNAL, logHead + "Query Information");
		id = this.isvc.queryRemoteExecutableInformation(
			classname, hostname, sources);
		try {
			requestedIds = Integer.parseInt(id);
			logger.logInfo(CAT_NG_INTERNAL,
				logHead + "Query Information Request Id [" + id + "]");
		} catch (NumberFormatException e) {
			logger.logError(CAT_NG_INTERNAL, logHead + e.getMessage());
			throw new NgException(e.getMessage());
		}
	}


	// interface for PriorityInformationContainer
	private BlockingQueue<REIEntry> container = null;
	void setContainer(BlockingQueue<REIEntry> cont) {
		this.container = cont;
	}

	// interface for InformationService
	void put(String qid, String result, String rei) {
		try  {
			boolean res;
			if (result.charAt(0) == 'S') {
				res = true;
			} else {
				res = false;
			}
			REIEntry data = new REIEntry(Integer.parseInt(qid), res, rei);
			this.container.put(data);
		} catch (NumberFormatException e) {
			logger.logError(CAT_NG_INTERNAL, logHead +  e.getMessage());
		} catch (InterruptedException e) {
			logger.logError(CAT_NG_INTERNAL, logHead + e.getMessage());
		}
	}

    	boolean requested(int qid) {
		if (requestedIds == qid)
			return true;
		return false;
	}


	public boolean valid() {
		throw new UnsupportedOperationException("not implemented");
	}

	public int nSources() {
		return this.sources.length;
	}

	public void exit() {
		this.isvc.exit();
	}

//// tentative

	public String toString() {
		return "Type: " + type + " Sources: " + toString_sources();
	}

	private String toString_sources() {
		StringBuilder sb = new StringBuilder(sources[0]);
		for (int i = 1; i < sources.length; i++) {
			sb.append("," + sources[i]);
		}
		return sb.toString();
	}

}

