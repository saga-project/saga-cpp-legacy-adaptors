/*
 * $RCSfile: ngclTransferTimeout.c,v $ $Revision: 1.2 $ $Date: 2008/02/06 08:11:50 $
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
 * Module of TransferTimeout for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclTransferTimeout.c,v $ $Revision: 1.2 $ $Date: 2008/02/06 08:11:50 $")

/**
 * Prototype declaration of internal functions.
 */
static int ngcllTransferTimeoutEvent(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllTransferTimeoutEventTimeSet(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllTransferTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, time_t *, int *);
static int ngcllExecutableTransferTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, ngclExecutable_t *, time_t *, int *);
static int ngcllTransferTimeoutCheck(
    ngclContext_t *, int *);
static int ngcllExecutableTransferTimeoutCheck(
    ngclContext_t *, ngclExecutable_t *, int *);

/**
 * TransferTimeout: Initialize
 */
int
ngcliTransferTimeoutInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliTransferTimeoutInitialize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    context->ngc_transferTimeoutHandle = NULL;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Initialize the Transfer Timeout check module.\n"); 

    /* Construct the I/O handle. */
    handle = ngiIOhandleConstruct(context->ngc_event, log, error);
    if (handle == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Construct the I/O handle failed.\n"); 
        return 0;
    }

    context->ngc_transferTimeoutHandle = handle;

    /* Register the Time Event callback. */
    result = ngiIOhandleTimeEventCallbackRegister(
        handle,
        ngcllTransferTimeoutEvent, context,
        ngcllTransferTimeoutEventTimeSet, context,
        log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Register the Time Event to handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Finalize
 */
int
ngcliTransferTimeoutFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliTransferTimeoutFinalize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    handle = context->ngc_transferTimeoutHandle;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Finalize the Transfer Timeout check module.\n"); 

    /* Unregister the Time Event Callback. */
    result = ngiIOhandleTimeEventCallbackUnregister(
        handle, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Unregister the Time Event to handle failed.\n"); 
        return 0;
    }

    /* Destruct the I/O handle. */
    result = ngiIOhandleDestruct(handle, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Destruct the I/O handle failed.\n"); 
        return 0;
    }

    context->ngc_transferTimeoutHandle = NULL;

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Transfer was started.
 *   The timeout measurement for this transfer is also starting.
 */
int
ngcliTransferTimeoutTransferStart(
    ngclSession_t *session,
    ngcliTransferTimeoutType_t type,
    int *error)
{
    static const char fName[] = "ngcliTransferTimeoutTransferStart";
    int timeout, result;
    time_t *timeoutTime;
    ngLog_t *log;

    /* Check the arguments */
    assert(session != NULL);
    assert(type > NGCLI_TRANSFER_TIMEOUT_NONE);
    assert(type < NGCLI_TRANSFER_TIMEOUT_NOMORE);

    log = session->ngs_context->ngc_log;

    timeout = 0;
    timeoutTime = NULL;

    switch (type) {
    case NGCLI_TRANSFER_TIMEOUT_ARGUMENT:
        timeout = session->ngs_transferTimeout_argument;
        timeoutTime = &session->ngs_transferTimeoutTime_argument;
        break;
    case NGCLI_TRANSFER_TIMEOUT_RESULT:
        timeout = session->ngs_transferTimeout_result;
        timeoutTime = &session->ngs_transferTimeoutTime_result;
        break;
    case NGCLI_TRANSFER_TIMEOUT_CB_ARGUMENT:
        timeout = session->ngs_transferTimeout_cbArgument;
        timeoutTime = &session->ngs_transferTimeoutTime_cbArgument;
        break;
    case NGCLI_TRANSFER_TIMEOUT_CB_RESULT:
        timeout = session->ngs_transferTimeout_cbResult;
        timeoutTime = &session->ngs_transferTimeoutTime_cbResult;
        break;
    default:
        abort();
    }
    assert(timeoutTime != NULL);

    /* Reset */
    *timeoutTime = 0;

    if (timeout <= 0) {
        /* TransferTimeout was not used in this session */
        return 1;
    }

    /* Set timeout time for this transfer */
    *timeoutTime = time(NULL) + timeout;

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        session->ngs_context->ngc_transferTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Transfer was done.
 *   The timeout measurement for this transfer is also done.
 */
int
ngcliTransferTimeoutTransferDone(
    ngclSession_t *session,
    ngcliTransferTimeoutType_t type,
    int *error)
{
    static const char fName[] = "ngcliTransferTimeoutTransferDone";
    int timeout, result;
    time_t *timeoutTime;
    ngLog_t *log;

    /* Check the arguments */
    assert(session != NULL);
    assert(type > NGCLI_TRANSFER_TIMEOUT_NONE);
    assert(type < NGCLI_TRANSFER_TIMEOUT_NOMORE);

    log = session->ngs_context->ngc_log;

    switch (type) {
    case NGCLI_TRANSFER_TIMEOUT_ARGUMENT:
        timeout = session->ngs_transferTimeout_argument;
        timeoutTime = &session->ngs_transferTimeoutTime_argument;
        break;
    case NGCLI_TRANSFER_TIMEOUT_RESULT:
        timeout = session->ngs_transferTimeout_result;
        timeoutTime = &session->ngs_transferTimeoutTime_result;
        break;
    case NGCLI_TRANSFER_TIMEOUT_CB_ARGUMENT:
        timeout = session->ngs_transferTimeout_cbArgument;
        timeoutTime = &session->ngs_transferTimeoutTime_cbArgument;
        break;
    case NGCLI_TRANSFER_TIMEOUT_CB_RESULT:
        timeout = session->ngs_transferTimeout_cbResult;
        timeoutTime = &session->ngs_transferTimeoutTime_cbResult;
        break;
    default:
        abort();
    }
    assert(timeoutTime != NULL);

    if (timeout <= 0) {
        /* TransferTimeout was not used in this session */
        return 1;
    }

    /* No need to check anymore */
    *timeoutTime = 0;

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        session->ngs_context->ngc_sessionTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Time event arrived.
 */
static int
ngcllTransferTimeoutEvent(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllTransferTimeoutEvent";
    int *error, errorEntity;
    ngclContext_t *context;
    int result;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngclContext_t *)arg;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (state == NGI_IOHANDLE_STATE_CANCELED) {
        ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "transfer timeout event callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking transfer timeout. May be, one transfer was timeout.\n"); 

    /* Check the transfer timeout */
    result = ngcllTransferTimeoutCheck(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "checking transfer timeout failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Time change event arrived.
 */
static int
ngcllTransferTimeoutEventTimeSet(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllTransferTimeoutEventTimeSet";
    int *error, errorEntity;
    ngclContext_t *context;
    time_t timeoutTime;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngclContext_t *)arg;
    log = context->ngc_log;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (state == NGI_IOHANDLE_STATE_CANCELED) {
        ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "transfer timeout event time change callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    /* Get earliest timeout time */
    result = ngcllTransferTimeoutGetEarliestTimeoutTime(
        context, &timeoutTime, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Getting earliest timeout time failed.\n"); 
        return 0;
    }

    /* Set new timeout time */
    result = ngiIOhandleTimeEventTimeSet(
        context->ngc_transferTimeoutHandle, timeoutTime, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Set the event time failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * TransferTimeout: Get an earliest timeout time
 */
static int
ngcllTransferTimeoutGetEarliestTimeoutTime(
    ngclContext_t *context,
    time_t *timeoutTime,
    int *error)
{
    static const char fName[] = "ngcllTransferTimeoutGetEarliestTimeoutTime";
    ngclExecutable_t *executable;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(timeoutTime != NULL);

    *timeoutTime = 0;

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        return 0;
    }

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, NULL)) != NULL) {

        assert(executable != NULL);

        if (executable->nge_status >= NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
            continue;
        }

        result = ngcllExecutableTransferTimeoutGetEarliestTimeoutTime(
            context, executable, timeoutTime, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the earliest timeout time.\n"); 
            goto error;
        }
    }

    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngclExecutableListReadUnlock(context, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * TransferTimeout: Get an earliest timeout time in specified executable
 */
static int
ngcllExecutableTransferTimeoutGetEarliestTimeoutTime(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    time_t *timeoutTime,
    int *error)
{
    static const char fName[] =
        "ngcllExecutableTransferTimeoutGetEarliestTimeoutTime";
    time_t *sessionTimeoutTime;
    ngclSession_t *session;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(timeoutTime != NULL);

    /* Do not clear the timeoutTime here, timeoutTime value should remain */

    sessionTimeoutTime = NULL;

    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
        return 0;
    }

    session = NULL; /* retrieve head item */
    while ((session = ngclSessionGetNext(executable, session, error)) != NULL) {

        assert(session != NULL);

        if (session->ngs_status >= NG_SESSION_STATUS_DONE) {
            continue;
        }

        sessionTimeoutTime = &session->ngs_transferTimeoutTime_argument;
        if ((session->ngs_transferTimeout_argument > 0) &&
            (*sessionTimeoutTime > 0)) {
            if (*timeoutTime <= 0) {
                *timeoutTime = *sessionTimeoutTime;

            } else if (*sessionTimeoutTime < *timeoutTime) {
                *timeoutTime = *sessionTimeoutTime;
            }
        }

        sessionTimeoutTime = &session->ngs_transferTimeoutTime_result;
        if ((session->ngs_transferTimeout_result > 0) &&
            (*sessionTimeoutTime > 0)) {
            if (*timeoutTime <= 0) {
                *timeoutTime = *sessionTimeoutTime;

            } else if (*sessionTimeoutTime < *timeoutTime) {
                *timeoutTime = *sessionTimeoutTime;
            }
        }

        sessionTimeoutTime = &session->ngs_transferTimeoutTime_cbArgument;
        if ((session->ngs_transferTimeout_cbArgument > 0) &&
            (*sessionTimeoutTime > 0)) {
            if (*timeoutTime <= 0) {
                *timeoutTime = *sessionTimeoutTime;

            } else if (*sessionTimeoutTime < *timeoutTime) {
                *timeoutTime = *sessionTimeoutTime;
            }
        }

        sessionTimeoutTime = &session->ngs_transferTimeoutTime_cbResult;
        if ((session->ngs_transferTimeout_cbResult > 0) &&
            (*sessionTimeoutTime > 0)) {
            if (*timeoutTime <= 0) {
                *timeoutTime = *sessionTimeoutTime;

            } else if (*sessionTimeoutTime < *timeoutTime) {
                *timeoutTime = *sessionTimeoutTime;
            }
        }
        sessionTimeoutTime = NULL;
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * TransferTimeout: Check timeout and proceed timeout.
 */
static int
ngcllTransferTimeoutCheck(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllTransferTimeoutCheck";
    ngclExecutable_t *executable;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking transfer timeout.\n"); 

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        return 0;
    }

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, error)) != NULL) {

        result = ngcllExecutableTransferTimeoutCheck(
            context, executable, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the transfer timeout on Executable.\n"); 
            goto error;
        }
    }

    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return 0;
    }

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "checking transfer timeout done.\n"); 

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngclExecutableListReadUnlock(context, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * TransferTimeout: Check the Executable.
 */
static int
ngcllExecutableTransferTimeoutCheck(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    static const char fName[] = "ngcllExecutableTransferTimeoutCheck";
    int wasTimeout, result, sessionTimeout;
    time_t now, *sessionTimeoutTime;
    ngclSession_t *session;
    char *typeStr;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    if (executable->nge_status >= NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
        return 1;
    }

    sessionTimeout = 0;
    sessionTimeoutTime = NULL;
    typeStr = "none";

    wasTimeout = 0;
    now = time(NULL);

    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
        return 0;
    }

    session = NULL; /* retrieve head item */
    while ((session = ngclSessionGetNext(executable, session, error)) != NULL) {

        assert(session != NULL);

        if (session->ngs_status >= NG_SESSION_STATUS_DONE) {
            continue;
        }

        sessionTimeout = session->ngs_transferTimeout_argument;
        sessionTimeoutTime = &session->ngs_transferTimeoutTime_argument;
        if ((sessionTimeout > 0) && (*sessionTimeoutTime > 0)) {

            if (now >= *sessionTimeoutTime) {
                wasTimeout = 1;
                typeStr = "argument";
                break;
            }
        }

        sessionTimeout = session->ngs_transferTimeout_result;
        sessionTimeoutTime = &session->ngs_transferTimeoutTime_result;
        if ((sessionTimeout > 0) && (*sessionTimeoutTime > 0)) {

            if (now >= *sessionTimeoutTime) {
                wasTimeout = 1;
                typeStr = "result";
                break;
            }
        }

        sessionTimeout = session->ngs_transferTimeout_cbArgument;
        sessionTimeoutTime = &session->ngs_transferTimeoutTime_cbArgument;
        if ((sessionTimeout > 0) && (*sessionTimeoutTime > 0)) {

            if (now >= *sessionTimeoutTime) {
                wasTimeout = 1;
                typeStr = "callback argument";
                break;
            }
        }

        sessionTimeout = session->ngs_transferTimeout_cbResult;
        sessionTimeoutTime = &session->ngs_transferTimeoutTime_cbResult;
        if ((sessionTimeout > 0) && (*sessionTimeoutTime > 0)) {

            if (now >= *sessionTimeoutTime) {
                wasTimeout = 1;
                typeStr = "callback result";
                break;
            }
        }

        sessionTimeout = 0;
        sessionTimeoutTime = NULL;
    }

    if (wasTimeout != 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "transfer timeout for %s (%d seconds) occurred"
            " for executable on \"%s\".\n",
            typeStr, sessionTimeout, executable->nge_hostName);
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
        return 0;
    }

    /* Session timeout occurred */
    if (wasTimeout != 0) {
        result = ngcliExecutableUnusable(
            executable, NG_ERROR_TIMEOUT, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Executable unusable.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

