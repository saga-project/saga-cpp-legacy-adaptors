#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile$ $Revision$ $Date$";
#endif /* NGIS_NO_WARN_RCSID */
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

#include <string.h>
#include <errno.h>

#include "ngInvokeServer.h"
#include "ngisUtility.h"

#define NGISL_LINE_BUFFER_INITIAL_SIZE 1024U

static void ngislLineBufferCallback(void *, int, void *, size_t,
    ngisCallbackResult_t);
static int ngislStringCheckValid(void *, size_t);
static int ngislLineBufferGrow(ngisLineBuffer_t *, size_t);
static int ngislLineBufferCall(ngisLineBuffer_t *);
static char *ngislLineBufferGetLine(ngisLineBuffer_t *);

ngisLineBuffer_t *
ngisLineBufferCreate(
    int fd,
    const char *separator)
{
    ngisLineBuffer_t *new = NULL;
    char *buffer  = NULL;
    char *copySep = NULL;
    static const char fName[] = "ngisLineBufferCreate";

    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(NGIS_STRING_IS_NONZERO(separator));

    new = NGIS_ALLOC(ngisLineBuffer_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName, 
            "Can't allocate storage for line buffer.\n");
        goto error;
    }

    buffer = malloc(NGISL_LINE_BUFFER_INITIAL_SIZE);
    if (buffer == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        goto error;
    }
    buffer[0] = '\0';

    copySep = strdup(separator);
    if (copySep == NULL) {
        ngisErrorPrint(NULL, fName, "Can't copy string.\n");
        goto error;
    }

    new->nglb_fd            = fd;
    new->nglb_buffer        = buffer;
    new->nglb_first         = buffer;
    new->nglb_last          = buffer;
    new->nglb_capacity       = NGISL_LINE_BUFFER_INITIAL_SIZE;
    new->nglb_separator     = copySep;
    new->nglb_func          = NULL;
    new->nglb_arg           = NULL;
    new->nglb_callbackValid = 0;
    new->nglb_result        = NGIS_CALLBACK_RESULT_SUCCESS;
    
    return new;
error:
    NGIS_NULL_CHECK_AND_FREE(copySep);
    NGIS_NULL_CHECK_AND_FREE(buffer);
    NGIS_NULL_CHECK_AND_FREE(new);

    return NULL;
}

void
ngisLineBufferDestroy(
    ngisLineBuffer_t *lBuffer)
{
    static const char fName[] = "ngisLineBufferDestroy";

    NGIS_ASSERT(lBuffer != NULL);

    /* Destroying */
    if (lBuffer->nglb_result == NGIS_CALLBACK_RESULT_CANCEL) {
        ngisWarningPrint(NULL, fName,
            "Do nothing, so this line buffer is destroying.");
        return;
    }

    lBuffer->nglb_result = NGIS_CALLBACK_RESULT_CANCEL;
    if (lBuffer->nglb_callbackValid != 0) {
        ngisCallbackCancel(lBuffer->nglb_callback);
    }

    free(lBuffer->nglb_buffer);
    free(lBuffer->nglb_separator);

    lBuffer->nglb_fd            = -1;
    lBuffer->nglb_buffer        = NULL;
    lBuffer->nglb_first         = NULL;
    lBuffer->nglb_last          = NULL;
    lBuffer->nglb_capacity       = 0U;
    lBuffer->nglb_separator     = NULL;
    lBuffer->nglb_func          = NULL;
    lBuffer->nglb_arg           = NULL;
    lBuffer->nglb_callbackValid = 0;
    lBuffer->nglb_result        = NGIS_CALLBACK_RESULT_CANCEL;

    NGIS_FREE(lBuffer);
    
    return;
}

int
ngisLineBufferReadLine(
    ngisLineBuffer_t *lBuffer,
    ngisLineBufferCallbackFunc_t func,
    void *arg)
{
    int result;
    static const char fName[] = "ngisLineBufferReadLine";

    NGIS_ASSERT(lBuffer != NULL);
    NGIS_ASSERT(func != NULL);

    /* Registered? */
    if (lBuffer->nglb_func != NULL) {
        ngisErrorPrint(NULL, fName,
            "Callback is already registered(fd=%d).\n", lBuffer->nglb_fd);
        return 0;
    }

    if (lBuffer->nglb_result == NGIS_CALLBACK_RESULT_CANCEL) {
        ngisErrorPrint(NULL, fName, "Callback is already canceled.\n");
        return 0;
    }

    lBuffer->nglb_func = func;
    lBuffer->nglb_arg  = arg;

    result = ngislLineBufferCall(lBuffer);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "Can't callback.\n");
        return 0;
    }
    return 1;
}

static void 
ngislLineBufferCallback(
    void *arg,
    int fd,
    void *data,
    size_t size,
    ngisCallbackResult_t cResult)
{
    ngisLineBuffer_t *lBuffer = (ngisLineBuffer_t *)arg;
    size_t emptySize;
    size_t newDataSize;
    size_t oldDataSize;
    int result;
    static const char fName[] = "ngislLineBufferCallback";

    lBuffer->nglb_callbackValid = 0;

    NGIS_ASSERT(lBuffer != NULL);

    lBuffer->nglb_result = cResult;
    switch (cResult) {
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(NULL, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisDebugPrint(NULL, fName, "EOF.\n");
        break;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(NULL, fName, "Can't read the line.\n");
        break;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    if (cResult == NGIS_CALLBACK_RESULT_SUCCESS) {
        NGIS_ASSERT(data != NULL);
        /* Includes NULL charactor? */
        if (!ngislStringCheckValid(data, size)) {
            ngisErrorPrint(NULL, fName,
                "There are invalid characters in the line.\n");
            lBuffer->nglb_result = NGIS_CALLBACK_RESULT_FAILED;
            goto call;
        }
        ngisDebugPrint(NULL, fName, "Read %lu bytes.\n", (unsigned long)size);

        oldDataSize = lBuffer->nglb_last - lBuffer->nglb_first + 1;
        newDataSize = oldDataSize + size;
        if (newDataSize > lBuffer->nglb_capacity) {
            /* Grow Buffer */
            result = ngislLineBufferGrow(lBuffer, newDataSize);
            if (result == 0) {
                ngisErrorPrint(NULL, fName, "Can't grow the buffer.\n");
                lBuffer->nglb_result = NGIS_CALLBACK_RESULT_FAILED;
                goto call;
            }
        }

        /* Slide data in Line Buffer */
        emptySize = lBuffer->nglb_capacity -
            ((lBuffer->nglb_last + 1) - lBuffer->nglb_buffer);
        if (emptySize < size) {
            NGIS_ASSERT(lBuffer->nglb_first != lBuffer->nglb_last);
            NGIS_ASSERT(lBuffer->nglb_first != lBuffer->nglb_buffer);
            memmove(lBuffer->nglb_first, lBuffer->nglb_buffer, oldDataSize);
            lBuffer->nglb_first = lBuffer->nglb_buffer;
            lBuffer->nglb_last  = lBuffer->nglb_first + oldDataSize;
        }

        /* Copy */
        memcpy(lBuffer->nglb_last, data, size);
        lBuffer->nglb_last += size;
        *lBuffer->nglb_last = '\0';


        ngisDebugPrint(NULL, fName, "\"%s\" in buffer.\n", lBuffer->nglb_first);
    }
call:
    result = ngislLineBufferCall(lBuffer);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "Can't callback.\n");
        return;
    }
    
    return;
}

static int
ngislStringCheckValid(
    void *data,
    size_t size)
{
    int i;
    char *p = data;
#if 0
    static const char fName[] = "ngislStringCheckValid";
#endif

    NGIS_ASSERT(data != NULL);

    /* Includes NULL charactor? */
    for (i = 0;i < size;++i) {
        if (p[i] == '\0') {
            return 0;
        }
    }
    return 1;
}

static int
ngislLineBufferGrow(
    ngisLineBuffer_t *lBuffer,
    size_t necessary)
{
    size_t newCapacity;
    size_t dataSize;
    char *tmp;
    static const char fName[] = "ngislLineBufferGrow";

    newCapacity = NGIS_MAX(lBuffer->nglb_capacity * 2, necessary);
    tmp = malloc(newCapacity);
    if (tmp == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        lBuffer->nglb_result = NGIS_CALLBACK_RESULT_FAILED;

        return 0;
    }
    dataSize = lBuffer->nglb_last - lBuffer->nglb_first;
    memcpy(tmp, lBuffer->nglb_first, dataSize + 1);
    free(lBuffer->nglb_buffer);

    lBuffer->nglb_buffer  = tmp;
    lBuffer->nglb_capacity = newCapacity;
    lBuffer->nglb_first   = tmp;
    lBuffer->nglb_last    = (char *)tmp + dataSize;

    return 1;
}

static char *
ngislLineBufferGetLine(
    ngisLineBuffer_t *lBuffer)
{
    char *endline;
    char *line;
#if 0
    static const char fName[] = "ngislLineBufferGetLine";
#endif

    NGIS_ASSERT(lBuffer);
    NGIS_ASSERT(lBuffer->nglb_result != NGIS_CALLBACK_RESULT_CANCEL);

    line = lBuffer->nglb_first;

    endline = strstr(lBuffer->nglb_first, lBuffer->nglb_separator);
    if (endline != NULL) {
        *endline = '\0';
        lBuffer->nglb_first = endline + strlen(lBuffer->nglb_separator);
    } else if (lBuffer->nglb_first < lBuffer->nglb_last) {
        switch (lBuffer->nglb_result) {
        case NGIS_CALLBACK_RESULT_FAILED:
        case NGIS_CALLBACK_RESULT_EOF:
            lBuffer->nglb_first += strlen(lBuffer->nglb_first);
            break;
        case NGIS_CALLBACK_RESULT_CANCEL:
            NGIS_ASSERT_NOTREACHED();
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
ngislLineBufferCall(
    ngisLineBuffer_t *lBuffer)
{
    char *line;
    void *arg;
    ngisLineBufferCallbackFunc_t func;
    static const char fName[] = "ngislLineBufferCall";

    NGIS_ASSERT(lBuffer != NULL);
    NGIS_ASSERT(lBuffer->nglb_callbackValid == 0);

    /* Rest Line(endline terminated) */
    line = ngislLineBufferGetLine(lBuffer);
    if (line != NULL) {
        arg  = lBuffer->nglb_arg;
        func = lBuffer->nglb_func;
        lBuffer->nglb_arg  = NULL;
        lBuffer->nglb_func = NULL;

        ngisDebugPrint(NULL, fName, "Callback with success.\n");
        func(arg, lBuffer, line, NGIS_CALLBACK_RESULT_SUCCESS);
    } else {
        if (lBuffer->nglb_result != NGIS_CALLBACK_RESULT_SUCCESS) {
            /* Error or EOF */
            arg  = lBuffer->nglb_arg;
            func = lBuffer->nglb_func;
            lBuffer->nglb_arg  = NULL;
            lBuffer->nglb_func = NULL;

            ngisDebugPrint(NULL, fName, "Callback with error.\n");
            func(arg, lBuffer, NULL, lBuffer->nglb_result);
        } else {
            /* Register Callback */
            lBuffer->nglb_callback = ngisCallbackRead(lBuffer->nglb_fd,
                ngislLineBufferCallback, lBuffer);
            if (!ngisCallbackIsValid(lBuffer->nglb_callback)) {
                ngisErrorPrint(NULL, fName,
                    "Can't register callback for reading.\n");
                lBuffer->nglb_result = NGIS_CALLBACK_RESULT_FAILED;
                ngislLineBufferCall(lBuffer);

                return 0;
            }
            lBuffer->nglb_callbackValid = 1;
        }
    }
    return 1;
}
