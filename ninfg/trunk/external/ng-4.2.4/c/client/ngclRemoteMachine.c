#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclRemoteMachine.c,v $ $Revision: 1.71 $ $Date: 2007/12/26 12:27:17 $";
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
 * Remote Machine Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngclRemoteMachineInformationManager_t *
    ngcllRemoteMachineInformationManagerAllocate(
    ngclContext_t *, int *);
static int ngcllRemoteMachineInformationManagerFree(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static ngclRemoteMachineInformationManager_t *
ngcllRemoteMachineInformationConstruct(
    ngclContext_t *,
    ngclRemoteMachineInformation_t *,
    int *);
static int ngcllRemoteMachineInformationDestruct(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static int ngcllRemoteMachineInformationManagerInitialize(
     ngclContext_t *,
     ngclRemoteMachineInformationManager_t *,
     ngclRemoteMachineInformation_t *,
     int *);
static int ngcllRemoteMachineInformationManagerFinalize(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static void ngcllRemoteMachineInformationInitializeMember(
    ngclRemoteMachineInformation_t *);
static void ngcllRemoteMachineInformationInitializePointer(
    ngclRemoteMachineInformation_t *);
static int ngcllRemoteMachineInformationRegisterExecutablePathInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
static int ngcllRemoteMachineInformationUnregisterExecutablePathInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
static int ngcllRemoteMachineInformationAppendNew(
    ngclContext_t *, char *, int *);
static int ngcllRemoteMachineInformationCheckRequireMPI(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    char *, int *, int *);
static int ngcllRemoteMachineInformationGetCopy(
    ngclContext_t *, char *, char *, ngclRemoteMachineInformation_t *, int *);
static int ngcllRemoteMachineInformationRelease(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
static int ngcllRemoteMachineInformationReplace(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngclRemoteMachineInformation_t *, int *);

/**
 * Register to the cache.
 * Information append at last of the list.
 */
int
ngcliRemoteMachineInformationCacheRegister(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    rmInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((rmInfo == NULL) ||
        (rmInfo->ngrmi_hostName == NULL) ||
        (rmInfo->ngrmi_gassScheme == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Remote Machine Information list.\n",
            fName);
        goto error;
    }
    listLocked = 1;

    /* Is Remote Machine Information available? */
    rmInfoMng = ngcliRemoteMachineInformationCacheGetWithTag(
        context, rmInfo->ngrmi_hostName, rmInfo->ngrmi_tagName, &subError);
    if (rmInfoMng != NULL) {

        /* Replace the Remote Machine Information */
        result = ngcllRemoteMachineInformationReplace(
            context, rmInfoMng, rmInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the Remote Machine Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Remote Machine Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (rmInfoMng == NULL) {
        rmInfoMng = ngcllRemoteMachineInformationConstruct(
            context, rmInfo, error);
        if (rmInfoMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Remote Machine Information.\n",
                fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliRemoteMachineInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Remote Machine Information list.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Unregister from the cache.
 * Information delete from the list.
 *
 * All informations are unregister from the cache, if NULL.
 */
int
ngcliRemoteMachineInformationCacheUnregister(
    ngclContext_t *context,
    char *hostName,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *curr;
    static const char fName[] = "ngcliRemoteMachineInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock RemoteMachineInformation list.\n", fName);
	return 0;
    }


    /* Get the data from the head of a list */
    curr = ngcliRemoteMachineInformationCacheGetNext(context, NULL, error);
    if (curr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: No Remote Machine Information was registered.\n", fName);
    }

    for (;curr != NULL;
        curr = ngcliRemoteMachineInformationCacheGetNext(
            context, NULL, error)) {

        if (hostName != NULL) {
            if (strcmp(curr->ngrmim_info.ngrmi_hostName, hostName) != 0) {
                continue;
            }
        }
    
        /* Destruct the data */
        result = ngcllRemoteMachineInformationDestruct(
            context, curr, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't destruct Remote Machine Information.\n", fName);
            goto error;
        }

    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
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
ngclRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGet(
    ngclContext_t *context,
    char *hostName,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *rmInfoMng, *foundRmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: hostname is NULL.\n", fName);
	return NULL;
    }

    foundRmInfoMng = NULL;

    /* Search given hostName as tag */
    rmInfoMng = context->ngc_rmInfo_head;
    for (; rmInfoMng != NULL; rmInfoMng = rmInfoMng->ngrmim_next) {
	if (rmInfoMng->ngrmim_info.ngrmi_tagName == NULL) {
	    continue;
	}
    
	if (strcmp(rmInfoMng->ngrmim_info.ngrmi_tagName, hostName) == 0) {
	    /* Found */
	    /** 
	     *  Pickup the last matched tag.
	     *  For the case of hostname differ for the tag,
	     *  after grpc_config_file_read_np().
	     */
	    foundRmInfoMng = rmInfoMng;
	}
    }
    if (foundRmInfoMng != NULL) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The Remote Machine Information for \"%s\""
            " was found by tag.\n", fName, hostName);
	return foundRmInfoMng;
    }

    /* Search given hostName as host name */
    rmInfoMng = context->ngc_rmInfo_head;
    for (; rmInfoMng != NULL; rmInfoMng = rmInfoMng->ngrmim_next) {
	assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);
	if (strcmp(rmInfoMng->ngrmim_info.ngrmi_hostName, hostName) == 0) {
	    /* Found */
	    return rmInfoMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Remote Machine Information is not found by host name \"%s\".\n",
	fName, hostName);
    return NULL;
}

/**
 * Get the information by host name and tag.
 * This function search the matched information both host name and tag.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGetWithTag(
    ngclContext_t *context,
    char *hostName,
    char *tagName,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGetWithTag";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: hostname is NULL.\n", fName);
	return NULL;
    }
    /* tagName will be both NULL or not */

    rmInfoMng = context->ngc_rmInfo_head;
    for (; rmInfoMng != NULL; rmInfoMng = rmInfoMng->ngrmim_next) {
	assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);
	if (strcmp(rmInfoMng->ngrmim_info.ngrmi_hostName, hostName) != 0) {
	    continue;
	}

	if (tagName == NULL) {
	    if (rmInfoMng->ngrmim_info.ngrmi_tagName == NULL) {
		/* Found */
		return rmInfoMng;
	    } else {
		continue;
	    }
	}

	assert(tagName != NULL);
	if (rmInfoMng->ngrmim_info.ngrmi_tagName == NULL) {
	    continue;
	}
    
	if (strcmp(rmInfoMng->ngrmim_info.ngrmi_tagName, tagName) == 0) {
	    /* Found */
	    return rmInfoMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Remote Machine Information is not found"
	" by host name \"%s\" tag name \"%s\".\n",
	fName, hostName, ((tagName != NULL) ? tagName : ""));
    return NULL;
}

/**
 * Get the next information.
 *
 * Return the information from the top of the list, if current is NULL.
 * Return the next information of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGetNext(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    if (current == NULL) {
	/* Return the first information */
	if (context->ngc_rmInfo_head != NULL) {
	    assert(context->ngc_rmInfo_tail != NULL);
            return context->ngc_rmInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngrmim_next != NULL) {
	    return current->ngrmim_next;
	}
    }

    /* The last information was reached */
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Remote Machine Information was reached.\n", fName);
    return NULL;
}

/**
 * Construct.
 */
static ngclRemoteMachineInformationManager_t *
ngcllRemoteMachineInformationConstruct(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    /* Allocate */
    rmInfoMng = ngcllRemoteMachineInformationManagerAllocate(context, error);
    if (rmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Machine Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllRemoteMachineInformationManagerInitialize(
        context, rmInfoMng, rmInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't initialize the Remote Machine Information.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclContextRegisterRemoteMachineInformation(
    	context, rmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Remote Machine Information for Ninf-G Context.\n",
	    fName);
	goto error;
    }

    /* Success */
    return rmInfoMng;

    /* Error occurred */
error:
    result = ngcllRemoteMachineInformationManagerFree(context, rmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for Remote Machine Information Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllRemoteMachineInformationDestruct(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Unregister */
    result = ngclContextUnregisterRemoteMachineInformation(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Remote Machine Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllRemoteMachineInformationManagerFinalize(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Remote Machine Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllRemoteMachineInformationManagerFree(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Remote Machine Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * Allocate.
 */
static ngclRemoteMachineInformationManager_t *
ngcllRemoteMachineInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    rmInfoMng =
	globus_libc_calloc(1, sizeof (ngclRemoteMachineInformationManager_t));
    if (rmInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Machine Information Manager.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return rmInfoMng;
}

/**
 * Deallocate.
 */
static int
ngcllRemoteMachineInformationManagerFree(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfo,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    globus_libc_free(rmInfo);

    /* Success */
    return 1;
}

/**
 * Allocate. (not Manager)
 */
ngclRemoteMachineInformation_t *
ngcliRemoteMachineInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclRemoteMachineInformation_t *rmInfo;
    static const char fName[] = "ngcliRemoteMachineInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    rmInfo =
	globus_libc_calloc(1, sizeof (ngclRemoteMachineInformation_t));
    if (rmInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Machine Information.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return rmInfo;
}

/**
 * Deallocate. (not Manager)
 */
int
ngcliRemoteMachineInformationFree(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteMachineInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(rmInfo);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllRemoteMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngclRemoteMachineInformationManager_t *rmInfoMng,
     ngclRemoteMachineInformation_t *rmInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(rmInfo != NULL);

    /* Copy to new information */
    result = ngcliRemoteMachineInformationCopy(
	context, rmInfo, &rmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the Remote Machine Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&rmInfoMng->ngrmim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Executable Path list */
    result = ngiRWlockInitialize(
	&rmInfoMng->ngrmim_rwlExecPath, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for list of Executable Path.\n",
	    fName);
	return 0;
    }

    /* Initialize the members */
    rmInfoMng->ngrmim_epInfo_head = NULL;
    rmInfoMng->ngrmim_epInfo_tail = NULL;
    rmInfoMng->ngrmim_mdsServer = NULL;
    rmInfoMng->ngrmim_hostDN = NULL;
    rmInfoMng->ngrmim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllRemoteMachineInformationManagerFinalize(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Unregister the pointer for the MDS Server */
    rmInfoMng->ngrmim_mdsServer = NULL;
    if (rmInfoMng->ngrmim_hostDN != NULL) {
        globus_libc_free(rmInfoMng->ngrmim_hostDN);
        rmInfoMng->ngrmim_hostDN = NULL;
    }

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(
	&rmInfoMng->ngrmim_rwlOwn, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Unregister Executable Path Information */
    result = ngcliExecutablePathInformationCacheUnregister(
	context, rmInfoMng, NULL, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister Executable Path information.\n", fName);
	return 0;
    }

    /* Destroy the Read/Write Lock for Executable Path list */
    result = ngiRWlockFinalize(
	&rmInfoMng->ngrmim_rwlExecPath, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock Executable Path.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclRemoteMachineInformationRelease(context,
	&rmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Remote Machine Information.\n", fName);
	return 0;
    }

    /* reset members */
    rmInfoMng->ngrmim_mdsServer = NULL;
    rmInfoMng->ngrmim_hostDN = NULL;
    rmInfoMng->ngrmim_epInfo_head = NULL;
    rmInfoMng->ngrmim_epInfo_tail = NULL;
    rmInfoMng->ngrmim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliRemoteMachineInformationCopy(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *src,
    ngclRemoteMachineInformation_t *dest,
    int *error)
{
    int i;
    int result;
    static const char fName[] = "ngcliRemoteMachineInformationCopy";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Initialize the members */
    ngcllRemoteMachineInformationInitializeMember(dest);

    /* Copy the members */
    *dest = *src;

    ngcllRemoteMachineInformationInitializePointer(dest);

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
                "for Remote Machine Information.\n", fName); \
            goto error; \
        } \
    } while(0)

    if (src->ngrmi_hostName != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_hostName);
    if (src->ngrmi_tagName != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_tagName);
    if (src->ngrmi_mdsServer != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_mdsServer);
    if (src->ngrmi_mdsTag != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_mdsTag);
    if (src->ngrmi_invokeServerType != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_invokeServerType);
    if (src->ngrmi_jobManager != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_jobManager);
    if (src->ngrmi_subject != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_subject);
    if (src->ngrmi_clientHostName != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_clientHostName);
    if (src->ngrmi_jobQueue != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_jobQueue);
    if (src->ngrmi_jobProject != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_jobProject);
    NGL_ALLOCATE(src, dest, ngrmi_gassScheme);
    if (src->ngrmi_workDirectory != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_workDirectory);

    /* Copy Invoke Server options */
    if (src->ngrmi_invokeServerNoptions > 0) {
	dest->ngrmi_invokeServerOptions = globus_libc_calloc(
	    src->ngrmi_invokeServerNoptions, sizeof (char *));
	if (dest->ngrmi_invokeServerOptions == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't allocate the storage for Remote Machine Information.\n",
		fName);
	    goto error;
	}
	/* copy all of elements */
	for (i = 0; i < src->ngrmi_invokeServerNoptions; i++) {
	    NGL_ALLOCATE(src, dest, ngrmi_invokeServerOptions[i]);
	}
    }

    /* Copy WS GRAM RSL Extensions */
    if (src->ngrmi_rslExtensionsSize > 0) {
	dest->ngrmi_rslExtensions = globus_libc_calloc(
	    src->ngrmi_rslExtensionsSize, sizeof (char *));
	if (dest->ngrmi_rslExtensions == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't allocate the storage for Remote Machine Information.\n",
		fName);
	    return 0;
	}
	/* copy all of elements */
	for (i = 0; i < src->ngrmi_rslExtensionsSize; i++) {
	    NGL_ALLOCATE(src, dest, ngrmi_rslExtensions[i]);
	}
    }

    /* log params */
    result = ngLogInformationCopy(
        &src->ngrmi_commLogInfo, &dest->ngrmi_commLogInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't copy Log Information.\n",
            fName);
        goto error;
    }

    /* debug params */
    if (src->ngrmi_debug.ngdi_display != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_debug.ngdi_display);
    if (src->ngrmi_debug.ngdi_terminalPath != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_debug.ngdi_terminalPath);
    if (src->ngrmi_debug.ngdi_debuggerPath != NULL)
	NGL_ALLOCATE(src, dest, ngrmi_debug.ngdi_debuggerPath);

    /* Copy environment */
    if (src->ngrmi_nEnvironments > 0) {
	dest->ngrmi_environment = globus_libc_calloc(
	    src->ngrmi_nEnvironments, sizeof (char *));
	if (dest->ngrmi_environment == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't allocate the storage for Remote Machine Information.\n",
		fName);
	    goto error;
	}
	/* copy all of elements */
	for (i = 0; i < src->ngrmi_nEnvironments; i++) {
	    NGL_ALLOCATE(src, dest, ngrmi_environment[i]);
	}
    }
#undef NGL_ALLOCATE

    if (src->ngrmi_nEnvironments > 0) {
	/* check all of elements */
	for (i = 0; i < src->ngrmi_nEnvironments; i++) {
	    if (dest->ngrmi_environment[i] == NULL) {
		NGI_SET_ERROR(error, NG_ERROR_MEMORY);
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't allocate the storage for Remote Machine Information.\n",
		fName);
		goto error;
	    }
	}
    }

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclRemoteMachineInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Machine Information.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclRemoteMachineInformationRelease(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteMachineInformationRelease";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRemoteMachineInformationRelease(context,
	rmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteMachineInformationRelease(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int i;
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationRelease";

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Deallocate the members */
    if (rmInfo->ngrmi_hostName != NULL)
	globus_libc_free(rmInfo->ngrmi_hostName);
    if (rmInfo->ngrmi_tagName != NULL)
	globus_libc_free(rmInfo->ngrmi_tagName);
    if (rmInfo->ngrmi_mdsServer != NULL)
	globus_libc_free(rmInfo->ngrmi_mdsServer);
    if (rmInfo->ngrmi_mdsTag != NULL)
	globus_libc_free(rmInfo->ngrmi_mdsTag);
    if (rmInfo->ngrmi_invokeServerType != NULL)
	globus_libc_free(rmInfo->ngrmi_invokeServerType);
    if (rmInfo->ngrmi_jobManager != NULL)
	globus_libc_free(rmInfo->ngrmi_jobManager);
    if (rmInfo->ngrmi_subject != NULL)
	globus_libc_free(rmInfo->ngrmi_subject);
    if (rmInfo->ngrmi_clientHostName != NULL)
	globus_libc_free(rmInfo->ngrmi_clientHostName);
    if (rmInfo->ngrmi_jobQueue != NULL)
	globus_libc_free(rmInfo->ngrmi_jobQueue);
    if (rmInfo->ngrmi_jobProject != NULL)
	globus_libc_free(rmInfo->ngrmi_jobProject);
    if (rmInfo->ngrmi_gassScheme != NULL)
        globus_libc_free(rmInfo->ngrmi_gassScheme);
    if (rmInfo->ngrmi_workDirectory != NULL)
	globus_libc_free(rmInfo->ngrmi_workDirectory);

    /* Invoke Server options */
    if (rmInfo->ngrmi_invokeServerOptions != NULL) {
	for (i = 0; i < rmInfo->ngrmi_invokeServerNoptions; i++) {
	    if (rmInfo->ngrmi_invokeServerOptions[i] != NULL)
	        globus_libc_free(rmInfo->ngrmi_invokeServerOptions[i]);
	}
	globus_libc_free(rmInfo->ngrmi_invokeServerOptions);
    }

    /* WS GRAM RSL Extensions */
    if (rmInfo->ngrmi_rslExtensions != NULL) {
	for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
	    globus_libc_free(rmInfo->ngrmi_rslExtensions[i]);
	}
	globus_libc_free(rmInfo->ngrmi_rslExtensions);
    }

    /* log params */
    result = ngLogInformationRelease(&rmInfo->ngrmi_commLogInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't release Log Information.\n",
            fName);
        return 0;
    }

    /* debug params */
    if (rmInfo->ngrmi_debug.ngdi_display != NULL)
	globus_libc_free(rmInfo->ngrmi_debug.ngdi_display);
    if (rmInfo->ngrmi_debug.ngdi_terminalPath != NULL)
	globus_libc_free(rmInfo->ngrmi_debug.ngdi_terminalPath);
    if (rmInfo->ngrmi_debug.ngdi_debuggerPath != NULL)
	globus_libc_free(rmInfo->ngrmi_debug.ngdi_debuggerPath);

    /* environment */
    if (rmInfo->ngrmi_environment != NULL) {
	for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
	    if (rmInfo->ngrmi_environment[i] != NULL)
	        globus_libc_free(rmInfo->ngrmi_environment[i]);
	}
	globus_libc_free(rmInfo->ngrmi_environment);
    }

    /* Initialize the members */
    ngcllRemoteMachineInformationInitializeMember(rmInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliRemoteMachineInformationInitialize(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteMachineInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    ngcllRemoteMachineInformationInitializeMember(rmInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllRemoteMachineInformationInitializeMember(
    ngclRemoteMachineInformation_t *rmInfo)
{
    /* Initialize the members */
    ngcllRemoteMachineInformationInitializePointer(rmInfo);
    ngLogInformationInitialize(&rmInfo->ngrmi_commLogInfo);
    rmInfo->ngrmi_portNo = 0;
    rmInfo->ngrmi_invokeServerNoptions = 0;
    rmInfo->ngrmi_mpiNcpus = 0;
    rmInfo->ngrmi_crypt = NG_PROTOCOL_CRYPT_NONE;
    rmInfo->ngrmi_protocol = NG_PROTOCOL_TYPE_XML;
    rmInfo->ngrmi_forceXDR = 0;
    rmInfo->ngrmi_jobStartTimeout = 0;
    rmInfo->ngrmi_jobEndTimeout = 0;
    rmInfo->ngrmi_jobMaxTime = 0;
    rmInfo->ngrmi_jobMaxWallTime = 0;
    rmInfo->ngrmi_jobMaxCpuTime = 0;
    rmInfo->ngrmi_jobHostCount = 0;
    rmInfo->ngrmi_jobMinMemory = 0;
    rmInfo->ngrmi_jobMaxMemory = 0;
    rmInfo->ngrmi_rslExtensionsSize = 0;
    rmInfo->ngrmi_heartBeat = 0;
    rmInfo->ngrmi_heartBeatTimeoutCount = 0;
    rmInfo->ngrmi_heartBeatTimeoutCountOnTransfer = 0;
    rmInfo->ngrmi_redirectEnable = 0;
    rmInfo->ngrmi_retryInfo.ngcri_count = 0;
    rmInfo->ngrmi_retryInfo.ngcri_interval = 0;
    rmInfo->ngrmi_retryInfo.ngcri_increase = 0.0;
    rmInfo->ngrmi_retryInfo.ngcri_useRandom = 0;
    rmInfo->ngrmi_compressionType = NG_COMPRESSION_TYPE_RAW;
    rmInfo->ngrmi_compressionThresholdNbytes = 0;
    rmInfo->ngrmi_argumentBlockSize = 0;
    rmInfo->ngrmi_coreDumpSize = 0;
    rmInfo->ngrmi_debug.ngdi_enable = 0;
    rmInfo->ngrmi_debugBusyLoop = 0;
    rmInfo->ngrmi_nEnvironments = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllRemoteMachineInformationInitializePointer(
    ngclRemoteMachineInformation_t *rmInfo)
{
    /* Initialize the pointers */
    rmInfo->ngrmi_hostName = NULL;
    rmInfo->ngrmi_tagName = NULL;
    rmInfo->ngrmi_mdsServer = NULL;
    rmInfo->ngrmi_mdsTag = NULL;
    rmInfo->ngrmi_invokeServerType = NULL;
    rmInfo->ngrmi_invokeServerOptions = NULL;
    rmInfo->ngrmi_gassScheme = NULL;
    rmInfo->ngrmi_jobManager = NULL;
    rmInfo->ngrmi_subject = NULL;
    rmInfo->ngrmi_clientHostName = NULL;
    rmInfo->ngrmi_jobQueue = NULL;
    rmInfo->ngrmi_jobProject = NULL;
    rmInfo->ngrmi_rslExtensions = NULL;
    rmInfo->ngrmi_workDirectory = NULL;
    rmInfo->ngrmi_commLogInfo.ngli_filePath = NULL;
    rmInfo->ngrmi_commLogInfo.ngli_suffix = NULL;
    rmInfo->ngrmi_debug.ngdi_terminalPath = NULL;
    rmInfo->ngrmi_debug.ngdi_display = NULL;
    rmInfo->ngrmi_debug.ngdi_debuggerPath = NULL;
    rmInfo->ngrmi_environment = NULL;
}

/**
 * Register the Executable Path Information to Remote Machine Information
 */
int
ngclRemoteMachineInformationRegisterExecutablePathInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteMachineInformationRegisterExecutablePathInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRemoteMachineInformationRegisterExecutablePathInformation(
	context, rmInfoMng, epInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteMachineInformationRegisterExecutablePathInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *exist;
    static const char fName[] = "ngcllRemoteMachineInformationRegisterExecutablePathInformation";

    /* Check the arguments */
    if ((rmInfoMng == NULL) || (epInfoMng == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(rmInfoMng,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write lock the list of Executable Path Information.\n",
	    fName);
	return 0;
    }

    /* Is the same path name already registered? */
    exist = ngcliExecutablePathInformationCacheGet(context, rmInfoMng,
	epInfoMng->ngepim_info.ngepi_className, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable Path Information of the classname \"%s\" on \"%s\" has already registered.\n",
	    fName, epInfoMng->ngepim_info.ngepi_className,
	    epInfoMng->ngepim_info.ngepi_hostName);
	goto error;
    }

    /* Append at last of the list */
    epInfoMng->ngepim_next = NULL;
    if (rmInfoMng->ngrmim_epInfo_head == NULL) {
    	/* Information is not registered */
	rmInfoMng->ngrmim_epInfo_head = epInfoMng;
	rmInfoMng->ngrmim_epInfo_tail = epInfoMng;
    } else {
    	/* Any information is registered */
	rmInfoMng->ngrmim_epInfo_tail->ngepim_next = epInfoMng;
	rmInfoMng->ngrmim_epInfo_tail = epInfoMng;
    }

    /* Unlock the list */
    result = ngcliExecutablePathInformationListWriteUnlock(rmInfoMng,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Executable Path Information.\n",
	    fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliExecutablePathInformationListWriteUnlock(rmInfoMng,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Executable Path Information.\n",
	    fName);
    	return 0;
    }

    return 0;
}

/**
 * Unregister the Executable Path Information to Remote Machine Information
 */
int
ngclRemoteMachineInformationUnregisterExecutablePathInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteMachineInformationUnregisterExecutablePathInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRemoteMachineInformationUnregisterExecutablePathInformation(
	context, rmInfoMng, epInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteMachineInformationUnregisterExecutablePathInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    ngcliExecutablePathInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcllRemoteMachineInformationUnregisterExecutablePathInformation";

    /* Check the arguments */
    if ((rmInfoMng == NULL) || (epInfoMng == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &rmInfoMng->ngrmim_epInfo_head;
    curr = rmInfoMng->ngrmim_epInfo_head;
    for (; curr != NULL; curr = curr->ngepim_next) {
	if (curr == epInfoMng) {
            /* Found */
	    *prevPtr = curr->ngepim_next;
            if (curr->ngepim_next == NULL) {
                rmInfoMng->ngrmim_epInfo_tail = prev;
            }
	    epInfoMng->ngepim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngepim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: Executable Path Information of class \"%s\" on \"%s\" has not registered.\n",
	fName, epInfoMng->ngepim_info.ngepi_className,
	fName, epInfoMng->ngepim_info.ngepi_hostName);

    return 0;
}

/**
 * GetCopy
 */
int
ngclRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    char *className,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteMachineInformationGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result != 1) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRemoteMachineInformationGetCopy(context,
	hostName, className, rmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    char *className,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    int requireMPI;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationGetCopy";

    /* Check the arguments */
    if ((hostName == NULL) || (className == NULL) || (rmInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadLock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Machine Information.\n", fName);
        return 0;
    }
 
    /* Find Remote Machine Information */
    rmInfoMng = ngcliRemoteMachineInformationCacheGet(context, hostName, error);
    if (rmInfoMng == NULL) {

        /* Unlock the Remote Machine Information */
        result = ngcliRemoteMachineInformationListReadUnlock(
            context, context->ngc_log, error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* Create Remote Machine Information from Default */
        result = ngcllRemoteMachineInformationAppendNew(
            context, hostName, error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't append the Remote Machine Information.\n", fName);
            return 0;
        }

        /* Lock the Remote Machine Information */
        result = ngcliRemoteMachineInformationListReadLock(
            context, context->ngc_log, error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the list of Remote Machine Information.\n",
                fName);
            return 0;
        }

        /* Find Remote Machine Information again */
        rmInfoMng = ngcliRemoteMachineInformationCacheGet(
            context, hostName, error);
        if (rmInfoMng == NULL) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Remote Machine Information.\n", fName);
            goto error;
        }
    }

    /* Check if the MPI path and cpus are needed */
    result = ngcllRemoteMachineInformationCheckRequireMPI(
        context, rmInfoMng, className, &requireMPI, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the Remote Machine Information MPI setting.\n",
            fName);
        goto error;
    }

    /* If incomplete data was found, then fill it from MDS */
    if (requireMPI != 0) {

        /* Data for MPI is incomplete. thus, retrieve MDS to set. */
        result = ngcliMDSaccessRemoteMachineInformationGet(
            context, rmInfoMng, error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Remote Machine Information.\n", fName);
            goto error;
        }

        assert(rmInfoMng->ngrmim_info.ngrmi_mpiNcpus > 0);
    }

    /* Copy the Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(context,
                    &rmInfoMng->ngrmim_info, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't copy the Remote Machine Information.\n", fName);
        goto error;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Failed */
    return 0;
}

static int
ngcllRemoteMachineInformationAppendNew(
    ngclContext_t *context,
    char *hostName,
    int *error)
{
    int result;
    ngclRemoteMachineInformation_t *rmInfo;
    static const char fName[] = "ngcllRemoteMachineInformationAppendNew";

    /* Check the arguments */
    assert(context != NULL);
    assert(hostName != NULL);

    /* Allocate RemoteMachineInformation */
    rmInfo = ngcliRemoteMachineInformationAllocate(context, error);
    if (rmInfo == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for "
            "the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Initialize RemoteMachineInformation */
    result = ngcliRemoteMachineInformationInitialize(context, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Get DefaultRemoteMachineInformation */
    result = ngclDefaultRemoteMachineInformationGetCopy(
        context, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Default Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Set hostname */
    assert(rmInfo->ngrmi_hostName == NULL);
    rmInfo->ngrmi_hostName = strdup(hostName);
    if (rmInfo->ngrmi_hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string.\n",
            fName);
        return 0;
    }

    /* Register to context */
    result = ngcliRemoteMachineInformationCacheRegister(context, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Release RemoteMachineInformation */
    result = ngclRemoteMachineInformationRelease(context, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Free RemoteMachineInformation */
    result = ngcliRemoteMachineInformationFree(context, rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't free the Remote Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngcllRemoteMachineInformationCheckRequireMPI(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *requireMPI,
    int *error)
{
    int result;
    int known, need, subError;
    ngLog_t *log;
    ngclRemoteClassInformationManager_t *rcInfoMng;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationCheckRequireMPI";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(requireMPI != NULL);

    log = context->ngc_log;
    *requireMPI = 0;

    /* Major case (defined in local_ldif file) */
    if (rmInfoMng->ngrmim_info.ngrmi_mpiNcpus > 0) {
        /* MPI data is available */

        *requireMPI = 0;

        /* Success */
        return 1;
    }

    /* Check if the MPI settings are required */
    need = 0;
    known = 0;

    /* Check from ExecutablePathInformation */
    result = ngcliExecutablePathInformationListReadLock(
        rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the list of Executable Path Information.\n",
            fName);
        return 0;
    }

    epInfoMng = ngcliExecutablePathInformationCacheGet(context, rmInfoMng,
        className, &subError);
    if (epInfoMng != NULL) {
        if (epInfoMng->ngepim_info.ngepi_backend == NG_BACKEND_NORMAL) {
            /* default value and just ignore */
            need = 0;
            known = 0;

        } else if ((epInfoMng->ngepim_info.ngepi_backend == NG_BACKEND_MPI) ||
            (epInfoMng->ngepim_info.ngepi_backend == NG_BACKEND_BLACS)) {
            /* <FUNCTION_INFO> backend was explicitly specified */
            need = 1;
            known = 1;

        } else {
            need = 0;
            known = 1;
        }
    }

    result = ngcliExecutablePathInformationListReadUnlock(
        rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the list of Executable Path Information.\n",
            fName);
        return 0;
    }

    if (known == 0) {
        /* Check from RemoteClassInformation */

        result = ngcliRemoteClassInformationListReadLock(
            context, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read lock the list of Remote Class Information.\n",
                fName);
            return 0;
        }

        rcInfoMng = ngcliRemoteClassInformationCacheGet(
            context, className, &subError);
        if (rcInfoMng != NULL) {
            if (rcInfoMng->ngrcim_info.ngrci_backend == NG_BACKEND_NORMAL) {
                need = 0;
                known = 1;

            } else
                if ((rcInfoMng->ngrcim_info.ngrci_backend == NG_BACKEND_MPI) ||
                (rcInfoMng->ngrcim_info.ngrci_backend == NG_BACKEND_BLACS)) {
                need = 1;
                known = 1;

            } else {
                need = 0;
                known = 1;
            }
        }

        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read unlock the list of Remote Class Information.\n",
                fName);
            return 0;
        }
    }

    if ((known == 0) || (need == 0)) {
        /* MPI data is unnecessary */
        *requireMPI = 0;

        /* Success */
        return 1;
    }

    /* Require MPI data */

    assert(rmInfoMng->ngrmim_info.ngrmi_mpiNcpus <= 0); /* Already checked */

    if ((epInfoMng == NULL) || (epInfoMng->ngepim_info.ngepi_mpiNcpus <= 0)) {
        /* MPI data is necessary */
        *requireMPI = 1;

        /* Success */
        return 1;
    }

    *requireMPI = 0;

    /* Success */
    return 1;
}

/**
 * Replace the Remote Machine Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllRemoteMachineInformationReplace(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *dstRmInfoMng,
    ngclRemoteMachineInformation_t *srcRmInfo,
    int *error)
{
    ngLog_t *log;
    int result, rmLocked;
    static const char fName[] = "ngcllRemoteMachineInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstRmInfoMng != NULL);
    assert(srcRmInfo != NULL);

    log = context->ngc_log;
    rmLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the Remote Machine Information for \"%s\".\n",
        fName, srcRmInfo->ngrmi_hostName);

    /* Lock */
    result = ngcliRemoteMachineInformationWriteLock(
        dstRmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Remote Machine Information.\n",
            fName);
        goto error;
    }
    rmLocked = 1;

    /* Release the Remote Machine Information */
    result = ngclRemoteMachineInformationRelease(
        context, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Copy the Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(
        context, srcRmInfo, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Unlock */
    result = ngcliRemoteMachineInformationWriteUnlock(
        dstRmInfoMng, log, error);
    rmLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (rmLocked != 0) {
        result = ngcliRemoteMachineInformationWriteUnlock(
            dstRmInfoMng, log, NULL);
        rmLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Remote Machine Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;
}

