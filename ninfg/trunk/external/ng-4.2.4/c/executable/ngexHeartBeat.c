#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexHeartBeat.c,v $ $Revision: 1.14 $ $Date: 2007/07/09 07:46:59 $";
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
 * Module of HeartBeat for Ninf-G Executable.
 */

#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "ngEx.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngexlHeartBeatInitializeMutexAndCond(ngexiContext_t *, int *);
static int ngexlHeartBeatFinalizeMutexAndCond(ngexiContext_t *, int *);
static int ngexlHeartBeatInitializeMember(ngexiContext_t *, int *);
#ifdef NG_PTHREAD
static void * ngexlHeartBeatSendThread(void *);
static int ngexlHeartBeatSendThreadStop(ngexiContext_t *, int *);
#else /* NG_PTHREAD */
static int ngexlHeartBeatSendSignalStart(ngexiContext_t *, int *);
static void ngexlHeartBeatSendSignalHandler(int);
static int ngexlHeartBeatSendSignalStop(ngexiContext_t *, int *);
#endif /* NG_PTHREAD */
static int ngexlHeartBeatSend(ngexiContext_t *, int *);

/**
 * HeartBeat: Initialize
 */
int
ngexiHeartBeatInitialize(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatInitialize";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    result = ngexlHeartBeatInitializeMember(context, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the heartbeat.\n", fName);
        return 0;
    }

    result = ngexlHeartBeatInitializeMutexAndCond(context, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Mutex and Condition Variable.\n", fName);
        return 0;
    }
    
    /* Success */
    return 1;
}

/**
 * HeartBeat: Finalize
 */
int
ngexiHeartBeatFinalize(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatFinalize";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    if (heartBeatSend->nghbs_continue != 0) {
        /* Stop sending heartbeat */
        result = ngexiHeartBeatStop(context, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL, "%s: stop heartbeat failed.\n",
                fName);
            /* not return, continue exiting */
        }
    }

    result = ngexlHeartBeatFinalizeMutexAndCond(context, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Mutex and Condition Variable.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Initialize the Mutex and Condition Variable.
 */
static int
ngexlHeartBeatInitializeMutexAndCond(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatInitializeMutexAndCond";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    assert(heartBeatSend->nghbs_mutexInitialized == 0);
    assert(heartBeatSend->nghbs_condInitialized == 0);

    /* Initialize the mutex */
    result = ngiMutexInitialize(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Mutex.\n", fName);
        goto error;
    }
    heartBeatSend->nghbs_mutexInitialized = 1;

    /* Initialize the condition variable */
    result = ngiCondInitialize(&heartBeatSend->nghbs_cond, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Condition variable.\n", fName);
        goto error;
    }
    heartBeatSend->nghbs_condInitialized = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Destroy the condition variable */
    if (heartBeatSend->nghbs_condInitialized != 0) {
        result = ngiCondDestroy(&heartBeatSend->nghbs_cond, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Condition variable.\n", fName);
        }
    }
    heartBeatSend->nghbs_condInitialized = 0;

    /* Destroy the mutex */
    if (heartBeatSend->nghbs_mutexInitialized != 0) {
        result = ngiMutexDestroy(&heartBeatSend->nghbs_mutex, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Mutex.\n", fName);
        }
    }
    heartBeatSend->nghbs_mutexInitialized = 0;

    /* Failed */
    return 0;
}

/**
 * HeartBeat: Finalize the Mutex and Condition Variable.
 */
static int
ngexlHeartBeatFinalizeMutexAndCond(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatFinalizeMutexAndCond";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Destroy the condition variable */
    if (heartBeatSend->nghbs_condInitialized != 0) {
        result = ngiCondDestroy(&heartBeatSend->nghbs_cond, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Condition variable.\n", fName);
        }
    }
    heartBeatSend->nghbs_condInitialized = 0;

    /* Destroy the mutex */
    if (heartBeatSend->nghbs_mutexInitialized != 0) {
        result = ngiMutexDestroy(&heartBeatSend->nghbs_mutex, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Mutex.\n", fName);
        }
    }
    heartBeatSend->nghbs_mutexInitialized = 0;

    /* Success */
    return 1;
}

/**
 * HeartBeat: Initialize the member.
 */
static int
ngexlHeartBeatInitializeMember(
    ngexiContext_t *context,
    int *error)
{
    ngexiHeartBeatSend_t *heartBeatSend;

    /* Check the arguments */
    assert(context != NULL);

    heartBeatSend = &context->ngc_heartBeatSend;

    /* nghbs_interval was already set from argv. Don't clear */

    heartBeatSend->nghbs_continue = 0;
    heartBeatSend->nghbs_stopped = 0;
    heartBeatSend->nghbs_mutexInitialized = 0;
    heartBeatSend->nghbs_condInitialized = 0;
    heartBeatSend->nghbs_actOldSet = 0;
    heartBeatSend->nghbs_sigBlockCount = 0;
    heartBeatSend->nghbs_hbPid = 0;

    /* Success */
    return 1;
}

/**
 * HeartBeat: Start sending
 */
int
ngexiHeartBeatStart(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatStart";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    if (heartBeatSend->nghbs_interval == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Executable %d: heartbeat disabled.\n", fName,
            context->ngc_commInfo.ngci_executableID);

        /* Do nothing */
        heartBeatSend->nghbs_continue = 0; /* FALSE */
        heartBeatSend->nghbs_stopped = 0;  /* FALSE */

        return 1;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Start sending heartbeat.\n", fName);
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: heartbeat is send every %d seconds.\n",
        fName, heartBeatSend->nghbs_interval);

#ifdef NG_PTHREAD
    {
        heartBeatSend->nghbs_continue = 1; /* TRUE */
        heartBeatSend->nghbs_stopped = 0;  /* FALSE */
     
        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: heartbeat is send by send thread.\n", fName);

        /* Create the heartbeat send thread */
        result = globus_thread_create(&heartBeatSend->nghbs_thread, NULL,
            ngexlHeartBeatSendThread, context);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't create thread for send heartbeat.\n", fName);
            return 0;
        }
    }
#else /* NG_PTHREAD */
    {
        heartBeatSend->nghbs_continue = 1; /* TRUE */
        heartBeatSend->nghbs_stopped = 0;  /* FALSE */
     
        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: heartbeat is send by SIGALRM signal.\n", fName);

        /* Start the heartbeat send signal */
        result = ngexlHeartBeatSendSignalStart(context, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't start sending heartbeat.\n", fName);
            return 0;
        }
    }
#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}

/**
 * HeartBeat: Stop sending
 */
int
ngexiHeartBeatStop(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatStop";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the heartbeat is working */
    if (heartBeatSend->nghbs_continue == 0) {

        /* Do nothing */
        return 1;
    }
    if (heartBeatSend->nghbs_stopped != 0) {

        /* Do nothing */
        return 1;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Stop sending heartbeat.\n", fName);

#ifdef NG_PTHREAD
    {
        /* Stop the heartbeat send thread */
        result = ngexlHeartBeatSendThreadStop(context, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't stop the thread to send heartbeat.\n", fName);
            return 0;
        }
    }
#else /* NG_PTHREAD */
    {
        /* Start the heartbeat send signal */
        result = ngexlHeartBeatSendSignalStop(context, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't stop sending heartbeat.\n", fName);
            return 0;
        }
    }
#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}


#ifdef NG_PTHREAD
/**
 * HeartBeat: Send thread
 */
static void *
ngexlHeartBeatSendThread(void *threadArgument)
{
    static const char fName[] = "ngexlHeartBeatSendThread";
    int result, wasTimedOut, mutexLocked;
    ngexiHeartBeatSend_t *heartBeatSend;
    int errorEntity, *error;
    ngexiContext_t *context;
    ngLog_t *log;

    /* Check the arguments */
    assert(threadArgument != NULL);

    context = (ngexiContext_t *)threadArgument;
    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;
    error = &errorEntity;
    mutexLocked = 0;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_stopped == 0);

    heartBeatSend->nghbs_hbPid = getpid();

    /**
     * Do periodic heartbeat send.
     */

    /* Lock */
    result = ngiMutexLock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    /* Wait the status */
    while (heartBeatSend->nghbs_continue) {
        result = ngiCondTimedWait(
            &heartBeatSend->nghbs_cond, &heartBeatSend->nghbs_mutex,
            heartBeatSend->nghbs_interval, &wasTimedOut, log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }

        /* Do periodic proceedings */
        if (wasTimedOut == 1) {
            /* Send the heartbeat */
            result = ngexlHeartBeatSend(context, error);
            if (result != 1) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Send heartbeat failed.\n", fName);
                goto error;
            }
        }
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        goto error;
    }

    /**
     * Tell the Main Thread that, heartbeat thread was stopped.
     */

    /* Lock */
    result = ngiMutexLock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    /* Set the status */
    heartBeatSend->nghbs_stopped = 1; /* TRUE */

    /* Notify signal */
    result = ngiCondSignal(&heartBeatSend->nghbs_cond, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        goto error;
    }

    /* Success */
    return NULL;

error:
    /* Set the status */
    heartBeatSend->nghbs_stopped = 1; /* TRUE */

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&heartBeatSend->nghbs_mutex, log, NULL);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
        }
    }

    /* Make the Executable unusable */
    result = ngexiContextUnusable(context, *error, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Failed to set the executable unusable.\n",
            fName);
    }

    /* Failed */
    return NULL;
}

/**
 * HeartBeat Stop the send thread
 */
static int
ngexlHeartBeatSendThreadStop(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSendThreadStop";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_continue == 1);

    /**
     * Tell the HeartBeat thread to stop
     */

    /* Lock */
    result = ngiMutexLock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Set the status */
    heartBeatSend->nghbs_continue = 0; /* to stop */

    /* Notify signal */
    result = ngiCondSignal(&heartBeatSend->nghbs_cond, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /**
     * Suppress unlock and lock, to ignore CondSignal(stopped) issue,
     * before the CondWait(stopped) starts its proceedings.
     */

    /**
     * Wait the HeartBeat thread to stop
     */

    /* Suppress lock. already locked */

    /* Wait the status */
    while (!heartBeatSend->nghbs_stopped) {
        result = ngiCondWait(
            &heartBeatSend->nghbs_cond, &heartBeatSend->nghbs_mutex,
            log, error);
        if (result != 1) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }
    }

    /* Unlock */
    result = ngiMutexUnlock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
error:
    /* Unlock */
    result = ngiMutexUnlock(&heartBeatSend->nghbs_mutex, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

#else /* NG_PTHREAD */

/**
 * HeartBeat: Start sending by signal handler
 */
static int
ngexlHeartBeatSendSignalStart(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSendSignalStart";
    ngexiHeartBeatSend_t *heartBeatSend;
    struct sigaction actSet;
    unsigned int alarmResult;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_continue == 1);
    assert(heartBeatSend->nghbs_stopped == 0);

    /* Initialize sigaction struct */
    actSet.sa_handler = ngexlHeartBeatSendSignalHandler;
    result = sigemptyset(&actSet.sa_mask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigemptyset() failed.\n", fName);
        return 0;
    }
    actSet.sa_flags = 0; /* No option */

    /* Register the signal handler */
    result = sigaction(SIGALRM, &actSet, &heartBeatSend->nghbs_actOld);
    heartBeatSend->nghbs_actOldSet = 1;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigaction(SIGALRM) failed.\n", fName);
        return 0;
    }
    
    /* Register next signal handler invocation */
    alarmResult = alarm(heartBeatSend->nghbs_interval);
    if (alarmResult != 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING,
            NULL, "%s: alarm() returned %u. "
            "There may possibly be already used the signal SIGALRM.\n",
            fName, alarmResult);
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Send by signal handler
 */
static void
ngexlHeartBeatSendSignalHandler(
    int signalNo)
{
    static const char fName[] = "ngexlHeartBeatSendSignalHandler";
    ngexiHeartBeatSend_t *heartBeatSend;
    unsigned int alarmResult;
    ngexiContext_t *context;
    int errorEntity, *error;
    ngLog_t *log;
    int result;

    error = &errorEntity;

    /* Get the context */
    context = ngexiContextGet(NULL, error);
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_stopped == 0);

    /* Check the arguments */
    if (signalNo != SIGALRM) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: signalNo %d != SIGALRM.\n",
            fName, signalNo);
        return;
    }

    /* Check stop requested */
    if (!heartBeatSend->nghbs_continue) {
        return;
    }

    /* Send the heartbeat */
    result = ngexlHeartBeatSend(context, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Send the heartbeat failed.\n", fName);

        /* Make the Executable unusable */
        result = ngexiContextUnusable(context, *error, NULL);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Failed to set the executable unusable.\n",
                fName);
        }

        return;
    }
 
    /* Register next signal handler invocation */
    alarmResult = alarm(heartBeatSend->nghbs_interval);
    if (alarmResult != 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING,
            NULL, "%s: alarm() returned %u. "
            "Something is going wrong with SIGALRM.\n",
            fName, alarmResult);
    }

    /* Success */
    return;
}

/**
 * HeartBeat: Stop sending by signal handler
 */
static int
ngexlHeartBeatSendSignalStop(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSendSignalStop";
    ngexiHeartBeatSend_t *heartBeatSend;
    struct sigaction actSet;
    unsigned int alarmResult;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_continue == 1);
    assert(heartBeatSend->nghbs_stopped == 0);

    assert(heartBeatSend->nghbs_actOldSet == 1);

    /* Cancel signal handler to invoke */
    alarmResult = alarm(0);
    if (alarmResult == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
            NULL, "%s: alarm() returned %u. "
            "SIGALRM is not remained already.\n",
            fName, alarmResult);
    }
    heartBeatSend->nghbs_continue = 0;

    /* Register back the previous signal handler */
    result = sigaction(SIGALRM, &heartBeatSend->nghbs_actOld, &actSet);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigaction(SIGALRM) failed.\n", fName);
        return 0;
    }
    heartBeatSend->nghbs_stopped = 1;

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */

/**
 * HeartBeat: Block or unblock sending by signal handler
 */
int
ngexiHeartBeatSendBlock(
    ngexiContext_t *context,
    ngexiHeartBeatSendBlockType_t blockType,
    int *error)
#ifdef NG_PTHREAD
{
    /* Only effective for NonThread version */

    /* Do nothing */
    return 1;
}
#else /* NG_PTHREAD */
{
    static const char fName[] = "ngexiHeartBeatSendBlock";
    ngexiHeartBeatSend_t *heartBeatSend;
    sigset_t sigSet, sigSetOld;
    int result, blockHow;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    /* Check the arguments */
    assert((blockType == NGEXI_HEARTBEAT_SEND_BLOCK) ||
        (blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK));

    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;

    assert(heartBeatSend->nghbs_sigBlockCount >= 0);

    if ((blockType == NGEXI_HEARTBEAT_SEND_BLOCK) &&
        (heartBeatSend->nghbs_sigBlockCount > 0)) {

        /* Already blocked. Countup only */
        heartBeatSend->nghbs_sigBlockCount++;
        assert(heartBeatSend->nghbs_sigBlockCount > 0);

        /* Success */
        return 1;
    }

    if ((blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK) &&
        (heartBeatSend->nghbs_sigBlockCount > 1)) {

        /* Block should still remain. Countdown only */
        heartBeatSend->nghbs_sigBlockCount--;
        assert(heartBeatSend->nghbs_sigBlockCount > 0);

        /* Success */
        return 1;
    }

    blockHow = (blockType == NGEXI_HEARTBEAT_SEND_BLOCK ? SIG_BLOCK :
               (blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK ? SIG_UNBLOCK :
               (abort(), 0)));
        
    result = sigemptyset(&sigSet);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigemptyset() failed.\n", fName);
        return 0;
    }

    result = sigaddset(&sigSet, SIGALRM);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigaddset() failed.\n", fName);
        return 0;
    }

    if (blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK) {
        heartBeatSend->nghbs_sigBlockCount--;
    }

    if (heartBeatSend->nghbs_sigBlockCount <= 0) {
        result = sigprocmask(blockHow, &sigSet, &sigSetOld);
    }
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: sigprocmask() failed.\n", fName);
        return 0;
    }

    if (blockType == NGEXI_HEARTBEAT_SEND_BLOCK) {
        heartBeatSend->nghbs_sigBlockCount++;
    }

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */


/**
 * HeartBeat Send
 */
static int
ngexlHeartBeatSend(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSend";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
        NULL, "%s: Executable %d: sending heartbeat.\n", fName,
        context->ngc_commInfo.ngci_executableID);

    result = ngexiProtocolNotifyIamAlive(
        context, context->ngc_protocol, log, error);
    if (result != 1) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Send the Notify heartbeat failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

