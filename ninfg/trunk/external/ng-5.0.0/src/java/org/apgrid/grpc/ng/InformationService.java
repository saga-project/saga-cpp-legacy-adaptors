/*
 * $RCSfile: InformationService.java,v $ $Revision: 1.14 $ $Date: 2008/03/25 05:39:07 $
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
import java.io.IOException;
import java.io.LineNumberReader;
import java.io.File;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;

/*
 * Representation of a Information Service.
 */
class InformationService implements ExtModule {

	private NgInformationManager infoMng = null;
	// type of Information Service
	private String type = null;
	// Information Service's option
	private List<String> option = null;
	private int timeout = 0;
	private NgLog logger = null;
	private ProcessCommunicator process = null;
	private boolean exit_requested = false;
	private InformationSource isrc = null;

	private static final String KEY_PATH    = "path";
	private static final String KEY_LOGFILE = "log_filePath";
	private static final String KEY_TIMEOUT = "timeout";

	private static final String QUERY_REI
		= "QUERY_REMOTE_EXECUTABLE_INFORMATION";
	private static final String REI_NOTIFY
		= "REMOTE_EXECUTABLE_INFORMATION_NOTIFY";
	private static final String CANCEL
		= "CANCEL_QUERY";

	private static final int DEFAULT_TIMEOUT = 0;


	/**
	 * Constructor
	 * 
	 * @param type Information Service Type
	 * @param optional_attr Optional Information Service Attributes
	 * @param option Information Service Option
	 * @param logger Logging class
	 */
	public InformationService(
		NgInformationManager infoMng, String type,
		Map<String, String> optional_attr,
		List<String> option, NgLog logger)
	throws NgException {
		this.infoMng = infoMng;
		this.type   = type;
		this.option = option;
		this.logger = logger;

		String timeout = optional_attr.get(KEY_TIMEOUT);
		this.timeout = getTimeout(timeout);
		String [] cmdarr = createCommandLine(optional_attr);
		try {
			this.process =
				new ProcessCommunicator(cmdarr);
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	private String [] createCommandLine(Map<String, String> attribute) {
		String path  = attribute.get(KEY_PATH);
		if (path == null)
			throw new NullPointerException("path is null");
		String logfile = attribute.get(KEY_LOGFILE);
		if (logfile == null) {
			logfile = infoMng.getLocalMachineInfo().getInformationServiceLog();
			if (logfile != null) {
				logfile = logfile + "." + this.type;
			}
		}
		String [] ret;
		if (logfile == null) {
			// only command
			ret = new String[1];
			ret[0] = path;
		} else {
			// command & log file argument
			ret = new String[3];
			ret[0] = path;
			ret[1] = "-l";
			ret[2] = logfile;
		}
		return ret;
	}

	private int getTimeout(String tm) {
		try {
			if (tm != null)
				return Integer.parseInt(tm);
		} catch (NumberFormatException e) {
		}
		return DEFAULT_TIMEOUT; 
	}

	void setContainer(InformationSource source) {
		this.isrc = source;
	}

	/**
	 * Request QUERY_FEATURES
	 * 
	 * @return Strings("name val") of reply. User should parse string.
	 */
	public List<String> queryFeatures() throws NgException {
		try  {
			ExtModuleRequest req = ExtModuleRequest.QUERY_FEATURES;
			logger.logDebug(CAT_NG_INTERNAL, "Request is " + req);

			ExtModuleReply rep = process.send( req );

			logger.logInfo(CAT_NG_INTERNAL,
				"Send QUERY_FEATURES request to Information Service");
			logger.logInfo(CAT_NG_INTERNAL,
				"Receive QUERY_FEATURES reply from Information Service");
			logger.logDebug(CAT_NG_INTERNAL, "Reply is " + rep);
			if ( !rep.result().equals("SM") ) {
				throw new NgException(req + " error. returned " + rep);
			}
			return rep.returnValues();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/**
	 * Request QUERY_REMOTE_EXECUTABLE_INFORMATION
	 *
	 * @param classname Class name
	 * @param hostname Host name
	 * @param source Information source
	 * @return Query Id
	 */
	public String queryRemoteExecutableInformation(String classname,
	 String hostname, String[] sources)
	 throws NgException {
		try {
			ExtModuleRequest req =
				buildQueryREIRequest(classname, hostname, sources);
			logger.logDebug(CAT_NG_INTERNAL, "Request is \n" + req);

			ExtModuleReply rep = process.send(req);

			logger.logInfo(CAT_NG_INTERNAL,
				"Send QUERY_REMOTE_EXECUTABLE_INFORMATION request to Information Service");
			logger.logInfo(CAT_NG_INTERNAL,
				"Receive QUERY_REMOTE_EXECUTABLE_INFORMATION reply from Information Service");
			logger.logDebug(CAT_NG_INTERNAL, "Reply is " + rep);
			if ( ! isReplySuccess(rep) ) {
				throw new NgException(QUERY_REI + " error. returned " + rep);
			}

			return rep.returnValue();
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/*
	 * build Request String for QUERY_REMOTE_EXECUTABLE_INFORMATION
	 */
	private ExtModuleRequest buildQueryREIRequest(String classname,
	 String hostname, String[] sources) {
		ExtModuleRequest req = ExtModuleRequest.issuesMultiple(QUERY_REI);
		req.addAttribute("classname", classname);
		req.addAttribute("hostname", hostname);
		for (int i = 0; i < sources.length; i++) {
			req.addAttribute("source", sources[i]);
		}

		for (String opt : option) {
			req.addAttribute(opt);
		}
		return req;
	}

	/**
	 * Request CANCEL_QUERY
	 * 
	 * @param query_id Return by QUERY_REMOTE_EXECUTABLE_INFORMATION
	 */
	public void cancelQuery(int query_id) throws NgException {
		try {
			ExtModuleRequest req =
				ExtModuleRequest.issuesSingle(CANCEL, String.valueOf(query_id));

			ExtModuleReply rep = process.send(req);

			logger.logInfo(CAT_NG_INTERNAL,
				"Send CANCEL_QUERY request to Information Service");

			if ( ! isReplySuccess(rep) ) {
				logger.logError(CAT_NG_INTERNAL,
					req + " error. returned " + rep);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}
	}

	/*
	 * @return true reply success
	 * @return false reply failed
	 */
	private boolean isReplySuccess(ExtModuleReply rep) {
		return rep.result().equals("S");
	}

	/**
	 * Request EXIT
	 */
	public void exit() {
		ExtModuleRequest req = ExtModuleRequest.EXIT;
		logger.logDebug(CAT_NG_INTERNAL, "Request is " + req);
		try {
			
			ExtModuleReply rep = process.send(req);
			logger.logInfo(CAT_NG_INTERNAL,
				"Send EXIT request to Information Service");

			logger.logInfo(CAT_NG_INTERNAL,
				"Receive EXIT reply from Information Service");
			logger.logDebug(CAT_NG_INTERNAL, "Reply is " + rep);
		} catch (IOException e) {
			logger.logError(CAT_NG_INTERNAL, e.getMessage());
		}
		exit_requested = true;
		process.exit();
	}

	private String find(String key, List<NgAttribute> values) {
		for (NgAttribute attr : values) {
			if (attr.getName().equals(key))
				return attr.getValue();
		}
		return null;
	}

	private String getRemoteExecInfo(List<NgAttribute> values) {
		StringBuilder sb = new StringBuilder();
		for (NgAttribute attr : values) {
			if (attr.getName().equals("remote_executable_information")) {
				sb.append(attr.getValue()).append("\n");
			}
		}
		return sb.toString();
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append(" Type:").append(type);
		sb.append(" Timeout:").append(timeout);
		sb.append(" Options:").append(option);
		return sb.toString();
	}

	public ExtModuleNotify createNotify() throws IOException {
		if (exit_requested) {
			logger.logInfo(NgLog.CAT_NG_INTERNAL, "EXIT requested.");
			return null;
		}
		LineNumberReader reader = process.getNotifyReader();
		String line = reader.readLine();
		if (line == null) {
			if (! exit_requested)
				logger.logInfo(NgLog.CAT_NG_INTERNAL, "reached EOF");
			return null;
		} 

		if (line.equals(RemoteExecutableInformationNotify.NOTIFY_NAME)) {
			return new RemoteExecutableInformationNotify(this, reader);
		} else {
			logger.logError(NgLog.CAT_NG_INTERNAL, "");
			throw new IOException("unknown notify received");
		}
	}

	/*
	 * Register the notify result
	 *
	 * @param info a notify parameter of REMOTE_EXECUTABLE_INFORMATION_NOTIFY
	 */
	public void register(List<NgAttribute> info) {
		String query_id = find("query_id", info);
		String result   = find("result", info);
		String val = null;
		if ( result.equals("F") ) {
			val = find("error_message", info);
		} else {
			val = getRemoteExecInfo(info);
		}
		logger.logInfo(CAT_NG_INTERNAL, 
			"Register Remote Executable Information to Queue.");
		isrc.put(query_id, result, val);
	}

	static class RemoteExecutableInformationNotify
	implements ExtModuleNotify {

		private static final String NOTIFY_NAME =
			"REMOTE_EXECUTABLE_INFORMATION_NOTIFY";
		private InformationService infoService;
		private List<NgAttribute> param;

		public RemoteExecutableInformationNotify(InformationService iSvc,
		                                         LineNumberReader reader)
		throws IOException {
			this.infoService = iSvc;
			this.param = ExtModuleNotifyParser.parse(reader, NOTIFY_NAME);
		}

		public void handle() {
			this.infoService.register(param);
		}

	}

}

