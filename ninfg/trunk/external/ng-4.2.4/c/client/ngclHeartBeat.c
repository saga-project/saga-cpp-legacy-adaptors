#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclHeartBeat.c,v $ $Revision: 1.19 $ $Date: 2006/08/22 05:52:09 $";
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
 * Module of HeartBeat for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngcllHeartBeatGetCurrentInterval(ngclContext_t *, int *, int *);
static int ngcllHeartBeatEvent(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllHeartBeatEventTimeSet(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllHeartBeatCheck(ngclContext_t *, int *);
static int ngcllExecutableHeartBeatCheck(
    ngclContext_t *, ngclExecutable_t *, int *);
static int ngcllExecutableHeartBeatStatusSet(
    ngclContext_t *, ngclExecutable_t *, ngclHeartBeatStatus_t, int, int *);
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
    ngcliObserveItem_t *observeItem;
    int enabled;

    /* Check the arguments */
    assert(context != NULL);

    context->ngc_heartBeat = NULL;

    enabled = 0;
#ifdef NG_PTHREAD
    /* heartbeat may used by re-configuration read. */
    enabled = 1;
#endif /* NG_PTHREAD */

    if (enabled == 0) {
        /* not used */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: heartbeat check feature is not used.\n", fName);
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Initialize the heartbeat check module.\n", fName);

    /* Construct */
    observeItem = ngcliObserveItemConstruct(
        context, &context->ngc_observe, error);
    if (observeItem == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the ObserveItem.\n", fName);
        return 0;
    }

    context->ngc_heartBeat = observeItem;

    observeItem->ngoi_eventTime = 0; /* not used at first time */
    observeItem->ngoi_interval = 0;  /* not used at first time */
    observeItem->ngoi_eventFunc = ngcllHeartBeatEvent;
    observeItem->ngoi_eventTimeSetFunc = ngcllHeartBeatEventTimeSet;

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
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observeItem = context->ngc_heartBeat;
    if (observeItem == NULL) {
        /* not used */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finalize the heartbeat check module.\n", fName);

    context->ngc_heartBeat = NULL;

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
 * HeartBeat: Change the interval
 */
int
ngcliHeartBeatIntervalChange(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliHeartBeatIntervalChange";
    ngcliObserveThread_t *observe;
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observe = &context->ngc_observe;
    observeItem = context->ngc_heartBeat;

    if (observeItem == NULL) {
        /* HeartBeat was not used */
        return 1;
    }

    /* Notify ObserveItem(ObserveThread) that interval was changed */
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
 * HeartBeat: ObserveThread event arrived
 */
static int
ngcllHeartBeatEvent(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllHeartBeatEvent";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    /* Check the heartbeat */
    result = ngcllHeartBeatCheck(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: checking heartbeat failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: ObserveThread eventTime setup
 */
static int
ngcllHeartBeatEventTimeSet(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllHeartBeatEventTimeSet";
    int result, interval, oldInterval;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    interval = 0;

    if (observeItem->ngoi_eventTimeChangeRequested != 0) {
        oldInterval = observeItem->ngoi_interval;

        /* Get interval */
        result = ngcllHeartBeatGetCurrentInterval(
            context, &interval, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Getting heartbeat interval failed.\n",
                fName);
            return 0;
        }

        /* Check interval */
        if (interval < 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: interval is wrong (%d).\n", fName, interval);
        }

        /* Set new interval */
        observeItem->ngoi_interval = interval;

        /* set eventTime if interval was shorten */
        if (interval <= 0) {
            observeItem->ngoi_eventTime = 0; /* Disable */

        } else if ((oldInterval == 0) || 
            ((now + interval) < observeItem->ngoi_eventTime)){

            observeItem->ngoi_eventTime = now + interval;
        }
    }

    /* Set next eventTime */
    if (observeItem->ngoi_eventExecuted != 0) {
        observeItem->ngoi_eventTime = now + observeItem->ngoi_interval;
    }

    /* Check disabled */
    if (observeItem->ngoi_interval <= 0) {
        observeItem->ngoi_eventTime = 0; /* Disable */
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

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: checking heartbeat interval.\n", fName);

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
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

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: checking heartbeat interval done.\n", fName);

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

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: checking heartbeat.\n", fName);

    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
        return 0;
    }

    executable = NULL; /* retrieve head item */
    while ((executable = ngclExecutableGetNext(
        context, executable, error)) != NULL) {

        result = ngcllExecutableHeartBeatCheck(context, executable, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't check the heartbeat on Executable.\n", fName);
            goto error;
        }
    }

    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: checking heartbeat done.\n", fName);

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngclExecutableListReadUnlock(context, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
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
    time_t received, now, warning, timeout, onTransfer;
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
    assert(executable->nge_heartBeatTimeoutOnTransfer >= 0);
    assert(executable->nge_heartBeatLastReceived > 0);

    received = executable->nge_heartBeatLastReceived;
    warning = received + executable->nge_heartBeatInterval;
    timeout = received + executable->nge_heartBeatTimeout;
    onTransfer = 0;

    if (executable->nge_heartBeatIsDataTransferring != 0) {
        if (executable->nge_heartBeatTimeoutOnTransfer == 0) {
            /* heartbeat check disabled while transferring data. */
            /* And no warning output. */
            return 1;
        }
        timeout = received + executable->nge_heartBeatTimeoutOnTransfer;
        warning = received + (executable->nge_heartBeatTimeoutOnTransfer / 2);
        /* Suppress warning while data transfer */
        onTransfer = 1;
    }
    now = time(NULL);

    status = NG_HEART_BEAT_STATUS_OK;
    if (now > timeout) {
        status = NG_HEART_BEAT_STATUS_ERROR;
    } else if (now > warning) {
        status = NG_HEART_BEAT_STATUS_WARNING;
    }

    if (status != NG_HEART_BEAT_STATUS_OK) {
        result = ngcllExecutableHeartBeatStatusSet(
            context, executable, status, onTransfer, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set heartbeat status\n", fName);
            return 0;
        }
    }

    /* Set back to OK */
    if ((status == NG_HEART_BEAT_STATUS_OK) &&
        (executable->nge_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING)) {

        result = ngcllExecutableHeartBeatStatusSet(
            context, executable, status, onTransfer, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set heartbeat status\n", fName);
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
    int onTransfer,
    int *error)
{
    static const char fName[] = "ngcllExecutableHeartBeatStatusSet";
    ngclSession_t *session;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: heartbeat timeout (%d seconds) occurred"
            "%s for executable on \"%s\".\n",
            fName, executable->nge_heartBeatTimeout,
            ((onTransfer != 0) ? " on data transfer" : ""),
            executable->nge_hostName);

    } else if (status == NG_HEART_BEAT_STATUS_WARNING) {

        /* warning only if status changed */
        if (executable->nge_heartBeatStatus != NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: heartbeat timeout warning (%d seconds) occurred"
                "%s for executable on \"%s\".\n",
                fName, executable->nge_heartBeatInterval,
                ((onTransfer != 0) ? " on data transfer" : ""),
                executable->nge_hostName);
        }
    } else if (status == NG_HEART_BEAT_STATUS_OK) {

        /* warning only if status changed */
        if (executable->nge_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: heartbeat revived again for executable on \"%s\".\n",
                fName, executable->nge_hostName);
        }
    }

    /* Set heartbeat status on Executable */
    executable->nge_heartBeatStatus = status;

    /* Set same status for sessions */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Session.\n", fName);
        return 0;
    }

    session = NULL; /* retrieve head item */
    while ((session = ngclSessionGetNext(executable, session, error)) != NULL) {

        result = ngcllSessionHeartBeatStatusSet(
            context, session, status, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't check the Session.\n", fName);
            goto error;
        }
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
        return 0;
    }

    /* If heartbeat was not sent, set the Executable unusable */
    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        result = ngcliExecutableUnusable(executable, NG_ERROR_TIMEOUT, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Executable unusable.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngclSessionListReadUnlock(context, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Session.\n", fName);
        return 0;
    }

    if (status == NG_HEART_BEAT_STATUS_ERROR) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
           NG_LOG_LEVEL_ERROR, NULL,
          "%s: heartbeat timeout occurred for the session.\n", fName);
    } else if (status == NG_HEART_BEAT_STATUS_WARNING) {

        /* warning only if status changed */
        if (session->ngs_heartBeatStatus != NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: heartbeat timeout warning occurred for the session.\n",
            fName);
        }
    } else if (status == NG_HEART_BEAT_STATUS_OK) {

        /* warning only if status changed */
        if (session->ngs_heartBeatStatus == NG_HEART_BEAT_STATUS_WARNING) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: heartbeat revived again for the session.\n",
            fName);
        }
    }

    /* Set heartbeat status on Session */
    session->ngs_heartBeatStatus = status;

    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Session.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

