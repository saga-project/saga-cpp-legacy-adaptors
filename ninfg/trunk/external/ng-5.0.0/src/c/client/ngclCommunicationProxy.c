/*
 * $RCSfile: ngclCommunicationProxy.c,v $ $Revision: 1.22 $ $Date: 2008/03/06 11:39:18 $
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
 * Module of Communication Proxy for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclCommunicationProxy.c,v $ $Revision: 1.22 $ $Date: 2008/03/06 11:39:18 $")

/**
 * Data type
 */
typedef struct ngcllCommunicationProxyNotifyCallbackArgument_s {
    ngcliCommunicationProxyManager_t * ngca_manager;
    int                                ngca_id;
} ngcllCommunicationProxyNotifyCallbackArgument_t;

/**
 * Prototype declaration of static functions.
 */

/* Communication Proxy Information Manager */
static ngcliCommunicationProxyInformationManager_t *
    ngcllCommunicationProxyInformationConstruct(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);
static int ngcllCommunicationProxyInformationDestruct(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *, int *);
static int ngcllCommunicationProxyInformationManagerInitialize(
     ngclContext_t *, ngcliCommunicationProxyInformationManager_t *,
     ngclCommunicationProxyInformation_t *, int *);
static int ngcllCommunicationProxyInformationManagerFinalize(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *, int *);
static int ngcllCommunicationProxyInformationRelease(
    ngclContext_t *, ngclCommunicationProxyInformation_t *, int *);
static void ngcllCommunicationProxyInformationInitializeMember(
    ngclCommunicationProxyInformation_t *);
static void ngcllCommunicationProxyInformationInitializePointer(
    ngclCommunicationProxyInformation_t *);
static int ngcllCommunicationProxyInformationGetCopy(
    ngclContext_t *, char *, ngclCommunicationProxyInformation_t *, int *);
static int ngcllCommunicationProxyInformationReplace(
    ngclContext_t *, ngcliCommunicationProxyInformationManager_t *,
    ngclCommunicationProxyInformation_t *, int *);

/* Communication Proxy Manager */
static int ngcllCommunicationProxyManagerInitialize(
    ngcliCommunicationProxyManager_t *, ngclContext_t *, int, int *);
static int ngcllCommunicationProxyManagerFinalize(
    ngcliCommunicationProxyManager_t *, int *);
static void ngcllCommunicationProxyManagerInitializeMember(
    ngcliCommunicationProxyManager_t *);
static int ngcllCommunicationProxyManagerLock(
    ngcliCommunicationProxyManager_t *, int *);
static int ngcllCommunicationProxyManagerUnlock(
    ngcliCommunicationProxyManager_t *, int *);
static int ngcllCommunicationProxyManagerRegister(
    ngcliCommunicationProxyManager_t *, ngcliCommunicationProxy_t*, int *);
static int ngcllCommunicationProxyManagerUnregister(
    ngcliCommunicationProxyManager_t *, ngcliCommunicationProxy_t*, int *);
static ngcliCommunicationProxy_t *ngcllCommunicationProxyManagerGetAvailableItem(
    ngcliCommunicationProxyManager_t *, char *, int *);
static ngcliCommunicationProxy_t *ngcllCommunicationProxyManagerGetItemFromID(
    ngcliCommunicationProxyManager_t *, int, int *);

/* Communication Proxy */
static int ngcllCommunicationProxyRelease(
    ngcliCommunicationProxy_t *, int *error);
static ngcliCommunicationProxy_t *ngcllCommunicationProxyConstruct(
    ngcliCommunicationProxyManager_t *, char *, int *);
static int ngcllCommunicationProxyDestruct(
    ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyInitialize(ngcliCommunicationProxy_t *,
    ngcliCommunicationProxyManager_t *, char *, int *);
static int ngcllCommunicationProxyFinalize(ngcliCommunicationProxy_t *, int *);
static void ngcllCommunicationProxyInitializeMember(ngcliCommunicationProxy_t *);
static int ngcllCommunicationProxyDisable(ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyQueryFeatures(ngcliCommunicationProxy_t *, int *error);
static int ngcllCommunicationProxySendInitialize(
    ngcliCommunicationProxy_t *, ngclContext_t *, unsigned short, int *);
static int ngcllCommunicationProxySendExit(
    ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyPrepareCommunication(
    ngcliCommunicationProxy_t *, ngclRemoteMachineInformation_t *,
    ngiLineList_t **, int *);
static ngcliCommunicationReplyArgument_t *
    ngcllCommunicationProxySendPrepareCommunicationRequest(
    ngcliCommunicationProxy_t *, ngclRemoteMachineInformation_t *, int *);
static int ngcllCommunicationProxyWaitCommunicationReplyNotify(
    ngcliCommunicationProxy_t *, ngcliCommunicationReplyArgument_t *, ngiLineList_t **, int *);
static int ngcllCommunicationProxyGetNewRequestID(ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyNotifyCallback(
    void *, ngiExternalModuleNotifyState_t, char *, char *,
    ngiLineList_t *, ngLog_t *, int *);

static int ngcllCommunicationProxySetDestroyGuard(ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyUnsetDestroyGuard(ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyEnterDestroy(ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationProxyIsSetDestroyGuard(ngcliCommunicationProxy_t *, int *);

static int ngcllCommunicationProxyRegisterCommunicationReplyArgument(
    ngcliCommunicationProxy_t *, ngcliCommunicationReplyArgument_t *, int *);
static int ngcllCommunicationProxyUnregisterCommunicationReplyArgument(
    ngcliCommunicationProxy_t *, ngcliCommunicationReplyArgument_t *, int *);
static int ngcllCommunicationProxyGetRequestIDfromNotify(ngiLineList_t *, ngLog_t *, int *);

/* COMMUNICATION_REPLY argument */
static ngcliCommunicationReplyArgument_t *ngcllCommunicationReplyArgumentConstruct(
    ngcliCommunicationProxy_t *, int, int *);
static int ngcllCommunicationReplyArgumentDestruct(
    ngcliCommunicationReplyArgument_t *, ngcliCommunicationProxy_t *, int *);
static int ngcllCommunicationReplyArgumentInitialize(
    ngcliCommunicationReplyArgument_t *, int, int *);
static int ngcllCommunicationReplyArgumentFinalize(
    ngcliCommunicationReplyArgument_t *, ngcliCommunicationProxy_t *, int *);
static void ngcllCommunicationReplyArgumentInitializeMember(
    ngcliCommunicationReplyArgument_t *);
static int ngcllCommunicationReplyArgumentSetValue(
    ngcliCommunicationReplyArgument_t *, ngiLineList_t *, ngLog_t *, int *);

static int ngcllLineListAppendCommunicationProxyEncodedString(
    ngiLineList_t *, const char *, ngLog_t *, int *);

/**
 * Local Macros
 */

/* If the following macro uses do-while,
 * a compiler prints warning in few environments. */
#define NGCLL_GOTO_ERROR() \
    {                      \
        ret = 0;           \
        error = NULL;      \
        goto finalize;     \
    }

#define NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng)       \
    do {                                                            \
        assert((comProxyMng)                              != NULL); \
        assert((comProxyMng)->ngcpm_context               != NULL); \
        assert((comProxyMng)->ngcpm_context->ngc_log      != NULL); \
        assert((comProxyMng)->ngcpm_externalModuleManager != NULL); \
    } while (0)

#define NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy) \
    do {                                           \
        assert((comProxy) != NULL);                \
        NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(  \
            (comProxy)->ngcp_manager);             \
    } while (0)

#define NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy)                  \
    do {                                                                  \
        NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);                       \
        assert(ngcllCommunicationProxyIsSetDestroyGuard(comProxy, NULL)); \
    } while (0)

/**
 * Data
 */
#define NGCLL_COMMUNICATION_PROXY_INITIALIZE_REQUEST             "INITIALIZE"
#define NGCLL_COMMUNICATION_PROXY_EXIT_REQUEST                  NGI_EXTERNAL_MODULE_REQUEST_EXIT
#define NGCLL_COMMUNICATION_PROXY_PREPARE_COMMUNICATION_REQUEST "PREPARE_COMMUNICATION"
#define NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY    "COMMUNICATION_REPLY"
#define NGCLL_COMMUNICATION_PROXY_PROTOCOL_VERSION              "1.0"

static char *ngcllCommunicationProxyMultilineNotifies[] = {
    "RELAY_COMMUNICATION_PROXY_INFORMATION_NOTIFY",
    "COMMUNICATION_REPLY",
    NULL
};
static char *ngcllCommunicationProxyNecessaryRequests[] = {
    NGCLL_COMMUNICATION_PROXY_INITIALIZE_REQUEST,
    NGCLL_COMMUNICATION_PROXY_EXIT_REQUEST,
    NGCLL_COMMUNICATION_PROXY_PREPARE_COMMUNICATION_REQUEST,
    NULL
};

/**
 * Functions
 */

/**
 * Information append at last of the list.
 */
int
ngcliCommunicationProxyInformationCacheRegister(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliCommunicationProxyInformationManager_t *cpInfoMng;
    static const char fName[] = "ngcliCommunicationProxyInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    cpInfoMng = NULL;
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
    if ((cpInfo == NULL) ||
        (cpInfo->ngcpi_type == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    /* Lock the list */
    result = ngcliCommunicationProxyInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock CommunicationProxyInformation list.\n");
        goto error;
    }
    listLocked = 1;

    /* Is Communication Proxy Information available? */
    cpInfoMng = ngcliCommunicationProxyInformationCacheGet(
        context, cpInfo->ngcpi_type, &subError);
    if (cpInfoMng != NULL) {

        /* Replace the Communication Proxy Information */
        result = ngcllCommunicationProxyInformationReplace(
            context, cpInfoMng, cpInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't replace the Communication Proxy Information.\n");
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliCommunicationProxyInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Communication Proxy Information list.\n");
        goto error;
    }

    /* Construct */
    if (cpInfoMng == NULL) {
        cpInfoMng = ngcllCommunicationProxyInformationConstruct(
            context, cpInfo, error);
        if (cpInfoMng == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't construct Communication Proxy Information.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliCommunicationProxyInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Communication Proxy Information list.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Information delete from the list.
 */
int
ngcliCommunicationProxyInformationCacheUnregister(
    ngclContext_t *context,
    char *typeName,
    int *error)
{
    int result;
    ngcliCommunicationProxyInformationManager_t *curr;
    static const char fName[] = "ngcliCommunicationProxyInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context Invalid.\n");
        return 0;
    }

    /* Lock the list */
    result = ngcliCommunicationProxyInformationListWriteLock(context,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock CommunicationProxyInformation list.\n");
        return 0;
    }

    if (typeName == NULL) {
        /* Delete all information */

        /* Get the data from the head of a list */
        curr = ngcliCommunicationProxyInformationCacheGetNext(context, NULL, error);
        if (curr == NULL) {
             ngclLogDebugContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "No Communication Proxy Information was registered.\n");
        }

        while (curr != NULL) {
            /* Destruct the data */
            result = ngcllCommunicationProxyInformationDestruct(context, curr, error);
            if (result == 0) {
                ngclLogErrorContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct Communication Proxy Information.\n");
                goto error;
            }

            /* Get next data from the list */
            curr = ngcliCommunicationProxyInformationCacheGetNext(
                context, NULL, error);
        }
    } else {
        /* Delete specified information */

        /* Get the data from the list by Communication Proxy name */
        curr = ngcliCommunicationProxyInformationCacheGet(context, typeName, error);
        if (curr == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Communication Proxy Information \"%s\" is not found.\n",
                typeName);
            goto error;
        }

        /* Destruct the data */
        result = ngcllCommunicationProxyInformationDestruct(context, curr, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct Communication Proxy Information.\n");
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliCommunicationProxyInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Communication Proxy Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliCommunicationProxyInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Communication Proxy Information.\n");
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
ngcliCommunicationProxyInformationManager_t *
ngcliCommunicationProxyInformationCacheGet(
    ngclContext_t *context,
    char *typeName,
    int *error)
{
    int result;
    ngcliCommunicationProxyInformationManager_t *cpInfoMng;
    static const char fName[] = "ngcliCommunicationProxyInformationCacheGet";

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

    cpInfoMng = context->ngc_cpInfo_head;
    for (; cpInfoMng != NULL; cpInfoMng = cpInfoMng->ngcpim_next) {
        assert(cpInfoMng->ngcpim_info.ngcpi_type != NULL);
        if (strcmp(cpInfoMng->ngcpim_info.ngcpi_type, typeName) == 0) {
            /* Found */
            return cpInfoMng;
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Communication Proxy Information is not found by name \"%s\".\n",
        typeName);
    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliCommunicationProxyInformationManager_t *
ngcliCommunicationProxyInformationCacheGetNext(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliCommunicationProxyInformationCacheGetNext";

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
        if (context->ngc_cpInfo_head != NULL) {
            assert(context->ngc_cpInfo_tail != NULL);
            return context->ngc_cpInfo_head;
        }
    } else {
        /* Return the next information */
        if (current->ngcpim_next != NULL) {
            return current->ngcpim_next;
        }
    }

    /* Not found */
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "The last Communication Proxy Information was reached.\n");
    return NULL;
}

/**
 * Construct.
 */
static ngcliCommunicationProxyInformationManager_t *
ngcllCommunicationProxyInformationConstruct(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int result;
    ngcliCommunicationProxyInformationManager_t *cpInfoMng;
    static const char fName[] = "ngcllCommunicationProxyInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfo != NULL);

    /* Allocate */
    cpInfoMng = NGI_ALLOCATE(ngcliCommunicationProxyInformationManager_t,
        context->ngc_log, error);
    if (cpInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Communication Proxy Information.\n");
        return NULL;
    }

    /* Initialize */
    result = ngcllCommunicationProxyInformationManagerInitialize(
        context, cpInfoMng, cpInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Communication Proxy Information.\n");
        goto error;
    }

    /* Register */
    result = ngcliContextRegisterCommunicationProxyInformation(
        context, cpInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Communication Proxy Information"
            " for Ninf-G Context.\n");
        goto error;
    }

    /* Success */
    return cpInfoMng;

    /* Error occurred */
error:
    result = ngcllCommunicationProxyInformationDestruct(context, cpInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the storage for Communication Proxy Information Manager.\n");
        return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllCommunicationProxyInformationDestruct(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *cpInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllCommunicationProxyInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterCommunicationProxyInformation(context,
        cpInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister the Communication Proxy Information.\n");
        return 0;
    }

    /* Finalize */
    result = ngcllCommunicationProxyInformationManagerFinalize(
        context, cpInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Communication Proxy Information.\n");
        return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliCommunicationProxyInformationManager_t, cpInfoMng,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate the Communication Proxy Information.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngclCommunicationProxyInformation_t *
ngcliCommunicationProxyInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclCommunicationProxyInformation_t *cpInfo;
    static const char fName[] = "ngcliCommunicationProxyInformationAllocate";

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
    cpInfo = ngiCalloc(1, sizeof (ngclCommunicationProxyInformation_t),
	log, error);
    if (cpInfo == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Communication Proxy Information.\n");
        return NULL;
    }

    return cpInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliCommunicationProxyInformationFree(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliCommunicationProxyInformationFree";

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
    if (cpInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    ngiFree(cpInfo, log, error);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllCommunicationProxyInformationManagerInitialize(
     ngclContext_t *context,
     ngcliCommunicationProxyInformationManager_t *cpInfoMng,
     ngclCommunicationProxyInformation_t *cpInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllCommunicationProxyInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfoMng != NULL);
    assert(cpInfo != NULL);

    /* reset members */
    cpInfoMng->ngcpim_next = NULL;

    /* Copy to new information */
    result = ngcliCommunicationProxyInformationCopy(context, cpInfo,
        &cpInfoMng->ngcpim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Communication Proxy Information.\n");
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&cpInfoMng->ngcpim_rwlOwn,
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
ngcllCommunicationProxyInformationManagerFinalize(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *cpInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllCommunicationProxyInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&cpInfoMng->ngcpim_rwlOwn,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't destroy Read/Write Lock for own instance.\n");
        return 0;
    }

    /* Release the information */
    result = ngclCommunicationProxyInformationRelease(context,
        &cpInfoMng->ngcpim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Communication Proxy Information.\n");
        return 0;
    }

    /* reset members */
    cpInfoMng->ngcpim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliCommunicationProxyInformationCopy(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *src,
    ngclCommunicationProxyInformation_t *dest,
    int *error)
{
    ngLog_t *log;
    int i, result;
    static const char fName[] = "ngcliCommunicationProxyInformationCopy";

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

    ngcllCommunicationProxyInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllCommunicationProxyInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING_IF_VALID(src, dest, member) \
    do { \
        if ((src)->member != NULL) { \
            (dest)->member = ngiStrdup((src)->member, log ,error); \
            if ((dest)->member == NULL) { \
                ngclLogErrorContext(context, \
                    NG_LOGCAT_NINFG_PURE, fName, \
                    "Can't allocate the storage " \
                    "for Communication Proxy Information.\n"); \
                goto error; \
            } \
        } \
    } while(0)

#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        NGL_COPY_STRING_IF_VALID(src, dest, member); \
    } while (0)


    NGL_COPY_STRING(src, dest, ngcpi_type);
    NGL_COPY_STRING_IF_VALID(src, dest, ngcpi_path);
    NGL_COPY_STRING_IF_VALID(src, dest, ngcpi_logFilePath);

    /* Copy Options */
    if (src->ngcpi_nOptions > 0) {
        dest->ngcpi_options = ngiCalloc(
            src->ngcpi_nOptions, sizeof(char *), log, error);
        if (dest->ngcpi_options == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate the storage for string table.\n");
            return 0;
        }
        /* copy all of elements */
        for (i = 0; i < src->ngcpi_nOptions; i++) {
            NGL_COPY_STRING(src, dest, ngcpi_options[i]);
        }
    }

#undef NGL_COPY_STRING
#undef NGL_COPY_STRING_IF_VALID

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclCommunicationProxyInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Communication Proxy Information.\n");
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclCommunicationProxyInformationRelease(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclCommunicationProxyInformationRelease";

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

    result = ngcllCommunicationProxyInformationRelease(context, cpInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllCommunicationProxyInformationRelease(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int i;
    ngLog_t *log;
    static const char fName[] = "ngcllCommunicationProxyInformationRelease";

    /* Check the arguments */
    assert(context != NULL);

    if (cpInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    log = context->ngc_log;

    /* Deallocate the members */
    if (cpInfo->ngcpi_type != NULL)
        ngiFree(cpInfo->ngcpi_type, log, error);
    if (cpInfo->ngcpi_path != NULL)
        ngiFree(cpInfo->ngcpi_path, log, error);
    if (cpInfo->ngcpi_logFilePath != NULL)
        ngiFree(cpInfo->ngcpi_logFilePath, log, error);
    if (cpInfo->ngcpi_options != NULL) {
        for (i = 0; i < cpInfo->ngcpi_nOptions; i++) {
            if (cpInfo->ngcpi_options[i] != NULL) {
                ngiFree(cpInfo->ngcpi_options[i], log, error);
            }
        }
        ngiFree(cpInfo->ngcpi_options, log, error);
    }

    /* Initialize the members */
    ngcllCommunicationProxyInformationInitializeMember(cpInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliCommunicationProxyInformationInitialize(
    ngclContext_t *context,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliCommunicationProxyInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    /* Check the arguments */
    if (cpInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    ngcllCommunicationProxyInformationInitializeMember(cpInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllCommunicationProxyInformationInitializeMember(
    ngclCommunicationProxyInformation_t *cpInfo)
{
    /* Initialize the members */
    cpInfo->ngcpi_bufferSize = 0;
    cpInfo->ngcpi_maxJobs = 0;
    cpInfo->ngcpi_nOptions = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllCommunicationProxyInformationInitializePointer(
    ngclCommunicationProxyInformation_t *cpInfo)
{
    /* Initialize the members */
    cpInfo->ngcpi_type = NULL;
    cpInfo->ngcpi_path = NULL;
    cpInfo->ngcpi_logFilePath = NULL;
    cpInfo->ngcpi_options = NULL;
}

/**
 * GetCopy
 */
int
ngclCommunicationProxyInformationGetCopy(
    ngclContext_t *context,
    char *typeName,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclCommunicationProxyInformationGetCopy";

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

    result = ngcllCommunicationProxyInformationGetCopy(context,
        typeName, cpInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllCommunicationProxyInformationGetCopy(
    ngclContext_t *context,
    char *typeName,
    ngclCommunicationProxyInformation_t *cpInfo,
    int *error)
{
    int result;
    ngcliCommunicationProxyInformationManager_t *cpInfoMng;
    static const char fName[] = "ngcllCommunicationProxyInformationGetCopy";

    /* Check the arguments */
    if ((typeName == NULL) || (cpInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

   /* Lock the Communication Proxy Information */
    result = ngcliCommunicationProxyInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Communication Proxy Information.\n");
        return 0;
    }

    /* Get the Communication Proxy Information */
    cpInfoMng = ngcliCommunicationProxyInformationCacheGet(context, typeName, error);
    if (cpInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Communication Proxy Information.\n");
        goto error;
    }

    /* Copy the Communication Proxy Information */
    result = ngcliCommunicationProxyInformationCopy(context,
                    &cpInfoMng->ngcpim_info, cpInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Communication Proxy Information.\n");
        goto error;
    }

    /* Unlock the Communication Proxy Information */
    result = ngcliCommunicationProxyInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Communication Proxy Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliCommunicationProxyInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Communication Proxy Information.\n");
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Replace the Communication Proxy Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllCommunicationProxyInformationReplace(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *dstIsInfoMng,
    ngclCommunicationProxyInformation_t *srcIsInfo,
    int *error)
{
    ngLog_t *log;
    int result, cpLocked;
    static const char fName[] = "ngcllCommunicationProxyInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstIsInfoMng != NULL);
    assert(srcIsInfo != NULL);

    log = context->ngc_log;
    cpLocked = 0;

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Replace the Communication Proxy Information for \"%s\".\n",
        srcIsInfo->ngcpi_type);

    /* Lock */
    result = ngcliCommunicationProxyInformationWriteLock(
        dstIsInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write lock the Communication Proxy Information.\n");
        goto error;
    }
    cpLocked = 1;

    /* Release the Communication Proxy Information */
    result = ngcllCommunicationProxyInformationRelease(
        context, &dstIsInfoMng->ngcpim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Communication Proxy Information.\n");
        goto error;
    }

    /* Copy the Communication Proxy Information */
    result = ngcliCommunicationProxyInformationCopy(
        context, srcIsInfo, &dstIsInfoMng->ngcpim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Communication Proxy Information.\n");
        goto error;
    }

    /* Unlock */
    result = ngcliCommunicationProxyInformationWriteUnlock(
        dstIsInfoMng, log, error);
    cpLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Communication Proxy Information.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (cpLocked != 0) {
        result = ngcliCommunicationProxyInformationWriteUnlock(
            dstIsInfoMng, log, NULL);
        cpLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Communication Proxy Information.\n");
        }
    }

    /* Failed */
    return 0;
}

/* Communication Proxy */

/**
 * Communication Proxy Manager: Construct
 */
ngcliCommunicationProxyManager_t *
ngcliCommunicationProxyManagerConstruct(
    ngclContext_t *context,
    int port,
    int *error)
{
    ngcliCommunicationProxyManager_t *comProxyMng = NULL;
    ngLog_t *log = NULL;
    int result;
    static const char fName[] = "ngcliCommunicationProxyManagerConstruct";

    assert(context != NULL);
    assert(context->ngc_log != NULL);
    assert(port > 0);

    log = context->ngc_log;

    comProxyMng = NGI_ALLOCATE(ngcliCommunicationProxyManager_t, log, error);
    if (comProxyMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Communication Proxy manager.\n");
        goto error;
    }

    result = ngcllCommunicationProxyManagerInitialize(
        comProxyMng, context, port, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Communication Proxy manager.\n");
        goto error;
    }

    return comProxyMng;

    /* Error occurred */
error:
    result = NGI_DEALLOCATE(ngcliCommunicationProxyManager_t,
        comProxyMng, log, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the Communication Proxy manager.\n");
    }

    return NULL;
}

/**
 * Communication Proxy Manager: Destruct
 */
int
ngcliCommunicationProxyManagerDestruct(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int *error)
{
    ngLog_t *log = NULL;
    int ret = 1;/* Success */
    int result;
    ngclContext_t *context = NULL;
    static const char fName[] = "ngcliCommunicationProxyManagerDestruct";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    result = ngcllCommunicationProxyManagerFinalize(comProxyMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Communication Proxy manager.\n");
        ret = 0;/* Failed */
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngcliCommunicationProxyManager_t,
        comProxyMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the Communication Proxy manager.\n");
        ret = 0;/* Failed */
        error = NULL;
    }
    return ret;
}

/**
 * Communication Proxy Manager: Prepare communication
 */
int
ngcliCommunicationProxyManagerPrepareCommunication(
    ngcliCommunicationProxyManager_t *comProxyMng,
    ngclRemoteMachineInformation_t *rmInfo,
    ngiLineList_t **paramList,
    int *error)
{
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngiExternalModuleManager_t *emMng = NULL;
    ngclContext_t *context = NULL;
    ngLog_t *log;
    int result;
    int comProxyID = -1;
    static const char fName[] = "ngcliCommunicationProxyManagerPrepareCommunication";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(rmInfo != NULL);
    assert(paramList != NULL);

    context = comProxyMng->ngcpm_context;
    emMng   = comProxyMng->ngcpm_externalModuleManager;
    log     = context->ngc_log;

    comProxy = ngcllCommunicationProxyManagerGetAvailableItem(
        comProxyMng, rmInfo->ngrmi_commProxyType, error);
    if (comProxy == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get an available Communication Proxy.\n");
        error = NULL;
        goto finalize;
    }

    result = ngcllCommunicationProxyPrepareCommunication(
        comProxy, rmInfo, paramList, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't prepare communication.\n");
        error = NULL;
        goto finalize;
    }

    result = ngiExternalModuleIDget(comProxy->ngcp_externalModule,
        &comProxyID, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get ID from External Module.\n");
        error = NULL;
        goto finalize;
    }

finalize:

    if ((comProxyID < 0) && (comProxy != NULL)) {
        result = ngcllCommunicationProxyDisable(comProxy, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Communication Proxy.\n");
            error = NULL;
        }
    }

    if (comProxy != NULL) {
        result = ngcllCommunicationProxyRelease(comProxy, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the Communication Proxy.\n");
            comProxyID = -1;
            error = NULL;
        }
    }

    return comProxyID;
}

/**
 * Communication Proxy Manager: Unref
 */
int
ngcliCommunicationProxyManagerUnref(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int ID,
    int *error)
{
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
    int result;
    int ret = 1;/* true */
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngcliCommunicationProxy_t *destroyingComProxy= NULL;
    int retired = 0;
    int locked = 0;
    int valid;
    static const char fName[] = "ngcliCommunicationProxyManagerUnref";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(ID > 0);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    comProxy = ngcllCommunicationProxyManagerGetItemFromID(
        comProxyMng, ID, error);
    if (comProxy == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get Communication Proxy from ID \"%d\".\n", ID);
        NGCLL_GOTO_ERROR();
    }

    result = ngiExternalModuleJobStopped(comProxy->ngcp_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't notify job stopped to External Module.\n");
        error = NULL;
        goto finalize;
    }

    result = ngiExternalModuleJobDone(
        comProxy->ngcp_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't tell job done to External Module.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiExternalModuleIsRetired(
        comProxy->ngcp_externalModule, &retired, log, error);
    if (result == 0) {
         ngclLogErrorContext(context,
             NG_LOGCAT_NINFG_PURE, fName,
             "Can't check whether external module is retired.\n");
        NGCLL_GOTO_ERROR();
    }

    if (retired != 0) {
        valid = 0;
        result = ngiExternalModuleIsValid(comProxy->ngcp_externalModule, 
            &valid, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't check whether External Module is valid or not.\n");
            ret = 0;
            error = NULL;
            /* Through */
        }
        if (valid != 0) {
            /* Send EXIT request */
            result = ngcllCommunicationProxySendExit(comProxy, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't send %s request.\n",
                    NGCLL_COMMUNICATION_PROXY_EXIT_REQUEST);
                ret = 0;
                error = NULL;
                /* Through */
            }
        }

        /* If get Communication Proxy Manager lock and destroy Communication
         * Proxy with Communication Proxy has set destroy guard, 
         * dead lock occurred.
         * Thus, releases Communication Proxy.
         */
        if (comProxy != NULL) {
            result = ngcllCommunicationProxyRelease(comProxy, error);
            comProxy = NULL;
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't release the Communication Proxy.\n");
                ret = 0;
                error = NULL;
                /* Through */
            }
        }

        result = ngcllCommunicationProxyManagerLock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Communication Proxy Manager.\n");
            ret = 0;
            error = NULL;
            /* Through */
        }
        locked = 1;

        comProxy = ngcllCommunicationProxyManagerGetItemFromID(
            comProxyMng, ID, error);
        if (comProxy == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get Communication Proxy from ID \"%d\".\n", ID);
            NGCLL_GOTO_ERROR();
        }

        destroyingComProxy = comProxy;
        result = ngcllCommunicationProxyUnsetDestroyGuard(comProxy, error);
        comProxy = NULL;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't set guard of destroying Communication Proxy.\n");
            ret = 0;
            error = NULL;
            /* Through */
        }

        /* Destroy */
        result = ngcllCommunicationProxyDestruct(destroyingComProxy, error);
        if (result == 0) {
             ngclLogErrorContext(context,
                 NG_LOGCAT_NINFG_PURE, fName,
                 "Can't destruct the Communication Proxy.\n");
            NGCLL_GOTO_ERROR();
        }
    }

finalize:
    if (comProxy != NULL) {
        result = ngcllCommunicationProxyRelease(comProxy, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }

    if (locked != 0) {
        locked = 0;
        result = ngcllCommunicationProxyManagerUnlock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Communication Proxy Manager.\n");
            error = NULL;
            ret = 0;
        }
    }

    return ret;
}

/**
 * Communication Proxy Manager: Initialize
 */
static int
ngcllCommunicationProxyManagerInitialize(
    ngcliCommunicationProxyManager_t *comProxyMng,
    ngclContext_t *context,
    int port,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    static const char fName[] = "ngcllCommunicationProxyManagerInitialize";
    
    ngcllCommunicationProxyManagerInitializeMember(comProxyMng);

    result = ngiRlockInitialize(
        &comProxyMng->ngcpm_rlock, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize rlock.\n");
        return 0;
    }

    comProxyMng->ngcpm_context               = context;
    comProxyMng->ngcpm_port                  = port;
    comProxyMng->ngcpm_externalModuleManager = context->ngc_externalModuleManager;
    comProxyMng->ngcpm_nLock                 = 0;
    SLIST_INIT(&comProxyMng->ngcpm_list);

    /* Success */
    return 1;
}

/**
 * Communication Proxy Manager: Finalize
 */
static int
ngcllCommunicationProxyManagerFinalize(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int *error)
{
    ngcliCommunicationProxy_t *it;
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
    int result;
    int ret = 1;/* Success */
    int rLocked = 0;
    int comLocked = 0;
    int valid = 0;
    static const char fName[] = "ngcllCommunicationProxyManagerFinalize";

    assert(comProxyMng->ngcpm_context != NULL);
    assert(comProxyMng->ngcpm_context->ngc_log != NULL);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&comProxyMng->ngcpm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the lock.\n");
        ret = 0;
        error = NULL;
    } else {
        rLocked = 1;
    }

    /* Send EXIT request all Communication Proxy */
    SLIST_FOREACH(it, &comProxyMng->ngcpm_list, ngcp_entry) {
        result = ngcllCommunicationProxySetDestroyGuard(it, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't set guard of destroying Communication Proxy.\n");
            ret = 0;
            error = NULL;
            continue;
        }
        valid = 0;
        result = ngiExternalModuleIsValid(it->ngcp_externalModule, 
            &valid, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't check whether External Module is valid or not.\n");
            ret = 0;
            error = NULL;
            /* Through */
        }
        if (valid != 0) {
            /* Send EXIT request */
            result = ngcllCommunicationProxySendExit(it, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't send %s request.\n",
                    NGCLL_COMMUNICATION_PROXY_EXIT_REQUEST);
                ret = 0;
                error = NULL;
                /* Through */
            }
        }
        result = ngcllCommunicationProxyUnsetDestroyGuard(it, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't set guard of destroying Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }
    if (rLocked != 0) {
        rLocked = 0;
        result = ngiRlockUnlock(&comProxyMng->ngcpm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the lock.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* All Communication Proxy Destruct */
    result = ngcllCommunicationProxyManagerLock(comProxyMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy Manager.\n");
        ret = 0;
        error = NULL;
    } else {
        comLocked = 1;
    }
    while ((it = SLIST_FIRST(&comProxyMng->ngcpm_list)) != NULL) {
        result = ngcllCommunicationProxyDestruct(it, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't initialize rlock.\n");
            ret = 0;
            error = NULL;
        }
    }
    if (comLocked != 0) {
        comLocked = 0;
        result = ngcllCommunicationProxyManagerUnlock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the lock.\n");
            ret = 0;
            error = NULL;
        }
    }

    result = ngiRlockFinalize(&comProxyMng->ngcpm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize Rlock.\n");
        ret = 0;
        error = NULL;
    }

    ngcllCommunicationProxyManagerInitializeMember(comProxyMng);

    return ret;
}

/**
 * Communication Proxy Manager: Zero clear data structure
 */
static void
ngcllCommunicationProxyManagerInitializeMember(
    ngcliCommunicationProxyManager_t *comProxyMng)
{
#if 0
    static const char fName[] = "ngcllCommunicationProxyManagerInitializeMember";
#endif

    comProxyMng->ngcpm_context               = NULL;
    comProxyMng->ngcpm_externalModuleManager = NULL;
    comProxyMng->ngcpm_rlock                 = NGI_RLOCK_NULL;
    comProxyMng->ngcpm_nLock                 = 0;

    SLIST_INIT(&comProxyMng->ngcpm_list);

    return;
}

/**
 * Communication Proxy Manager: Retire all Communication Proxy.
 */
int
ngcliCommunicationProxyManagerRetire(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int *error)
{
    ngcliCommunicationProxy_t *it;
    ngLog_t *log = NULL;
    int ret = 1;/* Success */
    int comLocked = 0;
    int result;
    ngclContext_t *context = NULL;
    static const char fName[] = "ngcliCommunicationProxyManagerRetire";

    /* Is Communication Proxy Manager available? */
    if (comProxyMng == NULL) {
        goto finalize;
    }

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    result = ngcllCommunicationProxyManagerLock(comProxyMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy Manager.\n");
        ret = 0;
        error = NULL;
    } else {
        comLocked = 1;
    }

    SLIST_FOREACH(it, &comProxyMng->ngcpm_list, ngcp_entry) {
        /* Retire */
        result = ngiExternalModuleRetire(it->ngcp_externalModule, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't retire the External Module.\n");
            ret = 0;
            error = NULL;
            goto finalize;
        }
    }

finalize:
    if (comLocked != 0) {
        comLocked = 0;
        result = ngcllCommunicationProxyManagerUnlock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the lock.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Communication Proxy Manager: Lock(reentrant)
 */
static int
ngcllCommunicationProxyManagerLock(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int *error)
{
    int locked = 0;
    ngiExternalModuleManager_t *emMng = NULL;
    ngclContext_t *context = NULL;
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllCommunicationProxyManagerLock";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    emMng   = comProxyMng->ngcpm_externalModuleManager;
    log = context->ngc_log;

    result = ngiRlockLock(&comProxyMng->ngcpm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the External Module.\n");
        goto error;
    }
    locked = 1;

    if (comProxyMng->ngcpm_nLock == 0) {
        result = ngiExternalModuleManagerListWriteLock(emMng, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the External Module.\n");
            goto error;
        }
    }
    assert(comProxyMng->ngcpm_nLock >= 0);
    comProxyMng->ngcpm_nLock++;

    return 1;
error:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxyMng->ngcpm_rlock, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
        }
    }
    return 0;
}

/**
 * Communication Proxy Manager: Unlock(reentrant)
 */
static int
ngcllCommunicationProxyManagerUnlock(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int *error)
{
    ngiExternalModuleManager_t *emMng = NULL;
    ngclContext_t *context = NULL;
    ngLog_t *log;
    int result;
    int ret = 1;
    static const char fName[] = "ngcllCommunicationProxyManagerUnlock";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    emMng   = comProxyMng->ngcpm_externalModuleManager;
    log = context->ngc_log;

    comProxyMng->ngcpm_nLock--;
    assert(comProxyMng->ngcpm_nLock >= 0);
    if (comProxyMng->ngcpm_nLock == 0) {
        result = ngiExternalModuleManagerListWriteUnlock(emMng, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
            error = NULL;
            ret = 0;
        }
    }

    result = ngiRlockUnlock(&comProxyMng->ngcpm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the External Module.\n");
        error = NULL;
        ret = 0;
    }
    return ret;
}

/**
 * Communication Proxy Manager: Register Communication Proxy
 */
static int
ngcllCommunicationProxyManagerRegister(
    ngcliCommunicationProxyManager_t *comProxyMng,
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
#if 0
    static const char fName[] = "ngcllCommunicationProxyManagerRegister";
#endif

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(comProxy != NULL);
    assert(comProxy->ngcp_registered == 0);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    SLIST_INSERT_HEAD(&comProxyMng->ngcpm_list, comProxy, ngcp_entry);
    comProxy->ngcp_registered = 1;

    return 1;
}

/**
 * Communication Proxy Manager: Unregister Communication Proxy
 */
static int
ngcllCommunicationProxyManagerUnregister(
    ngcliCommunicationProxyManager_t *comProxyMng,
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
#if 0
    static const char fName[] = "ngcllCommunicationProxyManagerUnregister";
#endif

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(comProxy != NULL);
    assert(comProxy->ngcp_registered != 0);

    context = comProxyMng->ngcpm_context;
    log = context->ngc_log;

    SLIST_REMOVE(&comProxyMng->ngcpm_list, comProxy,
        ngcliCommunicationProxy_s, ngcp_entry);
    comProxy->ngcp_registered = 0;

    return 1;
}

/**
 * Communication Proxy Manager: Get an available Communication Proxy.
 * Note: If Communication Proxy object of this function's return value is
 * locked.
 * Thus you must unlock it by ngcllCommunicationProxyRelease() when
 * you finish to use it.
 */
static ngcliCommunicationProxy_t*
ngcllCommunicationProxyManagerGetAvailableItem(
    ngcliCommunicationProxyManager_t *comProxyMng,
    char *type,
    int *error)
{
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngiExternalModuleManager_t *emMng = NULL;
    ngiExternalModule_t *module = NULL;
    ngclContext_t *context = NULL;
    ngLog_t *log;
    int result;
    int locked = 0;
    static const char fName[] = "ngcllCommunicationProxyManagerGetAvailableItem";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(type != NULL);

    context = comProxyMng->ngcpm_context;
    emMng   = comProxyMng->ngcpm_externalModuleManager;
    log     = context->ngc_log;

    /* Lock  */
    result = ngcllCommunicationProxyManagerLock(comProxyMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Communication Proxy Manager.\n");
        error = NULL;
        goto finalize;
    }
    locked = 1;

    /* get the Available External Module. */
    result = ngiExternalModuleAvailableGet(emMng,
        NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
        type, &module, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the available External Module.\n");
        error = NULL;
        goto finalize;
    }

    if (module != NULL) { /* Found */
        comProxy = ngiExternalModuleOwnerGet(module, log, error);
        if (comProxy == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Communication Proxy was not registered.\n");
            error = NULL;
            goto finalize;
        }
    } else {
        /* Create new Communication Proxy */
        comProxy = ngcllCommunicationProxyConstruct(comProxyMng, type, error);
        if (comProxy == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't construct the Communication Proxy.\n");
            error = NULL;
            goto finalize;
        }
    }

    result = ngiExternalModuleJobStarted(comProxy->ngcp_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't notify job started to External Module.\n");
        error = NULL;
        goto finalize;
    }

    result = ngcllCommunicationProxySetDestroyGuard(comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't set guard of destroying Communication Proxy.\n");
        error = NULL;
        goto finalize;
    }

finalize:
    /* Unlock the Invoke Server list */
    if (locked != 0) {
        locked = 0;
        result = ngcllCommunicationProxyManagerUnlock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
            error = NULL;
            comProxy = NULL;
        }
    }

    return comProxy;
}

/**
 * Communication Proxy Manager: Get a Communication Proxy from ID.
 * Note: If Communication Proxy object of this function's return value is
 * locked.
 * Thus you must unlock it by ngcllCommunicationProxyRelease() when
 * you finish to use it.
 */
static ngcliCommunicationProxy_t *
ngcllCommunicationProxyManagerGetItemFromID(
    ngcliCommunicationProxyManager_t *comProxyMng,
    int ID,
    int *error)
{
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngcliCommunicationProxy_t *it = NULL;
    int IDtmp;
    ngclContext_t *context = NULL;
    ngLog_t *log;
    int locked = 0;
    int result;
    static const char fName[] = "ngcllCommunicationProxyManagerGetItemFromID";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);
    assert(ID > 0);

    context = comProxyMng->ngcpm_context;
    log = comProxyMng->ngcpm_context->ngc_log;

    result = ngcllCommunicationProxyManagerLock(comProxyMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Communication Proxy Manager.\n");
        error = NULL;
        goto finalize;
    }
    locked = 1;

    SLIST_FOREACH(it, &comProxyMng->ngcpm_list, ngcp_entry) {
        result = ngiExternalModuleIDget(it->ngcp_externalModule,
            &IDtmp, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get ID from External Module.\n");
            error = NULL;
            goto finalize;
        }
        if (IDtmp == ID) {
            /* Found */
            result = ngcllCommunicationProxySetDestroyGuard(it, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't set guard of destroying Communication Proxy.\n");
                error = NULL;
                goto finalize;
            }
            comProxy = it;
            goto finalize;
        }
    }
    
    /* Not found */
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
        "There is no Communication Proxy whose ID is \"%d\"..\n", ID);
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    error = NULL;

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngcllCommunicationProxyManagerUnlock(comProxyMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the External Module.\n");
            error = NULL;
            comProxy = NULL;
        }
    }
    return comProxy;
}

/**
 * Communication Proxy: Release
 */
static int
ngcllCommunicationProxyRelease(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
#if 0
    static const char fName[] = "ngcllCommunicationProxyRelease";
#endif

    NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);

    return ngcllCommunicationProxyUnsetDestroyGuard(comProxy, error);
}

/**
 * Communication Proxy: Construct
 * Needs to lock external module manager.
 * Invokes the Communication Proxy process.
 */
static ngcliCommunicationProxy_t *
ngcllCommunicationProxyConstruct(
    ngcliCommunicationProxyManager_t *comProxyMng,
    char *type,
    int *error)
{
    ngLog_t *log;
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngclContext_t *context = NULL;
    int result;
    int initialized = 0;
    static const char fName[] = "ngcllCommunicationProxyConstruct";

    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    log     = context->ngc_log;

    comProxy = NGI_ALLOCATE(ngcliCommunicationProxy_t, log, error);
    if (comProxy == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for Communication Proxy.\n");
        goto error;
    }

    result = ngcllCommunicationProxyInitialize(
        comProxy, comProxyMng, type, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize Communication Proxy.\n");
        goto error;
    }
    initialized = 1;

    result = ngcllCommunicationProxyManagerRegister(
        comProxyMng, comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register Communication Proxy.\n");
        goto error;
    }

    return comProxy;
error:
    if (initialized != 0) {
        result = ngcllCommunicationProxyFinalize(comProxy, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct Communication Proxy.\n");
        }
        initialized = 0;
    }
    result = NGI_DEALLOCATE(
        ngcliCommunicationProxy_t, comProxy, log, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for Communication Proxy.\n");
    }
    comProxy = NULL;

    return NULL;
}

/**
 * Communication Proxy: Destruct
 */
static int
ngcllCommunicationProxyDestruct(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    ngLog_t *log;
    ngcliCommunicationProxyManager_t *comProxyMng = NULL;
    ngclContext_t *context = NULL;
    int result;
    int ret = 1;
    static const char fName[] = "ngcllCommunicationProxyDestruct";

    if (comProxy == NULL) {
        /* Success */
        return 1;
    }

    comProxyMng = comProxy->ngcp_manager;
    context     = comProxyMng->ngcpm_context;
    log         = context->ngc_log;

    result = ngcllCommunicationProxyEnterDestroy(
        comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't enter destruct Communication Proxy.\n");
        ret = 0;
        error = NULL;
    }

    result = ngcllCommunicationProxyManagerUnregister(
        comProxyMng, comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister Communication Proxy.\n");
        ret = 0;
        error = NULL;
    }

    result = ngcllCommunicationProxyFinalize(comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize Communication Proxy.\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(
        ngcliCommunicationProxy_t, comProxy, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for Communication Proxy.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Communication Proxy: Initialize
 */
static int
ngcllCommunicationProxyInitialize(
    ngcliCommunicationProxy_t *comProxy,
    ngcliCommunicationProxyManager_t *comProxyMng,
    char *type,
    int *error)
{
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngclLocalMachineInformation_t lmInfo;
    int gottenlmInfo = 0;
    ngclCommunicationProxyInformation_t *cpInfo = NULL;
    ngiExternalModuleManager_t *extMng = NULL;
    ngiExternalModule_t *module = NULL;
    ngcllCommunicationProxyNotifyCallbackArgument_t *cArg = NULL;
    int result;
    int ret = 1;
    int id;
    static const char fName[] = "ngcllCommunicationProxyInitialize";

    assert(comProxy != NULL);
    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxyMng);

    context = comProxyMng->ngcpm_context;
    extMng  = context->ngc_externalModuleManager;
    log     = context->ngc_log;

    ngcllCommunicationProxyInitializeMember(comProxy);

    comProxy->ngcp_sentInitialize    = 0;/* False */
    comProxy->ngcp_destroying        = 0;/* False */
    comProxy->ngcp_destroyGuardCount = 0;
    comProxy->ngcp_manager           = comProxyMng;
    comProxy->ngcp_requestIdSeed     = 1;

    result = ngiRlockInitialize(
        &comProxy->ngcp_rlock, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize rlock.\n");
        NGCLL_GOTO_ERROR();
    }

    {/* Get Information */
        result = ngclCommunicationProxyInformationGetCopy(
            context, type, &comProxy->ngcp_information, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get copy of Communication Proxy Information.\n");
            NGCLL_GOTO_ERROR();
        }
        comProxy->ngcp_informationGotten = 1;
        cpInfo = &comProxy->ngcp_information;

        result = ngclLocalMachineInformationGetCopy(
            context, &lmInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get copy of Local Machine Information.\n");
            NGCLL_GOTO_ERROR();
        }
        gottenlmInfo = 1;
    }

    module = ngiExternalModuleConstruct(
        extMng, NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
        NGI_EXTERNAL_MODULE_SUB_TYPE_CLIENT_COMMUNICATION_PROXY,
        type, cpInfo->ngcpi_path, lmInfo.nglmi_commProxyLog,
        cpInfo->ngcpi_logFilePath,
        cpInfo->ngcpi_maxJobs,
        ngcllCommunicationProxyMultilineNotifies,
        log, error);
    if (module == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the External Module.\n");
        NGCLL_GOTO_ERROR();
    }
    comProxy->ngcp_externalModule = module;

    result = ngiExternalModuleIDget(module, &id, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get ID from External Module.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiExternalModuleOwnerSet(module, comProxy, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register Communication Proxy .\n");
        NGCLL_GOTO_ERROR();
    }

    /* Register the Notify callback function. */
    cArg = NGI_ALLOCATE(
        ngcllCommunicationProxyNotifyCallbackArgument_t, log, error);
    if (cArg == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for argument of notify callback.\n");
        NGCLL_GOTO_ERROR();
    }
    cArg->ngca_manager = comProxyMng;
    cArg->ngca_id      = id;

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,
        "the notify callback argument address is %p.\n", cArg);

    result = ngiExternalModuleNotifyCallbackRegister(
        module, ngcllCommunicationProxyNotifyCallback,
        cArg, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Register The Notify callback failed.\n");
        NGCLL_GOTO_ERROR();
    }

    /* If ngiExternalModuleNotifyCallbackRegister() success,
     * does not free cArg */
    cArg = NULL; 

finalize:
    result = ngiFree(cArg, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free argument for notify callback.\n");
        error = 0;
        ret = 0;
    }
    cArg = NULL;

    if (gottenlmInfo != 0) {
        gottenlmInfo = 0;
        result = ngclLocalMachineInformationRelease(
            context, &lmInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't register Communication Proxy .\n");
            error = 0;
            ret = 0;
        }
    }

    if (ret == 0) {
        result = ngcllCommunicationProxyFinalize(comProxy, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't finalize Communication Proxy.\n");
        }
    }

    return ret;
}

/**
 * Communication Proxy: Finalize
 */
static int
ngcllCommunicationProxyFinalize(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngiExternalModuleManager_t *extMng = NULL;
    int result;
    int ret = 1;
    ngcliCommunicationProxyManager_t *comProxyMng = NULL;
    static const char fName[] = "ngcllCommunicationProxyFinalize";

    assert(comProxy != NULL);
    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(comProxy->ngcp_manager);

    comProxyMng = comProxy->ngcp_manager;
    context     = comProxyMng->ngcpm_context;
    extMng      = context->ngc_externalModuleManager;
    log         = context->ngc_log;

    if (comProxy->ngcp_externalModule != NULL) {
        result = ngiExternalModuleDestruct(
            comProxy->ngcp_externalModule, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct External Module.\n");
            error = NULL;
            ret = 0;
        }
        comProxy->ngcp_externalModule = NULL;
    }

    if (comProxy->ngcp_informationGotten != 0) {
        result = ngclCommunicationProxyInformationRelease(
            context, &comProxy->ngcp_information, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release Communication Proxy Information.\n");
            error = NULL;
            ret = 0;
        }
    }

    result = ngiRlockFinalize(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize rlock.\n");
        error = NULL;
        ret = 0;
    }

    ngcllCommunicationProxyInitializeMember(comProxy);

    return ret;
}

/**
 * Communication Proxy: Zero clear
 */
static void
ngcllCommunicationProxyInitializeMember(
    ngcliCommunicationProxy_t *comProxy)
{
#if 0
    static const char fName[] = "ngcllCommunicationProxyInitializeMember";
#endif
    comProxy->ngcp_registered        = 0;
    comProxy->ngcp_manager           = NULL;
    comProxy->ngcp_destroyGuardCount = 0;
    comProxy->ngcp_destroying        = 0;
    comProxy->ngcp_sentInitialize    = 0;
    comProxy->ngcp_requestIdSeed     = 0;
    comProxy->ngcp_rlock             = NGI_RLOCK_NULL;
    comProxy->ngcp_externalModule    = NULL;
    SLIST_INIT(&comProxy->ngcp_communicationReplyArgs);
    ngcllCommunicationProxyInformationInitializeMember(&comProxy->ngcp_information);
    ngcllCommunicationProxyInformationInitializePointer(&comProxy->ngcp_information);
    comProxy->ngcp_informationGotten = 0;

    return;
}

/**
 * Communication Proxy: Disable
 */
static int
ngcllCommunicationProxyDisable(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    ngLog_t *log = NULL;
    ngcliCommunicationProxyManager_t *mng= NULL;
    ngclContext_t *context = NULL;
    int result;
    static const char fName[] = "ngcllCommunicationProxyDisable";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    /* Make the External Module unusable. */
    result = ngiExternalModuleUnusable(
        comProxy->ngcp_externalModule, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't make the External Module unusable.\n");
        return 0;
    }

    result = ngiRlockBroadcast(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't broadcast signal.\n");
        return 0;
    }

    return 1;
}

/**
 * Communication Proxy: Send QUERY_FEATURES
 */
static int
ngcllCommunicationProxyQueryFeatures(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int requestSuccess = 0;
    ngiLineList_t *features = NULL;
    ngiLineList_t *requests = NULL;
    char *version = NULL;
    char *errorMessage = NULL;
    int result;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    char **p;
    char *it;
    int found;
    int ret = 1;
    static const char fName[] = "ngcllCommunicationProxyQueryFeatures";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    context = comProxy->ngcp_manager->ngcpm_context;
    log = context->ngc_log;

    /* Query Features. */
    result = ngiExternalModuleQueryFeatures(
        comProxy->ngcp_externalModule,
        &requestSuccess, &version, &features, &requests,
        &errorMessage, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed.\n");
        NGCLL_GOTO_ERROR();
    }
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed. : \"%s\"\n",
            ((errorMessage != NULL) ? errorMessage : ""));

        result = ngiFree(errorMessage, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the error message string.\n");
            ret = 0;
            error = NULL;
        }
        errorMessage = NULL;

        NGCLL_GOTO_ERROR();
    }

    assert(version  != NULL);
    assert(features != NULL);
    assert(requests != NULL);

    /* Check Version */
    if (strcmp(version, NGCLL_COMMUNICATION_PROXY_PROTOCOL_VERSION) != 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Version \"%s\" is unknown.\n", version);
        NGCLL_GOTO_ERROR();
    }

    /* Check Feature */
    /* Now, Ninf-G Client cannot use features. */

    /* Check REQUESTS */
    for (p = ngcllCommunicationProxyNecessaryRequests; *p != NULL;++p) {
        found = 0;
        it = NULL;
        while ((it = ngiLineListLineGetNext(requests, it, log, error)) != NULL) {
            if (strcmp(*p, it) == 0) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Communication Proxy does not \"%s\" request.\n", *p);
            NGCLL_GOTO_ERROR();
        }
    }

finalize:
    if (features != NULL) {
        result = ngiLineListDestruct(features, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy a line list.\n");
            ret = 0;
            error = NULL;
        }
        result = ngiLineListDestruct(requests, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy a line list.\n");
            ret = 0;
            error = NULL;
        }
        result = ngiFree(version, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free version string.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* If error, disable */

    return ret;
}

/**
 * Communication Proxy: Sends INITIALIZE request.
 * Need lock. 
 */
static int
ngcllCommunicationProxySendInitialize(
    ngcliCommunicationProxy_t *comProxy,
    ngclContext_t *context,
    unsigned short port,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    int replySuccess = 0;
    char *replyMessage = NULL;
    ngiLineList_t *arguments = NULL;
    ngclCommunicationProxyInformation_t *cpInfo;
    int ret = 1;
    int i;
    static const char fName[] = "ngcllCommunicationProxySendInitialize";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    log    = context->ngc_log;
    cpInfo = &comProxy->ngcp_information;

    arguments = ngiLineListConstruct(log, error);
    if (arguments == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize line list.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "listen_port %d", comProxy->ngcp_manager->ngcpm_port);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "buffer_size %d", cpInfo->ngcpi_bufferSize);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    /* option attributes of <CLIENT_COMMUNICATION> section */
    for (i = 0;i < cpInfo->ngcpi_nOptions;++i) {
        result = ngiLineListAppend(arguments,
            cpInfo->ngcpi_options[i], log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't append string to line list.\n");
            NGCLL_GOTO_ERROR();
        }
    }

    /* Send Request */
    result = ngiExternalModuleRequest(
        comProxy->ngcp_externalModule,
        NGCLL_COMMUNICATION_PROXY_INITIALIZE_REQUEST, NULL,
        arguments, 0, 
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        NGCLL_GOTO_ERROR();
    }

    if (replySuccess == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
        NGCLL_GOTO_ERROR();
    }

finalize:
    if (arguments != NULL) {
        result = ngiLineListDestruct(arguments, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy a line list.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* If error, disable */

    return ret;
}

/**
 * Communication Proxy: Send Exit
 */
static int
ngcllCommunicationProxySendExit(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    int replySuccess = 0;
    char *replyMessage = NULL;
    ngclContext_t *context = NULL;
    static const char fName[] = "ngcllCommunicationProxySendExit";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    context = comProxy->ngcp_manager->ngcpm_context;
    log     = context->ngc_log;

    /* Send Request */
    result = ngiExternalModuleRequest(
        comProxy->ngcp_externalModule,
        NGCLL_COMMUNICATION_PROXY_EXIT_REQUEST, NULL,
        NULL, 0, 
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        return 0;
    }

    if (replySuccess == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
        return 0;
    }

    /* Disable */

    return 1;
}

/**
 * Communication Proxy: Prepare communication
 * Sends PREPARE_COMMUNICATION requests, and waits COMMUNICATION_REPLY notify.
 */
static int
ngcllCommunicationProxyPrepareCommunication(
    ngcliCommunicationProxy_t *comProxy,
    ngclRemoteMachineInformation_t *rmInfo,
    ngiLineList_t **paramList,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngcliCommunicationProxyManager_t *mng = NULL;
    ngcliCommunicationReplyArgument_t *crArg = NULL;
    ngiLineList_t *comReplyArgs = NULL;
    ngiLineList_t *pList = NULL;
    int locked = 0;
    char *it;
    int ret = 1;
    int i;
    char *bufferSize = NULL;
    static const char fName[] = "ngcllCommunicationProxyPrepareCommunication";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;

    if (comProxy->ngcp_sentInitialize == 0) {
        result = ngcllCommunicationProxyQueryFeatures(comProxy, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't query the features.\n");
            NGCLL_GOTO_ERROR();
        }

        result = ngcllCommunicationProxySendInitialize(
            comProxy, context, mng->ngcpm_port, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Fails INITIALIZE request.\n");
            NGCLL_GOTO_ERROR();
        }
        comProxy->ngcp_sentInitialize = 1;
    }
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            NGCLL_GOTO_ERROR();
        }
    }

    crArg = ngcllCommunicationProxySendPrepareCommunicationRequest(
        comProxy, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't send \"%s\" request.\n",
            NGCLL_COMMUNICATION_PROXY_PREPARE_COMMUNICATION_REQUEST);
        NGCLL_GOTO_ERROR();
    }

    result = ngcllCommunicationProxyWaitCommunicationReplyNotify(
        comProxy, crArg, &comReplyArgs, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't wait \"%s\" notify.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        NGCLL_GOTO_ERROR();
    }

    /* Encode Options */
    pList = ngiLineListConstruct(log, error);
    if (pList == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize line list.\n");
        NGCLL_GOTO_ERROR();
    }

    bufferSize = ngiStrdupPrintf(log, error, "buffer_size %d",
        rmInfo->ngrmi_commProxyBufferSize);
    if (bufferSize == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't duplicate the string.\n");
        NGCLL_GOTO_ERROR();
    }

    /* Buffer Size */
    result = ngcllLineListAppendCommunicationProxyEncodedString(
        pList, bufferSize, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't add an encoded string to list.\n");
        NGCLL_GOTO_ERROR();
    }

    for (i = 0;i < rmInfo->ngrmi_commProxyNoptions;++i) {
        result = ngcllLineListAppendCommunicationProxyEncodedString(
            pList, rmInfo->ngrmi_commProxyOptions[i], log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't add an encoded string to list.\n");
            NGCLL_GOTO_ERROR();
        }
    }

    it = NULL;
    while ((it = ngiLineListLineGetNext(comReplyArgs, it, log, error)) != NULL) {
        result = ngcllLineListAppendCommunicationProxyEncodedString(
            pList, it, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't add an encoded string to list.\n");
            NGCLL_GOTO_ERROR();
        }
    }

finalize:
    result = ngiFree(bufferSize, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the string.\n");
        error = NULL;
        ret = 0;
    }

    if (comReplyArgs != NULL) {
        result = ngiLineListDestruct(comReplyArgs, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy line list.\n");
            error = NULL;
            ret = 0;
        }
        comReplyArgs = NULL;
    }

    if (ret == 0) {
        if (pList != NULL) {
            result = ngiLineListDestruct(pList, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destroy line list.\n");
                error = NULL;
                ret = 0;
            }
            pList = NULL;
        }
        result = ngcllCommunicationProxyDisable(comProxy, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Communication Proxy.\n");
        }

        if (locked != 0) {
            locked = 0;
            result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, NULL);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't lock Communication Proxy.\n");
            }
        }
    }
    *paramList = pList;

    return ret;
}

/**
 * Communication Proxy: Sends PREPARE_COMMUNICATION request and waits reply.
 */
static ngcliCommunicationReplyArgument_t *
ngcllCommunicationProxySendPrepareCommunicationRequest(
    ngcliCommunicationProxy_t *comProxy,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngcliCommunicationProxyManager_t *mng = NULL;
    ngiLineList_t *arguments;
    int requestID = -1;
    int i = 0;
    int replySuccess = 0;
    int ret = 1;
    char *replyMessage = NULL;
    ngcliCommunicationReplyArgument_t *crArg = NULL;
    static const char fName[] = "ngcllCommunicationProxySendPrepareCommunicationRequest";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    arguments = ngiLineListConstruct(log, error);
    if (arguments == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize line list.\n");
        NGCLL_GOTO_ERROR();
    }

    requestID = ngcllCommunicationProxyGetNewRequestID(
        comProxy, error);
    if (requestID < 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get new request ID.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "request_id %d", requestID);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    /* communication_proxy_option attributes of <SERVER> section */
    for (i = 0;i < rmInfo->ngrmi_commProxyNoptions;++i) {
        result = ngiLineListAppend(arguments, rmInfo->ngrmi_commProxyOptions[i], log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't append string to line list.\n");
            NGCLL_GOTO_ERROR();
        }
    }

    /* TCP_NODELAY */
    result = ngiLineListPrintf(arguments, log, error,
        "tcp_nodelay %s", (rmInfo->ngrmi_tcpNodelay != 0)?("true"):("false"));
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    /* TCP connect retry */
    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryCount %d", rmInfo->ngrmi_retryInfo.ngcri_count);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryBaseInterval %d", rmInfo->ngrmi_retryInfo.ngcri_interval);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryIncreaseRatio %lf", rmInfo->ngrmi_retryInfo.ngcri_increase);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryRandom %s", 
        (rmInfo->ngrmi_retryInfo.ngcri_useRandom!= 0)?("true"):("false"));
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        NGCLL_GOTO_ERROR();
    }

    crArg = ngcllCommunicationReplyArgumentConstruct(
        comProxy, requestID, error);
    if (crArg == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't create \"%s\" argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        NGCLL_GOTO_ERROR();
    }

    /* Send Request */
    result = ngiExternalModuleRequest(
        comProxy->ngcp_externalModule,
        NGCLL_COMMUNICATION_PROXY_PREPARE_COMMUNICATION_REQUEST,
        NULL, arguments, 0, 
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        NGCLL_GOTO_ERROR();
    }

    if (replySuccess == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
        NGCLL_GOTO_ERROR();
    }

finalize:
    if (arguments != NULL) {
        result = ngiLineListDestruct(arguments, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy line list.\n");
            error = NULL;
            ret = 0;
        }
        arguments = NULL;
    }
    if (ret == 0) {
        result = ngcllCommunicationReplyArgumentDestruct(
            crArg, comProxy, NULL);
        if (result == 0) {
            ret = 0;
            error = NULL;
        }
        crArg = NULL;

        result = ngcllCommunicationProxyDisable(comProxy, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Communication Proxy.\n");
        }
    }
    return crArg;
}

/**
 * Communication Proxy: Wait COMMUNICATION_REPLY notify
 */
static int
ngcllCommunicationProxyWaitCommunicationReplyNotify(
    ngcliCommunicationProxy_t *comProxy,
    ngcliCommunicationReplyArgument_t *crArg,
    ngiLineList_t **paramList, 
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngcliCommunicationProxyManager_t *mng = NULL;
    ngcliCommunicationReplyArgument_t *it = NULL;
    int locked = 0;
    int valid = 0;
    int ret = 1;
    int disable = 0;
    ngiLineList_t *pList = NULL;
    static const char fName[] = "ngcllCommunicationProxyWaitCommunicationReplyNotify";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;

    for (;;) {
        if (comProxy->ngcp_destroying != 0) {
            /* Has Entered "Destruct()" */
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Communication Proxy will be destroyed on another thread.\n");
            disable = 1;
            NGCLL_GOTO_ERROR();
        }
        result = ngiExternalModuleIsValid(comProxy->ngcp_externalModule, 
            &valid, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't check whether External Module is valid or not.\n");
            disable = 1;
            NGCLL_GOTO_ERROR();
        }
        if (valid == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "External module is not valid.\n");
            disable = 1;
            NGCLL_GOTO_ERROR();
        }

        /* Check whether COMMUNICATION_REPLY notify argument is valid or not. */
        ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Check whether %s argument is valid.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);

        SLIST_FOREACH(it, &comProxy->ngcp_communicationReplyArgs, ngcra_entry) {
            if (it == crArg) {
                break;
            }
        }
        if (it == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Target is waited by another thread.\n");
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            NGCLL_GOTO_ERROR();
        }

        if (crArg->ngcra_set != 0) {
            /* Receives COMMUNICATION_REPLY notify */
            break;
        }
        /* Check is valid? */

        result = ngiRlockWait(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait Rlock.\n");
            disable = 1;
            NGCLL_GOTO_ERROR();
        }
    }

    if (crArg->ngcra_result == 0) {
        /* Error message */
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Notify' result is \"failed\": %s.\n",
            crArg->ngcra_message);
        NGCLL_GOTO_ERROR();
    }

    pList = crArg->ngcra_arguments;
    crArg->ngcra_arguments = NULL;

finalize:
    result = ngcllCommunicationReplyArgumentDestruct(
        crArg, comProxy, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destroy %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        ret = 0;
        disable = 1;
        error = NULL;
    }

    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            disable = 1;
            error = NULL;
        }
    }

    if (disable != 0) {
        assert(ret == 0);
        /* Should disable Communication Proxy? */
        result = ngcllCommunicationProxyDisable(comProxy, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Communication Proxy.\n");
        }
    }

    if (ret == 0) {
        if (pList != NULL) {
            result = ngiLineListDestruct(pList, log, NULL);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the line list.\n");
            }
        }
    } else {
        *paramList = pList;
    }

    return ret;
}

/**
 * Communication Proxy; Get new request ID
 * This function returns new request ID.
 * If failed, it returns -1.
 */
static int
ngcllCommunicationProxyGetNewRequestID(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int id = -1;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    int locked = 0;
    int result;
    static const char fName[] = "ngcllCommunicationProxyGetNewRequestID";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    context = comProxy->ngcp_manager->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        error = NULL;
        goto finalize;
    }
    locked = 1;

    id = comProxy->ngcp_requestIdSeed++;

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock Communication Proxy.\n");
            error = NULL;
            id = -1;
        }
    }

    return id;
}

/**
 * Communication Proxy: Callback on receiving the notifies.
 */
static int
ngcllCommunicationProxyNotifyCallback(
    void *argument,
    ngiExternalModuleNotifyState_t state,
    char *notifyName,
    char *params,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error)
{
    ngcllCommunicationProxyNotifyCallbackArgument_t *cArg = argument;
    ngcliCommunicationProxy_t *comProxy = NULL;
    ngcliCommunicationReplyArgument_t *it = NULL;
    ngclContext_t *context = NULL;
    int id = -1;
    int found = 0;
    int result;
    int locked = 0;
    int ret = 1;
    int fatal = 0;
    static const char fName[] = "ngcllCommunicationProxyNotifyCallback";

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "Called with argument %p.\n", cArg);

    switch (state) {
    case NGI_EXTERNAL_MODULE_NOTIFY_NORMAL:
        break;
    case NGI_EXTERNAL_MODULE_NOTIFY_ERROR:
    case NGI_EXTERNAL_MODULE_NOTIFY_CLOSED:
        goto callback_end;
    default:
        assert("NOTREACHED" == NULL);
    }

    assert(notifyName != NULL);
    NGCLL_COMMUNICATION_PROXY_MANAGER_ASSERT(cArg->ngca_manager);

    context = cArg->ngca_manager->ngcpm_context;

    comProxy = ngcllCommunicationProxyManagerGetItemFromID(
        cArg->ngca_manager, cArg->ngca_id, error);
    if (comProxy == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get Communication Proxy whoes ID is \"%d\".\n",
            cArg->ngca_id);
        goto callback_end;
    }

    if (strcmp(notifyName, NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY) == 0) {
        NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

        id = ngcllCommunicationProxyGetRequestIDfromNotify(lines, log, error);
        if (id < 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't get request id from %s notify.\n",
                NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
            NGCLL_GOTO_ERROR();
        }

        result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            fatal = 1;
            NGCLL_GOTO_ERROR();
        }
        locked = 1;

        /* Look for ID in Communication Proxy */
        SLIST_FOREACH(it, &comProxy->ngcp_communicationReplyArgs, ngcra_entry) {
            if (it->ngcra_id == id) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Does not wait %s which has request_id %d.\n",
                NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY, id);
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            NGCLL_GOTO_ERROR();
        }

        result = ngcllCommunicationReplyArgumentSetValue(it, lines, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't set value to reply argument.\n");
            ret = 0;
            error = NULL;
            /* Through */
            /* Calls ngiRlockBroadcast() even if this function failed */
        }

        result = ngiRlockBroadcast(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast signal.\n");
            fatal = 1;
            NGCLL_GOTO_ERROR();
        }

        /* Unlock */
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            fatal = 1;
            NGCLL_GOTO_ERROR();
        }
    } else {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" is unknown notify.\n", notifyName);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        NGCLL_GOTO_ERROR();
    }

    result = ngcllCommunicationProxyRelease(comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            error = NULL;
            fatal = 1;
        }
    }

    /* Register the Notify callback function. */
    result = ngiExternalModuleNotifyCallbackRegister(
        comProxy->ngcp_externalModule, ngcllCommunicationProxyNotifyCallback,
        cArg, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Register the Notify callback failed.\n");
        ret = 0;
        error = NULL;
        fatal = 1;
    }

    if (lines != NULL) {
        result = ngiLineListDestruct(lines, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct line list.\n");
            ret = 0;
            error = NULL;
        }
        lines = NULL;
    }

    if (fatal != 0) {
        assert(ret == 0);
        assert(error == NULL);
        result = ngcllCommunicationProxyDisable(comProxy, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Communication Proxy.\n");
        }
        goto callback_end;
    }

    return ret;

callback_end:
    result = ngiFree(cArg, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free argument for notify callback.\n");
    }
    cArg = NULL;

    return ret;
}

/**
 * Communication Proxy: Set guard of destroying. 
 * When this function is called, Communication Proxy must be locked.
 */
static int
ngcllCommunicationProxySetDestroyGuard(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    int ret = 1;
    ngcliCommunicationProxyManager_t *mng = NULL;
    ngclContext_t *context = NULL;
    int changed = 0;
    int locked = 0;
    static const char fName[] = "ngcllCommunicationProxySetDestroyGuard";

    NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;
    assert(comProxy->ngcp_destroying == 0);
    comProxy->ngcp_destroyGuardCount++;
    changed = 1;

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            if (changed != 0) {
                comProxy->ngcp_destroyGuardCount--;
            }
        }
    }

    return ret;
}

/**
 * Communication Proxy: Unset guard of destroying. 
 */
static int
ngcllCommunicationProxyUnsetDestroyGuard(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngcliCommunicationProxyManager_t *mng = NULL;
    int ret = 1;
    int locked = 0;
    ngclContext_t *context = NULL;
    static const char fName[] = "ngcllCommunicationProxyUnsetDestroyGuard";

    NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;

    comProxy->ngcp_destroyGuardCount--;
    assert(comProxy->ngcp_destroyGuardCount >= 0);

    if (comProxy->ngcp_destroyGuardCount == 0) {
        result = ngiRlockBroadcast(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast.\n");
            NGCLL_GOTO_ERROR();
        }
    }

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Communication Proxy: Start destroying
 */
static int
ngcllCommunicationProxyEnterDestroy(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngcliCommunicationProxyManager_t *mng = NULL;
    int ret = 1;
    int locked = 0;
    ngclContext_t *context = NULL;
    static const char fName[] = "ngcllCommunicationProxyEnterDestroy";

    NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;

    comProxy->ngcp_destroying = 1;

    assert(comProxy->ngcp_destroyGuardCount >= 0);
    while (comProxy->ngcp_destroyGuardCount > 0) {
        result = ngiRlockWait(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait rlock.\n");
            NGCLL_GOTO_ERROR();
        }
    }
    assert(comProxy->ngcp_destroyGuardCount >= 0);

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Communication Proxy: Is set destroying guard?.
 */
static int
ngcllCommunicationProxyIsSetDestroyGuard(
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log;
    ngcliCommunicationProxyManager_t *mng;
    int locked = 0;
    ngclContext_t *context;
    int isSet = 0;
    static const char fName[] = "ngcllCommunicationProxyIsSetDestroyGuard";

    NGCLL_COMMUNICATION_PROXY_ASSERT(comProxy);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        error = NULL;
        goto finalize;
    }
    locked = 1;

    if (comProxy->ngcp_destroyGuardCount > 0) {
        isSet = 1;
    }

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            error = NULL;
        }
    }

    return isSet;
}

/**
 * Communication Proxy: Register COMMUNICATION_REPLY argument
 */
static int
ngcllCommunicationProxyRegisterCommunicationReplyArgument(
    ngcliCommunicationProxy_t *comProxy,
    ngcliCommunicationReplyArgument_t *crArg,
    int *error)
{
    int result;
    ngLog_t *log;
    ngcliCommunicationProxyManager_t *mng;
    int ret = 1;
    ngclContext_t *context;
    int locked = 0;
    static const char fName[] = "ngcllCommunicationProxyRegisterCommunicationReplyArgument";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);
    assert(crArg != NULL);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    assert(comProxy != NULL);
    assert(crArg    != NULL);
    assert(crArg->ngcra_registered == 0);

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;
    SLIST_INSERT_HEAD(&comProxy->ngcp_communicationReplyArgs, crArg, ngcra_entry);
    crArg->ngcra_registered = 1;/* True */

finalize:
    if (locked != 0) {
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Communication Proxy: Unregister COMMUNICATION_REPLY argument
 */
static int
ngcllCommunicationProxyUnregisterCommunicationReplyArgument(
    ngcliCommunicationProxy_t *comProxy,
    ngcliCommunicationReplyArgument_t *crArg,
    int *error)
{
    int result;
    ngLog_t *log;
    ngcliCommunicationProxyManager_t *mng;
    ngcliCommunicationReplyArgument_t *it;
    int ret = 1;
    int found = 0;
    ngclContext_t *context;
    int locked = 0;
    static const char fName[] = "ngcllCommunicationProxyUnregisterCommunicationReplyArgument";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);
    assert(crArg != NULL);

    mng     = comProxy->ngcp_manager;
    context = mng->ngcpm_context;
    log     = context->ngc_log;

    assert(comProxy != NULL);
    assert(crArg    != NULL);

    /* Always unregistered, Success. */
    if (crArg->ngcra_registered == 0) {
        return ret;
    }

    result = ngiRlockLock(&comProxy->ngcp_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock Communication Proxy.\n");
        NGCLL_GOTO_ERROR();
    }
    locked = 1;

    found = 0;
    SLIST_FOREACH(it, &comProxy->ngcp_communicationReplyArgs, ngcra_entry) {
        if (it == crArg) {
            found = 1;
            break;
        }
    }
    if (found == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "This Communication Proxy does not specified %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        NGCLL_GOTO_ERROR();
    }
    SLIST_REMOVE(&comProxy->ngcp_communicationReplyArgs,
        crArg, ngcliCommunicationReplyArgument_s, ngcra_entry);
    crArg->ngcra_registered = 0;/* False */

finalize:
    if (locked != 0) {
        locked = 0;
        result = ngiRlockUnlock(&comProxy->ngcp_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock Communication Proxy.\n");
            ret = 0;
            error = NULL;
        }
    }

    return ret;
}

/**
 * Get "request_id"'s value from notify's argument.
 */
static int
ngcllCommunicationProxyGetRequestIDfromNotify(
    ngiLineList_t *argument,
    ngLog_t *log,
    int *error)
{
    char *it = NULL;
    int id = -1;
    int id_tmp;
    int result;
#if 0
    static const char fName[] = "ngcllCommunicationProxyGetRequestIDfromNotify";
#endif

    assert(argument != NULL);

    while ((it = ngiLineListLineGetNext(argument, it, log, error)) != NULL) {
        result = sscanf(it, "request_id %d", &id_tmp);
        if (result == 1) {
            id = id_tmp;
            break;
        }
    }
    return id;
}

/**
 * COMMUNICATION_REPLY argument: Construct
 */
static ngcliCommunicationReplyArgument_t *
ngcllCommunicationReplyArgumentConstruct(
    ngcliCommunicationProxy_t *comProxy,
    int requestID,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngclContext_t *context = NULL;
    ngcliCommunicationReplyArgument_t *crArg = NULL;
    static const char fName[] = "ngcllCommunicationReplyArgumentConstruct";

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    context = comProxy->ngcp_manager->ngcpm_context;
    log     = context->ngc_log;

    crArg = NGI_ALLOCATE(ngcliCommunicationReplyArgument_t, log, error);
    if (crArg == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        goto error;
    }
    result = ngcllCommunicationReplyArgumentInitialize(
        crArg, requestID, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        goto error;
    }
    result = ngcllCommunicationProxyRegisterCommunicationReplyArgument(
        comProxy, crArg, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        goto error;
    }

    return crArg;
error:
    result = ngcllCommunicationReplyArgumentDestruct(
        crArg, comProxy, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destroy %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
    }

    return NULL;
}

/**
 * COMMUNICATION_REPLY argument: Destruct
 */
static int
ngcllCommunicationReplyArgumentDestruct(
    ngcliCommunicationReplyArgument_t *crArg,
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclContext_t *context;
    int ret = 1;
    static const char fName[] = "ngcllCommunicationReplyArgumentDestruct";

    if (crArg == NULL) {
        return 1;/* successful */
    }

    NGCLL_COMMUNICATION_PROXY_ASSERT_GUARD(comProxy);

    context = comProxy->ngcp_manager->ngcpm_context;
    log     = context->ngc_log;

    result = ngcllCommunicationProxyUnregisterCommunicationReplyArgument(
        comProxy, crArg, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        error = NULL;
        ret = 0;
    }

    result = ngcllCommunicationReplyArgumentFinalize(crArg, comProxy, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        error = NULL;
        ret = 0;
    }

    result = NGI_DEALLOCATE(ngcliCommunicationReplyArgument_t, crArg, log, error);
    if (crArg == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for %s argument.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * COMMUNICATION_REPLY argument: Initialize
 */
static int
ngcllCommunicationReplyArgumentInitialize(
    ngcliCommunicationReplyArgument_t *crArg,
    int requestID,
    int *error)
{
#if 0
    static const char fName[] = "ngcllCommunicationReplyArgumentInitialize";
#endif

    ngcllCommunicationReplyArgumentInitializeMember(crArg);

    crArg->ngcra_id = requestID;

    return 1;
}

/**
 * COMMUNICATION_REPLY argument: Finalize
 */
static int
ngcllCommunicationReplyArgumentFinalize(
    ngcliCommunicationReplyArgument_t *crArg,
    ngcliCommunicationProxy_t *comProxy,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclContext_t *context;
    int ret = 1;
    static const char fName[] = "ngcllCommunicationReplyArgumentFinalize";

    context = comProxy->ngcp_manager->ngcpm_context;
    log     = context->ngc_log;

    if (crArg->ngcra_arguments != NULL) {
        result = ngiLineListDestruct(crArg->ngcra_arguments, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the line list.\n");
            ret = 0;
            error = NULL;
        }
    }

    result = ngiFree(crArg->ngcra_message, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for error message of %s.\n",
            NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
        ret = 0;
        error = NULL;
    }

    ngcllCommunicationReplyArgumentInitializeMember(crArg);

    return ret;
}

/**
 * COMMUNICATION_REPLY argument: Zero clear
 */
static void
ngcllCommunicationReplyArgumentInitializeMember(
    ngcliCommunicationReplyArgument_t *crArg)
{
#if 0
    static const char fName[] = "ngcllCommunicationReplyArgumentInitializeMember";
#endif

    crArg->ngcra_registered = 0;
    crArg->ngcra_set        = 0;
    crArg->ngcra_id         = 0;
    crArg->ngcra_result     = 0;
    crArg->ngcra_arguments  = NULL;
    crArg->ngcra_message    = NULL;
    crArg->ngcra_errorCode  = 0;

    return;
}

/**
 * COMMUNICATION_REPLY argument: set value
 * NOTES: this function always set 'value set flag' of crArg.
 */
static int
ngcllCommunicationReplyArgumentSetValue(
    ngcliCommunicationReplyArgument_t *crArg,
    ngiLineList_t *argument,
    ngLog_t *log,
    int *error)
{
    char *it = NULL;
    ngiExternalModuleArgument_t *arg = NULL;
    int result;
    int appearID = 0;
    int appearResult = 0;
    ngiLineList_t *other = NULL;
    int nOther = 0;
    int local_error;
    static const char fName[] = "ngcllCommunicationReplyArgumentSetValue";

    assert(crArg != NULL);
    assert(argument != NULL);

    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* If failed this function, leave waiting loop. */
    crArg->ngcra_set = 1;

    other = ngiLineListConstruct(log, &local_error);
    if (other == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't create line list.\n");
        goto error;
    }

    while ((it = ngiLineListLineGetNext(argument, it, log, &local_error)) != NULL) {
        arg = NULL;
        arg = ngiExternalModuleArgumentConstruct(it, log, &local_error);
        if (arg == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't create notify argument.\n");
            goto error;
        }
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "line  = %s\n", it);
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "name  = %s\n", arg->ngea_name);
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "value = %s\n", arg->ngea_value);

        if (strcmp(arg->ngea_name, "request_id") == 0) {
            if (appearID != 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "\"request_id\" appears multi times.\n");
                NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
                goto error;
            }
            appearID = 1;/* True */
        } else if (strcmp(arg->ngea_name, "result") == 0) {
            if (appearResult != 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "\"result\" appears multi times.\n");
                NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
                goto error;
            }
            appearResult = 1;/* True */
            if (strcmp(arg->ngea_value, "S") == 0) {
                crArg->ngcra_result = 1;
            } else if (strcmp(arg->ngea_value, "F") == 0) {
                crArg->ngcra_result = 0;
            } else {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "\"result\" is not \"S\" nor \"F\".\n");
                NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);

                goto error;
            }
        } else if (strcmp(arg->ngea_name, "message") == 0) {
            if (crArg->ngcra_message == NULL) {
                crArg->ngcra_message = ngiStrdup(arg->ngea_value, log, &local_error);
                if (crArg->ngcra_message == NULL) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't copy error message of %s notify.\n",
                        NGCLL_COMMUNICATION_PROXY_COMMUNICATION_REPLY_NOTIFY);
                    goto error;
                }
            } else {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "\"message\" appears multi times.\n");
                NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
                goto error;
            }
        } else {
            nOther++;
            result = ngiLineListAppend(other, it, log, &local_error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't append to list.\n");
                goto error;
            }
        }
        result = ngiExternalModuleArgumentDestruct(arg, log, error);
        arg = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the external module argument.\n");
            goto error;
        }
    }

    /* Check arguments */
    if (appearID == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "COMMUNICATION_REPLY notify does not have \"request_id\".\n");
        NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
        goto error;
    }

    if (appearResult == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "COMMUNICATION_REPLY notify does not have \"result\".\n");
        NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
        goto error;
    }

    if (crArg->ngcra_result != 0) {
        /* Success */
        if (crArg->ngcra_message != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "COMMUNICATION_REPLY notify has \"message\","
                " though result is \"S\".\n");
            NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
            goto error;
        }
    } else {
        if (crArg->ngcra_message == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "COMMUNICATION_REPLY notify does not have \"message\","
                " though result is \"F\".\n");
            NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
            goto error;
        }
        if (nOther > 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "COMMUNICATION_REPLY notify has information"
                " for connecting to Client Communication Proxy,"
                " though result is \"F\".\n");
            NGI_SET_ERROR(&local_error, NG_ERROR_SYNTAX);
            goto error;
        }
    }

    crArg->ngcra_arguments = other;

    return 1;
error:
    result = ngiExternalModuleArgumentDestruct(arg, log, error);
    arg = NULL;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the external module argument.\n");
    }

    NGI_SET_ERROR(error, local_error);
    NGI_SET_ERROR(&crArg->ngcra_errorCode, local_error);

    return 0;
}

/**
 * Append string encoded by Communication Proxy Encode to LineList. 
 */
static int
ngcllLineListAppendCommunicationProxyEncodedString(
    ngiLineList_t *llist,
    const char *string,
    ngLog_t *log,
    int *error)
{
    char *tmpString = NULL;
    int ret = 1;
    int result;
    static const char fName[] =
        "ngcllLineListAppendCommunicationProxyEncodedString";

    tmpString = ngiCommunicationProxyEncode(string, log, error);
    if (tmpString == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't encode string by Communication Proxy Encode.\n");
        NGCLL_GOTO_ERROR();
    }

    result = ngiLineListAppend(llist, tmpString, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to list.\n");
        NGCLL_GOTO_ERROR();
    }
finalize:

    result = ngiFree(tmpString, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free string.\n");
        ret = 0;
        error = NULL;
    }
    tmpString = NULL;

    return ret;
}
