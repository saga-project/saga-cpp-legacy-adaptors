#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclObserve.c,v $ $Revision: 1.5 $ $Date: 2005/07/05 11:56:15 $";
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
 * Module of Observe Thread for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */

static ngcliObserveItem_t *ngcllObserveItemAllocate(
    ngclContext_t *, int *);
static int ngcllObserveItemFree(
    ngclContext_t *, ngcliObserveItem_t *, int *);
static int ngcllObserveItemInitialize(
    ngclContext_t *, ngcliObserveItem_t *, int *);
static int ngcllObserveItemFinalize(
    ngclContext_t *, ngcliObserveItem_t *, int *);
static void ngcllObserveItemInitializeMember(ngcliObserveItem_t *);
static void ngcllObserveItemInitializePointer(ngcliObserveItem_t *);
static int ngcllObserveItemRegister(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);
static int ngcllObserveItemUnregister(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);

static int ngcllObserveThreadInitialize(
    ngclContext_t *, ngcliObserveThread_t *, int *);
static int ngcllObserveThreadFinalize(
    ngclContext_t *, ngcliObserveThread_t *, int *);
static int ngcllObserveThreadInitializeMutexAndCond(
    ngclContext_t *, ngcliObserveThread_t *, int *);
static int ngcllObserveThreadFinalizeMutexAndCond(
    ngclContext_t *, ngcliObserveThread_t *, int *);
static void ngcllObserveThreadInitializeMember(ngcliObserveThread_t *);
static void ngcllObserveThreadInitializePointer(ngcliObserveThread_t *);

#ifdef NG_PTHREAD
static void * ngcllObserveThread(void *);
static int ngcllObserveThreadEventTimeChangeRequest(
    ngclContext_t *, ngcliObserveThread_t *, ngcliObserveItem_t *, int *);
static int ngcllObserveThreadStop(ngclContext_t *, int *);
static int ngcllObserveThreadGetSleepTime(
    ngclContext_t *, ngcliObserveThread_t *, int *, int *);
static int ngcllObserveThreadProceedEvents(
    ngclContext_t *, ngcliObserveThread_t *, time_t, int *);
static int ngcllObserveThreadProceedEventTimeChange(
    ngclContext_t *, ngcliObserveThread_t *, time_t, int *);
#endif /* NG_PTHREAD */

#define NGCLL_OBSERVE_THREAD_NO_EVENT_SLEEP 3600

/**
 * ObserveItem: Construct
 */
ngcliObserveItem_t *
ngcliObserveItemConstruct(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *error)
{
    static const char fName[] = "ngcliObserveItemConstruct";
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    /* Allocate */
    observeItem = ngcllObserveItemAllocate(context, error);
    if (observeItem == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for ObserveItem.\n", fName);
        return NULL;
    }

    /* Initialize */
    result = ngcllObserveItemInitialize(context, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the ObserveItem.\n", fName);
        return NULL;
    }

    /* Register */
    result = ngcllObserveItemRegister(
        context, observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the ObserveItem.\n", fName);
        return NULL;
    }

    /* Success */
    return observeItem;
}

/**
 * ObserveItem: Destruct
 */
int
ngcliObserveItemDestruct(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    static const char fName[] = "ngcliObserveItemDestruct";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    /* Unregister */
    result = ngcllObserveItemUnregister(
        context, observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unregister the ObserveItem.\n", fName);
        return 0;
    }

    /* Finalize */
    result = ngcllObserveItemFinalize(context, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the ObserveItem.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = ngcllObserveItemFree(context, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the storage for ObserveItem.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * ObserveItem: Allocate
 */
static ngcliObserveItem_t *
ngcllObserveItemAllocate(
    ngclContext_t *context,
    int *error)
{
    ngcliObserveItem_t *observeItem;
    static const char fName[] = "ngcllObserveItemAllocate";

    /* Allocate */
    observeItem = globus_libc_calloc(1, sizeof(ngcliObserveItem_t));
    if (observeItem == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for ObserveItem.\n", fName);
        return NULL;
    }

    /* Success */
    return observeItem;
}


/**
 * ObserveItem: Deallocate
 */
static int
ngcllObserveItemFree(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    /* Deallocate */
    globus_libc_free(observeItem);

    /* Success */
    return 1;
}

/**
 * ObserveItem: Initialize
 */
static int
ngcllObserveItemInitialize(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    ngcllObserveItemInitializeMember(observeItem);
    
    /* Success */
    return 1;
}

/**
 * ObserveItem: Finalize
 */
static int
ngcllObserveItemFinalize(
    ngclContext_t *context,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(observeItem != NULL);

    ngcllObserveItemInitializeMember(observeItem);

    /* Success */
    return 1;
}

/**
 * ObserveItem: Initialize the variable of members.
 */
static void
ngcllObserveItemInitializeMember(
    ngcliObserveItem_t *observeItem)
{
    /* Check the arguments */
    assert(observeItem != NULL);

    /* Initialize the pointers */
    ngcllObserveItemInitializePointer(observeItem);

    /* Initialize the members */
    observeItem->ngoi_eventTime = 0;
    observeItem->ngoi_eventExecuted = 0;
    observeItem->ngoi_eventTimeChangeRequested = 0;
    observeItem->ngoi_interval = 0;
}

/**
 * ObserveItem: Initialize the pointers.
 */
static void
ngcllObserveItemInitializePointer(
    ngcliObserveItem_t *observeItem)
{
    /* Check the arguments */
    assert(observeItem != NULL);

    /* Initialize the pointers */
    observeItem->ngoi_next = NULL;
    observeItem->ngoi_eventFunc = NULL;
    observeItem->ngoi_eventTimeSetFunc = NULL;
}

/**
 * ObserveItem: Register
 */
static int
ngcllObserveItemRegister(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    ngcliObserveItem_t **tail;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);
    assert(observeItem != NULL);
    assert(observeItem->ngoi_next == NULL);

    /* Append observeItem at the last of the list */
    tail = &observe->ngot_item_head;
    while (*tail != NULL) {
        tail = &(*tail)->ngoi_next;
    }
    *tail = observeItem;

    /* Success */
    return 1;
}

/**
 * ObserveItem: Unregister
 */
static int
ngcllObserveItemUnregister(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    ngcliObserveItem_t **tail;
    static const char fName[] = "ngcllObserveItemUnregister";

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);
    assert(observeItem != NULL);

    /* Find the observeItem */
    tail = &observe->ngot_item_head;
    while ((*tail != NULL) && (*tail != observeItem)) {
        tail = &(*tail)->ngoi_next;
    }

    if (*tail == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't find the ObserveItem in context.\n", fName);
        return 0;
    }

    /* Unregister the observeItem */
    *tail = (*tail)->ngoi_next;

    /* Success */
    return 1;
}

/**
 * ObserveItem: GetNext
 */
ngcliObserveItem_t *
ngcliObserveItemGetNext(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *current,
    int *error)
{
    ngcliObserveItem_t *observeItem;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    observeItem = NULL;

    if (current == NULL) {
        /* Return the first item */
        observeItem = observe->ngot_item_head;

    } else {
        /* Return the next item */
        observeItem = current->ngoi_next;
    }

    /* Success */
    return observeItem;
}

/**
 * ObserveItem: EventTime change request
 */
int
ngcliObserveItemEventTimeChangeRequest(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    static const char fName[] = "ngcliObserveItemEventTimeChangeRequest";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);
    assert(observeItem != NULL);

#if 0
    ngLogPrintf(context->ngc_log, NG_LOG_LEVEL_FATAL, NG_LOG_LEVEL_FATAL, NULL,
	"%s start...\n", fName);
#endif

#ifdef NG_PTHREAD
    result = ngcllObserveThreadEventTimeChangeRequest(
        context, observe, observeItem, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Changing ObserveItem eventTime failed.\n", fName);
        return 0;
    }
#else /* NG_PTHREAD */

    /* Do nothing */
    result = 1;
    assert(result != 0);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing is not supported for this GlobusToolkit flavor.\n",
         fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing is supported only for pthread version.\n",
         fName);

#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}


/**
 * ObserveThread: Initialize
 */
int
ngcliObserveThreadInitialize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliObserveThreadInitialize";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    result = ngcllObserveThreadInitialize(
        context, &context->ngc_observe, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Observe Thread.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * ObserveThread: Finalize
 */
int
ngcliObserveThreadFinalize(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliObserveThreadFinalize";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    result = ngcllObserveThreadFinalize(
        context, &context->ngc_observe, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't clear the Observe Thread.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * ObserveThread: Initialize
 */
static int
ngcllObserveThreadInitialize(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadInitialize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Initialize the Observe Thread module.\n", fName);

    ngcllObserveThreadInitializeMember(observe);

    result = ngcllObserveThreadInitializeMutexAndCond(
        context, observe, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Mutex and Condition Variable.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * ObserveThread: Finalize
 */
static int
ngcllObserveThreadFinalize(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadFinalize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Finalize the Observe Thread module.\n", fName);

    if (observe->ngot_continue != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Observe Thread is not stopped.\n", fName);
        return 0;
    }

    result = ngcllObserveThreadFinalizeMutexAndCond(
        context, &context->ngc_observe, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Mutex and Condition Variable.\n", fName);
        return 0;
    }

    ngcllObserveThreadInitializeMember(observe);

    /* Success */
    return 1;
}

/**
 * ObserveThread: Initialize the Mutex and Condition Variable.
 */
static int
ngcllObserveThreadInitializeMutexAndCond(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadInitializeMutexAndCond";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    log = context->ngc_log;

    assert(observe->ngot_mutexInitialized == 0);
    assert(observe->ngot_condInitialized == 0);

    /* Initialize the mutex */
    result = ngiMutexInitialize(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Mutex.\n", fName);
        goto error;
    }
    observe->ngot_mutexInitialized = 1;

    /* Initialize the condition variable */
    result = ngiCondInitialize(&observe->ngot_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Condition variable.\n", fName);
        goto error;
    }
    observe->ngot_condInitialized = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Destroy the condition variable */
    if (observe->ngot_condInitialized != 0) {
        result = ngiCondDestroy(&observe->ngot_cond, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Condition variable.\n", fName);
        }
    }
    observe->ngot_condInitialized = 0;

    /* Destroy the mutex */
    if (observe->ngot_mutexInitialized != 0) {
        result = ngiMutexDestroy(&observe->ngot_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Mutex.\n", fName);
        }
    }

    observe->ngot_mutexInitialized = 0;
    /* Failed */
    return 0;
}

/**
 * ObserveThread: Finalize the Mutex and Condition Variable.
 */
static int
ngcllObserveThreadFinalizeMutexAndCond(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadFinalizeMutexAndCond";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    log = context->ngc_log;

    /* Destroy the condition variable */
    if (observe->ngot_condInitialized != 0) {
        result = ngiCondDestroy(&observe->ngot_cond, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Condition variable.\n", fName);
        }
    }
    observe->ngot_condInitialized = 0;

    /* Destroy the mutex */
    if (observe->ngot_mutexInitialized != 0) {
        result = ngiMutexDestroy(&observe->ngot_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destroy the Mutex.\n", fName);
        }
    }
    observe->ngot_mutexInitialized = 0;

    /* Success */
    return 1;
}

/**
 * ObserveThread: Initialize the variable of members.
 */
static void
ngcllObserveThreadInitializeMember(
    ngcliObserveThread_t *observe)
{
    /* Check the arguments */
    assert(observe != NULL);

    ngcllObserveThreadInitializePointer(observe);

    observe->ngot_continue = 0;
    observe->ngot_stopped = 0;
    observe->ngot_mutexInitialized = 0;
    observe->ngot_condInitialized = 0;
}

/**
 * ObserveThread: Initialize the pointers.
 */
static void
ngcllObserveThreadInitializePointer(
    ngcliObserveThread_t *observe)
{
    /* Check the arguments */
    assert(observe != NULL);

    observe->ngot_item_head = NULL;
}

/**
 * ObserveThread: Start observing
 */
int
ngcliObserveThreadStart(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliObserveThreadStart";
    ngcliObserveThread_t *observe;
    int result, enabled;

    /* Check the arguments */
    assert(context != NULL);

    observe = &context->ngc_observe;

    enabled = 0;
    if (observe->ngot_item_head != NULL) {
        enabled = 1;
    }
    
    if (enabled == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Observe Thread is unnecessary.\n",
            fName);

        /* Do nothing */
        observe->ngot_continue = 0; /* FALSE */
        observe->ngot_stopped = 0;  /* FALSE */

        return 1;
    }

#ifdef NG_PTHREAD
    observe->ngot_continue = 1; /* TRUE */
    observe->ngot_stopped = 0;  /* FALSE */

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Start the Observe Thread.\n", fName);

    /* Create the Observe Thread */
    result = globus_thread_create(
        &observe->ngot_thread, NULL,
        ngcllObserveThread, context);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the thread for observe.\n", fName);
        return 0;
    }
#else /* NG_PTHREAD */

    /* Do nothing */
    result = 1;
    assert(result != 0);
    observe->ngot_continue = 0; /* FALSE */
    observe->ngot_stopped = 0;  /* FALSE */

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing is not supported for this GlobusToolkit flavor.\n",
         fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing is supported only for pthread version.\n",
         fName);
#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}

/**
 * ObserveThread: Stop checking
 */
int
ngcliObserveThreadStop(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliObserveThreadStop";
    ngcliObserveThread_t *observe;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    observe = &context->ngc_observe;

    /* Check the arguments */
    if (observe->ngot_stopped != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Observe Thread was already stopped.\n", fName);
        return 0;
    }

    /* Check if the Observe Thread is working */
    if (observe->ngot_continue == 0) {

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Observe Thread is not working.\n", fName);

        /* Do nothing */
        return 1;
    }

#ifdef NG_PTHREAD

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Stop the Observe Thread.\n", fName);

    result = ngcllObserveThreadStop(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't stop the thread to observe.\n", fName);
        return 0;
    }
    
#else /* NG_PTHREAD */

    /* Do nothing */
    result = 1;
    assert(result != 0);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing not supported for this GlobusToolkit flavor.\n", fName);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: observing is supported only for pthread version.\n", fName);

#endif /* NG_PTHREAD */

    /* Success */
    return 1;
}

#ifdef NG_PTHREAD
/**
 * ObserveThread: main proceeding
 */
static void *
ngcllObserveThread(void *threadArgument)
{
    static const char fName[] = "ngcllObserveThread";
    ngcliObserveThread_t *observe;
    int errorEntity, *error;
    int result, wasTimedOut, sleepSec;
    ngclContext_t *context;
    ngLog_t *log;
    time_t now;

    /* Check the arguments */
    assert(threadArgument != NULL);

    context = (ngclContext_t *)threadArgument;
    observe = &context->ngc_observe;
    log = context->ngc_log;
    error = &errorEntity;
    sleepSec = 0;
    now = 0;

    /* Check if the flag is valid */
    assert(observe->ngot_stopped == 0);

    /**
     * Do observing.
     */

    /* Lock */
    result = ngiMutexLock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return NULL;
    }

#if 0
    ngLogPrintf(context->ngc_log, NG_LOG_LEVEL_FATAL, NG_LOG_LEVEL_FATAL, NULL,
	"%s start...\n", fName);
#endif

    /* Get current time */
    now = time(NULL);

    /* Proceed event time change if necessary */
    result = ngcllObserveThreadProceedEventTimeChange(
	context, observe, now, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
	    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Proceeding Observe Thread event time change failed.\n",
	    fName);
	goto error;
    }

    /* Wait the status */
    while (observe->ngot_continue != 0) {

        /* Get sleep time */
        result = ngcllObserveThreadGetSleepTime(
            context, observe, &sleepSec, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Getting Observe Thread sleep time failed.\n", fName);
            goto error;
        }

        /* Cond wait sleep */
        result = ngiCondTimedWait(
            &observe->ngot_cond, &observe->ngot_mutex,
            sleepSec, &wasTimedOut, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }

        if (observe->ngot_continue == 0) {
            break;
        }

        /* Get current time */
        now = time(NULL);

        /* Proceed events if necessary */
        result = ngcllObserveThreadProceedEvents(
            context, observe, now, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Proceeding Observe Thread event failed.\n", fName);
            goto error;
        }

        /* Proceed event time change if necessary */
        result = ngcllObserveThreadProceedEventTimeChange(
            context, observe, now, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Proceeding Observe Thread event time change failed.\n",
                fName);
            goto error;
        }
    }

    /* Unlock */
    result = ngiMutexUnlock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return NULL;
    }

    /**
     * Tell the Main Thread that, Observe Thread was stopped.
     */

    /* Lock */
    result = ngiMutexLock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return NULL;
    }

    /* Set the status */
    observe->ngot_stopped = 1; /* TRUE */

    /* Notify signal */
    result = ngiCondSignal(&observe->ngot_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return NULL;
    }

    /* Success */
    return NULL;

error:
    /* Thread is stopped now */
    observe->ngot_stopped = 1; /* TRUE */

    /* Unlock */
    result = ngiMutexUnlock(&observe->ngot_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * ObserveThread: EventTime change request
 */
static int
ngcllObserveThreadEventTimeChangeRequest(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    ngcliObserveItem_t *observeItem,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadEventTimeChangeRequest";
    int requireLock, result;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);
    assert(observeItem != NULL);

    log = context->ngc_log;
    requireLock = 1;

    /**
     * Tell the Observe thread that event time was changed
     */

    /* Check if this thread is Observe thread */
    result = globus_thread_equal(
        observe->ngot_thread, globus_thread_self());
    if (result != 0) {
        /**
         * This thread is Observe thread itself,
         * and executing ngcllObserveThreadProceedEvents().
         * Thus, no need to lock and cond_signal.
         */
        requireLock = 0;
    }

    /* Lock */
    if (requireLock != 0) {
        result = ngiMutexLock(&observe->ngot_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex.\n", fName);
            return 0;
        }
    }

    /* Set the status */
    observeItem->ngoi_eventTimeChangeRequested = 1;

    /* Notify signal */
    if (requireLock != 0) {
        result = ngiCondSignal(&observe->ngot_cond, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't signal the Condition Variable.\n", fName);
            goto error;
        }
    }

    /* Unlock */
    if (requireLock != 0) {
        result = ngiMutexUnlock(&observe->ngot_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
error:
    /* Unlock */
    if (requireLock != 0) {
        result = ngiMutexUnlock(&observe->ngot_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ObserveThread: Stop the thread
 */
static int
ngcllObserveThreadStop(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadStop";
    ngcliObserveThread_t *observe;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    observe = &context->ngc_observe;

    /* Check if the flag is valid */
    assert(observe->ngot_continue == 1);

    /**
     * Tell the Observe thread to stop
     */

    /* Lock */
    result = ngiMutexLock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Set the status */
    observe->ngot_continue = 0; /* to stop */

    /* Notify signal */
    result = ngiCondSignal(&observe->ngot_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /**
     * Suppress unlock and lock, to ignore CondSignal(stopped) issue,
     * before the CondWait(stopped) is in the process.
     */

    /**
     * Wait the Observe thread to stop
     */

    /* Suppress lock. already locked */

    /* Wait the status */
    while (observe->ngot_stopped == 0) {
        result = ngiCondWait(
            &observe->ngot_cond, &observe->ngot_mutex,
            log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }
    }

    /* Unlock */
    result = ngiMutexUnlock(&observe->ngot_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
error:
    /* Unlock */
    result = ngiMutexUnlock(&observe->ngot_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * ObserveThread: Get minimum sleep time from ObserveItems
 */
static int
ngcllObserveThreadGetSleepTime(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    int *sleepSec,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadGetSleepTime";
    ngcliObserveItem_t *observeItem;
    time_t minTime, now;
    int found;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);
    assert(sleepSec != NULL);

    *sleepSec = 0;

    found = 0;
    minTime = 0;
    observeItem = NULL; /* retrieve head item */
    while ((observeItem = ngcliObserveItemGetNext(
        context, observe, observeItem, error)) != NULL) {

        assert(observeItem != NULL);

        /* Skip */
        if (observeItem->ngoi_eventTime == 0) {
            continue;
        }

        if ((found == 0) || (observeItem->ngoi_eventTime < minTime)) {
            minTime = observeItem->ngoi_eventTime;
        }
        found = 1;
    }

    now = time(NULL);

    /* To detect busy loop caused by eventTime no-refresh bug */
    if ((found != 0) && ((minTime + 60) <= now)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Already passed eventTime detected. event time=%ld (now=%ld)\n",
            fName, minTime, now);
    }

    if (found == 0) {
        /* No need to wakeup, but wakeup long long later */
        *sleepSec = NGCLL_OBSERVE_THREAD_NO_EVENT_SLEEP;
    } else {
        assert(minTime > 0);

        if (minTime <= now) {
            *sleepSec = 0;
        } else {
            *sleepSec = minTime - now;
        }
    }

    /* Success */
    return 1;
}

/**
 * ObserveThread: Proceed events on ObserveItems if necessary
 */
static int
ngcllObserveThreadProceedEvents(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadProceedEvents";
    ngcliObserveItemEvent_t eventFunc;
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    observeItem = NULL; /* retrieve head item */
    while ((observeItem = ngcliObserveItemGetNext(
        context, observe, observeItem, error)) != NULL) {

        assert(observeItem != NULL);

        observeItem->ngoi_eventExecuted = 0;

        /* Skip if disabled */
        if (observeItem->ngoi_eventTime == 0) {
            continue;
        }

        /* Invoke the event */
        if (observeItem->ngoi_eventTime <= now) {
            eventFunc = observeItem->ngoi_eventFunc;
            result = (*eventFunc)(context, observeItem, now, error);
            if (result == 0) {
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                    "%s: Observe event function returned by error.\n",
                    fName);
            }
            observeItem->ngoi_eventExecuted = 1;
        }
    }

    /* Success */
    return 1;
}


/**
 * ObserveThread: Proceed eventTime change on ObserveItems if necessary
 */
static int
ngcllObserveThreadProceedEventTimeChange(
    ngclContext_t *context,
    ngcliObserveThread_t *observe,
    time_t now,
    int *error)
{
    static const char fName[] = "ngcllObserveThreadProceedEventTimeChange";
    ngcliObserveItemEvent_t eventFunc;
    ngcliObserveItem_t *observeItem;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(observe != NULL);

    observeItem = NULL; /* retrieve head item */
    while ((observeItem = ngcliObserveItemGetNext(
        context, observe, observeItem, error)) != NULL) {

        assert(observeItem != NULL);

        /* Invoke the eventTime setup function */
        if ((observeItem->ngoi_eventExecuted != 0) ||
            (observeItem->ngoi_eventTimeChangeRequested != 0)) {

            eventFunc = observeItem->ngoi_eventTimeSetFunc;
            result = (*eventFunc)(context, observeItem, now, error);
            if (result == 0) {
                ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Observe eventTime setup function returned by error.\n",
                fName);
            }
        }

        /* Reset the flag */
        observeItem->ngoi_eventExecuted = 0;
        observeItem->ngoi_eventTimeChangeRequested = 0;
    }

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */

