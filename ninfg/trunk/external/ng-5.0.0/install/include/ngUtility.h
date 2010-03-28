/* 
 * $RCSfile: ngUtility.h,v $ $Revision: 1.70 $ $Date: 2008/03/28 08:50:58 $
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
#ifndef _NGUTILITY_H_
#define _NGUTILITY_H_

#include "ngCommon.h"

/**
 * Calculate the number of element of array.
 */
#define NGI_NELEMENTS(array) (sizeof (array) / sizeof (array[0]))

/**
 * Time of execution.
 */
typedef struct ngExecutionTime_s {
    /* Real time */
    struct {
	struct timeval nget_start;	/* Start time */
	struct timeval nget_end;	/* End time */
	struct timeval nget_execution;	/* Execution time (End - Start) */
    } nget_real;

    /* CPU time */
    struct {
	struct timeval nget_start;	/* Start time */
	struct timeval nget_end;	/* End time */
	struct timeval nget_execution;	/* Execution time (End - Start) */
    } nget_cpu;
} ngExecutionTime_t;


/**
 * Thread.
 */

/* Common part between Pthread and NonThread. */
typedef enum ngiSynchronizedObjectStatus_e {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED = 0,
    NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED,
    NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC
} ngiSynchronizedObjectStatus_t;

#ifdef NG_PTHREAD

/**
 * Thread for Pthread version.
 */

typedef pthread_t ngiThread_t;

/**
 * Thread: Mutex
 */
typedef struct ngiMutex_s {
    ngiSynchronizedObjectStatus_t ngm_status;
    pthread_mutex_t               ngm_mutex;
} ngiMutex_t;

#define NGI_MUTEX_INITIALIZER \
    {NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC, PTHREAD_MUTEX_INITIALIZER}

/**
 * Thread: Condition variable
 */
typedef struct ngiCond_s {
    ngiSynchronizedObjectStatus_t ngc_status;
    pthread_cond_t                ngc_cond;
} ngiCond_t;

#define NGI_COND_INITIALIZER \
    {NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC, PTHREAD_COND_INITIALIZER}

/**
 * Thread: Read/Write Lock
 */
#define NG_RWLOCK_MAX_LOCKS INT_MAX
typedef struct ngiRWlock_s {
    ngiSynchronizedObjectStatus_t ngrwl_status;
    ngiMutex_t                    ngrwl_mutex;
    ngiCond_t                     ngrwl_condWrite;
    ngiCond_t                     ngrwl_condRead;

    /* Number of threads, it waiting for write lock */
    int                           ngrwl_nWaitingWriteLock;

    int                           ngrwl_readLockCounter;
    int                           ngrwl_writeLockCounter;
    ngiThread_t                   ngrwl_owner;

    /* Write Lock -> Write Lock   O.K.
     * Write Lock -> Read Lock    O.K.
     * Read Lock  -> Write Lock   Dead Lock
     */
} ngiRWlock_t;

/**
 * Thread: Reentrant Lock
 */
typedef struct ngiRlock_s {
    ngiSynchronizedObjectStatus_t ngrl_status;
    ngiMutex_t                    ngrl_mutex;
    ngiCond_t                     ngrl_condLock;
    ngiCond_t                     ngrl_condSignal;
    int                           ngrl_lockLevel;
    int                           ngrl_nLockWaiter;
    int                           ngrl_nSignalWaiter;
    int                           ngrl_signalNumber;
    ngiThread_t                   ngrl_owner;
} ngiRlock_t;

#else /* NG_PTHREAD */

/**
 * Thread for NonThread version.
 */

typedef int ngiThread_t;

/**
 * NonThread: Mutex
 */
typedef struct ngiMutex_s {
    ngiSynchronizedObjectStatus_t ngm_status;
    int                           ngm_lockCount;
} ngiMutex_t;

#define NGI_MUTEX_INITIALIZER \
    {NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC, 0}

/**
 * NonThread: Condition variable
 */
typedef struct ngiCond_s {
    ngiSynchronizedObjectStatus_t ngc_status;
    struct ngiEvent_s             *ngc_event;
    int                           ngc_waiting;
    int                           ngc_signaled;
} ngiCond_t;

#define NGI_COND_INITIALIZER \
    {NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC, 0}

/**
 * NonThread: Read/Write Lock
 */
typedef struct ngiRWlock_s {
    ngiSynchronizedObjectStatus_t ngrwl_status;
    int                           ngrwl_readLockCount;
    int                           ngrwl_writeLockCount;
} ngiRWlock_t;

/**
 * NonThread: Reentrant Lock
 */
typedef struct ngiRlock_s {
    ngiSynchronizedObjectStatus_t ngrl_status;
    ngiMutex_t                    ngrl_mutex;
    ngiCond_t                     ngrl_cond;
    int                           ngrl_lockCount;
} ngiRlock_t;

#endif /* NG_PTHREAD */

/**
 * Thread for Pthread and NonThread common part.
 */

/**
 * Mutex
 */
#define NGI_MUTEX_NULL ngiMutexNull
#define NGI_MUTEX_IS_NULL(mutex) \
    ((mutex)->ngm_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED)
#define NGI_MUTEX_IS_VALID(mutex) (!NGI_MUTEX_IS_NULL(mutex))

extern const ngiMutex_t ngiMutexNull;

/**
 * Condition variable
 */
#define NGI_COND_NULL ngiCondNull;
#define NGI_COND_IS_NULL(cond) \
    ((cond)->ngc_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED)
#define NGI_COND_IS_VALID(cond) (!NGI_COND_IS_NULL(cond))

extern const ngiCond_t ngiCondNull;

/**
 * Read/Write Lock
 */
#define NG_RWLOCK_MAX_LOCKS INT_MAX

#define NGI_RWLOCK_NULL ngiRWlockNull
#define NGI_RWLOCK_IS_NULL(rwlock) \
    ((rwlock)->ngrwl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED)
#define NGI_RWLOCK_IS_VALID(rwlock) (!NGI_RWLOCK_IS_NULL(rwlock))

extern const ngiRWlock_t ngiRWlockNull;

/**
 * Reentrant Lock
 */
#define NGI_RLOCK_NULL ngiRlockNull
#define NGI_RLOCK_IS_NULL(rlock) \
    ((rlock)->ngrl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED)
#define NGI_RLOCK_IS_VALID(rlock) (!NGI_RLOCK_IS_NULL(rlock))

extern const ngiRlock_t ngiRlockNull;


/**
 * Log manager.
 */

/* Level of log */
typedef enum ngLogLevel_e {
    NG_LOG_LEVEL_OFF = 0,
    NG_LOG_LEVEL_FATAL,
    NG_LOG_LEVEL_ERROR,
    NG_LOG_LEVEL_WARNING,
    NG_LOG_LEVEL_INFORMATION,
    NG_LOG_LEVEL_DEBUG
} ngLogLevel_t;

/* Output */
typedef enum ngLogOutput_e {
    NG_LOG_NONE,
    NG_LOG_STDERR,
    NG_LOG_FILE
} ngLogOutput_t;

/* Category */
typedef struct ngLogCategory_s {
    char         *nglc_string;
    ngLogLevel_t  nglc_level;
} ngLogCategory_t;

/* Construct Argument */
typedef struct ngLogConstructArgument_s {
    ngLogOutput_t       nglca_output; 

    char               *nglca_filePath;    /* File name of log file */
    char               *nglca_suffix;	   /* Suffix of log file */
    int	                nglca_nFiles;	   /* Maximum number of log file */
    int	                nglca_maxFileSize; /* Maximum byte size of log file */
    int	                nglca_overWriteDir;/* Over write to the directory */
    int                 nglca_appending;   /* Open log file for appending(for External Module) */

    ngLogCategory_t    *nglca_categories;
} ngLogConstructArgument_t;

typedef struct ngLogBase_s {
    ngiMutex_t               nglb_mutex;        /* Mutex for this instance */
    ngLogConstructArgument_t nglb_arg;          /* Log information */
    ngLogLevel_t             nglb_defaultLevel; /* Default Log Level */

    FILE  *nglb_stream;             /* File stream of output */
    char  *nglb_fileFormat;         /* String decoded log_filePath */
    int	   nglb_currentNo;          /* Number of file of current */
    char  *nglb_fileName;           /* File name of current file */
    size_t nglb_fileNameBufferSize; /* File name buffer size */
    int	   nglb_nFigures;           /* Number of figures of file number */
    char  *nglb_appName;            /* Application name */
    size_t nglb_outputNbytes;       /* The output number of bytes */

    char  *nglb_hostName;           /* Host name */
    pid_t  nglb_pid;                /* Process ID */

    /* Only for Executable */
    int    nglb_executableID;       /* Executable ID for filename */
} ngLogBase_t;

typedef struct ngLog_s {
    ngLogBase_t ngl_base;
} ngLog_t;

typedef struct ngCommLogPairInfo_s {
    char *ngcp_localAppName;
    char *ngcp_localHostname;
    char *ngcp_remoteAppName;
    char *ngcp_remoteHostname;
} ngCommLogPairInfo_t;

typedef struct ngCommLog_s {
    ngLogBase_t         ngcl_base;
    ngCommLogPairInfo_t ngcl_pairInfo;
    ngExecutionTime_t   ngcl_execTime;
    int                 ngcl_firstTime;
} ngCommLog_t;

/* Log */
#define NGI_LOG_DEFAULT_GENERIC_LOGLEVEL       NG_LOG_LEVEL_ERROR
#define NGI_LOG_DEFAULT_COMMUNICATION_LOGLEVEL NG_LOG_LEVEL_OFF
#define NGI_LOG_EXECUTABLE_ID_NOT_APPEND (-1)
#define NGI_LOG_EXECUTABLE_ID_UNDEFINED  (-2)

#define NG_LOGCAT_DEFAULT        "default"
#define NG_LOGCAT_NINFG_LOG      "Ninf-G Log"
#define NG_LOGCAT_NINFG_LIB      "Ninf-G Library"
#define NG_LOGCAT_NINFG_IOEVENT  "Ninf-G I/O Event"
#define NG_LOGCAT_NINFG_PURE     "Ninf-G Pure"
#define NG_LOGCAT_NINFG_PROTOCOL "Ninf-G Protocol"
#define NG_LOGCAT_NINFG_GRPC     "Ninf-G GRPC"

#define NGI_ENVIRONMENT_LOG_LEVEL "NG_LOG_LEVEL"

/* default log rotation size */
#define NG_LOG_DEFAULT_ROTATION_SIZE (1 * 1024 * 1024) /* 1MB each */

/**
 * Time.
 */
#define NGI_TIMEVAL_STRING_MAX 128   /* "%lds %ldus" */

/**
 * Information about Debugger.
 */
typedef struct ngDebuggerInformation_s {
    int   ngdi_enable;          /* Debugging enable/disable */
    char *ngdi_terminalPath;    /* Path of Terminal emulator */
    char *ngdi_display;         /* DISPLAY environment variable */
    char *ngdi_debuggerPath;    /* Path of Debugger */
} ngDebuggerInformation_t;

/**
 * Random number
 */
typedef long ngiRandomNumber_t;

/**
 * Connect Retry Information and run status.
 */
typedef struct ngiConnectRetryInformation_s {
    int ngcri_count;
    int ngcri_interval;
    double ngcri_increase;
    int ngcri_useRandom;
} ngiConnectRetryInformation_t;

typedef struct ngiConnectRetryStatus_s {
    ngiConnectRetryInformation_t ngcrs_retryInfo;
    int ngcrs_retry;
    double ngcrs_nextInterval;
    ngiRandomNumber_t *ngcrs_randomSeed;
} ngiConnectRetryStatus_t;

/**
 * Utility.
 */
#define NGI_GETPWUID_BUFSIZE 1024

/**
 * Selector.
 */

typedef void *(*ngiSelectorConstruct_t)(
    ngLog_t *, int *);
typedef int (*ngiSelectorDestruct_t)(
    void *, ngLog_t *, int *);
typedef int (*ngiSelectorResize_t)(
    void *, int, ngLog_t *, int *);
typedef int (*ngiSelectorClear_t)(
    void *, ngLog_t *, int *);
typedef int (*ngiSelectorSet_t)(
    void *, int, int, int, ngLog_t *, int *);
typedef int (*ngiSelectorSetLast_t)(
    void *, int, ngLog_t *, int *);
typedef int (*ngiSelectorWait_t)(
    void *, ngLog_t *, int *);
typedef int (*ngiSelectorGet_t)(
    void *, int, int *, int *, ngLog_t *, int *);
typedef int (*ngiSelectorGetLast_t)(
    void *, int *, int *, int *, int *, ngLog_t *, int *);

typedef struct ngiSelectorFunctions_s {
    ngiSelectorConstruct_t ngsc_selectorConstruct;
    ngiSelectorDestruct_t ngsc_selectorDestruct;
    ngiSelectorResize_t ngsc_selectorResize;
    ngiSelectorClear_t ngsc_selectorClear;
    ngiSelectorSet_t ngsc_selectorSet;
    ngiSelectorSetLast_t ngsc_selectorSetLast;
    ngiSelectorWait_t ngsc_selectorWait;
    ngiSelectorGet_t ngsc_selectorGet;
    ngiSelectorGetLast_t ngsc_selectorGetLast;
} ngiSelectorFunctions_t;

#define NGI_SELECTOR_FLAG_IN    0x001
#define NGI_SELECTOR_FLAG_OUT   0x002
#define NGI_SELECTOR_FLAG_HUP   0x004
#define NGI_SELECTOR_FLAG_OTHER 0x008

typedef int (*ngiSelectorFunctionsSetFunction_t)(
    ngiSelectorFunctions_t *, ngLog_t *, int *);

#ifdef NGI_POLL_ENABLED
#define NGI_SELECTOR_FUNCTIONS_DEFAULT ngiSelectorPollFunctionsSet
#else
#ifdef NGI_SELECT_ENABLED
#define NGI_SELECTOR_FUNCTIONS_DEFAULT ngiSelectorSelectFunctionsSet
#else
#define NGI_SELECTOR_FUNCTIONS_DEFAULT NULL
#endif
#endif

/**
 * Ninf-G Event and I/O module.
 */

#define NGI_IO_HANDLE_LISTENER_CREATE_BACKLOG_DEFAULT -1

/* Define this if the read/write timing and size debug output is required. */
#if 0
#define NGI_IO_HANDLE_DEBUG_OUTPUT_RW 1
#endif

#ifdef NG_PTHREAD
typedef ngiMutex_t ngiEventMutex_t;
typedef ngiCond_t ngiEventCond_t;
typedef ngiThread_t ngiEventThread_t;

#else /* NG_PTHREAD */
typedef int ngiEventMutex_t; /* Just to ignore. */
typedef int ngiEventCond_t;
typedef int ngiEventThread_t;

#endif /* NG_PTHREAD */

typedef enum ngiEventNonThreadCondType_e {
    NGI_EVENT_NON_THREAD_COND_NONE,
    NGI_EVENT_NON_THREAD_COND_NO_TIMEOUT,
    NGI_EVENT_NON_THREAD_COND_WITH_TIMEOUT,
    NGI_EVENT_NON_THREAD_COND_NOMORE
} ngiEventNonThreadCondType_t;

struct ngiIOhandle_s;

typedef enum ngiIOhandleState_e {
    NGI_IOHANDLE_STATE_NONE,
    NGI_IOHANDLE_STATE_NORMAL,
    NGI_IOHANDLE_STATE_CLOSED,
    NGI_IOHANDLE_STATE_CANCELED,
    NGI_IOHANDLE_STATE_NOMORE
} ngiIOhandleState_t;

typedef int (*ngiIOhandleCallbackFunction_t)(
    void *arg, struct ngiIOhandle_s *ioHandle,
    ngiIOhandleState_t state, ngLog_t *log, int *error);

typedef struct ngiIOhandleStatus_s {
    int ngihs_callbackRegistering;
    int ngihs_callbackUnregistering;
    int ngihs_callbackUnregisterDone;
    int ngihs_callbackRegistered;
    ngiIOhandleCallbackFunction_t ngihs_callbackFunction;
    void *ngihs_callbackArgument;

    int ngihs_requireProcess;
    int ngihs_requesting;
    int ngihs_started;
    int ngihs_done;
    int ngihs_errorOccurred;
    int ngihs_errorCode;

    char *ngihs_buf;
    size_t ngihs_length;
    size_t ngihs_waitNbytes;
    size_t *ngihs_doneNbytesResult;
    size_t ngihs_currentNbytes;
    size_t ngihs_doneNbytes;
} ngiIOhandleStatus_t;

typedef struct ngiIOhandleChunkStatus_s {
    int ngihcs_callbackRegistering;
    int ngihcs_callbackUnregistering;
    int ngihcs_callbackUnregisterDone;
    int ngihcs_callbackRegistered;
    ngiIOhandleCallbackFunction_t ngihcs_callbackFunction;
    void *ngihcs_callbackArgument;
} ngiIOhandleChunkStatus_t;

typedef enum ngiIOhandleSocketType_e {
    NGI_IOHANDLE_SOCKET_TYPE_NONE,
    NGI_IOHANDLE_SOCKET_TYPE_TCP,
    NGI_IOHANDLE_SOCKET_TYPE_UNIX,
    NGI_IOHANDLE_SOCKET_TYPE_NOMORE
} ngiIOhandleSocketType_t;

typedef struct ngiIOhandleAcceptResult_s {
    ngiIOhandleSocketType_t ngiar_type;
    struct sockaddr ngiar_peerAddress;
} ngiIOhandleAcceptResult_t;

typedef struct ngiIOhandleSocketStatus_s {
    int ngihss_listenCallbackRegistering;
    int ngihss_listenCallbackUnregistering;
    int ngihss_listenCallbackUnregisterDone;
    int ngihss_listenCallbackRegistered;
    ngiIOhandleCallbackFunction_t ngihss_listenCallbackFunction;
    void *ngihss_listenCallbackArgument;

    int ngihss_requireProcess;
    int ngihss_requesting;
    int ngihss_started;
    int ngihss_done;
    int ngihss_errorOccurred;
    int ngihss_errorCode;

    ngiIOhandleSocketType_t ngihss_socketType;

    int ngihss_listenerCreateRequest;
    int ngihss_listenerCreatePort;
    int ngihss_listenerCreatePortAllocated;
    char *ngihss_listenerCreatePath;
    int ngihss_listenerCreateBacklog;

    int ngihss_acceptRequest;
    int ngihss_acceptNewFd;
    ngiIOhandleAcceptResult_t *ngihss_acceptResult;

    int ngihss_connectRequest;
    char *ngihss_connectHostName;
    int ngihss_connectPort;
    char *ngihss_connectPath;

    int ngihss_nodelayRequest;
    int ngihss_nodelayEnable;

} ngiIOhandleSocketStatus_t;

typedef enum ngiIOhandleFileOpenType_e {
    NGI_IOHANDLE_FILE_OPEN_TYPE_NONE,
    NGI_IOHANDLE_FILE_OPEN_TYPE_READ,
    NGI_IOHANDLE_FILE_OPEN_TYPE_WRITE,
    NGI_IOHANDLE_FILE_OPEN_TYPE_NOMORE
} ngiIOhandleFileOpenType_t;

typedef struct ngiIOhandleFileStatus_s {
    int ngihfs_requireProcess;
    int ngihfs_requesting;
    int ngihfs_started;
    int ngihfs_done;
    int ngihfs_errorOccurred;
    int ngihfs_errorCode;

    int ngihfs_fileOpenRequest;
    char *ngihfs_fileOpenPath;
    ngiIOhandleFileOpenType_t ngihfs_fileOpenType;

    int ngihfs_fdOpenRequest;
    int ngihfs_fdOpenFd;

} ngiIOhandleFileStatus_t;

typedef struct ngiIOhandleTimeStatus_s {
    int ngihts_callbackRegistering;
    int ngihts_callbackUnregistering;
    int ngihts_callbackUnregisterDone;
    int ngihts_callbackRegistered;

    ngiIOhandleCallbackFunction_t ngihts_timeCallbackFunction;
    void *ngihts_timeCallbackArgument;
    int ngihts_timeCallbackExecuting;

    ngiIOhandleCallbackFunction_t ngihts_changeCallbackFunction;
    void *ngihts_changeCallbackArgument;
    int ngihts_changeCallbackExecuting;

    time_t ngihts_eventTime;
    int ngihts_changeRequested;

} ngiIOhandleTimeStatus_t;

typedef struct ngiIOhandle_s {
    struct ngiIOhandle_s *ngih_next;
    struct ngiEventDriver_s *ngih_eventDriver;
    ngLog_t *ngih_log;

    int ngih_eventDriverID; /* To identify on debug. */
    int ngih_id;
    int ngih_valid;
    int ngih_userOpened;
    int ngih_fdEffective;
    int ngih_fd;
    int ngih_useSelector;

    ngiIOhandleStatus_t ngih_read;
    ngiIOhandleStatus_t ngih_write;
    ngiIOhandleChunkStatus_t ngih_chunk;
    ngiIOhandleSocketStatus_t ngih_socket;
    ngiIOhandleFileStatus_t ngih_file;
    ngiIOhandleTimeStatus_t ngih_time;

    int ngih_closeRequesting;
    int ngih_closeStarted;
    int ngih_closeDone;
    int ngih_closeErrorOccurred;
    int ngih_closeErrorCode;
} ngiIOhandle_t;


typedef struct ngiEventCallback_s {
    struct ngiEventCallbackManager_s *ngec_manager;
    struct ngiEventCallback_s *ngec_next;

    int ngec_id;

    ngiEventThread_t ngec_thread;
    int ngec_continue;
    int ngec_stopped;

    int ngec_userCallbackWorking;

} ngiEventCallback_t;

typedef enum ngiEventCallbackType_e {
    NGI_EVENT_CALLBACK_TYPE_NONE,
    NGI_EVENT_CALLBACK_TYPE_READ,
    NGI_EVENT_CALLBACK_TYPE_LISTENER,
    NGI_EVENT_CALLBACK_TYPE_TIME_EVENT,
    NGI_EVENT_CALLBACK_TYPE_TIME_CHANGE,
    NGI_EVENT_CALLBACK_TYPE_NOMORE
} ngiEventCallbackType_t;

typedef struct ngiEventCallbackManager_s {
    struct ngiEventDriver_s *ngecm_eventDriver;

    ngLog_t *ngecm_log;

    int ngecm_nCallbacks;
    ngiEventCallback_t *ngecm_callback_head;
    int ngecm_callbackIDmax;

    int ngecm_requesting;
    ngiEventCallbackType_t ngecm_callbackType;
    ngiIOhandleCallbackFunction_t ngecm_callbackFunction;
    void *ngecm_callbackArgument;
    ngiIOhandle_t *ngecm_callbackHandle;
    ngiIOhandleState_t ngecm_callbackState;
    int *ngecm_callbackExecuting;

    int ngecm_nWaiting;
    int ngecm_nWorking;

    ngiEventMutex_t ngecm_mutex;
    ngiEventCond_t ngecm_cond;
} ngiEventCallbackManager_t;


typedef struct ngiEventDriver_s {
    struct ngiEvent_s *nged_event;
    struct ngiEventDriver_s *nged_next;

    ngLog_t *nged_log;
    int nged_error;

    int nged_id;
    int nged_nHandles;
    ngiIOhandle_t *nged_ioHandle_head;
    int nged_handleIDmax;
    int nged_isPthread;
    ngiEventCallbackManager_t *nged_callbackManager;

    int nged_commandPipeValid;
    int nged_commandRead;
    int nged_commandWrite;

    ngiSelectorFunctions_t nged_selectorFunctions;
    void *nged_selector;
    int nged_selectorSize;

    int nged_handleNumRequesting;
    int nged_handleNumStarted;
    int nged_handleNumDone;
    int nged_handleNumErrorOccurred;
    int nged_handleNumErrorCode;
    int nged_handleNumIncrease;
    int nged_handleNumDecrease;
    ngiIOhandle_t *nged_handleNumTarget;

    ngiEventThread_t nged_thread;
    int nged_continue;
    int nged_stopped;

    ngiEventMutex_t nged_mutex;
    ngiEventCond_t nged_cond;

} ngiEventDriver_t;

typedef struct ngiEvent_s {
    ngLog_t *ngev_log;

    int ngev_nEventDrivers;
    ngiEventDriver_t *ngev_eventDriver_head;
    int ngev_eventDriverIDmax;
    ngiSelectorFunctions_t ngev_selectorFunctions;
    int ngev_isPthread;

    ngiEventMutex_t ngev_mutex;
} ngiEvent_t;


/* Callback waiter to ensure the callback stop and not run. */
typedef struct ngiIOhandleCallbackWaiter_s {
    int ngihcw_exist;
    ngiMutex_t ngihcw_mutex;
    ngiCond_t ngihcw_cond;
} ngiIOhandleCallbackWaiter_t;


/**
 * LineList.
 */
#define NGI_LINE_LIST_LINE_TMP_BUFFER_INITIAL_SIZE 1024

typedef struct ngiLineListLine_s {
    struct ngiLineListLine_s *nglll_next;
    char *nglll_line;
} ngiLineListLine_t;

typedef struct ngiLineList_s {
    int ngll_tmpBufferSize;
    char *ngll_tmpBuffer;
    int ngll_nLines;
    ngiLineListLine_t *ngll_lines_head;
} ngiLineList_t;

/**
 * External Module control.
 */

#define NGI_EXTERNAL_MODULE_LOG_FILE_SWITCH "-l"

#define NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_INVOKE_SERVER \
    "ng_invoke_server"
#define NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_CLIENT_COMM_PROXY \
    "ng_client_communication_proxy"
#define NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_REMOTE_COMM_PROXY \
    "ng_remote_communication_proxy"
#define NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_INFORMATION_SERVICE \
    "ng_information_service"
#define NGI_EXTERNAL_MODULE_PROGRAM_PATH_FORMAT "%s/bin/%s.%s"

#define NGI_EXTERNAL_MODULE_REPLY_MULTILINE "SM"
#define NGI_EXTERNAL_MODULE_REPLY_SUCCESS "S"
#define NGI_EXTERNAL_MODULE_REPLY_FAILURE "F"
#define NGI_EXTERNAL_MODULE_REPLY_MULTI_END "REPLY_END"
#define NGI_EXTERNAL_MODULE_NOTIFY_MULTI_END_LAST "_END"
#define NGI_EXTERNAL_MODULE_REQUEST_END_LAST "_END"

#define NGI_EXTERNAL_MODULE_REQUEST_EXIT "EXIT"
#define NGI_EXTERNAL_MODULE_REQUEST_QUERY_FEATURES "QUERY_FEATURES"
#define NGI_EXTERNAL_MODULE_QUERY_REQUESTS_VERSION "protocol_version"
#define NGI_EXTERNAL_MODULE_QUERY_REQUESTS_FEATURE "feature"
#define NGI_EXTERNAL_MODULE_QUERY_REQUESTS_REQUEST "request"

#define NGI_EXTERNAL_MODULE_LINE_TERMINATOR_STR  "\x0d\x0a"
#define NGI_EXTERNAL_MODULE_LINE_TERMINATOR_SIZE 2

#define NGI_EXTERNAL_MODULE_READ_BUFFER_INITIAL_SIZE 256

#define NGI_EXTERNAL_MODULE_PROCESS_WAIT_TIME 60
#define NGI_EXTERNAL_MODULE_PROCESS_EXIT_CHECK_PERIOD 1

#define NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage) \
    (((replyMessage) != NULL) ? (replyMessage) : ("Unknown error"))

typedef enum ngiExternalModuleType_e {
    NGI_EXTERNAL_MODULE_TYPE_NONE,
    NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER,
    NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
    NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE,
    NGI_EXTERNAL_MODULE_TYPE_NOMORE
} ngiExternalModuleType_t;

typedef enum ngiExternalModuleSubType_e {
    NGI_EXTERNAL_MODULE_SUB_TYPE_NONE,
    NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL,
    NGI_EXTERNAL_MODULE_SUB_TYPE_CLIENT_COMMUNICATION_PROXY,
    NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY,
    NGI_EXTERNAL_MODULE_SUB_TYPE_NOMORE
} ngiExternalModuleSubType_t;

typedef struct ngiExternalModuleReadBuffer_s {
    char *ngemrb_buf;
    size_t ngemrb_lineStart;
    size_t ngemrb_current;
    size_t ngemrb_termMatchCount;
    size_t ngemrb_termMatchStart;
    size_t ngemrb_bufSize;
    int  ngemrb_reachEOF;
} ngiExternalModuleReadBuffer_t;

typedef struct ngiExternalModuleRequestReplyStatus_s {
    int ngemrrs_requesting;
    int ngemrrs_replied;
    int ngemrrs_errorOccurred;
    int ngemrrs_errorCode;

    int ngemrrs_success;
    char *ngemrrs_message;
    ngiLineList_t *ngemrrs_lines;
} ngiExternalModuleRequestReplyStatus_t;

typedef enum ngiExternalModuleNotifyState_e {
    NGI_EXTERNAL_MODULE_NOTIFY_NONE,
    NGI_EXTERNAL_MODULE_NOTIFY_NORMAL,
    NGI_EXTERNAL_MODULE_NOTIFY_ERROR,
    NGI_EXTERNAL_MODULE_NOTIFY_CLOSED,
    NGI_EXTERNAL_MODULE_NOTIFY_CANCELED,
    NGI_EXTERNAL_MODULE_NOTIFY_NOMORE
} ngiExternalModuleNotifyState_t;

/* Note : Callback function must destruct the lines argument. */
typedef int (*ngiExternalModuleNotifyCallbackFunction_t)(
    void *arg, ngiExternalModuleNotifyState_t state,
    char *notifyName, char *message, ngiLineList_t *lines,
    ngLog_t *log, int *error);

typedef struct ngiExternalModuleNotifyStatus_s {
    int ngemns_callbackRegistered;
    int ngemns_callbackUnregisterRequested;
    ngiExternalModuleNotifyCallbackFunction_t ngemns_callbackFunction;
    void *ngemns_callbackArgument;

    char *ngemns_notifyName;
    char *ngemns_message;
} ngiExternalModuleNotifyStatus_t;

typedef enum ngiExternalModuleReaderType_e {
    NGI_EXTERNAL_MODULE_READER_TYPE_NONE,
    NGI_EXTERNAL_MODULE_READER_TYPE_REPLY,
    NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY,
    NGI_EXTERNAL_MODULE_READER_TYPE_NOMORE
} ngiExternalModuleReaderType_t;

typedef struct ngiExternalModuleReader_s {
    ngiExternalModuleReaderType_t ngemr_readerType;
    struct ngiExternalModule_s *ngemr_externalModule;
    int ngemr_valid;
    ngiIOhandle_t *ngemr_handle;
    int ngemr_handleOpen;
    int ngemr_callbackRegistered;
    int ngemr_callbackExecuting;
    ngiIOhandleCallbackWaiter_t ngemr_callbackWaiter;
    ngiExternalModuleReadBuffer_t ngemr_readBuffer;

    int ngemr_readingNextLine;
    int ngemr_isMultiLine;
    ngiLineList_t *ngemr_lines;
} ngiExternalModuleReader_t;

typedef struct ngiExternalModule_s {
    struct ngiExternalModuleManager_s *ngem_manager;
    struct ngiExternalModule_s *ngem_next;

    ngLog_t *ngem_log;
    int ngem_ID;               /* External Module ID */
    ngiExternalModuleType_t ngem_moduleType; /* Type of module */
    ngiExternalModuleSubType_t ngem_moduleSubType; /* Sub type of module */
    int ngem_moduleID;         /* ID of module */
    char *ngem_type;           /* Type of each module type */
    int ngem_typeCount;        /* count (ID) for this type */
    void *ngem_owner;          /* Owner */

    int ngem_valid;
    int ngem_working;
    int ngem_errorCode;

    ngiMutex_t ngem_mutex;
    ngiCond_t ngem_cond;

    int ngem_nJobsMax;   /* The maximum number of jobs */
    int ngem_nJobsStart; /* The count of start requested Jobs */
    int ngem_nJobsStop;  /* The count of stop requested Jobs */
    int ngem_nJobsDone;  /* The count of DONE Jobs */
    int ngem_serving;    /* The External Module is not retired. */

    pid_t ngem_processPid;

    ngiIOhandle_t *ngem_requestHandle;
    ngiExternalModuleReader_t ngem_replyReader;
    ngiExternalModuleReader_t ngem_notifyReader;

    ngiExternalModuleRequestReplyStatus_t ngem_requestReply;
    ngiExternalModuleNotifyStatus_t ngem_notifyStatus;

    int ngem_nMultiLineNotify;
    char **ngem_multiLineNotify;

} ngiExternalModule_t;

typedef struct ngiExternalModuleCount_s {
    struct ngiExternalModuleCount_s *ngemc_next;

    ngiExternalModuleType_t ngemc_moduleType; /* Module type */
    char *ngemc_type; /* Type */
    int  ngemc_count; /* Count */
} ngiExternalModuleCount_t;

typedef struct ngiExternalModuleManager_s {
    int ngemm_nExternalModule;
    ngiExternalModule_t *ngemm_externalModule_head;
    int ngemm_lastID;

    int ngemm_nInvokeServer;
    int ngemm_nCommunicationProxy;
    int ngemm_nInformationService;

    int ngemm_invokeServer_lastID;
    int ngemm_communicationProxy_lastID;
    int ngemm_informationService_lastID;

    int ngemm_nModuleCount;
    ngiExternalModuleCount_t *ngemm_moduleCount_head;

    ngiEvent_t *ngemm_event;
    ngiMutex_t ngemm_mutex;
    ngiRWlock_t ngemm_rwlOwn;
 
} ngiExternalModuleManager_t;

typedef struct ngiExternalModuleArgument_s {
    char *ngea_name;
    char *ngea_value;
} ngiExternalModuleArgument_t;

/**
 * Signal Manager
 */
typedef enum ngiSignalHandlerType_s {
    NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG,
    NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_USER
} ngiSignalHandlerType_t;

/**
 * Prototype declaration of APIs and internal functions.
 */

/**
 * ngUtility
 */
void *ngiMalloc(size_t, ngLog_t *, int *);
void *ngiCalloc(size_t, size_t, ngLog_t *, int *);
void *ngiRealloc(void *, size_t, ngLog_t *, int *);
int ngiFree(void *, ngLog_t *, int *);
char *ngiStrdup(const char *, ngLog_t *, int *);
char *ngiStrndup(const char *, size_t, ngLog_t *, int *);

void *ngiAllocate(size_t, size_t, const char *, ngLog_t *, int *);
int ngiDeallocate(void *, const char *, ngLog_t *, int *);

#define NGI_ALLOCATE(type, log, error) \
    ((type *)ngiAllocate(1, sizeof(type), #type, log, error))
#define NGI_ALLOCATE_ARRAY(type, n, log, error) \
    ((type *)ngiAllocate(n, sizeof(type), #type, log, error))
#define NGI_DEALLOCATE(type, ptr, log, error) \
    (((int (*)(type *, const char *, ngLog_t *, int *))\
      (void (*)(void))ngiDeallocate)(ptr, #type, log, error))

int ngiHostnameGet(char *, size_t, ngLog_t *, int *);

int ngiGetpwuid(uid_t, struct passwd **, char **, ngLog_t *, int *);
int ngiReleasePasswd(struct passwd *, char *, ngLog_t *, int *);

int ngiLocalTime(time_t, struct tm *, ngLog_t *, int *);

/**
 * Thread
 */
int ngiThreadCreate(ngiThread_t *,
    void *(threadFunction)(void *),
    void *, ngLog_t *, int *);
int ngiThreadJoin(ngiThread_t *, ngLog_t *, int *);
int ngiThreadYield(ngiEvent_t *, ngLog_t *, int *);
int ngiThreadEqual(ngiThread_t *, ngiThread_t *, int *, ngLog_t *, int *);
int ngiThreadSelf(ngiThread_t *, ngLog_t *, int *);
int ngiThreadSleep(int, int, ngiEvent_t *, ngLog_t *, int *);

/**
 * Synchronized objects
 */
int ngiMutexInitialize(ngiMutex_t *, ngLog_t *, int *);
int ngiMutexDestroy(ngiMutex_t *, ngLog_t *, int *);
int ngiMutexLock(ngiMutex_t *, ngLog_t *, int *);
int ngiMutexTryLock(ngiMutex_t *, ngLog_t *, int *);
int ngiMutexUnlock(ngiMutex_t *, ngLog_t *, int *);

/* Condition Variable */
int ngiCondInitialize(ngiCond_t *, ngiEvent_t *, ngLog_t *, int *);
int ngiCondDestroy(ngiCond_t *, ngLog_t *, int *);
int ngiCondWait(ngiCond_t *, ngiMutex_t *, ngLog_t *, int *);
int ngiCondTimedWait(
    ngiCond_t *, ngiMutex_t *, int, int *, ngLog_t *, int *);
int ngiCondSignal(ngiCond_t *, ngLog_t *, int *);
int ngiCondBroadcast(ngiCond_t *, ngLog_t *, int *);

/* Read/Write lock */
int ngiRWlockInitialize(ngiRWlock_t *, ngLog_t *, int *);
int ngiRWlockFinalize(ngiRWlock_t *, ngLog_t *, int *);
int ngiRWlockReadLock(ngiRWlock_t *, ngLog_t *, int *);
int ngiRWlockReadUnlock(ngiRWlock_t *, ngLog_t *, int *);
int ngiRWlockWriteLock(ngiRWlock_t *, ngLog_t *, int *);
int ngiRWlockWriteUnlock(ngiRWlock_t *, ngLog_t *, int *);

/* Reentrant lock */
int ngiRlockInitialize(ngiRlock_t *, ngiEvent_t *, ngLog_t *, int *);
int ngiRlockFinalize(ngiRlock_t *, ngLog_t *, int *);
int ngiRlockLock(ngiRlock_t *, ngLog_t *, int *);
int ngiRlockUnlock(ngiRlock_t *, ngLog_t *, int *);
int ngiRlockWait(ngiRlock_t *, ngLog_t *, int *);
int ngiRlockTimedWait(ngiRlock_t *, int, int *, ngLog_t *, int *);
int ngiRlockBroadcast(ngiRlock_t *, ngLog_t *, int *);

/**
 * Version
 */
int ngVersionGet(char **, ngLog_t *, int *);
int ngConfigureGet(char **, ngLog_t *, int *);

/**
 * ngLog
 */
int ngLogConstructArgumentInitialize(ngLogConstructArgument_t *, ngLog_t *, int *);
int ngLogConstructArgumentFinalize(ngLogConstructArgument_t *, ngLog_t *, int *);
int ngLogConstructArgumentCopy(ngLogConstructArgument_t *, ngLogConstructArgument_t *, ngLog_t *, int *);

ngLog_t *ngLogConstruct(const char *, ngLogConstructArgument_t *, ngLog_t *, int *);
ngLog_t *ngLogConstructForExecutable(const char *, int, ngLogConstructArgument_t *, ngLog_t *, int *);
int ngLogDestruct(ngLog_t *, ngLog_t *, int *);

int ngLogSetDefaultNull(void);

int ngiLogExecutableIDchanged(ngLog_t *, int, int *);
void ngLogPrintf(ngLog_t *, const char *, ngLogLevel_t, const char *,
    const char *, const char *, ...)NG_ATTRIBUTE_PRINTF(6, 7);
void ngLogVprintf(ngLog_t *, const char  *, ngLogLevel_t, const char *,
    const char *, const char *, va_list ap);

void ngLogDebug(ngLog_t *, const char *, const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngLogInfo(ngLog_t *, const char *,  const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngLogWarn(ngLog_t *, const char *,  const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngLogError(ngLog_t *, const char *, const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
void ngLogFatal(ngLog_t *, const char *, const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);

ngCommLog_t *ngCommLogConstruct(ngCommLogPairInfo_t *, ngLogConstructArgument_t *, ngLog_t *, int *);
ngCommLog_t *ngCommLogConstructForExecutable(ngCommLogPairInfo_t *, int, ngLogConstructArgument_t *, ngLog_t *, int *);
int ngCommLogDestruct(ngCommLog_t *, ngLog_t *, int *);

int ngCommLogSend(ngCommLog_t *, char *, size_t, ngLog_t *, int *);
int ngCommLogReceive(ngCommLog_t *, char *, size_t, ngLog_t *, int *);

ngLogLevel_t ngLogGetLogLevelFromEnv(void);

/**
 * Selector.
 */
void ngiSelectorFunctionsInitializeMember(
    ngiSelectorFunctions_t *);
void *ngiSelectorConstruct(
    ngiSelectorFunctions_t *, ngLog_t *, int *);
int ngiSelectorDestruct(
    ngiSelectorFunctions_t *, void *, ngLog_t *, int *);
int ngiSelectorResize(
    ngiSelectorFunctions_t *, void *, int, ngLog_t *, int *);
int ngiSelectorClear(
    ngiSelectorFunctions_t *, void *, ngLog_t *, int *);
int ngiSelectorSet(
    ngiSelectorFunctions_t *, void *, int, int, int, ngLog_t *, int *);
int ngiSelectorSetLast(
    ngiSelectorFunctions_t *, void *, int, ngLog_t *, int *);
int ngiSelectorWait(
    ngiSelectorFunctions_t *, void *, ngLog_t *, int *);
int ngiSelectorGet(
    ngiSelectorFunctions_t *, void *, int, int *, int *, ngLog_t *, int *);
int ngiSelectorGetLast(
    ngiSelectorFunctions_t *, void *, int *, int *, int *, int *,
    ngLog_t *, int *);

#ifdef NGI_POLL_ENABLED
int ngiSelectorPollFunctionsSet(
    ngiSelectorFunctions_t *, ngLog_t *, int *);
#endif /* NGI_POLL_ENABLED */

#ifdef NGI_SELECT_ENABLED
int ngiSelectorSelectFunctionsSet(
    ngiSelectorFunctions_t *, ngLog_t *, int *);
#endif /* NGI_SELECT_ENABLED */

/**
 * ngiEvent and ngiIOhandle.
 */
ngiEvent_t *ngiEventConstruct(
    ngiSelectorFunctionsSetFunction_t, ngLog_t *, int *);
int ngiEventDestruct(ngiEvent_t *, ngLog_t *, int *);
int ngiEventLogSet(ngiEvent_t *, ngLog_t *, int *);
int ngiEventNonThreadCondTimedWait(ngiEvent_t *,
    int *, ngiEventNonThreadCondType_t, int, int *, ngLog_t *, int *);
int ngiEventNonThreadYield(ngiEvent_t *, ngLog_t *, int *);

ngiIOhandle_t *ngiIOhandleConstruct(ngiEvent_t *, ngLog_t *, int *);
int ngiIOhandleDestruct(ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleClose(ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleRead(ngiIOhandle_t *,
    char *, size_t, size_t, size_t *, ngLog_t *, int *);
int ngiIOhandleWrite(ngiIOhandle_t *,
    char *, size_t, size_t, size_t *, ngLog_t *, int *);
int ngiIOhandleReadCallbackRegister(ngiIOhandle_t *,
    ngiIOhandleCallbackFunction_t, void *, ngLog_t *, int *);
int ngiIOhandleReadCallbackUnregister(ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleReadChunkCallbackRegister(ngiIOhandle_t *,
    ngiIOhandleCallbackFunction_t, void *, ngLog_t *, int *);
int ngiIOhandleReadChunkCallbackUnregister(
    ngiIOhandle_t *, ngLog_t *, int *);

int ngiIOhandleTCPlistenerCreate(ngiIOhandle_t *,
    int, int *, int, ngLog_t *, int *);
int ngiIOhandleTCPlistenerCallbackRegister(ngiIOhandle_t *,
    ngiIOhandleCallbackFunction_t, void *, ngLog_t *, int *);
int ngiIOhandleTCPlistenerCallbackUnregister(
    ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleTCPaccept(
    ngiIOhandle_t *, ngiIOhandle_t *, ngiIOhandleAcceptResult_t **,
    ngLog_t *log, int *error);
int ngiIOhandleTCPconnect(
    ngiIOhandle_t *, char *, int, ngLog_t *, int *);
int ngiIOhandleTCPnodelaySet(
    ngiIOhandle_t *, int, ngLog_t *, int *);
int ngiIOhandleUNIXlistenerCreate(ngiIOhandle_t *,
    char *, int, ngLog_t *, int *);
int ngiIOhandleUNIXlistenerCallbackRegister(ngiIOhandle_t *,
    ngiIOhandleCallbackFunction_t, void *, ngLog_t *, int *);
int ngiIOhandleUNIXlistenerCallbackUnregister(
    ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleUNIXaccept(
    ngiIOhandle_t *, ngiIOhandle_t *, ngiIOhandleAcceptResult_t **,
    ngLog_t *, int *);
int ngiIOhandleUNIXconnect(
    ngiIOhandle_t *, char *, ngLog_t *, int *);

ngiIOhandleAcceptResult_t * ngiIOhandleAcceptResultConstruct(
    ngLog_t *, int *);
int ngiIOhandleAcceptResultDestruct(
    ngiIOhandleAcceptResult_t *, ngLog_t *, int *);
int ngiIOhandleAcceptResultLogOutput(
    ngiIOhandleAcceptResult_t *, ngLog_t *, int *);
int ngiIOhandleAcceptResultIsLocalhost(
    ngiIOhandleAcceptResult_t *, int *, ngLog_t *, int *);

int ngiIOhandleFileOpen(
    ngiIOhandle_t *, char *, ngiIOhandleFileOpenType_t, ngLog_t *, int *);
int ngiIOhandleFdOpen(
    ngiIOhandle_t *, int, ngLog_t *, int *);

int ngiIOhandleTimeEventTimeChangeRequest(
    ngiIOhandle_t *, ngLog_t *, int *);
int ngiIOhandleTimeEventTimeSet(
    ngiIOhandle_t *, time_t, ngLog_t *, int *);
int ngiIOhandleTimeEventCallbackRegister(
    ngiIOhandle_t *,
    ngiIOhandleCallbackFunction_t, void *,
    ngiIOhandleCallbackFunction_t, void *,
    ngLog_t *, int *);
int ngiIOhandleTimeEventCallbackUnregister(
    ngiIOhandle_t *, ngLog_t *, int *);

int ngiIOhandleCallbackWaiterInitialize(
    ngiIOhandleCallbackWaiter_t *, ngiEvent_t *, ngLog_t *, int *);
int ngiIOhandleCallbackWaiterFinalize(
    ngiIOhandleCallbackWaiter_t *, ngLog_t *, int *);
int ngiIOhandleCallbackWaiterStart(
    ngiIOhandleCallbackWaiter_t *, ngLog_t *, int *);
int ngiIOhandleCallbackWaiterEnd(
    ngiIOhandleCallbackWaiter_t *, ngLog_t *, int *);
int ngiIOhandleCallbackWaiterWait(
    ngiIOhandleCallbackWaiter_t *, ngLog_t *, int *);


/**
 * Line List
 */
ngiLineList_t *ngiLineListConstruct(ngLog_t *, int *);
int ngiLineListDestruct(ngiLineList_t *, ngLog_t *, int *);
int ngiLineListAppend(ngiLineList_t *, char *, ngLog_t *, int *);
int ngiLineListPrintf(ngiLineList_t *, ngLog_t *, int *, char *, ...)
    NG_ATTRIBUTE_PRINTF(4, 5);
char * ngiLineListLineGetNext(ngiLineList_t *, char *, ngLog_t *, int *);

/**
 * External Module
 */
ngiExternalModuleManager_t *ngiExternalModuleManagerConstruct(
    ngiEvent_t *, ngLog_t *, int *);
int ngiExternalModuleManagerDestruct(
    ngiExternalModuleManager_t *, ngLog_t *, int *);
int ngiExternalModuleManagerListReadLock(
    ngiExternalModuleManager_t *, ngLog_t *, int *);
int ngiExternalModuleManagerListReadUnlock(
    ngiExternalModuleManager_t *, ngLog_t *, int *);
int ngiExternalModuleManagerListWriteLock(
    ngiExternalModuleManager_t *, ngLog_t *, int *);
int ngiExternalModuleManagerListWriteUnlock(
    ngiExternalModuleManager_t *, ngLog_t *, int *);
ngiExternalModule_t *ngiExternalModuleConstruct(
    ngiExternalModuleManager_t *,
    ngiExternalModuleType_t, ngiExternalModuleSubType_t,
    char *, char *, char *, char *, int, char **, ngLog_t *, int *);
int ngiExternalModuleDestruct(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleProgramCheckAccess(
    ngiExternalModuleType_t, ngiExternalModuleSubType_t,
    char *, char *, ngLog_t *, int *);
int ngiExternalModuleIsValid(
    ngiExternalModule_t *, int *, ngLog_t *, int *);
int ngiExternalModuleOwnerSet(
    ngiExternalModule_t *, void *, ngLog_t *, int *);
void *ngiExternalModuleOwnerGet(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleJobStarted(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleJobStopped(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleJobDone(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleRetire(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleIDget(
    ngiExternalModule_t *, int *, ngLog_t *, int *);
int ngiExternalModuleModuleTypeIDget(
    ngiExternalModule_t *, int *, ngLog_t *, int *);
int ngiExternalModuleModuleTypeCountGet(
    ngiExternalModule_t *, int *, ngLog_t *, int *);
int ngiExternalModuleAvailableGet(
    ngiExternalModuleManager_t *, ngiExternalModuleType_t,
    char *, ngiExternalModule_t **, ngLog_t *, int *);
int ngiExternalModuleRequest(
    ngiExternalModule_t *, char *, char *, ngiLineList_t *,
    int, int *, char **, ngiLineList_t **, ngLog_t *, int *);
int ngiExternalModuleNotifyCallbackRegister(
    ngiExternalModule_t *,
    ngiExternalModuleNotifyCallbackFunction_t, void *, ngLog_t *, int *);
int ngiExternalModuleNotifyCallbackUnregister(
    ngiExternalModule_t *, ngLog_t *, int *);
char *ngiExternalModuleNotifyStateToString(
    ngiExternalModuleNotifyState_t state);
int ngiExternalModuleIsRetired(
    ngiExternalModule_t *, int *, ngLog_t *, int *);
int ngiExternalModuleUnusable(
    ngiExternalModule_t *, ngLog_t *, int *);
int ngiExternalModuleQueryFeatures(
    ngiExternalModule_t *, int *, char **,
    ngiLineList_t **, ngiLineList_t **, char **, ngLog_t *, int *);
int ngiExternalModuleProgramNameGet(ngiExternalModuleType_t,
    ngiExternalModuleSubType_t, char *, char **, ngLog_t *, int *);

/* Argument of multi line reply or notify */
ngiExternalModuleArgument_t *ngiExternalModuleArgumentConstruct(
    char * src, ngLog_t *, int *);
int ngiExternalModuleArgumentDestruct(
    ngiExternalModuleArgument_t *, ngLog_t *, int *);

/* Signal Manager */
int ngiSignalManagerInitialize(int *, ngLog_t *, int *);
int ngiSignalManagerFinalize(int, ngLog_t *, int *);
int ngiSignalManagerStart(int, ngLog_t *, int *);
int ngiSignalManagerStop(int, ngLog_t *, int *);
int ngiSignalManagerLogSet(int, ngLog_t *, ngLog_t *, int *);
int ngiSignalManagerSignalNamesGet(
    char ***, int **, int *, ngLog_t *, int *);
int ngiSignalManagerSignalNamesDestruct(
    char **, int *, int, ngLog_t *, int *);
int ngiSignalManagerSignalHandlerRegister(
    int, ngiSignalHandlerType_t, int *, int, void (*)(int),
    ngLog_t *, int *);
int ngiSignalManagerSignalMaskReset(void);


/**
 * Utility
 */
int ngiSleepSecond(int);
int ngiSleepTimeval(struct timeval *, int, ngLog_t *, int *);
int ngiSetStartTime(ngExecutionTime_t *, ngLog_t *, int *);
int ngiSetEndTime(ngExecutionTime_t *, ngLog_t *, int *);
int ngiStringToTimeval(char *, struct timeval *, ngLog_t *, int *);
int ngiTimevalToString(char *, int, struct timeval *, ngLog_t *, int *);

int ngiDebuggerInformationFinalize(
    ngDebuggerInformation_t *, ngLog_t *, int *);
void ngiDebuggerInformationInitializeMember(ngDebuggerInformation_t *);
void ngiDebuggerInformationInitializePointer(ngDebuggerInformation_t *);

char *ngiDefaultTemporaryDirectoryNameGet(ngLog_t *, int *);
char *ngiTemporaryFileCreate(char *, ngLog_t *, int *);
int ngiTemporaryFileDestroy(char *, ngLog_t *, int *);

int ngiConnectRetryInformationInitialize(
    ngiConnectRetryInformation_t *, ngLog_t *, int *);
int ngiConnectRetryInformationFinalize(
    ngiConnectRetryInformation_t *, ngLog_t *, int *);
int ngiConnectRetryInitialize(
    ngiConnectRetryStatus_t *, ngiConnectRetryInformation_t *,
    ngiRandomNumber_t *, ngLog_t *, int *);
int ngiConnectRetryFinalize(
    ngiConnectRetryStatus_t *, ngLog_t *, int *);
int ngiConnectRetryGetNextRetrySleepTime(
    ngiConnectRetryStatus_t *, int *, struct timeval *, ngLog_t *, int *);

struct timeval ngiTimevalAdd(struct timeval, struct timeval);
struct timeval ngiTimevalSub(struct timeval, struct timeval);
int ngiTimevalCompare(struct timeval, struct timeval);
int ngiGetTimeOfDay(struct timeval *, ngLog_t *, int *);

char *ngiStrdupPrintf(ngLog_t *, int *, const char *, ...)NG_ATTRIBUTE_PRINTF(3, 4);
char *ngiStrdupVprintf(const char *, va_list, ngLog_t *, int *);
int ngiStringToInt(const char *, int *, ngLog_t *, int *);

#define NGI_STRING_IS_VALID(str) (((str) != NULL) && (strlen(str) > 0))

int ngiRandomNumberInitialize(
    ngiRandomNumber_t *, ngLog_t *, int *);
int ngiRandomNumberFinalize(
    ngiRandomNumber_t *, ngLog_t *, int *);
int ngiRandomNumberGetLong(
    ngiRandomNumber_t *, long *, ngLog_t *, int *);
int ngiRandomNumberGetDouble(
    ngiRandomNumber_t *, double *, ngLog_t *, int *);

#endif /* _NGUTILITY_H_ */
