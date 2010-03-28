/*
 * $RCSfile: ngclInformationService.c,v $ $Revision: 1.30 $ $Date: 2008/03/18 03:54:29 $
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
 * Module of Information Service for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclInformationService.c,v $ $Revision: 1.30 $ $Date: 2008/03/18 03:54:29 $")

/**
 * Data type
 */
typedef struct ngcllQueryResultWaiter_s {
    STAILQ_ENTRY(ngcllQueryResultWaiter_s) ngqrw_entry;
    ngcliInformationService_t             *ngqrw_infoServ;
    ngcliInformationServiceQuerySession_t *ngqrw_session;
} ngcllQueryResultWaiter_t;

STAILQ_HEAD(ngcllQueryResultWaiterList_s, ngcllQueryResultWaiter_s);
typedef struct ngcllQueryResultWaiterList_s ngcllQueryResultWaiterList_t;

/**
 * Prototype declaration of static functions.
 */
/* Information Service Information */
static ngcliInformationServiceInformationManager_t *
ngcllInformationServiceInformationConstruct(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error);
static int
ngcllInformationServiceInformationDestruct(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error);
static int
ngcllInformationServiceInformationManagerInitialize(
     ngclContext_t *context,
     ngcliInformationServiceInformationManager_t *isInfoMng,
     ngclInformationServiceInformation_t *isInfo,
     int *error);
static int
ngcllInformationServiceInformationManagerFinalize(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error);
static int
ngcllInformationServiceInformationRelease(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error);
static void
ngcllInformationServiceInformationInitializeMember(
    ngclInformationServiceInformation_t *isInfo);
static void
ngcllInformationServiceInformationInitializePointer(
    ngclInformationServiceInformation_t *isInfo);
static int
ngcllInformationServiceInformationGetCopy(
    ngclContext_t *context,
    char *tagName,
    ngclInformationServiceInformation_t *isInfo,
    int *error);
static int
ngcllInformationServiceInformationReplace(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *dstIsInfoMng,
    ngclInformationServiceInformation_t *srcIsInfo,
    int *error);

/* Information Service Manager */
static int ngcllInformationServiceManagerInitialize(
    ngcliInformationServiceManager_t *, ngclContext_t *, int *);
static int ngcllInformationServiceManagerFinalize(
    ngcliInformationServiceManager_t *, int *);
static void ngcllInformationServiceManagerInitializeMember(
    ngcliInformationServiceManager_t *);
static int ngcllInformationServiceContextGetLogFilePath(
    ngclContext_t *, char **, int *);
static ngcliInformationService_t *
    ngcllInformationServiceManagerFindByTag(
    ngcliInformationServiceManager_t *,
    char *, int *);

/* Information Service */
static ngcliInformationService_t *ngcllInformationServiceConstruct(
    ngcliInformationServiceManager_t *,
    ngclInformationServiceInformation_t *, int *);
static int ngcllInformationServiceDestruct(
    ngcliInformationService_t *, int *);
static int ngcllInformationServiceInitialize(
    ngcliInformationService_t *, ngcliInformationServiceManager_t *, 
    ngclInformationServiceInformation_t *, int *);
static int ngcllInformationServiceFinalize(
    ngcliInformationService_t *, int *);
static void ngcllInformationServiceInitializeMember(
    ngcliInformationService_t *);

static ngcliInformationServiceQuerySession_t *
    ngcllInformationServiceQuery(
    ngcliInformationService_t *, char *, char *, int *);
static int ngcllInformationServiceInvokeProcess(
    ngcliInformationService_t *, int *);
static int ngcllInformationServiceCheckQueryResult(
    ngcliInformationService_t *, ngcliInformationServiceQuerySession_t *, int *, int *);
static int ngcllInformationServiceWaitQueryResult(
    ngcliInformationService_t *, ngcliInformationServiceQuerySession_t *, int *, time_t,
    ngclExecutablePathInformation_t *, ngRemoteClassInformation_t *, int *);
static int ngcllInformationServiceCancelQuery(
    ngcliInformationService_t *, ngcliInformationServiceQuerySession_t *, int *);
static int ngcllInformationServiceQueryFeatures(
    ngcliInformationService_t *, int *);
static char *ngcllInformationServiceSendQueryREI(
    ngcliInformationService_t *, char *, char *, int *);
static int ngcllInformationServiceSendExit(
    ngcliInformationService_t *, int *);
static int ngcllInformationServiceNotifyCallback(
    void *, ngiExternalModuleNotifyState_t, char *, char *,
    ngiLineList_t *, ngLog_t *, int *);
static int ngcllInformationServiceDisable(
    ngcliInformationService_t *, int *);

/* Information Service Query Session */
static ngcliInformationServiceQuerySession_t *
    ngcllInformationServiceQuerySessionConstruct(
        ngcliInformationService_t *, char *, ngLog_t *, int *);
static int ngcllInformationServiceQuerySessionDestruct(
    ngcliInformationServiceQuerySession_t *,ngLog_t *,  int *);

static int ngcllInformationServiceQuerySessionInitialize(
    ngcliInformationServiceQuerySession_t *,
    ngcliInformationService_t *, char *,ngLog_t *,  int *);
static int ngcllInformationServiceQuerySessionFinalize(
    ngcliInformationServiceQuerySession_t *, ngLog_t *, int *);

static void ngcllInformationServiceQuerySessionInitializeMember(
    ngcliInformationServiceQuerySession_t *);

/* Information Service Query Result */
static ngcliInformationServiceQueryResult_t *
    ngcllParseREInotifyArguments(ngiLineList_t *, ngLog_t *, int *);
static int ngcllParseREInotifyArgumentsOneLine(
    ngcliInformationServiceQueryResult_t *, char *, ngLog_t *, int *);
static ngcliInformationServiceQueryResult_t *
    ngcllInformationServiceQueryResultConstruct(ngLog_t *, int *);
static int ngcllInformationServiceQueryResultDestruct(
    ngcliInformationServiceQueryResult_t *, ngLog_t *, int *);
static int ngcllInformationServiceQueryResultInitialize(
    ngcliInformationServiceQueryResult_t *, ngLog_t *, int *);
static int ngcllInformationServiceQueryResultFinalize(
    ngcliInformationServiceQueryResult_t *, ngLog_t *, int *);
static void ngcllInformationServiceQueryResultInitializeMember(
    ngcliInformationServiceQueryResult_t *);

/* Other */
static int ngcllQueryResultWaiterListPush(ngcllQueryResultWaiterList_t *,
    ngcliInformationService_t *, ngcliInformationServiceQuerySession_t *, 
    ngLog_t *, int *);
static int ngcllQueryResultWaiterListPop(ngcllQueryResultWaiterList_t *,
    ngLog_t *, int *);
static void ngcllQueryResultWaiterInitializeMember(ngcllQueryResultWaiter_t *);

static int ngcllQueryResultWaitersFindSuccessfulQuery(
    ngcllQueryResultWaiterList_t *, int *, ngLog_t *, int *);
static int ngcllInformationServiceQueryAndPush(
    ngcliInformationService_t *, char *, char *,
    ngcllQueryResultWaiterList_t *, int *);

/* Query Manager */
static int ngcllQueryManagerInitialize(ngcliQueryManager_t *, ngclContext_t *, int *);
static int ngcllQueryManagerFinalize(ngcliQueryManager_t *, int *);
static void ngcllQueryManagerInitializeMember(ngcliQueryManager_t *);

static int ngcllQueryManagerExistQuery(
    ngcliQueryManager_t *, char *, char *, int *);
static int ngcllQueryManagerDoQuery(
    ngcliQueryManager_t *, char *, char *, char *, int *);
static int ngcllQueryManagerSetInformation(ngcliQueryManager_t *, char *, char *, int,
    ngclExecutablePathInformation_t *, ngRemoteClassInformation_t *, int *);
static int ngcllQueryManagerWait(
    ngcliQueryManager_t *, char *, char *, ngcliInformationServiceQuery_t *, int *);


/**
 * Macros
 */
#define NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng)       \
    do {                                                      \
        assert((isMng) != NULL);                              \
        assert((isMng)->ngism_context != NULL);               \
        assert((isMng)->ngism_externalModuleManager != NULL); \
    } while (0)

#define NGCLL_INFORMATION_SERVICE_ASSERT(infoServ)                         \
    do {                                                                   \
        assert((infoServ) != NULL);                                        \
        NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT((infoServ)->ngis_manager);\
    } while (0) 

#define ngis_tagName ngis_information.ngisi_tag
#define NGCLL_INFORMATION_SERVICE_USABLE(infoServ) \
    ((infoServ)->ngis_disabled == 0)


/**
 * Data
 */
#define NGCLL_INFORMATION_SERVICE_QUERY_REQUEST \
    "QUERY_REMOTE_EXECUTABLE_INFORMATION"
#define NGCLL_INFORMATION_SERVICE_CANCEL_REQUEST \
    "CANCEL_QUERY"
#define NGCLL_INFORMATION_SERVICE_EXIT_REQUEST \
    NGI_EXTERNAL_MODULE_REQUEST_EXIT
#define NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY \
    "REMOTE_EXECUTABLE_INFORMATION_NOTIFY"

static char *ngcllInformationServiceNecessaryRequests[] = {
    NGCLL_INFORMATION_SERVICE_QUERY_REQUEST,
    NGCLL_INFORMATION_SERVICE_CANCEL_REQUEST,
    NGCLL_INFORMATION_SERVICE_EXIT_REQUEST,
    NULL
};

#define NGCLL_INFORMATION_SERVICE_PROTOCOL_VERSION "1.0"

#define NGCLL_RESULT_SUCCESS       1
#define NGCLL_RESULT_FAILED        0
#define NGCLL_RESULT_NOT_APPEARED -1

static char *ngcllInformationServiceMultilineNotifies[] = {
    NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY,
    NULL
};


/**
 * Functions
 */

/**
 * Information append at last of the list.
 */
int
ngcliInformationServiceInformationCacheRegister(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliInformationServiceInformationManager_t *isInfoMng;
    static const char fName[] =
        "ngcliInformationServiceInformationCacheRegister";

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
    result = ngcliInformationServiceInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock InformationServiceInformation list.\n");
        goto error;
    }
    listLocked = 1;

    /* Is Information Service Information available? */
    if (isInfo->ngisi_tag != NULL) {
        isInfoMng = ngcliInformationServiceInformationCacheGet(
            context, isInfo->ngisi_tag, &subError);
        if (isInfoMng != NULL) {
     
            /* Replace the Information Service Information */
            result = ngcllInformationServiceInformationReplace(
                context, isInfoMng, isInfo, error);
            if (result == 0) {
                ngclLogErrorContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't replace the Information Service Information.\n");
                goto error;
            }
        }
    }

    /* Unlock the list */
    result = ngcliInformationServiceInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Information Service Information list.\n");
        goto error;
    }

    /* Construct */
    if (isInfoMng == NULL) {
        isInfoMng = ngcllInformationServiceInformationConstruct(
            context, isInfo, error);
        if (isInfoMng == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't construct Information Service Information.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliInformationServiceInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Information Service Information list.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Information delete from the list.
 */
int
ngcliInformationServiceInformationCacheUnregister(
    ngclContext_t *context,
    char *tagName,
    int *error)
{
    int result;
    ngcliInformationServiceInformationManager_t *curr;
    static const char fName[] =
        "ngcliInformationServiceInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context Invalid.\n");
        return 0;
    }

    /* Lock the list */
    result = ngcliInformationServiceInformationListWriteLock(context,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock InformationServiceInformation list.\n");
        return 0;
    }

    if (tagName == NULL) {
        /* Delete all information */

        /* Get the data from the head of a list */
        curr = ngcliInformationServiceInformationCacheGetNext(
            context, NULL, error);
        if (curr == NULL) {
             ngclLogDebugContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "No Information Service Information was registered.\n");
        }

        while (curr != NULL) {
            /* Destruct the data */
            result = ngcllInformationServiceInformationDestruct(
                context, curr, error);
            if (result == 0) {
                ngclLogErrorContext(context,
                    NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct Information Service Information.\n");
                goto error;
            }

            /* Get next data from the list */
            curr = ngcliInformationServiceInformationCacheGetNext(
                context, NULL, error);
        }
    } else {
        /* Delete specified information */

        /* Get the data from the list by Information Service name */
        curr = ngcliInformationServiceInformationCacheGet(
            context, tagName, error);
        if (curr == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Information Service Information \"%s\" is not found.\n",
                tagName);
            goto error;
        }

        /* Destruct the data */
        result = ngcllInformationServiceInformationDestruct(
            context, curr, error);
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct Information Service Information.\n");
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliInformationServiceInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Information Service Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliInformationServiceInformationListWriteUnlock(context,
        context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the list of Information Service Information.\n");
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
ngcliInformationServiceInformationManager_t *
ngcliInformationServiceInformationCacheGet(
    ngclContext_t *context,
    char *tagName,
    int *error)
{
    int result;
    char *curTagName;
    ngcliInformationServiceInformationManager_t *isInfoMng;
    static const char fName[] = "ngcliInformationServiceInformationCacheGet";

    curTagName = NULL;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    isInfoMng = context->ngc_infoServiceInfo_head;
    if (isInfoMng == NULL) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogInfoContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Information Service Information is not found.\n");
        return NULL;
    }

    /* tagName can be NULL. */
    if (tagName == NULL) {
        return isInfoMng;
    }

    for (; isInfoMng != NULL; isInfoMng = isInfoMng->ngisim_next) {
        curTagName = isInfoMng->ngisim_info.ngisi_tag;
        if ((curTagName != NULL) && (strcmp(curTagName, tagName) == 0)) {
            /* Found */
            return isInfoMng;
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Information Service Information is not found by tag name \"%s\".\n",
        tagName);
    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliInformationServiceInformationManager_t *
ngcliInformationServiceInformationCacheGetNext(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliInformationServiceInformationCacheGetNext";

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
        if (context->ngc_infoServiceInfo_head != NULL) {
            assert(context->ngc_infoServiceInfo_tail != NULL);
            return context->ngc_infoServiceInfo_head;
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
        "The last Information Service Information was reached.\n");
    return NULL;
}

/**
 * Construct.
 */
static ngcliInformationServiceInformationManager_t *
ngcllInformationServiceInformationConstruct(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int result;
    ngcliInformationServiceInformationManager_t *isInfoMng;
    static const char fName[] = "ngcllInformationServiceInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfo != NULL);

    /* Allocate */
    isInfoMng = NGI_ALLOCATE(ngcliInformationServiceInformationManager_t,
        context->ngc_log, error);
    if (isInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Information Service Information.\n");
        return NULL;
    }

    /* Initialize */
    result = ngcllInformationServiceInformationManagerInitialize(
        context, isInfoMng, isInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Information Service Information.\n");
        goto error;
    }

    /* Register */
    result = ngcliContextRegisterInformationServiceInformation(
        context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Information Service Information"
            " for Ninf-G Context.\n");
        goto error;
    }

    /* Success */
    return isInfoMng;

    /* Error occurred */
error:
    result = ngcllInformationServiceInformationDestruct(context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the storage for Information Service Information Manager.\n");
        return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllInformationServiceInformationDestruct(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllInformationServiceInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterInformationServiceInformation(context,
        isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister the Information Service Information.\n");
        return 0;
    }

    /* Finalize */
    result = ngcllInformationServiceInformationManagerFinalize(
        context, isInfoMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service Information.\n");
        return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliInformationServiceInformationManager_t,
        isInfoMng, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate the Information Service Information.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngclInformationServiceInformation_t *
ngcliInformationServiceInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclInformationServiceInformation_t *isInfo;
    static const char fName[] = "ngcliInformationServiceInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    /* Allocate new storage */
    isInfo = ngiCalloc(1, sizeof (ngclInformationServiceInformation_t),
        context->ngc_log, error);
    if (isInfo == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for Information Service Information.\n");
        return NULL;
    }

    return isInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliInformationServiceInformationFree(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliInformationServiceInformationFree";

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
ngcllInformationServiceInformationManagerInitialize(
     ngclContext_t *context,
     ngcliInformationServiceInformationManager_t *isInfoMng,
     ngclInformationServiceInformation_t *isInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllInformationServiceInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);
    assert(isInfo != NULL);

    /* reset members */
    isInfoMng->ngisim_next = NULL;

    /* Copy to new information */
    result = ngcliInformationServiceInformationCopy(context, isInfo,
        &isInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Information Service Information.\n");
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
ngcllInformationServiceInformationManagerFinalize(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllInformationServiceInformationManagerFinalize";

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
    result = ngclInformationServiceInformationRelease(context,
        &isInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Information Service Information.\n");
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
ngcliInformationServiceInformationCopy(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *src,
    ngclInformationServiceInformation_t *dest,
    int *error)
{
    ngLog_t *log;
    int i, result;
    static const char fName[] = "ngcliInformationServiceInformationCopy";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL,
            NG_LOGCAT_NINFG_PURE, fName,
            "Ninf-G Context is not valid.\n");
        return 0;
    }

    /* Check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

    log = context->ngc_log;

    ngcllInformationServiceInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllInformationServiceInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = ngiStrdup((src)->member, log, error); \
        if ((dest)->member == NULL) { \
            ngclLogErrorContext(context, \
                NG_LOGCAT_NINFG_PURE, fName, \
                "Can't allocate the storage " \
                "for Information Service Information.\n"); \
            goto error; \
        } \
    } while(0)

#define  NGL_COPY_STRING_IF_VALID(str, dest, member) \
    do {\
        if ((src)->member != NULL) { \
            NGL_COPY_STRING(str, dest, member); \
        } \
    } while (0)

    NGL_COPY_STRING_IF_VALID(src, dest, ngisi_tag);
    NGL_COPY_STRING(src, dest, ngisi_type);
    NGL_COPY_STRING_IF_VALID(src, dest, ngisi_path);
    NGL_COPY_STRING_IF_VALID(src, dest, ngisi_logFilePath);

    /* Copy Sources */
    if (src->ngisi_nSources > 0) {
        dest->ngisi_sources = ngiCalloc(
            src->ngisi_nSources, sizeof(char *), log, error);
        if (dest->ngisi_sources == NULL) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate the storage for string table.\n");
            return 0;
        }
        /* copy all of elements */
        for (i = 0; i < src->ngisi_nSources; i++) {
            NGL_COPY_STRING(src, dest, ngisi_sources[i]);
        }
    }

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
    result = ngclInformationServiceInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Information Service Information.\n");
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclInformationServiceInformationRelease(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclInformationServiceInformationRelease";

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

    result = ngcllInformationServiceInformationRelease(context, isInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllInformationServiceInformationRelease(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceInformationRelease";
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
    if (isInfo->ngisi_tag != NULL)
        ngiFree(isInfo->ngisi_tag, log, error);
    if (isInfo->ngisi_type != NULL)
        ngiFree(isInfo->ngisi_type, log, error);
    if (isInfo->ngisi_path != NULL)
        ngiFree(isInfo->ngisi_path, log, error);
    if (isInfo->ngisi_logFilePath != NULL)
        ngiFree(isInfo->ngisi_logFilePath, log, error);
    if (isInfo->ngisi_sources != NULL) {
        for (i = 0; i < isInfo->ngisi_nSources; i++) {
            if (isInfo->ngisi_sources[i] != NULL) {
                ngiFree(isInfo->ngisi_sources[i], log, error);
            }
        }
        ngiFree(isInfo->ngisi_sources, log, error);
    }
    if (isInfo->ngisi_options != NULL) {
        for (i = 0; i < isInfo->ngisi_nOptions; i++) {
            if (isInfo->ngisi_options[i] != NULL) {
                ngiFree(isInfo->ngisi_options[i], log, error);
            }
        }
        ngiFree(isInfo->ngisi_options, log, error);
    }

    /* Initialize the members */
    ngcllInformationServiceInformationInitializeMember(isInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliInformationServiceInformationInitialize(
    ngclContext_t *context,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliInformationServiceInformationInitialize";

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

    ngcllInformationServiceInformationInitializeMember(isInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllInformationServiceInformationInitializeMember(
    ngclInformationServiceInformation_t *isInfo)
{
    /* Initialize the members */
    isInfo->ngisi_timeout = 0;
    isInfo->ngisi_nSources = 0;
    isInfo->ngisi_nOptions = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllInformationServiceInformationInitializePointer(
    ngclInformationServiceInformation_t *isInfo)
{
    /* Initialize the members */
    isInfo->ngisi_tag = NULL;
    isInfo->ngisi_type = NULL;
    isInfo->ngisi_path = NULL;
    isInfo->ngisi_logFilePath = NULL;
    isInfo->ngisi_sources = NULL;
    isInfo->ngisi_options = NULL;
}

/**
 * GetCopy
 */
int
ngclInformationServiceInformationGetCopy(
    ngclContext_t *context,
    char *tagName,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclInformationServiceInformationGetCopy";

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

    result = ngcllInformationServiceInformationGetCopy(context,
        tagName, isInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllInformationServiceInformationGetCopy(
    ngclContext_t *context,
    char *tagName,
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    int result;
    ngcliInformationServiceInformationManager_t *isInfoMng;
    static const char fName[] = "ngcllInformationServiceInformationGetCopy";

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Invalid argument.\n");
        return 0;
    }

   /* Lock the Information Service Information */
    result = ngcliInformationServiceInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Information Service Information.\n");
        return 0;
    }

    /* Get the Information Service Information */
    isInfoMng = ngcliInformationServiceInformationCacheGet(
        context, tagName, error);
    if (isInfoMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the Information Service Information.\n");
        goto error;
    }

    /* Copy the Information Service Information */
    result = ngcliInformationServiceInformationCopy(context,
                    &isInfoMng->ngisim_info, isInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Information Service Information.\n");
        goto error;
    }

    /* Unlock the Information Service Information */
    result = ngcliInformationServiceInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Information Service Information.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliInformationServiceInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Information Service Information.\n");
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Replace the Information Service Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInformationServiceInformationReplace(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *dstIsInfoMng,
    ngclInformationServiceInformation_t *srcIsInfo,
    int *error)
{
    ngLog_t *log;
    int result, isLocked;
    static const char fName[] = "ngcllInformationServiceInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstIsInfoMng != NULL);
    assert(srcIsInfo != NULL);

    log = context->ngc_log;
    isLocked = 0;

    /* log */
     ngclLogDebugContext(context,
        NG_LOGCAT_NINFG_PURE, fName,
        "Replace the Information Service Information for \"%s\".\n",
        srcIsInfo->ngisi_type);

    /* Lock */
    result = ngcliInformationServiceInformationWriteLock(
        dstIsInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write lock the Information Service Information.\n");
        goto error;
    }
    isLocked = 1;

    /* Release the Information Service Information */
    result = ngcllInformationServiceInformationRelease(
        context, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Information Service Information.\n");
        goto error;
    }

    /* Copy the Information Service Information */
    result = ngcliInformationServiceInformationCopy(
        context, srcIsInfo, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Information Service Information.\n");
        goto error;
    }

    /* Unlock */
    result = ngcliInformationServiceInformationWriteUnlock(
        dstIsInfoMng, log, error);
    isLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't write unlock the Information Service Information.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (isLocked != 0) {
        result = ngcliInformationServiceInformationWriteUnlock(
            dstIsInfoMng, log, NULL);
        isLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context,
                NG_LOGCAT_NINFG_PURE, fName,
                "Can't write unlock the Information Service Information.\n");
        }
    }

    /* Failed */
    return 0;
}


/**
 * Information Service Manager: Construct
 */
ngcliInformationServiceManager_t *
ngcliInformationServiceManagerConstruct(
    ngclContext_t *context,
    int *error)
{
    ngcliInformationServiceManager_t *isMng = NULL;
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcliInformationServiceManagerConstruct";

    assert(context != NULL);

    log = context->ngc_log;

    isMng = NGI_ALLOCATE(ngcliInformationServiceManager_t, log, error);
    if (isMng == NULL) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Information Service Manager.\n");
        goto error;
    }

    result = ngcllInformationServiceManagerInitialize(isMng, context, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Information Service Manager.\n");
        goto error;
    }

    return isMng;
error:
    result = ngcliInformationServiceManagerDestruct(isMng, NULL);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Service Manager.\n");
    }
    return NULL;
}

/**
 * Information Service Manager: Destruct
 */
int
ngcliInformationServiceManagerDestruct(
    ngcliInformationServiceManager_t *isMng,
    int *error)
{
    int ret = 1;
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
    int result;
    static const char fName[] = "ngcliInformationServiceManagerDestruct";

    if (isMng == NULL) {
        return 1; /* Success */
    }

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);
    context = isMng->ngism_context;
    log = context->ngc_log;

    result = ngcllInformationServiceManagerFinalize(isMng, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service Manager.\n");
        error = NULL;
        ret = 0;
    }

    result = NGI_DEALLOCATE(
        ngcliInformationServiceManager_t, isMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the Information Service Manager.\n");
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * Information Service Manager: Initialize
 */
static int
ngcllInformationServiceManagerInitialize(
    ngcliInformationServiceManager_t *isMng,
    ngclContext_t *context,
    int *error)
{
    int locked = 0;
    int result;
    int ret = 0;
    ngLog_t *log;
    ngcliInformationServiceInformationManager_t *it;
    ngcliInformationService_t *infoServ = NULL;
    static const char fName[] = "ngcllInformationServiceManagerInitialize";

    log = context->ngc_log;

    ngcllInformationServiceManagerInitializeMember(isMng);
    
    SLIST_INIT(&isMng->ngism_list);
    isMng->ngism_context = context;
    isMng->ngism_externalModuleManager = context->ngc_externalModuleManager;
    isMng->ngism_nLock = 0;

    result = ngiRWlockInitialize(&isMng->ngism_rwLock, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize rlock.\n");
        goto finalize;
    }

    result = ngcllInformationServiceContextGetLogFilePath(
        context, &isMng->ngism_logFilePath, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get the default log file path from context.\n");
        goto finalize;
    }

    /* Lock the Information Service Information */
    result = ngcliInformationServiceInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Information Service Information.\n");
        goto finalize;
    }
    locked = 1;

    /* Construct the Information Services by each <INFORMATION_SOURCE> */
    it = NULL;
    while (1) {
        /* Get the data from the head of a list */
        it = ngcliInformationServiceInformationCacheGetNext(
            context, it, error);
        if (it == NULL) {
             break;
        }
        infoServ = ngcllInformationServiceConstruct(
            isMng, &it->ngisim_info, error);
        if (infoServ == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't construct the Information Service.\n");
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (locked != 0) {
        /* Unlock the Information Service Information */
        result = ngcliInformationServiceInformationListReadUnlock(
            context, context->ngc_log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the list of Information Service Information.\n");
            ret = 0;
            error = NULL;
        }
        locked = 0;
    }
    return ret;
}

/**
 * Information Service Manager: Finalize
 */
static int
ngcllInformationServiceManagerFinalize(
    ngcliInformationServiceManager_t *isMng,
    int *error)
{
    int result;
    int ret = 1;
    ngcliInformationServiceInformationManager_t *it;
    ngclContext_t *context = NULL;
    ngLog_t *log = NULL;
    ngcliInformationService_t *infoServ = NULL;
    static const char fName[] = "ngcllInformationServiceManagerFinalize";

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);

    context = isMng->ngism_context;
    log = context->ngc_log;

    /* Destruct the Information Services by each <INFORMATION_SOURCE> */
    it = NULL;
    while (!SLIST_EMPTY(&isMng->ngism_list)) {
        infoServ = SLIST_FIRST(&isMng->ngism_list);
        assert(infoServ != NULL);

        result = ngcllInformationServiceDestruct(infoServ, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the Information Service.\n");
            error = NULL;
            ret = 0;
        }
    }

    result = ngiFree(isMng->ngism_logFilePath, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }

    result = ngiRWlockFinalize(&isMng->ngism_rwLock, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize rlock.\n");
        error = NULL;
        ret = 0;
    }

    ngcllInformationServiceManagerInitializeMember(isMng);

    return ret;
}

/**
 * Information Service Manager: Zero clear
 */
static void
ngcllInformationServiceManagerInitializeMember(
    ngcliInformationServiceManager_t *isMng)
{
    SLIST_INIT(&isMng->ngism_list);
    isMng->ngism_context = NULL;
    isMng->ngism_externalModuleManager = NULL;
    isMng->ngism_rwLock = NGI_RWLOCK_NULL;
    isMng->ngism_nLock = 0;
    isMng->ngism_logFilePath = NULL;

    return;
}

/**
 * Get the default log file path from local machine information in the context.
 *
 * If "information_service_log" attribute in <CLIENT> session is specified,
 * the its value is set to the pointer pointed by "logFilePath".
 * It must be free after it is used.
 *
 * Otherwise NULL is set.
 */
static int
ngcllInformationServiceContextGetLogFilePath(
    ngclContext_t *context,
    char **logFilePath,
    int *error)
{
    int ret = 0;
    int locked = 0;
    ngLog_t *log;
    ngclLocalMachineInformation_t *lmInfo = NULL;
    int result;
    static const char fName[] = "ngcllInformationServiceContextGetLogFilePath";

    assert(context != NULL);
    assert(logFilePath != NULL);

    log = context->ngc_log;
    *logFilePath = NULL;

    /* Lock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Local Machine Information.\n");
        goto finalize;
    }
    locked = 0;

    lmInfo = &context->ngc_lmInfo->nglmim_info;
    if (lmInfo->nglmi_infoServiceLog != NULL) {
        *logFilePath  = ngiStrdup(lmInfo->nglmi_infoServiceLog, log ,error);
        if (*logFilePath  == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the string.\n");
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    result = ngcliLocalMachineInformationListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Local Machine Information.\n");
        ret = 0;
        error = NULL;
    }
    if (ret == 0) {
        assert(error == NULL);
        result = ngiFree(*logFilePath, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the storage for the string.\n");
        }
    }

    return ret;
}

/**
 * Information Service Manager: Query Information
 */
int
ngcliInformationServiceManagerQuery(
    ngcliInformationServiceManager_t *isMng,
    char *hostName,
    char *className,
    char *tag,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngcliInformationService_t *tagInfoServ = NULL;
    ngcliInformationService_t *it = NULL;
    ngclContext_t *context;
    ngLog_t *log;
    int result;
    int ret = 0;
    int queryResult = 0;
    int doneWait = 0;
    int locked = 0; 
    time_t startTime = -1;
    ngcllQueryResultWaiterList_t waiters;
    ngcllQueryResultWaiter_t *itWaiter = NULL;
    static const char fName[] = "ngcliInformationServiceManagerQuery";

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);
    assert(NGI_STRING_IS_VALID(hostName));
    assert(NGI_STRING_IS_VALID(className));
    assert(epInfo != NULL);
    assert(rcInfo != NULL);

    context = isMng->ngism_context;
    log = context->ngc_log;
    STAILQ_INIT(&waiters);

    startTime = time(NULL);

    /* Read Lock */
    result = ngiRWlockReadLock(&isMng->ngism_rwLock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service Manager.\n");
        goto finalize;
    }
    locked = 1;

    if (SLIST_EMPTY(&isMng->ngism_list)) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "There are no available Information Service.\n");
        goto finalize;
    }

    if (tag != NULL) {
        tagInfoServ = ngcllInformationServiceManagerFindByTag(
            isMng, tag, error);
        if (tagInfoServ == NULL) {
            ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "There is no <INFORMATION_SOURCE> whose tag is \"%s\""
                " in configuration file.\n", tag);
            /* Ignores error */
        }
    }

    if (tagInfoServ != NULL) {
        it = tagInfoServ;
        result = ngcllInformationServiceQueryAndPush(
            it, hostName, className, &waiters, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't query the information.\n");
        }
    }

    SLIST_FOREACH(it, &isMng->ngism_list, ngis_entry) {
        if (tagInfoServ == it) {
            continue;
        }
        result = ngcllQueryResultWaitersFindSuccessfulQuery(
            &waiters, &queryResult, log, error);
        if (result == 0) {
            ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't find the successful query.\n");
            goto finalize;
        }
        if (queryResult != 0) {
            /* The query's result is success */
            break;
        }
        result = ngcllInformationServiceQueryAndPush(
            it, hostName, className, &waiters, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't query the information.\n");
        }
    }

    /* Wait */
    itWaiter = STAILQ_FIRST(&waiters);
    queryResult = 0;
    while ((queryResult == 0) && (itWaiter != NULL)) {
        result = ngcllInformationServiceWaitQueryResult(
            itWaiter->ngqrw_infoServ, itWaiter->ngqrw_session, &queryResult, startTime,
            epInfo, rcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait query result.\n");
            /* Ignores error */
        }
        itWaiter = STAILQ_NEXT(itWaiter, ngqrw_entry);
    }
    doneWait = 1;

    if (queryResult == 0) {
        /* NGI_SET_ERROR(error, NG_ERROR_); */
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get information for hostname=\"%s\" classname=\"%s\""
            " from the Information Services.\n", hostName, className);
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (doneWait == 0) {
        assert(itWaiter == NULL);    
        itWaiter = STAILQ_FIRST(&waiters);
    }
    while (itWaiter != NULL) {
        result = ngcllInformationServiceCancelQuery(
            itWaiter->ngqrw_infoServ, itWaiter->ngqrw_session, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't cancel query.\n");
            /* Ignore error */
        }
        itWaiter = STAILQ_NEXT(itWaiter, ngqrw_entry);
    }

    while (!STAILQ_EMPTY(&waiters)) {
        result = ngcllQueryResultWaiterListPop(&waiters, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't pop query id to list.\n");
            ret = 0;
            error = NULL;
        }
    }

    /* Read Lock */
    if (locked != 0) {
        result = ngiRWlockReadUnlock(&isMng->ngism_rwLock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Information Service Manager.\n");
            ret = 0;
            error = NULL;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Information Service Manager: Find Information Service by tag name.
 */
static ngcliInformationService_t *
ngcllInformationServiceManagerFindByTag(
    ngcliInformationServiceManager_t *isMng,
    char *tag,
    int *error)
{
    ngcliInformationService_t *it;
    ngcliInformationService_t *ret = NULL;

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);
    assert(tag != NULL);
    assert(strlen(tag) > 0);

    SLIST_FOREACH(it, &isMng->ngism_list, ngis_entry) {
        if (strcmp(tag, it->ngis_information.ngisi_tag) == 0) {
            ret = it;
            break;
        }
    }

    return ret;
}

/**
 * Information Service Manager: Register the Information Service
 */
static int
ngcllInformationServiceManagerRegister(
    ngcliInformationServiceManager_t *isMng,
    ngcliInformationService_t *infoServ,
    int *error)
{
    ngcliInformationService_t *it = NULL;
    ngcliInformationService_t *last = NULL;
    

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);

    assert(isMng != NULL);
    assert(infoServ->ngis_registered == 0);

    SLIST_FOREACH(it, &isMng->ngism_list, ngis_entry) {
        last = it;
    }
    if (last == NULL) {
        SLIST_INSERT_HEAD(&isMng->ngism_list, infoServ, ngis_entry);
    } else {
        SLIST_INSERT_AFTER(last, infoServ, ngis_entry);
    }


    infoServ->ngis_registered = 1;/* true */

    return 1;
}

/**
 * Information Service Manager: Unregister the Information Service
 */
static int
ngcllInformationServiceManagerUnregister(
    ngcliInformationServiceManager_t *isMng,
    ngcliInformationService_t *infoServ,
    int *error)
{
    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);
    assert(infoServ!= NULL);
    assert(infoServ->ngis_registered != 0);

    SLIST_REMOVE(&isMng->ngism_list,
        infoServ, ngcliInformationService_s, ngis_entry);
    infoServ->ngis_registered = 0;

    return 1;
}

/**
 * Information Service: Construct
 */
static ngcliInformationService_t *
ngcllInformationServiceConstruct(
    ngcliInformationServiceManager_t *isMng,
    ngclInformationServiceInformation_t *info,
    int *error)
{
    ngcliInformationService_t *infoServ = NULL;
    ngclContext_t *context;
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllInformationServiceConstruct";

    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);

    context = isMng->ngism_context;
    log = context->ngc_log;

    infoServ = NGI_ALLOCATE(ngcliInformationService_t, log, error);
    if (infoServ == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Information Service.\n");
        goto error;
    }

    result = ngcllInformationServiceInitialize(infoServ, isMng, info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Information Service.\n");
        goto error;
    }

    result = ngcllInformationServiceManagerRegister(isMng, infoServ, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Information Service to Manager.\n");
        goto error;
    }

    return infoServ;
error:
    result = ngcllInformationServiceDestruct(infoServ, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Service.\n");
    }
    return NULL;
}

/**
 * Information Service: Destruct
 */
static int
ngcllInformationServiceDestruct(
    ngcliInformationService_t *infoServ,
    int *error)
{
    ngclContext_t *context;
    ngcliInformationServiceManager_t *isMng;
    ngLog_t *log;
    int result;
    int ret = 1;
    static const char fName[] = "ngcllInformationServiceDestruct";

    if (infoServ == NULL) {
        /* Do nothing */
        return 1;
    }

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    
    isMng   = infoServ->ngis_manager;
    context = isMng->ngism_context;
    log     = context->ngc_log;

    result = ngcllInformationServiceManagerUnregister(isMng, infoServ, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unregister the Information Service from the Manager.\n");
        ret = 0;
        error = NULL;
    }

    result = ngcllInformationServiceFinalize(infoServ, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service.\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngcliInformationService_t, infoServ, log ,error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Information Service.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Information Service: Initialize
 */
static int
ngcllInformationServiceInitialize(
    ngcliInformationService_t *infoServ,
    ngcliInformationServiceManager_t *isMng, 
    ngclInformationServiceInformation_t *isInfo,
    int *error)
{
    ngclContext_t *context;
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllInformationServiceInitialize";

    assert(infoServ != NULL);
    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);

    context = isMng->ngism_context;
    log     = context->ngc_log;

    ngcllInformationServiceInitializeMember(infoServ);

    infoServ->ngis_manager = isMng;

    result = ngcliInformationServiceInformationCopy(
        context, isInfo, &infoServ->ngis_information, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Information Service Information.\n");
        goto error;
    }
    infoServ->ngis_informationGotten = 1;

    result = ngiRlockInitialize(&infoServ->ngis_rlock, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize rlock.\n");
        goto error;
    }

    return 1;
error:
    result = ngcllInformationServiceFinalize(infoServ, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service.\n");
    }

    return 0;
}

/**
 * Information Service: Finalize
 */
static int
ngcllInformationServiceFinalize(
    ngcliInformationService_t *infoServ,
    int *error)
{
    ngcliInformationServiceManager_t *isMng;
    ngcliInformationServiceQuerySession_t *it;
    ngcliInformationServiceQuerySession_t *tit;
    ngclContext_t *context;
    ngLog_t *log;
    int ret = 1;
    int result;
    int valid = 0;
    int locked = 0;
    static const char fName[] = "ngcllInformationServiceFinalize";

    isMng = infoServ->ngis_manager;
    if (isMng == NULL) {
        /* Do nothing */
        return 1;
    }
    NGCLL_INFORMATION_SERVICE_MANAGER_ASSERT(isMng);
    assert(infoServ->ngis_registered == 0);

    context = isMng->ngism_context;
    log = context->ngc_log;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        ret = 0;
        error = NULL;
    } else {
        locked = 1;
    }

    if (infoServ->ngis_externalModule != NULL) {
        result = ngiExternalModuleIsValid(infoServ->ngis_externalModule, &valid, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't check whether External Module is valid or not.\n");
            ret = 0;
            error = NULL;
            valid = 0;
        }
        if (valid != 0) {
            /* Send EXIT request */
            result = ngcllInformationServiceSendExit(infoServ, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't send %s request.\n",
                    NGCLL_INFORMATION_SERVICE_EXIT_REQUEST);
                ret = 0;
                error = NULL;
                /* Through */
            }
        }

        result = ngiExternalModuleNotifyCallbackUnregister(
            infoServ->ngis_externalModule, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Unregister The Notify callback failed.\n");
            ret = 0;
            error = NULL;
            /* Through */
        }
    }

    if (locked != 0) {
        while (infoServ->ngis_notifyHandling != 0) {
            result = ngiRlockWait(&infoServ->ngis_rlock, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't lock the Information Service.\n");
                error = NULL;
                ret = 0;
            }
        }
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    if (infoServ->ngis_externalModule != NULL) {
        result = ngiExternalModuleDestruct(
            infoServ->ngis_externalModule, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the External Module.\n");
            ret = 0;
            error = NULL;
        }

        infoServ->ngis_externalModule = NULL;;
    }

    SLIST_FOREACH_SAFE(it, &infoServ->ngis_querySessions, ngiqs_entry, tit) {
        result = ngcllInformationServiceQuerySessionDestruct(it, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the query session.\n");
            ret = 0;
            error = NULL;
        }
    }

    SLIST_INIT(&infoServ->ngis_querySessions);

    result = ngiRlockFinalize(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize rlock.\n");
        ret = 0;
        error = NULL;
    }

    if (infoServ->ngis_informationGotten != 0) {
        result = ngclInformationServiceInformationRelease(context, 
            &infoServ->ngis_information, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't release the Information Service Information.\n");
            ret = 0;
            error = NULL;
        }
        infoServ->ngis_informationGotten = 0;
    }

    ngcllInformationServiceInitializeMember(infoServ);

    return ret;
}

/**
 * Information Service: Zero clear
 */
static void
ngcllInformationServiceInitializeMember(
    ngcliInformationService_t *infoServ)
{
    assert(infoServ != NULL);

    ngcllInformationServiceInformationInitializeMember(
        &infoServ->ngis_information);
    infoServ->ngis_registered        = 0;
    infoServ->ngis_manager           = NULL;
    infoServ->ngis_informationGotten = 0;
    infoServ->ngis_rlock             = NGI_RLOCK_NULL;
    infoServ->ngis_disabled          = 0;
    infoServ->ngis_notifyHandling    = 0;
    SLIST_INIT(&infoServ->ngis_querySessions);
    infoServ->ngis_externalModule    = NULL;;
}

/**
 * Information Service: Sends QUERY_REMOTE_EXECUTABLE_INFORMATION
 * If successful, returns query id. otherwise returns NULL.
 * query id must be released after it is used.
 */
static ngcliInformationServiceQuerySession_t *
ngcllInformationServiceQuery(
    ngcliInformationService_t *infoServ,
    char *hostName,
    char *className,
    int *error)
{
    ngclInformationServiceInformation_t *isInfo;
    ngcliInformationServiceManager_t *isMng;
    ngiExternalModuleManager_t *extMng;
    ngclContext_t *context;
    ngLog_t *log;
    int ret = 0;
    ngcliInformationServiceQuerySession_t *session = NULL;
    int result;
    int locked = 0;
    char *queryID = NULL;
    static const char fName[] = "ngcllInformationServiceQuery";

    assert(hostName  != NULL);
    assert(className != NULL);

    isMng   = infoServ->ngis_manager;
    extMng  = isMng->ngism_externalModuleManager;
    isInfo  = &infoServ->ngis_information;
    context = isMng->ngism_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if (!NGCLL_INFORMATION_SERVICE_USABLE(infoServ)) {
        /* for reinvoking */
        if (infoServ->ngis_externalModule != NULL) {
            result = ngiExternalModuleDestruct(infoServ->ngis_externalModule, log, error);
            infoServ->ngis_externalModule = NULL;
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the External Module.\n");
                goto finalize;
            }
        }
        infoServ->ngis_disabled = 0;
    }
    if (infoServ->ngis_externalModule == NULL) {
        result = ngcllInformationServiceInvokeProcess(infoServ, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't invoke the Information Service process.\n");
            goto finalize;
        }
    }
    queryID = ngcllInformationServiceSendQueryREI(
        infoServ, hostName, className, error);
    if (queryID == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't send QUERY_REMOTE_EXECUTABLE_INFORMATION request.\n");
        goto finalize;
    }

    session = ngcllInformationServiceQuerySessionConstruct(
        infoServ, queryID, log, error);
    if (session == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the Query Session.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    result = ngiFree(queryID, log, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the string.\n");
    }
    queryID = NULL;

    return session;
}

/**
 * Information Service: Invokes the Process
 */
static int
ngcllInformationServiceInvokeProcess(
    ngcliInformationService_t *infoServ,
    int *error)
{
    ngclInformationServiceInformation_t *isInfo;
    ngcliInformationServiceManager_t *isMng;
    ngiExternalModuleManager_t *extMng;
    ngclContext_t *context;
    ngLog_t *log;
    int ret = 0;
    ngiExternalModule_t *module = NULL;
    int result;
    int locked = 0;
    char *logFile = NULL;
    char *logFileDef = NULL;
    static const char fName[] = "ngcllInformationServiceInvokeProcess";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(infoServ->ngis_externalModule == NULL);

    isMng   = infoServ->ngis_manager;
    extMng  = isMng->ngism_externalModuleManager;
    isInfo  = &infoServ->ngis_information;
    context = isMng->ngism_context;
    log     = context->ngc_log;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if (isInfo->ngisi_logFilePath != NULL) {
        logFile = isInfo->ngisi_logFilePath;
    } else if (isMng->ngism_logFilePath != NULL) {
        logFileDef = ngiStrdupPrintf(log, error, "%s.%s",
            isMng->ngism_logFilePath, isInfo->ngisi_tag);
        if (logFileDef == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate the log filename.\n");
            goto finalize;
        }

        logFile = logFileDef;
    }

    /* Information Service is not used by Job.
     * Thus max jobs is invalid.
     */
    module = ngiExternalModuleConstruct(
        extMng, NGI_EXTERNAL_MODULE_TYPE_INFORMATION_SERVICE,
        NGI_EXTERNAL_MODULE_SUB_TYPE_NORMAL,
        isInfo->ngisi_type, isInfo->ngisi_path, NULL, logFile, 0,
        ngcllInformationServiceMultilineNotifies, log, error);
    if (module == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the External Module.\n");
        goto finalize;
    }
    infoServ->ngis_externalModule = module;

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,
        "The notify callback argument address is %p.\n", isMng);

    result = ngiExternalModuleNotifyCallbackRegister(
        module, ngcllInformationServiceNotifyCallback,
        infoServ, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
        "Register The Notify callback failed.\n");
        goto finalize;
    }
    infoServ->ngis_notifyHandling = 1;

    /* Query Features */
    result = ngcllInformationServiceQueryFeatures(
        infoServ, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't query features.\n");
        goto finalize;
    }

    ret = 1;

finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiFree(logFileDef, log, error);
    if (result == 0) {
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't deallocate log filename.\n");
        }
    }

    if (ret == 0) {
        result = ngcllInformationServiceDisable(infoServ, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Information Service.\n");
        }
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }
    return ret;
}

/**
 * Information Service: Checks Query's result
 *
 * If REMOTE_EXECUTABLE_INFORMATION_NOTIFY has been received
 * and its result is successful, "found" is set to true.
 */
static int
ngcllInformationServiceCheckQueryResult(
    ngcliInformationService_t *infoServ,
    ngcliInformationServiceQuerySession_t *session,
    int *found,
    int *error)
{
    int result; 
    ngLog_t *log;
    ngclContext_t *context;
    int ret = 0;
    int locked = 0;
    static const char fName[] = "ngcllInformationServiceCheckQueryResult";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(found != NULL);
    assert(session != NULL);
    assert(session->ngiqs_owner == infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;
    *found = 0;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if ((session->ngiqs_result != NULL) &&
        (session->ngiqs_result->ngiqr_result != 0)) {
        /* Query is successful. */
        *found = 1;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }
    return ret;
}

/**
 * Information Service: Waits Query's result
 */
static int
ngcllInformationServiceWaitQueryResult(
    ngcliInformationService_t *infoServ,
    ngcliInformationServiceQuerySession_t *session,
    int *found, 
    time_t startTime,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result; 
    ngLog_t *log;
    ngclContext_t *context;
    int ret = 0;
    int locked = 0;
    time_t endTime = -1;
    time_t remain;
    int timeout = 0;
    static const char fName[] = "ngcllInformationServiceWaitQueryResult";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(found != NULL);
    assert(epInfo != NULL);
    assert(rcInfo != NULL);
    assert(session != NULL);
    assert(session->ngiqs_owner == infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;
    *found = 0;
    if (infoServ->ngis_information.ngisi_timeout > 0) {
        endTime = startTime + infoServ->ngis_information.ngisi_timeout;
    }

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if (session->ngiqs_waiting != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "This query session is waited for another thread.\n");
        goto finalize;
    }
    session->ngiqs_waiting = 1;

    /* Wait notify */
    while ((session->ngiqs_result == NULL) &&
           (NGCLL_INFORMATION_SERVICE_USABLE(infoServ))) {
        if (endTime < 0) {
            result = ngiRlockWait(&infoServ->ngis_rlock, log, error);
        } else {
            remain = endTime - time(NULL);
            if (remain <= 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Timeout occurred.\n");
                goto finalize;
            }
            result = ngiRlockTimedWait(
                &infoServ->ngis_rlock, remain, &timeout, log, error);
        }
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait the condition for the Information Service.\n");
            goto finalize;
        }
        if (timeout != 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Timeout occurred.\n");
            goto finalize;
        }
    }

    if (session->ngiqs_result == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "The Information Service has been disabled.\n");
        goto finalize;
    }

    if (session->ngiqs_result->ngiqr_result != 0) {
        /* Query is successful. */
        /* Set rcInfo and epInfo */
        assert(session->ngiqs_result->ngiqr_info != NULL);
        result = ngcliParseRemoteExecutableInformation(
            context, session->ngiqs_result->ngiqr_info,
            epInfo, rcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't parse the Remote Executable Information.\n");
            goto finalize;
        }
        *found = 1;
    } else {
        /* handling errorCode for callback. */
        assert(session->ngiqs_result->ngiqr_message != NULL);
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "%s\n", session->ngiqs_result->ngiqr_message);
        NGI_SET_ERROR(error, session->ngiqs_result->ngiqr_errorCode);
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    session->ngiqs_waiting = 0;

    result = ngcllInformationServiceQuerySessionDestruct(session, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Service Query Session.\n");
        error = NULL;
        ret = 0;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }
    return ret;
}

/**
 * Information Service: Cancels query
 */
static int
ngcllInformationServiceCancelQuery(
    ngcliInformationService_t *infoServ,
    ngcliInformationServiceQuerySession_t *session,
    int *error)
{
    ngLog_t *log;
    ngclContext_t *context;
    int replySuccess;
    char *replyMessage = NULL;
    int ret = 0;
    int result;
    int locked = 0;
    static const char fName[] = "ngcllInformationServiceCancelQuery";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(session != NULL);
    assert(session->ngiqs_owner == infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if (session->ngiqs_result != NULL) {
        if (session->ngiqs_waiting != 0) {
            /* Do nothing */
        } else {
            /* Already has received the result */
            result = ngcllInformationServiceQuerySessionDestruct(session, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the Information Service Query Session.\n");
                goto finalize;
            }
        }
    } else {
        if (session->ngiqs_canceled != 0) {
            /* Already canceled, does nothing */
        } else {
            /* Set cancel flag */
            session->ngiqs_canceled = 1;
            if (NGCLL_INFORMATION_SERVICE_USABLE(infoServ)) {
                /* Send Request */
                assert(session->ngiqs_id);
                result = ngiExternalModuleRequest(infoServ->ngis_externalModule,
                    NGCLL_INFORMATION_SERVICE_CANCEL_REQUEST, session->ngiqs_id,
                    NULL, 0, &replySuccess, &replyMessage, NULL, log, error);
                if (result == 0) {
                    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                        "Request to the Information Service failed.\n");
                    goto finalize;
                }

                if (replySuccess == 0) {
                    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                        "Request's result is \"failed\":%s\n", 
                        NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
                    goto finalize;
                }
            } else {
                result = ngcllInformationServiceQuerySessionDestruct(session, log, error);
                if (result == 0) {
                    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't destruct the Information Service Query Session.\n");
                    goto finalize;
                }
            }
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    result = ngiFree(replyMessage, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }
    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Information Service: Sends QUERY_FEATURES
 */
static int 
ngcllInformationServiceQueryFeatures(
    ngcliInformationService_t *infoServ,
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
    int ret = 0;
    static const char fName[] = "ngcllInformationServiceQueryFeatures";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;

    /* Query Features. */
    result = ngiExternalModuleQueryFeatures(
        infoServ->ngis_externalModule,
        &requestSuccess, &version, &features, &requests,
        &errorMessage, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed.\n");
        goto finalize;
    }
    if (requestSuccess == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed: \"%s\".\n",
            ((errorMessage != NULL) ? errorMessage : ""));

        result = ngiFree(errorMessage, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free error message string.\n");
            ret = 0;
            error = NULL;
        }
        errorMessage = NULL;

        goto finalize;
    }

    assert(version  != NULL);
    assert(features != NULL);
    assert(requests != NULL);

    /* Check Version */
    if (strcmp(version, NGCLL_INFORMATION_SERVICE_PROTOCOL_VERSION) != 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Version \"%s\" is unknown.\n", version);
        goto finalize;
    }

    /* Check Feature */
    /* Now, Ninf-G Client cannot use features. */

    /* Check REQUESTS */
    for (p = ngcllInformationServiceNecessaryRequests; *p != NULL;++p) {
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
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
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

    return ret;
}

/**
 * Information Service: Sends QUERY_REMOTE_EXECUTABLE_INFORMATION
 * If successful, returns query id. otherwise returns NULL.
 * query id must be released after it is used.
 */
static char *
ngcllInformationServiceSendQueryREI(
    ngcliInformationService_t *infoServ,
    char *hostName,
    char *className,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclContext_t *context;
    ngiLineList_t *arguments = NULL;
    int i;
    int replySuccess;
    char *replyMessage = NULL;
    char *queryID = NULL;
    int ret = 0;
    static const char fName[] = "ngcllInformationServiceSendQueryREI";

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;

    arguments = ngiLineListConstruct(log, error);
    if (arguments == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize line list.\n");
        goto finalize;
    }

    /* Append options to list */
    result = 1;
    result = result &&
        ngiLineListPrintf(arguments, log, error, "classname %s", className) &&
        ngiLineListPrintf(arguments, log, error, "hostname %s", hostName);
    for (i = 0;i < infoServ->ngis_information.ngisi_nSources;++i) {
        result = result &&
            ngiLineListPrintf(arguments, log, error, "source %s",
                infoServ->ngis_information.ngisi_sources[i]);
    }

    for (i = 0;i < infoServ->ngis_information.ngisi_nOptions;++i) {
        result = result &&
            ngiLineListAppend(arguments,
                infoServ->ngis_information.ngisi_options[i], log, error);
    }
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    /* Send Request */
    result = ngiExternalModuleRequest(
        infoServ->ngis_externalModule,
        NGCLL_INFORMATION_SERVICE_QUERY_REQUEST, NULL,
        arguments, 0,
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        goto finalize;
    }

    if (replySuccess == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
        goto finalize;
    }
    if (replyMessage == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get query id from %s reply.\n",
            NGCLL_INFORMATION_SERVICE_QUERY_REQUEST);
        goto finalize;
    }
    queryID = replyMessage;
    replyMessage = NULL;
    ret = 1;
finalize:
    if (ret != 0) {
        error = NULL;
    }

    result = ngiFree(replyMessage, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the storage for the string.\n");
        error = NULL;
        ret = 0;
    }

    if (arguments != NULL) {
        result = ngiLineListDestruct(arguments, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the Line List.\n");
            error = NULL;
            ret = 0;
        }
    }
    if (ret == 0) {
        result = ngiFree(queryID, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the storage for the string.\n");
            error = NULL;
        }
        queryID = NULL;
    }
    return queryID;
}

/**
 * Information Service: Send "EXIT"
 */
static int
ngcllInformationServiceSendExit(
    ngcliInformationService_t *infoServ,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    int replySuccess = 0;
    char *replyMessage = NULL;
    ngclContext_t *context = NULL;
    int ret = 0;
    static const char fName[] = "ngcllInformationServiceSendExit";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log     = context->ngc_log;
    
    if (infoServ->ngis_externalModule != NULL) {
        /* Send Request */
        result = ngiExternalModuleRequest(
            infoServ->ngis_externalModule,
            NGCLL_INFORMATION_SERVICE_EXIT_REQUEST, NULL,
            NULL, 0, 
            &replySuccess, &replyMessage, NULL, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Request to the Information Service failed.\n");
            goto finalize;
        }

        if (replySuccess == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "%s: Request's result is \"failed\": %s.\n",
                infoServ->ngis_tagName,
                NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiFree(replyMessage, log ,error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the string.\n");
    }

    return ret;
}

/**
 * Callback function for the notifies of the Information Service.
 */
static int
ngcllInformationServiceNotifyCallback(
    void *argument,
    ngiExternalModuleNotifyState_t state,
    char *notifyName,
    char *params,
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error)
{
    ngcliInformationService_t *infoServ = argument;
    ngclContext_t *context = NULL;
    int result;
    int ret = 0;
    int fatal = 0;
    int locked = 0;
    ngcliInformationServiceQueryResult_t *queryResult = NULL;
    ngcliInformationServiceQuerySession_t *it;
    ngcliInformationServiceQuerySession_t *session = NULL;
    static const char fName[] = "ngcllInformationServiceNotifyCallback";

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "Called with argument %p, STATUS = %d.\n", argument, (int)state);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;

    switch (state) {
    case NGI_EXTERNAL_MODULE_NOTIFY_NORMAL:
        break;
    case NGI_EXTERNAL_MODULE_NOTIFY_ERROR:
        ret = 0;
        error = NULL;
        goto fatal;
    case NGI_EXTERNAL_MODULE_NOTIFY_CLOSED:
    case NGI_EXTERNAL_MODULE_NOTIFY_CANCELED:
        ret = 1;
        goto callback_end;
    default:
        assert("NOTREACHED" == NULL);
    }

    assert(notifyName != NULL);

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        goto finalize;
    }
    locked = 1;

    if (infoServ->ngis_disabled != 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Information Service has been disabled.\n");
        goto callback_end;
    }

    if (strcmp(notifyName, NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY) == 0) {

        queryResult = ngcllParseREInotifyArguments(lines, log, error);
        if (queryResult == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't parse REMOTE_EXECUTABLE_INFORMATION_NOTIFY's argument.\n");
            goto finalize;
        }
        /* Found QUERY */
        SLIST_FOREACH(it, &infoServ->ngis_querySessions, ngiqs_entry) {
            if ((it->ngiqs_id != NULL) &&
                (strcmp(it->ngiqs_id, queryResult->ngiqr_id) == 0)) {
                session = it;
                break;
            }
        }
        if (session == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" notify's query id is \"%s\", but the Information"
                " Service does not query with \"%s\".\n",
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY,
                queryResult->ngiqr_id, queryResult->ngiqr_id);
            goto finalize;
        }
        session->ngiqs_result = queryResult;
        queryResult = NULL;

        if ((session->ngiqs_canceled != 0) &&
            (session->ngiqs_waiting == 0)) {
            result = ngcllInformationServiceQuerySessionDestruct(session, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the Information Service Query Session.\n");
                goto finalize;
            }
        } else {
            result = ngiRlockBroadcast(&infoServ->ngis_rlock, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't broadcast signal.\n");
                goto finalize;
            }
        }
    } else {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" is unknown notify.\n", notifyName);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret != 0) {
        error = NULL;
    }

    /* Register the Notify callback function. */
    result = ngiExternalModuleNotifyCallbackRegister(
        infoServ->ngis_externalModule, ngcllInformationServiceNotifyCallback,
        infoServ, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Failed to register the notify handling callback function.\n");
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
fatal:
        result = ngcllInformationServiceDisable(infoServ, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't disable the Information Service.\n");
            ret = 0;
            error = NULL;
        }

callback_end:
        if (locked == 0) {
            result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't lock the Information Service.\n");
                error = NULL;
                ret = 0;
            } else {
                locked = 1;
            }
        }
        infoServ->ngis_notifyHandling = 0;
        result = ngiRlockBroadcast(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast signal for the Information Service.\n");
            error = NULL;
            ret = 0;
        }
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Information Service.\n");
            error = NULL;
            ret = 0;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Information Service: Disable 
 */
static int
ngcllInformationServiceDisable(
    ngcliInformationService_t *infoServ,
    int *error)
{
    int result;
    int ret = 1;
    int locked = 0;
    ngclContext_t *context;
    ngLog_t *log;
    ngcliInformationServiceQuerySession_t *it;
    ngcliInformationServiceQuerySession_t *tit;
    static const char fName[] = "ngcllInformationServiceDisable";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);

    context = infoServ->ngis_manager->ngism_context;
    log =  context->ngc_log;

    result = ngiRlockLock(&infoServ->ngis_rlock, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Information Service.\n");
        ret = 0;
        error = NULL;
    } else {
        locked = 1;
    }

    infoServ->ngis_disabled = 1;

    SLIST_FOREACH_SAFE(it, &infoServ->ngis_querySessions, ngiqs_entry, tit) {
        if (it->ngiqs_canceled != 0) {
            result = ngcllInformationServiceQuerySessionDestruct(it, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't destruct the query session.\n");
                ret = 0;
                error = NULL;
            }
        } else {
            result = ngiFree(it->ngiqs_id, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't free storage for the string.\n");
                ret = 0;
                error = NULL;
            }
            it->ngiqs_id = NULL;
        }
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&infoServ->ngis_rlock, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Information Service.\n");
            ret = 0;
            error = NULL;
        }
        locked = 0;
    }

    return ret;
}

/**
 * Information Service Query Session: Construct
 */
static ngcliInformationServiceQuerySession_t *
ngcllInformationServiceQuerySessionConstruct(
    ngcliInformationService_t *infoServ,
    char *queryID,
    ngLog_t *log,
    int *error)
{
    ngcliInformationServiceQuerySession_t *session = NULL;
    int result;
    static const char fName[] = "ngcllInformationServiceQuerySessionConstruct";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(NGI_STRING_IS_VALID(queryID));

    session = NGI_ALLOCATE(ngcliInformationServiceQuerySession_t, log, error);
    if (session == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Information Service Query Session\n");
        goto error;
    }

    result = ngcllInformationServiceQuerySessionInitialize(
        session, infoServ, queryID, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Information Service Query Session\n");
        goto error;
    }

    SLIST_INSERT_HEAD(&infoServ->ngis_querySessions, session, ngiqs_entry);
    session->ngiqs_registered = 1;

    return session;
error:
    result = ngcllInformationServiceQuerySessionDestruct(session, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct for the Information Service Query Session\n");
    }

    return NULL;
}

/**
 * Information Service Query Session: Destruct
 */
static int
ngcllInformationServiceQuerySessionDestruct(
    ngcliInformationServiceQuerySession_t *session,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllInformationServiceQuerySessionDestruct";

    if (session == NULL) {
        /* Do nothing*/
        return 0;
    }

    NGCLL_INFORMATION_SERVICE_ASSERT(session->ngiqs_owner);

    /* Unregister */
    SLIST_REMOVE(&session->ngiqs_owner->ngis_querySessions, session,
        ngcliInformationServiceQuerySession_s, ngiqs_entry);
    session->ngiqs_registered = 0;

    result = ngcllInformationServiceQuerySessionFinalize(session, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service Query Session\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngcliInformationServiceQuerySession_t, session, log, error);
    if (session == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Information Service Query Session\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Information Service Query Session: Initialize
 */
static int
ngcllInformationServiceQuerySessionInitialize(
    ngcliInformationServiceQuerySession_t *session,
    ngcliInformationService_t *infoServ,
    char *queryID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllInformationServiceQuerySessionInitialize";

    assert(session != NULL);
    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(NGI_STRING_IS_VALID(queryID));

    ngcllInformationServiceQuerySessionInitializeMember(session);
    /* Deep copy */

    session->ngiqs_owner      = infoServ;
    session->ngiqs_result     = 0;
    session->ngiqs_registered = 0;
    session->ngiqs_canceled   = 0;
    session->ngiqs_waiting    = 0;
    session->ngiqs_id         = ngiStrdup(queryID, log, error);
    if (session->ngiqs_id == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't duplicate the string.\n");
    }

    return 1;
}

/**
 * Information Service Query Session: Finalize
 */
static int
ngcllInformationServiceQuerySessionFinalize(
    ngcliInformationServiceQuerySession_t *session,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllInformationServiceQuerySessionFinalize";

    assert(session != NULL);
    NGCLL_INFORMATION_SERVICE_ASSERT(session->ngiqs_owner);
    assert(session->ngiqs_registered == 0);

    result = ngiFree(session->ngiqs_id, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free storage for the string.\n");
        ret = 0;
        error = NULL;
    }

    result = ngcllInformationServiceQueryResultDestruct(session->ngiqs_result, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Session Query Result.\n");
        ret = 0;
        error = NULL;
    }

    ngcllInformationServiceQuerySessionInitializeMember(session);

    return 1;
}

static  void
ngcllInformationServiceQuerySessionInitializeMember(
    ngcliInformationServiceQuerySession_t *session)
{
    session->ngiqs_id = NULL;
    session->ngiqs_owner = NULL;
    session->ngiqs_result = NULL;
    session->ngiqs_registered = 0;
    session->ngiqs_canceled   = 0;
    session->ngiqs_waiting    = 0;
}

#define NGCLL_REI_NOTIFY_ID       "query_id"
#define NGCLL_REI_NOTIFY_RESULT   "result"
#define NGCLL_REI_NOTIFY_INFO     "remote_executable_information"
#define NGCLL_REI_NOTIFY_MESSAGE  "error_message"

/**
 * Parse REMOTE_EXECUTABLE_INFORMATION_NOTIFY's arguments
 */
static ngcliInformationServiceQueryResult_t *
ngcllParseREInotifyArguments(
    ngiLineList_t *lines,
    ngLog_t *log,
    int *error)
{
    ngcliInformationServiceQueryResult_t *queryResult = NULL;
    char *it = NULL;
    int result;
    static const char fName[] = "ngcllParseREInotifyArguments";

    queryResult = ngcllInformationServiceQueryResultConstruct(log, error);
    if (queryResult == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the Information Service Query Result.\n");
        goto error;
    }

    while ((it = ngiLineListLineGetNext(lines, it, log, NULL)) != NULL) {
        result = ngcllParseREInotifyArgumentsOneLine(
            queryResult, it, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't parse a line of \"%s\" arguments.\n",
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY);
            goto error;
        }
    }

    /* Check */
    if (queryResult->ngiqr_id == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" option is necessary, but does not appear in \"%s\" notify.\n",
            NGCLL_REI_NOTIFY_ID,
            NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        goto error;
    }

    if (queryResult->ngiqr_result == NGCLL_RESULT_NOT_APPEARED) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" option is necessary, but does not appear in \"%s\" notify.\n",
            NGCLL_REI_NOTIFY_RESULT,
            NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        goto error;
    }

    if (queryResult->ngiqr_result == NGCLL_RESULT_SUCCESS) {
        if (queryResult->ngiqr_info == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" option is necessary when success, "
                "but does not appear in \"%s\" notify.\n",
                NGCLL_REI_NOTIFY_INFO,
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY);
            goto error;
        }
        if (queryResult->ngiqr_message != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" option is invalid when success,\n",
                NGCLL_REI_NOTIFY_MESSAGE);
            goto error;
        }
    }

    if (queryResult->ngiqr_result == NGCLL_RESULT_FAILED) {
        if (queryResult->ngiqr_message == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" option is necessary when failed, "
                "but does not appear in \"%s\" notify.\n",
                NGCLL_REI_NOTIFY_MESSAGE,
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY);
            goto error;
        }
        if (queryResult->ngiqr_info != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" option is invalid when failed,\n",
                NGCLL_REI_NOTIFY_INFO);
            goto error;
        }
    }

    return queryResult;
error:
    result = ngcllInformationServiceQueryResultDestruct(queryResult, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Service Query Result.\n");
    }

    return NULL;
}

/**
 * Parses one line of REMOTE_EXECUTABLE_INFORMATION_NOTIFY's arguments.
 */
static int
ngcllParseREInotifyArgumentsOneLine(
    ngcliInformationServiceQueryResult_t *queryResult,
    char *line,
    ngLog_t *log,
    int *error)
{
    int ret = 0;
    ngiExternalModuleArgument_t *emArg;
    int result;
    static const char fName[] = "ngcllParseREInotifyArgumentsOneLine";

    assert(queryResult != NULL);
    assert(NGI_STRING_IS_VALID(line));

    emArg = ngiExternalModuleArgumentConstruct(line, log, error);
    if (emArg == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't parse notify's argument: %s.\n", line);
        goto finalize;
    }
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "line  = %s\n", line);
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "name  = %s\n", emArg->ngea_name);
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName, "value = %s\n", emArg->ngea_value);

    /* Handling query_id option */
    if (strcmp(emArg->ngea_name, NGCLL_REI_NOTIFY_ID) == 0) {
        if (queryResult->ngiqr_id != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "%s notify should not have more than one \"%s\" option.\n",
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY,
                NGCLL_REI_NOTIFY_ID);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }
        errno = 0;
        queryResult->ngiqr_id = ngiStrdup(emArg->ngea_value, log, error);
        if (queryResult->ngiqr_id == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the string for query id \"%s\".\n",
                emArg->ngea_value);
            goto finalize;
        }

    } else
    /* Handling result option */
    if (strcmp(emArg->ngea_name, NGCLL_REI_NOTIFY_RESULT) == 0) {
        if (queryResult->ngiqr_result != NGCLL_RESULT_NOT_APPEARED) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "%s notify should not have more than one \"%s\" option.\n",
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY,
                NGCLL_REI_NOTIFY_RESULT);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }
        if (strcmp(emArg->ngea_value, "S") == 0) {
            /* Success */
            queryResult->ngiqr_result = NGCLL_RESULT_SUCCESS;
        } else if (strcmp(emArg->ngea_value, "F") == 0) {
            /* Failed */
            queryResult->ngiqr_result = NGCLL_RESULT_FAILED;
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" option is invalid: %s.\n", 
                NGCLL_REI_NOTIFY_RESULT, emArg->ngea_value);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }
    } else
    /* Handling remote_executable_information option*/
    if (strcmp(emArg->ngea_name, NGCLL_REI_NOTIFY_INFO) == 0) {
        if (queryResult->ngiqr_info == NULL) {
            queryResult->ngiqr_info = ngiLineListConstruct(log, error);
            if (queryResult->ngiqr_info == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't construct the line list.\n");
                goto finalize;
            }
        }
        result = ngiLineListAppend(queryResult->ngiqr_info, emArg->ngea_value, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't append the string to the line list.\n");
            goto finalize;
        }
    } else
    /* Handling error_message option*/
    if (strcmp(emArg->ngea_name, NGCLL_REI_NOTIFY_MESSAGE) == 0) {
        if (queryResult->ngiqr_message != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "%s notify should not have more than one \"%s\" option.\n",
                NGCLL_INFORMATION_SERVICE_REMOTE_EXECUTABLE_INFORMATION_NOTIFY,
                NGCLL_REI_NOTIFY_MESSAGE);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }
        queryResult->ngiqr_message= ngiStrdup(emArg->ngea_value, log, error);
        if (queryResult->ngiqr_message == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't duplicate the string.\n");
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiExternalModuleArgumentDestruct(emArg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't duplicate the string.\n");
        error = NULL;
        ret = 0;
    }
    return ret;
}

/**
 * Information Service Query Result: Construct
 */
static ngcliInformationServiceQueryResult_t *
ngcllInformationServiceQueryResultConstruct(
    ngLog_t *log,
    int *error)
{
    int result;
    ngcliInformationServiceQueryResult_t *queryResult = NULL;
    static const char fName[] = "ngcllInformationServiceQueryResultConstruct";

    queryResult = NGI_ALLOCATE(
        ngcliInformationServiceQueryResult_t, log, error);
    if (queryResult == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Information Service"
            " Query Result.\n");
        goto error;
    }
    result = ngcllInformationServiceQueryResultInitialize(
        queryResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Information Service Query Result.\n");
        goto error;
    }

    return queryResult;
error:
    result = ngcllInformationServiceQueryResultDestruct(
        queryResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Information Service Query Result.\n");
    }
    return NULL;
}

/**
 * Information Service Query Result: Destruct
 */
static int
ngcllInformationServiceQueryResultDestruct(
    ngcliInformationServiceQueryResult_t *queryResult,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllInformationServiceQueryResultDestruct";

    if (queryResult == NULL) {
        return 1;
    }
    result = ngcllInformationServiceQueryResultFinalize(
        queryResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Information Service Query Result.\n");
        ret = 0;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngcliInformationServiceQueryResult_t, 
        queryResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Information Service"
            " Query Result.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Information Service Query Result: Initialize
 */
static int
ngcllInformationServiceQueryResultInitialize(
    ngcliInformationServiceQueryResult_t *queryResult,
    ngLog_t *log,
    int *error)
{
    ngcllInformationServiceQueryResultInitializeMember(queryResult);

    queryResult->ngiqr_id     = NULL;
    queryResult->ngiqr_result = NGCLL_RESULT_NOT_APPEARED;

    return 1;
}

/**
 * Information Service Query Result: Finalize
 */
static int
ngcllInformationServiceQueryResultFinalize(
    ngcliInformationServiceQueryResult_t *queryResult,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllInformationServiceQueryResultFinalize";

    assert(queryResult != NULL);

    if (queryResult->ngiqr_info != NULL) {
        result = ngiLineListDestruct(queryResult->ngiqr_info, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
                "Can't destruct the line list.\n");
            error = NULL;
            ret = 0;
        }
        queryResult->ngiqr_info      = NULL;
    }

    result = ngiFree(queryResult->ngiqr_message, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }

    result = ngiFree(queryResult->ngiqr_id, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }

    ngcllInformationServiceQueryResultInitializeMember(queryResult);

    return ret;
}

/**
 * Information Service Query Result: Zero clear
 */
static void
ngcllInformationServiceQueryResultInitializeMember(
    ngcliInformationServiceQueryResult_t *queryResult)
{
    assert(queryResult != NULL);

    queryResult->ngiqr_id        = 0;
    queryResult->ngiqr_result    = 0;
    queryResult->ngiqr_info      = NULL;
    queryResult->ngiqr_errorCode = 0;
    queryResult->ngiqr_message   = NULL;

    return;
}

/**
 * Query Result Waiter List: Push
 * Warning: The response responsibility of releasing "queryID"
 */
static int
ngcllQueryResultWaiterListPush(
    ngcllQueryResultWaiterList_t *list,
    ngcliInformationService_t *infoServ,
    ngcliInformationServiceQuerySession_t *session,
    ngLog_t *log,
    int *error)
{
    ngcllQueryResultWaiter_t *waiter = NULL;
    int result;
    static const char fName[] = "ngcllQueryResultWaiterListPush";

    assert(list != NULL);
    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(session != NULL);

    waiter = NGI_ALLOCATE(ngcllQueryResultWaiter_t, log, error);
    if (waiter == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Query Result Waiter.\n");
        goto error;
    }
    ngcllQueryResultWaiterInitializeMember(waiter);

    waiter->ngqrw_infoServ = infoServ;
    waiter->ngqrw_session  = session; 

    STAILQ_INSERT_TAIL(list, waiter, ngqrw_entry);

    return 1;
error:
    result = NGI_DEALLOCATE(ngcllQueryResultWaiter_t, waiter, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Query Result Waiter.\n");
    }
    return 0;
}

/**
 * Query Result Waiter List: Pop
 */
static int
ngcllQueryResultWaiterListPop(
    ngcllQueryResultWaiterList_t *list,
    ngLog_t *log,
    int *error)
{
    ngcllQueryResultWaiter_t *waiter = NULL;
    int result;
    int ret = 1;
    static const char fName[] = "ngcllQueryResultWaiterListPop";

    assert(list != NULL);

    if (STAILQ_EMPTY(list)) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "List is empty.\n");
        return 0;
    }
    waiter = STAILQ_FIRST(list);
    assert(waiter != NULL);
    STAILQ_REMOVE_HEAD(list, ngqrw_entry);

    ngcllQueryResultWaiterInitializeMember(waiter);

    result= NGI_DEALLOCATE(ngcllQueryResultWaiter_t, waiter, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Query Result Waiter.\n");
        ret = 0;
        error = NULL;
    }
    return ret;
}

/**
 * Query Result Waiter: Zero clear
 */
static void
ngcllQueryResultWaiterInitializeMember(
    ngcllQueryResultWaiter_t *waiter)
{
    assert(waiter != NULL);

    waiter->ngqrw_infoServ = NULL;
    waiter->ngqrw_session  = NULL;

    return;
}


/**
 * Query Result Waiters: Find successful query
 */
static int
ngcllQueryResultWaitersFindSuccessfulQuery(
    ngcllQueryResultWaiterList_t *waiters,
    int *found,
    ngLog_t *log,
    int *error)
{
    ngcllQueryResultWaiter_t *it = NULL;
    int result;
    int queryResult;
    static const char fName[] = "ngcllQueryResultWaitersFindSuccessfulQuery";

    assert(waiters != NULL);
    assert(found != NULL);

    *found = 0;

    STAILQ_FOREACH(it, waiters, ngqrw_entry) {
        if (NGCLL_INFORMATION_SERVICE_USABLE(it->ngqrw_infoServ)) {
            result = ngcllInformationServiceCheckQueryResult(
                it->ngqrw_infoServ, it->ngqrw_session, &queryResult, error);
            if (result == 0) {
                ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't query result.\n");
                /* Ignore error */
            }
            if (queryResult != 0) {
                *found = 1;
                break;
            }
        }
    }

    return 1;
}

static int
ngcllInformationServiceQueryAndPush(
    ngcliInformationService_t *infoServ, 
    char *hostName,
    char *className,
    ngcllQueryResultWaiterList_t *waiters,
    int *error)
{
    char *id = NULL;
    ngclContext_t *context = NULL;
    int result;
    ngLog_t *log;
    ngcliInformationServiceQuerySession_t *session = NULL;
    static const char fName[] = "ngcllInformationServiceQueryAndPush";

    NGCLL_INFORMATION_SERVICE_ASSERT(infoServ);
    assert(NGI_STRING_IS_VALID(hostName));
    assert(NGI_STRING_IS_VALID(className));
    assert(waiters != NULL);

    context = infoServ->ngis_manager->ngism_context;
    log = context->ngc_log;

    session = ngcllInformationServiceQuery(infoServ, hostName, className, error);
    if (session == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't query information to the Information Service"
            "(tag is \"%s\".\n", infoServ->ngis_tagName);
        goto error;
    } else {
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Waits %s.\n", infoServ->ngis_information.ngisi_tag);

        result = ngcllQueryResultWaiterListPush(
            waiters, infoServ, session, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't push query id to list.\n");
            goto error;
        }
    }
    return 1;
error:
    if (id != NULL) {
        /* Cancel */
        result = ngcllInformationServiceCancelQuery(infoServ, session, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't cancel query \"%s\".\n", id);
        }
    }
    return 0;
}


/**
 * Query Manager: Construct
 */
ngcliQueryManager_t *
ngcliQueryManagerConstruct(
    ngclContext_t *context,
    int *error)
{
    ngLog_t *log;
    ngcliQueryManager_t *qMng = NULL;
    int result;
    static const char fName[] = "ngcliQueryManagerConstruct";

    log = context->ngc_log;

    qMng = NGI_ALLOCATE(ngcliQueryManager_t, log, error);
    if (qMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for the Query Manager.\n");
        goto error;
    }
    result = ngcllQueryManagerInitialize(qMng, context, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Query Manager.\n");
        goto error;
    }

    return qMng;
error:
    result = ngcliQueryManagerDestruct(qMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Query Manager.\n");
        goto error;
    }
    return NULL;
}

/**
 * Query Manager: Destruct
 */
int
ngcliQueryManagerDestruct(
    ngcliQueryManager_t *qMng,
    int *error)
{
    ngLog_t *log;
    ngclContext_t *context = NULL;
    int result;
    int ret = 1;
    static const char fName[] = "ngcliQueryManagerDestruct";

    if (qMng == NULL) {
        /* do nothing */
        return 1;
    }

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngcllQueryManagerFinalize(qMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Query Manager.\n");
        ret = 1;
        error = NULL;
    }

    result = NGI_DEALLOCATE(ngcliQueryManager_t, qMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for the Query Manager.\n");
        ret = 1;
        error = NULL;
    }

    return ret;
}

/**
 * Query Manager: Initialize
 */
static int
ngcllQueryManagerInitialize(
    ngcliQueryManager_t *qMng,
    ngclContext_t *context,
    int *error)
{
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllQueryManagerInitialize";

    assert(qMng != NULL);
    assert(context != NULL);

    ngcllQueryManagerInitializeMember(qMng);

    qMng->ngqm_context = context;
    log = context->ngc_log;

    SLIST_INIT(&qMng->ngqm_queries);

    result = ngiRlockInitialize(&qMng->ngqm_rlock, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Rlock.\n");
        goto error;
    }

    qMng->ngqm_isMng = ngcliInformationServiceManagerConstruct(context, error);
    if (qMng->ngqm_isMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the Information Service Manager.\n");
        goto error;
    }

    return 1;
error:
    result = ngcllQueryManagerFinalize(qMng, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Query Manager.\n");
    }
    return 0;
}

/**
 * Query Manager: Finalize
 */
static int
ngcllQueryManagerFinalize(
    ngcliQueryManager_t *qMng,
    int *error)
{
    ngclContext_t *context;
    ngLog_t *log;
    int result;
    int ret = 1;
    static const char fName[] = "ngcllQueryManagerFinalize";

    assert(qMng != NULL);
    assert(SLIST_EMPTY(&qMng->ngqm_queries));

    context = qMng->ngqm_context;
    log = context->ngc_log;

    if (qMng->ngqm_isMng != NULL) {
        result = ngcliInformationServiceManagerDestruct(qMng->ngqm_isMng, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destruct the Information Service Manager.\n");
            error = NULL;
            ret = 0;
        }
    }

    result = ngiRlockFinalize(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the Rlock.\n");
        error = NULL;
        ret = 0;
    }

    ngcllQueryManagerInitializeMember(qMng);

    return ret;
}

/**
 * Query Manager: Zero clear
 */
static void
ngcllQueryManagerInitializeMember(
    ngcliQueryManager_t *qMng)
{
    qMng->ngqm_context   = NULL;
    qMng->ngqm_isMng     = NULL;
    qMng->ngqm_rlock     = NGI_RLOCK_NULL;
    qMng->ngqm_rereading = 0; /* false */
    qMng->ngqm_nQueries  = 0;
    SLIST_INIT(&qMng->ngqm_queries);

    return;
}

/**
 * Query Manager: Query
 */
int
ngcliQueryManagerQuery(
    ngcliQueryManager_t *qMng,
    char *hostName,
    char *className,
    char *tag,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngclContext_t *context;
    ngLog_t *log;
    int result;
    int exist;
    ngcliInformationServiceQuery_t query;
    int ret = 0;
    int locked = 0;
    int inserted = 0;
    int inc = 0;
    static const char fName[] = "ngcliQueryManagerQuery";

    assert(qMng != NULL);

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    while (qMng->ngqm_rereading != 0) {
        result = ngiRlockWait(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait the Query Manager.\n");
            goto finalize;
        }
    }
    qMng->ngqm_nQueries++;
    inc = 1;

    exist = ngcllQueryManagerExistQuery(qMng, hostName, className, error);

    query.ngisq_hostName  = hostName;
    query.ngisq_className = className;
    query.ngisq_epInfo    = epInfo;
    query.ngisq_rcInfo    = rcInfo;
    query.ngisq_done      = 0; /* false */
    query.ngisq_result    = 0; /* failed */
    SLIST_INSERT_HEAD(&qMng->ngqm_queries, &query, ngisq_entry);
    inserted = 1;

    assert(locked != 0);
    result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
    locked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Query Manager.\n");
        goto finalize;
    }

    if (exist == 0) {
        result = ngcllQueryManagerDoQuery(qMng, hostName, className, tag, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't query the information.\n");
            goto finalize;
        }

    } else {
        result = ngcllQueryManagerWait(qMng, hostName, className, &query, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait the query.\n");
            goto finalize;
        }
    }

    assert(query.ngisq_done != 0);
    if (query.ngisq_result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't get information.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (locked == 0) {
        result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't lock the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
        locked = 1;
    }
    if (inc != 0) {
        assert(qMng->ngqm_nQueries > 0);
        qMng->ngqm_nQueries--;
        inc = 0;
        if (qMng->ngqm_nQueries == 0) {
            result = ngiRlockBroadcast(&qMng->ngqm_rlock, log, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't broadcast the Query Manager.\n");
                ret = 0;
                error = NULL;
            }
        }
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast the signal for the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}

int
ngcliQueryManagerLockForReread(
    ngcliQueryManager_t *qMng,
    int *error)
{
    int ret = 0;
    int result;
    int locked = 0;
    ngLog_t *log;
    ngclContext_t *context;
    static const char fName[] = "ngcliQueryManagerLockForReread";

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    if (qMng->ngqm_rereading != 0) {
        goto finalize;
    }

    qMng->ngqm_rereading = 1;

    while (qMng->ngqm_nQueries > 0) {
        result = ngiRlockWait(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait the Query Manager.\n");
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast the signal for the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}

int
ngcliQueryManagerUnlockForReread(
    ngcliQueryManager_t *qMng,
    int *error)
{
    int ret = 0;
    int result;
    int locked = 0;
    ngLog_t *log;
    ngclContext_t *context;
    static const char fName[] = "ngcliQueryManagerUnlockForReread";

    assert(qMng != NULL);

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    if (qMng->ngqm_rereading == 0) {
        goto finalize;
    }

    qMng->ngqm_rereading = 0;

    result = ngiRlockBroadcast(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't broadcast signal for the Query Manager.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast the signal for the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}

int
ngcliQueryManagerReconstruct(
    ngcliQueryManager_t *qMng,
    int *error)
{
    int ret = 0;
    int result;
    int locked = 0;
    ngLog_t *log;
    ngclContext_t *context;
    ngcliInformationServiceManager_t *isMng = NULL;
    ngcliInformationServiceManager_t *oldIsMng = NULL;
    static const char fName[] = "ngcliQueryManagerReconstruct";

    assert(qMng != NULL);

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    if (qMng->ngqm_rereading == 0) {
        goto finalize;
    }

    isMng = ngcliInformationServiceManagerConstruct(qMng->ngqm_context, error);
    if (isMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct a new Information Service Manager.\n");
        goto finalize;
    }
    oldIsMng = qMng->ngqm_isMng;
    qMng->ngqm_isMng = isMng;

    result = ngcliInformationServiceManagerDestruct(oldIsMng, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the old Information Service Manager.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't broadcast the signal for the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}


/**
 * Query Manager: exists query?
 */
static int
ngcllQueryManagerExistQuery(
    ngcliQueryManager_t *qMng,
    char *hostName,
    char *className,
    int *error)
{
    ngcliInformationServiceQuery_t *it;

    assert(qMng != NULL);
    assert(NGI_STRING_IS_VALID(hostName));
    assert(NGI_STRING_IS_VALID(className));

    SLIST_FOREACH(it, &qMng->ngqm_queries, ngisq_entry) {
        if ((strcmp(it->ngisq_hostName, hostName) == 0) &&
            (strcmp(it->ngisq_className, className) == 0)) {
            return 1;
        }
    }

    /* not found */
    return 0;
}

/**
 * Query Manager: Do query
 */
static int
ngcllQueryManagerDoQuery(
    ngcliQueryManager_t *qMng,
    char *hostName,
    char *className,
    char *tag,
    int *error)
{
    ngclContext_t *context;
    int epInfoInitialized = 0;
    int rcInfoInitialized = 0;
    ngclExecutablePathInformation_t epInfo;
    ngRemoteClassInformation_t rcInfo;
    int result;
    ngLog_t *log;
    int ret = 0;
    static const char fName[] = "ngcllQueryManagerDoQuery";

    assert(qMng != NULL);
    assert(NGI_STRING_IS_VALID(hostName));
    assert(NGI_STRING_IS_VALID(className));

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngcliExecutablePathInformationInitialize(context, &epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Executable Path Information.\n");
        goto finalize;
    }
    epInfoInitialized = 1;

    result = ngcliRemoteClassInformationInitialize(context, &rcInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the Remote Class Information.\n");
        goto finalize;
    }
    rcInfoInitialized = 1;

    result = ngcliInformationServiceManagerQuery(
        qMng->ngqm_isMng, hostName, className, tag, &epInfo, &rcInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't query the information.\n");
        goto finalize;
    }

    result = ngcliExecutablePathInformationCacheRegister(context, &epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Executable Path Information.\n");
        goto finalize;
    }

    result = ngcliRemoteClassInformationCacheRegister(context, &rcInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't register the Remote Class Information.\n");
        goto finalize;
    }

    ret = 1;

finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngcllQueryManagerSetInformation(qMng, hostName, className, ret, &epInfo, &rcInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't set information to each query.\n");
        ret = 0;
        error = NULL;
    }

    if (epInfoInitialized != 0) {
        result = ngclExecutablePathInformationRelease(context, &epInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't register the Executable Path Information.\n");
            ret = 0;
            error = NULL;
        }
    }

    if (rcInfoInitialized != 0) {
        result = ngclRemoteClassInformationRelease(context, &rcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't register the Remote Class Information.\n");
            ret = 0;
            error = NULL;

        }
    }
    return ret;
}

static int
ngcllQueryManagerSetInformation(
    ngcliQueryManager_t *qMng,
    char *hostName,
    char *className,
    int queryResult,
    ngclExecutablePathInformation_t *epInfo,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngclContext_t *context;
    ngLog_t *log;
    int ret = 0;
    int result;
    int locked = 0;
    ngcliInformationServiceQuery_t *it;
    ngcliInformationServiceQuery_t *tvar;
    static const char fName[] = "ngcllQueryManagerSetInformation";

    assert(qMng != NULL);
    assert(NGI_STRING_IS_VALID(hostName));
    assert(NGI_STRING_IS_VALID(className));

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    SLIST_FOREACH_SAFE(it, &qMng->ngqm_queries, ngisq_entry, tvar) {
        if ((strcmp(it->ngisq_hostName, hostName) == 0) &&
            (strcmp(it->ngisq_className, className) == 0)) {
            SLIST_REMOVE(&qMng->ngqm_queries, it,
                ngcliInformationServiceQuery_s, ngisq_entry);
            it->ngisq_done = 1;
            it->ngisq_result = 0;/* Failed */
            if (queryResult == 0) {
                continue;
            }
            if (it->ngisq_epInfo != NULL) {
                result = ngcliExecutablePathInformationCopy(context, epInfo, it->ngisq_epInfo, error);
                if (result == 0) {
                    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't copy the executable path information.\n");
                    continue;
                }
            }

            if (it->ngisq_rcInfo != NULL) {
                result = ngcliRemoteClassInformationCopy(context, rcInfo, it->ngisq_rcInfo, error);
                if (result == 0) {
                    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't copy the remote class information.\n");
                    continue;
                }
            }
            it->ngisq_result = 1;/* Successful */
        }
    }

    result = ngiRlockBroadcast(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't broadcast signal for the Query Manager.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = 0;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        locked = 0;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}

static int
ngcllQueryManagerWait(
    ngcliQueryManager_t *qMng,
    char *hostName,
    char *className,
    ngcliInformationServiceQuery_t *query,
    int *error)
{
    ngclContext_t *context;
    ngLog_t *log;
    int ret = 0;
    int result;
    int locked = 0;
    static const char fName[] = "ngcllQueryManagerWait";

    context = qMng->ngqm_context;
    log = context->ngc_log;

    result = ngiRlockLock(&qMng->ngqm_rlock, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Query Manager.\n");
        goto finalize;
    }
    locked = 1;

    while (query->ngisq_done == 0) {
        result = ngiRlockWait(&qMng->ngqm_rlock, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't wait signal for the Query Manager.\n");
            goto finalize;
        }
    }


    ret = 1;
finalize:
    if (ret == 0) {
        error = 0;
    }

    if (locked != 0) {
        result = ngiRlockUnlock(&qMng->ngqm_rlock, log, error);
        locked = 0;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock the Query Manager.\n");
            ret = 0;
            error = NULL;
        }
    }
    return ret;
}
