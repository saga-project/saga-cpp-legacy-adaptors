/*
 * $RCSfile: RemoteMachineInfoKeys.java,v $ $Revision: 1.11 $ $Date: 2008/03/16 03:26:02 $
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

/*
 * This class has keys of RemoteMachineInfo.
 */
public final class RemoteMachineInfoKeys {

	private RemoteMachineInfoKeys() {}

	// keys for RemoteMachineInformation
	public static final String HOSTNAME = "hostname";
	public static final String TAG = "tag";
	public static final String PORT = "port";
	public static final String MPI_NCPUS = "mpi_runNoOfCPUs";
	public static final String JOBMANAGER = "jobmanager";
	public static final String SUBJECT = "subject";

	// why not JOB_
	public static final String QUEUE = "job_queue";
	public static final String PROJECT = "job_project";
	public static final String HOSTCOUNT = "job_hostCount";
	public static final String MINMEMORY = "job_minMemory";
	public static final String MAXMEMORY = "job_maxMemory";
	public static final String MAXTIME = "job_maxTime";
	public static final String MAXWALLTIME = "job_maxWallTime";
	public static final String MAXCPUTIME = "job_maxCpuTime";

	public static final String JOB_STARTTIMEOUT = "job_startTimeout";
	public static final String JOB_STOPTIMEOUT = "job_stopTimeout";

	public static final String HEARTBEAT = "heartbeat";
	public static final String HEARTBEAT_TIMEOUTCOUNT
		= "heartbeat_timeoutCount";
	public static final String REDIRECT_OUTERR = "redirect_outerr";
	public static final String ARG_TRANS = "argument_transfer";
	public static final String COMPRESS = "compress";
	public static final String COMPRESS_THRESHOLD = "compress_threshold";

	public static final String COMMLOG_ENABLE = "commLog_enable";
	public static final String COMMLOG_FILEPATH = "commLog_filePath";
	public static final String COMMLOG_SUFFIX = "commLog_suffix";
	public static final String COMMLOG_NFILES = "commLog_nFiles";
	public static final String COMMLOG_MAXFILESIZE = "commLog_maxFileSize";
	public static final String COMMLOG_OVERWRITEDIR
		= "commLog_overwriteDirectory";

	public static final String WORK_DIR = "workDirectory";
	public static final String CORE_SIZE = "coreDumpSize";

	public static final String DEBUG = "debug";
	public static final String DEBUG_DISPLAY = "debug_display";
	public static final String DEBUG_TERM = "debug_terminal";
	public static final String DEBUG_DEBUGGER = "debug_debugger";
	public static final String DEBUG_BUSYLOOP = "debug_busyLoop";

	public static final String ENVIRONMENT = "environment";
	public static final String TCP_NODELAY = "tcp_nodelay";
	public static final String BLOCK_SIZE = "argument_blockSize";

	public static final String TCP_CONNECT_RETRY_COUNT
		= "tcp_connect_retryCount";
	public static final String TCP_CONNECT_RETRY_BASEINTERVAL
		= "tcp_connect_retryBaseInterval";
	public static final String TCP_CONNECT_RETRY_INCREASERATIO
		= "tcp_connect_retryIncreaseRatio";
	public static final String TCP_CONNECT_RETRY_RANDOM
		= "tcp_connect_retryRandom";

	public static final String KEEP_CONNECTION = "keep_connection";

	public static final String INVOKE_SERVER = "invoke_server";
	public static final String INVOKE_SERVER_OPTION = "invoke_server_option";
	public static final String CLIENT_HOSTNAME = "client_hostname";
	public static final String JOB_RSL_EXTENSION = "job_rslExtensions";

	public static final String COMMUNICATION_PROXY_TYPE =
		"communication_proxy";
        public static final String COMMUNICATION_PROXY_STAGING =
                "communication_proxy_staging";
        public static final String COMMUNICATION_PROXY_PATH =
                "communication_proxy_path";
        public static final String COMMUNICATION_PROXY_BUFFER_SIZE =
                "communication_proxy_buffer_size";
	public static final String COMMUNICATION_PROXY_OPTION =
		"communication_proxy_option";
}

