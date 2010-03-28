#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngisUtility.c,v $ $Revision: 1.6 $ $Date: 2006/08/21 02:19:05 $";
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
 * Utility for Invoke Server
 */

#ifdef NG_PTHREAD

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>
#include "ngInvokeServer.h"

/**
 * Prototype
 */
static void ngislReadBufferInitializeMember(ngisiReadBuffer_t *);

/**
 * Log Initialize
 */
int
ngisiLogInitialize(
    ngisiContext_t *context,
    ngisiLog_t *log,
    char *logFileName,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(log != NULL);

    log->ngisl_enabled = 0;
    log->ngisl_fileName = NULL;
    log->ngisl_fp = NULL;

    if (logFileName == NULL) {
        log->ngisl_enabled = 0;

        /* Success */
        return 1;
    }

    log->ngisl_enabled = 1;
    log->ngisl_fileName = strdup(logFileName);
    if (log->ngisl_fileName == NULL) {
        /* No error output */
        return 0;
    }

    log->ngisl_fp = fopen(logFileName, NGISI_LOG_OPEN_MODE);
    if (log->ngisl_fp == NULL) {
        /* No error output */
        return 0;
    }

    setvbuf(log->ngisl_fp, NULL, _IONBF, 0);
    
    /* Success */
    return 1;
}

/**
 * Log Finalize
 */
int
ngisiLogFinalize(
    ngisiContext_t *context,
    ngisiLog_t *log,
    int *error)
{
    int result;

    if (log->ngisl_fileName != NULL) {
        free(log->ngisl_fileName);
        log->ngisl_fileName = NULL;
    }

    if (log->ngisl_fp != NULL) {
        result = fclose(log->ngisl_fp);
        if (result != 0) {
            /* No error output */
            return 0;
        }
        log->ngisl_fp = NULL;
    }

    log->ngisl_enabled = 0;

    /* Success */
    return 1;
}

/**
 * Log Printf
 */
int
ngisiLogPrintf(
    ngisiContext_t *context,
    ngisiLogLevel_t level,
    const char *functionName,
    char *format,
    ...)
{
    va_list ap;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    va_start(ap, format);

    result = ngisiLogVprintfInternal(
        context, level, functionName, NULL, format, ap);

    va_end(ap);

    return result;
}

/**
 * Log Vprintf for Internal use
 */
int
ngisiLogVprintfInternal(
    ngisiContext_t *context,
    ngisiLogLevel_t level,
    const char *functionName,
    char *message,
    char *format,
    va_list ap)
{
    ngisiLog_t *log;
    char *levelStr;

    /* Check the arguments */
    assert(context != NULL);
    assert(functionName != NULL);
    assert(format != NULL);

    log = &context->ngisc_log;
    
    if (log->ngisl_enabled == 0) {
        return 1;
    }

    levelStr = (level == NGISI_LOG_LEVEL_DEBUG ? "debug" :
        (level == NGISI_LOG_LEVEL_WARNING ? "WARN" :
        (level == NGISI_LOG_LEVEL_ERROR ? "ERROR" :
        "BUG")));

    fprintf(log->ngisl_fp, "%ld: ", (long)time(NULL));

    fprintf(log->ngisl_fp, "%-5s: ", levelStr);

    if (message != NULL) {
        fprintf(log->ngisl_fp, "%s: ", message);
    }

    fprintf(log->ngisl_fp, "%s: ", functionName);

    vfprintf(log->ngisl_fp, format, ap);

    /* Success */
    return 1;
}

/**
 * Mutex Initialize
 */
int
ngisiMutexInitialize(
    ngisiContext_t *context,
    globus_mutex_t *mutex,
    int *error)
{
    static const char fName[] = "ngisiMutexInitialize";
    int result;

    result = globus_mutex_init(mutex, NULL);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex Finalize
 */
int
ngisiMutexFinalize(
    ngisiContext_t *context,
    globus_mutex_t *mutex,
    int *error)
{
    static const char fName[] = "ngisiMutexFinalize";
    int result;

    result = globus_mutex_destroy(mutex);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex Lock
 */
int
ngisiMutexLock(
    ngisiContext_t *context,
    globus_mutex_t *mutex,
    int *error)
{
    static const char fName[] = "ngisiMutexLock";
    int result;

    result = globus_mutex_lock(mutex);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Mutex Unlock
 */
int
ngisiMutexUnlock(
    ngisiContext_t *context,
    globus_mutex_t *mutex,
    int *error)
{
    static const char fName[] = "ngisiMutexUnlock";
    int result;

    result = globus_mutex_unlock(mutex);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cond Initialize
 */
int
ngisiCondInitialize(
    ngisiContext_t *context,
    globus_cond_t *cond,
    int *error)
{
    static const char fName[] = "ngisiCondInitialize";
    int result;

    result = globus_cond_init(cond, NULL);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cond Finalize
 */
int
ngisiCondFinalize(
    ngisiContext_t *context,
    globus_cond_t *cond,
    int *error)
{
    static const char fName[] = "ngisiCondFinalize";
    int result;

    result = globus_cond_destroy(cond);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Destroy the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cond Signal
 */
int
ngisiCondSignal(
    ngisiContext_t *context,
    globus_cond_t *cond,
    int *error)
{
    static const char fName[] = "ngisiCondSignal";
    int result;

    result = globus_cond_signal(cond);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Signal the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cond Wait
 */
int
ngisiCondWait(
    ngisiContext_t *context,
    globus_cond_t *cond,
    globus_mutex_t *mutex,
    int *error)
{
    static const char fName[] = "ngisiCondWait";
    int result;

    result = globus_cond_wait(cond, mutex);
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Wait the cond failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cond Timed Wait
 */
int
ngisiCondTimedWait(
    ngisiContext_t *context,
    globus_cond_t *cond,
    globus_mutex_t *mutex,
    int relativeSec,
    int *timeout,
    int *error)
{
    static const char fName[] = "ngisiCondTimedWait";
    globus_abstime_t abstime;
    struct timeval tv;
    int result;

    *timeout = 0;

    /* Get current time */
    result = gettimeofday(&tv, NULL);
    if (result < 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed: %s.\n", "gettimeofday()", strerror(errno));
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
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d: Condition variable timed wait error.\n",
            "globus_cond_timedwait()", result);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read Buffer Initialize
 */
int
ngisiReadBufferInitialize(
    ngisiContext_t *context,
    ngisiReadBuffer_t *readBuffer,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(readBuffer != NULL);

    ngislReadBufferInitializeMember(readBuffer);

    /* Success */
    return 1;
}

/**
 * Read Buffer Finalize
 */
int
ngisiReadBufferFinalize(
    ngisiContext_t *context,
    ngisiReadBuffer_t *readBuffer,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(readBuffer != NULL);

    if (readBuffer->ngisrb_buf != NULL) {
        free(readBuffer->ngisrb_buf);
        readBuffer->ngisrb_buf = NULL;
    }

    ngislReadBufferInitializeMember(readBuffer);

    /* Success */
    return 1;
}

/**
 * Read Buffer Initialize Member
 */
static void
ngislReadBufferInitializeMember(
    ngisiReadBuffer_t *readBuffer)
{
    /* Check the arguments */
    assert(readBuffer != NULL);

    readBuffer->ngisrb_buf = NULL;
    readBuffer->ngisrb_bufSize = 0;
    readBuffer->ngisrb_reachEOF = 0;
}

/**
 * Read Buffer Read One Line
 * This function removes the terminating characters.
 */
int
ngisiReadLine(
    ngisiContext_t *context,
    FILE *readFp,
    ngisiReadBuffer_t *readBuffer,
    int *error)
{
    static const char fName[] = "ngisiReadLine";
    int bufSize, oldBufSize, newBufSize, terminateSize;
    char *p, *buf, *oldBuf, *terminateStr, *termMatchStart;
    int cur, c, finish, termMatchCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(readFp != NULL);
    assert(readBuffer != NULL);

    if (readBuffer->ngisrb_reachEOF != 0) {
        return 1;
    }

    buf = readBuffer->ngisrb_buf;
    bufSize = readBuffer->ngisrb_bufSize;

    terminateStr  = NGISI_LINE_TERMINATOR_STR;
    terminateSize = NGISI_LINE_TERMINATOR_SIZE;

    cur = 0;
    p = buf;
    termMatchCount = 0;
    termMatchStart = NULL;
    finish = 0;

    do {
        c = fgetc(readFp);

        if (cur >= bufSize) {
            /* Renew buffer */
            oldBuf = buf;
            oldBufSize = bufSize;

            if (bufSize <= 0) {
                newBufSize = NGISI_READ_BUFFER_INITIAL_SIZE;
            } else {
                newBufSize = bufSize * 2;
            }

            buf = (char *)globus_libc_calloc(newBufSize, sizeof(char));
            if (buf == NULL) {
                ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                    "Allocate the Read buffer failed.\n");
                return 0;
            }
            bufSize = newBufSize;
            newBufSize = 0;

            if (oldBuf != NULL) {
                memcpy(buf, oldBuf, oldBufSize);

                globus_libc_free(oldBuf);
                oldBuf = NULL;
                oldBufSize = 0;
            }

            p = buf + cur;

            readBuffer->ngisrb_buf = buf;
            readBuffer->ngisrb_bufSize = bufSize;
        }

        *p = ((c != EOF) ? c : '\0');

        if (c == EOF) {
            readBuffer->ngisrb_reachEOF = 1;
            finish = 1;
        }

        /* Check terminate charactor */
        if (*p == terminateStr[termMatchCount]) {
            if (termMatchCount == 0) {
                termMatchStart = p;
            }
            termMatchCount++;
        } else {
            termMatchCount = 0;
        }

        if (termMatchCount >= terminateSize) {
            finish = 1;
            assert(termMatchStart != NULL);
            *termMatchStart = '\0';
        }

        cur++;
        p++;

    } while (finish == 0);

    /* Success */
    return 1;
}

#endif /* NG_PTHREAD */

