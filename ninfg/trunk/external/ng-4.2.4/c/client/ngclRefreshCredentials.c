#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclRefreshCredentials.c,v $ $Revision: 1.5 $ $Date: 2005/06/21 04:49:58 $";
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
 * Module of RefreshCredentials for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngcllRefreshCredentialsIntervalGet(ngclContext_t *, int *, int *);
static int ngcllRefreshCredentialsEvent(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllRefreshCredentialsEventTimeSet(
    ngclContext_t *, ngcliObserveItem_t *, time_t, int *);
static int ngcllRefreshCredentials(ngclContext_t *, int *);

/**
 * RefreshCredentials: Initialize
 */
int
ngcliRefreshCredentialsInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliRefreshCredentialsInitialize";
    ngcliObserveItem_t *observeItem;
    int result, interval;

    /* Check the arguments */
    assert(context != NULL);

    context->ngc_refreshCredentialsInfo = NULL;

    /* Get the refresh credentials interval */
    result = ngcllRefreshCredentialsIntervalGet(context, &interval, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the refresh credentials interval.\n", fName);
        return 0;
    }

    if (interval <= 0) {
        /* not used */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: refresh credentials feature is not used.\n", fName);
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Initialize the Refresh Credentials module.\n", fName);

    /* Construct */
    observeItem = ngcliObserveItemConstruct(
        context, &context->ngc_observe, error);
    if (observeItem == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the ObserveItem.\n", fName);
        return 0;
    }

    context->ngc_refreshCredentialsInfo = observeItem;

    observeItem->ngoi_eventTime = time(NULL) + interval;
    observeItem->ngoi_interval = interval;
    observeItem->ngoi_eventFunc = ngcllRefreshCredentialsEvent;
    observeItem->ngoi_eventTimeSetFunc = ngcllRefreshCredentialsEventTimeSet;

    /* Success */
    return 1;
}

/**
 * RefreshCredentials: Finalize
 */
int
ngcliRefreshCredentialsFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliRefreshCredentialsFinalize";
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observeItem = context->ngc_refreshCredentialsInfo;
    if (observeItem == NULL) {
        /* not used */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finalize the Refresh Credentials module.\n", fName);

    context->ngc_refreshCredentialsInfo = NULL;

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
 * RefreshCredentials: Get the refresh interval
 */
static int
ngcllRefreshCredentialsIntervalGet(
    ngclContext_t *context,
    int *interval,
    int *error)
{
    static const char fName[] = "ngcllRefreshCredentialsIntervalGet";
    ngclLocalMachineInformationManager_t *lmInfoMng;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(interval != NULL);

    log = context->ngc_log;
    *interval = 0;

    /* Lock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Local Machine Information.\n", fName);
        return 0;
    }

    /* Get refresh proxy cred interval */
    lmInfoMng = ngcliLocalMachineInformationCacheGet(context, error);
    if (lmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Local Machine Information.\n", fName);
        goto error;
    }

    /* set the interval of refresh proxy cert */
    *interval = lmInfoMng->nglmim_info.nglmi_refreshInterval;

    /* Unlock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Local Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Local Machine Information.\n",
            fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * RefreshCredentials: ObserveThread event arrived
 */
static int
ngcllRefreshCredentialsEvent(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllRefreshCredentialsEvent";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    /* Perform the refreshCredentialsInfo */
    result = ngcllRefreshCredentials(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Refreshing credentials failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * RefreshCredentials: ObserveThread eventTime setup
 */
static int
ngcllRefreshCredentialsEventTimeSet(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    time_t now,
    int *error)
{   
    static const char fName[] = "ngcllRefreshCredentialsEventTimeSet";

    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    if (observeItem->ngoi_eventTimeChangeRequested != 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: interval won't be changed.\n",
            fName);
    }

    /* Check interval */
    if (observeItem->ngoi_interval <= 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: interval is wrong (%d).\n",
            fName, observeItem->ngoi_interval);
    }

    /* Set next eventTime */
    if (observeItem->ngoi_eventExecuted != 0) {
        observeItem->ngoi_eventTime = now + observeItem->ngoi_interval;
    }

    /* Success */
    return 1;
}

/**
 * RefreshCredentials: Perform Refresh Credentials
 */
static int
ngcllRefreshCredentials(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllRefreshCredentials";
    ngcliJobManager_t *jobManager;
    int result;
    static gss_cred_id_t ng_gss_credential = GSS_C_NO_CREDENTIAL;

    /* Check the arguments */
    assert(context != NULL);

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: refresh credentials.\n", fName);

    /* acquire credential */
    result = ngcliGlobusAcquireCredential(&ng_gss_credential,
        context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't acquire proxy credential.\n", fName);
        return 0;
    }

    /* Reset credential */
    result = ngcliJobSetCredential(ng_gss_credential, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set proxy credential.\n", fName);
        return 0;
    }

    /* lock the list of JobManager */
    result = ngcliContextJobManagerListReadLock(context, context->ngc_log,
        error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of JobManager.\n", fName);
        return 0;
    }

    jobManager = NULL; /* retrieve head item */
    while ((jobManager = ngcliContextGetNextJobManager(
        context, jobManager, error)) != NULL) {

	/* refresh credential of JobManager */
	result = ngcliJobRefreshCredential(jobManager, error);
	if (result != 1) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_WARNING, NULL,
		"%s: Failed to refresh credential.\n", fName);
        }
    }

    result = ngcliContextJobManagerListReadUnlock(context, context->ngc_log,
        error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of JobManager.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: refresh credentials done.\n", fName);

    /* Success */
    return 1;
}

