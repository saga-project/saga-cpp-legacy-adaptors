/*
 * $RCSfile: ServerSectionParser.java,v $ $Revision: 1.14 $ $Date: 2008/03/16 03:26:02 $
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
package org.apgrid.grpc.ng.config;

import java.util.List;

import org.apgrid.grpc.ng.NgConfigSection;
import org.apgrid.grpc.ng.NgConfigEntry;
import org.apgrid.grpc.ng.NgConfigException;
import static org.apgrid.grpc.ng.RemoteMachineInfoKeys.*;

final class ServerSectionParser extends SectionParser {
	private static final String name = "SERVER";

	public ServerSectionParser() {
	}

	/* (non-Javadoc) 
	 * @see org.apgrid.grpc.ng.SectionParser#checkEntry
	 */
	boolean checkEntry(List<NgConfigEntry> entries, NgConfigEntry ent)
	 throws NgConfigException {
		if (ent == null) {
			throw new NgConfigException("Argument NgConfigEntry is null");
		}

		String key = ent.getKey();
		if (! isAttribute(key)) {
			throw new NgConfigException("\"" + key
										+ "\" is not attribute of " 
										+ name + " section.");
		}

		if ( isRedefine(entries, ent) ) {
			throw new NgConfigException("attribute \"" + key + "\" redefined");
		}

		// check the port
		if (key.equals("port")) {
			return checkPort(ent);
		}

		// check the attribute starts with "job_"
		if (key.startsWith("job_")) {
			return checkJob(ent);
		}

		// check the heartbeat
		if (key.equals("heartbeat")) {
			return checkHeartbeat(ent);
		}

		// check the heartbeat timeout count (& on transfer)
		if (key.startsWith("heartbeat_timeout")) {
			return checkHeartbeatTimeoutCount(ent);
		}

		// check the redirect outerror
		if (key.equals("redirect_outerr")) {
			return checkRedirectOuterr(ent);
		}

		// check the tcp nodelay
		if (key.equals("tcp_nodelay")) {
			return checkTcpNodelay(ent);
		}

		// check the tcp connect
		if (key.startsWith("tcp_connect")) {
			return checkTcpConnect(ent);
		}

		// check the keep connection
		if (key.equals(KEEP_CONNECTION)) {
			return checkKeepConnection(ent);
		}

		// check the argument transfer
		if (key.equals("argument_transfer")) {
			return checkArgumentTransfer(ent);
		}

		// check the compress
		if (key.equals("compress")) {
			return checkCompress(ent);
		}

		// check the compress_threshold
		if (key.equals("compress_threshold")) {
			return checkCompressThreshold(ent);
		}

		// check the argument block size
		if (key.equals("argument_blockSize")) {
			return checkArgumentBlockSize(ent);
		}

		// check the communication logs
		if (key.startsWith("commLog_")) {
			return checkCommLog(ent);
		}

		// check the debug
		if (key.equals("debug")) {
			return checkDebug(ent);
		}

		// check the debug busy-loop
		if (key.equals("debug")) {
			return checkDebugBusyLoop(ent);
		}

		return true;
	}

	String getName() {
		return this.name;
	}

	String [] getAttributes() {
		return this.attributes;
	}

	String [] getRequiredAttributes() {
		return this.required_attributes;
	}

	private boolean isRedefine(final List<NgConfigEntry> entries,
	 final NgConfigEntry ent) {

		if ( isPermittedMultipleDefine(ent.getKey()) ) { return false; }
		if (contains(entries, ent) < 0) {
			return false;
		}
		return true;
	}

	private boolean isPermittedMultipleDefine(String key) {
		if (key.equals("hostname") ||
                        key.equals("communication_proxy_option") ||
			key.equals("invoke_server_option") ||
			key.equals("mpi_runNoOfCPUs") ||
			key.equals("environment") ) {
			return true;
		}
		return false;
	}

/////////////////////////////////////////////////
/////     Check Methods begin               /////
/////////////////////////////////////////////////

	private boolean checkPort(NgConfigEntry port) throws NgConfigException {
		EntryChecker.checkNegativeNumber(port);
		return true;
	}

	private boolean checkJob(final NgConfigEntry job)
	 throws NgConfigException {
		String key = job.getKey();
		if (key.endsWith("startTimeout")) {
			return checkJobStartTimeout(job);
		}
		if (key.endsWith("stopTimeout")) {
			return checkJobStopTimeout(job);
		}
		if (key.endsWith("maxTime") ||
			key.endsWith("maxWallTime") ||
			key.endsWith("maxCpuTime")) {
			return checkJobMaxTime(job);
		}
		if (key.endsWith("hostCount")) {
			return checkJobHostCount(job);
		}
		if (key.endsWith("minMemory") ||
			key.endsWith("maxMemory")) {
			return checkJobMemory(job);
		}

		return true;
	}

	private boolean checkJobStartTimeout(final NgConfigEntry job_startTimeout) 
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(job_startTimeout);
		return true;
	}

	private boolean checkJobStopTimeout(final NgConfigEntry job_stopTimeout)
	 throws NgConfigException {
		EntryChecker.checkInteger(job_stopTimeout);
		return true;
	}

	private boolean checkJobMaxTime(final NgConfigEntry job_maxTime)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(job_maxTime);
		return true;
	}

	private boolean checkJobHostCount(final NgConfigEntry job_hostCount)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(job_hostCount);
		return true; 
	}

	private boolean checkJobMemory(final NgConfigEntry job_memory)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(job_memory);
		return true;
	}

	private boolean checkHeartbeat(final NgConfigEntry heartbeat)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(heartbeat);
		return true;
	}

	private boolean checkHeartbeatTimeoutCount(final NgConfigEntry heartbeat_timeoutCount)
	 throws NgConfigException {
		String key = heartbeat_timeoutCount.getKey();
		if (key.endsWith("OnTransfer")) {
			return checkHeartbeatTimeoutCountOnTransfer(heartbeat_timeoutCount);
		}
		EntryChecker.checkNegativeNumber(heartbeat_timeoutCount);
		return true;
	}

	private boolean checkHeartbeatTimeoutCountOnTransfer(final NgConfigEntry heartbeat_timeoutCountOnTransfer)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(heartbeat_timeoutCountOnTransfer);
		return true;
	}

	private boolean checkRedirectOuterr(final NgConfigEntry redirect_outerr)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(redirect_outerr);
		return true;
	}

	private boolean checkTcpNodelay(final NgConfigEntry tcp_nodelay)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(tcp_nodelay);
		return true;
	}

	private boolean checkTcpConnect(final NgConfigEntry tcp_connect)
	 throws NgConfigException {
		String key = tcp_connect.getKey();
		if (key.endsWith("retryCount")) {
			return checkTcpConnectRetryCount(tcp_connect);
		}
		if (key.endsWith("retryBaseInterval")) {
			return checkTcpConnectRetryBaseInterval(tcp_connect);
		}
		if (key.endsWith("retryIncreaseRatio")) {
			return checkTcpConnectRetryIncreaseRatio(tcp_connect);
		}
		if (key.endsWith("retryRandom")) {
			return checkTcpConnectRetryRandom(tcp_connect);
		} 
		return true;
	}

	private boolean checkTcpConnectRetryCount(final NgConfigEntry tcp_connect_retryCount) 
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(tcp_connect_retryCount);
		return true; 
	}


	private boolean checkTcpConnectRetryBaseInterval(final NgConfigEntry tcp_connect_retryBaseInterval)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(tcp_connect_retryBaseInterval);
		return true; 
	}

	private boolean checkTcpConnectRetryIncreaseRatio(final NgConfigEntry tcp_connect_retryIncreaseRatio)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(tcp_connect_retryIncreaseRatio);
		return true; 
	}

	private boolean checkTcpConnectRetryRandom(final NgConfigEntry tcp_connect_retryRandom)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(tcp_connect_retryRandom);
		return true;
	}

	private boolean checkKeepConnection(final NgConfigEntry keep_connection)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(keep_connection);
		return true;
	}

	private boolean checkArgumentTransfer(final NgConfigEntry argument_transfer)
	 throws NgConfigException {
		String value = argument_transfer.getValue();
		if (value.equals("wait") || 
			value.equals("nowait") ||
			value.equals("copy") ) {
			return true;
		}
		throw new NgConfigException("Invalid value " + argument_transfer);
	}

	private boolean checkCompress(final NgConfigEntry compress)
	 throws NgConfigException {
		String value = compress.getValue();
		if (value.equals("raw") ||
			value.equals("zlib")) {
			return true;
		}
		throw new NgConfigException("Invalid value " + compress);	
	}

	private boolean checkCompressThreshold(final NgConfigEntry compress_threshold)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(compress_threshold);
		return true;
	}

	private boolean checkArgumentBlockSize(final NgConfigEntry argument_blockSize)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(argument_blockSize);
		return true; 
	}

	private boolean checkCommLog(final NgConfigEntry commLog)
	 throws NgConfigException {
		String key = commLog.getKey();
		if (key.endsWith("enable")) {
			return checkCommLogEnable(commLog);
		}
		if (key.endsWith("nFiles")) {
			return checkCommLogNumFiles(commLog);
		}
		if (key.endsWith("maxFileSize")) {
			return checkCommLogMaxFileSize(commLog);
		}
		if (key.endsWith("overwriteDirectory")) {
			return checkCommLogOverwriteDirectory(commLog);
		}

		return true;
	}

	private boolean checkCommLogEnable(final NgConfigEntry commLog_enable)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(commLog_enable);
		return true;
	}

	private boolean checkCommLogNumFiles(final NgConfigEntry commLog_nFiles)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(commLog_nFiles);
		return true;
	}

	private boolean checkCommLogMaxFileSize(final NgConfigEntry commLog_maxFileSize)
	 throws NgConfigException {
		EntryChecker.checkNegativeNumber(commLog_maxFileSize);
		return true; 
	}

	private boolean checkCommLogOverwriteDirectory(final NgConfigEntry commLog_overwriteDirectory)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(commLog_overwriteDirectory);
		return true; 
	}

	private boolean checkDebug(final NgConfigEntry debug)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(debug);
		return true; 
	}

	private boolean checkDebugBusyLoop(final NgConfigEntry debug_busyLoop)
	 throws NgConfigException {
		EntryChecker.checkTypeBoolean(debug_busyLoop);
		return true;
	}

/////////////////////////////////////////////////
/////     Check Methods end                 /////
/////////////////////////////////////////////////


	private static final String [] required_attributes = {
		"hostname"
	};

	private static final String [] attributes = {
		"hostname",
		"tag",
		"port",
		"invoke_server",
		"invoke_server_option",
		"mpi_runNoOfCPUs",
		"jobmanager",
		"subject",
		"client_hostname",
		"job_startTimeout",
		"job_stopTimeout",
		"job_maxTime",
		"job_maxWallTime",
		"job_maxCpuTime",
		"job_queue",
		"job_project",
		"job_hostCount",
		"job_minMemory",
		"job_maxMemory",
		"job_rslExtensions",
		"heartbeat",
		"heartbeat_timeoutCount",
		"heartbeat_timeoutCountOnTransfer",
		"redirect_outerr",
		"tcp_nodelay",
		"tcp_connect_retryCount",
		"tcp_connect_retryBaseInterval",
		"tcp_connect_retryIncreaseRatio",
		"tcp_connect_retryRandom",
		KEEP_CONNECTION,
		"argument_transfer",
		"compress",
		"compress_threshold",
		"argument_blockSize",
		"workDirectory",
		"coreDumpSize",
		"commLog_enable",
		"commLog_filePath",
		"commLog_suffix",
		"commLog_nFiles",
		"commLog_maxFileSize",
		"commLog_overwriteDirectory",
		"debug",
		"debug_display",
		"debug_terminal",
		"debug_debugger",
		"debug_busyLoop",
		"environment",
                "communication_proxy",
                "communication_proxy_staging",
                "communication_proxy_path",
                "communication_proxy_buffer_size",
                "communication_proxy_option"
	};
}
