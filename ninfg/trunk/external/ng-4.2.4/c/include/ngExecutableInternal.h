/* 
 * $RCSfile: ngExecutableInternal.h,v $ $Revision: 1.51 $ $Date: 2008/07/02 09:29:39 $
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
#ifndef _NGEXECUTABLEINTERNAL_H_
#define _NGEXECUTABLEINTERNAL_H_

#if defined(sun) || defined(NG_OS_AIX)
#include <stddef.h>
#elif defined(NG_OS_IRIX) || defined(NG_OS_FREEBSD)
#include <sys/endian.h>
#elif defined(NG_OS_OSF5_1) || defined(NG_OS_MACOSX)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#if defined(LITTLE_ENDIAN) || defined(_LITTLE_ENDIAN)
#define NG_LITTLE_ENDIAN
#endif

#include <signal.h>

/**
 * This file define the Data Structures and Constant Values for Pure Ninf-G
 * Executable.
 */

/**
 * Define the Constant Values.
 */
/* File name of Configuration */
#define NGEXI_CONFIG_FILENAME_SYSTEM "/var/gridrpc/ngstub.conf"
#define NGEXI_CONFIG_FILENAME_USER "/.ngstubrc"

/* File prefix of GASS transfer */
#define NGEXI_LOCAL_FILE_PREFIX "file:"

/* Return values of function ngexStubGetRequest() */
#define	NGEXI_INVOKE_METHOD	1	/* Invoke the metod */
#define NGEXI_EXIT	0	/* Receive exit request */
#define NGEXI_FAIL	-1	/* Error occurred, it can't recovered */

/* Debugger information (default) */
#define NGEXI_DEBUG_TERMINAL		"xterm"
#define NGEXI_DEBUG_DEBUGGER		"gdb"
#define NGEXI_DEBUG_TERMINAL_OPTION	"-e"
#define NGEXI_DEBUG_WAIT_FOR_ATTACH	10
#define NGEXI_DEBUG_COMMAND_MAX		1024

/* The setting to avoid GT GASS API retry-Segmentation fault bug */
#define NGEXI_AVOID_GASS_API_RETRY_SEGV_BUG
#define NGEXI_AVOID_GASS_API_AVOID_DEACTIVATE_FREEZE

/* save_stdout stdout buffering */
#define NGEXI_SAVE_STDOUT_NO_BUFFERING     0

/**
 * Argument for ngexStubGetArgument().
 */
typedef union ngexiArgument_u {
    ngiArgumentData_t *nga_scalar;
    ngiArgumentPointer_t *nga_array;
} ngexiArgument_u;

/**
 * Information about Local Machine.
 */
typedef struct ngexLocalMachineInformation_s {
    char	*nglmi_path;    /* invoked path (argv[0]) */
    char	*nglmi_tmpDir;	/* Temporary Directory */
    ngLogInformation_t	nglmi_log;	/* Log Information */
    ngLogInformation_t nglmi_commLog;	/* Communication Log Information */
    char	*nglmi_saveStdout;	/* Save stdout to file */
    char	*nglmi_saveStderr;	/* Save stderr to file */
    int         *nglmi_signals;		/* signals */
    int		nglmi_continueOnError;	/* continue on error */
} ngexiLocalMachineInformation_t;

/**
 * Information about Communication.
 */
typedef struct ngexCommunicationInformation_s {
    char	*ngci_hostName;		/* Host name of client */
    in_port_t	ngci_portNo;		/* Port number of client */
    char	*ngci_gassServer;	/* URL of GASS server */
    ngProtocolCrypt_t	ngci_crypt;	/* Type of Crypt */
    ngProtocolType_t	ngci_protoType;	/* Type of protocol */
    int ngci_tcpNodelay;                /* TCP_NODELAY option */
    int	ngci_contextID;		/* Context ID */
    int ngci_jobID;		/* Job ID */
    int ngci_executableID;	/* Executable ID */
} ngexiCommunicationInformation_t;

/**
 * Status of Executable
 */
typedef enum ngexiExecutableStatus_e {
    NGEXI_EXECUTABLE_ERROR=-1,
    NGEXI_EXECUTABLE_STATUS_MIN=0,
    NGEXI_EXECUTABLE_STATUS_INITIALIZING,
    NGEXI_EXECUTABLE_STATUS_IDLE,
    NGEXI_EXECUTABLE_STATUS_INVOKED,
    NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT,
    NGEXI_EXECUTABLE_STATUS_CALCULATING,
    NGEXI_EXECUTABLE_STATUS_SUSPENDED,
    NGEXI_EXECUTABLE_STATUS_CALCULATION_END,
    NGEXI_EXECUTABLE_STATUS_TRANSFER_RESULT,
    NGEXI_EXECUTABLE_STATUS_PULL_WAIT,
    NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED,
    NGEXI_EXECUTABLE_STATUS_CB_NOTIFY,
    NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_ARGUMENT,
    NGEXI_EXECUTABLE_STATUS_CB_WAIT,
    NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_RESULT,
    NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED,
    NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED,
    NGEXI_EXECUTABLE_STATUS_END,
    NGEXI_EXECUTABLE_STATUS_MAX
} ngexiExecutableStatus_t;

/**
 * Session Information.
 */
typedef struct ngexiSessionInformation_s {
    /* Method */
    ngExecutionTime_t ngsi_transferArgument;
    ngExecutionTime_t ngsi_transferFileClientToRemote;
    ngExecutionTime_t ngsi_calculation;
    ngExecutionTime_t ngsi_transferResult;
    ngExecutionTime_t ngsi_transferFileRemoteToClient;

    /* Callback */
    ngExecutionTime_t ngsi_callbackCalculation;

    /* The sum of execution time of callback functions */
    int ngsi_callbackNtimesCalled;
    struct timeval ngsi_sumCallbackTransferArgumentReal;
    struct timeval ngsi_sumCallbackTransferArgumentCPU;
    struct timeval ngsi_sumCallbackCalculationReal;
    struct timeval ngsi_sumCallbackCalculationCPU;
    struct timeval ngsi_sumCallbackTransferResultReal;
    struct timeval ngsi_sumCallbackTransferResultCPU;

    /* Compression Information */
    int ngsi_nCompressionInformations;
    ngCompressionInformation_t *ngsi_compressionInformation;
} ngexiSessionInformation_t;

/**
 * Information for managing GASS copy.
 */
typedef struct ngexiGASScopyManager_s {
    globus_gass_copy_handleattr_t      nggcm_copy_ha;
    globus_gass_copy_handle_t          nggcm_copy_h;
    globus_gass_copy_attr_t            nggcm_src_copy;
    globus_gass_copy_attr_t            nggcm_dest_copy;
    globus_url_t                       nggcm_src_url;
    globus_url_t                       nggcm_dest_url;
    globus_gass_transfer_requestattr_t nggcm_src_trans;
    globus_gass_transfer_requestattr_t nggcm_dest_trans;

    globus_mutex_t	nggcm_mutex;
    globus_cond_t	nggcm_cond;
    int			nggcm_done;
    int			nggcm_result;
    int			nggcm_error;

    int                 nggcm_gassCopyByProcess;
    int                 nggcm_failedCount;
} ngexiGASScopyManager_t;

/* Direction of transfer */
typedef enum ngexiGASScopyDirection_e {
    NGEXI_GASS_COPY_REMOTE_TO_LOCAL,	/* Copy from remote to local */
    NGEXI_GASS_COPY_LOCAL_TO_REMOTE 	/* Copy from local to remote */
} ngexiGASScopyDirection_t;

/* HeartBeat */
typedef struct ngexiHeartBeatSend_s {
    int nghbs_interval;  /* heartbeat interval */
    int nghbs_continue;  /* flag for heartbeat thread to continue */
    int nghbs_stopped;  /* flag to tell if the heartbeat thread stopped */
    globus_mutex_t nghbs_mutex;
    globus_cond_t nghbs_cond;
    int nghbs_mutexInitialized;
    int nghbs_condInitialized;
    globus_thread_t nghbs_thread;
    int nghbs_actOldSet;
    struct sigaction nghbs_actOld;
    int nghbs_sigBlockCount;
    pid_t nghbs_hbPid;
} ngexiHeartBeatSend_t;

typedef enum ngexiHeartBeatSendBlockType_e {
    NGEXI_HEARTBEAT_SEND_BLOCK,
    NGEXI_HEARTBEAT_SEND_UNBLOCK
} ngexiHeartBeatSendBlockType_t;

/**
 * Ninf-G Context for Executable.
 */
typedef struct ngexiContext_s {
    /* Local Machine Information */
    ngexiLocalMachineInformation_t	ngc_lmInfo;

    /* Information about Debugger */
    ngDebuggerInformation_t		ngc_dbgInfo;

    /* Client Machine Information */
    ngexiCommunicationInformation_t	ngc_commInfo;

    /* Remote Class Information */
    ngRemoteClassInformation_t		*ngc_rcInfo;

    /* Connect Retry */
    ngiConnectRetryInformation_t        ngc_retryInfo;

    /* GASS Copy Manager */
    ngexiGASScopyManager_t		*ngc_gassCopy;

    /* Protocol Manager */
    ngiProtocol_t	*ngc_protocol;

    /* Communication Manager */
    ngiCommunication_t	*ngc_communication;

    ngLog_t	*ngc_log;	/* Information about Log */
    ngLog_t	*ngc_commLog;	/* Information about Log for Communication */

    ngexiExecutableStatus_t ngc_executableStatus; /* Status of Executable */

    /* Mutex, condition for Status of Executable */
    int ngc_mutexInitialized;
    int ngc_condInitialized;
    globus_mutex_t ngc_mutex;
    globus_cond_t  ngc_cond;

    ngexiSessionInformation_t ngc_sessionInfo;

    int ngc_sessionID;	/* Session ID */
    int ngc_methodID;	/* Method ID invoked at last */

    /* Argument Data for Method */
    ngiArgument_t *ngc_methodArgument;

    /* Argument Data for Callback */
    ngiArgument_t *ngc_callbackArgument;

    /* Heart beat */
    ngexiHeartBeatSend_t ngc_heartBeatSend;

    /* Random Number */
    ngiRandomNumber_t ngc_random;

    ngExecutionTime_t ngc_communicationTime;

    int ngc_rank;

    /* Error code */
    int ngc_error;
    int ngc_cbError; /* it occurred in I/O callback function */
} ngexiContext_t;

/**
 * Prototype declaration of internal APIs.
 */
/* Ninf-G Executable Context */
int ngexiContextInitialize(
    ngexiContext_t *, int, char *[], ngRemoteClassInformation_t *, int, int *);
int ngexiContextFinalize(ngexiContext_t *, int *);
ngRemoteMethodInformation_t *ngexiRemoteMethodInformationGet(
    ngexiContext_t *, int, ngLog_t *, int *);
void ngexiContextSetSessionID(ngexiContext_t *, int);
int ngexiContextGetSessionID(ngexiContext_t *);
void ngexiContextSetRemoteMethodID(ngexiContext_t *, int);
int ngexiContextGetMethodID(ngexiContext_t *);
void ngexiSessionInformationGet(ngexiContext_t *, ngexiSessionInformation_t *);
int ngexiContextSwitchStdoutStderr(ngexiContext_t *, int, int *);

int ngexiContextExecutableStatusCheck(
    ngexiContext_t *, const ngexiExecutableStatus_t *, int, ngLog_t *, int *);
int ngexiContextExecutableStatusSet(
    ngexiContext_t *, ngexiExecutableStatus_t, int *);
ngexiExecutableStatus_t ngexiContextExecutableStatusGet(
    ngexiContext_t *, int *);
int ngexiContextExecutableStatusWait(ngexiContext_t *,
    ngexiExecutableStatus_t *, int, ngexiExecutableStatus_t *, int *);
char * ngexiContextExecutableStatusStringGet(
    ngexiExecutableStatus_t, ngLog_t *, int *);

int ngexiContextSetError(ngexiContext_t *, int, int *);
int ngexiContextGetError(ngexiContext_t *, int *);
int ngexiContextSetCbError(ngexiContext_t *, int, int *);
int ngexiContextGetCbError(ngexiContext_t *, int *);
int ngexiContextUnusable(ngexiContext_t *, int, int *);

void ngexiContextSetMethodArgument(ngexiContext_t *, ngiArgument_t *);
void ngexiContextGetMethodArgument(ngexiContext_t *, ngiArgument_t **);
int ngexiContextReleaseMethodArgument(ngexiContext_t *, ngLog_t *, int *);
void ngexiContextSetCallbackArgument(ngexiContext_t *, ngiArgument_t *);
void ngexiContextGetCallbackArgument(ngexiContext_t *, ngiArgument_t **);
int ngexiContextReleaseCallbackArgument(ngexiContext_t *, ngLog_t *, int *);

int ngexiContextInitializeSessionInformation(
    ngexiContext_t *, ngLog_t *, int *);

int ngexiContextSetMethodStartTime(ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodEndTime(ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferArgumentStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferArgumentEndTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferFileClientToRemoteStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferFileClientToRemoteEndTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferResultStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferResultEndTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferResultEndTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferFileRemoteToClientStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferFileRemoteToClientEndTime(
    ngexiContext_t *, ngLog_t *, int *);

int ngexiContextSetCallbackStartTime(ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackEndTime(ngexiContext_t *, ngLog_t *, int *);

int ngexiContextMakeSessionInformation(
    ngexiContext_t *, ngiArgument_t *, ngLog_t *, int *);
int ngexiContextReleaseSessionInformation(ngexiContext_t *, ngLog_t *, int *);

ngexiContext_t *ngexiContextGet(ngLog_t *, int *);

/* Local Machine Information */
int ngexiLocalMachineInformationInitialize(
    ngexiLocalMachineInformation_t *, ngLog_t *, int *);
int ngexiLocalMachineInformationFinalize(
    ngexiLocalMachineInformation_t *, ngLog_t *, int *);

/* heartbeat */
int ngexiHeartBeatInitialize(ngexiContext_t *, int *);
int ngexiHeartBeatFinalize(ngexiContext_t *, int *);
int ngexiHeartBeatStart(ngexiContext_t *, int *);
int ngexiHeartBeatStop(ngexiContext_t *, int *);
int ngexiHeartBeatSendBlock(ngexiContext_t *,
    ngexiHeartBeatSendBlockType_t, int *error);

/* Configuration file */
int ngexiConfigFileRead(ngexiContext_t *, char *, char *, int *);

/* Protocol */
int ngexiProcessNegotiation(ngexiContext_t *, ngLog_t *, int *);
int ngexiProtocolRegisterCallback(ngexiContext_t *, int *);
int ngexiProtocolReplyByStatus(ngexiContext_t *, ngiProtocol_t *,
    ngexiExecutableStatus_t, ngLog_t *, int *);
int ngexiProtocolNotifyCalculationEnd(
    ngexiContext_t *, ngiProtocol_t *, ngLog_t *, int *);
int ngexiProtocolNotifyInvokeCallback(
    ngexiContext_t *, ngiProtocol_t *, long, ngLog_t *, int *);
int ngexiProtocolNotifyIamAlive(
    ngexiContext_t *, ngiProtocol_t *, ngLog_t *, int *);

ngRemoteMethodInformation_t * ngexiProtocolGetRemoteMethodInformation(
    ngiProtocol_t *, ngLog_t *, int *);

/* GASS Copy Manager */
ngexiGASScopyManager_t *ngexiGASScopyConstruct(ngLog_t *, int *);
int ngexiGASScopyDestruct(ngexiGASScopyManager_t *, ngLog_t *, int *);
ngexiGASScopyManager_t *ngexiGASScopyAllocate(ngLog_t *, int *);
int ngexiGASScopyFree(ngexiGASScopyManager_t *, ngLog_t *, int *);
int ngexiGASScopyInitialize(ngexiGASScopyManager_t *, ngLog_t *, int *);
int ngexiGASScopyFinalize(ngexiGASScopyManager_t *, ngLog_t *, int *);
int ngexiGASScopyFile(ngexiGASScopyManager_t *, char *, char *, char *,
    ngexiGASScopyDirection_t, ngiConnectRetryInformation_t,
    ngiRandomNumber_t *, ngLog_t *, int *);
int ngexiGASScopyProcess(int, char **, int *);

/* Globus */
int ngexiGlobusInitialize(ngLog_t *log, int *error);
int ngexiGlobusFinalize(ngLog_t *log, int *error);
int ngexiGlobusIoRegisterSelect(globus_io_handle_t *,
    globus_io_callback_t, void *, ngLog_t *, int *);
int ngexiGlobusIoUnregister(globus_io_handle_t *, ngLog_t *, int *);

/* Signal Manager */
int ngexiSignalManagerInitialize(ngLog_t *, int *);
int ngexiSignalManagerFinalize(ngLog_t *, int *);
int ngexiSignalManagerStart(ngLog_t *, int *);
int ngexiSignalManagerStop(ngLog_t *, int *);
int ngexiSignalManagerLogSet(ngLog_t *, ngLog_t *, int *);
int ngexiSignalManagerRegister(int *, int, ngLog_t *, int *);
int ngexiTemporaryFileRegister(char *, ngLog_t *, int *);
int ngexiTemporaryFileUnregister(char *, ngLog_t *, int *);

/* Remote Class Information */
int ngexiPrintRemoteClassInformation(
    ngRemoteClassInformation_t *, FILE *, ngLog_t *, int *);
int ngexiPrintRemoteClassInformationToBuffer(ngexiContext_t *,
    ngRemoteClassInformation_t *, char **, ngLog_t *, int *);

#endif /* _NGEXECUTABLEINTERNAL_H_ */
