/*
 * $RCSfile: ngexHeartBeat.c,v $ $Revision: 1.12 $ $Date: 2008/02/14 10:19:44 $
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
 * Module of HeartBeat for Ninf-G Executable.
 */

#include "ngEx.h"

NGI_RCSID_EMBED("$RCSfile: ngexHeartBeat.c,v $ $Revision: 1.12 $ $Date: 2008/02/14 10:19:44 $")

/**
 * Data type
 */
typedef enum ngexlHeartBeatSenderType_e {
    NGEXL_HEARTBEAT_SENDER_TYPE_NONE,
    NGEXL_HEARTBEAT_SENDER_TYPE_NGEVENT,
    NGEXL_HEARTBEAT_SENDER_TYPE_SIGNAL,
    NGEXL_HEARTBEAT_SENDER_TYPE_NOMORE
} ngexlHeartBeatSenderType_t;

/**
 * Prototype declaration of internal functions.
 */
static int ngexlHeartBeatHandleEvent(
    void *arg, ngiIOhandle_t *handle,
    ngiIOhandleState_t state, ngLog_t *argLog, int *argError);
static int ngexlHeartBeatHandleEventTimeSet(
    void *arg, ngiIOhandle_t *handle,
    ngiIOhandleState_t state, ngLog_t *argLog, int *argError);
static int ngexlHeartBeatSendSignalStart(
    ngexiContext_t *context, int *error);
static int ngexlHeartBeatSendSignalStop(
    ngexiContext_t *context, int *error);
#ifndef NG_PTHREAD
static void ngexlHeartBeatSendSignalHandler(int signalNo);
#endif /* NG_PTHREAD */
static int ngexlHeartBeatSend(
    ngexiContext_t *context, ngexlHeartBeatSenderType_t type, int *error);


/**
 * HeartBeat: Initialize.
 */
int
ngexiHeartBeatInitialize(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatInitialize";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngiIOhandle_t *handle;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;

    handle = NULL;

    ngexiHeartBeatInitializeMember(heartBeatSend);

    heartBeatSend->nghbs_interval = context->ngc_heartBeatInterval;

    /* Construct the I/O handle. */
    handle = ngiIOhandleConstruct(context->ngc_event, log, error);
    if (handle == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Construct the I/O handle failed.\n");
        return 0;
    }
    heartBeatSend->nghbs_timeHandle = handle;

#ifdef NG_PTHREAD
    heartBeatSend->nghbs_isPthread = 1;
#else /* NG_PTHREAD */
    heartBeatSend->nghbs_isPthread = 0;
#endif /* NG_PTHREAD */
    
    /* Success */
    return 1;
}

/**
 * HeartBeat: Finalize.
 */
int
ngexiHeartBeatFinalize(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatFinalize";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;
    handle = heartBeatSend->nghbs_timeHandle;

    if (heartBeatSend->nghbs_continue != 0) {
        /* Stop sending heartbeat */
        result = ngexiHeartBeatStop(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "stop heartbeat failed.\n"); 
            /* not return, continue exiting */
        }
    }

    /* Destruct the I/O handle. */
    result = ngiIOhandleDestruct(handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Destruct the I/O handle failed.\n"); 
        return 0;
    }

    ngexiHeartBeatInitializeMember(heartBeatSend);

    /* Success */
    return 1;
}

/**
 * HeartBeat: Initialize the member.
 */
void
ngexiHeartBeatInitializeMember(
    ngexiHeartBeatSend_t *heartBeatSend)
{

    /* Check the arguments */
    assert(heartBeatSend != NULL);

    heartBeatSend->nghbs_isPthread = 0;
    heartBeatSend->nghbs_continue = 0;
    heartBeatSend->nghbs_handleWorking = 0;
    heartBeatSend->nghbs_signalWorking = 0;
    heartBeatSend->nghbs_timeHandle = NULL;
    heartBeatSend->nghbs_eventTime = 0;
    heartBeatSend->nghbs_interval = 0;
#ifndef NG_PTHREAD
    heartBeatSend->nghbs_actOldSet = 0;
    heartBeatSend->nghbs_sigBlockCount = 0;
    heartBeatSend->nghbs_sigBlockRead = 0;
    heartBeatSend->nghbs_sigBlockWrite = 0;
#endif /* NG_PTHREAD */
}

/**
 * HeartBeat: Start sending heartbeat.
 */
int
ngexiHeartBeatStart(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatStart";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;
    handle = heartBeatSend->nghbs_timeHandle;

    if (heartBeatSend->nghbs_interval == 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable %d: heartbeat disabled.\n",
            context->ngc_commInfo.ngci_executableID); 

        /* Do nothing */
        heartBeatSend->nghbs_continue = 0;
        heartBeatSend->nghbs_handleWorking = 0;
        heartBeatSend->nghbs_signalWorking = 0;

        return 1;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Start sending heartbeat.\n"); 
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "heartbeat is send every %d seconds.\n",
        heartBeatSend->nghbs_interval); 

    if (heartBeatSend->nghbs_isPthread != 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat is send by Ninf-G Time Event.\n"); 
    } else {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat is send by SIGALRM signal on calculating RPC.\n"); 
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat is send by Ninf-G Time Event on protocol process.\n"); 
    }

    heartBeatSend->nghbs_continue = 1;
    heartBeatSend->nghbs_handleWorking = 1;

    /* Register the Time Event callback. */
    result = ngiIOhandleTimeEventCallbackRegister(
        handle,
        ngexlHeartBeatHandleEvent, context,
        ngexlHeartBeatHandleEventTimeSet, context,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Register the Time Event to handle failed.\n"); 
        return 0;
    }

    heartBeatSend->nghbs_eventTime =
        time(NULL) + heartBeatSend->nghbs_interval;

    /* Set the first event time to handle. */
    result = ngiIOhandleTimeEventTimeChangeRequest(
        handle, log,  error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Change Time request for handle failed.\n"); 
        return 0;
    }

    if (heartBeatSend->nghbs_isPthread == 0) {

        /* Start the heartbeat send signal */
        result = ngexlHeartBeatSendSignalStart(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't start sending heartbeat.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Stop sending heartbeat.
 */
int
ngexiHeartBeatStop(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexiHeartBeatStop";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngiIOhandle_t *handle;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;
    handle = heartBeatSend->nghbs_timeHandle;

    /* Check if the heartbeat is working */
    if (heartBeatSend->nghbs_continue == 0) {

        /* Do nothing */
        return 1;
    }

    if ((heartBeatSend->nghbs_handleWorking == 0) &&
        (heartBeatSend->nghbs_signalWorking == 0)) {

        /* Do nothing */
        return 1;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Stop sending heartbeat.\n"); 

    /* Unregister the Time Event Callback. */
    result = ngiIOhandleTimeEventCallbackUnregister(
        handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unregister the Time Event to handle failed.\n"); 
        return 0;
    }

    heartBeatSend->nghbs_handleWorking = 0;

    if (heartBeatSend->nghbs_isPthread == 0) {
        /* Start the heartbeat send signal */
        result = ngexlHeartBeatSendSignalStop(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't stop sending heartbeat.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Time event arrived.
 */
static int
ngexlHeartBeatHandleEvent(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngexlHeartBeatHandleEvent";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngexiContext_t *context;
    int *error, errorEntity;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngexiContext_t *)arg;
    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (state == NGI_IOHANDLE_STATE_CANCELED) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat send event callback canceled.\n"); 

        heartBeatSend->nghbs_handleWorking = 0;

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

#ifndef NG_PTHREAD
    /* Is already writing? */
    if (heartBeatSend->nghbs_sigBlockWrite > 0) {
        time_t now;

        /**
         * If something protocol is sending, client knows
         * the Ninf-G Executable is alive.
         */
        now = time(NULL); 
        if (now >= heartBeatSend->nghbs_eventTime) {

            ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                "skip sending heartbeat by event handler."
                " other protocol is already sending.\n"); 

            heartBeatSend->nghbs_eventTime =
                now + NGEXI_HEARTBEAT_SEND_PENDING_SLEEP;
        }

        return 1;
    }
#endif /* NG_PTHREAD */

    /* Send the heartbeat. */
    result = ngexlHeartBeatSend(
        context, NGEXL_HEARTBEAT_SENDER_TYPE_NGEVENT, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Send the heartbeat failed.\n"); 

        /* Make the Executable unusable */
        result = ngexiContextUnusable(context, *error, NULL);
        if (result == 0) {
            ngLogError(log,
                NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Failed to set the executable unusable.\n");
        }

        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Time event eventTime setup.
 */
static int
ngexlHeartBeatHandleEventTimeSet(
    void *arg,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "ngexlHeartBeatHandleEventTimeSet";
    ngexiHeartBeatSend_t *heartBeatSend;
    ngexiContext_t *context;
    int *error, errorEntity;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(arg != NULL);

    context = (ngexiContext_t *)arg;
    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (state == NGI_IOHANDLE_STATE_CANCELED) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat send event time change callback canceled.\n"); 

        heartBeatSend->nghbs_handleWorking = 0;

        return 1;
    }
    assert(state == NGI_IOHANDLE_STATE_NORMAL);

    /* Set the eventTime to handle. */
    result = ngiIOhandleTimeEventTimeSet(
        heartBeatSend->nghbs_timeHandle,
        heartBeatSend->nghbs_eventTime, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Set the event time failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

#ifdef NG_PTHREAD

/**
 * HeartBeat: Start sending by signal handler. (Just error)
 */
static int
ngexlHeartBeatSendSignalStart(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSendSignalStart";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,
        "heartbeat send by signal is not exist on Pthread version.\n");

    /* Failed */
    return 0;
}

/**
 * HeartBeat: Stop sending by signal handler. (Just error)
 */
static int
ngexlHeartBeatSendSignalStop(
    ngexiContext_t *context,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSendSignalStop";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,
        "heartbeat send by signal is not exist on Pthread version.\n");

    /* Failed */
    return 0;
}

#else /* NG_PTHREAD */

/**
 * HeartBeat: Start sending by signal handler.
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
    assert(heartBeatSend->nghbs_continue != 0);
    assert(heartBeatSend->nghbs_signalWorking == 0);

    heartBeatSend->nghbs_signalWorking = 1;

    /* Initialize sigaction struct */
    actSet.sa_handler = ngexlHeartBeatSendSignalHandler;
    result = sigemptyset(&actSet.sa_mask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigemptyset() failed.\n"); 
        return 0;
    }
    actSet.sa_flags = 0; /* No option */

    /* Register the signal handler */
    result = sigaction(SIGALRM, &actSet, &heartBeatSend->nghbs_actOld);
    heartBeatSend->nghbs_actOldSet = 1;
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigaction(SIGALRM) failed.\n"); 
        return 0;
    }
    
    /* Register next signal handler invocation */
    alarmResult = alarm(heartBeatSend->nghbs_interval);
    if (alarmResult != 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "alarm() returned %u. "
            "There may possibly be already used the signal SIGALRM.\n",
            alarmResult); 
    }

    /* Success */
    return 1;
}

/**
 * HeartBeat: Stop sending by signal handler.
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

    if (heartBeatSend->nghbs_signalWorking != 0) {
        /* Success */
        return 1;
    }

    assert(heartBeatSend->nghbs_actOldSet == 1);

    /* Cancel signal handler to invoke */
    alarmResult = alarm(0);
    if (alarmResult == 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "alarm() returned %u. "
            "SIGALRM is not remained already.\n",
            alarmResult); 
    }
    heartBeatSend->nghbs_continue = 0;

    /* Register back the previous signal handler */
    result = sigaction(SIGALRM, &heartBeatSend->nghbs_actOld, &actSet);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigaction(SIGALRM) failed.\n"); 
        return 0;
    }
    heartBeatSend->nghbs_signalWorking = 0;

    /* Success */
    return 1;
}

/**
 * HeartBeat: Send by signal handler.
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
    unsigned int sleepTime;
    ngLog_t *log;
    time_t now;
    int result;

    error = &errorEntity;

    /* Get the context */
    context = ngexiContextGet(NULL, error);
    assert(context != NULL);

    log = context->ngc_log;
    heartBeatSend = &context->ngc_heartBeatSend;

    /* Check if the flag is valid */
    assert(heartBeatSend->nghbs_interval > 0);
    assert(heartBeatSend->nghbs_signalWorking != 0);

    /* Check the arguments */
    if (signalNo != SIGALRM) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "signalNo %d != SIGALRM.\n", signalNo); 
        return;
    }

    /* Check stop requested */
    if (heartBeatSend->nghbs_continue == 0) {
        return;
    }

    /* Send the heartbeat */
    result = ngexlHeartBeatSend(
        context, NGEXL_HEARTBEAT_SENDER_TYPE_SIGNAL, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Send the heartbeat failed.\n"); 

        /* Make the Executable unusable */
        result = ngexiContextUnusable(context, *error, NULL);
        if (result == 0) {
            ngLogError(log,
                NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Failed to set the executable unusable.\n");
        }

        return;
    }

    now = time(NULL);
    sleepTime = 1; /* minimum */
    if (heartBeatSend->nghbs_eventTime > now) {
        sleepTime = heartBeatSend->nghbs_eventTime - now;
    }
    assert(sleepTime > 0);
 
    /* Register next signal handler invocation */
    alarmResult = alarm(sleepTime);
    if (alarmResult != 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "alarm(%d) returned %u. "
            "Something is going wrong with SIGALRM.\n",
            sleepTime, alarmResult); 
    }

    /* Success */
    return;
}

#endif /* NG_PTHREAD */

/**
 * HeartBeat: Block or unblock sending by signal handler.
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
    int result, blockHow, isBlock, isRead;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    /* Check the arguments */
    assert(blockType > NGEXI_HEARTBEAT_SEND_NONE);
    assert(blockType < NGEXI_HEARTBEAT_SEND_NOMORE);

    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;

    assert(heartBeatSend->nghbs_sigBlockCount >= 0);
    assert(heartBeatSend->nghbs_sigBlockRead >= 0);
    assert(heartBeatSend->nghbs_sigBlockWrite >= 0);

    isBlock = 0;
    isRead = 0;
    if (blockType == NGEXI_HEARTBEAT_SEND_BLOCK_READ) {
        isBlock = 1;
        isRead = 1;
    } else if (blockType == NGEXI_HEARTBEAT_SEND_BLOCK_WRITE) {
        isBlock = 1;
        isRead = 0;
    } else if (blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK_READ) {
        isBlock = 0;
        isRead = 1;
    } else if (blockType == NGEXI_HEARTBEAT_SEND_UNBLOCK_WRITE) {
        isBlock = 0;
        isRead = 0;
    } else {
        abort();
    }

    if ((isBlock != 0) && (heartBeatSend->nghbs_sigBlockCount > 0)) {

        /* Already blocked. Countup only */
        heartBeatSend->nghbs_sigBlockCount++;
        if (isRead != 0) {
            heartBeatSend->nghbs_sigBlockRead++;
        } else {
            heartBeatSend->nghbs_sigBlockWrite++;
        }
        assert(heartBeatSend->nghbs_sigBlockCount > 0);
        assert(heartBeatSend->nghbs_sigBlockRead >= 0);
        assert(heartBeatSend->nghbs_sigBlockWrite >= 0);

        /* Success */
        return 1;
    }

    if ((isBlock == 0) && (heartBeatSend->nghbs_sigBlockCount > 1)) {

        /* Block should still remain. Countdown only */
        if (isRead != 0) {
            heartBeatSend->nghbs_sigBlockRead--;
        } else {
            heartBeatSend->nghbs_sigBlockWrite--;
        }
        heartBeatSend->nghbs_sigBlockCount--;
        assert(heartBeatSend->nghbs_sigBlockRead >= 0);
        assert(heartBeatSend->nghbs_sigBlockWrite >= 0);
        assert(heartBeatSend->nghbs_sigBlockCount > 0);

        /* Success */
        return 1;
    }

    blockHow = SIG_UNBLOCK;
    if (isBlock != 0) {
        blockHow = SIG_BLOCK;
    } else {
        blockHow = SIG_UNBLOCK;
    }
        
    result = sigemptyset(&sigSet);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigemptyset() failed.\n"); 
        return 0;
    }

    result = sigaddset(&sigSet, SIGALRM);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigaddset() failed.\n"); 
        return 0;
    }

    if (isBlock == 0) {
        if (isRead != 0) {
            heartBeatSend->nghbs_sigBlockRead--;
        } else {
            heartBeatSend->nghbs_sigBlockWrite--;
        }
        heartBeatSend->nghbs_sigBlockCount--;
        assert(heartBeatSend->nghbs_sigBlockRead == 0);
        assert(heartBeatSend->nghbs_sigBlockWrite == 0);
        assert(heartBeatSend->nghbs_sigBlockCount == 0);
    }

    if (heartBeatSend->nghbs_sigBlockCount <= 0) {
        result = sigprocmask(blockHow, &sigSet, &sigSetOld);
    }
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigprocmask() failed.\n"); 
        return 0;
    }

    if (isBlock != 0) {
        heartBeatSend->nghbs_sigBlockCount++;
        if (isRead != 0) {
            heartBeatSend->nghbs_sigBlockRead++;
        } else {
            heartBeatSend->nghbs_sigBlockWrite++;
        }
        assert(heartBeatSend->nghbs_sigBlockCount > 0);
        assert(heartBeatSend->nghbs_sigBlockRead >= 0);
        assert(heartBeatSend->nghbs_sigBlockWrite >= 0);
    }

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */


/**
 * HeartBeat: Send.
 */
static int
ngexlHeartBeatSend(
    ngexiContext_t *context,
    ngexlHeartBeatSenderType_t type,
    int *error)
{
    static const char fName[] = "ngexlHeartBeatSend";
    ngexiHeartBeatSend_t *heartBeatSend;
    char *sender;
    ngLog_t *log;
    time_t now;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(type > NGEXL_HEARTBEAT_SENDER_TYPE_NONE);
    assert(type < NGEXL_HEARTBEAT_SENDER_TYPE_NOMORE);

    heartBeatSend = &context->ngc_heartBeatSend;
    log = context->ngc_log;

    if (type == NGEXL_HEARTBEAT_SENDER_TYPE_NGEVENT) {
        sender = "event handler";
    } else if (type == NGEXL_HEARTBEAT_SENDER_TYPE_SIGNAL) {
        sender = "signal handler";
    } else {
        abort();
    }
    assert(sender != NULL);

    /**
     * Note: Sending heartbeat is done by both Ninf-G Event and
     * signal. Thus, there are cases of ignore.
     * But the losing of heartbeat send will not happen,
     * because the signal or Ninf-G Event invokes heartbeat send
     * immediately, if the signal unblocked or Ninf-G Event cond_wait()
     * entered.
     */

    now = time(NULL);

    if (now >= heartBeatSend->nghbs_eventTime) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable %d: sending heartbeat by %s.\n",
            context->ngc_commInfo.ngci_executableID,
            sender); 
     
        heartBeatSend->nghbs_eventTime =
            time(NULL) + heartBeatSend->nghbs_interval;

        result = ngexiProtocolNotifyIamAlive(
            context, context->ngc_protocol, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Send the Notify heartbeat failed.\n"); 
            return 0;
        }

    } else {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable %d: skip sending heartbeat by %s. remain %ld sec.\n",
            context->ngc_commInfo.ngci_executableID,
            sender, (long int)heartBeatSend->nghbs_eventTime - now); 
    }

    /* Success */
    return 1;
}

