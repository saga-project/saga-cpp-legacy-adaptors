/* 
 * $RCSfile: ng.h,v $ $Revision: 1.33 $ $Date: 2008/03/27 08:41:36 $
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
#ifndef _NG_H_
#define _NG_H_

/**
 * This file define the Data Structures and Constant Values for user of
 * Pure Ninf-G.
 */

#include "ngInternal.h"

/**
 * Argument Stack.
 */
#define ngclArgumentStack_s ngiArgumentStack_s
#define ngclArgumentStack_t ngiArgumentStack_t

/**
 * Information about Local Machine.
 */
typedef struct ngclLocalMachineInformation_s {
    char       *nglmi_hostName;		/* Host name of local machine */
    int		nglmi_saveNsessions;	/* Number of sessions to save */
    ngLogInformation_t nglmi_logInfo;	/* Log Information */
    ngLogLevelInformation_t nglmi_logLevels; /* Log Levels */
    char       *nglmi_tmpDir;           /* Temporary directory */
    int		nglmi_refreshInterval;	/* Interval of Refresh the proxy cert */
    char       *nglmi_invokeServerLog;  /* Invoke Server log file name */
    char       *nglmi_commProxyLog;	/* Client Comm. Proxy log */
    char       *nglmi_infoServiceLog;	/* Info. Service log */
    int		nglmi_fortranCompatible;/* Fortran compatible */
    int        *nglmi_signals;		/* signals */
    int         nglmi_listenPort;       /* Listen port */
} ngclLocalMachineInformation_t;

/**
 * Information about Invoke Server.
 */
typedef struct ngclInvokeServerInformation_s {
    char        *ngisi_type;		/* type */
    char        *ngisi_path;		/* path */
    int          ngisi_maxJobs;		/* max jobs */
    char        *ngisi_logFilePath;	/* log file path */
    int          ngisi_statusPoll;      /* status polling interval */
    int          ngisi_nOptions;        /* Num of Options */
    char       **ngisi_options;		/* Options */
} ngclInvokeServerInformation_t;

/**
 * Information about Communication Proxy.
 */
typedef struct ngclCommunicationProxyInformation_s {
    char        *ngcpi_type;		/* type */
    char        *ngcpi_path;		/* path */
    int          ngcpi_bufferSize;	/* buffer size */
    int          ngcpi_maxJobs;		/* max jobs */
    char        *ngcpi_logFilePath;	/* log file path */
    int          ngcpi_nOptions;        /* Num of Options */
    char       **ngcpi_options;		/* Options */
} ngclCommunicationProxyInformation_t;

/**
 * Information about Information Service.
 */
typedef struct ngclInformationServiceInformation_s {
    char        *ngisi_tag;		/* tag */
    char        *ngisi_type;		/* type */
    char        *ngisi_path;		/* path */
    char        *ngisi_logFilePath;	/* log file path */
    int          ngisi_timeout;		/* timeout */
    int          ngisi_nSources;        /* Num of Sources */
    char       **ngisi_sources;		/* Sources */
    int          ngisi_nOptions;        /* Num of Options */
    char       **ngisi_options;		/* Options */
} ngclInformationServiceInformation_t;

/*
 * Container of mpi_runNoOfCPUs in <SERVER> section
 */
typedef struct ngclMPInCPUsInformation_s {
    char                                  *ngmni_className;
    int                                    ngmni_nCPUs;
    SLIST_ENTRY(ngclMPInCPUsInformation_s) ngmni_entry;
} ngclMPInCPUsInformation_t;

SLIST_HEAD(ngclMPInCPUsInformationList_s, ngclMPInCPUsInformation_s);
typedef struct ngclMPInCPUsInformationList_s ngclMPInCPUsInformationList_t;

/**
 * Information about Remote Machine.
 */
typedef struct ngclRemoteMachineInformation_s {
    /* Information about Remote Machine */
    char	*ngrmi_hostName;	/* Host name */
    char	*ngrmi_tagName;		/* Tag name */
    int		ngrmi_portNo;		/* Port number */

    /* Information about Invoke Server */
    char	*ngrmi_invokeServerType; /* Invoke Server Type */
    int		ngrmi_invokeServerNoptions; /* Invoke Server Num of Options */
    char	**ngrmi_invokeServerOptions;/* Invoke Server Options */

    /* Information about Remote Communication Proxy */
    char	*ngrmi_commProxyType;               /* type */
    int		ngrmi_commProxyStaging;             /* staging */
    char	*ngrmi_commProxyPath;               /* path */
    int		ngrmi_commProxyBufferSize;          /* buffer size */
    int		ngrmi_commProxyNoptions;            /* Num of Options */
    char	**ngrmi_commProxyOptions;           /* Options */

    /* Information about Information Service */
    char	*ngrmi_infoServiceTag; /* Tag */

    /* Information about MPI */
    int                           ngrmi_mpiNcpus;    /* Number of CPUs */
    ngclMPInCPUsInformationList_t ngrmi_mpiNcpusList;/* Number of CPUs each function */

    /* Information about Protocol */
    int		ngrmi_keepConnect;	/* Keep connection */
    int		ngrmi_forceXDR;		/* Force XDR */

    /* Information about Job Manager */
    char	*ngrmi_jobManager;	/* Job manager to be use */
    char	*ngrmi_subject;		/* Subject to be use */
    char	*ngrmi_clientHostName;	/* Client Host name */
    int		ngrmi_jobStartTimeout;	/* Timeout at the time of Job start */
    int		ngrmi_jobEndTimeout;	/* Timeout at the time of Job end */
    int		ngrmi_jobMaxTime;	/* GRAM RSL: maxTime */
    int		ngrmi_jobMaxWallTime;	/* GRAM RSL: maxWallTime */
    int		ngrmi_jobMaxCpuTime;	/* GRAM RSL: maxCpuTime */
    char	*ngrmi_jobQueue;	/* GRAM RSL: queue */
    char	*ngrmi_jobProject;	/* GRAM RSL: project */
    int		ngrmi_jobHostCount;	/* GRAM RSL: hostCount */
    int		ngrmi_jobMinMemory;     /* GRAM RSL: minMemory */
    int		ngrmi_jobMaxMemory;	/* GRAM RSL: maxMemory */
    int		ngrmi_rslExtensionsSize; /* WS GRAM RSL Extensions */
    char	**ngrmi_rslExtensions;  /* WS GRAM RSL Extensions */

    /* Information about Heart Beat */
    int		ngrmi_heartBeat;	/* Interval of heart beat */
    int		ngrmi_heartBeatTimeoutCount;	/* Timeout count */

    /* Information about others */
    int		ngrmi_redirectEnable;	/* Redirect from remote machine */
    int		ngrmi_tcpNodelay;	/* TCP_NODELAY option */
    ngiConnectRetryInformation_t ngrmi_retryInfo; /* Connect Retry */
    ngArgumentTransfer_t	ngrmi_argumentTransfer;

    /* Information about Compression */
    ngCompressionType_t		ngrmi_compressionType;
    int	ngrmi_compressionThresholdNbytes;	/* Number of bytes of threshold */

    /* Information about division for argument/result data. */
    int ngrmi_argumentBlockSize;

    /* Information about working directory */
    char *ngrmi_workDirectory;

    /* Information about core dump size */
    int ngrmi_coreDumpSize;

    /* Information about Communication Log */
    int                ngrmi_commLogEnable;
    ngLogInformation_t ngrmi_commLogInfo;

    /* Information about Debugger */
    ngDebuggerInformation_t ngrmi_debug;

    /* Information about busy loop for debugger attach */
    int ngrmi_debugBusyLoop;

    /* Information about Environment Variable */
    int ngrmi_nEnvironments;	/* Number of Environment Variables */
    char **ngrmi_environment;	/* Environment Variables */
} ngclRemoteMachineInformation_t;

/**
 * Information about Ninf-G Executable Path.
 */
typedef struct ngclExecutablePathInformation_s {
    char *ngepi_hostName;	/* Host name */
    char *ngepi_className;	/* Class name of remote class */
    char *ngepi_path;		/* Path of Ninf-G Executable at remote machine */
    int ngepi_stagingEnable;	/* staging */
    ngBackend_t ngepi_backend;	/* Backend */
    int ngepi_sessionTimeout;   /* Session Timeout */
    int ngepi_transferTimeout_argument;   /* Transfer Timeout : argument */
    int ngepi_transferTimeout_result;     /* Transfer Timeout : result */
    int ngepi_transferTimeout_cbArgument; /* Transfer Timeout : cb argument */
    int ngepi_transferTimeout_cbResult;   /* Transfer Timeout : cb result */
} ngclExecutablePathInformation_t;

/**
 * Information for managing Session.
 */
/* Status of Session */
typedef enum ngclSessionStatus_e {
    NG_SESSION_STATUS_INITIALIZED,
    NG_SESSION_STATUS_WAIT_CONNECT,
    NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE,
    NG_SESSION_STATUS_INVOKE_REQUESTED,
    NG_SESSION_STATUS_INVOKE_DONE,
    NG_SESSION_STATUS_TRANSARG_REQUESTED,
    NG_SESSION_STATUS_TRANSARG_DONE,
    NG_SESSION_STATUS_CALCULATE_EXECUTING,
    NG_SESSION_STATUS_CALCULATE_DONE,
    NG_SESSION_STATUS_SUSPEND_REQUESTED,
    NG_SESSION_STATUS_SUSPEND_DONE,
    NG_SESSION_STATUS_RESUME_REQUESTED,
    NG_SESSION_STATUS_RESUME_DONE,
    NG_SESSION_STATUS_CANCEL_REQUESTED,
    NG_SESSION_STATUS_CANCEL_DONE,
    NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED,
    NG_SESSION_STATUS_CB_TRANSARG_REQUESTED,
    NG_SESSION_STATUS_CB_TRANSARG_DONE,
    NG_SESSION_STATUS_CB_EXECUTING,
    NG_SESSION_STATUS_CB_EXECUTE_DONE,
    NG_SESSION_STATUS_CB_TRANSRES_REQUESTED,
    NG_SESSION_STATUS_CB_TRANSRES_DONE,
    NG_SESSION_STATUS_TRANSRES_REQUESTED,
    NG_SESSION_STATUS_TRANSRES_DONE,
    NG_SESSION_STATUS_PULLBACK_REQUESTED,
    NG_SESSION_STATUS_PULLBACK_DONE,
    NG_SESSION_STATUS_DONE
} ngclSessionStatus_t;

/* Status of Heart Beat */
typedef enum ngclHeartBeatStatus_e {
    NG_HEART_BEAT_STATUS_OK,
    NG_HEART_BEAT_STATUS_WARNING,	/* It has not received several times */
    NG_HEART_BEAT_STATUS_ERROR		/* It has not received */
} ngclHeartBeatStatus_t;

/**
 * Attribute of Session
 */ 
typedef struct ngclSessionAttribute_s {
    /* Wait to transfer argument, when this value is true */
    ngArgumentTransfer_t        ngsa_waitArgumentTransfer;
    int				ngsa_timeout;
    int				ngsa_transferTimeout_argument;
    int				ngsa_transferTimeout_result;
    int				ngsa_transferTimeout_cbArgument;
    int				ngsa_transferTimeout_cbResult;
} ngclSessionAttribute_t;

#define NGCL_SESSION_ATTRIBUTE_MEMBER_UNDEFINED -1

/* Information for managing Session */
typedef struct ngclSession_s {
    /* Link list of Session */
    struct ngclSession_s	*ngs_next;	/* for internal */
    struct ngclSession_s	*ngs_apiNext;	/* for API */
    struct ngclSession_s	*ngs_waitNext;	/* for waiting */
    struct ngclSession_s	*ngs_cancelNext;	/* for canceling */

    /* The list of sessions which execution ended */
    struct ngclSession_s	*ngs_doneNext;

    struct ngclContext_s	*ngs_context;	/* Ninf-G Context */
    struct ngclExecutable_s	*ngs_executable; /* Executable */

    /*
     * User defined data.
     *
     * If ngs_userDestructer is NULL, ngclSessionDestruct() will free()
     * ngs_userData. Otherwise, ngclSessionDestruct() will not operate
     * ngs_userData.
     */
    void	*ngs_userData;
    void	(*ngs_userDestructer)(struct ngclSession_s *);
    /* End of user defined data */

    /* Mutex and condition for this instance */
    ngiMutex_t	ngs_mutex;
    ngiCond_t	ngs_cond;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngs_rwlOwn;

    int	ngs_ID;		/* ID number of this session */
    ngclSessionStatus_t	ngs_status;	/* Status of Session */
    int ngs_cancelRequest; /* The flag of cancel request */
    int	ngs_error;	/* Error code */
    int	ngs_cbError;	/* Error code, it occurred in callback function */
 
    int ngs_wasFailureNotified; /* Was failure notified to user? */

    /* Status of Heart Beat */
    ngclHeartBeatStatus_t ngs_heartBeatStatus;

    /* Session Timeout */
    int    ngs_timeout;
    time_t ngs_timeoutTime;

    /* Transfer Timeout */
    int ngs_transferTimeout_argument;
    time_t ngs_transferTimeoutTime_argument;
    int ngs_transferTimeout_result;
    time_t ngs_transferTimeoutTime_result;
    int ngs_transferTimeout_cbArgument;
    time_t ngs_transferTimeoutTime_cbArgument;
    int ngs_transferTimeout_cbResult;
    time_t ngs_transferTimeoutTime_cbResult;

    /* Argument Data */
    struct ngiArgument_s *ngs_arg;

    /* Remote Method Information */
    int ngs_rmInfoExist;
    ngRemoteMethodInformation_t ngs_rmInfo;

    /* Session Information */
    ngSessionInformation_t ngs_info;

    /* wait Transfer Argument */
    ngArgumentTransfer_t ngs_waitArgumentTransfer;

    /* Execution time */
    struct {
    	ngExecutionTime_t ngs_transferArgument;
    	ngExecutionTime_t ngs_transferFileClientToRemote;
    	ngExecutionTime_t ngs_calculation;
    	ngExecutionTime_t ngs_transferResult;
    	ngExecutionTime_t ngs_transferFileRemoteToClient;

    	ngExecutionTime_t ngs_callbackTransferArgument;
    	ngExecutionTime_t ngs_callbackCalculation;
    	ngExecutionTime_t ngs_callbackTransferResult;

	int ngs_callbackNtimesCalled;
	struct timeval ngs_sumCallbackTransferArgumentReal;
	struct timeval ngs_sumCallbackTransferArgumentCPU;
	struct timeval ngs_sumCallbackCalculationReal;
	struct timeval ngs_sumCallbackCalculationCPU;
	struct timeval ngs_sumCallbackTransferResultReal;
	struct timeval ngs_sumCallbackTransferResultCPU;
    } ngs_executionTime;
} ngclSession_t;

/**
 * Attribute of Executable Handle
 */
typedef struct ngclExecutableAttribute_s {
    char       *ngea_hostName;		/* Host name of remote machine */
    int		ngea_portNo;		/* Port number of remote machine */
    char       *ngea_jobManager;	/* Jobmanager of remote machine */
    char       *ngea_subject;		/* Subject of remote machine */
    char       *ngea_className;		/* Class name of remote class */
    int		ngea_invokeNjobs;	/* The number of JOBs which invoke */
    /* Timeout time at the time of a JOB start/stop (second) */
    int		ngea_jobStartTimeout;
    int		ngea_jobStopTimeout;
    /* Wait to transfer argument, when this value is true */
    ngArgumentTransfer_t	ngea_waitArgumentTransfer;
    char       *ngea_queueName;         /* Queue name of remote machine */
    int         ngea_mpiNcpus;          /* Number of CPUs */
} ngclExecutableAttribute_t;

#define NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED -1
#define NGCL_EXECUTABLE_ATTRIBUTE_JOB_STOP_TIMEOUT_WAIT_FOREVER -2

/**
 * Information for managing Executable.
 */
/* Status of Executable */
typedef enum ngclExecutableStatus_e {
    NG_EXECUTABLE_STATUS_INITIALIZED,
    NG_EXECUTABLE_STATUS_CONNECTING,
    NG_EXECUTABLE_STATUS_CONNECTED,
    NG_EXECUTABLE_STATUS_IDLE,
    NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST,
    NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED,
    NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_DONE,
    NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_REQUESTED,
    NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE,
    NG_EXECUTABLE_STATUS_RESET_REQUESTED,
    NG_EXECUTABLE_STATUS_RESET_DONE,
    NG_EXECUTABLE_STATUS_EXIT_REQUESTED,
    NG_EXECUTABLE_STATUS_EXIT_DONE,
    NG_EXECUTABLE_STATUS_DONE
} ngclExecutableStatus_t;

/* Executable Handle */
typedef struct ngclExecutable_s {
    /* Link list of Executable */
    struct ngclExecutable_s	*nge_next;	/* for Ninf-G Context */
    struct ngclExecutable_s	*nge_jobNext;	/* for job */
    struct ngclExecutable_s	*nge_apiNext;	/* for API */
    struct ngclExecutable_s	*nge_multiHandleNext; /* for array_init */

    /* Management data for array of Executable */
    struct ngclExecutable_s	*nge_top;	/* Top of this array */

    struct ngclContext_s	*nge_context;	/* Ninf-G Context */
    struct ngcliJobManager_s	*nge_jobMng;	/* Job Manager */
    struct ngiProtocol_s	*nge_protocol;	/* Protocol */

    /* Link list of Session */
    ngclSession_t	*nge_session_head;
    ngclSession_t	*nge_session_tail;

    /* Number of sessions */
    struct {
	ngiMutex_t	ngens_mutex;
	ngiCond_t	ngens_cond;
	int		ngens_nSessions;
    } nge_nSessions;

    /*
     * User defined data.
     *
     * If ngs_userDestructer is NULL, ngclExecutableDestruct() will
     * free() ngs_userData. Otherwise, ngclExecutableDestruct() will not
     * operate ngs_userData.
     */
    void       *nge_userData;
    void      (*nge_userDestructer)(struct ngclExecutable_s *);
    /* End of user defined data */

    /* Mutex, condition and Read/Write lock for this instance */
    ngiMutex_t	nge_mutex;
    ngiCond_t	nge_cond;
    ngiRWlock_t	nge_rwlOwn;

    /* Host name of Remote Machine */
    char *nge_hostName;
    char *nge_tagName;

    /* Communication Log */
    int nge_commLogEnable;
    ngLogInformation_t nge_commLogInfo;
    ngCommLog_t *nge_commLog;

    /* Remote Class Information */
    char *nge_rcInfoName;
    int nge_rcInfoExist;
    ngRemoteClassInformation_t	nge_rcInfo;

    int	nge_ID;		/* Identification number of Executable */
    ngclExecutableStatus_t nge_status;		/* Status */
    int nge_locked;	/* Any body locked this instance */
    int nge_sending;	/* Any body sending the request */
    ngArgumentTransfer_t nge_waitArgumentTransfer;

    int	nge_error;	/* Error code */
    int	nge_cbError;	/* Error code, it occurred in callback function */
    int nge_requestResult;	/* The result of Protocol Request */

    /* Is I/O Callback running or registered */
    ngiIOhandleCallbackWaiter_t nge_ioCallbackWaiter;

    /* HeartBeat control */
    int nge_heartBeatInterval;  /* Heart Beat Interval (second) */
    int nge_heartBeatTimeout; /* Heart Beat Timeout (second) */
    time_t nge_heartBeatLastReceived; /* Last Heart Beat received time */
    ngclHeartBeatStatus_t nge_heartBeatStatus;	/* Status of Heart Beat */

    int nge_sessionTimeout;  /* Session Timeout */
    int nge_transferTimeout_argument;  /* Transfer Timeout */
    int nge_transferTimeout_result;  /* Transfer Timeout */
    int nge_transferTimeout_cbArgument;  /* Transfer Timeout */
    int nge_transferTimeout_cbResult;  /* Transfer Timeout */

    time_t nge_jobStartTimeoutTime; /* Job Start Timeout Time */

    /* Keep Connection */
    int nge_keepConnect;
    int nge_connecting;
    int nge_connectionCloseRequested;

    /* Execution time */
    struct {
	ngExecutionTime_t nge_queryRemoteMachineInformation;
	ngExecutionTime_t nge_queryRemoteClassInformation;
    } nge_executionTime;
} ngclExecutable_t;

/**
 * List for managing Executable.
 */
typedef struct ngclExecutableList_s {
    ngclExecutable_t	*ngel_head;
    ngclExecutable_t	*ngel_tail;
} ngclExecutableList_t;

/**
 * Information about Ninf-G Executable
 */
typedef struct ngclExecutableInformation_s {
    char *ngei_ninfgProtocolVersion; /* Version number of Ninf-G protocol */
    char *ngei_ninfgVersion;         /* Version number of Ninf-G */
} ngclExecutableInformation_t;

/**
 * Information for managing Local Machine Information.
 */
typedef struct ngcliLocalMachineInformationManager_s {
    /* Read/Write Lock for this instance */
    ngiRWlock_t	nglmim_rwlOwn;

    /* Information for Local Machine */
    ngclLocalMachineInformation_t	nglmim_info;
} ngcliLocalMachineInformationManager_t;

/**
 * Information for managing Invoke Server Information.
 */
typedef struct ngcliInvokeServerInformationManager_s {
    /* Link list of Invoke Server Information */
    struct ngcliInvokeServerInformationManager_s  *ngisim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngisim_rwlOwn;

    /* Information about Invoke Server */
    ngclInvokeServerInformation_t	ngisim_info;
} ngcliInvokeServerInformationManager_t;

/**
 * Information for managing Communication Proxy Information.
 */
typedef struct ngcliCommunicationProxyInformationManager_s {
    /* Link list of Communication Proxy Information */
    struct ngcliCommunicationProxyInformationManager_s  *ngcpim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngcpim_rwlOwn;

    /* Information about Communication Proxy */
    ngclCommunicationProxyInformation_t	ngcpim_info;
} ngcliCommunicationProxyInformationManager_t;

/**
 * Information for managing Information Service Information.
 */
typedef struct ngcliInformationServiceInformationManager_s {
    /* Link list of Information Service Information */
    struct ngcliInformationServiceInformationManager_s  *ngisim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngisim_rwlOwn;

    /* Information about Information Service */
    ngclInformationServiceInformation_t	ngisim_info;
} ngcliInformationServiceInformationManager_t;

/**
 * Information for managing Ninf-G Executable Path.
 */
typedef struct ngcliExecutablePathInformationManager_s {
    /* Link list of Executable Path Information */
    struct ngcliExecutablePathInformationManager_s	*ngepim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngepim_rwlOwn;

    int ngepim_active;

    /* Information about Ninf-G Executable Path */
    ngclExecutablePathInformation_t	ngepim_info;
} ngcliExecutablePathInformationManager_t;

/**
 * Information for managing Remote Machine Information .
 */
typedef struct ngcliRemoteMachineInformationManager_s {
    /* Link list of Remote Machine Information */
    struct ngcliRemoteMachineInformationManager_s	*ngrmim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t	ngrmim_rwlOwn;

    /* Information about Remote Machine */
    ngclRemoteMachineInformation_t	ngrmim_info;
} ngcliRemoteMachineInformationManager_t;

/**
 * Information for managing Remote Class Information.
 */
typedef struct ngcliRemoteClassInformationManager_s {
    /* Link list of Remote Class Information */
    struct ngcliRemoteClassInformationManager_s  *ngrcim_next;

    /* Read/Write Lock for this instance */
    ngiRWlock_t      ngrcim_rwlOwn;

    int ngrcim_active;

    /* Information about Remote Class */
    ngRemoteClassInformation_t	ngrcim_info;
} ngcliRemoteClassInformationManager_t;

/**
 * Heartbeat.
 */
typedef struct ngcliHeartBeatCheck_s {
    ngiIOhandle_t *nghbc_timeHandle;
    time_t nghbc_eventTime;
    int nghbc_interval;
    int nghbc_changeRequested;
    int nghbc_eventExecuted;
} ngcliHeartBeatCheck_t;

/**
 * Ninf-G Context
 */
typedef struct ngclContext_s {
    /* Link list for Ninf-G Context */
    struct ngclContext_s	*ngc_next;
    struct ngclContext_s	*ngc_apiNext;

    /*
     * User defined data.
     *
     * If ngc_userDestructer is NULL, ngclContextDestruct() will free()
     * ngc_userData. Otherwise, ngclContextDestruct() will not operate
     * ngc_userData.
     */
    void	*ngc_userData;
    void	(*ngc_userDestructer)(struct ngclContext_s *);
    /* End of user defined data */

    ngiRWlock_t	ngc_rwlOwn;	/* Read/Write Lock for this instance */

    /* Event */
    ngiEvent_t	*ngc_event;

    /* Communication Manager */
    struct ngiCommunication_s	*ngc_comm;	/* Communication */

    /* Protocol Manager */
    struct ngiProtocol_s	*ngc_proto;	/* Listen Protocol */
    ngiIOhandleCallbackWaiter_t  ngc_protoCallbackWaiter;

    ngLog_t	*ngc_log;	/* Data for managing log */
    int ngc_ID;		/* Identification number of this instance */
    int	ngc_error;	/* Error code */
    int	ngc_cbError;	/* Error code, it occurred in callback function */

    /* Information about Local Machine */
    ngiRWlock_t	ngc_rwlLmInfo;	/* Read/Write Lock for the following lists */
    ngcliLocalMachineInformationManager_t	*ngc_lmInfo;

    /* Information about Invoke Server */
    ngiRWlock_t	ngc_rwlInvokeServerInfo;    /* Read/Write Lock for the lists */
    ngcliInvokeServerInformationManager_t	*ngc_invokeServerInfo_head;
    ngcliInvokeServerInformationManager_t	*ngc_invokeServerInfo_tail;

    /* Information about Communication Proxy */
    ngiRWlock_t	ngc_rwlCpInfo;	/* Read/Write Lock for the following lists */
    ngcliCommunicationProxyInformationManager_t	*ngc_cpInfo_head;
    ngcliCommunicationProxyInformationManager_t	*ngc_cpInfo_tail;

    /* Information about Information Service */
    ngiRWlock_t	ngc_rwlInfoServiceInfo;	/* Read/Write Lock for the lists */
    ngcliInformationServiceInformationManager_t	*ngc_infoServiceInfo_head;
    ngcliInformationServiceInformationManager_t	*ngc_infoServiceInfo_tail;

    /* Information about Remote Machine */
    ngiRWlock_t	ngc_rwlRmInfo;	/* Read/Write Lock for the following lists */
    ngcliRemoteMachineInformationManager_t	*ngc_rmInfo_default;
    ngcliRemoteMachineInformationManager_t	*ngc_rmInfo_head;
    ngcliRemoteMachineInformationManager_t	*ngc_rmInfo_tail;

    /* Information about Executable Path */
    ngiRWlock_t	ngc_rwlEpInfo;	/* Read/Write Lock for the following lists */
    ngcliExecutablePathInformationManager_t	*ngc_epInfo_head;
    ngcliExecutablePathInformationManager_t	*ngc_epInfo_tail;

    /* Information about Remote Class */
    ngiRWlock_t	ngc_rwlRcInfo;	/* Read/Write Lock for the following lists */
    ngcliRemoteClassInformationManager_t	*ngc_rcInfo_head;
    ngcliRemoteClassInformationManager_t	*ngc_rcInfo_tail;

    /* Information for managing Job */
    int			ngc_nJobs;	/* The number of existing jobs */
    ngiRWlock_t	ngc_rwlJobMng;	/* Read/Write Lock for the following lists */
    int			ngc_jobID;	/* Job ID */
    struct ngcliJobManager_s	*ngc_jobMng_head;
    struct ngcliJobManager_s	*ngc_jobMng_tail;

    /* External Module */
    ngiExternalModuleManager_t *ngc_externalModuleManager;

    /* Invoke Server */
    int			ngc_nInvokeServers;
    ngiRWlock_t	ngc_rwlInvokeMng;/* Read/Write Lock for the following lists */
    struct ngcliInvokeServerManager_s *ngc_invokeMng_head;
    struct ngcliInvokeServerManager_s *ngc_invokeMng_tail;

    /* Communication Proxy */
    struct ngcliCommunicationProxyManager_s *ngc_communicationProxyManager;

    /* Information Service */
    struct ngcliQueryManager_s *ngc_queryManager;

    /* Executable Handle */
    ngiRWlock_t	ngc_rwlExecutable; /* Read/Write Lock for following data */
    int	ngc_nExecutables;	/* The number of existing executables */
    int	ngc_executableID;	/* Identification number of Executable */
    ngclExecutable_t	*ngc_executable_head;
    ngclExecutable_t	*ngc_executable_tail;
    ngclExecutable_t	*ngc_destruction_executable_head;
    ngclExecutable_t	*ngc_destruction_executable_tail;

    ngiMutex_t	ngc_mutexExecutable;
    ngiCond_t	ngc_condExecutable;
    int ngc_flagExecutable;

    /* Information for managing Session */
    ngiRWlock_t	ngc_rwlSession;	/* Read/Write Lock for following data */
    int	ngc_nSessions;		/* The number of existing sessions */
    int	ngc_sessionID;		/* Identification number of Session */
    ngiRWlock_t	ngc_rwlSessionList; /* Read/Write Lock for list of all Sessions */

    ngiMutex_t	ngc_mutexSession;
    ngiCond_t	ngc_condSession;
    int ngc_flagSession;
#if 0
/* Note: examination
 * Context has no link to session.
 * Session is owned by Executable.
 */
    ngclSession_t	*ngc_session_head;
    ngclSession_t	*ngc_session_tail;
#endif

    /* Configuration file */
    ngiMutex_t ngc_mutexConfig;
    int		ngc_configFileReadCount;
    int		ngc_configFileReading;

    /* HeartBeat */
    ngcliHeartBeatCheck_t ngc_heartBeatCheck;

    /* SessionTimeout */
    ngiIOhandle_t *ngc_sessionTimeoutHandle;

    /* TransferTimeout */
    ngiIOhandle_t *ngc_transferTimeoutHandle;

    /* Job Start Timeout */
    ngiIOhandle_t *ngc_jobStartTimeoutHandle;

    /* Random Number Status */
    ngiRandomNumber_t ngc_randomStatus;

} ngclContext_t;

/**
 * Define the Lock/Unlock macros.
 */
/* Ninf-G Context */
#define ngclContextReadLock(context, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlOwn, (context)->ngc_log, (error));
#define ngclContextReadUnlock(context, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlOwn, (context)->ngc_log, (error));
#define ngclContextWriteLock(context, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlOwn, (context)->ngc_log, (error));
#define ngclContextWriteUnlock(context, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlOwn, (context)->ngc_log, (error));

#define ngclExecutableListReadLock(context, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlExecutable, (context)->ngc_log, (error));
#define ngclExecutableListReadUnlock(context, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlExecutable, (context)->ngc_log, (error));
#define ngclExecutableListWriteLock(context, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlExecutable, (context)->ngc_log, (error));
#define ngclExecutableListWriteUnlock(context, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlExecutable, (context)->ngc_log, (error));

/* Executable Handle */
#define ngclExecutableReadLock(executable, error) \
    ngiRWlockReadLock(&(executable)->nge_rwlOwn, (executable)->nge_context->ngc_log, (error));
#define ngclExecutableReadUnlock(executable, error) \
    ngiRWlockReadUnlock(&(executable)->nge_rwlOwn, (executable)->nge_context->ngc_log, (error));
#define ngclExecutableWriteLock(executable, error) \
    ngiRWlockWriteLock(&(executable)->nge_rwlOwn, (executable)->nge_context->ngc_log, (error));
#define ngclExecutableWriteUnlock(executable, error) \
    ngiRWlockWriteUnlock(&(executable)->nge_rwlOwn, (executable)->nge_context->ngc_log, (error));

/* Session */
#define ngclSessionListReadLock(context, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlSession, (context)->ngc_log, (error));
#define ngclSessionListReadUnlock(context, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlSession, (context)->ngc_log, (error));
#define ngclSessionListWriteLock(context, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlSession, (context)->ngc_log, (error));
#define ngclSessionListWriteUnlock(context, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlSession, (context)->ngc_log, (error));

/* Session */
#define ngclSessionReadLock(session, error) \
    ngiRWlockReadLock(&(session)->ngs_rwlOwn, (session)->ngs_context->ngc_log, (error));
#define ngclSessionReadUnlock(session, error) \
    ngiRWlockReadUnlock(&(session)->ngs_rwlOwn, (session)->ngs_context->ngc_log, (error));
#define ngclSessionWriteLock(session, error) \
    ngiRWlockWriteLock(&(session)->ngs_rwlOwn, (session)->ngs_context->ngc_log, (error));
#define ngclSessionWriteUnlock(session, error) \
    ngiRWlockWriteUnlock(&(session)->ngs_rwlOwn, (session)->ngs_context->ngc_log, (error));

/**
 * Prototype declaration of APIs for user.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Ninf-G Manager */
int ngclNinfgManagerSetSignalHandler(
    int, void (*)(int), ngLog_t *, int *);
int ngclNinfgManagerYieldForCallback(ngclContext_t *, int *);

/* Ninf-G Context */
ngclContext_t *ngclContextConstruct(char *, int *);
int ngclContextDestruct(ngclContext_t *, int *);
int ngclContextConfigurationFileRead(ngclContext_t *, char *, int *);
int ngclContextRegisterUserData(
    ngclContext_t *, void *, void (*)(ngclContext_t *), int *);
int ngclContextUnregisterUserData(ngclContext_t *, int *);
int ngclContextGetUserData(ngclContext_t *, void **, int *);
int ngclContextGetError(ngclContext_t *, int *);

/* Local Machine Information */
int ngclLocalMachineInformationGetCopy(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngclLocalMachineInformationRelease(
    ngclContext_t *context, ngclLocalMachineInformation_t *lmInfo, int *error);

/* Invoke Server Information */
int ngclInvokeServerInformationGetCopy(
    ngclContext_t *, char *, ngclInvokeServerInformation_t *, int *);
int ngclInvokeServerInformationRelease(
    ngclContext_t *, ngclInvokeServerInformation_t *, int *);

/* Communication Proxy Information */
int ngclCommunicationProxyInformationGetCopy(
    ngclContext_t *, char *, ngclCommunicationProxyInformation_t *, int *);
int ngclCommunicationProxyInformationRelease(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);

/* Information Service Information */
int ngclInformationServiceInformationGetCopy(
    ngclContext_t *, char *, ngclInformationServiceInformation_t *, int *);
int ngclInformationServiceInformationRelease(
    ngclContext_t *, ngclInformationServiceInformation_t *, int *);

/* Remote Machine Information */
int ngclRemoteMachineInformationGetCopy(
    ngclContext_t *, char *, ngclRemoteMachineInformation_t *, int *);
int ngclDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngclRemoteMachineInformationRelease(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);

/* Executable Path Information */
int ngclExecutablePathInformationGetCopy(
    ngclContext_t *, char *, char *, ngclExecutablePathInformation_t *, int *);
int ngclExecutablePathInformationRelease(
    ngclContext_t *, ngclExecutablePathInformation_t *, int *);

/* Remote Class Information */
int ngclRemoteClassInformationGetCopy(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, char *,
    ngRemoteClassInformation_t *, int *);
int ngclRemoteClassInformationRelease(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);

/* Executable */
ngclExecutable_t *ngclExecutableConstruct(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
int ngclExecutableDestruct(ngclExecutable_t *, int, int *);
int ngclExecutableInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *, int *);
int ngclExecutableFinalize(ngclExecutable_t *, int *, int *);
int ngclExecutableRegister(ngclContext_t *, ngclExecutable_t *, int, int *);
int ngclExecutableUnregister(ngclExecutable_t *, int *);
int ngclExecutableRegisterUserData(
    ngclExecutable_t *, void *, void (*)(ngclExecutable_t *), int *);
int ngclExecutableUnregisterUserData(ngclExecutable_t *, int *);
int ngclExecutableGetUserData(ngclExecutable_t *, void **, int *);
int ngclExecutableGetError(ngclExecutable_t *, int *);

int ngclExecutableAttributeInitialize(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
int ngclExecutableAttributeFinalize(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);

int ngclExecutableUserListInitialize(
    ngclContext_t *, ngclExecutableList_t *, int *);
int ngclExecutableUserListFinalize(
    ngclContext_t *, ngclExecutableList_t *, int *);
int ngclExecutableUserListRegister(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
int ngclExecutableUserListUnregister(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
ngclExecutable_t *ngclExecutableUserListGetNext(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
ngclExecutable_t *ngclExecutableMultiHandleListGetNext(
    ngclContext_t *, ngclExecutable_t *, int *);

int ngclExecutableGetCopy(ngclContext_t *, int, ngclExecutable_t *, int *);
int ngclExecutableCopy(
    ngclContext_t *, ngclExecutable_t *, ngclExecutable_t *, int *);
int ngclExecutableRelease(ngclContext_t *, ngclExecutable_t *, int *);
ngclExecutable_t * ngclExecutableGet(ngclContext_t *, int, int *);
ngclExecutable_t * ngclExecutableGetNext(
    ngclContext_t *, ngclExecutable_t *, int *);
ngclSession_t * ngclExecutableGetSession(ngclExecutable_t *, int, int *);
int ngclExecutableGetInformation(
    ngclExecutable_t *, ngclExecutableInformation_t *, int *);
int ngclExecutableReset(ngclExecutable_t *, int *);

int ngclExecutableIsIdle(ngclContext_t *, ngclExecutable_t *, int *);
int ngclExecutableGetID(ngclExecutable_t *, int *);


/* Session */
ngclSession_t *ngclSessionConstruct(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);
int ngclSessionDestruct(ngclSession_t *, int *);
ngclSession_t *ngclSessionAllocate(ngclContext_t *, ngclExecutable_t *, int *);
int ngclSessionFree(ngclSession_t *, int *);
int ngclSessionInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngclSessionAttribute_t *, int *);
int ngclSessionFinalize(ngclSession_t *, int *);
int ngclSessionRegister(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionUnregister(ngclSession_t *, int *);
int ngclSessionUnregisterWithoutLock(ngclSession_t *, int *);
int ngclSessionRegisterUserData(
    ngclSession_t *, void *, void (*)(ngclSession_t *), int *);
int ngclSessionUnregisterUserData(ngclSession_t *, int *);
int ngclSessionGetUserData(ngclSession_t *, void **, int *);
ngclExecutable_t *ngclSessionGetExecutable(ngclSession_t *, int *);
ngclSession_t *ngclSessionGetList(
    ngclContext_t *, ngclExecutable_t *, int *, int, int *);
int ngclSessionReleaseList(ngclSession_t *, int *);
ngclSession_t *ngclSessionGet(
    ngclContext_t *, ngclExecutable_t *, int , int *);
ngclSession_t *ngclSessionGetNext(ngclExecutable_t *, ngclSession_t *, int *);
ngclSession_t *ngclSessionGetCancelList(
    ngclContext_t *, ngclExecutable_t *, int *, int, int *);
int ngclSessionReleaseCancelList(ngclSession_t *, int *);
int ngclSessionGetError(ngclSession_t *, int *);
int ngclSessionSetError(ngclSession_t *, int , int *);
int ngclSessionSetCbError(ngclSession_t *, int , int *);
int ngclSessionStart(ngclSession_t *, char *, int *, ...);
int ngclSessionStartVarg(ngclSession_t *, char *, va_list, int *);
int ngclSessionStartWithArgumentStack(
    ngclSession_t *, char *, ngclArgumentStack_t *, int *);
int ngclSessionSuspend(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionResume(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionCancel(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionWaitAll(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionWaitAny(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
int ngclSessionAttributeInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);
int ngclSessionAttributeFinalize(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);

/* Argument Stack */
ngclArgumentStack_t *ngclArgumentStackConstruct(ngclContext_t *, int, int *);
int ngclArgumentStackDestruct(ngclContext_t *, ngclArgumentStack_t *, int *);
int ngclArgumentStackPush(
    ngclContext_t *, ngclArgumentStack_t *, void *, int *);
void *ngclArgumentStackPop(ngclContext_t *, ngclArgumentStack_t *, int *);

/* Log */
void ngclLogPrintfContext(
    ngclContext_t *, const char *, ngLogLevel_t, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(5, 6);
void ngclLogPrintfExecutable(
    ngclExecutable_t *, const char *, ngLogLevel_t, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(5, 6);
void ngclLogPrintfSession(
    ngclSession_t *, const char *, ngLogLevel_t, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(5, 6);

void ngclLogDebugContext(
    ngclContext_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogInfoContext(
    ngclContext_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogWarnContext(
    ngclContext_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogErrorContext(
    ngclContext_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogFatalContext(
    ngclContext_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);

void ngclLogDebugExecutable(
    ngclExecutable_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogInfoExecutable(
    ngclExecutable_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogWarnExecutable(
    ngclExecutable_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogErrorExecutable(
    ngclExecutable_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogFatalExecutable(
    ngclExecutable_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);

void ngclLogDebugSession(
    ngclSession_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogInfoSession(
    ngclSession_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogWarnSession(
    ngclSession_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogErrorSession(
    ngclSession_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngclLogFatalSession(
    ngclSession_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);

/* Read/Write Lock */
int ngclRWlockInitialize(ngclContext_t *, ngiRWlock_t *, int *);
int ngclRWlockFinalize(ngclContext_t *, ngiRWlock_t *, int *);
int ngclRWlockReadLock(ngclContext_t *, ngiRWlock_t *, int *);
int ngclRWlockReadUnlock(ngclContext_t *, ngiRWlock_t *, int *);
int ngclRWlockWriteLock(ngclContext_t *, ngiRWlock_t *, int *);
int ngclRWlockWriteUnlock(ngclContext_t *, ngiRWlock_t *, int *);

/* Exclusive lock */
#define ngExclusiveLock_t ngiRWlock_t

#define ngclExclusiveLockInitialize(context, exclusiveLock, error) \
    ngclRWlockInitialize((context), (exclusiveLock), (error))
#define ngclExclusiveLockFinalize(context, exclusiveLock, error) \
    ngclRWlockFinalize((context), (exclusiveLock), (error))
#define ngclExclusiveLock(context, exclusiveLock, error) \
    ngclRWlockWriteLock((context), (exclusiveLock), (error))
#define ngclExclusiveUnlock(context, exclusiveLock, error) \
    ngclRWlockWriteUnlock((context), (exclusiveLock), (error)) \

/* Version */
char *ngclGetVersion(int *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "ngClientInternal.h"
#endif /* _NG_H_ */
