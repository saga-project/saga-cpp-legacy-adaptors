/*
 * $RCSfile: ngisRefreshCredential.c,v $ $Revision: 1.6 $ $Date: 2008/02/26 06:32:39 $
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
 * Refresh Credential for Invoke Server.
 */

#include "ngEnvironment.h"
#ifdef NG_PTHREAD

#include "ngInvokeServer.h"

NGI_RCSID_EMBED("$RCSfile: ngisRefreshCredential.c,v $ $Revision: 1.6 $ $Date: 2008/02/26 06:32:39 $")

/**
 * Macro.
 */
#define NGISL_REFRESH_CREDENTIALS_SLEEP_WAIT_SEC (60 * 60)

/**
 * Prototype declaration of internal functions.
 */
static void ngislRefreshCredentialInitializeMember(
    ngisiRefreshCredential_t *ref);
static int ngislRefreshCredentialThreadStart(
    ngisiContext_t *context, ngisiRefreshCredential_t *ref, int *error);
static int ngislRefreshCredentialThreadStop(
    ngisiContext_t *context, ngisiRefreshCredential_t *ref, int *error);
static void * ngislRefreshCredentialThread(void *arg);

/**
 * RefreshCredential: Initialize.
 */
int
ngisiRefreshCredentialInitialize(
    ngisiContext_t *context,
    ngisiRefreshCredential_t *ref,
    int *error)
{
    static const char fName[] = "ngisiRefreshCredentialInitialize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(ref != NULL);

    ngislRefreshCredentialInitializeMember(ref);

    result = ngisiMutexInitialize(
        context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the mutex failed.\n");
        return 0;
    }
    ref->ngisrc_mutexInitialized = 1;

    result = ngisiCondInitialize(
        context, &ref->ngisrc_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the cond failed.\n");
        return 0;
    }
    ref->ngisrc_condInitialized = 1;

    ref->ngisrc_updateInterval = 0; /* disable at first */

    ref->ngisrc_continue = 0; /* Thread is not invoked at first. */
    ref->ngisrc_working = 0;

    /* Success */
    return 1;
}

/**
 * RefreshCredential: Finalize.
 */
int
ngisiRefreshCredentialFinalize(
    ngisiContext_t *context,
    ngisiRefreshCredential_t *ref,
    int *error)
{
    static const char fName[] = "ngisiRefreshCredentialFinalize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(ref != NULL);

    if ((ref->ngisrc_continue != 0) ||
        (ref->ngisrc_working != 0)) {
        result = ngislRefreshCredentialThreadStop(
            context, ref, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Can't stop the Refresh Credential thread.\n");
            return 0;
        }
    }

    if (ref->ngisrc_condInitialized != 0) {
        ref->ngisrc_condInitialized = 0;
        result = ngisiCondFinalize(
            context, &ref->ngisrc_cond, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Finalize the cond failed.\n");
            return 0;
        }
    }

    if (ref->ngisrc_mutexInitialized != 0) {
        ref->ngisrc_mutexInitialized = 0;
        result = ngisiMutexFinalize(
            context, &ref->ngisrc_mutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Finalize the mutex failed.\n");
            return 0;
        }
    }

    ngislRefreshCredentialInitializeMember(ref);

    /* Success */
    return 1;
}

/**
 * RefreshCredential: Initialize member.
 */
static void
ngislRefreshCredentialInitializeMember(
    ngisiRefreshCredential_t *ref)
{
    /* Check the arguments */
    assert(ref != NULL);

    ref->ngisrc_updateInterval = 0;
    ref->ngisrc_nextEventTime = 0;

    ref->ngisrc_continue = 0;
    ref->ngisrc_working = 0;

    ref->ngisrc_mutexInitialized = 0;
    ref->ngisrc_condInitialized = 0;
}

/**
 * RefreshCredential: Set update interval.
 */
int
ngisiRefreshCredentialUpdateIntervalSet(
    ngisiContext_t *context,
    ngisiRefreshCredential_t *ref,
    int newInterval,
    int *error)
{
    static const char fName[] = "ngisiRefreshCredentialUpdateIntervalSet";
    time_t oldEventTime, nextEventTime;
    int mutexLocked, result, notify;
    int oldInterval;

    /* Check the arguments */
    assert(context != NULL);
    assert(ref != NULL);

    mutexLocked = 0;
    notify = 0;

    /* Lock */
    result = ngisiMutexLock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    oldInterval = ref->ngisrc_updateInterval;
    oldEventTime = ref->ngisrc_nextEventTime;

    if (newInterval != oldInterval) {
        notify = 1;

        if (newInterval > 0) {
            ref->ngisrc_updateInterval = newInterval;

            /* Select the early event time */
            nextEventTime = time(NULL) + newInterval;
            if ((oldEventTime > 0) && (nextEventTime > oldEventTime)) {
                nextEventTime = oldEventTime;
            }
            ref->ngisrc_nextEventTime = nextEventTime;
        } else {
            ref->ngisrc_updateInterval = 0;
            ref->ngisrc_nextEventTime = 0;
        }

        /* Notify signal */
        result = ngisiCondBroadcast(
            context, &ref->ngisrc_cond, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Cond signal failed.\n");
            goto error;
        }
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    if ((notify != 0) && (newInterval > 0) &&
        (ref->ngisrc_continue == 0) && (ref->ngisrc_working == 0)) {
        /* Start the thread */
        result = ngislRefreshCredentialThreadStart(
            context, ref, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Can't start the Refresh Credential thread.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}


/**
 * RefreshCredential: Start thread.
 */
static int
ngislRefreshCredentialThreadStart(
    ngisiContext_t *context,
    ngisiRefreshCredential_t *ref,
    int *error)
{
    static const char fName[] = "ngislRefreshCredentialThreadStart";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(ref != NULL);

    if (ref->ngisrc_working != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Refresh Credential thread is already working.\n");
        return 0;
    }

    /* Check if the flag is valid */
    assert(ref->ngisrc_continue == 0);

    ref->ngisrc_continue = 1; /* TRUE */

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Create the Refresh Credential thread.\n");

    /* Create the Refresh Credential thread */
    result = globus_thread_create(
        &ref->ngisrc_thread, NULL,
        ngislRefreshCredentialThread, context);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Can't create thread for Refresh Credential.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * RefreshCredential: Stop thread.
 */
static int
ngislRefreshCredentialThreadStop(
    ngisiContext_t *context,
    ngisiRefreshCredential_t *ref,
    int *error)
{
    static const char fName[] = "ngislRefreshCredentialThreadStop";
    int mutexLocked, result;

    /* Check the arguments */
    assert(context != NULL);
    assert(ref != NULL);

    mutexLocked = 0;

    if (ref->ngisrc_working == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "Refresh Credential thread is already stopped.\n");
        return 1;
    }


    /* Check if the flag is valid */
    assert(ref->ngisrc_continue != 0);

    /**
     * Tell the Refresh Credential thread to stop.
     */

    /* Lock */
    result = ngisiMutexLock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    /* Set the status */
    ref->ngisrc_continue = 0; /* to stop */

    /* Notify signal */
    result = ngisiCondBroadcast(
        context, &ref->ngisrc_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Cond signal failed.\n");
        goto error;
    }

    /**
     * Suppress unlock and lock, to ignore CondBroadcast(!working) issue,
     * before the CondWait(!working) starts its proceedings.
     */

    /**
     * Wait the Refresh Credential thread to stop.
     */

    /* Suppress lock. already locked */

    /* Wait the status */
    while (ref->ngisrc_working != 0) {
        result = ngisiCondWait(context,
            &ref->ngisrc_cond, &ref->ngisrc_mutex,
            error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Can't wait the Condition Variable.\n");
            goto error;
        }
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * RefreshCredential: thread.
 */
static void *
ngislRefreshCredentialThread(
    void *arg)
{
    static const char fName[] = "ngislRefreshCredentialThread";
    ngisiRefreshCredential_t *ref;
    int sleepSec, wasTimeout;
    ngisiContext_t *context;
    int mutexLocked, result;
    int *error, errorEntity;
    time_t now, timeoutTime;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngisiContext_t *)arg;
    ref = &context->ngisc_refreshCredential;
    error = &errorEntity;
    *error = 0;

    /* Check if the flag is valid */
    assert(ref->ngisrc_working == 0);

    mutexLocked = 0;
    ref->ngisrc_working = 1; /* TRUE */
    sleepSec = 0;

    /* Log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Refresh Credential thread invoked.\n");

    /* Lock */
    result = ngisiMutexLock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    sleepSec = 0; /* Do not wait at first. */

    /* Wait the status */
    while (ref->ngisrc_continue != 0) {
        wasTimeout = 0;

        /* Wait the event time. */
        if (sleepSec > 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
                "sleep %d sec for next update.\n", sleepSec);

            result = ngisiCondTimedWait(context,
                &ref->ngisrc_cond, &ref->ngisrc_mutex,
                sleepSec, &wasTimeout, error);
            if (result == 0) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "Can't wait the Condition Variable.\n");
                goto error;
            }
        }

        now = time(NULL);
        timeoutTime = ref->ngisrc_nextEventTime;

        /* Perform refresh credential. */
        if ((timeoutTime > 0) && (now >= timeoutTime)) {

            result = ngisiJobRefreshCredential(context, error);
            if (result == 0) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "Update the credential failed.\n");
                /* Not return */
            }

            if (ref->ngisrc_updateInterval > 0) {
                ref->ngisrc_nextEventTime = now + ref->ngisrc_updateInterval;
            } else {
                ref->ngisrc_nextEventTime = 0;
            }
        } 

        timeoutTime = ref->ngisrc_nextEventTime;
        sleepSec = NGISL_REFRESH_CREDENTIALS_SLEEP_WAIT_SEC;
        if (timeoutTime > 0) {
            sleepSec = timeoutTime - time(NULL);
        }
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /**
     * Tell the Main Thread that, Refresh Credential thread was stopped.
     */

    /* Log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Refresh Credential thread exiting.\n");

    /* Lock */
    result = ngisiMutexLock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    /* Set the status */
    ref->ngisrc_working = 0; /* FALSE */

    /* Notify signal */
    result = ngisiCondBroadcast(
        context, &ref->ngisrc_cond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Cond signal failed.\n");
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return NULL;

    /* Error occurred */
error:
    /* Set the status */
    ref->ngisrc_working = 0; /* FALSE */

    /* Log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Refresh Credential thread exiting by error.\n");

    /* Unlock */
    if (mutexLocked != 0) {
        /* Notify signal */
        result = ngisiCondBroadcast(
            context, &ref->ngisrc_cond, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Cond signal failed.\n");
        }

        mutexLocked = 0;
        result = ngisiMutexUnlock(context, &ref->ngisrc_mutex, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the mutex failed.\n");
        }
    }

    /* Failed */
    return NULL;
}

#endif /* NG_PTHREAD */

