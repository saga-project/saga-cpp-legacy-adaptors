#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclInvokeServer.c,v $ $Revision: 1.31 $ $Date: 2007/12/26 12:27:17 $";
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
 * Module of Invoke Server for Ninf-G Client.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#ifdef linux
#include <sys/poll.h>
#else /* linux */
#include <poll.h>
#endif /* linux */

#include "ng.h"

/**
 * Data type
 */

#ifdef NG_PTHREAD
typedef enum ngcllInvokeServerStatus_e {
    NGI_INVOKE_SERVER_STATUS_UNDEFINED, /* for dummy */
    NGI_INVOKE_SERVER_STATUS_PENDING,
    NGI_INVOKE_SERVER_STATUS_ACTIVE,
    NGI_INVOKE_SERVER_STATUS_DONE,
    NGI_INVOKE_SERVER_STATUS_FAILED,
    NGI_INVOKE_SERVER_STATUS_NOMORE     /* for dummy */
} ngcllInvokeServerStatus_t;

typedef struct ngcllInvokeServerStatusTable_s {
    int				ngisst_valid;
    ngcllInvokeServerStatus_t	ngisst_status;
    char			*ngisst_name;
} ngcllInvokeServerStatusTable_t;
#endif /* NG_PTHREAD */

/**
 * Prototype declaration of static functions.
 */
static ngcliInvokeServerInformationManager_t *
    ngcllInvokeServerInformationManagerAllocate(ngclContext_t *, int *);
static int
ngcllInvokeServerInformationManagerFree(
    ngclContext_t *,
    ngcliInvokeServerInformationManager_t *,
    int *);
static ngcliInvokeServerInformationManager_t *
ngcllInvokeServerInformationConstruct(
    ngclContext_t *,
    ngclInvokeServerInformation_t *,
    int *);
static int
ngcllInvokeServerInformationDestruct(
    ngclContext_t *,
    ngcliInvokeServerInformationManager_t *,
    int *);
static int
ngcllInvokeServerInformationManagerInitialize(
     ngclContext_t *,
     ngcliInvokeServerInformationManager_t *,
     ngclInvokeServerInformation_t *,
     int *);
static int
ngcllInvokeServerInformationManagerFinalize(
    ngclContext_t *,
    ngcliInvokeServerInformationManager_t *,
    int *);
static void
ngcllInvokeServerInformationInitializeMember(
    ngclInvokeServerInformation_t *);
static void
ngcllInvokeServerInformationInitializePointer(
    ngclInvokeServerInformation_t *isInfo);
static int ngcllInvokeServerInformationGetCopy(
    ngclContext_t *, char *, ngclInvokeServerInformation_t *, int *);
static int ngcllInvokeServerInformationRelease(
    ngclContext_t *, ngclInvokeServerInformation_t *, int *);
static int ngcllInvokeServerInformationReplace(
    ngclContext_t *, ngcliInvokeServerInformationManager_t *,
    ngclInvokeServerInformation_t *, int *);

#ifdef NG_PTHREAD
static ngcliInvokeServerManager_t *ngcllInvokeServerAllocate(
    ngclContext_t *, int *);
static int ngcllInvokeServerFree(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerInitialize(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *,
    ngcliJobManager_t *, int *);
static int ngcllInvokeServerFinalize(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerRegister(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerUnregister(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static void ngcllInvokeServerInitializeMember(ngcliInvokeServerManager_t *);
static void ngcllInvokeServerInitializePointer(ngcliInvokeServerManager_t *);
static void ngcllInvokeServerReaderInitializeMember(
    ngcliInvokeServerReader_t *);
static int ngcllInvokeServerInitializeMutexAndCond(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerFinalizeMutexAndCond(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static void ngcllInvokeServerRequestReplyInitializeMember(
    ngcliInvokeServerRequestReply_t *);
static void ngcllInvokeServerRequestReplyClear(
    ngcliInvokeServerRequestReply_t *);
#endif /* NG_PTHREAD */

static void ngcllInvokeServerJobInitializeMember(ngcliInvokeServerJob_t *);

#ifdef NG_PTHREAD
static int ngcllInvokeServerTreatRetired(
    ngclContext_t *, int *);
static int ngcllInvokeServerFind(
    ngclContext_t *, char *, ngcliInvokeServerManager_t **, int *);
static int ngcllInvokeServerCountUp(
    ngclContext_t *, char *, int *, int *);
static ngcliInvokeServerCount_t *ngcllInvokeServerCountConstruct(
    ngclContext_t *, char *, int *);
static int ngcllInvokeServerCountDestruct(
    ngclContext_t *, ngcliInvokeServerCount_t *, int *);
static ngcliInvokeServerCount_t *ngcllInvokeServerCountAllocate(
    ngclContext_t *, int *);
static int ngcllInvokeServerCountFree(
    ngclContext_t *, ngcliInvokeServerCount_t *, int *);
static int ngcllInvokeServerCountInitialize(
    ngclContext_t *, char *, ngcliInvokeServerCount_t *, int *);
static int ngcllInvokeServerCountFinalize(
    ngclContext_t *, ngcliInvokeServerCount_t *, int *);
static void ngcllInvokeServerCountInitializeMember(
    ngcliInvokeServerCount_t *);
static int ngcllInvokeServerCountRegister(
    ngclContext_t *, ngcliInvokeServerCount_t *, int *);
static int ngcllInvokeServerCountUnregister(
    ngclContext_t *, ngcliInvokeServerCount_t *, int *);

static int ngcllInvokeServerProcessInvoke(ngclContext_t *,
    ngcliInvokeServerManager_t *, ngcliJobManager_t *, int *);
static void ngcllInvokeServerProcessInvokeChild(
    char *, char *[], int *, int *, int *);
static void ngcllInvokeServerProcessInvokeChildChild(
    char *, char *[], int *, int *, int *);
static int ngcllInvokeServerProgramNameGet(
    ngclContext_t *, char *, size_t, char *, int *);

static int ngcllInvokeServerReplyReaderThreadStart(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerNotifyReaderThreadStart(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerReplyReaderThreadStop(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerNotifyReaderThreadStop(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static void * ngcllInvokeServerReplyReaderThread(void *);
static void * ngcllInvokeServerNotifyReaderThread(void *);
static int ngcllInvokeServerReplyProcess(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliInvokeServerReadBuffer_t *, int *);
static int ngcllInvokeServerReplyProcessJobStatus(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *, int *);
static int ngcllInvokeServerNotifyProcess(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliInvokeServerReadBuffer_t *, int *);
static int ngcllInvokeServerNotifyProcessCreateNotify(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *, char *, int *);
static int ngcllInvokeServerNotifyProcessStatusNotify(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *, char *, int *);
static int ngcllInvokeServerNotifyJobStatusChange(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *,
    ngcllInvokeServerStatus_t, char *, char *, int *);
static int ngcllInvokeServerUnusable(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);

static int ngcllInvokeServerReadBufferInitialize(
    ngclContext_t *, ngcliInvokeServerReadBuffer_t *, int *);
static int ngcllInvokeServerReadBufferFinalize(
    ngclContext_t *, ngcliInvokeServerReadBuffer_t *, int *);
static void ngcllInvokeServerReadBufferInitializeMember(
    ngcliInvokeServerReadBuffer_t *);
static int ngcllInvokeServerReadLine(ngclContext_t *,
    ngcliInvokeServerManager_t *, ngcliInvokeServerReader_t *, int *);

static int ngcllInvokeServerRequestLock(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerRequestUnlock(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerReplySet(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerReplyWait(
    ngclContext_t *, ngcliInvokeServerManager_t *, int *);
static int ngcllInvokeServerJobIDset(
    ngclContext_t *, ngcliInvokeServerManager_t *,ngcliJobManager_t *, int *);
static int ngcllInvokeServerJobIDwait(
    ngclContext_t *, ngcliInvokeServerManager_t *,ngcliJobManager_t *, int *);

static int ngcllInvokeServerRequest(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliInvokeServerRequestType_t, ngcliJobManager_t *, int *);
static int ngcllInvokeServerRequestJobCreateArgument(
    ngclContext_t *, FILE *, ngcliJobManager_t *, int *);

static int ngcllInvokeServerReplyJobCreateTreat(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, char *, int *);
static int ngcllInvokeServerReplyJobStatusTreat(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, char *, int *);
static int ngcllInvokeServerReplyJobDestroyTreat(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, char *, int *);
static int ngcllInvokeServerReplyExitTreat(
    ngclContext_t *, ngcliInvokeServerManager_t *, char *, int *);

static int ngcllInvokeServerJobStdoutStderrCreate(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, int *);
static int ngcllInvokeServerJobStdoutStderrDestroy(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, int *);
static int ngcllInvokeServerJobStdoutStderrOutput(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, int *);
static int ngcllInvokeServerJobStdoutStderrOutputSub(
    ngclContext_t *, ngcliInvokeServerManager_t *,
    ngcliJobManager_t *, char *, FILE *, char *, int *);
#endif /* NG_PTHREAD */

/**
 * Data
 */
#ifdef NG_PTHREAD
static const ngcllInvokeServerStatusTable_t ngcllInvokeServerStatusTable[] = {
    {1, NGI_INVOKE_SERVER_STATUS_PENDING, "PENDING"},
    {1, NGI_INVOKE_SERVER_STATUS_ACTIVE,  "ACTIVE"},
    {1, NGI_INVOKE_SERVER_STATUS_DONE,    "DONE"},
    {1, NGI_INVOKE_SERVER_STATUS_FAILED,  "FAILED"},
    {0, NGI_INVOKE_SERVER_STATUS_NOMORE,  NULL},
};
#endif /* NG_PTHREAD */


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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((isInfo == NULL) ||
        (isInfo->ngisi_type == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliInvokeServerInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock InvokeServerInformation list.\n", fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the Invoke Server Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Invoke Server Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (isInfoMng == NULL) {
        isInfoMng = ngcllInvokeServerInformationConstruct(
            context, isInfo, error);
        if (isInfoMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct Invoke Server Information.\n", fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Invoke Server Information list.\n",
                fName);
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
	ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
	    "%s: Ninf-G Context Invalid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliInvokeServerInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock InvokeServerInformation list.\n", fName);
	return 0;
    }

    if (typeName == NULL) {
	/* Delete all information */

	/* Get the data from the head of a list */
        curr = ngcliInvokeServerInformationCacheGetNext(context, NULL, error);
	if (curr == NULL) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
		"%s: No Invoke Server Information was registered.\n", fName);
	}

        while (curr != NULL) {
	    /* Destruct the data */
            result = ngcllInvokeServerInformationDestruct(context, curr, error);
	    if (result == 0) {
	        ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't destruct Invoke Server Information.\n", fName);
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
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Invoke Server Information \"%s\" is not found.\n",
		fName, typeName);
	    goto error;
	}

	/* Destruct the data */
        result = ngcllInvokeServerInformationDestruct(context, curr, error);
	if (result == 0) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't destruct Invoke Server Information.\n", fName);
	    goto error;
	}
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't write unlock the list of Invoke Server Information.\n",
	    fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't write unlock the list of Invoke Server Information.\n",
	    fName);
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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (typeName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: hostname is NULL.\n", fName);
	return NULL;
    }

    isInfoMng = context->ngc_isInfo_head;
    for (; isInfoMng != NULL; isInfoMng = isInfoMng->ngisim_next) {
	assert(isInfoMng->ngisim_info.ngisi_type != NULL);
	if (strcmp(isInfoMng->ngisim_info.ngisi_type, typeName) == 0) {
	    /* Found */
	    return isInfoMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Invoke Server Information is not found by name \"%s\".\n",
	fName, typeName);
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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    if (current == NULL) {
	/* Return the first information */
	if (context->ngc_isInfo_head != NULL) {
	    assert(context->ngc_isInfo_tail != NULL);
            return context->ngc_isInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngisim_next != NULL) {
	    return current->ngisim_next;
	}
    }

    /* Not found */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Invoke Server Information was reached.\n", fName);
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
    isInfoMng = ngcllInvokeServerInformationManagerAllocate(context, error);
    if (isInfoMng == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Invoke Server Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllInvokeServerInformationManagerInitialize(
        context, isInfoMng, isInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Invoke Server Information.\n",
	    fName);
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterInvokeServerInformation(
    	context, isInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Invoke Server Information"
            " for Ninf-G Context.\n", fName);
	goto error;
    }

    /* Success */
    return isInfoMng;

    /* Error occurred */
error:
    result = ngcllInvokeServerInformationDestruct(context, isInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't free the storage for Invoke Server Information Manager.\n",
	    fName);
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
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Invoke Server Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllInvokeServerInformationManagerFinalize(context, isInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Invoke Server Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllInvokeServerInformationManagerFree(context, isInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Invoke Server Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage.
 */
static ngcliInvokeServerInformationManager_t *
ngcllInvokeServerInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngcliInvokeServerInformationManager_t *isInfoMng;
    static const char fName[] = "ngcllInvokeServerInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    isInfoMng = globus_libc_calloc(1,
	sizeof (ngcliInvokeServerInformationManager_t));
    if (isInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Invoke Server Information Manager.\n",
	    fName);
	return NULL;
    }

    return isInfoMng;
}

/**
 * Free the information storage.
 */
static int
ngcllInvokeServerInformationManagerFree(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    globus_libc_free(isInfoMng);

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
    ngclInvokeServerInformation_t *isInfo;
    static const char fName[] = "ngcliInvokeServerInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    isInfo = globus_libc_calloc(1, sizeof (ngclInvokeServerInformation_t));
    if (isInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Invoke Server Information.\n",
	    fName);
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
    static const char fName[] = "ngcliInvokeServerInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(isInfo);

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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't copy the Invoke Server Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&isInfoMng->ngisim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for own instance.\n", fName);
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
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclInvokeServerInformationRelease(context, 
	&isInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Invoke Server Information.\n", fName);
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
    int i, result;
    static const char fName[] = "ngcliInvokeServerInformationCopy";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    ngcllInvokeServerInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllInvokeServerInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_ALLOCATE(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = strdup((src)->member); \
        if ((dest)->member == NULL) { \
            NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
            ngclLogPrintfContext(context, \
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, \
                "%s: Can't allocate the storage " \
                "for Invoke Server Information.\n", fName); \
            goto error; \
        } \
    } while(0)

    NGL_ALLOCATE(src, dest, ngisi_type);
    if (src->ngisi_path != NULL) {
        NGL_ALLOCATE(src, dest, ngisi_path);
    }

    if (src->ngisi_logFilePath != NULL) {
        NGL_ALLOCATE(src, dest, ngisi_logFilePath);
    }

    /* Copy Options */
    if (src->ngisi_nOptions > 0) {
        dest->ngisi_options = globus_libc_calloc(
            src->ngisi_nOptions, sizeof(char *));
        if (dest->ngisi_options == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for string table.\n",
                fName);
            return 0;
        }
        /* copy all of elements */
        for (i = 0; i < src->ngisi_nOptions; i++) {
            NGL_ALLOCATE(src, dest, ngisi_options[i]);
        }
    }

#undef NGL_ALLOCATE

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclInvokeServerInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Invoke Server Information.\n", fName);
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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
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
    int i;

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Deallocate the members */
    if (isInfo->ngisi_type != NULL)
        globus_libc_free(isInfo->ngisi_type);
    if (isInfo->ngisi_path != NULL)
        globus_libc_free(isInfo->ngisi_path);
    if (isInfo->ngisi_logFilePath != NULL)
        globus_libc_free(isInfo->ngisi_logFilePath);
    if (isInfo->ngisi_options != NULL) {
        for (i = 0; i < isInfo->ngisi_nOptions; i++) {
            if (isInfo->ngisi_options[i] != NULL) {
                globus_libc_free(isInfo->ngisi_options[i]);
            }
        }
        globus_libc_free(isInfo->ngisi_options);
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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (isInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
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
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

   /* Lock the Invoke Server Information */
    result = ngcliInvokeServerInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Invoke Server Information.\n", fName);
        return 0;
    }

    /* Get the Invoke Server Information */
    isInfoMng = ngcliInvokeServerInformationCacheGet(context, typeName, error);
    if (isInfoMng == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server Information.\n", fName);
        goto error;
    }

    /* Copy the Invoke Server Information */
    result = ngcliInvokeServerInformationCopy(context,
                    &isInfoMng->ngisim_info, isInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't copy the Invoke Server Information.\n", fName);
        goto error;
    }

    /* Unlock the Invoke Server Information */
    result = ngcliInvokeServerInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Invoke Server Information.\n",
            fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Invoke Server Information.\n",
            fName);
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
    int result, mdsLocked;
    static const char fName[] = "ngcllInvokeServerInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstIsInfoMng != NULL);
    assert(srcIsInfo != NULL);

    log = context->ngc_log;
    mdsLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the Invoke Server Information for \"%s\".\n",
        fName, srcIsInfo->ngisi_type);

    /* Lock */
    result = ngcliInvokeServerInformationWriteLock(
        dstIsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Invoke Server Information.\n",
            fName);
        goto error;
    }
    mdsLocked = 1;

    /* Release the Invoke Server Information */
    result = ngcllInvokeServerInformationRelease(
        context, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Invoke Server Information.\n",
            fName);
        goto error;
    }

    /* Copy the Invoke Server Information */
    result = ngcliInvokeServerInformationCopy(
        context, srcIsInfo, &dstIsInfoMng->ngisim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Invoke Server Information.\n",
            fName);
        goto error;
    }

    /* Unlock */
    result = ngcliInvokeServerInformationWriteUnlock(
        dstIsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Invoke Server Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (mdsLocked != 0) {
        result = ngcliInvokeServerInformationWriteUnlock(
            dstIsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Invoke Server Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;
}

#ifdef NG_PTHREAD
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
    int result, locked, allocated, initialized;
    ngcliInvokeServerManager_t *invokeMng;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeServerType != NULL);
    assert(jobMng != NULL);

    locked = 0;
    allocated = 0;
    initialized = 0;
    log = context->ngc_log;
    invokeMng = NULL;

    /* Lock the list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Invoke Server.\n", fName);
        goto error;
    }
    locked = 1;

    /* Find the Invoke Server */
    result = ngcllInvokeServerFind(
        context, invokeServerType, &invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to find the Invoke Server %s.\n",
            fName, invokeServerType);
        goto error;
    }

    if (invokeMng != NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) available.\n",
            fName, invokeServerType, invokeMng->ngism_typeCount);
        
    } else {

        /* Allocate */
        invokeMng = ngcllInvokeServerAllocate(context, error);
        if (invokeMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Invoke Server.\n", fName);
            goto error;
        }
        allocated = 1;
     
        /* Initialize */
        result = ngcllInvokeServerInitialize(
            context, invokeMng, invokeServerType, jobMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Invoke Server %s.\n",
                fName, invokeServerType);
            goto error;
        }
        initialized = 1;
     
        /* Register */
        result = ngcllInvokeServerRegister(
            context, invokeMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the Invoke Server %s(%d).\n",
                fName, invokeServerType, invokeMng->ngism_typeCount);
            goto error;
        }
     
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Invoke Server %s(%d) created.\n",
            fName, invokeServerType, invokeMng->ngism_typeCount);
    }

    /* Count up nJobs */
    invokeMng->ngism_nJobsStart++; 

    /* Unlock the list */
    locked = 0;
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Invoke Server.\n", fName);
        goto error;
    }

    /* Success */
    return invokeMng;

    /* Error occurred */
error:
    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't construct the Invoke Server %s.\n",
        fName, invokeServerType);

    /* Finalize */
    if ((invokeMng != NULL) && (initialized != 0)) {
        initialized = 0;
        result = ngcllInvokeServerFinalize(context, invokeMng, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Invoke Server.\n", fName);
        }
    }

    /* Deallocate */
    if ((invokeMng != NULL) && (allocated != 0)) {
        allocated = 0;
        result = ngcllInvokeServerFree(context, invokeMng, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the Invoke Server.\n", fName);
        }
        invokeMng = NULL;
    }

    /* Unlock the list */
    if (locked != 0) {
        locked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Invoke Server.\n", fName);
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
    ngcliInvokeServerManager_t *curInvokeMng;
    int result, locked, destructAll;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);

    locked = 0;
    destructAll = 0;
    log = context->ngc_log;

    if (invokeMng == NULL) {
        destructAll = 1;
    }

    /* Lock the list */
    if (requireLock != 0) {
        result = ngcliContextInvokeServerManagerListWriteLock(
            context, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Invoke Server.\n", fName);
            goto error;
        }
        locked = 1;
    }

    if (destructAll != 0) {
        curInvokeMng = context->ngc_invokeMng_head;
    } else {
        curInvokeMng = invokeMng;
    }
    
    while (curInvokeMng != NULL) {

        /* log */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Destructing the Invoke Server %s(%d).\n",
            fName, curInvokeMng->ngism_serverType,
            curInvokeMng->ngism_typeCount);

        /* Unregister */
        result = ngcllInvokeServerUnregister(context, curInvokeMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unregister the Invoke Server.\n", fName);
            goto error;
        }
     
        /* Finalize */
        result = ngcllInvokeServerFinalize(context, curInvokeMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Invoke Server.\n", fName);
            goto error;
        }
     
        /* Deallocate */
        result = ngcllInvokeServerFree(context, curInvokeMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the Invoke Server.\n", fName);
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

    if (destructAll != 0) {
        result = ngcllInvokeServerCountDestruct(context, NULL, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Invoke Server Counts.\n", fName);
            goto error;
        }
    }

    /* Unlock the list */
    if (locked != 0) {
        locked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Invoke Server.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't destruct the Invoke Server.\n", fName);

    /* Unlock the list */
    if (locked != 0) {
        locked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Invoke Server.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

#else /* NG_PTHREAD */

/**
 * Construct (Error on NonThread version)
 */
ngcliInvokeServerManager_t *
ngcliInvokeServerConstruct(
    ngclContext_t *context,
    char *invokeServerType,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerConstruct";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Destruct (Do nothing on NonThread version)
 */
int
ngcliInvokeServerDestruct(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int requireLock,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerDestruct";

    /* Check the arguments */
    assert(context != NULL);

    if (invokeMng != NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server is not supported "
            "for this GlobusToolkit flavor.\n",
            fName);

        return 0;
    }

    /* Do nothing */

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
/**
 * Allocate
 */
static ngcliInvokeServerManager_t *
ngcllInvokeServerAllocate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerAllocate";
    ngcliInvokeServerManager_t *invokeMng;

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate the Invoke Server Manager */
    invokeMng = globus_libc_calloc(
        1, sizeof(ngcliInvokeServerManager_t));
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Invoke Server Manager.\n",
            fName);
        return NULL;
    }

    /* Success */
    return invokeMng;
}

/**
 * Deallocate
 */
static int
ngcllInvokeServerFree(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    globus_libc_free(invokeMng);

    /* Success */
    return 1;
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
    int result, typeCount, maxJobs;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(invokeServerType != NULL);
    assert(jobMng != NULL);

    maxJobs = 0;
    typeCount = 0;

    ngcllInvokeServerInitializeMember(invokeMng);

    /* Initialize the Read Buffer for Reply Reader */
    result = ngcllInvokeServerReadBufferInitialize(
        context, &invokeMng->ngism_replyReader.ngisr_readBuffer, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initilaize the Read Buffer for Reply Reader.\n",
            fName);
        return 0;
    }

    /* Initialize the Read Buffer for Notify Reader */
    result = ngcllInvokeServerReadBufferInitialize(
        context, &invokeMng->ngism_notifyReader.ngisr_readBuffer, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initilaize the Read Buffer for Notify Reader.\n",
            fName);
        return 0;
    }

    /* Initialize the Mutex and Cond */
    result = ngcllInvokeServerInitializeMutexAndCond(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initilaize Mutex and Cond for "
            "Invoke Server Manager.\n", fName);
        return 0;
    }

    invokeMng->ngism_context = context;

    /* Invoke Server ID */
    invokeMng->ngism_ID = context->ngc_invokeServerID;
    context->ngc_invokeServerID++;

    /* Max Jobs for this Invoke Server */
    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    maxJobs = jobMng->ngjm_attr.ngja_isInfo.ngisi_maxJobs;

    /* Count Up the Number of Invoke Servers */
    result = ngcllInvokeServerCountUp(
        context, invokeServerType, &typeCount, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't count up the number of Invoke Server %s.\n",
            fName, invokeServerType);
        return 0;
    }

    invokeMng->ngism_typeCount = typeCount;
    invokeMng->ngism_nJobsMax = maxJobs;

    /* Copy the Invoke Server Type */
    invokeMng->ngism_serverType = strdup(invokeServerType);
    if (invokeMng->ngism_serverType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string.\n",
            fName);
        return 0;
    }

    /* Reset the counters */
    invokeMng->ngism_nJobsStart = 0;
    invokeMng->ngism_nJobsStop = 0;
    invokeMng->ngism_nJobsDone = 0;
    invokeMng->ngism_maxRequestID = 0;

    /* Set the state */
    invokeMng->ngism_working = 1;
    invokeMng->ngism_valid = 1;
    invokeMng->ngism_errorCode = NG_ERROR_NO_ERROR;

    /* Invoke the process */
    result = ngcllInvokeServerProcessInvoke(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't start the Invoke Server %s(%d) process.\n",
            fName, invokeServerType, typeCount);
        return 0;
    }

    /* Start the Reply Reader Thread */
    result = ngcllInvokeServerReplyReaderThreadStart(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't start the Invoke Server %s(%d) Reply Reader thread.\n",
            fName, invokeServerType, typeCount);
        return 0;
    }

    /* Start the Notify Reader Thread */
    result = ngcllInvokeServerNotifyReaderThreadStart(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't start the Invoke Server %s(%d) Notify Reader thread.\n",
            fName, invokeServerType, typeCount);
        return 0;
    }

    /* Convert the Request stream */
    invokeMng->ngism_requestFp = fdopen(invokeMng->ngism_requestFd, "w");
    if (invokeMng->ngism_requestFp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't fdopen(%d) for Request file descriptor: %s.\n",
            fName, invokeMng->ngism_requestFd, strerror(errno));
        return 0;
    }
    setvbuf(invokeMng->ngism_requestFp, NULL, _IONBF, 0);

    /* Success */
    return 1;
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

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    assert(invokeMng->ngism_serverType != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    returnCode = 1;

    /* Finalizing the Invoke Server */
    invokeMng->ngism_working = 0;

    /* log */
    if (invokeMng->ngism_valid == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) was dead abnormally.\n",
            fName, serverType, typeCount);
    }

    /* Request the EXIT and wait Reply */
    if (invokeMng->ngism_valid != 0) {
        result = ngcllInvokeServerRequest(
            context, invokeMng, NGCLI_INVOKE_SERVER_REQUEST_TYPE_EXIT,
            NULL, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Request to the Invoke Server.\n", fName);
            returnCode = 0;
        }
    }

    /* Close Request stream */
    result = fclose(invokeMng->ngism_requestFp);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing pipe for child stdin failed: %s.\n",
            fName, strerror(errno));
        returnCode = 0;
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Request pipe for Invoke Server %s(%d) closed.\n",
        fName, serverType, typeCount);

    /* Stop the Reply Reader Thread */
    result = ngcllInvokeServerReplyReaderThreadStop(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't stop the Invoke Server %s(%d) Reply Reader thread.\n",
            fName, serverType, typeCount);
        returnCode = 0;
    }

    /* Stop the Notify Reader Thread */
    result = ngcllInvokeServerNotifyReaderThreadStop(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't stop the Invoke Server %s(%d) Notify Reader thread.\n",
            fName, serverType, typeCount);
        returnCode = 0;
    }

    /* Close Reply stream */
    result = fclose(invokeMng->ngism_replyReader.ngisr_fp);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing pipe for child stdout failed: %s.\n",
            fName, strerror(errno));
        returnCode = 0;
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Reply pipe for Invoke Server %s(%d) closed.\n",
        fName, serverType, typeCount);

    /* Close Notify stream */
    result = fclose(invokeMng->ngism_notifyReader.ngisr_fp);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing pipe for child stderr failed: %s.\n",
            fName, strerror(errno));
        returnCode = 0;
    }
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Notify pipe for Invoke Server %s(%d) closed.\n",
        fName, serverType, typeCount);

    /* Finalize the Read Buffer for Notify Reader */
    result = ngcllInvokeServerReadBufferFinalize(
        context, &invokeMng->ngism_notifyReader.ngisr_readBuffer, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Read Buffer for Notify Reader.\n",
            fName);
        returnCode = 0;
    }

    /* Finalize the Read Buffer for Reply Reader */
    result = ngcllInvokeServerReadBufferFinalize(
        context, &invokeMng->ngism_replyReader.ngisr_readBuffer, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Read Buffer for Reply Reader.\n",
            fName);
        returnCode = 0;
    }

    /* Finalize the Mutex and Cond */
    result = ngcllInvokeServerFinalizeMutexAndCond(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize Mutex and Cond for "
            "Invoke Server Manager.\n", fName);
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
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Invoke Server Manager is not found.\n", fName);

    return 0;
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

    ngcllInvokeServerInitializePointer(invokeMng);
    ngcllInvokeServerRequestReplyInitializeMember(
        &invokeMng->ngism_requestReply);

    ngcllInvokeServerReaderInitializeMember(
        &invokeMng->ngism_replyReader);
    ngcllInvokeServerReaderInitializeMember(
        &invokeMng->ngism_notifyReader);

    invokeMng->ngism_ID = 0;
    invokeMng->ngism_mutexInitialized = 0;
    invokeMng->ngism_condInitialized = 0;
    invokeMng->ngism_nJobsStart = 0;
    invokeMng->ngism_nJobsStop = 0;
    invokeMng->ngism_nJobsDone = 0;
    invokeMng->ngism_requestFd = -1;
    invokeMng->ngism_replyFd = -1;
    invokeMng->ngism_notifyFd = -1;
    invokeMng->ngism_maxRequestID = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllInvokeServerInitializePointer(
    ngcliInvokeServerManager_t *invokeMng)
{
    /* Check the arguments */
    assert(invokeMng != NULL);

    invokeMng->ngism_next = NULL;
    invokeMng->ngism_context = NULL;
    invokeMng->ngism_serverType = NULL;
    invokeMng->ngism_requestFp = NULL;
}

/**
 * Initialize the Request or Reply members.
 */
static void
ngcllInvokeServerReaderInitializeMember(
    ngcliInvokeServerReader_t *reader)
{
    /* Check the arguments */
    assert(reader != NULL);

    reader->ngisr_continue = 0;
    reader->ngisr_stopped = 0;
    reader->ngisr_fd = -1;
    reader->ngisr_fp = NULL;
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Mutex.\n", fName);
        return 0;
    }
    invokeMng->ngism_mutexInitialized = 1;

    /* Initialize the condition variable */
    result = ngiCondInitialize(&invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Condition variable.\n", fName);
        return 0;
    }
    invokeMng->ngism_condInitialized = 1;

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
    if (invokeMng->ngism_mutexInitialized != 0) {
        result = ngiMutexDestroy(
            &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Mutex.\n", fName);
            return 0;
        }
    }
    invokeMng->ngism_mutexInitialized = 0;

    /* Finalize the cond */
    if (invokeMng->ngism_condInitialized != 0) {
        result = ngiCondDestroy(
            &invokeMng->ngism_cond, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Condition variable.\n", fName);
            return 0;
        }
    }
    invokeMng->ngism_condInitialized = 0;

    /* Success */
    return 1;
}

/**
 * Initialize the Invoke Server Request Reply
 */
static void
ngcllInvokeServerRequestReplyInitializeMember(
    ngcliInvokeServerRequestReply_t *requestReply)
{
    /* Check the arguments */
    assert(requestReply != NULL);

    requestReply->ngisrr_requesting = 0;
    requestReply->ngisrr_replied = 0;
    requestReply->ngisrr_requestJobStatus = 0;
    requestReply->ngisrr_invokeJobID = NULL;
    requestReply->ngisrr_result = 0;
    requestReply->ngisrr_status = 0;
    requestReply->ngisrr_errorString = NULL;
}

/**
 * Clear the Invoke Server Request Reply
 */
static void
ngcllInvokeServerRequestReplyClear(
    ngcliInvokeServerRequestReply_t *requestReply)
{
    /* Check the arguments */
    assert(requestReply != NULL);

    requestReply->ngisrr_requestJobStatus = 0;

    if (requestReply->ngisrr_invokeJobID != NULL) {
        globus_libc_free(requestReply->ngisrr_invokeJobID);
    }
    requestReply->ngisrr_invokeJobID = NULL;

    requestReply->ngisrr_result = 0;
    requestReply->ngisrr_status = 0;

    if (requestReply->ngisrr_errorString != NULL) {
        globus_libc_free(requestReply->ngisrr_errorString);
    }
    requestReply->ngisrr_errorString = NULL;
}

#endif /* NG_PTHREAD */

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
    /* Check the arguments */
    assert(context != NULL);
    assert(invokeServerJobInfo != NULL);

    if (invokeServerJobInfo->ngisj_invokeJobID != NULL) {
        globus_libc_free(invokeServerJobInfo->ngisj_invokeJobID);
        invokeServerJobInfo->ngisj_invokeJobID = NULL;
    }

    if (invokeServerJobInfo->ngisj_stdoutFile != NULL) {
        globus_libc_free(invokeServerJobInfo->ngisj_stdoutFile);
        invokeServerJobInfo->ngisj_stdoutFile = NULL;
    }

    if (invokeServerJobInfo->ngisj_stderrFile != NULL) {
        globus_libc_free(invokeServerJobInfo->ngisj_stderrFile);
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

#ifdef NG_PTHREAD
/**
 * Treat the retired Invoke Servers.
 */
static int
ngcllInvokeServerTreatRetired(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerTreatRetired";
    ngcliInvokeServerManager_t *cur;
    int result, locked, touched;
    ngLog_t *log;
  
    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    locked = 0;
    touched = 0;

    /* Lock the list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Invoke Server.\n", fName);
        goto error;
    }
    locked = 1;

    do {
        touched = 0;

        cur = context->ngc_invokeMng_head;
        for (; cur != NULL; cur = cur->ngism_next) {
            if (cur->ngism_nJobsMax == 0) {
                continue;
            }
            if (cur->ngism_nJobsStart < cur->ngism_nJobsMax) {
                continue;
            }
            if (cur->ngism_nJobsStop < cur->ngism_nJobsMax) {
                continue;
            }
            if (cur->ngism_nJobsDone < cur->ngism_nJobsMax) {
                continue;
            }
            
            /* Found the retired Invoke Server */
            touched = 1;

            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: Invoke Server %s(%d) retired.\n",
                fName, cur->ngism_serverType, cur->ngism_typeCount);

            result = ngcliInvokeServerDestruct(context, cur, 0, error);
            if (result == 0) {
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                    "%s: Can't destruct the Invoke Server.\n", fName);

                result = ngcllInvokeServerUnregister(context, cur, NULL);
                if (result == 0) {
                    ngclLogPrintfContext(context,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                        "%s: Can't unregister the Invoke Server.\n", fName);
                }
            }

            break;
        }

    } while (touched != 0);

    /* Unlock the list */
    locked = 0;
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Invoke Server.\n", fName);
        goto error;
    }


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the list */
    if (locked != 0) {
        locked = 0;
        result = ngcliContextInvokeServerManagerListWriteUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Invoke Server.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Find the Invoke Server.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerFind(
    ngclContext_t *context,
    char *serverType,
    ngcliInvokeServerManager_t **invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerFind";
    ngcliInvokeServerManager_t *cur;
    int subError, typeCount;

   /* Check the arguments */
    assert(context != NULL);
    assert(serverType != NULL);
    assert(invokeMng != NULL);

    *invokeMng = NULL;

    cur = NULL;
    typeCount = -1;
    subError = NG_ERROR_NO_ERROR;

    /* Find the Invoke Server */
    cur = ngcliContextGetInvokeServerManager(
        context, serverType, -1, &subError);
    if (cur == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s not found, create new Invoke Server.\n",
            fName, serverType);

        /* Not found */
        *invokeMng = NULL;

        return 1;
    }

    typeCount = cur->ngism_typeCount;

    if ((cur->ngism_nJobsMax != 0) &&
        (cur->ngism_nJobsStart >= cur->ngism_nJobsMax)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) jobs exceeded (%d jobs),"
            " create new Invoke Server.\n",
            fName, serverType, typeCount, cur->ngism_nJobsMax);

        /* Not found */
        *invokeMng = NULL;

        return 1;
    }

    if ((cur->ngism_valid == 0) || (cur->ngism_working == 0)) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) was dead, create new Invoke Server.\n",
            fName, serverType, typeCount);

        /* Not found */
        *invokeMng = NULL;

        return 1;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) available.\n",
        fName, serverType, typeCount);

    *invokeMng = cur;

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Up.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerCountUp(
    ngclContext_t *context,
    char *serverType,
    int *currentCount,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountUp";
    ngcliInvokeServerCount_t *invokeCount, *cur;

    /* Check the arguments */
    assert(context != NULL);
    assert(serverType != NULL);
    assert(currentCount != NULL);

    *currentCount = -1;

    /* Find the Invoke Server Count */
    invokeCount = NULL;
    cur = context->ngc_invokeCount_head;
    for (; cur != NULL; cur = cur->ngisc_next) {
        if (strcmp(cur->ngisc_serverType, serverType) == 0) {
            /* Found */
            invokeCount = cur;
            break;
        }
    }

    /* Not Found */
    if (invokeCount == NULL) {

        /* Create new Invoke Server Count */
        invokeCount = ngcllInvokeServerCountConstruct(
            context, serverType, error);
        if (invokeCount == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invoke Server %s Count Construct failed.\n",
                fName, serverType);
            return 0;
        }
    }

    assert(invokeCount != NULL);

    *currentCount = invokeCount->ngisc_count;
    invokeCount->ngisc_count++;

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Construct.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static ngcliInvokeServerCount_t *
ngcllInvokeServerCountConstruct(
    ngclContext_t *context,
    char *serverType,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountConstruct";
    ngcliInvokeServerCount_t *invokeCount;
    int allocated, initialized, result;

    /* Check the arguments */
    assert(context != NULL);
    assert(serverType != NULL);

    allocated = 0;
    initialized = 0;

    /* Allocate */
    invokeCount = ngcllInvokeServerCountAllocate(context, error);
    if (invokeCount == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Invoke Server Count.\n",
            fName);
        goto error;
    }
    allocated = 1;

    /* Initialize */
    result = ngcllInvokeServerCountInitialize(
        context, serverType, invokeCount, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Invoke Server %s Count.\n",
            fName, serverType);
        goto error;
    }
    initialized = 1;

    /* Register */
    result = ngcllInvokeServerCountRegister(
        context, invokeCount, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Invoke Server %s Count.\n",
            fName, serverType);
        goto error;
    }

    /* Success */
    return invokeCount;

    /* Error occurred */
error:

    /* Finalize */
    if ((invokeCount != NULL) && (initialized != 0)) {
        initialized = 0;
        result = ngcllInvokeServerCountFinalize(context, invokeCount, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Invoke Server Count.\n", fName);
        }
    }

    /* Deallocate */
    if ((invokeCount != NULL) && (allocated != 0)) {
        allocated = 0;
        result = ngcllInvokeServerCountFree(context, invokeCount, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the Invoke Server Count.\n", fName);
        }
        invokeCount = NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Invoke Server Count Destruct.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllInvokeServerCountDestruct(
    ngclContext_t *context,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountDestruct";
    ngcliInvokeServerCount_t *curInvokeCount;
    int destructAll, result;

    /* Check the arguments */
    assert(context != NULL);

    destructAll = 0;

    if (invokeCount == NULL) {
        destructAll = 1;
    }

    if (destructAll != 0) {
        curInvokeCount = context->ngc_invokeCount_head;
    } else {
        curInvokeCount = invokeCount;
    }

    while (curInvokeCount != NULL) {
        /* Unregister */
        result = ngcllInvokeServerCountUnregister(
            context, curInvokeCount, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unregister the Invoke Server Count.\n", fName);
            return 0;
        }
     
        /* Finalize */
        result = ngcllInvokeServerCountFinalize(
            context, curInvokeCount, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Invoke Server Count.\n", fName);
            return 0;
        }
     
        /* Deallocate */
        result = ngcllInvokeServerCountFree(context, curInvokeCount, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the storage for Invoke Server Count.\n",
                fName);
            return 0;
        }

        /* Next Invoke Count */
        if (destructAll != 0) {
            /* Get the new head */
            curInvokeCount = context->ngc_invokeCount_head;
        } else {
            curInvokeCount = NULL;
        }
    }

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Allocate.
 */
static ngcliInvokeServerCount_t *
ngcllInvokeServerCountAllocate(
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountAllocate";
    ngcliInvokeServerCount_t *invokeCount;

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate the Invoke Server Count */
    invokeCount = globus_libc_calloc(
        1, sizeof(ngcliInvokeServerCount_t));
    if (invokeCount == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Invoke Server Count.\n",
            fName);
        return NULL;
    }

    /* Success */
    return invokeCount;
}

/**
 * Invoke Server Count Deallocate.
 */
static int
ngcllInvokeServerCountFree(
    ngclContext_t *context,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
#if 0
    static const char fName[] = "ngcllInvokeServerCountFree";
#endif

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeCount != NULL);

    /* Deallocate the Invoke Server Count */
    globus_libc_free(invokeCount);

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Initialize.
 */
static int
ngcllInvokeServerCountInitialize(
    ngclContext_t *context,
    char *serverType,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(serverType != NULL);
    assert(invokeCount != NULL);

    ngcllInvokeServerCountInitializeMember(invokeCount);

    invokeCount->ngisc_serverType = strdup(serverType);
    if (invokeCount->ngisc_serverType == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string.\n",
            fName);
        return 0;
    }

    invokeCount->ngisc_count = 0;

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Finalize.
 */
static int
ngcllInvokeServerCountFinalize(
    ngclContext_t *context,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
#if 0
    static const char fName[] = "ngcllInvokeServerCountFinalize";
#endif

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeCount != NULL);

    assert(invokeCount->ngisc_serverType != NULL);
    globus_libc_free(invokeCount->ngisc_serverType);
    invokeCount->ngisc_serverType = NULL;

    ngcllInvokeServerCountInitializeMember(invokeCount);

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Initialize Member.
 */
static void
ngcllInvokeServerCountInitializeMember(
    ngcliInvokeServerCount_t *invokeCount)
{
#if 0
    static const char fName[] = "ngcllInvokeServerCountInitializeMember";
#endif

    /* Check the arguments */
    assert(invokeCount != NULL);

    invokeCount->ngisc_next = NULL;
    invokeCount->ngisc_serverType = NULL;
    invokeCount->ngisc_count = 0;
}

/**
 * Invoke Server Count Register.
 */
static int
ngcllInvokeServerCountRegister(
    ngclContext_t *context,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
#if 0
    static const char fName[] = "ngcllInvokeServerCountRegister";
#endif

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeCount != NULL);

    /* Append at last of the list */
    invokeCount->ngisc_next = NULL;
    if (context->ngc_invokeCount_head == NULL) {
        /* No information is registered yet */
        assert(context->ngc_invokeCount_tail == NULL);
        context->ngc_invokeCount_head = invokeCount;
        context->ngc_invokeCount_tail = invokeCount;
    } else {
        assert(context->ngc_invokeCount_tail != NULL);
        assert(context->ngc_invokeCount_tail->ngisc_next == NULL);
        context->ngc_invokeCount_tail->ngisc_next = invokeCount;
        context->ngc_invokeCount_tail = invokeCount;
    }

    /* Success */
    return 1;
}

/**
 * Invoke Server Count Unregister.
 */
static int
ngcllInvokeServerCountUnregister(
    ngclContext_t *context,
    ngcliInvokeServerCount_t *invokeCount,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerCountUnregister";
    ngcliInvokeServerCount_t *prev, *curr;


    /* Check the arguments */
    assert(context != NULL);
    assert(invokeCount != NULL);

    /* Find the Invoke Server Manager */
    prev = NULL;
    curr = context->ngc_invokeCount_head;
    for (; curr != invokeCount; curr = curr->ngisc_next) {
        if (curr == NULL)
            goto notFound;
        prev = curr;
    }

    /* Unregister the Invoke Server Manager */
    if (invokeCount == context->ngc_invokeCount_head)
        context->ngc_invokeCount_head = invokeCount->ngisc_next;
    if (invokeCount == context->ngc_invokeCount_tail)
        context->ngc_invokeCount_tail = prev;
    if (prev != NULL)
        prev->ngisc_next = invokeCount->ngisc_next;
    invokeCount->ngisc_next = NULL;

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: Invoke Server Count is not found.\n", fName);

    return 0;
}

/**
 * Invoke the Invoke Server process.
 */
static int
ngcllInvokeServerProcessInvoke(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerProcessInvoke";
    int childStdin[2], childStdout[2], childStderr[2];
    char invokeServerProgram[NGI_FILE_NAME_MAX];
    int argCur, result, exitStatus, exitCode;
    char logFileName[NGI_FILE_NAME_MAX];
    char *serverType, *serverPath;
    pid_t childPid, returnedPid;
    char *invokeArgs[6]; /* path, log-switch, log-file, NULL */
    size_t logLength;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(invokeMng->ngism_serverType != NULL);
    assert(jobMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    argCur = 0;
    invokeArgs[argCur] = NULL;
    serverPath = NULL;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Creating the process for Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    serverPath = jobMng->ngjm_attr.ngja_isInfo.ngisi_path; /* may be NULL */

    /* Check the Invoke Server file */
    result = ngcliInvokeServerProgramCheckAccess(
        context, serverType, serverPath, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) program access check failed.\n",
            fName, serverType, typeCount);
        return 0;
    }

    if (serverPath == NULL) {
        /* Get the Invoke Server name */
        result = ngcllInvokeServerProgramNameGet(
            context, invokeServerProgram, sizeof(invokeServerProgram),
            serverType, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Invoke Server name for %s(%d).\n",
                fName, serverType, typeCount);
            return 0;
        }
        serverPath = invokeServerProgram;
    }

    invokeArgs[argCur++] = serverPath;
    invokeArgs[argCur] = NULL;

    /* Get the log file name */
    if ((jobMng->ngjm_attr.ngja_isInfo.ngisi_logFilePath != NULL) ||
        (jobMng->ngjm_attr.ngja_lmInfo.nglmi_invokeServerLog != NULL)) {

        logLength = 0;
        logFileName[0] = '\0';

        if (jobMng->ngjm_attr.ngja_isInfo.ngisi_logFilePath != NULL) {
            logLength = snprintf(
                logFileName, sizeof(logFileName),
                "%s",
                jobMng->ngjm_attr.ngja_isInfo.ngisi_logFilePath);

        } else if (jobMng->ngjm_attr.ngja_lmInfo.nglmi_invokeServerLog
            != NULL) {
            logLength = snprintf(
                logFileName, sizeof(logFileName),
                "%s.%s",
                jobMng->ngjm_attr.ngja_lmInfo.nglmi_invokeServerLog,
                serverType);
        }

        if (invokeMng->ngism_nJobsMax > 0) {
            snprintf(
                &logFileName[logLength], sizeof(logFileName) - logLength,
                "-%d", invokeMng->ngism_typeCount);
        }

        invokeArgs[argCur++] = NGCLI_INVOKE_SERVER_LOG_FILE_SWITCH;
        invokeArgs[argCur++] = logFileName;
        invokeArgs[argCur] = NULL;

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) log file name is \"%s\".\n",
            fName, serverType, typeCount, logFileName);
    }

    /* Create parent and child pipe for child stdin/stdout/stderr */
    result = pipe(childStdin);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: creating stdin pipe failed: %s.\n", fName, strerror(errno));
        return 0;
    }
    result = pipe(childStdout);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: creating stdout pipe failed: %s.\n", fName, strerror(errno));
        return 0;
    }
    result = pipe(childStderr);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: creating stderr pipe failed: %s.\n", fName, strerror(errno));
        return 0;
    }

    invokeMng->ngism_requestFd  = childStdin[1];  /* child reads */
    invokeMng->ngism_replyFd = childStdout[0];    /* child writes */
    invokeMng->ngism_notifyFd = childStderr[0];   /* child writes */

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) pipe = %d/%d/%d (in/out/err).\n",
        fName, serverType, typeCount,
        invokeMng->ngism_requestFd, invokeMng->ngism_replyFd,
        invokeMng->ngism_notifyFd);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoking Invoke Server %s(%d) process.\n",
        fName, serverType, typeCount);

    childPid = fork();
    if (childPid < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: fork() failed: %s.\n", fName, strerror(errno));
        return 0;
    }

    if (childPid == 0) {
        /* child */
        ngcllInvokeServerProcessInvokeChild(
            serverPath, invokeArgs,
            childStdin, childStdout, childStderr);

        /* NOT REACHED */
        abort();
    }

    /* parent */

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
        NULL, "%s: Invoke Server %s(%d) parent process pid = %ld.\n",
        fName, serverType, typeCount, (long)childPid);

    /* Close unnecessary side of parent pipe */
    result = close(childStdin[0]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stdin read failed: %s.\n",
            fName, strerror(errno));
    }
    result = close(childStdout[1]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stdout write failed: %s.\n",
            fName, strerror(errno));
    }
    result = close(childStderr[1]);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stderr write failed: %s.\n",
            fName, strerror(errno));
    }

    /* Wait the child, which exits immediately */
    returnedPid = waitpid(childPid, &exitStatus, 0);
    if (returnedPid < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: waitpid() failed: %ld %s.\n",
            fName, (long)returnedPid, strerror(errno));
        return 0;
    }

    exitCode = exitStatus >> 8;
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) parent process returned"
        " by %d (status = 0x%x).\n",
        fName, serverType, typeCount, exitCode, exitStatus);

    /* Success */
    return 1;
}

/**
 * Invoke Server child process forked, re-fork and exec Invoke Server.
 */
static void
ngcllInvokeServerProcessInvokeChild(
    char *invokeServerProgram,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr)
{
    pid_t childPid;

    /* Do not touch any Ninf-G or Globus Toolkit data. */
    /* Do not return. */

    /* Check the arguments */
    assert(invokeServerProgram != NULL);
    assert(invokeArgs != NULL);
    assert(childStdin != NULL);
    assert(childStdout != NULL);
    assert(childStderr != NULL);

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
        ngcllInvokeServerProcessInvokeChildChild(
            invokeServerProgram, invokeArgs,
            childStdin, childStdout, childStderr);

        /* NOT REACHED */
        abort();
    }

    /* parent */

    /* Success */
    _exit(0); /* Use _exit() to untouch stdout buffer. */
}

/**
 * Invoke Server child process forked, exec Invoke Server.
 */
static void
ngcllInvokeServerProcessInvokeChildChild(
    char *invokeServerProgram,
    char *invokeArgs[],
    int *childStdin,
    int *childStdout,
    int *childStderr)
{
    int result;

    /* Do not return. */

    /* Check the arguments */
    assert(invokeServerProgram != NULL);
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

    result = execv(invokeServerProgram, invokeArgs);
    /* If the exec() was successful, the process will not return here. */

    /* error */

    _exit(1);
}
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
/**
 * Check whether the Invoke Server program accessible or not.
 * path == NULL : use default path.
 */
int
ngcliInvokeServerProgramCheckAccess(
    ngclContext_t *context,
    char *serverType,
    char *serverPath,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerProgramCheckAccess";
    char invokeServerProgram[NGI_FILE_NAME_MAX], *programPath;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(serverType != NULL);

    programPath = NULL;

    if (serverPath != NULL) {
        programPath = serverPath;
    } else {
        /* Get the program name */
        result = ngcllInvokeServerProgramNameGet(
            context, invokeServerProgram, sizeof(invokeServerProgram),
            serverType, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Invoke Server name for %s.\n",
                fName, serverType);
            return 0;
        }
        programPath = invokeServerProgram;
    }

    result = access(programPath, X_OK);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server \"%s\" access error: %s.\n",
            fName, programPath, strerror(errno));
        return 0;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s program available.\n",
        fName, serverType);

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */

/**
 * Check whether the Invoke Server program accessible or not.
 * (Error on NonThread version)
 */
int
ngcliInvokeServerProgramCheckAccess(
    ngclContext_t *context,
    char *serverType,
    char *serverPath,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerProgramCheckAccess";

    /* Check the arguments */
    assert(context != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
/**
 * Get the Invoke Server name.
 */
static int
ngcllInvokeServerProgramNameGet(
    ngclContext_t *context,
    char *nameBuffer,
    size_t bufferSize,
    char *serverType,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerProgramNameGet";
    char *ngDir;

    /* Check the arguments */
    assert(context != NULL);
    assert(nameBuffer != NULL);
    assert(serverType != NULL);

    ngDir = getenv(NGCLI_INVOKE_SERVER_NG_DIR_ENV_NAME);
    if (ngDir == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Environment variable $%s undefined.\n",
            fName, NGCLI_INVOKE_SERVER_NG_DIR_ENV_NAME);
        return 0; 
    }

    snprintf(
        nameBuffer, bufferSize,
        NGCLI_INVOKE_SERVER_PROGRAM_PATH_FORMAT,
        ngDir, 
        NGCLI_INVOKE_SERVER_PROGRAM_NAME_BASE,
        serverType);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server program path is \"%s\"\n",
        fName, nameBuffer);

    /* Success */
    return 1;
}

/**
 * Start the Reply Reader thread
 */
static int
ngcllInvokeServerReplyReaderThreadStart(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyReaderThreadStart";
    ngcliInvokeServerReader_t *reader;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    reader = &invokeMng->ngism_replyReader;
    reader->ngisr_fd = invokeMng->ngism_replyFd;
    reader->ngisr_continue = 1;
    reader->ngisr_stopped = 0;

    /* Create the Reply Reader thread */
    result = globus_thread_create(
        &reader->ngisr_thread, NULL,
        ngcllInvokeServerReplyReaderThread, invokeMng);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the thread for reply reader.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Start the Notify Reader thread
 */
static int
ngcllInvokeServerNotifyReaderThreadStart(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyReaderThreadStart";
    ngcliInvokeServerReader_t *reader;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    reader = &invokeMng->ngism_notifyReader;
    reader->ngisr_fd = invokeMng->ngism_notifyFd;
    reader->ngisr_continue = 1;
    reader->ngisr_stopped = 0;

    /* Create the Reply Reader thread */
    result = globus_thread_create(
        &reader->ngisr_thread, NULL,
        ngcllInvokeServerNotifyReaderThread, invokeMng);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the thread for notify reader.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Stop the Reply Reader thread
 */
static int
ngcllInvokeServerReplyReaderThreadStop(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyReaderThreadStop";
    ngcliInvokeServerReader_t *reader;
    int result, typeCount;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    reader = &invokeMng->ngism_replyReader;
    log = context->ngc_log;

    if (reader->ngisr_stopped != 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Reply Reader already stopped for Invoke Server %s(%d).\n",
            fName, serverType, typeCount);

        /* Success */
        return 1;
    }

    /**
     * Tell the Reply Reader thread to stop
     */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Stopping Reply Reader thread for Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    reader->ngisr_continue = 0;

    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Wait the stop */
    while (reader->ngisr_stopped == 0) {
        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }
    }

    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Reply Reader thread stopped for Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
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
 * Stop the Notify Reader thread
 */
static int
ngcllInvokeServerNotifyReaderThreadStop(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyReaderThreadStop";
    ngcliInvokeServerReader_t *reader;
    int result, typeCount;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    reader = &invokeMng->ngism_notifyReader;
    log = context->ngc_log;

    if (reader->ngisr_stopped != 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Notify Reader already stopped for Invoke Server %s(%d).\n",
            fName, serverType, typeCount);

        /* Success */
        return 1;
    }

    /**
     * Tell the Notify Reader thread to stop
     */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Stopping Notify Reader thread for Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    reader->ngisr_continue = 0;

    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Wait the stop */
    while (reader->ngisr_stopped == 0) {
        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
        }
    }

    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Notify Reader thread stopped for Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
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
 * Reply Reader thread
 */
static void *
ngcllInvokeServerReplyReaderThread(void *threadArgument)
{
    static const char fName[] = "ngcllInvokeServerReplyReaderThread";
    ngcliInvokeServerReadBuffer_t *readBuffer;
    ngcliInvokeServerManager_t *invokeMng;
    ngcliInvokeServerReader_t *reader;
    int *error, errorEntity, result;
    int finish, mutexLocked;
    ngclContext_t *context;
    char *serverType;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(threadArgument != NULL);

    invokeMng = (ngcliInvokeServerManager_t *)threadArgument;
    context = invokeMng->ngism_context;
    reader = &invokeMng->ngism_replyReader;
    readBuffer = &reader->ngisr_readBuffer;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;
    error = &errorEntity;
    mutexLocked = 0;

    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the state */
    if ((reader->ngisr_continue == 0) ||
        (reader->ngisr_stopped != 0)) {
    
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Reply Reader state.\n", fName);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Reply Reader thread invoked.\n",
        fName, serverType, typeCount);

    reader->ngisr_fp = fdopen(reader->ngisr_fd, "r");
    if (reader->ngisr_fp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't fdopen(%d) for Reply Reader: %s.\n",
            fName, reader->ngisr_fd, strerror(errno));
        goto error;
    }
    setvbuf(reader->ngisr_fp, NULL, _IONBF, 0);

    finish = 0;
    do {
        result = ngcllInvokeServerReadLine(
            context, invokeMng, reader, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Reply read failed.\n", fName);
            goto error;
        }

        /* Is thread stop requested? */
        if ((invokeMng->ngism_valid == 0) ||
            (reader->ngisr_continue == 0)) {
            finish = 1;
            break;
        }

        /* Is connection closed? */
        if ((strcmp(readBuffer->ngisrb_buf, "") == 0) &&
            (readBuffer->ngisrb_reachEOF != 0)) {

            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: Connection closed for Reply Reader.\n", fName);

            finish = 1;
            break;
        }

        result = ngcllInvokeServerReplyProcess(
            context, invokeMng, readBuffer, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Reply read failed.\n", fName);
            goto error;
        }

        if (readBuffer->ngisrb_reachEOF != 0) {
            finish = 1;
        }
    } while (finish == 0);

    /* Is it unexpected? */
    if ((invokeMng->ngism_working != 0) &&
        (invokeMng->ngism_valid != 0)) {

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: InvokeServer %s(%d) Reply Reader thread"
            " got unexpected close.\n",
            fName, serverType, typeCount);

        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Reply Reader thread return.\n",
        fName, serverType, typeCount);

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    reader->ngisr_stopped = 1;
    invokeMng->ngism_valid = 0;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        goto error;
    }

    /* Success */
    return NULL;

    /* Error occurred */
error:
    /* Make this Invoke Server Unusable */
    result = ngcllInvokeServerUnusable(
        context, invokeMng, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the InvokeServer %s(%d) unusable.\n",
            fName, serverType, typeCount);
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Reply Reader thread return by error.\n",
        fName, serverType, typeCount);

    /* Lock */
    if (mutexLocked == 0) {
        result = ngiMutexLock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex.\n", fName);
        }
    }
    mutexLocked = 1;

    reader->ngisr_stopped = 1;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &invokeMng->ngism_cond, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
    }

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Notify Reader thread
 */
static void *
ngcllInvokeServerNotifyReaderThread(void *threadArgument)
{
    static const char fName[] = "ngcllInvokeServerNotifyReaderThread";
    ngcliInvokeServerReadBuffer_t *readBuffer;
    ngcliInvokeServerManager_t *invokeMng;
    ngcliInvokeServerReader_t *reader;
    int *error, errorEntity, result;
    int finish, mutexLocked;
    ngclContext_t *context;
    char *serverType;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(threadArgument != NULL);

    invokeMng = (ngcliInvokeServerManager_t *)threadArgument;
    context = invokeMng->ngism_context;
    reader = &invokeMng->ngism_notifyReader;
    readBuffer = &reader->ngisr_readBuffer;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;
    error = &errorEntity;
    mutexLocked = 0;

    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the state */
    if ((reader->ngisr_continue == 0) ||
        (reader->ngisr_stopped != 0)) {
    
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Notify Reader state.\n", fName);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Notify Reader thread invoked.\n",
        fName, serverType, typeCount);

    reader->ngisr_fp = fdopen(reader->ngisr_fd, "r");
    if (reader->ngisr_fp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't fdopen(%d) for Notify Reader: %s.\n",
            fName, reader->ngisr_fd, strerror(errno));
        goto error;
    }
    setvbuf(reader->ngisr_fp, NULL, _IONBF, 0);

    finish = 0;
    do {
        result = ngcllInvokeServerReadLine(
            context, invokeMng, reader, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Notify read failed.\n", fName);
            goto error;
        }

        /* Is thread stop requested? */
        if ((invokeMng->ngism_valid == 0) ||
            (reader->ngisr_continue == 0)) {
            finish = 1;
            break;
        }

        /* Is connection closed? */
        if ((strcmp(readBuffer->ngisrb_buf, "") == 0) &&
            (readBuffer->ngisrb_reachEOF != 0)) {

            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: Connection closed for Notify Reader.\n", fName);

            finish = 1;
            break;
        }

        result = ngcllInvokeServerNotifyProcess(
            context, invokeMng, readBuffer, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Notify read failed.\n", fName);
            goto error;
        }

        if (readBuffer->ngisrb_reachEOF != 0) {
            finish = 1;
        }
    } while (finish == 0);

    /* Is it unexpected? */
    if ((invokeMng->ngism_working != 0) &&
        (invokeMng->ngism_valid != 0)) {

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: InvokeServer %s(%d) Notify Reader thread"
            " got unexpected close.\n",
            fName, serverType, typeCount);

        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Notify Reader thread return.\n",
        fName, serverType, typeCount);

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    reader->ngisr_stopped = 1;
    invokeMng->ngism_valid = 0;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        goto error;
    }

    /* Success */
    return NULL;

    /* Error occurred */
error:
    /* Make this Invoke Server Unusable */
    result = ngcllInvokeServerUnusable(
        context, invokeMng, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the InvokeServer %s(%d) unusable.\n",
            fName, serverType, typeCount);
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: InvokeServer %s(%d) Notify Reader thread return by error.\n",
        fName, serverType, typeCount);

    /* Lock */
    if (mutexLocked == 0) {
        result = ngiMutexLock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex.\n", fName);
        }
    }
    mutexLocked = 1;

    reader->ngisr_stopped = 1;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &invokeMng->ngism_cond, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
    }

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Process the Reply
 */
static int
ngcllInvokeServerReplyProcess(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliInvokeServerReadBuffer_t *readBuffer,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyProcess";
    ngcliInvokeServerRequestReply_t *requestReply;
    char *cur, *serverType;
    int result, typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(readBuffer != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;

    cur = readBuffer->ngisrb_buf;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Got the Reply \"%s\" from Invoke Server %s(%d).\n", fName,
        (cur != NULL ? cur : ""), serverType, typeCount);

    if (requestReply->ngisrr_requesting == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Got the Reply \"%s\" from Invoke Server %s(%d)"
            " which is not Requested.\n",
            fName, (cur != NULL ? cur : ""), serverType, typeCount);

        /* Do not finish the Reply Reader thread */
        goto finish;
    }

    /* Skip space */
    while(isspace((int)*cur)) {
        cur++;
    }

    if (strncmp(cur, NGCLI_INVOKE_SERVER_RESULT_SUCCESS,
        strlen(NGCLI_INVOKE_SERVER_RESULT_SUCCESS)) == 0) {

        requestReply->ngisrr_result = 1;

        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_SUCCESS);

        /* Skip space */
        while(isspace((int)*cur)) {
            cur++;
        }

        if (requestReply->ngisrr_requestJobStatus != 0) {
            /* JOB_STATUS reply */
            result = ngcllInvokeServerReplyProcessJobStatus(
                context, invokeMng, cur, error);
            if (result == 0) {
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't process JOB_STATUS reply.\n",
                    fName);
                goto error;
            }
        }
    } else if (strncmp(cur, NGCLI_INVOKE_SERVER_RESULT_FAILURE,
        strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE)) == 0) {

        requestReply->ngisrr_result = 0;

        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE);

        /* Skip space */
        while(isspace((int)*cur)) {
            cur++;
        }

        if (*cur != '\0') {
            requestReply->ngisrr_errorString = strdup(cur);
            if (requestReply->ngisrr_errorString == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string on"
                    " Invoke Server %s(%d).\n",
                    fName, serverType, typeCount);
                goto error;
            }
        }

    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid reply \"%s\" on Invoke Server %s(%d).\n",
            fName, readBuffer->ngisrb_buf, serverType, typeCount);
    }

finish:
    /* Notify to main thread */
    result = ngcllInvokeServerReplySet(context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the Reply.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Notify to main thread */
    result = ngcllInvokeServerReplySet(context, invokeMng, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the Reply.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Process the Reply JOB_STATUS
 */
static int
ngcllInvokeServerReplyProcessJobStatus(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *argument,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyProcessJobStatus";
    char *cur, *serverType, *statusName, *messageString, *invokeJobID;
    ngcliInvokeServerRequestReply_t *requestReply;
    ngcllInvokeServerStatusTable_t *statusTable;
    int result, i, found, jobMngLocked;
    ngcllInvokeServerStatus_t status;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(argument != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;

    statusTable = NULL;
    statusName = NULL;
    messageString = NULL;
    jobMngLocked = 0;

    cur = argument;

    invokeJobID = requestReply->ngisrr_invokeJobID;
    if (invokeJobID == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Serve %s(%d) JOB_STATUS reply JobID is NULL.\n",
            fName, serverType, typeCount);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    /* Find the status */
    found = 0;
    status = NGI_INVOKE_SERVER_STATUS_UNDEFINED;
    statusTable =
        (ngcllInvokeServerStatusTable_t *)ngcllInvokeServerStatusTable;
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state returned from Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Job Manager.\n", fName);
        goto error;
    }
    jobMngLocked = 1;

    result = ngcllInvokeServerNotifyJobStatusChange(
        context, invokeMng, invokeJobID, status, statusName,
        messageString, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the status for Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);
        goto error;
    }

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Job Manager.\n", fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Job Manager.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify.
 */
static int
ngcllInvokeServerNotifyProcess(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliInvokeServerReadBuffer_t *readBuffer,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerNotifyProcess";
    char *createNotifyString, *statusNotifyString, *argument;
    int result, jobMngLocked;
    char *serverType;
    int typeCount;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(readBuffer != NULL);

    createNotifyString = "CREATE_NOTIFY";
    statusNotifyString = "STATUS_NOTIFY";
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    log = context->ngc_log;
    argument = NULL;
    jobMngLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Got the Notify \"%s\" from Invoke Server %s(%d).\n",
        fName,
        (readBuffer->ngisrb_buf != NULL ? readBuffer->ngisrb_buf : "NULL"),
        serverType, typeCount);

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Job Manager.\n", fName);
        goto error;
    }
    jobMngLocked = 1;

    if (strncmp(readBuffer->ngisrb_buf,
        createNotifyString, strlen(createNotifyString)) == 0) {

        argument = readBuffer->ngisrb_buf + strlen(createNotifyString);

        result = ngcllInvokeServerNotifyProcessCreateNotify(
            context, invokeMng, createNotifyString, argument, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't process %s.\n", fName, createNotifyString);
            goto error;
        }

    } else if (strncmp(readBuffer->ngisrb_buf,
        statusNotifyString, strlen(statusNotifyString)) == 0) {

        argument = readBuffer->ngisrb_buf + strlen(statusNotifyString);

        result = ngcllInvokeServerNotifyProcessStatusNotify(
            context, invokeMng, statusNotifyString, argument, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't process %s.\n", fName, statusNotifyString);
            goto error;
        }

    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown Notify \"%s\" ignored.\n",
            fName, readBuffer->ngisrb_buf);
    }

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Job Manager.\n", fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Job Manager.\n", fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid RequestID for %s.\n", fName, notifyName);

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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: %s result for RequestID %d is success.\n",
            fName, notifyName, requestID);

        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_SUCCESS);
        notifyResult = 1;

    } else if (strncmp(cur, NGCLI_INVOKE_SERVER_RESULT_FAILURE,
        strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE)) == 0) {

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: %s result for RequestID %d is failure.\n",
            fName, notifyName, requestID);

        /* Print error message */
        cur += strlen(NGCLI_INVOKE_SERVER_RESULT_FAILURE);

        /* Skip space */
        while(isspace((int)*cur)) {
            cur++;
        }

        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: RequestID %d JobID creation failed by \"%s\".\n",
            fName, requestID, cur);

        notifyResult = 0;

    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown result for %s. (RequestID %d)\n",
            fName, notifyName, requestID);

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
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Invoke Server %s(%d) Job ID too long"
                    " for %s RequestID %d.\n",
                    fName, serverType, typeCount, notifyName, requestID);

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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Job is not found by Invoke Server %s(%d) RequestID %d.\n",
            fName, serverType, typeCount, requestID);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the JobID %s for Invoke Server %s(%d)"
            " RequestID %d.\n",
            fName,
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
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: This job is not the Invoke Server job. Do nothing.\n",
                fName);
        }
        if (jobMng->ngjm_invokeServerInfo.ngisj_invokeServer == NULL) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invoke Server registered to this job is NULL."
                " Do nothing.\n",
                fName);
        }
        if (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID != NULL) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: JobID for RequestID %d is already available (%s)."
                " Do nothing.\n", fName, requestID,
                jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID);
        }
        notifyResult = 0;
    }

    /* Register the JobID */
    if ((notifyResult != 0) && (invokeJobIDvalid != 0)) {
        tmp = strdup(invokeJobID);
        if (tmp == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for the Job ID.\n",
                fName);
            goto error;
        }
        jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID = tmp;
     
        /* log */
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Invoke Server %s(%d) Job RequestID is %d, JobID is \"%s\".\n",
            fName, serverType, typeCount,
            jobMng->ngjm_invokeServerInfo.ngisj_requestID,
            jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID);
    }

    /* Notify JobID */
    result = ngcllInvokeServerJobIDset(context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Notify Invoke Server JobID.\n",
            fName);
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
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Notify Invoke Server JobID.\n",
                fName);
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
    ngcllInvokeServerStatusTable_t *statusTable;
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invoke Server %s(%d) Job ID too long.\n",
                fName, serverType, typeCount);

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
    status = NGI_INVOKE_SERVER_STATUS_UNDEFINED;
    statusTable =
        (ngcllInvokeServerStatusTable_t *)ngcllInvokeServerStatusTable;
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state returned from Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);

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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the status for Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Job is not found by Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't set the status %s"
            " for Invoke Server %s(%d) JobID %s job.\n",
            fName, statusName, serverType, typeCount, invokeJobID);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    assert(jobMng != NULL);

    /* Print the debug message */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
        NULL, "%s: Job status %s.\n", fName, statusName);

    /* log */
    if ((messageString != NULL) && (*messageString != '\0')) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: \"%s\".\n", fName, messageString);
    }

    switch(status) {
    case NGI_INVOKE_SERVER_STATUS_PENDING:
        jobStatus = NGI_JOB_STATUS_PENDING;
        msg = "pending";
        break;

    case NGI_INVOKE_SERVER_STATUS_ACTIVE:
        jobStatus = NGI_JOB_STATUS_ACTIVE;
        msg = "active";

        /* Set the end time */
        result = ngcliJobSetEndTimeOfInvoke(jobMng, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the End time.\n", fName);
        }
        break;

    case NGI_INVOKE_SERVER_STATUS_DONE:
        jobStatus = NGI_JOB_STATUS_DONE;
        msg = "done";
        break;

    case NGI_INVOKE_SERVER_STATUS_FAILED:
        jobStatus = NGI_JOB_STATUS_FAILED;
        msg = "failed";

        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE,
            (jobMng->ngjm_requestCancel == 0 ?
                NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_INFORMATION), NULL,
            "%s: Invoke Server %s(%d) Job failed.\n",
            fName, serverType, typeCount);
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid status on Invoke Server %s(%d) JobID %s.\n",
            fName, serverType, typeCount, invokeJobID);
        return 0;
    }

    /* Is job dead? */
    if ((jobStatus == NGI_JOB_STATUS_FAILED) ||
        (jobStatus == NGI_JOB_STATUS_DONE)) {

        result = ngcliExecutableJobDone(jobMng, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't noify the Job done to Executable.\n", fName);
            /* Not return. */
        }
    }

    /* Print the information */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Job is %s.\n", fName, msg);

    /* Notify the Job Status */
    result = ngcliJobNotifyStatus(jobMng, jobStatus, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't noify the Job Status.\n", fName);

        /* Do not finish the Reply Reader thread */
        return 1;
    }

    /* Count DONE jobs */
    if ((jobStatus == NGI_JOB_STATUS_FAILED) ||
        (jobStatus == NGI_JOB_STATUS_DONE)) {
        invokeMng->ngism_nJobsDone++;

        result = ngcllInvokeServerTreatRetired(context, NULL);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't treat the retired Invoke Servers.\n", fName);
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
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }
    mutexLocked = 1;

    /* Is already done? */
    if (invokeMng->ngism_valid == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) already unusable.\n",
            fName, serverType, typeCount);

        /* Unlock */
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
            return 0;
        }

        /* Success */
        return 1;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Setting Invoke Server %s(%d) unusable.\n",
        fName, serverType, typeCount);

    /* Set the error code */
    invokeMng->ngism_errorCode = NG_ERROR_JOB_DEAD;

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Job Manager.\n", fName);
        goto error;
    }
    jobMngLocked = 1;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: All jobs in the Invoke Server %s(%d) making unusable.\n",
        fName, serverType, typeCount);

    /* Set all Jobs related to this Invoke Server unusable */
    for (jobMng = context->ngc_jobMng_head; jobMng != NULL;
        jobMng = jobMng->ngjm_next) {

        if (jobMng->ngjm_useInvokeServer == 0) {
            continue;
        }

        if (invokeMng == jobMng->ngjm_invokeServerInfo.ngisj_invokeServer) {
            /* found */
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: This job is managed by Invoke Server %s(%d).\n",
                fName, serverType, typeCount);

            result = ngcliExecutableJobDone(jobMng, log, error);
            if (result == 0) {
                ngcliLogPrintfJob(jobMng,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't noify the Job done to Executable.\n",
                    fName);
                /* Not return */
            }

            /* Notify the Job Status */
            result = ngcliJobNotifyStatus(
                jobMng, NGI_JOB_STATUS_DONE, log, error);
            if (result == 0) {
                ngcliLogPrintfJob(jobMng,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't noify the Job Status.\n", fName);
                /* Not return */
            }
        }
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: All jobs in the Invoke Server %s(%d) making unusable finished.\n",
        fName, serverType, typeCount);

    /* Unlock the list of Job Manager */
    jobMngLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Job Manager.\n", fName);
        goto error;
    }

    /* Stop request the Reply and Notify Reader thread */
    invokeMng->ngism_replyReader.ngisr_continue = 0;
    invokeMng->ngism_notifyReader.ngisr_continue = 0;

    /* No more Invoke Server valid */
    invokeMng->ngism_valid = 0;

    /* Notify the stop */
    result = ngiCondBroadcast(
        &invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable.\n", fName);
        goto error;
    }

    /* Unlock */
    mutexLocked = 0;
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't unusable the Invoke Server %s(%d).\n",
        fName, serverType, typeCount);

    /* Unlock the list of Job Manager */
    if (jobMngLocked != 0) {
        jobMngLocked = 0;
        result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Job Manager.\n", fName);
        }
    }

    /* Unlock */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
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
    ngLog_t *log;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(valid != NULL);

    log = context->ngc_log;

    *valid = 1;

    /* Wait until Unusable finish */

    /* Lock */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Check the flag */
    if (invokeMng->ngism_valid == 0) {
        *valid = 0;
    }

    if (invokeMng->ngism_working == 0) {
        *valid = 0;
    }

    /* Unlock */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read Buffer Initialize
 */
static int
ngcllInvokeServerReadBufferInitialize(
    ngclContext_t *context,
    ngcliInvokeServerReadBuffer_t *readBuffer,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(readBuffer != NULL);

    ngcllInvokeServerReadBufferInitializeMember(readBuffer);

    /* Success */
    return 1;
}

/**
 * Read Buffer Finalize
 */
static int
ngcllInvokeServerReadBufferFinalize(
    ngclContext_t *context,
    ngcliInvokeServerReadBuffer_t *readBuffer,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(readBuffer != NULL);

    if (readBuffer->ngisrb_buf != NULL) {
        free(readBuffer->ngisrb_buf);
        readBuffer->ngisrb_buf = NULL;
    }

    ngcllInvokeServerReadBufferInitializeMember(readBuffer);

    /* Success */
    return 1;
}

/**
 * Read Buffer Initialize Member
 */
static void
ngcllInvokeServerReadBufferInitializeMember(
    ngcliInvokeServerReadBuffer_t *readBuffer)
{
    /* Check the arguments */
    assert(readBuffer != NULL);

    readBuffer->ngisrb_buf = NULL;
    readBuffer->ngisrb_bufSize = 0;
    readBuffer->ngisrb_reachEOF = 0;
}

/**
 * Read one line from Invoke Server.
 */
static int
ngcllInvokeServerReadLine(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliInvokeServerReader_t *reader,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReadLine";
    int bufSize, oldBufSize, newBufSize, terminateSize, result;
    char *p, *buf, *oldBuf, *terminateStr, *termMatchStart;
    int cur, c, finish, available, termMatchCount;
    ngcliInvokeServerReadBuffer_t *readBuffer;
    struct pollfd pollFd;
    int sleepInterval;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(reader != NULL);

    readBuffer = &reader->ngisr_readBuffer;

    if (readBuffer->ngisrb_reachEOF != 0) {
        /* Success */
        return 1;
    }

    log = context->ngc_log;

    buf = readBuffer->ngisrb_buf;
    bufSize = readBuffer->ngisrb_bufSize;

    terminateStr  = NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR;
    terminateSize = NGCLI_INVOKE_SERVER_LINE_TERMINATOR_SIZE;

    cur = 0;
    p = buf;
    termMatchCount = 0;
    termMatchStart = NULL;
    finish = 0;
    available = 0;
    sleepInterval = 500; /* 500 ms */

    /* Wait the data or thread stop request */
    do {
        /* Lock */
        result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex.\n", fName);
            return 0;
        }

        /* Check the valid flag */
        if (invokeMng->ngism_valid == 0) {
            finish = 1;
        }

        /* Check the stop flag */
        if (reader->ngisr_continue == 0) {
            finish = 1;
        }

        /* Unlock */
        result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
            return 0;
        }

        if (finish != 0) {
            /* Success */
            return 1;
        }

        /* Sleep and poll the reply or notify data available */
        pollFd.fd = reader->ngisr_fd;
        pollFd.events = POLLIN;
        pollFd.revents = 0;

        result = poll(&pollFd, 1, sleepInterval);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: poll failed: %s.\n",
                fName, strerror(errno));
            return 0;

        } else if (result > 0) {
            available = 1;
        }
    } while (available == 0);

    /* Read one line */
    do {
        c = fgetc(reader->ngisr_fp);

        if (cur >= bufSize) {
            /* Renew buffer */
            oldBuf = buf;
            oldBufSize = bufSize;

            if (bufSize <= 0) {
                newBufSize = NGCLI_INVOKE_SERVER_READ_BUFFER_INITIAL_SIZE;
            } else {
                newBufSize = bufSize * 2;
            }

            buf = (char *)globus_libc_calloc(newBufSize, sizeof(char));
            if (buf == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngclLogPrintfContext(context,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for read buffer.\n",
                    fName);
                return 0;
            }
            bufSize = newBufSize;
            newBufSize = 0;

            if (oldBuf != NULL) {
                memcpy(buf, oldBuf, oldBufSize);

                globus_libc_free(oldBuf);
                oldBuf = NULL;
                oldBufSize = 0;
            }

            p = buf + cur;

            readBuffer->ngisrb_buf = buf;
            readBuffer->ngisrb_bufSize = bufSize;
        }

        *p = ((c != EOF) ? c : '\0');

        if (c == EOF) {
            readBuffer->ngisrb_reachEOF = 1;
            finish = 1;
        }

        /* Check terminate charactor */
        if (*p == terminateStr[termMatchCount]) {
            if (termMatchCount == 0) {
                termMatchStart = p;
            }
            termMatchCount++;
        } else {
            termMatchCount = 0;
        }

        if (termMatchCount >= terminateSize) {
            finish = 1;
            assert(termMatchStart != NULL);
            *termMatchStart = '\0';
        }

        cur++;
        p++;

    } while (finish == 0);
    
    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server for %s.\n",
            fName, serverType);
        return 0;
    }

    typeCount = invokeMng->ngism_typeCount;

    /* Set Invoke Server to the Job */
    jobMng->ngjm_invokeServerInfo.ngisj_invokeServer = invokeMng;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server %s(%d) validity.\n",
            fName, serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) already dead.\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) Job Start.\n",
        fName, serverType, typeCount);

    /* Create the stdout/stderr temporary file */
    result = ngcllInvokeServerJobStdoutStderrCreate(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the file for Job stdout/stderr.\n",
            fName);
        /* Not return */
    }

    /* Request the JOB_CREATE and wait Reply */
    result = ngcllInvokeServerRequest(
        context, invokeMng,
        NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_CREATE,
        jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Request to the Invoke Server.\n", fName);
        return 0;
    }
    
    /* Wait the Invoke Server JobID (CREATE_NOTIFY) */
    result = ngcllInvokeServerJobIDwait(context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't wait the Invoke Server JobID.\n", fName);
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

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    valid = 0;

    invokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server.\n", fName);
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server %s(%d) validity.\n",
            fName, serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Invoke Server %s(%d) already dead.\n",
            fName, serverType, typeCount);

        /* Success */
        return 1;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) Job Stop.\n",
        fName, serverType, typeCount);

    /* Output the Job stdout/stderr */
    result = ngcllInvokeServerJobStdoutStderrOutput(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) Job output stdout/stderr failed.\n",
            fName, serverType, typeCount);
        /* Not return */
    }

    /* Destroy the Job stdout/stderr */
    result = ngcllInvokeServerJobStdoutStderrDestroy(
        context, invokeMng, jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) Job stdout/stderr destroy failed.\n",
            fName, serverType, typeCount);
        /* Not return */
    }

    /* Is job destroyed? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed != 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The job is already destroyed.\n", fName);

        /* Success */
        return 1;
    }

    /* Request the JOB_DESTROY and wait Reply */
    result = ngcllInvokeServerRequest(
        context, invokeMng,
        NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY,
        jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Request to the Invoke Server.\n", fName);
        return 0;
    }

    jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed = 1;
    invokeMng->ngism_nJobsStop++;

    result = ngcllInvokeServerTreatRetired(context, NULL);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't treat the retired Invoke Servers.\n", fName);
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

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(doNotWait != NULL);

    valid = 0;
    *doNotWait = 0;

    invokeMng = jobMng->ngjm_invokeServerInfo.ngisj_invokeServer;
    if (invokeMng == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server.\n", fName);
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server %s(%d) validity.\n",
            fName, serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        *doNotWait = 1;

        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Invoke Server %s(%d) already dead.\n",
            fName, serverType, typeCount);

        /* Success */
        return 1;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) Job Cancel.\n",
        fName, serverType, typeCount);

    /* Is job destroyed? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed != 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The job is already destroyed.\n", fName);

        /* Success */
        return 1;
    }

    /* Done? */
    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Job is already done. suppress job cancel.\n", fName);

        /* Success */
        return 1;
    }

    /* Cancel requested? */
    if (jobMng->ngjm_requestCancel == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Job cancel was not requested: All Executables are exited.\n",
            fName);

        /* Success */
        return 1;
    }

    /* Request the JOB_DESTROY and wait Reply */
    result = ngcllInvokeServerRequest(
        context, invokeMng,
        NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY,
        jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Request to the Invoke Server.\n", fName);
        return 0;
    }

    jobMng->ngjm_invokeServerInfo.ngisj_jobDestroyed = 1;
    invokeMng->ngism_nJobsStop++;

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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server.\n", fName);
        return 0;
    }

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Is Invoke Server valid? */
    result = ngcllInvokeServerIsValid(context, invokeMng, &valid, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Invoke Server %s(%d) validity.\n",
            fName, serverType, typeCount);
        return 0;
    }
    if (valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) already dead.\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) Job Status Get.\n",
        fName, serverType, typeCount);

    /* Request the JOB_STATUS and wait Reply */
    result = ngcllInvokeServerRequest(
        context, invokeMng,
        NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_STATUS,
        jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Request to the Invoke Server.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

#else /* NG_PTHREAD */
/**
 * Start Job on the Invoke Server (Error on NonThread version)
 */
int
ngcliInvokeServerJobStart(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStart";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Stop Job on the Invoke Server (Error on NonThread version)
 */
int
ngcliInvokeServerJobStop(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStop";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Cancel Job on the Invoke Server (Error on NonThread version)
 */
int
ngcliInvokeServerJobCancel(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *doNotWait,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobCancel";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(doNotWait != NULL);

    *doNotWait = 1;

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Get the Job Status from the Invoke Server (Error on NonThread version)
 */
int
ngcliInvokeServerJobStatusGet(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcliInvokeServerJobStatusGet";

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invoke Server is not supported "
        "for this GlobusToolkit flavor.\n", fName);

    /* Failed */
    return 0;
}
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
/**
 * Request Lock
 */
static int
ngcllInvokeServerRequestLock(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestLock";
    int typeCount, result;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Wait unlock */
    while ((invokeMng->ngism_valid != 0) &&
        (invokeMng->ngism_requestReply.ngisrr_requesting != 0)) {

        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Cond for InvokeServer %s(%d).\n",
                fName, serverType, typeCount);
            goto error;
        }
    }

    /* Is Invoke Server valid? */
    if (invokeMng->ngism_valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) was dead.\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Lock the Request for InvokeServer %s(%d).\n",
        fName, serverType, typeCount);

    /* Lock the Request */
    invokeMng->ngism_requestReply.ngisrr_requesting = 1;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Request Unlock
 */
static int
ngcllInvokeServerRequestUnlock(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestUnlock";
    int typeCount, result;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Unlock the Request for InvokeServer %s(%d).\n",
        fName, serverType, typeCount);

    /* Is Request Locked? */
    if (invokeMng->ngism_requestReply.ngisrr_requesting == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request is not locked for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* Unlock the Request */
    invokeMng->ngism_requestReply.ngisrr_requesting = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Cond for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Reply Set
 */
static int
ngcllInvokeServerReplySet(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplySet";
    int typeCount, result;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Reply arrived for InvokeServer %s(%d).\n",
        fName, serverType, typeCount);

    /* Is Reply performed? */
    if (invokeMng->ngism_requestReply.ngisrr_replied != 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Already Replied for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
    }

    /* Set the Reply */
    invokeMng->ngism_requestReply.ngisrr_replied = 1;

    /* Notify signal */
    result = ngiCondBroadcast(&invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Cond for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Reply Wait
 */
static int
ngcllInvokeServerReplyWait(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyWait";
    int typeCount, result;
    char *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    log = context->ngc_log;
    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;

    /* Lock the mutex */
    result = ngiMutexLock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Wait reply */
    while ((invokeMng->ngism_valid != 0) &&
        (invokeMng->ngism_requestReply.ngisrr_replied == 0)) {

        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Cond for InvokeServer %s(%d).\n",
                fName, serverType, typeCount);
            goto error;
        }
    }

    /* Is Invoke Server valid? */
    if ((invokeMng->ngism_working != 0) &&
        (invokeMng->ngism_valid == 0)) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) was dead.\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Waiting Reply for InvokeServer %s(%d) done.\n",
        fName, serverType, typeCount);

    /* Reset */
    invokeMng->ngism_requestReply.ngisrr_replied = 0;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Invoke Server JobID Set
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server JobID arrived.\n", fName);

    /* Is JobID Notify already came? */
    if (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset != 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Invoke Server JobID already exists.\n", fName);
    }

    /* Set the Reply */
    jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset = 1;

    /* Notify signal */
    result = ngiCondBroadcast(&invokeMng->ngism_cond, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Cond for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Success */
    return 1;

   /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, NULL);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Invoke Server JobID Wait
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Wait JobID */
    while ((invokeMng->ngism_valid != 0) &&
        (jobMng->ngjm_invokeServerInfo.ngisj_invokeJobIDset == 0)) {

        result = ngiCondWait(
            &invokeMng->ngism_cond, &invokeMng->ngism_mutex, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Cond for InvokeServer %s(%d).\n",
                fName, serverType, typeCount);
            goto error;
        }
    }

    /* Is Invoke Server valid? */
    if (invokeMng->ngism_valid == 0) {
        NGI_SET_ERROR(error, invokeMng->ngism_errorCode);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) was dead.\n",
            fName, serverType, typeCount);
        goto error;
    }

    invokeJobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Invoke Server %s(%d) JobID \"%s\" arrived.\n",
        fName, serverType, typeCount,
        (invokeJobID != NULL ? invokeJobID : "not"));

    /* Unlock the mutex */
    result = ngiMutexUnlock(&invokeMng->ngism_mutex, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    if (invokeJobID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: JobID was not arrived for RequestID %d"
            " from Invoke Server %s(%d).\n",
            fName, jobMng->ngjm_invokeServerInfo.ngisj_requestID,
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for InvokeServer %s(%d).\n",
            fName, serverType, typeCount);
        return 0;
    }

    /* Failed */
    return 0;
}


/**
 * Request the Invoke Server and wait Reply.
 */
static int
ngcllInvokeServerRequest(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliInvokeServerRequestType_t requestType,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequest";
    int result, requestLocked, requestID, typeCount;
    ngcliInvokeServerRequestReply_t *requestReply;
    char *requestString, *jobID, *serverType;
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(requestType > NGCLI_INVOKE_SERVER_REQUEST_TYPE_UNDEFINED);
    assert(requestType < NGCLI_INVOKE_SERVER_REQUEST_TYPE_NOMORE);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;
    requestString = NULL;
    requestLocked = 0;
    log = context->ngc_log;

    /* Get the requestString */
    switch (requestType) {
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_CREATE:
        requestString = "JOB_CREATE";
        break;
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_STATUS:
        requestString = "JOB_STATUS";
        break;
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY:
        requestString = "JOB_DESTROY";
        break;
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_EXIT:
        requestString = "EXIT";
        break;
    default:
        /* NOT REACHED */
        abort();
    }

    /* Lock the Request */
    result = ngcllInvokeServerRequestLock(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Request.\n", fName);
        goto error;
    }
    requestLocked = 1;

    /* Reset Reply */
    requestReply->ngisrr_replied = 0;
    ngcllInvokeServerRequestReplyClear(requestReply);

    /* Print each protocol */
    switch (requestType) {
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_CREATE:

        invokeMng->ngism_maxRequestID++;
        requestID = invokeMng->ngism_maxRequestID;
        jobMng->ngjm_invokeServerInfo.ngisj_requestID = requestID;

        /* log */
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: New RequestID %d on Invoke Server %s(%d).\n",
            fName, requestID, serverType, typeCount);

        /* log */
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Request \"%s %d\" for Invoke Server %s(%d).\n",
            fName, requestString, requestID, serverType, typeCount);

        /* Set the start time */
        result = ngiSetStartTime(&jobMng->ngjm_invoke, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Start time.\n", fName);
            goto error;
        }

        fprintf(invokeMng->ngism_requestFp,
            "%s %d%s", requestString, requestID,
            NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR);

        result = ngcllInvokeServerRequestJobCreateArgument(
            context, invokeMng->ngism_requestFp, jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the Invoke Server %s arguments.\n",
                fName, requestString);
            goto error;
        }
        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_STATUS:
        jobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;
        if (jobID == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get JobID for Request \"%s\""
                " on Invoke Server %s(%d).\n",
                fName, requestString, serverType, typeCount);
            goto error;
        }

        requestReply->ngisrr_requestJobStatus = 1;
        requestReply->ngisrr_invokeJobID = strdup(jobID);
        if (requestReply->ngisrr_invokeJobID == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for JobID.\n", fName);
            goto error;
        }
        
        /* log */
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Request \"%s %s\" for Invoke Server %s(%d).\n",
            fName, requestString, jobID, serverType, typeCount);

        fprintf(invokeMng->ngism_requestFp,
            "%s %s%s", requestString, jobID,
            NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR);

        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY:

        jobID = jobMng->ngjm_invokeServerInfo.ngisj_invokeJobID;
        if (jobID == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get JobID for Request \"%s\""
                " on Invoke Server %s(%d).\n",
                fName, requestString, serverType, typeCount);
            goto error;
        }

        /* log */
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Request \"%s %s\" for Invoke Server %s(%d).\n",
            fName, requestString, jobID, serverType, typeCount);

        fprintf(invokeMng->ngism_requestFp,
            "%s %s%s", requestString, jobID,
            NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR);

        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_EXIT:

        /* log */
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Request \"%s\" for Invoke Server %s(%d).\n",
            fName, requestString, serverType, typeCount);

        fprintf(invokeMng->ngism_requestFp,
            "%s%s", requestString, NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR);

        break;

    default:
        /* NOT REACHED */
        abort();
    }

    /* Wait the Reply */
    result = ngcllInvokeServerReplyWait(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't wait the Invoke Server %s(%d) %s reply.\n",
            fName, serverType, typeCount, requestString);
        goto error;
    }

    /* Treat the Reply */
    switch (requestType) {
    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_CREATE:
        result = ngcllInvokeServerReplyJobCreateTreat(
            context, invokeMng, jobMng, requestString, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't treat the Invoke Server %s reply.\n",
                fName, requestString);
            goto error;
        }
        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_STATUS:
        result = ngcllInvokeServerReplyJobStatusTreat(
            context, invokeMng, jobMng, requestString, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't treat the Invoke Server %s reply.\n",
                fName, requestString);
            goto error;
        }
        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_JOB_DESTROY:
        result = ngcllInvokeServerReplyJobDestroyTreat(
            context, invokeMng, jobMng, requestString, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't treat the Invoke Server %s reply.\n",
                fName, requestString);
            goto error;
        }
        break;

    case NGCLI_INVOKE_SERVER_REQUEST_TYPE_EXIT:
        result = ngcllInvokeServerReplyExitTreat(
            context, invokeMng, requestString, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't treat the Invoke Server %s reply.\n",
                fName, requestString);
            goto error;
        }
        break;

    default:
        /* NOT REACHED */
        abort();
    }

    /* Clear the RequestReply member */
    ngcllInvokeServerRequestReplyClear(requestReply);

    /* Unlock the Request */
    requestLocked = 0;
    result = ngcllInvokeServerRequestUnlock(
        context, invokeMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Request.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (requestLocked != 0) {
        ngcllInvokeServerRequestReplyClear(requestReply);

        result = ngcllInvokeServerRequestUnlock(
            context, invokeMng, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Request.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Output the Request JOB_CREATE argument
 */
static int
ngcllInvokeServerRequestJobCreateArgument(
    ngclContext_t *context,
    FILE *requestFp,
    ngcliJobManager_t *jobMng,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerRequestJobCreateArgument";
    char *buf, *arg, *crypt, *protocol, *backend, *workDirectory, *p;
    size_t bufferNbytes, argLength, bufLength;
    ngclRemoteMachineInformation_t *rmInfo;
    ngclLocalMachineInformation_t *lmInfo;
    char *trueStr, *falseStr, *tmpFile, *tmpDir;
    int i, count, portNo, interval;
    ngiCommunication_t *comm;

    /* Check the arguments */
    assert(context != NULL);
    assert(requestFp != NULL);
    assert(jobMng != NULL);
    
    buf = NULL;
    arg = NULL;
    comm = NULL;
    crypt = NULL;
    protocol = NULL;
    backend = NULL;
    portNo = 0;
    workDirectory = NULL;
    tmpFile = NULL;
    tmpDir = NULL;
    interval = 0;
    count = 0;
    argLength = 0;
    bufLength = 0;

    trueStr = "true";
    falseStr = "false";

    rmInfo = &jobMng->ngjm_attr.ngja_rmInfo;
    lmInfo = &jobMng->ngjm_attr.ngja_lmInfo;
    bufferNbytes = NGCLI_RSL_NBYTES;

    /* Allocate */
    arg = globus_libc_malloc(bufferNbytes);
    if (arg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for string.\n", fName);
        return 0;
    }

    /* Allocate */
    buf = globus_libc_malloc(bufferNbytes);
    if (buf == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't allocate the storage for string.\n", fName);
        return 0;
    }

    /* Get the Crypt type and Port No. */
    switch (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_crypt) {
    case NG_PROTOCOL_CRYPT_NONE:
        crypt = "none";
        comm = jobMng->ngjm_context->ngc_commNone;
        break;

    case NG_PROTOCOL_CRYPT_AUTHONLY:
        crypt = "authonly";
        comm = jobMng->ngjm_context->ngc_commAuthonly;
        break;

    case NG_PROTOCOL_CRYPT_GSI:
        crypt = "GSI";
        comm = jobMng->ngjm_context->ngc_commGSI;
        break;

    case NG_PROTOCOL_CRYPT_SSL:
        crypt = "SSL";
        comm = jobMng->ngjm_context->ngc_commSSL;
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Unknown Crypt Type %s",
            jobMng->ngjm_attr.ngja_rmInfo.ngrmi_crypt);
        goto error;
    }

    /* Get the protocol */
    switch (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_protocol) {
    case NG_PROTOCOL_TYPE_XML:
        protocol = "XML";
        break;

    case NG_PROTOCOL_TYPE_BINARY:
        protocol = "binary";
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Unknown Protocol Type %s",
            jobMng->ngjm_attr.ngja_rmInfo.ngrmi_protocol);
        goto error;
    }

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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Unknown Backend Type %d",
            jobMng->ngjm_attr.ngja_rmInfo.ngrmi_protocol);
        goto error;
    }

    /* Work Directory */
    workDirectory = NULL;
    if (rmInfo->ngrmi_workDirectory != NULL) {
        workDirectory = strdup(rmInfo->ngrmi_workDirectory);
        if (workDirectory == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the Executable Path.\n", fName);
            goto error;
        }
    } else if (jobMng->ngjm_attr.ngja_stagingEnable == 0) {
        assert(jobMng->ngjm_attr.ngja_executablePath != NULL);
        workDirectory = strdup(jobMng->ngjm_attr.ngja_executablePath);
        if (workDirectory == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the Executable Path.\n", fName);
            goto error;
        }

        p = strrchr(workDirectory, '/');
        if (p != NULL) {
            assert(*p == '/');
            *p = '\0';
        }
    }

    /* Print the JOB_CREATE arguments */

#define NGCLL_PRINT_ARGUMENT(fp, buf, len, jobMng, format, name, value) \
    { \
        bufLength = snprintf((buf), (len), (format), (name), (value)); \
        if (bufLength >= ((len) - 1)) { \
            goto overflow; \
        } \
         \
        fprintf((fp), "%s%s", \
            (buf), NGCLI_INVOKE_SERVER_LINE_TERMINATOR_STR); \
         \
        ngcliLogPrintfJob((jobMng), \
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL, \
            "%s: JOB_CREATE argument: \"%s\"\n", fName, (buf)); \
    }

    /* invoke_server_option on <INVOKE_SERVER> section */
    assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
    for (i = 0; i < jobMng->ngjm_attr.ngja_isInfo.ngisi_nOptions; i++) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s%s", "",
            jobMng->ngjm_attr.ngja_isInfo.ngisi_options[i])
    }

    /* invoke_server_option on <SERVER> section */
    for (i = 0; i < rmInfo->ngrmi_invokeServerNoptions; i++) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s%s", "",
            rmInfo->ngrmi_invokeServerOptions[i])
    }

    /* hostname */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "hostname",
        jobMng->ngjm_attr.ngja_hostName)

    /* port */
    portNo = jobMng->ngjm_attr.ngja_portNo;
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %d", "port",
        ((portNo != NGI_PORT_ANY) ? portNo : 0))

    /* jobmanager */
    if (jobMng->ngjm_attr.ngja_jobManager != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "jobmanager",
            jobMng->ngjm_attr.ngja_jobManager);
    }

    /* subject */
    if (jobMng->ngjm_attr.ngja_subject != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "subject",
            jobMng->ngjm_attr.ngja_subject);
    }

    /* client_name */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "client_name",
        jobMng->ngjm_attr.ngja_clientHostName)

    /* path (remote path or local path) */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "executable_path",
        jobMng->ngjm_attr.ngja_executablePath)

    /* backend */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "backend",
        backend)

    /* count */
    count = 1; /* for dummy */
    if ((jobMng->ngjm_attr.ngja_backend == NG_BACKEND_MPI) ||
        (jobMng->ngjm_attr.ngja_backend == NG_BACKEND_BLACS)) {
        count = jobMng->ngjm_attr.ngja_mpiNcpus;
    } else {
        count = jobMng->ngjm_attr.ngja_invokeNjobs;
    }
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %d", "count",
        count)

    /* staging */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "staging",
        ((jobMng->ngjm_attr.ngja_stagingEnable != 0) ? trueStr : falseStr))


    /* argument : client */
    argLength = snprintf(arg, bufferNbytes,
        "--client=%s:%d",
        jobMng->ngjm_attr.ngja_clientHostName, comm->ngc_portNo);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : gassServer */
    if (jobMng->ngjm_attr.ngja_gassUrl != NULL) {
        argLength = snprintf(arg, bufferNbytes,
            "--gassServer=%s",
            jobMng->ngjm_attr.ngja_gassUrl);
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* argument : crypt */
    argLength = snprintf(arg, bufferNbytes,
        "--crypt=%s",
        crypt);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : protocol */
    argLength = snprintf(arg, bufferNbytes,
        "--protocol=%s",
        protocol);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : contextID */
    argLength = snprintf(arg, bufferNbytes,
        "--contextID=%d",
        jobMng->ngjm_context->ngc_ID);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : jobID */
    argLength = snprintf(arg, bufferNbytes,
        "--jobID=%d",
        jobMng->ngjm_ID);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : heartBeat */
    argLength = snprintf(arg, bufferNbytes,
        "--heartbeat=%d",
        rmInfo->ngrmi_heartBeat);
    if (argLength >= (bufferNbytes - 1))
        goto overflow;

    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "argument", arg)


    /* argument : TCP Connect Retry */
    if (rmInfo->ngrmi_retryInfo.ngcri_count > 0) {
        argLength = snprintf(arg, bufferNbytes,
            "--connectRetry=%d,%d,%g,%s",
            rmInfo->ngrmi_retryInfo.ngcri_count,
            rmInfo->ngrmi_retryInfo.ngcri_interval,
            rmInfo->ngrmi_retryInfo.ngcri_increase,
            (rmInfo->ngrmi_retryInfo.ngcri_useRandom == 1 ?
                "random" : "fixed"));
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* argument : coreDumpSize */
    if (rmInfo->ngrmi_coreDumpSize != -2) {
        argLength = snprintf(arg, bufferNbytes,
            "--coreDumpSize=%d",
            rmInfo->ngrmi_coreDumpSize);
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* argument : debug by terminal */
    if (rmInfo->ngrmi_debug.ngdi_enable) {
        if (rmInfo->ngrmi_debug.ngdi_terminalPath != NULL) {
            argLength = snprintf(arg, bufferNbytes,
                "--debugTerminal=%s",
                rmInfo->ngrmi_debug.ngdi_terminalPath);
            if (argLength >= (bufferNbytes - 1))
                goto overflow;

            NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
                "%s %s", "argument", arg)
        }

        if (rmInfo->ngrmi_debug.ngdi_display != NULL) {
            argLength = snprintf(arg, bufferNbytes,
                "--debugDisplay=%s",
                rmInfo->ngrmi_debug.ngdi_display);
            if (argLength >= (bufferNbytes - 1))
                goto overflow;

            NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
                "%s %s", "argument", arg)
        }

        if (rmInfo->ngrmi_debug.ngdi_debuggerPath != NULL) {
            argLength = snprintf(arg, bufferNbytes,
                "--debugger=%s",
                rmInfo->ngrmi_debug.ngdi_debuggerPath);
            if (argLength >= (bufferNbytes - 1))
                goto overflow;

            NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
                "%s %s", "argument", arg)
        }

        argLength = snprintf(arg, bufferNbytes,
            "--debugEnable=%d",
            rmInfo->ngrmi_debug.ngdi_enable);
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* argument : debugBusyLoop */
    if (rmInfo->ngrmi_debugBusyLoop) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: debug_busyLoop was set."
            " please attach Remote Executable by debugger.\n",
            fName);

        argLength = snprintf(arg, bufferNbytes,
            "--debugBusyLoop=%d",
            rmInfo->ngrmi_debugBusyLoop);
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* argument : tcpNodelay */
    if (lmInfo->nglmi_tcpNodelay != 0) {
        argLength = snprintf(arg, bufferNbytes,
            "--tcpNodelay=%d",
            1 /* true */);
        if (argLength >= (bufferNbytes - 1))
            goto overflow;

        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "argument", arg)
    }


    /* workDirectory */
    if (workDirectory != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "work_directory",
            workDirectory)
    }

    /* GASS URL (This may send while Ninf-G Client invokes GASS Server) */
    if (jobMng->ngjm_attr.ngja_gassUrl != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "gass_url",
            jobMng->ngjm_attr.ngja_gassUrl)
    }

    /* redirect stdout/stderr */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %s", "redirect_enable",
        ((rmInfo->ngrmi_redirectEnable != 0) ? trueStr : falseStr))

    /* redirect stdout filename */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stdoutFile;
    if (tmpFile != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "stdout_file",
            tmpFile)
    }

    /* redirect stdout filename */
    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stderrFile;
    if (tmpFile != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "stderr_file",
            tmpFile)
    }

    /* environment variable */
    for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "environment",
            rmInfo->ngrmi_environment[i])
    }

    /* tmp_dir */
    tmpDir = jobMng->ngjm_attr.ngja_lmInfo.nglmi_tmpDir;
    if (tmpDir != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "tmp_dir",
            tmpDir)
    }

    /* jobStatus polling */
    interval = jobMng->ngjm_attr.ngja_isInfo.ngisi_statusPoll;
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %d", "status_polling",
        ((interval > 0) ? interval : 0))

    /* refresh credentials */
    interval = jobMng->ngjm_attr.ngja_lmInfo.nglmi_refreshInterval;
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s %d", "refresh_credential",
        ((interval > 0) ? interval : 0))

    /* maxTime */
    if (rmInfo->ngrmi_jobMaxTime >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "max_time",
            rmInfo->ngrmi_jobMaxTime)
    }

    /* maxWallTime */
    if (rmInfo->ngrmi_jobMaxWallTime >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "max_wall_time",
            rmInfo->ngrmi_jobMaxWallTime)
    }

    /* maxCpuTime */
    if (rmInfo->ngrmi_jobMaxCpuTime >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "max_cpu_time",
            rmInfo->ngrmi_jobMaxCpuTime)
    }

    /* queue */
    if (jobMng->ngjm_attr.ngja_queueName != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "queue_name",
            jobMng->ngjm_attr.ngja_queueName)
    }

    /* project */
    if (rmInfo->ngrmi_jobProject != NULL) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "project",
            rmInfo->ngrmi_jobProject)
    }

    /* hostCount */
    if (rmInfo->ngrmi_jobHostCount >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "host_count",
            rmInfo->ngrmi_jobHostCount)
    }

    /* minMemory */
    if (rmInfo->ngrmi_jobMinMemory >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "min_memory",
            rmInfo->ngrmi_jobMinMemory)
    }

    /* maxMemory */
    if (rmInfo->ngrmi_jobMaxMemory >= 0) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %d", "max_memory",
            rmInfo->ngrmi_jobMaxMemory)
    }

    /* job_rslExtensions */
    for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
        NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
            "%s %s", "rsl_extensions",
            rmInfo->ngrmi_rslExtensions[i])
    }

    /* END of JOB_CREATE arguments */
    NGCLL_PRINT_ARGUMENT(requestFp, buf, bufferNbytes, jobMng,
        "%s%s", "JOB_CREATE_END", 
        "")

#undef NGCLL_PRINT_ARGUMENT

    if (workDirectory != NULL) {
        globus_libc_free(workDirectory);
        workDirectory = NULL;
    }

    globus_libc_free(buf);
    buf = NULL;
    globus_libc_free(arg);
    arg = NULL;

    /* Success */
    return 1;

    /* Error occurred */
overflow:
    /* Overflow */
    NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: JOB_CREATE argument buffer is overflow.\n", fName);

error:
    /* Deallocate */
    if (buf != NULL) {
        globus_libc_free(buf);
        buf = NULL;
    }
    if (arg != NULL) {
        globus_libc_free(arg);
        arg = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * Treat the Reply JOB_CREATE
 */
static int
ngcllInvokeServerReplyJobCreateTreat(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    char *requestString,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyJobCreateTreat";
    ngcliInvokeServerRequestReply_t *requestReply;
    char *serverType;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;

    /* log */
    if (requestReply->ngisrr_errorString != NULL) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE,
            (requestReply->ngisrr_result == 0 ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_WARNING), NULL,
            "%s: Error on Invoke Server %s(%d): \"%s\".\n",
            fName, serverType, typeCount,
            requestReply->ngisrr_errorString);
    }

    if (requestReply->ngisrr_result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) request %s failed.\n",
            fName, serverType, typeCount, requestString);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Treat the Reply JOB_STATUS
 */
static int
ngcllInvokeServerReplyJobStatusTreat(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    char *requestString,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyJobStatusTreat";
    ngcliInvokeServerRequestReply_t *requestReply;
    char *serverType;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;

    /* log */
    if (requestReply->ngisrr_errorString != NULL) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE,
            (requestReply->ngisrr_result == 0 ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_WARNING), NULL,
            "%s: Error on Invoke Server %s(%d): \"%s\".\n",
            fName, serverType, typeCount,
            requestReply->ngisrr_errorString);
    }

    if (requestReply->ngisrr_result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) request %s failed.\n",
            fName, serverType, typeCount, requestString);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Treat the Reply JOB_DESTROY
 */
static int
ngcllInvokeServerReplyJobDestroyTreat(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    ngcliJobManager_t *jobMng,
    char *requestString,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyJobDestroyTreat";
    ngcliInvokeServerRequestReply_t *requestReply;
    char *serverType;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(jobMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;

    /* log */
    if (requestReply->ngisrr_errorString != NULL) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE,
            (requestReply->ngisrr_result == 0 ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_WARNING), NULL,
            "%s: Error on Invoke Server %s(%d): \"%s\".\n",
            fName, serverType, typeCount,
            requestReply->ngisrr_errorString);
    }

    if (requestReply->ngisrr_result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server %s(%d) request %s failed.\n",
            fName, serverType, typeCount, requestString);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Treat the Reply EXIT
 */
static int
ngcllInvokeServerReplyExitTreat(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    char *requestString,
    int *error)
{
    static const char fName[] = "ngcllInvokeServerReplyExitTreat";
    ngcliInvokeServerRequestReply_t *requestReply;
    char *serverType;
    int justWarning;
    int typeCount;

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    serverType = invokeMng->ngism_serverType;
    typeCount = invokeMng->ngism_typeCount;
    requestReply = &invokeMng->ngism_requestReply;
    justWarning = 0;

    if (invokeMng->ngism_working == 0) {
        /* This wall happen every time on EXIT reply. */
        justWarning = 1;
    }

    /* log */
    if (requestReply->ngisrr_errorString != NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE,
            (((requestReply->ngisrr_result == 0) && (justWarning == 0)) ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_WARNING), NULL,
            "%s: Error on Invoke Server %s(%d): \"%s\".\n",
            fName, serverType, typeCount,
            requestReply->ngisrr_errorString);
    }

    if (requestReply->ngisrr_result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE,
            ((justWarning != 0) ?
            NG_LOG_LEVEL_WARNING : NG_LOG_LEVEL_ERROR), NULL,
            "%s: Invoke Server %s(%d) request %s failed.\n",
            fName, serverType, typeCount, requestString);

        if (justWarning == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            return 0;
        }
    }

    /* Success */
    return 1;
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The Job do not require stdout/stderr file.\n",
            fName);

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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Temporary File Name for stdout.\n",
            fName);
        /* Not return */
    } else {
        /* Register the stdout filename */
        result = ngcliNinfgManagerTemporaryFileRegister(tmpFileStdout, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't register the Temporary File Name \"%s\" for stdout.\n",
                fName, tmpFileStdout);
            /* Not return */
        }
    }

    /* Create the stderr filename */
    tmpFileStderr = ngiTemporaryFileCreate(tmpDir, log, error);
    if (tmpFileStderr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Temporary File Name for stderr.\n",
            fName);
        /* Not return */
    } else {
        /* Register the stderr filename */
        result = ngcliNinfgManagerTemporaryFileRegister(tmpFileStderr, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't register the Temporary File Name \"%s\" for stderr.\n",
                fName, tmpFileStderr);
            /* Not return */
        }
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The Temporary File Name for stdout is \"%s\".\n",
        fName, ((tmpFileStdout != NULL) ? tmpFileStdout : "NULL"));

    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The Temporary File Name for stderr is \"%s\".\n",
        fName, ((tmpFileStderr != NULL) ? tmpFileStderr : "NULL"));

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
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't unregister the Temporary File \"%s\" for stdout.\n",
                fName, tmpFile);
            /* Not return */
        }

        result = ngiTemporaryFileDestroy(tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Temporary File \"%s\" for stdout.\n",
                fName, tmpFile);
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
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't unregister the Temporary File \"%s\" for stderr.\n",
                fName, tmpFile);
            /* Not return */
        }

        result = ngiTemporaryFileDestroy(tmpFile, log, error);
        if (result == 0) {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destroy the Temporary File \"%s\" for stderr.\n",
                fName, tmpFile);
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
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't output the job stdout.\n",
                fName);
            /* Not return */
        }
    }

    tmpFile = jobMng->ngjm_invokeServerInfo.ngisj_stderrFile;
    if (tmpFile != NULL) {
        result = ngcllInvokeServerJobStdoutStderrOutputSub(
            context, invokeMng, jobMng, tmpFile, stderr, "stderr", error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't output the job stdout.\n",
                fName);
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Opening Job %s file \"%s\" failed: %s.\n",
            fName, destName, tmpFile, strerror(errno));
        /* It's not the serious error. No problem. */
        return 1;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: output the Job %s start.\n",
        fName, destName);

    while ((c = fgetc(fp)) != EOF) {
        result = fputc(c, destFp);
        if (result == EOF) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't output the Job %s file.\n",
                fName, destName);
            break;
        }
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: output the Job %s end.\n",
        fName, destName);

    result = fclose(fp);
    if (result != 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Closing Job %s file \"%s\" failed: %s.\n",
            fName, destName, tmpFile, strerror(errno));
        /* Not return */
    }

    /* Success */
    return 1;
}

#endif /* NG_PTHREAD */

