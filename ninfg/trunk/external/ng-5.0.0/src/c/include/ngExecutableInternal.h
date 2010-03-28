/* 
 * $RCSfile: ngExecutableInternal.h,v $ $Revision: 1.24 $ $Date: 2008/02/06 07:16:36 $
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
#ifndef _NGEXECUTABLEINTERNAL_H_
#define _NGEXECUTABLEINTERNAL_H_

#include "ngInternal.h"

/**
 * This file define the Data Structures and Constant Values for Pure Ninf-G
 * Executable.
 */

/**
 * Define the Constant Values.
 */
/* File name of Configuration */
#define NGEXI_CONFIG_FILENAME_USER "/.ngstubrc"

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

/* save_stdout stdout buffering */
#define NGEXI_SAVE_STDOUT_NO_BUFFERING     0

#define NGEXI_LINE_BUFFER_SIZE 1024

#define NGEXI_HEARTBEAT_SEND_PENDING_SLEEP 1
#define NGEXI_HEARTBEAT_WAIT_CLOSE_AFTER_CONNECT 60

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
typedef struct ngexiLocalMachineInformation_s {
    char	*nglmi_path;    /* invoked path (argv[0]) */
    char	*nglmi_tmpDir;	/* Temporary Directory */
    ngLogInformation_t nglmi_logInfo;/* Log Information */
    ngLogLevelInformation_t nglmi_logLevels;/* Log Levels */
    ngLogInformation_t nglmi_commLogInfo; /* Communication Log Information */
    int          nglmi_commLogEnable; /* Communication Log Enable? */
    char	*nglmi_saveStdout;	/* Save stdout to file */
    char	*nglmi_saveStderr;	/* Save stderr to file */
    int         *nglmi_signals;		/* signals */
    int          nglmi_continueOnError;  /* continue on error */
    char        *nglmi_commProxyLogFilePath; /* Comm. Proxy log */
} ngexiLocalMachineInformation_t;

/**
 * Information about Communication.
 */
typedef struct ngexiCommunicationInformation_s {
    char	    *ngci_connectbackAddress; /* address for connect back */
    char            *ngci_hostname;           /* client hostname */
    int              ngci_simpleAuthNumber;   /* Simple Auth Number */
    int              ngci_tcpNodelay;         /* TCP_NODELAY option */
    int	             ngci_contextID;	      /* Context ID */
    int              ngci_jobID;	      /* Job ID */
    int              ngci_executableID;	      /* Executable ID */
} ngexiCommunicationInformation_t;

/**
 * Information about Communication Proxy.
 */
typedef struct ngexiCommunicationProxyInformation_s {
    char          *ngcpi_type;
    char          *ngcpi_path;
    ngiLineList_t *ngcpi_options;
} ngexiCommunicationProxyInformation_t;

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
    ngExecutionTime_t ngsi_callbackTransferArgument;
    ngExecutionTime_t ngsi_callbackCalculation;
    ngExecutionTime_t ngsi_callbackTransferResult;

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

/* HeartBeat */
typedef struct ngexiHeartBeatSend_s {
    int nghbs_isPthread;
    int nghbs_continue;
    int nghbs_handleWorking;
    int nghbs_signalWorking;

    ngiIOhandle_t *nghbs_timeHandle;

    time_t nghbs_eventTime;
    int nghbs_interval;

#ifndef NG_PTHREAD
    int nghbs_actOldSet;
    struct sigaction nghbs_actOld;
    int nghbs_sigBlockCount;
    int nghbs_sigBlockRead;
    int nghbs_sigBlockWrite;
#endif /* NG_PTHREAD */
} ngexiHeartBeatSend_t;

typedef enum ngexiHeartBeatSendBlockType_e {
    NGEXI_HEARTBEAT_SEND_NONE,
    NGEXI_HEARTBEAT_SEND_BLOCK_READ,
    NGEXI_HEARTBEAT_SEND_BLOCK_WRITE,
    NGEXI_HEARTBEAT_SEND_UNBLOCK_READ,
    NGEXI_HEARTBEAT_SEND_UNBLOCK_WRITE,
    NGEXI_HEARTBEAT_SEND_NOMORE
} ngexiHeartBeatSendBlockType_t;

typedef enum ngexiAfterCloseType_e {
    NGEXI_AFTER_CLOSE_TYPE_NONE,
    NGEXI_AFTER_CLOSE_TYPE_CALCULATION_END,
    NGEXI_AFTER_CLOSE_TYPE_INVOKE_CALLBACK,
    NGEXI_AFTER_CLOSE_TYPE_NOMORE
} ngexiAfterCloseType_t;

/**
 * Communication Proxy
 */
typedef struct ngexiCommunicationProxy_s {
    struct ngexiContext_s      *ngcp_context;
    ngiExternalModuleManager_t *ngcp_externalModuleManager;
    ngiExternalModule_t        *ngcp_externalModule; /* External Module */
} ngexiCommunicationProxy_t;

/**
 * Ninf-G Context for Executable.
 */
typedef struct ngexiContext_s {
    /* Ninf-G Event */
    ngiEvent_t	*ngc_event;

    /* Local Machine Information */
    ngexiLocalMachineInformation_t	ngc_lmInfo;

    /* Information about Debugger */
    ngDebuggerInformation_t		ngc_dbgInfo;

    /* Client Machine Information */
    ngexiCommunicationInformation_t	ngc_commInfo;

    /* Communication Proxy Information */
    ngexiCommunicationProxyInformation_t ngc_commProxyInfo;

    /* Remote Class Information */
    ngRemoteClassInformation_t		*ngc_rcInfo;

    /* Connect Retry */
    ngiConnectRetryInformation_t        ngc_retryInfo;

    /* Protocol Manager */
    ngiProtocol_t	*ngc_protocol;

    /* Communication Manager */
    ngiCommunication_t	*ngc_communication;

    /* I/O Callback Waiter */
    ngiIOhandleCallbackWaiter_t ngc_ioCallbackWaiter;

    ngLog_t	*ngc_log;	/* Information about Log */
    ngCommLog_t	*ngc_commLog;	/* Information about Log for Communication */

    ngexiExecutableStatus_t ngc_executableStatus; /* Status of Executable */

    /* Mutex, condition for Status of Executable */
    ngiMutex_t ngc_mutex;
    ngiCond_t  ngc_cond;

    ngexiSessionInformation_t ngc_sessionInfo;

    int ngc_sessionID;	/* Session ID */
    int ngc_methodID;	/* Method ID invoked at last */

    /* Argument Data for Method */
    ngiArgument_t *ngc_methodArgument;

    /* Argument Data for Callback */
    ngiArgument_t *ngc_callbackArgument;

    /* Heart beat */
    int ngc_heartBeatInterval;
    ngexiHeartBeatSend_t ngc_heartBeatSend;

    /* Random Number */
    ngiRandomNumber_t ngc_random;

    /* Connection Close */
    int ngc_connectLocked;
    int ngc_connecting;
    int ngc_connectCloseRequested;

    /* Wait the Transfer Result Data */
    int ngc_afterCloseArrived;
    ngexiAfterCloseType_t ngc_afterCloseType;

    /* Communication Proxy */
    ngexiCommunicationProxy_t *ngc_commProxy;

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

int ngexiContextConnectionClose(ngexiContext_t *, int *);
int ngexiContextConnectionEstablishAndLock(
    ngexiContext_t *, int *, int *);
int ngexiContextConnectionUnlock(ngexiContext_t *, int *);
int ngexiContextConnectionCloseWait(ngexiContext_t *, int *);

int ngexiContextAfterCloseWaitStart(ngexiContext_t *, int *);
int ngexiContextAfterCloseWaitEnd(ngexiContext_t *, int *);
int ngexiContextAfterCloseWait(
    ngexiContext_t *, ngexiAfterCloseType_t, int *, int *);
int ngexiContextAfterCloseArrived(
    ngexiContext_t *, ngexiAfterCloseType_t, int, int *);


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
int ngexiContextSetMethodTransferFileRemoteToClientStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetMethodTransferFileRemoteToClientEndTime(
    ngexiContext_t *, ngLog_t *, int *);

int ngexiContextSetCallbackStartTime(ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackEndTime(ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackTransferArgumentStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackTransferArgumentEndTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackTransferResultStartTime(
    ngexiContext_t *, ngLog_t *, int *);
int ngexiContextSetCallbackTransferResultEndTime(
    ngexiContext_t *, ngLog_t *, int *);

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
void ngexiHeartBeatInitializeMember(ngexiHeartBeatSend_t *);
int ngexiHeartBeatStart(ngexiContext_t *, int *);
int ngexiHeartBeatStop(ngexiContext_t *, int *);
int ngexiHeartBeatSendBlock(ngexiContext_t *,
    ngexiHeartBeatSendBlockType_t, int *error);

/* Configuration file */
int ngexiConfigFileRead(ngexiContext_t *, char *, char *, int *);

/* Protocol */
int ngexiProcessNegotiation(ngexiContext_t *, ngLog_t *, int *);
int ngexiProcessNegotiationSecondConnect(
    ngexiContext_t *, ngLog_t *, int *);
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

/* Communication Proxy */
ngexiCommunicationProxy_t *ngexiCommunicationProxyConstruct(
    ngexiContext_t *, char *, char *, ngiLineList_t *,
    char **, ngLog_t *, int *);
int ngexiCommunicationProxyDestruct(
    ngexiCommunicationProxy_t *, int *);

#endif /* _NGEXECUTABLEINTERNAL_H_ */
