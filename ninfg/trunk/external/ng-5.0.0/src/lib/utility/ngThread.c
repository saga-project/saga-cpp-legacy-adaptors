/*
 * $RCSfile: ngThread.c,v $ $Revision: 1.18 $ $Date: 2008/03/19 08:43:47 $
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
 * Thread module for Ninf-G.
 */

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngThread.c,v $ $Revision: 1.18 $ $Date: 2008/03/19 08:43:47 $")

#ifdef NG_PTHREAD

/**
 * Thread by Pthread version implementation.
 */

/**
 * Prototype declaration of internal functions.
 */

/**
 * External data.
 */
const ngiMutex_t  ngiMutexNull  = {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED};
const ngiCond_t   ngiCondNull   = {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED};

const ngiRWlock_t ngiRWlockNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    0, /* ngrwl_nWaitingWriteLock */
    0, /* ngrwl_readLockCounter   */
    0  /* ngrwl_writeLockCounter  */
};

const ngiRlock_t ngiRlockNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    0, /* ngrl_lockLevel    */
    0, /* ngrl_nLockWaiter  */
    0, /* ngrl_nSigWaiter   */
    0  /* ngrl_signalNumber */
};

/**
 * Functions.
 */

/**
 * Thread: Create.
 */
int
ngiThreadCreate(
    ngiThread_t *thread,
    void *(threadFunction)(void *),
    void *threadArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadCreate";
    int result;

    /* Check the arguments */
    if (thread == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        goto error;
    }

    result = pthread_create(
        thread, NULL,
        threadFunction, (void *)threadArgument);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "%s failed: %d: %s.\n",
            "pthread_create()", result, strerror(result)); 
        goto error;
    }
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Thread: Join.
 */
int
ngiThreadJoin(
    ngiThread_t *thread,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadJoin";
    int result;

    /* Check the arguments */
    if (thread == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        goto error;
    }

    result = pthread_join(*thread, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "%s failed: %d: %s.\n",
            "pthread_join()", result, strerror(result)); 
        goto error;
    }
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Thread: Yield.
 */
int
ngiThreadYield(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    /* Event can be NULL for Pthread version. */

    sched_yield();
        
    /* Success */
    return 1;
}

/**
 * Thread: Equal.
 */
int
ngiThreadEqual(
    ngiThread_t *thread1,
    ngiThread_t *thread2,
    int *isEqual,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadEqual";
    int result;

    /* Check the arguments */
    if ((thread1 == NULL) || (thread2 == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        goto error;
    }
    if (isEqual == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. isEqual is NULL.\n"); 
        goto error;
    }

    *isEqual = 0;

    result = pthread_equal(*thread1, *thread2);
    if (result != 0) {
        *isEqual = 1;
    }
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Thread: Self.
 */
int
ngiThreadSelf(
    ngiThread_t *threadResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadSelf";

    /* Check the arguments */
    if (threadResult == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        goto error;
    }

    *threadResult = pthread_self();
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Mutex: Initialize
 */
int
ngiMutexInitialize(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiMutexInitialize";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Mutex is already initialized.\n");
        return 0;
    }

    result = pthread_mutex_init(&mutex->ngm_mutex, NULL);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
            "pthread_mutex_init failed: %s.\n", strerror(result));
	return 0;
    }
    mutex->ngm_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Success */
    return 1;
}

/**
 * Mutex: Destroy
 */
int
ngiMutexDestroy(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiMutexDestroy";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if (mutex->ngm_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Mutex is not initialized.\n");
        /* Success */
        return 1;
    }
    
    if (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    mutex->ngm_status = NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED;
    result = pthread_mutex_destroy(&mutex->ngm_mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "pthread_mutex_destroy failed: %s.\n", strerror(result));
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Lock
 */
int
ngiMutexLock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiMutexLock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Mutex is not initialized.\n");
        return 0;
    }

    result = pthread_mutex_lock(&mutex->ngm_mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_mutex_lock failed: %s.\n", strerror(result));
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Try lock
 */
int
ngiMutexTryLock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiMutexTryLock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Mutex is not initialized.\n");
        return 0;
    }

    result = pthread_mutex_trylock(&mutex->ngm_mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_mutex_trylock failed: %s.\n", strerror(result));
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Unlock
 */
int
ngiMutexUnlock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiMutexUnlock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Mutex is not initialized.\n");
        return 0;
    }

    result = pthread_mutex_unlock(&mutex->ngm_mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_mutex_unlock failed : %s.\n", strerror(result));
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Condition variable: Initialize
 */
int
ngiCondInitialize(
    ngiCond_t *cond,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondInitialize";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    /* Event can be NULL for Pthread version. */

    if (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is already initialized.\n");
        return 0;
    }

    result = pthread_cond_init(&cond->ngc_cond, NULL);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_init failed: %s.\n", strerror(result));
	return 0;
    }
    cond->ngc_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Success */
    return 1;
}

/**
 * Condition variable: Destroy
 */
int
ngiCondDestroy(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondDestroy";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (cond->ngc_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is not initialized.\n");
        /* Success */
        return 1;
    }

    if (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    cond->ngc_status = NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED;
    result = pthread_cond_destroy(&cond->ngc_cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_destroy failed: %s.\n", strerror(result));
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Wait
 */
int
ngiCondWait(
    ngiCond_t *cond,
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondWait";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is not initialized.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Mutex is not initialized.\n");
        return 0;
    }

    result = pthread_cond_wait(&cond->ngc_cond, &mutex->ngm_mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_wait failed: %s.\n", strerror(result));
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Timed wait
 */
int
ngiCondTimedWait(
    ngiCond_t *cond,
    ngiMutex_t *mutex,
    int relativeSec,
    int *timeout,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondTimedWait";
    struct timeval tv;
    struct timespec abstime;

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is not initialized.\n");
        return 0;
    }


    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Mutex is not initialized.\n");
        return 0;
    }

    /* Get current time */
    result = gettimeofday(&tv, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "gettimeofday failed: %s.\n", strerror(errno));
	return 0;
    }

    /* Calculate absolute time */
    abstime.tv_sec = tv.tv_sec + relativeSec;
    abstime.tv_nsec = tv.tv_usec * 1000;

    result = pthread_cond_timedwait(&cond->ngc_cond, &mutex->ngm_mutex, &abstime);
    /* Time out? */
    if (result == ETIMEDOUT) {
	*timeout = 1;
	return 1;
    }

    /* Error? */
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_timedwait failed: %s.\n",
	    strerror(result));
	return 0;
    }

    /* Success */
    *timeout = 0;
    return 1;
}

/**
 * Condition variable: Signal
 */
int
ngiCondSignal(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondSignal";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is not initialized.\n");
        return 0;
    }

    result = pthread_cond_signal(&cond->ngc_cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_signal failed: %s.\n", strerror(result));
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Broadcast
 */
int
ngiCondBroadcast(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondBroadcast";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to condition variable.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Condition variable is not initialized.\n");
        return 0;
    }

    result = pthread_cond_broadcast(&cond->ngc_cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "pthread_cond_broadcast failed: %s.\n", strerror(result));
	return 0;
    }

    return 1;
}

/**
 * Read/Write Lock: Initialize
 */
int
ngiRWlockInitialize(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRWlockInitialize";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Read/Write lock is already initialized.\n");
        return 0;
    }

    *rwLock = ngiRWlockNull;
    rwLock->ngrwl_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Initialize the mutex */
    result = ngiMutexInitialize(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't initialize the Mutex.\n");
        goto error;
    }

    /* Initialize the condition variable */
    result = ngiCondInitialize(&rwLock->ngrwl_condWrite, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't initialize the Condition Variable.\n");
        goto error;
    }

    /* Initialize the condition variable */
    result = ngiCondInitialize(&rwLock->ngrwl_condRead, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't initialize the Condition Variable.\n");
        goto error;
    }

    /* Initialize other variables */
    rwLock->ngrwl_nWaitingWriteLock = 0;
    rwLock->ngrwl_readLockCounter   = 0;
    rwLock->ngrwl_writeLockCounter  = 0;

    /* Success */
    return 1;
error:
    result = ngiRWlockFinalize(rwLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't finalize the RWlock.\n");
    }
    *rwLock = ngiRWlockNull;

    return 0;
}

/**
 * Read/Write Lock: Finalize
 */
int
ngiRWlockFinalize(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiRWlockFinalize";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
	    "RWlock is not initialized.\n");
        /* Success */
        return 1;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    assert(rwLock->ngrwl_readLockCounter >= 0);
    assert(rwLock->ngrwl_writeLockCounter >= 0);
    if ((rwLock->ngrwl_readLockCounter> 0) &&
        (rwLock->ngrwl_writeLockCounter > 0)) {
        NGI_SET_ERROR(error, NG_ERROR_THREAD);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, "RWlock is locked.\n");
        error = NULL;
        ret = 0;
    }

    /* Destroy the conditional variable */
    result = ngiCondDestroy(&rwLock->ngrwl_condRead, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Can't destroy the Condition Variable.\n");
        error = NULL;
        ret = 0;
    }

    /* Destroy the conditional variable */
    result = ngiCondDestroy(&rwLock->ngrwl_condWrite, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Can't destroy the Condition Variable.\n");
        error = NULL;
        ret = 0;
    }

    /* Destroy the mutex */
    result = ngiMutexDestroy(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Can't destroy the Mutex.\n");
        error = NULL;
        ret = 0;
    }
    *rwLock = ngiRWlockNull;

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Read lock
 */
int
ngiRWlockReadLock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    ngiThread_t self;
    int result;
    int locked = 0;
    int ret = 1;
    int isEqual;
    static const char fName[] = "ngiRWlockReadLock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "RWlock is not initialized.\n");
        return 0;
    }

    result = ngiThreadSelf(&self, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't get current thread id.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the Mutex.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }
    locked = 1;

    /* Does it locking for write? */
    while (rwLock->ngrwl_writeLockCounter > 0) {
        result = ngiThreadEqual(&rwLock->ngrwl_owner, &self, &isEqual, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't compare thread id.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
        if (isEqual != 0) {
            break;
        }

    	result = ngiCondWait(
	    &rwLock->ngrwl_condRead, &rwLock->ngrwl_mutex, log, error);
	if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    	"Can't wait the Condition Variable.\n");
            error = NULL;
            ret = 0;
            goto finalize;
	}
    }

    /* Does counter over? */
    if (rwLock->ngrwl_readLockCounter >= NG_RWLOCK_MAX_LOCKS) {
    	NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "The counter of Read/Write Lock overflowed.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }

    /* Increment the counter */
    rwLock->ngrwl_readLockCounter++;

finalize:
    /* Unlock the mutex */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the Mutex.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Read/Write Lock: Read unlock
 */
int
ngiRWlockReadUnlock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    int result;
    int locked = 0;
    int ret = 1;
    ngiCond_t *cond = NULL;
    static const char fName[] = "ngiRWlockReadUnlock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "RWlock is not initialized.\n");
        return 0;
    }

    /* lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the Mutex.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }
    locked = 1;

    /* Does anybody locking for read? */
    if (rwLock->ngrwl_readLockCounter <= 0) {
        /* Nobody locking for read */
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Nobody locking for read.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }

    /* Decrement the counter */
    rwLock->ngrwl_readLockCounter--;

    /* Does nobody locking? */
    if ((rwLock->ngrwl_readLockCounter == 0) &&
        (rwLock->ngrwl_writeLockCounter == 0)) {
	/* Wakeup any threads, which waiting */
	if (rwLock->ngrwl_nWaitingWriteLock > 0) {
            cond = &rwLock->ngrwl_condWrite;
        } else {
            cond = &rwLock->ngrwl_condRead;
        }
        result = ngiCondBroadcast(cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't broadcast the Condition Variable.\n");
            error = NULL;
            ret = 0;
            goto finalize;
        }
    }

finalize:
    /* Unlock the mutex */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the Mutex.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Read/Write Lock: Write lock
 */
int
ngiRWlockWriteLock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    ngiThread_t self;
    int result;
    int ret = 1;
    int incNwait = 0;
    int locked = 0;
    int isEqual;
    static const char fName[] = "ngiRWlockWriteLock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "RWlock is not initialized.\n");
        return 0;
    }

    result = ngiThreadSelf(&self, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't get current thread id.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the Mutex.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    /* Does it locking? */
    rwLock->ngrwl_nWaitingWriteLock++;
    incNwait = 1;

    while ((rwLock->ngrwl_writeLockCounter > 0) ||
           (rwLock->ngrwl_readLockCounter > 0)) {
        if (rwLock->ngrwl_readLockCounter == 0) {
            result = ngiThreadEqual(&rwLock->ngrwl_owner, &self, &isEqual, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                    "Can't compare thread id.\n");
                ret = 0;
                error = NULL;
                goto finalize;
            }
            if (isEqual != 0) {
                break;
            }
        }
    	result = ngiCondWait(
	    &rwLock->ngrwl_condWrite, &rwLock->ngrwl_mutex, log, error);
	if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    	"Can't wait the Condition Variable.\n");
            ret = 0;
            error = NULL;
            goto finalize;
	}
    }

    /* Decrement the counter */
    rwLock->ngrwl_writeLockCounter++;
    rwLock->ngrwl_owner = self;

finalize:
    /* Dec */
    if (incNwait != 0) {
        incNwait = 0;
        rwLock->ngrwl_nWaitingWriteLock--;
    }
    /* Unlock the mutex */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the Mutex.\n");
            ret = 0;
            error = NULL;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Read/Write Lock: Write unlock
 */
int
ngiRWlockWriteUnlock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    int locked = 0;
    ngiCond_t *cond = NULL;
    static const char fName[] = "ngiRWlockWriteUnlock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "RWlock is not initialized.\n");
        return 0;
    }

    /* lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the Mutex.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }
    locked = 1;

    /* Does anybody locking for write? */
    if (rwLock->ngrwl_writeLockCounter == 0) {
        /* Nobody locking for write */
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Nobody locking for write.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }

    /* Does anybody locking for write? */
    if (rwLock->ngrwl_readLockCounter > 0) {
        /* Nobody locking for write */
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Invalid status: Read locked when Write Unlock.\n");
        error = NULL;
        ret = 0;
        goto finalize;
    }

    /* Increment the counter */
    rwLock->ngrwl_writeLockCounter--;

    /* Wakeup any threads, which waiting */
    if (rwLock->ngrwl_writeLockCounter == 0) {
        if (rwLock->ngrwl_nWaitingWriteLock > 0) {
            cond = &rwLock->ngrwl_condWrite;
        } else {
            cond = &rwLock->ngrwl_condRead;
        }
        result = ngiCondBroadcast(cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't broadcast the Condition Variable.\n");
            error = NULL;
            ret = 0;
            goto finalize;
        }
    }

finalize:

    /* Unlock the mutex */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the Mutex.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Reentrant lock: initialize
 */
int
ngiRlockInitialize(
    ngiRlock_t *rlock,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRlockInitialize";

    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    /* Event can be NULL for Pthread version. */

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Rlock is already initialized.\n");
        return 0;
    }

    *rlock = NGI_RLOCK_NULL;

    result = ngiMutexInitialize(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't initialize the mutex.\n");
        goto error;
    }
    result = ngiCondInitialize(&rlock->ngrl_condLock, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't initialize the conditional variable.\n");
        goto error;
    }
    result = ngiCondInitialize(&rlock->ngrl_condSignal, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't initialize the conditional variable.\n");
        goto error;
    }
    rlock->ngrl_lockLevel     = 0;
    rlock->ngrl_nLockWaiter   = 0;
    rlock->ngrl_nSignalWaiter = 0;
    rlock->ngrl_signalNumber  = 0;

    rlock->ngrl_status  = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    return 1;
error:
    result = ngiMutexDestroy(&rlock->ngrl_mutex, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the mutex.\n");
    }
    result = ngiCondDestroy(&rlock->ngrl_condLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the conditional variable.\n");
    }
    result = ngiCondDestroy(&rlock->ngrl_condSignal, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the conditional variable.\n");
    }

    *rlock = NGI_RLOCK_NULL;

    return 0;
}

/**
 * Reentrant lock: finalize
 */
int
ngiRlockFinalize(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiRlockFinalize";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Rlock is not initialized.\n");
        /* Success */
        return 1;
    }
    
    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    if (rlock->ngrl_lockLevel != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Rlock is locked.\n");
        ret = 0;
        error = NULL;
    }

    result = ngiMutexDestroy(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the mutex.\n");
        ret = 0;
        error = NULL;
    }
    result = ngiCondDestroy(&rlock->ngrl_condLock, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the conditional variable.\n");
        ret = 0;
        error = NULL;
    }
    result = ngiCondDestroy(&rlock->ngrl_condSignal, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't destroy the conditional variable.\n");
        ret = 0;
        error = NULL;
    }

    *rlock = NGI_RLOCK_NULL;

    return ret;
}

/**
 * Reentrant lock: Lock
 */
int
ngiRlockLock(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    ngiThread_t self;
    int isEqual = 0;
    int ret = 1;
    int result;
    int waiting = 0;
    int locked = 0;
    static const char fName[] = "ngiRlockLock";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Rlock is not initialized.\n");
        return 0;
    }

    result = ngiThreadSelf(&self, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't get current thread id.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    result = ngiMutexLock(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the synchronized variable.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    rlock->ngrl_nLockWaiter++;
    waiting = 1;/* true */
    while (rlock->ngrl_lockLevel > 0) {
        result = ngiThreadEqual(&rlock->ngrl_owner, &self, &isEqual, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't compare thread id.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
        if (isEqual != 0) {
            break;
        }
        result = ngiCondWait(&rlock->ngrl_condLock, &rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't wait the synchronized variable.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }
    waiting = 0;
    rlock->ngrl_nLockWaiter--;

    rlock->ngrl_lockLevel++;
    rlock->ngrl_owner = self;

finalize:
    if (waiting != 0) {
        assert(locked != 0);
        rlock->ngrl_nLockWaiter--;
    }
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the synchronized object.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Reentrant lock: Unlock
 */
int
ngiRlockUnlock(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    int locked = 0;
    static const char fName[] = "ngiRlockUnlock";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Rlock is not initialized.\n");
        return 0;
    }

    result = ngiMutexLock(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the synchronized variable.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    if (rlock->ngrl_lockLevel <= 0) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Nobody locking.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    rlock->ngrl_lockLevel--;

    if ((rlock->ngrl_lockLevel == 0) && (rlock->ngrl_nLockWaiter > 0)) {
        result = ngiCondBroadcast(&rlock->ngrl_condLock, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't broadcast the signal for the synchronized variable.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the synchronized object.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Reentrant lock: Wait 
 */
int
ngiRlockWait(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    return ngiRlockTimedWait(rlock, -1, NULL, log, error);
}

int
ngiRlockTimedWait(
    ngiRlock_t *rlock,
    int relativeSec,
    int *timeout,
    ngLog_t *log,
    int *error)
{
    int lockLevel = 0;
    int signalNumber = 0;
    ngiThread_t self;
    int isEqual = 0;
    int ret = 1;
    int result;
    int sigWaiting = 0;
    int lockWaiting = 0;
    int locked = 0;
    int runlocked = 0;
    time_t endTime = -1;
    time_t remain;
    int localTimeout;
    static const char fName[] = "ngiRlockTimedWait";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    if ((relativeSec >= 0) && (timeout == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to timeout.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Rlock is not initialized.\n");
        return 0;
    }


    if (relativeSec >= 0) {
        *timeout = 0;
        endTime = time(NULL) + relativeSec;
    }

    /* Lock */
    result = ngiMutexLock(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the mutex.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    /* Mutex must have been locked */
    assert(rlock->ngrl_lockLevel >= 0);
    if (rlock->ngrl_lockLevel == 0) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Rlock has not been locked.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    result = ngiThreadSelf(&self, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't get thread id.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    
    result = ngiThreadEqual(&rlock->ngrl_owner, &self, &isEqual, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Can't compare thread id.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    if (!isEqual) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName,
	    "Rlock's owner is not this thread..\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Unlock */
    lockLevel = rlock->ngrl_lockLevel;
    rlock->ngrl_lockLevel = 0;
    runlocked = 1;
    if (rlock->ngrl_nLockWaiter > 0) {
        result = ngiCondBroadcast(&rlock->ngrl_condLock, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't broadcast the signal for the synchronized variable.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }

    /* Wait Signal */
    signalNumber = rlock->ngrl_signalNumber;
    rlock->ngrl_nSignalWaiter++;
    sigWaiting = 1;
    while (signalNumber == rlock->ngrl_signalNumber) {
        if (relativeSec < 0) {
            assert(endTime < 0);
            result = ngiCondWait(
                &rlock->ngrl_condSignal, &rlock->ngrl_mutex, log, error);
        } else {
            assert(endTime >= 0);
            remain = endTime - time(NULL);
            if (remain < 0) {
                *timeout = 1;
                break;
            }
            result = ngiCondTimedWait(
                &rlock->ngrl_condSignal, &rlock->ngrl_mutex, remain, &localTimeout, log, error);
        }
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't wait the signal.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
        if (relativeSec >= 0) {
            if (localTimeout != 0) {
                *timeout = 1;
                break;
            }
        }
    }
    sigWaiting = 0;
    rlock->ngrl_nSignalWaiter--;

    /* Wait Unlock */
    rlock->ngrl_nLockWaiter++;
    lockWaiting = 1;
    while (rlock->ngrl_lockLevel > 0) {
        result = ngiCondWait(
            &rlock->ngrl_condLock, &rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't wait the signal.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }
    lockWaiting = 0;
    rlock->ngrl_nLockWaiter--;

finalize:
    if (sigWaiting != 0) {
        rlock->ngrl_nSignalWaiter--;
    }

    if (lockWaiting != 0) {
        rlock->ngrl_nLockWaiter--;
    }

    if (runlocked != 0) {
        assert(rlock->ngrl_lockLevel == 0);
        rlock->ngrl_lockLevel = lockLevel;
        rlock->ngrl_owner = self;
        runlocked = 0;
    }

    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the synchronized object.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Reentrant lock: Broadcast
 */
int
ngiRlockBroadcast(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    int locked = 0;
    static const char fName[] = "ngiRlockBroadcast";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Rlock is not initialized.\n");
        return 0;
    }

    result = ngiMutexLock(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
	    "Can't lock the mutex.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    if (rlock->ngrl_nSignalWaiter > 0) {
        rlock->ngrl_signalNumber++;
        result = ngiCondBroadcast(&rlock->ngrl_condSignal, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't broadcast the signal for the synchronized variable.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&rlock->ngrl_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "Can't unlock the synchronized object.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

#else /* NG_PTHREAD */

/**
 * Thread by NonThread version implementation.
 */

/**
 * Prototype declaration of internal functions.
 */

/**
 * External data.
 */
const ngiMutex_t ngiMutexNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    0 /* ngm_lockCount */
};

const ngiCond_t ngiCondNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    0, /* ngc_waiting */
    0  /* ngc_signaled */
};

const ngiRWlock_t ngiRWlockNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    0,  /* ngrwl_readLockCount */
    0   /* ngrwl_writeLockCount */
};

const ngiRlock_t ngiRlockNull = {
    NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
    0  /* ngrl_lockCount */
};

/**
 * Functions.
 */

/**
 * NonThread: Thread: Create.
 */
int
ngiThreadCreate(
    ngiThread_t *thread,
    void *(threadFunction)(void *),
    void *threadArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadCreate";

    /* Check the arguments */
    if (thread == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        return 0;
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "Thread create for NonThread is not exist.\n");
        
    /* Failed */
    return 0;
}

/**
 * NonThread: Thread: Join.
 */
int
ngiThreadJoin(
    ngiThread_t *thread,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadJoin";

    /* Check the arguments */
    if (thread == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        return 0;
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "Thread create for NonThread is not exist.\n");
        
    /* Failed */
    return 0;
}

/**
 * NonThread: Thread: Yield.
 */
int
ngiThreadYield(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadYield";
    int result;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The event is NULL.\n"); 
        goto error;
    }

    result = ngiEventNonThreadYield(event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Yield to ngEvent failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * NonThread: Thread: Equal.
 */
int
ngiThreadEqual(
    ngiThread_t *thread1,
    ngiThread_t *thread2,
    int *isEqual,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadEqual";

    /* Check the arguments */
    if ((thread1 == NULL) || (thread2 == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        return 0;
    }
    if (isEqual == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. isEqual is NULL.\n"); 
        return 0;
    }
        
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "Thread equal for NonThread is not exist.\n");
        
    /* Failed */
    return 0;
}

/**
 * NonThread: Thread: Self.
 */
int
ngiThreadSelf(
    ngiThread_t *threadResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadSelf";

    /* Check the arguments */
    if (threadResult == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The thread is NULL.\n"); 
        return 0;
    }

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "Thread self for NonThread is not exist.\n");
        
    /* Failed */
    return 0;
}

/**
 * NonThread: Mutex: Initialize
 */
int
ngiMutexInitialize(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMutexInitialize";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Mutex is already initialized.\n");
        return 0;
    }

    mutex->ngm_lockCount = 0;

    mutex->ngm_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Success */
    return 1;
}

/**
 * NonThread: Mutex: Destroy
 */
int
ngiMutexDestroy(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMutexDestroy";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if (mutex->ngm_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "Mutex is not initialized.\n");
        /* Success */
        return 1;
    }
    
    if (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    mutex->ngm_status = NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED;

    if (mutex->ngm_lockCount != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "lock count %d is not zero.\n", mutex->ngm_lockCount);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NonThread: Mutex: Lock
 */
int
ngiMutexLock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMutexLock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Mutex is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    mutex->ngm_lockCount++;

    /* Success */
    return 1;
}

/**
 * NonThread: Mutex: Try lock
 */
int
ngiMutexTryLock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMutexTryLock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Mutex is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    mutex->ngm_lockCount++;

    /* Success */
    return 1;
}

/**
 * NonThread: Mutex: Unlock
 */
int
ngiMutexUnlock(
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMutexUnlock";

    /* Check the arguments */
    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Mutex is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    mutex->ngm_lockCount--;

    /* Success */
    return 1;
}

/**
 * NonThread: Condition variable: Initialize
 */
int
ngiCondInitialize(
    ngiCond_t *cond,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCondInitialize";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. event is NULL.\n");
        return 0;
    }

    if (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is already initialized.\n");
        return 0;
    }

    cond->ngc_event = event;
    cond->ngc_waiting = 0;
    cond->ngc_signaled = 0;

    cond->ngc_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Success */
    return 1;
}

/**
 * NonThread: Condition variable: Destroy
 */
int
ngiCondDestroy(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCondDestroy";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (cond->ngc_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is not initialized.\n");
        /* Success */
        return 1;
    }

    if (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    cond->ngc_status = NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED;

    cond->ngc_event = NULL;
    cond->ngc_waiting = 0;
    cond->ngc_signaled = 0;

    return 1;
}

/**
 * NonThread: Condition variable: Wait
 */
int
ngiCondWait(
    ngiCond_t *cond,
    ngiMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondWait";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is not initialized.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Mutex is not initialized.\n");
        return 0;
    }

    cond->ngc_signaled = 0;
    cond->ngc_waiting = 1;

    result = ngiEventNonThreadCondTimedWait(
        cond->ngc_event, &cond->ngc_signaled,
        NGI_EVENT_NON_THREAD_COND_NO_TIMEOUT, 0, NULL, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Cond wait by Event failed.\n");
        return 0;
    }

    cond->ngc_waiting = 0;

    return 1;
}

/**
 * NonThread: Condition variable: Timed wait
 */
int
ngiCondTimedWait(
    ngiCond_t *cond,
    ngiMutex_t *mutex,
    int relativeSec,
    int *timeout,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondTimedWait";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if (mutex == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to mutex.\n");
        return 0;
    }

    if (timeout == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to timeout.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is not initialized.\n");
        return 0;
    }

    if ((mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) && 
        (mutex->ngm_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Mutex is not initialized.\n");
        return 0;
    }

    *timeout = 0;

    cond->ngc_signaled = 0;
    cond->ngc_waiting = 1;

    result = ngiEventNonThreadCondTimedWait(
        cond->ngc_event, &cond->ngc_signaled,
        NGI_EVENT_NON_THREAD_COND_WITH_TIMEOUT, relativeSec, timeout,
        log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Cond timed wait by Event failed.\n");
        return 0;
    }

    cond->ngc_waiting = 0;

    /* Success */
    return 1;
}

/**
 * NonThread: Condition variable: Signal
 */
int
ngiCondSignal(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCondSignal";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is not initialized.\n");
        return 0;
    }

    cond->ngc_signaled = 1;

    return 1;
}

/**
 * NonThread: Condition variable: Broadcast
 */
int
ngiCondBroadcast(
    ngiCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCondBroadcast";

    /* Check the arguments */
    if (cond == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to condition variable.\n");
        return 0;
    }

    if ((cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) &&
        (cond->ngc_status != NGI_SYNCHRONIZED_OBJECT_STATUS_STATIC)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Condition variable is not initialized.\n");
        return 0;
    }

    cond->ngc_signaled = 1;

    return 1;
}

/**
 * NonThread: Read/Write Lock: Initialize
 */
int
ngiRWlockInitialize(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockInitialize";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Read/Write lock is already initialized.\n");
        return 0;
    }

    *rwLock = ngiRWlockNull;
    rwLock->ngrwl_status = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Initialize other variables */
    rwLock->ngrwl_readLockCount = 0;
    rwLock->ngrwl_writeLockCount = 0;

    /* Success */
    return 1;
}

/**
 * NonThread: Read/Write Lock: Finalize
 */
int
ngiRWlockFinalize(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockFinalize";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "RWlock is not initialized.\n");
        /* Success */
        return 1;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    *rwLock = ngiRWlockNull;

    /* Success */
    return 1;
}

/**
 * NonThread: Read/Write Lock: Read lock
 */
int
ngiRWlockReadLock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockReadLock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "RWlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rwLock->ngrwl_readLockCount++;

    /* Success */
    return 1;
}

/**
 * NonThread: Read/Write Lock: Read unlock
 */
int
ngiRWlockReadUnlock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockReadUnlock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "RWlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rwLock->ngrwl_readLockCount--;

    /* Success */
    return 1;
}

/**
 * NonThread: Read/Write Lock: Write lock
 */
int
ngiRWlockWriteLock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockWriteLock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "RWlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rwLock->ngrwl_writeLockCount++;

    /* Success */
    return 1;
}

/**
 * NonThread: Read/Write Lock: Write unlock
 */
int
ngiRWlockWriteUnlock(
    ngiRWlock_t *rwLock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRWlockWriteUnlock";

    /* Check the arguments */
    if (rwLock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to RWlock.\n");
        return 0;
    }

    if (rwLock->ngrwl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "RWlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rwLock->ngrwl_writeLockCount--;

    /* Success */
    return 1;
}

/**
 * NonThread: Reentrant lock: initialize
 */
int
ngiRlockInitialize(
    ngiRlock_t *rlock,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRlockInitialize";

    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. The event is NULL.\n"); 
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Rlock is already initialized.\n");
        return 0;
    }

    *rlock = NGI_RLOCK_NULL;

    result = ngiMutexInitialize(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the mutex.\n");
        goto error;
    }
    result = ngiCondInitialize(&rlock->ngrl_cond, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize the conditional variable.\n");
        goto error;
    }

    rlock->ngrl_status  = NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED;

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngiMutexDestroy(&rlock->ngrl_mutex, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destroy the mutex.\n");
    }
    result = ngiCondDestroy(&rlock->ngrl_cond, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destroy the conditional variable.\n");
    }

    *rlock = NGI_RLOCK_NULL;

    /* Failed */
    return 0;
}

/**
 * NonThread: Reentrant lock: finalize
 */
int
ngiRlockFinalize(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiRlockFinalize";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status == NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "Rlock is not initialized.\n");
        /* Success */
        return 1;
    }
    
    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid status.\n");
        return 0;
    }

    if (rlock->ngrl_lockCount != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "lock count %d is not zero.\n", rlock->ngrl_lockCount);
        ret = 0;
        error = NULL;
    }

    result = ngiMutexDestroy(&rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destroy the mutex.\n");
        ret = 0;
        error = NULL;
    }
    result = ngiCondDestroy(&rlock->ngrl_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destroy the conditional variable.\n");
        ret = 0;
        error = NULL;
    }

    *rlock = NGI_RLOCK_NULL;

    return ret;
}

/**
 * NonThread: Reentrant lock: Lock
 */
int
ngiRlockLock(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRlockLock";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Rlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rlock->ngrl_lockCount++;

    /* Success */
    return 1;
}

/**
 * NonThread: Reentrant lock: Unlock
 */
int
ngiRlockUnlock(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRlockUnlock";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Rlock is not initialized.\n");
        return 0;
    }

    /* Lock is not performed for NonThread version. */

    rlock->ngrl_lockCount--;

    /* Success */
    return 1;
}

/**
 * NonThread: Reentrant lock: Wait 
 */
int
ngiRlockWait(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRlockWait";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Rlock is not initialized.\n");
        return 0;
    }

    /* Just wait by NonThread CondWait. */
    result = ngiCondWait(
        &rlock->ngrl_cond, &rlock->ngrl_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Wait the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NonThread: Reentrant lock: Wait 
 */
int
ngiRlockTimedWait(
    ngiRlock_t *rlock,
    int relativeSec,
    int *timeout,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRlockTimedWait";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Rlock is not initialized.\n");
        return 0;
    }

    /* Just wait by NonThread CondWait. */
    result = ngiCondTimedWait(
        &rlock->ngrl_cond, &rlock->ngrl_mutex,
        relativeSec, timeout, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Wait the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NonThread: Reentrant lock: Broadcast
 */
int
ngiRlockBroadcast(
    ngiRlock_t *rlock,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiRlockBroadcast";

    /* Check the arguments */
    if (rlock == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid pointer to Rlock.\n");
        return 0;
    }

    if (rlock->ngrl_status != NGI_SYNCHRONIZED_OBJECT_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Rlock is not initialized.\n");
        return 0;
    }

    /* Just signal by NonThread CondBroadcast. */
    result = ngiCondBroadcast(
        &rlock->ngrl_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Broadcast the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

#endif /* NG_PTHREAD */

/**
 * Thread: Sleep.
 * This is the utility function, which uses ngiMutex_t and ngiCond_t.
 * Thus, this function is effective both Pthread and NonThread version.
 */
int
ngiThreadSleep(
    int sleepSec,
    int allowAbortReturn,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiThreadSleep";
    int mutexInitialized, condInitialized, mutexLocked;
    int result, remain, cont, isTimeout;
    time_t timeNow, timeEnd;
    ngiMutex_t mutex;
    ngiCond_t cond;

    mutex = NGI_MUTEX_NULL;
    cond = NGI_COND_NULL;

    mutexInitialized = 0;
    condInitialized = 0;
    mutexLocked = 0;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. event is NULL.\n");
        goto error;
    }

    result = ngiMutexInitialize(&mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Initialize the mutex failed.\n");
        goto error;
    }
    mutexInitialized = 1;

    result = ngiCondInitialize(&cond, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Initialize the cond failed.\n");
        goto error;
    }
    condInitialized = 1;

    /* Lock */
    result = ngiMutexLock(&mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName, 
        "sleeping %d seconds.\n", sleepSec);

    timeEnd = time(NULL) + sleepSec;

    cont = 1;
    while (cont) {
        timeNow = time(NULL);
        if (timeNow >= timeEnd) {
            cont = 0;
            break;
        }
        remain = timeEnd - timeNow;

        isTimeout = 0;
        result = ngiCondTimedWait(
            &cond, &mutex, remain, &isTimeout, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Wait the Cond failed.\n");
            goto error;
        }

        if (allowAbortReturn != 0) {
            cont = 0;
            break;
        }
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    condInitialized = 0;
    result = ngiCondDestroy(&cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destroy the cond failed.\n");
        goto error;
    }

    mutexInitialized = 0;
    result = ngiMutexDestroy(&mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destroy the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the Mutex failed.\n");
        }
    }

    if (condInitialized != 0) {
        condInitialized = 0;
        result = ngiCondDestroy(&cond, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Destroy the cond failed.\n");
        }
    }

    if (mutexInitialized != 0) {
        mutexInitialized = 0;
        result = ngiMutexDestroy(&mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Destroy the mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

