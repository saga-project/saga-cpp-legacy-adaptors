/*
 * $RCSfile: ngclInvokeServer.c,v $ $Revision: 1.37 $ $Date: 2008/03/28 06:57:41 $
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
 * Module of Invoke Server for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclInvokeServer.c,v $ $Revision: 1.37 $ $Date: 2008/03/28 06:57:41 $")

/**
 * Data type
 */

typedef enum ngcllInvokeServerStatus_e {
    NGCLI_INVOKE_SERVER_STATUS_UNDEFINED,
    NGCLI_INVOKE_SERVER_STATUS_PENDING,
    NGCLI_INVOKE_SERVER_STATUS_ACTIVE,
    NGCLI_INVOKE_SERVER_STATUS_DONE,
    NGCLI_INVOKE_SERVER_STATUS_FAILED,
    NGCLI_INVOKE_SERVER_STATUS_NOMORE
} ngcllInvokeServerStatus_t;

typedef struct ngcllInvokeServerStatusTable_s {
    int ngisst_valid;
    ngcllInvokeServerStatus_t ngisst_status;
    char *ngisst_name;
} ngcllInvokeServerStatusTable_t;

/**
 * Prototype declaration of static functions.
 */
static ngcliInvokeServerInformationManager_t *
ngcllInvokeServerInformationConstruct(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error);
static int
ngcllInvokeServerInformationDestruct(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error);
static int
ngcllInvokeServerInformationManagerInitialize(
     ngclContext_t *context,
     ngcliInvokeServerInformationManager_t *isInfoMng,
     ngclInvokeServerInformation_t *isInfo,
     int *error);
static int
ngcllInvokeServerInformationManagerFinalize(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error);
static int
ngcllInvokeServerInformationRelease(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error);
static void
ngcllInvokeServerInformationInitializeMember(
    ngclInvokeServerInformation_t *isInfo);
static void
ngcllInvokeServerInformationInitializePointer(
    ngclInvokeServerInformation_t *isInfo);
static int
ngcllInvokeServerInformationGetCopy(
    ngclContext_t *context,
    char *typeName,
    ngclInvokeServerInformation_t *isInfo,
    int *error);
static int
ngcllInvokeServerInformationReplace(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *dstIsInfoMng,
    ngclInvokeServerInformation_t *srcIsInfo,
    int *error);
static int
ngcllInvokeServerInitialize(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *invokeServerType,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerQueryFeatures(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerFinalize(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static void
ngcllInvokeServerInitializeMember(
    ngcliInvokeServerManager_t *invokeMng);
static int
ngcllInvokeServerInitializeMutexAndCond(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerFinalizeMutexAndCond(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static void
ngcllInvokeServerJobInitializeMember(
    ngcliInvokeServerJob_t *invokeServerJobInfo);
static int
ngcllInvokeServerRegister(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerUnregister(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerTreatRetired(
    ngclContext_t *context,
    int *error);
static int
ngcllInvokeServerIsValid(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *valid,
    int *error);
static int
ngcllInvokeServerRequestJobCreate(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerRequestJobCreateArgument(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngiLineList_t *arguments,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerRequestJobStatus(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerReplyJobStatus(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *invokeJobID,
    char *argument,
    int *error);
static int
ngcllInvokeServerRequestJobDestroy(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerRequestExit(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerNotifyCallback(
    void *argument,
    ngiExternalModuleNotifyState_t state,
    char *notifyName,
    char *message,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error);
static int
ngcllInvokeServerNotifyProcessCreateNotify(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *notifyName,
    char *argument,
    int *error);
static int
ngcllInvokeServerNotifyProcessStatusNotify(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *notifyName,
    char *argument,
    int *error);
static int
ngcllInvokeServerNotifyJobStatusChange(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *invokeJobID,
    ngcllInvokeServerStatus_t status,
    char *statusName,
    char *messageString,
    int *error);
static int
ngcllInvokeServerUnusable(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error);
static int
ngcllInvokeServerJobIDset(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerJobIDwait(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerJobStdoutStderrCreate(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerJobStdoutStderrDestroy(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerJobStdoutStderrOutput(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error);
static int
ngcllInvokeServerJobStdoutStderrOutputSub(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    char *tmpFile,
    FILE *destFp,
    char *destName,
    int *error);

/**
 * Data
 */
static const ngcllInvokeServerStatusTable_t ngcllInvokeServerStatusTable[] = {
    {1, NGCLI_INVOKE_SERVER_STATUS_PENDING, "PENDING"},
    {1, NGCLI_INVOKE_SERVER_STATUS_ACTIVE,  "ACTIVE"},
    {1, NGCLI_INVOKE_SERVER_STATUS_DONE,    "DONE"},
    {1, NGCLI_INVOKE_SERVER_STATUS_FAILED,  "FAILED"},
    {0, NGCLI_INVOKE_SERVER_STATUS_NOMORE,  NULL},
};


/**
 * Functions
 */

/**
 * Information append at last of the list.
 */
int
ngcliInvokeServerInformationCacheRegister(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliInvokeServerInformationManager_t *isInfoMng;
    static const char fName[] = "ngcliInvokeServerInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    isInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((isInfo == NULL) ||
        (isInfo->ngisi_type == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    /* Lock the list */
    result = ngcliInvokeServerInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock InvokeServerInformation list.\n");
        goto error;
    }
    listLocked = 1;

    /* Is Invoke Server Information available? */
    isInfoMng = ngcliInvokeServerInformationCacheGet(
        context, isInfo->ngisi_type, &subError);
    if (isInfoMng != NULL) {

        /* Replace the Invoke Server Information */
        result = ngcllInvokeServerInformationReplace(
            context, isInfoMng, isInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't replace the Invoke Server Information.\n");
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Invoke Server Information list.\n");
        goto error;
    }

    /* Construct */
    if (isInfoMng == NULL) {
        isInfoMng = ngcllInvokeServerInformationConstruct(
            context, isInfo, error);
        if (isInfoMng == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't construct Invoke Server Information.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliInvokeServerInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Invoke Server Information list.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Information delete from the list.
 */
int
ngcliInvokeServerInformationCacheUnregister(
    ngclContext_t *context,
    char *typeName,
    int *error)
{
    int result;
    ngcliInvokeServerInformationManager_t *curr;
    static const char fName[] = "ngcliInvokeServerInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context Invalid.\n");
        return 0;
    }

    /* Lock the list */
    result = ngcliInvokeServerInformationListWriteLock(context,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock InvokeServerInformation list.\n");
        return 0;
    }

    if (typeName == NULL) {
        /* Delete all information */

        /* Get the data from the head of a list */
        curr = ngcliInvokeServerInformationCacheGetNext(context, NULL, error);
        if (curr == NULL) {
             ngclLogDebugContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "No Invoke Server Information was registered.\n");
        }

        while (curr != NULL) {
            /* Destruct the data */
            result = ngcllInvokeServerInformationDestruct(context, curr, error);
            if (result == 0) {
                ngclLogErrorContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct Invoke Server Information.\n");
                goto error;
            }

            /* Get next data from the list */
            curr = ngcliInvokeServerInformationCacheGetNext(
                context, NULL, error);
        }
    } else {
        /* Delete specified information */

        /* Get the data from the list by Invoke Server name */
        curr = ngcliInvokeServerInformationCacheGet(context, typeName, error);
        if (curr == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Invoke Server Information \"%s\" is not found.\n",
                typeName);
            goto error;
        }

        /* Destruct the data */
        result = ngcllInvokeServerInformationDestruct(context, curr, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct Invoke Server Information.\n");
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Invoke Server Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliInvokeServerInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Invoke Server Information.\n");
        return 0;
    }

    return 0;
}

/**
 * Get the information by host name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliInvokeServerInformationManager_t *
ngcliInvokeServerInformationCacheGet(
    ngclContext_t *context,
    char *typeName,
    int *error)
{
    int result;
    ngcliInvokeServerInformationManager_t *isInfoMng;
    static const char fName[] = "ngcliInvokeServerInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    /* Check the arguments */
    if (typeName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "type name is NULL.\n");
        return NULL;
    }

    isInfoMng = context->ngc_invokeServerInfo_head;
    for (; isInfoMng != NULL; isInfoMng = isInfoMng->ngisim_next) {
        assert(isInfoMng->ngisim_info.ngisi_type != NULL);
        if (strcmp(isInfoMng->ngisim_info.ngisi_type, typeName) == 0) {
            /* Found */
            return isInfoMng;
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server Information is not found by name \"%s\".\n",
        typeName);
    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliInvokeServerInformationManager_t *
ngcliInvokeServerInformationCacheGetNext(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliInvokeServerInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    if (current == NULL) {
        /* Return the first information */
        if (context->ngc_invokeServerInfo_head != NULL) {
            assert(context->ngc_invokeServerInfo_tail != NULL);
            return context->ngc_invokeServerInfo_head;
        }
    } else {
        /* Return the next information */
        if (current->ngisim_next != NULL) {
            return current->ngisim_next;
        }
    }

    /* Not found */
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "The last Invoke Server Information was reached.\n");
    return NULL;
}

/**
 * Construct.
 */
static ngcliInvokeServerInformationManager_t *
ngcllInvokeServerInformationConstruct(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int result;
    ngcliInvokeServerInformationManager_t *isInfoMng;
    static const char fName[] = "ngcllInvokeServerInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfo != NULL);

    /* Allocate */
    isInfoMng = NGI_ALLOCATE(ngcliInvokeServerInformationManager_t,
        context->ngc_log, error);
    if (isInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Invoke Server Information.\n");
        return NULL;
    }

    /* Initialize */
    result = ngcllInvokeServerInformationManagerInitialize(
        context, isInfoMng, isInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Invoke Server Information.\n");
        goto error;
    }

    /* Register */
    result = ngcliContextRegisterInvokeServerInformation(
        context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Invoke Server Information"
            " for Ninf-G Context.\n");
        goto error;
    }

    /* Success */
    return isInfoMng;

    /* Error occurred */
error:
    result = ngcllInvokeServerInformationDestruct(context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the storage for Invoke Server Information Manager.\n");
        return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllInvokeServerInformationDestruct(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllInvokeServerInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterInvokeServerInformation(context,
        isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister the Invoke Server Information.\n");
        return 0;
    }

    /* Finalize */
    result = ngcllInvokeServerInformationManagerFinalize(
        context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Invoke Server Information.\n");
        return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliInvokeServerInformationManager_t, isInfoMng,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate the Invoke Server Information.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngclInvokeServerInformation_t *
ngcliInvokeServerInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclInvokeServerInformation_t *isInfo;
    static const char fName[] = "ngcliInvokeServerInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Allocate new storage */
    isInfo = ngiCalloc(1, sizeof (ngclInvokeServerInformation_t),
        log, error);
    if (isInfo == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Invoke Server Information.\n");
        return NULL;
    }

    return isInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliInvokeServerInformationFree(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliInvokeServerInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    ngiFree(isInfo, log, error);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllInvokeServerInformationManagerInitialize(
     ngclContext_t *context,
     ngcliInvokeServerInformationManager_t *isInfoMng,
     ngclInvokeServerInformation_t *isInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllInvokeServerInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);
    assert(isInfo != NULL);

    /* reset members */
    isInfoMng->ngisim_next = NULL;

    /* Copy to new information */
    result = ngcliInvokeServerInformationCopy(context, isInfo,
        &isInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Invoke Server Information.\n");
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&isInfoMng->ngisim_rwlOwn,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize Read/Write Lock for own instance.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllInvokeServerInformationManagerFinalize(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllInvokeServerInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&isInfoMng->ngisim_rwlOwn,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't destroy Read/Write Lock for own instance.\n");
        return 0;
    }

    /* Release the information */
    result = ngclInvokeServerInformationRelease(context,
        &isInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Invoke Server Information.\n");
        return 0;
    }

    /* reset members */
    isInfoMng->ngisim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliInvokeServerInformationCopy(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *src,
    ngclInvokeServerInformation_t *dest,
    int *error)
{
    ngLog_t *log;
    int i, result;
    static const char fName[] = "ngcliInvokeServerInformationCopy";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    ngcllInvokeServerInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllInvokeServerInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = ngiStrdup((src)->member, log, error); \
        if ((dest)->member == NULL) { \
            ngclLogErrorContext(context, \
                NG_LOGCAT_NINFG_PURE, fName, \
                "Can't allocate the storage " \
                "for Invoke Server Information.\n"); \
            goto error; \
        } \
    } while(0)

#define  NGL_COPY_STRING_IF_VALID(str, dest, member) \
    do {\
        if ((src)->member != NULL) { \
            NGL_COPY_STRING(str, dest, member); \
        } \
    } while (0)

    NGL_COPY_STRING(src, dest, ngisi_type);
    NGL_COPY_STRING_IF_VALID(src, dest, ngisi_path);
    NGL_COPY_STRING_IF_VALID(src, dest, ngisi_logFilePath);

    /* Copy Options */
    if (src->ngisi_nOptions > 0) {
        dest->ngisi_options = ngiCalloc(
            src->ngisi_nOptions, sizeof(char *), log, error);
        if (dest->ngisi_options == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate the storage for string table.\n");
            return 0;
        }
        /* copy all of elements */
        for (i = 0; i < src->ngisi_nOptions; i++) {
            NGL_COPY_STRING(src, dest, ngisi_options[i]);
        }
    }

#undef NGL_COPY_STRING_IF_VALID
#undef NGL_COPY_STRING

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclInvokeServerInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Invoke Server Information.\n");
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclInvokeServerInformationRelease(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclInvokeServerInformationRelease";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllInvokeServerInformationRelease(context, isInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllInvokeServerInformationRelease(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInformationRelease";
    ngLog_t *log;
    int i;

    /* Check the arguments */
    assert(context != NULL);

    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Deallocate the members */
    if (isInfo->ngisi_type != NULL)
        ngiFree(isInfo->ngisi_type, log, error);
    if (isInfo->ngisi_path != NULL)
        ngiFree(isInfo->ngisi_path, log, error);
    if (isInfo->ngisi_logFilePath != NULL)
        ngiFree(isInfo->ngisi_logFilePath, log, error);
    if (isInfo->ngisi_options != NULL) {
        for (i = 0; i < isInfo->ngisi_nOptions; i++) {
            if (isInfo->ngisi_options[i] != NULL) {
                ngiFree(isInfo->ngisi_options[i], log, error);
            }
        }
        ngiFree(isInfo->ngisi_options, log, error);
    }

    /* Initialize the members */
    ngcllInvokeServerInformationInitializeMember(isInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliInvokeServerInformationInitialize(
    ngclContext_t *context,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliInvokeServerInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    ngcllInvokeServerInformationInitializeMember(isInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllInvokeServerInformationInitializeMember(
    ngclInvokeServerInformation_t *isInfo)
{
    /* Initialize the members */

    ngcllInvokeServerInformationInitializePointer(isInfo);
    isInfo->ngisi_maxJobs = 0;
    isInfo->ngisi_statusPoll = 0;
    isInfo->ngisi_nOptions = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllInvokeServerInformationInitializePointer(
    ngclInvokeServerInformation_t *isInfo)
{
    /* Initialize the members */
    isInfo->ngisi_type = NULL;
    isInfo->ngisi_path = NULL;
    isInfo->ngisi_logFilePath = NULL;
    isInfo->ngisi_options = NULL;
}

/**
 * GetCopy
 */
int
ngclInvokeServerInformationGetCopy(
    ngclContext_t *context,
    char *typeName,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclInvokeServerInformationGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllInvokeServerInformationGetCopy(context,
        typeName, isInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllInvokeServerInformationGetCopy(
    ngclContext_t *context,
    char *typeName,
    ngclInvokeServerInformation_t *isInfo,
    int *error)
{
    int result;
    ngcliInvokeServerInformationManager_t *isInfoMng;
    static const char fName[] = "ngcllInvokeServerInformationGetCopy";

    /* Check the arguments */
    if ((typeName == NULL) || (isInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

   /* Lock the Invoke Server Information */
    result = ngcliInvokeServerInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Invoke Server Information.\n");
        return 0;
    }

    /* Get the Invoke Server Information */
    isInfoMng = ngcliInvokeServerInformationCacheGet(context, typeName, error);
    if (isInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server Information.\n");
        goto error;
    }

    /* Copy the Invoke Server Information */
    result = ngcliInvokeServerInformationCopy(context,
                    &isInfoMng->ngisim_info, isInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Invoke Server Information.\n");
        goto error;
    }

    /* Unlock the Invoke Server Information */
    result = ngcliInvokeServerInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Invoke Server Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliInvokeServerInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Invoke Server Information.\n");
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Replace the Invoke Server Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerInformationReplace(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *dstIsInfoMng,
    ngclInvokeServerInformation_t *srcIsInfo,
    int *error)
{
    ngLog_t *log;
    int result, isLocked;
    static const char fName[] = "ngcllInvokeServerInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstIsInfoMng != NULL);
    assert(srcIsInfo != NULL);

    log = context->ngc_log;
    isLocked = 0;

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Replace the Invoke Server Information for \"%s\".\n",
        srcIsInfo->ngisi_type);

    /* Lock */
    result = ngcliInvokeServerInformationWriteLock(
        dstIsInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write lock the Invoke Server Information.\n");
        goto error;
    }
    isLocked = 1;

    /* Release the Invoke Server Information */
    result = ngcllInvokeServerInformationRelease(
        context, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Invoke Server Information.\n");
        goto error;
    }

    /* Copy the Invoke Server Information */
    result = ngcliInvokeServerInformationCopy(
        context, srcIsInfo, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Invoke Server Information.\n");
        goto error;
    }

    /* Unlock */
    result = ngcliInvokeServerInformationWriteUnlock(
        dstIsInfoMng, log, error);
    isLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Invoke Server Information.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (isLocked != 0) {
        result = ngcliInvokeServerInformationWriteUnlock(
            dstIsInfoMng, log, NULL);
        isLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Invoke Server Information.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Construct
 */
ngcliInvokeServerManager_t *
ngcliInvokeServerConstruct(
    ngclContext_t *context,
    char *invokeServerType,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerConstruct";
    int result, isLocked, extLocked, allocated, initialized;
    ngcliInvokeServerManager_t *invokeMng;
    ngiExternalModuleManager_t *extMng;
    ngiExternalModule_t *module;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeServerType != NULL);
    assert(jobMng != NULL);

    isLocked = 0;
    extLocked = 0;
    allocated = 0;
    initialized = 0;
    extMng = context->ngc_externalModuleManager;
    module = NULL;
    log = context->ngc_log;
    invokeMng = NULL;

    /* Lock the Invoke Server list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Invoke Server.\n");
        goto error;
    }
    isLocked = 1;

    /* Lock the External Module list */
    result = ngiExternalModuleManagerListWriteLock(
        extMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the External Module.\n");
        goto error;
    }
    extLocked = 1;

    /* Get the Available External Module. */
    result = ngiExternalModuleAvailableGet(
        extMng,
        NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER,
        invokeServerType, &module,
        log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Check the available External Module failed.\n");
        goto error;
    }

    /* Is External Module available? */
    if (module != NULL) {

        invokeMng = (ngcliInvokeServerManager_t *)
            ngiExternalModuleOwnerGet(module, log, error);
        if (invokeMng == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Invoke Server was not registered.\n");
            goto error;
        }

         ngclLogDebugContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) available.\n",
            invokeServerType, invokeMng->ngism_typeCount);

    } else {
        /* Allocate */
        invokeMng = NGI_ALLOCATE(
            ngcliInvokeServerManager_t, context->ngc_log, error);
        if (invokeMng == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate the storage for Invoke Server.\n");
            goto error;
        }
        allocated = 1;

        /* Initialize */
        result = ngcllInvokeServerInitialize(
            context, invokeMng, invokeServerType, jobMng, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't initialize the Invoke Server %s.\n",
                invokeServerType);
            goto error;
        }
        initialized = 1;

        /* Register */
        result = ngcllInvokeServerRegister(
            context, invokeMng, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't register the Invoke Server %s(%d).\n",
                invokeServerType, invokeMng->ngism_typeCount);
            goto error;
        }

        ngclLogInfoContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) created.\n",
            invokeServerType, invokeMng->ngism_typeCount);
    }

    /* Count Up Started job. */
    assert(invokeMng != NULL);
    result = ngiExternalModuleJobStarted(
        invokeMng->ngism_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Tell the job start to External Module failed.\n");
        goto error;
    }

    /* Unlock the External Module list */
    extLocked = 0;
    result = ngiExternalModuleManagerListWriteUnlock(
        extMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the External Module.\n");
        goto error;
    }

    /* Unlock the Invoke Server list */
    isLocked = 0;
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Invoke Server.\n");
        goto error;
    }

    /* Success */
    return invokeMng;

    /* Error occurred */
error:
    /* log */
    ngclLogErrorContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Can't construct the Invoke Server %s.\n",
        invokeServerType);

    /* Finalize */
    if ((invokeMng != NULL) && (initialized != 0)) {
        initialized = 0;
        result = ngcllInvokeServerFinalize(context, invokeMng, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't finalize the Invoke Server.\n");
        }
    }

    /* Deallocate */
    if ((invokeMng != NULL) && (allocated != 0)) {
        allocated = 0;
        result = NGI_DEALLOCATE(ngcliInvokeServerManager_t, invokeMng,
            context->ngc_log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't deallocate the Invoke Server.\n");
        }
        invokeMng = NULL;
    }

    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListWriteUnlock(
            extMng, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
        }
    }

    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }

    /* Failed */
    return NULL;
}


/**
 * Destruct
 */
int
ngcliInvokeServerDestruct(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int requireLock,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerDestruct";
    int result, isLocked, extLocked, destructAll;
    ngcliInvokeServerManager_t *curInvokeMng;
    ngiExternalModuleManager_t *extMng;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    extMng = context->ngc_externalModuleManager;
    log = context->ngc_log;

    isLocked = 0;
    extLocked = 0;
    destructAll = 0;

    if (invokeMng == NULL) {
        destructAll = 1;
    }

    /* Lock the list */
    if (requireLock != 0) {
        result = ngcliContextInvokeServerManagerListWriteLock(
            context, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Invoke Server.\n");
            goto error;
        }
        isLocked = 1;

        result = ngiExternalModuleManagerListWriteLock(
            extMng, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the External Module.\n");
            goto error;
        }
        extLocked = 1;
    }

    if (destructAll != 0) {
        curInvokeMng = context->ngc_invokeMng_head;
    } else {
        curInvokeMng = invokeMng;
    }

    while (curInvokeMng != NULL) {

        /* log */
        ngclLogInfoContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Destructing the Invoke Server %s(%d).\n",
            curInvokeMng->ngism_serverType,
            curInvokeMng->ngism_typeCount);

        /* Unregister */
        result = ngcllInvokeServerUnregister(context, curInvokeMng, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unregister the Invoke Server.\n");
            goto error;
        }

        /* Finalize */
        result = ngcllInvokeServerFinalize(context, curInvokeMng, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't finalize the Invoke Server.\n");
            goto error;
        }

        /* Deallocate */
        result = NGI_DEALLOCATE(ngcliInvokeServerManager_t,
            curInvokeMng, context->ngc_log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't deallocate the Invoke Server.\n");
            goto error;
        }

        /* Next Invoke Mng */
        if (destructAll != 0) {
            /* Get the new head */
            curInvokeMng = context->ngc_invokeMng_head;
        } else {
            curInvokeMng = NULL;
        }
    }

    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListWriteUnlock(
            extMng, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
            goto error;
        }
    }

    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* log */
    ngclLogErrorContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Can't destruct the Invoke Server.\n");

    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListWriteUnlock(
            extMng, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }
    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Initialize
 */
static int
ngcllInvokeServerInitialize(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *invokeServerType,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInitialize";
    char *serverPath, *logFilePath, *logFilePathCommon;
    int result, typeCount, maxJobs, invokeServerID;
    ngiExternalModule_t *module;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(invokeServerType != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;

    maxJobs = 0;
    typeCount = 0;
    invokeServerID = 0;
    serverPath = NULL;
    logFilePath = NULL;
    logFilePathCommon = NULL;
    module = NULL;

    ngcllInvokeServerInitializeMember(invokeMng);

    /* Initialize the Mutex and Cond */
    result = ngcllInvokeServerInitializeMutexAndCond(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize Mutex and Cond for "
            "Invoke Server Manager.\n");
        return 0;
    }

    invokeMng->ngism_context = context;

    invokeMng->ngism_valid = 1;
    invokeMng->ngism_working = 1;
    invokeMng->ngism_errorCode = NG_ERROR_NO_ERROR;

    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    assert(jobMng->ngjm_attr.ngja_lmInfoExist != 0);
    serverPath = jobMng->ngjm_attr.ngja_isInfo.ngisi_path; /* may be NULL */
    maxJobs = jobMng->ngjm_attr.ngja_isInfo.ngisi_maxJobs;

    logFilePath = jobMng->ngjm_attr.ngja_isInfo.ngisi_logFilePath;
    logFilePathCommon = jobMng->ngjm_attr.ngja_lmInfo.nglmi_invokeServerLog;


    /* Construct the External Module */
    module = ngiExternalModuleConstruct(
        context->ngc_externalModuleManager,
        NGI_EXTERNAL_MODULE_TYPE_INVOKE_SERVER,
        NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL,
        invokeServerType, serverPath,
        logFilePathCommon, logFilePath,
        maxJobs,
        NULL, /* No multi line Notify. */
        log, error);
    if (module == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Construct the External Module for Invoke Server %s failed.\n",
            invokeServerType);
        return 0;
    }
    invokeMng->ngism_externalModule = module;

    /* Register Invoke Server to External Module. */
    result = ngiExternalModuleOwnerSet(
        module, (void *)invokeMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server was not registered.\n");
        goto error;
    }

    /* Invoke Server ID */
    result = ngiExternalModuleModuleTypeIDget(
        module, &invokeServerID, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Get the Invoke Server ID failed.\n");
        goto error;
    }
    invokeMng->ngism_ID = invokeServerID;

    /* Copy the Invoke Server Type */
    invokeMng->ngism_serverType = strdup(invokeServerType);
    if (invokeMng->ngism_serverType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't duplicate the string.\n");
        return 0;
    }

    /* Invoke Server type count */
    result = ngiExternalModuleModuleTypeCountGet(
        module, &typeCount, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Get the Invoke Server type count failed.\n");
        goto error;
    }
    invokeMng->ngism_typeCount = typeCount;

    invokeMng->ngism_nJobsMax = maxJobs;

    /* Reset the counters */
    invokeMng->ngism_maxRequestID = 0;

    /* Register the Notify callback function. */
    result = ngiExternalModuleNotifyCallbackRegister(
        module,
        ngcllInvokeServerNotifyCallback,
        invokeMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Register the Notify callback failed.\n");
        goto error;
    }

    /* Query Features. */
    result = ngcllInvokeServerQueryFeatures(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed.\n");
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
 * Query Features.
 */
static int
ngcllInvokeServerQueryFeatures(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerQueryFeatures";
    char *serverType, *version, *errorMessage;
    int result, typeCount, requestSuccess;
    ngiLineList_t *features, *requests;
    char *cur, *candidate;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    requestSuccess = 0;
    version = NULL;
    features = NULL;
    requests = NULL;
    errorMessage = NULL;
    candidate = NULL;
    cur = NULL;

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Query Features. */
    result = ngiExternalModuleQueryFeatures(
        invokeMng->ngism_externalModule,
        &requestSuccess,
        &version, &features, &requests,
        &errorMessage, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed.\n");
        goto error;
    }

    if (requestSuccess == 0) {
        invokeMng->ngism_isG4 = 1;

        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed: <%s>. "
            "Assume this Invoke Server %s(%d) is Ninf-G4 version.\n",
            ((errorMessage != NULL) ? errorMessage : ""),
            serverType, typeCount);

        result = ngiFree(errorMessage, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Deallocate the string failed.\n");
            goto error;
        }
        errorMessage = NULL;

        /* Success */
        return 1;
    }

    /* Ninf-G 5.0.0 is the version 2.0 of Invoke Server. */
    /* Note : Latter version of Ninf-G must check the version. */

    if ((version == NULL) || (features == NULL) || (requests == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "invalid version, features, requests.\n");
        goto error;
    }

    /* Check the features. */
    cur = NULL;
    while ((cur = ngiLineListLineGetNext(
        features, cur, log, error)) != NULL) {

        candidate = NGCLI_INVOKE_SERVER_FEATURE_STAGING_AUTH_NUMBER;
        if (strcmp(cur, candidate) == 0) {

            ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "auth number can be staged as file on %s %s(%d).\n",
                "Invoke Server", serverType, typeCount);

            invokeMng->ngism_isAuthNumberStaging = 1;

            continue;
        }

	candidate = NGCLI_INVOKE_SERVER_FEATURE_STAGING_COMMUNICATION_PROXY;
	if (strcmp(cur, candidate) == 0) {

	    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,
		"remote communication proxy can be staged on %s %s(%d).\n",
                "Invoke Server", serverType, typeCount);

	    invokeMng->ngism_isCommunicationProxyStaging = 1;

	    continue;
	}

        /* else */
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Unknown feature \"%s\" on %s %s(%d).\n",
            cur, "Invoke Server", serverType, typeCount);
    }

    /* requests check suppressed. */

    result = ngiFree(version, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Deallocate the string failed.\n");
        goto error;
    }
    version = NULL;

    result = ngiLineListDestruct(features, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the Line List failed.\n");
        goto error;
    }
    features = NULL;

    result = ngiLineListDestruct(requests, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Destruct the Line List failed.\n");
        goto error;
    }
    requests = NULL;
    

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}


/**
 * Finalize
 */
static int
ngcllInvokeServerFinalize(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerFinalize";
    int result, returnCode, typeCount;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    assert(invokeMng->ngism_serverType != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;

    returnCode = 1;

    /* Finalizing the Invoke Server */
    invokeMng->ngism_working = 0;

    /* log */
    if (invokeMng->ngism_valid == 0) {
        ngclLogPrintfContext(context,
            NG_LOGCAT_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) was dead abnormally.\n",
            fName, serverType, typeCount);
    }

    /* Request EXIT and wait Reply. */
    result = ngcllInvokeServerRequestExit(context, invokeMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Request the EXIT to the Invoke Server failed.\n");
        returnCode = 0;
    }

    result = ngiExternalModuleDestruct(
        invokeMng->ngism_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Destruct the External Module failed.\n");
        returnCode = 0;
    }

    /* Finalize the Mutex and Cond */
    result = ngcllInvokeServerFinalizeMutexAndCond(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize Mutex and Cond for "
            "Invoke Server Manager.\n");
        returnCode = 0;
    }

    if (invokeMng->ngism_serverType != NULL) {
        free(invokeMng->ngism_serverType);
        invokeMng->ngism_serverType = NULL;
    }

    ngcllInvokeServerInitializeMember(invokeMng);

    if (returnCode == 0) {
        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
ngcllInvokeServerInitializeMember(
    ngcliInvokeServerManager_t *invokeMng)
{
    /* Check the arguments */
    assert(invokeMng != NULL);


    invokeMng->ngism_next = NULL;
    invokeMng->ngism_context = NULL;
    invokeMng->ngism_ID = 0;
    invokeMng->ngism_serverType = NULL;
    invokeMng->ngism_typeCount = 0;

    invokeMng->ngism_valid = 0;
    invokeMng->ngism_working = 0;
    invokeMng->ngism_errorCode = NG_ERROR_NO_ERROR;

    invokeMng->ngism_mutex = NGI_MUTEX_NULL;
    invokeMng->ngism_cond = NGI_COND_NULL;

    invokeMng->ngism_externalModule = NULL;
    invokeMng->ngism_nJobsMax = 0;
    invokeMng->ngism_maxRequestID = 0;

    invokeMng->ngism_isG4 = 0;
    invokeMng->ngism_isAuthNumberStaging = 0;
    invokeMng->ngism_isCommunicationProxyStaging = 0;
}

/**
 * Initialize mutex and cond
 */
static int
ngcllInvokeServerInitializeMutexAndCond(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerInitializeMutexAndCond";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;

    /* Initialize the mutex */
    result = ngiMutexInitialize(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Mutex.\n");
        return 0;
    }

    /* Initialize the condition variable */
    result = ngiCondInitialize(
        &invokeMng->ngism_cond, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Condition variable.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize mutex and cond
 */
static int
ngcllInvokeServerFinalizeMutexAndCond(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerFinalizeMutexAndCond";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;

    /* Finalize the mutex */
    result = ngiMutexDestroy(
        &invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Mutex.\n");
        return 0;
    }

    /* Finalize the cond */
    result = ngiCondDestroy(
        &invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Condition variable.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Retire all Invoke Server.
 */
int
ngcliInvokeServerRetire(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerRetire";
    ngcliInvokeServerManager_t *curInvokeMng;
    ngiExternalModuleManager_t *extMng;
    int result, isLocked, extLocked;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    isLocked = 0;
    extLocked = 0;
    extMng = context->ngc_externalModuleManager;

    /* Lock the list */
    result = ngcliContextInvokeServerManagerListReadLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Invoke Server.\n");
        goto error;
    }
    isLocked = 1;

    if (context->ngc_invokeMng_head == NULL) {
        goto final;
    }

    result = ngiExternalModuleManagerListReadLock(
        extMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the External Module.\n");
        goto error;
    }
    extLocked = 1;

    curInvokeMng = context->ngc_invokeMng_head;
    while (curInvokeMng != NULL) {

        /* log */
        ngclLogInfoContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Retire the Invoke Server %s(%d).\n",
            curInvokeMng->ngism_serverType,
            curInvokeMng->ngism_typeCount);

        /* Retire */
        result = ngiExternalModuleRetire(
            curInvokeMng->ngism_externalModule, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Retire the External Module failed.\n");
            goto error;
        }

        curInvokeMng = curInvokeMng->ngism_next;
    }

    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListReadUnlock(
            extMng, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
            goto error;
        }
    }

final:

    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListReadUnlock(
            context, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListWriteUnlock(
            extMng, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }
    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Initialize the Invoke Server information for each Job.
 */
int
ngcliInvokeServerJobInitialize(
    ngclContext_t *context,
    ngcliInvokeServerJob_t *invokeServerJobInfo,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(invokeServerJobInfo != NULL);

    ngcllInvokeServerJobInitializeMember(invokeServerJobInfo);

    /* Success */
    return 1;
}

/**
 * Finalize the Invoke Server information for each Job.
 */
int
ngcliInvokeServerJobFinalize(
    ngclContext_t *context,
    ngcliInvokeServerJob_t *invokeServerJobInfo,
    int *error)
{
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeServerJobInfo != NULL);

    log = context->ngc_log;

    if (invokeServerJobInfo->ngisj_invokeJobID != NULL) {
        ngiFree(invokeServerJobInfo->ngisj_invokeJobID, log, error);
        invokeServerJobInfo->ngisj_invokeJobID = NULL;
    }

    if (invokeServerJobInfo->ngisj_stdoutFile != NULL) {
        ngiFree(invokeServerJobInfo->ngisj_stdoutFile, log, error);
        invokeServerJobInfo->ngisj_stdoutFile = NULL;
    }

    if (invokeServerJobInfo->ngisj_stderrFile != NULL) {
        ngiFree(invokeServerJobInfo->ngisj_stderrFile, log, error);
        invokeServerJobInfo->ngisj_stderrFile = NULL;
    }

    invokeServerJobInfo->ngisj_invokeServer = NULL;

    ngcllInvokeServerJobInitializeMember(invokeServerJobInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the member of InvokeServerJob
 */
static void
ngcllInvokeServerJobInitializeMember(
    ngcliInvokeServerJob_t *invokeServerJobInfo)
{
    /* Check the arguments */
    assert(invokeServerJobInfo != NULL);

    invokeServerJobInfo->ngisj_invokeServer = NULL;
    invokeServerJobInfo->ngisj_requestID = 0;
    invokeServerJobInfo->ngisj_invokeJobID = NULL;
    invokeServerJobInfo->ngisj_invokeJobIDset = 0;
    invokeServerJobInfo->ngisj_jobDestroyed = 0;
    invokeServerJobInfo->ngisj_stdoutFile = NULL;
    invokeServerJobInfo->ngisj_stderrFile = NULL;
}

/**
 * Register the Invoke Server Manager to Ninf-G Context.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerRegister(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    /* Append at last of the list */
    invokeMng->ngism_next = NULL;
    if (context->ngc_invokeMng_head == NULL) {
        /* No information is registered yet */
        assert(context->ngc_invokeMng_tail == NULL);
        context->ngc_invokeMng_head = invokeMng;
        context->ngc_invokeMng_tail = invokeMng;
    } else {
        /* Some information are registered */
        assert(context->ngc_invokeMng_tail != NULL);
        assert(context->ngc_invokeMng_tail->ngism_next == NULL);
        context->ngc_invokeMng_tail->ngism_next = invokeMng;
        context->ngc_invokeMng_tail = invokeMng;
    }

    /* Count up */
    context->ngc_nInvokeServers++;

    /* Success */
    return 1;
}

/**
 * Unregister the Invoke Server Manager from Ninf-G Context.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerUnregister(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerUnregister";
    ngcliInvokeServerManager_t *prev, *curr;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(context->ngc_invokeMng_head != NULL);
    assert(context->ngc_invokeMng_tail != NULL);

    /* Find the Invoke Server Manager */
    prev = NULL;
    curr = context->ngc_invokeMng_head;
    for (; curr != invokeMng; curr = curr->ngism_next) {
        if (curr == NULL)
            goto notFound;
        prev = curr;
    }

    /* Unregister the Invoke Server Manager */
    if (invokeMng == context->ngc_invokeMng_head)
        context->ngc_invokeMng_head = invokeMng->ngism_next;
    if (invokeMng == context->ngc_invokeMng_tail)
        context->ngc_invokeMng_tail = prev;
    if (prev != NULL)
        prev->ngism_next = invokeMng->ngism_next;
    invokeMng->ngism_next = NULL;

    /* Count down */
    context->ngc_nInvokeServers--;

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogWarnContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server Manager is not found.\n");

    return 0;
}


/**
 * Treat the retired Invoke Servers.
 */
static int
ngcllInvokeServerTreatRetired(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerTreatRetired";
    int result, isLocked, extLocked, touched, retired;
    ngiExternalModuleManager_t *extMng;
    ngcliInvokeServerManager_t *cur;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    extMng = context->ngc_externalModuleManager;
    log = context->ngc_log;
    isLocked = 0;
    extLocked = 0;
    touched = 0;

    /* Lock the Invoke Server list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Invoke Server.\n");
        goto error;
    }
    isLocked = 1;

    /* Lock the External Module list */
    result = ngiExternalModuleManagerListWriteLock(
        extMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the External Module.\n");
        goto error;
    }
    extLocked = 1;

    do {
        touched = 0;

        cur = context->ngc_invokeMng_head;
        for (; cur != NULL; cur = cur->ngism_next) {
            if (cur->ngism_nJobsMax == 0) {
                continue;
            }

            retired = 0;
            result = ngiExternalModuleIsRetired(
                cur->ngism_externalModule, &retired, log, error);
            if (result == 0) {
                 ngclLogDebugContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Check the External Module retired failed.\n");
            }

            if (retired == 0) {
                continue;
            }

            /* Found the retired Invoke Server */
            touched = 1;

             ngclLogDebugContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Invoke Server %s(%d) retired.\n",
                cur->ngism_serverType, cur->ngism_typeCount);

            result = ngcliInvokeServerDestruct(context, cur, 0, error);
            if (result == 0) {
                 ngclLogDebugContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the Invoke Server.\n");

                result = ngcllInvokeServerUnregister(context, cur, NULL);
                if (result == 0) {
                     ngclLogDebugContext(context,
                        NG_LOGCAT_NINFG_PURE, fName,
                        "Can't unregister the Invoke Server.\n");
                }
            }

            break;
        }

    } while (touched != 0);

    /* Unlock the External Module list */
    extLocked = 0;
    result = ngiExternalModuleManagerListWriteUnlock(
        extMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the External Module.\n");
        goto error;
    }

    /* Unlock the Invoke Server list */
    isLocked = 0;
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Invoke Server.\n");
        goto error;
    }


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the External Module list */
    if (extLocked != 0) {
        extLocked = 0;
        result = ngiExternalModuleManagerListWriteUnlock(
            extMng, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
        }
    }

    /* Unlock the Invoke Server list */
    if (isLocked != 0) {
        isLocked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Invoke Server.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Invoke Server Is Valid (Unusable not performed?)
 */
static int
ngcllInvokeServerIsValid(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *valid,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerIsValid";
    int result, locked, moduleValid;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(valid != NULL);

    log = context->ngc_log;
    locked = 0;

    *valid = 1;

    /* Wait until Unusable finish. */

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Mutex.\n");
        goto error;
    }
    locked = 1;

    /* Check the flag. */
    if (invokeMng->ngism_valid == 0) {
        *valid = 0;
    }

    if (invokeMng->ngism_working == 0) {
        *valid = 0;
    }

    /* Check the External Module. */
    moduleValid = 0;
    result = ngiExternalModuleIsValid(
        invokeMng->ngism_externalModule, &moduleValid, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Check the External Module valid failed.\n");
        goto error;
    }

    if (moduleValid == 0) {
        *valid = 0;
    }

    /* Unlock */
    locked = 0;
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    *valid = 0;

    /* Unlock */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Mutex.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Start Job on the Invoke Server
 * If the Invoke Server is not started, the Invoke Server is started.
 */
int
ngcliInvokeServerJobStart(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStart";
    ngcliInvokeServerManager_t *invokeMng;
    int result, valid, typeCount;
    char *serverType;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(jobMng->ngjm_attr.ngja_rmInfoExist != 0);
    assert(jobMng->ngjm_attr.ngja_rmInfo.ngrmi_invokeServerType != NULL);
    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    assert(jobMng->ngjm_attr.ngja_isInfo.ngisi_type != NULL);

    serverType = jobMng->ngjm_attr.ngja_rmInfo.ngrmi_invokeServerType;
    typeCount = -1;
    valid = 0;

    /* Construct the Invoke Server if not done. */
    invokeMng = ngcliInvokeServerConstruct(
        context, serverType, jobMng, error);
    if (invokeMng == NULL) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server for %s.\n",
            serverType);
        return 0;
    }

    typeCount = invokeMng->ngism_typeCount;

    /* Set Invoke Server to the Job */
    jobMng->ngjm_invokeServerInfo.ngisj_invokeServer = invokeMng;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server %s(%d) validity.\n",
            serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) already dead.\n",
            serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server %s(%d) Job Start.\n",
        serverType, typeCount);

    /* Create the stdout/stderr temporary file */
    result = ngcllInvokeServerJobStdoutStderrCreate(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the file for Job stdout/stderr.\n");
        /* Not return */
    }

    /* Request the JOB_CREATE and wait Reply */
    result = ngcllInvokeServerRequestJobCreate(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't Request to the Invoke Server.\n");
        return 0;
    }

    /* Wait the Invoke Server JobID (CREATE_NOTIFY) */
    result = ngcllInvokeServerJobIDwait(context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't wait the Invoke Server JobID.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Stop Job on the Invoke Server
 */
int
ngcliInvokeServerJobStop(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStop";
    ngcliInvokeServerManager_t *invokeMng;
    int result, valid, typeCount;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    valid = 0;
    log = context->ngc_log;

    invokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server.\n");
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server %s(%d) validity.\n",
            serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        ngcliLogInfoJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) already dead.\n",
            serverType, typeCount);

        /* Success */
        return 1;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server %s(%d) Job Stop.\n",
        serverType, typeCount);

    /* Output the Job stdout/stderr */
    result = ngcllInvokeServerJobStdoutStderrOutput(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) Job output stdout/stderr failed.\n",
            serverType, typeCount);
        /* Not return */
    }

    /* Destroy the Job stdout/stderr */
    result = ngcllInvokeServerJobStdoutStderrDestroy(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) Job stdout/stderr destroy failed.\n",
            serverType, typeCount);
        /* Not return */
    }

    /* Is job destroyed? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed != 0) {
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "The job is already destroyed.\n");

        /* Success */
        return 1;
    }

    /* Request the JOB_DESTROY and wait Reply */
    result = ngcllInvokeServerRequestJobDestroy(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't Request to the Invoke Server.\n");
        return 0;
    }

    jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed = 1;

    /* Tell the job stop to the External Module. */
    result = ngiExternalModuleJobStopped(
        invokeMng->ngism_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Tell the job stop to External Module failed.\n");
        return 0;
    }

    result = ngcllInvokeServerTreatRetired(context, NULL);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't treat the retired Invoke Servers.\n");
    }

    /* Success */
    return 1;
}

/**
 * Cancel Job on the Invoke Server
 */
int
ngcliInvokeServerJobCancel(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *doNotWait,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobCancel";
    ngcliInvokeServerManager_t *invokeMng;
    int result, valid, typeCount;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(doNotWait != NULL);

    valid = 0;
    *doNotWait = 0;
    log = context->ngc_log;

    invokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server.\n");
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server %s(%d) validity.\n",
            serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        *doNotWait = 1;

        ngcliLogInfoJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) already dead.\n",
            serverType, typeCount);

        /* Success */
        return 1;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server %s(%d) Job Cancel.\n",
        serverType, typeCount);

    /* Is job destroyed? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed != 0) {
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "The job is already destroyed.\n");

        /* Success */
        return 1;
    }

    /* Done? */
    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Job is already done. suppress job cancel.\n");

        /* Success */
        return 1;
    }

    /* Cancel requested? */
    if (jobMng->ngjm_requestCancel == 0) {
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Job cancel was not requested: All Executables are exited.\n");

        /* Success */
        return 1;
    }

    /* Request the JOB_DESTROY and wait Reply */
    result = ngcllInvokeServerRequestJobDestroy(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't Request to the Invoke Server.\n");
        return 0;
    }

    jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed = 1;

    /* Tell the job stop to the External Module. */
    result = ngiExternalModuleJobStopped(
        invokeMng->ngism_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Tell the job stop to External Module failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the Job Status from the Invoke Server
 */
int
ngcliInvokeServerJobStatusGet(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStatusGet";
    ngcliInvokeServerManager_t *invokeMng;
    int result, valid, typeCount;
    char *serverType;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    valid = 0;

    invokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server.\n");
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Invoke Server %s(%d) validity.\n",
            serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) already dead.\n",
            serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server %s(%d) Job Status Get.\n",
        serverType, typeCount);

    /* Request the JOB_STATUS and wait Reply */
    result = ngcllInvokeServerRequestJobStatus(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't Request to the Invoke Server.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Request the JOB_CREATE and wait reply.
 */
static int
ngcllInvokeServerRequestJobCreate(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestJobCreate";
    char requestIDstring[NGI_INT_MAX_DECIMAL_DIGITS];
    int result, typeCount, requestID, replySuccess;
    char *serverType, *requestName, *replyMessage;
    ngiLineList_t *arguments;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    arguments = NULL;
    log = context->ngc_log;

    requestName = NGCLI_INVOKE_SERVER_REQUEST_JOB_CREATE;
    invokeMng->ngism_maxRequestID++;
    requestID = invokeMng->ngism_maxRequestID;
    jobMng->ngjm_invokeServerInfo.ngisj_requestID = requestID;

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "New RequestID %d on Invoke Server %s(%d).\n",
        requestID, serverType, typeCount);

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Request \"%s %d\" for Invoke Server %s(%d).\n",
        requestName, requestID, serverType, typeCount);

    /* Set the start time */
    result = ngiSetStartTime(&jobMng->ngjm_invoke, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't set the Start time.\n");
        goto error;
    }

    snprintf(
        requestIDstring, sizeof(requestIDstring),
        "%d", requestID);

    arguments = ngiLineListConstruct(log, error);
    if (arguments == NULL) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Construct the Line List failed.\n");
        goto error;
    }

    result = ngcllInvokeServerRequestJobCreateArgument(
        context, invokeMng, arguments, jobMng, error);
    if ((result == 0) || (arguments == NULL)) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Create the Invoke Server %s arguments failed.\n",
            requestName);
        goto error;
    }

    /* Request and wait Reply. */
    replySuccess = 0;
    replyMessage = NULL;
    result = ngiExternalModuleRequest(
        invokeMng->ngism_externalModule,
        requestName, requestIDstring, arguments,
        0, /* No timeout is implemented. */
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Request the Invoke Server failed.\n");
        goto error;
    }

    if (replySuccess == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Error on Invoke Server %s(%d): \"%s\".\n",
            serverType, typeCount,
            ((replyMessage != NULL) ? replyMessage : ""));

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) request %s failed.\n",
            serverType, typeCount, requestName);
        goto error;
    }

    assert(replySuccess != 0);
    if (replyMessage != NULL) {
        ngcliLogWarnJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unexpected message on Invoke Server %s(%d) %s: \"%s\".\n",
            serverType, typeCount, requestName, replyMessage);
    }

    result = ngiLineListDestruct(arguments, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Destruct the Line List failed.\n");
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
 * Create the Request JOB_CREATE argument.
 */
static int
ngcllInvokeServerRequestJobCreateArgument(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngiLineList_t *arguments,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestJobCreateArgument";
    char *backend, *workDirectory, *p;
    char *tmpFile, *tmpDir, *trueStr, *falseStr;
    ngclRemoteMachineInformation_t *rmInfo;
    int result, portNo, interval, count, i;
    ngiCommunication_t *comm;
    ngiLineList_t *arg;
    ngLog_t *log;
    char *remoteCommProxyPath;
    char *remoteCommProxyPathTmp;
    char *it;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(arguments != NULL);
    assert(jobMng != NULL);

    arg = arguments;

    comm = NULL;
    backend = NULL;
    portNo = 0;
    trueStr = NULL;
    falseStr = NULL;
    workDirectory = NULL;
    tmpFile = NULL;
    tmpDir = NULL;
    p = NULL;
    interval = 0;
    count = 0;

    trueStr = "true";
    falseStr = "false";

    remoteCommProxyPath = NULL;
    remoteCommProxyPathTmp = NULL;

    log = context->ngc_log;

    rmInfo = &jobMng->ngjm_attr.ngja_rmInfo;
    comm = jobMng->ngjm_context->ngc_comm;

    /* Get the backend */
    switch (jobMng->ngjm_attr.ngja_backend) {
    case NG_BACKEND_NORMAL:
        backend = "NORMAL";
        break;

    case NG_BACKEND_MPI:
        backend = "MPI";
        break;

    case NG_BACKEND_BLACS:
        backend = "BLACS";
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unknown Backend Type %d.\n",
            jobMng->ngjm_attr.ngja_backend);
        goto error;
    }

    /* Work Directory */
    workDirectory = NULL;
    if (rmInfo->ngrmi_workDirectory != NULL) {
        workDirectory = strdup(rmInfo->ngrmi_workDirectory);
        if (workDirectory == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the Executable Path.\n");
            goto error;
        }
    } else if (jobMng->ngjm_attr.ngja_stagingEnable == 0) {
        assert(jobMng->ngjm_attr.ngja_executablePath != NULL);
        workDirectory = strdup(jobMng->ngjm_attr.ngja_executablePath);
        if (workDirectory == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the Executable Path.\n");
            goto error;
        }

        p = strrchr(workDirectory, '/');
        if (p != NULL) {
            assert(*p == '/');
            *p = '\0';
        }
    }

    /* Print the JOB_CREATE arguments */

    /* invoke_server_option on <INVOKE_SERVER> section */
    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    for (i = 0; i < jobMng->ngjm_attr.ngja_isInfo.ngisi_nOptions; i++) {

        result = ngiLineListPrintf(arg, log, error,
            "%s", jobMng->ngjm_attr.ngja_isInfo.ngisi_options[i]);
        if (result == 0) {
            goto error;
        }
    }

    /* invoke_server_option on <SERVER> section */
    for (i = 0; i < rmInfo->ngrmi_invokeServerNoptions; i++) {
        result = ngiLineListPrintf(arg, log, error,
            "%s", rmInfo->ngrmi_invokeServerOptions[i]);
        if (result == 0) {
            goto error;
        }
    }

    /* hostname */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "hostname", jobMng->ngjm_attr.ngja_hostName);
    if (result == 0) {
        goto error;
    }

    /* port */
    portNo = jobMng->ngjm_attr.ngja_portNo;
    result = ngiLineListPrintf(arg, log, error,
        "%s %d", "port", ((portNo != NGI_PORT_ANY) ? portNo : 0));
    if (result == 0) {
        goto error;
    }

    /* jobmanager */
    if (jobMng->ngjm_attr.ngja_jobManager != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "jobmanager", jobMng->ngjm_attr.ngja_jobManager);
        if (result == 0) {
            goto error;
        }
    }

    /* subject */
    if (jobMng->ngjm_attr.ngja_subject != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "subject", jobMng->ngjm_attr.ngja_subject);
        if (result == 0) {
            goto error;
        }
    }

    /* client_name */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "client_name", jobMng->ngjm_attr.ngja_clientHostName);
    if (result == 0) {
        goto error;
    }

    /* path (remote path or local path) */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "executable_path", jobMng->ngjm_attr.ngja_executablePath);
    if (result == 0) {
        goto error;
    }

    /* backend */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "backend", backend);
    if (result == 0) {
        goto error;
    }

    /* count */
    count = 1; /* for dummy */
    if ((jobMng->ngjm_attr.ngja_backend == NG_BACKEND_MPI) ||
        (jobMng->ngjm_attr.ngja_backend == NG_BACKEND_BLACS)) {
        count = jobMng->ngjm_attr.ngja_mpiNcpus;
    } else {
        count = jobMng->ngjm_attr.ngja_invokeNjobs;
    }
    result = ngiLineListPrintf(arg, log, error,
        "%s %d", "count", count);
    if (result == 0) {
        goto error;
    }

    /* staging */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "staging",
        ((jobMng->ngjm_attr.ngja_stagingEnable != 0) ? trueStr : falseStr));
    if (result == 0) {
        goto error;
    }

    /* Simple Auth Number */
    if (invokeMng->ngism_isAuthNumberStaging != 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "auth_number",
            jobMng->ngjm_simpleAuthNumber);
        if (result == 0) {
            goto error;
        }
    }

    /* argument : connectbackAddress */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s=ng_tcp://%s:%d/", "argument", "--connectbackAddress",
        jobMng->ngjm_attr.ngja_clientHostName, comm->ngc_portNo);
    if (result == 0) {
        goto error;
    }

    /* argument : Simple Auth Number */
    if (invokeMng->ngism_isAuthNumberStaging == 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%d", "argument", "--authNumber",
            jobMng->ngjm_simpleAuthNumber);
        if (result == 0) {
            goto error;
        }
    }

    /* argument : contextID */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s=%d", "argument", "--contextID", jobMng->ngjm_context->ngc_ID);
    if (result == 0) {
        goto error;
    }

    /* argument : jobID */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s=%d", "argument", "--jobID", jobMng->ngjm_ID);
    if (result == 0) {
        goto error;
    }

    /* argument : heartBeat */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s=%d", "argument", "--heartbeat", rmInfo->ngrmi_heartBeat);
    if (result == 0) {
        goto error;
    }

    /* argument : TCP Connect Retry */
    if (rmInfo->ngrmi_retryInfo.ngcri_count > 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%d,%d,%g,%s", "argument", "--connectRetry",
            rmInfo->ngrmi_retryInfo.ngcri_count,
            rmInfo->ngrmi_retryInfo.ngcri_interval,
            rmInfo->ngrmi_retryInfo.ngcri_increase,
            ((rmInfo->ngrmi_retryInfo.ngcri_useRandom == 1) ?
                "random" : "fixed"));
        if (result == 0) {
            goto error;
        }
    }

    /* argument : coreDumpSize */
    if (rmInfo->ngrmi_coreDumpSize != -2) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%d", "argument", "--coreDumpSize",
            rmInfo->ngrmi_coreDumpSize);
        if (result == 0) {
            goto error;
        }
    }

    /* argument : debug by terminal */
    if (rmInfo->ngrmi_debug.ngdi_enable) {
        if (rmInfo->ngrmi_debug.ngdi_terminalPath != NULL) {
            result = ngiLineListPrintf(arg, log, error,
                "%s %s=%s", "argument", "--debugTerminal",
                rmInfo->ngrmi_debug.ngdi_terminalPath);
            if (result == 0) {
                goto error;
            }
        }

        if (rmInfo->ngrmi_debug.ngdi_display != NULL) {
            result = ngiLineListPrintf(arg, log, error,
                "%s %s=%s", "argument", "--debugDisplay",
                rmInfo->ngrmi_debug.ngdi_display);
            if (result == 0) {
                goto error;
            }
        }

        if (rmInfo->ngrmi_debug.ngdi_debuggerPath != NULL) {
            result = ngiLineListPrintf(arg, log, error,
                "%s %s=%s", "argument", "--debugger",
                rmInfo->ngrmi_debug.ngdi_debuggerPath);
            if (result == 0) {
                goto error;
            }
        }

        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%d", "argument", "--debugEnable",
            rmInfo->ngrmi_debug.ngdi_enable);
        if (result == 0) {
            goto error;
        }
    }

    /* argument : debugBusyLoop */
    if (rmInfo->ngrmi_debugBusyLoop) {
        ngcliLogInfoJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "debug_busyLoop was set."
            " please attach Remote Executable by debugger.\n");

        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%d", "argument", "--debugBusyLoop",
            rmInfo->ngrmi_debugBusyLoop);
        if (result == 0) {
            goto error;
        }
    }

    /* argument : tcpNodelay */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s=%d", "argument", "--tcpNodelay",
        rmInfo->ngrmi_tcpNodelay);
    if (result == 0) {
        goto error;
    }

    if (rmInfo->ngrmi_commProxyType != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%s", "argument", "--communicationProxyType",
            rmInfo->ngrmi_commProxyType);
        if (result == 0) {
            goto error;
        }
    }

    /* If Remote Communication Proxy is staged in,
     * --communicationProxyPath options is appended by Invoke Server. */
    if ((rmInfo->ngrmi_commProxyPath != NULL) &&
        (rmInfo->ngrmi_commProxyStaging == 0)) {
        assert(rmInfo->ngrmi_commProxyType != NULL);
        result = ngiLineListPrintf(arg, log, error,
            "%s %s=%s", "argument", "--communicationProxyPath",
            rmInfo->ngrmi_commProxyPath);
        if (result == 0) {
            goto error;
        }
    }

    if (jobMng->ngjm_clientCommunicationProxyInfo != NULL) {
        assert(rmInfo->ngrmi_commProxyType != NULL);
        it = NULL;
        while ((it = ngiLineListLineGetNext(
            jobMng->ngjm_clientCommunicationProxyInfo,
            it, log, error)) != NULL) {

            result = ngiLineListPrintf(arg, log, error,
                "%s %s=%s", "argument", "--communicationProxyOption", it);
            if (result == 0) {
                goto error;
            }
        }
    }

    /* workDirectory */
    if (workDirectory != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "work_directory", workDirectory);
        if (result == 0) {
            goto error;
        }
    }

    /* redirect stdout/stderr */
    result = ngiLineListPrintf(arg, log, error,
        "%s %s", "redirect_enable",
        ((rmInfo->ngrmi_redirectEnable != 0) ? trueStr : falseStr));
    if (result == 0) {
        goto error;
    }

    /* redirect stdout filename */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile;
    if (tmpFile != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "stdout_file", tmpFile);
        if (result == 0) {
            goto error;
        }
    }

    /* redirect stdout filename */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stderrFile;
    if (tmpFile != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "stderr_file", tmpFile);
        if (result == 0) {
            goto error;
        }
    }

    /* environment variable */
    for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "environment", rmInfo->ngrmi_environment[i]);
        if (result == 0) {
            goto error;
        }
    }

    /* tmp_dir */
    tmpDir = jobMng->ngjm_attr.ngja_lmInfo.nglmi_tmpDir;
    if (tmpDir != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "tmp_dir", tmpDir);
        if (result == 0) {
            goto error;
        }
    }

    /* jobStatus polling */
    interval = jobMng->ngjm_attr.ngja_isInfo.ngisi_statusPoll;
    result = ngiLineListPrintf(arg, log, error,
        "%s %d", "status_polling", ((interval > 0) ? interval : 0));
    if (result == 0) {
        goto error;
    }

    /* refresh credentials */
    interval = jobMng->ngjm_attr.ngja_lmInfo.nglmi_refreshInterval;
    result = ngiLineListPrintf(arg, log, error,
        "%s %d", "refresh_credential", ((interval > 0) ? interval : 0));
    if (result == 0) {
        goto error;
    }

    /* maxTime */
    if (rmInfo->ngrmi_jobMaxTime >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "max_time", rmInfo->ngrmi_jobMaxTime);
        if (result == 0) {
            goto error;
        }
    }

    /* maxWallTime */
    if (rmInfo->ngrmi_jobMaxWallTime >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "max_wall_time", rmInfo->ngrmi_jobMaxWallTime);
        if (result == 0) {
            goto error;
        }
    }

    /* maxCpuTime */
    if (rmInfo->ngrmi_jobMaxCpuTime >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "max_cpu_time", rmInfo->ngrmi_jobMaxCpuTime);
        if (result == 0) {
            goto error;
        }
    }

    /* queue */
    if (jobMng->ngjm_attr.ngja_queueName != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "queue_name", jobMng->ngjm_attr.ngja_queueName);
        if (result == 0) {
            goto error;
        }
    }

    /* project */
    if (rmInfo->ngrmi_jobProject != NULL) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "project", rmInfo->ngrmi_jobProject);
        if (result == 0) {
            goto error;
        }
    }

    /* hostCount */
    if (rmInfo->ngrmi_jobHostCount >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "host_count", rmInfo->ngrmi_jobHostCount);
        if (result == 0) {
            goto error;
        }
    }

    /* minMemory */
    if (rmInfo->ngrmi_jobMinMemory >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "min_memory", rmInfo->ngrmi_jobMinMemory);
        if (result == 0) {
            goto error;
        }
    }

    /* maxMemory */
    if (rmInfo->ngrmi_jobMaxMemory >= 0) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %d", "max_memory", rmInfo->ngrmi_jobMaxMemory);
        if (result == 0) {
            goto error;
        }
    }

    /* communication_proxy_staging */
    if (rmInfo->ngrmi_commProxyStaging != 0) {
        assert(rmInfo->ngrmi_commProxyType != NULL);

	if (invokeMng->ngism_isCommunicationProxyStaging == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngcliLogErrorJob(jobMng,
		NG_LOGCAT_NINFG_PURE, fName,
		"Invoke server is not supporting staging of Remote Communication Proxy.\n");
	    goto error;
	}

        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "communication_proxy_staging", trueStr);
        if (result == 0) {
            goto error;
        }

        if (rmInfo->ngrmi_commProxyPath != NULL) {
            remoteCommProxyPath = rmInfo->ngrmi_commProxyPath;
        } else {
            result = ngiExternalModuleProgramNameGet(
                NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
                NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY,
                rmInfo->ngrmi_commProxyType,
                &remoteCommProxyPathTmp, log, error);
            if (result == 0) {
                goto error;
            }
            remoteCommProxyPath = remoteCommProxyPathTmp;
        }

        /* communication_proxy_path */
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "communication_proxy_path", remoteCommProxyPath);
        if (result == 0) {
            goto error;
        }
        if (remoteCommProxyPathTmp != NULL) {
            result = ngiFree(remoteCommProxyPathTmp, log, error);
            if (result == 0) {
                goto error;
            }
        }
    }

    /* job_rslExtensions */
    for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
        result = ngiLineListPrintf(arg, log, error,
            "%s %s", "rsl_extensions", rmInfo->ngrmi_rslExtensions[i]);
        if (result == 0) {
            goto error;
        }

    }

    if (workDirectory != NULL) {
        ngiFree(workDirectory, log, error);
        workDirectory = NULL;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (remoteCommProxyPathTmp != NULL) {
        result = ngiFree(remoteCommProxyPathTmp, log, error);
        if (result == 0) {
            ;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Request the JOB_STATUS and wait reply.
 */
static int
ngcllInvokeServerRequestJobStatus(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestJobStatus";
    char *serverType, *requestName, *jobID, *replyMessage;
    int typeCount, replySuccess, result;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    requestName = NGCLI_INVOKE_SERVER_REQUEST_JOB_STATUS;
    jobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;
    if (jobID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get JobID for Request \"%s\""
            " on Invoke Server %s(%d).\n",
            requestName, serverType, typeCount);
        goto error;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Request \"%s %s\" for Invoke Server %s(%d).\n",
        requestName, jobID, serverType, typeCount);

    /* Request and wait Reply. */
    replySuccess = 0;
    replyMessage = NULL;
    result = ngiExternalModuleRequest(
        invokeMng->ngism_externalModule,
        requestName, jobID, NULL,
        0, /* No timeout is implemented. */
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Request the Invoke Server failed.\n");
        goto error;
    }

    if (replySuccess == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Error on Invoke Server %s(%d): \"%s\".\n",
            serverType, typeCount,
            ((replyMessage != NULL) ? replyMessage : ""));

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) request %s failed.\n",
            serverType, typeCount, requestName);
        goto error;
    }

    assert(replySuccess != 0);

    result = ngcllInvokeServerReplyJobStatus(
        context, invokeMng, jobID, replyMessage, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Process the Invoke Server Reply failed.\n");
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
 * Process the Reply JOB_STATUS.
 */
static int
ngcllInvokeServerReplyJobStatus(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *jobID,
    char *argument,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyJobStatus";
    char *cur, *serverType, *statusName, *messageString;
    const ngcllInvokeServerStatusTable_t *statusTable;
    int result, i, found, jobMngLocked;
    ngcllInvokeServerStatus_t status;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobID != NULL);
    assert(argument != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    statusTable = NULL;
    statusName = NULL;
    messageString = NULL;
    jobMngLocked = 0;

    cur = argument;

    /* Find the status */
    found = 0;
    status = NGCLI_INVOKE_SERVER_STATUS_UNDEFINED;
    statusTable = ngcllInvokeServerStatusTable;
    for (i = 0; statusTable[i].ngisst_valid != 0; i++) {
        if (strncmp(cur, statusTable[i].ngisst_name,
            strlen(statusTable[i].ngisst_name)) == 0) {

            status = statusTable[i].ngisst_status;
            statusName = statusTable[i].ngisst_name;
            cur += strlen(statusTable[i].ngisst_name);

            found = 1;
            break;
        }
    }

    if (found == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid state returned from Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, jobID);

        return 0;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Job Manager.\n");
        goto error;
    }
    jobMngLocked = 1;

    result = ngcllInvokeServerNotifyJobStatusChange(
        context, invokeMng, jobID, status, statusName,
        messageString, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't change the status for Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, jobID);
        goto error;
    }

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Job Manager.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the list of Job Manager */
    if (jobMngLocked != 0) {
        jobMngLocked = 0;
        result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the list of Job Manager.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Request the JOB_DESTROY and wait reply.
 */
static int
ngcllInvokeServerRequestJobDestroy(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestJobDestroy";
    char *serverType, *requestName, *jobID, *replyMessage;
    int result, typeCount, replySuccess;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;

    requestName = NGCLI_INVOKE_SERVER_REQUEST_JOB_DESTROY;
    jobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;
    if (jobID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get JobID for Request \"%s\""
            " on Invoke Server %s(%d).\n",
            requestName, serverType, typeCount);
        goto error;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Request \"%s %s\" for Invoke Server %s(%d).\n",
        requestName, jobID, serverType, typeCount);

    /* Request and wait Reply. */
    replySuccess = 0;
    replyMessage = NULL;
    result = ngiExternalModuleRequest(
        invokeMng->ngism_externalModule,
        requestName, jobID, NULL,
        0, /* No timeout is implemented. */
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Request the Invoke Server failed.\n");
        goto error;
    }

    if (replySuccess == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Error on Invoke Server %s(%d): \"%s\".\n",
            serverType, typeCount,
            ((replyMessage != NULL) ? replyMessage : ""));

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) request %s failed.\n",
            serverType, typeCount, requestName);
        goto error;
    }

    assert(replySuccess != 0);
    if (replyMessage != NULL) {
        ngcliLogWarnJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unexpected message on Invoke Server %s(%d) %s: \"%s\".\n",
            serverType, typeCount, requestName, replyMessage);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Request the EXIT and wait reply.
 */
static int
ngcllInvokeServerRequestExit(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestExit";
    char *serverType, *requestName, *replyMessage;
    int result, typeCount, replySuccess;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;

    requestName = NGCLI_INVOKE_SERVER_REQUEST_EXIT;

    /* log */
    ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Request \"%s\" for Invoke Server %s(%d).\n",
        requestName, serverType, typeCount);

    /* Request and wait Reply. */
    replySuccess = 0;
    replyMessage = NULL;
    result = ngiExternalModuleRequest(
        invokeMng->ngism_externalModule,
        requestName, NULL, NULL,
        0, /* No timeout is implemented. */
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Request the Invoke Server failed.\n");
        goto error;
    }

    if (replySuccess == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Error on Invoke Server %s(%d): \"%s\".\n",
            serverType, typeCount,
            ((replyMessage != NULL) ? replyMessage : ""));

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) request %s failed.\n",
            serverType, typeCount, requestName);
        goto error;
    }

    assert(replySuccess != 0);
    if (replyMessage != NULL) {
        ngclLogWarnContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unexpected message on Invoke Server %s(%d) %s: \"%s\".\n",
            serverType, typeCount, requestName, replyMessage);
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Notify callback.
 */
static int
ngcllInvokeServerNotifyCallback(
    void *argument,
    ngiExternalModuleNotifyState_t state,
    char *notifyName,
    char *message,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyCallback";
    char *createNotifyString, *statusNotifyString, *stateName;
    ngcliInvokeServerManager_t *invokeMng;
    ngclContext_t *context;

    int result, jobMngLocked;
    char *serverType;
    int typeCount;

    /* Check the arguments */
    assert(argument != NULL);
    assert(state > NGI_EXTERNAL_MODULE_NOTIFY_NONE);
    assert(state < NGI_EXTERNAL_MODULE_NOTIFY_NOMORE);

    invokeMng = (ngcliInvokeServerManager_t *)argument;
    context = invokeMng->ngism_context;

    createNotifyString = NGCLI_INVOKE_SERVER_NOTIFY_CREATE;
    statusNotifyString = NGCLI_INVOKE_SERVER_NOTIFY_STATUS;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;
    jobMngLocked = 0;
    stateName = ngiExternalModuleNotifyStateToString(state);

    ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Got the Notify (state=%s) for Invoke Server %s(%d).\n",
        stateName, serverType, typeCount);

    if (state == NGI_EXTERNAL_MODULE_NOTIFY_CANCELED) {
        return 1;
    }

    if ((state == NGI_EXTERNAL_MODULE_NOTIFY_CLOSED) &&
        (invokeMng->ngism_working == 0)) {
        return 1;
    }

    if ((state == NGI_EXTERNAL_MODULE_NOTIFY_ERROR) ||
        (state == NGI_EXTERNAL_MODULE_NOTIFY_CLOSED)) {

        /* log */
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Got the Notify %s from Invoke Server %s(%d).\n",
            stateName, serverType, typeCount);

        /* Make the Invoke Server unusable. */
        result = ngcllInvokeServerUnusable(context, invokeMng, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Unusable failed.\n");
            goto error;
        }

        goto error;
    }

    assert(state == NGI_EXTERNAL_MODULE_NOTIFY_NORMAL);

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Got the Notify \"%s\" from Invoke Server %s(%d).\n",
        notifyName, serverType, typeCount);

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Job Manager.\n");
        goto error;
    }
    jobMngLocked = 1;

    if (strcmp(notifyName, createNotifyString) == 0) {

        result = ngcllInvokeServerNotifyProcessCreateNotify(
            context, invokeMng, createNotifyString, message, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't process %s.\n", createNotifyString);
            goto error;
        }

    } else if (strcmp(notifyName, statusNotifyString) == 0) {

        result = ngcllInvokeServerNotifyProcessStatusNotify(
            context, invokeMng, statusNotifyString, message, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't process %s.\n", statusNotifyString);
            goto error;
        }

    } else {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unknown Notify \"%s\" ignored.\n",
            notifyName);
    }

    /* Register the Notify callback function. */
    result = ngiExternalModuleNotifyCallbackRegister(
        invokeMng->ngism_externalModule,
        ngcllInvokeServerNotifyCallback,
        invokeMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Register the Notify callback failed.\n");
        goto error;
    }

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Job Manager.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Job Manager */
    if (jobMngLocked != 0) {
        jobMngLocked = 0;
        result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the list of Job Manager.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify CREATE_NOTIFY.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerNotifyProcessCreateNotify(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *notifyName,
    char *argument,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyProcessCreateNotify";
    int result, requestID, i, found, notifyResult, invokeJobIDvalid;
    char invokeJobID[NGCLI_INVOKE_SERVER_JOB_ID_STR_MAX];
    ngcliInvokeServerManager_t *curInvokeMng;
    char *cur, *end, *tmp, *serverType;
    ngcliJobManager_t *jobMng;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(argument != NULL);

    jobMng = NULL;
    curInvokeMng = NULL;
    notifyResult = 0;
    invokeJobID[0] = '\0';
    invokeJobIDvalid = 0;
    requestID = 0;
    end = NULL;
    tmp = NULL;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    cur = argument;

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Read the RequestID */
    requestID = strtol(cur, &end, 10);
    if ((end == cur) || (requestID <= 0)) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid RequestID for %s.\n", notifyName);

        /* Do not finish the Reply Reader thread */
        return 1;
    }
    cur = end;

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Read the Success or Fail */
    notifyResult = 0;
    if (strncmp(cur, NGCLI_INVOKE_SERVER_RESULT_SUCCESS,
        strlen(NGCLI_INVOKE_SERVER_RESULT_SUCCESS)) == 0) {

        /* log */
         ngclLogDebugContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "%s result for RequestID %d is success.\n",
            notifyName, requestID);

        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_SUCCESS);
        notifyResult = 1;

    } else if (strncmp(cur, NGCLI_INVOKE_SERVER_RESULT_FAILURE,
        strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE)) == 0) {

        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "%s result for RequestID %d is failure.\n",
            notifyName, requestID);

        /* Print error message */
        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE);

        /* Skip space */
        while(isspace((int)*cur)) {
            cur++;
        }

        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "RequestID %d JobID creation failed by \"%s\".\n",
            requestID, cur);

        notifyResult = 0;

    } else {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Unknown result for %s. (RequestID %d)\n",
            notifyName, requestID);

        notifyResult = 0;
    }

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Read the Job ID */
    if (notifyResult != 0) {
        invokeJobIDvalid = 1;
        i = 0;
        while (!isspace((int)*cur) && (*cur != '\0')) {
            invokeJobID[i] = *cur;
            cur++;
            i++;
            if (i >= (sizeof(invokeJobID) - 1)) {
                ngclLogErrorContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Invoke Server %s(%d) Job ID too long"
                    " for %s RequestID %d.\n",
                    serverType, typeCount, notifyName, requestID);

                invokeJobIDvalid = 0;
                break;
            }
        }
        if (i < (sizeof(invokeJobID) - 1)) {
            invokeJobID[i] = '\0';
        }
    }

    /* Find the job by Request ID */
    found = 0;

    for (jobMng = context->ngc_jobMng_head; jobMng != NULL;
        jobMng = jobMng->ngjm_next) {

       if (jobMng->ngjm_useInvokeServer == 0) {
            continue;
        }

        /* Compare Invoke Server Type */
        curInvokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
        if (curInvokeMng == NULL) {
            continue;
        }

        if (curInvokeMng->ngism_serverType == NULL) {
            continue;
        }

        assert(invokeMng->ngism_serverType != NULL);

        if (strcmp(curInvokeMng->ngism_serverType,
            invokeMng->ngism_serverType) != 0) {
            continue;
        }

        if (curInvokeMng->ngism_typeCount != invokeMng->ngism_typeCount) {
            continue;
        }

        /* Compare Invoke Server RequestID */
        if (jobMng->ngjm_invokeServerInfo.ngisj_requestID == requestID) {
            found = 1;
            break;
        }
    }

    if ((found == 0) || (jobMng == NULL)) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Job is not found by Invoke Server %s(%d) RequestID %d.\n",
            serverType, typeCount, requestID);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't set the JobID %s for Invoke Server %s(%d)"
            " RequestID %d.\n",
            ((invokeJobIDvalid != 0) ? invokeJobID : "(not valid)"),
            serverType, typeCount, requestID);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    assert(jobMng != NULL);

    /* Check the validity of jobMng */
    if ((jobMng->ngjm_useInvokeServer == 0) ||
        (jobMng->ngjm_invokeServerInfo.ngisj_invokeServer == NULL) ||
        (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID != NULL)) {

        if (jobMng->ngjm_useInvokeServer == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "This job is not the Invoke Server job. Do nothing.\n");
        }
        if (jobMng->ngjm_invokeServerInfo.ngisj_invokeServer == NULL) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Invoke Server registered to this job is NULL."
                " Do nothing.\n");
        }
        if (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID != NULL) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "JobID for RequestID %d is already available (%s)."
                " Do nothing.\n", requestID,
                jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID);
        }
        notifyResult = 0;
    }

    /* Register the JobID */
    if ((notifyResult != 0) && (invokeJobIDvalid != 0)) {
        tmp = strdup(invokeJobID);
        if (tmp == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the string for the Job ID.\n");
            goto error;
        }
        jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID = tmp;

        /* log */
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) Job RequestID is %d, JobID is \"%s\".\n",
            serverType, typeCount,
            jobMng->ngjm_invokeServerInfo.ngisj_requestID,
            jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID);
    }

    /* Notify JobID */
    result = ngcllInvokeServerJobIDset(context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't Notify Invoke Server JobID.\n");
        return 0;
    }

    /* Success */
    return 1;


    /* Error occurred */
error:
    /* Notify to main thread */
    if ((requestID > 0) && (jobMng != NULL)) {
        result = ngcllInvokeServerJobIDset(context, invokeMng, jobMng, NULL);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't Notify Invoke Server JobID.\n");
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify STATUS_NOTIFY.
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerNotifyProcessStatusNotify(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *notifyName,
    char *argument,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyProcessStatusNotify";
    char invokeJobID[NGCLI_INVOKE_SERVER_JOB_ID_STR_MAX];
    char *cur, *serverType, *statusName, *messageString;
    const ngcllInvokeServerStatusTable_t *statusTable;
    ngcllInvokeServerStatus_t status;
    int result, i, found, typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(argument != NULL);

    invokeJobID[0] = '\0';
    statusTable = NULL;
    statusName = NULL;
    messageString = NULL;

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    cur = argument;

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Read the Job ID */
    i = 0;
    while (!isspace((int)*cur) && (*cur != '\0')) {
        invokeJobID[i] = *cur;
        cur++;
        i++;
        if (i >= (sizeof(invokeJobID) - 1)) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Invoke Server %s(%d) Job ID too long.\n",
                serverType, typeCount);

            /* Do not finish the Reply Reader thread */
            return 1;
        }
    }
    invokeJobID[i] = '\0';

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Find the status */
    found = 0;
    status = NGCLI_INVOKE_SERVER_STATUS_UNDEFINED;
    statusTable = ngcllInvokeServerStatusTable;
    for (i = 0; statusTable[i].ngisst_valid != 0; i++) {
        if (strncmp(cur, statusTable[i].ngisst_name,
            strlen(statusTable[i].ngisst_name)) == 0) {

            status = statusTable[i].ngisst_status;
            statusName = statusTable[i].ngisst_name;
            cur += strlen(statusTable[i].ngisst_name);

            found = 1;
            break;
        }
    }

    if (found == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid state returned from Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, invokeJobID);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    /* Get the string */
    messageString = cur;

    result = ngcllInvokeServerNotifyJobStatusChange(
        context, invokeMng, invokeJobID, status, statusName,
        messageString, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't change the status for Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, invokeJobID);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Job Status Change
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerNotifyJobStatusChange(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *invokeJobID,
    ngcllInvokeServerStatus_t status,
    char *statusName,
    char *messageString,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyJobStatusChange";
    ngcliInvokeServerManager_t *curInvokeMng;
    int result, found, typeCount;
    ngcliJobStatus_t jobStatus;
    ngcliJobManager_t *jobMng;
    char *serverType, *msg;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(invokeJobID != NULL);
    assert(statusName != NULL);

    found = 0;
    curInvokeMng = NULL;
    jobMng = NULL;
    jobStatus = NGI_JOB_STATUS_UNKNOWN;
    msg = NULL;

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Find the job by Job ID */
    found = 0;

    for (jobMng = context->ngc_jobMng_head; jobMng != NULL;
        jobMng = jobMng->ngjm_next) {

        if (jobMng->ngjm_useInvokeServer == 0) {
            continue;
        }

        /* Compare Invoke Server Type */
        curInvokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
        if (curInvokeMng == NULL) {
            continue;
        }

        if (curInvokeMng->ngism_serverType == NULL) {
            continue;
        }

        assert(invokeMng->ngism_serverType != NULL);

        if (strcmp(curInvokeMng->ngism_serverType,
            invokeMng->ngism_serverType) != 0) {
            continue;
        }

        if (curInvokeMng->ngism_typeCount != invokeMng->ngism_typeCount) {
            continue;
        }

        /* Compare Invoke Server JobID */
        if (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID == NULL) {
            continue;
        }

        if (strcmp(jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID,
            invokeJobID) == 0) {
            found = 1;
            break;
        }
    }

    if ((found == 0) || (jobMng == NULL)) {
        /* Job may not available, if the timeout or nowait was occurred */
        ngclLogWarnContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Job is not found by Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, invokeJobID);
        ngclLogWarnContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't set the status %s"
            " for Invoke Server %s(%d) JobID %s job.\n",
            statusName, serverType, typeCount, invokeJobID);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    assert(jobMng != NULL);

    /* Print the debug message */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Job status %s.\n", statusName);

    /* log */
    if ((messageString != NULL) && (*messageString != '\0')) {
        ngcliLogInfoJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\".\n", messageString);
    }

    switch(status) {
    case NGCLI_INVOKE_SERVER_STATUS_PENDING:
        jobStatus = NGI_JOB_STATUS_PENDING;
        msg = "pending";
        break;

    case NGCLI_INVOKE_SERVER_STATUS_ACTIVE:
        jobStatus = NGI_JOB_STATUS_ACTIVE;
        msg = "active";

        /* Set the end time */
        result = ngcliJobSetEndTimeOfInvoke(jobMng, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't set the End time.\n");
        }
        break;

    case NGCLI_INVOKE_SERVER_STATUS_DONE:
        jobStatus = NGI_JOB_STATUS_DONE;
        msg = "done";
        break;

    case NGCLI_INVOKE_SERVER_STATUS_FAILED:
        jobStatus = NGI_JOB_STATUS_FAILED;
        msg = "failed";

        ngcliLogPrintfJob(jobMng,
            NG_LOGCAT_NINFG_PURE,
            (jobMng->ngjm_requestCancel == 0 ?
                NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_INFORMATION), fName,
            "Invoke Server %s(%d) Job failed.\n",
            serverType, typeCount);
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid status on Invoke Server %s(%d) JobID %s.\n",
            serverType, typeCount, invokeJobID);
        return 0;
    }

    /* Is job dead? */
    if ((jobStatus == NGI_JOB_STATUS_FAILED) ||
        (jobStatus == NGI_JOB_STATUS_DONE)) {

        result = ngcliExecutableJobDone(jobMng, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't noify the Job done to Executable.\n");
            /* Not return. */
        }
    }

    /* Print the information */
    ngcliLogInfoJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Job is %s.\n", msg);

    /* Notify the Job Status */
    result = ngcliJobNotifyStatus(jobMng, jobStatus, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't noify the Job Status.\n");

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    /* Count DONE jobs */
    if ((jobStatus == NGI_JOB_STATUS_FAILED) ||
        (jobStatus == NGI_JOB_STATUS_DONE)) {

        /* Tell the job done to the External Module. */
        result = ngiExternalModuleJobDone(
            invokeMng->ngism_externalModule, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Tell the job done to External Module failed.\n");
            return 0;
        }

        result = ngcllInvokeServerTreatRetired(context, NULL);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't treat the retired Invoke Servers.\n");
        }
    }

    /* Success */
    return 1;
}

/**
 * Invoke Server Unusable
 */
static int
ngcllInvokeServerUnusable(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerUnusable";
    int result, mutexLocked, jobMngLocked;
    ngcliJobManager_t *jobMng;
    char *serverType;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    mutexLocked = 0;
    jobMngLocked = 0;
    jobMng = NULL;
    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Mutex.\n");
        return 0;
    }
    mutexLocked = 1;

    /* Is already done? */
    if (invokeMng->ngism_valid == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) already unusable.\n",
            serverType, typeCount);

        /* Unlock */
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Mutex.\n");
            return 0;
        }

        /* Success */
        return 1;
    }

    /* log */
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Setting Invoke Server %s(%d) unusable.\n",
        serverType, typeCount);

    /* Set the error code */
    invokeMng->ngism_errorCode = NG_ERROR_JOB_DEAD;

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Job Manager.\n");
        goto error;
    }
    jobMngLocked = 1;

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "All jobs in the Invoke Server %s(%d) making unusable.\n",
        serverType, typeCount);

    /* Set all Jobs related to this Invoke Server unusable */
    for (jobMng = context->ngc_jobMng_head; jobMng != NULL;
        jobMng = jobMng->ngjm_next) {

        if (jobMng->ngjm_useInvokeServer == 0) {
            continue;
        }

        if (invokeMng == jobMng->ngjm_invokeServerInfo.ngisj_invokeServer) {
            /* found */
            ngcliLogDebugJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "This job is managed by Invoke Server %s(%d).\n",
                serverType, typeCount);

            result = ngcliExecutableJobDone(jobMng, log, error);
            if (result == 0) {
                ngcliLogErrorJob(jobMng,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't noify the Job done to Executable.\n");
                /* Not return */
            }

            /* Notify the Job Status */
            result = ngcliJobNotifyStatus(
                jobMng, NGI_JOB_STATUS_DONE, log, error);
            if (result == 0) {
                ngcliLogErrorJob(jobMng,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't noify the Job Status.\n");
                /* Not return */
            }
        }
    }

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "All jobs in the Invoke Server %s(%d) making unusable finished.\n",
        serverType, typeCount);

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Job Manager.\n");
        goto error;
    }

    /* No more Invoke Server valid. */
    invokeMng->ngism_valid = 0;

    /* Make the External Module unusable. */
    result = ngiExternalModuleUnusable(
        invokeMng->ngism_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Make the External Module unusable failed.\n");
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngclLogErrorContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Can't unusable the Invoke Server %s(%d).\n",
        serverType, typeCount);

    /* Unlock the list of Job Manager */
    if (jobMngLocked != 0) {
        jobMngLocked = 0;
        result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the list of Job Manager.\n");
        }
    }

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Mutex.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Invoke Server JobID Set.
 */
static int
ngcllInvokeServerJobIDset(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobIDset";
    int typeCount, result;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server JobID arrived.\n");

    /* Is JobID Notify already came? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset != 0) {
        ngcliLogWarnJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server JobID already exists.\n");
    }

    /* Set the Reply */
    jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset = 1;

    /* Notify signal */
    result = ngiCondBroadcast(&invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't signal the Cond for InvokeServer %s(%d).\n",
            serverType, typeCount);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Invoke Server JobID Wait.
 */
static int
ngcllInvokeServerJobIDwait(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobIDwait";
    char *invokeJobID, *serverType;
    int typeCount, result;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    invokeJobID = NULL;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    /* Wait JobID */
    while ((invokeMng->ngism_valid != 0) &&
        (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset == 0)) {

        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait the Cond for InvokeServer %s(%d).\n",
                serverType, typeCount);
            goto error;
        }
    }

    /* Is Invoke Server valid? */
    if (invokeMng->ngism_valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invoke Server %s(%d) was dead.\n",
            serverType, typeCount);
        goto error;
    }

    invokeJobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "Invoke Server %s(%d) JobID \"%s\" arrived.\n",
        serverType, typeCount,
        (invokeJobID != NULL ? invokeJobID : "not"));

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    if (invokeJobID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "JobID was not arrived for RequestID %d"
            " from Invoke Server %s(%d).\n",
            jobMng->ngjm_invokeServerInfo.ngisj_requestID,
            serverType, typeCount);

        /* Failed */
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Mutex for InvokeServer %s(%d).\n",
            serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Create the Job stdout/stderr output file.
 */
static int
ngcllInvokeServerJobStdoutStderrCreate(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobStdoutStderrCreate";
    char *tmpDir, *tmpFileStdout, *tmpFileStderr;
    ngclRemoteMachineInformation_t *rmInfo;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;
    rmInfo = &jobMng->ngjm_attr.ngja_rmInfo;
    tmpDir = NULL;
    tmpFileStdout = NULL;
    tmpFileStderr = NULL;

    if (rmInfo->ngrmi_redirectEnable == 0) {
        ngcliLogDebugJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "The Job do not require stdout/stderr file.\n");

        jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile = NULL;
        jobMng->ngjm_invokeServerInfo.ngisj_stderrFile = NULL;

        /* Success */
        return 1;
    }

    /* Get the tmpDir */
    tmpDir = NULL;
    assert(jobMng->ngjm_attr.ngja_lmInfoExist != 0);
    if (jobMng->ngjm_attr.ngja_lmInfo.nglmi_tmpDir != NULL) {
        tmpDir = jobMng->ngjm_attr.ngja_lmInfo.nglmi_tmpDir;
    }

    /* Create the stdout filename */
    tmpFileStdout = ngiTemporaryFileCreate(tmpDir, log, error);
    if (tmpFileStdout == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the Temporary File Name for stdout.\n");
        /* Not return */
    } else {
        /* Register the stdout filename */
        result = ngcliNinfgManagerTemporaryFileRegister(tmpFileStdout, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogWarnJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't register the Temporary File Name \"%s\" for stdout.\n",
                tmpFileStdout);
            /* Not return */
        }
    }

    /* Create the stderr filename */
    tmpFileStderr = ngiTemporaryFileCreate(tmpDir, log, error);
    if (tmpFileStderr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the Temporary File Name for stderr.\n");
        /* Not return */
    } else {
        /* Register the stderr filename */
        result = ngcliNinfgManagerTemporaryFileRegister(tmpFileStderr, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogWarnJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't register the Temporary File Name \"%s\" for stderr.\n",
                tmpFileStderr);
            /* Not return */
        }
    }

    /* log */
    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "The Temporary File Name for stdout is \"%s\".\n",
        ((tmpFileStdout != NULL) ? tmpFileStdout : "NULL"));

    ngcliLogDebugJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "The Temporary File Name for stderr is \"%s\".\n",
        ((tmpFileStderr != NULL) ? tmpFileStderr : "NULL"));

    jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile = tmpFileStdout;
    jobMng->ngjm_invokeServerInfo.ngisj_stderrFile = tmpFileStderr;

    /* Success */
    return 1;
}

/**
 * Destroy the Job stdout/stderr output file.
 */
static int
ngcllInvokeServerJobStdoutStderrDestroy(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobStdoutStderrDestroy";
    char *tmpFile;
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    log = context->ngc_log;

    /* Destroy the stdout file */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile;
    if (tmpFile != NULL) {
        result = ngcliNinfgManagerTemporaryFileUnregister(
            tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogWarnJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unregister the Temporary File \"%s\" for stdout.\n",
                tmpFile);
            /* Not return */
        }

        result = ngiTemporaryFileDestroy(tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy the Temporary File \"%s\" for stdout.\n",
                tmpFile);
            /* Not return */
        }
        jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile = NULL;
    }

    /* Destroy the stderr file */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stderrFile;
    if (tmpFile != NULL) {
        result = ngcliNinfgManagerTemporaryFileUnregister(
            tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogWarnJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't unregister the Temporary File \"%s\" for stderr.\n",
                tmpFile);
            /* Not return */
        }

        result = ngiTemporaryFileDestroy(tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy the Temporary File \"%s\" for stderr.\n",
                tmpFile);
            /* Not return */
        }
        jobMng->ngjm_invokeServerInfo.ngisj_stderrFile = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Output the Job stdout/stderr output file.
 * Note :
 *     The Job stdout/stderr output is not realtime output.
 *     Output is performed when the Job is destruct.
 *     (simple implementation)
 */
static int
ngcllInvokeServerJobStdoutStderrOutput(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobStdoutStderrOutput";
    char *tmpFile;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile;
    if (tmpFile != NULL) {
        result = ngcllInvokeServerJobStdoutStderrOutputSub(
            context, invokeMng, jobMng, tmpFile, stdout, "stdout", error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't output the job stdout.\n");
            /* Not return */
        }
    }

    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stderrFile;
    if (tmpFile != NULL) {
        result = ngcllInvokeServerJobStdoutStderrOutputSub(
            context, invokeMng, jobMng, tmpFile, stderr, "stderr", error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't output the job stdout.\n");
            /* Not return */
        }
    }

    /* Success */
    return 1;
}

/**
 * Output the Job stdout or stderr from file.
 */
static int
ngcllInvokeServerJobStdoutStderrOutputSub(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    char *tmpFile,
    FILE *destFp,
    char *destName,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerJobStdoutStderrOutputSub";
    int result, c;
    FILE *fp;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);
    assert(tmpFile != NULL);
    assert(destFp != NULL);
    assert(destName != NULL);

    /* Open the file */
    fp = fopen(tmpFile, "r");
    if (fp == NULL) {
        ngcliLogErrorJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Opening Job %s file \"%s\" failed: %s.\n",
            destName, tmpFile, strerror(errno));
        /* It's not the serious error. No problem. */
        return 1;
    }

    /* log */
    ngcliLogInfoJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "output the Job %s start.\n",
        destName);

    while ((c = fgetc(fp)) != EOF) {
        result = fputc(c, destFp);
        if (result == EOF) {
            ngcliLogErrorJob(jobMng,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't output the Job %s file.\n",
                destName);
            break;
        }
    }

    /* log */
    ngcliLogInfoJob(jobMng,
        NG_LOGCAT_NINFG_PURE, fName,
        "output the Job %s end.\n",
        destName);

    result = fclose(fp);
    if (result != 0) {
        ngcliLogWarnJob(jobMng,
            NG_LOGCAT_NINFG_PURE, fName,
            "Closing Job %s file \"%s\" failed: %s.\n",
            destName, tmpFile, strerror(errno));
        /* Not return */
    }

    /* Success */
    return 1;
}

