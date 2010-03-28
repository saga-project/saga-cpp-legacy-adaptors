#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclSessionTimeout.c,v $ $Revision: 1.3 $ $Date: 2005/06/21 04:49:58 $";
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
 * Module of SessionTimeout for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngcllSessionTimeoutEvent(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllSessionTimeoutEventTimeSet(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);

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
    ngcliObserveItem_t *observeItem;
    int enabled;

    /* Check the arguments */
    assert(context != NULL);

    context->ngc_sessionTimeout = NULL;

    enabled = 0;
#ifdef NG_PTHREAD
    /* Session timeout may used by session attribute */
    enabled = 1;
#endif /* NG_PTHREAD */

    if (enabled == 0) {
        /* not used */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: session timeout check feature is not used.\n", fName);
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Initialize the Session Timeout check module.\n", fName);

    /* Construct */
    observeItem = ngcliObserveItemConstruct(
        context, &context->ngc_observe, error);
    if (observeItem == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the ObserveItem.\n", fName);
        return 0;
    }

    context->ngc_sessionTimeout = observeItem;

    observeItem->ngoi_eventTime = 0; /* not used at first time */
    observeItem->ngoi_interval = 0;  /* not used */
    observeItem->ngoi_eventFunc = ngcllSessionTimeoutEvent;
    observeItem->ngoi_eventTimeSetFunc = ngcllSessionTimeoutEventTimeSet;

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
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observeItem = context->ngc_sessionTimeout;
    if (observeItem == NULL) {
        /* not used */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finalize the Session Timeout check module.\n", fName);

    context->ngc_sessionTimeout = NULL;

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
    ngcliObserveThread_t *observe;
    ngcliObserveItem_t *observeItem;
    int timeout, result;

    /* Check the arguments */
    assert(context != NULL);
    assert(session != NULL);

    observe = &context->ngc_observe;
    observeItem = context->ngc_sessionTimeout;

    /* Reset */
    session->ngs_timeoutTime = 0;

#ifndef NG_PTHREAD
    if (session->ngs_timeout > 0) {
        ngclLogPrintfSession(session,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Session timeout %d was set.\n",
            fName, session->ngs_timeout);
        ngclLogPrintfSession(session,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Session timeout is not supported for this flavor.\n",
            fName);

        /* disable */
        session->ngs_timeout = 0;
    }
#endif /* NG_PTHREAD */

    if (observeItem == NULL) {
        /* SessionTimeout was not used in this client */
        return 1;
    }

    /* Get the timeout time */
    timeout = session->ngs_timeout;

    if (timeout <= 0) {
        /* SessionTimeout was not used in this session */
        return 1;
    }

    /* Set timeout time for this session */
    session->ngs_timeoutTime = time(NULL) + timeout;

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
    ngcliObserveThread_t *observe;
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(session != NULL);

    observe = &context->ngc_observe;
    observeItem = context->ngc_sessionTimeout;

    if (observeItem == NULL) {
        /* SessionTimeout was not used in this client */
        return 1;
    }

    if (session->ngs_timeout <= 0) {
        /* SessionTimeout was not used in this session */
        return 1;
    }

    /* No need to check anymore */
    session->ngs_timeoutTime = 0;

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
 * SessionTimeout: ObserveThread event arrived
 */
static int
ngcllSessionTimeoutEvent(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllSessionTimeoutEvent";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Checking session timeout. May be, one session was timeout.\n",
        fName);

    /* Check the session timeout */
    result = ngcllSessionTimeoutCheck(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: checking session timeout failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SessionTimeout: ObserveThread eventTime setup
 */
static int
ngcllSessionTimeoutEventTimeSet(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllSessionTimeoutEventTimeSet";
    time_t timeoutTime;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    if ((observeItem->ngoi_eventTimeChangeRequested != 0) ||
        (observeItem->ngoi_eventExecuted != 0)) {

        /* Get earliest timeout time */
        result = ngcllSessionTimeoutGetEarliestTimeoutTime(
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

        result = ngcllExecutableSessionTimeoutGetEarliestTimeoutTime(
            context, executable, timeoutTime, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the earliest timeout time.\n", fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Session.\n", fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
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

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Checking session timeout.\n", fName);

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

        result = ngcllExecutableSessionTimeoutCheck(context, executable, error);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Session.\n", fName);
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
            ngclLogPrintfSession(session,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: session timeout (%d seconds) occurred"
                " for executable on \"%s\".\n",
                fName, session->ngs_timeout,
                executable->nge_hostName);

            wasTimeout = 1;
            break;
        }
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
        return 0;
    }

    /* Session timeout occurred */
    if (wasTimeout != 0) {
        result = ngcliExecutableUnusable(executable, NG_ERROR_TIMEOUT, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Executable unusable.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

