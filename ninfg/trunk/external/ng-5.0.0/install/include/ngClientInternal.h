/*
 * $RCSfile: ngClientInternal.h,v $ $Revision: 1.32 $ $Date: 2008/03/28 08:50:58 $
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
#ifndef _NGCLIENTINTERNAL_H_
#define _NGCLIENTINTERNAL_H_

#include "ngInternal.h"

/**
 * This file define the Data Structures and Constant Values for Pure Ninf-G
 * internal.
 */

/**
 * Define the Constant Values.
 */
/* Name of environment variable */
#define NGCLI_ENVIRONMENT_CONFIG_FILE "NG_CONFIG_FILE"

/* configuration file name for home directory */
#define NGCLI_CONFIG_FILE_HOME "/.ngconf"

/* Number of bytes of RSL default */
#define NGCLI_RSL_NBYTES (1024 * 8)

/* Invoke Server */
#define NGCLI_INVOKE_SERVER_PROGRAM_NAME_BASE    "ng_invoke_server"
#define NGCLI_INVOKE_SERVER_PROGRAM_PATH_FORMAT  "%s/bin/%s.%s"
#define NGCLI_INVOKE_SERVER_LOG_FILE_SWITCH      "-l"
#define NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR  "\x0d\x0a"
#define NGCLI_INVOKE_SERVER_LINE_TERMINATOR_SIZE 2
#define NGCLI_INVOKE_SERVER_READ_BUFFER_INITIAL_SIZE 256
#define NGCLI_INVOKE_SERVER_JOB_ID_STR_MAX       1024

#define NGCLI_INVOKE_SERVER_REQUEST_JOB_CREATE  "JOB_CREATE"
#define NGCLI_INVOKE_SERVER_REQUEST_JOB_STATUS  "JOB_STATUS"
#define NGCLI_INVOKE_SERVER_REQUEST_JOB_DESTROY "JOB_DESTROY"
#define NGCLI_INVOKE_SERVER_REQUEST_EXIT        "EXIT"

#define NGCLI_INVOKE_SERVER_RESULT_SUCCESS       "S"
#define NGCLI_INVOKE_SERVER_RESULT_FAILURE       "F"

#define NGCLI_INVOKE_SERVER_NOTIFY_CREATE "CREATE_NOTIFY"
#define NGCLI_INVOKE_SERVER_NOTIFY_STATUS "STATUS_NOTIFY"

#define NGCLI_INVOKE_SERVER_FEATURE_STAGING_AUTH_NUMBER "STAGING_AUTH_NUMBER"
#define NGCLI_INVOKE_SERVER_FEATURE_STAGING_COMMUNICATION_PROXY "STAGING_COMMUNICATION_PROXY"

/**
 * Set the error code.
 */
#define NGI_SET_ERROR_CONTEXT(context, code, error) \
    do { \
	if ((context) != NULL) \
	    ngcliContextSetError(context, code, error); \
    } while (0)

#define NGI_SET_CB_ERROR_CONTEXT(context, code, error) \
    do { \
	if ((context) != NULL) \
	    ngcliContextSetCbError(context, code, error); \
    } while (0)

#define NGI_SET_ERROR_EXECUTABLE(executable, code, error) \
    do { \
	if ((executable) != NULL) \
	    ngcliExecutableSetError(executable, code, error); \
    } while (0)

#define NGI_SET_CB_ERROR_EXECUTABLE(executable, code) \
    do { \
	if ((executable) != NULL) \
	    ngclContextSetCbError(executable, code); \
    } while (0)

#define NGI_SET_ERROR_SESSION(session, code, error) \
    do { \
	if ((session) != NULL) \
	    ngclSessionSetError(session, code, error); \
    } while (0)

#define NGI_SET_CB_ERROR_SESSION(session, code) \
    do { \
	if ((session) != NULL) \
	    ngclContextSetCbError(session, code); \
    } while (0)

/**
 * Define the constant values.
 */


/**
 * Attribute of Job Management.
 */
typedef struct ngcliJobAttribute_s {
    char	*ngja_hostName;		/* Host name of remote machine */
    int		ngja_portNo;		/* Port number of job manager on remote */
    char	*ngja_jobManager;	/* Name of job manager */
    char	*ngja_subject;		/* Name of Subject */
    char	*ngja_clientHostName;	/* Client Host name */
    int		ngja_mpiNcpus;		/* Number of CPUs of MPI */
    ngBackend_t	ngja_backend;		/* Backend */
    char	*ngja_executablePath;	/* Path of Ninf-G Executable */
    int		ngja_stagingEnable;     /* staging */
    int		ngja_invokeNjobs;	/* The number of JOBs which invoke */
    /* Timeout time at the time of a JOB start/stop (second) */
    int		ngja_startTimeout;
    /* if stopTimeout is zero,      doesn't wait  job done.
     * if stopTimeout is a positive number, waits job done for value of stopTimeout.
     * if stopTimeout is a negative number, waits job done endless.
     */
    int		ngja_stopTimeout;
    char        *ngja_queueName;        /* Queue name of remote machine */
    int         ngja_forceXDR;          /* Force XDR */

    int ngja_lmInfoExist;
    int ngja_isInfoExist;
    int ngja_rmInfoExist;
    int ngja_rcInfoExist;
    int ngja_epInfoExist;
    ngclLocalMachineInformation_t ngja_lmInfo;	/* Local Machine Information */
    ngclInvokeServerInformation_t ngja_isInfo;	/* Invoke Server Information */
    ngclRemoteMachineInformation_t ngja_rmInfo;	/* Remote Machine Information */
    ngRemoteClassInformation_t ngja_rcInfo;	/* Remote Class Information */
    ngclExecutablePathInformation_t ngja_epInfo;/* Executable Path Information */
} ngcliJobAttribute_t;

/**
 * Job data for Invoke Server
 */
typedef struct ngcliInvokeServerJob_s {
    struct ngcliInvokeServerManager_s *ngisj_invokeServer;
    int		ngisj_requestID;
    char	*ngisj_invokeJobID;
    int		ngisj_invokeJobIDset;
    int         ngisj_jobDestroyed;
    char        *ngisj_stdoutFile;
    char        *ngisj_stderrFile;
} ngcliInvokeServerJob_t;

/* Status of Job */
typedef enum ngcliJobStatus_e {
    NGI_JOB_STATUS_UNKNOWN = -1,	/* Unknown */
    NGI_JOB_STATUS_INITIALIZING = 0,	/* Initializing */
    NGI_JOB_STATUS_INVOKED,		/* Invoked */
    NGI_JOB_STATUS_PENDING,		/* Pending */
    NGI_JOB_STATUS_ACTIVE,		/* Activated */
    NGI_JOB_STATUS_FAILED,		/* Failed */
    NGI_JOB_STATUS_UNSUBMITTED,		/* Unsubmitted */
    NGI_JOB_STATUS_STAGE_IN,		/* Stage in */
    NGI_JOB_STATUS_STAGE_OUT,		/* Stage out */
    NGI_JOB_STATUS_TIMEOUT,		/* Job can not start */
    NGI_JOB_STATUS_CANCEL_TIMEOUT,	/* Job can not stopped */
    NGI_JOB_STATUS_DONE,		/* Done */
    NGI_JOB_STATUS_DESTROYED		/* Destroyed */
} ngcliJobStatus_t;

/**
 * Information for managing Job.
 */
typedef struct ngcliJobManager_s {
    /* Link list of information for managing Job */
    struct ngcliJobManager_s		*ngjm_next;

    struct ngclContext_s       *ngjm_context;	/* Ninf-G Context */

    /* Link list of Executable Handle */
    struct ngclExecutable_s    *ngjm_executable_head;
    struct ngclExecutable_s    *ngjm_executable_tail;

    /* Link list of Executable Handles which are being destructing */
    struct ngclExecutable_s    *ngjm_destruction_executable_head;
    struct ngclExecutable_s    *ngjm_destruction_executable_tail;

    /* Monitor for callback function */
    ngiMutex_t	ngjm_monMutex;
    ngiCond_t	ngjm_monCond;

    /* Read/Write Lock for this instance */
    int ngjm_rwlOwnInitialized;
    ngiRWlock_t	ngjm_rwlOwn;

    int ngjm_ID;		/* Job ID */
    int ngjm_useInvokeServer;		/* Is Invoke Server used? */
    int ngjm_simpleAuthNumber;		/* Simple Auth Number */
    ngcliJobAttribute_t	ngjm_attr;	/* Attribute of Job Manager */

    ngcliInvokeServerJob_t ngjm_invokeServerInfo; /* Invoke Server Job Data */
    int		ngjm_nExecutables;	/* Number of Ninf-G Executables */
    int		ngjm_requestCancel;	/* Request cancel the Job */
    ngcliJobStatus_t ngjm_status;	/* Status of Job */
    int         ngjm_commProxyID;/* Communication Proxy ID used by this job */
    ngiLineList_t *ngjm_clientCommunicationProxyInfo;

    ngExecutionTime_t ngjm_invoke;
    int		ngjm_invokeMeasured;
} ngcliJobManager_t;


/**
 * Information for Invoke Server.
 */
typedef struct ngcliInvokeServerManager_s {
    /* Link list of information for managing Invoke Server */
    struct ngcliInvokeServerManager_s	*ngism_next;

    struct ngclContext_s	*ngism_context; /* Ninf-G Context */

    int		ngism_ID;		/* Invoke Server ID */
    char	*ngism_serverType;	/* Invoke Server Type */
    int         ngism_typeCount;  /* Invoke Server count (ID) for this type */

    int		ngism_valid;		/* Invoke Server is valid */
    int		ngism_working;		/* Invoke Server is working */
    int		ngism_errorCode;	/* error code for invalid */

    /* Mutex and condition variable for this instance */
    ngiMutex_t	ngism_mutex;
    ngiCond_t   ngism_cond;

    ngiExternalModule_t *ngism_externalModule; /* External Module */

    int         ngism_nJobsMax;         /* The maximum number of jobs */
    int		ngism_maxRequestID;	/* ID for JOB_CREATE */

    int		ngism_isG4;		/* Is Ninf-G4 Invoke Server? */
    int		ngism_isAuthNumberStaging; /* Staging auth_number? */
    int		ngism_isCommunicationProxyStaging;
    			/* Staging remote communication proxy? */

} ngcliInvokeServerManager_t;

/**
 * Argument of COMMUNICATION_REPLY(Communication Proxy)
 */
typedef struct ngcliCommunicationReplyArgument_s {
    SLIST_ENTRY(ngcliCommunicationReplyArgument_s) ngcra_entry;
    int                                            ngcra_registered;
    int                                            ngcra_set;
    int                                            ngcra_id;
    int                                            ngcra_result;
    ngiLineList_t                                 *ngcra_arguments;
    int                                            ngcra_errorCode;
    char                                          *ngcra_message;
} ngcliCommunicationReplyArgument_t;

/**
 * Communication Proxy
 */
typedef struct ngcliCommunicationProxy_s {
    SLIST_ENTRY(ngcliCommunicationProxy_s) ngcp_entry;
    int                                    ngcp_registered;

    struct ngcliCommunicationProxyManager_s *ngcp_manager;

    int                                 ngcp_destroyGuardCount;
    int                                 ngcp_destroying;
    int                                 ngcp_sentInitialize;
    ngclCommunicationProxyInformation_t ngcp_information;
    int                                 ngcp_informationGotten;
    int                                 ngcp_requestIdSeed;
    int                                 ngcp_nUsing;
    ngiRlock_t                          ngcp_rlock;

    SLIST_HEAD(ngcliCommunicationReplyArguments_s,
        ngcliCommunicationReplyArgument_s) ngcp_communicationReplyArgs;

    ngiExternalModule_t *ngcp_externalModule; /* External Module */
} ngcliCommunicationProxy_t;

/**
 * Communication Proxy Manager
 */
typedef struct ngcliCommunicationProxyManager_s {
    SLIST_HEAD(ngcliCommunicationProxyList_s, ngcliCommunicationProxy_s) ngcpm_list;
    int                                       ngcpm_port;
    ngclContext_t                            *ngcpm_context;
    ngiExternalModuleManager_t               *ngcpm_externalModuleManager;
    ngiRlock_t                                ngcpm_rlock;
    int                                       ngcpm_nLock;
} ngcliCommunicationProxyManager_t;

typedef struct ngcliInformationServiceManager_s          ngcliInformationServiceManager_t;    
typedef struct ngcliInformationService_s                 ngcliInformationService_t;
typedef struct ngcliInformationServiceQuerySession_s     ngcliInformationServiceQuerySession_t;
typedef struct ngcliInformationServiceQueryResult_s      ngcliInformationServiceQueryResult_t;
typedef struct ngcliInformationServiceQuerySessionList_s ngcliInformationServiceQuerySessionList_t;
typedef struct ngcliInformationServiceList_s             ngcliInformationServiceList_t;

SLIST_HEAD(ngcliInformationServiceQuerySessionList_s, ngcliInformationServiceQuerySession_s);
SLIST_HEAD(ngcliInformationServiceList_s, ngcliInformationService_s);

/**
 * Query result of Information Service
 * (Arguments of REMOTE_EXECUTABLE_INFORMATION_NOTIFY)
 */
struct ngcliInformationServiceQueryResult_s {
    char          *ngiqr_id;
    int            ngiqr_result;
    ngiLineList_t *ngiqr_info;
    int            ngiqr_errorCode;
    char          *ngiqr_message;
};

/**
 * Query Session of Information Service
 */
struct ngcliInformationServiceQuerySession_s {
    SLIST_ENTRY(ngcliInformationServiceQuerySession_s) ngiqs_entry;
    int                                                ngiqs_registered;
    int                                                ngiqs_canceled;
    int                                                ngiqs_waiting;
    char                                              *ngiqs_id;
    ngcliInformationService_t                         *ngiqs_owner;
    ngcliInformationServiceQueryResult_t              *ngiqs_result;
};

/**
 * Information Service
 */
struct ngcliInformationService_s {
    SLIST_ENTRY(ngcliInformationService_s)     ngis_entry;
    int                                        ngis_registered;
    int                                        ngis_disabled;
    int                                        ngis_notifyHandling;
    ngcliInformationServiceManager_t          *ngis_manager;
    ngclInformationServiceInformation_t        ngis_information;
    int                                        ngis_informationGotten;
    ngiRlock_t                                 ngis_rlock;
    ngcliInformationServiceQuerySessionList_t  ngis_querySessions;
    ngiExternalModule_t                       *ngis_externalModule;
};

/**
 * Information Service Manager
 */
struct ngcliInformationServiceManager_s {
    ngcliInformationServiceList_t  ngism_list;
    ngclContext_t                 *ngism_context;
    char                          *ngism_logFilePath;
    ngiExternalModuleManager_t    *ngism_externalModuleManager;
    ngiRWlock_t                    ngism_rwLock;
    int                            ngism_nLock;
};

typedef struct ngcliInformationServiceQuery_s ngcliInformationServiceQuery_t;
SLIST_HEAD(ngcliInformationServiceQueryList_s, ngcliInformationServiceQuery_s);
typedef struct ngcliInformationServiceQueryList_s ngcliInformationServiceQueryList_t;

struct ngcliInformationServiceQuery_s {
    SLIST_ENTRY(ngcliInformationServiceQuery_s)  ngisq_entry;
    ngclExecutablePathInformation_t             *ngisq_epInfo;
    ngRemoteClassInformation_t                  *ngisq_rcInfo;
    int                                          ngisq_done;
    int                                          ngisq_result;
    char                                        *ngisq_className;
    char                                        *ngisq_hostName;
};

typedef struct ngcliQueryManager_s {
    ngclContext_t                      *ngqm_context;
    ngcliInformationServiceManager_t   *ngqm_isMng;
    ngcliInformationServiceQueryList_t  ngqm_queries;
    ngiRlock_t                          ngqm_rlock;
    int                                 ngqm_nQueries;
    int                                 ngqm_rereading;
} ngcliQueryManager_t;

/**
 * Transfer Timeout
 */
typedef enum ngcliTransferTimeoutType_e {
    NGCLI_TRANSFER_TIMEOUT_NONE,
    NGCLI_TRANSFER_TIMEOUT_ARGUMENT,
    NGCLI_TRANSFER_TIMEOUT_RESULT,
    NGCLI_TRANSFER_TIMEOUT_CB_ARGUMENT,
    NGCLI_TRANSFER_TIMEOUT_CB_RESULT,
    NGCLI_TRANSFER_TIMEOUT_NOMORE
} ngcliTransferTimeoutType_t;

/**
 * Define the Lock/Unlock macros.
 */
#if 0 /* Is this necessary? */
/* List of Ninf-G Context */
#define ngcliContextListReadLock(log, error) \
    ngiRWlockReadLock(&ngcllNinfgManager.ngnm_rwlContext, (log), (error));
#define ngcliContextListReadUnlock(log, error) \
    ngiRWlockReadUnlock(&ngcllNinfgManager.ngnm_rwlContext, (log), (error));
#define ngcliContextListWriteLock(log, error) \
    ngiRWlockWriteLock(&ngcllNinfgManager.ngnm_rwlContext, (log), (error));
#define ngcliContextListWriteUnlock(log, error) \
    ngiRWlockWriteUnlock(&ngcllNinfgManager.ngnm_rwlContext, (log), (error));
#endif /* 0 */

/* List of Local Machine Information */
#define ngcliLocalMachineInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlLmInfo, (log), (error));

/* List of Invoke Server Information */
#define ngcliInvokeServerInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlInvokeServerInfo, (log), (error));
#define ngcliInvokeServerInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlInvokeServerInfo, (log), (error));
#define ngcliInvokeServerInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlInvokeServerInfo, (log), (error));
#define ngcliInvokeServerInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlInvokeServerInfo, (log), (error));

/* List of Communication Proxy Information */
#define ngcliCommunicationProxyInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlCpInfo, (log), (error));
#define ngcliCommunicationProxyInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlCpInfo, (log), (error));
#define ngcliCommunicationProxyInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlCpInfo, (log), (error));
#define ngcliCommunicationProxyInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlCpInfo, (log), (error));

/* List of Information Service Information */
#define ngcliInformationServiceInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlInfoServiceInfo, (log), (error));
#define ngcliInformationServiceInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlInfoServiceInfo, (log), (error));
#define ngcliInformationServiceInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlInfoServiceInfo, (log), (error));
#define ngcliInformationServiceInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlInfoServiceInfo, (log), (error));

/* List of Remote Machine Information */
#define ngcliRemoteMachineInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliRemoteMachineInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliRemoteMachineInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliRemoteMachineInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlRmInfo, (log), (error));

/* List of Default Remote Machine Information */
#define ngcliDefaultRemoteMachineInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliDefaultRemoteMachineInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliDefaultRemoteMachineInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlRmInfo, (log), (error));
#define ngcliDefaultRemoteMachineInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlRmInfo, (log), (error));

/* List of Executable Path Information */
#define ngcliExecutablePathInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlEpInfo, (log), (error));
#define ngcliExecutablePathInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlEpInfo, (log), (error));
#define ngcliExecutablePathInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlEpInfo, (log), (error));
#define ngcliExecutablePathInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlEpInfo, (log), (error));

/* List of Remote Class Information */
#define ngcliRemoteClassInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlRcInfo, (log), (error));
#define ngcliRemoteClassInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlRcInfo, (log), (error));
#define ngcliRemoteClassInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlRcInfo, (log), (error));
#define ngcliRemoteClassInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlRcInfo, (log), (error));

/* List of Remote Method Information */
#define ngcliRemoteMethodInformationListReadLock(context, rcInfo, log, error) \
    ngiRWlockReadLock(&(rcInfo)->ngrci_rwlRmInfo, (log), (error));
#define ngcliRemoteMethodInformationListReadUnlock(context, rcInfo, log, error) \
    ngiRWlockReadUnlock(&(rcInfo)->ngrci_rwlRmInfo, (log), (error));
#define ngcliRemoteMethodInformationListWriteLock(context, rcInfo, log, error) \
    ngiRWlockWriteLock(&(rcInfo)->ngrci_rwlRmInfo, (log), (error));
#define ngcliRemoteMethodInformationListWriteUnlock(context, rcInfo, log, error) \
    ngiRWlockWriteUnlock(&(rcInfo)->ngrci_rwlRmInfo, (log), (error));

/* List of Job Manager */
#define ngcliContextJobManagerListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlJobMng, (log), (error));
#define ngcliContextJobManagerListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlJobMng, (log), (error));
#define ngcliContextJobManagerListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlJobMng, (log), (error));
#define ngcliContextJobManagerListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlJobMng, (log), (error));

/* List of Invoke Server Manager */
#define ngcliContextInvokeServerManagerListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlInvokeMng, (log), (error));
#define ngcliContextInvokeServerManagerListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlInvokeMng, (log), (error));
#define ngcliContextInvokeServerManagerListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlInvokeMng, (log), (error));
#define ngcliContextInvokeServerManagerListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlInvokeMng, (log), (error));

/* Ninf-G Manager */
#define ngcliNinfgManagerReadLock(log, error) \
    ngiRWlockReadLock(&ngcllNinfgManager.ngnm_rwlOwn, (log), (error));
#define ngcliNinfgManagerReadUnlock(log, error) \
    ngiRWlockReadUnlock(&ngcllNinfgManager.ngnm_rwlOwn, (log), (error));
#define ngcliNinfgManagerWriteLock(log, error) \
    ngiRWlockWriteLock(&ngcllNinfgManager.ngnm_rwlOwn, (log), (error));
#define ngcliNinfgManagerWriteUnlock(log, error) \
    ngiRWlockWriteUnlock(&ngcllNinfgManager.ngnm_rwlOwn, (log), (error));

/* Local Machine Information */
#define ngcliLocalMachineInformationReadLock(lmInfo, log, error) \
    ngiRWlockReadLock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationReadUnlock(lmInfo, log, error) \
    ngiRWlockReadUnlock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationWriteLock(lmInfo, log, error) \
    ngiRWlockWriteLock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationWriteUnlock(lmInfo, log, error) \
    ngiRWlockWriteUnlock(&(lmInfo)->nglmim_rwlOwn, (log), (error));

/* Invoke Server Information */
#define ngcliInvokeServerInformationReadLock(isInfo, log, error) \
    ngiRWlockReadLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationReadUnlock(isInfo, log, error) \
    ngiRWlockReadUnlock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationWriteLock(isInfo, log, error) \
    ngiRWlockWriteLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationWriteUnlock(isInfo, log, error) \
    ngiRWlockWriteUnlock(&(isInfo)->ngisim_rwlOwn, (log), (error));

/* Communication Proxy Information */
#define ngcliCommunicationProxyInformationReadLock(cpInfo, log, error) \
    ngiRWlockReadLock(&(cpInfo)->ngcpim_rwlOwn, (log), (error));
#define ngcliCommunicationProxyInformationReadUnlock(cpInfo, log, error) \
    ngiRWlockReadUnlock(&(cpInfo)->ngcpim_rwlOwn, (log), (error));
#define ngcliCommunicationProxyInformationWriteLock(cpInfo, log, error) \
    ngiRWlockWriteLock(&(cpInfo)->ngcpim_rwlOwn, (log), (error));
#define ngcliCommunicationProxyInformationWriteUnlock(cpInfo, log, error) \
    ngiRWlockWriteUnlock(&(cpInfo)->ngcpim_rwlOwn, (log), (error));

/* Information Service Information */
#define ngcliInformationServiceInformationReadLock(isInfo, log, error) \
    ngiRWlockReadLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInformationServiceInformationReadUnlock(isInfo, log, error) \
    ngiRWlockReadUnlock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInformationServiceInformationWriteLock(isInfo, log, error) \
    ngiRWlockWriteLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInformationServiceInformationWriteUnlock(isInfo, log, error) \
    ngiRWlockWriteUnlock(&(isInfo)->ngisim_rwlOwn, (log), (error));

/* Remote Machine Information */
#define ngcliRemoteMachineInformationReadLock(rmInfo, log, error) \
    ngiRWlockReadLock(&(rmInfo)->ngrmim_rwlOwn, (log), (error));
#define ngcliRemoteMachineInformationReadUnlock(rmInfo, log, error) \
    ngiRWlockReadUnlock(&(rmInfo)->ngrmim_rwlOwn, (log), (error));
#define ngcliRemoteMachineInformationWriteLock(rmInfo, log, error) \
    ngiRWlockWriteLock(&(rmInfo)->ngrmim_rwlOwn, (log), (error));
#define ngcliRemoteMachineInformationWriteUnlock(rmInfo, log, error) \
    ngiRWlockWriteUnlock(&(rmInfo)->ngrmim_rwlOwn, (log), (error));

/* Executable Path Information */
#define ngcliExecutablePathInformationReadLock(epInfo, log, error) \
    ngiRWlockReadLock(&(epInfo)->ngepim_rwlOwn, (log), (error));
#define ngcliExecutablePathInformationReadUnlock(epInfo, log, error) \
    ngiRWlockReadUnlock(&(epInfo)->ngepim_rwlOwn, (log), (error));
#define ngcliExecutablePathInformationWriteLock(epInfo, log, error) \
    ngiRWlockWriteLock(&(epInfo)->ngepim_rwlOwn, (log), (error));
#define ngcliExecutablePathInformationWriteUnlock(epInfo, log, error) \
    ngiRWlockWriteUnlock(&(epInfo)->ngepim_rwlOwn, (log), (error));

/* Remote Class Information */
#define ngcliRemoteClassInformationReadLock(rcInfo, log, error) \
    ngiRWlockReadLock(&(rcInfo)->ngrcim_rwlOwn, (log), (error));
#define ngcliRemoteClassInformationReadUnlock(rcInfo, log, error) \
    ngiRWlockReadUnlock(&(rcInfo)->ngrcim_rwlOwn, (log), (error));
#define ngcliRemoteClassInformationWriteLock(rcInfo, log, error) \
    ngiRWlockWriteLock(&(rcInfo)->ngrcim_rwlOwn, (log), (error));
#define ngcliRemoteClassInformationWriteUnlock(rcInfo, log, error) \
    ngiRWlockWriteUnlock(&(rcInfo)->ngrcim_rwlOwn, (log), (error));

/* Remote Method Information */
#define ngcliRemoteMethodInformationReadLock(rmInfo, log, error) \
    ngiRWlockReadLock(&(rmInfo)->ngrmi_rwlOwn, (log), (error));
#define ngcliRemoteMethodInformationReadUnlock(rmInfo, log, error) \
    ngiRWlockReadUnlock(&(rmInfo)->ngrmi_rwlOwn, (log), (error));
#define ngcliRemoteMethodInformationWriteLock(rmInfo, log, error) \
    ngiRWlockWriteLock(&(rmInfo)->ngrmi_rwlOwn, (log), (error));
#define ngcliRemoteMethodInformationWriteUnlock(rmInfo, log, error) \
    ngiRWlockWriteUnlock(&(rmInfo)->ngrmi_rwlOwn, (log), (error));

/* Job Manager */
#define ngcliJobManagerReadLock(jobMng, log, error) \
    ngiRWlockReadLock(&(jobMng)->ngjm_rwlOwn, (log), (error));
#define ngcliJobManagerReadUnlock(jobMng, log, error) \
    ngiRWlockReadUnlock(&(jobMng)->ngjm_rwlOwn, (log), (error));
#define ngcliJobManagerWriteLock(jobMng, log, error) \
    ngiRWlockWriteLock(&(jobMng)->ngjm_rwlOwn, (log), (error));
#define ngcliJobManagerWriteUnlock(jobMng, log, error) \
    ngiRWlockWriteUnlock(&(jobMng)->ngjm_rwlOwn, (log), (error));

/**
 * Prototype declaration of internal APIs.
 */
/* Ninf-G Manager */
int ngcliNinfgManagerInitialize(ngLog_t *, int *);
int ngcliNinfgManagerFinalize(ngLog_t *, int *);
int ngcliNinfgManagerSignalManagerStart(ngLog_t *, int *);
int ngcliNinfgManagerSignalManagerStop(ngLog_t *, int *);
int ngcliNinfgManagerSignalManagerLogSet(ngLog_t *, ngLog_t *, int *);
int ngcliNinfgManagerSignalRegister(int *, int, ngLog_t *, int *);
int ngcliNinfgManagerCreateContextID(ngLog_t *, int *);
int ngcliNinfgManagerIsContextValid(ngclContext_t *, ngLog_t *, int *);
int ngcliNinfgManagerRegisterContext(ngclContext_t *, ngLog_t *, int *);
int ngcliNinfgManagerUnregisterContext(ngclContext_t *, ngLog_t *, int *);
ngclContext_t * ngcliNinfgManagerGetContext(int id, ngLog_t *, int *);
int ngcliNinfgManagerTemporaryFileRegister(char *, ngLog_t *, int *);
int ngcliNinfgManagerTemporaryFileUnregister(char *, ngLog_t *, int *);


/* Context */
int ngcliContextWaitExecutable(ngclContext_t *, ngLog_t *, int *);
ngclSession_t *ngcliContextGetAllSessionList(ngclContext_t *, int *);
ngclSession_t *ngcliContextGetSessionList(ngclContext_t *, int *, int, int *);
ngclSession_t *ngcliContextGetSession(ngclContext_t *, int, int *);
ngclSession_t *ngcliContextGetSessionCancelList(
    ngclContext_t *, int *, int, int *);
ngclSession_t *ngcliContextGetAllSessionWaitList(ngclContext_t *, int *);
ngclSession_t *ngcliContextGetAllSessionCancelList(ngclContext_t *, int *);
int ngcliContextNotifySession(ngclContext_t *, ngLog_t *, int *);
int ngcliContextWaitAllSession(
    ngclContext_t *, ngclSession_t *, ngLog_t *, int *);
int ngcliContextWaitAnySession(
    ngclContext_t *, ngclSession_t *, ngLog_t *, int *);
int ngcliContextSetError(ngclContext_t *, int , int *);
int ngcliContextSetCbError(ngclContext_t *, int , int *);
int ngcliContextIsValid(ngclContext_t *, int *);
int ngcliContextRegisterJobManager(ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliContextUnregisterJobManager(
    ngclContext_t *, ngcliJobManager_t *, int *);
ngcliJobManager_t *ngcliContextGetJobManager(ngclContext_t *, int id, int *);
ngcliJobManager_t *ngcliContextGetNextJobManager(
    ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliContextRegisterInvokeServerManager(
    ngclContext_t *, struct ngcliInvokeServerManager_s *, int *);
int ngcliContextUnregisterInvokeServerManager(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
int ngcliContextRegisterCommunicationProxyManager(
    ngclContext_t *, ngcliCommunicationProxyManager_t *, int *);
int ngcliContextUnregisterCommunicationProxyManager(
    ngclContext_t *, ngcliCommunicationProxyManager_t *, int *);
ngcliInvokeServerManager_t *ngcliContextGetInvokeServerManager(
    ngclContext_t *, char *, int, int *);
ngcliInvokeServerManager_t *ngcliContextGetNextInvokeServerManager(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
int ngcliContextNotifyExecutable(ngclContext_t *, ngLog_t *, int *);
int ngcliContextRandomNumberGet(ngclContext_t *, long *, int *);

/* HeartBeat */
int ngcliHeartBeatInitialize(ngclContext_t *, int *);
int ngcliHeartBeatFinalize(ngclContext_t *, int *);
int ngcliHeartBeatIntervalChange(ngclContext_t *, int *);

/* Session Timeout */
int ngcliSessionTimeoutInitialize(ngclContext_t *, int *);
int ngcliSessionTimeoutFinalize(ngclContext_t *, int *);
int ngcliSessionTimeoutSessionStart(ngclContext_t *, ngclSession_t *, int *);
int ngcliSessionTimeoutSessionDone(ngclContext_t *, ngclSession_t *, int *);

/* Transfer Timeout */
int ngcliTransferTimeoutInitialize(ngclContext_t *, int *);
int ngcliTransferTimeoutFinalize(ngclContext_t *, int *);
int ngcliTransferTimeoutTransferStart(
    ngclSession_t *, ngcliTransferTimeoutType_t, int *);
int ngcliTransferTimeoutTransferDone(
    ngclSession_t *, ngcliTransferTimeoutType_t, int *);

/* Job Start Timeout */
int ngcliJobStartTimeoutInitialize(ngclContext_t *, int *);
int ngcliJobStartTimeoutFinalize(ngclContext_t *, int *);
int ngcliJobStartTimeoutJobStart(ngclContext_t *, ngcliJobManager_t *,
    int *);
int ngcliJobStartTimeoutJobStarted(ngclContext_t *, ngclExecutable_t *,
    int *);

/* Configuration file */
int ngcliConfigFileRead(ngclContext_t *, char *, int *);

/* Context for Local Machine Information */
int ngcliContextRegisterLocalMachineInformation(
    ngclContext_t *, ngcliLocalMachineInformationManager_t *, int *);
int ngcliContextUnregisterLocalMachineInformation(
    ngclContext_t *, ngcliLocalMachineInformationManager_t *, int *);

/* Context for Invoke Server Information */
int ngcliContextRegisterInvokeServerInformation(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *, int *);
int ngcliContextUnregisterInvokeServerInformation(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *, int *);

/* Context for Communication Proxy Information */
int ngcliContextRegisterCommunicationProxyInformation(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *, int *);
int ngcliContextUnregisterCommunicationProxyInformation(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *, int *);

/* Context for Information Service Information */
int ngcliContextRegisterInformationServiceInformation(
    ngclContext_t *, ngcliInformationServiceInformationManager_t *, int *);
int ngcliContextUnregisterInformationServiceInformation(
    ngclContext_t *, ngcliInformationServiceInformationManager_t *, int *);

/* Context for Remote Machine Information */
int ngcliContextRegisterRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
int ngcliContextUnregisterRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);

/* Context for Default Remote Machine Information */
int ngcliContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
int ngcliContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);

/* Context for Executable Path Information */
int ngcliContextRegisterExecutablePathInformation(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);
int ngcliContextUnregisterExecutablePathInformation(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);

/* Context for Remote Class Information */
int ngcliContextRegisterRemoteClassInformation(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);
int ngcliContextUnregisterRemoteClassInformation(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);

/* Local Machine Information */
int ngcliLocalMachineInformationCacheRegister(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationCacheUnregister(ngclContext_t *, int *);
ngcliLocalMachineInformationManager_t *ngcliLocalMachineInformationCacheGet(
    ngclContext_t *context, int *error);
ngclLocalMachineInformation_t * ngcliLocalMachineInformationAllocate(
    ngclContext_t *, int *);
int ngcliLocalMachineInformationFree(ngclContext_t *,
    ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationInitialize(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationCopy(ngclContext_t *,
    ngclLocalMachineInformation_t *, ngclLocalMachineInformation_t *, int *);

/* Invoke Server Information */
int ngcliInvokeServerInformationCacheRegister(
    ngclContext_t *, ngclInvokeServerInformation_t *, int *);
int ngcliInvokeServerInformationCacheUnregister(ngclContext_t *, char *, int *);
ngcliInvokeServerInformationManager_t *
    ngcliInvokeServerInformationCacheGet(ngclContext_t *, char *, int *);
ngcliInvokeServerInformationManager_t *
    ngcliInvokeServerInformationCacheGetNext(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *, int *error);
ngclInvokeServerInformation_t *ngcliInvokeServerInformationAllocate(
    ngclContext_t *, int *);
int ngcliInvokeServerInformationFree(ngclContext_t *,
    ngclInvokeServerInformation_t *, int *);
int ngcliInvokeServerInformationInitialize(ngclContext_t *,
    ngclInvokeServerInformation_t *, int *);
int ngcliInvokeServerInformationCopy(ngclContext_t *,
    ngclInvokeServerInformation_t *, ngclInvokeServerInformation_t *, int *);

/* Communication Proxy Information */
int ngcliCommunicationProxyInformationCacheRegister(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);
int ngcliCommunicationProxyInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngcliCommunicationProxyInformationManager_t *
    ngcliCommunicationProxyInformationCacheGet(
    ngclContext_t *, char *, int *a);
ngcliCommunicationProxyInformationManager_t *
    ngcliCommunicationProxyInformationCacheGetNext(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *, int *);
ngclCommunicationProxyInformation_t *
    ngcliCommunicationProxyInformationAllocate(ngclContext_t *, int *);
int ngcliCommunicationProxyInformationFree(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);
int ngcliCommunicationProxyInformationCopy(
    ngclContext_t *, ngclCommunicationProxyInformation_t *,
    ngclCommunicationProxyInformation_t *dest, int *);
int ngcliCommunicationProxyInformationInitialize(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);

/* Information Service Information */
int ngcliInformationServiceInformationCacheRegister(
    ngclContext_t *, ngclInformationServiceInformation_t *, int *);
int ngcliInformationServiceInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngcliInformationServiceInformationManager_t *
    ngcliInformationServiceInformationCacheGet(
    ngclContext_t *, char *, int *);
ngcliInformationServiceInformationManager_t *
    ngcliInformationServiceInformationCacheGetNext(
    ngclContext_t *, ngcliInformationServiceInformationManager_t *, int *);
ngclInformationServiceInformation_t *
    ngcliInformationServiceInformationAllocate( ngclContext_t *, int *);
int ngcliInformationServiceInformationFree(
    ngclContext_t *, ngclInformationServiceInformation_t *, int *);
int ngcliInformationServiceInformationCopy(
    ngclContext_t *, ngclInformationServiceInformation_t *,
    ngclInformationServiceInformation_t *, int *);
int ngcliInformationServiceInformationInitialize(
    ngclContext_t *, ngclInformationServiceInformation_t *, int *);

/* Remote Machine Information */
int ngcliRemoteMachineInformationCacheRegister(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngcliRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGet(ngclContext_t *, char *, int *);
ngcliRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGetWithTag(ngclContext_t *,
    char *, char *, int *);
ngcliRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGetNext(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *error);
ngclRemoteMachineInformation_t * ngcliRemoteMachineInformationAllocate(
    ngclContext_t *, int *);
int ngcliRemoteMachineInformationFree(ngclContext_t *,
    ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationInitialize(ngclContext_t *,
    ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *,
    ngclRemoteMachineInformation_t *, int *);

int ngcliRemoteMachineInformationAppendMPInCPUs(
    ngclRemoteMachineInformation_t *,
    char *, int, ngLog_t *log, int *);
int ngcliRemoteMachineInformationGetMPInCPUs(
    ngclRemoteMachineInformation_t *,
    char *, ngLog_t *, int *);

/* Default Remote Machine Information */
int ngcliDefaultRemoteMachineInformationCacheRegister(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngcliDefaultRemoteMachineInformationCacheUnregister(
    ngclContext_t *, int *);
ngcliRemoteMachineInformationManager_t *
    ngcliDefaultRemoteMachineInformationCacheGet(
    ngclContext_t *, int *);

/* Remote Class Information */
int ngcliRemoteClassInformationCacheRegister(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngcliRemoteClassInformationManager_t *ngcliRemoteClassInformationCacheGet(
    ngclContext_t *, char *, int *);
ngcliRemoteClassInformationManager_t *
    ngcliRemoteClassInformationCacheGetNext(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);
int ngcliRemoteClassInformationCacheInactivate(
    ngclContext_t *, char *, int *);
ngRemoteClassInformation_t *ngcliRemoteClassInformationAllocate(
    ngclContext_t *, int *);
int ngcliRemoteClassInformationFree(ngclContext_t *,
    ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationInitialize(ngclContext_t *,
    ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationCopy(ngclContext_t *,
    ngRemoteClassInformation_t *, ngRemoteClassInformation_t *, int *);

int ngcliParseRemoteExecutableInformation(ngclContext_t *, ngiLineList_t *,
    ngclExecutablePathInformation_t *, ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationGenerate(ngclContext_t *, char *, int,
    ngRemoteClassInformation_t *, int *);

/* Executable Path Information */
int ngcliExecutablePathInformationCacheRegister(ngclContext_t *,
    ngclExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationCacheUnregister(ngclContext_t *,
    char *, char *, int *);
ngcliExecutablePathInformationManager_t *
    ngcliExecutablePathInformationCacheGet(ngclContext_t *,
    char *, char *, int *);
int ngcliExecutablePathInformationCacheInactivate(
    ngclContext_t *, char *, char *, int *);
ngcliExecutablePathInformationManager_t *
    ngcliExecutablePathInformationCacheGetNext(ngclContext_t *,
    ngcliExecutablePathInformationManager_t *, int *);
ngclExecutablePathInformation_t * ngcliExecutablePathInformationAllocate(
    ngclContext_t *, int *);
int ngcliExecutablePathInformationFree(ngclContext_t *,
    ngclExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationInitialize(
    ngclContext_t *, ngclExecutablePathInformation_t *, int *);
int ngclExecutablePathInformationGetCopy(
    ngclContext_t *, char *, char *, 
    ngclExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationGetCopyWithQuery(
    ngclContext_t *, char *, char *, char *, 
    ngclExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationCopy(ngclContext_t *,
    ngclExecutablePathInformation_t *,
    ngclExecutablePathInformation_t *, int *);

/* Remote Method Information */
ngRemoteMethodInformation_t *ngcliRemoteMethodInformationCacheGet(
    ngclContext_t *, ngRemoteClassInformation_t *, char *, int *);
ngRemoteMethodInformation_t *ngcliRemoteMethodInformationCacheGetNext(
    ngclContext_t *, ngRemoteClassInformation_t *,
    ngRemoteMethodInformation_t *, int *);
int ngcliRemoteMethodInformationGetCopy(ngclContext_t *,
    ngRemoteClassInformation_t *, char *,
    ngRemoteMethodInformation_t *, int *);
ngRemoteMethodInformation_t *ngcliRemoteMethodInformationAllocate(
    ngclContext_t *, int, int *);
int ngcliRemoteMethodInformationFree(ngclContext_t *,
    ngRemoteMethodInformation_t *, int *);
int ngcliRemoteMethodInformationCopy(ngclContext_t *,
    ngRemoteMethodInformation_t *, ngRemoteMethodInformation_t *, int *);
int ngcliRemoteMethodInformationRelease(ngclContext_t *,
    ngRemoteMethodInformation_t *, int *);
int ngcliRemoteMethodInformationInitialize(ngclContext_t *,
    ngRemoteMethodInformation_t *, int *);
ngArgumentInformation_t *ngcliArgumentInformationAllocate(
    ngclContext_t *, int, int *);
int ngcliArgumentInformationInitialize(ngclContext_t *,
    ngArgumentInformation_t *, int *);
ngSubscriptInformation_t *ngcliSubscriptInformationAllocate(
    ngclContext_t *, int, int *);
int ngcliSubscriptInformationInitialize(
    ngclContext_t *, ngSubscriptInformation_t *, int *);
ngExpressionElement_t *ngcliExpressionElementAllocate(
    ngclContext_t *, int, int *);
int ngcliExpressionElementInitialize(
    ngclContext_t *, ngExpressionElement_t *, int *);


/* Job Manager */
ngcliJobManager_t *ngcliJobConstruct(
    ngclContext_t *, ngclExecutable_t *, ngcliJobAttribute_t *, int *);
int ngcliJobDestruct(ngcliJobManager_t *, int *);
int ngcliJobNotifyStatus(
    ngcliJobManager_t *, ngcliJobStatus_t, ngLog_t *, int *);
int ngcliJobWaitStatus(
    ngcliJobManager_t *, ngcliJobStatus_t, ngLog_t *, int *);
int ngcliJobSetEndTimeOfInvoke(ngcliJobManager_t *, ngLog_t *, int *);
int ngcliJobIsValid(ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliJobAttributeInitialize(
    ngclContext_t *, ngclExecutableAttribute_t *, ngcliJobAttribute_t *, int *);
int ngcliJobAttributeFinalize(
    ngclContext_t *, ngcliJobAttribute_t *, int *);
int ngcliJobAttributeCopy(
    ngclContext_t *, ngcliJobAttribute_t *, ngcliJobAttribute_t *, int *);
int ngcliJobAttributeRelease(ngclContext_t *, ngcliJobAttribute_t *, int *);
int ngcliJobRegisterExecutable(
    ngcliJobManager_t *, ngclExecutable_t *, int, int *error);
int ngcliJobUnregisterExecutable(
    ngcliJobManager_t *, ngclExecutable_t *, int *error);
int ngcliJobUnregisterExecutableFromDestructionList(
    ngcliJobManager_t *, ngclExecutable_t *, int *, int *);
ngclExecutable_t *ngcliJobGetExecutable(ngcliJobManager_t *, int, int *);
ngclExecutable_t *ngcliJobGetNextExecutable(
    ngcliJobManager_t *, ngclExecutable_t *, int *);
int ngcliJobStart(ngcliJobManager_t *, int *);
int ngcliJobRequestCancel(ngcliJobManager_t *, ngLog_t *, int *);
int ngcliJobStop(ngcliJobManager_t *, int *);

int ngcliJobGetLocalMachineInformationFortranCompatible(
    ngcliJobManager_t *, int *, int *);

/* Invoke Server */
ngcliInvokeServerManager_t *ngcliInvokeServerConstruct(
    ngclContext_t *, char *, ngcliJobManager_t *, int *);
int ngcliInvokeServerDestruct(
    ngclContext_t *, ngcliInvokeServerManager_t *, int, int *);
int ngcliInvokeServerRetire(ngclContext_t *, int *);
int ngcliInvokeServerJobInitialize(
    ngclContext_t *, ngcliInvokeServerJob_t *, int *);
int ngcliInvokeServerJobFinalize(
    ngclContext_t *, ngcliInvokeServerJob_t *, int *);
int ngcliInvokeServerJobStart(ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliInvokeServerJobStop(
    ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliInvokeServerJobCancel(
    ngclContext_t *, ngcliJobManager_t *, int *, int *);
int ngcliInvokeServerJobStatusGet(ngclContext_t *, ngcliJobManager_t *, int *);

/* Communication Proxy */
ngcliCommunicationProxyManager_t *ngcliCommunicationProxyManagerConstruct(
    ngclContext_t *, int , int *);
int ngcliCommunicationProxyManagerDestruct(
    ngcliCommunicationProxyManager_t *, int *);
int ngcliCommunicationProxyManagerRetire(
    ngcliCommunicationProxyManager_t *, int *);
int ngcliCommunicationProxyManagerPrepareCommunication(
    ngcliCommunicationProxyManager_t *, ngclRemoteMachineInformation_t *, ngiLineList_t **, int *);
int ngcliCommunicationProxyManagerUnref(
    ngcliCommunicationProxyManager_t *, int, int *);

/* Information Service */
ngcliInformationServiceManager_t *ngcliInformationServiceManagerConstruct(ngclContext_t *, int *);
int ngcliInformationServiceManagerDestruct(ngcliInformationServiceManager_t *, int *);
int ngcliInformationServiceManagerQuery(ngcliInformationServiceManager_t *,
    char *, char *, char *, 
    ngclExecutablePathInformation_t *, ngRemoteClassInformation_t *, int *);

ngcliQueryManager_t *ngcliQueryManagerConstruct(ngclContext_t *, int *);
int ngcliQueryManagerDestruct(ngcliQueryManager_t *, int *);
int ngcliQueryManagerQuery(ngcliQueryManager_t *, char *, char *, char *, 
    ngclExecutablePathInformation_t *, ngRemoteClassInformation_t *, int *);
int ngcliQueryManagerLockForReread(ngcliQueryManager_t *, int *);
int ngcliQueryManagerReconstruct(ngcliQueryManager_t *, int *);
int ngcliQueryManagerUnlockForReread(ngcliQueryManager_t *, int *);

/* Executable */
int ngcliExecutableListRotate(ngclContext_t *, ngLog_t *, int *);

int ngcliExecutableConnecting(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliExecutableConnected(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableIdle(ngclExecutable_t *, ngLog_t *, int *);

int ngcliExecutableJobDone(ngcliJobManager_t *, ngLog_t *, int *);
int ngcliExecutableExecuteSession(ngclExecutable_t *, ngLog_t *, int *);

int ngcliExecutableLock(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableTryLock(ngclExecutable_t *, int *, ngLog_t *, int *);
int ngcliExecutableUnlock(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableLockWithSend(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableUnlockWithSend(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableUnlockWithAll(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableLockTrySend(ngclExecutable_t *, int *, ngLog_t *, int *);

int ngcliExecutableNsessionsIncrement(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableNsessionsDecrement(ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableNsessionsWaitUntilZero(ngclExecutable_t *, ngLog_t *, int *);

int ngcliExecutableStatusSet(
    ngclExecutable_t *, ngclExecutableStatus_t, ngLog_t *, int *);
int ngcliExecutableStatusWait(
    ngclExecutable_t *, ngclExecutableStatus_t, ngLog_t *, int *);
int ngcliExecutableStatusWaitTimeout(
    ngclExecutable_t *, ngclExecutableStatus_t, int, ngLog_t *, int *);
int ngcliExecutableStatusWaitConnected(
    ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableStatusWaitIdle(ngclExecutable_t *, ngLog_t *, int *);

int ngcliExecutableStatusWaitExited(
    ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableRemoteClassInformationNotify(
    ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableRemoteClassInformationWait(
    ngclExecutable_t *, ngLog_t *, int *);
int ngcliExecutableUnusable(ngclExecutable_t *, int, int *);

int ngcliExecutableSetError(ngclExecutable_t *, int, int *);
int ngcliExecutableSetCbError(ngclExecutable_t *, int, int *);
int ngcliExecutableGetCbError(ngclExecutable_t *, int *, int *);

int ngcliExecutableIsValid(ngclContext_t *, ngclExecutable_t *, int *);
int ngcliExecutableIsAvailable(ngclContext_t *, ngclExecutable_t *, int *);  

ngclSession_t *ngcliExecutableGetAllSessionList(ngclExecutable_t *, int *);
ngclSession_t *ngcliExecutableGetSessionList(
    ngclExecutable_t *, int *, int, int *);
ngclSession_t *ngcliExecutableGetSessionCancelList(
    ngclExecutable_t *, int *, int, int *);
ngclSession_t *ngcliExecutableGetAllSessionWaitList(ngclExecutable_t *, int *);
ngclSession_t *ngcliExecutableGetAllSessionCancelList(ngclExecutable_t *, int *);
int ngcliExecutableRemoteClassInformationCheck(
    ngclExecutable_t *, int *, int *);
int ngcliExecutableRemoteClassInformationArrived(
    ngclExecutable_t *, char *, int *);

int ngcliExecutableHeartBeatTimeInitialize(ngclExecutable_t *, int *);
int ngcliExecutableHeartBeatArrive(ngclExecutable_t *, int *error);

int ngcliExecutableGetLocalMachineInformationFortranCompatible(
    ngclExecutable_t *, int *, int *);

/* Session */
ngclSession_t *
ngcliSessionGetWaitList(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, ngLog_t *, int *);
int ngcliSessionReleaseWaitList(ngclSession_t *, ngLog_t *, int *);
ngclSession_t *
ngcliSessionGetCancelList(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, ngLog_t *, int *);

ngclSession_t *ngcliSessionGetNext(
    ngclExecutable_t *, ngclSession_t *, int *);

int ngcliSessionSetExecutionTime(
    ngclContext_t *, ngcliJobManager_t *, ngclExecutable_t *,
    ngclSession_t *, ngLog_t *, int *);

int ngcliSessionWasDone(ngclSession_t *, ngLog_t *, int *);

int ngcliSessionStatusSetDone(ngclSession_t *, ngLog_t *, int *);
int ngcliSessionStatusSet(
    ngclSession_t *, ngclSessionStatus_t, ngLog_t *, int *);
int ngcliSessionStatusWait(
    ngclSession_t *, ngclSessionStatus_t, ngLog_t *, int *);
int ngcliSessionStatusWaitGreaterEqual(
    ngclSession_t *, ngclSessionStatus_t, ngLog_t *, int *);
int ngcliSessionUnusable(ngclExecutable_t *, ngclSession_t *, int, int *);

int ngcliSessionIsValid(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);

int ngcliSessionInvokeCallback(void (*)(void), ngiArgument_t *, ngLog_t *, int *);

char *ngcliSessionStatusToString(ngclSessionStatus_t);

/* Protocol */
int ngcliProtocolRequestQueryFunctionInformation(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);
#if 0 /* Temporary comment out */
int ngcliProtocolRequestQueryExecutableInformation(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);
#endif /* Temporary comment out */
int ngcliProtocolRequestResetExecutable(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestExitExecutable(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestInvokeSession(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestTransferResultData(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestCancelSession(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestTransferCallbackArgumentData(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngcliProtocolRequestTransferCallbackResultData(
    ngclSession_t *, ngiProtocol_t *, int, ngLog_t *, int *);
int ngcliProtocolRequestConnectionClose(
    ngclExecutable_t *, ngiProtocol_t *, ngLog_t *, int *);

int ngcliCallbackAccept(void *, ngiIOhandle_t *, ngiIOhandleState_t,
    ngLog_t *, int *);

ngRemoteMethodInformation_t *ngcliProtocolGetRemoteMethodInformation(
    ngiProtocol_t *, ngLog_t *, int *);
ngiArgument_t *ngcliProtocolGetArgument(ngiProtocol_t *, ngLog_t *, int *);
char *ngcliProtocolGetSessionInformation(ngiProtocol_t *, ngLog_t *, int *);

/* Log */
void ngcliLogPrintfJob(ngcliJobManager_t *,
    const char *, ngLogLevel_t, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(5, 6);

void ngcliLogDebugJob(
    ngcliJobManager_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngcliLogInfoJob(
    ngcliJobManager_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngcliLogWarnJob(
    ngcliJobManager_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngcliLogErrorJob(
    ngcliJobManager_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngcliLogFatalJob(
    ngcliJobManager_t *, const char *, const char *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);

#endif /* _NGCLIENTINTERNAL_H */
