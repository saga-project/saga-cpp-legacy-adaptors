/*
 * $RCSfile: ngEvent.c,v $ $Revision: 1.64 $ $Date: 2008/03/28 08:50:58 $
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
 * Module of Event for Ninf-G.
 */

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngEvent.c,v $ $Revision: 1.64 $ $Date: 2008/03/28 08:50:58 $")

/**
 * Data.
 */
typedef enum nglEventDriverHandleOperationType_e {
    NGL_EVENT_DRIVER_HANDLE_OP_NONE   = 0,
    NGL_EVENT_DRIVER_HANDLE_OP_READ   = 1,
    NGL_EVENT_DRIVER_HANDLE_OP_WRITE  = 2,
    NGL_EVENT_DRIVER_HANDLE_OP_SOCKET = 3,
    NGL_EVENT_DRIVER_HANDLE_OP_SOCKET_CB = 4,
    NGL_EVENT_DRIVER_HANDLE_OP_FILE = 5,
    NGL_EVENT_DRIVER_HANDLE_OP_TIME_EVENT  = 6,
    NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE = 7,
    NGL_EVENT_DRIVER_HANDLE_OP_TIME_CB = 8,
    NGL_EVENT_DRIVER_HANDLE_OP_CLOSE = 9,
    NGL_EVENT_DRIVER_HANDLE_OP_NUMBER = 10,
    NGL_EVENT_DRIVER_HANDLE_OP_THREAD_STOP = 11,
    NGL_EVENT_DRIVER_HANDLE_OP_NOMORE = 12
} nglEventDriverHandleOperationType_t;

typedef enum nglEventDriverReturnMode_e {
    NGL_EVENT_DRIVER_RETURN_MODE_NONE,
    NGL_EVENT_DRIVER_RETURN_MODE_BLOCK,
    NGL_EVENT_DRIVER_RETURN_MODE_TIMEOUT,
    NGL_EVENT_DRIVER_RETURN_MODE_NONBLOCK,
    NGL_EVENT_DRIVER_RETURN_MODE_NOMORE
} nglEventDriverReturnMode_t;

typedef enum nglEventDriverHandleNumberMode_e {
    NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_NONE,
    NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_APPEND,
    NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_REMOVE,
    NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_NOMORE
} nglEventDriverHandleNumberMode_t;

typedef enum nglEventDriverCallbackRegisterMode_e {
    NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NONE,
    NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
    NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
    NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NOMORE
} nglEventDriverCallbackRegisterMode_t;


/**
 * Prototype declaration of internal functions.
 */
static int nglEventInitialize(
    ngiEvent_t *event, ngiSelectorFunctionsSetFunction_t setFunction,
    ngLog_t *log, int *error);
static int nglEventFinalize(ngiEvent_t *event, ngLog_t *log, int *error);
static void nglEventInitializeMember(ngiEvent_t *event);
static int nglEventEventDriverRegister(
    ngiEvent_t *event, ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventEventDriverUnregister(
    ngiEvent_t *event, ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventEventDriverRequire(
    ngiEvent_t *event, ngiEventDriver_t **driver, ngLog_t *log, int *error);

static int nglEventMutexInitialize(
    ngiEventMutex_t *mutex, ngLog_t *log, int *error);
static int nglEventMutexDestroy(
    ngiEventMutex_t *mutex, ngLog_t *log, int *error);
static int nglEventMutexLock(
    ngiEventMutex_t *mutex, ngLog_t *log, int *error);
static int nglEventMutexUnlock(
    ngiEventMutex_t *mutex, ngLog_t *log, int *error);
static int nglEventCondInitialize(
    ngiEventCond_t *cond, ngLog_t *log, int *error);
static int nglEventCondDestroy(
    ngiEventCond_t *cond, ngLog_t *log, int *error);
static int nglEventCondWait(
    ngiEventCond_t *cond, ngiEventMutex_t *mutex, ngLog_t *log, int *error);
static int nglEventCondBroadcast(
    ngiEventCond_t *cond, ngLog_t *log, int *error);
static int nglEventThreadCreate(
    ngiEventThread_t *thread, void *(threadFunction)(void *),
    void *threadArgument, ngLog_t *log, int *error);
static int nglEventThreadJoin(
    ngiEventThread_t *thread, ngLog_t *log, int *error);

static ngiEventDriver_t *nglEventDriverConstruct(
    ngiEvent_t *event, ngLog_t *log, int *error);
static int nglEventDriverDestruct(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventDriverInitialize(
    ngiEventDriver_t *driver, ngiEvent_t *event, ngLog_t *log, int *error);
static int nglEventDriverFinalize(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static void nglEventDriverInitializeMember(ngiEventDriver_t *driver);
static int nglEventDriverThreadStart(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventDriverThreadStop(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static void *nglEventDriverThread(void *arg);

static int nglEventDriverLoop(
    ngiEventDriver_t *driver, nglEventDriverReturnMode_t mode,
    int timeoutMilli, ngLog_t *log, int *error);
static int nglEventDriverPrepare(
    ngiEventDriver_t *driver, nglEventDriverReturnMode_t mode,
    int timeoutMilli, ngLog_t *log, int *error);
static int nglEventDriverWait(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventDriverProcess(
    ngiEventDriver_t *driver, nglEventDriverReturnMode_t mode,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverProcessSelector(
    ngiEventDriver_t *driver, int *finish, ngLog_t *log, int *error);
static int nglEventDriverProcessTime(
    ngiEventDriver_t *driver, int *finish, ngLog_t *log, int *error);

static int nglEventDriverProcessHandleSelector(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int flags, int *finish, int isReadCloseHup, ngLog_t *log, int *error);
static int nglEventDriverProcessHandleOperation(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverProcessHandleTime(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    struct timeval timeNow, int *finish, ngLog_t *log, int *error);
static int nglEventDriverHandleRequestDone(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverHandleClosed(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverHandleOperationErrorOccurred(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverHandleIneffective(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int *finish, ngLog_t *log, int *error);

static int nglEventDriverHandleCallbackCall(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    ngiIOhandleState_t state, int *finish, ngLog_t *log, int *error);
static int nglEventDriverCallbackRun(
    ngiEventDriver_t *driver, ngiIOhandle_t *callbackHandle,
    ngiEventCallbackType_t callbackType,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument, ngiIOhandleState_t callbackState,
    ngLog_t *log, int *error);
static int nglEventDriverHandleChunkCallbackCall(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngiIOhandleState_t state, int *finish, ngLog_t *log, int *error);

static int nglEventDriverCommandNotify(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    ngLog_t *log, int *error);
static int nglEventDriverCommandThreadSend(
    ngiEventDriver_t *driver, nglEventDriverHandleOperationType_t type,
    ngLog_t *log, int *error);
static int nglEventDriverCommandThreadReceive(
    ngiEventDriver_t *driver, int flags, int *finish, int isReadCloseHup,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommand(
    ngiEventDriver_t *driver, nglEventDriverHandleOperationType_t type,
    int *finish, ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandle(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleReadCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleReadCallbackCancel(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleReadChunkCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleReadChunkCallbackCancel(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleSocket(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketListenerCreate(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *requireProcess,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketListenerCreateTCP(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *listenFd,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketListenerCreateUNIX(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *listenFd,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketAccept(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int *requireProcess, ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketConnect(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *requireProcess,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketConnectTCP(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *connectFd,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketConnectUNIX(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *connectFd,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketNodelay(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle, int *requireProcess,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketListenerCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleSocketListenerCallbackCancel(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleFile(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleFileOpen(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int *requireProcess, ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleFileFd(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int *requireProcess, ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleTimeChange(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleTimeCallbackCancel(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverProcessCommandHandleClose(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleNumber(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleAppend(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverProcessCommandHandleRemove(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);

static int nglEventDriverRequestCommandHandleNumber(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleNumberMode_t mode,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleClose(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleOperate(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    char *buf, size_t length, size_t waitNbytes, size_t *doneNbytes,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleReadCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction, void *callbackArgument,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleReadChunkCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction, void *callbackArgument,
    ngLog_t *log, int *error);

static int nglEventDriverRequestCommandHandleSocket(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngiIOhandleSocketType_t socketType,
    int isListenerCreate, int listenerPort, int *listenerPortAllocated,
    char *listenerPath, int listenerBacklog,
    int isAccept, ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    int isConnect, char *connectHost, int connectPort, char *connectPath,
    int isNodelay, int nodelayEnable,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction, void *callbackArgument,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleFile(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int isFileOpen, char *path, ngiIOhandleFileOpenType_t fileOpenType,
    int isFdOpen, int fd, ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleTime(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    int isTimeChangeRequest, int isTimeSet, time_t eventTime,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandHandleTimeCallbackRegister(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t eventCallbackFunction,
    void *eventCallbackArgument,
    ngiIOhandleCallbackFunction_t changeCallbackFunction,
    void *changeCallbackArgument,
    ngLog_t *log, int *error);

static int nglEventDriverRequestCommandNonThreadCondWait(
    ngiEventDriver_t *driver, int *waitFlag,
    ngiEventNonThreadCondType_t type, int timeoutSec, int *wasTimeout,
    ngLog_t *log, int *error);
static int nglEventDriverRequestCommandNonThreadYield(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);

static int nglEventDriverRequestDoneWait(
    ngiEventDriver_t *driver, ngLog_t *log, int *error);

static ngiEventCallbackManager_t *nglEventCallbackManagerConstruct(
    int initialCallbacks, ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventCallbackManagerDestruct(
    ngiEventCallbackManager_t *manager, ngLog_t *log, int *error);
static int nglEventCallbackManagerInitialize(
    ngiEventCallbackManager_t *manager, int initialCallbacks,
    ngiEventDriver_t *driver, ngLog_t *log, int *error);
static int nglEventCallbackManagerFinalize(
    ngiEventCallbackManager_t *manager, ngLog_t *log, int *error);
static void nglEventCallbackManagerInitializeMember(
    ngiEventCallbackManager_t *manager);
static int nglEventCallbackManagerCallbackRegister(
    ngiEventCallbackManager_t *manager, ngiEventCallback_t *callback,
    ngLog_t *log, int *error);
static int nglEventCallbackManagerCallbackUnregister(
    ngiEventCallbackManager_t *manager, ngiEventCallback_t *callback,
    ngLog_t *log, int *error);
static int nglEventCallbackManagerCallbackRequire(
    ngiEventCallbackManager_t *manager, ngLog_t *log, int *error);

static ngiEventCallback_t *nglEventCallbackConstruct(
    ngiEventCallbackManager_t *manager, ngLog_t *log, int *error);
static int nglEventCallbackDestruct(
    ngiEventCallback_t *callback, ngLog_t *log, int *error);
static int nglEventCallbackInitialize(
    ngiEventCallback_t *callback, ngiEventCallbackManager_t *manager,
    ngLog_t *log, int *error);
static int nglEventCallbackFinalize(
    ngiEventCallback_t *callback, ngLog_t *log, int *error);
static void nglEventCallbackInitializeMember(ngiEventCallback_t *callback);
static int nglEventCallbackThreadStart(
    ngiEventCallback_t *callback, ngLog_t *log, int *error);
static int nglEventCallbackThreadStop(
    ngiEventCallback_t *callback, ngLog_t *log, int *error);
static void *nglEventCallbackThread(void *arg);
static int nglEventCallbackRunRequest(
    ngiEventDriver_t *driver, ngiIOhandle_t *handle,
    ngiEventCallbackType_t callbackType,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument, ngiIOhandleState_t state,
    int *callbackExecuting, ngLog_t *log, int *error);

static int nglIOhandleInitialize(
    ngiIOhandle_t *handle, ngiEvent_t *event, ngLog_t *log, int *error);
static int nglIOhandleFinalize(
    ngiIOhandle_t *handle, ngLog_t *log, int *error);
static void nglIOhandleInitializeMember(ngiIOhandle_t *handle);
static void nglIOhandleChunkStatusInitializeMember(
    ngiIOhandleChunkStatus_t *status);
static void nglIOhandleStatusInitializeMember(
    ngiIOhandleStatus_t *status);
static void nglIOhandleSocketStatusInitializeMember(
    ngiIOhandleSocketStatus_t *status);
static void nglIOhandleFileStatusInitializeMember(
    ngiIOhandleFileStatus_t *status);
static void nglIOhandleTimeStatusInitializeMember(
    ngiIOhandleTimeStatus_t *status);
static int nglIOhandleOperate(
    ngiIOhandle_t *handle, nglEventDriverHandleOperationType_t type,
    char *buf, size_t length, size_t waitNbytes, size_t *doneNbytes,
    ngLog_t *log, int *error);
static int nglIOhandleSocketOperate(
    ngiIOhandle_t *handle, ngiIOhandleSocketType_t socketType,
    int isListenerCreate, int listenerPort, int *listenerPortAllocated,
    char *listenerPath, int listenerBacklog,
    int isAccept, ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    int isConnect, char *connectHost, int connectPort, char *connectPath,
    int isNodelay, int nodelayEnable,
    ngLog_t *log, int *error);

static int nglIOhandleAcceptResultInitialize(
    ngiIOhandleAcceptResult_t *acceptResult, ngLog_t *log, int *error);
static int nglIOhandleAcceptResultFinalize(
    ngiIOhandleAcceptResult_t *acceptResult, ngLog_t *log, int *error);
static void nglIOhandleAcceptResultInitializeMember(
    ngiIOhandleAcceptResult_t *acceptResult);

void
nglIOhandleCallbackWaiterInitializeMember(
    ngiIOhandleCallbackWaiter_t *waiter);


/**
 * Functions.
 */

/**
 * Event: Construct.
 */
ngiEvent_t *
ngiEventConstruct(
    ngiSelectorFunctionsSetFunction_t setFunction,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiEventConstruct";
    int result, allocated, initialized;
    ngiEvent_t *event;

    event = NULL;
    allocated = 0;
    initialized = 0;

    /* setFunction may be NULL. */

    /* Allocate */
    event = NGI_ALLOCATE(ngiEvent_t, log, error);
    if (event == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Allocate the Event failed.\n");
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglEventInitialize(event, setFunction, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Initialize the Event failed.\n");
        goto error;
    }
    initialized = 1;

    /* Success */
    return event;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Event: Destruct.
 */
int
ngiEventDestruct(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiEventDestruct";
    int result;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. The event is NULL.\n");
        goto error;
    }

    /* Finalize */
    result = nglEventFinalize(event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Finalize the Event failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiEvent_t, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Deallocate the Event failed.\n");
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
 * Event: Initialize.
 */
static int
nglEventInitialize(
    ngiEvent_t *event,
    ngiSelectorFunctionsSetFunction_t setFunction,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventInitialize";
    ngiEventDriver_t *driver;
    int result;

    /* Check the arguments */
    assert(event != NULL);

    nglEventInitializeMember(event);

    result = nglEventMutexInitialize(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Initialize the mutex failed.\n");
        goto error;
    }

    event->ngev_log = log;

    event->ngev_nEventDrivers = 0;
    event->ngev_eventDriver_head = NULL;
    event->ngev_eventDriverIDmax = 0;

#ifdef NG_PTHREAD
    event->ngev_isPthread = 1;
#else /* NG_PTHREAD */
    event->ngev_isPthread = 0;
#endif /* NG_PTHREAD */

    /* Set the Selector. */
    if (setFunction == NULL) {
        setFunction = NGI_SELECTOR_FUNCTIONS_DEFAULT;
    }

    if (setFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "The Selector function is not available.\n");
        goto error;
    }

    result = (*setFunction)(&event->ngev_selectorFunctions, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Set the Selector functions failed.\n");
        goto error;
    }

    /* Prepare the first Event Driver. */
    driver = NULL;
    result = nglEventEventDriverRequire(
        event, &driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Prepare the Event Driver for I/O handle failed.\n");
        goto error;
    }
    assert(driver != NULL);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event: Finalize.
 */
static int
nglEventFinalize(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventFinalize";
    ngiEventDriver_t *driver, *nextDriver;
    int result;

    /* Check the arguments */
    assert(event != NULL);

    result = nglEventMutexLock(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Lock the mutex failed.\n");
        goto error;
    }

    /* Currently, destructing flag is not used. */

    result = nglEventMutexUnlock(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    driver = event->ngev_eventDriver_head;
    while (driver != NULL) {
        nextDriver = driver->nged_next;

        result = nglEventEventDriverUnregister(
            event, driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Unregister the Event Driver failed.\n");
            goto error;
        }

        result = nglEventDriverDestruct(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Destruct the Event Driver failed.\n"); 
            goto error;
        }

        driver = nextDriver;
    }

    result = nglEventMutexDestroy(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the mutex failed.\n"); 
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
 * Event: Initialize the members.
 */
static void
nglEventInitializeMember(
    ngiEvent_t *event)
{
    /* Check the arguments */
    assert(event != NULL);

    event->ngev_log = NULL;
    event->ngev_nEventDrivers = 0;
    event->ngev_eventDriver_head = NULL;
    event->ngev_eventDriverIDmax = 0;
    ngiSelectorFunctionsInitializeMember(&event->ngev_selectorFunctions);
    event->ngev_isPthread = 0;
}

/**
 * Event: Register Event Driver.
 * Note:
 * Lock the Event before using this function, and unlock the Event after use.
 */
static int
nglEventEventDriverRegister(
    ngiEvent_t *event,
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventEventDriverRegister";
    ngiEventDriver_t **tail;
    int count;

    /* Check the arguments */
    assert(event != NULL);
    assert(driver != NULL);
    assert(driver->nged_next == NULL);

    count = 0;

    /* Find the tail. */
    tail = &event->ngev_eventDriver_head;
    while (*tail != NULL) {
        if (*tail == driver) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The Event Driver is already registered.\n"); 
            goto error;
        }

        tail = &(*tail)->nged_next;
        count++;
    }

    *tail = driver;
    count++;

    event->ngev_nEventDrivers = count;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event: Unregister Event Driver.
 * Note:
 * Lock the Event before using this function, and unlock the Event after use.
 */
static int
nglEventEventDriverUnregister(
    ngiEvent_t *event,
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventEventDriverUnregister";
    ngiEventDriver_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(event != NULL);
    assert(driver != NULL);

    /* Count the number of Callbacks. */
    count = 0;
    cur = event->ngev_eventDriver_head;
    while (cur != NULL) {
        count++;
        cur = cur->nged_next;
    }
    event->ngev_nEventDrivers = count;

    /* Delete the data from the list. */
    prevPtr = &event->ngev_eventDriver_head;
    cur = event->ngev_eventDriver_head;
    for (; cur != NULL; cur = cur->nged_next) {
        if (cur == driver) {
            /* Unlink the list */
            *prevPtr = cur->nged_next;
            driver->nged_next = NULL;
            count--;
            event->ngev_nEventDrivers = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->nged_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The Event Driver is not registered.\n"); 
    return 0;
}

/**
 * Event: Require the Event Driver for new ngiIOhandle.
 */
static int
nglEventEventDriverRequire(
    ngiEvent_t *event,
    ngiEventDriver_t **driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventEventDriverRequire";
    ngiEventDriver_t *foundDriver, *newDriver;
    int result, mutexLocked;

    foundDriver = NULL;
    newDriver = NULL;
    mutexLocked = 0;

    /* Check the arguments */
    assert(event != NULL);
    assert(driver != NULL);

    *driver = NULL;

    result = nglEventMutexLock(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    /* Currently, multiple Event Driver is not implemented. */

    foundDriver = NULL;
    if (event->ngev_eventDriver_head != NULL) {
        foundDriver = event->ngev_eventDriver_head;

    } else {

        /* Construct */
        newDriver = nglEventDriverConstruct(event, log, error);
        if (newDriver == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Construct the Event Driver failed.\n"); 
            goto error;
        }

        /* Register */
        result = nglEventEventDriverRegister(
            event, newDriver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Register the Event Driver failed.\n"); 
            goto error;
        }

        foundDriver = newDriver;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&event->ngev_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    *driver = foundDriver;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&event->ngev_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}


/**
 * Event: Set the log.
 * Note: This function supports only NonThread version and
 *    must be called before ngiIOhandle_t construct.
 */
int
ngiEventLogSet(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiEventLogSet";
    ngiEventDriver_t *cur;
    int isPthread;

#ifdef NG_PTHREAD
    isPthread = 1;
#else /* NG_PTHREAD */
    isPthread = 0;
#endif /* NG_PTHREAD */

    if (isPthread != 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Set the log to Ninf-G Event is not supported on Pthread.\n"); 
        goto error;
    }

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The event is NULL.\n"); 
        goto error;
    }

    event->ngev_log = log;

    cur = event->ngev_eventDriver_head;
    while (cur != NULL) {
        cur->nged_log = log;

        assert(cur->nged_ioHandle_head == NULL);
        assert(cur->nged_callbackManager == NULL);

        /* NonThread Event have one Event Driver though.  */
        cur = cur->nged_next;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}


#ifdef NG_PTHREAD

/**
 * Event: Pthread: Mutex Initialize.
 */
static int
nglEventMutexInitialize(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventMutexInitialize";
    int result;

    /* Check Arguments */
    assert(mutex != NULL);
 
    result = ngiMutexInitialize(mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the mutex failed.\n"); 
        return 0;
    }
       
    /* Success */
    return 1;
}

/**
 * Event: Pthread: Mutex Destroy.
 */
static int
nglEventMutexDestroy(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventMutexDestroy";
    int result;

    /* Check Arguments */
    assert(mutex != NULL);
 
    result = ngiMutexDestroy(mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the mutex failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Mutex Lock.
 */
static int
nglEventMutexLock(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventMutexLock";
    int result;

    /* Check Arguments */
    assert(mutex != NULL);
 
    result = ngiMutexLock(mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Mutex Unlock.
 */
static int
nglEventMutexUnlock(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventMutexUnlock";
    int result;

    /* Check Arguments */
    assert(mutex != NULL);
 
    result = ngiMutexUnlock(mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Cond Initialize.
 */
static int
nglEventCondInitialize(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCondInitialize";
    int result;

    /* Check Arguments */
    assert(cond != NULL);
 
    result = ngiCondInitialize(cond, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the cond failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Cond Destroy.
 */
static int
nglEventCondDestroy(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCondDestroy";
    int result;

    /* Check Arguments */
    assert(cond != NULL);
 
    result = ngiCondDestroy(cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the cond failed.\n"); 
        return 0;
    }
       
    /* Success */
    return 1;
}

/**
 * Event: Pthread: Cond Wait.
 */
static int
nglEventCondWait(
    ngiEventCond_t *cond,
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCondWait";
    int result;

    /* Check Arguments */
    assert(cond != NULL);
    assert(mutex != NULL);
 
    result = ngiCondWait(cond, mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Wait the cond failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Cond Broadcast.
 */
static int
nglEventCondBroadcast(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCondBroadcast";
    int result;

    /* Check Arguments */
    assert(cond != NULL);
 
    result = ngiCondBroadcast(cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Thread Create.
 */
static int
nglEventThreadCreate(
    ngiEventThread_t *thread,
    void *(threadFunction)(void *),
    void *threadArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventThreadCreate";
    int result;

    /* Check Arguments */
    assert(thread != NULL);
    assert(threadFunction != NULL);
 
    result = ngiThreadCreate(
        thread, threadFunction, threadArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Create the thread failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Event: Pthread: Thread Join.
 */
static int
nglEventThreadJoin(
    ngiEventThread_t *thread,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventThreadJoin";
    int result;

    /* Check Arguments */
    assert(thread != NULL);
 
    result = ngiThreadJoin(
        thread,  log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Join the thread failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

#else /* NG_PTHREAD */

/**
 * Event: NonThread: Mutex Initialize.
 */
static int
nglEventMutexInitialize(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(mutex != NULL);

    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Mutex Destroy.
 */
static int
nglEventMutexDestroy(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(mutex != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Mutex Lock.
 */
static int
nglEventMutexLock(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(mutex != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Mutex Unlock.
 */
static int
nglEventMutexUnlock(
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(mutex != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Cond Initialize.
 */
static int
nglEventCondInitialize(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(cond != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Cond Destroy.
 */
static int
nglEventCondDestroy(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(cond != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Cond Wait.
 */
static int
nglEventCondWait(
    ngiEventCond_t *cond,
    ngiEventMutex_t *mutex,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(cond != NULL);
    assert(mutex != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Cond Broadcast.
 */
static int
nglEventCondBroadcast(
    ngiEventCond_t *cond,
    ngLog_t *log,
    int *error)
{
    /* Check Arguments */
    assert(cond != NULL);
 
    /* Do nothing. */

    /* Success */
    return 1;
}

/**
 * Event: NonThread: Thread Create.
 */
static int
nglEventThreadCreate(
    ngiEventThread_t *thread,
    void *(threadFunction)(void *),
    void *threadArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventThreadCreate";

    /* Check Arguments */
    assert(thread != NULL);
    assert(threadFunction != NULL);
 
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Thread create for NonThread is not exist.\n"); 

    /* Failed */
    return 0;
}

/**
 * Event: NonThread: Thread Join.
 */
static int
nglEventThreadJoin(
    ngiEventThread_t *thread,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventThreadJoin";

    /* Check Arguments */
    assert(thread != NULL);
 
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Thread join for NonThread is not exist.\n"); 

    /* Failed */
    return 0;
}

#endif /* NG_PTHREAD */


/**
 * Event Driver: Construct.
 * Note:
 * Lock the Event before using this function, and unlock the Event after use.
 */
static ngiEventDriver_t *
nglEventDriverConstruct(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverConstruct";
    ngiEventDriver_t *driver;
    int result, allocated, initialized;

    driver = NULL;
    allocated = 0;
    initialized = 0;

    /* Check the arguments */
    assert(event != NULL);

    /* Allocate */
    driver = NGI_ALLOCATE(ngiEventDriver_t, log, error);
    if (driver == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Allocate the Event Driver failed.\n"); 
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglEventDriverInitialize(driver, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the Event Driver failed.\n"); 
        goto error;
    }
    initialized = 1;

    /* Success */
    return driver;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Event Driver: Destruct.
 */
static int
nglEventDriverDestruct(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverDestruct";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    /* Finalize */
    result = nglEventDriverFinalize(driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Finalize the Event Driver failed.\n"); 
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiEventDriver_t, driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Deallocate the Event Driver failed.\n"); 
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
 * Event Driver: Initialize.
 * Note:
 * Lock the Event before using this function, and unlock the Event after use.
 */
static int
nglEventDriverInitialize(
    ngiEventDriver_t *driver,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverInitialize";
    int result, initialCallbackDrivers;
    ngiEventCallbackManager_t *cbMng;
    ngiSelectorFunctions_t *funcs;
    void *selector;
    int pipeBuf[2];

    /* Check the arguments */
    assert(driver != NULL);
    assert(event != NULL);

    cbMng = NULL;
    funcs = NULL;
    selector = NULL;

    nglEventDriverInitializeMember(driver);

    result = nglEventMutexInitialize(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the mutex failed.\n"); 
        goto error;
    }

    result = nglEventCondInitialize(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the cond failed.\n"); 
        goto error;
    }

    driver->nged_event = event;
    driver->nged_next = NULL;
    driver->nged_log = event->ngev_log;

    event->ngev_eventDriverIDmax++;
    driver->nged_id = event->ngev_eventDriverIDmax;

    driver->nged_error = NG_ERROR_NO_ERROR;

    driver->nged_nHandles = 0;
    driver->nged_ioHandle_head = NULL;
    driver->nged_handleIDmax = 0;

    driver->nged_isPthread = event->ngev_isPthread;

    driver->nged_selectorFunctions = event->ngev_selectorFunctions;

    funcs = &driver->nged_selectorFunctions;
    selector = ngiSelectorConstruct(funcs, log, error);
    if (selector == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Construct the Selector failed.\n"); 
        goto error;
    }
    driver->nged_selector = selector;

    driver->nged_selectorSize = 1;
    result = ngiSelectorResize(
        funcs, selector, driver->nged_selectorSize, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Resize the Selector to %d failed.\n",
            driver->nged_selectorSize); 
        goto error;
    }
     
    driver->nged_callbackManager = NULL;

    if (driver->nged_isPthread != 0) {
        initialCallbackDrivers = 0;
        cbMng = NULL;

        cbMng = nglEventCallbackManagerConstruct(
            initialCallbackDrivers, driver, log, error);
        if (cbMng == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Construct the Event Callback Manager failed.\n"); 
            goto error;
        }

        driver->nged_callbackManager = cbMng;

        /* Create the command pipe to driver thread. */
        result = pipe(pipeBuf);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n", "pipe()", errno, strerror(errno)); 
            goto error;
        }
     
        driver->nged_commandPipeValid = 1;
        driver->nged_commandRead = pipeBuf[0];
        driver->nged_commandWrite = pipeBuf[1];
     
        result = fcntl(driver->nged_commandRead,
            F_SETFL, O_NONBLOCK);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n", "fcntl()", errno, strerror(errno)); 
            goto error;
        }

        result = nglEventDriverThreadStart(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Start the Event Driver failed.\n"); 
            goto error;
        }
    }

    if (driver->nged_isPthread == 0) {
        driver->nged_continue = 1;
        driver->nged_stopped = 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Event Driver: Finalize.
 */
static int
nglEventDriverFinalize(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverFinalize";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    /* Currently, destruct remaining I/O handles is not implemented. */

    if (driver->nged_isPthread != 0) {
        result = nglEventDriverThreadStop(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Stop the Event Driver failed.\n"); 
            goto error;
        }
    }

    if (driver->nged_isPthread == 0) {
        driver->nged_continue = 0;
        driver->nged_stopped = 0;
    }

    if (driver->nged_callbackManager != NULL) {
        result = nglEventCallbackManagerDestruct(
            driver->nged_callbackManager, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Destruct the Event Callback Manager failed.\n"); 
            goto error;
        }
        driver->nged_callbackManager = NULL;
    }

    result = ngiSelectorDestruct(
        &driver->nged_selectorFunctions,
        driver->nged_selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destruct the Selector failed.\n"); 
        goto error;
    }
    driver->nged_selector = NULL;

    result = nglEventMutexDestroy(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the mutex failed.\n"); 
        goto error;
    }

    result = nglEventCondDestroy(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the cond failed.\n"); 
        goto error;
    }

    nglEventDriverInitializeMember(driver);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Event Driver: Initialize the members.
 */
static void
nglEventDriverInitializeMember(
    ngiEventDriver_t *driver)
{
    /* Check the arguments */
    assert(driver != NULL);

    driver->nged_event = NULL;
    driver->nged_next = NULL;

    driver->nged_log = NULL;
    driver->nged_error = 0;
    driver->nged_id = 0;
    driver->nged_nHandles = 0;
    driver->nged_ioHandle_head = NULL;
    driver->nged_handleIDmax = 0;
    driver->nged_isPthread = 0;
    driver->nged_callbackManager = NULL;

    driver->nged_commandPipeValid = 0;
    driver->nged_commandRead = -1; /* invalid */
    driver->nged_commandWrite = -1; /* invalid */

    ngiSelectorFunctionsInitializeMember(&driver->nged_selectorFunctions);
    driver->nged_selector = NULL;
    driver->nged_selectorSize = 0;

    driver->nged_handleNumRequesting = 0;
    driver->nged_handleNumStarted = 0;
    driver->nged_handleNumDone = 0;
    driver->nged_handleNumErrorOccurred = 0;
    driver->nged_handleNumErrorCode = 0;
    driver->nged_handleNumIncrease = 0;
    driver->nged_handleNumDecrease = 0;
    driver->nged_handleNumTarget = NULL;

    driver->nged_continue = 0;
    driver->nged_stopped = 0;
}

/**
 * Event Driver: Register I/O handle.
 * Note:
 * Lock the Event Driver before using this function,
 * and unlock the Event Driver after use.
 */
static int
nglEventDriverIOhandleRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverIOhandleRegister";
    ngiIOhandle_t **tail;
    int count;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(handle->ngih_next == NULL);

    count = 0;

    /* Find the tail. */
    tail = &driver->nged_ioHandle_head;
    while (*tail != NULL) {
        if (*tail == handle) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The I/O handle is already registered.\n"); 
            goto error;
        }

        tail = &(*tail)->ngih_next;
        count++;
    }

    *tail = handle;
    count++;

    driver->nged_nHandles = count;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Unregister I/O handle.
 * Note:
 * Lock the Event Driver before using this function,
 * and unlock the Event Driver after use.
 */
static int
nglEventDriverIOhandleUnregister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverIOhandleUnregister";
    ngiIOhandle_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    /* Count the number of handles. */
    count = 0;
    cur = driver->nged_ioHandle_head;
    while (cur != NULL) {
        count++;
        cur = cur->ngih_next;
    }
    driver->nged_nHandles = count;

    /* Delete the data from the list. */
    prevPtr = &driver->nged_ioHandle_head;
    cur = driver->nged_ioHandle_head;
    for (; cur != NULL; cur = cur->ngih_next) {
        if (cur == handle) {
            /* Unlink the list */
            *prevPtr = cur->ngih_next;
            handle->ngih_next = NULL;
            count--;
            driver->nged_nHandles = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->ngih_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The I/O handle is not registered.\n"); 
    return 0;
}

/**
 * Event Driver: Thread Start.
 */
static int
nglEventDriverThreadStart(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverThreadStart";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    driver->nged_continue = 1;
    driver->nged_stopped = 0;

    result = nglEventThreadCreate(
        &driver->nged_thread, nglEventDriverThread, (void *)driver,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Create the thread failed.\n");
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
 * Event Driver: Thread Stop.
 */
static int
nglEventDriverThreadStop(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverThreadStop";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    if (driver->nged_stopped != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event driver already stopped.\n"); 

    } else {

        /**
         * Tell the Event driver to stop.
         */
     
        driver->nged_continue = 0; /* to stop */
     
        result = nglEventDriverCommandNotify(
            driver, NULL, NGL_EVENT_DRIVER_HANDLE_OP_THREAD_STOP, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Notify the command to Event Driver failed.\n"); 
            goto error;
        }
    }

    result = nglEventThreadJoin(
        &driver->nged_thread, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Join the thread failed.\n");
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
 * Event Driver: Driver Thread.
 */
static void *
nglEventDriverThread(
    void *arg)
{
    static const char fName[] = "nglEventDriverThread";
    int result, mutexLocked, returnCode;
    ngiEventDriver_t *driver;
    ngLog_t *log;
    int *error;

    /* Check the arguments */
    assert(arg != NULL);

    driver = (ngiEventDriver_t *)arg;
    returnCode = 1;
    mutexLocked = 0;

    log = driver->nged_log;
    error = &driver->nged_error;

    result = nglEventDriverLoop(
        driver, NGL_EVENT_DRIVER_RETURN_MODE_BLOCK,
        0, log, error);
    if (result == 0) {
        returnCode = 0;
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event Loop failed.\n");
        /* Not return. */
    }

    /**
     * Tell the Main Driver that, Event Driver thread was stopped.
     */

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    driver->nged_stopped = 1;
    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    if (returnCode == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event Thread return by error.\n");
        return NULL;
    }

    /* Success */
    return NULL;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        assert(driver != NULL);
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
        mutexLocked = 0;
    }

    /* Failed */
    return NULL;
}

/**
 * Event Driver: Driver thread main loop.
 *
 * Note: On Pthread version, this function not return until
 *     ngiEventDestruct() called.
 *     On NonThread version, this function return if the
 *     callback executed, read or write operation done.
 */
static int
nglEventDriverLoop(
    ngiEventDriver_t *driver,
    nglEventDriverReturnMode_t mode,
    int timeoutMilli,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverLoop";
    int cont, result, finish;
    int mutexLocked;

    /* Check the arguments */
    assert(driver != NULL);
    assert(mode > NGL_EVENT_DRIVER_RETURN_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_RETURN_MODE_NOMORE);

    mutexLocked = 0;

    finish = 0;

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n"); 
        goto error;
    }
    mutexLocked = 1;

    cont = 1;
    do {
        result = nglEventDriverPrepare(
            driver, mode, timeoutMilli, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Prepare the Event Driver failed.\n"); 
            goto error;
        }

        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
            goto error;
        }

        result = nglEventDriverWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the Event Driver failed.\n"); 
            goto error;
        }

        result = nglEventMutexLock(&driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n"); 
            goto error;
        }
        mutexLocked = 1;

        finish = 0;
        result = nglEventDriverProcess(
            driver, mode, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Process the Event Driver failed.\n"); 
            goto error;
        }
        if (finish != 0) {
            cont = 0;
        }
        
    } while (cont != 0);

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Prepare.
 */
static int
nglEventDriverPrepare(
    ngiEventDriver_t *driver,
    nglEventDriverReturnMode_t mode,
    int timeoutMilli,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverPrepare";
    int nFds, selectorSize, flags, timeout, nonBlockingOperation;
    struct timeval timeNow, timeEventOccur, timeWait;
    ngiIOhandleTimeStatus_t *timeStatus;
    time_t recentTime, timeWaitMilli;
    ngiSelectorFunctions_t *funcs;
    ngiIOhandle_t *handle;
    void *selector;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(mode > NGL_EVENT_DRIVER_RETURN_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_RETURN_MODE_NOMORE);

    nonBlockingOperation = 0;
    timeStatus = NULL;
    recentTime = 0;

    funcs = &driver->nged_selectorFunctions;
    selector = driver->nged_selector;
    selectorSize = driver->nged_selectorSize;

    assert(selectorSize > 0);
    assert(selector != NULL);

    result = ngiSelectorClear(funcs, selector, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Clear the Selector failed.\n");
        goto error;
    }

    nFds = 0;

    if (driver->nged_isPthread != 0) {
        /* For event notify. */
        result = ngiSelectorSet(funcs, selector,
            nFds, driver->nged_commandRead, NGI_SELECTOR_FLAG_IN,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Set the Selector failed.\n");
            goto error;
        }

        nFds++;
    }

    /* Clear the flag */
    handle = driver->nged_ioHandle_head;
    while (handle != NULL) {
        handle->ngih_useSelector = 0;
        handle = handle->ngih_next;
    }

    handle = driver->nged_ioHandle_head;
    while (handle != NULL) {
        if ((handle->ngih_valid == 0) ||
            (handle->ngih_userOpened == 0) ||
            (handle->ngih_fdEffective == 0)) {
            handle = handle->ngih_next;
            continue;
        }
        assert(handle->ngih_fd >= 0);

        if ((handle->ngih_read.ngihs_requireProcess != 0) ||
            (handle->ngih_write.ngihs_requireProcess != 0) ||
            (handle->ngih_socket.ngihss_requireProcess != 0)) {

            flags = 0;
            if (handle->ngih_read.ngihs_requireProcess != 0) {
                flags |= NGI_SELECTOR_FLAG_IN;
            }
            if (handle->ngih_write.ngihs_requireProcess != 0) {
                flags |= NGI_SELECTOR_FLAG_OUT;
            }
            if (handle->ngih_socket.ngihss_requireProcess != 0) {
                flags |= NGI_SELECTOR_FLAG_IN;
            }

            result = ngiSelectorSet(funcs, selector,
                nFds, handle->ngih_fd, flags, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Set the Selector for handle %d.%d failed.\n",
                    handle->ngih_eventDriverID, handle->ngih_id);
                goto error;
            }
            handle->ngih_useSelector = 1;

            nFds++;

            /* For 0 byte read. */
            if ((handle->ngih_read.ngihs_requireProcess != 0) &&
                (handle->ngih_read.ngihs_requesting != 0) &&
                (handle->ngih_read.ngihs_waitNbytes == 0)) {

                nonBlockingOperation = 1;
            }
        }

        handle = handle->ngih_next;
    }

    assert(nFds <= selectorSize);

    /* Set the timeout */
    timeout = -1; /* block */

    if (mode == NGL_EVENT_DRIVER_RETURN_MODE_BLOCK) {
        timeout = -1; /* block */

    } else if (mode == NGL_EVENT_DRIVER_RETURN_MODE_TIMEOUT) {
        timeout = timeoutMilli;

    } else if (mode == NGL_EVENT_DRIVER_RETURN_MODE_NONBLOCK) {
        timeout = 0; /* non block */
    } else {
        abort();
    }

    if (nonBlockingOperation != 0) {
        timeout = 0; /* non block */
    }

    if (timeout != 0) {

        /* Find the most recent event time. */
        recentTime = 0;
        handle = driver->nged_ioHandle_head;
        while (handle != NULL) {
            if (handle->ngih_valid == 0) {
                handle = handle->ngih_next;
                continue;
            }
     
            timeStatus = &handle->ngih_time;

            if ((timeStatus->ngihts_callbackRegistered != 0) &&
                (timeStatus->ngihts_timeCallbackExecuting == 0) &&
                (timeStatus->ngihts_changeCallbackExecuting == 0) &&
                (timeStatus->ngihts_eventTime > 0)) {

                if (recentTime == 0) {
                    recentTime = timeStatus->ngihts_eventTime;

                } else if (timeStatus->ngihts_eventTime < recentTime) {
                    recentTime = timeStatus->ngihts_eventTime;
                }
            }

            handle = handle->ngih_next;
        }

        if (recentTime > 0) {
            result = ngiGetTimeOfDay(&timeNow, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Get current time failed.\n");
                goto error;
            }

            timeEventOccur.tv_sec = recentTime;
            timeEventOccur.tv_usec = 0;

            if (ngiTimevalCompare(timeNow, timeEventOccur) > 0) {
                timeout = 0; /* non block */

            } else {
                timeWait = ngiTimevalSub(timeEventOccur, timeNow);
                timeWaitMilli = (timeWait.tv_sec * 1000) +
                    (timeWait.tv_usec / 1000);

                if (timeout == -1) {
                    timeout = timeWaitMilli;

                } else if (timeWaitMilli < timeout) {
                    timeout = timeWaitMilli;
                }
            }
        }
    }

    result = ngiSelectorSetLast(funcs, selector, timeout, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
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
 * Event Driver: Wait.
 */
static int
nglEventDriverWait(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverWait";
    int result;

    assert(driver != NULL);

    result = ngiSelectorWait(
        &driver->nged_selectorFunctions,
        driver->nged_selector,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
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
 * Event Driver: Process.
 */
static int
nglEventDriverProcess(
    ngiEventDriver_t *driver,
    nglEventDriverReturnMode_t mode,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcess";
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(mode > NGL_EVENT_DRIVER_RETURN_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_RETURN_MODE_NOMORE);
    assert(finish != NULL);

    *finish = 0;

    /* Process Selector events. */
    result = nglEventDriverProcessSelector(
        driver, finish, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Process Selector events failed.\n");
        goto error;
    }
    if (*finish != 0) {

        /* Success */
        return 1;
    }

    /* Process time events. */
    result = nglEventDriverProcessTime(
        driver, finish, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Process time events failed.\n");
        goto error;
    }
    if (*finish != 0) {

        /* Success */
        return 1;
    }

    if ((mode == NGL_EVENT_DRIVER_RETURN_MODE_TIMEOUT) ||
        (mode == NGL_EVENT_DRIVER_RETURN_MODE_NONBLOCK)) {
        *finish = 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process Selector events.
 */
static int
nglEventDriverProcessSelector(
    ngiEventDriver_t *driver,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessSelector";
    int isFds, resultFds, resultBits, proceedFds, isReadCloseHup;
    int result, idx, flags, selectorSize;
    ngiSelectorFunctions_t *funcs;
    ngiIOhandle_t *handle;
    void *selector;

    /* Check the arguments */
    assert(driver != NULL);
    assert(finish != NULL);

    handle = NULL;
    isFds = 0;
    resultFds = 0;
    resultBits = 0;
    isReadCloseHup = 0;

    funcs = &driver->nged_selectorFunctions;
    selector = driver->nged_selector;
    selectorSize = driver->nged_selectorSize;

    result = ngiSelectorGetLast(
        funcs, selector, &isFds, &resultFds, &resultBits, &isReadCloseHup,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "GetLast the Selector failed.\n");
        goto error;
    }

    if (isFds != 0) {
        if (resultFds == 0) {
            return 1;
        }
        assert(resultFds > 0);
    } else {
        if (resultBits == 0) {
            return 1;
        }
    }

    proceedFds = 0;

    idx = 0;
    if (driver->nged_isPthread != 0) {
        flags = 0;

        /* Process Command Event. */
        result = ngiSelectorGet(
            funcs, selector, driver->nged_commandRead,
            &idx, &flags, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Get the Selector for fd %d failed.\n",
                driver->nged_commandRead);
            goto error;
        }

        if (flags != 0) {
     
            /* Receive and Process the command. */
            result = nglEventDriverCommandThreadReceive(
                driver, flags, finish, isReadCloseHup, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Process read event for command failed.\n"); 
                goto error;
            }
     
            /**
             * Note:
             *  The handle event should not be processed
             *  after command event process,
             *  because command event may reallocate the Selector.
             *  And It's better to process command event before handling
             *  any I/O handle, because command may I/O handle close.
             *  In this case, on going read or write is no more processed.
             */
            
            /* Success */
            return 1;
        }
    }

    /* Process Handle Selector Events. */
    handle = driver->nged_ioHandle_head;
    while (handle != NULL) {
        if (handle->ngih_useSelector == 0) {
            handle = handle->ngih_next;
            continue;
        }
        handle->ngih_useSelector = 0;

        flags = 0;
        result = ngiSelectorGet(
            funcs, selector, handle->ngih_fd,
            &idx, &flags, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Get the Selector for fd %d failed.\n", handle->ngih_fd);
            goto error;
        }

        /* Was any flag set? */
        if (flags == 0) {
            handle = handle->ngih_next;
            continue;
        }
        
        /* Process the handle Selector events. */
        result = nglEventDriverProcessHandleSelector(
            driver, handle, flags, finish, isReadCloseHup, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Process event for handle failed.\n"); 
            goto error;
        }

        proceedFds++;

        if (*finish != 0) {
            break;
        }
    }

    if (*finish == 0) {
        if ((isFds != 0) && (proceedFds != resultFds)) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The occurred event %d and proceed event %d mismatch.\n",
                resultFds, proceedFds); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process time events.
 */
static int
nglEventDriverProcessTime(
    ngiEventDriver_t *driver,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessTime";
    struct timeval timeNow;
    ngiIOhandle_t *handle;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(finish != NULL);

    handle = NULL;


    result = ngiGetTimeOfDay(&timeNow, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Get current time failed.\n");
        goto error;
    }

    /* Process Handle Time Events. */

    handle = driver->nged_ioHandle_head;
    while (handle != NULL) {

        /* Process the handle time events. */
        result = nglEventDriverProcessHandleTime(
            driver, handle, timeNow, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Process event for handle failed.\n"); 
            goto error;
        }

        if (*finish != 0) {
            break;
        }

        handle = handle->ngih_next;
    }

    if (*finish != 0) {

        /* Success */
        return 1;
    }

    /* The timeout process is currently not supported. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}


/**
 * Event Driver: Process handle Selector events.
 */
static int
nglEventDriverProcessHandleSelector(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int flags,
    int *finish,
    int isReadCloseHup,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessHandleSelector";
    int result, gotIN, gotOUT;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(finish != NULL);

    gotIN = 0;
    gotOUT = 0;

    /**
     * Call respective operations one by one.
     */

    if (flags & NGI_SELECTOR_FLAG_IN) {
        gotIN = 1;

        result = nglEventDriverProcessHandleOperation(
            driver, handle,
            NGL_EVENT_DRIVER_HANDLE_OP_READ, finish,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The read event process for handle failed.\n"); 
            goto error;
        }

        if (*finish != 0) {
            /* Success */
            return 1;
        }
    }

    if (flags & NGI_SELECTOR_FLAG_OUT) {
        gotOUT = 1;

        result = nglEventDriverProcessHandleOperation(
            driver, handle,
            NGL_EVENT_DRIVER_HANDLE_OP_WRITE, finish,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The write event process for handle failed.\n"); 
            goto error;
        }

        if (*finish != 0) {
            /* Success */
            return 1;
        }
    }

    if (flags & NGI_SELECTOR_FLAG_HUP) {
        if ((isReadCloseHup != 0) &&
            ((handle->ngih_read.ngihs_callbackRegistered != 0) ||
             (handle->ngih_read.ngihs_requesting != 0))) {
            /**
             * Note: On Linux and Solaris poll(), pipe close is notified by
             * POLLHUP and then, EOF by read().
             * POLLIN is still effective while the available data exists.
             */
            if (gotIN == 0) {
                gotIN = 1;
                ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "The other side of handle %d.%d was closed (POLLHUP)."
                    " Checking EOF.\n",
                    handle->ngih_eventDriverID, handle->ngih_id);
             
                /* Read EOF. */
                result = nglEventDriverProcessHandleOperation(
                    driver, handle,
                    NGL_EVENT_DRIVER_HANDLE_OP_READ, finish,
                    log, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                        "The read event (by POLLHUP) process"
                        " for handle failed.\n"); 
                    goto error;
                }
                
                if (*finish != 0) {
                /* Success */
                    return 1;
                }
            }
            /* Skip if POLLIN available. */
        } 

        /* Wait writable */
        if (((handle->ngih_write.ngihs_callbackRegistered != 0) ||
            (handle->ngih_write.ngihs_requesting != 0))) {
            if (gotOUT == 0) {
                gotOUT = 1;
                result = nglEventDriverHandleClosed(
                    driver, handle, finish, log, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                        "handle close failed.\n"); 
                    goto error;
                }

                if (*finish != 0) {
                    /* Success */
                    return 1;
                }
            }
        } 
    }

    if (flags & ~(NGI_SELECTOR_FLAG_IN | NGI_SELECTOR_FLAG_OUT |
        NGI_SELECTOR_FLAG_HUP)) {

        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d got unexpected Selector event 0x%x.\n",
            handle->ngih_eventDriverID, handle->ngih_id, flags); 

        result = nglEventDriverHandleClosed(
            driver, handle, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "handle close failed.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* The handle is Ineffective. */
    result = nglEventDriverHandleIneffective(
        driver, handle, finish, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalidate the handle failed.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process handle read/write operation.
 */
static int
nglEventDriverProcessHandleOperation(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessHandleOperation";
    size_t length, waitNbytes, currentNbytes, remainSize;
    ngiIOhandleSocketStatus_t *socketStatus;
    ngiIOhandleChunkStatus_t *chunkStatus;
    ngiIOhandleStatus_t *status;
    char *buf, *startPosition;
    ngiIOhandleState_t state;
    ssize_t resultSize;
    int fd, result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(finish != NULL);

    status = NULL;
    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        status = &handle->ngih_read;
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {
        status = &handle->ngih_write;
    } else {
        abort();
    }
    assert(status != NULL);
    chunkStatus = &handle->ngih_chunk;
    socketStatus = &handle->ngih_socket;
    state = NGI_IOHANDLE_STATE_NONE;

    if ((status->ngihs_callbackRegistered != 0) ||
        (socketStatus->ngihss_listenCallbackRegistered != 0)) {

        /**
         * If NonThread, suspend read callback, while the
         * ngiIOhandleWrite() is not returned.
         */
        if ((driver->nged_isPthread == 0) &&
            (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) &&
            (handle->ngih_write.ngihs_requesting != 0)) {

            return 1;
        }

        state = NGI_IOHANDLE_STATE_NORMAL;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            return 0;
        }

        return 1;
    }

    fd = handle->ngih_fd;
    assert(fd >= 0);

    buf = status->ngihs_buf;
    length = status->ngihs_length;
    waitNbytes = status->ngihs_waitNbytes;
    currentNbytes = status->ngihs_currentNbytes;

    assert(buf != NULL);
    assert(length > 0);
    assert(waitNbytes <= length);
    assert(currentNbytes <= length);

    startPosition = &(buf[currentNbytes]);
    remainSize = length - currentNbytes;

    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {

        resultSize = read(fd, startPosition, remainSize);

        if (resultSize < 0) {
            if (errno == EAGAIN) {
                /* for read() bug on Solaris */
                ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "%s failed by EAGAIN: %d: %s. retry.\n",
                    "read()", errno, strerror(errno)); 

                return 1;
            }
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n",
                "read()", errno, strerror(errno)); 

            result = nglEventDriverHandleOperationErrorOccurred(
                driver, handle, type, finish, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Setting handle error failed.\n"); 
                goto error;
            }

            return 1;
        }

        if (resultSize == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d was closed. read() = 0. got EOF.\n",
                handle->ngih_eventDriverID, handle->ngih_id); 

            result = nglEventDriverHandleClosed(
                driver, handle, finish, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "handle close failed.\n"); 
                goto error;
            }

            return 1;
        }

        assert(resultSize > 0);

        /* Call read chunk callback. */
        if (chunkStatus->ngihcs_callbackRegistered != 0) {
            state = NGI_IOHANDLE_STATE_NORMAL;
            result = nglEventDriverHandleChunkCallbackCall(
                driver, handle, state, finish, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Failed to invoke chunk callback.\n"); 
                goto error;
            }
        }

    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {

        resultSize = write(fd, startPosition, remainSize);

        if (resultSize < 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n",
                "write()", errno, strerror(errno)); 

            result = nglEventDriverHandleOperationErrorOccurred(
                driver, handle, type, finish, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Setting handle error failed.\n"); 
                goto error;
            }

            return 1;
        }

        /* resultSize == 0 indicates, no bytes were written. */

    } else {
        abort();
    }

    currentNbytes += resultSize;
    status->ngihs_currentNbytes = currentNbytes;

    if (currentNbytes >= waitNbytes) {
        result = nglEventDriverHandleRequestDone(
            driver, handle, type, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Requesting to handle done failed.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process handle time event.
 */
static int
nglEventDriverProcessHandleTime(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    struct timeval timeNow,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessHandleTime";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleTimeStatus_t *timeStatus;
    ngiIOhandleStatus_t *status;
    ngiIOhandleState_t state;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(finish != NULL);

    state = NGI_IOHANDLE_STATE_NONE;

    /* Process non-blocking read. */
    status = &handle->ngih_read;

    if ((status->ngihs_requesting != 0) &&
        (status->ngihs_requireProcess != 0) &&
        (status->ngihs_waitNbytes == 0)) {

        assert(status->ngihs_started != 0);
        assert(status->ngihs_callbackRegistered == 0);

        type = NGL_EVENT_DRIVER_HANDLE_OP_READ;

        result = nglEventDriverHandleRequestDone(
            driver, handle, type, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Requesting to handle done failed.\n"); 
            goto error;
        }

        if (*finish != 0) {
            /* Success */
            return 1;
        }
    }

    /* Process Time Event. */
    timeStatus = &handle->ngih_time;

    if ((timeStatus->ngihts_callbackRegistered != 0) &&
        (timeStatus->ngihts_timeCallbackExecuting == 0) &&
        (timeStatus->ngihts_changeCallbackExecuting == 0) &&
        (timeStatus->ngihts_eventTime > 0) &&
        (timeStatus->ngihts_eventTime <= timeNow.tv_sec)) {

        timeStatus->ngihts_eventTime = 0; /* disable */

        type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_EVENT;
        state = NGI_IOHANDLE_STATE_NORMAL;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            goto error;
        }

        if (*finish != 0) {
            /* Success */
            return 1;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Handle request done.
 */
static int
nglEventDriverHandleRequestDone(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverHandleRequestDone";
    ngiIOhandleStatus_t *status;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(finish != NULL);

    status = NULL;
    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        status = &handle->ngih_read;
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {
        status = &handle->ngih_write;
    } else {
        abort();
    }
    assert(status != NULL);

    /* Lock: mutex is already locked. */
    status->ngihs_requireProcess = 0;
    status->ngihs_doneNbytes = status->ngihs_currentNbytes;
    status->ngihs_currentNbytes = 0;
    status->ngihs_done = 1;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }
    /* Unlock: mutex is already locked. */

    if (driver->nged_isPthread == 0) {
        *finish = 1;
    }

    return 1;
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Handle closed.
 */
static int
nglEventDriverHandleClosed(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverHandleClosed";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleSocketStatus_t *socketStatus;
    ngiIOhandleChunkStatus_t *chunkStatus;
    ngiIOhandleFileStatus_t *fileStatus;
    ngiIOhandleStatus_t *status;
    ngiIOhandleState_t state;
    int errorOccurred, errorCode;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(finish != NULL);

    type = NGL_EVENT_DRIVER_HANDLE_OP_NONE;
    status = NULL;
    chunkStatus = NULL;
    socketStatus = NULL;
    fileStatus = NULL;
    state = NGI_IOHANDLE_STATE_NONE;

    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    /* Lock: mutex is already locked. */

    ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The handle %d.%d closed.\n",
        handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_fdEffective != 0) {

        result = close(handle->ngih_fd);
        if (result != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n",
                "close()", errno, strerror(errno)); 
            errorOccurred = 1;
            NGI_SET_ERROR(&errorCode, NG_ERROR_SYSCALL);
        }

        handle->ngih_fd = -1; /* invalid */
        handle->ngih_fdEffective = 0;
    }

    /* Notify done for each request. */

    /* Read Chunk callback. */
    chunkStatus = &handle->ngih_chunk;
    if (chunkStatus->ngihcs_callbackRegistered != 0) {
        state = NGI_IOHANDLE_STATE_CLOSED;
        result = nglEventDriverHandleChunkCallbackCall(
            driver, handle, state, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke chunk callback.\n"); 
            /* Not return. */
        }
    }

    /* Read request. */
    status = &handle->ngih_read;
    type = NGL_EVENT_DRIVER_HANDLE_OP_READ;

    if ((status->ngihs_requesting == 1) &&
        (status->ngihs_done == 0)) {

        if ((status->ngihs_currentNbytes > 0) &&
            (status->ngihs_currentNbytes < status->ngihs_waitNbytes)) {
            status->ngihs_errorOccurred = 1;
            status->ngihs_errorCode = NG_ERROR_COMMUNICATION;
        }
        /* read() == 0 is return as readNbytes == 0, success. */

        result = nglEventDriverHandleRequestDone(
            driver, handle, type, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Requesting to handle done failed.\n"); 
            /* Not return. */
        }
    }

    if (status->ngihs_callbackRegistered != 0) {
        state = NGI_IOHANDLE_STATE_CLOSED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            /* Not return. */
        }
    }

    /* Write request. */
    status = &handle->ngih_write;
    type = NGL_EVENT_DRIVER_HANDLE_OP_WRITE;
    if ((status->ngihs_requesting == 1) &&
        (status->ngihs_done == 0)) {
        if (status->ngihs_currentNbytes < status->ngihs_waitNbytes) {
            status->ngihs_errorOccurred = 1;
            status->ngihs_errorCode = NG_ERROR_COMMUNICATION;
        }
        result = nglEventDriverHandleRequestDone(
            driver, handle, type, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Requesting to handle done failed.\n"); 
            /* Not return. */
        }
    }

    /* Socket request. */
    socketStatus = &handle->ngih_socket;
    if ((socketStatus->ngihss_requesting != 0) &&
        (socketStatus->ngihss_done == 0)) {
        /* It wont be happen, while async Socket connect is not implement. */
        socketStatus->ngihss_errorOccurred = 0;
        socketStatus->ngihss_errorCode = NG_ERROR_COMMUNICATION;
        socketStatus->ngihss_requireProcess = 0;
        socketStatus->ngihss_done = 1;
    }

    if (socketStatus->ngihss_listenCallbackRegistered != 0) {
        type = NGL_EVENT_DRIVER_HANDLE_OP_READ;
        state = NGI_IOHANDLE_STATE_CLOSED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            /* Not return. */
        }
    }

    /* File request. */
    fileStatus = &handle->ngih_file;
    if ((fileStatus->ngihfs_requesting != 0) &&
        (fileStatus->ngihfs_done == 0)) {
        fileStatus->ngihfs_errorOccurred = 1;
        fileStatus->ngihfs_errorCode = NG_ERROR_COMMUNICATION;
        fileStatus->ngihfs_requireProcess = 0;
        fileStatus->ngihfs_done = 1;
    }

    /* Close request. */
    if (handle->ngih_closeRequesting != 0) {
        if (errorOccurred != 0) {
            handle->ngih_closeErrorOccurred = 1;
            handle->ngih_closeErrorCode = errorCode;
        }
        handle->ngih_closeDone = 1;
    }

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    if (driver->nged_isPthread == 0) {
        *finish = 1;
    }

    /* Success */
    return 1;

error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Handle operation error occurred.
 */
static int
nglEventDriverHandleOperationErrorOccurred(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverHandleOperationErrorOccurred";
    ngiIOhandleStatus_t *status;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(finish != NULL);

    status = NULL;
    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        status = &handle->ngih_read;
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {
        status = &handle->ngih_write;
    } else {
        abort();
    }
    assert(status != NULL);

    /* Set the error. */
    status->ngihs_errorOccurred = 1;
    status->ngihs_errorCode = NG_ERROR_COMMUNICATION;

    result = nglEventDriverHandleRequestDone(
        driver, handle, type, finish, log, error);
    if (result == 0) {
         ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
             "handle notify failed.\n"); 
        return 0;
    }

    if (driver->nged_isPthread == 0) {
        *finish = 1;
    }

    /* Success */
    return 1;
}

/**
 * Event Driver: Handle is no more effective.
 */
static int
nglEventDriverHandleIneffective(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *finish,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglEventDriverHandleIneffective";

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(finish != NULL);

    handle->ngih_fdEffective = 0;

    /* Lock: mutex is already locked. */

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    if (driver->nged_isPthread == 0) {
        *finish = 1;
    }

    /* Success */
    return 1;
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Handle callback call.
 */
static int
nglEventDriverHandleCallbackCall(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    ngiIOhandleState_t state,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverHandleCallbackCall";
    ngiIOhandleCallbackFunction_t callbackFunction;
    ngiIOhandleSocketStatus_t *socketStatus;
    ngiIOhandleTimeStatus_t *timeStatus;
    ngiEventCallbackType_t callbackType;
    ngiIOhandleStatus_t *status;
    int *callbackExecuting;
    void *callbackArgument;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(state > NGI_IOHANDLE_STATE_NONE);
    assert(state < NGI_IOHANDLE_STATE_NOMORE);
    assert(finish != NULL);
    
    if ((type != NGL_EVENT_DRIVER_HANDLE_OP_READ) &&
        (type != NGL_EVENT_DRIVER_HANDLE_OP_TIME_EVENT) &&
        (type != NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Callback type %d is not treated.\n", type); 
        goto error;
    }

    callbackType = NGI_EVENT_CALLBACK_TYPE_NONE;
    callbackFunction = NULL;
    callbackArgument = NULL;
    callbackExecuting = NULL;

    status = &handle->ngih_read;
    socketStatus = &handle->ngih_socket;
    timeStatus = &handle->ngih_time;

    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        if (status->ngihs_callbackRegistered != 0) {
     
            callbackType = NGI_EVENT_CALLBACK_TYPE_READ;
            callbackFunction = status->ngihs_callbackFunction;
            callbackArgument = status->ngihs_callbackArgument;
            callbackExecuting = NULL; /* Do not wait done */
     
            assert(callbackFunction != NULL);

            /* Unregister the callback. */
            status->ngihs_callbackFunction = NULL;
            status->ngihs_callbackArgument = NULL;
            status->ngihs_callbackRegistered = 0;
     
            status->ngihs_requireProcess = 0;
     
        } else if (socketStatus->ngihss_listenCallbackRegistered != 0) {
     
            callbackType = NGI_EVENT_CALLBACK_TYPE_LISTENER;
            callbackFunction = socketStatus->ngihss_listenCallbackFunction;
            callbackArgument = socketStatus->ngihss_listenCallbackArgument;
            callbackExecuting = NULL; /* Do not wait done */
           
            assert(callbackFunction != NULL);

            /* Unregister the callback. */
            socketStatus->ngihss_listenCallbackFunction = NULL;
            socketStatus->ngihss_listenCallbackArgument = NULL;
            socketStatus->ngihss_listenCallbackRegistered = 0;
     
            socketStatus->ngihss_requireProcess = 0;
        } else {
            abort();
        }
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_TIME_EVENT) {

        callbackType = NGI_EVENT_CALLBACK_TYPE_TIME_EVENT;
        callbackFunction = timeStatus->ngihts_timeCallbackFunction;
        callbackArgument = timeStatus->ngihts_timeCallbackArgument;
        callbackExecuting = &timeStatus->ngihts_timeCallbackExecuting;
        
        assert(callbackFunction != NULL);

        /* Note : Time callback is not unregistered in normal state. */

        /* Unregister the callback. */
        if (state == NGI_IOHANDLE_STATE_CANCELED) {
            timeStatus->ngihts_timeCallbackFunction = NULL;
            timeStatus->ngihts_timeCallbackArgument = NULL;

            if ((timeStatus->ngihts_timeCallbackFunction == NULL) &&
                (timeStatus->ngihts_changeCallbackFunction == NULL)) {
                timeStatus->ngihts_callbackRegistered = 0;
            }
        }

    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE) {

        callbackType = NGI_EVENT_CALLBACK_TYPE_TIME_CHANGE;
        callbackFunction = timeStatus->ngihts_changeCallbackFunction;
        callbackArgument = timeStatus->ngihts_changeCallbackArgument;
        callbackExecuting = &timeStatus->ngihts_changeCallbackExecuting;
        
        assert(callbackFunction != NULL);

        /* Note : Time callback is not unregistered in normal state. */

        /* Unregister the callback. */
        if (state == NGI_IOHANDLE_STATE_CANCELED) {
            timeStatus->ngihts_changeCallbackFunction = NULL;
            timeStatus->ngihts_changeCallbackArgument = NULL;

            if ((timeStatus->ngihts_timeCallbackFunction == NULL) &&
                (timeStatus->ngihts_changeCallbackFunction == NULL)) {
                timeStatus->ngihts_callbackRegistered = 0;
            }
        }

    } else {
        abort();
    }

    if (callbackExecuting != NULL) {
        *callbackExecuting = 1;
    }
     
    /* Call the callback. */
    if (driver->nged_isPthread != 0) {

        result = nglEventCallbackRunRequest(
            driver, handle,
            callbackType, callbackFunction, callbackArgument,
            state, callbackExecuting, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Request the callback run failed.\n"); 
            goto error;
        }
    } else {

        result = nglEventDriverCallbackRun(
            driver, handle,
            callbackType, callbackFunction, callbackArgument,
            state, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Run the callback failed.\n"); 
            goto error;
        }

        if (callbackExecuting != NULL) {
            *callbackExecuting = 0;
        }

        /* Process Time Change callback after Time Event callback. */
        if ((callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_EVENT) &&
            (state == NGI_IOHANDLE_STATE_NORMAL)) {

            handle->ngih_time.ngihts_changeRequested = 1;

            result = nglEventDriverCommandNotify(
                driver, handle,
                NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE,
                log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "The command to Event Driver failed.\n"); 
                goto error;
            }
        }
    }

    if (driver->nged_isPthread == 0) {
        *finish = 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Run the Callback.
 */
static int
nglEventDriverCallbackRun(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *callbackHandle,
    ngiEventCallbackType_t callbackType,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngiIOhandleState_t callbackState,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverCallbackRun";
    char *callbackName, *stateName;
    int result, errorNumber;
    int id, driverID;

    /* Check the arguments */
    assert(driver != NULL);
    assert(callbackHandle != NULL);
    assert(callbackType > NGI_EVENT_CALLBACK_TYPE_NONE);
    assert(callbackType < NGI_EVENT_CALLBACK_TYPE_NOMORE);
    assert(callbackFunction != NULL);
    assert(callbackState > NGI_IOHANDLE_STATE_NONE);
    assert(callbackState < NGI_IOHANDLE_STATE_NOMORE);
    /* callbackArgument can be NULL. */

    callbackName = NULL;
    stateName = NULL;

    if (callbackType == NGI_EVENT_CALLBACK_TYPE_READ) {
        callbackName = "read";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_LISTENER) {
        callbackName = "socket listener";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_EVENT) {
        callbackName = "time event";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_CHANGE) {
        callbackName = "time change";

    } else {
        abort();
    }

    if (callbackState == NGI_IOHANDLE_STATE_NORMAL) {
        stateName = "normal";    
    } else if (callbackState == NGI_IOHANDLE_STATE_CLOSED) {
        stateName = "closed";    
    } else if (callbackState == NGI_IOHANDLE_STATE_CANCELED) {
        stateName = "canceled";    
    }

    /* Note : After the callback executed, handle may not exist. */
    driverID = callbackHandle->ngih_eventDriverID;
    id = callbackHandle->ngih_id;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The %s callback(state=%s) for handle %d.%d is calling.\n",
        callbackName, stateName, driverID, id);

    errorNumber = NG_ERROR_NO_ERROR;
    result = (*callbackFunction)(
        callbackArgument, callbackHandle, callbackState,
        log, &errorNumber);
    if (result == 0) {
        /* Error from user of Event Module. */
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "The %s callback function for handle %d.%d returned by error.\n",
            callbackName, driverID, id);

        /* Just forget. */
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The %s callback(state=%s) for handle %d.%d return.\n",
        callbackName, stateName, driverID, id);

    /* Success */
    return 1;
}

/**
 * Event Driver: Handle chunk callback call.
 * Note: Chunk callback is called on driver thread directly.
 */
static int
nglEventDriverHandleChunkCallbackCall(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngiIOhandleState_t state,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverHandleChunkCallbackCall";
    ngiIOhandleCallbackFunction_t callbackFunction;
    ngiIOhandleChunkStatus_t *status;
    int result, errorNumber;
    void *callbackArgument;
    char *callbackName;
    int id, driverID;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(state > NGI_IOHANDLE_STATE_NONE);
    assert(state < NGI_IOHANDLE_STATE_NOMORE);
    assert(finish != NULL);

    driverID = handle->ngih_eventDriverID;
    id = handle->ngih_id;

    status = &handle->ngih_chunk;
    
    callbackName = "read chunk";
    callbackFunction = status->ngihcs_callbackFunction;
    callbackArgument = status->ngihcs_callbackArgument;

    assert(callbackFunction != NULL);
    /* callbackArgument can be NULL. */

    /* Note : Read chunk callback is not unregistered in normal state. */

    /* Unregister the callback. */
    if (state != NGI_IOHANDLE_STATE_NORMAL) {

        status->ngihcs_callbackFunction = NULL;
        status->ngihcs_callbackArgument = NULL;
        status->ngihcs_callbackRegistered = 0;

        ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The %s callback(state=%s) for handle %d.%d call.\n",
            callbackName,
            ((state == NGI_IOHANDLE_STATE_CLOSED) ? "closed" :
            ((state == NGI_IOHANDLE_STATE_CANCELED) ? "canceled" :
            "unknown")),
            driverID, id);
    }
     
    /* No log output, because chunk callback is called many times. */

    /* Call the callback directly. */
    errorNumber = NG_ERROR_NO_ERROR;
    result = (*callbackFunction)(
        callbackArgument, handle, state, log, &errorNumber);
    if (result == 0) {
        /* Error from user of Event Module. */
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "The %s callback function for handle %d.%d returned by error.\n",
            callbackName, driverID, id);

        /* Just forget. */
    }

    /* No finish on NonThread. */

    /* Success */
    return 1;
}

/**
 * Event Driver: Command notify.
 */
static int
nglEventDriverCommandNotify(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverCommandNotify";
    int result, finish;

    /* Check the arguments */
    assert(driver != NULL);
    /* handle may NULL or not NULL. */
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);

    /**
     * Note : handle information is not passed to driver thread.
     */

    if (driver->nged_isPthread) {
        /* Send the command to driver thread. */
        result = nglEventDriverCommandThreadSend(
            driver, type, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Send the command to driver thread failed.\n");
            goto error;
        }
        
    } else {
        finish = 0;

        /* Process the command directly. */
        result = nglEventDriverProcessCommand(
            driver, type, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Process command failed.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Send the command to driver thread.
 */
static int
nglEventDriverCommandThreadSend(
    ngiEventDriver_t *driver,
    nglEventDriverHandleOperationType_t type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverCommandThreadSend";
    ssize_t resultSize;
    char buf[1];

    /* Check the arguments */
    assert(driver != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);

    buf[0] = (char)type;
    resultSize = write(driver->nged_commandWrite, buf, 1);
    if (resultSize <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n", "write()", errno, strerror(errno)); 
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
 * Event Driver: Receive the command by driver thread.
 */
static int
nglEventDriverCommandThreadReceive(
    ngiEventDriver_t *driver,
    int flags,
    int *finish,
    int isReadCloseHup,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverCommandThreadReceive";
    nglEventDriverHandleOperationType_t type;
    ssize_t resultSize;
    char buf[1];
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(finish != NULL);

    if (isReadCloseHup != 0) {
        if (flags & ~(NGI_SELECTOR_FLAG_IN | NGI_SELECTOR_FLAG_HUP)) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unknown event occurred for command pipe.\n"); 
            goto error;
        }
    } else {
        if (flags & ~NGI_SELECTOR_FLAG_IN) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unknown event occurred for command pipe.\n"); 
            goto error;
        }
    }

    resultSize = read(driver->nged_commandRead, buf, 1);

    if (resultSize < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "read()", errno, strerror(errno)); 
        goto error;
    }
    if (resultSize == 0) {
        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command connection closed.\n"); 
        goto error;
    }

    type = (nglEventDriverHandleOperationType_t)buf[0];

    if ((resultSize != 1) ||
        (type <= NGL_EVENT_DRIVER_HANDLE_OP_NONE) ||
        (type >= NGL_EVENT_DRIVER_HANDLE_OP_NOMORE)) {

        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The expected command was not arrived (size = %ld: %d).\n",
            (long)resultSize, type); 
        goto error;
    }

    /* Process the command. */
    result = nglEventDriverProcessCommand(
        driver, type, finish, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Process the command failed.\n");
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
 * Event Driver: Process command.
 */
static int
nglEventDriverProcessCommand(
    ngiEventDriver_t *driver,
    nglEventDriverHandleOperationType_t type,
    int *finish,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommand";
    ngiIOhandle_t *handle;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(finish != NULL);

    /* Note: type is not decoded. */

    if (driver->nged_continue == 0) {
        /* The driver thread finish. */
        *finish = 1;

        /* Success */
        return 1;
    }

    /* Process each handle operation. */
    handle = driver->nged_ioHandle_head;
    while (handle != NULL) {

        result = nglEventDriverProcessCommandHandle(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Command for handle process failed.\n"); 
            goto error;
        }

        handle = handle->ngih_next;
    }

    /* Process handle append/remove operation. */
    if ((driver->nged_handleNumStarted == 0) &&
        (driver->nged_handleNumRequesting != 0)) {

        driver->nged_handleNumStarted = 1;

        result = nglEventDriverProcessCommandHandleNumber(
            driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Number of handles operation failed.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for the handle.
 */
static int
nglEventDriverProcessCommandHandle(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandle";
    ngiIOhandleChunkStatus_t *chunkStatus;
    ngiIOhandleSocketStatus_t *socketStatus;
    ngiIOhandleFileStatus_t *fileStatus;
    ngiIOhandleTimeStatus_t *timeStatus;
    ngiIOhandleStatus_t *status;
    int result;

    assert(driver != NULL);
    assert(handle != NULL);

    status = NULL;
    chunkStatus = NULL;
    socketStatus = NULL;
    fileStatus = NULL;
    timeStatus = NULL;

    if (handle->ngih_valid == 0) {
        /* Skip the handle. */
        return 1;
    }

    /* Read operation start trigger. */
    status = &handle->ngih_read;
    if ((status->ngihs_started == 0) &&
        (status->ngihs_requesting != 0)) {
        status->ngihs_currentNbytes = 0;
        status->ngihs_started = 1;
        status->ngihs_requireProcess = 1;
    }

    /* Write operation start trigger. */
    status = &handle->ngih_write;
    if ((status->ngihs_started == 0) &&
        (status->ngihs_requesting != 0)) {
        status->ngihs_currentNbytes = 0;
        status->ngihs_started = 1;
        status->ngihs_requireProcess = 1;
    }

    /* Read callback operation trigger. */
    status = &handle->ngih_read;
    if ((status->ngihs_callbackRegistered == 0) &&
        (status->ngihs_callbackRegistering != 0)) {
        status->ngihs_callbackRegistering = 0;
        status->ngihs_callbackRegistered = 1;
        status->ngihs_requireProcess = 1;

        result = nglEventDriverProcessCommandHandleReadCallbackRegister(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Register read callback process failed.\n"); 
            goto error;
        }
    }

    /* Read callback unregister process. */
    if ((status->ngihs_callbackUnregistering != 0) &&
        (status->ngihs_callbackUnregisterDone == 0)) {

        result = nglEventDriverProcessCommandHandleReadCallbackCancel(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Cancel read callback process failed.\n"); 
            goto error;
        }
    }

    /* Read chunk callback operation trigger. */
    chunkStatus = &handle->ngih_chunk;
    if ((chunkStatus->ngihcs_callbackRegistered == 0) &&
        (chunkStatus->ngihcs_callbackRegistering != 0)) {
        chunkStatus->ngihcs_callbackRegistering = 0;
        chunkStatus->ngihcs_callbackRegistered = 1;

        result = nglEventDriverProcessCommandHandleReadChunkCallbackRegister(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Register read chunk callback process failed.\n"); 
            goto error;
        }
    }

    /* Read chunk callback unregister process. */
    if ((chunkStatus->ngihcs_callbackUnregistering != 0) &&
        (chunkStatus->ngihcs_callbackUnregisterDone == 0)) {

        result = nglEventDriverProcessCommandHandleReadChunkCallbackCancel(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Cancel read chunk callback process failed.\n"); 
            goto error;
        }
    }

    /* Socket operation trigger and process. */
    socketStatus = &handle->ngih_socket;
    if ((socketStatus->ngihss_started == 0) &&
        (socketStatus->ngihss_requesting != 0)) {

        socketStatus->ngihss_started = 1;
        socketStatus->ngihss_requireProcess = 1; /* Reset by Socket process. */

        result = nglEventDriverProcessCommandHandleSocket(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Socket command process failed.\n"); 
            goto error;
        }
    }

    /* Socket listen callback operation trigger. */
    if ((socketStatus->ngihss_listenCallbackRegistered == 0) &&
        (socketStatus->ngihss_listenCallbackRegistering != 0)) {
        socketStatus->ngihss_listenCallbackRegistering = 0;
        socketStatus->ngihss_listenCallbackRegistered = 1;
        socketStatus->ngihss_requireProcess = 1;

        result =
            nglEventDriverProcessCommandHandleSocketListenerCallbackRegister(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Register listen callback process failed.\n"); 
            goto error;
        }
    }

    /* Socket listen callback unregister process. */
    if ((socketStatus->ngihss_listenCallbackUnregistering != 0) &&
        (socketStatus->ngihss_listenCallbackUnregisterDone == 0)) {
        result =
            nglEventDriverProcessCommandHandleSocketListenerCallbackCancel(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Cancel listen callback process failed.\n"); 
            goto error;
        }
    }

    /* File operation trigger and process. */
    fileStatus = &handle->ngih_file;
    if ((fileStatus->ngihfs_started == 0) &&
        (fileStatus->ngihfs_requesting != 0)) {

        fileStatus->ngihfs_started = 1;
        fileStatus->ngihfs_requireProcess = 1; /* Reset by File process. */

        result = nglEventDriverProcessCommandHandleFile(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "File command process failed.\n"); 
            goto error;
        }
    }

    /* Time callback register process. */
    timeStatus = &handle->ngih_time;
    if ((timeStatus->ngihts_callbackRegistered == 0) &&
        (timeStatus->ngihts_callbackRegistering != 0)) {
        timeStatus->ngihts_callbackRegistering = 0;
        timeStatus->ngihts_callbackRegistered = 1;

        result = nglEventCondBroadcast(&driver->nged_cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Broadcast the cond failed.\n"); 
            goto error;
        }
    }

    /* Time callback unregister process. */
    if ((timeStatus->ngihts_callbackUnregistering != 0) &&
        (timeStatus->ngihts_callbackUnregisterDone == 0)) {

        result = nglEventDriverProcessCommandHandleTimeCallbackCancel(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Cancel time event callback process failed.\n"); 
            goto error;
        }
    }

    /* Time change operation process. */
    if ((timeStatus->ngihts_callbackRegistered != 0) &&
        (timeStatus->ngihts_changeRequested != 0) &&
        (timeStatus->ngihts_timeCallbackExecuting == 0) &&
        (timeStatus->ngihts_changeCallbackExecuting == 0)) {

        /* Note : Change Request is also treated after the callback done. */
        result = nglEventDriverProcessCommandHandleTimeChange(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Time change command process failed.\n"); 
            goto error;
        }
    }

    /* Close operation process. */
    if ((handle->ngih_closeStarted == 0) &&
        (handle->ngih_closeRequesting != 0)) {

        handle->ngih_closeStarted = 1;

        result = nglEventDriverProcessCommandHandleClose(
            driver, handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "handle close process failed.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle read callback register.
 */
static int
nglEventDriverProcessCommandHandleReadCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleReadCallbackRegister";
    int result, finish;

    finish = 0;

    if ((handle->ngih_userOpened == 0) ||
        (handle->ngih_fdEffective == 0)) {

        /* Call the CLOSED callback. */
        result = nglEventDriverHandleClosed(
            driver, handle, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "handle close failed.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */

    /* Notify register done. */
    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle read callback cancel.
 */
static int
nglEventDriverProcessCommandHandleReadCallbackCancel(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleReadCallbackCancel";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleStatus_t *status;
    ngiIOhandleState_t state;
    int result, finish;

    status = &handle->ngih_read;
    type = NGL_EVENT_DRIVER_HANDLE_OP_NONE;
    state = NGI_IOHANDLE_STATE_NONE;
    finish = 0;

    if (status->ngihs_callbackRegistered != 0) {

        type = NGL_EVENT_DRIVER_HANDLE_OP_READ;
        state = NGI_IOHANDLE_STATE_CANCELED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */
    
    status->ngihs_callbackUnregisterDone = 1;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle read chunk callback register.
 */
static int
nglEventDriverProcessCommandHandleReadChunkCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleReadChunkCallbackRegister";
    int result, finish;

    finish = 0;

    if ((handle->ngih_userOpened == 0) ||
        (handle->ngih_fdEffective == 0)) {

        /* Call the CLOSED callback. */
        result = nglEventDriverHandleClosed(
            driver, handle, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "handle close failed.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */

    /* Notify register done. */
    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle read chunk callback cancel.
 */
static int
nglEventDriverProcessCommandHandleReadChunkCallbackCancel(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleReadChunkCallbackCancel";
    ngiIOhandleChunkStatus_t *status;
    ngiIOhandleState_t state;
    int result, finish;

    status = &handle->ngih_chunk;
    state = NGI_IOHANDLE_STATE_NONE;
    finish = 0;

    if (status->ngihcs_callbackRegistered != 0) {

        state = NGI_IOHANDLE_STATE_CANCELED;
        result = nglEventDriverHandleChunkCallbackCall(
            driver, handle, state, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke chunk callback.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */
    
    status->ngihcs_callbackUnregisterDone = 1;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle socket.
 */
static int
nglEventDriverProcessCommandHandleSocket(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleSocket";
    ngiIOhandleSocketStatus_t *status;
    int errorOccurred, errorCode;
    int requireProcess, result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    status = &handle->ngih_socket;
    requireProcess = 0;

    /**
     * The error is reported to caller driver.
     */
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    if (status->ngihss_listenerCreateRequest != 0) {
        result = nglEventDriverProcessCommandHandleSocketListenerCreate(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Socket listener creation failed.\n"); 
            errorOccurred = 1;
        }

    } else if (status->ngihss_acceptRequest != 0) {
        result = nglEventDriverProcessCommandHandleSocketAccept(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Socket accept failed.\n"); 
            errorOccurred = 1;
        }

    } else if (status->ngihss_connectRequest != 0) {
        result = nglEventDriverProcessCommandHandleSocketConnect(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Socket connect failed.\n"); 
            errorOccurred = 1;
        }

    } else if (status->ngihss_nodelayRequest != 0) {
        result = nglEventDriverProcessCommandHandleSocketNodelay(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Socket nodelay set failed.\n"); 
            errorOccurred = 1;
        }

    } else {
        abort();
    }
    
    if (errorOccurred != 0) {
        status->ngihss_errorOccurred = 1;
        status->ngihss_errorCode = errorCode;
        requireProcess = 0;
    }

    if (requireProcess == 0) {

        /* Lock: mutex is already locked. */
        status->ngihss_requireProcess = 0;
        status->ngihss_done = 1;
     
        result = nglEventCondBroadcast(&driver->nged_cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Broadcast the cond failed.\n"); 
            goto error;
        }

        /* Unlock: mutex is already locked. */
    }

    /* Success */
    return 1;
error:
    return 0;
}

/**
 * Event Driver: Process command for handle Socket listener create.
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCreate(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCreate";
    ngiIOhandleSocketStatus_t *status;
    ngiIOhandleSocketType_t socketType;
    int listenFd, backlog;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_socket;
    socketType = status->ngihss_socketType;
    listenFd = -1;

    backlog = status->ngihss_listenerCreateBacklog;
    if (backlog == NGI_IO_HANDLE_LISTENER_CREATE_BACKLOG_DEFAULT) {
        backlog = SOMAXCONN;
    }

    if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
        result = nglEventDriverProcessCommandHandleSocketListenerCreateTCP(
            driver, handle, &listenFd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Create Listener for TCP failed.\n");
            goto error;
        }
    } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
        result = nglEventDriverProcessCommandHandleSocketListenerCreateUNIX(
            driver, handle, &listenFd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Create Listener for UNIX domain socket failed.\n");
            goto error;
        }
    } else {
        abort();
    }

    result = listen(listenFd, backlog);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "listen()", errno, strerror(errno)); 
        goto error;
    }

    result = fcntl(listenFd,
        F_SETFL, O_NONBLOCK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "fcntl()", errno, strerror(errno)); 
        goto error;
    }

    handle->ngih_fd = listenFd;
    handle->ngih_fdEffective = 1;

    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for TCP Socket listener create.
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCreateTCP(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *listenFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCreateTCP";
    ngiIOhandleSocketStatus_t *status;
    unsigned short portNo, allocated;
    struct sockaddr_in addr;
    ngiSockLen_t addrLen;
    int result, fd;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(listenFd != NULL);

    *listenFd = -1;

    status = &handle->ngih_socket;
    status->ngihss_listenerCreatePortAllocated = 0;
    allocated = 0;
    fd = 0;

    /* Argument port 0 means to allocate any available port. */
    /* Port 0 for bind() means to allocate any available port too. */
    portNo = status->ngihss_listenerCreatePort; /* int to ushort */

    memset(&addr, 0, sizeof(addr));

    /* getaddrinfo() did not worked well for server. */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNo);
    result = inet_aton("0.0.0.0", &(addr.sin_addr));
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed.\n", "inet_aton()"); 
        goto error;
    }
    
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "socket()", errno, strerror(errno)); 
        goto error;
    }

    result = bind(fd,
        (struct sockaddr *)&addr, sizeof(addr));
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "bind()", errno, strerror(errno)); 
        goto error;
    }

    /* Get the allocated port number. */
    memset(&addr, 0, sizeof(addr));
    addrLen = sizeof(addr);

    result = getsockname(fd,
        (struct sockaddr *)&addr,  &addrLen);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "getsockname()", errno, strerror(errno)); 
        goto error;
    }
    if (addrLen != sizeof(addr)) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed. The input and output size is different.\n",
            "getsockname()"); 
        goto error;
    }

    allocated = ntohs(addr.sin_port);
    status->ngihss_listenerCreatePortAllocated = allocated;

    *listenFd = fd;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#ifdef NGI_UNIX_SOCKET_ENABLED

/**
 * Event Driver: Process command for handle UNIX domain socket listener create.
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCreateUNIX(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *listenFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCreateUNIX";
    ngiIOhandleSocketStatus_t *status;
    struct sockaddr_un addr;
    ngiSockLen_t addrLen;
    size_t pathLen;
    int result, fd;
    char *path;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(listenFd != NULL);

    *listenFd = -1;

    status = &handle->ngih_socket;
    path = NULL;
    pathLen = 0;

    /**
     * Note: The UNIX socket is only used on Linux.
     * Linux environment have user based access control on UNIX socket.
     * Currently, access check on Linux is not implemented.
     */

    path = status->ngihss_listenerCreatePath;
    pathLen = strlen(path);

    if (pathLen >= NGI_UNIX_SOCKET_PATH_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "path \"%s\" is too long. over %d.\n",
            path, NGI_UNIX_SOCKET_PATH_MAX); 
        goto error;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_INET;
    strcpy(addr.sun_path, path); /* string is wrote in the buffer. */

    addrLen = sizeof(addr.sun_family) + pathLen + 1;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "socket()", errno, strerror(errno)); 
        goto error;
    }

    result = bind(fd,
        (struct sockaddr *)&addr, addrLen);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "bind()", errno, strerror(errno)); 
        goto error;
    }

    *listenFd = fd;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#else /* NGI_UNIX_SOCKET_ENABLED */

/**
 * Event Driver: Process command for handle UNIX domain socket listener create.
 * (Just error)
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCreateUNIX(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *listenFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCreateUNIX";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* This error may notified to Event Driver to return error. */

    /* Failed */
    return 0;
}

#endif /* NGI_UNIX_SOCKET_ENABLED */


/**
 * Event Driver: Process command for handle socket accept.
 */
static int
nglEventDriverProcessCommandHandleSocketAccept(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketAccept";
    ngiIOhandleAcceptResult_t *acceptResult;
    ngiIOhandleSocketType_t socketType;
    ngiIOhandleSocketStatus_t *status;
    struct sockaddr *peerAddr;
    int listenFd, connectFd;
    ngiSockLen_t peerAddrLen;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_socket;
    socketType = status->ngihss_socketType;

    acceptResult = ngiIOhandleAcceptResultConstruct(log, error);
    if (acceptResult == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Construct the Accept Result failed.\n");
        goto error;
    }

    acceptResult->ngiar_type = socketType;

    listenFd = handle->ngih_fd;
    if (listenFd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The fd for I/O handle invalid.\n"); 
        goto error;
    }

    peerAddr = &(acceptResult->ngiar_peerAddress);

    memset(peerAddr, 0, sizeof(*peerAddr));
    peerAddrLen = sizeof(*peerAddr);

    connectFd = accept(listenFd,
        (struct sockaddr *)peerAddr, &peerAddrLen);
    if (connectFd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n", "accept()", errno, strerror(errno)); 
        goto error;
    }

    result = fcntl(connectFd,
        F_SETFL, O_NONBLOCK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n", "fcntl()", errno, strerror(errno)); 
        goto error;
    }

    status->ngihss_acceptNewFd = connectFd;
    status->ngihss_acceptResult = acceptResult;

    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle socket connect.
 */
static int
nglEventDriverProcessCommandHandleSocketConnect(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketConnect";
    ngiIOhandleSocketType_t socketType;
    ngiIOhandleSocketStatus_t *status;
    int connectFd;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_socket;
    socketType = status->ngihss_socketType;
    connectFd = -1;

    if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
        result = nglEventDriverProcessCommandHandleSocketConnectTCP(
            driver, handle, &connectFd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Connect for TCP failed.\n");
            goto error;
        }
    } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
        result = nglEventDriverProcessCommandHandleSocketConnectUNIX(
            driver, handle, &connectFd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Connect for UNIX domain socket failed.\n");
            goto error;
        }
    } else {
        abort();
    }

    result = fcntl(connectFd,
        F_SETFL, O_NONBLOCK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "fcntl()", errno, strerror(errno)); 
        goto error;
    }

    handle->ngih_fd = connectFd;
    handle->ngih_fdEffective = 1;

    /* Currently, non blocking connect() is not implemented. */
    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle TCP socket connect.
 */
static int
nglEventDriverProcessCommandHandleSocketConnectTCP(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *connectFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketConnectTCP";
    ngiIOhandleSocketStatus_t *status;
    struct addrinfo addrHints, *addrList;
    int serverPort;
    char serverPortStr[NGI_INT_MAX_DECIMAL_DIGITS];
    char *serverName;
    int result, fd;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(connectFd != NULL);

    *connectFd = -1;

    status = &handle->ngih_socket;
    serverName = status->ngihss_connectHostName;
    serverPort = status->ngihss_connectPort;
    result = snprintf(serverPortStr, sizeof(serverPortStr),
        "%d", serverPort);
    if (result <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed.\n", "snprintf()"); 
        goto error;
    }

    memset(&addrHints, 0, sizeof(addrHints));
    addrHints.ai_family = PF_INET; /* or AF_UNSPEC ? */
    addrHints.ai_socktype = SOCK_STREAM;
    addrHints.ai_protocol = 0;

    addrList = NULL;
    result = getaddrinfo(
        serverName, serverPortStr, &addrHints, &addrList);
    if ((result != 0) || (addrList == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "getaddrinfo()", result, gai_strerror(result)); 
        goto error;
    }

    /* Note : Use first one on addrList. */

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "socket()", errno, strerror(errno)); 
        goto error;
    }

    result = connect(fd, addrList->ai_addr, addrList->ai_addrlen);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "connect()", errno, strerror(errno)); 
        goto error;
    }

    /* Currently, non blocking connect() is not implemented. */
    
    freeaddrinfo(addrList);

    *connectFd = fd;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#ifdef NGI_UNIX_SOCKET_ENABLED

/**
 * Event Driver: Process command for UNIX domain socket connect.
 */
static int
nglEventDriverProcessCommandHandleSocketConnectUNIX(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *connectFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketConnectUNIX";
    ngiIOhandleSocketStatus_t *status;
    struct sockaddr_un addr;
    ngiSockLen_t addrLen;
    size_t pathLen;
    int result, fd;
    char *path;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(connectFd != NULL);

    *connectFd = -1;

    status = &handle->ngih_socket;
    path = NULL;
    pathLen = 0;
    addrLen = 0;

    path = status->ngihss_connectPath;
    pathLen = strlen(path);

    if (pathLen >= NGI_UNIX_SOCKET_PATH_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "path \"%s\" is too long. over %d.\n",
            path, NGI_UNIX_SOCKET_PATH_MAX); 
        goto error;
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path); /* string is wrote in the buffer. */

    addrLen = sizeof(addr.sun_family) + pathLen + 1;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "socket()", errno, strerror(errno)); 
        goto error;
    }

    result = connect(fd, (struct sockaddr *)&addr, addrLen);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "connect()", errno, strerror(errno)); 
        goto error;
    }

    /* Currently, non blocking connect() is not implemented. */

    *connectFd = fd;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#else /* NGI_UNIX_SOCKET_ENABLED */

/**
 * Event Driver: Process command for handle UNIX domain socket connect.
 * (Just error)
 */
static int
nglEventDriverProcessCommandHandleSocketConnectUNIX(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *connectFd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketConnectUNIX";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* This error may notified to Event Driver to return error. */

    /* Failed */
    return 0;
}

#endif /* NGI_UNIX_SOCKET_ENABLED */

/**
 * Event Driver: Process command for handle socket nodelay.
 */
static int
nglEventDriverProcessCommandHandleSocketNodelay(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketNodelay";
    ngiIOhandleSocketStatus_t *status;
    int result, fd, enable;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_socket;
    enable = status->ngihss_nodelayEnable;

    fd = handle->ngih_fd;
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "The fd for I/O handle invalid.\n");
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName,
        "Set TCP_NODELAY to %d for handle %d.%d.\n",
        enable, handle->ngih_eventDriverID, handle->ngih_id);

    /* Set TCP_NODELAY */
    result = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
        &enable, sizeof(enable));
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "setsockopt()", errno, strerror(errno)); 
        goto error;
    }

    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}


/**
 * Event Driver: Process command for handle socket listener callback register.
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCallbackRegister";
    int result, finish;

    finish = 0;

    if ((handle->ngih_userOpened == 0) ||
        (handle->ngih_fdEffective == 0)) {

        /* Call the CLOSED callback. */
        result = nglEventDriverHandleClosed(
            driver, handle, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "handle close failed.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */

    /* Notify register done. */
    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle socket listener callback cancel.
 */
static int
nglEventDriverProcessCommandHandleSocketListenerCallbackCancel(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleSocketListenerCallbackCancel";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleSocketStatus_t *status;
    ngiIOhandleState_t state;
    int result, finish;

    status = &handle->ngih_socket;
    type = NGL_EVENT_DRIVER_HANDLE_OP_NONE;
    state = NGI_IOHANDLE_STATE_NONE;
    finish = 0;

    if (status->ngihss_listenCallbackRegistered != 0) {

        type = NGL_EVENT_DRIVER_HANDLE_OP_READ;
        state = NGI_IOHANDLE_STATE_CANCELED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */
    
    status->ngihss_listenCallbackUnregisterDone = 1;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle file.
 */
static int
nglEventDriverProcessCommandHandleFile(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleFile";
    ngiIOhandleFileStatus_t *status;
    int errorOccurred, errorCode;
    int requireProcess, result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    status = &handle->ngih_file;
    requireProcess = 0;

    /**
     * The error is reported to caller driver.
     */
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    if (status->ngihfs_fileOpenRequest != 0) {
        result = nglEventDriverProcessCommandHandleFileOpen(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "File open failed.\n"); 
            errorOccurred = 1;
        }

    } else if (status->ngihfs_fdOpenRequest != 0) {
        result = nglEventDriverProcessCommandHandleFileFd(
            driver, handle, &requireProcess, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "FD open failed.\n"); 
            errorOccurred = 1;
        }

    } else {
        abort();
    }
    
    if (errorOccurred != 0) {
        status->ngihfs_errorOccurred = 1;
        status->ngihfs_errorCode = errorCode;
        requireProcess = 0;
    }

    if (requireProcess == 0) {
        /* Lock: mutex is already locked. */
        status->ngihfs_requireProcess = 0;
        status->ngihfs_done = 1;
     
        result = nglEventCondBroadcast(&driver->nged_cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Broadcast the cond failed.\n"); 
            goto error;
        }
        /* Unlock: mutex is already locked. */
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle file open.
 */
static int
nglEventDriverProcessCommandHandleFileOpen(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleFileOpen";
    ngiIOhandleFileStatus_t *status;
    int result, fd, flags;
    char *path;
    mode_t mode;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_file;
    path = NULL;
    fd = -1;

    flags = 0;
    mode = 0;

    switch (status->ngihfs_fileOpenType) {
    case NGI_IOHANDLE_FILE_OPEN_TYPE_READ:
        flags = O_RDONLY;
        break;

    case NGI_IOHANDLE_FILE_OPEN_TYPE_WRITE:
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        break;

    default:
        abort();
    }

    path = status->ngihfs_fileOpenPath;
    assert(path != NULL);

    fd = open(path, flags, mode);
    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s for \"%s\" failed: %d: %s.\n",
            "open()", path, errno, strerror(errno)); 
        goto error;
    }

    result = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "fcntl()", errno, strerror(errno)); 
        goto error;
    }

    handle->ngih_fd = fd;
    handle->ngih_fdEffective = 1;

    /* Currently, non blocking open() is not implemented. */
    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle file descriptor open.
 */
static int
nglEventDriverProcessCommandHandleFileFd(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int *requireProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleFileFd";
    ngiIOhandleFileStatus_t *status;
    int result, fd;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(requireProcess != NULL);

    *requireProcess = 0;

    status = &handle->ngih_file;

    fd = status->ngihfs_fdOpenFd;
    assert(fd >= 0);

    result = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "fcntl()", errno, strerror(errno)); 
        goto error;
    }

    handle->ngih_fd = fd;
    handle->ngih_fdEffective = 1;

    *requireProcess = 0;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle time change.
 */
static int
nglEventDriverProcessCommandHandleTimeChange(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleTimeChange";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleTimeStatus_t *status;
    ngiIOhandleState_t state;
    int result, finish;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    status = &handle->ngih_time;

    /* Reset the flag. */
    status->ngihts_changeRequested = 0;

    type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE;
    state = NGI_IOHANDLE_STATE_NORMAL;
    result = nglEventDriverHandleCallbackCall(
        driver, handle, type, state, &finish, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Failed to invoke callback.\n"); 
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
 * Event Driver: Process command for handle time callback cancel.
 */
static int
nglEventDriverProcessCommandHandleTimeCallbackCancel(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverProcessCommandHandleTimeCallbackCancel";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleTimeStatus_t *status;
    ngiIOhandleState_t state;
    int result, finish;

    status = &handle->ngih_time;
    type = NGL_EVENT_DRIVER_HANDLE_OP_NONE;
    state = NGI_IOHANDLE_STATE_NONE;
    finish = 0;

    status->ngihts_eventTime = 0;
    status->ngihts_changeRequested = 0;

    if (status->ngihts_callbackRegistered != 0) {

        type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_EVENT;
        state = NGI_IOHANDLE_STATE_CANCELED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            goto error;
        }

        type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE;
        state = NGI_IOHANDLE_STATE_CANCELED;
        result = nglEventDriverHandleCallbackCall(
            driver, handle, type, state, &finish, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Failed to invoke callback.\n"); 
            goto error;
        }
    }

    /* Lock: mutex is already locked. */
    
    status->ngihts_callbackUnregisterDone = 1;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle close.
 */
static int
nglEventDriverProcessCommandHandleClose(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleClose";
    int result, finish;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    finish = 0;

    /* Close the handle. */
    result = nglEventDriverHandleClosed(
        driver, handle, &finish, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "handle close failed.\n"); 
        goto error;
    }

    /* Just ignore finish flag. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: Process command for handle append or remove.
 */
static int
nglEventDriverProcessCommandHandleNumber(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleNumber";
    ngiIOhandle_t *target;
    int errorOccurred, errorCode;
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    target = NULL;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    /* Lock: mutex is already locked. */

    assert((driver->nged_handleNumIncrease != 0) ||
        (driver->nged_handleNumDecrease != 0));
    assert((driver->nged_handleNumIncrease == 0) ||
        (driver->nged_handleNumDecrease == 0));

    target = driver->nged_handleNumTarget;
    assert(target != NULL);

    if (driver->nged_handleNumIncrease != 0) {
        result = nglEventDriverProcessCommandHandleAppend(
            driver, target, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Append the I/O handle failed.\n"); 
            errorOccurred = 1;
        }
    } else if (driver->nged_handleNumDecrease != 0) {
        result = nglEventDriverProcessCommandHandleRemove(
            driver, target, log, &errorCode);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Remove the I/O handle failed.\n"); 
            errorOccurred = 1;
        }
    } else {
        abort();
    }

    if (errorOccurred != 0) {
        driver->nged_handleNumErrorOccurred = 1;
        driver->nged_handleNumErrorCode = errorCode;
    }

    driver->nged_handleNumTarget = NULL;
    driver->nged_handleNumIncrease = 0;
    driver->nged_handleNumDecrease = 0;
    driver->nged_handleNumDone = 1;
    assert(driver->nged_handleNumRequesting != 0); /* Keep 1. */
    
    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    /* Unlock: mutex is already locked. */

    /* Success */
    return 1;
error:
    return 0;
}

/**
 * Event Driver: Process command for handle append.
 */
static int
nglEventDriverProcessCommandHandleAppend(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleAppend";
    int selectorSize, nextSelectorSize;
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    selectorSize = 0;

    assert(driver->nged_handleNumIncrease != 0);
    assert(driver->nged_handleNumDecrease == 0);

    nextSelectorSize = driver->nged_nHandles + 1 + 1;

    if (driver->nged_selectorSize < nextSelectorSize) {

        selectorSize = driver->nged_selectorSize;

        do {
            selectorSize *= 2;
        } while (selectorSize < nextSelectorSize);
     
        result = ngiSelectorResize(
            &driver->nged_selectorFunctions,
            driver->nged_selector,
            selectorSize, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Resize the Selector to %d failed.\n", selectorSize); 
            goto error;
        }

        driver->nged_selectorSize = selectorSize;
    }

    handle->ngih_eventDriverID = driver->nged_id;
    driver->nged_handleIDmax++;
    handle->ngih_id = driver->nged_handleIDmax;

    /* Register the I/O handle. */
    result = nglEventDriverIOhandleRegister(
        driver, handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the I/O handle failed.\n"); 
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
 * Event Driver: Process command for handle remove.
 */
static int
nglEventDriverProcessCommandHandleRemove(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverProcessCommandHandleRemove";
    int result;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    assert(driver->nged_handleNumIncrease == 0);
    assert(driver->nged_handleNumDecrease != 0);

    /* Currently, Selector is not resized. */

    /* Unregister the I/O handle. */
    result = nglEventDriverIOhandleUnregister(
        driver, handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the I/O handle failed.\n"); 
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
 * Event Driver: Request command for handle append or remove.
 */
static int
nglEventDriverRequestCommandHandleNumber(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleNumberMode_t mode,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandHandleNumber";
    int errorOccurred, errorCode;
    int result, mutexLocked;

    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(mode > NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_NOMORE);


    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    if (driver->nged_isPthread != 0) {
        /* Wait until other request done. */
        while (driver->nged_handleNumRequesting != 0) {
            result = nglEventCondWait(
                &driver->nged_cond, &driver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Wait the cond failed.\n"); 
                goto error;
            }
        }
    }

    assert(driver->nged_handleNumRequesting == 0);
    assert(driver->nged_handleNumDone == 0);
    assert(driver->nged_handleNumTarget == NULL);

    driver->nged_handleNumErrorOccurred = 0;
    driver->nged_handleNumErrorCode = NG_ERROR_NO_ERROR;

    driver->nged_handleNumIncrease = 0;
    driver->nged_handleNumDecrease = 0;

    if (mode == NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_APPEND) {
        driver->nged_handleNumIncrease = 1;
    } else if (mode == NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_REMOVE) {
        driver->nged_handleNumDecrease = 1;
    } else {
        abort();
    }
    driver->nged_handleNumTarget = handle;
    driver->nged_handleNumDone = 0;
    driver->nged_handleNumRequesting = 1;

    handle->ngih_id = 0;

    result = nglEventDriverCommandNotify(
        driver, NULL, NGL_EVENT_DRIVER_HANDLE_OP_NUMBER, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Notify the command to Event Driver failed.\n"); 
        goto error;
    }

    while (driver->nged_handleNumDone == 0) {
        result = nglEventDriverRequestDoneWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait request done failed.\n"); 
            goto error;
        }
    }

    errorOccurred = driver->nged_handleNumErrorOccurred;
    errorCode = driver->nged_handleNumErrorCode;

    driver->nged_handleNumIncrease = 0;
    driver->nged_handleNumDecrease = 0;
    driver->nged_handleNumTarget = NULL;

    driver->nged_handleNumErrorOccurred = 0;
    driver->nged_handleNumErrorCode = 0;

    driver->nged_handleNumRequesting = 0;
    driver->nged_handleNumStarted = 0;
    driver->nged_handleNumDone = 0;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    if (errorOccurred != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The error is occured.\n"); 
        NGI_SET_ERROR(error, errorCode);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if ((mutexLocked != 0) && (driver != NULL)) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle close.
 */
static int
nglEventDriverRequestCommandHandleClose(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandHandleClose";
    int errorOccurred, errorCode;
    int result, mutexLocked;

    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName,
        "The handle %d.%d close.\n",
        handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id); 
        goto error;
    }

    if (handle->ngih_userOpened == 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is not open, close not performed.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto success;
    }

    if (handle->ngih_fdEffective == 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is already closed, close not performed.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto success;
    }

    if (driver->nged_isPthread != 0) {
        while (handle->ngih_closeRequesting != 0) {
            result = nglEventCondWait(
                &driver->nged_cond, &driver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Wait the cond failed.\n"); 
                goto error;
            }
        }
    }

    assert(handle->ngih_closeRequesting == 0);

    handle->ngih_closeErrorOccurred = 0;
    handle->ngih_closeErrorCode = NG_ERROR_NO_ERROR;

    handle->ngih_closeRequesting = 1;

    result = nglEventDriverCommandNotify(
        driver, NULL, NGL_EVENT_DRIVER_HANDLE_OP_CLOSE, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Notify the command to Event Driver failed.\n"); 
        goto error;
    }

    while(handle->ngih_closeDone == 0) {
        result = nglEventDriverRequestDoneWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait request done failed.\n");
            goto error;
        }
    }

    errorOccurred = handle->ngih_closeErrorOccurred;
    errorCode = handle->ngih_closeErrorCode;

    handle->ngih_closeErrorOccurred = 0;
    handle->ngih_closeErrorCode = 0;

    handle->ngih_closeRequesting = 0;
    handle->ngih_closeStarted = 0;
    handle->ngih_closeDone = 0;

    handle->ngih_userOpened = 0;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

success:

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle read or write.
 */
static int
nglEventDriverRequestCommandHandleOperate(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    char *buf,
    size_t length,
    size_t waitNbytes,
    size_t *doneNbytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandHandleOperate";
    int result, mutexLocked, debugOutput, eofAndSkip;
    int errorOccurred, errorCode;
    ngiIOhandleStatus_t *status;
    char *typeName;

    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;
    eofAndSkip = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);
    assert(buf != NULL);
    assert(waitNbytes <= length);
    assert(doneNbytes != NULL);

    *doneNbytes = 0;

#ifdef NGI_IO_HANDLE_DEBUG_OUTPUT_RW
    debugOutput = 1;
#else /* NGI_IO_HANDLE_DEBUG_OUTPUT_RW */
    debugOutput = 0;
#endif /* NGI_IO_HANDLE_DEBUG_OUTPUT_RW */

    status = NULL;
    typeName = NULL;
    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        status = &handle->ngih_read;
        typeName = "read";
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {
        status = &handle->ngih_write;
        typeName = "write";
    } else {
        abort();
    }
    assert(status != NULL);

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    if (debugOutput != 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s(len=%lu, wait=%lu) for handle %d.%d.\n",
            typeName, (unsigned long)length, (unsigned long)waitNbytes,
            handle->ngih_eventDriverID, handle->ngih_id);
    }

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (handle->ngih_userOpened == 0) {
        /**
         * Note: read() after close() just return EOF, not an Error.
         * Because, read() timing on read callback is unknown,
         * while the main thread calls close().
         */

        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is not open.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        eofAndSkip = 1;
    }

    if (handle->ngih_fdEffective == 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is closed. return EOF.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        eofAndSkip = 1;
    }

    while ((status->ngihs_requesting != 0) ||
        (status->ngihs_callbackRegistering != 0) ||
        (status->ngihs_callbackUnregistering != 0)) {

        if (driver->nged_isPthread == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is entering double operation.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto error;
        }        

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }
    }

    assert(status->ngihs_requesting == 0);

    if (status->ngihs_callbackRegistered != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The callback for handle %d.%d is already registered.\n",
            handle->ngih_eventDriverID, handle->ngih_id); 
        goto error;
    }

    if (eofAndSkip != 0) {
        *doneNbytes = 0;
        goto success;
    }

    status->ngihs_buf = buf;
    status->ngihs_length = length;
    status->ngihs_waitNbytes = waitNbytes;
    status->ngihs_doneNbytesResult = doneNbytes;
    status->ngihs_errorOccurred = 0;
    status->ngihs_errorCode = NG_ERROR_NO_ERROR;

    status->ngihs_requesting = 1;

    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    while(status->ngihs_done == 0) {
        result = nglEventDriverRequestDoneWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait request done failed.\n");
            goto error;
        }
    }

    *doneNbytes = status->ngihs_doneNbytes;
    errorOccurred = status->ngihs_errorOccurred;
    errorCode = status->ngihs_errorCode;

    status->ngihs_buf = NULL;
    status->ngihs_length = 0;
    status->ngihs_waitNbytes = 0;
    status->ngihs_doneNbytesResult = NULL;
    status->ngihs_doneNbytes = 0;
    status->ngihs_errorOccurred = 0;
    status->ngihs_errorCode = 0;

    status->ngihs_requesting = 0;
    status->ngihs_started = 0;
    status->ngihs_done = 0;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

success:

    if (debugOutput != 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s(len=%lu, wait=%lu) for handle %d.%d return by size %lu.\n",
            typeName, (unsigned long)length, (unsigned long)waitNbytes,
            handle->ngih_eventDriverID, handle->ngih_id,
            (unsigned long)*doneNbytes);
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    if (errorOccurred != 0) {
        NGI_SET_ERROR(error, errorCode);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (debugOutput != 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s(len=%lu, wait=%lu) for handle %d.%d return by error.\n",
            typeName, (unsigned long)length, (unsigned long)waitNbytes,
            handle->ngih_eventDriverID, handle->ngih_id);
    }

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle read callback register.
 * Note: If the callback is not registered,
 *   unregister will do nothing and success.
 */
static int
nglEventDriverRequestCommandHandleReadCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandHandleReadCallbackRegister";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleStatus_t *status;
    int result, mutexLocked;
    char *modeName;

    mutexLocked = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(mode > NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NOMORE);

    type = NGL_EVENT_DRIVER_HANDLE_OP_READ;
    status = &handle->ngih_read;

    modeName = NULL;
    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        modeName = "Register";
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        modeName = "Unregister";
    } else {
        abort();
    }

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read callback for handle %d.%d.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open. accept register anyway.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
        }
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto success;
        }
    } else {
        abort();
    }

    /* No (handle->ngih_fdEffective == 0) check. */

    /* Wait the requestable state. */
    while (1) {
        if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
            if ((status->ngihs_callbackRegistering == 0) &&
                (status->ngihs_callbackUnregistering == 0) &&
                (status->ngihs_requesting == 0)) {
                break;
            }
        } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
            if ((status->ngihs_callbackRegistering == 0) &&
                (status->ngihs_callbackUnregistering == 0) &&
                (status->ngihs_requesting == 0)) {
                break;
            }
        } else {
            abort();
        }

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        assert(callbackFunction != NULL);
        /* callbackArgument may be NULL. */

        status->ngihs_callbackFunction = callbackFunction;
        status->ngihs_callbackArgument = callbackArgument;
        status->ngihs_callbackRegistering = 1;

    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        assert(callbackFunction == NULL);
        assert(callbackArgument == NULL);

        status->ngihs_callbackUnregisterDone = 0;
        status->ngihs_callbackUnregistering = 1;
    } else {
        abort();
    }

    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {

        /* Wait Unregistered. */
        while ((status->ngihs_callbackUnregisterDone == 0) ||
            (status->ngihs_callbackRegistered != 0)){

            result = nglEventDriverRequestDoneWait(driver, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                    "Wait request done failed.\n");
                goto error;
            }
        }
        assert(status->ngihs_callbackUnregistering != 0);
        assert(status->ngihs_callbackUnregisterDone != 0);

        assert(status->ngihs_callbackRegistering == 0);
        assert(status->ngihs_callbackRegistered == 0);
        assert(status->ngihs_callbackFunction == NULL);
        assert(status->ngihs_callbackArgument == NULL);

        status->ngihs_callbackUnregisterDone = 0;
        status->ngihs_callbackUnregistering = 0;
    }

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

success:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read callback for handle %d.%d return.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read callback for handle %d.%d return by error.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle read chunk callback register.
 * Note: If the callback is not registered,
 *   unregister will do nothing and success.
 */
static int
nglEventDriverRequestCommandHandleReadChunkCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandHandleReadChunkCallbackRegister";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleChunkStatus_t *status;
    int result, mutexLocked;
    char *modeName;

    mutexLocked = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(mode > NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NOMORE);

    type = NGL_EVENT_DRIVER_HANDLE_OP_READ;
    status = &handle->ngih_chunk;

    modeName = NULL;
    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        modeName = "Register";
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        modeName = "Unregister";
    } else {
        abort();
    }

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read chunk callback for handle %d.%d.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open. accept register anyway.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
        }
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto success;
        }
    } else {
        abort();
    }

    /* No (handle->ngih_fdEffective == 0) check. */

    /* Wait the requestable state. */
    while (1) {
        if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
            if ((status->ngihcs_callbackRegistering == 0) &&
                (status->ngihcs_callbackUnregistering == 0)) {
                break;
            }
        } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
            if ((status->ngihcs_callbackRegistering == 0) &&
                (status->ngihcs_callbackUnregistering == 0)) {
                break;
            }
        } else {
            abort();
        }

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        assert(callbackFunction != NULL);
        /* callbackArgument may be NULL. */

        status->ngihcs_callbackFunction = callbackFunction;
        status->ngihcs_callbackArgument = callbackArgument;
        status->ngihcs_callbackRegistering = 1;

    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        assert(callbackFunction == NULL);
        assert(callbackArgument == NULL);

        status->ngihcs_callbackUnregisterDone = 0;
        status->ngihcs_callbackUnregistering = 1;
    } else {
        abort();
    }

    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {

        /* Wait Unregistered. */
        while ((status->ngihcs_callbackUnregisterDone == 0) ||
            (status->ngihcs_callbackRegistered != 0)){

            result = nglEventDriverRequestDoneWait(driver, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                    "Wait request done failed.\n");
                goto error;
            }
        }
        assert(status->ngihcs_callbackUnregistering != 0);
        assert(status->ngihcs_callbackUnregisterDone != 0);

        assert(status->ngihcs_callbackRegistering == 0);
        assert(status->ngihcs_callbackRegistered == 0);
        assert(status->ngihcs_callbackFunction == NULL);
        assert(status->ngihcs_callbackArgument == NULL);

        status->ngihcs_callbackUnregisterDone = 0;
        status->ngihcs_callbackUnregistering = 0;
    }

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

success:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read chunk callback for handle %d.%d return.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the read chunk callback for handle %d.%d return by error.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle 
 *     socket listener create, accept, connect.
 */
static int
nglEventDriverRequestCommandHandleSocket(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngiIOhandleSocketType_t socketType,
    int isListenerCreate,
    int listenerPort,
    int *listenerPortAllocated,
    char *listenerPath,
    int listenerBacklog,
    int isAccept,
    ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    int isConnect,
    char *connectHost,
    int connectPort,
    char *connectPath,
    int isNodelay,
    int nodelayEnable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandHandleSocket";
    char *listenerPathName, *connectHostName, *connectPathName;
    ngiIOhandleAcceptResult_t *newAcceptResult;
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleSocketStatus_t *status;
    int gotListenerPort;
    int errorOccurred, errorCode;
    int result, mutexLocked;
    int newAcceptFd;

    status = NULL;
    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    gotListenerPort = -1;
    newAcceptFd = -1;
    newAcceptResult = NULL;
    listenerPathName = NULL;
    connectHostName = NULL;
    connectPathName = NULL;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(socketType > NGI_IOHANDLE_SOCKET_TYPE_NONE);
    assert(socketType < NGI_IOHANDLE_SOCKET_TYPE_NOMORE);

    assert((isListenerCreate != 0) || (isAccept != 0)  ||
        (isConnect != 0) || (isNodelay != 0));

    if (isListenerCreate != 0) {
         if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
             assert(listenerPort >= 0);
             assert(listenerPortAllocated != NULL);

         } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
             assert(listenerPath != NULL);

         } else {
             abort();
         }
         assert(isListenerCreate != 0);
         assert(isAccept == 0);
         assert(isConnect == 0);
         assert(isNodelay == 0);

    } else if (isAccept != 0) {
         assert(connectHandle != NULL);
         /* Accept Result can be NULL. */

         assert(isListenerCreate == 0);
         assert(isAccept != 0);
         assert(isConnect == 0);
         assert(isNodelay == 0);

    } else if (isConnect != 0) {
         if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
             assert(connectHost != NULL);
             assert(connectPort >= 0);

         } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
             assert(connectPath != NULL);

         } else {
             abort();
         }

         assert(isListenerCreate == 0);
         assert(isAccept == 0);
         assert(isConnect != 0);
         assert(isNodelay == 0);

    } else if (isNodelay != 0) {
         /* nodelayEnable is 0 or not 0. */

         assert(isListenerCreate == 0);
         assert(isAccept == 0);
         assert(isConnect == 0);
         assert(isNodelay != 0);
    } else {
         abort();
    }

    if (isListenerCreate == 0) {
        assert(listenerPort == 0);
        assert(listenerPortAllocated == NULL);
        assert(listenerPath == NULL);
        assert(listenerBacklog == 0);
    }
    if (isAccept == 0) {
        assert(connectHandle == NULL);
        assert(acceptResult == NULL);
    }
    if (isConnect == 0) {
        assert(connectHost == NULL);
        assert(connectPort == 0);
        assert(connectPath == NULL);
    }

    if (isNodelay == 0) {
        assert(nodelayEnable == 0);
    }

    if (isListenerCreate != 0) {
        *listenerPortAllocated = 0;
    }


    type = NGL_EVENT_DRIVER_HANDLE_OP_SOCKET;
    status = &handle->ngih_socket;

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if ((isListenerCreate != 0) || (isConnect != 0)) {
        if (handle->ngih_userOpened != 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is already open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto error;
        }
    } else if ((isAccept != 0) || (isNodelay != 0)) {
        if (handle->ngih_userOpened == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto error;
        }
    } else {
        abort();
    }

    if (driver->nged_isPthread != 0) {
        while(status->ngihss_requesting != 0) {
            result = nglEventCondWait(
                &driver->nged_cond, &driver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Wait the cond failed.\n"); 
                goto error;
            }
        }
    }

    assert(status->ngihss_requesting == 0);
    assert(status->ngihss_done == 0);

    if (status->ngihss_listenCallbackRegistering != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is requesting to register listener callback.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    assert(status->ngihss_listenerCreateRequest == 0);
    assert(status->ngihss_acceptRequest == 0);
    assert(status->ngihss_connectRequest == 0);
    assert(status->ngihss_nodelayRequest == 0);

    assert(status->ngihss_done == 0);

    status->ngihss_socketType = socketType;

    status->ngihss_errorOccurred = 0;
    status->ngihss_errorCode = NG_ERROR_NO_ERROR;

    if (isListenerCreate != 0) {
        handle->ngih_userOpened = 1;

        status->ngihss_listenerCreatePort = 0;
        status->ngihss_listenerCreatePortAllocated = 0;
        status->ngihss_listenerCreatePath = NULL;

        if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
            status->ngihss_listenerCreatePort = listenerPort;
            status->ngihss_listenerCreatePortAllocated = 0;

        } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
            listenerPathName = ngiStrdup(listenerPath, log, error);
            if (listenerPathName == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                    "Duplicate the path failed.\n");
                goto error;
            }

            status->ngihss_listenerCreatePath = listenerPathName;
            listenerPathName = NULL;

        } else {
            abort();
        }
        status->ngihss_listenerCreateBacklog = listenerBacklog;
        status->ngihss_listenerCreateRequest = 1;

    } else if (isAccept != 0) {
        status->ngihss_acceptNewFd = -1;
        status->ngihss_acceptResult = NULL;
        status->ngihss_acceptRequest = 1;

    } else if (isConnect != 0) {
        handle->ngih_userOpened = 1;

        status->ngihss_connectHostName = NULL;
        status->ngihss_connectPort = 0;
        status->ngihss_connectPath = NULL;

        if (socketType == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
            connectHostName = ngiStrdup(connectHost, log, error);
            if (connectHostName == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Duplicate the host failed.\n"); 
                goto error;
            }
            status->ngihss_connectHostName = connectHostName;
            status->ngihss_connectPort = connectPort;
            connectHostName = NULL;

        } else if (socketType == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
            connectPathName = ngiStrdup(connectPath, log, error);
            if (connectPathName == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Duplicate the path failed.\n"); 
                goto error;
            }
            status->ngihss_connectPath = connectPathName;
            connectPathName = NULL;

        } else {
            abort();
        }

        status->ngihss_connectRequest = 1;

    } else if (isNodelay != 0) {
        status->ngihss_nodelayEnable = nodelayEnable;
        status->ngihss_nodelayRequest = 1;

    } else {
        abort();
    }

    status->ngihss_requesting = 1;

    /* Notify to Event Driver. */
    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    /* Wait request done. */
    while(status->ngihss_done == 0) {
        result = nglEventDriverRequestDoneWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait request done failed.\n");
            goto error;
        }
    }

    errorOccurred = status->ngihss_errorOccurred;
    errorCode = status->ngihss_errorCode;

    if (isListenerCreate != 0) {
        gotListenerPort = status->ngihss_listenerCreatePortAllocated;

        listenerPathName = status->ngihss_listenerCreatePath;
        if (listenerPathName != NULL) {
            result = ngiFree(listenerPathName, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Deallocate the path failed.\n"); 
                goto error;
            }
            listenerPathName = NULL;
        }

        status->ngihss_listenerCreateRequest = 0;
        status->ngihss_listenerCreatePort = 0;
        status->ngihss_listenerCreatePortAllocated = 0;
        status->ngihss_listenerCreatePath = NULL;
        status->ngihss_listenerCreateBacklog = 0;

    } else if (isAccept != 0) {
        newAcceptFd = status->ngihss_acceptNewFd;
        newAcceptResult = status->ngihss_acceptResult;
     
        status->ngihss_acceptRequest = 0;
        status->ngihss_acceptNewFd = -1;
        status->ngihss_acceptResult = NULL;

    } else if (isConnect != 0) {
        connectHostName = status->ngihss_connectHostName;
        if (connectHostName != NULL) {
            result = ngiFree(connectHostName, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Deallocate the hostname failed.\n"); 
                goto error;
            }
            connectHostName = NULL;
        }

        connectPathName = status->ngihss_connectPath;
        if (connectPathName != NULL) {
            result = ngiFree(connectPathName, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Deallocate the path failed.\n"); 
                goto error;
            }
            connectPathName = NULL;
        }
     
        status->ngihss_connectRequest = 0;
        status->ngihss_connectHostName = NULL;
        status->ngihss_connectPort = 0;
        status->ngihss_connectPath = NULL;

    } else if (isNodelay != 0) {
        status->ngihss_nodelayRequest = 0;
        status->ngihss_nodelayEnable = 0;

    } else {
        abort();
    }

    status->ngihss_socketType = NGI_IOHANDLE_SOCKET_TYPE_NONE;

    status->ngihss_errorOccurred = 0;
    status->ngihss_errorCode = 0;

    status->ngihss_requesting = 0;
    status->ngihss_started = 0;
    status->ngihss_done = 0;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    if (errorOccurred != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The error was occurred.\n"); 
        NGI_SET_ERROR(error, errorCode);
        goto error;
    }

    if (isListenerCreate != 0) {
        *listenerPortAllocated = gotListenerPort;

    } else if (isAccept != 0) {

        if (acceptResult != NULL) {
            /* Store the result */
            *acceptResult = newAcceptResult; 
        } else {
            result = ngiIOhandleAcceptResultDestruct(
                newAcceptResult, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Destruct the Accept Result failed.\n");
                goto error;
            }
        }
        newAcceptResult = NULL;

        /* Set the new connection fd to connectHandle. */
        handle = connectHandle;
        status = NULL;
     
        /* The connectHandle driver may different than the first one. */
        driver = handle->ngih_eventDriver;
        assert(driver != NULL);
     
        result = nglEventMutexLock(&driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n");
            goto error;
        }
        mutexLocked = 1;

        if (handle->ngih_userOpened != 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is already open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto error;
        }

        handle->ngih_userOpened = 1;
     
        handle->ngih_fd = newAcceptFd;
        handle->ngih_fdEffective = 1;

        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
            goto error;
        }

    } else if (isConnect != 0) {
        assert(isConnect != 0); /* Do nothing */

    } else if (isNodelay != 0) {
        assert(isNodelay != 0); /* Do nothing */

    } else {
        abort();
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if ((mutexLocked != 0) && (driver != NULL)) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle
 *     socket listener callback register.
 * Note: If the callback is not registered,
 *   unregister will do nothing and success.
 */
static int
nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandHandleSocketListenerCallbackRegister";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleSocketStatus_t *status;
    int result, mutexLocked;
    char *modeName;

    mutexLocked = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(mode > NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NOMORE);

    type = NGL_EVENT_DRIVER_HANDLE_OP_SOCKET_CB;
    status = &handle->ngih_socket;

    modeName = NULL;
    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        modeName = "Register";
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        modeName = "Unregister";
    } else {
        abort();
    }

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the listener callback for handle %d.%d.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open. accept register anyway.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
        }
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        if (handle->ngih_userOpened == 0) {
            ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The handle %d.%d is not open.\n",
                handle->ngih_eventDriverID, handle->ngih_id);
            goto success;
        }
    } else {
        abort();
    }

    /* No (handle->ngih_fdEffective == 0) check. */

    /* Wait the requestable state. */
    while (1) {
        if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
            if ((status->ngihss_listenCallbackRegistering == 0) &&
                (status->ngihss_listenCallbackUnregistering == 0) &&
                (status->ngihss_requesting == 0)) {
                break;
            }
        } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
            if ((status->ngihss_listenCallbackRegistering == 0) &&
                (status->ngihss_listenCallbackUnregistering == 0) &&
                (status->ngihss_requesting == 0)) {
                break;
            }
        
        } else {
            abort();
        }

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait the cond failed.\n");
            goto error;
        }
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        assert(callbackFunction != NULL);
        /* callbackArgument may be NULL. */

        status->ngihss_listenCallbackFunction = callbackFunction;
        status->ngihss_listenCallbackArgument = callbackArgument;
        status->ngihss_listenCallbackRegistering = 1;

    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        assert(callbackFunction == NULL);
        assert(callbackArgument == NULL);

        status->ngihss_listenCallbackUnregisterDone = 0;
        status->ngihss_listenCallbackUnregistering = 1;
    } else {
        abort();
    }

    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {

        /* Wait Unregistered. */
        while ((status->ngihss_listenCallbackUnregisterDone == 0) ||
            (status->ngihss_listenCallbackRegistered != 0)) {

            result = nglEventDriverRequestDoneWait(driver, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                    "Wait request done failed.\n");
                goto error;
            }
        }
        assert(status->ngihss_listenCallbackUnregistering != 0);
        assert(status->ngihss_listenCallbackUnregisterDone != 0);

        assert(status->ngihss_listenCallbackRegistering == 0);
        assert(status->ngihss_listenCallbackRegistered == 0);
        assert(status->ngihss_listenCallbackFunction == NULL);
        assert(status->ngihss_listenCallbackArgument == NULL);

        status->ngihss_listenCallbackUnregisterDone = 0;
        status->ngihss_listenCallbackUnregistering = 0;
    }

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

success:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the listener callback for handle %d.%d return.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the listener callback for handle %d.%d return by error.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle file.
 */
static int
nglEventDriverRequestCommandHandleFile(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int isFileOpen,
    char *path,
    ngiIOhandleFileOpenType_t fileOpenType,
    int isFdOpen,
    int fd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandHandleFile";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleFileStatus_t *status;
    int errorOccurred, errorCode;
    int result, mutexLocked;
    char *fileOpenPathName;

    status = NULL;
    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    fileOpenPathName = NULL;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    assert((isFileOpen != 0) || (isFdOpen != 0));

    if (isFileOpen != 0) {
        assert(path != NULL);
        assert(fileOpenType > NGI_IOHANDLE_FILE_OPEN_TYPE_NONE);
        assert(fileOpenType < NGI_IOHANDLE_FILE_OPEN_TYPE_NOMORE);

    } else if (isFdOpen != 0) {
        assert(fd >= 0);

    } else {
        abort();
    }

    if (isFileOpen == 0) {
        assert(path == NULL);
        assert(fileOpenType == NGI_IOHANDLE_FILE_OPEN_TYPE_NONE);
    }
    if (isFdOpen == 0) {
        assert(fd == -1);
    }

    type = NGL_EVENT_DRIVER_HANDLE_OP_FILE;
    status = &handle->ngih_file;

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (handle->ngih_userOpened != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is already open.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (driver->nged_isPthread != 0) {
        while(status->ngihfs_requesting != 0) {
            result = nglEventCondWait(
                &driver->nged_cond, &driver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Wait the cond failed.\n"); 
                goto error;
            }
        }
    }

    assert(status->ngihfs_requesting == 0);
    assert(status->ngihfs_done == 0);

    assert(status->ngihfs_fileOpenRequest == 0);
    assert(status->ngihfs_fdOpenRequest == 0);

    status->ngihfs_errorOccurred = 0;
    status->ngihfs_errorCode = NG_ERROR_NO_ERROR;

    if (isFileOpen != 0) {
        handle->ngih_userOpened = 1;

        fileOpenPathName = ngiStrdup(path, log, error);
        if (fileOpenPathName == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Duplicate the path failed.\n");
            goto error;
        }

        status->ngihfs_fileOpenPath = fileOpenPathName;
        status->ngihfs_fileOpenType = fileOpenType;
        fileOpenPathName = NULL;

        status->ngihfs_fileOpenRequest = 1;

    } else if (isFdOpen != 0) {
        handle->ngih_userOpened = 1;

        status->ngihfs_fdOpenFd = fd;
        status->ngihfs_fdOpenRequest = 1;

    } else {
        abort();
    }

    status->ngihfs_requesting = 1;

    /* Notify to Event Driver. */
    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    /* Wait request done. */
    while(status->ngihfs_done == 0) {
        result = nglEventDriverRequestDoneWait(driver, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait request done failed.\n");
            goto error;
        }
    }

    errorOccurred = status->ngihfs_errorOccurred;
    errorCode = status->ngihfs_errorCode;

    if (isFileOpen != 0) {
        fileOpenPathName = status->ngihfs_fileOpenPath;
        if (fileOpenPathName != NULL) {
            result = ngiFree(fileOpenPathName, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Deallocate the path failed.\n"); 
                goto error;
            }
            fileOpenPathName = NULL;
        }

        status->ngihfs_fileOpenRequest = 0;
        status->ngihfs_fileOpenPath = NULL;
        status->ngihfs_fileOpenType = NGI_IOHANDLE_FILE_OPEN_TYPE_NONE;

    } else if (isFdOpen != 0) {
        status->ngihfs_fdOpenRequest = 0;
        status->ngihfs_fdOpenFd = -1;

    } else {
        abort();
    }

    status->ngihfs_errorOccurred = 0;
    status->ngihfs_errorCode = 0;

    status->ngihfs_requesting = 0;
    status->ngihfs_started = 0;
    status->ngihfs_done = 0;

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    if (errorOccurred != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The error was occurred.\n");
        NGI_SET_ERROR(error, errorCode);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle time.
 */
static int
nglEventDriverRequestCommandHandleTime(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    int isTimeChangeRequest,
    int isTimeSet,
    time_t eventTime,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandHandleTime";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleTimeStatus_t *status;
    int errorOccurred, errorCode;
    int result, mutexLocked;

    status = NULL;
    mutexLocked = 0;
    errorOccurred = 0;
    errorCode = NG_ERROR_NO_ERROR;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);

    assert((isTimeChangeRequest != 0) || (isTimeSet != 0));

    if (isTimeSet == 0) {
        assert(eventTime == 0);
    }

    type = NGL_EVENT_DRIVER_HANDLE_OP_NONE;
    status = &handle->ngih_time;

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    /* Wait the requestable state. */
    while ((status->ngihts_callbackRegistering != 0) ||
        (status->ngihts_callbackUnregistering != 0)) {

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Wait the cond failed.\n");
            goto error;
        }
    }

    if (status->ngihts_callbackRegistered == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The time event callback for handle %d.%d is not registered.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    if (isTimeChangeRequest != 0) {
        type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE;
        status->ngihts_changeRequested = 1;

    } else if (isTimeSet != 0) {
        status->ngihts_eventTime = eventTime;

    } else {
        abort();
    }

    if (isTimeChangeRequest != 0) {
        /* Notify to Event Driver. */
        result = nglEventDriverCommandNotify(
            driver, handle, type, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The command to Event Driver failed.\n"); 
            goto error;
        }
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: Request command for handle time callback register.
 */
static int
nglEventDriverRequestCommandHandleTimeCallbackRegister(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    nglEventDriverCallbackRegisterMode_t mode,
    ngiIOhandleCallbackFunction_t eventCallbackFunction,
    void *eventCallbackArgument,
    ngiIOhandleCallbackFunction_t changeCallbackFunction,
    void *changeCallbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandHandleTimeCallbackRegister";
    nglEventDriverHandleOperationType_t type;
    ngiIOhandleTimeStatus_t *status;
    int result, mutexLocked;
    char *modeName;

    mutexLocked = 0;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(mode > NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NONE);
    assert(mode < NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_NOMORE);

    type = NGL_EVENT_DRIVER_HANDLE_OP_TIME_CB;
    status = &handle->ngih_time;

    modeName = NULL;
    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        modeName = "Register";
    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        modeName = "Unregister";
    } else {
        abort();
    }

    result = nglEventMutexLock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName,
        "%s the time event callback for handle %d.%d.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (handle->ngih_valid == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The handle %d.%d is invalid.\n",
            handle->ngih_eventDriverID, handle->ngih_id);
        goto error;
    }

    /* Wait the requestable state. */
    while (1) {
        if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
            if ((status->ngihts_callbackRegistering == 0) &&
                (status->ngihts_callbackUnregistering == 0)) {
                break;
            }
        } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
            /* Unregister must be wait the callback function return. */
            if ((status->ngihts_timeCallbackExecuting == 0) &&
                (status->ngihts_changeCallbackExecuting == 0) &&
                (status->ngihts_callbackRegistering == 0) &&
                (status->ngihts_callbackUnregistering == 0)) {
                break;
            }
        } else {
            abort();
        }

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER) {
        assert(eventCallbackFunction != NULL);
        assert(changeCallbackFunction != NULL);
        /* callbackArgument may be NULL. */

        status->ngihts_timeCallbackFunction = eventCallbackFunction;
        status->ngihts_timeCallbackArgument = eventCallbackArgument;
        status->ngihts_changeCallbackFunction = changeCallbackFunction;
        status->ngihts_changeCallbackArgument = changeCallbackArgument;

        status->ngihts_callbackRegistering = 1;

    } else if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {
        assert(eventCallbackFunction == NULL);
        assert(eventCallbackArgument == NULL);
        assert(changeCallbackFunction == NULL);
        assert(changeCallbackArgument == NULL);

        status->ngihts_callbackUnregisterDone = 0;
        status->ngihts_callbackUnregistering = 1;
    } else {
        abort();
    }

    result = nglEventDriverCommandNotify(
        driver, handle, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The command to Event Driver failed.\n"); 
        goto error;
    }

    if (mode == NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER) {

        /* Wait Unregistered. */
        while ((status->ngihts_callbackUnregisterDone == 0) ||
            (status->ngihts_callbackRegistered != 0) ||
            (status->ngihts_timeCallbackExecuting != 0) ||
            (status->ngihts_changeCallbackExecuting != 0)){

            result = nglEventDriverRequestDoneWait(driver, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                    "Wait request done failed.\n");
                goto error;
            }
        }
        assert(status->ngihts_callbackUnregistering != 0);
        assert(status->ngihts_callbackUnregisterDone != 0);

        assert(status->ngihts_callbackRegistering == 0);
        assert(status->ngihts_callbackRegistered == 0);
        assert(status->ngihts_timeCallbackFunction == NULL);
        assert(status->ngihts_timeCallbackArgument == NULL);
        assert(status->ngihts_timeCallbackExecuting == 0);
        assert(status->ngihts_changeCallbackFunction == NULL);
        assert(status->ngihts_changeCallbackArgument == NULL);
        assert(status->ngihts_changeCallbackExecuting == 0);

        status->ngihts_callbackUnregisterDone = 0;
        status->ngihts_callbackUnregistering = 0;
    }

    result = nglEventCondBroadcast(&driver->nged_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the time event callback for handle %d.%d return.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    result = nglEventMutexUnlock(&driver->nged_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "%s the time event callback for handle %d.%d return by error.\n",
        modeName, handle->ngih_eventDriverID, handle->ngih_id);

    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&driver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Event Driver: NonThread Cond Wait and Cond Timed Wait.
 */
static int
nglEventDriverRequestCommandNonThreadCondWait(
    ngiEventDriver_t *driver,
    int *waitFlag,
    ngiEventNonThreadCondType_t type,
    int timeoutSec,
    int *wasTimeout,
    ngLog_t *log,
    int *error)
{
    static const char fName[] =
        "nglEventDriverRequestCommandNonThreadCondWait";
    struct timeval timeNow, timeWait, timeStart, timeEnd;
    nglEventDriverReturnMode_t mode;
    int result, useTimeout;
    int timeWaitMilli;

    /* Check the arguments */
    assert(driver != NULL);
    assert(waitFlag != NULL);
    assert(type > NGI_EVENT_NON_THREAD_COND_NONE);
    assert(type < NGI_EVENT_NON_THREAD_COND_NOMORE);

    mode = NGL_EVENT_DRIVER_RETURN_MODE_NONE;
    useTimeout = 0;
    timeWaitMilli = 0;

    if (type == NGI_EVENT_NON_THREAD_COND_NO_TIMEOUT) {
        mode = NGL_EVENT_DRIVER_RETURN_MODE_BLOCK;

    } else if (type == NGI_EVENT_NON_THREAD_COND_WITH_TIMEOUT) {
        mode = NGL_EVENT_DRIVER_RETURN_MODE_TIMEOUT;
        useTimeout = 1;

    } else {
        abort();
    }

    /* Setup timeout */
    if (useTimeout != 0) {
        assert(wasTimeout != NULL);

        *wasTimeout = 0;

        result = ngiGetTimeOfDay(&timeStart, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Get current time failed.\n");
            goto error;
        }

        timeWait.tv_sec = timeoutSec;
        timeWait.tv_usec = 0;
        timeWaitMilli = timeoutSec * 1000;

        timeEnd = ngiTimevalAdd(timeStart, timeWait);
    }

    /* Wait until done. */
    while (*waitFlag == 0) {
        result = nglEventDriverLoop(
            driver, mode, timeWaitMilli, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Event loop failed.\n"); 
            goto error;
        }

        if (useTimeout != 0) {
            result = ngiGetTimeOfDay(&timeNow, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Get current time failed.\n");
                goto error;
            }

            if (ngiTimevalCompare(timeNow, timeEnd) > 0) {
                break;
            }

            timeWait = ngiTimevalSub(timeEnd, timeNow);
            timeWaitMilli = (timeWait.tv_sec * 1000) +
                (timeWait.tv_usec / 1000);
        }
    }

    /* Check the timeout. */
    if (useTimeout != 0) {
        result = ngiGetTimeOfDay(&timeNow, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Get current time failed.\n");
            goto error;
        }

        if (ngiTimevalCompare(timeNow, timeEnd) > 0) {
            *wasTimeout = 1;
        }
    }

    assert(((useTimeout != 0) && (*wasTimeout != 0)) ||
        (*waitFlag != 0));

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Driver: NonThread Yield.
 */
static int
nglEventDriverRequestCommandNonThreadYield(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestCommandNonThreadYield";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    /* Just execute Event Loop one time. */
    result = nglEventDriverLoop(
        driver, NGL_EVENT_DRIVER_RETURN_MODE_NONBLOCK,
        0, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event loop failed.\n"); 
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
 * Event Driver: Wait the Request done.
 */
static int
nglEventDriverRequestDoneWait(
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventDriverRequestDoneWait";
    int result;

    /* Check the arguments */
    assert(driver != NULL);

    if (driver->nged_isPthread != 0) {

        result = nglEventCondWait(
            &driver->nged_cond, &driver->nged_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }

    } else {

        result = nglEventDriverLoop(
            driver, NGL_EVENT_DRIVER_RETURN_MODE_BLOCK, 0,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Event loop failed.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Callback Manager: Construct.
 */
static ngiEventCallbackManager_t *
nglEventCallbackManagerConstruct(
    int initialCallbacks,
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerConstruct";
    int result, allocated, initialized;
    ngiEventCallbackManager_t *manager;

    manager = NULL;
    allocated = 0;
    initialized = 0;

    /* Check the arguments */
    assert(driver != NULL);

    /* Allocate */
    manager = NGI_ALLOCATE(ngiEventCallbackManager_t, log, error);
    if (manager == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Allocate the Event Callback Manager failed.\n"); 
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglEventCallbackManagerInitialize(
        manager, initialCallbacks, driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the Event Callback Manager failed.\n"); 
        goto error;
    }
    initialized = 1;

    /* Success */
    return manager;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * Event Callback Manager: Destruct.
 */
static int
nglEventCallbackManagerDestruct(
    ngiEventCallbackManager_t *manager,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerDestruct";
    int result;

    /* Check the arguments */
    assert(manager != NULL);

    /* Finalize */
    result = nglEventCallbackManagerFinalize(manager, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Finalize the Event Callback Manager failed.\n"); 
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiEventCallbackManager_t, manager, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Deallocate the Event Callback Manager failed.\n"); 
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
 * Event Callback Manager: Initialize.
 */
static int
nglEventCallbackManagerInitialize(
    ngiEventCallbackManager_t *manager,
    int initialCallbacks,
    ngiEventDriver_t *driver,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglEventCallbackManagerInitialize";

    /* Check the arguments */
    assert(manager != NULL);
    assert(initialCallbacks >= 0);
    assert(driver != NULL);

    nglEventCallbackManagerInitializeMember(manager);

    manager->ngecm_eventDriver = driver;
    manager->ngecm_log = driver->nged_log;

    manager->ngecm_nWaiting = 0;
    manager->ngecm_nWorking = 0;

    manager->ngecm_nCallbacks = 0;
    manager->ngecm_callback_head = NULL;
    manager->ngecm_callbackIDmax = 0;

    result = nglEventMutexInitialize(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the mutex failed.\n");
        goto error;
    }

    result = nglEventCondInitialize(&manager->ngecm_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the cond failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;
error:

    return 0;
}

/**
 * Event Callback Manager: Finalize.
 */
static int
nglEventCallbackManagerFinalize(
    ngiEventCallbackManager_t *manager,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerFinalize";
    ngiEventCallback_t *callback, *nextCallback;
    int result;

    callback = manager->ngecm_callback_head;
    while (callback != NULL) {
        nextCallback = callback->ngec_next;

        result = nglEventCallbackManagerCallbackUnregister(
            manager, callback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unregister the Event Callback failed.\n"); 
            goto error;
        }

        result = nglEventCallbackDestruct(callback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Destruct the Event Callback failed.\n"); 
            goto error;
        }

        callback = nextCallback;
    }

    result = nglEventMutexDestroy(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the mutex failed.\n"); 
        goto error;
    }

    result = nglEventCondDestroy(&manager->ngecm_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the cond failed.\n"); 
        goto error;
    }

    nglEventCallbackManagerInitializeMember(manager);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Callback Manager: Initialize the members.
 */
static void
nglEventCallbackManagerInitializeMember(
    ngiEventCallbackManager_t *manager)
{
    /* Check the arguments */
    assert(manager != NULL);

    manager->ngecm_eventDriver = NULL;

    manager->ngecm_log = NULL;

    manager->ngecm_nCallbacks = 0;
    manager->ngecm_callback_head = NULL;
    manager->ngecm_callbackIDmax = 0;

    manager->ngecm_requesting = 0;
    manager->ngecm_callbackType = NGI_EVENT_CALLBACK_TYPE_NONE;
    manager->ngecm_callbackFunction = NULL;
    manager->ngecm_callbackArgument = NULL;
    manager->ngecm_callbackHandle = NULL;
    manager->ngecm_callbackState = NGI_IOHANDLE_STATE_NONE;
    manager->ngecm_callbackExecuting = NULL;

    manager->ngecm_nWaiting = 0;
    manager->ngecm_nWorking = 0;
}

/**
 * Event Callback Manager: Register Callback.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static int
nglEventCallbackManagerCallbackRegister(
    ngiEventCallbackManager_t *manager,
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerCallbackRegister";
    ngiEventCallback_t **tail;
    int count;

    /* Check the arguments */
    assert(manager != NULL);
    assert(callback != NULL);
    assert(callback->ngec_next == NULL);

    count = 0;

    /* Find the tail. */
    tail = &manager->ngecm_callback_head;
    while (*tail != NULL) {
        if (*tail == callback) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "The Event Callback is already registered.\n"); 
            goto error;
        }

        tail = &(*tail)->ngec_next;
        count++;
    }

    *tail = callback;
    count++;

    manager->ngecm_nCallbacks = count;
    
    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Callback Manager: Unregister Callback.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static int
nglEventCallbackManagerCallbackUnregister(
    ngiEventCallbackManager_t *manager,
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerCallbackUnregister";
    ngiEventCallback_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(manager != NULL);
    assert(callback != NULL);

    /* Count the number of Callbacks. */
    count = 0;
    cur = manager->ngecm_callback_head;
    while (cur != NULL) {
        count++;
        cur = cur->ngec_next;
    }
    manager->ngecm_nCallbacks = count;

    /* Delete the data from the list. */
    prevPtr = &manager->ngecm_callback_head;
    cur = manager->ngecm_callback_head;
    for (; cur != NULL; cur = cur->ngec_next) {
        if (cur == callback) {
            /* Unlink the list */
            *prevPtr = cur->ngec_next;
            callback->ngec_next = NULL;
            count--;
            manager->ngecm_nCallbacks = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->ngec_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The Event Callback is not registered.\n"); 
    return 0;
}

/**
 * Event Callback Manager: Require Callback Thread.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static int
nglEventCallbackManagerCallbackRequire(
    ngiEventCallbackManager_t *manager,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackManagerCallbackRequire";
    ngiEventCallback_t *callback;
    int result;

    /* Check the arguments */
    assert(manager != NULL);

    /**
     * Note: Currently, the number of callback managing is
     * very simple.
     */

    /* Is free callback driver available? */
    if (manager->ngecm_nWaiting > 0) {

        /* Success */
        return 1;
    }

    /* Construct */
    callback = nglEventCallbackConstruct(
        manager, log, error);
    if (callback == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Construct the Event Callback failed.\n"); 
        goto error;
    }

    /* Register */
    result = nglEventCallbackManagerCallbackRegister(
        manager, callback, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the Event Callback failed.\n"); 
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
 * Event Callback: Construct.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static ngiEventCallback_t *
nglEventCallbackConstruct(
    ngiEventCallbackManager_t *manager,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackConstruct";
    int result, allocated, initialized;
    ngiEventCallback_t *callback;

    callback = NULL;
    allocated = 0;
    initialized = 0;

    /* Check the arguments */
    assert(manager != NULL);

    /* Allocate */
    callback = NGI_ALLOCATE(ngiEventCallback_t, log, error);
    if (callback == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Allocate the Event Callback failed.\n"); 
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglEventCallbackInitialize(
        callback, manager, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the Event Callback failed.\n"); 
        goto error;
    }
    initialized = 1;

    /* Success */
    return callback;

    /* Error occurred */
error:
    /* Failed */
    return NULL;
}

/**
 * Event Callback: Destruct.
 */
static int
nglEventCallbackDestruct(
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackDestruct";
    int result;

    /* Check the arguments */
    assert(callback != NULL);

    /* Finalize */
    result = nglEventCallbackFinalize(callback, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Finalize the Event Callback failed.\n"); 
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiEventCallback_t, callback, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Deallocate the Event Callback failed.\n"); 
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
 * Event Callback: Initialize.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static int
nglEventCallbackInitialize(
    ngiEventCallback_t *callback,
    ngiEventCallbackManager_t *manager,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackInitialize";
    int result;

    /* Check the arguments */
    assert(callback != NULL);
    assert(manager != NULL);

    nglEventCallbackInitializeMember(callback);

    callback->ngec_manager = manager;
    callback->ngec_next = NULL;

    manager->ngecm_callbackIDmax++;
    callback->ngec_id = manager->ngecm_callbackIDmax;

    callback->ngec_userCallbackWorking = 0;

    /* Start the Callback Thread. */
    result = nglEventCallbackThreadStart(callback, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event Callback Thread start failed.\n"); 
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
 * Event Callback: Finalize.
 */
static int
nglEventCallbackFinalize(
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackFinalize";
    int result;

    /* Stop the Callback Thread. */
    result = nglEventCallbackThreadStop(callback, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Event Callback Thread stop failed.\n"); 
        goto error;
    }

    nglEventCallbackInitializeMember(callback);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Event Callback: Initialize the members.
 */
static void
nglEventCallbackInitializeMember(
    ngiEventCallback_t *callback)
{
    /* Check the arguments */
    assert(callback != NULL);

    callback->ngec_manager = NULL;
    callback->ngec_next = NULL;

    callback->ngec_id = 0;

    callback->ngec_continue = 0;
    callback->ngec_stopped = 0;

    callback->ngec_userCallbackWorking = 0;
}

/**
 * Event Callback: Thread start.
 * Note:
 * Lock the Event Callback Manager before using this function,
 * and unlock the Event Callback Manager after use.
 */
static int
nglEventCallbackThreadStart(
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackThreadStart";
    int result;

    /* Check the arguments */
    assert(callback != NULL);

    callback->ngec_continue = 1;
    callback->ngec_stopped = 0;

    result = nglEventThreadCreate(
        &callback->ngec_thread, nglEventCallbackThread, (void *)callback,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Create the thread failed.\n");
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
 * Event Callback: Thread stop.
 */
static int
nglEventCallbackThreadStop(
    ngiEventCallback_t *callback,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackThreadStop";
    ngiEventCallbackManager_t *manager;
    int result, mutexLocked;

    /* Check the arguments */
    assert(callback != NULL);

    manager = callback->ngec_manager;
    mutexLocked = 0;

    if (callback->ngec_stopped != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Callback thread already stopped.\n"); 

        /* Success */
        return 1;
    }

    /**
     * Tell the Callback thread to stop.
     */
    result = nglEventMutexLock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    callback->ngec_continue = 0; /* to stop */

    result = nglEventCondBroadcast(&manager->ngecm_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /**
     * Wait the Callback driver to stop.
     */

    result = nglEventThreadJoin(
        &callback->ngec_thread, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Join the thread failed.\n");
        goto error;
    }

    /* Success */
    return 1;

error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&manager->ngecm_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    return 0;
}

/**
 * Event Callback: Thread.
 */
static void *
nglEventCallbackThread(
    void *arg)
{
    static const char fName[] = "nglEventCallbackThread";
    ngiIOhandleCallbackFunction_t callbackFunction;
    ngiEventCallbackType_t callbackType;
    ngiEventCallbackManager_t *manager;
    ngiEventDriver_t *callbackDriver;
    ngiIOhandleState_t callbackState;
    ngiIOhandle_t *callbackHandle;
    ngiEventCallback_t *callback;
    int result, workDone, cont, *callbackExecuting;
    int mutexLocked, driverLocked;
    int *error, errorEntity;
    void *callbackArgument;
    ngLog_t *log;

    /* Check the arguments */
    assert(arg != NULL);

    callback = (ngiEventCallback_t *)arg;
    manager = callback->ngec_manager;
    log = manager->ngecm_log;
    errorEntity = NG_ERROR_NO_ERROR;
    error = &errorEntity;
    mutexLocked = 0;
    driverLocked = 0;

    workDone = 0;

    callbackDriver = NULL;
    callbackType = NGI_EVENT_CALLBACK_TYPE_NONE;
    callbackFunction = NULL;
    callbackArgument = NULL;
    callbackHandle = NULL;
    callbackState = NGI_IOHANDLE_STATE_NONE;
    callbackExecuting = NULL;
    callback->ngec_userCallbackWorking = 0;

    assert(callback->ngec_stopped == 0);

    result = nglEventMutexLock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    manager->ngecm_nWaiting++;

    mutexLocked = 0;
    result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    cont = 1;
    while (cont != 0) {
        result = nglEventMutexLock(&manager->ngecm_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n");
            goto error;
        }
        mutexLocked = 1;

        if (workDone != 0) {
            manager->ngecm_nWorking--;
            manager->ngecm_nWaiting++;
            callback->ngec_userCallbackWorking = 0;
            workDone = 0;
        }

        while ((manager->ngecm_requesting == 0) &&
            (callback->ngec_continue != 0)) {
            result = nglEventCondWait(
                &manager->ngecm_cond, &manager->ngecm_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Wait the cond failed.\n"); 
                goto error;
            }
        }

        if (callback->ngec_continue == 0) {
            cont = 0;
            mutexLocked = 0;
            result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Unlock the mutex failed.\n"); 
                goto error;
            }
            break;
        }

        callbackType = manager->ngecm_callbackType;
        callbackFunction = manager->ngecm_callbackFunction;
        callbackArgument = manager->ngecm_callbackArgument;
        callbackHandle = manager->ngecm_callbackHandle;
        callbackState = manager->ngecm_callbackState;
        callbackExecuting = manager->ngecm_callbackExecuting;
        callbackDriver = callbackHandle->ngih_eventDriver;

        manager->ngecm_requesting = 0;
        manager->ngecm_callbackType = NGI_EVENT_CALLBACK_TYPE_NONE;
        manager->ngecm_callbackFunction = NULL;
        manager->ngecm_callbackArgument = NULL;
        manager->ngecm_callbackHandle = NULL;
        manager->ngecm_callbackState = NGI_IOHANDLE_STATE_NONE;
        manager->ngecm_callbackExecuting = NULL;

        manager->ngecm_nWorking++;
        manager->ngecm_nWaiting--;
        callback->ngec_userCallbackWorking = 1;

        result = nglEventCondBroadcast(&manager->ngecm_cond, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Broadcast the cond failed.\n"); 
            goto error;
        }

        mutexLocked = 0;
        result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
            goto error;
        }
         
        result = nglEventDriverCallbackRun(
            callbackDriver, callbackHandle, callbackType,
            callbackFunction, callbackArgument, callbackState,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Run the callback failed.\n"); 
            goto error;
        }
                
        if (callbackExecuting != NULL) {

            /* Lock the driver mutex. */
            result = nglEventMutexLock(
                &callbackDriver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Lock the mutex failed.\n"); 
                goto error;
            }
            driverLocked = 1;
         
            if ((callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_EVENT) &&
                (callbackState == NGI_IOHANDLE_STATE_NORMAL)) {
                callbackHandle->ngih_time.ngihts_changeRequested = 1;
            }
         
            *callbackExecuting = 0;
         
            /* Signal the driver cond. */
            result = nglEventCondBroadcast(
                &callbackDriver->nged_cond, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Broadcast the cond failed.\n"); 
                goto error;
            }
         
            /**
             * Notify command to Event Driver and
             * invoke next command process for time change callback.
             */
            if (((callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_EVENT) ||
                (callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_CHANGE)) &&
                (callbackState == NGI_IOHANDLE_STATE_NORMAL)) {
         
                result = nglEventDriverCommandNotify(
                    callbackDriver, callbackHandle,
                    NGL_EVENT_DRIVER_HANDLE_OP_TIME_CHANGE,
                    log, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                        "The command to Event Driver failed.\n"); 
                    goto error;
                }
            }
         
            /* Unlock the driver mutex. */
            driverLocked = 0;
            result = nglEventMutexUnlock(
                &callbackDriver->nged_mutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                    "Unlock the mutex failed.\n"); 
                goto error;
            }
        }

        workDone = 1;
    }

    /**
     * Tell the Main Thread that, Callback Thread was stopped.
     */

    result = nglEventMutexLock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    callback->ngec_stopped = 1;
    result = nglEventCondBroadcast(&manager->ngecm_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    return NULL;

error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&manager->ngecm_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    if ((driverLocked != 0) && (callbackDriver != NULL)) {
        driverLocked = 0;
        result = nglEventMutexUnlock(
            &callbackDriver->nged_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    return NULL;
}

/**
 * Event Callback: Run request.
 */
static int
nglEventCallbackRunRequest(
    ngiEventDriver_t *driver,
    ngiIOhandle_t *handle,
    ngiEventCallbackType_t callbackType,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngiIOhandleState_t state,
    int *callbackExecuting,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglEventCallbackRunRequest";
    ngiEventCallback_t *callback, **tailCallback;
    ngiEventCallbackManager_t *manager;
    char *callbackName, *stateName;
    int result, mutexLocked;

    /* Check the arguments */
    assert(driver != NULL);
    assert(handle != NULL);
    assert(callbackType > NGI_EVENT_CALLBACK_TYPE_NONE);
    assert(callbackType < NGI_EVENT_CALLBACK_TYPE_NOMORE);
    assert(callbackFunction != NULL);
    assert(state > NGI_IOHANDLE_STATE_NONE);
    assert(state < NGI_IOHANDLE_STATE_NOMORE);

    manager = driver->nged_callbackManager;
    callback = NULL;
    tailCallback = NULL;
    callbackName = NULL;
    stateName = NULL;
    mutexLocked = 0;

    if (callbackType == NGI_EVENT_CALLBACK_TYPE_READ) {
        callbackName = "read";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_LISTENER) {
        callbackName = "socket listener";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_EVENT) {
        callbackName = "time event";

    } else if (callbackType == NGI_EVENT_CALLBACK_TYPE_TIME_CHANGE) {
        callbackName = "time change";

    } else {
        abort();
    }

    if (state == NGI_IOHANDLE_STATE_NORMAL) {
        stateName = "normal";    
    } else if (state == NGI_IOHANDLE_STATE_CLOSED) {
        stateName = "closed";    
    } else if (state == NGI_IOHANDLE_STATE_CANCELED) {
        stateName = "canceled";    
    }

    /* Log from Calling thread */
    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "The %s callback(state=%s) for handle %d.%d is calling"
        " by callback thread.\n",
        callbackName, stateName,
        handle->ngih_eventDriverID, handle->ngih_id);

    result = nglEventMutexLock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    result = nglEventCallbackManagerCallbackRequire(
        manager, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The Event Callback was not prepared.\n"); 
        goto error;
    }

    while (manager->ngecm_requesting != 0) {
        result = nglEventCondWait(
            &manager->ngecm_cond, &manager->ngecm_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n"); 
            goto error;
        }
    }

    assert(manager->ngecm_nWaiting >= 0);
    assert(manager->ngecm_nWorking <= manager->ngecm_nCallbacks);

    manager->ngecm_requesting = 1;
    manager->ngecm_callbackType = callbackType;
    manager->ngecm_callbackFunction = callbackFunction;
    manager->ngecm_callbackArgument = callbackArgument;
    manager->ngecm_callbackHandle = handle;
    manager->ngecm_callbackState = state;
    manager->ngecm_callbackExecuting = callbackExecuting;

    result = nglEventCondBroadcast(&manager->ngecm_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n"); 
        goto error;
    }

    mutexLocked = 0;
    result = nglEventMutexUnlock(&manager->ngecm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unlock the mutex failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglEventMutexUnlock(&manager->ngecm_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Unlock the mutex failed.\n"); 
        }
    }

    /* Failed */
    return 0;
}


/**
 * Event: NonThread Cond Wait and Cond Timed Wait.
 */
int
ngiEventNonThreadCondTimedWait(
    ngiEvent_t *event,
    int *waitFlag,
    ngiEventNonThreadCondType_t type,
    int timeoutSec,
    int *wasTimeout,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiEventNonThreadCondTimedWait";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The event is NULL.\n"); 
        goto error;
    }

    if (waitFlag == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The wait flag is NULL.\n"); 
        goto error;
    }

    if (!((type > NGI_EVENT_NON_THREAD_COND_NONE) &&
        (type < NGI_EVENT_NON_THREAD_COND_NOMORE))) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The cond wait type is NULL.\n"); 
        goto error;
    }

    if ((type == NGI_EVENT_NON_THREAD_COND_WITH_TIMEOUT) &&
        (wasTimeout == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The timeout result is NULL.\n"); 
        goto error;
    }

    if (event->ngev_isPthread != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "CondWait by Ninf-G Event is not required on Pthread version.\n"); 
        goto error;
    }

    driver = event->ngev_eventDriver_head;
    
    result = nglEventDriverRequestCommandNonThreadCondWait(
        driver, waitFlag, type, timeoutSec, wasTimeout, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Wait the cond by ngEvent failed.\n");
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
 * Event: NonThread Yield.
 */
int
ngiEventNonThreadYield(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiEventNonThreadYield";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The event is NULL.\n"); 
        goto error;
    }

    driver = event->ngev_eventDriver_head;

    result = nglEventDriverRequestCommandNonThreadYield(
        driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Thread yield failed.\n");
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
 * I/O handle: Construct.
 */
ngiIOhandle_t *
ngiIOhandleConstruct(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleConstruct";
    int result, allocated, initialized;
    ngiIOhandle_t *handle;

    handle = NULL;
    allocated = 0;
    initialized = 0;

    /* Check the arguments */
    if (event == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The event is NULL.\n"); 
        goto error;
    }

    /* Allocate */
    handle = NGI_ALLOCATE(ngiIOhandle_t, log, error);
    if (handle == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Allocate the I/O handle failed.\n"); 
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglIOhandleInitialize(handle, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the I/O handle failed.\n"); 
        goto error;
    }
    initialized = 1;

    /* Success */
    return handle;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * I/O handle: Destruct.
 */
int
ngiIOhandleDestruct(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleDestruct";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    /* Finalize */
    result = nglIOhandleFinalize(handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Finalize the I/O handle failed.\n"); 
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiIOhandle_t, handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Deallocate the I/O handle failed.\n"); 
        goto error;
    }

    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * I/O handle: Initialize.
 */
static int
nglIOhandleInitialize(
    ngiIOhandle_t *handle,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglIOhandleInitialize";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    assert(handle != NULL);
    assert(event != NULL);

    /* Get the Event Driver. */
    result = nglEventEventDriverRequire(
        event, &driver, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Prepare the Event Driver for I/O handle failed.\n"); 
        goto error;
    }

    assert(driver != NULL);

    handle->ngih_next = NULL;

    handle->ngih_eventDriver = driver;
    handle->ngih_log = event->ngev_log;
    handle->ngih_valid = 1;

    handle->ngih_id = 0;

    result = nglEventDriverRequestCommandHandleNumber(
        driver, handle,
        NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_APPEND,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Append the I/O handle failed.\n"); 
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Construct the handle %d.%d.\n",
        handle->ngih_eventDriverID, handle->ngih_id);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * I/O handle: Finalize.
 */
static int
nglIOhandleFinalize(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglIOhandleFinalize";
    ngiEventDriver_t *driver;
    int result, driverID, id;

    /* Check the arguments */
    assert(handle != NULL);

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);
    
    driverID = handle->ngih_eventDriverID;
    id = handle->ngih_id;

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Destruct the handle %d.%d start.\n",
        driverID, id);

    /* Unregister the Read callback. */
    result = nglEventDriverRequestCommandHandleReadCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the read callback failed.\n"); 
        goto error;
    }

    /* Unregister the Socket Listener callback. */
    result =
        nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the TCP listener callback failed.\n");
        goto error;
    }

    /* Close the handle. */
    result = nglEventDriverRequestCommandHandleClose(
        driver, handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The close operation failed.\n"); 
        goto error;
    }

    /* Remove the handle. */
    result = nglEventDriverRequestCommandHandleNumber(
        driver, handle,
        NGL_EVENT_DRIVER_HANDLE_NUMBER_MODE_REMOVE,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Remove the I/O handle failed.\n"); 
        goto error;
    }

    handle->ngih_valid = 0;

    nglIOhandleInitializeMember(handle);

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Destruct the handle %d.%d end.\n",
        driverID, id);

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngLogDebug(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "Destruct the handle %d.%d end by error.\n",
        driverID, id);

    /* Failed */
    return 0;
}
 
/**
 * I/O handle: Initialize the members.
 */
static void
nglIOhandleInitializeMember(
    ngiIOhandle_t *handle)
{
    /* Check the arguments */
    assert(handle != NULL);

    handle->ngih_next = NULL;
    handle->ngih_eventDriver = NULL;
    handle->ngih_log = NULL;

    handle->ngih_eventDriverID = 0;
    handle->ngih_id = 0;
    handle->ngih_valid = 0;
    handle->ngih_userOpened = 0;
    handle->ngih_fdEffective = 0;
    handle->ngih_fd = -1; /* invalid */
    handle->ngih_useSelector = 0;

    nglIOhandleStatusInitializeMember(&handle->ngih_read);
    nglIOhandleStatusInitializeMember(&handle->ngih_write);
    nglIOhandleChunkStatusInitializeMember(&handle->ngih_chunk);
    nglIOhandleSocketStatusInitializeMember(&handle->ngih_socket);
    nglIOhandleFileStatusInitializeMember(&handle->ngih_file);
    nglIOhandleTimeStatusInitializeMember(&handle->ngih_time);

    handle->ngih_closeRequesting = 0;
    handle->ngih_closeStarted = 0;
    handle->ngih_closeDone = 0;
    handle->ngih_closeErrorOccurred = 0;
    handle->ngih_closeErrorCode = 0;
}

/**
 * I/O handle: Initialize the status members.
 */
static void
nglIOhandleStatusInitializeMember(
    ngiIOhandleStatus_t *status)
{
    /* Check the arguments */
    assert(status != NULL);

    status->ngihs_callbackRegistering = 0;
    status->ngihs_callbackUnregistering = 0;
    status->ngihs_callbackUnregisterDone = 0;
    status->ngihs_callbackRegistered = 0;
    status->ngihs_callbackFunction = NULL;
    status->ngihs_callbackArgument = NULL;

    status->ngihs_requireProcess = 0;
    status->ngihs_requesting = 0;
    status->ngihs_started = 0;
    status->ngihs_done = 0;
    status->ngihs_errorOccurred = 0;
    status->ngihs_errorCode = 0;

    status->ngihs_buf = NULL;
    status->ngihs_length = 0;
    status->ngihs_waitNbytes = 0;
    status->ngihs_doneNbytesResult = NULL;
    status->ngihs_currentNbytes = 0;
    status->ngihs_doneNbytes = 0;
}

/**
 * I/O handle: Initialize the Read Chunk status members.
 */
static void
nglIOhandleChunkStatusInitializeMember(
    ngiIOhandleChunkStatus_t *status)
{
    /* Check the arguments */
    assert(status != NULL);

    status->ngihcs_callbackRegistering = 0;
    status->ngihcs_callbackUnregistering = 0;
    status->ngihcs_callbackUnregisterDone = 0;
    status->ngihcs_callbackRegistered = 0;
    status->ngihcs_callbackFunction = NULL;
    status->ngihcs_callbackArgument = NULL;
}


/**
 * I/O handle: Initialize the Socket status members.
 */
static void
nglIOhandleSocketStatusInitializeMember(
    ngiIOhandleSocketStatus_t *status)
{
    /* Check the arguments */
    assert(status != NULL);

    status->ngihss_listenCallbackRegistering = 0;
    status->ngihss_listenCallbackUnregistering = 0;
    status->ngihss_listenCallbackUnregisterDone = 0;
    status->ngihss_listenCallbackRegistered = 0;
    status->ngihss_listenCallbackFunction = NULL;
    status->ngihss_listenCallbackArgument = NULL;
    status->ngihss_requireProcess = 0;

    status->ngihss_requesting = 0;
    status->ngihss_started = 0;
    status->ngihss_done = 0;
    status->ngihss_errorOccurred = 0;
    status->ngihss_errorCode = 0;
    status->ngihss_socketType = NGI_IOHANDLE_SOCKET_TYPE_NONE;

    status->ngihss_listenerCreateRequest = 0;
    status->ngihss_listenerCreatePort = 0;
    status->ngihss_listenerCreatePortAllocated = 0;
    status->ngihss_listenerCreatePath = NULL;
    status->ngihss_listenerCreateBacklog = 0;

    status->ngihss_acceptRequest = 0;
    status->ngihss_acceptNewFd = -1; /* invalid */
    status->ngihss_acceptResult = NULL;

    status->ngihss_connectRequest = 0;
    status->ngihss_connectHostName = NULL;
    status->ngihss_connectPort = 0;
    status->ngihss_connectPath = NULL;

    status->ngihss_nodelayRequest = 0;
    status->ngihss_nodelayEnable = 0;
}

/**
 * I/O handle: Initialize the File status members.
 */
static void
nglIOhandleFileStatusInitializeMember(
    ngiIOhandleFileStatus_t *status)
{
    /* Check the arguments */
    assert(status != NULL);

    status->ngihfs_requireProcess = 0;
    status->ngihfs_requesting = 0;
    status->ngihfs_started = 0;
    status->ngihfs_done = 0;
    status->ngihfs_errorOccurred = 0;
    status->ngihfs_errorCode = 0;

    status->ngihfs_fileOpenRequest = 0;
    status->ngihfs_fileOpenPath = NULL;
    status->ngihfs_fileOpenType = NGI_IOHANDLE_FILE_OPEN_TYPE_NONE;

    status->ngihfs_fdOpenRequest = 0;
    status->ngihfs_fdOpenFd = -1; /* invalid */
}

/**
 * I/O handle: Initialize the Time status members.
 */
static void
nglIOhandleTimeStatusInitializeMember(
    ngiIOhandleTimeStatus_t *status)
{
    /* Check the arguments */
    assert(status != NULL);

    status->ngihts_callbackRegistering = 0;
    status->ngihts_callbackUnregistering = 0;
    status->ngihts_callbackUnregisterDone = 0;
    status->ngihts_callbackRegistered = 0;

    status->ngihts_timeCallbackFunction = NULL;
    status->ngihts_timeCallbackArgument = NULL;
    status->ngihts_timeCallbackExecuting = 0;

    status->ngihts_changeCallbackFunction = NULL;
    status->ngihts_changeCallbackArgument = NULL;
    status->ngihts_changeCallbackExecuting = 0;

    status->ngihts_eventTime = 0;
    status->ngihts_changeRequested = 0;
}


/**
 * I/O handle: Close.
 */
int
ngiIOhandleClose(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleClose";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleClose(
        driver, handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The close operation failed.\n"); 
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
 * I/O handle: Read.
 */
int
ngiIOhandleRead(
    ngiIOhandle_t *handle,
    char *buf,
    size_t length,
    size_t waitNbytes,
    size_t *readNbytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleRead";
    int result;

    result = nglIOhandleOperate(
        handle, NGL_EVENT_DRIVER_HANDLE_OP_READ,
        buf, length, waitNbytes, readNbytes,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The read operation failed.\n"); 
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
 * I/O handle: Write.
 */
int
ngiIOhandleWrite(
    ngiIOhandle_t *handle,
    char *buf,
    size_t length,
    size_t waitNbytes,
    size_t *writeNbytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleWrite";
    int result;

    result = nglIOhandleOperate(
        handle, NGL_EVENT_DRIVER_HANDLE_OP_WRITE,
        buf, length, waitNbytes, writeNbytes,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The write operation failed.\n"); 
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
 * I/O handle: Operate read or write.
 */
static int
nglIOhandleOperate(
    ngiIOhandle_t *handle,
    nglEventDriverHandleOperationType_t type,
    char *buf,
    size_t length,
    size_t waitNbytes,
    size_t *doneNbytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglIOhandleOperate";
    ngiEventDriver_t *driver;
    char *typeStr;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. The handle is NULL.\n");
        goto error;
    }

    assert(type > NGL_EVENT_DRIVER_HANDLE_OP_NONE);
    assert(type < NGL_EVENT_DRIVER_HANDLE_OP_NOMORE);

    typeStr = NULL;
    if (type == NGL_EVENT_DRIVER_HANDLE_OP_READ) {
        typeStr = "read";
    } else if (type == NGL_EVENT_DRIVER_HANDLE_OP_WRITE) {
        typeStr = "write";
    } else {
        abort();
    }
    assert(typeStr != NULL);

    if (buf == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. The buffer is NULL.\n");
        goto error;
    }

    if (length <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. length <= 0.\n");
        goto error;
    }

    if (waitNbytes > length) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. waitNbytes > length.\n");
        goto error;
    }

    if (doneNbytes == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. A pointer to store result is NULL.\n");
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleOperate(
        driver, handle, type,
        buf, length, waitNbytes, doneNbytes,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The %s operation failed.\n", typeStr); 
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
 * I/O handle: Read callback register.
 */
int
ngiIOhandleReadCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleReadCallbackRegister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (callbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The callback function is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleReadCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
        callbackFunction, callbackArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the read callback failed.\n"); 
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
 * I/O handle: Read callback unregister.
 */
int
ngiIOhandleReadCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleReadCallbackUnregister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleReadCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the read callback failed.\n"); 
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
 * I/O handle: Read Chunk callback register.
 */
int
ngiIOhandleReadChunkCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleReadChunkCallbackRegister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (callbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The callback function is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleReadChunkCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
        callbackFunction, callbackArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the read chunk callback failed.\n"); 
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
 * I/O handle: Read Chunk callback unregister.
 */
int
ngiIOhandleReadChunkCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleReadChunkCallbackUnregister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleReadChunkCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the read chunk callback failed.\n"); 
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
 * I/O handle: TCP listener create.
 */
int
ngiIOhandleTCPlistenerCreate(
    ngiIOhandle_t *handle,
    int port,
    int *portAllocated,
    int backlog,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPlistenerCreate";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (portAllocated == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The portAllocated is NULL.\n"); 
        goto error;
    }

    *portAllocated = -1;

    /* Operate. */
    result = nglIOhandleSocketOperate(
        handle, NGI_IOHANDLE_SOCKET_TYPE_TCP,
        1, port, portAllocated, NULL, backlog,
        0, NULL, NULL,
        0, NULL, 0, NULL,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "TCP listener create operation failed.\n"); 
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
 * I/O handle: TCP listener callback register.
 */
int
ngiIOhandleTCPlistenerCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPlistenerCallbackRegister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (callbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The callback function is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result =
        nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
        callbackFunction, callbackArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the TCP listener callback failed.\n");
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
 * I/O handle: TCP listener callback unregister.
 */
int
ngiIOhandleTCPlistenerCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPlistenerCallbackUnregister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result =
        nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the TCP listener callback failed.\n");
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
 * I/O handle: TCP accept.
 * Note: Accept Result is allocated if the acceptResult is not NULL.
 * Note: The caller must release the acceptResult.
 */
int
ngiIOhandleTCPaccept(
    ngiIOhandle_t *listenerHandle,
    ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPaccept";
    int result;

    /* Check the arguments */
    if (listenerHandle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle for listener is NULL.\n"); 
        goto error;
    }

    if (connectHandle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle for accept is NULL.\n"); 
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        listenerHandle, NGI_IOHANDLE_SOCKET_TYPE_TCP,
        0, 0, NULL, NULL, 0,
        1, connectHandle, acceptResult,
        0, NULL, 0, NULL,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "TCP accept operation failed.\n"); 
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
 * I/O handle: TCP connect.
 */
int
ngiIOhandleTCPconnect(
    ngiIOhandle_t *handle,
    char *host,
    int port,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPconnect";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        handle, NGI_IOHANDLE_SOCKET_TYPE_TCP,
        0, 0, NULL, NULL, 0,
        0, NULL, NULL,
        1, host, port, NULL,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "TCP connect operation failed.\n"); 
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
 * I/O handle: Set TCP_NODELAY.
 */
int
ngiIOhandleTCPnodelaySet(
    ngiIOhandle_t *handle,
    int enable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTCPnodelaySet";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        handle, NGI_IOHANDLE_SOCKET_TYPE_TCP,
        0, 0, NULL, NULL, 0,
        0, NULL, NULL,
        0, NULL, 0, NULL,
        1, enable,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "TCP connect operation failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#ifdef NGI_UNIX_SOCKET_ENABLED

/**
 * I/O handle: UNIX socket listener create.
 */
int
ngiIOhandleUNIXlistenerCreate(
    ngiIOhandle_t *handle,
    char *path,
    int backlog,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCreate";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (path == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The path is NULL.\n"); 
        goto error;
    }

    if (strlen(path) >= NGI_UNIX_SOCKET_PATH_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "The path \"%s\" is too long. max is %d.\n",
            path, NGI_UNIX_SOCKET_PATH_MAX); 
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        handle, NGI_IOHANDLE_SOCKET_TYPE_UNIX,
        1, 0, NULL, path, backlog,
        0, NULL, NULL,
        0, NULL, 0, NULL,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "UNIX socket listener create operation failed.\n"); 
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
 * I/O handle: UNIX socket listener callback register.
 */
int
ngiIOhandleUNIXlistenerCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCallbackRegister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (callbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The callback function is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result =
        nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
        callbackFunction, callbackArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the UNIX socket listener callback failed.\n");
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
 * I/O handle: UNIX socket listener callback unregister.
 */
int
ngiIOhandleUNIXlistenerCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCallbackUnregister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result =
        nglEventDriverRequestCommandHandleSocketListenerCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Unregister the UNIX socket listener callback failed.\n");
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
 * I/O handle: UNIX socket accept.
 * Note: Accept Result is allocated if the acceptResult is not NULL.
 * Note: The caller must release the acceptResult.
 */
int
ngiIOhandleUNIXaccept(
    ngiIOhandle_t *listenerHandle,
    ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXaccept";
    int result;

    /* Check the arguments */
    if (listenerHandle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle for listener is NULL.\n"); 
        goto error;
    }

    if (connectHandle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle for accept is NULL.\n"); 
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        listenerHandle, NGI_IOHANDLE_SOCKET_TYPE_UNIX,
        0, 0, NULL, NULL, 0,
        1, connectHandle, acceptResult,
        0, NULL, 0, NULL,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "UNIX socket accept operation failed.\n"); 
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
 * I/O handle: UNIX socket connect.
 */
int
ngiIOhandleUNIXconnect(
    ngiIOhandle_t *handle,
    char *path,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXconnect";
    int result;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (path == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. The path is NULL.\n");
        goto error;
    }

    if (strlen(path) >= NGI_UNIX_SOCKET_PATH_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "The path \"%s\" is too long. max is %d.\n",
            path, NGI_UNIX_SOCKET_PATH_MAX);
        goto error;
    }

    /* Operate. */
    result = nglIOhandleSocketOperate(
        handle, NGI_IOHANDLE_SOCKET_TYPE_UNIX,
        0, 0, NULL, NULL, 0,
        0, NULL, NULL,
        1, NULL, 0, path,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "UNIX socket connect operation failed.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

#else /* NGI_UNIX_SOCKET_ENABLED */

/**
 * I/O handle: UNIX socket listener create. (Just error)
 */
int
ngiIOhandleUNIXlistenerCreate(
    ngiIOhandle_t *handle,
    char *path,
    int backlog,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCreate";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* Failed */
    return 0;
}

/**
 * I/O handle: UNIX socket listener callback register. (Just error)
 */
int
ngiIOhandleUNIXlistenerCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCallbackRegister";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* Failed */
    return 0;
}

/**
 * I/O handle: UNIX socket listener callback unregister. (Just error)
 */
int
ngiIOhandleUNIXlistenerCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXlistenerCallbackUnregister";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* Failed */
    return 0;
}

/**
 * I/O handle: UNIX socket accept. (Just error)
 * Note: Accept Result is allocated if the acceptResult is not NULL.
 * Note: The caller must release the acceptResult.
 */
int
ngiIOhandleUNIXaccept(
    ngiIOhandle_t *listenerHandle,
    ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXaccept";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* Failed */
    return 0;
}

/**
 * I/O handle: UNIX socket connect. (Just error)
 */
int
ngiIOhandleUNIXconnect(
    ngiIOhandle_t *handle,
    char *path,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleUNIXconnect";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
        "UNIX domain socket authentication is not supported"
        " on this environment.\n"); 

    /* Failed */
    return 0;
}

#endif /* NGI_UNIX_SOCKET_ENABLED */

/**
 * I/O handle: Operate Socket listener create, accept, connect.
 */
static int
nglIOhandleSocketOperate(
    ngiIOhandle_t *handle,
    ngiIOhandleSocketType_t socketType,
    int isListenerCreate,
    int listenerPort,
    int *listenerPortAllocated,
    char *listenerPath,
    int listenerBacklog,
    int isAccept,
    ngiIOhandle_t *connectHandle,
    ngiIOhandleAcceptResult_t **acceptResult,
    int isConnect,
    char *connectHost,
    int connectPort,
    char *connectPath,
    int isNodelay,
    int nodelayEnable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglIOhandleSocketOperate";
    ngiEventDriver_t *driver;
    int result;

    /* Check the arguments */
    assert(handle != NULL);
    assert(socketType > NGI_IOHANDLE_SOCKET_TYPE_NONE);
    assert(socketType < NGI_IOHANDLE_SOCKET_TYPE_NOMORE);

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);
 
    result = nglEventDriverRequestCommandHandleSocket(
        driver, handle, socketType,
        isListenerCreate,
        listenerPort, listenerPortAllocated, listenerPath, listenerBacklog,
        isAccept, connectHandle, acceptResult,
        isConnect, connectHost, connectPort, connectPath,
        isNodelay, nodelayEnable,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "socket operation failed.\n"); 
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
 * I/O handle Accept Result: Construct.
 */
ngiIOhandleAcceptResult_t *
ngiIOhandleAcceptResultConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleAcceptResultConstruct";
    ngiIOhandleAcceptResult_t *acceptResult;
    int result, allocated;

    allocated = 0;

    acceptResult = NGI_ALLOCATE(ngiIOhandleAcceptResult_t, log, error);
    if (acceptResult == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Allocate the Accept Result failed.\n");
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglIOhandleAcceptResultInitialize(
        acceptResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Initialize the Accept Result failed.\n");
        goto error;
    }

    /* Success */
    return acceptResult;

    /* Error occurred */
error:

    if (allocated != 0) {
        allocated = 0;
        result = NGI_DEALLOCATE(
            ngiIOhandleAcceptResult_t, acceptResult, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
                "Deallocate the Accept Result failed.\n");
        }
    }

    /* Failed */
    return NULL;
}

/**
 * I/O handle Accept Result: Destruct.
 */
int
ngiIOhandleAcceptResultDestruct(
    ngiIOhandleAcceptResult_t *acceptResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleAcceptResultDestruct";
    int result;

    /* Check the arguments */

    if (acceptResult == NULL) {
        /* Success */
        return 1;
    }

    /* Finalize */
    result = nglIOhandleAcceptResultFinalize(
        acceptResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Finalize the Accept Result failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(
        ngiIOhandleAcceptResult_t, acceptResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Deallocate the Accept Result failed.\n");
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
 * I/O handle Accept Result: Initialize.
 */
static int
nglIOhandleAcceptResultInitialize(
    ngiIOhandleAcceptResult_t *acceptResult,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(acceptResult != NULL);

    nglIOhandleAcceptResultInitializeMember(acceptResult);

    /* Success */
    return 1;
}

/**
 * I/O handle Accept Result: Finalize.
 */
static int
nglIOhandleAcceptResultFinalize(
    ngiIOhandleAcceptResult_t *acceptResult,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */

    if (acceptResult == NULL) {
        /* Success */
        return 1;
    }

    nglIOhandleAcceptResultInitializeMember(acceptResult);

    /* Success */
    return 1;
}

/**
 * I/O handle Accept Result: Initialize the members.
 */
static void
nglIOhandleAcceptResultInitializeMember(
    ngiIOhandleAcceptResult_t *acceptResult)
{
    /* Check the arguments */
    assert(acceptResult != NULL);

    acceptResult->ngiar_type = NGI_IOHANDLE_SOCKET_TYPE_NONE;

    memset(&(acceptResult->ngiar_peerAddress), 0,
        sizeof(acceptResult->ngiar_peerAddress));
}

/**
 * I/O handle Accept Result: Output the log.
 */
int
ngiIOhandleAcceptResultLogOutput(
    ngiIOhandleAcceptResult_t *acceptResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleAcceptResultLogOutput";
    char ipAddress[NGI_HOST_NAME_MAX];
    int result;

    /* Check the arguments */
    if (acceptResult == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The Accept Result is NULL.\n"); 
        goto error;
    }

    if (acceptResult->ngiar_type == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
        /* Note: DNS lookup may take time, Thus, just output IP address. */

        result = getnameinfo(
            &(acceptResult->ngiar_peerAddress),
            sizeof(acceptResult->ngiar_peerAddress),
            ipAddress, sizeof(ipAddress),
            NULL, 0,
            NI_NUMERICHOST);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n",
                "getnameinfo()", result, gai_strerror(result)); 
            goto error;
        }
     
        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "New connection accepted from %s.\n",
            ipAddress); 
    
    } else if (acceptResult->ngiar_type == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {

        ngLogInfo(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "New Connection accepted from %s.\n",
            "UNIX socket"); 
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}
 
/**
 * I/O handle Accept Result: Is localhost?
 */
int
ngiIOhandleAcceptResultIsLocalhost(
    ngiIOhandleAcceptResult_t *acceptResult,
    int *isLocal,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleAcceptResultIsLocalhost";
    char peerIP[NGI_HOST_NAME_MAX], localIP[NGI_HOST_NAME_MAX];
    struct addrinfo addrHints, *addrList, *addrCur;
    int result;

    addrList = NULL;
    addrCur = NULL;

    /* Check the arguments */
    if (acceptResult == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The Accept Result is NULL.\n"); 
        goto error;
    }

    if (isLocal == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The isLocal flag pointer is NULL.\n"); 
        goto error;
    }

    *isLocal = 0;

    if (acceptResult->ngiar_type == NGI_IOHANDLE_SOCKET_TYPE_UNIX) {
        *isLocal = 1;

        /* Success */
        return 1;
    }

    assert(acceptResult->ngiar_type == NGI_IOHANDLE_SOCKET_TYPE_TCP);

    /* Get peer IP address. */
    result = getnameinfo(
        &acceptResult->ngiar_peerAddress,
        sizeof(acceptResult->ngiar_peerAddress),
        peerIP, sizeof(peerIP),
        NULL, 0,
        NI_NUMERICHOST);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "getnameinfo()", result, gai_strerror(result)); 
        goto error;
    }

    /* Check local IP address. */
    memset(&addrHints, 0, sizeof(addrHints));
    addrHints.ai_family = PF_INET; /* or AF_UNSPEC ? */
    addrHints.ai_socktype = SOCK_STREAM;
    addrHints.ai_protocol = 0;

    addrList = NULL;
    result = getaddrinfo(
        "localhost",
        NULL, &addrHints, &addrList);
    if ((result != 0) || (addrList == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "%s failed: %d: %s.\n",
            "getaddrinfo()", result, gai_strerror(result)); 
        goto error;
    }

    addrCur = addrList;
    while (addrCur != NULL) {

        if (addrCur->ai_family !=
            acceptResult->ngiar_peerAddress.sa_family) {
            addrCur = addrCur->ai_next;
            continue;
        }
        
        result = getnameinfo(
            addrCur->ai_addr, addrCur->ai_addrlen,
            localIP, sizeof(localIP),
            NULL, 0,
            NI_NUMERICHOST);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "%s failed: %d: %s.\n",
                "getnameinfo()", result, gai_strerror(result)); 
            goto error;
        }

        /* Compare local and peer. */
        if (strcmp(localIP, peerIP) == 0) {
            *isLocal = 1;
        }

        addrCur = addrCur->ai_next;
    }

    freeaddrinfo(addrList);
    addrList = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (addrList != NULL) {
        freeaddrinfo(addrList);
        addrList = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * I/O handle: Open file.
 */
int
ngiIOhandleFileOpen(
    ngiIOhandle_t *handle,
    char *path,
    ngiIOhandleFileOpenType_t type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleFileOpen";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (path == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. The path is NULL.\n");
        goto error;
    }

    if ((type <= NGI_IOHANDLE_FILE_OPEN_TYPE_NONE) ||
        (type >= NGI_IOHANDLE_FILE_OPEN_TYPE_NOMORE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName,
            "Invalid argument. wrong type %d.\n", type);
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    /* Operate. */
    result = nglEventDriverRequestCommandHandleFile(
        driver, handle,
        1, path, type,
        0, -1,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "File operation failed.\n"); 
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
 * I/O handle: Open file descriptor.
 */
int
ngiIOhandleFdOpen(
    ngiIOhandle_t *handle,
    int fd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleFdOpen";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (fd < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. fd is negative.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    /* Operate. */
    result = nglEventDriverRequestCommandHandleFile(
        driver, handle,
        0, NULL, NGI_IOHANDLE_FILE_OPEN_TYPE_NONE,
        1, fd,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "File operation failed.\n"); 
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
 * I/O handle: Time Event Time Change Request.
 * This function invokes the Event Time Change callback.
 */
int
ngiIOhandleTimeEventTimeChangeRequest(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTimeEventTimeChangeRequest";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    /* Operate. */
    result = nglEventDriverRequestCommandHandleTime(
        driver, handle,
        1,
        0, 0,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Time change request failed.\n"); 
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
 * I/O handle: Time Event Time Set.
 * This function is called only from Time Event Change callback.
 * eventTime is specified by absolute time get from time().
 * eventTime == 0 indicates no event callback performed.
 */
int
ngiIOhandleTimeEventTimeSet(
    ngiIOhandle_t *handle,
    time_t eventTime,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTimeEventTimeSet";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    /* Operate. */
    result = nglEventDriverRequestCommandHandleTime(
        driver, handle,
        0,
        1, eventTime,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Time set failed.\n"); 
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
 * I/O handle: Time Event Callback Register.
 */
int
ngiIOhandleTimeEventCallbackRegister(
    ngiIOhandle_t *handle,
    ngiIOhandleCallbackFunction_t eventCallbackFunction,
    void *eventCallbackArgument,
    ngiIOhandleCallbackFunction_t changeCallbackFunction,
    void *changeCallbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTimeEventCallbackRegister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    if (eventCallbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The event callback function is NULL.\n"); 
        goto error;
    }

    if (changeCallbackFunction == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The time change callback function is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleTimeCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_REGISTER,
        eventCallbackFunction, eventCallbackArgument,
        changeCallbackFunction, changeCallbackArgument,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the read callback failed.\n"); 
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
 * I/O handle: Time Event Callback Unregister.
 */
int
ngiIOhandleTimeEventCallbackUnregister(
    ngiIOhandle_t *handle,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleTimeEventCallbackUnregister";
    ngiEventDriver_t *driver;
    int result;

    driver = NULL;

    /* Check the arguments */
    if (handle == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The handle is NULL.\n"); 
        goto error;
    }

    driver = handle->ngih_eventDriver;
    assert(driver != NULL);

    result = nglEventDriverRequestCommandHandleTimeCallbackRegister(
        driver, handle,
        NGL_EVENT_DRIVER_CALLBACK_REGISTER_MODE_UNREGISTER,
        NULL, NULL, NULL, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Register the read callback failed.\n"); 
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
 * I/O handle Callback Waiter: Initialize.
 */
int
ngiIOhandleCallbackWaiterInitialize(
    ngiIOhandleCallbackWaiter_t *waiter,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleCallbackWaiterInitialize";
    int result;

    /* Check the arguments */
    if (waiter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The waiter is NULL.\n"); 
        goto error;
    }

    nglIOhandleCallbackWaiterInitializeMember(waiter);

    /* Initialize the Mutex. */
    result = ngiMutexInitialize(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the mutex failed.\n");
        goto error;
    }

    /* Initialize the Cond. */
    result = ngiCondInitialize(&waiter->ngihcw_cond, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Initialize the cond failed.\n");
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
 * I/O handle Callback Waiter: Finalize.
 */
int
ngiIOhandleCallbackWaiterFinalize(
    ngiIOhandleCallbackWaiter_t *waiter,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleCallbackWaiterFinalize";
    int result;

    /* Check the arguments */
    if (waiter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The waiter is NULL.\n"); 
        goto error;
    }

    /* Destroy the Mutex. */
    result = ngiMutexDestroy(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the mutex failed.\n");
        goto error;
    }

    /* Destroy the Cond. */
    result = ngiCondDestroy(&waiter->ngihcw_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Destroy the cond failed.\n");
        goto error;
    }

    nglIOhandleCallbackWaiterInitializeMember(waiter);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * I/O handle Callback Waiter: Initialize the members.
 */
void
nglIOhandleCallbackWaiterInitializeMember(
    ngiIOhandleCallbackWaiter_t *waiter)
{

    /* Check the arguments */
    assert(waiter != NULL);

    waiter->ngihcw_exist = 0;
    waiter->ngihcw_mutex = NGI_MUTEX_NULL;
    waiter->ngihcw_cond = NGI_COND_NULL;
}

/**
 * I/O handle Callback Waiter: Callback start.
 */
int
ngiIOhandleCallbackWaiterStart(
    ngiIOhandleCallbackWaiter_t *waiter,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleCallbackWaiterStart";
    int mutexLocked, result;

    mutexLocked = 0;

    /* Check the arguments */
    if (waiter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The waiter is NULL.\n"); 
        goto error;
    }

    result = ngiMutexLock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    assert(waiter->ngihcw_exist == 0);
    waiter->ngihcw_exist = 1;

    mutexLocked = 0;
    result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked == 0) {
        result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * I/O handle Callback Waiter: Callback end.
 */
int
ngiIOhandleCallbackWaiterEnd(
    ngiIOhandleCallbackWaiter_t *waiter,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleCallbackWaiterEnd";
    int mutexLocked, result;

    mutexLocked = 0;

    /* Check the arguments */
    if (waiter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The waiter is NULL.\n"); 
        goto error;
    }

    result = ngiMutexLock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    assert(waiter->ngihcw_exist != 0);
    waiter->ngihcw_exist = 0;

    result = ngiCondBroadcast(&waiter->ngihcw_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Broadcast the cond failed.\n");
        goto error;
    }

    mutexLocked = 0;
    result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked == 0) {
        result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * I/O handle Callback Waiter: Wait the callback end.
 */
int
ngiIOhandleCallbackWaiterWait(
    ngiIOhandleCallbackWaiter_t *waiter,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiIOhandleCallbackWaiterWait";
    int mutexLocked, result;

    mutexLocked = 0;

    /* Check the arguments */
    if (waiter == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Invalid argument. The waiter is NULL.\n"); 
        goto error;
    }

    result = ngiMutexLock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }
    mutexLocked = 1;

    while (waiter->ngihcw_exist != 0) {
        result = ngiCondWait(
            &waiter->ngihcw_cond, &waiter->ngihcw_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Wait the cond failed.\n");
            goto error;
        }
    }

    mutexLocked = 0;
    result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
            "Lock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (mutexLocked == 0) {
        result = ngiMutexUnlock(&waiter->ngihcw_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_IOEVENT, fName, 
                "Lock the mutex failed.\n");
        }
    }

    /* Failed */
    return 0;
}

