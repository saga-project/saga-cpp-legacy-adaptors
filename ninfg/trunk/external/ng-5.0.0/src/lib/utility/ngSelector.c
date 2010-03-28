/*
 * $RCSfile: ngSelector.c,v $ $Revision: 1.2 $ $Date: 2008/01/17 09:42:45 $
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
 * Module of Selector for Ninf-G.
 */

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngSelector.c,v $ $Revision: 1.2 $ $Date: 2008/01/17 09:42:45 $")

/**
 * Data.
 */


/**
 * Prototype declaration of internal functions.
 */

/**
 * Functions.
 */

/**
 * Selector: Initialize the Functions members.
 */
void
ngiSelectorFunctionsInitializeMember(
    ngiSelectorFunctions_t *funcs)
{
    /* Check the arguments */
    assert(funcs != NULL);

    funcs->ngsc_selectorConstruct = NULL;
    funcs->ngsc_selectorDestruct = NULL;
    funcs->ngsc_selectorResize = NULL;
    funcs->ngsc_selectorClear = NULL;
    funcs->ngsc_selectorSet = NULL;
    funcs->ngsc_selectorSetLast = NULL;
    funcs->ngsc_selectorWait = NULL;
    funcs->ngsc_selectorGet = NULL;
    funcs->ngsc_selectorGetLast = NULL;
}

/**
 * Selector: Construct.
 */
void *
ngiSelectorConstruct(
    ngiSelectorFunctions_t *funcs,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorConstruct";
    ngiSelectorConstruct_t func;
    void *selector;

    func = NULL;
    selector = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorConstruct;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    selector = (*func)(log, error);
    if (selector == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Construct the Selector failed.\n"); 
        goto error;
    }

    /* Success */
    return selector;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Selector: Destruct.
 */
int
ngiSelectorDestruct(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorDestruct";
    ngiSelectorDestruct_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorDestruct;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destruct the Selector failed.\n"); 
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
 * Selector: Resize.
 */
int
ngiSelectorResize(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    int newSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorResize";
    ngiSelectorResize_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorResize;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, newSize, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Resize the Selector failed.\n"); 
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
 * Selector: Clear.
 */
int
ngiSelectorClear(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorClear";
    ngiSelectorClear_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorClear;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Clear the Selector failed.\n"); 
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
 * Selector: Set.
 */
int
ngiSelectorSet(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    int idx,
    int fd,
    int flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorSet";
    ngiSelectorSet_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorSet;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, idx, fd, flags, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Set the Selector failed.\n"); 
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
 * Selector: Set Last.
 */
int
ngiSelectorSetLast(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    int timeoutMilliSec,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorSetLast";
    ngiSelectorSetLast_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorSetLast;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, timeoutMilliSec, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "SetLast the Selector failed.\n"); 
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
 * Selector: Wait.
 */
int
ngiSelectorWait(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorWait";
    ngiSelectorWait_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorWait;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Wait the Selector failed.\n"); 
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
 * Selector: Get.
 */
int
ngiSelectorGet(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    int fd,
    int *idx,
    int *flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorGet";
    ngiSelectorGet_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorGet;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, fd, idx, flags, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Get the Selector failed.\n"); 
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
 * Selector: Get Last.
 */
int
ngiSelectorGetLast(
    ngiSelectorFunctions_t *funcs,
    void *selector,
    int *isFds,
    int *fds,
    int *bits,
    int *isReadCloseHup,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorGetLast";
    ngiSelectorGetLast_t func;
    int result;

    func = NULL;

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    if (selector == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    func = funcs->ngsc_selectorGetLast;

    if (func == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector function is NULL.\n"); 
        goto error;
    }

    result = (*func)(selector, isFds, fds, bits, isReadCloseHup, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "GetLast the Selector failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#ifdef NGI_POLL_ENABLED

/**
 * Selector Poll: A Selector implemented by poll().
 */

/**
 * Data.
 */
typedef struct nglSelectorPoll_s {
    int ngsp_bufSize;
    struct pollfd *ngsp_pollFdBuf;
    int ngsp_nFds;
    int ngsp_timeout;
    int ngsp_result;
} nglSelectorPoll_t;


/**
 * Prototype declaration of internal functions.
 */

static void *nglSelectorPollConstruct(
    ngLog_t *log, int *error);
static int nglSelectorPollDestruct(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorPollInitialize(
    nglSelectorPoll_t *selector, ngLog_t *log, int *error);
static int nglSelectorPollFinalize(
    nglSelectorPoll_t *selector, ngLog_t *log, int *error);
static void nglSelectorPollInitializeMember(
    nglSelectorPoll_t *selector);
static int nglSelectorPollResize(
    void *arg, int newSize, ngLog_t *log, int *error);
static int nglSelectorPollClear(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorPollSet(
    void *arg, int idx, int fd, int flags, ngLog_t *log, int *error);
static int nglSelectorPollSetLast(
    void *arg, int timeoutMilliSec, ngLog_t *log, int *error);
static int nglSelectorPollWait(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorPollGet(
    void *arg, int fd, int *idx, int *flags, ngLog_t *log, int *error);
static int nglSelectorPollGetLast(
    void *arg, int *isFds, int *fds, int *bits, int *isReadCloseHup,
    ngLog_t *log, int *error);

/**
 * Functions.
 */

/**
 * Selector Poll: Set Functions.
 */
int
ngiSelectorPollFunctionsSet(
    ngiSelectorFunctions_t *funcs,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorPollFunctionsSet";

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    ngiSelectorFunctionsInitializeMember(funcs);

    funcs->ngsc_selectorConstruct = nglSelectorPollConstruct;
    funcs->ngsc_selectorDestruct = nglSelectorPollDestruct;
    funcs->ngsc_selectorResize = nglSelectorPollResize;
    funcs->ngsc_selectorClear = nglSelectorPollClear;
    funcs->ngsc_selectorSet = nglSelectorPollSet;
    funcs->ngsc_selectorSetLast = nglSelectorPollSetLast;
    funcs->ngsc_selectorWait = nglSelectorPollWait;
    funcs->ngsc_selectorGet = nglSelectorPollGet;
    funcs->ngsc_selectorGetLast = nglSelectorPollGetLast;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Construct.
 */
static void *
nglSelectorPollConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollConstruct";
    nglSelectorPoll_t *selector;
    int result;

    /* Allocate */
    selector = NGI_ALLOCATE(nglSelectorPoll_t, log, error);
    if (selector == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Allocate the Selector Poll failed.\n");
        goto error;
    }

    /* Initialize */
    result = nglSelectorPollInitialize(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Initialize the Selector Poll failed.\n");
        goto error;
    }

    /* Success */
    return (void *)selector;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Selector Poll: Destruct.
 */
static int
nglSelectorPollDestruct(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollDestruct";
    nglSelectorPoll_t *selector;
    int result;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    /* Finalize */
    result = nglSelectorPollFinalize(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Finalize the Selector Poll failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(nglSelectorPoll_t, selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Deallocate the Selector Poll failed.\n");
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
 * Selector Poll: Initialize.
 */
static int
nglSelectorPollInitialize(
    nglSelectorPoll_t *selector,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollInitialize";
    struct pollfd *pollFdBuf;

    /* Check the arguments */
    assert(selector != NULL);

    pollFdBuf = NULL;

    nglSelectorPollInitializeMember(selector);

    pollFdBuf = ngiCalloc(1, sizeof(struct pollfd), log, error);
    if (pollFdBuf == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Allocate the pollfd failed.\n");
        goto error;
    }
 
    selector->ngsp_bufSize = 1;
    selector->ngsp_pollFdBuf = pollFdBuf;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Finalize.
 */
static int
nglSelectorPollFinalize(
    nglSelectorPoll_t *selector,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollFinalize";
    int result;

    /* Check the arguments */
    assert(selector != NULL);

    /* Deallocate */
    result = ngiFree(selector->ngsp_pollFdBuf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Deallocate the pollfd failed.\n");
        goto error;
    }
    selector->ngsp_pollFdBuf = NULL;

    nglSelectorPollInitializeMember(selector);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Initialize the members.
 */
static void
nglSelectorPollInitializeMember(
    nglSelectorPoll_t *selector)
{
    /* Check the arguments */
    assert(selector != NULL);

    selector->ngsp_bufSize = 0;
    selector->ngsp_pollFdBuf = NULL;
    selector->ngsp_nFds = 0;
    selector->ngsp_timeout = 0;
    selector->ngsp_result = 0;
}

/**
 * Selector Poll: Resize.
 */
static int
nglSelectorPollResize(
    void *arg,
    int newSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollResize";
    nglSelectorPoll_t *selector;
    struct pollfd *pollFdBuf;
    int bufSize;

    bufSize = 0;
    pollFdBuf = NULL;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    bufSize = selector->ngsp_bufSize;
    pollFdBuf = selector->ngsp_pollFdBuf;

    if (newSize <= bufSize) {
        /* Just increase. */

        /* Success */
        return 1;
    }

    pollFdBuf = ngiRealloc(
        pollFdBuf, sizeof(struct pollfd) * newSize, log, error);
    if (pollFdBuf == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Resize the pollfd buffer failed.\n");
        goto error;
    }

    selector->ngsp_bufSize = newSize;
    selector->ngsp_pollFdBuf = pollFdBuf;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Clear.
 */
static int
nglSelectorPollClear(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollClear";
    nglSelectorPoll_t *selector;
    struct pollfd *pollFdBuf;
    int bufSize;

    bufSize = 0;
    pollFdBuf = NULL;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    bufSize = selector->ngsp_bufSize;
    pollFdBuf = selector->ngsp_pollFdBuf;

    memset(pollFdBuf, 0, sizeof(struct pollfd) * bufSize);

    selector->ngsp_nFds = 0;
    selector->ngsp_timeout = 0;
    selector->ngsp_result = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Set.
 */
static int
nglSelectorPollSet(
    void *arg,
    int idx,
    int fd,
    int flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollSet";
    nglSelectorPoll_t *selector;
    struct pollfd *pollFdBuf;
    int bufSize;

    bufSize = 0;
    pollFdBuf = NULL;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    bufSize = selector->ngsp_bufSize;
    pollFdBuf = selector->ngsp_pollFdBuf;

    /* Check the arguments */
    if (idx < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The index %d invalid.\n", idx); 
        goto error;
    }
    if (idx >= bufSize) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The poll buffer index %d >= size %d.\n", idx, bufSize); 
        goto error;
    }

    pollFdBuf[idx].fd = fd;

    pollFdBuf[idx].events = 0;
    if (flags & NGI_SELECTOR_FLAG_IN) {
        pollFdBuf[idx].events |= POLLIN;
    }
    if (flags & NGI_SELECTOR_FLAG_OUT) {
        pollFdBuf[idx].events |= POLLOUT;
    }
    if (flags & ~(NGI_SELECTOR_FLAG_IN | NGI_SELECTOR_FLAG_OUT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The flags %d invalid.\n", flags); 
        goto error;
    }

    pollFdBuf[idx].revents = 0;

    if (idx >= selector->ngsp_nFds) {
        selector->ngsp_nFds = idx + 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Set Last.
 */
static int
nglSelectorPollSetLast(
    void *arg,
    int timeoutMilliSec,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollSetLast";
    nglSelectorPoll_t *selector;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    selector->ngsp_timeout = timeoutMilliSec;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Wait.
 * Note: Wait must treat EINTR.
 */
static int
nglSelectorPollWait(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollWait";
    nglSelectorPoll_t *selector;
    int result, errorNumber;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    result = poll(
        selector->ngsp_pollFdBuf,
        selector->ngsp_nFds,
        selector->ngsp_timeout);

    selector->ngsp_result = result;

    if (result < 0) {
        errorNumber = errno;
        if (errorNumber == EINTR) {
            selector->ngsp_result = 0;
            return 1;
        }

        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "%s failed: %d: %s.\n",
            "poll()", errorNumber, strerror(errorNumber));
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
 * Selector Poll: Get.
 * Get searches fd entry from idx'th entry of buffer.
 */
static int
nglSelectorPollGet(
    void *arg,
    int fd,
    int *idx,
    int *flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollGet";
    struct pollfd *pollFdBuf, *pollFd;
    nglSelectorPoll_t *selector;
    int i;

    pollFdBuf = NULL;
    pollFd = NULL;

    /* Check the arguments */
    if ((arg == NULL) || (idx == NULL) || (flags == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The %s is NULL.\n",
            ((arg == NULL) ? "selector" :
            ((idx == NULL) ? "index" :
            ((flags == NULL) ? "flags" : "unknown")))); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The fd is negative (%d).\n", fd); 
        goto error;
    }

    if (*idx >= selector->ngsp_nFds) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The Selector index %d overflow.\n", *idx); 
        goto error;
    }

    i = *idx;
    pollFdBuf = selector->ngsp_pollFdBuf;

    while (pollFdBuf[i].fd != fd) {
        i++;

        if (i >= selector->ngsp_nFds) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The fd %d not found on the selector.\n", fd); 
            goto error;
        }
    }

    pollFd = &pollFdBuf[i];

    *flags = 0;
    if (pollFd->revents & POLLIN) {
        *flags |= NGI_SELECTOR_FLAG_IN;
    }
    if (pollFd->revents & POLLOUT) {
        *flags |= NGI_SELECTOR_FLAG_OUT;
    }
    if (pollFd->revents & POLLHUP) {
        *flags |= NGI_SELECTOR_FLAG_HUP;
    }
    if (pollFd->revents & ~(POLLIN | POLLOUT | POLLHUP)) {
        *flags |= NGI_SELECTOR_FLAG_OTHER;

        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The selector got unexpected poll event 0x%x.\n",
            pollFd->revents); 
    }

    i++;
    *idx = i;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Poll: Get Last.
 * isReadCloseHup points the poll() behavior.
 */
static int
nglSelectorPollGetLast(
    void *arg,
    int *isFds,
    int *fds,
    int *bits,
    int *isReadCloseHup,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorPollGetLast";
    nglSelectorPoll_t *selector;

    /* Check the arguments */
    if ((arg == NULL) || (isFds == NULL) || (fds == NULL) ||
        (bits == NULL) || (isReadCloseHup == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The %s is NULL.\n",
            ((arg == NULL) ? "selector" :
            ((isFds == NULL) ? "isFds" :
            ((fds == NULL) ? "fds" :
            ((bits == NULL) ? "bits" :
            ((isReadCloseHup == NULL) ? "isReadCloseHup" :
            "unknown")))))); 
        goto error;
    }

    selector = (nglSelectorPoll_t *)arg;

    *isFds = 1; /* poll() returns number of event occurred fds. */
    *fds = selector->ngsp_result;
    *bits = 0;

    /**
     * Note: On Linux and Solaris poll(), pipe close is notified by
     * POLLHUP and then, EOF by read().
     * POLLIN is still effective while the available data exists.
     */
    *isReadCloseHup = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#endif /* NGI_POLL_ENABLED */

#ifdef NGI_SELECT_ENABLED

/**
 * Selector Select: A Selector implemented by select().
 */

/**
 * Data.
 */
typedef struct nglSelectorSelect_s {
    int ngss_nfds;
    fd_set ngss_readfds;
    fd_set ngss_writefds;
    int ngss_timeout;
    struct timeval ngss_timeoutTimeVal;
    int ngss_result;
} nglSelectorSelect_t;


/**
 * Prototype declaration of internal functions.
 */

static void *nglSelectorSelectConstruct(
    ngLog_t *log, int *error);
static int nglSelectorSelectDestruct(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorSelectInitialize(
    nglSelectorSelect_t *selector, ngLog_t *log, int *error);
static int nglSelectorSelectFinalize(
    nglSelectorSelect_t *selector, ngLog_t *log, int *error);
static void nglSelectorSelectInitializeMember(
    nglSelectorSelect_t *selector);
static int nglSelectorSelectResize(
    void *arg, int newSize, ngLog_t *log, int *error);
static int nglSelectorSelectClear(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorSelectSet(
    void *arg, int idx, int fd, int flags, ngLog_t *log, int *error);
static int nglSelectorSelectSetLast(
    void *arg, int timeoutMilliSec, ngLog_t *log, int *error);
static int nglSelectorSelectWait(
    void *arg, ngLog_t *log, int *error);
static int nglSelectorSelectGet(
    void *arg, int fd, int *idx, int *flags, ngLog_t *log, int *error);
static int nglSelectorSelectGetLast(
    void *arg, int *isFds, int *fds, int *bits, int *isReadCloseHup,
    ngLog_t *log, int *error);

/**
 * Functions.
 */

/**
 * Selector Select: Set Functions.
 */
int
ngiSelectorSelectFunctionsSet(
    ngiSelectorFunctions_t *funcs,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSelectorSelectFunctionsSet";

    /* Check the arguments */
    if (funcs == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector functions is NULL.\n"); 
        goto error;
    }

    ngiSelectorFunctionsInitializeMember(funcs);

    funcs->ngsc_selectorConstruct = nglSelectorSelectConstruct;
    funcs->ngsc_selectorDestruct = nglSelectorSelectDestruct;
    funcs->ngsc_selectorResize = nglSelectorSelectResize;
    funcs->ngsc_selectorClear = nglSelectorSelectClear;
    funcs->ngsc_selectorSet = nglSelectorSelectSet;
    funcs->ngsc_selectorSetLast = nglSelectorSelectSetLast;
    funcs->ngsc_selectorWait = nglSelectorSelectWait;
    funcs->ngsc_selectorGet = nglSelectorSelectGet;
    funcs->ngsc_selectorGetLast = nglSelectorSelectGetLast;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Construct.
 */
static void *
nglSelectorSelectConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectConstruct";
    nglSelectorSelect_t *selector;
    int result;

    /* Allocate */
    selector = NGI_ALLOCATE(nglSelectorSelect_t, log, error);
    if (selector == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Allocate the Selector Select failed.\n");
        goto error;
    }

    /* Initialize */
    result = nglSelectorSelectInitialize(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Initialize the Selector Select failed.\n");
        goto error;
    }

    /* Success */
    return (void *)selector;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Selector Select: Destruct.
 */
static int
nglSelectorSelectDestruct(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectDestruct";
    nglSelectorSelect_t *selector;
    int result;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    /* Finalize */
    result = nglSelectorSelectFinalize(selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Finalize the Selector Select failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(nglSelectorSelect_t, selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Deallocate the Selector Select failed.\n");
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
 * Selector Select: Initialize.
 */
static int
nglSelectorSelectInitialize(
    nglSelectorSelect_t *selector,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(selector != NULL);

    nglSelectorSelectInitializeMember(selector);

    /* Success */
    return 1;
}

/**
 * Selector Select: Finalize.
 */
static int
nglSelectorSelectFinalize(
    nglSelectorSelect_t *selector,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(selector != NULL);

    nglSelectorSelectInitializeMember(selector);

    /* Success */
    return 1;
}

/**
 * Selector Select: Initialize the members.
 */
static void
nglSelectorSelectInitializeMember(
    nglSelectorSelect_t *selector)
{
    /* Check the arguments */
    assert(selector != NULL);

    selector->ngss_nfds = 0;
    selector->ngss_result = 0;
}

/**
 * Selector Select: Resize.
 */
static int
nglSelectorSelectResize(
    void *arg,
    int newSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectResize";

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    /* No buffer resize required. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Clear.
 */
static int
nglSelectorSelectClear(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectClear";
    nglSelectorSelect_t *selector;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    selector->ngss_nfds = 0;
    FD_ZERO(&selector->ngss_readfds);
    FD_ZERO(&selector->ngss_writefds);
    selector->ngss_timeout = 0;
    selector->ngss_timeoutTimeVal.tv_sec = 0;
    selector->ngss_timeoutTimeVal.tv_usec = 0;
    selector->ngss_result = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Set.
 */
static int
nglSelectorSelectSet(
    void *arg,
    int idx,
    int fd,
    int flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectSet";
    nglSelectorSelect_t *selector;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    /* Check the arguments */
    if (idx < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The index %d invalid.\n", idx); 
        goto error;
    }

    if (flags & NGI_SELECTOR_FLAG_IN) {
        FD_SET(fd, &selector->ngss_readfds);
    }
    if (flags & NGI_SELECTOR_FLAG_OUT) {
        FD_SET(fd, &selector->ngss_writefds);
    }
    if (flags & ~(NGI_SELECTOR_FLAG_IN | NGI_SELECTOR_FLAG_OUT)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The flags %d invalid.\n", flags); 
        goto error;
    }

    if (fd >= selector->ngss_nfds) {
        selector->ngss_nfds = fd + 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Set Last.
 */
static int
nglSelectorSelectSetLast(
    void *arg,
    int timeoutMilliSec,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectSetLast";
    nglSelectorSelect_t *selector;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    selector->ngss_timeout = timeoutMilliSec;
    if (timeoutMilliSec >= 0) {
        selector->ngss_timeoutTimeVal.tv_sec = timeoutMilliSec / 1000;
        selector->ngss_timeoutTimeVal.tv_usec = (timeoutMilliSec % 1000) * 1000;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Wait.
 * Note: Wait must treat EINTR.
 */
static int
nglSelectorSelectWait(
    void *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectWait";
    nglSelectorSelect_t *selector;
    struct timeval *tv;
    int result, errorNumber;

    /* Check the arguments */
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The selector is NULL.\n"); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    tv = NULL;
    if (selector->ngss_timeout >= 0) {
        tv = &selector->ngss_timeoutTimeVal;
    }

    result = select(
        selector->ngss_nfds,
        &(selector->ngss_readfds),
        &(selector->ngss_writefds),
        NULL,
        tv);

    selector->ngss_result = result;

    if (result < 0) {
        errorNumber = errno;
        if (errorNumber == EINTR) {
            selector->ngss_result = 0;
            return 1;
        }

        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "%s failed: %d: %s.\n",
            "select()", errorNumber, strerror(errorNumber));
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
 * Selector Select: Get.
 * Get searches fd entry from idx'th entry of buffer.
 */
static int
nglSelectorSelectGet(
    void *arg,
    int fd,
    int *idx,
    int *flags,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectGet";
    nglSelectorSelect_t *selector;

    /* Check the arguments */
    if ((arg == NULL) || (idx == NULL) || (flags == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The %s is NULL.\n",
            ((arg == NULL) ? "selector" :
            ((idx == NULL) ? "index" :
            ((flags == NULL) ? "flags" : "unknown")))); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The fd is negative (%d).\n", fd); 
        goto error;
    }

    if (fd >= selector->ngss_nfds) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The Selector fd %d overflow.\n", fd); 
        goto error;
    }

    /* Do not touch *idx on select. */

    *flags = 0;
    if (FD_ISSET(fd, &(selector->ngss_readfds)) != 0) {
        *flags |= NGI_SELECTOR_FLAG_IN;
    }
    if (FD_ISSET(fd, &(selector->ngss_writefds)) != 0) {
        *flags |= NGI_SELECTOR_FLAG_OUT;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Selector Select: Get Last.
 * isReadCloseHup points the poll() behavior.
 */
static int
nglSelectorSelectGetLast(
    void *arg,
    int *isFds,
    int *fds,
    int *bits,
    int *isReadCloseHup,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglSelectorSelectGetLast";
    nglSelectorSelect_t *selector;

    /* Check the arguments */
    if ((arg == NULL) || (isFds == NULL) || (fds == NULL) ||
        (bits == NULL) || (isReadCloseHup == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The %s is NULL.\n",
            ((arg == NULL) ? "selector" :
            ((isFds == NULL) ? "isFds" :
            ((fds == NULL) ? "fds" :
            ((bits == NULL) ? "bits" :
            ((isReadCloseHup == NULL) ? "isReadCloseHup" :
            "unknown")))))); 
        goto error;
    }

    selector = (nglSelectorSelect_t *)arg;

    *isFds = 0; /* select() returns number of event occurred bits. */
    *fds = 0;
    *bits = selector->ngss_result;
    *isReadCloseHup = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#endif /* NGI_SELECT_ENABLED */

