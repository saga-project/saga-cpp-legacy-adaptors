#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile$ $Revision$ $Date$";
#endif /* NGIS_NO_WARN_RCSID */
/*
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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "ngisUtility.h"
#include "ngInvokeServer.h"

#if 0
#define NGISL_PROTOCOL_LINE_SEPARATOR "\n" /* for Debug */
#else
#define NGISL_PROTOCOL_LINE_SEPARATOR "\r\n"
#endif

#define NGISL_REQUEST_ID_JOB_CREATE  0
#define NGISL_REQUEST_ID_JOB_STATUS  1
#define NGISL_REQUEST_ID_JOB_DESTROY 2
#define NGISL_REQUEST_ID_EXIT        3

/**
 * Informations of each request
 */
typedef struct ngislRequestInformation_s {
    char *ngri_string;
    int   ngri_needID;
}ngislRequestInformation_t;

ngislRequestInformation_t ngislRequestInformation[] = {
    {NGIS_JOB_CREATE_REQUEST , 1}, 
    {NGIS_JOB_STATUS_REQUEST , 1},
    {NGIS_JOB_DESTROY_REQUEST, 1},
    {NGIS_EXIT_REQUEST       , 0}
};

/* File Local Functions */
static void ngislProtocolRequestCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static void ngislProtocolCreateArgumentCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static int ngislProtocolAnalyzeRequest(ngisProtocol_t *, char *, char **);
static int ngisProtocolProcessJobCreateRequest(ngisProtocol_t *, char *);
static int ngisProtocolProcessJobStatusRequest(ngisProtocol_t *, char *);
static int ngisProtocolProcessJobDestroyRequest(ngisProtocol_t *, char *);
static int ngisProtocolProcessExitRequest(ngisProtocol_t *);

static void ngislProtocolReplyCallback(void *, int, ngisCallbackResult_t);
static void ngislProtocolCreateReplyCallback(void *, int, ngisCallbackResult_t);
static void ngislProtocolExitReplyCallback(void *, int, ngisCallbackResult_t);

static int ngislProtocolSendNotify(ngisProtocol_t *, char *, ...)
   NGIS_ATTRIBUTE_PRINTF(2, 3);
static void ngislProtocolNotifyCallback(void *, int, ngisCallbackResult_t);

/**
 * Protocol: Create
 */
ngisProtocol_t *
ngisProtocolCreate()
{
    ngisProtocol_t *new = NULL;
    ngisLineBuffer_t *lBuffer = NULL;
    int result;
    ngisLog_t *log = NULL;
    ngisLog_t *newLog = NULL;
    static const char fName[] = "ngisProtocolCreate";

    newLog = ngisLogCreate("Protocol");
    if (newLog == NULL) {
        ngisErrorPrint(log, fName, "Can't create a log.\n");
        goto error;
    }
    log = newLog;

    new = NGIS_ALLOC(ngisProtocol_t);
    if (new == NULL) {
        ngisErrorPrint(log, fName, "Can't allocate storage for protocol.\n");
        goto error;
    }
    
    /* Initialize members */
    new->ngp_stdio.ngsio_in    = 0;
    new->ngp_stdio.ngsio_out   = 1;
    new->ngp_stdio.ngsio_error = 2;        
    new->ngp_lineBuffer        = NULL;
    new->ngp_optionContainer   = NULL;
    new->ngp_requestId         = NULL;
    new->ngp_log               = newLog;
    new->ngp_notifyCallback    = NULL;
    new->ngp_notifyQueueInitialized  = 0;
    new->ngp_jobCreateFailureMessage = NULL;
    NGIS_LIST_SET_INVALID_VALUE(&new->ngp_jobList);

    NGIS_LIST_INITIALIZE(ngisJob_t, &new->ngp_jobList);

    lBuffer = ngisLineBufferCreate(new->ngp_stdio.ngsio_in,
        NGISL_PROTOCOL_LINE_SEPARATOR);
    if (lBuffer == NULL) {
        ngisErrorPrint(log, fName, 
            "Can't create line buffer for receiving Invoke Server Protocol.\n");
        goto error;
    }
    new->ngp_lineBuffer = lBuffer;

    /* Register Callback for reading a line. */
    result = ngisLineBufferReadLine(
        lBuffer, ngislProtocolRequestCallback, (void *)new);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't register for reading a line.\n");
        goto error;
    }
    ngisDebugPrint(log, fName, "Creates new protocol.\n");
    
    return new;
error:
    if (lBuffer != NULL) {
        ngisLineBufferDestroy(lBuffer);
        lBuffer = NULL;
    }

    if (!NGIS_LIST_IS_INVALID_VALUE(&new->ngp_jobList)) {
        NGIS_LIST_FINALIZE(ngisJob_t, &new->ngp_jobList);
    }
    NGIS_NULL_CHECK_AND_FREE(new);

    if (newLog != NULL) {
        result = ngisLogDestroy(newLog);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy the log.\n");
        }
    }
    return NULL;
}

/**
 * Protocol: Destroy
 */
int
ngisProtocolDestroy(
    ngisProtocol_t *protocol)
{
    int ret = 1;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolDestroy";

    NGIS_ASSERT(protocol != NULL);

    log = protocol->ngp_log;

    NGIS_NULL_CHECK_AND_FREE(protocol->ngp_requestId);
    
    if (protocol->ngp_optionContainer != NULL) {
        result = ngisOptionContainerDestroy(protocol->ngp_optionContainer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy the option container.\n");
            ret = 0;
        }
        protocol->ngp_optionContainer = NULL;
    }
   
    result = ngisProtocolDisable(protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disable the protocol.\n");
        ret = 0;
    }

    NGIS_LIST_FINALIZE(ngisJob_t, &protocol->ngp_jobList);

    result = ngisLogDestroy(protocol->ngp_log);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy the log.\n");
        ret = 0;
    }
    protocol->ngp_log = NULL;

    protocol->ngp_stdio.ngsio_in    = -1;
    protocol->ngp_stdio.ngsio_out   = -1;
    protocol->ngp_stdio.ngsio_error = -1;        
    protocol->ngp_lineBuffer        = 0;
    protocol->ngp_optionContainer   = NULL;
    protocol->ngp_requestId         = NULL;
    protocol->ngp_notifyCallback    = NULL;
    protocol->ngp_notifyQueueInitialized  = 0;
    protocol->ngp_jobCreateFailureMessage = NULL;
    
    NGIS_FREE(protocol);
    
    return ret;
}

/**
 * Protocol: Callback function called on receiving request from Ninf-G client
 */
static void
ngislProtocolRequestCallback(
    void *arg,
    ngisLineBuffer_t *lBuffer,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = (ngisProtocol_t *)arg;
    char *id = NULL;
    int request;
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngislProtocolRequestCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(lBuffer != NULL);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName,
            "Can't read the request of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisDebugPrint(log, fName, "End of file.\n");
        goto protocol_end;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    ngisDebugPrint(log, fName, "Read \"%s\".\n", line);
    
    /* Command */
    request = ngislProtocolAnalyzeRequest(protocol, line, &id);
    if (request < 0) {
        ngisErrorPrint(log, fName,
            "Can't analyze \"%s\" as a request.\n", line);

        result = ngisProtocolSendFailureReply(protocol,
            ngislProtocolReplyCallback, "Unknown Request");
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't send request.\n");
            goto protocol_end;
        }
        /* Failed */
        return;
    }
    switch(request) {
    case NGISL_REQUEST_ID_JOB_CREATE:
        result = ngisProtocolProcessJobCreateRequest(protocol, id);
        break;
    case NGISL_REQUEST_ID_JOB_STATUS:
        result = ngisProtocolProcessJobStatusRequest(protocol, id);
        break;
    case NGISL_REQUEST_ID_JOB_DESTROY:
        result = ngisProtocolProcessJobDestroyRequest(protocol, id);
        break;
    case NGISL_REQUEST_ID_EXIT:
        result = ngisProtocolProcessExitRequest(protocol);
        break;
    default:
        result = 0;
        NGIS_ASSERT_NOTREACHED();
    }
    if (result == 0) {
        /* Doesn't print log */
        goto protocol_end;
    }
    return;

protocol_end:
    ngisDebugPrint(log, fName, "Disables the protocol.\n");
    result = ngisProtocolDisable(protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disables the protocol.\n");
    }

    return;
}

/**
 * Protocol: Analyze Request of Invoke Server Protocol.
 */
static int
ngislProtocolAnalyzeRequest(
    ngisProtocol_t *protocol,
    char *line,
    char **id)
{
    char *requestString = NULL;
    char *p = NULL;
    int i;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngislProtocolAnalyzeRequest";
    
    NGIS_ASSERT_STRING(line);
    NGIS_ASSERT(id != NULL);

    log = protocol->ngp_log;

    /* Get kind of requests */
    for (i = 0;i < NGIS_NELEMENTS(ngislRequestInformation);++i) {
        requestString = ngislRequestInformation[i].ngri_string;
        if (strncmp(line, requestString, strlen(requestString)) == 0) {
            p = &line[strlen(requestString)];
            break;
        }
    }
    if (i == NGIS_NELEMENTS(ngislRequestInformation)) {
        /* not Found, */
        goto error;
    }
    
    if (ngislRequestInformation[i].ngri_needID != 0) {
        /* Get ID */
        if (!isspace((int)*p)) {
            goto error;
        }
        *id = ++p;
    } else {
        if (strlen(p) > 0) {
            goto error;
        }
        *id = NULL;
    }
    ngisDebugPrint(log, fName, "%s request is received.\n", requestString);    

    return i;

error:
    ngisErrorPrint(log, fName, "\"%s\" is unknown request.\n", line);    

    return -1;
}

/**
 * Protocol: Process JOB_CREATE Request.
 */
static int
ngisProtocolProcessJobCreateRequest(
    ngisProtocol_t *protocol,
    char *requestID)
{
    int result;
    ngisOptionContainer_t *opts = NULL;
    char *copyRequestID= NULL;
    int fatal = 0;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolProcessJobCreateRequest";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_STRING(requestID);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Process JOB_CREATE.\n");

    protocol->ngp_jobCreateFailureMessage = NULL;

    opts = ngisOptionContainerCreate();
    if (opts == NULL) {
        protocol->ngp_jobCreateFailureMessage
            = "Can't create option container.";
        goto finalize;
    }
           
    copyRequestID = strdup(requestID);
    if (copyRequestID == NULL) {
       protocol->ngp_jobCreateFailureMessage
           = "Can't allocate storage for Request ID.";
       goto finalize;
    }
            
    protocol->ngp_optionContainer = opts;
    protocol->ngp_requestId = copyRequestID;
    opts = NULL;
    copyRequestID = NULL;

finalize:
    if (protocol->ngp_jobCreateFailureMessage != NULL) {
        ngisErrorPrint(log, fName, "%s.\n",
            protocol->ngp_jobCreateFailureMessage);
    }
    NGIS_NULL_CHECK_AND_FREE(copyRequestID);

    if (opts != NULL) {
        result = ngisOptionContainerDestroy(opts);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy option container.\n");
        }
    }

    result = ngisLineBufferReadLine(protocol->ngp_lineBuffer,
        ngislProtocolCreateArgumentCallback, protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register callback function for reading line.\n");
        fatal = 1;
    }

    /* If Error isn't fatal, Successful */
    return fatal == 0;
}

/**
 * Protocol: Process JOB_STATUS Request.
 */
static int
ngisProtocolProcessJobStatusRequest(
    ngisProtocol_t *protocol,
    char *jobID)
{
    ngisJob_t *job;
    char *failureReplyMessage = NULL;
    int fatal = 0;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolProcessJobStatusRequest";

    NGIS_ASSERT(protocol  != NULL);
    NGIS_ASSERT_STRING(jobID);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Process JOB_STATUS.\n");

    job = ngisProtocolFindJob(protocol, jobID);
    if (job == NULL) {
        failureReplyMessage = "Invalid job id.";
        goto error;
    }

    result = ngisProtocolSendStatusSuccessReply(
        protocol, ngislProtocolReplyCallback, ngisJobGetStatus(job));
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't send reply.\n");
        fatal = 1;
        goto error;
    }    

    return 1;
error:
    if (fatal == 0) {
        NGIS_ASSERT(failureReplyMessage != NULL);
        ngisErrorPrint(log, fName, failureReplyMessage);
        result = ngisProtocolSendFailureReply(
            protocol, ngislProtocolReplyCallback, failureReplyMessage);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't send reply.\n");
            fatal = 1;
        }    
    }

    /* If Error is'nt fatal, Successful */
    return fatal == 0;
}

/**
 * Protocol: Process JOB_DESTROY Request.
 */
static int
ngisProtocolProcessJobDestroyRequest(
    ngisProtocol_t *protocol,
    char *jobID)
{
    ngisJob_t *job;
    char *failureReplyMessage = NULL;
    int fatal = 0;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolProcessJobDestroyRequest";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_STRING(jobID);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Process JOB_DESTROY.\n");

    job = ngisProtocolFindJob(protocol, jobID);
    if (job == NULL) {
        failureReplyMessage = "Invalid Job ID";
        goto error;
    }

    switch (ngisJobGetStatus(job)) {
    case NGIS_STATUS_DONE:
    case NGIS_STATUS_FAILED:
        result = ngisJobDestroy(job);
        if (result == 0) {
            failureReplyMessage = "Can't destroy the job.";
            goto error;
        }
        job = NULL;
        break;
    default:
        result = ngisJobCancel(job);
        if (result == 0) {
            failureReplyMessage = "Can't cancel.";
            goto error;
        }
    }

    result = ngisProtocolSendSuccessReply(
        protocol, ngislProtocolReplyCallback);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't send reply.\n");
        fatal = 1;
        goto error;
    }    

    return 1;
error:
    if (fatal == 0) {
        NGIS_ASSERT(failureReplyMessage != NULL);
        ngisErrorPrint(log, fName, failureReplyMessage);
        result = ngisProtocolSendFailureReply(protocol,
            ngislProtocolReplyCallback, failureReplyMessage);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't send reply.\n");
            fatal = 1;
        }    
    }

    /* If Error isn't fatal, Successful */
    return fatal == 0;
}

/**
 * Protocol: Process JOB_EXIT Request.
 */
static int 
ngisProtocolProcessExitRequest(
    ngisProtocol_t *protocol)
{
    int result;
    int fatal = 0;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolProcessExitRequest";

    NGIS_ASSERT(protocol != NULL);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Process EXIT\n");

    result = ngisProtocolSendSuccessReply(protocol,
        ngislProtocolExitReplyCallback);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't send reply.\n");
        fatal = 1;
    }

    return fatal == 0;
}

/**
 * Protocol: Process argument of JOB_CREATE 
 */
static void
ngislProtocolCreateArgumentCallback(
    void *arg,
    ngisLineBuffer_t *lBuffer,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = (ngisProtocol_t *)arg;
    int result;
    int fatal = 0;
    ngisLog_t *log;
    static const char fName[] = "ngislProtocolCreateArgumentCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(lBuffer != NULL);

    log = protocol->ngp_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName,
            "Can't read argument of \"CREATE\" request.\n");
        fatal = 1;
        goto finalize;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexpect EOF.\n");
        fatal = 1;
        goto finalize;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisErrorPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    NGIS_ASSERT_STRING(line);

    /* Analyzes arguments of JOB_CREATE */
    if (strcmp(line, NGIS_JOB_CREATE_REQUEST_END) != 0) {
        ngisDebugPrint(log, fName,
            "Process argument of JOB_CREATE(%s).\n", line);

        if (protocol->ngp_jobCreateFailureMessage != NULL) {
            /* Do nothing if error already has occurred. */;
        } else {
            NGIS_ASSERT(protocol->ngp_optionContainer != NULL);
            result = ngisOptionContainerAdd(
                protocol->ngp_optionContainer, line);
            if (result == 0) {
                protocol->ngp_jobCreateFailureMessage = 
                    "Can't add item to option container";
                ngisErrorPrint(log, fName, "%s.\n",
                    protocol->ngp_jobCreateFailureMessage);
            }
        }

        /* Next Line */
        result = ngisLineBufferReadLine(
            lBuffer, ngislProtocolCreateArgumentCallback, arg);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for reading a lined.\n");
            fatal = 1;
            goto finalize;
        }
        return;
    }

    /* JOB_CREATE_END */
    ngisDebugPrint(log, fName, "JOB_CREATE_END is received.\n");
        
    if (protocol->ngp_jobCreateFailureMessage != NULL) {
        result = ngisProtocolSendFailureReply(protocol,
            ngislProtocolReplyCallback, protocol->ngp_jobCreateFailureMessage);
        protocol->ngp_jobCreateFailureMessage = NULL;
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't send reply.\n");
            fatal = 1;
        }
        goto finalize;
    }
        
    /* Send Reply */
    result = ngisProtocolSendSuccessReply(
        protocol, ngislProtocolCreateReplyCallback);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't send reply.\n");
        fatal = 1;
        goto finalize;
    }
    return;

finalize:

    result = ngisOptionContainerDestroy(protocol->ngp_optionContainer);
    protocol->ngp_optionContainer = NULL;
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy option container.\n");
    }
    NGIS_NULL_CHECK_AND_FREE(protocol->ngp_requestId);

    if (fatal == 0) {
        /* Next Line */
        result = ngisLineBufferReadLine(
            lBuffer, ngislProtocolRequestCallback, arg);
        if (result == 0) {
            ngisErrorPrint(log, fName, 
                "Can't register callback for reading a line.\n");
            fatal = 1;
        }
    }

    if (fatal != 0) {/* Fatal */
        ngisDebugPrint(log, fName, "Disables the protocol.\n");
        result = ngisProtocolDisable(protocol);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't disable the protocol.\n");
        }
    }

    return;
}

static void
ngislProtocolCreateReplyCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = arg;
    int fatal = 0;
    ngisJob_t *job = NULL;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislProtocolCreateReplyCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_FD(fd);

    log = protocol->ngp_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't send \"Create \" reply.\n");
        fatal = 1;
        goto finalize;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisErrorPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Next Line */
    result = ngisLineBufferReadLine(protocol->ngp_lineBuffer,
        ngislProtocolRequestCallback, (void *)protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't register for reading a line.\n");
        fatal = 1;
        goto finalize;
    }

    job = ngisJobCreate(protocol, protocol->ngp_requestId,
        protocol->ngp_optionContainer);
    if (job == NULL) {
        ngisErrorPrint(log, fName, "Can't create a job.\n");
        goto finalize;
    }

finalize:

    if (job == NULL) {
        result = ngisProtocolSendCreateNotify(
            protocol, 0, protocol->ngp_requestId, "Can't submit a job");
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't send notify.\n");
            fatal = 1;
        }
    }

    if (protocol->ngp_optionContainer != NULL) {
        result = ngisOptionContainerDestroy(protocol->ngp_optionContainer);
        protocol->ngp_optionContainer = NULL;
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy option container.\n");
        }
    }

    NGIS_ASSERT(protocol->ngp_requestId);
    NGIS_NULL_CHECK_AND_FREE(protocol->ngp_requestId);

    if (fatal != 0) {
        ngisDebugPrint(log, fName, "Disables the protocol.\n");
        result = ngisProtocolDisable(protocol);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't disable the protocol.\n");
        }
    }
    return;
}

/**
 * Invoke Server: Register Job 
 */
int
ngisProtocolRegisterJob(
    ngisProtocol_t *protocol,
    ngisJob_t *job)
{
    int result;
    static const char fName[] = "ngisProtocolRegisterJob";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(job != NULL);
    
    result = NGIS_LIST_INSERT_TAIL(ngisJob_t,
        &protocol->ngp_jobList, job);
    if (result == 0) {
        ngisErrorPrint(protocol->ngp_log, fName,
            "Can't insert the job handle to list.\n");
        return 0;
    }
    job->ngj_protocol = protocol;
    
    return 1;
}

/**
 * Invoke Server: Unregister Job 
 */
int
ngisProtocolUnregisterJob(
    ngisProtocol_t *protocol,
    ngisJob_t *job)
{
    int result;
    static const char fName[] = "ngisProtocolUnregisterJob";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(job != NULL);

    result = NGIS_LIST_ERASE_BY_ADDRESS(ngisJob_t,
        &protocol->ngp_jobList, job);
    if (result == 0) {
        ngisErrorPrint(protocol->ngp_log, fName,
            "Can't erase the job handle from list.\n");
        return 0;
    }
    job->ngj_protocol = NULL;

    return 1;
}

/**
 * Invoke Server: Find Job  by Job ID.
 */
ngisJob_t *
ngisProtocolFindJob(
    ngisProtocol_t * protocol,
    char* jobID)
{
    NGIS_LIST_ITERATOR_OF(ngisJob_t) it;
    NGIS_LIST_ITERATOR_OF(ngisJob_t) last;
    ngisJob_t *job;
#if 0
    static const char fName[] = "ngisProtocolFindJob";
#endif
    NGIS_ASSERT(jobID != NULL && strlen(jobID) > 0);
    
    it = NGIS_LIST_BEGIN(ngisJob_t, &protocol->ngp_jobList);
    last = NGIS_LIST_END(ngisJob_t, &protocol->ngp_jobList);
    while (it != last) {
        job = NGIS_LIST_GET(ngisJob_t, it);
        if ((job->ngj_jobID != NULL) &&
            (strcmp(job->ngj_jobID, jobID) == 0)) {
            return job;
        }
        it = NGIS_LIST_NEXT(ngisJob_t, it);
    }

    /* Not found */    
    return NULL;
}

/**
 * Protocol: Disable
 */
int
ngisProtocolDisable(
    ngisProtocol_t *protocol)
{
    NGIS_LIST_ITERATOR_OF(ngisJob_t) it;
    NGIS_LIST_ITERATOR_OF(ngisJob_t) last;
    ngisJob_t *job;
    ngisLog_t *log = NULL;
    int result;
    int ret = 1;
    static const char fName[] = "ngisProtocolDisable";

    NGIS_ASSERT(protocol != NULL);
    log = protocol->ngp_log;

    /* All Job  Destroy */
    last = NGIS_LIST_END(ngisJob_t, &protocol->ngp_jobList);
    while ((it = NGIS_LIST_BEGIN(ngisJob_t, &protocol->ngp_jobList)) != last) {
        job = NGIS_LIST_GET(ngisJob_t, it);
        NGIS_ASSERT(job != NULL);

        switch (ngisJobGetStatus(job)) {
        case NGIS_STATUS_DONE:
        case NGIS_STATUS_FAILED:
            result = ngisJobDestroy(job);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't destroy the job.");
            }
            break;
        default:
            /* Unregister before canceling the job, because the job is
             * destroyed in canceling the job */
            result = ngisProtocolUnregisterJob(protocol, job);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't unregister the job.");
            }
            result = ngisJobCancel(job);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't cancel the job.");
            }
            break;
        }
        job = NULL;
    }

    /* Unregister Source */
    if (protocol->ngp_lineBuffer != NULL) {
        ngisLineBufferDestroy(protocol->ngp_lineBuffer);
        protocol->ngp_lineBuffer = NULL;
    }

    if (ngisCallbackIsValid(protocol->ngp_notifyCallback)) {
        ngisCallbackCancel(protocol->ngp_notifyCallback);
        protocol->ngp_notifyCallback = NULL;
    }

    if (protocol->ngp_notifyQueueInitialized != 0) {
        ngisStringBufferFinalize(&protocol->ngp_notifyQueue);
        protocol->ngp_notifyQueueInitialized = 0;
    }

    return ret;
}

int
ngisProtocolSendSuccessReply(
    ngisProtocol_t *protocol,
    ngisWriteStringCallbackFunc_t func)
{
    ngisCallback_t callback;
    static const char fName[] = "ngisProtocolSendSuccessReply";

    NGIS_ASSERT(protocol != NULL);

    callback = ngisCallbackWriteFormat(
        protocol->ngp_stdio.ngsio_out, func, protocol,
        "S%s",NGISL_PROTOCOL_LINE_SEPARATOR);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(NULL, fName,
            "Can't register function for sending reply.\n");
        return 0;
    }
    return 1;
}

/**
 * Send Status Success Reply
 */
int
ngisProtocolSendStatusSuccessReply(
    ngisProtocol_t *protocol,
    ngisWriteStringCallbackFunc_t func,
    ngisJobStatus_t status)
{
    ngisCallback_t callback;
    static const char fName[] = "ngisProtocolSendStatusSuccessReply";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT((NGIS_STATUS_PENDING <= status) &&
           (NGIS_STATUS_FAILED  >= status));

    callback = ngisCallbackWriteFormat(
        protocol->ngp_stdio.ngsio_out, func, protocol, "S %s%s",
        ngisJobStatusStrings[status], NGISL_PROTOCOL_LINE_SEPARATOR);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(NULL, fName,
            "Can't register function for sending reply.\n");
        return 0;
    }
    return 1;
}

/**
 * Protocol: Send Failure Reply
 */
int
ngisProtocolSendFailureReply(
    ngisProtocol_t *protocol,
    ngisWriteStringCallbackFunc_t func,
    char *message)
{
    ngisCallback_t callback;
    static const char fName[] = "ngisProtocolSendFailureReply";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(message != NULL);
           
    callback = ngisCallbackWriteFormat(
        protocol->ngp_stdio.ngsio_out, func, protocol, "F %s%s",
        message, NGISL_PROTOCOL_LINE_SEPARATOR);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(NULL, fName,
            "Can't register function for sending reply.\n");
        return 0;
    }
    return 1;
}

/**
 * Protocol: Send Create Notify
 */
int
ngisProtocolSendCreateNotify(
    ngisProtocol_t *protocol,
    int successful,
    char *requestID,
    char *string)
{
    char *createResult;
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisProtocolSendCreateNotify";
    
    NGIS_ASSERT(protocol  != NULL);
    NGIS_ASSERT(requestID != NULL);
    NGIS_ASSERT(string    != NULL);

    log = protocol->ngp_log;
    
    if (successful != 0 ){
        createResult = "S";
    } else {
        createResult = "F";
    }

    result = ngislProtocolSendNotify(protocol, "%s %s %s %s%s",
        NGIS_CREATE_NOTIFY, requestID, createResult, string,
        NGISL_PROTOCOL_LINE_SEPARATOR);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for sending notify.\n");
        return 0;
    }
    return 1;
}

/**
 * Protocol: Send Status Notify
 */
int
ngisProtocolSendStatusNotify(
    ngisProtocol_t *protocol,
    char *jobID,
    ngisJobStatus_t status,
    char *string)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisProtocolSendStatusNotify";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT(jobID    != NULL);
    NGIS_ASSERT(string   != NULL);
    NGIS_ASSERT((NGIS_STATUS_PENDING <= status) &&
                (NGIS_STATUS_FAILED  >= status));
    log = protocol->ngp_log;

    result = ngislProtocolSendNotify(protocol, "%s %s %s %s%s",
        NGIS_STATUS_NOTIFY, jobID, ngisJobStatusStrings[status], 
        string, NGISL_PROTOCOL_LINE_SEPARATOR);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't register function for writing.\n");
        return 0;
    }
    return 1;
}

static void
ngislProtocolExitReplyCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislProtocolExitReplyCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_FD(fd);

    log = protocol->ngp_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't send \"Exit\" reply.\n");
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisErrorPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    ngisDebugPrint(log, fName, "Disables the protocol.\n");
    result = ngisProtocolDisable(protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disables the protocol.\n");
    }
        
    return;
}

static void
ngislProtocolReplyCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislProtocolReplyCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_FD(fd);
    
    log = protocol->ngp_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't send reply.\n");
        goto protocol_end;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisErrorPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Next Request */
    result = ngisLineBufferReadLine(protocol->ngp_lineBuffer,
        ngislProtocolRequestCallback, (void *)protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't register for reading a line.\n");
        goto protocol_end;
    }
    return;

protocol_end:
    ngisDebugPrint(log, fName, "Disables the protocol.\n");
    result = ngisProtocolDisable(protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disables the protocol.\n");
    }
    return;
}

static int
ngislProtocolSendNotify(
    ngisProtocol_t *protocol,
    char *format,
    ...)
{
    ngisCallback_t callback;
    va_list ap;
    int ret = 0;
    ngisLog_t *log;
    int result;
    static const char fName[] = "ngislProtocolSendNotify";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_STRING(format);

    log = protocol->ngp_log;

    va_start(ap, format);

    if (ngisCallbackIsValid(protocol->ngp_notifyCallback)) {
        /* Function already registered */
        if (protocol->ngp_notifyQueueInitialized == 0) {
            result = ngisStringBufferInitialize(
                &protocol->ngp_notifyQueue);
            if (result == 0) {
                ngisErrorPrint(log, fName,
                    "Can't initialize string buffer.\n");
                goto finalize;
            }
            protocol->ngp_notifyQueueInitialized = 1;
        }
        result = ngisStringBufferVformat(
            &protocol->ngp_notifyQueue, format, ap);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't append string to string buffer.\n");
            goto finalize;
        }
    } else {
        callback = ngisCallbackWriteVformat(
            protocol->ngp_stdio.ngsio_error, ngislProtocolNotifyCallback,
            protocol, format, ap);
        if (!ngisCallbackIsValid(callback)) {
            ngisErrorPrint(log, fName,
                "Can't register function for sending notify.\n");
            goto finalize;
        }
        protocol->ngp_notifyCallback = callback;
    }

    ret = 1;
finalize:
    va_end(ap);

    if (ret == 0) {
        result = ngisProtocolDisable(protocol);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't disable the protocol.\n");
        }
    }

    return ret;
}

static void
ngislProtocolNotifyCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisProtocol_t *protocol = arg;
    int result;
    ngisLog_t *log;
    char *string = NULL;
    static const char fName[] = "ngislProtocolNotifyCallback";

    NGIS_ASSERT(protocol != NULL);
    NGIS_ASSERT_FD(fd);
    
    log = protocol->ngp_log;
    protocol->ngp_notifyCallback = NULL;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't send notify.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisErrorPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    if (protocol->ngp_notifyQueueInitialized != 0) {
        string = ngisStringBufferRelease(&protocol->ngp_notifyQueue);
        ngisStringBufferFinalize(&protocol->ngp_notifyQueue);
        protocol->ngp_notifyQueueInitialized = 0;
        if (!NGIS_STRING_IS_NONZERO(string)) {
            ngisErrorPrint(log, fName, "Can't get string.\n");
            goto error;
        }

        result = ngislProtocolSendNotify(protocol, "%s", string);
        free(string);
        string = NULL;
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for sending notify.\n");
            goto error;
        }
    }

    return;
error:
    ngisDebugPrint(log, fName, "Disables the protocol.\n");
    result = ngisProtocolDisable(protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disables the protocol.\n");
    }
    return;
}
