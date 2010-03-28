/*
 * $RCSfile: ngclSessionTimeout.c,v $ $Revision: 1.6 $ $Date: 2007/12/19 08:26:15 $
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
 * Module of SessionTimeout for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclSessionTimeout.c,v $ $Revision: 1.6 $ $Date: 2007/12/19 08:26:15 $")

/**
 * Prototype declaration of internal functions.
 */
static int ngcllSessionTimeoutEvent(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllSessionTimeoutEventTimeSet(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllSessionTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, time_t *, int *);
static int ngcllExecutableSessionTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, ngclExecutable_t *, time_t *, int *);
static int ngcllSessionTimeoutCheck(
    ngclContext_t *, int *);
static int ngcllExecutableSessionTimeoutCheck(
    ngclContext_t *, ngclExecutable_t *, int *);


/**
 * SessionTimeout: Initialize
 */
int
ngcliSessionTimeoutInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliSessionTimeoutInitialize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    context->ngc_sessionTimeoutHandle = NULL;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Initialize the Session Timeout check module.\n"); 

    /* Construct the I/O handle. */
    handle = ngiIOhandleConstruct(context->ngc_event, log, error);
    if (handle == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Construct the I/O handle failed.\n"); 
        return 0;
    }

    context->ngc_sessionTimeoutHandle = handle;

    /* Register the Time Event callback. */
    result = ngiIOhandleTimeEventCallbackRegister(
        handle,
        ngcllSessionTimeoutEvent, context,
        ngcllSessionTimeoutEventTimeSet, context,
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
 * SessionTimeout: Finalize
 */
int
ngcliSessionTimeoutFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliSessionTimeoutFinalize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    handle = context->ngc_sessionTimeoutHandle;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Finalize the Session Timeout check module.\n"); 

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

    context->ngc_sessionTimeoutHandle = NULL;

    /* Success */
    return 1;
}

/**
 * SessionTimeout: Session was started.
 *   The timeout measurement for this session is also starting.
 */
int
ngcliSessionTimeoutSessionStart(
    ngclContext_t *context,
    ngclSession_t *session,
    int *error)
{
    static const char fName[] = "ngcliSessionTimeoutSessionStart";
    int timeout, result;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(session != NULL);

    log = context->ngc_log;

    /* Reset */
    session->ngs_timeoutTime = 0;

    /* Get the timeout time */
    timeout = session->ngs_timeout;

    if (timeout <= 0) {
        /* SessionTimeout was not used in this session */
        return 1;
    }

    /* Set timeout time for this session */
    session->ngs_timeoutTime = time(NULL) + timeout;

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        context->ngc_sessionTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SessionTimeout: Session was done.
 *   The timeout measurement for this session is also finished.
 */
int
ngcliSessionTimeoutSessionDone(
    ngclContext_t *context,
    ngclSession_t *session,
    int *error)
{
    static const char fName[] = "ngcliSessionTimeoutSessionDone";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(session != NULL);

    log = context->ngc_log;

    if (session->ngs_timeout <= 0) {
        /* SessionTimeout was not used in this session */
        return 1;
    }

    /* No need to check anymore */
    session->ngs_timeoutTime = 0;

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        context->ngc_sessionTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SessionTimeout: Time event arrived.
 */
static int
ngcllSessionTimeoutEvent(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllSessionTimeoutEvent";
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
            "session timeout event callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking session timeout. May be, one session was timeout.\n"); 

    /* Check the session timeout */
    result = ngcllSessionTimeoutCheck(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "checking session timeout failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SessionTimeout: Time change event arrived.
 */
static int
ngcllSessionTimeoutEventTimeSet(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllSessionTimeoutEventTimeSet";
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
            "session timeout event time change callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    /* Get earliest timeout time */
    result = ngcllSessionTimeoutGetEarliestTimeoutTime(
        context, &timeoutTime, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Getting earliest timeout time failed.\n"); 
        return 0;
    }

    /* Set new timeout time */
    result = ngiIOhandleTimeEventTimeSet(
        context->ngc_sessionTimeoutHandle, timeoutTime, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Set the event time failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SessionTimeout: Get an earliest timeout time
 */
static int
ngcllSessionTimeoutGetEarliestTimeoutTime(
    ngclContext_t *context,
    time_t *timeoutTime,
    int *error)
{
    static const char fName[] = "ngcllSessionTimeoutGetEarliestTimeoutTime";
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

        result = ngcllExecutableSessionTimeoutGetEarliestTimeoutTime(
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
 * SessionTimeout: Get an earliest timeout time in specified executable
 */
static int
ngcllExecutableSessionTimeoutGetEarliestTimeoutTime(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    time_t *timeoutTime,
    int *error)
{
    static const char fName[] =
        "ngcllExecutableSessionTimeoutGetEarliestTimeoutTime";
    ngclSession_t *session;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(timeoutTime != NULL);

    /* Do not clear the timeoutTime here, timeoutTime value should remain */

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

        if (session->ngs_timeout <= 0) {
            /* This session don't use session timeout */
            continue;
        }

        if (session->ngs_timeoutTime <= 0) {
            continue;
        }

        if (*timeoutTime <= 0) {
            *timeoutTime = session->ngs_timeoutTime;
            continue;
        }

        if (session->ngs_timeoutTime < *timeoutTime) {
            *timeoutTime = session->ngs_timeoutTime;
        }
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
 * SessionTimeout: Check timeout and proceed timeout.
 */
static int
ngcllSessionTimeoutCheck(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllSessionTimeoutCheck";
    ngclExecutable_t *executable;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking session timeout.\n"); 

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        return 0;
    }

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, error)) != NULL) {

        result = ngcllExecutableSessionTimeoutCheck(
            context, executable, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the session timeout on Executable.\n"); 
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
        "checking session timeout done.\n"); 

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
 * SessionTimeout: Check the Executable.
 */
static int
ngcllExecutableSessionTimeoutCheck(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    static const char fName[] = "ngcllExecutableSessionTimeoutCheck";
    ngclSession_t *session;
    int wasTimeout, result;
    time_t now;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    if (executable->nge_status >= NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
        return 1;
    }

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

        if (session->ngs_timeout <= 0) {
            /* This session don't use session timeout */
            continue;
        }

        if (session->ngs_timeoutTime <= 0) {
            continue;
        }

        if (now >= session->ngs_timeoutTime) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "session timeout (%d seconds) occurred"
                " for executable on \"%s\".\n",
                session->ngs_timeout, executable->nge_hostName); 

            wasTimeout = 1;
            break;
        }
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

