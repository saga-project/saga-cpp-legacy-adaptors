/*
 * $RCSfile: ngclJobStartTimeout.c,v $ $Revision: 1.5 $ $Date: 2007/12/13 06:34:16 $
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
 * Module of JobStartTimeout for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclJobStartTimeout.c,v $ $Revision: 1.5 $ $Date: 2007/12/13 06:34:16 $")

/**
 * Prototype declaration of internal functions.
 */
static int ngcllJobStartTimeoutEvent(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllJobStartTimeoutEventTimeSet(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllJobStartTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, time_t *, int *);
static int ngcllJobStartTimeoutCheck(ngclContext_t *, int *);


/**
 * JobStartTimeout: Initialize
 */
int
ngcliJobStartTimeoutInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliJobStartTimeoutInitialize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    context->ngc_jobStartTimeoutHandle = NULL;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Initialize the Job Start Timeout check module.\n"); 

    /* Construct the I/O handle. */
    handle = ngiIOhandleConstruct(context->ngc_event, log, error);
    if (handle == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Construct the I/O handle failed.\n"); 
        return 0;
    }

    context->ngc_jobStartTimeoutHandle = handle;

    /* Register the Time Event callback. */
    result = ngiIOhandleTimeEventCallbackRegister(
        handle,
        ngcllJobStartTimeoutEvent, context,
         ngcllJobStartTimeoutEventTimeSet, context,
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
 * JobStartTimeout: Finalize
 */
int
ngcliJobStartTimeoutFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliJobStartTimeoutFinalize";
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    handle = context->ngc_jobStartTimeoutHandle;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Finalize the Job Start Timeout check module.\n"); 

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

    context->ngc_jobStartTimeoutHandle = NULL;

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: Job Start
 */
int
ngcliJobStartTimeoutJobStart(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliJobStartTimeoutJobStart";
    ngclExecutable_t *executable;
    int timeout, result;
    time_t timeoutTime;
    int executableListLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;
    executableListLocked = 0;

    /* Get the timeout time */
    timeout = jobMng->ngjm_attr.ngja_startTimeout;

    if (timeout <= 0) {
        /* Job start timeout was not used */
        return 1;
    }

    /* Set timeout time for each executable handles */
    timeoutTime = time(NULL) + timeout;

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while ((executable = ngcliJobGetNextExecutable(
        jobMng, executable, error)) != NULL) {

        assert(executable != NULL);

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
            goto error;
        }

        executable->nge_jobStartTimeoutTime = timeoutTime;

        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        goto error;
    }

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        context->ngc_jobStartTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error */
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Executable.\n"); 
        }
    }
    return 0;
}

/**
 * JobStartTimeout: Job Started.
 */
int
ngcliJobStartTimeoutJobStarted(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    static const char fName[] = "ngcliJobStartTimeoutJobStarted";
    ngcliJobManager_t *jobMng;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    jobMng = executable->nge_jobMng;
    log = context->ngc_log;

    if (jobMng->ngjm_attr.ngja_startTimeout <= 0) {
        /* Job Start Timeout was not used */
        return 1;
    }

    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        return 0;
    }

    /* No need to check anymore */
    executable->nge_jobStartTimeoutTime = 0;

    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        return 0;
    }

    /* Notify Time handle that the event time was changed. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        context->ngc_jobStartTimeoutHandle, log,  error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: Time event arrived.
 */
static int
ngcllJobStartTimeoutEvent(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllJobStartTimeoutEvent";
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
            "job start timeout event callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking job start timeout."
        " May be, one executable handle was timeout.\n"); 

    /* Check the job start timeout */
    result = ngcllJobStartTimeoutCheck(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "checking job start timeout failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: Time change event arrived.
 */
static int
ngcllJobStartTimeoutEventTimeSet(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngcllJobStartTimeoutEventTimeSet";
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
            "job start timeout event time change callback canceled.\n"); 

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    /* Get earliest timeout time */
    result = ngcllJobStartTimeoutGetEarliestTimeoutTime(
        context, &timeoutTime, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Getting earliest timeout time failed.\n"); 
        return 0;
    }

    /* Set new timeout time */
    result = ngiIOhandleTimeEventTimeSet(
        context->ngc_jobStartTimeoutHandle, timeoutTime, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Set the event time failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: Get an earliest timeout time
 */
static int
ngcllJobStartTimeoutGetEarliestTimeoutTime(
    ngclContext_t *context,
    time_t *timeoutTime,
    int *error)
{
    static const char fName[] = "ngcllJobStartTimeoutGetEarliestTimeoutTime";
    ngclExecutable_t *executable;
    int executableListLocked;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(timeoutTime != NULL);

    log = context->ngc_log;
    executableListLocked = 0;;
    *timeoutTime = 0;

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, NULL)) != NULL) {

        assert(executable != NULL);

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
            goto error;
        }

        if ((executable->nge_status < NG_EXECUTABLE_STATUS_IDLE) &&
            (executable->nge_jobMng->ngjm_attr.ngja_startTimeout > 0) &&
            (executable->nge_jobStartTimeoutTime > 0)) {
            if (*timeoutTime <= 0) {
                *timeoutTime = executable->nge_jobStartTimeoutTime;
            } else if (executable->nge_jobStartTimeoutTime < *timeoutTime) {
                *timeoutTime = executable->nge_jobStartTimeoutTime;
            }
        }

        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Executable.\n"); 
        }
    }
    return 0;
}

/**
 * JobStartTimeout: Check timeout and proceed timeout.
 */
static int
ngcllJobStartTimeoutCheck(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllJobStartTimeoutCheck";
    ngclExecutable_t *executable;
    ngLog_t *log;
    int executableListLocked;
    int result;
    int wasTimeout;
    time_t now;

    /* Check the arguments */
    assert(context != NULL);

    wasTimeout = 0;
    now = time(NULL);
    executableListLocked = 0;
    log = context->ngc_log;

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking job start timeout.\n"); 

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while (((executable = ngclExecutableGetNext(
             context, executable, error)) != NULL)) {

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }

        wasTimeout = 0;
        if ((executable->nge_status < NG_EXECUTABLE_STATUS_IDLE) &&
            (executable->nge_jobMng->ngjm_attr.ngja_startTimeout > 0) &&
            (executable->nge_jobStartTimeoutTime > 0)) {
            if (now >= executable->nge_jobStartTimeoutTime) {
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                    "job start timeout (%d seconds) occurred"
                    " for executable on \"%s\".\n",
                    executable->nge_jobMng->ngjm_attr.ngja_startTimeout,
                    executable->nge_hostName); 

                wasTimeout = 1;
            }
        }

        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }

        /* Job start timeout occurred */
        if (wasTimeout != 0) {
            result = ngcliExecutableUnusable(executable, NG_ERROR_TIMEOUT, error);
            if (result == 0) {
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't set the Executable unusable.\n"); 
                goto error;
            }
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        goto error;
    }


    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking job start timeout done.\n"); 

    /* Success */
    return 1;

    /*Error*/
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Executable.\n"); 
        }
    }
    return 0;
}
