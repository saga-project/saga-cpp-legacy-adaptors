/* 
 * $RCSfile: ng.h,v $ $Revision: 1.192 $ $Date: 2007/12/26 12:27:17 $
 * $AIST_Release: 4.2.4 $
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <globus_common.h>
#include <globus_module.h>
#include <globus_gram_client.h>
#include <globus_gram_protocol.h>
#include <globus_gass_server_ez.h>
#include <globus_io.h>
#include <globus_libc.h>
#include "ngFunctionInformation.h"
#include "ngCommon.h"
#include "ngInternal.h"

#ifndef NGI_NO_MDS2_MODULE
#include <ldap.h>
#endif /* NGI_NO_MDS2_MODULE */

#ifndef NGI_NO_MDS4_MODULE
#include <IndexService_client.h>
#endif /* NGI_NO_MDS4_MODULE */

/**
 * Argument Stack.
 */
#define ngclArgumentStack_s ngiArgumentStack_s
#define ngclArgumentStack_t ngiArgumentStack_t

/**
 * Information for managing GASS server.
 */
typedef struct ngclGASSserverManager_s {
    /* Link list of GASS Server Manager */
    struct ngclGASSserverManager_s	*nggsm_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	nggsm_rwlOwn;

    /* Setup parameters to GASS server */
    char	*nggsm_scheme;	/* scheme */
    unsigned long        nggsm_options; /* optional parameter */
#if 0 /* Is this necessary? */
    globus_gass_transfer_listenerattr_t nggsm_listenerAttr;
    globus_gass_transfer_requestattr_t	nggsm_requestAttr;
#endif

    /* Parameters provided by GASS server */
    globus_gass_transfer_listener_t	nggsm_listener;
    char	*nggsm_url;	/* URL */
} ngclGASSserverManager_t;

/**
 * Information about Local Machine.
 */
typedef struct ngclLocalMachineInformation_s {
    char       *nglmi_hostName;		/* Host name of local machine */
    int		nglmi_saveNsessions;	/* Number of sessions to save */
    ngLogInformation_t nglmi_logInfo;	/* Log Information */
    char       *nglmi_gassUrl;		/* URL of GASS server */
    in_port_t	nglmi_gassPort;		/* Port number of GASS server */
    char       *nglmi_tmpDir;           /* Temporary directory */
    int		nglmi_refreshInterval;	/* Interval of Refresh the proxy cert */
    char       *nglmi_invokeServerLog;  /* Invoke Server log file name */
    int		nglmi_fortranCompatible;/* Fortran compatible */
    int         nglmi_listenPort;        /* Listen port for crypt false */
    int         nglmi_listenPortAuthOnly;/* Listen port for crypt auth_only*/
    int         nglmi_listenPortGSI;     /* Listen port for crypt GSI */
    int         nglmi_listenPortSSL;     /* Listen port for crypt SSL */
    int		nglmi_tcpNodelay;	/* TCP_NODELAY option */
} ngclLocalMachineInformation_t;


/**
 * MDS type
 */
typedef enum ngclMDSserverInformationType_e {
    NGCL_MDS_SERVER_TYPE_NONE,
    NGCL_MDS_SERVER_TYPE_MDS2,
    NGCL_MDS_SERVER_TYPE_MDS4
} ngclMDSserverInformationType_t;

/**
 * Information about MDS server.
 */
typedef struct ngclMDSserverInformation_s {
    char        *ngmsi_hostName;	/* Host name of MDS server */
    char        *ngmsi_tagName;         /* Tag name of this entry */
    ngclMDSserverInformationType_t ngmsi_type;     /* Type of MDS */

    in_port_t    ngmsi_portNo;		/* Port number of MDS server */
    char        *ngmsi_protocol;        /* http or https of SERVICE_URL */
    char        *ngmsi_path;            /* path of SERVICE_URL */
    char        *ngmsi_subject;         /* subject */
    char        *ngmsi_voName;		/* vo name */

    /* Timeout at the time of query */
    int		ngmsi_clientTimeout;	/* Where client side */
    int		ngmsi_serverTimeout;	/* Where server side */
} ngclMDSserverInformation_t;

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

typedef struct ngclRemoteMachineInformation_s {
    /* Information about Remote Machine */
    char	*ngrmi_hostName;	/* Host name */
    char	*ngrmi_tagName;		/* Tag name */
    in_port_t	ngrmi_portNo;		/* Port number of gate-keeper */

    /* Information about MDS server */
    char	*ngrmi_mdsServer;	/* MDS server */
    char	*ngrmi_mdsTag;		/* MDS tag */

    /* Information about Invoke Server */
    char	*ngrmi_invokeServerType; /* Invoke Server Type */
    int		ngrmi_invokeServerNoptions; /* Invoke Server Num of Options */
    char	**ngrmi_invokeServerOptions;/* Invoke Server Options */

    /* Information about MPI */
    int		ngrmi_mpiNcpus;		/* Number of CPUs */

    /* Information about GASS server */
    char	*ngrmi_gassScheme;	/* Scheme of GASS server */

    /* Information about Protocol */
    ngProtocolCrypt_t	ngrmi_crypt;	/* Type of crypt */
    ngProtocolType_t	ngrmi_protocol;	/* Type of protocol */
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
    int		ngrmi_heartBeatTimeoutCountOnTransfer; /* Transfer timeout */

    /* Information about others */
    int		ngrmi_redirectEnable;	/* Redirect from remote machine */
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
    ngLogInformation_t		ngrmi_commLogInfo;

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
typedef struct ngcliExecutablePathInformation_s {
    char *ngepi_hostName;	/* Host name */
    char *ngepi_className;	/* Class name of remote class */
    char *ngepi_path;		/* Path of Ninf-G Executable at remote machine */
    int ngepi_stagingEnable;	/* staging */
    ngBackend_t ngepi_backend;	/* Backend */
    int ngepi_mpiNcpus;		/* Number of CPUs */
    int ngepi_sessionTimeout;   /* Session Timeout */
} ngcliExecutablePathInformation_t;

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
    int ngs_mutexInitialized;
    int ngs_condInitialized;
    globus_mutex_t	ngs_mutex;
    globus_cond_t	ngs_cond;

    /* Read/Write Lock for this instance */
    int ngs_rwlOwnInitialized;
    ngRWlock_t	ngs_rwlOwn;

    int	ngs_ID;		/* ID number of this session */
    ngclSessionStatus_t	ngs_status;	/* Status of Session */
    int ngs_cancelRequest; /* The flag of cancel request */
    int	ngs_error;	/* Error code */
    int	ngs_cbError;	/* Error code, it occurred in callback function */
 
    int ngs_wasFailureNotified; /* Was failure notified to user? */

    /* Status of Heart Beat */
    ngclHeartBeatStatus_t ngs_heartBeatStatus;

    /* Timeout */
    int    ngs_timeout;
    time_t ngs_timeoutTime;

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
	int		ngens_mutexInitialized;
	int		ngens_condInitialized;
	globus_mutex_t	ngens_mutex;
	globus_cond_t	ngens_cond;
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
    int nge_mutexInitialized;
    int nge_condInitialized;
    int nge_rwlOwnInitialized;
    globus_mutex_t	nge_mutex;
    globus_cond_t	nge_cond;
    ngRWlock_t	nge_rwlOwn;

    /* Host name of Remote Machine */
    char *nge_hostName;
    char *nge_tagName;

    /* Communication Log */
    int nge_commLogInfoExist;
    ngLogInformation_t nge_commLogInfo;
    ngLog_t *nge_commLog;

    /* Remote Class Information */
    char *nge_rcInfoName;
    int nge_rcInfoExist;
    ngRemoteClassInformation_t	nge_rcInfo;

    int	nge_ID;		/* Identification number of Executable */
    ngclExecutableStatus_t nge_status;		/* Status */
    int nge_locked;	/* Any body locked this instance */
    int nge_sending;	/* Any body sending the request */
    ngArgumentTransfer_t nge_waitArgumentTransfer;

    ngProtocolType_t		nge_protoType;	/* Type of protocol */
    int	nge_error;	/* Error code */
    int	nge_cbError;	/* Error code, it occurred in callback function */
    int nge_requestResult;	/* The result of Protocol Request */

    /* I/O Callback running or is registered */
    int nge_ioCallbackRegistered;
    int	nge_ioCallbackRegisteredMutexInitialized;
    int	nge_ioCallbackRegisteredCondInitialized;
    globus_mutex_t	nge_ioCallbackRegisteredMutex;
    globus_cond_t	nge_ioCallbackRegisteredCond;

    /* HeartBeat control */
    int nge_heartBeatInterval;  /* Heart Beat Interval (second) */
    int nge_heartBeatTimeout; /* Heart Beat Timeout (second) */
    int nge_heartBeatTimeoutOnTransfer; /* Timeout on data transfer (second) */
    time_t nge_heartBeatLastReceived; /* Last Heart Beat received time */
    int nge_heartBeatIsDataTransferring; /* Is transferring data? */
    ngclHeartBeatStatus_t nge_heartBeatStatus;	/* Status of Heart Beat */

    int nge_sessionTimeout;  /* Session Timeout */

    time_t nge_jobStartTimeoutTime; /* Job Start Timeout Time */

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
typedef struct ngclLocalMachineInformationManager_s {
#if 0 /* Is this necessary? */
    /* Lins list of Local Machine Information */
    struct ngclLocalMachineInformationManager_s	*nglmim_next;
#endif

    /* Read/Write Lock for this instance */
    ngRWlock_t	nglmim_rwlOwn;

    /* Information for Local Machine */
    ngclLocalMachineInformation_t	nglmim_info;
} ngclLocalMachineInformationManager_t;

/**
 * Information for managing MDS Server Information.
 */
typedef struct ngcliMDSserverInformationManager_s {
    /* Link list of MDS Server Information */
    struct ngcliMDSserverInformationManager_s  *ngmsim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	ngmsim_rwlOwn;

    /* LDAP structure for searching */
#ifndef NGI_NO_MDS2_MODULE
    LDAP	*ngmsim_ldap;
#endif /* NGI_NO_MDS2_MODULE */
    int		ngmsim_ldapInitialized;

#ifndef NGI_NO_MDS4_MODULE
    IndexService_client_handle_t	ngmsim_index;
    globus_soap_message_attr_t	ngmsim_index_message_attr;
    gss_name_t       ngmsim_subjectIdentity;
    globus_reltime_t ngmsim_clientTimeout;
#endif /* NGI_NO_MDS4_MODULE */
    int		ngmsim_indexInitialized;

    /* Information about MDS server */
    ngclMDSserverInformation_t	ngmsim_info;
} ngcliMDSserverInformationManager_t;

/**
 * Information for managing Invoke Server Information.
 */
typedef struct ngcliInvokeServerInformationManager_s {
    /* Link list of Invoke Server Information */
    struct ngcliInvokeServerInformationManager_s  *ngisim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	ngisim_rwlOwn;

    /* Information about Invoke Server */
    ngclInvokeServerInformation_t	ngisim_info;
} ngcliInvokeServerInformationManager_t;

/**
 * Information for managing Ninf-G Executable Path.
 */
typedef struct ngcliExecutablePathInformationManager_s {
    /* Link list of Executable Path Information */
    struct ngcliExecutablePathInformationManager_s	*ngepim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	ngepim_rwlOwn;

    /* Information about Ninf-G Executable Path */
    ngcliExecutablePathInformation_t	ngepim_info;
} ngcliExecutablePathInformationManager_t;

/**
 * Information for managing Remote Machine Information .
 */
typedef struct ngclRemoteMachineInformationManager_s {
    /* Link list of Remote Machine Information */
    struct ngclRemoteMachineInformationManager_s	*ngrmim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	ngrmim_rwlOwn;

    /* Link list of Executable Path Information */
    ngRWlock_t	ngrmim_rwlExecPath;	/* Read/Write Lock for list */
    ngcliExecutablePathInformationManager_t	*ngrmim_epInfo_head;
    ngcliExecutablePathInformationManager_t	*ngrmim_epInfo_tail;

    /* MDS server and Root DN which found this information */
    ngcliMDSserverInformationManager_t	*ngrmim_mdsServer;
    char	*ngrmim_hostDN;

    /* Information about Remote Machine */
    ngclRemoteMachineInformation_t	ngrmim_info;
} ngclRemoteMachineInformationManager_t;

/**
 * Information for managing Remote Class Information.
 */
typedef struct ngclRemoteClassInformationManager_s {
    /* Link list of Remote Class Information */
    struct ngclRemoteClassInformationManager_s  *ngrcim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t      ngrcim_rwlOwn;

    /* Information about Remote Class */
    ngRemoteClassInformation_t	ngrcim_info;
} ngclRemoteClassInformationManager_t;

#if 0 /* Is this necessary? */
/**
 * Information for managing Remote Method Information.
 */
typedef struct ngRemoteMethodInformationManager_s {
    /* Link list of Remote Method Information */
    struct ngclRemoteMethodInformationManager_s	*ngrmim_next;

    /* Read/Write Lock for this instance */
    ngRWlock_t	ngrmim_rwlOwn;

    /* Information about Remote Method */
    ngRemoteMethodInformation_t	ngrmim_info;
} ngRemoteMethodInformationManager_t;
#endif /* 0 */

/* Observe Thread Items */
struct ngcliObserveItem_s;
typedef int (*ngcliObserveItemEvent_t)(
    struct ngclContext_s *, struct ngcliObserveItem_s *, time_t, int *);

typedef struct ngcliObserveItem_s {
    /* Link list of Observe Items */
    struct ngcliObserveItem_s *ngoi_next;

    time_t ngoi_eventTime;                  /* next event time */
    ngcliObserveItemEvent_t ngoi_eventFunc; /* function for event */
    int ngoi_eventExecuted;               /* event was executed in the loop */
    int ngoi_eventTimeChangeRequested;      /* change requested */
                                        /* a function for to set event time */
    ngcliObserveItemEvent_t ngoi_eventTimeSetFunc;
    int ngoi_interval;                      /* event interval (user data) */

} ngcliObserveItem_t;

/* Observe Thread */
typedef struct ngcliObserveThread_s {
    int ngot_continue;        /* flag for observe thread to continue */
    int ngot_stopped;         /* flag to tell if the observe thread stopped */
    globus_mutex_t ngot_mutex;
    globus_cond_t  ngot_cond;
    int ngot_mutexInitialized;
    int ngot_condInitialized;
    globus_thread_t ngot_thread;

    ngcliObserveItem_t *ngot_item_head;
} ngcliObserveThread_t;

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

    ngRWlock_t	ngc_rwlOwn;	/* Read/Write Lock for this instance */

    /* Communication Manager */
    struct ngiCommunication_s	*ngc_commNone;	/* None crypt */
    struct ngiCommunication_s	*ngc_commAuthonly; /* Authentication only */
    struct ngiCommunication_s	*ngc_commGSI;	/* GSI */
    struct ngiCommunication_s	*ngc_commSSL;	/* SSL */

    /* Protocol Manager */
    struct ngiProtocol_s	*ngc_protoNone;	/* None crypt */
    struct ngiProtocol_s	*ngc_protoAuthonly; /* Authentication only */
    struct ngiProtocol_s	*ngc_protoGSI;	/* GSI */
    struct ngiProtocol_s	*ngc_protoSSL;	/* SSL */

    ngLog_t	*ngc_log;	/* Data for managing log */
    int ngc_ID;		/* Identification number of this instance */
    int	ngc_error;	/* Error code */
    int	ngc_cbError;	/* Error code, it occurred in callback function */

    /* Information for managing GASS Server */
    ngRWlock_t	ngc_rwlGassMng;	/* Read/Write Lock for the following lists */
    ngclGASSserverManager_t	*ngc_gassMng_head;
    ngclGASSserverManager_t	*ngc_gassMng_tail;

    /* Information about Local Machine */
    ngRWlock_t	ngc_rwlLmInfo;	/* Read/Write Lock for the following lists */
    ngclLocalMachineInformationManager_t	*ngc_lmInfo;

    /* Information about MDS server */
    ngRWlock_t	ngc_rwlMdsInfo;	/* Read/Write Lock for the following lists */
    ngcliMDSserverInformationManager_t	*ngc_mdsInfo_head;
    ngcliMDSserverInformationManager_t	*ngc_mdsInfo_tail;

    /* Information about Invoke Server */
    ngRWlock_t	ngc_rwlIsInfo;	/* Read/Write Lock for the following lists */
    ngcliInvokeServerInformationManager_t	*ngc_isInfo_head;
    ngcliInvokeServerInformationManager_t	*ngc_isInfo_tail;

    /* Information about Remote Machine */
    ngRWlock_t	ngc_rwlRmInfo;	/* Read/Write Lock for the following lists */
    ngclRemoteMachineInformationManager_t	*ngc_rmInfo_default;
    ngclRemoteMachineInformationManager_t	*ngc_rmInfo_head;
    ngclRemoteMachineInformationManager_t	*ngc_rmInfo_tail;

#if 0 /* Is this necessary? */
    /* Information about Executable Path */
    ngRWlock_t	ngc_rwlEpInfo;	/* Read/Write Lock for the following lists */
    ngcliExecutablePathInformation_t	*ngc_epInfo_head;
    ngcliExecutablePathInformation_t	*ngc_epInfo_tail;
#endif

    /* Information about Remote Class */
    ngRWlock_t	ngc_rwlRcInfo;	/* Read/Write Lock for the following lists */
    ngclRemoteClassInformationManager_t	*ngc_rcInfo_head;
    ngclRemoteClassInformationManager_t	*ngc_rcInfo_tail;

    /* Information for managing Job */
    int			ngc_nJobs;	/* The number of existing jobs */
    ngRWlock_t	ngc_rwlJobMng;	/* Read/Write Lock for the following lists */
    int			ngc_jobID;	/* Job ID */
    struct ngcliJobManager_s	*ngc_jobMng_head;
    struct ngcliJobManager_s	*ngc_jobMng_tail;

    /* Invoke Server */
    int			ngc_nInvokeServers;
    int			ngc_invokeServerID;
    ngRWlock_t	ngc_rwlInvokeMng;/* Read/Write Lock for the following lists */
    struct ngcliInvokeServerCount_s   *ngc_invokeCount_head;
    struct ngcliInvokeServerCount_s   *ngc_invokeCount_tail;
    struct ngcliInvokeServerManager_s *ngc_invokeMng_head;
    struct ngcliInvokeServerManager_s *ngc_invokeMng_tail;

    /* Executable Handle */
    ngRWlock_t	ngc_rwlExecutable; /* Read/Write Lock for following data */
    int	ngc_nExecutables;	/* The number of existing executables */
    int	ngc_executableID;	/* Identification number of Executable */
    ngclExecutable_t	*ngc_executable_head;
    ngclExecutable_t	*ngc_executable_tail;
    ngclExecutable_t	*ngc_destruction_executable_head;
    ngclExecutable_t	*ngc_destruction_executable_tail;

    globus_mutex_t	ngc_mutexExecutable;
    globus_cond_t	ngc_condExecutable;
    int ngc_flagExecutable;

    /* Information for managing Session */
    ngRWlock_t	ngc_rwlSession;	/* Read/Write Lock for following data */
    int	ngc_nSessions;		/* The number of existing sessions */
    int	ngc_sessionID;		/* Identification number of Session */
    ngRWlock_t	ngc_rwlSessionList; /* Read/Write Lock for list of all Sessions */

    globus_mutex_t	ngc_mutexSession;
    globus_cond_t	ngc_condSession;
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
    int		ngc_configFileReadCount;
    int		ngc_configFileReading;

    /* MDS Access */
    int ngc_mdsAccessEnabled;

    /* HeartBeat */
    ngcliObserveItem_t *ngc_heartBeat;

    /* SessionTimeout */
    ngcliObserveItem_t *ngc_sessionTimeout;

    /* Refresh Credentials */
    ngcliObserveItem_t *ngc_refreshCredentialsInfo;

    /* Job Start Timeout */
    ngcliObserveItem_t *ngc_jobStartTimeout;

    /* Observe Thread */
    ngcliObserveThread_t ngc_observe;

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
int ngclNinfgManagerYieldForCallback(void);

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
int ngclContextRegisterLocalMachineInformation(
    ngclContext_t *, ngclLocalMachineInformationManager_t *, int *);
int ngclContextUnregisterLocalMachineInformation(
    ngclContext_t *, ngclLocalMachineInformationManager_t *, int *);
int ngclLocalMachineInformationGetCopy(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngclLocalMachineInformationRelease(
    ngclContext_t *context, ngclLocalMachineInformation_t *lmInfo, int *error);

/* MDS Server Information */
int ngclMDSserverInformationGetCopy(
    ngclContext_t *, char *, ngclMDSserverInformation_t *, int *);
int ngclMDSserverInformationRelease(
    ngclContext_t *, ngclMDSserverInformation_t *, int *);

/* Invoke Server Information */
int ngclInvokeServerInformationGetCopy(
    ngclContext_t *, char *, ngclInvokeServerInformation_t *, int *);
int ngclInvokeServerInformationRelease(
    ngclContext_t *, ngclInvokeServerInformation_t *, int *);

/* Remote Machine Information */
int ngclContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
int ngclContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
int ngclContextRegisterRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
int ngclContextUnregisterRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
int ngclRemoteMachineInformationGetCopy(
    ngclContext_t *, char *, char *, ngclRemoteMachineInformation_t *, int *);
int ngclDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngclRemoteMachineInformationRelease(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);

/* Executable Path Information */
int ngclRemoteMachineInformationRegisterExecutablePathInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
int ngclRemoteMachineInformationUnregisterExecutablePathInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
int ngclExecutablePathInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, char *,
    ngcliExecutablePathInformation_t *, int *error);
int ngclExecutablePathInformationRelease(
    ngclContext_t *, ngcliExecutablePathInformation_t *, int *error);

/* Remote Class Information */
int ngclContextRegisterRemoteClassInformation(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
int ngclContextUnregisterRemoteClassInformation(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
int ngclRemoteClassInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, char *,
    ngRemoteClassInformation_t *, int *);
int ngclRemoteClassInformationRelease(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);

/* Executable */
ngclExecutable_t *ngclExecutableConstruct(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
int ngclExecutableDestruct(ngclExecutable_t *, int, int *);
ngclExecutable_t *ngclExecutableAllocate(ngclContext_t *, int, int *);
int ngclExecutableFree(ngclExecutable_t *, int *);
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
int ngclArgumentStackDestruct(ngclArgumentStack_t *, int *);
int ngclArgumentStackPush(ngclArgumentStack_t *, void *arg, int *);
void *ngclArgumentStackPop(ngclArgumentStack_t *, int *);

/* Log */
int ngclLogPrintfContext(
    ngclContext_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, ...);
int ngclLogPrintfExecutable(
    ngclExecutable_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, ...);
int ngclLogPrintfSession(
    ngclSession_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, ...);

/* Read/Write Lock */
int ngclRWlockInitialize(ngclContext_t *, ngRWlock_t *, int *);
int ngclRWlockFinalize(ngclContext_t *, ngRWlock_t *, int *);
int ngclRWlockReadLock(ngclContext_t *, ngRWlock_t *, int *);
int ngclRWlockReadUnlock(ngclContext_t *, ngRWlock_t *, int *);
int ngclRWlockWriteLock(ngclContext_t *, ngRWlock_t *, int *);
int ngclRWlockWriteUnlock(ngclContext_t *, ngRWlock_t *, int *);

/* Exclusive lock */
#define ngExclusiveLock_t ngRWlock_t

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

#include "ngInternal.h"
#include "ngClientInternal.h"
#endif /* _NG_H_ */
