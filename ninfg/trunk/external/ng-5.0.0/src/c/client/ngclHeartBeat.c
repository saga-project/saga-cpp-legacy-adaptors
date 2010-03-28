/*
 * $RCSfile: ngclHeartBeat.c,v $ $Revision: 1.5 $ $Date: 2007/12/13 06:34:16 $
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
 * Module of HeartBeat for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclHeartBeat.c,v $ $Revision: 1.5 $ $Date: 2007/12/13 06:34:16 $")

/**
 * Prototype declaration of internal functions.
 */
static void ngcllHeartBeatInitializeMember(ngcliHeartBeatCheck_t *);
static int ngcllHeartBeatEvent(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllHeartBeatEventTimeSet(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllHeartBeatGetCurrentInterval(
    ngclContext_t *, int *, int *);
static int ngcllHeartBeatCheck(ngclContext_t *, int *);
static int ngcllExecutableHeartBeatCheck(
    ngclContext_t *, ngclExecutable_t *, int *);
static int ngcllExecutableHeartBeatStatusSet(
    ngclContext_t *, ngclExecutable_t *, ngclHeartBeatStatus_t, int *);
static int ngcllSessionHeartBeatStatusSet(
    ngclContext_t *, ngclSession_t *, ngclHeartBeatStatus_t, int *);

/**
 * HeartBeat: Initialize
 */
int
ngcliHeartBeatInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliHeartBeatInitialize";
    ngcliHeartBeatCheck_t *heartBeat;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeat = &context->ngc_heartBeatCheck;
    ngcllHeartBeatInitializeMember(heartBeat);

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Initialize the heartbeat check module.\n"); 

    heartBeat->nghbc_eventTime = 0;  /* not used at first time. */
    heartBeat->nghbc_interval = 0;
    heartBeat->nghbc_changeRequested = 0;
    heartBeat->nghbc_eventExecuted = 0;

    /* Construct the I/O handle. */
    handle = ngiIOhandleConstruct(context->ngc_event, log, error);
    if (handle == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Construct the I/O handle failed.\n"); 
        return 0;
    }

    heartBeat->nghbc_timeHandle = handle;

    /* Register the Time Event callback. */
    result = ngiIOhandleTimeEventCallbackRegister(
        handle,
        ngcllHeartBeatEvent, context,
        ngcllHeartBeatEventTimeSet, context,
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
 * HeartBeat: Finalize
 */
int
ngcliHeartBeatFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliHeartBeatFinalize";
    ngcliHeartBeatCheck_t *heartBeat;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeat = &context->ngc_heartBeatCheck;
    handle = heartBeat->nghbc_timeHandle;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Finalize the heartbeat check module.\n"); 

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

    ngcllHeartBeatInitializeMember(heartBeat);

    /* Success */
    return 1;
}

/**
 * HeartBeat: Initialize the members.
 */
static void
ngcllHeartBeatInitializeMember(
    ngcliHeartBeatCheck_t *heartBeat)
{
    /* Check the arguments */
    assert(heartBeat != NULL);

    heartBeat->nghbc_timeHandle = NULL;
    heartBeat->nghbc_eventTime = 0;
    heartBeat->nghbc_interval = 0;
    heartBeat->nghbc_changeRequested = 0;
    heartBeat->nghbc_eventExecuted = 0;
}

/**
 * HeartBeat: Change the interval
 */
int
ngcliHeartBeatIntervalChange(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliHeartBeatIntervalChange";
    ngcliHeartBeatCheck_t *heartBeat;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    heartBeat = &context->ngc_heartBeatCheck;
    handle = heartBeat->nghbc_timeHandle;
    log = context->ngc_log;

    heartBeat->nghbc_changeRequested = 1;

    /* Notify Time handle that the interval was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        handle, log,  error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Time event arrived.
 */
static int
ngcllHeartBeatEvent(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllHeartBeatEvent";
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
            "heartbeat event callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    /* Check the heartbeat */
    result = ngcllHeartBeatCheck(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "checking heartbeat failed.\n"); 
        return 0;
    }
    context->ngc_heartBeatCheck.nghbc_eventExecuted = 1;

    /* Success */
    return 1;
}

/**
 * HeartBeat: Time event eventTime setup.
 */
static int
ngcllHeartBeatEventTimeSet(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllHeartBeatEventTimeSet";
    int result, interval, oldInterval;
    ngcliHeartBeatCheck_t *heartBeat;
    int *error, errorEntity;
    ngclContext_t *context;
    ngLog_t *log;
    time_t now;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngclContext_t *)arg;
    heartBeat = &context->ngc_heartBeatCheck;
    log = context->ngc_log;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (state == NGI_IOHANDLE_STATE_CANCELED) {
        ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat event time change callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    now = time(NULL);
    interval = 0;

    if (heartBeat->nghbc_changeRequested != 0) {
        heartBeat->nghbc_changeRequested = 0;

        oldInterval = heartBeat->nghbc_interval;

        /* Get interval. */
        result = ngcllHeartBeatGetCurrentInterval(
            context, &interval, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Getting heartbeat interval failed.\n"); 
            return 0;
        }

        /* Check interval. */
        if (interval < 0) {
            ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "interval is wrong (%d).\n", interval); 
        }

        /* Set new interval. */
        heartBeat->nghbc_interval = interval;

        /* Set eventTime if interval was shorten. */
        if (interval <= 0) {
            heartBeat->nghbc_eventTime = 0; /* Disable */

        } else if ((oldInterval == 0) || 
            ((now + interval) < heartBeat->nghbc_eventTime)){

            heartBeat->nghbc_eventTime = now + interval;
        }
    }

    /* Set next eventTime */
    if (heartBeat->nghbc_eventExecuted != 0) {
        heartBeat->nghbc_eventExecuted = 0;
        heartBeat->nghbc_eventTime = now + heartBeat->nghbc_interval;
    }

    /* Check disabled */
    if (heartBeat->nghbc_interval <= 0) {
        heartBeat->nghbc_eventTime = 0; /* Disable */
    }

    /* Set the eventTime to handle. */
    result = ngiIOhandleTimeEventTimeSet(
        heartBeat->nghbc_timeHandle,
        heartBeat->nghbc_eventTime, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Set the event time failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Get the check interval
 */
static int
ngcllHeartBeatGetCurrentInterval(
    ngclContext_t *context,
    int *interval,
    int *error)
{
    static const char fName[] = "ngcllHeartBeatGetCurrentInterval";
    ngclExecutable_t *executable;
    int found, minInterval;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(interval != NULL);

    found = 0;
    minInterval = 0;
    *interval = 0;

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "checking heartbeat interval.\n"); 

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

        if (executable->nge_heartBeatInterval <= 0) {
            /* This executable don't use heartbeat */
            continue;
        }

        if ((found == 0) ||
            (executable->nge_heartBeatInterval < minInterval)) {
            minInterval = executable->nge_heartBeatInterval;
        }
        found = 1;
    }

    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return 0;
    }

    /* Set check interval */
    if (found == 0) {
        *interval = 0;
    } else {
        assert(minInterval > 0);

        /**
         * check interval is a half time from minimum send interval.
         * to not to mis judgement.
         */
        minInterval = minInterval / 2;
        if (minInterval <= 0) {
            minInterval = 1;
        }
        *interval = minInterval;
    }

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "checking heartbeat interval done (interval=%d sec).\n",
        minInterval); 

    /* Success */
    return 1;
}


/**
 * HeartBeat: Check
 */
static int
ngcllHeartBeatCheck(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllHeartBeatCheck";
    ngclExecutable_t *executable;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "checking heartbeat.\n"); 

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        return 0;
    }

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, error)) != NULL) {

        result = ngcllExecutableHeartBeatCheck(context, executable, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the heartbeat on Executable.\n"); 
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
        "checking heartbeat done.\n"); 

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
 * HeartBeat: Check the Executable.
 */
static int
ngcllExecutableHeartBeatCheck(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    static const char fName[] = "ngcllExecutableHeartBeatCheck";
    time_t received, now, warning, timeout;
    ngclHeartBeatStatus_t status;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    if (executable->nge_heartBeatInterval <= 0) {
        /* This executable don't use heartbeat */

        return 1;
    }

    if ((executable->nge_status <= NG_EXECUTABLE_STATUS_CONNECTING) ||
        (executable->nge_status >= NG_EXECUTABLE_STATUS_EXIT_REQUESTED)) {
        /* heartbeat is send after the connection was created. */
        /* heartbeat is not send after the connection was closed. */

        return 1;
    }

    assert(executable->nge_heartBeatTimeout > 0);
    assert(executable->nge_heartBeatLastReceived > 0);

    received = executable->nge_heartBeatLastReceived;
    warning = received + executable->nge_heartBeatInterval;
    timeout = received + executable->nge_heartBeatTimeout;

    now = time(NULL);

    status = NG_HEART_BEAT_STATUS_OK;
    if (now > timeout) {
        status = NG_HEART_BEAT_STATUS_ERROR;
    } else if (now > warning) {
        status = NG_HEART_BEAT_STATUS_WARNING;
    }

    if (status != NG_HEART_BEAT_STATUS_OK) {
        result = ngcllExecutableHeartBeatStatusSet(
            context, executable, status, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set heartbeat status\n"); 
            return 0;
        }
    }

    /* Set back to OK */
    if ((status == NG_HEART_BEAT_STATUS_OK) &&
        (executable->nge_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING)) {

        result = ngcllExecutableHeartBeatStatusSet(
            context, executable, status, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set heartbeat status\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Set the status of the Executable.
 */
static int
ngcllExecutableHeartBeatStatusSet(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclHeartBeatStatus_t status,
    int *error)
{
    static const char fName[] = "ngcllExecutableHeartBeatStatusSet";
    ngclSession_t *session;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat timeout (%d seconds) occurred"
            " for executable on \"%s\".\n",
            executable->nge_heartBeatTimeout,
            executable->nge_hostName); 

    } else if (status == NG_HEART_BEAT_STATUS_WARNING) {

        /* warning only if status changed */
        if (executable->nge_heartBeatStatus != NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "heartbeat timeout warning (%d seconds) occurred"
                " for executable on \"%s\".\n",
                executable->nge_heartBeatInterval,
                executable->nge_hostName); 
        }
    } else if (status == NG_HEART_BEAT_STATUS_OK) {

        /* warning only if status changed */
        if (executable->nge_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "heartbeat revived again for executable on \"%s\".\n",
                executable->nge_hostName); 
        }
    }

    /* Set heartbeat status on Executable */
    executable->nge_heartBeatStatus = status;

    /* Set same status for sessions */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
        return 0;
    }

    session = NULL; /* retrieve head item */
    while ((session = ngclSessionGetNext(executable, session, error)) != NULL) {

        result = ngcllSessionHeartBeatStatusSet(
            context, session, status, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the Session.\n"); 
            goto error;
        }
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
        return 0;
    }

    /* If heartbeat was not sent, set the Executable unusable */
    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        result = ngcliExecutableUnusable(executable, NG_ERROR_TIMEOUT, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Executable unusable.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngclSessionListReadUnlock(context, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * HeartBeat: Set the status of the Session.
 */
static int
ngcllSessionHeartBeatStatusSet(
    ngclContext_t *context,
    ngclSession_t *session,
    ngclHeartBeatStatus_t status,
    int *error)
{
    static const char fName[] = "ngcllSessionHeartBeatStatusSet";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(session != NULL);

    result = ngclSessionWriteLock(session, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Session.\n"); 
        return 0;
    }

    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat timeout occurred for the session.\n"); 
    } else if (status == NG_HEART_BEAT_STATUS_WARNING) {

        /* warning only if status changed */
        if (session->ngs_heartBeatStatus != NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogWarnSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "heartbeat timeout warning occurred for the session.\n"); 
        }
    } else if (status == NG_HEART_BEAT_STATUS_OK) {

        /* warning only if status changed */
        if (session->ngs_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogWarnSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "heartbeat revived again for the session.\n"); 
        }
    }

    /* Set heartbeat status on Session */
    session->ngs_heartBeatStatus = status;

    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Session.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

