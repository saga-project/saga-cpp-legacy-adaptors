/* 
 * $RCSfile: ngCommon.h,v $ $Revision: 1.34 $ $Date: 2006/08/03 07:33:50 $
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
#ifndef _NGCOMMON_H_
#define _NGCOMMON_H_

/**
 * This file define the Data Structures and Constant Values for stub
 */

/**
 * Define the error code.
 */
#define NG_ERROR_NO_ERROR		0	/* No error */
#define NG_ERROR_GLOBUS			1	/* Error occurred in API of Globus Toolkit */
#define NG_ERROR_MEMORY			2	/* Memory error */
#define NG_ERROR_FILE			3	/* File access error */
#define NG_ERROR_PROTOCOL		4	/* Protocol error */
#define NG_ERROR_COMMUNICATION		5	/* Communication error */
#define NG_ERROR_INVALID_ARGUMENT	6	/* Invalid argument */
#define NG_ERROR_INVALID_STATE		7	/* Invalid state */
#define NG_ERROR_SOCKET			8	/* Socket error */
#define NG_ERROR_ALREADY		9	/* Data is already registered */
#define NG_ERROR_NOT_EXIST		10	/* Data is not exist */
#define NG_ERROR_EXIST			11	/* Data is exist */
#define NG_ERROR_JOB_DEAD		12	/* Job is dead */
#define NG_ERROR_EXCEED_LIMIT		13	/* Exceed limit */
#define NG_ERROR_JOB_INVOKE		14	/* Invoke job */
#define NG_ERROR_JOB_CANCEL		15	/* Cancel job */
#define NG_ERROR_THREAD			16	/* Thread error */
#define NG_ERROR_SYSCALL		17	/* System call and standard library error */
#define NG_ERROR_TIMEOUT		18	/* Timeout */
#define NG_ERROR_NOT_LOCKED		19	/* Not locked */
#define NG_ERROR_UNLOCK			20	/* Unlock */
#define	NG_ERROR_INITIALIZE		21	/* Initialize */
#define NG_ERROR_FINALIZE		22	/* Finalize */
#define NG_ERROR_OVERFLOW		23	/* Overflow */
#define NG_ERROR_UNDERFLOW		24	/* Underflow */
#define NG_ERROR_SYNTAX			25	/* Syntax error */
#define NG_ERROR_DISCONNECT		26	/* Connection was closed */
#define NG_ERROR_CANCELED		27	/* Session was canceled */
#define NG_ERROR_CONFIGFILE_NOT_FOUND   28      /* Configuration file not found */
#define NG_ERROR_CONFIGFILE_SYNTAX      29      /* Syntax error of Configuration file */
#define NG_ERROR_COMPRESSION            30      /* Compression error */

/**
 * Define the type of COMPLEX.
 */
typedef struct scomplex_s {
    float r;
    float i;
} scomplex;

typedef struct dcomplex_s {
    double r;
    double i;
} dcomplex;

/**
 * Read/Write Lock
 */
#define NG_RWLOCK_MAX_LOCKS INT_MAX
typedef struct ngRWlock_s {
    globus_mutex_t      ngrwl_mutex;
    globus_cond_t       ngrwl_condWrite;
    globus_cond_t       ngrwl_condRead;

    /* Number of threads, it waiting for write lock */
    int         ngrwl_waitingWriteLock;

    /**
     * Counter for Read/Write lock
     * equal 0 (= 0): Nobody is locking.
     * greater than 0 ( > 0): Anybody is locking for read.
     * less than 0 (< 0): A thread is locking for write.
     */
    int         ngrwl_lockCounter;
} ngRWlock_t;

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
 * Log manager.
 */
/* Type of log */
typedef enum ngLogType_e {
    NG_LOG_TYPE_GENERIC,	/* Generic log */
    NG_LOG_TYPE_COMMUNICATION	/* Communication log */
} ngLogType_t;

/* Category of log */
typedef enum ngLogCategory_e {
    NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
    NG_LOG_CATEGORY_NINFG_PROTOCOL,
    NG_LOG_CATEGORY_NINFG_PURE,
    NG_LOG_CATEGORY_NINFG_GRPC
} ngLogCategory_t;

/* Level of log */
typedef enum ngLogLevel_e {
    NG_LOG_LEVEL_OFF,
    NG_LOG_LEVEL_FATAL,
    NG_LOG_LEVEL_ERROR,
    NG_LOG_LEVEL_WARNING,
    NG_LOG_LEVEL_INFORMATION,
    NG_LOG_LEVEL_DEBUG
} ngLogLevel_t;

typedef struct ngLogInformation_s {
    int		ngli_enable;		/* enable log? */
    /* Log level */
    ngLogLevel_t	ngli_level;			/* For all */
    ngLogLevel_t	ngli_levelGlobusToolkit;	/* For Globus Toolkit */
    ngLogLevel_t	ngli_levelNinfgProtocol;	/* For Ninf-G Protocol */
    ngLogLevel_t	ngli_levelNinfgInternal;	/* For Ninf-G Internal */
    ngLogLevel_t	ngli_levelGrpc;	/* For Grid RPC */

    char	*ngli_filePath;		/* File name of log file */
    char	*ngli_suffix;		/* Suffix of log file */
    int		ngli_nFiles;		/* Maximum number of log file */
    int		ngli_maxFileSize;	/* Maximum byte size of log file */
    int		ngli_overWriteDir;	/* Over write to the directory */
} ngLogInformation_t;

typedef struct ngLog_s {
    ngLogType_t	ngl_logType;		/* Type of log */
    int		ngl_mutexInitialized;
    globus_mutex_t ngl_mutex;		/* Mutex for this instance */
    ngLogInformation_t ngl_info;	/* Log Information */
    FILE	*ngl_stream;		/* File stream of output */
    char	*ngl_fileFormat;	/* File format of Log File */
    int		ngl_requireExecutableID;/* Require executable ID? */
    int		ngl_requireFileNumber;  /* Require file number? */
    int		ngl_currentNo;		/* Number of file of current */
    char	*ngl_fileName;		/* File name of current file */
    int		ngl_fileNameBufferSize;	/* File name buffer size */
    int		ngl_executableID;       /* Executable ID for filename */
    int		ngl_nDigits;		/* Digits of file number */
    char	*ngl_hostName;		/* Host name */
    pid_t	ngl_pid;		/* Process ID */
    char	*ngl_message;		/* Any message */
    size_t	ngl_outputNbytes;	/* The output number of bytes */

    int		ngl_startTimeInitialized; /* Initialized the start time */
    ngExecutionTime_t ngl_execTime;	/* Execution time */
} ngLog_t;

/* default log rotation size */
#define NG_LOG_ROTATE_MAX_FILE_SIZE  (1 * 1024 * 1024) /* 1MB each */

/**
 * Information about Debugger.
 */
typedef struct ngDebuggerInformation_s {
    int   ngdi_enable;		/* Debugging enable/disable */
    char *ngdi_terminalPath;	/* Path of Terminal emulator */
    char *ngdi_display;		/* DISPLAY environment variable */
    char *ngdi_debuggerPath;	/* Path of Debugger */
} ngDebuggerInformation_t;

/**
 * Information about Remote Machine.
 */

/* Crypt type */
typedef enum ngProtocolCrypt_e {
    NG_PROTOCOL_CRYPT_NONE,	/* No crypt */
    NG_PROTOCOL_CRYPT_AUTHONLY,	/* Authentication only */
    NG_PROTOCOL_CRYPT_GSI,	/* GSI */
    NG_PROTOCOL_CRYPT_SSL	/* SSL */
} ngProtocolCrypt_t;

/* Type of protocol */
typedef enum ngProtocolType_e {
    NG_PROTOCOL_TYPE_XML,	/* XML (as default) */
    NG_PROTOCOL_TYPE_BINARY	/* Binary */
} ngProtocolType_t;

/**
 * XDR
 */
typedef enum ngXDR_e {
    NG_XDR_NOT_USE,	/* Not use XDR */
    NG_XDR_USE		/* Use XDR */
} ngXDR_t;

/* Wait for completion of transfer argument */
typedef enum ngArgumentTransfer_e {
    NG_ARGUMENT_TRANSFER_UNDEFINED,	/* undefined */
    NG_ARGUMENT_TRANSFER_WAIT,		/* Wait for completion */
    NG_ARGUMENT_TRANSFER_NOWAIT,	/* Nowait for completion */
    NG_ARGUMENT_TRANSFER_COPY		/* Copy the argument */
} ngArgumentTransfer_t;

/* Type of Compression */
typedef enum ngclCompressionType_e {
    NG_COMPRESSION_TYPE_RAW,	/* No compression */
    NG_COMPRESSION_TYPE_ZLIB	/* Compress by zlib */
} ngCompressionType_t;

/**
 * Information of Session.
 */
/* Measured by Executable */
typedef struct ngSessionInformationExecutable_s {
    /* Remote method */
    struct timeval ngsie_transferArgument;
    struct timeval ngsie_transferFileClientToRemote;
    struct timeval ngsie_calculation;
    struct timeval ngsie_transferResult;
    struct timeval ngsie_transferFileRemoteToClient;

    /* Callback */
    struct timeval ngsie_callbackTransferArgument;
    struct timeval ngsie_callbackCalculation;
    struct timeval ngsie_callbackTransferResult;
} ngSessionInformationExecutable_t;

/* Measured by Client */
typedef struct ngSessionInformationClient_s {
    struct timeval ngsic_queryRemoteMachineInformation;
    struct timeval ngsic_queryRemoteClassInformation;
    struct timeval ngsic_invokeExecutable;

    struct timeval ngsic_transferArgument;
    struct timeval ngsic_calculation;
    struct timeval ngsic_transferResult;

    struct timeval ngsic_callbackTransferArgument;
    struct timeval ngsic_callbackCalculation;
    struct timeval ngsic_callbackTransferResult;
} ngSessionInformationClient_t;

typedef struct ngCompressionInformationElement_s {
    int ngcie_measured;
    size_t ngcie_lengthRaw;
    size_t ngcie_lengthCompressed;
    ngExecutionTime_t ngcie_timeMeasurement;
    struct timeval ngcie_executionRealTime;
    struct timeval ngcie_executionCPUtime;
} ngCompressionInformationElement_t;

typedef struct ngCompressionInformationComplex_s {
    ngCompressionInformationElement_t ngcic_compression;
    ngCompressionInformationElement_t ngcic_decompression;
} ngCompressionInformationComplex_t;

typedef struct ngCompressionInformation_s {
    ngCompressionInformationComplex_t ngci_in;
    ngCompressionInformationComplex_t ngci_out;
} ngCompressionInformation_t;

typedef struct ngSessionInformation_s {
    int ngsi_executableCallbackNtimesCalled;
    ngSessionInformationExecutable_t ngsi_executableRealTime;
    ngSessionInformationExecutable_t ngsi_executableCPUtime;

    int ngsi_clientCallbackNtimesCalled;
    ngSessionInformationClient_t ngsi_clientRealTime;
    ngSessionInformationClient_t ngsi_clientCPUtime;

    int ngsi_nCompressionInformations;
    ngCompressionInformation_t *ngsi_compressionInformation;
} ngSessionInformation_t;

/**
 * Prototype declaration of APIs and internal functions.
 */
int ngLogInformationInitialize(ngLogInformation_t *logInfo);
int ngLogInformationFinalize(ngLogInformation_t *logInfo);
int ngLogInformationCopy(ngLogInformation_t *, ngLogInformation_t *, int *);
int ngLogInformationRelease(ngLogInformation_t *, int *);
int ngLogPrintf(ngLog_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, ...);
int ngLogVprintf(ngLog_t *, ngLogCategory_t, ngLogLevel_t, int *, char *, va_list);

#endif /* _NGCOMMON_H_ */
