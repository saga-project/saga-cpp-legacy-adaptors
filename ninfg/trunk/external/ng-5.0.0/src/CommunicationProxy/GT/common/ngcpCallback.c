/*
 * $RCSfile: ngcpCallback.c,v $ $Revision: 1.3 $ $Date: 2008/03/28 03:52:30 $
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
#include "ngcpCallback.h"
#include "ngemList.h"
#include "ngUtility.h"
#include "ngemLog.h"

NGI_RCSID_EMBED("$RCSfile: ngcpCallback.c,v $ $Revision: 1.3 $ $Date: 2008/03/28 03:52:30 $")

/* Pair of callback function and user's data */
typedef struct ngcplCallbackPair_s {
    ngcpCallbackFunc_t  ngcp_function;
    void               *ngcp_userData;
} ngcplCallbackPair_t;

NGEM_DECLARE_LIST_OF(ngcplCallbackPair_t);

/*
 * Callback Manager
 */
typedef struct ngcplCallback_s {
    NGEM_LIST_OF(ngcplCallbackPair_t) ngc_queue;
    int                               ngc_pipe[2];
    ngiRlock_t                        ngc_rlock;
    ngemCallbackResult_t              ngc_result;
    ngemCallback_t                    ngc_callback;
} ngcplCallback_t;

bool            ngcplCallbackInitialized = false;
ngcplCallback_t ngcplCallback;

static void ngcplReadCallback(void *, int, void *, size_t, ngemCallbackResult_t);

/**
 * Must be called in main thread;
 */
ngemResult_t
ngcpCallbackInitialize(void)
{
    ngLog_t *log = NULL;
    int result;
    bool callbackInitialized = false;
    ngemResult_t nResult;
    int i;
    NGEM_FNAME(ngcpCallbackInitialize);

    log = ngemLogGetDefault();

    if (ngcplCallbackInitialized) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Callback is already initialized.\n");
        return NGEM_FAILED;
    }
    /* Initialize */
    NGEM_LIST_SET_INVALID_VALUE(&ngcplCallback.ngc_queue);
    ngcplCallback.ngc_rlock   = NGI_RLOCK_NULL;
    ngcplCallback.ngc_result  = NGEM_CALLBACK_RESULT_SUCCESS;
    ngcplCallback.ngc_pipe[0] = -1;
    ngcplCallback.ngc_pipe[1] = -1;

    NGEM_LIST_INITIALIZE(ngcplCallbackPair_t, &ngcplCallback.ngc_queue);

    result = ngiRlockInitialize(&ngcplCallback.ngc_rlock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't initialize a rlock.\n");
        goto error;
    }
    result = pipe(ngcplCallback.ngc_pipe);
    if (result < 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "pipe: %s.\n", strerror(errno));
        goto error;
    }

    nResult = ngemCallbackManagerInitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't initialize the callback.\n");
        goto error;
    }
    callbackInitialized = true;

    ngcplCallback.ngc_callback = ngemCallbackRead(
        ngcplCallback.ngc_pipe[0], ngcplReadCallback, &ngcplCallback);
    if (ngemCallbackIsValid(ngcplCallback.ngc_callback) == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't register for reading form pipe.\n");
        goto error;
    }
    ngemCallbackSetDaemon(ngcplCallback.ngc_callback);

    ngcplCallbackInitialized = true;

    return NGEM_SUCCESS;
error:
    if (callbackInitialized) {
        callbackInitialized = false;
        nResult = ngemCallbackManagerFinalize();
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                "Can't finalize the callback.\n");
        }
    }
    for (i = 0;i < 2;++i) {
        if (ngcplCallback.ngc_pipe[i] >= 0) {
            result = close(ngcplCallback.ngc_pipe[i]);
            if (result < 0) {
                ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                    "close: %s.\n", strerror(errno));
            }
        }
    }
    NGEM_LIST_FINALIZE(ngcplCallbackPair_t, &ngcplCallback.ngc_queue);
    result = ngiRlockFinalize(&ngcplCallback.ngc_rlock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't finalize a rlock.\n");
    }

    return NGEM_FAILED;
}

/**
 * Must be called in main thread;
 */
ngemResult_t
ngcpCallbackFinalize(void)
{
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_LIST_ITERATOR_OF(ngcplCallbackPair_t) it;
    ngcplCallbackPair_t *val;
    ngemResult_t nResult;
    int i;
    int result;
    NGEM_FNAME(ngcpCallbackFinalize);

    log = ngemLogGetDefault();

    if (!ngcplCallbackInitialized) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Callback has not been initialized.\n");
        return NGEM_FAILED;
    }

    NGEM_LIST_FOREACH(ngcplCallbackPair_t, &ngcplCallback.ngc_queue, it, val) {
        NGEM_ASSERT(val != NULL);
        NGEM_ASSERT(val->ngcp_function != NULL);
        val->ngcp_function(val->ngcp_userData, NGEM_CALLBACK_RESULT_CANCEL);
        NGI_DEALLOCATE(ngcplCallbackPair_t, val, log, NULL);
    }
    NGEM_LIST_FINALIZE(ngcplCallbackPair_t, &ngcplCallback.ngc_queue);

    for (i = 0;i < 2;++i) {
        if (ngcplCallback.ngc_pipe[i] >= 0) {
            result = close(ngcplCallback.ngc_pipe[i]);
            if (result < 0) {
                ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                    "close: %s.\n", strerror(errno));
                ret = NGEM_FAILED;
            }
            ngcplCallback.ngc_pipe[i] = -1;
        }
    }

    result = ngiRlockFinalize(&ngcplCallback.ngc_rlock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't finalize a rlock.\n");
        ret = NGEM_FAILED;
    }

    NGEM_LIST_SET_INVALID_VALUE(&ngcplCallback.ngc_queue);
    ngcplCallback.ngc_rlock   = NGI_RLOCK_NULL;
    ngcplCallback.ngc_pipe[0] = -1;
    ngcplCallback.ngc_pipe[1] = -1;

    nResult = ngemCallbackManagerFinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't finalize the callback.\n");
        ret = NGEM_FAILED;
    }

    ngcplCallbackInitialized = false;

    return ret;
}

/**
 * Executes callback function in callback manager system.
 * This function is called on another thread.
 */
ngemResult_t
ngcpCallbackOneshot(
    ngcpCallbackFunc_t callback,
    void *arg)
{
    ngcplCallbackPair_t *pair = NULL;
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_FAILED;
    ngemResult_t nResult;
    int result;
    bool locked = false;
    NGEM_FNAME(ngcpCallbackOneshot);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&ngcplCallback.ngc_rlock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't lock the callback.\n");
        goto finalize;
    }
    locked = true;
    if (ngcplCallback.ngc_result != NGEM_CALLBACK_RESULT_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Invalid status. Callback is not valid.\n");
        goto finalize;
    }

    pair = NGI_ALLOCATE(ngcplCallbackPair_t, log, NULL);
    if (pair == NULL) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't allocate storage for the callback pair.\n");
        goto finalize;
    }
    pair->ngcp_function = callback;
    pair->ngcp_userData = arg;

    result = write(ngcplCallback.ngc_pipe[1], "C", 1);
    if (result < 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't push the queue.\n");
        goto finalize;
    }

    nResult = NGEM_LIST_INSERT_TAIL(ngcplCallbackPair_t,
        &ngcplCallback.ngc_queue, pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't push the queue.\n");
        goto finalize;
    }
    pair = NULL;

    ret = NGEM_SUCCESS;
finalize:

    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&ngcplCallback.ngc_rlock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                "Can't unlock the callback.\n");
            ret = NGEM_FAILED;
        }
    }
    result = NGI_DEALLOCATE(ngcplCallbackPair_t, pair, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't allocate storage for the callback pair.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Callback function for ngcpCallbackOneshot().
 */
static void
ngcplReadCallback(
    void *userData,
    int fd,
    void *data,
    size_t nRead,
    ngemCallbackResult_t cResult)
{
    ngLog_t *log;
    int result;
    bool locked = false;
    NGEM_LIST_ITERATOR_OF(ngcplCallbackPair_t) it;
    ngcplCallbackPair_t *val;
    NGEM_FNAME(ngcplReadCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_CALLBACK, fName,
        "Called.\n");

    result = ngiRlockLock(&ngcplCallback.ngc_rlock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't lock the callback.\n");
        goto finalize;
    }
    locked = true;

    ngcplCallback.ngc_result = cResult;
    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
            "Can't read from the pipe.\n");
        break;
    case NGEM_CALLBACK_RESULT_EOF:
        ngLogInfo(log, NGCP_LOGCAT_CALLBACK, fName, "EOF\n");
        break;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGCP_LOGCAT_CALLBACK, fName,
            "Callback is canceled.\n");
        break;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    while (!NGEM_LIST_IS_EMPTY(ngcplCallbackPair_t, &ngcplCallback.ngc_queue)) {
        it = NGEM_LIST_BEGIN(ngcplCallbackPair_t, &ngcplCallback.ngc_queue);
        NGEM_ASSERT(it != NULL);
        val = NGEM_LIST_GET(ngcplCallbackPair_t, it);
        NGEM_ASSERT(val != NULL);
        NGEM_ASSERT(val->ngcp_function != NULL);

        /* Callback */
        ngLogDebug(log, NGCP_LOGCAT_CALLBACK, fName,
            "Calls Callback function.\n");
        val->ngcp_function(val->ngcp_userData, cResult);

        NGI_DEALLOCATE(ngcplCallbackPair_t, val, log, NULL);
        NGEM_LIST_ERASE(ngcplCallbackPair_t, it);
    }

    if (cResult == NGEM_CALLBACK_RESULT_SUCCESS) {
        ngcplCallback.ngc_callback = ngemCallbackRead(
            ngcplCallback.ngc_pipe[0], ngcplReadCallback, &ngcplCallback);
        if (ngemCallbackIsValid(ngcplCallback.ngc_callback) == 0) {
            ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                "Can't register for reading form pipe.\n");
            goto finalize;
        }
        ngemCallbackSetDaemon(ngcplCallback.ngc_callback);
    }

finalize:
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&ngcplCallback.ngc_rlock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCP_LOGCAT_CALLBACK, fName,
                "Can't unlock the callback.\n");
        }
    }
    return;
}
