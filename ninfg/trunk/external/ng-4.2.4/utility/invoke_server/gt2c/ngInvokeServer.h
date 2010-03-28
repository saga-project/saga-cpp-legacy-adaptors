/* 
 * $RCSfile: ngInvokeServer.h,v $ $Revision: 1.9 $ $Date: 2006/10/11 08:13:49 $
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
#ifndef _NG_INVOKE_SERVER_H_
#define _NG_INVOKE_SERVER_H_

/**
 * Invoke Server for GT2 GRAM. C version.
 * The type name is "GT2c".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <globus_common.h>
#include <globus_module.h>
#include <globus_gram_client.h>
#include <globus_gram_protocol.h>
#include <globus_libc.h>


/**
 * Define
 */
#define NGISI_SERVER_TYPE "GT2c"

#define NGISI_FP_REQUEST  stdin
#define NGISI_FP_REPLY    stdout
#define NGISI_FP_NOTIFY   stderr

#define NGISI_JOB_NUM_MAX 256
#define NGISI_USE_REDIRECT_FILE

#define NGISI_LINE_TERMINATOR_STR   "\x0d\x0a"
#define NGISI_LINE_TERMINATOR_SIZE  2
#define NGISI_READ_BUFFER_INITIAL_SIZE 1024
#define NGISI_CREATE_ATTR_INITIAL_SIZE 1
#define NGISI_CREATE_ATTR_STR_MAX 1024
#define NGISI_CREATE_ATTR_END     "JOB_CREATE_END"

#define NGISI_PATH_MAX      1024
#define NGISI_HOSTNAME_MAX  1024
#define NGISI_PROTOCOL_MAX  1024
#define NGISI_LINE_MAX      1024
#define NGISI_ID_STR_MAX    256
#define NGISI_RSL_STR_MAX   (16 * 1024)
#if 1
#define NGISI_LOG_OPEN_MODE "a"
#else
#define NGISI_LOG_OPEN_MODE "w"
#endif

/**
 * Data type
 */
typedef enum ngisiLogLevel_e {
    NGISI_LOG_LEVEL_UNDEFINED, /* for dummy */
    NGISI_LOG_LEVEL_OFF,
    NGISI_LOG_LEVEL_ERROR,
    NGISI_LOG_LEVEL_WARNING,
    NGISI_LOG_LEVEL_DEBUG,
    NGISI_LOG_LEVEL_NOMORE     /* for dummy */
} ngisiLogLevel_t;

typedef struct ngisiLog_s {
    int             ngisl_enabled;
    ngisiLogLevel_t ngisl_level; 
    char           *ngisl_fileName;
    FILE           *ngisl_fp;
} ngisiLog_t;

typedef struct ngisiReadBuffer_s {
    char   *ngisrb_buf;
    int     ngisrb_bufSize;
    int     ngisrb_reachEOF;
} ngisiReadBuffer_t;

typedef struct ngisiCreateAttrElement_s {
    char *ngisce_name;
    char *ngisce_value;
    int   ngisce_treated;
} ngisiCreateAttrElement_t;

typedef struct ngisiCreateAttr_s {
    int ngisca_nAttrs;
    int ngisca_arraySize;
    ngisiCreateAttrElement_t *ngisca_attrs;
} ngisiCreateAttr_t;

typedef enum ngisiRequestType_e {
    NGISI_REQUEST_UNDEFINED,    /* for dummy */
    NGISI_REQUEST_UNKNOWN,      /* Invalid request */
    NGISI_REQUEST_CLOSED,       /* Connection closed */
    NGISI_REQUEST_JOB_CREATE,
    NGISI_REQUEST_JOB_STATUS,
    NGISI_REQUEST_JOB_DESTROY,
    NGISI_REQUEST_EXIT,
    NGISI_REQUEST_NOMORE        /* for dummy */
} ngisiRequestType_t;

typedef struct ngisiRequestTypeTable_s {
    int                ngisrt_valid;   /* finish by valid == 0 */
    ngisiRequestType_t ngisrt_type;
    char              *ngisrt_name;
} ngisiRequestTypeTable_t;

typedef enum ngisiRequestReaderStatus_e {
    NGISI_REQUEST_READER_STATUS_UNDEFINED,   /* for dummy */
    NGISI_REQUEST_READER_STATUS_INITIALIZED,
    NGISI_REQUEST_READER_STATUS_PROCESSING,
    NGISI_REQUEST_READER_STATUS_EXITING,
    NGISI_REQUEST_READER_STATUS_DONE,
    NGISI_REQUEST_READER_STATUS_NOMORE       /* for dummy */
} ngisiRequestReaderStatus_t;

typedef struct ngisiRequestReader_s {
    int             ngisr_continue;
    int             ngisr_stopped;
    globus_mutex_t  ngisr_mutex;
    globus_cond_t   ngisr_cond;
    int             ngisr_mutexInitialized;
    int             ngisr_condInitialized;
    globus_thread_t ngisr_thread;

    ngisiRequestReaderStatus_t ngisr_status;
    ngisiReadBuffer_t          ngisr_readBuffer;
} ngisiRequestReader_t;

typedef enum ngisiReplyResult_e {
    NGISI_REPLY_UNDEFINED,   /* for dummy */
    NGISI_REPLY_SUCCESS,
    NGISI_REPLY_FAIL,
    NGISI_REPLY_NOMORE       /* for dummy */
} ngisiReplyResult_t;

typedef enum ngisiJobBackend_e {
    NGISI_BACKEND_UNDEFINED,  /* for dummy */
    NGISI_BACKEND_NORMAL,
    NGISI_BACKEND_MPI,
    NGISI_BACKEND_BLACS,
    NGISI_BACKEND_NOMORE      /* for dummy */
} ngisiJobBackend_t;

typedef struct ngisiJobAttribute_s {
    char  *ngisja_hostName;
    int    ngisja_portNo;
    char  *ngisja_jobManager;
    char  *ngisja_subject;
    char  *ngisja_clientName;
    char  *ngisja_exePath;
    char  *ngisja_backend;
    int    ngisja_count;
    int    ngisja_staging;
    int    ngisja_nArgs;
    char **ngisja_args;
    char  *ngisja_workDirectory;
    char  *ngisja_gassURL;
    int    ngisja_redirect;
    char  *ngisja_stdoutFile;
    char  *ngisja_stderrFile;
    int    ngisja_nEnv;
    char **ngisja_env;
    int    ngisja_pollingInterval;
    int    ngisja_refreshCred;
    int    ngisja_maxTime;
    int    ngisja_maxWallTime;
    int    ngisja_maxCpuTime;
    char  *ngisja_queue;
    char  *ngisja_project;
    int    ngisja_hostCount;
    int    ngisja_minMemory;
    int    ngisja_maxMemory;
    int    ngisja_rslExtensionsSize;
    char **ngisja_rslExtensions;
} ngisiJobAttribute_t;

typedef enum ngisiJobStatus_e {
    NGISI_JOB_STATUS_UNDEFINED,   /* for dummy */
    NGISI_JOB_STATUS_INITIALIZED,
    NGISI_JOB_STATUS_PENDING,
    NGISI_JOB_STATUS_ACTIVE,
    NGISI_JOB_STATUS_FAILED,
    NGISI_JOB_STATUS_DONE,
    NGISI_JOB_STATUS_NOMORE       /* for dummy */
} ngisiJobStatus_t;

typedef struct ngisiJob_s {
    struct ngisiContext_s *ngisj_context;
    struct ngisiJob_s  *ngisj_next;

    globus_mutex_t      ngisj_mutex;
    int                 ngisj_mutexInitialized;

    int                 ngisj_internalJobID;
    char               *ngisj_requestID;   
    char               *ngisj_jobID;
    ngisiJobAttribute_t ngisj_attr;
    ngisiJobStatus_t    ngisj_status;
    int                 ngisj_destructableStatus;
    int                 ngisj_destructableProtocol;

    char               *ngisj_rmContact;
    char               *ngisj_rsl;
    char               *ngisj_callbackContact;
    char               *ngisj_jobContact;
} ngisiJob_t;

typedef struct ngisiRefreshCredential_s {
    int             ngisrc_updateInterval;
    time_t          ngisrc_nextEventTime;

    int             ngisrc_continue;
    int             ngisrc_working;
    globus_thread_t ngisrc_thread;

    globus_mutex_t  ngisrc_mutex;
    globus_cond_t   ngisrc_cond;
    int             ngisrc_mutexInitialized;
    int             ngisrc_condInitialized;
} ngisiRefreshCredential_t;

typedef struct ngisiContext_s {
    char                *ngisc_serverType;
    ngisiLog_t           ngisc_log;
    ngisiRequestReader_t ngisc_requestReader;

    globus_mutex_t       ngisc_jobMutex;
    int                  ngisc_jobMutexInitialized;

    ngisiRefreshCredential_t ngisc_refreshCredential;

    int                  ngisc_maxJobID;
    int                  ngisc_nJobs;
    struct ngisiJob_s   *ngisc_job_head;
} ngisiContext_t;

/**
 * Prototype
 */
int ngisiContextInitialize(
    ngisiContext_t *, char *, char *, int, char **, int *);
int ngisiContextFinalize(ngisiContext_t *, int *);
int ngisiGlobusInitialize(ngisiContext_t *, int *);
int ngisiGlobusFinalize(ngisiContext_t *, int *);
int ngisiContextJobListLock(ngisiContext_t *, int *);
int ngisiContextJobListUnlock(ngisiContext_t *, int *);

int ngisiLogInitialize(
    ngisiContext_t *, ngisiLog_t *, char *, int *);
int ngisiLogFinalize(ngisiContext_t *, ngisiLog_t *, int *);
int ngisiLogPrintf(
    ngisiContext_t *, ngisiLogLevel_t, const char *, char *, ...);
int ngisiLogVprintfInternal(
    ngisiContext_t *, ngisiLogLevel_t, const char *, char *, char *, va_list);

int ngisiRequestReaderInitialize(
    ngisiContext_t *, ngisiRequestReader_t *, int *);
int ngisiRequestReaderFinalize(
    ngisiContext_t *, ngisiRequestReader_t *, int *);
int ngisiRequestReaderWaitDone(ngisiContext_t *, int *);
int ngisiCreateAttrGetCount(
    ngisiContext_t *, ngisiCreateAttr_t *, char *, int *, int *);
int ngisiCreateAttrGet(
    ngisiContext_t *, ngisiCreateAttr_t *, char *, int, char **, int *);
int ngisiCreateAttrGetRemain(
    ngisiContext_t *, ngisiCreateAttr_t *, int *, char ***, int *);
int ngisiNotifyJobStatus(
    ngisiContext_t *, char *, ngisiJobStatus_t, char *, int *);

ngisiJob_t *ngisiJobConstruct(
    ngisiContext_t *, char *, ngisiCreateAttr_t *, int *);
int ngisiJobDestruct(ngisiContext_t *, ngisiJob_t *, int *);
int ngisiJobDestructAll(ngisiContext_t *, int *);
int ngisiJobGetJobID(ngisiContext_t *, ngisiJob_t *, char **, int *);
int ngisiJobStart(ngisiContext_t *, ngisiJob_t *, int *);
int ngisiJobProtocolDestroy(ngisiContext_t *, char *, int *);
int ngisiJobProtocolStatus(
    ngisiContext_t *, char *, ngisiJobStatus_t *, int *);
int ngisiJobConnectionClosed(ngisiContext_t *, int *);
int ngisiJobRefreshCredential(ngisiContext_t *, int *);

int ngisiRefreshCredentialInitialize(
    ngisiContext_t *, ngisiRefreshCredential_t *, int *);
int ngisiRefreshCredentialFinalize(
    ngisiContext_t *, ngisiRefreshCredential_t *, int *);
int ngisiRefreshCredentialUpdateIntervalSet(
    ngisiContext_t *, ngisiRefreshCredential_t *, int, int *);

int ngisiMutexInitialize(ngisiContext_t *, globus_mutex_t *, int *);
int ngisiMutexFinalize(ngisiContext_t *, globus_mutex_t *, int *);
int ngisiMutexLock(ngisiContext_t *, globus_mutex_t *, int *);
int ngisiMutexUnlock(ngisiContext_t *, globus_mutex_t *, int *);

int ngisiCondInitialize(ngisiContext_t *, globus_cond_t *, int *);
int ngisiCondFinalize(ngisiContext_t *, globus_cond_t *, int *);
int ngisiCondSignal(ngisiContext_t *, globus_cond_t *, int *);
int ngisiCondWait(ngisiContext_t *, globus_cond_t *, globus_mutex_t *, int *);
int ngisiCondTimedWait(ngisiContext_t *,
    globus_cond_t *, globus_mutex_t *, int, int *, int *);

int ngisiReadBufferInitialize(ngisiContext_t *, ngisiReadBuffer_t *, int *);
int ngisiReadBufferFinalize(ngisiContext_t *, ngisiReadBuffer_t *, int *);
int ngisiReadLine(ngisiContext_t *, FILE *, ngisiReadBuffer_t *, int *);

#endif /* _NG_INVOKE_SERVER_H_ */

