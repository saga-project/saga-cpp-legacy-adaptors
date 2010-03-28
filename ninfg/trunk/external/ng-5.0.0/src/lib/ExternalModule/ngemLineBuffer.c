/*
 * $RCSfile: ngemLineBuffer.c,v $ $Revision: 1.8 $ $Date: 2008/02/14 10:57:33 $
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

#include "ngemCallbackManager.h"
#include "ngemUtility.h"
#include "ngemLog.h"

NGI_RCSID_EMBED("$RCSfile: ngemLineBuffer.c,v $ $Revision: 1.8 $ $Date: 2008/02/14 10:57:33 $")

#define NGEML_LINE_BUFFER_INITIAL_SIZE 1024U

static void ngemlLineBufferCallback(void *, int, void *, size_t,
    ngemCallbackResult_t);
static int ngemlStringCheckValid(void *, size_t);
static int ngemlLineBufferGrow(ngemLineBuffer_t *, size_t);
static int ngemlLineBufferCall(ngemLineBuffer_t *);
static char *ngemlLineBufferGetLine(ngemLineBuffer_t *);

ngemLineBuffer_t *
ngemLineBufferCreate(
    int fd,
    const char *separator)
{
    ngemLineBuffer_t *new = NULL;
    char *buffer  = NULL;
    char *copySep = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemLineBufferCreate";

    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(NGEM_STRING_IS_NONZERO(separator));

    log = ngemLogGetDefault();

    new = NGI_ALLOCATE(ngemLineBuffer_t, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, 
            "Can't allocate storage for line buffer.\n");
        goto error;
    }

    buffer = ngiMalloc(NGEML_LINE_BUFFER_INITIAL_SIZE, log, NULL);
    if (buffer == NULL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName,
            "Can't allocate storage for buffer.\n");
        goto error;
    }
    buffer[0] = '\0';

    copySep = strdup(separator);
    if (copySep == NULL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Can't copy string.\n");
        goto error;
    }

    new->nglb_fd            = fd;
    new->nglb_buffer        = buffer;
    new->nglb_first         = buffer;
    new->nglb_last          = buffer;
    new->nglb_capacity      = NGEML_LINE_BUFFER_INITIAL_SIZE;
    new->nglb_separator     = copySep;
    new->nglb_func          = NULL;
    new->nglb_arg           = NULL;
    new->nglb_callbackValid = false;
    new->nglb_result        = NGEM_CALLBACK_RESULT_SUCCESS;
    
    return new;
error:
    ngiFree(copySep, log, NULL);
    ngiFree(buffer, log, NULL);
    NGI_DEALLOCATE(ngemLineBuffer_t, new, log, NULL);

    return NULL;
}

void
ngemLineBufferDestroy(
    ngemLineBuffer_t *lBuffer)
{
    ngLog_t *log;
    static const char fName[] = "ngemLineBufferDestroy";

    log = ngemLogGetDefault();

    if (lBuffer == NULL) {
        return;
    }

    /* Destroying */
    if (lBuffer->nglb_result == NGEM_CALLBACK_RESULT_CANCEL) {
        ngLogWarn(log, NGEM_LOGCAT_CALLBACK, fName,
            "Do nothing, so this line buffer is destroying.");
        return;
    }

    lBuffer->nglb_result = NGEM_CALLBACK_RESULT_CANCEL;
    if (lBuffer->nglb_callbackValid == true) {
        ngemCallbackCancel(lBuffer->nglb_callback);
    }

    ngiFree(lBuffer->nglb_buffer, log, NULL);
    ngiFree(lBuffer->nglb_separator, log, NULL);

    lBuffer->nglb_fd            = -1;
    lBuffer->nglb_buffer        = NULL;
    lBuffer->nglb_first         = NULL;
    lBuffer->nglb_last          = NULL;
    lBuffer->nglb_capacity      = 0U;
    lBuffer->nglb_separator     = NULL;
    lBuffer->nglb_func          = NULL;
    lBuffer->nglb_arg           = NULL;
    lBuffer->nglb_callbackValid = false;
    lBuffer->nglb_result        = NGEM_CALLBACK_RESULT_CANCEL;

    NGI_DEALLOCATE(ngemLineBuffer_t, lBuffer, log, NULL);
    
    return;
}

ngemResult_t
ngemLineBufferReadLine(
    ngemLineBuffer_t *lBuffer,
    ngemLineBufferCallbackFunc_t func,
    void *arg)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngemLineBufferReadLine";

    NGEM_ASSERT(lBuffer != NULL);
    NGEM_ASSERT(func != NULL);

    log = ngemLogGetDefault();

    /* Registered? */
    if (lBuffer->nglb_func != NULL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName,
            "Callback is already registered(fd=%d).\n", lBuffer->nglb_fd);
        return NGEM_FAILED;
    }

    if (lBuffer->nglb_result == NGEM_CALLBACK_RESULT_CANCEL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Callback is already canceled.\n");
        return NGEM_FAILED;
    }

    lBuffer->nglb_func = func;
    lBuffer->nglb_arg  = arg;

    result = ngemlLineBufferCall(lBuffer);
    if (result == 0) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Can't callback.\n");
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

static void 
ngemlLineBufferCallback(
    void *arg,
    int fd,
    void *data,
    size_t size,
    ngemCallbackResult_t cResult)
{
    ngemLineBuffer_t *lBuffer = (ngemLineBuffer_t *)arg;
    size_t emptySize;
    size_t newDataSize;
    size_t oldDataSize;
    int result;
    ngLog_t *log;
    static const char fName[] = "ngemlLineBufferCallback";

    log = ngemLogGetDefault();
    lBuffer->nglb_callbackValid = false;

    NGEM_ASSERT(lBuffer != NULL);

    lBuffer->nglb_result = cResult;
    switch (cResult) {
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_EOF:
        ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "EOF.\n");
        break;
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Can't read the line.\n");
        break;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    if (cResult == NGEM_CALLBACK_RESULT_SUCCESS) {
        NGEM_ASSERT(data != NULL);
        /* Includes NULL character? */
        if (!ngemlStringCheckValid(data, size)) {
            ngLogError(log, NGEM_LOGCAT_CALLBACK, fName,
                "There are invalid characters in the line.\n");
            lBuffer->nglb_result = NGEM_CALLBACK_RESULT_FAILED;
            goto call;
        }
        ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Read %lu bytes.\n", (unsigned long)size);

        oldDataSize = lBuffer->nglb_last - lBuffer->nglb_first + 1;
        newDataSize = oldDataSize + size;
        if (newDataSize > lBuffer->nglb_capacity) {
            /* Grow Buffer */
            result = ngemlLineBufferGrow(lBuffer, newDataSize);
            if (result == 0) {
                ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Can't grow the buffer.\n");
                lBuffer->nglb_result = NGEM_CALLBACK_RESULT_FAILED;
                goto call;
            }
        }

        /* Slide data in Line Buffer */
        emptySize = lBuffer->nglb_capacity -
            ((lBuffer->nglb_last + 1) - lBuffer->nglb_buffer);
        if (emptySize < size) {
            NGEM_ASSERT(lBuffer->nglb_first != lBuffer->nglb_last);
            NGEM_ASSERT(lBuffer->nglb_first != lBuffer->nglb_buffer);

            memmove(lBuffer->nglb_buffer, lBuffer->nglb_first, oldDataSize);
            lBuffer->nglb_first = lBuffer->nglb_buffer;
            lBuffer->nglb_last  = lBuffer->nglb_first + oldDataSize - 1;
        }

        /* Copy */
        memcpy(lBuffer->nglb_last, data, size);
        lBuffer->nglb_last += size;
        *lBuffer->nglb_last = '\0';


        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "\"%s\" in buffer.\n", lBuffer->nglb_first);
    }
call:
    result = ngemlLineBufferCall(lBuffer);
    if (result == 0) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName, "Can't callback.\n");
        return;
    }
    
    return;
}

static int
ngemlStringCheckValid(
    void *data,
    size_t size)
{
    int i;
    char *p = data;
#if 0
    static const char fName[] = "ngemlStringCheckValid";
#endif

    NGEM_ASSERT(data != NULL);

    /* Includes NULL character? */
    for (i = 0;i < size;++i) {
        if (p[i] == '\0') {
            return 0;
        }
    }
    return 1;
}

static int
ngemlLineBufferGrow(
    ngemLineBuffer_t *lBuffer,
    size_t necessary)
{
    size_t newCapacity;
    size_t dataSize;
    char *tmp;
    ngLog_t *log;
    static const char fName[] = "ngemlLineBufferGrow";

    log = ngemLogGetDefault();

    NGEM_ASSERT(lBuffer != NULL);

    ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Line Buffer Grow.\n");

    newCapacity = NGEM_MAX(lBuffer->nglb_capacity * 2, necessary);
    tmp = ngiMalloc(newCapacity, log, NULL);
    if (tmp == NULL) {
        ngLogError(log, NGEM_LOGCAT_CALLBACK, fName,
            "Can't allocate storage for buffer.\n");
        lBuffer->nglb_result = NGEM_CALLBACK_RESULT_FAILED;

        return 0;
    }
    dataSize = lBuffer->nglb_last - lBuffer->nglb_first;
    memcpy(tmp, lBuffer->nglb_first, dataSize + 1);
    NGEM_ASSERT(tmp[dataSize] == '\0');
    ngiFree(lBuffer->nglb_buffer, log, NULL);

    lBuffer->nglb_buffer  = tmp;
    lBuffer->nglb_capacity = newCapacity;
    lBuffer->nglb_first   = tmp;
    lBuffer->nglb_last    = ((char *)tmp) + dataSize;

    return 1;
}

static char *
ngemlLineBufferGetLine(
    ngemLineBuffer_t *lBuffer)
{
    char *endline;
    char *line;
#if 0
    static const char fName[] = "ngemlLineBufferGetLine";
#endif

    NGEM_ASSERT(lBuffer);
    NGEM_ASSERT(lBuffer->nglb_result != NGEM_CALLBACK_RESULT_CANCEL);
    NGEM_ASSERT_STRING(lBuffer->nglb_separator);

    line = lBuffer->nglb_first;

    endline = strstr(lBuffer->nglb_first, lBuffer->nglb_separator);
    if (endline != NULL) {
        *endline = '\0';
        lBuffer->nglb_first = endline + strlen(lBuffer->nglb_separator);
    } else if (lBuffer->nglb_first < lBuffer->nglb_last) {
        switch (lBuffer->nglb_result) {
        case NGEM_CALLBACK_RESULT_FAILED:
        case NGEM_CALLBACK_RESULT_EOF:
            if (strlen(lBuffer->nglb_first) == 0) {
                line = NULL;
            } else {
                lBuffer->nglb_first += strlen(lBuffer->nglb_first);
            }
            break;
        case NGEM_CALLBACK_RESULT_CANCEL:
            NGEM_ASSERT_NOTREACHED();
        default:
            line = NULL;
            /* Do nothing */;
        }
    } else {
        line = NULL;
    }

    if (lBuffer->nglb_first == lBuffer->nglb_last) {
        lBuffer->nglb_first = lBuffer->nglb_buffer;
        lBuffer->nglb_last  = lBuffer->nglb_buffer;
    }
    return line;
}

static int
ngemlLineBufferCall(
    ngemLineBuffer_t *lBuffer)
{
    char *line;
    void *arg;
    ngemLineBufferCallbackFunc_t func = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemlLineBufferCall";

    NGEM_ASSERT(lBuffer != NULL);
    NGEM_ASSERT(lBuffer->nglb_callbackValid == false);
    NGEM_ASSERT(ngemlStringCheckValid(lBuffer->nglb_first, lBuffer->nglb_last - lBuffer->nglb_first));

    log = ngemLogGetDefault();

    /* Rest Line(endline terminated) */
    line = ngemlLineBufferGetLine(lBuffer);
    if (line != NULL) {
        arg  = lBuffer->nglb_arg;
        func = lBuffer->nglb_func;
        lBuffer->nglb_arg  = NULL;
        lBuffer->nglb_func = NULL;

        ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Callback with success.\n");
        func(arg, lBuffer, line, NGEM_CALLBACK_RESULT_SUCCESS);
    } else {
        if (lBuffer->nglb_result != NGEM_CALLBACK_RESULT_SUCCESS) {
            /* Error or EOF */
            arg  = lBuffer->nglb_arg;
            func = lBuffer->nglb_func;
            lBuffer->nglb_arg  = NULL;
            lBuffer->nglb_func = NULL;

            ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Callback with error.\n");
            func(arg, lBuffer, NULL, lBuffer->nglb_result);
        } else {
            /* Register Callback */
            lBuffer->nglb_callback = ngemCallbackRead(lBuffer->nglb_fd,
                ngemlLineBufferCallback, lBuffer);
            if (!ngemCallbackIsValid(lBuffer->nglb_callback)) {
                ngLogError(log, NGEM_LOGCAT_CALLBACK, fName,
                    "Can't register callback for reading.\n");
                lBuffer->nglb_result = NGEM_CALLBACK_RESULT_FAILED;
                ngemlLineBufferCall(lBuffer);

                return 0;
            }
            lBuffer->nglb_callbackValid = true;
        }
    }
    return 1;
}
