/*
 * $RCSfile: ngemCallbackManager.h,v $ $Revision: 1.2 $ $Date: 2008/02/25 05:21:46 $
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
#ifndef NGEM_CALLBACK_MANAGER_H
#define NGEM_CALLBACK_MANAGER_H

#include "ngemEnvironment.h"
#include "ngemType.h"

#include "ngemUtility.h"
#include "ngemList.h"

#define NGEM_LOGCAT_CALLBACK    "EM Callback"
#define NGEM_LOGCAT_LINEBUFFER  "EM Line Buffer"

/* Enum */
typedef enum ngemCallbackResult_e {
    NGEM_CALLBACK_RESULT_SUCCESS,
    NGEM_CALLBACK_RESULT_CANCEL,
    NGEM_CALLBACK_RESULT_EOF,
    NGEM_CALLBACK_RESULT_FAILED
} ngemCallbackResult_t;

typedef struct ngemlCallbackEntity_s ngemlCallbackEntity_t;
typedef struct ngemLineBuffer_s ngemLineBuffer_t;

typedef void (*ngemReadCallbackFunc_t)
    (void *, int, void *, size_t, ngemCallbackResult_t);
typedef void (*ngemWriteCallbackFunc_t)
    (void *, int, void *, size_t, size_t, ngemCallbackResult_t);
typedef void (*ngemTimerCallbackFunc_t)(void *, ngemCallbackResult_t);
typedef void (*ngemWaitCallbackFunc_t)
    (void *, pid_t, int, ngemCallbackResult_t);
typedef void (*ngemWriteStringCallbackFunc_t)
    (void *, int, ngemCallbackResult_t);
typedef void (*ngemLineBufferCallbackFunc_t)
    (void *, ngemLineBuffer_t *, char *, ngemCallbackResult_t);

NGEM_DECLARE_LIST_OF(ngemSourceBase_t);
NGEM_DECLARE_LIST_OF(ngemTimer_t);
NGEM_DECLARE_LIST_OF(ngemProcessWaiter_t);
NGEM_DECLARE_LIST_OF(char);
NGEM_DECLARE_LIST_OF(ngemlCallbackEntity_t);

typedef NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) ngemCallback_t;

/**
 * Callback by reading each line.
 */
struct ngemLineBuffer_s {
    int                          nglb_fd;
    char                        *nglb_buffer;
    char                        *nglb_first;
    char                        *nglb_last;
    size_t                       nglb_capacity;
    char                        *nglb_separator;
    ngemLineBufferCallbackFunc_t nglb_func;
    void                        *nglb_arg;
    ngemCallback_t               nglb_callback;
    bool                         nglb_callbackValid;
    ngemCallbackResult_t         nglb_result;
};

ngemResult_t ngemCallbackManagerInitialize(void);
ngemResult_t ngemCallbackManagerRun(void);
ngemResult_t ngemCallbackManagerFinalize(void);

/* Callback Manager */
ngemCallback_t ngemCallbackRead(int, ngemReadCallbackFunc_t, void *arg);
ngemCallback_t ngemCallbackWrite(
    int, ngemWriteCallbackFunc_t, void *, size_t, void *);
ngemCallback_t ngemCallbackWriteFormat(int, ngemWriteStringCallbackFunc_t,
    void *, const char *, ...) NG_ATTRIBUTE_PRINTF(4, 5);
ngemCallback_t ngemCallbackWriteVformat(int, ngemWriteStringCallbackFunc_t,
    void *, const char *, va_list);
ngemCallback_t ngemCallbackSetTimer(int, ngemTimerCallbackFunc_t, void *);
ngemCallback_t ngemCallbackWait(pid_t, ngemWaitCallbackFunc_t, void *);

void ngemCallbackSetDaemon(ngemCallback_t);

void ngemCallbackCancel(ngemCallback_t);
bool ngemCallbackIsValid(ngemCallback_t);

ngemLineBuffer_t *ngemLineBufferCreate(int, const char *);
void ngemLineBufferDestroy(ngemLineBuffer_t *);

ngemResult_t ngemLineBufferReadLine(
    ngemLineBuffer_t *, ngemLineBufferCallbackFunc_t, void *);

#endif
