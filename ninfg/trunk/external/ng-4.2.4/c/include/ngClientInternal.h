/* 
 * $RCSfile: ngClientInternal.h,v $ $Revision: 1.140 $ $Date: 2008/07/17 06:55:23 $
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
#ifndef _NGCLIENTINTERNAL_H_
#define _NGCLIENTINTERNAL_H_

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

/* MDS */
#define NGCLI_MDS2_DEFAULT_PORT 2135
#define NGCLI_MDS4_DEFAULT_PORT 8443
#define NGCLI_MDS4_SERVICE_URL_PROTO "https"
#define NGCLI_MDS4_SERVICE_URL_PATH \
    "/wsrf/services/org/apgrid/ninf/ng4/grpcinfo/GrpcInfoService"

/* Job cancel command */
#define NGCLI_JOBCANCEL_COMMAND	 "globus-job-cancel"
#define NGCLI_JOBCANCEL_ARGUMENT "-q"

/* Invoke Server */
#define NGCLI_INVOKE_SERVER_NG_DIR_ENV_NAME      "NG_DIR"
#define NGCLI_INVOKE_SERVER_PROGRAM_NAME_BASE    "ng_invoke_server"
#define NGCLI_INVOKE_SERVER_PROGRAM_PATH_FORMAT  "%s/bin/%s.%s"
#define NGCLI_INVOKE_SERVER_LOG_FILE_SWITCH      "-l"
#define NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR  "\x0d\x0a"
#define NGCLI_INVOKE_SERVER_LINE_TERMINATOR_SIZE 2
#define NGCLI_INVOKE_SERVER_READ_BUFFER_INITIAL_SIZE 256
#define NGCLI_INVOKE_SERVER_RESULT_SUCCESS       "S"
#define NGCLI_INVOKE_SERVER_RESULT_FAILURE       "F"
#define NGCLI_INVOKE_SERVER_JOB_ID_STR_MAX       1024


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

/* Options of GASS server */
#define NGCLI_GASS_OPTION \
    GLOBUS_GASS_SERVER_EZ_LINE_BUFFER | \
    GLOBUS_GASS_SERVER_EZ_READ_ENABLE | \
    GLOBUS_GASS_SERVER_EZ_WRITE_ENABLE | \
    GLOBUS_GASS_SERVER_EZ_STDOUT_ENABLE | \
    GLOBUS_GASS_SERVER_EZ_STDERR_ENABLE | \
    GLOBUS_GASS_SERVER_EZ_TILDE_EXPAND | \
    GLOBUS_GASS_SERVER_EZ_TILDE_USER_EXPAND

/**
 * Attribute of Job Management.
 */
typedef struct ngcliJobAttribute_s {
    char	*ngja_gassUrl;		/* URL of GASS */
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
    ngcliExecutablePathInformation_t ngja_epInfo;/* Executable Path Information */
} ngcliJobAttribute_t;

/**
 * MDS Server Cache Get mode
 * If MDSserverInformationCacheGet() tagName is NULL,
 * MODE_MATCH   : return tagName != NULL mdsInfoMng if available.
 * MODE_PRECISE : search tagName == NULL mdsInfoMng.
 */
typedef enum ngcliMDSserverInformationCacheGetMode_e {
    NGCLI_MDS_SERVER_CACHE_GET_NONE,
    NGCLI_MDS_SERVER_CACHE_GET_MODE_MATCH,
    NGCLI_MDS_SERVER_CACHE_GET_MODE_PRECISE
} ngcliMDSserverInformationCacheGetMode_t;

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
    int ngjm_monMutexInitialized;
    globus_mutex_t	ngjm_monMutex;
    int ngjm_monCondInitialized;
    globus_cond_t	ngjm_monCond;

    /* Read/Write Lock for this instance */
    int ngjm_rwlOwnInitialized;
    ngRWlock_t	ngjm_rwlOwn;

    int ngjm_ID;		/* Job ID */
    int ngjm_useInvokeServer;		/* Is Invoke Server used? */
    ngcliJobAttribute_t	ngjm_attr;	/* Attribute of Job Manager */

    ngcliInvokeServerJob_t ngjm_invokeServerInfo; /* Invoke Server Job Data */
    char       *ngjm_callbackContact;	/* Contact for callback */
    char       *ngjm_jobContact;	/* Contact for job management */
    char       *ngjm_rmContact;		/* Resource Manager Contact */
    int		ngjm_nExecutables;	/* Number of Ninf-G Executables */
    int		ngjm_requestCancel;	/* Request cancel the Job */
    ngcliJobStatus_t ngjm_status;	/* Status of Job */

    ngExecutionTime_t ngjm_invoke;
    int		ngjm_invokeMeasured;
} ngcliJobManager_t;

/**
 * Information for Invoke Server Read Buffer
 */
typedef struct ngcliInvokeServerReadBuffer_s {
    char       *ngisrb_buf;
    int		ngisrb_bufSize;
    int		ngisrb_reachEOF;
} ngcliInvokeServerReadBuffer_t;

/**
 * Invoke Server Request
 */
typedef enum ngcliInvokeServerRequestType_e {
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_UNDEFINED,
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_CREATE,   /* JOB_CREATE */
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_STATUS,   /* JOB_STATUS */
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY,  /* JOB_DESTROY */
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_EXIT,         /* EXIT */
    NGCLI_INVOKE_SERVER_REQUEST_TYPE_NOMORE
} ngcliInvokeServerRequestType_t;

/**
 * Information for Invoke Server Request and Reply synchronize.
 */
typedef struct ngcliInvokeServerRequestReply_s {
    int         ngisrr_requesting;
    int         ngisrr_replied;
    int         ngisrr_requestJobStatus;
    char       *ngisrr_invokeJobID;
    int         ngisrr_result;
    int         ngisrr_status;
    char       *ngisrr_errorString;
} ngcliInvokeServerRequestReply_t;

/**
 * Information for Invoke Server Request or Reply reader thread.
 */
typedef struct ngcliInvokeServerReader_s {
    int         ngisr_continue;
    int         ngisr_stopped;
    globus_thread_t ngisr_thread;
    int         ngisr_fd;
    FILE       *ngisr_fp;
    ngcliInvokeServerReadBuffer_t ngisr_readBuffer;
} ngcliInvokeServerReader_t;

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

    int		ngism_working;		/* Invoke Server is not exit */
    int		ngism_valid;		/* Invoke Server is valid */
    int		ngism_errorCode;	/* error code for invalid */

    /* Mutex and condition variable for this instance */
    int		ngism_mutexInitialized;
    int		ngism_condInitialized;
    globus_mutex_t	ngism_mutex;
    globus_cond_t       ngism_cond;

    int         ngism_nJobsMax;         /* The maximum number of jobs */
    int		ngism_nJobsStart;	/* The count of start requested Jobs */
    int		ngism_nJobsStop;	/* The count of stop requested Jobs */
    int		ngism_nJobsDone;	/* The count of DONE Jobs */

    int		ngism_requestFd;	/* pipe to child stdin */
    FILE       *ngism_requestFp;
    int		ngism_replyFd;		/* pipe to child stdout */
    int		ngism_notifyFd;		/* pipe to child stderr */

    ngcliInvokeServerReader_t ngism_replyReader;
    ngcliInvokeServerReader_t ngism_notifyReader;

    int		ngism_maxRequestID;	/* ID for JOB_CREATE */
    ngcliInvokeServerRequestReply_t    ngism_requestReply;
} ngcliInvokeServerManager_t;

/**
 * Each Invoke Server count.
 */
typedef struct ngcliInvokeServerCount_s {
    struct ngcliInvokeServerCount_s *ngisc_next;

    char *ngisc_serverType;    /* Invoke Server Type */
    int   ngisc_count;         /* Invoke Server count */
} ngcliInvokeServerCount_t;


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

/* List of GASS Server Manager */
#define ngcliGASSserverManagerListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlGassMng, (log), (error));
#define ngcliGASSserverManagerListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlGassMng, (log), (error));
#define ngcliGASSserverManagerListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlGassMng, (log), (error));
#define ngcliGASSserverManagerListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlGassMng, (log), (error));

/* List of Local Machine Information */
#define ngcliLocalMachineInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlLmInfo, (log), (error));
#define ngcliLocalMachineInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlLmInfo, (log), (error));

/* List of MDS Server Information */
#define ngcliMDSserverInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlMdsInfo, (log), (error));
#define ngcliMDSserverInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlMdsInfo, (log), (error));
#define ngcliMDSserverInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlMdsInfo, (log), (error));
#define ngcliMDSserverInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlMdsInfo, (log), (error));

/* List of Invoke Server Information */
#define ngcliInvokeServerInformationListReadLock(context, log, error) \
    ngiRWlockReadLock(&(context)->ngc_rwlIsInfo, (log), (error));
#define ngcliInvokeServerInformationListReadUnlock(context, log, error) \
    ngiRWlockReadUnlock(&(context)->ngc_rwlIsInfo, (log), (error));
#define ngcliInvokeServerInformationListWriteLock(context, log, error) \
    ngiRWlockWriteLock(&(context)->ngc_rwlIsInfo, (log), (error));
#define ngcliInvokeServerInformationListWriteUnlock(context, log, error) \
    ngiRWlockWriteUnlock(&(context)->ngc_rwlIsInfo, (log), (error));

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
#define ngcliExecutablePathInformationListReadLock(rmInfo, log, error) \
    ngiRWlockReadLock(&(rmInfo)->ngrmim_rwlExecPath, (log), (error));
#define ngcliExecutablePathInformationListReadUnlock(rmInfo, log, error) \
    ngiRWlockReadUnlock(&(rmInfo)->ngrmim_rwlExecPath, (log), (error));
#define ngcliExecutablePathInformationListWriteLock(rmInfo, log, error) \
    ngiRWlockWriteLock(&(rmInfo)->ngrmim_rwlExecPath, (log), (error));
#define ngcliExecutablePathInformationListWriteUnlock(rmInfo, log, error) \
    ngiRWlockWriteUnlock(&(rmInfo)->ngrmim_rwlExecPath, (log), (error));

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

/* GASS Server Manager */
#define ngcliGASSserverManagerReadLock(gsMng, log, error) \
    ngiRWlockReadLock(&(gsMng)->nggsm_rwlOwn, (log), (error));
#define ngcliGASSserverManagerReadUnlock(gsMng, log, error) \
    ngiRWlockReadUnlock(&(gsMng)->nggsm_rwlOwn, (log), (error));
#define ngcliGASSserverManagerWriteLock(gsMng, log, error) \
    ngiRWlockWriteLock(&(gsMng)->nggsm_rwlOwn, (log), (error));
#define ngcliGASSserverManagerWriteUnlock(gsMng, log, error) \
    ngiRWlockWriteUnlock(&(gsMng)->nggsm_rwlOwn, (log), (error));

/* Local Machine Information */
#define ngcliLocalMachineInformationReadLock(lmInfo, log, error) \
    ngiRWlockReadLock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationReadUnlock(lmInfo, log, error) \
    ngiRWlockReadUnlock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationWriteLock(lmInfo, log, error) \
    ngiRWlockWriteLock(&(lmInfo)->nglmim_rwlOwn, (log), (error));
#define ngcliLocalMachineInformationWriteUnlock(lmInfo, log, error) \
    ngiRWlockWriteUnlock(&(lmInfo)->nglmim_rwlOwn, (log), (error));

/* MDS Server Information */
#define ngcliMDSserverInformationReadLock(mdsInfo, log, error) \
    ngiRWlockReadLock(&(mdsInfo)->ngmsim_rwlOwn, (log), (error));
#define ngcliMDSserverInformationReadUnlock(mdsInfo, log, error) \
    ngiRWlockReadUnlock(&(mdsInfo)->ngmsim_rwlOwn, (log), (error));
#define ngcliMDSserverInformationWriteLock(mdsInfo, log, error) \
    ngiRWlockWriteLock(&(mdsInfo)->ngmsim_rwlOwn, (log), (error));
#define ngcliMDSserverInformationWriteUnlock(mdsInfo, log, error) \
    ngiRWlockWriteUnlock(&(mdsInfo)->ngmsim_rwlOwn, (log), (error));

/* Invoke Server Information */
#define ngcliInvokeServerInformationReadLock(isInfo, log, error) \
    ngiRWlockReadLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationReadUnlock(isInfo, log, error) \
    ngiRWlockReadUnlock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationWriteLock(isInfo, log, error) \
    ngiRWlockWriteLock(&(isInfo)->ngisim_rwlOwn, (log), (error));
#define ngcliInvokeServerInformationWriteUnlock(isInfo, log, error) \
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
int ngcliNinfgManagerSetGlobusHostName(char *, ngLog_t *, int *);
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
ngcliInvokeServerManager_t *ngcliContextGetInvokeServerManager(
    ngclContext_t *, char *, int, int *);
ngcliInvokeServerManager_t *ngcliContextGetNextInvokeServerManager(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
int ngcliContextNotifyExecutable(ngclContext_t *, ngLog_t *, int *);

/* HeartBeat */
int ngcliHeartBeatInitialize(ngclContext_t *, int *);
int ngcliHeartBeatFinalize(ngclContext_t *, int *);
int ngcliHeartBeatIntervalChange(ngclContext_t *, int *);

/* Session Timeout */
int ngcliSessionTimeoutInitialize(ngclContext_t *, int *);
int ngcliSessionTimeoutFinalize(ngclContext_t *, int *);
int ngcliSessionTimeoutSessionStart(ngclContext_t *, ngclSession_t *, int *);
int ngcliSessionTimeoutSessionDone(ngclContext_t *, ngclSession_t *, int *);

/* Refresh proxy credentials */
int ngcliRefreshCredentialsInitialize(ngclContext_t *, int *);
int ngcliRefreshCredentialsFinalize(ngclContext_t *, int *);

/* Job Start Timeout */
int ngcliJobStartTimeoutInitialize(ngclContext_t *, int *);
int ngcliJobStartTimeoutFinalize(ngclContext_t *, int *);
int ngcliJobStartTimeoutJobStart(ngclContext_t *, ngcliJobManager_t *,
    int *);
int ngcliJobStartTimeoutJobStarted(ngclContext_t *, ngclExecutable_t *,
    int *);

/* Observe Thread */
ngcliObserveItem_t * ngcliObserveItemConstruct(
    ngclContext_t *, ngcliObserveThread_t *, int *);
int ngcliObserveItemDestruct(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);
ngcliObserveItem_t * ngcliObserveItemGetNext(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);
int ngcliObserveItemEventTimeChangeRequest(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);

int ngcliObserveThreadInitialize(ngclContext_t *, int *);
int ngcliObserveThreadFinalize(ngclContext_t *, int *);
int ngcliObserveThreadStart(ngclContext_t *, int *);
int ngcliObserveThreadStop(ngclContext_t *, int *);

/* GASS Server Manager */
ngclGASSserverManager_t *ngcliContextGASSserverManagerGet(
    ngclContext_t *, char *, int *);
ngclGASSserverManager_t *ngcliContextGASSserverManagerGetNext(
    ngclContext_t *, ngclGASSserverManager_t *, int *);

/* Configuration file */
int ngcliConfigFileRead(ngclContext_t *, char *, int *);

/* Local Machine Information */
int ngcliLocalMachineInformationCacheRegister(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationCacheUnregister(ngclContext_t *, int *);
ngclLocalMachineInformationManager_t *ngcliLocalMachineInformationCacheGet(
    ngclContext_t *context, int *error);
ngclLocalMachineInformation_t * ngcliLocalMachineInformationAllocate(
    ngclContext_t *, int *);
int ngcliLocalMachineInformationFree(ngclContext_t *,
    ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationInitialize(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
int ngcliLocalMachineInformationCopy(ngclContext_t *,
    ngclLocalMachineInformation_t *, ngclLocalMachineInformation_t *, int *);

/* MDS Server Information */
int ngcliContextRegisterMDSserverInformation(
    ngclContext_t *, ngcliMDSserverInformationManager_t *, int *);
int ngcliContextUnregisterMDSserverInformation(
    ngclContext_t *, ngcliMDSserverInformationManager_t *, int *);

/* Invoke Server Information */
int ngcliContextRegisterInvokeServerInformation(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *, int *);
int ngcliContextUnregisterInvokeServerInformation(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *, int *);

/* Remote Machine Information */
int ngcliRemoteMachineInformationCacheRegister(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngclRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGet(ngclContext_t *, char *, int *);
ngclRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGetWithTag(ngclContext_t *,
    char *, char *, int *);
ngclRemoteMachineInformationManager_t *
    ngcliRemoteMachineInformationCacheGetNext(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *error);
ngclRemoteMachineInformation_t * ngcliRemoteMachineInformationAllocate(
    ngclContext_t *, int *);
int ngcliRemoteMachineInformationFree(ngclContext_t *,
    ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationInitialize(ngclContext_t *,
    ngclRemoteMachineInformation_t *, int *);
int ngcliRemoteMachineInformationCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *,
    ngclRemoteMachineInformation_t *, int *);

/* Default Remote Machine Information */
int ngcliDefaultRemoteMachineInformationCacheRegister(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
int ngcliDefaultRemoteMachineInformationCacheUnregister(
    ngclContext_t *, int *);
ngclRemoteMachineInformationManager_t *
    ngcliDefaultRemoteMachineInformationCacheGet(
    ngclContext_t *, int *);

/* MDS Server Information */
int ngcliMDSserverInformationCacheRegister(
    ngclContext_t *, ngclMDSserverInformation_t *, int *);
int ngcliMDSserverInformationCacheUnregister(
    ngclContext_t *, char *, char *, int *);
ngcliMDSserverInformationManager_t *
    ngcliMDSserverInformationCacheGet(ngclContext_t *, char *, char *,
    ngcliMDSserverInformationCacheGetMode_t, int *);
ngcliMDSserverInformationManager_t *
    ngcliMDSserverInformationCacheGetNext(
    ngclContext_t *, ngcliMDSserverInformationManager_t *, int *error);
ngclMDSserverInformation_t *ngcliMDSserverInformationAllocate(
    ngclContext_t *, int *);
int ngcliMDSserverInformationFree(ngclContext_t *,
    ngclMDSserverInformation_t *, int *);
int ngcliMDSserverInformationInitialize(ngclContext_t *,
    ngclMDSserverInformation_t *, int *);
int ngcliMDSserverInformationCopy(ngclContext_t *,
    ngclMDSserverInformation_t *, ngclMDSserverInformation_t *, int *);

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

/* Remote Class Information */
int ngcliRemoteClassInformationCacheRegister(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationCacheUnregister(
    ngclContext_t *, char *, int *);
ngclRemoteClassInformationManager_t *ngcliRemoteClassInformationCacheGet(
    ngclContext_t *, char *, int *);
ngclRemoteClassInformationManager_t *
    ngcliRemoteClassInformationCacheGetNext(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
ngRemoteClassInformation_t *ngcliRemoteClassInformationAllocate(
    ngclContext_t *, int *);
int ngcliRemoteClassInformationFree(ngclContext_t *,
    ngRemoteClassInformation_t *, int *);
int ngcliRemoteClassInformationInitialize(ngclContext_t *,
    ngRemoteClassInformation_t *, int *);

int ngcliRemoteClassInformationCopy(ngclContext_t *,
    ngRemoteClassInformation_t *, ngRemoteClassInformation_t *, int *);
ngRemoteClassInformation_t *ngcliRemoteClassInformationGenerate(
    ngclContext_t *, char *, int, int *);

/* Executable Path Information */
int ngcliExecutablePathInformationCacheRegister(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationCacheUnregister(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *, char *, int *);
ngcliExecutablePathInformationManager_t *
    ngcliExecutablePathInformationCacheGet(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *, char *, int *);
ngcliExecutablePathInformationManager_t *
    ngcliExecutablePathInformationCacheGetNext(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
ngcliExecutablePathInformation_t * ngcliExecutablePathInformationAllocate(
    ngclContext_t *, int *);
int ngcliExecutablePathInformationFree(ngclContext_t *,
    ngcliExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationInitialize(
    ngclContext_t *, ngcliExecutablePathInformation_t *, int *);
int ngcliExecutablePathInformationCopy(ngclContext_t *,
    ngcliExecutablePathInformation_t *,
    ngcliExecutablePathInformation_t *, int *);

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
int ngcliJobCancelByForce(ngcliJobManager_t *, int *);
int ngcliJobIsAlive(ngclExecutable_t *, int *);
int ngcliJobSetCredential(gss_cred_id_t, int *);
int ngcliJobRefreshCredential(ngcliJobManager_t *, int *);

int ngcliJobGetLocalMachineInformationFortranCompatible(
    ngcliJobManager_t *, int *, int *);

/* Invoke Server */
ngcliInvokeServerManager_t *ngcliInvokeServerConstruct(
    ngclContext_t *, char *, ngcliJobManager_t *, int *);
int ngcliInvokeServerDestruct(
    ngclContext_t *, ngcliInvokeServerManager_t *, int, int *);
int ngcliInvokeServerJobInitialize(
    ngclContext_t *, ngcliInvokeServerJob_t *, int *);
int ngcliInvokeServerJobFinalize(
    ngclContext_t *, ngcliInvokeServerJob_t *, int *);
int ngcliInvokeServerProgramCheckAccess(
    ngclContext_t *, char *, char *, int *);
int ngcliInvokeServerJobStart(ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliInvokeServerJobStop(
    ngclContext_t *, ngcliJobManager_t *, int *);
int ngcliInvokeServerJobCancel(
    ngclContext_t *, ngcliJobManager_t *, int *, int *);
int ngcliInvokeServerJobStatusGet(ngclContext_t *, ngcliJobManager_t *, int *);

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

int ngcliExecutableIoCallbackStart(ngclExecutable_t *, ngLog_t *,int *error);
int ngcliExecutableIoCallbackFinish(ngclExecutable_t *, ngLog_t *,int *error);
int ngcliExecutableWaitIoCallbackFinished(ngclExecutable_t *, ngLog_t *,int *error);

int ngcliExecutableHeartBeatTimeInitialize(ngclExecutable_t *, int *);
int ngcliExecutableHeartBeatArrive(ngclExecutable_t *, int *error);
int ngcliExecutableHeartBeatDataTransferStart(
    ngclExecutable_t *, int *);
int ngcliExecutableHeartBeatDataTransferStop(
    ngclExecutable_t *, int *);

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

int ngcliSessionInvokeCallback(void (*)(), ngiArgument_t *, ngLog_t *, int *);

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

void ngcliCallbackAccept(void *, globus_io_handle_t *, globus_result_t);

int ngcliCallbackIsCanceled(globus_result_t, ngLog_t *, int *);

ngRemoteMethodInformation_t *ngcliProtocolGetRemoteMethodInformation(
    ngiProtocol_t *, ngLog_t *, int *);
ngiArgument_t *ngcliProtocolGetArgument(ngiProtocol_t *, ngLog_t *, int *);
char *ngcliProtocolGetSessionInformation(ngiProtocol_t *, ngLog_t *, int *);

/* MDS */
int ngcliMDSaccessInitialize(ngclContext_t *, int *);
int ngcliMDSaccessFinalize(ngclContext_t *, int *);
int ngcliMDSaccessMDSserverInformationInitialize(
    ngclContext_t *, ngcliMDSserverInformationManager_t *, int *);
int ngcliMDSaccessMDSserverInformationFinalize(
    ngclContext_t *, ngcliMDSserverInformationManager_t *, int *);
int ngcliMDSaccessRemoteMachineInformationGet(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *, int *);
int ngcliMDSaccessRemoteExecutableInformationGet(ngclContext_t *,
    ngclRemoteMachineInformationManager_t *, char *, int *);

/* Log */
int ngcliLogPrintfJob(
    ngcliJobManager_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, ...);

/* Globus */
int ngcliGlobusInitialize(ngLog_t *, int *);
int ngcliGlobusFinalize(ngLog_t *, int *);
int ngcliGlobusCommonInitialize(ngLog_t *, int *);
int ngcliGlobusCommonFinalize(ngLog_t *, int *);
int ngcliGlobusGRAMclientInitialize(ngLog_t *, int *);
ngclGASSserverManager_t * ngcliGASSserverConstruct(
    char *gassScheme, unsigned long options, ngLog_t *, int *);
int ngcliGASSserverDestruct(ngclGASSserverManager_t *, ngLog_t *, int *);
int ngcliGlobusIoTcpRegisterListen(
    globus_io_handle_t *, globus_io_callback_t, void *, ngLog_t *, int *);
int ngcliGlobusIoRegisterSelect(
    globus_io_handle_t *, globus_io_callback_t, void *, ngLog_t *, int *);
int ngcliGlobusIoUnregister(globus_io_handle_t *, ngLog_t *, int *);
int ngcliGlobusAcquireCredential(gss_cred_id_t *, ngLog_t *, int *);

#endif /* _NGCLIENTINTERNAL_H */
