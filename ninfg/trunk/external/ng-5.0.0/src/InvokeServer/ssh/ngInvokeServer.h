/*
 * $RCSfile: ngInvokeServer.h,v $ $Revision: 1.6 $ $Date: 2008/03/03 09:11:21 $
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
#ifndef _NG_INVOKE_SERVER_H_
#define _NG_INVOKE_SERVER_H_

#include "ngEnvironment.h"
#include "ngisList.h"
#include "ngisLog.h"
#include "ngisUtility.h"

/* CONSTANTS */
#define NGIS_JOB_CREATE_REQUEST  "JOB_CREATE"
#define NGIS_JOB_STATUS_REQUEST  "JOB_STATUS"
#define NGIS_JOB_DESTROY_REQUEST "JOB_DESTROY"
#define NGIS_JOB_QUERY_FEATURES  "QUERY_FEATURES"
#define NGIS_EXIT_REQUEST        "EXIT"

#define NGIS_JOB_CREATE_REQUEST_END  "JOB_CREATE_END"

#define NGIS_CREATE_NOTIFY "CREATE_NOTIFY"
#define NGIS_STATUS_NOTIFY "STATUS_NOTIFY"

#define NGIS_AUTH_NUMBER_FEATURE "STAGING_AUTH_NUMBER"

/* TYPES */

/* Enum */
typedef enum ngisCallbackResult_e {
    NGIS_CALLBACK_RESULT_SUCCESS,
    NGIS_CALLBACK_RESULT_CANCEL,
    NGIS_CALLBACK_RESULT_EOF,
    NGIS_CALLBACK_RESULT_FAILED
} ngisCallbackResult_t;

/**
 * Job Status
 */
typedef enum ngisJobStatus_s {
    NGIS_STATUS_PENDING,
    NGIS_STATUS_ACTIVE,
    NGIS_STATUS_DONE,
    NGIS_STATUS_FAILED
} ngisJobStatus_t;

#define NGIS_JOB_STATUS_ASSERT(status) \
    NGIS_ASSERT((status == NGIS_STATUS_PENDING) || \
                (status == NGIS_STATUS_ACTIVE)  || \
                (status == NGIS_STATUS_DONE)    || \
                (status == NGIS_STATUS_FAILED))

/**
 * Job Backend
 */
typedef enum ngisJobBackend_e {
    NGIS_BACKEND_NORMAL,
    NGIS_BACKEND_MPI,
    NGIS_BACKEND_BLACS
} ngisJobBackend_t;

/* Declare */
typedef struct ngisOptionContainer_s ngisOptionContainer_t;
typedef struct ngisJob_s             ngisJob_t;
typedef struct ngisProtocol_s        ngisProtocol_t;
typedef struct ngisOptionElement_s   ngisOptionElement_t;
typedef struct ngislCallbackEntity_s ngislCallbackEntity_t;
typedef struct ngisLineBuffer_s      ngisLineBuffer_t;

typedef int (*ngisJobInitialize_t)(ngisJob_t *, ngisOptionContainer_t *);
typedef int (*ngisJobFinalize_t)(ngisJob_t *);
typedef int (*ngisJobCancel_t)(ngisJob_t *);
typedef void (*ngisReadCallbackFunc_t)
    (void *, int, void *, size_t, ngisCallbackResult_t);
typedef void (*ngisWriteCallbackFunc_t)
    (void *, int, void *, size_t, size_t, ngisCallbackResult_t);
typedef void (*ngisTimerCallbackFunc_t)(void *, ngisCallbackResult_t);
typedef void (*ngisWaitCallbackFunc_t)
    (void *, pid_t, int, ngisCallbackResult_t);
typedef void (*ngisWriteStringCallbackFunc_t)
    (void *, int, ngisCallbackResult_t);
typedef void (*ngisLineBufferCallbackFunc_t)
    (void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

/* Lists */
NGIS_DECLARE_LIST_OF(ngisOptionElement_t);
NGIS_DECLARE_LIST_OF(ngisJob_t);
NGIS_DECLARE_LIST_OF(ngisSourceBase_t);
NGIS_DECLARE_LIST_OF(ngisTimer_t);
NGIS_DECLARE_LIST_OF(ngisProcessWaiter_t);
NGIS_DECLARE_LIST_OF(char);
NGIS_DECLARE_LIST_OF(ngislCallbackEntity_t);

typedef NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) ngisCallback_t;

/**
 * Container for "invoke_server_option" sent from Ninf-G client
 */
struct ngisOptionElement_s {
    char *ngoe_name;
    char *ngoe_value;
};

struct ngisOptionContainer_s {
    NGIS_LIST_OF(ngisOptionElement_t) ngoc_list;
};

typedef NGIS_LIST_ITERATOR_OF(ngisOptionElement_t) ngisOption_t;
/**
 * Mapping of status and string.
 */
extern const char *ngisJobStatusStrings[];

/**
 * Job Function Table
 */
typedef struct ngisJobTypeInformation_s {
    size_t              ngjti_size;
    ngisJobInitialize_t ngjti_initializer;
    ngisJobFinalize_t   ngjti_finalizer;
    ngisJobCancel_t     ngjti_canceler;
} ngisJobTypeInformation_t;

/**
 * Job
 */
struct ngisJob_s {
    char           *ngj_jobID;
    char           *ngj_requestID;
    ngisProtocol_t *ngj_protocol;
    ngisJobStatus_t ngj_status;
    ngisLog_t      *ngj_log;
    int             ngj_destroyRequested;
};

/**
 * Ninf-G Protocol Talker.
 */
struct ngisProtocol_s {
    ngisStandardIO_t         ngp_stdio;
    ngisLineBuffer_t        *ngp_lineBuffer;
    ngisOptionContainer_t   *ngp_optionContainer;
    char                    *ngp_requestId;
    char                    *ngp_jobCreateFailureMessage;
    NGIS_LIST_OF(ngisJob_t)  ngp_jobList;
    ngisCallback_t           ngp_notifyCallback;
    ngisStringBuffer_t       ngp_notifyQueue;
    int                      ngp_notifyQueueInitialized;
    ngisLog_t               *ngp_log;
};

/**
 * Job Attribute
 */
typedef struct ngisJobAttributes_s {
    char              *ngja_hostname;         /* Always sent */
    unsigned short     ngja_port;             /* Always sent */
    char              *ngja_jobManager;
    char              *ngja_clientHostname;   /* Always sent */
    char              *ngja_executablePath;   /* Always sent */
    ngisJobBackend_t   ngja_jobBackend;       /* Always sent */
    NGIS_LIST_OF(char) ngja_arguments;
    char              *ngja_workDirectory; 
    NGIS_LIST_OF(char) ngja_environments;
    unsigned int       ngja_statusPolling;    /* Always sent */
    unsigned int       ngja_refreshCredential;/* Always sent */
    unsigned int       ngja_count;            /* Always sent */
    int                ngja_staging;          /* Always sent */
    int                ngja_redirectEnable;   /* Always sent */
    char              *ngja_stdoutFile;
    char              *ngja_stderrFile;
    int                ngja_maxTime;
    int                ngja_maxWallTime;
    int                ngja_maxCpuTime;
    char              *ngja_project;
    char              *ngja_queueName;
    int                ngja_hostCount;
    int                ngja_minMemory;
    int                ngja_maxMemory;
    char              *ngja_tmpDir;
    unsigned int       ngja_authNumber;
    int                ngja_commProxyStaging;
    char              *ngja_commProxyPath;

    /* GT */
    char              *ngja_gassURL;
    NGIS_LIST_OF(char) ngja_rslExtensions;
} ngisJobAttributes_t;

/**
 * Callback by reading each line.
 */
struct ngisLineBuffer_s {
    int                          nglb_fd;
    char                        *nglb_buffer;
    char                        *nglb_first;
    char                        *nglb_last;
    size_t                       nglb_capacity;
    char                        *nglb_separator;
    ngisLineBufferCallbackFunc_t nglb_func;
    void                        *nglb_arg;
    ngisCallback_t               nglb_callback;
    int                          nglb_callbackValid;
    ngisCallbackResult_t         nglb_result;
};

/**
 * Invoke Server
 */ 
typedef struct ngInvokeServer_s {
    ngisJobTypeInformation_t *ngis_jobTypeInformation;
    ngisProtocol_t           *ngis_protocol;
    ngisLog_t                *ngis_log;
} ngInvokeServer_t;

/* Option Container */
ngisOptionContainer_t *ngisOptionContainerCreate(void);
int ngisOptionContainerDestroy(ngisOptionContainer_t *);

int          ngisOptionContainerAdd(ngisOptionContainer_t *, char *);
char *       ngisOptionContainerGet(ngisOptionContainer_t *, char *);
ngisOption_t ngisOptionContainerBegin(ngisOptionContainer_t *);
ngisOption_t ngisOptionContainerEnd(ngisOptionContainer_t *);
ngisOption_t ngisOptionErase(ngisOption_t);
ngisOption_t ngisOptionContainerFindFirst(ngisOptionContainer_t *, char *);

ngisOption_t ngisOptionNext(ngisOption_t);

ngisOption_t ngisOptionFind(ngisOption_t, ngisOption_t, char *);
char *ngisOptionName(ngisOption_t);
char *ngisOptionValue(ngisOption_t);
int ngisOptionIs(ngisOption_t, char *);

/* Job */
ngisJob_t *ngisJobCreate(ngisProtocol_t *, char *, ngisOptionContainer_t *);
int ngisJobDestroy(ngisJob_t *);
int ngisJobSetStatus(ngisJob_t *, ngisJobStatus_t, char *);
ngisJobStatus_t ngisJobGetStatus(ngisJob_t *);
int ngisJobRegisterID(ngisJob_t *, char *);
int ngisJobCancel(ngisJob_t *);

/* Protocol */
ngisProtocol_t *ngisProtocolCreate();
int  ngisProtocolDestroy(ngisProtocol_t *);
int ngisProtocolDisable(ngisProtocol_t *);
int ngisProtocolRegisterJob(ngisProtocol_t *, ngisJob_t *);
int ngisProtocolUnregisterJob(ngisProtocol_t *, ngisJob_t *);
ngisJob_t *ngisProtocolFindJob(ngisProtocol_t *, char *);

int ngisProtocolSendSuccessReply(
    ngisProtocol_t *, ngisWriteStringCallbackFunc_t);
int ngisProtocolSendStatusSuccessReply(
    ngisProtocol_t *, ngisWriteStringCallbackFunc_t, ngisJobStatus_t);
int ngisProtocolSendQueryFeaturesSuccessReply(
    ngisProtocol_t *, ngisWriteStringCallbackFunc_t);
int ngisProtocolSendFailureReply(
    ngisProtocol_t *, ngisWriteStringCallbackFunc_t, char *);

int ngisProtocolSendCreateNotify(ngisProtocol_t *, int, char *, char *);
int ngisProtocolSendStatusNotify(
    ngisProtocol_t *, char *, ngisJobStatus_t, char *);
/* Job Attributes */
ngisJobAttributes_t *ngisJobAttributesCreate(ngisOptionContainer_t *);
void ngisJobAttributesDestroy(ngisJobAttributes_t *);

int ngisAttrSetInt(int *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetLong(long *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetUshort(unsigned short *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetUint(unsigned int *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetUlong(unsigned long *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetEnum(int *, ngisOptionContainer_t *, char *, int, char **, int);
int ngisAttrSetBool(int *, ngisOptionContainer_t *, char *, int);
int ngisAttrSetBackend(ngisJobBackend_t *,
    ngisOptionContainer_t *, char *, int);
int ngisAttrSetString(char **, ngisOptionContainer_t *, char *, int);
int ngisAttrSetStringList(
    NGIS_LIST_OF(char) *, ngisOptionContainer_t *, char *);

int ngisAttrDefaultSetString(char **, char *);

/* Invoke Server */
int ngInvokeServerInitialize(ngisJobTypeInformation_t *);
int ngInvokeServerRun(void);
int ngInvokeServerFinalize(void);

int ngisCallbackManagerInitialize(void);
int ngisCallbackManagerRun(void);
int ngisCallbackManagerFinalize(void);

/* Callback Manager */
ngisCallback_t ngisCallbackRead(int, ngisReadCallbackFunc_t, void *arg);
ngisCallback_t ngisCallbackWrite(
    int, ngisWriteCallbackFunc_t, void *, size_t, void *);
ngisCallback_t ngisCallbackWriteFormat(int, ngisWriteStringCallbackFunc_t,
    void *, const char *, ...) NG_ATTRIBUTE_PRINTF(4, 5);
ngisCallback_t ngisCallbackWriteVformat(int, ngisWriteStringCallbackFunc_t,
    void *, const char *, va_list);
ngisCallback_t ngisCallbackSetTimer(int, ngisTimerCallbackFunc_t, void *);
ngisCallback_t ngisCallbackWait(pid_t, ngisWaitCallbackFunc_t, void *);

void ngisCallbackCancel(ngisCallback_t);
int ngisCallbackIsValid(ngisCallback_t);

ngisLineBuffer_t *ngisLineBufferCreate(int, const char *);
void ngisLineBufferDestroy(ngisLineBuffer_t *);

int ngisLineBufferReadLine(
    ngisLineBuffer_t *, ngisLineBufferCallbackFunc_t, void *);

#endif /* _NG_INVOKE_SERVER_H_ */
