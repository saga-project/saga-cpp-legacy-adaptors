/*
 * $RCSfile: ngExternalModule.c,v $ $Revision: 1.34 $ $Date: 2008/03/28 08:50:58 $
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
 * Module of External Module control for Ninf-G.
 */

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngExternalModule.c,v $ $Revision: 1.34 $ $Date: 2008/03/28 08:50:58 $")

/**
 * Data
 */

/**
 * Prototype declaration of internal functions.
 */
static int
nglExternalModuleManagerInitialize(
    ngiExternalModuleManager_t *extMng,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleManagerFinalize(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleManagerInitializeMember(
    ngiExternalModuleManager_t *extMng);
static ngiExternalModule_t *
nglExternalModuleManagerGetExternalModule(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    int count,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleInitialize(
    ngiExternalModule_t *module,
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    ngiExternalModuleSubType_t moduleSubType,
    char *type,
    char *userExecutablePath,
    char *logFilePathCommon,
    char *logFilePath,
    int maxJobs,
    char **multiLineNotify,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleFinalize(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleInitializeMember(
    ngiExternalModule_t *module);
static int
nglExternalModuleRegister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleUnregister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static char *
nglExternalModuleTypeName(
    ngiExternalModuleType_t moduleType);
static int
nglExternalModuleCountUp(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    int *currentCount,
    ngLog_t *log,
    int *error);
static ngiExternalModuleCount_t *
nglExternalModuleCountConstruct(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleCountDestruct(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleCountInitialize(
    ngiExternalModuleCount_t *moduleCount,
    ngiExternalModuleType_t moduleType,
    char *type,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleCountFinalize(
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleCountInitializeMember(
    ngiExternalModuleCount_t *moduleCount);
static int
nglExternalModuleCountRegister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleCountUnregister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleProcessInvoke(
    ngiExternalModule_t *module,
    char *userExecutablePath,
    char *logFilePathCommon,
    char *logFilePath,
    int *fds,
    pid_t *processPid,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleProcessInvokeChild(
    char *externalModuleProgram,
    int requireChildChild,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr);
static void
nglExternalModuleProcessInvokeChildExec(
    char *externalModuleProgram,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr);
static int
nglExternalModuleProcessWait(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleProcessExitStatusPrint(
    ngiExternalModule_t *module,
    int exitStatus,
    int isExternalModuleProcess,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReaderInitialize(
    ngiExternalModuleReader_t *reader,
    ngiExternalModuleReaderType_t readerType,
    ngiExternalModule_t *module,
    ngiEvent_t *event,
    int fd,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReaderFinalize(
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleReaderInitializeMember(
    ngiExternalModuleReader_t *reader);
static int
nglExternalModuleReaderCallback(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError);
static int
nglExternalModuleReaderProcessError(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    int isWorking,
    ngiIOhandleState_t handleState,
    int isEOF,
    int cancelRequested,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReaderProcessCharacters(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    char *buf,
    size_t length,
    size_t readNbytes,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReaderProcessLine(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    char *line,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReplyFirstLineParse(
    char *target,
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    int *multiLine,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReplyMultiLineEndCheck(
    char *target,
    int *isEnd,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReplyProcess(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleNotifyFirstLineParse(
    char *target,
    int nMultiLineNotify,
    char **multiLineNotify,
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    int *multiLine,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleNotifyMultiLineEndCheck(
    char *target,
    char *notifyName,
    int *isEnd,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleNotifyProcess(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleNotifyCallbackCall(
    ngiExternalModule_t *module,
    ngiExternalModuleNotifyState_t state,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReadBufferInitialize(
    ngiExternalModuleReadBuffer_t *readBuffer,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReadBufferFinalize(
    ngiExternalModuleReadBuffer_t *readBuffer,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleReadBufferInitializeMember(
    ngiExternalModuleReadBuffer_t *readBuffer);
static int
nglExternalModuleRequestReplyStatusInitialize(
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleRequestReplyStatusFinalize(
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleRequestReplyStatusInitializeMember(
    ngiExternalModuleRequestReplyStatus_t *requestReply);
static int
nglExternalModuleNotifyStatusInitialize(
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleNotifyStatusFinalize(
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleNotifyStatusInitializeMember(
    ngiExternalModuleNotifyStatus_t *notifyStatus);
static int
nglExternalModuleRequestLock(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleRequestUnlock(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReplySet(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleReplyWait(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleRequestSend(
    ngiExternalModule_t *module,
    char *requestName,
    char *requestArgument,
    ngiLineList_t *requestOptions,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleRequestLineOutput(
    ngiExternalModule_t *module,
    char *requestName,
    int isFirstLine,
    ngLog_t *log,
    int *error,
    const char *fmt,
    ...);
static int
nglExternalModuleArgumentInitialize(
    ngiExternalModuleArgument_t *arg,
    char *src,
    ngLog_t *log,
    int *error);
static int
nglExternalModuleArgumentFinalize(
    ngiExternalModuleArgument_t *arg,
    ngLog_t *log,
    int *error);
static void
nglExternalModuleArgumentInitializeMember(
    ngiExternalModuleArgument_t *arg);

/**
 * Functions
 */

/**
 * ExternalModuleManager: Construct.
 */
ngiExternalModuleManager_t *
ngiExternalModuleManagerConstruct(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerConstruct";
    ngiExternalModuleManager_t *extMng;
    int result;

    extMng = NULL;

    /* Allocate */
    extMng = NGI_ALLOCATE(ngiExternalModuleManager_t, log, error);
    if (extMng == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the External Module Manager failed.\n");
        goto error;
    }

    /* Initialize */
    result = nglExternalModuleManagerInitialize(
        extMng, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the External Module Manager failed.\n");
        goto error;
    }

    /* Success */
    return extMng;

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * ExternalModuleManager: Destruct.
 */
int
ngiExternalModuleManagerDestruct(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerDestruct";
    int result;

    /* Check the arguments */
    if (extMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. External Module is NULL.\n");
        return 0;
    }

    /* Finalize */
    result = nglExternalModuleManagerFinalize(
        extMng, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the External Module Manager failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiExternalModuleManager_t, extMng, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the External Module Manager failed.\n");
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
 * ExternalModuleManager: Initialize.
 */
static int
nglExternalModuleManagerInitialize(
    ngiExternalModuleManager_t *extMng,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleManagerInitialize";
    int result;

    /* Check the arguments */
    assert(extMng != NULL);

    nglExternalModuleManagerInitializeMember(extMng);

    extMng->ngemm_nExternalModule = 0;
    extMng->ngemm_externalModule_head = NULL;

    extMng->ngemm_lastID = 0;

    extMng->ngemm_nInvokeServer = 0;
    extMng->ngemm_nCommunicationProxy = 0;
    extMng->ngemm_nInformationService = 0;

    extMng->ngemm_invokeServer_lastID = 0;
    extMng->ngemm_communicationProxy_lastID = 0;
    extMng->ngemm_informationService_lastID = 0;

    extMng->ngemm_nModuleCount = 0;
    extMng->ngemm_moduleCount_head = NULL;

    extMng->ngemm_event = event;

    result = ngiMutexInitialize(&extMng->ngemm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the mutex failed.\n");
        goto error;
    }

    result = ngiRWlockInitialize(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the RWlock failed.\n");
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
 * ExternalModuleManager: Finalize.
 */
static int
nglExternalModuleManagerFinalize(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleManagerFinalize";
    int result;

    /* Check the arguments */
    assert(extMng != NULL);

    /* Destruct the existing External Modules */
    while (extMng->ngemm_externalModule_head != NULL) {
        result = ngiExternalModuleDestruct(
            extMng->ngemm_externalModule_head, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Destruct the External Module failed.\n");
            goto error;
        }
    }
    
    /* Destruct the all module count. */
    result = nglExternalModuleCountDestruct(
        extMng, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the External Module count failed.\n");
        goto error;
    }
    
    result = ngiRWlockFinalize(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the RWlock failed.\n");
        goto error;
    }

    result = ngiMutexDestroy(&extMng->ngemm_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destroy the mutex failed.\n");
        goto error;
    }

    nglExternalModuleManagerInitializeMember(extMng);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleManager: Initialize the members.
 */
static void
nglExternalModuleManagerInitializeMember(
    ngiExternalModuleManager_t *extMng)
{
    /* Check the arguments */
    assert(extMng != NULL);

    extMng->ngemm_nExternalModule = 0;
    extMng->ngemm_externalModule_head = 0;
    extMng->ngemm_lastID = 0;
    extMng->ngemm_nInvokeServer = 0;
    extMng->ngemm_nCommunicationProxy = 0;
    extMng->ngemm_nInformationService = 0;
    extMng->ngemm_invokeServer_lastID = 0;
    extMng->ngemm_communicationProxy_lastID = 0;
    extMng->ngemm_informationService_lastID = 0;
    extMng->ngemm_nModuleCount = 0;
    extMng->ngemm_moduleCount_head = 0;
}

/**
 * ExternalModuleManager: Get the External Module by type and count.
 * count == -1 : find the last of "type" External Module.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static ngiExternalModule_t *
nglExternalModuleManagerGetExternalModule(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    int count,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleManagerGetExternalModule";
    ngiExternalModule_t *cur, *returnModule;
    char *moduleName;

    moduleName = NULL;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);

    /* Check the arguments */
    if (type == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module type is NULL.\n");
        goto error;
    }

    moduleName = nglExternalModuleTypeName(moduleType);

    returnModule = NULL;
    cur = extMng->ngemm_externalModule_head;
    for (; cur != NULL; cur = cur->ngem_next) {
        assert(cur->ngem_moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
        assert(cur->ngem_moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
        assert(cur->ngem_type != NULL);

        if ((cur->ngem_moduleType == moduleType) &&
            (strcmp(cur->ngem_type, type) == 0)) {

            if (count == -1) {
                /* Found */
                if (returnModule == NULL) {
                    returnModule = cur;

                } else {
                    if (cur->ngem_typeCount >= returnModule->ngem_typeCount) {
                        returnModule = cur;
                    }
                }
            } else {
                if (cur->ngem_typeCount == count) {
                    /* Found */
                    returnModule = cur;
                    break;
                }
            }
        }
    }

    /* Found */
    if (returnModule != NULL) {
        return returnModule;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "External Module is not found by type %s \"%s\" (%d%s).\n",
        moduleName, type, count, ((count == -1) ? ":last" : ""));

    /* Error occurred */
error:

    /* Failed */
    return NULL;
}

/**
 * ExternalModule: Read Lock.
 */
int
ngiExternalModuleManagerListReadLock(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerListReadLock";
    int result;

    /* Check the arguments */
    if (extMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module Manager is NULL.\n");
        goto error;
    }

    result = ngiRWlockReadLock(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Read lock the RWlock failed.\n");
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
 * ExternalModule: Read Unlock.
 */
int
ngiExternalModuleManagerListReadUnlock(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerListReadUnlock";
    int result;

    /* Check the arguments */
    if (extMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module Manager is NULL.\n");
        goto error;
    }

    result = ngiRWlockReadUnlock(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Read unlock the RWlock failed.\n");
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
 * ExternalModule: Write Lock.
 */
int
ngiExternalModuleManagerListWriteLock(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerListWriteLock";
    int result;

    /* Check the arguments */
    if (extMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module Manager is NULL.\n");
        goto error;
    }

    result = ngiRWlockWriteLock(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Write lock the RWlock failed.\n");
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
 * ExternalModule: Write Unlock.
 */
int
ngiExternalModuleManagerListWriteUnlock(
    ngiExternalModuleManager_t *extMng,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleManagerListWriteUnlock";
    int result;

    /* Check the arguments */
    if (extMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module Manager is NULL.\n");
        goto error;
    }

    result = ngiRWlockWriteUnlock(&extMng->ngemm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Write unlock the RWlock failed.\n");
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
 * ExternalModule: Construct.
 * Note: maxJobs 0 indicates unlimited jobs.
 * Note: multiLineNotify is array of string and last entry must be NULL.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngiExternalModule_t *
ngiExternalModuleConstruct(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    ngiExternalModuleSubType_t moduleSubType,
    char *type,
    char *userExecutablePath,
    char *logFilePathCommon,
    char *logFilePath,
    int maxJobs,
    char **multiLineNotify,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleConstruct";
    ngiExternalModule_t *module;
    int result, allocated, initialized;

    module = NULL;
    allocated = 0;
    initialized = 0;

    /* Check the arguments */
    if ((extMng == NULL) || (type == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((extMng == NULL) ? "External Module" :
            ((type == NULL) ? "type" : "unknown")));
        return 0;
    }

    /* Allocate */
    module = NGI_ALLOCATE(ngiExternalModule_t, log, error);
    if (module == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the External Module failed.\n");
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglExternalModuleInitialize(
        module, extMng, moduleType, moduleSubType, type,
        userExecutablePath, logFilePathCommon, logFilePath,
        maxJobs, multiLineNotify,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the External Module failed.\n");
        goto error;
    }
    initialized = 1;

    /* Register */
    result = nglExternalModuleRegister(
        extMng, module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Register the External Module failed.\n");
        goto error;
    }

    /* Success */
    return module;

    /* Error occurred */
error:
    if (initialized != 0) {
        initialized = 0;
        result = nglExternalModuleFinalize(
            module, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Finalize the External Module failed.\n");
        }
    }

    if (allocated != 0) {
        allocated = 0;
        result = NGI_DEALLOCATE(ngiExternalModule_t, module, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Deallocate the External Module failed.\n");
            goto error;
        }
    }

    /* Failed */
    return NULL;
}

/**
 * ExternalModule: Destruct.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
int
ngiExternalModuleDestruct(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleDestruct";
    ngiExternalModuleManager_t *extMng;
    int result;

    extMng = NULL;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. External Module is NULL.\n");
        return 0;
    }

    extMng = module->ngem_manager;

    /* Unregister */
    result = nglExternalModuleUnregister(
        extMng, module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the External Module failed.\n");
        goto error;
    }

    /* Finalize */
    result = nglExternalModuleFinalize(
        module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the External Module failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiExternalModule_t, module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the External Module failed.\n");
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
 * ExternalModule: Initialize.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleInitialize(
    ngiExternalModule_t *module,
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    ngiExternalModuleSubType_t moduleSubType,
    char *type,
    char *userExecutablePath,
    char *logFilePathCommon,
    char *logFilePath,
    int maxJobs,
    char **multiLineNotify,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleInitialize";
    int result, typeCount, count, i, fds[3];
    char *moduleName, *typeCopy;
    char **newNotifyTable, *tmp;
    ngiIOhandle_t *handle;
    ngiEvent_t *event;
    pid_t processPid;

    typeCopy = NULL;
    typeCount = 0;
    fds[0] = -1;
    fds[1] = -1;
    fds[2] = -1;
    processPid = 0;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
    assert(moduleSubType > NGI_EXTERNAL_MODULE_SUB_TYPE_NONE);
    assert(moduleSubType < NGI_EXTERNAL_MODULE_SUB_TYPE_NOMORE);
    assert(type != NULL);

    event = extMng->ngemm_event;

    nglExternalModuleInitializeMember(module);

    result = ngiMutexInitialize(
        &module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the mutex failed.\n");
        goto error;
    }

    result = ngiCondInitialize(
        &module->ngem_cond, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the cond failed.\n");
        goto error;
    }

    module->ngem_manager = extMng;
    module->ngem_next = NULL;

    module->ngem_log = log;

    extMng->ngemm_lastID++;
    module->ngem_ID = extMng->ngemm_lastID;

    module->ngem_moduleType = moduleType;
    module->ngem_moduleSubType = moduleSubType;
    moduleName = nglExternalModuleTypeName(moduleType);
    module->ngem_moduleID = 0;

    switch (moduleType) {
    case NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER :
        extMng->ngemm_nInvokeServer++;
        extMng->ngemm_invokeServer_lastID++;
        module->ngem_moduleID = extMng->ngemm_invokeServer_lastID;
        break;

    case NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY :
        extMng->ngemm_nCommunicationProxy++;
        extMng->ngemm_communicationProxy_lastID++;
        module->ngem_moduleID = extMng->ngemm_communicationProxy_lastID;
        break;

    case NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE :
        extMng->ngemm_nInformationService++;
        extMng->ngemm_informationService_lastID++;
        module->ngem_moduleID = extMng->ngemm_informationService_lastID;
        break;

    default:
        abort();
    }

    typeCopy = ngiStrdup(type, log, error);
    if (typeCopy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Duplicate the string failed.\n");
        goto error;
    }
    module->ngem_type = typeCopy;

    /* Count Up the Number of External Module. */
    typeCount = -1;
    result = nglExternalModuleCountUp(
        extMng, moduleType, type, &typeCount, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Count up the number of %s %s failed.\n",
            moduleName, type);
        goto error;
    }

    module->ngem_typeCount = typeCount;
    module->ngem_owner = NULL;

    /* Set the state */
    module->ngem_valid = 1;
    module->ngem_working = 1;
    module->ngem_errorCode = NG_ERROR_NO_ERROR;

    module->ngem_nJobsMax = maxJobs;
    module->ngem_nJobsStart = 0;
    module->ngem_nJobsStop = 0;
    module->ngem_nJobsDone = 0;
    module->ngem_serving = 1;

    count = 0;
    newNotifyTable = NULL;

    if (multiLineNotify != NULL) {
        for (i = 0; multiLineNotify[i] != NULL; i++) {
            count++;
        }
    }

    if (count > 0) {
        newNotifyTable = ngiCalloc(count, sizeof(char *), log, error);
        if (newNotifyTable == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "calloc for multi line Notify table failed.\n");
            goto error;
        }

        for (i = 0; i < count; i++) {
            tmp = ngiStrdup(multiLineNotify[i], log, error);
            if (tmp == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Duplicate the Notify name failed.\n");
                goto error;
            }
            newNotifyTable[i] = tmp;
        }
    }
    module->ngem_nMultiLineNotify = count;
    module->ngem_multiLineNotify = newNotifyTable;

    /* Invoke the process */
    result = nglExternalModuleProcessInvoke(
        module, userExecutablePath, logFilePathCommon, logFilePath,
        fds, &processPid, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invoke the %s %s process failed.\n",
            moduleName, type);
        goto error;
    }

    module->ngem_processPid = processPid;

    /* Open the Request handle. */
    handle = ngiIOhandleConstruct(event, log, error);
    if (handle == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Construct the I/O handle failed.\n");
        goto error;
    }

    result = ngiIOhandleFdOpen(handle, fds[0], log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Open the Request pipe fd failed.\n");
        goto error;
    }

    module->ngem_requestHandle = handle;
    handle = NULL;

    /* Initialize the Reply Reader. */
    result = nglExternalModuleReaderInitialize(
        &module->ngem_replyReader,
        NGI_EXTERNAL_MODULE_READER_TYPE_REPLY,
        module, event, fds[1], log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Reply reader failed.\n");
        goto error;
    }

    /* Initialize the Notify Reader. */
    result = nglExternalModuleReaderInitialize(
        &module->ngem_notifyReader,
        NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY,
        module, event, fds[2], log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Notify reader failed.\n");
        goto error;
    }

    /* Initialize the Request Reply Status */
    result = nglExternalModuleRequestReplyStatusInitialize(
        &module->ngem_requestReply, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Request Reply status failed.\n");
        goto error;
    }

    /* Initialize the Notify Status */
    result = nglExternalModuleNotifyStatusInitialize(
        &module->ngem_notifyStatus, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Notify status failed.\n");
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
 * ExternalModule: Finalize.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleFinalize(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleFinalize";
    ngiExternalModuleManager_t *extMng;
    ngiExternalModuleType_t moduleType;
    int result, i, typeCount;
    char *moduleName, *type;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. External Module is NULL.\n");
        return 0;
    }

    extMng = module->ngem_manager;
    moduleType = module->ngem_moduleType;
    moduleName = nglExternalModuleTypeName(moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Destruct the External Module for %s %s(%d).\n",
        moduleName, type, typeCount);

    module->ngem_working = 0; /* terminating. */

    /**
     * following is not performed though.
     * request the EXIT and wait reply if not requested.
     * unregister the callback.
     */

    result = nglExternalModuleReaderFinalize(
        &module->ngem_replyReader, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Reply reader failed.\n");
        goto error;
    }

    result = nglExternalModuleReaderFinalize(
        &module->ngem_notifyReader, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Notify reader failed.\n");
        goto error;
    }

    result = ngiIOhandleDestruct(
        module->ngem_requestHandle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the handle failed.\n");
        goto error;
    }
    module->ngem_requestHandle = NULL;

    /* Wait the process */
    result = nglExternalModuleProcessWait(
        module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Wait the %s %s(%d) process failed.\n",
            moduleName, type, typeCount);
        goto error;
    }

    /* Finalize the Request Reply Status */
    result = nglExternalModuleRequestReplyStatusFinalize(
        &module->ngem_requestReply, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Request Reply status failed.\n");
        goto error;
    }

    /* Finalize the Notify Status */
    result = nglExternalModuleNotifyStatusFinalize(
        &module->ngem_notifyStatus, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Notify status failed.\n");
        goto error;
    }

    switch (moduleType) {
    case NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER :
        extMng->ngemm_nInvokeServer--;
        break;

    case NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY :
        extMng->ngemm_nCommunicationProxy--;
        break;

    case NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE :
        extMng->ngemm_nInformationService--;
        break;

    default:
        abort();
    }


    for (i = 0; i < module->ngem_nMultiLineNotify; i++) {
        ngiFree(module->ngem_multiLineNotify[i], log, error);
        module->ngem_multiLineNotify[i] = NULL;
    }

    if (module->ngem_multiLineNotify != NULL) {
        ngiFree(module->ngem_multiLineNotify, log, error);
    }
    module->ngem_multiLineNotify = NULL;
    module->ngem_nMultiLineNotify = 0;

    type = NULL;
    ngiFree(module->ngem_type, log, error);
    module->ngem_type = NULL;
    
    result = ngiMutexDestroy(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destroy the mutex failed.\n");
        goto error;
    }

    result = ngiCondDestroy(&module->ngem_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destroy the cond failed.\n");
        goto error;
    }

    nglExternalModuleInitializeMember(module);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Initialize the members.
 */
static void
nglExternalModuleInitializeMember(
    ngiExternalModule_t *module)
{
    /* Check the arguments */
    assert(module != NULL);

    module->ngem_manager = NULL;
    module->ngem_next = NULL;

    module->ngem_log = NULL;
    module->ngem_ID = 0;
    module->ngem_moduleType = NGI_EXTERNAL_MODULE_TYPE_NONE;
    module->ngem_moduleSubType = NGI_EXTERNAL_MODULE_SUB_TYPE_NONE;
    module->ngem_moduleID = 0;
    module->ngem_type = NULL;
    module->ngem_typeCount = 0;
    module->ngem_owner = NULL;

    module->ngem_valid = 0;
    module->ngem_working = 0;
    module->ngem_errorCode = NG_ERROR_NO_ERROR;

    module->ngem_mutex = NGI_MUTEX_NULL;
    module->ngem_cond = NGI_COND_NULL;

    module->ngem_nJobsMax = 0;
    module->ngem_nJobsStart = 0;
    module->ngem_nJobsStop = 0;
    module->ngem_nJobsDone = 0;
    module->ngem_serving = 0;

    module->ngem_processPid = 0;

    module->ngem_requestHandle = NULL;
    nglExternalModuleReaderInitializeMember(&module->ngem_replyReader);
    nglExternalModuleReaderInitializeMember(&module->ngem_notifyReader);

    nglExternalModuleRequestReplyStatusInitializeMember(
        &module->ngem_requestReply);
    nglExternalModuleNotifyStatusInitializeMember(
        &module->ngem_notifyStatus);

    module->ngem_nMultiLineNotify = 0;
    module->ngem_multiLineNotify = NULL;
}

/**
 * ExternalModule: Register External Module.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleRegister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleRegister";
    ngiExternalModule_t **tail;
    int count;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(module != NULL);
    assert(module->ngem_next == NULL);

    count = 0;

    /* Find the tail. */
    tail = &extMng->ngemm_externalModule_head;
    while (*tail != NULL) {
        if (*tail == module) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "The External Module is already registered.\n");
            goto error;
        }

        tail = &(*tail)->ngem_next;
        count++;
    }

    *tail = module;
    count++;

    extMng->ngemm_nExternalModule = count;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Unregister External Module.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleUnregister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleUnregister";
    ngiExternalModule_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(module != NULL);

    /* Count the number of External Modules. */
    count = 0;
    cur = extMng->ngemm_externalModule_head;
    while (cur != NULL) {
        count++;
        cur = cur->ngem_next;
    }
    extMng->ngemm_nExternalModule = count;

    /* Delete the data from the list. */
    prevPtr = &extMng->ngemm_externalModule_head;
    cur = extMng->ngemm_externalModule_head;
    for (; cur != NULL; cur = cur->ngem_next) {
        if (cur == module) {
            /* Unlink the list */
            *prevPtr = cur->ngem_next;
            module->ngem_next = NULL;
            count--;
            extMng->ngemm_nExternalModule = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->ngem_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "The External Module is not registered.\n");
    return 0;
}

/**
 * ExternalModule: Convert enum to string.
 */
static char *
nglExternalModuleTypeName(
    ngiExternalModuleType_t moduleType)
{
    switch(moduleType) {
    case NGI_EXTERNAL_MODULE_TYPE_NONE:
        return "invalid (zero)";

    case NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER:
        return "Invoke Server";

    case NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY:
        return "Communication Proxy";

    case NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE:
        return "Information Service";

    case NGI_EXTERNAL_MODULE_TYPE_NOMORE:
        return "invalid (max)";

    default:
        break;
    }
    
    return "invalid (overflow)";
}

/**
 * ExternalModule: Count Up.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleCountUp(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    int *currentCount,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountUp";
    ngiExternalModuleCount_t *moduleCount, *cur;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
    assert(type != NULL);
    assert(currentCount != NULL);

    *currentCount = -1;

    /* Find the External Module Count */
    moduleCount = NULL;
    cur = extMng->ngemm_moduleCount_head;
    for (; cur != NULL; cur = cur->ngemc_next) {
        if ((cur->ngemc_moduleType == moduleType) &&
            (strcmp(cur->ngemc_type, type) == 0)) {

            /* Found */
            moduleCount = cur;
            break;
        }
    }

    /* Not Found */
    if (moduleCount == NULL) {

        /* Create new External Module Count */
        moduleCount = nglExternalModuleCountConstruct(
            extMng, moduleType, type, log, error);
        if (moduleCount == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "External Module %s %s Count Construct failed.\n",
                nglExternalModuleTypeName(moduleType), type);
            return 0;
        }
    }

    assert(moduleCount != NULL);

    *currentCount = moduleCount->ngemc_count;
    moduleCount->ngemc_count++;

    /* Success */
    return 1;
}

/**
 * ExternalModuleCount: Construct.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static ngiExternalModuleCount_t *
nglExternalModuleCountConstruct(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountConstruct";
    ngiExternalModuleCount_t *moduleCount;
    int allocated, initialized, result;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
    assert(type != NULL);

    allocated = 0;
    initialized = 0;

    /* Allocate */
    moduleCount = NGI_ALLOCATE(ngiExternalModuleCount_t, log, error);
    if (moduleCount == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the External Module Count failed.\n");
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = nglExternalModuleCountInitialize(
        moduleCount, moduleType, type, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the External Module %s %s Count failed.\n",
            nglExternalModuleTypeName(moduleType), type);
        goto error;
    }
    initialized = 1;

    /* Register */
    result = nglExternalModuleCountRegister(
        extMng, moduleCount, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Register the External Module %s %s Count failed.\n",
            nglExternalModuleTypeName(moduleType), type);
        goto error;
    }

    /* Success */
    return moduleCount;

    /* Error occurred */
error:

    /* Finalize */
    if ((moduleCount != NULL) && (initialized != 0)) {
        initialized = 0;
        result = nglExternalModuleCountFinalize(
            moduleCount, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Finalize the External Module Count failed.\n");
        }
    }

    /* Deallocate */
    if ((moduleCount != NULL) && (allocated != 0)) {
        allocated = 0;
        result = NGI_DEALLOCATE(
            ngiExternalModuleCount_t, moduleCount, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Deallocate the External Module Count failed.\n");
        }
        moduleCount = NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * ExternalModuleCount: Destruct.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
nglExternalModuleCountDestruct(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountDestruct";
    ngiExternalModuleCount_t *cur;
    int destructAll, result;

    /* Check the arguments */
    assert(extMng != NULL);

    destructAll = 0;

    if (moduleCount == NULL) {
        destructAll = 1;
    }

    if (destructAll != 0) {
        cur = extMng->ngemm_moduleCount_head;
    } else {
        cur = moduleCount;
    }

    while (cur != NULL) {
        /* Unregister */
        result = nglExternalModuleCountUnregister(
            extMng, cur, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unregister the External Module Count failed.\n");
            return 0;
        }
     
        /* Finalize */
        result = nglExternalModuleCountFinalize(
            cur, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Finalize the External Module Count failed.\n");
            return 0;
        }
     
        /* Deallocate */
        result = NGI_DEALLOCATE(ngiExternalModuleCount_t, cur, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Deallocate the External Module Count failed.\n");
            return 0;
        }

        /* Next Invoke Count */
        if (destructAll != 0) {
            /* Get the new head */
            cur = extMng->ngemm_moduleCount_head;
        } else {
            cur = NULL;
        }
    }

    /* Success */
    return 1;
}

/**
 * ExternalModuleCount: Initialize.
 */
static int
nglExternalModuleCountInitialize(
    ngiExternalModuleCount_t *moduleCount,
    ngiExternalModuleType_t moduleType,
    char *type,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountInitialize";
    char *typeCopy;

    /* Check the arguments */
    assert(moduleCount != NULL);
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
    assert(type != NULL);
    assert(moduleCount != NULL);

    nglExternalModuleCountInitializeMember(moduleCount);

    moduleCount->ngemc_next = NULL;
    moduleCount->ngemc_moduleType = moduleType;

    typeCopy = ngiStrdup(type, log, error);
    if (typeCopy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Duplicate the string failed.\n");
        return 0;
    }
    moduleCount->ngemc_type = typeCopy;

    moduleCount->ngemc_count = 0;

    /* Success */
    return 1;
}

/**
 * ExternalModuleCount: Finalize.
 */
static int
nglExternalModuleCountFinalize(
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(moduleCount != NULL);

    assert(moduleCount->ngemc_type != NULL);
    ngiFree(moduleCount->ngemc_type, log, error);
    moduleCount->ngemc_type = NULL;

    nglExternalModuleCountInitializeMember(moduleCount);

    /* Success */
    return 1;
}

/**
 * ExternalModuleCount: Initialize Member.
 */
static void
nglExternalModuleCountInitializeMember(
    ngiExternalModuleCount_t *moduleCount)
{
    /* Check the arguments */
    assert(moduleCount != NULL);

    moduleCount->ngemc_next = NULL;
    moduleCount->ngemc_moduleType = NGI_EXTERNAL_MODULE_TYPE_NONE;
    moduleCount->ngemc_type = NULL;
    moduleCount->ngemc_count = 0;
}

/**
 * ExternalModuleCount: Register.
 */
static int
nglExternalModuleCountRegister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountRegister";
    ngiExternalModuleCount_t **tail;
    int count;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleCount != NULL);

    count = 0;

    /* Find the tail. */
    tail = &extMng->ngemm_moduleCount_head;
    while (*tail != NULL) {
        if (*tail == moduleCount) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "The External Module Count is already registered.\n");
            goto error;
        }

        tail = &(*tail)->ngemc_next;
        count++;
    }

    *tail = moduleCount;
    count++;

    extMng->ngemm_nModuleCount = count;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleCount: Unregister.
 */
static int
nglExternalModuleCountUnregister(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleCount_t *moduleCount,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleCountUnregister";
    ngiExternalModuleCount_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(extMng != NULL);
    assert(moduleCount != NULL);

    /* Count the number of elements. */
    count = 0;
    cur = extMng->ngemm_moduleCount_head;
    while (cur != NULL) {
        count++;
        cur = cur->ngemc_next;
    }
    extMng->ngemm_nModuleCount = count;

    /* Delete the data from the list. */
    prevPtr = &extMng->ngemm_moduleCount_head;
    cur = extMng->ngemm_moduleCount_head;
    for (; cur != NULL; cur = cur->ngemc_next) {
        if (cur == moduleCount) {
            /* Unlink the list */
            *prevPtr = cur->ngemc_next;
            moduleCount->ngemc_next = NULL;
            count--;
            extMng->ngemm_nModuleCount = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->ngemc_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "The External Module Count is not registered.\n");
    return 0;
}

/**
 * ExternalModule: Invoke the External Module process.
 */
static int
nglExternalModuleProcessInvoke(
    ngiExternalModule_t *module,
    char *userExecutablePath,
    char *logFilePathCommon,
    char *logFilePath,
    int *fds,
    pid_t *processPid,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleProcessInvoke";
    int childStdin[2], childStdout[2], childStderr[2];
    char *defaultExecutablePath;
    int argCur, result, exitStatus;
    char logFileName[NGI_FILE_NAME_MAX];
    ngiExternalModuleType_t moduleType;
    ngiExternalModuleSubType_t moduleSubType;
    char *moduleName, *type, *executablePath;
    pid_t childPid, returnedPid;
    char *moduleSubTypeString;
    int requireChildChild;
    char *invokeArgs[6]; /* path, log-switch, log-file, NULL */
    size_t logLength;
    int typeCount;

    /* Check the arguments */
    assert(module != NULL);
    assert(module->ngem_type != NULL);
    assert(fds != NULL);
    assert(processPid != NULL);

    moduleType = module->ngem_moduleType;
    moduleName = nglExternalModuleTypeName(moduleType);
    moduleSubType = module->ngem_moduleSubType;
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;
    argCur = 0;
    invokeArgs[argCur] = NULL;
    executablePath = NULL;
    defaultExecutablePath = NULL;
    requireChildChild = 0;

    fds[0] = -1;
    fds[1] = -1;
    fds[2] = -1;
    *processPid = 0;
    moduleSubTypeString = "";

    /* log */
    if (moduleType == NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY) {
        if (moduleSubType ==
            NGI_EXTERNAL_MODULE_SUB_TYPE_CLIENT_COMMUNICATION_PROXY) {
            moduleSubTypeString = "Client ";
        } else if (moduleSubType ==
            NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY) {
            moduleSubTypeString = "Remote ";
        }
    }

    ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
        "Creating the process for %s%s %s(%d).\n",
        moduleSubTypeString, moduleName, type, typeCount);

    executablePath = userExecutablePath; /* may be NULL */

    /* Check the External Module executable file */
    result = ngiExternalModuleProgramCheckAccess(
        moduleType, moduleSubType, type, executablePath, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) program access check failed.\n",
            moduleName, type, typeCount);
        goto error;
    }

    if (executablePath == NULL) {
        /* Get the External Module name */
        result = ngiExternalModuleProgramNameGet(
            moduleType, moduleSubType, type, &defaultExecutablePath,
            log, error);
        if ((result == 0) || (defaultExecutablePath == NULL)) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "%s %s(%d) executable name get failed.\n",
                moduleName, type, typeCount);
            goto error;
        }
        executablePath = defaultExecutablePath;
    }

    invokeArgs[argCur++] = executablePath;
    invokeArgs[argCur] = NULL;

    /* Get the log file name */
    if ((logFilePath != NULL) ||
        (logFilePathCommon != NULL)) {

        logLength = 0;
        logFileName[0] = '\0';

        if (logFilePath != NULL) {
            logLength = snprintf(
                logFileName, sizeof(logFileName),
                "%s",
                logFilePath);

        } else if (logFilePathCommon != NULL) {
            logLength = snprintf(
                logFileName, sizeof(logFileName),
                "%s.%s",
                logFilePathCommon,
                type);
        }

        if ((module->ngem_nJobsMax > 0) || (typeCount > 0)) {
            snprintf(
                &logFileName[logLength], sizeof(logFileName) - logLength,
                "-%d", typeCount);
        }

        invokeArgs[argCur++] = NGI_EXTERNAL_MODULE_LOG_FILE_SWITCH;
        invokeArgs[argCur++] = logFileName;
        invokeArgs[argCur] = NULL;

        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) log file name is \"%s\".\n",
            moduleName, type, typeCount, logFileName);
    }

    requireChildChild = 0;
    if (moduleType == NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER) {
        requireChildChild = 1;
    }

#ifdef NGI_THREAD_WAITPID_HAS_BUG
    requireChildChild = 1;
#endif /* NGI_THREAD_WAITPID_HAS_BUG */
    
    /* Create parent and child pipe for child stdin/stdout/stderr */
    result = pipe(childStdin);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "creating stdin pipe failed: %s.\n", strerror(errno));
        goto error;
    }
    result = pipe(childStdout);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "creating stdout pipe failed: %s.\n", strerror(errno));
        goto error;
    }
    result = pipe(childStderr);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "creating stderr pipe failed: %s.\n", strerror(errno));
        goto error;
    }

    fds[0] = childStdin[1];  /* child reads */
    fds[1] = childStdout[0]; /* child writes */
    fds[2] = childStderr[0]; /* child writes */

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "%s %s(%d) pipe = %d/%d/%d (in/out/err).\n",
        moduleName, type, typeCount,
        fds[0], fds[1], fds[2]);

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Invoking %s %s(%d) process by %s.\n",
        moduleName, type, typeCount,
        ((requireChildChild != 0) ? "fork, fork, exec" : "fork, exec"));

    childPid = fork();
    if (childPid < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %s.\n", "fork()", strerror(errno));
        return 0;
    }

    if (childPid == 0) {
        /* child */
        nglExternalModuleProcessInvokeChild(
            executablePath, requireChildChild, invokeArgs,
            childStdin, childStdout, childStderr);

        /* NOT REACHED */
        abort();
    }

    /* parent */

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "%s %s(%d)%s process pid = %ld.\n",
        moduleName, type, typeCount,
        ((requireChildChild != 0) ? " parent" : ""),
        (long)childPid);

    /* Close unnecessary side of parent pipe */
    result = close(childStdin[0]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "closing pipe for stdin read failed: %s.\n",
            strerror(errno));
    }
    result = close(childStdout[1]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "closing pipe for stdout write failed: %s.\n",
            strerror(errno));
    }
    result = close(childStderr[1]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "closing pipe for stderr write failed: %s.\n",
            strerror(errno));
    }

    if (requireChildChild != 0) {
        /* Wait the child, which exits immediately */
        returnedPid = waitpid(childPid, &exitStatus, 0);
        if (returnedPid < 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "waitpid() failed: %ld %s.\n",
                (long)returnedPid, strerror(errno));
            goto error;
        }
     
        result = nglExternalModuleProcessExitStatusPrint(
            module, exitStatus, 0, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Print the Exit code failed.\n");
            goto error;
        }

    } else {
        *processPid = childPid;
    }

    if (defaultExecutablePath != NULL) {
        ngiFree(defaultExecutablePath, log, error);
        defaultExecutablePath = NULL;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (defaultExecutablePath != NULL) {
        ngiFree(defaultExecutablePath, log, NULL);
        defaultExecutablePath = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * External Module child process forked, re-fork and exec External Module.
 */
static void
nglExternalModuleProcessInvokeChild(
    char *externalModuleProgram,
    int requireChildChild,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr)
{
    pid_t childPid;

    /* Do not touch any Ninf-G data. */
    /* Do not return. */

    /* Check the arguments */
    assert(externalModuleProgram != NULL);
    assert(invokeArgs != NULL);
    assert(childStdin != NULL);
    assert(childStdout != NULL);
    assert(childStderr != NULL);

    if (requireChildChild == 0) {
        nglExternalModuleProcessInvokeChildExec(
            externalModuleProgram, invokeArgs,
            childStdin, childStdout, childStderr);

        /* NOT REACHED */
        abort();
    }

    /**
     * child fork() again.
     * Ninf-G client do not wait() the Invoke Server process exit().
     * Thus, Invoke Server process exit() will treated by init process.
     * To do this, the parent process of Invoke Server should exit.
     */

    childPid = fork();
    if (childPid < 0) {
        _exit(1);
    }

    if (childPid == 0) {
        /* child */
        nglExternalModuleProcessInvokeChildExec(
            externalModuleProgram, invokeArgs,
            childStdin, childStdout, childStderr);

        /* NOT REACHED */
        abort();
    }

    /* parent */

    /* Success */
    _exit(0); /* Use _exit() to untouch stdout buffer. */
}

/**
 * External Module child process forked, exec External Module.
 */
static void
nglExternalModuleProcessInvokeChildExec(
    char *externalModuleProgram,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr)
{
    int result;

    /* Do not return. */

    /* Check the arguments */
    assert(externalModuleProgram != NULL);
    assert(invokeArgs != NULL);
    assert(childStdin != NULL);
    assert(childStdout != NULL);
    assert(childStderr != NULL);

    /* Reset signal mask */
    result = ngiSignalManagerSignalMaskReset();
    if (result == 0) {
        _exit(1);
    }

    /* Close unnecessary side of child pipe */
    result = close(childStdin[1]);
    if (result != 0) {
        _exit(1);
    }
    result = close(childStdout[0]);
    if (result != 0) {
        _exit(1);
    }
    result = close(childStderr[0]);
    if (result != 0) {
        _exit(1);
    }

    /* Connect pipe to stdin/stdout/stderr */
    result = dup2(childStdin[0], STDIN_FILENO);
    if (result < 0) {
        _exit(1);
    }
    result = dup2(childStdout[1], STDOUT_FILENO);
    if (result < 0) {
        _exit(1);
    }
    result = dup2(childStderr[1], STDERR_FILENO);
    if (result < 0) {
        _exit(1);
    }

    /* Close copy-from descriptors */
    result = close(childStdin[0]);
    if (result != 0) {
        _exit(1);
    }
    result = close(childStdout[1]);
    if (result != 0) {
        _exit(1);
    }
    result = close(childStderr[1]);
    if (result != 0) {
        _exit(1);
    }

    result = execv(externalModuleProgram, invokeArgs);
    /* If the exec() was successful, the process will not return here. */

    /* error */

    _exit(1);
}

/**
 * ExternalModule: Wait the External Module process.
 */
static int
nglExternalModuleProcessWait(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleProcessWait";
    int signals[2], nSignals, exitStatus, childExited;
    ngiExternalModuleType_t moduleType;
    pid_t childPid, returnedPid;
    int sleepSec, oneSleepSec;
    time_t timeNow, timeEnd;
    char *moduleName, *type;
    char *signalStrings[2];
    int result, i, cont;
    ngiEvent_t *event;
    int typeCount;

    /* Check the arguments */
    assert(module != NULL);

    moduleType = module->ngem_moduleType;
    moduleName = nglExternalModuleTypeName(moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Is wait required? */
    if (module->ngem_processPid == 0) {
        /* log */
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "Wait the %s %s(%d) process is not necessary. return.\n",
            moduleName, type, typeCount);

        /* Success */
        return 1;
    }

    event = module->ngem_manager->ngemm_event;
    childPid = module->ngem_processPid;

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Wait the %s %s(%d) process (pid %ld).\n",
        moduleName, type, typeCount, (long)childPid);

    signals[0] = SIGTERM;
    signalStrings[0] = "SIGTERM";

    signals[1] = SIGKILL;
    signalStrings[1] = "SIGKILL";

    nSignals = 2;

    childExited = 0;

    for (i = 0; i < nSignals; i++) {
        sleepSec = NGI_EXTERNAL_MODULE_PROCESS_WAIT_TIME;
        timeEnd = time(NULL) + sleepSec;
        
        cont = 1;
        while (cont) {
            /* Check the child. */
            returnedPid = waitpid(childPid, &exitStatus, WNOHANG);
            if (returnedPid < 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "%s failed: %ld %s.\n",
                    "waitpid()", (long)returnedPid, strerror(errno));
                goto error;

            } else if (returnedPid == 0) {
                /* Not changed. */

            } else if (returnedPid != childPid) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Returned pid is wrong. target pid %ld returned %ld.\n",
                    (long) childPid, (long)returnedPid);
                goto error;
            } else {
                /* Child exited successfully. */
                assert(returnedPid == childPid);

                result = nglExternalModuleProcessExitStatusPrint(
                    module, exitStatus, 1, log, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                        "Print the Exit code failed.\n");
                    goto error;
                }

                cont = 0;
                childExited = 1;
                break;
            }

            timeNow = time(NULL);
            if (timeNow >= timeEnd) {
                cont = 0;
                break;
            }

            oneSleepSec = NGI_EXTERNAL_MODULE_PROCESS_EXIT_CHECK_PERIOD;

            result = ngiThreadSleep(oneSleepSec, 1, event, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Sleep failed.\n");
                goto error;
            }
        }

        if (childExited != 0) {
            break;
        }

        /* Timeout */       

        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "Timeout %d sec occur.\n", sleepSec);

        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "Send the %s %s(%d) process (pid %ld) to signal %s.\n",
            moduleName, type, typeCount, (long)childPid,
            signalStrings[i]);

        result = kill(childPid, signals[i]);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "%s failed: %s.\n",
                "kill()", strerror(errno));
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
 * ExternalModule: Print the exit status of External Module.
 */
static int
nglExternalModuleProcessExitStatusPrint(
    ngiExternalModule_t *module,
    int exitStatus,
    int isExternalModuleProcess,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleProcessExitStatusPrint";
    char *moduleName, *type, *selfOrParent;
    ngiExternalModuleType_t moduleType;
    int exitCode, signalCode;
    int typeCount;

    /* Check the arguments */
    assert(module != NULL);

    moduleType = module->ngem_moduleType;
    moduleName = nglExternalModuleTypeName(moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    selfOrParent = "";
    if (isExternalModuleProcess == 0) {
        selfOrParent = " parent";
    }

    if (WIFEXITED(exitStatus)) {
        exitCode = WEXITSTATUS(exitStatus);

        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d)%s process exit by %d (status = 0x%x).\n",
            moduleName, type, typeCount, selfOrParent,
            exitCode, exitStatus);
    }

    if (WIFSIGNALED(exitStatus)) {
        signalCode = WTERMSIG(exitStatus);

        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d)%s process terminated by signal %d (status = 0x%x).\n",
            moduleName, type, typeCount, selfOrParent,
            signalCode, exitStatus);
    }

    /* Success */
    return 1;
}


/**
 * Check whether the External Module program accessible or not.
 * path == NULL : use default path.
 */
int
ngiExternalModuleProgramCheckAccess(
    ngiExternalModuleType_t moduleType,
    ngiExternalModuleSubType_t moduleSubType,
    char *type,
    char *modulePath,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleProgramCheckAccess";
    char *defaultPath, *programPath, *moduleName;
    int result;

    /* Check the arguments */
    if ((moduleType <= NGI_EXTERNAL_MODULE_TYPE_NONE) ||
        (moduleType >= NGI_EXTERNAL_MODULE_TYPE_NOMORE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Module Type (%d) is invalid.\n",
            moduleType);
        return 0;
    }
    if ((moduleSubType <= NGI_EXTERNAL_MODULE_SUB_TYPE_NONE) ||
        (moduleSubType >= NGI_EXTERNAL_MODULE_SUB_TYPE_NOMORE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Module Sub Type (%d) is invalid.\n",
            moduleSubType);
        return 0;
    }

    programPath = NULL;
    defaultPath = NULL;
    moduleName = nglExternalModuleTypeName(moduleType);

    if (modulePath != NULL) {
        programPath = modulePath;

    } else {
        /* Get the program name */
        result = ngiExternalModuleProgramNameGet(
            moduleType, moduleSubType, type, &defaultPath, log, error);
        if ((result == 0) || (defaultPath == NULL)){
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Get the %s %s program name failed.\n",
                moduleName, type);
            goto error;
        }
        programPath = defaultPath;
    }

    result = access(programPath, X_OK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s \"%s\" access error: %s.\n",
            moduleName, type, programPath, strerror(errno));
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "%s %s program \"%s\" available.\n",
        moduleName, type, programPath);

    if (defaultPath != NULL) {
        ngiFree(defaultPath, log, error);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (defaultPath != NULL) {
        ngiFree(defaultPath, log, NULL);
    }

    /* Failed */
    return 0;
}

/**
 * Get the External Module program name.
 * Note: caller must be free the allocated name string.
 */
int
ngiExternalModuleProgramNameGet(
    ngiExternalModuleType_t moduleType,
    ngiExternalModuleSubType_t moduleSubType,
    char *type,
    char **name,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleProgramNameGet";
    char *ngDir, *base, *programName;
    int length;

    /* Check the arguments */
    assert(moduleType > NGI_EXTERNAL_MODULE_TYPE_NONE);
    assert(moduleType < NGI_EXTERNAL_MODULE_TYPE_NOMORE);
    assert(moduleSubType > NGI_EXTERNAL_MODULE_SUB_TYPE_NONE);
    assert(moduleSubType < NGI_EXTERNAL_MODULE_SUB_TYPE_NOMORE);
    assert(type != NULL);
    assert(name != NULL);

    *name = NULL;

    ngDir = NULL;
    base = NULL;
    programName = NULL;

    /* Get the NG_DIR. */
    if ((moduleType == NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY) &&
        (moduleSubType ==
        NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY)) {

        /* From compile time installation dir. */
        ngDir = NGI_NG_DIR;
    } else {

        /* From environment variable. */
        ngDir = getenv(NGI_NG_DIR_ENV_NAME);
        if (ngDir == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Environment variable $%s undefined.\n",
                NGI_NG_DIR_ENV_NAME);
            return 0; 
        }
    }
    assert(ngDir != NULL);

    if (moduleType == NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER) {
        if (moduleSubType != NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unknown module sub type %d.\n", moduleSubType);
            return 0; 
        }

        base = NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_INVOKE_SERVER;

    } else if (moduleType == NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY) {
        switch(moduleSubType) {
        case NGI_EXTERNAL_MODULE_SUB_TYPE_CLIENT_COMMUNICATION_PROXY:
            base = NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_CLIENT_COMM_PROXY;
            break;
        case NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY:
            base = NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_REMOTE_COMM_PROXY;
            break;
        default:
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unknown module sub type %d for Communication Proxy.\n",
                moduleSubType);
            return 0; 
        }

    } else if (moduleType == NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE) {
        if (moduleSubType != NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unknown module sub type %d.\n", moduleSubType);
            return 0; 
        }

        base = NGI_EXTERNAL_MODULE_PROGRAM_NAME_BASE_INFORMATION_SERVICE;

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module type %d invalid.\n",
            moduleType);
        return 0; 
    }
    assert(base != NULL);

    length = strlen(NGI_EXTERNAL_MODULE_PROGRAM_PATH_FORMAT);
    length += strlen(ngDir);
    length += strlen(base);
    length += strlen(type);
    length++;

    programName = ngiCalloc(length, sizeof(char), log, error);
    if (programName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "calloc for External Module program name failed.\n");
        return 0; 
    }

    snprintf(programName, length,
        NGI_EXTERNAL_MODULE_PROGRAM_PATH_FORMAT,
        ngDir, base, type);

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "The installed %s %s program path is \"%s\"\n",
        nglExternalModuleTypeName(moduleType), type, programName);

    *name = programName;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Check the Available External Module and return it.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
int
ngiExternalModuleAvailableGet(
    ngiExternalModuleManager_t *extMng,
    ngiExternalModuleType_t moduleType,
    char *type,
    ngiExternalModule_t **module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleAvailableGet";
    ngiExternalModule_t *cur;
    int typeCount, subError;
    char *moduleName;

    *module = NULL;

    cur = NULL;
    typeCount = -1;
    subError = NG_ERROR_NO_ERROR;
    moduleName = nglExternalModuleTypeName(moduleType);

    /* Check the arguments */
    if ((extMng == NULL) || (type == NULL) || (module == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((extMng == NULL) ? "External Module" :
            ((type == NULL) ? "type" :
            ((module == NULL) ? "module" : "unknown"))));
        return 0;
    }

    /* Find the External Module. */
    cur = nglExternalModuleManagerGetExternalModule(
        extMng, moduleType, type, -1, log, &subError);
    if (cur == NULL) {
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module for %s %s not found, create new %s.\n",
            moduleName, type, moduleName);

        /* Not found */
        *module = NULL;

        return 1;
    }

    typeCount = cur->ngem_typeCount;

    if (cur->ngem_serving == 0) {
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) is requested to retire, create new %s.\n",
            moduleName, type, typeCount, moduleName);

        /* Not found */
        *module = NULL;

        return 1;
    }

    if ((cur->ngem_nJobsMax != 0) &&
        ((cur->ngem_nJobsStart >= cur->ngem_nJobsMax) ||
        (cur->ngem_nJobsStop >= cur->ngem_nJobsMax) ||
        (cur->ngem_nJobsDone >= cur->ngem_nJobsMax))) {
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) jobs exceeded (%d jobs), create new %s.\n",
            moduleName, type, typeCount, cur->ngem_nJobsMax, moduleName);

        /* Not found */
        *module = NULL;

        return 1;
    }

    if ((cur->ngem_valid == 0) || (cur->ngem_working == 0)) {
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) was dead, create new %s.\n",
            moduleName, type, typeCount, moduleName);

        /* Not found */
        *module = NULL;

        return 1;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "%s %s(%d) available.\n",
        moduleName, type, typeCount);

    *module = cur;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Is valid?
 */
int
ngiExternalModuleIsValid(
    ngiExternalModule_t *module,
    int *valid,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleIsValid";
    int result;

    /* Check the arguments */
    if ((module == NULL) || (valid == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((valid == NULL) ? "valid" : "unknown")));
        return 0;
    }

    *valid = 1;

    /* Wait until Unusable finish. */

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        return 0;
    }

    /* Check the flag */
    if (module->ngem_valid == 0) {
        *valid = 0;
    }

    if (module->ngem_working == 0) {
        *valid = 0;
    }

    /* Unlock */
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * ExternalModule: Set the owner.
 */
int
ngiExternalModuleOwnerSet(
    ngiExternalModule_t *module,
    void *owner,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleOwnerSet";

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    /* Perhaps, owner may NULL. */

    module->ngem_owner = owner;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Get the owner.
 */
void *
ngiExternalModuleOwnerGet(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleOwnerGet";

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    if (module->ngem_owner == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module owner is not found.\n");
        return NULL;
    }

    /* Success */
    return module->ngem_owner;
}

/**
 * ExternalModule: Job was started.
 */
int
ngiExternalModuleJobStarted(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleJobStarted";
    char *moduleName, *type;
    int typeCount;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Check the state */
    if ((module->ngem_nJobsMax != 0) &&
        (module->ngem_nJobsStart >= module->ngem_nJobsMax)) {

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid count of jobs for %s %s(%d),"
            " started=%d, stopped=%d, done=%d, max=%d.\n",
            moduleName, type, typeCount,
            module->ngem_nJobsStart + 1,
            module->ngem_nJobsStop,
            module->ngem_nJobsDone,
            module->ngem_nJobsMax);
        return 0;
    }

    module->ngem_nJobsStart++;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Job was stopped.
 */
int
ngiExternalModuleJobStopped(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleJobStopped";
    char *moduleName, *type;
    int typeCount;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Check the state */
    if (module->ngem_nJobsStop >= module->ngem_nJobsStart) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid count of jobs for %s %s(%d),"
            " started=%d, stopped=%d, done=%d, max=%d.\n",
            moduleName, type, typeCount,
            module->ngem_nJobsStart,
            module->ngem_nJobsStop + 1,
            module->ngem_nJobsDone,
            module->ngem_nJobsMax);
        return 0;
    }

    module->ngem_nJobsStop++;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Job was done.
 */
int
ngiExternalModuleJobDone(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleJobDone";
    char *moduleName, *type;
    int typeCount;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Check the state */
    if (module->ngem_nJobsDone >= module->ngem_nJobsStart) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid count of jobs for %s %s(%d),"
            " started=%d, stopped=%d, done=%d, max=%d.\n",
            moduleName, type, typeCount,
            module->ngem_nJobsStart,
            module->ngem_nJobsStop,
            module->ngem_nJobsDone + 1,
            module->ngem_nJobsMax);
        return 0;
    }

    module->ngem_nJobsDone++;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Retire.
 */
int
ngiExternalModuleRetire(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleRetire";

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            "External Module");
        return 0;
    }

    module->ngem_serving = 0;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Get the ID of External Module.
 */
int
ngiExternalModuleIDget(
    ngiExternalModule_t *module,
    int *ID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleIDget";

    /* Check the arguments */
    if ((module == NULL) || (ID == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((ID     == NULL) ? "ID" : "unknown")));
        return 0;
    }

    *ID = module->ngem_ID;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Get the ID of Invoke Server or Communication Proxy,
 *   or Information Service.
 */
int
ngiExternalModuleModuleTypeIDget(
    ngiExternalModule_t *module,
    int *typeID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleModuleTypeIDget";

    /* Check the arguments */
    if ((module == NULL) || (typeID == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((typeID == NULL) ? "typeID" : "unknown")));
        return 0;
    }

    *typeID = module->ngem_moduleID;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Get the Count(ID) of each module.
 */
int
ngiExternalModuleModuleTypeCountGet(
    ngiExternalModule_t *module,
    int *typeCount,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleModuleTypeCountGet";

    /* Check the arguments */
    if ((module == NULL) || (typeCount == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((typeCount == NULL) ? "typeCount" : "unknown")));
        return 0;
    }

    *typeCount = module->ngem_typeCount;

    /* Success */
    return 1;
}

/**
 * ExternalModuleReader: Initialize.
 */
static int
nglExternalModuleReaderInitialize(
    ngiExternalModuleReader_t *reader,
    ngiExternalModuleReaderType_t readerType,
    ngiExternalModule_t *module,
    ngiEvent_t *event,
    int fd,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReaderInitialize";
    ngiIOhandle_t *handle;
    int result;

    /* Check the arguments */
    assert(reader != NULL);

    handle = NULL;

    nglExternalModuleReaderInitializeMember(reader);

    result = ngiIOhandleCallbackWaiterInitialize(
        &reader->ngemr_callbackWaiter,
        module->ngem_manager->ngemm_event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Callback Waiter failed.");
        goto error;
    }

    reader->ngemr_readerType = readerType;
    reader->ngemr_externalModule = module;

    /* Open the pipe handle. */
    handle = ngiIOhandleConstruct(event, log, error);
    if (handle == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Construct the I/O handle failed.\n");
        goto error;
    }

    result = ngiIOhandleFdOpen(handle, fd, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Open the pipe handle for fd %d failed.\n", fd);
        goto error;
    }
    reader->ngemr_handle = handle;
    reader->ngemr_handleOpen = 1;

    result = nglExternalModuleReadBufferInitialize(
        &reader->ngemr_readBuffer, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the read buffer failed.\n");
        goto error;
    }

    reader->ngemr_readingNextLine = 0;
    reader->ngemr_isMultiLine = 0;
    reader->ngemr_lines = NULL;

    reader->ngemr_valid = 1;

    /* Register the I/O handle callback */
    if (readerType == NGI_EXTERNAL_MODULE_READER_TYPE_REPLY) {
        result = ngiIOhandleCallbackWaiterStart(
            &reader->ngemr_callbackWaiter, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Start the Callback Waiter failed.\n");
            goto error;
        }

        result = ngiIOhandleReadCallbackRegister(
            handle, nglExternalModuleReaderCallback,
            reader, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Register the callback failed.\n");
            goto error;
        }
        reader->ngemr_callbackRegistered = 1;

        /**
         * Note: I/O handle callback for Notify reader is registered
         * after the Notify Callback is registered.
         */
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReader: Finalize.
 */
static int
nglExternalModuleReaderFinalize(
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReaderFinalize";
    int result;

    /* Check the arguments */
    assert(reader != NULL);

    result = ngiIOhandleClose(
        reader->ngemr_handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Close the handle failed.\n");
        goto error;
    }

    result = ngiIOhandleCallbackWaiterWait(
        &reader->ngemr_callbackWaiter, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Wait the Callback Waiter failed.\n");
        goto error;
    }

    result = ngiIOhandleDestruct(
        reader->ngemr_handle, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the handle failed.\n");
        goto error;
    }
    reader->ngemr_handle = NULL;

    result = nglExternalModuleReadBufferFinalize(
        &reader->ngemr_readBuffer, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the read buffer failed.\n");
        goto error;
    }

    result = ngiIOhandleCallbackWaiterFinalize(
        &reader->ngemr_callbackWaiter, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Callback Waiter failed.");
        goto error;
    }

    nglExternalModuleReaderInitializeMember(reader);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReader: Initialize the members.
 */
static void
nglExternalModuleReaderInitializeMember(
    ngiExternalModuleReader_t *reader)
{
    /* Check the arguments */
    assert(reader != NULL);

    reader->ngemr_readerType = NGI_EXTERNAL_MODULE_READER_TYPE_NONE;
    reader->ngemr_externalModule = NULL;
    reader->ngemr_valid = 0;
    reader->ngemr_handle = NULL;
    reader->ngemr_handleOpen = 0;
    reader->ngemr_callbackRegistered = 0;
    reader->ngemr_callbackExecuting = 0;
    reader->ngemr_readingNextLine = 0;
    reader->ngemr_isMultiLine = 0;
    reader->ngemr_lines = NULL;

    nglExternalModuleReadBufferInitializeMember(
        &reader->ngemr_readBuffer);
}

/**
 * ExternalModuleReader: Callback.
 */
static int
nglExternalModuleReaderCallback(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    static const char fName[] = "nglExternalModuleReaderCallback";
    int result, *error, errorEntity, locked, proceeded;
    ngiExternalModuleReadBuffer_t *readBuffer;
    int readCallbackRegister, cancelRequested;
    ngiExternalModuleReader_t *reader;
    ngiExternalModule_t *module;
    size_t length, readNbytes;
    ngiIOhandle_t *handle;
    char *buf, *stateName, *readerTypeName;
    int callbackEnd;
    ngLog_t *log;

    /* Check the arguments */
    assert(cbArg != NULL);
    assert(argState > NGI_IOHANDLE_STATE_NONE);
    assert(argState < NGI_IOHANDLE_STATE_NOMORE);

    reader = (ngiExternalModuleReader_t *)cbArg;
    readBuffer = &reader->ngemr_readBuffer;
    module = reader->ngemr_externalModule;
    handle = reader->ngemr_handle;
    readerTypeName = NULL;
    stateName = NULL;
    log = module->ngem_log;

    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    callbackEnd = 1;
    locked = 0;
    proceeded = 0;
    readCallbackRegister = 0;
    cancelRequested = 0;

    /* Set Reader type name. */
    if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_REPLY) { 
        readerTypeName = "reply";
    } else if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY) { 
        readerTypeName = "notify";
    } else {
        abort();
    }
    assert(readerTypeName != NULL);

    /* Set arg state name. */
    if (argState == NGI_IOHANDLE_STATE_NORMAL) {
        stateName = "normal";
    } else if (argState == NGI_IOHANDLE_STATE_CLOSED) {
        stateName = "closed";
    } else if (argState ==  NGI_IOHANDLE_STATE_CANCELED) {
        stateName = "canceled";
    } else {
        abort();
    }
    assert(stateName != NULL);

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "The %s reader callback (state=%s) for %s %s(%d) handle %d.%d.\n",
        readerTypeName, stateName,
        nglExternalModuleTypeName(module->ngem_moduleType),
        module->ngem_type,
        module->ngem_typeCount,
        reader->ngemr_handle->ngih_eventDriverID,
        reader->ngemr_handle->ngih_id);

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    reader->ngemr_callbackExecuting = 1;
    reader->ngemr_callbackRegistered = 0;

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    if (argState != NGI_IOHANDLE_STATE_NORMAL) {

        if ((module->ngem_working == 0) ||
            (argState == NGI_IOHANDLE_STATE_CANCELED)) {
            ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
                "%s reader state is %d (%s).\n",
                readerTypeName, argState, stateName);
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "%s reader state is %d (%s).\n",
                readerTypeName, argState, stateName);
        }

        /* Process Error. */
        result = nglExternalModuleReaderProcessError(
            module, reader, module->ngem_working, argState, 0, 0,
            log, error);
        if (result == 0) {
            goto error2;
        }

        goto finish;
    }

    assert(readBuffer->ngemrb_buf != NULL);
    assert(readBuffer->ngemrb_current < readBuffer->ngemrb_bufSize);
    buf = &readBuffer->ngemrb_buf[readBuffer->ngemrb_current];
    length = readBuffer->ngemrb_bufSize - readBuffer->ngemrb_current;
    readNbytes = 0;

    result = ngiIOhandleRead(
        handle, buf, length, 1, &readNbytes, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "read failed.\n");
        goto error;
    }

    if (readNbytes == 0) {
        /* Process Error. */
        result = nglExternalModuleReaderProcessError(
            module, reader, module->ngem_working, argState, 1, 0,
            log, error);
        if (result == 0) {
            goto error2;
        }

        goto finish;
    }

    /* Process read result. */
    proceeded = 1;
    result = nglExternalModuleReaderProcessCharacters(
        module, reader, buf, length, readNbytes, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Process read characters failed.\n");
        goto error;
    }

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    readCallbackRegister = 0;
    cancelRequested = 0;

    if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_REPLY) {
        readCallbackRegister = 1;

    } else if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY) {

        if (module->ngem_notifyStatus.ngemns_callbackUnregisterRequested != 0) {
            readCallbackRegister = 0;
            cancelRequested = 1;
            callbackEnd = 1;

        } else if (module->ngem_notifyStatus.ngemns_callbackRegistered != 0) {
            if (reader->ngemr_callbackRegistered != 0) {
                callbackEnd = 0;
            } else {
                readCallbackRegister = 1;
            }
        }

    } else {
        abort();
    }

    /* Register the Callback. */
    if (readCallbackRegister != 0) {
        assert(cancelRequested == 0);

        result = ngiIOhandleReadCallbackRegister(
            handle, nglExternalModuleReaderCallback,
            reader, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Register the callback failed.\n");
            goto error;
        }
        reader->ngemr_callbackRegistered = 1;
        callbackEnd = 0;
    }

    /* Call the Notify Cancel Callback. */
    if (cancelRequested != 0) {
        assert(readCallbackRegister == 0);

        /* Unlock */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the Mutex failed.\n");
            goto error;
        }

        /* Process Error. */
        result = nglExternalModuleReaderProcessError(
            module, reader, module->ngem_working, argState, 0,
            cancelRequested, log, error);
        if (result == 0) {
            goto error2;
        }

        /* Lock */
        result = ngiMutexLock(&module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Lock the Mutex failed.\n");
            goto error2;
        }
        locked = 1;
    }

finish:

    reader->ngemr_callbackExecuting = 0;

    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &reader->ngemr_callbackWaiter, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "End the Callback Waiter failed.\n");
            goto error;
        }
    }

    /* Unlock */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the Mutex failed.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    if (proceeded == 0) {
        /* Process Error. */
        result = nglExternalModuleReaderProcessError(
            module, reader, module->ngem_working, argState, 0, 0,
            log, error);
    }

error2:

    reader->ngemr_callbackExecuting = 0;

    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &reader->ngemr_callbackWaiter, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "End the Callback Waiter failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReader: Process Error.
 */
static int
nglExternalModuleReaderProcessError(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    int isWorking,
    ngiIOhandleState_t handleState,
    int isEOF,
    int cancelRequested,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReaderProcessError";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    ngiExternalModuleNotifyStatus_t *notifyStatus;
    ngiExternalModuleNotifyState_t notifyState;
    int isError, result;

    /* Check the arguments */
    assert(module != NULL);
    assert(reader != NULL);
    assert(handleState > NGI_IOHANDLE_STATE_NONE);
    assert(handleState < NGI_IOHANDLE_STATE_NOMORE);

    isError = 0;
    requestReply = &module->ngem_requestReply;
    notifyStatus = &module->ngem_notifyStatus;
    notifyState = NGI_EXTERNAL_MODULE_NOTIFY_NONE;

    if ((handleState == NGI_IOHANDLE_STATE_CLOSED) || (isEOF != 0)) {
        reader->ngemr_handleOpen = 0;
    }

    if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_REPLY) {

        if (isWorking != 0) {
            requestReply->ngemrrs_errorOccurred = 1;
            requestReply->ngemrrs_errorCode = NG_ERROR_PIPE;
            isError = 1;
        }

        result = nglExternalModuleReplySet(
            module, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Notify the Reply failed.\n");
            goto error;
        }

    } else if (reader->ngemr_readerType ==
        NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY) {

        notifyStatus->ngemns_callbackUnregisterRequested = 0;

        notifyState = NGI_EXTERNAL_MODULE_NOTIFY_ERROR;

        if (cancelRequested != 0) {
            notifyState = NGI_EXTERNAL_MODULE_NOTIFY_CANCELED;

        } else if ((handleState == NGI_IOHANDLE_STATE_CLOSED) ||
            (isEOF != 0)) {
            notifyState = NGI_EXTERNAL_MODULE_NOTIFY_CLOSED;

        } else if (handleState == NGI_IOHANDLE_STATE_CANCELED) {
            notifyState = NGI_EXTERNAL_MODULE_NOTIFY_CANCELED;

        } else {
            notifyState = NGI_EXTERNAL_MODULE_NOTIFY_ERROR;
        }

        result = nglExternalModuleNotifyCallbackCall(
            module, notifyState, NULL, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Call the Notify callback failed.\n");
            goto error;
        }

    } else {
        abort();
    }

    if (isError != 0) {
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReader: Process Characters.
 */
static int
nglExternalModuleReaderProcessCharacters(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    char *buf,
    size_t length,
    size_t readNbytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReaderProcessCharacters";
    ngiExternalModuleReadBuffer_t *rb;
    int lineCompleteOnce, lineComplete;
    size_t i, nextLineStart, copyFromStart, copyFromEnd, copySize;
    int result;
    char *terminateStr, *oldBuf, *newBuf;
    size_t terminateSize, oldBufSize, newBufSize;
    char *p;

    /* Check the arguments */
    assert(module != NULL);
    assert(reader != NULL);
    assert(buf != NULL);
    assert(length > 0);

    rb = &reader->ngemr_readBuffer;
    terminateStr = NGI_EXTERNAL_MODULE_LINE_TERMINATOR_STR;
    terminateSize = NGI_EXTERNAL_MODULE_LINE_TERMINATOR_SIZE;

    lineCompleteOnce = 0;

    for (i = 0; i < readNbytes; i++) {
        p = &buf[i];
        lineComplete = 0;
        nextLineStart = 0;

        /* Check terminate character. */
        if (*p == terminateStr[rb->ngemrb_termMatchCount]) {
            if (rb->ngemrb_termMatchCount == 0) {
                rb->ngemrb_termMatchStart =
                    rb->ngemrb_current + i;
            }
            rb->ngemrb_termMatchCount++;
        } else {
            if (*p == terminateStr[0]) {
                rb->ngemrb_termMatchStart = rb->ngemrb_current + i;
                rb->ngemrb_termMatchCount = 1;
            } else {
                rb->ngemrb_termMatchStart = 0;
                rb->ngemrb_termMatchCount = 0;
            }
        }

        if (rb->ngemrb_termMatchCount >= terminateSize) {
            lineComplete = 1;
            lineCompleteOnce = 1;

            rb->ngemrb_buf[rb->ngemrb_termMatchStart] = '\0';

            rb->ngemrb_termMatchCount = 0;
            rb->ngemrb_termMatchStart = 0;

            nextLineStart = rb->ngemrb_current + i + 1;
        }

        if (lineComplete != 0) {

            /* Process one line. */
            result = nglExternalModuleReaderProcessLine(
                module, reader,
                &rb->ngemrb_buf[rb->ngemrb_lineStart],
                log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Process line failed.\n");
                goto error;
            }

            rb->ngemrb_lineStart = nextLineStart;
        }
    }

    if (lineCompleteOnce != 0) {

        /* Remove already proceed buffers. */
        copyFromStart = rb->ngemrb_lineStart;
        copyFromEnd = rb->ngemrb_current + readNbytes;
        assert(copyFromEnd <= rb->ngemrb_bufSize);

        if (copyFromStart < copyFromEnd) {
            copySize = copyFromEnd - copyFromStart;

            for (i = 0; i < copySize; i++) {
                rb->ngemrb_buf[i] = rb->ngemrb_buf[copyFromStart + i];
            }
            rb->ngemrb_current = copySize;
        } else {
            rb->ngemrb_current = 0;
        }
        if (rb->ngemrb_termMatchCount > 0) {
            rb->ngemrb_termMatchStart -= copyFromStart;
        }
        rb->ngemrb_lineStart = 0;

    } else {
        rb->ngemrb_current += readNbytes;
    }

    assert(rb->ngemrb_current <= rb->ngemrb_bufSize);
    if (rb->ngemrb_current == rb->ngemrb_bufSize) {

        /* Buffer full, renew buffer. */
        oldBuf = rb->ngemrb_buf;
        oldBufSize = rb->ngemrb_bufSize;

        newBufSize = oldBufSize * 2;

        newBuf = ngiCalloc(newBufSize, sizeof(char), log, error);
        if (newBuf == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Allocate the storage for read buffer failed.\n");
            goto error;
        }

        memcpy(newBuf, oldBuf, oldBufSize);

        result = ngiFree(oldBuf, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Deallocate the storage for read buffer failed.\n");
            goto error;
        }
        oldBuf = NULL;
        oldBufSize = 0;

        rb->ngemrb_buf = newBuf;
        rb->ngemrb_bufSize = newBufSize;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReader: Process Line.
 */
static int
nglExternalModuleReaderProcessLine(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    char *line,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReaderProcessLine";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    ngiExternalModuleNotifyStatus_t *notifyStatus;
    char *moduleName, *type, *readerTypeName, *notifyName;
    int result, typeCount, isComplete, multiLine, isEnd;
    ngiLineList_t *lines;

    /* Check the arguments */
    assert(module != NULL);
    assert(reader != NULL);
    assert(line != NULL);

    requestReply = &module->ngem_requestReply;
    notifyStatus = &module->ngem_notifyStatus;
    notifyName = NULL;
    lines = NULL;

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;
    readerTypeName = NULL;

    switch(reader->ngemr_readerType) {
    case NGI_EXTERNAL_MODULE_READER_TYPE_REPLY:
        readerTypeName = "Reply";
        break;
    case NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY:
        readerTypeName = "Notify";
        break;
    default:
        abort();
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
        "Got the %s \"%s\" from %s %s(%d).\n",
        readerTypeName, line, moduleName, type, typeCount);

    isComplete = 0;
    multiLine = 0;

    if (reader->ngemr_readingNextLine == 0) {
        assert(reader->ngemr_lines == NULL);

        switch(reader->ngemr_readerType) {
        case NGI_EXTERNAL_MODULE_READER_TYPE_REPLY:

            result = nglExternalModuleReplyFirstLineParse(
                line, requestReply, &multiLine, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Parse the reply failed.\n");
                goto error;
            }
            break;

        case NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY:

            result = nglExternalModuleNotifyFirstLineParse(
                line,
                module->ngem_nMultiLineNotify,
                module->ngem_multiLineNotify,
                notifyStatus, &multiLine, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Parse the reply failed.\n");
                goto error;
            }
            break;

        default:
            abort();
        }

        if (multiLine != 0) {
            reader->ngemr_isMultiLine = 1;

            lines = ngiLineListConstruct(log, error);
            if (lines == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Construct the Line List failed.\n");
                goto error;
            }
            reader->ngemr_lines = lines;
            reader->ngemr_readingNextLine = 1;

        } else {
            reader->ngemr_isMultiLine = 0;
            reader->ngemr_readingNextLine = 0;
            isComplete = 1;
        }

    } else {
        assert(reader->ngemr_lines != NULL);

        isEnd = 0;

        switch(reader->ngemr_readerType) {
        case NGI_EXTERNAL_MODULE_READER_TYPE_REPLY:

            result = nglExternalModuleReplyMultiLineEndCheck(
                line, &isEnd, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Check the multi line Reply failed.\n");
                goto error;
            }

            break;

        case NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY:

            notifyName = notifyStatus->ngemns_notifyName;

            result = nglExternalModuleNotifyMultiLineEndCheck(
                line, notifyName, &isEnd, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Check the multi line Notify failed.\n");
                goto error;
            }

            break;

        default:
            abort();
        }
        
        if (isEnd == 0) {
            result = ngiLineListAppend(
                reader->ngemr_lines, line, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Append the reply line failed.\n");
                goto error;
            }

            reader->ngemr_readingNextLine = 1;
        } else {
            reader->ngemr_readingNextLine = 0;

            isComplete = 1;
        }
    }

    if (isComplete != 0) {
        switch(reader->ngemr_readerType) {
        case NGI_EXTERNAL_MODULE_READER_TYPE_REPLY:

            result = nglExternalModuleReplyProcess(
                module, reader, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Process the Reply failed.\n");
                goto error;
            }
            break;

        case NGI_EXTERNAL_MODULE_READER_TYPE_NOTIFY:

            result = nglExternalModuleNotifyProcess(
                module, reader, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Process the Notify failed.\n");
                goto error;
            }
            break;

        default:
            abort();
        }

        reader->ngemr_isMultiLine = 0;
        reader->ngemr_lines = NULL; /* Already Proceeded. */
        reader->ngemr_readingNextLine = 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Parse the first line of Reply.
 */
static int
nglExternalModuleReplyFirstLineParse(
    char *target,
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    int *multiLine,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReplyFirstLineParse";
    char *msg, *cur;

    /* Check the arguments */
    assert(target != NULL);
    assert(requestReply != NULL);
    assert(multiLine != NULL);

    *multiLine = 0;
    msg = NULL;
    cur = NULL;

    requestReply->ngemrrs_success = 0;
    requestReply->ngemrrs_message = NULL;
    requestReply->ngemrrs_lines = NULL;

    cur = target;

    /* Skip space. */
    while(isspace((int)*cur)) {
        cur++;
    } 

    if (strncmp(cur, NGI_EXTERNAL_MODULE_REPLY_MULTILINE,
        strlen(NGI_EXTERNAL_MODULE_REPLY_MULTILINE)) == 0) {

        requestReply->ngemrrs_success = 1;
        requestReply->ngemrrs_message = NULL;
        *multiLine = 1;

        cur += strlen(NGI_EXTERNAL_MODULE_REPLY_MULTILINE);

    } else if (strncmp(cur, NGI_EXTERNAL_MODULE_REPLY_SUCCESS,
        strlen(NGI_EXTERNAL_MODULE_REPLY_SUCCESS)) == 0) {

        requestReply->ngemrrs_success = 1;
        requestReply->ngemrrs_message = NULL;
        *multiLine = 0;

        cur += strlen(NGI_EXTERNAL_MODULE_REPLY_SUCCESS);

        /* Skip space. */
        while(isspace((int)*cur)) {
            cur++;
        } 

        if (*cur != '\0') {
            msg = ngiStrdup(cur, log, error);
            if (msg == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Duplicate the reply message string failed.\n");
                goto error;
            }
            requestReply->ngemrrs_message = msg;
        }

    } else if (strncmp(cur, NGI_EXTERNAL_MODULE_REPLY_FAILURE,
        strlen(NGI_EXTERNAL_MODULE_REPLY_FAILURE)) == 0) {

        requestReply->ngemrrs_success = 0;
        requestReply->ngemrrs_message = NULL;
        *multiLine = 0;

        cur += strlen(NGI_EXTERNAL_MODULE_REPLY_FAILURE);

        /* Skip space. */
        while(isspace((int)*cur)) {
            cur++;
        } 

        if (*cur != '\0') {
            /* Copy the error message. */
            msg = ngiStrdup(cur, log, error);
            if (msg == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Duplicate the reply message string failed.\n");
                goto error;
            }
            requestReply->ngemrrs_message = msg;
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
 * ExternalModule: Check the end of Multi line Reply.
 */
static int
nglExternalModuleReplyMultiLineEndCheck(
    char *target,
    int *isEnd,
    ngLog_t *log,
    int *error)
{
    char *cur, *cmp;

    /* Check the arguments */
    assert(target != NULL);
    assert(isEnd != NULL);

    *isEnd = 0;

    cur = target;

    cmp = NGI_EXTERNAL_MODULE_REPLY_MULTI_END;
    if (strncmp(cur, cmp, strlen(cmp)) != 0) {

        *isEnd = 0;
        return 1;
    }

    cur += strlen(cmp);

    if ((*cur != '\0') && (!isspace((int)*cur))) {
        *isEnd = 0;
        return 1;
    }
    
    *isEnd = 1;

    /* Success */
    return 1;
}

/**
 * ExternalModule: Process the Reply.
 */
static int
nglExternalModuleReplyProcess(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReplyProcess";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result;

    /* Check the arguments */
    assert(module != NULL);
    assert(reader != NULL);

    requestReply = &module->ngem_requestReply;

    if (reader->ngemr_isMultiLine != 0) {
        requestReply->ngemrrs_lines = reader->ngemr_lines;

        reader->ngemr_isMultiLine = 0;
        reader->ngemr_lines = NULL;
    }

    result = nglExternalModuleReplySet(
        module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Notify the Reply failed.\n");
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
 * ExternalModule: Parse the first line of Notify.
 */
static int
nglExternalModuleNotifyFirstLineParse(
    char *target,
    int nMultiLineNotify,
    char **multiLineNotify,
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    int *multiLine,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleNotifyFirstLineParse";
    char *cur, *msg, *notifyName, *tmpCur;
    int i, isMultiLine;

    /* Check the arguments */
    assert(target != NULL);
    assert(notifyStatus != NULL);
    assert(multiLine != NULL);

    *multiLine = 0;
    cur = NULL;
    msg = NULL;

    notifyStatus->ngemns_notifyName = NULL;
    notifyStatus->ngemns_message = NULL;

    cur = target;

    /* Skip space. */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Get the Notify name. */
    notifyName = ngiStrdup(cur, log, error);
    if (notifyName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Duplicate the Notify name string failed.\n");
        goto error;
    }

    tmpCur = notifyName;
    while ((*tmpCur != '\0') && (!isspace((int)*tmpCur))) {
        tmpCur++;
    }
    *tmpCur = '\0';
    tmpCur = NULL;
    notifyStatus->ngemns_notifyName = notifyName;

    /* Skip Notify name. */
    while ((*cur != '\0') && (!isspace((int)*cur))) {
        cur++;
    }

    /* Skip space. */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Get the remaining strings. */
    if (*cur != '\0') {
        /* Invoke Server JOB_STATUS reply is treated as message. */
        msg = ngiStrdup(cur, log, error);
        if (msg == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Duplicate the Notify message string failed.\n");
            goto error;
        }
        notifyStatus->ngemns_message = msg;
    }

    /* Is multi line Notify? */
    isMultiLine = 0;
    for (i = 0; i < nMultiLineNotify; i++) {
        if (strcmp(multiLineNotify[i], notifyName) == 0) {
            isMultiLine = 1;
        }
    }

    if (isMultiLine != 0) {
        *multiLine = 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Check the end of Multi line Notify.
 */
static int
nglExternalModuleNotifyMultiLineEndCheck(
    char *target,
    char *notifyName,
    int *isEnd,
    ngLog_t *log,
    int *error)
{
    char *cur, *cmp;

    /* Check the arguments */
    assert(target != NULL);
    assert(notifyName != NULL);
    assert(isEnd != NULL);

    *isEnd = 0;

    cur = target;

    /* Check Notify name. */
    cmp = notifyName;
    if (strncmp(cur, cmp, strlen(cmp)) != 0) {

        *isEnd = 0;
        return 1;
    }

    cur += strlen(cmp);

    /* Check remaining "_END". */
    cmp = NGI_EXTERNAL_MODULE_NOTIFY_MULTI_END_LAST;
    if (strncmp(cur, cmp, strlen(cmp)) != 0) {

        *isEnd = 0;
        return 1;
    }

    cur += strlen(cmp);

    if ((*cur != '\0') && (!isspace((int)*cur))) {
        *isEnd = 0;
        return 1;
    }

    *isEnd = 1;

    /* Success */
    return 1;
}


/**
 * ExternalModule: Process the Notify.
 */
static int
nglExternalModuleNotifyProcess(
    ngiExternalModule_t *module,
    ngiExternalModuleReader_t *reader,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleNotifyProcess";
    ngiLineList_t *lines;
    int result;

    /* Check the arguments */
    assert(module != NULL);

    lines = NULL;

    if (reader->ngemr_isMultiLine != 0) {
        lines = reader->ngemr_lines;

        reader->ngemr_isMultiLine = 0;
        reader->ngemr_lines = NULL;
    }

    result = nglExternalModuleNotifyCallbackCall(
        module, NGI_EXTERNAL_MODULE_NOTIFY_NORMAL, lines, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Call the Notify callback failed.\n");
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
 * ExternalModule: Notify Callback.
 */
static int
nglExternalModuleNotifyCallbackCall(
    ngiExternalModule_t *module,
    ngiExternalModuleNotifyState_t state,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleNotifyCallbackCall";
    char *moduleName, *type, *stateName, *notifyName, *notifyMessage;
    ngiExternalModuleNotifyCallbackFunction_t callbackFunction;
    ngiExternalModuleNotifyStatus_t *notifyStatus;
    int result, locked, typeCount;
    void *callbackArgument;

    /* Check the arguments */
    assert(module != NULL);
    assert(state > NGI_EXTERNAL_MODULE_NOTIFY_NONE);
    assert(state < NGI_EXTERNAL_MODULE_NOTIFY_NOMORE);

    locked = 0;
    notifyStatus = &module->ngem_notifyStatus;

    callbackFunction = NULL;
    callbackArgument = NULL;
    notifyName = NULL;
    notifyMessage = NULL;
    stateName = NULL;

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Set the state name */
    if (state == NGI_EXTERNAL_MODULE_NOTIFY_NORMAL) {
        stateName = "normal";
    } else if (state == NGI_EXTERNAL_MODULE_NOTIFY_ERROR) {
        stateName = "error";
    } else if (state == NGI_EXTERNAL_MODULE_NOTIFY_CLOSED) {
        stateName = "closed";
    } else if (state == NGI_EXTERNAL_MODULE_NOTIFY_CANCELED) {
        stateName = "canceled";
    } else {
        abort();
    }

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    if (notifyStatus->ngemns_callbackRegistered == 0) {
        /**
         * It may not be happen, because notify callback is normally
         * called from I/O handle read callback,
         * which is registered by Notify callback register.
         */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log,
            NG_LOGCAT_NINFG_PURE, fName,
            "The Notify callback for %s %s(%d) is not registered.\n",
            moduleName, type, typeCount);
        goto error;
    }

    if (state == NGI_EXTERNAL_MODULE_NOTIFY_NORMAL) {
        notifyName = notifyStatus->ngemns_notifyName;
        notifyMessage = notifyStatus->ngemns_message;

        notifyStatus->ngemns_notifyName = NULL;
        notifyStatus->ngemns_message = NULL;
    }

    callbackFunction = notifyStatus->ngemns_callbackFunction;
    callbackArgument = notifyStatus->ngemns_callbackArgument;

    notifyStatus->ngemns_callbackFunction = NULL;
    notifyStatus->ngemns_callbackArgument = NULL;
    notifyStatus->ngemns_callbackRegistered = 0;

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    /* Log */
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "The Notify callback (state=%s) for %s %s(%d) calling.\n",
        stateName, moduleName, type, typeCount);

    /* Call the callback without lock, which can register next callback. */
    result = (*callbackFunction)(
        callbackArgument, state,
        notifyName, notifyMessage, lines, log, error);
    if (result == 0) {
        /* Error from user of External Module. */
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "The Notify callback for %s %s(%d) returned by error.\n",
            moduleName, type, typeCount);

        /* Just forget. */
    }

    /* Deallocate if exists. */
    result = ngiFree(notifyName, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the Notify name failed.\n");
        goto error;
    }

    result = ngiFree(notifyMessage, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the Notify message failed.\n");
        goto error;
    }

    /* lines are destruct by user callback function. */

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModuleReadBuffer: Initialize.
 */
static int
nglExternalModuleReadBufferInitialize(
    ngiExternalModuleReadBuffer_t *readBuffer,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReadBufferInitialize";
    size_t newSize;
    char *newBuf;

    /* Check the arguments */
    assert(readBuffer != NULL);

    nglExternalModuleReadBufferInitializeMember(readBuffer);

    newSize = NGI_EXTERNAL_MODULE_READ_BUFFER_INITIAL_SIZE;

    newBuf = ngiCalloc(newSize, sizeof(char), log, error);
    if (newBuf == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the storage for read buffer failed.\n");
        return 0;
    }
    readBuffer->ngemrb_buf = newBuf;
    readBuffer->ngemrb_bufSize = newSize;

    /* Success */
    return 1;
}

/**
 * ExternalModuleReadBuffer: Finalize.
 */
static int
nglExternalModuleReadBufferFinalize(
    ngiExternalModuleReadBuffer_t *readBuffer,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReadBufferFinalize";
    int result;

    /* Check the arguments */
    assert(readBuffer != NULL);

    result = ngiFree(readBuffer->ngemrb_buf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the storage for read buffer failed.\n");
        return 0;
    }
    readBuffer->ngemrb_buf = NULL;

    nglExternalModuleReadBufferInitializeMember(readBuffer);

    /* Success */
    return 1;
}

/**
 * ExternalModuleReadBuffer: Initialize Member.
 */
static void
nglExternalModuleReadBufferInitializeMember(
    ngiExternalModuleReadBuffer_t *readBuffer)
{
    /* Check the arguments */
    assert(readBuffer != NULL);

    readBuffer->ngemrb_buf = NULL;
    readBuffer->ngemrb_lineStart = 0;
    readBuffer->ngemrb_current = 0;
    readBuffer->ngemrb_termMatchCount = 0;
    readBuffer->ngemrb_termMatchStart = 0;
    readBuffer->ngemrb_bufSize = 0;
    readBuffer->ngemrb_reachEOF = 0;
}

/**
 * ExternalModuleRequestReplyStatus: Initialize.
 */
static int
nglExternalModuleRequestReplyStatusInitialize(
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(requestReply != NULL);

    nglExternalModuleRequestReplyStatusInitializeMember(requestReply);

    /* Success */
    return 1;
}

/**
 * ExternalModuleRequestReplyStatus: Finalize.
 */
static int
nglExternalModuleRequestReplyStatusFinalize(
    ngiExternalModuleRequestReplyStatus_t *requestReply,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(requestReply != NULL);

    nglExternalModuleRequestReplyStatusInitializeMember(requestReply);

    /* Success */
    return 1;
}

/**
 * ExternalModuleRequestReplyStatus: Initialize the members.
 */
static void
nglExternalModuleRequestReplyStatusInitializeMember(
    ngiExternalModuleRequestReplyStatus_t *requestReply)
{
    /* Check the arguments */
    assert(requestReply != NULL);

    requestReply->ngemrrs_requesting = 0;
    requestReply->ngemrrs_replied = 0;
    requestReply->ngemrrs_errorOccurred = 0;
    requestReply->ngemrrs_errorCode = NG_ERROR_NO_ERROR;

    requestReply->ngemrrs_success = 0;
    requestReply->ngemrrs_message = NULL;
    requestReply->ngemrrs_lines = NULL;
}

/**
 * ExternalModuleNotifyStatus: Initialize.
 */
static int
nglExternalModuleNotifyStatusInitialize(
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(notifyStatus != NULL);

    nglExternalModuleNotifyStatusInitializeMember(notifyStatus);

    /* Success */
    return 1;
}

/**
 * ExternalModuleNotifyStatus: Finalize.
 */
static int
nglExternalModuleNotifyStatusFinalize(
    ngiExternalModuleNotifyStatus_t *notifyStatus,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(notifyStatus != NULL);

    nglExternalModuleNotifyStatusInitializeMember(notifyStatus);

    /* Success */
    return 1;
}

/**
 * ExternalModuleNotifyStatus: Initialize the members.
 */
static void
nglExternalModuleNotifyStatusInitializeMember(
    ngiExternalModuleNotifyStatus_t *notifyStatus)
{
    /* Check the arguments */
    assert(notifyStatus != NULL);

    notifyStatus->ngemns_callbackRegistered = 0;
    notifyStatus->ngemns_callbackUnregisterRequested = 0;
    notifyStatus->ngemns_callbackFunction = NULL;
    notifyStatus->ngemns_callbackArgument = NULL;

    notifyStatus->ngemns_notifyName = NULL;
    notifyStatus->ngemns_message = NULL;
}

/**
 * ExternalModule: Lock the request.
 */
static int
nglExternalModuleRequestLock(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleRequestLock";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result, locked;

    /* Check the arguments */
    assert(module != NULL);

    locked = 0;
    requestReply = &module->ngem_requestReply;

    /* Lock the mutex */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Wait unlock */
    while ((module->ngem_valid != 0) &&
        (requestReply->ngemrrs_requesting != 0)) {

        result = ngiCondWait(
            &module->ngem_cond, &module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Wait the cond failed.\n");
            goto error;
        }
    }

    /* Is External Module valid? */
    if (module->ngem_valid == 0) {
        NGI_SET_ERROR(error, module->ngem_errorCode);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module is not valid.\n");
        goto error;
    }

    /* Lock the Request */
    requestReply->ngemrrs_requesting = 1;

    /* Unlock the mutex */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Unlock the request.
 */
static int
nglExternalModuleRequestUnlock(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleRequestUnlock";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result, locked;

    /* Check the arguments */
    assert(module != NULL);

    locked = 0;
    requestReply = &module->ngem_requestReply;

    /* Lock the mutex */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Is Request Locked? */
    if (requestReply->ngemrrs_requesting == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Request is not locked.\n");
        goto error;
    }

    /* Unlock the Request */
    requestReply->ngemrrs_requesting = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&module->ngem_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Notify the cond failed.\n");
        goto error;
    }

    /* Unlock the mutex */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Set the Reply.
 */
static int
nglExternalModuleReplySet(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReplySet";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result, locked;

    /* Check the arguments */
    assert(module != NULL);

    locked = 0;
    requestReply = &module->ngem_requestReply;

    /* Lock the mutex */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Set the Reply */
    requestReply->ngemrrs_replied = 1;

    /* Notify signal */
    result = ngiCondBroadcast(&module->ngem_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Notify the cond failed.\n");
        goto error;
    }

    /* Unlock the mutex */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Wait the Reply.
 */
static int
nglExternalModuleReplyWait(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleReplyWait";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result, locked;

    /* Check the arguments */
    assert(module != NULL);

    locked = 0;
    requestReply = &module->ngem_requestReply;

    /* Lock the mutex. */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Wait the Reply. */
    while ((module->ngem_valid != 0) &&
        (requestReply->ngemrrs_replied == 0)) {

        result = ngiCondWait(
            &module->ngem_cond, &module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Wait the cond failed.\n");
            goto error;
        }
    }

    /* Is External Module valid? */
    if (module->ngem_valid == 0) {
        if (module->ngem_working != 0) {
            NGI_SET_ERROR(error, module->ngem_errorCode);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "External Module is not valid.\n");
            goto error;
        } else {
            /* External Module is exiting and may closed. */
            ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
                "External Module is not valid, may already closed.\n");
        }
    }

    if (requestReply->ngemrrs_errorOccurred != 0) {
        NGI_SET_ERROR(error, requestReply->ngemrrs_errorCode);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "External Module Request Reply was failed.\n");
        goto error;
    }

    /* Reset. */
    requestReply->ngemrrs_replied = 0;

    /* Unlock the mutex */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}


/**
 * ExternalModule: Send Request and wait Reply.
 * Note: Caller must release the returned replyMessage and
 * destruct the replyLines.
 */
int
ngiExternalModuleRequest(
    ngiExternalModule_t *module,
    char *requestName,
    char *requestArgument,
    ngiLineList_t *requestOptions,
    int requestTimeout,
    int *replySuccess,
    char **replyMessage,
    ngiLineList_t **replyLines,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleRequest";
    ngiExternalModuleRequestReplyStatus_t *requestReply;
    int result, locked, valid, typeCount;
    char *moduleName, *type;

    /* Check the arguments */
    if ((module == NULL) || (requestName == NULL) ||
        (replySuccess == NULL) || (replyMessage == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((requestName == NULL) ? "request name" :
            ((replySuccess == NULL) ? "reply success" :
            ((replyMessage == NULL) ? "reply message" : "unknown")))));
        return 0;
    }

    locked = 0;

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    *replySuccess = 0;
    *replyMessage = NULL;
    if (replyLines != NULL) {
        *replyLines = NULL;
    }

    requestReply = &module->ngem_requestReply;

    result = nglExternalModuleRequestLock(module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Request failed.\n");
        goto error;
    }
    locked = 1;

    if (strcmp(requestName, NGI_EXTERNAL_MODULE_REQUEST_EXIT) == 0) {
        /* The External Module is exiting. */
        module->ngem_working = 0;

    } else {
        result = ngiExternalModuleIsValid(module, &valid, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Check the valid failed.\n");
            goto error;
        }

        if (valid == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "The External Module is invalid.\n");
            goto error;
        }
    }

    requestReply->ngemrrs_errorOccurred = 0;
    requestReply->ngemrrs_errorCode = NG_ERROR_NO_ERROR;
    requestReply->ngemrrs_success = 0;
    requestReply->ngemrrs_message = NULL;
    requestReply->ngemrrs_lines = NULL;

    result = nglExternalModuleRequestSend(
        module, requestName, requestArgument, requestOptions,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Send the request failed.\n");
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Request was sent. Start waiting Reply for %s %s(%d).\n",
        moduleName, type, typeCount);

    /* Note: No timeout currently. */
    result = nglExternalModuleReplyWait(module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Wait the reply failed.\n");
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Reply arrived for %s %s(%d).\n",
        moduleName, type, typeCount);

    *replySuccess = requestReply->ngemrrs_success;
    *replyMessage = requestReply->ngemrrs_message;
    if (replyLines != NULL) {
        *replyLines = requestReply->ngemrrs_lines;

    } else if (requestReply->ngemrrs_lines != NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Notify have multiple lines, but cannot set.\n");
        goto error;
    }

    requestReply->ngemrrs_errorOccurred = 0;
    requestReply->ngemrrs_errorCode = NG_ERROR_NO_ERROR;
    requestReply->ngemrrs_success = 0;
    requestReply->ngemrrs_message = NULL;
    requestReply->ngemrrs_lines = NULL;

    locked = 0;
    result = nglExternalModuleRequestUnlock(module, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Request failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
    locked = 0;
        result = nglExternalModuleRequestUnlock(module, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the Request failed.\n");
        }
    }


    /* Failed */
    return 0;
}

/**
 * ExternalModule: Send the Request.
 */
static int
nglExternalModuleRequestSend(
    ngiExternalModule_t *module,
    char *requestName,
    char *requestArgument,
    ngiLineList_t *requestOptions,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglExternalModuleRequestSend";
    char *moduleName, *type, *tmp1, *tmp2, *cur;
    int result, typeCount;

    /* Check the arguments */
    assert(module != NULL);
    assert(requestName != NULL);

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    tmp1 = NULL;
    tmp2 = NULL;
    if (requestArgument != NULL) {
        tmp1 = " ";
        tmp2 = requestArgument;
    } else {
        tmp1 = "";
        tmp2 = "";
    }

    result = nglExternalModuleRequestLineOutput(
        module, requestName, 1, log, error,
        "%s%s%s", requestName, tmp1, tmp2);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Send the Request for %s %s(%d) failed.\n",
            moduleName, type, typeCount);
        goto error;
    }

    if (requestOptions != NULL) {
        cur = NULL; /* retrieve head item. */
        while ((cur = ngiLineListLineGetNext(
            requestOptions, cur, log, error)) != NULL) {

            result = nglExternalModuleRequestLineOutput(
                module, requestName, 0, log, error,
                "%s", cur);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Send the Request argument for %s %s(%d) failed.\n",
                    moduleName, type, typeCount);
                goto error;
            }
        }

        result = nglExternalModuleRequestLineOutput(
            module, requestName, 0, log, error,
            "%s%s",
            requestName,
            NGI_EXTERNAL_MODULE_REQUEST_END_LAST);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Send the Request argument end for %s %s(%d) failed.\n",
                moduleName, type, typeCount);
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
 * ExternalModule: Output the Request Line.
 */
static int
nglExternalModuleRequestLineOutput(
    ngiExternalModule_t *module,
    char *requestName,
    int isFirstLine,
    ngLog_t *log,
    int *error,
    const char *fmt,
    ...)
{
    static const char fName[] = "nglExternalModuleRequestLineOutput";
    size_t lineSize, writeNbytes;
    char *line, *lineWithTerm;
    char *moduleName, *type;
    int result, typeCount;
    ngiIOhandle_t *handle;
    va_list ap;

    /* Check the arguments */
    assert(module != NULL);
    assert(requestName != NULL);

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;
    handle = module->ngem_requestHandle;
    line = NULL;
    lineWithTerm = NULL;
    lineSize = 0;
    writeNbytes = 0;

    va_start(ap, fmt);

    line = ngiStrdupVprintf(fmt, ap, log, error);

    va_end(ap);

    if (line == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Create the %s %s Request string failed.\n",
            moduleName, requestName);
        goto error;
    }

    if (isFirstLine != 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "Request \"%s\" for %s %s(%d).\n",
            line, moduleName, type, typeCount);
    } else {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s argument: \"%s\".\n",
            requestName, line);
    }

    lineWithTerm = ngiStrdupPrintf(
        log, error, "%s%s", line,
        NGI_EXTERNAL_MODULE_LINE_TERMINATOR_STR);
    if (lineWithTerm == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Create the %s %s Request string failed.\n",
            moduleName, requestName);
        goto error;
    }

    lineSize = strlen(lineWithTerm);

    result = ngiIOhandleWrite(
        handle, lineWithTerm, lineSize, lineSize, &writeNbytes,
        log, error);
    if ((result == 0) || (writeNbytes != lineSize)) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Write the %s %s Request string failed.\n",
            moduleName, requestName);
        goto error;
    }

    result = ngiFree(line, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the storage for read buffer failed.\n");
        goto error;
    }

    result = ngiFree(lineWithTerm, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the storage for read buffer failed.\n");
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
 * ExternalModule: Register the Notify callback.
 * Note: Called callback function must destruct the lines argument.
 */
int
ngiExternalModuleNotifyCallbackRegister(
    ngiExternalModule_t *module,
    ngiExternalModuleNotifyCallbackFunction_t callbackFunction,
    void *callbackArgument,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleNotifyCallbackRegister";
    ngiExternalModuleNotifyStatus_t *notifyStatus;
    ngiExternalModuleReader_t *reader;
    int result, locked, typeCount;
    char *moduleName, *type;

    locked = 0;

    /* Check the arguments */
    if ((module == NULL) || (callbackFunction == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            (((module == NULL) ? "External Module" :
            ((callbackFunction == NULL) ? "callback function" :
            "Unknown"))));
        return 0;
    }

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    notifyStatus = &module->ngem_notifyStatus;
    reader = &module->ngem_notifyReader;

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Log */
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Register the Notify callback for %s %s(%d).\n",
        moduleName, type, typeCount);

    if (notifyStatus->ngemns_callbackRegistered != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "The Notify callback for %s %s(%d) is already registered.\n",
            moduleName, type, typeCount);
        goto error;
    }

    /* Set the callback. */
    notifyStatus->ngemns_callbackFunction = callbackFunction;
    notifyStatus->ngemns_callbackArgument = callbackArgument;
    notifyStatus->ngemns_callbackRegistered = 1;

    if (reader->ngemr_callbackExecuting == 0) {

        result = ngiIOhandleCallbackWaiterStart(
            &reader->ngemr_callbackWaiter, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Start the Callback Waiter failed.\n");
            goto error;
        }

        /* Register the I/O handle callback. */
        result = ngiIOhandleReadCallbackRegister(
            reader->ngemr_handle, nglExternalModuleReaderCallback,
            reader, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Register the callback failed.\n");
            goto error;
        }
        reader->ngemr_callbackRegistered = 1;

    }

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Unregister the Notify callback.
 */
int
ngiExternalModuleNotifyCallbackUnregister(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleNotifyCallbackUnregister";
    ngiExternalModuleNotifyStatus_t *notifyStatus;
    ngiExternalModuleReader_t *reader;
    int result, locked, typeCount;
    char *moduleName, *type;

    locked = 0;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. External Module is NULL.\n");
        return 0;
    }

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    notifyStatus = &module->ngem_notifyStatus;
    reader = &module->ngem_notifyReader;

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    /* Log */
    ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
        "Unregister the Notify callback for %s %s(%d).\n",
        moduleName, type, typeCount);

    if ((notifyStatus->ngemns_callbackRegistered == 0) &&
        (reader->ngemr_callbackRegistered == 0) &&
        (reader->ngemr_callbackExecuting == 0)) {
        /* Log */
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "The Notify callback for %s %s(%d) is not working"
            " and not registered.\n",
            moduleName, type, typeCount);

    } else {
        notifyStatus->ngemns_callbackUnregisterRequested = 1;
    }

    /* Cancel the Read Callback. */
    if (reader->ngemr_callbackRegistered != 0) {
        reader->ngemr_callbackRegistered = 0;
        result = ngiIOhandleReadCallbackUnregister(
            reader->ngemr_handle, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Register the callback failed.\n");
            goto error;
        }
    }

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Return state name.
 */
char *
ngiExternalModuleNotifyStateToString(
    ngiExternalModuleNotifyState_t state)
{
    char *name;

    name = NULL;
    switch (state) {
    case NGI_EXTERNAL_MODULE_NOTIFY_NORMAL:   name = "normal";   break;
    case NGI_EXTERNAL_MODULE_NOTIFY_ERROR:    name = "error";    break;
    case NGI_EXTERNAL_MODULE_NOTIFY_CLOSED:   name = "closed";   break;
    case NGI_EXTERNAL_MODULE_NOTIFY_CANCELED: name = "canceled"; break;
    default:
        name = "unknown";
    }

    return name;
}


/**
 * ExternalModule: Is retired?
 */
int
ngiExternalModuleIsRetired(
    ngiExternalModule_t *module,
    int *retired,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleIsRetired";

    /* Check the arguments */
    if ((module == NULL) || (retired == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((retired == NULL) ? "retired" : "unknown")));
        return 0;
    }

    *retired = 0;

    if (module->ngem_serving == 0) {
        if (module->ngem_nJobsStop < module->ngem_nJobsStart) {
            /* Not retired. */
            return 1;
        }
     
        if (module->ngem_nJobsDone < module->ngem_nJobsStart) {
            /* Not retired. */
            return 1;
        }

        /* Retired. */
        *retired = 1;

        return 1;
    }

    if (module->ngem_nJobsMax == 0) {
        /* Not retired. */
        return 1;
    }
    assert(module->ngem_nJobsMax > 0);

    if (module->ngem_nJobsStart < module->ngem_nJobsMax) {
        /* Not retired. */
        return 1;
    }

    if (module->ngem_nJobsStop < module->ngem_nJobsMax) {
        /* Not retired. */
        return 1;
    }

    if (module->ngem_nJobsDone < module->ngem_nJobsMax) {
        /* Not retired. */
        return 1;
    }

    /* Retired. */
    *retired = 1;

    /* Success */
    return 1;
}
    
/**
 * ExternalModule: Unusable.
 */
int
ngiExternalModuleUnusable(
    ngiExternalModule_t *module,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleUnusable";
    ngiExternalModuleType_t moduleType;
    int result, locked, typeCount;
    char *moduleName, *type;

    locked = 0;

    /* Check the arguments */
    if (module == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. External Module is NULL.\n");
        return 0;
    }

    moduleType = module->ngem_moduleType;
    moduleName = nglExternalModuleTypeName(moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    /* Lock */
    result = ngiMutexLock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Lock the Mutex failed.\n");
        goto error;
    }
    locked = 1;

    ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
        "The External Module for %s %s(%d) making unusable.\n",
        moduleName, type, typeCount);

    /* Is already done? */
    if (module->ngem_valid == 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s %s(%d) already unusable.\n",
            moduleName, type, typeCount);

        /* Unlock */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the Mutex failed.\n");
            goto error;
        }

        /* Success */
        return 1;
    }

    /* No more External Module valid */
    module->ngem_valid = 0;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &module->ngem_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Signal the Cond failed.\n");
        goto error;
    }

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&module->ngem_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unlock the Mutex failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
        "Make the External Module unusable failed.\n");

    if (locked != 0) {
        /* Unlock the mutex */
        locked = 0;
        result = ngiMutexUnlock(&module->ngem_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Unlock the mutex failed.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * ExternalModule: Request QUERY_FEATURES.
 * Note: Caller must release the returned version, feature,
 *   requests, errorMessage.
 */
int
ngiExternalModuleQueryFeatures(
    ngiExternalModule_t *module,
    int *requestSuccess,
    char **version,
    ngiLineList_t **features,
    ngiLineList_t **requests,
    char **errorMessage,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiExternalModuleQueryFeatures";
    ngiLineList_t *replyLines, *returnFeatures, *returnRequests;
    char *requestName, *replyMessage, *returnVersion;
    int result, replySuccess, typeCount;
    ngiExternalModuleArgument_t *arg;
    char *curLine, *candidate;
    char *moduleName, *type;
    
    replyMessage = NULL;
    replyLines = NULL;
    returnVersion = NULL;
    returnFeatures = NULL;
    returnRequests = NULL;
    curLine = NULL;
    candidate = NULL;
    arg = NULL;

    /* Check the arguments */
    if ((module == NULL) || (requestSuccess == NULL) ||
        (version == NULL) || (features == NULL) || (requests == NULL) ||
        (errorMessage == NULL)) {

        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. %s is NULL.\n",
            ((module == NULL) ? "External Module" :
            ((requestSuccess == NULL) ? "replySuccess" :
            ((version == NULL) ? "version" :
            ((features == NULL) ? "features" :
            ((requests == NULL) ? "requests" :
            ((errorMessage == NULL) ? "errorMessage" : "unknown")))))));
        goto error;
    }

    *requestSuccess = 0;
    *version = NULL;
    *features = NULL;
    *requests = NULL;
    *errorMessage = NULL;

    moduleName = nglExternalModuleTypeName(module->ngem_moduleType);
    type = module->ngem_type;
    typeCount = module->ngem_typeCount;

    requestName = NGI_EXTERNAL_MODULE_REQUEST_QUERY_FEATURES;

    result = ngiExternalModuleRequest(
        module, requestName, NULL, NULL,
        0, /* No timeout is implemented. */
        &replySuccess, &replyMessage, &replyLines, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Request to %s %s(%d) failed.\n",
            moduleName, type, typeCount);
        goto error;
    }

    if (replySuccess == 0) {
        /* Note: Ninf-G4 Invoke Server do not have QUERY_FEATURES. */

        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s for %s %s (%d) failed: \"%s\".\n",
            requestName, moduleName, type, typeCount,
            ((replyMessage != NULL) ? replyMessage : ""));

        *requestSuccess = replySuccess;
        *errorMessage = replyMessage;

        /* Success */
        return 1;
    }

    if (replyMessage != NULL) {
        ngLogDebug(log, NG_LOGCAT_NINFG_LIB, fName,
            "Unexpected message for %s %s(%d): \"%s\".\n",
            moduleName, type, typeCount, replyMessage);

        result = ngiFree(replyMessage, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Deallocate the storage for reply message failed.\n");
            goto error;
        }

        replyMessage = NULL;
    }

    if (replyLines == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "No lines on %s on %s %s(%d).\n",
            requestName, moduleName, type, typeCount);
        goto error;
    }

    returnFeatures = ngiLineListConstruct(log, error);
    if (returnFeatures == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Construct the Line List failed.\n");
        goto error;
    }
    
    returnRequests = ngiLineListConstruct(log, error);
    if (returnRequests == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Construct the Line Buffer failed.\n");
        goto error;
    }
    
    curLine = NULL;
    while ((curLine = ngiLineListLineGetNext(
        replyLines, curLine, log, error)) != NULL) {

        arg = ngiExternalModuleArgumentConstruct(curLine, log, error);
        if ((arg == NULL) || (arg->ngea_name == NULL)) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Invalid line \"%s\" on %s on %s %s(%d).\n",
                curLine, requestName, moduleName, type, typeCount);
            goto error;
        }

        candidate = NGI_EXTERNAL_MODULE_QUERY_REQUESTS_VERSION;
        if (strcmp(arg->ngea_name, candidate) == 0) {

            if (returnVersion != NULL) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "The version was already appeared.\n");
                goto error;
            }

            if (arg->ngea_value == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "The version was not found.\n");
                goto error;
            }

            returnVersion = ngiStrdup(arg->ngea_value, log, error);
            if (returnVersion == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Duplicate the version string failed.\n");
                goto error;
            }

            result = ngiExternalModuleArgumentDestruct(arg, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Destruct the argument failed.\n");
                goto error;
            }
            arg = NULL;

            continue;
        }

        candidate = NGI_EXTERNAL_MODULE_QUERY_REQUESTS_FEATURE;
        if (strcmp(arg->ngea_name, candidate) == 0) {

            if (arg->ngea_value == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "The feature was not found.\n");
                goto error;
            }

            result = ngiLineListAppend(
                returnFeatures, arg->ngea_value, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Append the LineList failed.\n");
                goto error;
            }

            result = ngiExternalModuleArgumentDestruct(arg, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Destruct the argument failed.\n");
                goto error;
            }
            arg = NULL;

            continue;
        }

        candidate = NGI_EXTERNAL_MODULE_QUERY_REQUESTS_REQUEST;
        if (strcmp(arg->ngea_name, candidate) == 0) {

            if (arg->ngea_value == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "The request was not found.\n");
                goto error;
            }

            result = ngiLineListAppend(
                returnRequests, arg->ngea_value, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Append the LineList failed.\n");
                goto error;
            }

            result = ngiExternalModuleArgumentDestruct(arg, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Destruct the argument failed.\n");
                goto error;
            }
            arg = NULL;

            continue;
        }

        /* else */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid element \"%s\" on %s %s(%d).\n",
            arg->ngea_name, moduleName, type, typeCount);
        goto error;
    }

    result = ngiLineListDestruct(replyLines, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the Line List failed.\n");
        goto error;
    }
    replyLines = NULL;

    *requestSuccess = replySuccess;
    *version = returnVersion;
    *features = returnFeatures;
    *requests = returnRequests;
    *errorMessage = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * External Module Argument: Construct
 */
ngiExternalModuleArgument_t *
ngiExternalModuleArgumentConstruct(
    char * src,
    ngLog_t *log,
    int *error)
{
    ngiExternalModuleArgument_t *arg = NULL;
    int result;
    static const char fName[] = "ngiExternalModuleArgumentConstruct";

    arg = NGI_ALLOCATE(ngiExternalModuleArgument_t, log, error);
    if (arg == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for notify argument.\n");
        goto error;
    }
    result = nglExternalModuleArgumentInitialize(arg, src, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't initialize notify argument.\n");
        goto error;
    }

    return arg;
error:
    result = ngiExternalModuleArgumentDestruct(arg, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't destroy notify argument.\n");
    }

    return NULL;
}

/**
 * Notify Argument: Destruct
 */
int
ngiExternalModuleArgumentDestruct(
    ngiExternalModuleArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiExternalModuleArgumentDestruct";

    if (arg == NULL) {
        /* Success */
        return 1;
    }

    result = nglExternalModuleArgumentFinalize(arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize notify argument.\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngiExternalModuleArgument_t, arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for notify argument.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Notify Argument: Initialize
 */
static int
nglExternalModuleArgumentInitialize(
    ngiExternalModuleArgument_t *arg,
    char *src,
    ngLog_t *log,
    int *error)
{
    int name_len = 0;
    int value_len = 0;
    char *name = NULL;
    char *value = NULL;
    char *valueHead;
    int i;
    int j;
    int result;
    static const char fName[] = "nglExternalModuleArgumentInitialize";

    assert(arg != NULL);
    assert(src != NULL);

    nglExternalModuleArgumentInitializeMember(arg);

    i = 0;

    /* Get the name of option */
    for (;(src[i] != '\0') && (!isspace((int)src[i]));++i);
    name_len = i;
    if (name_len == 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Name is empty.\n");
        goto error;
    }

    name = ngiStrndup(src, name_len, log, error);
    if (name == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't copy the string.\n");
        goto error;
    }
    arg->ngea_name = name;

    /* Skip space */
    for (;(src[i] != '\0') && (isspace((int)src[i]));++i);
    valueHead = &src[i];

    /* Get the value of option */
    if (strlen(valueHead) > 0) {

        value = ngiStrdup(valueHead, log, error);
        if (value == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't copy the string.\n");
            goto error;
        }
        arg->ngea_value = value;
        value_len = strlen(valueHead);

        for (j = value_len-1;(j >= 0) && (isspace((int)value[j]));--j) {
            value[j] = '\0';
        }
    } else {
        value = ngiStrdup("", log, error);
        if (value == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "Can't allocate storage for value of argument.\n");
            goto error;
        }
        arg->ngea_value = value;
    }
    return 1;
error:
    result = nglExternalModuleArgumentFinalize(arg, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't finalize notify argument.\n");
    }
    return 0;
}

/**
 * Notify Argument: Finalize
 */
static int
nglExternalModuleArgumentFinalize(
    ngiExternalModuleArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "nglExternalModuleArgumentFinalize";

    result = ngiFree(arg->ngea_name, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for name of argument.\n");
        ret = 0;
        error = NULL;
    }
    result = ngiFree(arg->ngea_value, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't deallocate storage for value of argument.\n");
        ret = 0;
        error = NULL;
    }
    nglExternalModuleArgumentInitializeMember(arg);

    return ret;
}

/**
 * Notify Argument: Zero clear
 */
static void
nglExternalModuleArgumentInitializeMember(
    ngiExternalModuleArgument_t *arg)
{
#if 0
    static const char fName[] = "nglExternalModuleArgumentInitializeMember";
#endif
    arg->ngea_name = NULL;
    arg->ngea_value = NULL;

    return;
}

