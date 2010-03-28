#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngUtility.c,v $ $Revision: 1.43 $ $Date: 2006/02/02 07:04:36 $";
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
 * @file ngiUtility.c
 * Utility module for Ninf-G internal.
 */

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#ifdef linux
#include <sys/poll.h>
#else /* linux */
#include <poll.h>
#endif /* linux */
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static void nglConnectRetryInformationInitializeMember(
    ngiConnectRetryInformation_t *);
static void nglConnectRetryInitializeMember(ngiConnectRetryStatus_t *);
static void nglRandomNumberInitializeMember(ngiRandomNumber_t *);

/**
 * Sleep seconds.
 */
int
ngiSleepSecond(int sec)
{
    int result;
    struct timeval tv;

    tv.tv_sec = sec;
    tv.tv_usec = 0;
    result = select(0, NULL, NULL, NULL, &tv);
    if (result < 0) {
	if (EINTR)
	    return 0;
	else
	    return -1;
    }

    return 0;
}

/**
 * Sleep by struct timeval
 */
int
ngiSleepTimeval(
    struct timeval *sleepTime,
    int returnByEINTR,
    ngLog_t *log,
    int *error)
{
    int result;
    long sleepMilliSec;
    struct timeval timeNow, timeEnd, timeTmp;
    static const char fName[] = "ngiSleepTimeval";

    /* Check the arguments */
    assert(sleepTime != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Sleeping %ld.%06ld seconds.\n",
        fName, sleepTime->tv_sec, sleepTime->tv_usec);

    /* Get finish time */
    result = gettimeofday(&timeNow, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: gettimeofday() failed by %d: %s.\n",
            fName, result, strerror(result));
        return 0;
    }

    timeEnd = ngiTimevalAdd(timeNow, *sleepTime);

    /* loop until finish time arrive */
    while(1) {
        /* Check finished */
        result = gettimeofday(&timeNow, NULL);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: gettimeofday() failed by %d: %s.\n",
                fName, result, strerror(result));
            return 0;
        }

        if (ngiTimevalCompare(timeNow, timeEnd) >= 0) {
            break;
        }

        /* Get sleep time and sleep */
        timeTmp = ngiTimevalSub(timeEnd, timeNow);
        sleepMilliSec = (timeTmp.tv_sec * 1000) + (timeTmp.tv_usec / 1000);

        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: start poll() for %ld ms.\n", fName, sleepMilliSec);

        result = poll(NULL, 0, sleepMilliSec);
        if (result < 0) {
            if (errno == EINTR) {
                if (returnByEINTR != 0) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
                        NULL, "%s: return by EINTR.\n", fName);
                    break;
                } else {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
                        NULL,
                        "%s: poll() returned by EINTR. continue poll().\n",
                        fName);
                    continue;
                }
            } else {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: poll() failed by %d: %s.\n",
                    fName, result, strerror(result));
                return 0;
            }
        }
    }
    
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: sleep finished. return.\n", fName);

    /* Success */
    return 1;
}

/**
 * Execution time: Set start time.
 */
int
ngiSetStartTime(ngExecutionTime_t *time, ngLog_t *log, int *error)
{
    int result;
    struct rusage rusage;
    static const char fName[] = "ngiSetStartTime";

    /* Check the arguments */
    assert(time != NULL);

    /* Get the real start time */
    result = gettimeofday(&time->nget_real.nget_start, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: gettimeofday failed.\n", fName);
	return 0;
    }

    /* Get the cpu start time */
    result = getrusage(RUSAGE_SELF, &rusage);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: getrusage failed.\n", fName);
	return 0;
    }
    time->nget_cpu.nget_start = ngiTimevalAdd(
        rusage.ru_utime, rusage.ru_stime);

    /* Success */
    return 1;
}

/**
 * Execution time: Set end time.
 */
int
ngiSetEndTime(ngExecutionTime_t *time, ngLog_t *log, int *error)
{
    int result;
    struct rusage rusage;
    static const char fName[] = "ngiSetEndTime";

    /* Check the arguments */
    assert(time != NULL);

    /* Get the real end time */
    result = gettimeofday(&time->nget_real.nget_end, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: gettimeofday failed.\n", fName);
	return 0;
    }

    /* Get the cpu end time */
    result = getrusage(RUSAGE_SELF, &rusage);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: getrusage failed.\n", fName);
	return 0;
    }

    time->nget_cpu.nget_end = ngiTimevalAdd(
        rusage.ru_utime, rusage.ru_stime);

    /* Calculate the real execution time */
    time->nget_real.nget_execution = ngiTimevalSub(
        time->nget_real.nget_end, time->nget_real.nget_start);

    /* Calculate the cpu execution time */
    time->nget_cpu.nget_execution = ngiTimevalSub(
        time->nget_cpu.nget_end, time->nget_cpu.nget_start);

    /* Success */
    return 1;
}

/**
 * Convert string to time.
 */
int
ngiStringToTime(char *string, struct timeval *tv, ngLog_t *log, int *error)
{
    char *work, *tmp, *curr, *end;
    static const char fName[] = "ngiStringToTime";

    /* Duplicate the string */
    work = strdup(string);
    if (work == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for string.\n", fName);
	return 0;
    }
    curr = work;

    /* Get the string of second */
    tmp = strchr(curr, 's');
    if (tmp == NULL)
	goto syntaxError;
    *tmp = '\0';

    /* Convert to second */
    tv->tv_sec = strtol(curr, &end, 0);
    if (end == curr)
	goto syntaxError;

    /* Get the micro second */
    curr = tmp + 1;
    if (*curr == '\0')
	goto syntaxError;
    tmp = strchr(curr, 'u');
    if ((tmp == NULL) || (tmp[1] != 's'))
	goto syntaxError;

    /* Convert to micro second */
    tv->tv_usec = strtol(curr, &end, 0);
    if (end == curr)
	goto syntaxError;

    /* Deallocate the work */
    globus_libc_free(work);

    /* Success */
    return 1;

    /* Syntax error */
syntaxError:
    globus_libc_free(work);

    NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	NULL, "%s: Session Information: Syntax error: %s.\n",
	fName, string);
    return 0;
}

/**
 * Debugger Information: Finalize
 */
int
ngiDebuggerInformationFinalize(
    ngDebuggerInformation_t *dbgInfo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(dbgInfo != NULL);

    /* Deallocate the members */
    globus_libc_free(dbgInfo->ngdi_terminalPath);
    globus_libc_free(dbgInfo->ngdi_display);
    globus_libc_free(dbgInfo->ngdi_debuggerPath);

    /* Initialize the members */
    ngiDebuggerInformationInitializeMember(dbgInfo);

    /* Success */
    return 1;
}

/**
 * Debugger Information: Initialize the members.
 */
void
ngiDebuggerInformationInitializeMember(ngDebuggerInformation_t *dbgInfo)
{
    /* Check the argument */
    assert(dbgInfo != NULL);

    /* Initialize the pointers */
    ngiDebuggerInformationInitializePointer(dbgInfo);

    /* Initialize the members */
    dbgInfo->ngdi_enable = 0;
}

/**
 * Debugger Information: Initialize the pointers.
 */
void
ngiDebuggerInformationInitializePointer(ngDebuggerInformation_t *dbgInfo)
{
    /* Check the argument */
    assert(dbgInfo != NULL);

    /* Initialize the pointers */
    dbgInfo->ngdi_display = NULL;
    dbgInfo->ngdi_terminalPath = NULL;
    dbgInfo->ngdi_debuggerPath = NULL;
}

/**
 * Read/Write Lock: Initialize
 */
int
ngiRWlockInitialize(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockInitialize";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* Initialize the mutex */
    result = ngiMutexInitialize(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex.\n", fName);
	return 0;
    }

    /* Initialize the condition variable */
    result = ngiCondInitialize(&rwLock->ngrwl_condWrite, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Condition Variable.\n", fName);
        ngiMutexDestroy(&rwLock->ngrwl_mutex, log, error);
	return 0;
    }

    /* Initialize the condition variable */
    result = ngiCondInitialize(&rwLock->ngrwl_condRead, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Condition Variable.\n", fName);
    	ngiCondDestroy(&rwLock->ngrwl_condWrite, log, error);
        ngiMutexDestroy(&rwLock->ngrwl_mutex, log, error);
	return 0;
    }

    /* Initialize other variables */
    rwLock->ngrwl_waitingWriteLock = 0;
    rwLock->ngrwl_lockCounter = 0;

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Finalize
 */
int
ngiRWlockFinalize(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockFinalize";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* Destroy the conditional variable */
    result = ngiCondDestroy(&rwLock->ngrwl_condRead, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy the Condition Variable.\n", fName);
    	return 0;
    }

    /* Destroy the conditional variable */
    result = ngiCondDestroy(&rwLock->ngrwl_condWrite, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy the Condition Variable.\n", fName);
    	return 0;
    }

    /* Destroy the mutex */
    result = ngiMutexDestroy(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Read lock
 */
int
ngiRWlockReadLock(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockReadLock";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Does it locking for write? */
    while (rwLock->ngrwl_lockCounter < 0) {
    	result = ngiCondWait(
	    &rwLock->ngrwl_condRead, &rwLock->ngrwl_mutex, log, error);
	if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Does counter over? */
    if (rwLock->ngrwl_lockCounter >= NG_RWLOCK_MAX_LOCKS) {
    	NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The counter of Read/Write Lock overflowed.\n", fName);
	goto error;
    }

    /* Increment the counter */
    rwLock->ngrwl_lockCounter++;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    return 1;
}

/**
 * Read/Write Lock: Read unlock
 */
int
ngiRWlockReadUnlock(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockReadUnlock";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Does anybody locking for read? */
    if (rwLock->ngrwl_lockCounter <= 0) {
        /* Nobody locking for read */
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Nobody locking for read.\n", fName);
	goto error;
    }

    /* Decrement the counter */
    rwLock->ngrwl_lockCounter--;

    /* Does nobody locking? */
    if (rwLock->ngrwl_lockCounter == 0) {
	/* Wakeup any threads, which waiting */
	if (rwLock->ngrwl_waitingWriteLock > 0) {
	    result = ngiCondSignal(&rwLock->ngrwl_condWrite, log, error);
	    if (result == 0) {
        	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't signal the Condition Variable.\n", fName);
	        goto error;
	    }
	} else {
	    result = ngiCondBroadcast(&rwLock->ngrwl_condRead, log, error);
	    if (result == 0) {
        	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't broadcast the Condition Variable.\n", fName);
	        goto error;
	    }
	}
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    return 0;
}

/**
 * Read/Write Lock: Write lock
 */
int
ngiRWlockWriteLock(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockWriteLock";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Does it locking? */
    rwLock->ngrwl_waitingWriteLock++;
    while (rwLock->ngrwl_lockCounter != 0) {
    	result = ngiCondWait(
	    &rwLock->ngrwl_condWrite, &rwLock->ngrwl_mutex, log, error);
	if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }
    rwLock->ngrwl_waitingWriteLock--;

    /* Decrement the counter */
    rwLock->ngrwl_lockCounter--;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    return 1;
}

/**
 * Read/Write Lock: Write unlock
 */
int
ngiRWlockWriteUnlock(ngRWlock_t *rwLock, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiRWlockWriteUnlock";

    /* Check the arguments */
    assert(rwLock != NULL);

    /* lock the mutex */
    result = ngiMutexLock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Does anybody locking for write? */
    if (rwLock->ngrwl_lockCounter >= 0) {
        /* Nobody locking for write */
	NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Nobody locking for write.\n", fName);
	goto error;
    }

    /* Increment the counter */
    rwLock->ngrwl_lockCounter++;

    /* Wakeup any threads, which waiting */
    if (rwLock->ngrwl_waitingWriteLock > 0) {
	result = ngiCondSignal(&rwLock->ngrwl_condWrite, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't signal the Condition Variable.\n", fName);
	    goto error;
	}
    } else {
	result = ngiCondBroadcast(&rwLock->ngrwl_condRead, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't broadcast the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&rwLock->ngrwl_mutex, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    return 0;
}

/**
 * Mutex: Initialize
 */
int
ngiMutexInitialize(globus_mutex_t *mutex, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiMutexInitialize";

    result = globus_mutex_init(mutex, NULL);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_mutex_init failed by %d: Mutex initialize error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Destroy
 */
int
ngiMutexDestroy(globus_mutex_t *mutex, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiMutexDestroy";

    result = globus_mutex_destroy(mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_mutex_destroy failed by %d: Mutex destroy error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Lock
 */
int
ngiMutexLock(globus_mutex_t *mutex, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiMutexLock";

    result = globus_mutex_lock(mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_mutex_lock failed by %d: Mutex lock error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Try lock
 */
int
ngiMutexTryLock(globus_mutex_t *mutex, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiMutexTryLock";

    result = globus_mutex_trylock(mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_mutex_trylock failed by %d: Mutex try lock error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex: Unlock
 */
int
ngiMutexUnlock(globus_mutex_t *mutex, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiMutexUnlock";

    result = globus_mutex_unlock(mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_mutex_unlock failed by %d: Mutex unlock error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Condition variable: Initialize
 */
int
ngiCondInitialize(globus_cond_t *cond, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiCondInitialize";

    result = globus_cond_init(cond, NULL);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_cond_init failed by %d: Condition variable initialize error.\n",
	    fName, result);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Condition variable: Destroy
 */
int
ngiCondDestroy(globus_cond_t *cond, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiCondDestroy";

    result = globus_cond_destroy(cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_cond_destroy failed by %d: Condition variable destroy destroy error.\n",
	    fName, result);
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Wait
 */
int
ngiCondWait(
    globus_cond_t *cond,
    globus_mutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondWait";

    result = globus_cond_wait(cond, mutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_cond_wait failed by %d: Condition variable wait error.\n",
	    fName, result);
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Timed wait
 */
int
ngiCondTimedWait(
    globus_cond_t *cond,
    globus_mutex_t *mutex,
    int relativeSec,
    int *timeout,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiCondTimedWait";
    struct timeval tv;
    globus_abstime_t abstime;

    /* Get current time */
    result = gettimeofday(&tv, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: gettimeofday failed: %s.\n",
	    fName, strerror(errno));
	return 0;
    }

    /* Calculate absolute time */
    abstime.tv_sec = tv.tv_sec + relativeSec;
    abstime.tv_nsec = tv.tv_usec * 1000;

    /* Wait for condition variable */
    result = globus_cond_timedwait(cond, mutex, &abstime);

    /* Time out? */
    if (result == ETIMEDOUT) {
	*timeout = 1;
	return 1;
    }

    /* Error? */
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_cond_timedwait failed by %d: Condition variable timed wait error.\n",
	    fName, result);
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
ngiCondSignal(globus_cond_t *cond, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiCondSignal";

    result = globus_cond_signal(cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_cond_signal failed by %d: Condition variable signal error.\n",
	    fName, result);
	return 0;
    }

    return 1;
}

/**
 * Condition variable: Broadcast
 */
int
ngiCondBroadcast(globus_cond_t *cond, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiCondBroadcast";

    result = globus_cond_broadcast(cond);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: globus_cond_broadcast failed by %d: Conditional variable broadcast error.\n",
	    fName, result);
	return 0;
    }

    return 1;
}

/**
 * Temporary file: Default temporary directory name get
 */
char *
ngiDefaultTemporaryDirectoryNameGet(
    ngLog_t *log,
    int *error)
{
    char *tmpDir;
    char *ret;
    static const char fName[] = "ngiDefaultTemporaryDirectoryNameGet";

    tmpDir = getenv(NGI_ENVIRONMENT_TMPDIR);
    if (tmpDir == NULL) {
	tmpDir = NGI_TMP_DIR;
    }
    /* Print Warning If ${TMPDIR} is empty string. */
    else if (strlen(tmpDir) == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING,
                    NULL, "%s: the TMPDIR environment variable is empty.\n",
		    fName);
    }

    ret = strdup(tmpDir);
    if (ret == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for string.\n", fName);

        /* Failed */
	return NULL;
    }

    /* Success */
    return ret;
}

/**
 * Temporary file: Create
 */
char *
ngiTemporaryFileCreate(
    char *prefix,
    ngLog_t *log,
    int *error)
{
    int result;
    int length;
    int fd = -1;
    int prefix_allocated = 0;
    char *tmpFile = NULL;
    static const char fName[] = "ngiTemporaryFileCreate";

    /* Check the argument */
    if (prefix == NULL) {
	prefix = ngiDefaultTemporaryDirectoryNameGet(log, error);
	if (prefix == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Can't get default temporary directory name.\n",
		fName);
		goto error;
	}
	prefix_allocated = 1;
    }

    /* Allocate */
    length = strlen(prefix) + 1 + strlen(NGI_TMP_FILE) + 1;
    tmpFile = globus_libc_malloc(length);
    if (tmpFile == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s:Can't allocate the storage for Temporary File Name.\n",
	    fName);
	goto error;
    }

    /* Copy */
    snprintf(tmpFile, length, "%s/%s", prefix, NGI_TMP_FILE);

    /* Create */
    fd = mkstemp(tmpFile);
    if (fd < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: mkstemp \"%s\" failed: %s.\n",
            fName, tmpFile, strerror(errno));
	goto error;
    }

    /* Close */
    result = close(fd);
    if (result < 0) {
	fd = -1;
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: close failed: %s.\n", fName, strerror(errno));
	goto error;
    }

    /* Deallocate prefix if allocated */
    if (prefix_allocated != 0) {
	assert(prefix != NULL);
	globus_libc_free(prefix);
	prefix = NULL;
        prefix_allocated = 0;
    }

    /* Success */
    return tmpFile;

    /* Error occurred */
error:
    if (fd >= 0) {
	/* Close the Temporary File */
	result = close(fd);
	if (result < 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: close failed: %s.\n", fName, strerror(errno));
	}
	fd = -1;

	/* Unlink the Temporary File */
	result = unlink(tmpFile);
	if (result < 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: unlink failed: %s.\n", fName, strerror(errno));
	}
    }

    /* Deallocate the Temporary File Name */
    if (tmpFile != NULL) {
	globus_libc_free(tmpFile);
	tmpFile = NULL;
    }

    /* Deallocate prefix if allocated */
    if (prefix_allocated != 0) {
	assert(prefix != NULL);
	globus_libc_free(prefix);
	prefix = NULL;
        prefix_allocated = 0;
    }

    /* Failed */
    return NULL;
}

/**
 * Temporary file name: Destroy
 */
int
ngiTemporaryFileDestroy(char *tmpFile, ngLog_t *log, int *error)
{
    int result;
    int retResult = 1;
    static const char fName[] = "ngiTemporaryFileDestroy";

    /* Check the arguments */
    assert(tmpFile != NULL);

    /* Unlink the Temporary File */
    result = unlink(tmpFile);
    if (result < 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: unlink failed: %s.\n", fName, strerror(errno));
	retResult = 0;
    }

    /* Deallocate */
    globus_libc_free(tmpFile);

    /* Success */
    return retResult;
}

/**
 * Connect Retry Information : Initialize.
 */
int
ngiConnectRetryInformationInitialize(
    ngiConnectRetryInformation_t *retryInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInformationInitialize";

    /* Check the arguments */
    if (retryInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: retryInfo is NULL.\n", fName);
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInformationInitializeMember(retryInfo);

    /* Success */
    return 1;
}

/**
 * Connect Retry Information : Finalize.
 */
int
ngiConnectRetryInformationFinalize(
    ngiConnectRetryInformation_t *retryInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInformationFinalize";

    /* Check the arguments */
    if (retryInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: retryInfo is NULL.\n", fName);
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInformationInitializeMember(retryInfo);

    /* Success */
    return 1;
}

static void
nglConnectRetryInformationInitializeMember(
    ngiConnectRetryInformation_t *retryInfo)
{
    /* Check the arguments */
    assert(retryInfo != NULL);

    retryInfo->ngcri_count = 0;
    retryInfo->ngcri_interval = 0;
    retryInfo->ngcri_increase = 0.0;
    retryInfo->ngcri_useRandom = 0;
}

/**
 * Connect Retry : Initialize.
 */
int
ngiConnectRetryInitialize(
    ngiConnectRetryStatus_t *retryStatus,
    ngiConnectRetryInformation_t *retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInitialize";

    /* Check the arguments */
    if ((retryStatus == NULL) ||
        (retryInfo == NULL) ||
        (randomSeed == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInitializeMember(retryStatus);

    /* Copy the Retry Information */
    retryStatus->ngcrs_retryInfo = *retryInfo;

    /* Set to initial state */
    retryStatus->ngcrs_retry = retryInfo->ngcri_count; /* full count */
    retryStatus->ngcrs_nextInterval = (double)retryInfo->ngcri_interval;

    /* Set the Random Number Seed */
    retryStatus->ngcrs_randomSeed = randomSeed;

    /* Success */
    return 1;
}

/**
 * Connect Retry : Finalize.
 */
int
ngiConnectRetryFinalize(
    ngiConnectRetryStatus_t *retryStatus,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryFinalize";

    /* Check the arguments */
    if (retryStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Do not free Random Number Seed */

    /* Initialize the members */
    nglConnectRetryInitializeMember(retryStatus);

    /* Success */
    return 1;
}

static void
nglConnectRetryInitializeMember(
    ngiConnectRetryStatus_t *retryStatus)
{
    /* Check the arguments */
    assert(retryStatus != NULL);

    nglConnectRetryInformationInitializeMember(&retryStatus->ngcrs_retryInfo);

    retryStatus->ngcrs_retry = 0;
    retryStatus->ngcrs_nextInterval = 0.0;
    retryStatus->ngcrs_randomSeed = NULL;
}

/**
 * Connect Retry : Get next retry sleep time.
 */
int
ngiConnectRetryGetNextRetrySleepTime(
    ngiConnectRetryStatus_t *retryStatus,
    int *doRetry,
    struct timeval *sleepTime,
    ngLog_t *log,
    int *error)
{
    int result;
    long sleepSec, sleepUsec;
    double nextRetrySec, randomNo;
    static const char fName[] = "ngiConnectRetryGetNextRetrySleepTime";

    *doRetry = 0;

    /* Check the arguments */
    if ((retryStatus == NULL) || (doRetry == NULL) || (sleepTime == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    if (retryStatus->ngcrs_retry > 0) {
        nextRetrySec = retryStatus->ngcrs_nextInterval;
        if (retryStatus->ngcrs_retryInfo.ngcri_useRandom == 1) {

            result = ngiRandomNumberGetDouble(
                retryStatus->ngcrs_randomSeed, &randomNo, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
                    NULL, "%s: Random number generation failed.\n", fName);
                return 0;
            }

            nextRetrySec = retryStatus->ngcrs_nextInterval * randomNo;

            /* log */
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
                NULL, "%s: got the random number %g from 0 to %g.\n",
                fName, nextRetrySec, retryStatus->ngcrs_nextInterval);
        }

        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
            NULL, "%s: last %d retry will performed after %g second sleep.\n",
            fName, retryStatus->ngcrs_retry, nextRetrySec);

        *doRetry = 1;
        retryStatus->ngcrs_retry--;

        sleepSec = (long)nextRetrySec;
        nextRetrySec -= (double)sleepSec;
        sleepUsec = (long)((double)nextRetrySec * 1000 * 1000);
        nextRetrySec = 0;

        sleepTime->tv_sec = sleepSec; 
        sleepTime->tv_usec = sleepUsec; 

        /* next interval */
        retryStatus->ngcrs_nextInterval *=
            retryStatus->ngcrs_retryInfo.ngcri_increase;

        if (retryStatus->ngcrs_nextInterval < 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING,
                NULL, "%s: Next retry time %g is invalid.\n",
                fName, retryStatus->ngcrs_nextInterval);
        }

    } else {
        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
            NULL, "%s: retry count exhausted. do not retry.\n",
            fName);

        *doRetry = 0;
    }

    /* Success */
    return 1;
}

/**
 * struct timeval operations : Add
 */
struct timeval
ngiTimevalAdd(
    struct timeval tvA,
    struct timeval tvB)
{
    struct timeval tmp;

    tmp.tv_usec = tvA.tv_usec + tvB.tv_usec;
    tmp.tv_sec = tvA.tv_sec + tvB.tv_sec + tmp.tv_usec / (1000 * 1000);
    tmp.tv_usec %= 1000 * 1000;

    return tmp;
}

/**
 * struct timeval operations : Subtract
 */
struct timeval
ngiTimevalSub(
    struct timeval tvA,
    struct timeval tvB)
{
    struct timeval tmp;
    long carry;

    if (tvA.tv_usec < tvB.tv_usec) {
        carry = 1 + (tvB.tv_usec / (1000 * 1000));
        tvA.tv_sec -= carry;
        tvA.tv_usec += carry * 1000 * 1000;
    }

    tmp.tv_sec = tvA.tv_sec - tvB.tv_sec;
    tmp.tv_usec = tvA.tv_usec - tvB.tv_usec;

    return tmp;
}

/**
 * struct timeval operations : Compare
 *   if (tvA <  tvB) return -1;
 *   if (tvA == tvB) return  0;
 *   if (tvA >  tvB) return +1;
 */
int
ngiTimevalCompare(
    struct timeval tvA,
    struct timeval tvB)
{
    if (tvA.tv_sec > tvB.tv_sec) {
        return 1;
    }

    if (tvA.tv_sec < tvB.tv_sec) {
        return -1;
    }

    assert (tvA.tv_sec == tvB.tv_sec);

    if (tvA.tv_usec > tvB.tv_usec) {
        return 1;
    }

    if (tvA.tv_usec < tvB.tv_usec) {
        return -1;
    }

    assert (tvA.tv_usec == tvB.tv_usec);

    return 0;
}

#define NGL_MODULUS     2147483647 /* 2^31 - 1 */
#define NGL_MULTIPLIER  48271

/**
 * Random Number.
 * Avoid libc function rand() or random() use,
 *  for to avoid user code influence.
 */
int
ngiRandomNumberInitialize(
    ngiRandomNumber_t *randomNumberStatus,
    ngLog_t *log,
    int *error)
{
    long seedNo;
    static const char fName[] = "ngiRandomNumberInitialize";

    /* Check the arguments */
    if (randomNumberStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Initialize the members */
    nglRandomNumberInitializeMember(randomNumberStatus);

    /* Initialize the seed */
    seedNo = ((long)getpid() + (long)time(NULL)) % NGL_MODULUS;

    *randomNumberStatus = seedNo;

    /* Success */
    return 1;
}

int
ngiRandomNumberFinalize(
    ngiRandomNumber_t *randomNumberStatus,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRandomNumberFinalize";

    /* Check the arguments */
    if (randomNumberStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Initialize the members */
    nglRandomNumberInitializeMember(randomNumberStatus);

    /* Success */
    return 1;
}

static void
nglRandomNumberInitializeMember(
    ngiRandomNumber_t *randomNumberStatus)
{
    /* Check the arguments */
    assert(randomNumberStatus != NULL);

    *randomNumberStatus = 0;
}


/**
 * Get the Random number by long int.
 */
int
ngiRandomNumberGetLong(
    ngiRandomNumber_t *randomNumberStatus,
    long *randomNo,
    ngLog_t *log,
    int *error)
{
    long seedNo;
    static const char fName[] = "ngiRandomNumberGetLong";

    /* Check the arguments */
    if ((randomNumberStatus == NULL) || (randomNo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    *randomNo = 0;

    seedNo = *randomNumberStatus;
    seedNo = NGL_MULTIPLIER 
        * (seedNo % (NGL_MODULUS / NGL_MULTIPLIER))
        - (NGL_MODULUS % NGL_MULTIPLIER) 
            * (seedNo / (NGL_MODULUS / NGL_MULTIPLIER));
    seedNo = ((seedNo > 0) ? seedNo : seedNo + NGL_MODULUS);
    
    *randomNumberStatus = seedNo;
    *randomNo = seedNo;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
        NULL, "%s: Got the random number (long int)%ld.\n",
        fName, *randomNo);
    
    /* Success */
    return 1;
}

/**
 * Get the Random number. random from 0.0 to 1.0 on double.
 */
int
ngiRandomNumberGetDouble(
    ngiRandomNumber_t *randomNumberStatus,
    double *randomNo,
    ngLog_t *log,
    int *error)
{
    int result;
    long randomNoLong;
    static const char fName[] = "ngiRandomNumberGetDouble";

    /* Check the arguments */
    if ((randomNumberStatus == NULL) || (randomNo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    *randomNo = 0.0;

    randomNoLong = 0;
    result = ngiRandomNumberGetLong(
        randomNumberStatus, &randomNoLong, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Getting random number failed.\n", fName);
        return 0;
    }

    *randomNo = ((double)randomNoLong / NGL_MODULUS);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
        NULL, "%s: Got the random number (double)%g.\n",
        fName, *randomNo);
    
    /* Success */
    return 1;
}

