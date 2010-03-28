/*
 * $RCSfile: ngisRequest.c,v $ $Revision: 1.13 $ $Date: 2008/02/26 06:32:39 $
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

/**
 * Request for Invoke Server
 */

#include "ngEnvironment.h"
#ifdef NG_PTHREAD

#include "ngInvokeServer.h"

NGI_RCSID_EMBED("$RCSfile: ngisRequest.c,v $ $Revision: 1.13 $ $Date: 2008/02/26 06:32:39 $")

/**
 * Prototype
 */
static void ngislRequestReaderInitializeMember(ngisiRequestReader_t *);
static int ngislRequestReaderStart(ngisiContext_t *, int *);
static int ngislRequestReaderStopCheck(ngisiContext_t *, int *);
static void *ngislRequestReaderThread(void *);
static int ngislRequestReaderStatusSet(
    ngisiContext_t *, ngisiRequestReaderStatus_t, int *);
static int ngislRequestReaderProcess(ngisiContext_t *, int *);

static int ngislRequestRead(
    ngisiContext_t *, ngisiRequestType_t *, char **, char **,
    ngisiCreateAttr_t **, int *);
static int ngislRequestAttributesRead(
    ngisiContext_t *, FILE *, ngisiReadBuffer_t *,
    ngisiCreateAttr_t **, int *);
static void ngislCheckAttribute(ngisiContext_t *context, char *attr);
static int ngislRequestProcess(
    ngisiContext_t *, ngisiRequestType_t, char *, char *,
    ngisiCreateAttr_t *, int *, int *);
static int ngislRequestProcessJobCreate(
    ngisiContext_t *, char *, char *, ngisiCreateAttr_t *, int *);
static int ngislRequestProcessJobStatus(
    ngisiContext_t *, char *, char *, int *);
static int ngislRequestProcessJobDestroy(
    ngisiContext_t *, char *, char *, int *);
static int ngislRequestProcessQueryFeatures(
    ngisiContext_t *, char *, int *);
static int ngislRequestProcessExit(
    ngisiContext_t *, char *, int *);
static int ngislRequestProcessClosed(ngisiContext_t *, int *);
static int ngislRequestProcessUnknown(ngisiContext_t *, int *);
static int ngislRequestProcessDefault(ngisiContext_t *, int *);

static int ngislCreateAttrInitialize(
    ngisiContext_t *, ngisiCreateAttr_t *, int *);
static int ngislCreateAttrFinalize(
    ngisiContext_t *, ngisiCreateAttr_t *, int *);
static void ngislCreateAttrInitializeMember(
    ngisiCreateAttr_t *);
static int ngislCreateAttrAdd(
    ngisiContext_t *, ngisiCreateAttr_t *, char *, char *, int *);

static int ngislReply(
    ngisiContext_t *, ngisiReplyResult_t, char *, int *);
static int ngislReplyStatus(
    ngisiContext_t *, ngisiReplyResult_t, ngisiJobStatus_t, char *, int *);
static int ngislNotifyJobCreate(
    ngisiContext_t *, char *, ngisiReplyResult_t, char *, char *, int *);
static int ngislReplyNotifyJobStatusGetString(
    ngisiContext_t *, ngisiJobStatus_t, char **, int *);

/**
 * Data
 */
static const ngisiRequestTypeTable_t ngislRequestTypeTable[] = {
    {1, NGISI_REQUEST_JOB_CREATE,     "JOB_CREATE"},
    {1, NGISI_REQUEST_JOB_STATUS,     "JOB_STATUS"},
    {1, NGISI_REQUEST_JOB_DESTROY,    "JOB_DESTROY"},
    {1, NGISI_REQUEST_QUERY_FEATURES, "QUERY_FEATURES"},
    {1, NGISI_REQUEST_EXIT,           "EXIT"},
    {0, NGISI_REQUEST_NOMORE,         NULL},
};

/**
 * Request Reader Initialize
 */
int
ngisiRequestReaderInitialize(
    ngisiContext_t *context,
    ngisiRequestReader_t *requestReader,
    int *error)
{
    static const char fName[] = "ngisiRequestReaderInitialize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(requestReader != NULL);

    ngislRequestReaderInitializeMember(requestReader);

    result = ngisiMutexInitialize(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the mutex failed.\n");
        return 0;
    }
    requestReader->ngisr_mutexInitialized = 1;

    result = ngisiCondInitialize(
        context, &requestReader->ngisr_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the cond failed.\n");
        return 0;
    }
    requestReader->ngisr_condInitialized = 1;

    result = ngisiReadBufferInitialize(
        context, &requestReader->ngisr_readBuffer, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Read Buffer failed.\n");
        return 0;
    }

    result = ngislRequestReaderStatusSet(
        context, NGISI_REQUEST_READER_STATUS_INITIALIZED, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Set the Request Reader Status failed.\n");
        return 0;
    }

    result = ngislRequestReaderStart(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Start the Request Reader failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request Reader Finalize
 */
int
ngisiRequestReaderFinalize(
    ngisiContext_t *context,
    ngisiRequestReader_t *requestReader,
    int *error)
{
    static const char fName[] = "ngisiRequestReaderFinalize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(requestReader != NULL);

    result = ngislRequestReaderStopCheck(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Stop check failed.\n");
        return 0;
    }

    result = ngisiReadBufferFinalize(
        context, &requestReader->ngisr_readBuffer, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Read Buffer failed.\n");
        return 0;
    }

    if (requestReader->ngisr_condInitialized != 0) {
        requestReader->ngisr_condInitialized = 0;
        result = ngisiCondFinalize(
            context, &requestReader->ngisr_cond, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Finalize the cond failed.\n");
            return 0;
        }
    }

    if (requestReader->ngisr_mutexInitialized != 0) {
        requestReader->ngisr_mutexInitialized = 0;
        result = ngisiMutexFinalize(
            context, &requestReader->ngisr_mutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Finalize the mutex failed.\n");
            return 0;
        }
    }

    ngislRequestReaderInitializeMember(requestReader);

    /* Success */
    return 1;
}

/**
 * Request Reader Initialize Member
 */
static void
ngislRequestReaderInitializeMember(
    ngisiRequestReader_t *requestReader)
{
    requestReader->ngisr_continue = 0;
    requestReader->ngisr_working = 0;
    requestReader->ngisr_mutexInitialized = 0;
    requestReader->ngisr_condInitialized = 0;
    requestReader->ngisr_status = NGISI_REQUEST_READER_STATUS_UNDEFINED;
}

/**
 * Request Reader Start
 */
static int
ngislRequestReaderStart(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestReaderStart";
    ngisiRequestReader_t *requestReader;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    requestReader = &context->ngisc_requestReader;

    requestReader->ngisr_continue = 1;
    requestReader->ngisr_working = 1;

    /* Create the Request Reader thread */
    result = globus_thread_create(
        &requestReader->ngisr_thread, NULL,
        ngislRequestReaderThread, context);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Create the Request Reader thread failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request Reader Stop Check
 */
static int
ngislRequestReaderStopCheck(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestReaderStopCheck";

    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Request Reader Stop check not implemented.\n");

    /* Success */
    return 1;
}

/**
 * Request Reader Status Set
 */
static int
ngislRequestReaderStatusSet(
    ngisiContext_t *context,
    ngisiRequestReaderStatus_t status,
    int *error)
{
    static const char fName[] = "ngislRequestReaderStatusSet";
    ngisiRequestReader_t *requestReader;
    int result, locked;

    /* Check the arguments */
    assert(context != NULL);

    requestReader = &context->ngisc_requestReader;
    locked = 0;

    /* Lock the mutex */
    result = ngisiMutexLock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Request Reader mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Update the status */
    requestReader->ngisr_status = status;

    /* Signal */
    result = ngisiCondBroadcast(
        context, &requestReader->ngisr_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Cond Signal the Request Reader failed.\n");
        goto error;
    }

    /* Unlock the mutex */
    locked = 0;
    result = ngisiMutexUnlock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Request Reader mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        locked = 0;
        result = ngisiMutexUnlock(
            context, &requestReader->ngisr_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Request Reader mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}
    
/**
 * Request Reader Set Done
 */
int
ngisiRequestReaderSetDone(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiRequestReaderSetDone";
    ngisiRequestReader_t *requestReader;
    int result, locked;

    /* Check the arguments */
    assert(context != NULL);

    requestReader = &context->ngisc_requestReader;
    locked = 0;

    /* Lock the mutex */
    result = ngisiMutexLock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Request Reader mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Set done */
    requestReader->ngisr_working = 0;

    result = ngisiCondBroadcast(context,
        &requestReader->ngisr_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Broadcast the cond failed.\n");
        goto error;
    }

    /* Unlock the mutex */
    locked = 0;
    result = ngisiMutexUnlock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Request Reader mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        locked = 0;
        result = ngisiMutexUnlock(
            context, &requestReader->ngisr_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Request Reader mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Request Reader Wait Done
 */
int
ngisiRequestReaderWaitDone(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiRequestReaderWaitDone";
    ngisiRequestReader_t *requestReader;
    int result, locked;

    /* Check the arguments */
    assert(context != NULL);

    requestReader = &context->ngisc_requestReader;
    locked = 0;

    /* Lock the mutex */
    result = ngisiMutexLock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Request Reader mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Wait done */
    while ((requestReader->ngisr_status < NGISI_REQUEST_READER_STATUS_DONE)
        && (requestReader->ngisr_working != 0)) {
        result = ngisiCondWait(context,
            &requestReader->ngisr_cond,
            &requestReader->ngisr_mutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Cond Wait the Request Reader failed.\n");
            goto error;
        }
    }

    /* Unlock the mutex */
    locked = 0;
    result = ngisiMutexUnlock(
        context, &requestReader->ngisr_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Request Reader mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        locked = 0;
        result = ngisiMutexUnlock(
            context, &requestReader->ngisr_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Request Reader mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Request Reader Thread
 */
static void *
ngislRequestReaderThread(
    void *threadArgument)
{
    static const char fName[] = "ngislRequestReaderThread";
    ngisiRequestReader_t *requestReader;
    ngisiContext_t *context;
    int *error, errorEntity;
    int result;

    assert(threadArgument != NULL);

    context = (ngisiContext_t *)threadArgument;
    requestReader = &context->ngisc_requestReader;
    error = &errorEntity;

    /* Check if the flag is valid */
    assert(requestReader->ngisr_continue != 0);

    requestReader->ngisr_working = 1;

    /* Process Request Reader */
    result = ngislRequestReaderProcess(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Process the Request Reader failed.\n");
    }

    result = ngisiRequestReaderSetDone(context, error);
    requestReader->ngisr_working = 0;
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Set done to the Request Reader failed.\n");
    }

    /* Success */
    return NULL;
}

/**
 * Request Reader Process
 */
static int
ngislRequestReaderProcess(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestReaderProcess";
    ngisiRequestType_t request;
    ngisiCreateAttr_t *attrs;
    char *requestName, *id;
    int result, finish;

    /* Check the arguments */
    assert(context != NULL);

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Request Reader started working.\n");

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Request Reader pid=%ld.\n", (long)getpid());

    do {
        requestName = NULL;
        id = NULL;
        attrs = NULL;
        finish = 0;

        result = ngislRequestRead(
            context, &request, &requestName, &id, &attrs, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Read the Request failed.\n");
            goto error;
        }

        finish = 0;
        result = ngislRequestProcess(
            context, request, requestName, id, attrs, &finish, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process the Request failed.\n");
            /* Return to loop */
        }

        requestName = NULL;

        if (id != NULL) {
            globus_libc_free(id);
            id = NULL;
        }

        if (attrs != NULL) {
            result = ngislCreateAttrFinalize(context, attrs, error);
            if (result == 0) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "Finalize the Create Attr failed.\n");
                goto error;
            }
            globus_libc_free(attrs);

            attrs = NULL;
        }

    } while (finish == 0);

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Request Reader thread exiting.\n");

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
        "Request Reader thread exiting by error.\n");

    /* Failed */
    return 0;
}

/**
 * Request Read
 */
static int
ngislRequestRead(
    ngisiContext_t *context,
    ngisiRequestType_t *request,
    char **requestName,
    char **id,
    ngisiCreateAttr_t **attrs,
    int *error)
{
    static const char fName[] = "ngislRequestRead";
    char idBuf[NGISI_ID_STR_MAX], *requestStr, *returnIdStr, *p;
    ngisiReadBuffer_t *readBuffer;
    int result, found, i, idCur;

    /* Check the arguments */
    assert(context != NULL);
    assert(request != NULL);
    assert(requestName != NULL);
    assert(id != NULL);
    assert(attrs != NULL);

    *request = NGISI_REQUEST_UNDEFINED;
    *requestName = NULL;
    *id = NULL;
    *attrs = NULL;

    p = NULL;
    requestStr = NULL;

    readBuffer = &context->ngisc_requestReader.ngisr_readBuffer;

    result = ngisiReadLine(
        context, NGISI_FP_REQUEST, readBuffer, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Read the Line failed.\n");
        return 0;
    }

    if (readBuffer->ngisrb_buf == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Read buffer is NULL.\n");
        return 0;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Read Request \"%s\".\n", readBuffer->ngisrb_buf);

    found = 0;
    for (i = 0; ngislRequestTypeTable[i].ngisrt_valid != 0; i++) {
        if (strncmp(readBuffer->ngisrb_buf,
            ngislRequestTypeTable[i].ngisrt_name,
            strlen(ngislRequestTypeTable[i].ngisrt_name)) == 0) {

            *request = ngislRequestTypeTable[i].ngisrt_type;
            requestStr = ngislRequestTypeTable[i].ngisrt_name;
            *requestName = requestStr;

            found = 1;
            break;
        }
    }

    if (found == 0) {
        if (readBuffer->ngisrb_reachEOF != 0) {
            *request = NGISI_REQUEST_CLOSED;
        } else {
            *request = NGISI_REQUEST_UNKNOWN;
        }

        return 1;
    }

    assert(found != 0);
    if ((*request == NGISI_REQUEST_EXIT) ||
        (*request == NGISI_REQUEST_QUERY_FEATURES)) {
        return 1;
    }

    p = readBuffer->ngisrb_buf;
    p += strlen(requestStr);

    /* Skip space */
    while (isspace((int)*p)) {
        p++;
    }

    /* Get the ID */
    idCur = 0;
    while (isgraph((int)*p)) {
        idBuf[idCur] = *p;
        idCur++;
        p++;
        if (idCur + 1 >= NGISI_ID_STR_MAX) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "id overflow (%d chars).\n", NGISI_ID_STR_MAX);
            return 0;
        }
    }
    idBuf[idCur] = '\0';

    if (idCur > 0) {
        returnIdStr = strdup(idBuf);
        if (returnIdStr == NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "strdup(ID str) failed.\n");
            return 0;
        }
        *id = returnIdStr;
    } else {
        *id = NULL;
    }

    if (*request == NGISI_REQUEST_JOB_CREATE) {

        result = ngislRequestAttributesRead(
            context, NGISI_FP_REQUEST, readBuffer, attrs, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Read the Request Attributes failed.\n");
            return 0;
        }
        
    }

    /* Success */
    return 1;
}

/**
 * Request Attributes Read
 */
static int
ngislRequestAttributesRead(
    ngisiContext_t *context,
    FILE *readFp,
    ngisiReadBuffer_t *readBuffer,
    ngisiCreateAttr_t **attrs,
    int *error)
{
    static const char fName[] = "ngislRequestAttributesRead";
    char attrNameBuf[NGISI_CREATE_ATTR_STR_MAX];
    ngisiCreateAttr_t *newAttr;
    int result, finish, cur;
    char *p, *strStart;

    /* Check the arguments */
    assert(context != NULL);
    assert(readFp != NULL);
    assert(readBuffer != NULL);
    assert(attrs != NULL);

    *attrs = NULL;

    newAttr = (ngisiCreateAttr_t *)globus_libc_calloc(
        1, sizeof(ngisiCreateAttr_t));
    if (newAttr == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the Create Attr failed.\n");
        return 0;
    }

    result = ngislCreateAttrInitialize(context, newAttr, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Create Attr failed.\n");
        return 0;
    }

    *attrs = newAttr;

    finish = 0;
    do {
        result = ngisiReadLine(
            context, NGISI_FP_REQUEST, readBuffer, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Read line failed.\n");
            return 0;
        }

        /* log */
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Read Attr \"%s\".\n", readBuffer->ngisrb_buf);


        strStart = readBuffer->ngisrb_buf;
        p = strStart;
        cur = 0;

        /* Get attribute name */
        while (isgraph((int)*p)) {
            attrNameBuf[cur] = *p;
            p++;
            cur++;
        }
        attrNameBuf[cur] = '\0';
        cur = 0;
     
        if (strlen(attrNameBuf) == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Attr name is 0.\n");
            return 0;
        }

        /* Find the value */
        while (isspace((int)*p)) {
            p++;
        }

        if (strcmp(attrNameBuf, NGISI_CREATE_ATTR_END) == 0) {
            finish = 1;
        } else {
            ngislCheckAttribute(context, attrNameBuf);
            result = ngislCreateAttrAdd(
                context, newAttr, attrNameBuf, p, error);
            if (result == 0) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "Add the Create Attr failed.\n");
                return 0;
            }
        }

    } while (finish == 0);

    /* Success */
    return 1;
}

/**
 * Check the attribute.
 */
typedef struct ngislAttributeTable_s {
    char *attr;
    int supported;    /* 0: Unsupported, 1: Supported */
} ngislAttributeTable_t;

ngislAttributeTable_t ngislAttributeTable[] = {
    {"hostname",           1},
    {"port",               1},
    {"jobmanager",         1},
    {"subject",            1},
    {"client_name",        1},
    {"executable_path",    1},
    {"backend",            1},
    {"count",              1},
    {"staging",            1},
    {"argument",           1},
    {"work_directory",     1},
    {"redirect_enable",    1},
    {"stdout_file",        1},
    {"stderr_file",        1},
    {"environment",        1},
    {"tmp_dir",            1},
    {"status_polling",     0},
    {"refresh_credential", 1},
    {"max_time",           1},
    {"max_wall_time",      1},
    {"max_cpu_time",       1},
    {"queue_name",         1},
    {"project",            1},
    {"host_count",         1},
    {"min_memory",         1},
    {"max_memory",         1},
    {"rsl_extensions",     1},
    {NULL},
};

static void
ngislCheckAttribute(
    ngisiContext_t *context,
    char *attr)
{
    int i;
    static const char fName[] = "ngislCheckAttribute";

    for (i = 0; ngislAttributeTable[i].attr != NULL; i++) {
        if (strcmp(attr, ngislAttributeTable[i].attr) == 0) {
            break;
        }
    }

    if (ngislAttributeTable[i].attr == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
            "Unknown attribute \"%s\".\n", attr);
    } else if (ngislAttributeTable[i].supported == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
            "Unsupported attribute \"%s\".\n", attr);
    }
}

/**
 * Request Process
 */
static int
ngislRequestProcess(
    ngisiContext_t *context,
    ngisiRequestType_t request,
    char *requestName,
    char *id,
    ngisiCreateAttr_t *attrs,
    int *finish,
    int *error)
{
    static const char fName[] = "ngislRequestProcess";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(finish != NULL);

    *finish = 0;

    switch(request) {
    case NGISI_REQUEST_JOB_CREATE:
        result = ngislRequestProcessJobCreate(
            context, requestName, id, attrs, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Job Create failed.\n");
            return 0;
        }
        break;

    case NGISI_REQUEST_JOB_STATUS:
        result = ngislRequestProcessJobStatus(
            context, requestName, id, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Job Status failed.\n");
            return 0;
        }
        break;

    case NGISI_REQUEST_JOB_DESTROY:
        result = ngislRequestProcessJobDestroy(
            context, requestName, id, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Job Destroy failed.\n");
            return 0;
        }
        break;

    case NGISI_REQUEST_QUERY_FEATURES:
        result = ngislRequestProcessQueryFeatures(
            context, requestName, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Query Features failed.\n");
            return 0;
        }
        break;

    case NGISI_REQUEST_EXIT:
        result = ngislRequestProcessExit(
            context, requestName, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Exit failed.\n");
            return 0;
        }
        *finish = 1;

        break;

    case NGISI_REQUEST_CLOSED:
        result = ngislRequestProcessClosed(context, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Connection closed failed.\n");
            return 0;
        }
        *finish = 1;

        break;

    case NGISI_REQUEST_UNKNOWN:
        result = ngislRequestProcessUnknown(context, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Unknown failed.\n");
            return 0;
        }

        break;

    default:
        result = ngislRequestProcessDefault(context, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Process Default failed.\n");
            return 0;
        }
        break;
    }

    /* Success */
    return 1;
}

/**
 * Request Process JOB_CREATE
 */
static int
ngislRequestProcessJobCreate(
    ngisiContext_t *context,
    char *requestName,
    char *id,
    ngisiCreateAttr_t *attrs,
    int *error)
{
    static const char fName[] = "ngislRequestProcessJobCreate";
    char *requestID, *jobID;
    int result, notifyDone;
    ngisiJob_t *job;

    requestID = id;
    job = NULL;
    jobID = NULL;
    notifyDone = 0;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process %s.\n", requestName);

    result = ngislReply(context, NGISI_REPLY_SUCCESS, NULL, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Reply failed.\n");
        return 0;
    }

    job = ngisiJobConstruct(context, requestID, attrs, error);
    if (job == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Job Consturct failed.\n");
        goto error;
    }

    result = ngisiJobGetJobID(context, job, &jobID, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Get the JobID failed.\n");
        goto error;
    }

    notifyDone = 1;
    result = ngislNotifyJobCreate(
        context, requestID, NGISI_REPLY_SUCCESS,
        jobID, NULL, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Notify the Job Create failed.\n");
        goto error;
    }

    result = ngisiJobStart(context, job, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Start the Job failed.\n");
        goto error;
    }

    assert(jobID != NULL);
    globus_libc_free(jobID);
    jobID = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (notifyDone == 0) {
        result = ngislNotifyJobCreate(
            context, requestID, NGISI_REPLY_FAIL,
            NULL, NULL, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Notify the Job Create failed.\n");
        }
    }
 
    /* Failed */
    return 0;
}

/**
 * Request Process JOB_STATUS
 */
static int
ngislRequestProcessJobStatus(
    ngisiContext_t *context,
    char *requestName,
    char *id,
    int *error)
{
    static const char fName[] = "ngislRequestProcessJobStatus";
    ngisiJobStatus_t status;
    int result;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process %s.\n", requestName);

    result = ngisiJobProtocolStatus(context, id, &status, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Process Job Status failed.\n");
        return 0;
    }

    result = ngislReplyStatus(
        context, NGISI_REPLY_SUCCESS, status, NULL, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Reply Job Status failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request Process JOB_DESTROY
 */
static int
ngislRequestProcessJobDestroy(
    ngisiContext_t *context,
    char *requestName,
    char *id,
    int *error)
{
    static const char fName[] = "ngislRequestProcessJobDestroy";
    int result;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process %s.\n", requestName);

    result = ngisiJobProtocolDestroy(context, id, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Job Destroy process failed.\n");
        return 0;
    }

    result = ngislReply(context, NGISI_REPLY_SUCCESS, NULL, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Reply Job Destroy failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request Process QUERY_FEATURES
 */
static int
ngislRequestProcessQueryFeatures(
    ngisiContext_t *context,
    char *requestName,
    int *error)
{
    int i;

    fprintf(NGISI_FP_REPLY, "SM%s",
        NGISI_LINE_TERMINATOR_STR);
    fprintf(NGISI_FP_REPLY, "protocol_version 2.0%s",
        NGISI_LINE_TERMINATOR_STR);

    for (i = 0; ngislRequestTypeTable[i].ngisrt_valid != 0; i++) {
        fprintf(NGISI_FP_REPLY, "request %s%s",
            ngislRequestTypeTable[i].ngisrt_name,
            NGISI_LINE_TERMINATOR_STR);
    }
    fprintf(NGISI_FP_REPLY, "REPLY_END%s", NGISI_LINE_TERMINATOR_STR);

    /* Success */
    return 1;
}

/**
 * Request Process EXIT
 */
static int
ngislRequestProcessExit(
    ngisiContext_t *context,
    char *requestName,
    int *error)
{
    static const char fName[] = "ngislRequestProcessExit";
    int result;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process %s.\n", requestName);

    result = ngislReply(context, NGISI_REPLY_SUCCESS, NULL, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Reply Exit failed.\n");
        /* Not return */
    }

    result = ngislRequestReaderStatusSet(
        context, NGISI_REQUEST_READER_STATUS_DONE, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Set the Request Reader status failed.\n");
    }

    /* Success */
    return 1;
}

/**
 * Request Process Connection Closed
 */
static int
ngislRequestProcessClosed(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestProcessClosed";
    int result;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process Connection Closed.\n");

    result = ngislRequestReaderStatusSet(
        context, NGISI_REQUEST_READER_STATUS_DONE, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Set the Request Reader status failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request Process Unkown Request
 */
static int
ngislRequestProcessUnknown(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestProcessUnknown";

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process Unknown.\n");

    /* Success */
    return 1;
}

/**
 * Request Process Unknown2
 */
static int
ngislRequestProcessDefault(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislRequestProcessDefault";

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Process Unknown.\n");

    /* Success */
    return 1;
}

/**
 * Create Attr Initialize
 */
static int
ngislCreateAttrInitialize(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);

    ngislCreateAttrInitializeMember(attr);

    attr->ngisca_nAttrs = 0;

    /* Success */
    return 1;
}

/**
 * Create Attr Finalize
 */
static int
ngislCreateAttrFinalize(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    int *error)
{
    int i;

    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);

    if (attr->ngisca_nAttrs > 0) {
        assert(attr->ngisca_attrs != NULL);

        for (i = 0; i < attr->ngisca_nAttrs; i++) {
            assert(attr->ngisca_attrs[i].ngisce_name != NULL);
            globus_libc_free(attr->ngisca_attrs[i].ngisce_name);
            attr->ngisca_attrs[i].ngisce_name = NULL;

            if (attr->ngisca_attrs[i].ngisce_value != NULL) {
                globus_libc_free(attr->ngisca_attrs[i].ngisce_value);
            }
            attr->ngisca_attrs[i].ngisce_value = NULL;
        }

        globus_libc_free(attr->ngisca_attrs);
        attr->ngisca_attrs = NULL;
    }

    ngislCreateAttrInitializeMember(attr);

    /* Success */
    return 1;
}

/**
 * Create Attr Initialize Member
 */
static void
ngislCreateAttrInitializeMember(
    ngisiCreateAttr_t *attr)
{
    /* Check the arguments */
    assert(attr != NULL);

    attr->ngisca_attrs = NULL;
    attr->ngisca_nAttrs = 0;
}

/**
 * Create Attr Add
 */
static int
ngislCreateAttrAdd(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    char *name,
    char *value,
    int *error)
{
    static const char fName[] = "ngislCreateAttrAdd";
    ngisiCreateAttrElement_t *newArray, *oldArray;
    int i, newArraySize;
    char *newName, *newValue;

    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);
    assert(name != NULL);

    /* Check and renew size */
    if (attr->ngisca_nAttrs >= attr->ngisca_arraySize) {
        oldArray = attr->ngisca_attrs;

        if (attr->ngisca_arraySize <= 0) {
            newArraySize = NGISI_CREATE_ATTR_INITIAL_SIZE;
        } else {
            newArraySize = attr->ngisca_arraySize * 2;
        }

        newArray = (ngisiCreateAttrElement_t *)globus_libc_calloc(
            newArraySize, sizeof(ngisiCreateAttrElement_t));
        if (newArray == NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Allocate the Create Attr failed.\n");
            return 0;
        }

        for (i = 0; i < newArraySize; i++) {
            newArray[i].ngisce_name    = NULL;
            newArray[i].ngisce_value   = NULL;
            newArray[i].ngisce_treated = 0;
        }

        for (i = 0; i < attr->ngisca_arraySize; i++) {
            newArray[i].ngisce_name    = oldArray[i].ngisce_name;
            newArray[i].ngisce_value   = oldArray[i].ngisce_value;
            newArray[i].ngisce_treated = oldArray[i].ngisce_treated;
        }

        attr->ngisca_arraySize = newArraySize;
        attr->ngisca_attrs = newArray;

        if (oldArray != NULL) {
            globus_libc_free(oldArray);
        }
    }
    assert(attr->ngisca_nAttrs < attr->ngisca_arraySize);

    /* Add to tail */
    i = attr->ngisca_nAttrs;

    newName = strdup(name);
    if (newName == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(new name) failed.\n");
        return 0;
    }

    if (value != NULL) {
        newValue = strdup(value);
        if (newValue == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(new value) failed.\n");
        return 0;
        }
    } else {
        newValue = NULL;
    }

    attr->ngisca_attrs[i].ngisce_name = newName;
    attr->ngisca_attrs[i].ngisce_value = newValue;

    attr->ngisca_nAttrs++;
    
    /* Success */
    return 1;
}

/**
 * Create Attr Get Count
 */
int
ngisiCreateAttrGetCount(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    char *name,
    int *count,
    int *error)
{
    int i, nAttrs;

    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);
    assert(name != NULL);
    assert(count != NULL);

    *count = 0;
    nAttrs = 0;

    /* Count the attribute */
    for (i = 0; i < attr->ngisca_nAttrs; i++) {
        assert(attr->ngisca_attrs[i].ngisce_name != NULL);
        if (strcmp(name, attr->ngisca_attrs[i].ngisce_name) == 0) {

            nAttrs++;
        }
    }

    *count = nAttrs;

    /* Success */
    return 1;
}

/**
 * Create Attr Get
 */
int
ngisiCreateAttrGet(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    char *name,
    int count,
    char **retValue,
    int *error)
{
    int i, matched, found;

    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);
    assert(name != NULL);
    assert(count >= 0);
    assert(retValue != NULL);

    *retValue = NULL;

    /* Find the attribute */
    matched = 0;
    found = 0;
    for (i = 0; i < attr->ngisca_nAttrs; i++) {
        assert(attr->ngisca_attrs[i].ngisce_name != NULL);
        if (strcmp(name, attr->ngisca_attrs[i].ngisce_name) == 0) {
            if (matched >= count) {

                *retValue = attr->ngisca_attrs[i].ngisce_value;
                attr->ngisca_attrs[i].ngisce_treated = 1;

                found = 1;
                break;
            } else {
                matched++;
            }
        }
    }

    if (found == 0) {
        /* Not found */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Create Attr Get Remain
 * Returns attribute names which is not treated.
 */
int
ngisiCreateAttrGetRemain(
    ngisiContext_t *context,
    ngisiCreateAttr_t *attr,
    int *remainCount,
    char ***remainTable,
    int *error)
{
    static const char fName[] = "ngisiCreateAttrGetRemain";
    int size, i, cur;
    char **newTable, *name; 

    /* Check the arguments */
    assert(context != NULL);
    assert(attr != NULL);
    assert(remainCount != NULL);
    assert(remainTable != NULL);

    *remainCount = 0;
    *remainTable = NULL;

    /* Count the remain */
    size = 0;
    for (i = 0; i < attr->ngisca_nAttrs; i++) {
        assert(attr->ngisca_attrs[i].ngisce_name != NULL);
        if (attr->ngisca_attrs[i].ngisce_treated == 0) {
            size++;
        }
    }

    if (size == 0) {
        *remainCount = 0;
        *remainTable = NULL;

        /* Success */
        return 1;
    }

    newTable = (char **)globus_libc_calloc(size, sizeof(char *));
    if (newTable == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the string table failed.\n");
        return 0;
    }

    cur = 0;
    for (i = 0; i < attr->ngisca_nAttrs; i++) {
        assert(attr->ngisca_attrs[i].ngisce_name != NULL);
        if (attr->ngisca_attrs[i].ngisce_treated == 0) {
            name = strdup(attr->ngisca_attrs[i].ngisce_name);
            if (name == NULL) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "strdup() failed.\n");
                return 0;
            }
            newTable[cur] = name;
            cur++;
            assert(cur <= size);
        }
    }

    *remainCount = size;
    *remainTable = newTable;

    /* Success */
    return 1;
}

/**
 * Reply
 */
static int
ngislReply(
    ngisiContext_t *context,
    ngisiReplyResult_t replyResult,
    char *message,
    int *error)
{
    static const char fName[] = "ngislReply";
    char buf[NGISI_PROTOCOL_MAX];

    /* Check the arguments */
    assert(context != NULL);

    buf[0] = '\0';

    if (replyResult == NGISI_REPLY_SUCCESS) {
        if (message != NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
                "Reply success requires no message : \"%s\".\n", message);
        }

        snprintf(buf, sizeof(buf), "S");

    } else if (replyResult == NGISI_REPLY_FAIL) {
        snprintf(buf, sizeof(buf), "F%s%s",
            ((message != NULL) ? " " : ""),
            ((message != NULL) ? message : ""));

    } else {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Invalid Reply Result %d\n", replyResult);
        return 0;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Send Reply \"%s\".\n", buf);

    fprintf(NGISI_FP_REPLY, "%s%s", buf, NGISI_LINE_TERMINATOR_STR);

    /* Success */
    return 1;
}

/**
 * Reply Status
 */
static int
ngislReplyStatus(
    ngisiContext_t *context,
    ngisiReplyResult_t replyResult,
    ngisiJobStatus_t status,
    char *message,
    int *error)
{
    static const char fName[] = "ngislReplyStatus";
    char buf[NGISI_PROTOCOL_MAX];
    char *statusString;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    statusString = NULL;
    buf[0] = '\0';

    if (replyResult == NGISI_REPLY_SUCCESS) {
        if (message != NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
                "Reply success requires no message : \"%s\".\n", message);
        }

        result = ngislReplyNotifyJobStatusGetString(
            context, status, &statusString, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Get string for status failed\n");
            return 0;
        }
        assert(statusString != NULL);

        snprintf(buf, sizeof(buf), "S %s", statusString);

    } else if (replyResult == NGISI_REPLY_FAIL) {
        snprintf(buf, sizeof(buf), "F%s%s",
            ((message != NULL) ? " " : ""),
            ((message != NULL) ? message : ""));

    } else {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Invalid Reply Result %d\n", replyResult);
        return 0;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Send Reply \"%s\".\n", buf);

    fprintf(NGISI_FP_REPLY, "%s%s", buf, NGISI_LINE_TERMINATOR_STR);

    /* Success */
    return 1;
}

/**
 * Notify Job Create
 */
static int
ngislNotifyJobCreate(
    ngisiContext_t *context,
    char *requestID,
    ngisiReplyResult_t replyResult,
    char *jobID,
    char *message,
    int *error)
{
    static const char fName[] = "ngislNotifyJobCreate";
    char buf[NGISI_PROTOCOL_MAX];
    size_t length;

    /* Check the arguments */
    assert(context != NULL);
    assert(requestID != NULL);

    length = 0;

    length += snprintf(&buf[length], sizeof(buf) - length,
        "CREATE_NOTIFY %s ", requestID);
    
    if (replyResult == NGISI_REPLY_SUCCESS) {
        if (jobID == NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "JobID is NULL.\n");
            return 0;
        }
        if (message != NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
                "Notify success requires no message : \"%s\".\n", message);
        }

        length += snprintf(&buf[length], sizeof(buf) - length,
            "S %s", jobID);

    } else if (replyResult == NGISI_REPLY_FAIL) {
        if (jobID != NULL) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
                "Notify failure requires no jobID : \"%s\".\n", jobID);
        }

        length += snprintf(&buf[length], sizeof(buf) - length,
            "F%s%s",
            ((message != NULL) ? " " : ""),
            ((message != NULL) ? message : ""));

    } else {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Invalid Reply Result %d\n", replyResult);
        return 0;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Send Notify \"%s\".\n", buf);

    fprintf(NGISI_FP_NOTIFY,"%s%s", buf, NGISI_LINE_TERMINATOR_STR);

    /* Success */
    return 1;
}

/**
 * Notify Job Status
 */
int
ngisiNotifyJobStatus(
    ngisiContext_t *context,
    char *jobID,
    ngisiJobStatus_t status,
    char *message,
    int *error)
{
    static const char fName[] = "ngisiNotifyJobStatus";
    char buf[NGISI_PROTOCOL_MAX];
    char *statusString;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobID != NULL);

    statusString = NULL;

    result = ngislReplyNotifyJobStatusGetString(
        context, status, &statusString, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Get string for status failed\n");
        return 0;
    }

    assert(statusString != NULL);

    snprintf(buf, sizeof(buf), 
        "STATUS_NOTIFY %s %s%s%s",
        jobID, statusString,
        (message != NULL ? " " : ""),
        (message != NULL ? message : ""));

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Send Notify \"%s\".\n", buf);

    fprintf(NGISI_FP_NOTIFY,"%s%s", buf, NGISI_LINE_TERMINATOR_STR);

    /* Success */
    return 1;
}

static int
ngislReplyNotifyJobStatusGetString(
    ngisiContext_t *context,
    ngisiJobStatus_t status,
    char **statusString,
    int *error)
{
    static const char fName[] = "ngislReplyNotifyJobStatusGetString";

    /* Check the arguments */
    assert(context != NULL);
    assert(statusString != NULL);

    *statusString = "unknown";

    switch(status) {
    case NGISI_JOB_STATUS_INITIALIZED:
        *statusString = "PENDING"; /* Report as PENDING */
        break;

    case NGISI_JOB_STATUS_PENDING:
        *statusString = "PENDING";
        break;

    case NGISI_JOB_STATUS_ACTIVE:
        *statusString = "ACTIVE";
        break;

    case NGISI_JOB_STATUS_FAILED:
        *statusString = "FAILED";
        break;

    case NGISI_JOB_STATUS_DONE:
        *statusString = "DONE";
        break;

    default:
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unknown Job Status %d.\n", status);
        return 0;
    }
   
    /* Success */
    return 1;
}
    
#endif /* NG_PTHREAD */

