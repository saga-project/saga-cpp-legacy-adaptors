#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclJobStartTimeout.c,v $ $Revision: 1.3 $ $Date: 2005/07/04 08:49:47 $";
#endif /* NG_OS_IRIX */
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

/**
 * Module of JobStartTimeout for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "ng.h"
#include "ngClientInternal.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngcllJobStartTimeoutEvent(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllJobStartTimeoutEventTimeSet(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);

static int ngcllJobStartTimeoutGetEarliestTimeoutTime(
    ngclContext_t *, time_t *, int *);
static int ngcllJobStartTimeoutCheck(
    ngclContext_t *, int *);

/**
 * JobStartTimeout: Initialize
 */
int
ngcliJobStartTimeoutInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliJobStartTimeoutInitialize";
    ngcliObserveItem_t *observeItem;
    int enabled;

    /* Check the arguments */
    assert(context != NULL);

    context->ngc_jobStartTimeout = NULL;

    enabled = 0;

#ifdef NG_PTHREAD
    /* 
     * Job start timeout check with Observe Thread
     * only when Globus Toolkit's flavor is Pthread flavor.
     */
    enabled = 1;
#endif /* NG_PTHREAD */

    if (enabled == 0) {
        /* not used */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Job start timeout check by Observe Thread is not used.\n",
            fName);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Note: Job start timeout isn't available, "
            "if \"argument_transfer\" is nowait or copy.\n", fName);
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Initialize the Job Start Timeout check module.\n", fName);

    /* Construct */
    observeItem = ngcliObserveItemConstruct(
        context, &context->ngc_observe, error);
    if (observeItem == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the ObserveItem.\n", fName);
        return 0;
    }

    context->ngc_jobStartTimeout = observeItem;

    observeItem->ngoi_eventTime = 0; /* not used at first time */
    observeItem->ngoi_interval = 0;  /* not used */
    observeItem->ngoi_eventFunc = ngcllJobStartTimeoutEvent;
    observeItem->ngoi_eventTimeSetFunc = ngcllJobStartTimeoutEventTimeSet;

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
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observeItem = context->ngc_jobStartTimeout;
    if (observeItem == NULL) {
        /* not used */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finalize the Job Start Timeout check module.\n", fName);

    context->ngc_jobStartTimeout = NULL;

    /* Destruct */
    result = ngcliObserveItemDestruct(
        context, &context->ngc_observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't destruct the ObserveItem.\n", fName);
        return 0;
    }

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
    ngcliObserveThread_t *observe;
    ngcliObserveItem_t *observeItem;
    int timeout, result;
    time_t timeoutTime;
    int executableListLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    observe = &context->ngc_observe;
    observeItem = context->ngc_jobStartTimeout;
    log = context->ngc_log;
    executableListLocked = 0;

    if (observeItem == NULL) {
        /* JobStartTimeout was not used in this client */
        return 1;
    }

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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while ((executable = ngcliJobGetNextExecutable(
        jobMng, executable, error)) != NULL) {

        assert(executable != NULL);

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Executable.\n", fName);
            goto error;
        }

        executable->nge_jobStartTimeoutTime = timeoutTime;

        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
        goto error;
    }

    /* Notify ObserveItem(ObserveThread) that event time was changed */
    result = ngcliObserveItemEventTimeChangeRequest(
        context, observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Change request for Observe Thread failed.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error */
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Executable.\n", fName);
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
    ngcliObserveThread_t *observe;
    ngcliObserveItem_t *observeItem;
    ngcliJobManager_t *jobMng;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    observe = &context->ngc_observe;
    observeItem = context->ngc_jobStartTimeout;
    jobMng = executable->nge_jobMng;
    log = context->ngc_log;

    if (observeItem == NULL) {
        /* job start was not used in this client */
        return 1;
    }

    if (jobMng->ngjm_attr.ngja_startTimeout <= 0) {
        /* Job Start Timeout was not used */
        return 1;
    }

    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        return 0;
    }

    /* No need to check anymore */
    executable->nge_jobStartTimeoutTime = 0;

    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        return 0;
    }

    /* Notify ObserveItem(ObserveThread) that event time was changed */
    result = ngcliObserveItemEventTimeChangeRequest(
        context, observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Change request for Observe Thread failed.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: ObserveThread event arrived
 */
static int
ngcllJobStartTimeoutEvent(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllJobStartTimeoutEvent";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Checking job start timeout."
        " May be, one executable handle was timeout.\n",
        fName);

    /* Check the job start timeout */
    result = ngcllJobStartTimeoutCheck(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: checking job start timeout failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * JobStartTimeout: ObserveThread eventTime setup
 */
static int
ngcllJobStartTimeoutEventTimeSet(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllJobStartTimeoutEventTimeSet";
    time_t timeoutTime;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    if ((observeItem->ngoi_eventTimeChangeRequested != 0) ||
        (observeItem->ngoi_eventExecuted != 0)) {

        /* Get earliest timeout time */
        result = ngcllJobStartTimeoutGetEarliestTimeoutTime(
            context, &timeoutTime, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Getting earliest timeout time failed.\n",
                fName);
            return 0;
        }

        /* Set new timeout time */
        observeItem->ngoi_eventTime = timeoutTime;
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, NULL)) != NULL) {

        assert(executable != NULL);

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Executable.\n", fName);
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
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Executable.\n", fName);
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

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Checking job start timeout.\n", fName);

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
        goto error;
    }
    executableListLocked = 1;

    executable = NULL; /* retrieve head item */
    while (((executable = ngclExecutableGetNext(
             context, executable, error)) != NULL)) {

        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        wasTimeout = 0;
        if ((executable->nge_status < NG_EXECUTABLE_STATUS_IDLE) &&
            (executable->nge_jobMng->ngjm_attr.ngja_startTimeout > 0) &&
            (executable->nge_jobStartTimeoutTime > 0)) {
            if (now >= executable->nge_jobStartTimeoutTime) {
                ngclLogPrintfExecutable(executable,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: job start timeout (%d seconds) occurred"
                    " for executable on \"%s\".\n",
                    fName, executable->nge_jobMng->ngjm_attr.ngja_startTimeout,
                    executable->nge_hostName);

                wasTimeout = 1;
            }
        }

        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        /* Job start timeout occurred */
        if (wasTimeout != 0) {
            result = ngcliExecutableUnusable(executable, NG_ERROR_TIMEOUT, error);
            if (result == 0) {
                ngclLogPrintfExecutable(executable,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't set the Executable unusable.\n", fName);
                goto error;
            }
        }
    }

    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
        goto error;
    }


    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Checking job start timeout done.\n", fName);

    /* Success */
    return 1;

    /*Error*/
error:
    if (executableListLocked != 0) {
        result = ngclExecutableListReadUnlock(context, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Executable.\n", fName);
        }
    }
    return 0;
}
