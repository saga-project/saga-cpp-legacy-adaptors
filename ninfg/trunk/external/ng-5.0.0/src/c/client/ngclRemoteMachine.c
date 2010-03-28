/*
 * $RCSfile: ngclRemoteMachine.c,v $ $Revision: 1.18 $ $Date: 2008/02/06 08:11:50 $
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
 * Remote Machine Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclRemoteMachine.c,v $ $Revision: 1.18 $ $Date: 2008/02/06 08:11:50 $")

/**
 * Prototype declaration of static functions.
 */
static ngcliRemoteMachineInformationManager_t *
ngcllRemoteMachineInformationConstruct(
    ngclContext_t *,
    ngclRemoteMachineInformation_t *,
    int *);
static int ngcllRemoteMachineInformationDestruct(
    ngclContext_t *,
    ngcliRemoteMachineInformationManager_t *,
    int *);
static int ngcllRemoteMachineInformationManagerInitialize(
     ngclContext_t *,
     ngcliRemoteMachineInformationManager_t *,
     ngclRemoteMachineInformation_t *,
     int *);
static int ngcllRemoteMachineInformationManagerFinalize(
    ngclContext_t *,
    ngcliRemoteMachineInformationManager_t *,
    int *);
static void ngcllRemoteMachineInformationInitializeMember(
    ngclRemoteMachineInformation_t *);
static void ngcllRemoteMachineInformationInitializePointer(
    ngclRemoteMachineInformation_t *);
static int ngcllRemoteMachineInformationAppendNew(
    ngclContext_t *, char *, int *);
static int ngcllRemoteMachineInformationGetCopy(
    ngclContext_t *, char *, ngclRemoteMachineInformation_t *, int *);
static int ngcllRemoteMachineInformationRelease(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
static int ngcllRemoteMachineInformationReplace(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *,
    ngclRemoteMachineInformation_t *, int *);

static ngclMPInCPUsInformation_t *ngcllMPInCPUsInformationConstruct(
    char *, int, ngLog_t *, int *);
static int ngcllMPInCPUsInformationDestruct(
    ngclMPInCPUsInformation_t *, ngLog_t *, int *);
static int ngcllMPInCPUsInformationInitialize(
    ngclMPInCPUsInformation_t *, char *, int, ngLog_t *, int *);
static int ngcllMPInCPUsInformationFinalize(
    ngclMPInCPUsInformation_t *, ngLog_t *, int *);
static void ngcllMPInCPUsInformationInitializeMember(
    ngclMPInCPUsInformation_t *);
static int ngcllRemoteMachineInformationRegisterMPInCPUs(
    ngclRemoteMachineInformation_t *, ngclMPInCPUsInformation_t *, 
    ngLog_t *, int *);
static int ngcllRemoteMachineInformationUnregisterAllMPInCPUs(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);

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
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    rmInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((rmInfo == NULL) ||
        (rmInfo->ngrmi_hostName == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Remote Machine Information list.\n"); 
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
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't replace the Remote Machine Information.\n"); 
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Remote Machine Information list.\n"); 
        goto error;
    }

    /* Construct */
    if (rmInfoMng == NULL) {
        rmInfoMng = ngcllRemoteMachineInformationConstruct(
            context, rmInfo, error);
        if (rmInfoMng == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Remote Machine Information.\n"); 
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
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Remote Machine Information list.\n"); 
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
    ngcliRemoteMachineInformationManager_t *curr;
    static const char fName[] = "ngcliRemoteMachineInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock RemoteMachineInformation list.\n"); 
	return 0;
    }


    /* Get the data from the head of a list */
    curr = ngcliRemoteMachineInformationCacheGetNext(context, NULL, error);
    if (curr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "No Remote Machine Information was registered.\n"); 
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
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct Remote Machine Information.\n"); 
            goto error;
        }

    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
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
ngcliRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGet(
    ngclContext_t *context,
    char *hostName,
    int *error)
{
    int result;
    ngcliRemoteMachineInformationManager_t *rmInfoMng, *foundRmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostname is NULL.\n"); 
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
	ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The Remote Machine Information for \"%s\""
            " was found by tag.\n", hostName); 
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
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Machine Information is not found by host name \"%s\".\n",
        hostName); 
    return NULL;
}

/**
 * Get the information by host name and tag.
 * This function search the matched information both host name and tag.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGetWithTag(
    ngclContext_t *context,
    char *hostName,
    char *tagName,
    int *error)
{
    int result;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGetWithTag";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostname is NULL.\n"); 
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
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Machine Information is not found"
        " by host name \"%s\" tag name \"%s\".\n",
        hostName, ((tagName != NULL) ? tagName : "")); 
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
ngcliRemoteMachineInformationManager_t *
ngcliRemoteMachineInformationCacheGetNext(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteMachineInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Remote Machine Information was reached.\n"); 
    return NULL;
}

/**
 * Construct.
 */
static ngcliRemoteMachineInformationManager_t *
ngcllRemoteMachineInformationConstruct(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    /* Allocate */
    rmInfoMng = NGI_ALLOCATE(ngcliRemoteMachineInformationManager_t,
        context->ngc_log, error);
    if (rmInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Remote Machine Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllRemoteMachineInformationManagerInitialize(
        context, rmInfoMng, rmInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Remote Machine Information.\n"); 
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterRemoteMachineInformation(
    	context, rmInfoMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Remote Machine Information for Ninf-G Context.\n"); 
	goto error;
    }

    /* Success */
    return rmInfoMng;

    /* Error occurred */
error:
    result = NGI_DEALLOCATE(ngcliRemoteMachineInformationManager_t,
        rmInfoMng, context->ngc_log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the storage for Remote Machine Information Manager.\n"); 
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterRemoteMachineInformation(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Remote Machine Information.\n"); 
	return 0;
    }

    /* Finalize */
    result = ngcllRemoteMachineInformationManagerFinalize(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Remote Machine Information.\n"); 
	return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliRemoteMachineInformationManager_t,

    	rmInfoMng, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Remote Machine Information.\n"); 
	return 0;
    }

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
    ngLog_t *log;
    ngclRemoteMachineInformation_t *rmInfo;
    static const char fName[] = "ngcliRemoteMachineInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Allocate new storage */
    rmInfo =
       ngiCalloc(1, sizeof (ngclRemoteMachineInformation_t), log, error);
    if (rmInfo == NULL) {
       ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
           "Can't allocate the storage for Remote Machine Information.\n"); 
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
    ngLog_t *log;
    static const char fName[] = "ngcliRemoteMachineInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngiFree(rmInfo, log, error);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllRemoteMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngcliRemoteMachineInformationManager_t *rmInfoMng,
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Machine Information.\n"); 
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&rmInfoMng->ngrmim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Initialize the members */
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllRemoteMachineInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    log = context->ngc_log;

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(
	&rmInfoMng->ngrmim_rwlOwn, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destroy Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Release the information */
    result = ngclRemoteMachineInformationRelease(context,
	&rmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
	return 0;
    }

    /* reset members */
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
    ngLog_t *log;
    ngclMPInCPUsInformation_t *ncpuInfo = NULL;
    static const char fName[] = "ngcliRemoteMachineInformationCopy";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Initialize the members */
    ngcllRemoteMachineInformationInitializeMember(dest);

    /* Copy the members */
    *dest = *src;

    SLIST_INIT(&dest->ngrmi_mpiNcpusList);
    ngcllRemoteMachineInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        if ((src)->member != NULL) { \
            (dest)->member = ngiStrdup((src)->member, log, error); \
            if ((dest)->member == NULL) { \
                ngclLogErrorContext(context, \
                    NG_LOGCAT_NINFG_PURE, fName,  \
                    "Can't allocate the storage " \
                    "for Remote Machine Information.\n"); \
                goto error; \
            } \
        } \
    } while(0)

#define NGL_COPY_STRING_ARRAY(src, dest, member, nMember) \
    do { \
        if ((src)->nMember > 0) { \
            (dest)->member = ngiCalloc( \
                (src)->nMember, sizeof (char *), log, error); \
            if ((dest)->member == NULL) { \
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName, \
                    "Can't allocate the storage for" \
                    " Remote Machine Information.\n"); \
                goto error; \
            } \
            for (i = 0; i < (src)->nMember; i++) { \
                NGL_COPY_STRING(src, dest, member[i]);\
            } \
        } \
    } while (0)
    
    NGL_COPY_STRING(src, dest, ngrmi_hostName); 
    NGL_COPY_STRING(src, dest, ngrmi_tagName);
    NGL_COPY_STRING(src, dest, ngrmi_invokeServerType);
    NGL_COPY_STRING(src, dest, ngrmi_commProxyType);
    NGL_COPY_STRING(src, dest, ngrmi_commProxyPath);
    NGL_COPY_STRING(src, dest, ngrmi_infoServiceTag);
    NGL_COPY_STRING(src, dest, ngrmi_jobManager);
    NGL_COPY_STRING(src, dest, ngrmi_subject);
    NGL_COPY_STRING(src, dest, ngrmi_clientHostName);
    NGL_COPY_STRING(src, dest, ngrmi_jobQueue);
    NGL_COPY_STRING(src, dest, ngrmi_jobProject);
    NGL_COPY_STRING(src, dest, ngrmi_workDirectory);

    NGL_COPY_STRING_ARRAY(src, dest,
        ngrmi_invokeServerOptions, ngrmi_invokeServerNoptions);
    NGL_COPY_STRING_ARRAY(src, dest,
        ngrmi_commProxyOptions, ngrmi_commProxyNoptions);
    NGL_COPY_STRING_ARRAY(src, dest,
        ngrmi_rslExtensions, ngrmi_rslExtensionsSize);

    /* log params */
    result = ngiLogInformationCopy(
        &dest->ngrmi_commLogInfo, &src->ngrmi_commLogInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy Log Information.\n"); 
        goto error;
    }

    /* debug params */
    NGL_COPY_STRING(src, dest, ngrmi_debug.ngdi_display);
    NGL_COPY_STRING(src, dest, ngrmi_debug.ngdi_terminalPath);
    NGL_COPY_STRING(src, dest, ngrmi_debug.ngdi_debuggerPath);

    NGL_COPY_STRING_ARRAY(src, dest, ngrmi_environment, ngrmi_nEnvironments);

#undef NGL_COPY_STRING
#undef NGL_COPY_STRING_ARRAY

    if (src->ngrmi_nEnvironments > 0) {
	/* check all of elements */
	for (i = 0; i < src->ngrmi_nEnvironments; i++) {
	    if (dest->ngrmi_environment[i] == NULL) {
		NGI_SET_ERROR(error, NG_ERROR_MEMORY);
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't allocate the storage for Remote Machine Information.\n"); 
		goto error;
	    }
	}
    }

    SLIST_FOREACH(ncpuInfo, &src->ngrmi_mpiNcpusList, ngmni_entry) {
        result = ngcliRemoteMachineInformationAppendMPInCPUs(
            dest, ncpuInfo->ngmni_className, ncpuInfo->ngmni_nCPUs, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the MPI number of CPUs Information.\n"); 
            goto error;
        }
    }

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclRemoteMachineInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
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
    ngLog_t *log;
    int ret = 1;
    static const char fName[] = "ngcllRemoteMachineInformationRelease";

    /* Check the arguments */
    assert(context != NULL);

    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Deallocate the members */
    ngiFree(rmInfo->ngrmi_hostName,                    log, error);
    ngiFree(rmInfo->ngrmi_tagName,                     log, error);
    ngiFree(rmInfo->ngrmi_invokeServerType,            log, error);
    ngiFree(rmInfo->ngrmi_commProxyType,               log, error);
    ngiFree(rmInfo->ngrmi_commProxyPath,               log, error);
    ngiFree(rmInfo->ngrmi_infoServiceTag,              log, error);
    ngiFree(rmInfo->ngrmi_jobManager,                  log, error);
    ngiFree(rmInfo->ngrmi_subject,                     log, error);
    ngiFree(rmInfo->ngrmi_clientHostName,              log, error);
    ngiFree(rmInfo->ngrmi_jobQueue,                    log, error);
    ngiFree(rmInfo->ngrmi_jobProject,                  log, error);
    ngiFree(rmInfo->ngrmi_workDirectory,               log, error);

    /* Invoke Server options */
    if (rmInfo->ngrmi_invokeServerOptions != NULL) {
	for (i = 0; i < rmInfo->ngrmi_invokeServerNoptions; i++) {
	    if (rmInfo->ngrmi_invokeServerOptions[i] != NULL)
	        ngiFree(rmInfo->ngrmi_invokeServerOptions[i], log, error);
	}
	ngiFree(rmInfo->ngrmi_invokeServerOptions, log, error);
    }

    /* Remote Communication Proxy options */
    if (rmInfo->ngrmi_commProxyOptions != NULL) {
	for (i = 0; i < rmInfo->ngrmi_commProxyNoptions; i++) {
	    if (rmInfo->ngrmi_commProxyOptions[i] != NULL)
	        ngiFree(rmInfo->ngrmi_commProxyOptions[i], log, error);
	}
	ngiFree(rmInfo->ngrmi_commProxyOptions, log, error);
    }

    /* WS GRAM RSL Extensions */
    if (rmInfo->ngrmi_rslExtensions != NULL) {
	for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
	    ngiFree(rmInfo->ngrmi_rslExtensions[i], log, error);
	}
	ngiFree(rmInfo->ngrmi_rslExtensions, log, error);
    }

    /* log params */
    result = ngiLogInformationFinalize(&rmInfo->ngrmi_commLogInfo, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release Log Information.\n"); 
        error = NULL;
        ret = 0;
    }

    /* debug params */
    ngiFree(rmInfo->ngrmi_debug.ngdi_display, log, error);
    ngiFree(rmInfo->ngrmi_debug.ngdi_terminalPath, log, error);
    ngiFree(rmInfo->ngrmi_debug.ngdi_debuggerPath, log, error);

    /* environment */
    if (rmInfo->ngrmi_environment != NULL) {
	for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
	    if (rmInfo->ngrmi_environment[i] != NULL)
	        ngiFree(rmInfo->ngrmi_environment[i], log, error);
	}
	ngiFree(rmInfo->ngrmi_environment, log, error);
    }

    result = ngcllRemoteMachineInformationUnregisterAllMPInCPUs(
        context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unregister MPI number of CPUs informations.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Initialize the members */
    ngcllRemoteMachineInformationInitializeMember(rmInfo);

    return ret;
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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllRemoteMachineInformationInitializeMember(rmInfo);

    result = ngiLogInformationInitialize(&rmInfo->ngrmi_commLogInfo,
        context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the communication log information.\n"); 
        return 0;
    }

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

    ngiLogInformationInitializeMember(&rmInfo->ngrmi_commLogInfo);

    rmInfo->ngrmi_portNo = 0;
    rmInfo->ngrmi_invokeServerNoptions = 0;
    rmInfo->ngrmi_commProxyStaging = 0;
    rmInfo->ngrmi_commProxyBufferSize = 0;
    rmInfo->ngrmi_commProxyNoptions = 0;
    rmInfo->ngrmi_mpiNcpus = 0;
    SLIST_INIT(&rmInfo->ngrmi_mpiNcpusList);
    rmInfo->ngrmi_keepConnect = 0;
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
    rmInfo->ngrmi_redirectEnable = 0;
    rmInfo->ngrmi_tcpNodelay = 0;
    rmInfo->ngrmi_retryInfo.ngcri_count = 0;
    rmInfo->ngrmi_retryInfo.ngcri_interval = 0;
    rmInfo->ngrmi_retryInfo.ngcri_increase = 0.0;
    rmInfo->ngrmi_retryInfo.ngcri_useRandom = 0;
    rmInfo->ngrmi_compressionType = NG_COMPRESSION_TYPE_RAW;
    rmInfo->ngrmi_compressionThresholdNbytes = 0;
    rmInfo->ngrmi_argumentBlockSize = 0;
    rmInfo->ngrmi_coreDumpSize = 0;
    rmInfo->ngrmi_commLogEnable = 0;
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
    rmInfo->ngrmi_invokeServerType = NULL;
    rmInfo->ngrmi_invokeServerOptions = NULL;
    rmInfo->ngrmi_commProxyType = NULL;
    rmInfo->ngrmi_commProxyPath = NULL;
    rmInfo->ngrmi_commProxyOptions = NULL;
    rmInfo->ngrmi_infoServiceTag = NULL;
    rmInfo->ngrmi_jobManager = NULL;
    rmInfo->ngrmi_subject = NULL;
    rmInfo->ngrmi_clientHostName = NULL;
    rmInfo->ngrmi_jobQueue = NULL;
    rmInfo->ngrmi_jobProject = NULL;
    rmInfo->ngrmi_rslExtensions = NULL;
    rmInfo->ngrmi_workDirectory = NULL;
    rmInfo->ngrmi_debug.ngdi_terminalPath = NULL;
    rmInfo->ngrmi_debug.ngdi_display = NULL;
    rmInfo->ngrmi_debug.ngdi_debuggerPath = NULL;
    rmInfo->ngrmi_environment = NULL;
}

/**
 * GetCopy
 */
int
ngclRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteMachineInformationGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRemoteMachineInformationGetCopy(context,
	hostName, rmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllRemoteMachineInformationGetCopy";

    /* Check the arguments */
    if ((hostName == NULL) || (rmInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Lock the Remote Machine Information List */
    result = ngcliRemoteMachineInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Remote Machine Information.\n"); 
        return 0;
    }
 
    /* Find Remote Machine Information */
    rmInfoMng = ngcliRemoteMachineInformationCacheGet(context, hostName, error);
    if (rmInfoMng == NULL) {

        /* Unlock the Remote Machine Information */
        result = ngcliRemoteMachineInformationListReadUnlock(
            context, context->ngc_log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Machine Information.\n"); 
            return 0;
        }

        /* Create Remote Machine Information from Default */
        result = ngcllRemoteMachineInformationAppendNew(
            context, hostName, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't append the Remote Machine Information.\n"); 
            return 0;
        }

        /* Lock the Remote Machine Information */
        result = ngcliRemoteMachineInformationListReadLock(
            context, context->ngc_log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the list of Remote Machine Information.\n"); 
            return 0;
        }

        /* Find Remote Machine Information again */
        rmInfoMng = ngcliRemoteMachineInformationCacheGet(
            context, hostName, error);
        if (rmInfoMng == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the Remote Machine Information.\n"); 
            goto error;
        }
    }

    /* Copy the Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(context,
                    &rmInfoMng->ngrmim_info, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Remote Machine Information.\n"); 
        goto error;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Remote Machine Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Remote Machine Information.\n"); 
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
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for "
            "the Remote Machine Information.\n"); 
        return 0;
    }

    /* Initialize RemoteMachineInformation */
    result = ngcliRemoteMachineInformationInitialize(context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Remote Machine Information.\n"); 
        return 0;
    }

    /* Get DefaultRemoteMachineInformation */
    result = ngclDefaultRemoteMachineInformationGetCopy(
        context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Default Remote Machine Information.\n"); 
        return 0;
    }

    /* Set hostname */
    assert(rmInfo->ngrmi_hostName == NULL);
    rmInfo->ngrmi_hostName = strdup(hostName);
    if (rmInfo->ngrmi_hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't duplicate the string.\n"); 
        return 0;
    }

    /* Register to context */
    result = ngcliRemoteMachineInformationCacheRegister(context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the Remote Machine Information.\n"); 
        return 0;
    }

    /* Release RemoteMachineInformation */
    result = ngclRemoteMachineInformationRelease(context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
        return 0;
    }

    /* Free RemoteMachineInformation */
    result = ngcliRemoteMachineInformationFree(context, rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't free the Remote Machine Information.\n"); 
        return 0;
    }

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
    ngcliRemoteMachineInformationManager_t *dstRmInfoMng,
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
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Replace the Remote Machine Information for \"%s\".\n",
        srcRmInfo->ngrmi_hostName); 

    /* Lock */
    result = ngcliRemoteMachineInformationWriteLock(
        dstRmInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the Remote Machine Information.\n"); 
        goto error;
    }
    rmLocked = 1;

    /* Release the Remote Machine Information */
    result = ngclRemoteMachineInformationRelease(
        context, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Machine Information.\n"); 
        goto error;
    }

    /* Copy the Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(
        context, srcRmInfo, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Machine Information.\n"); 
        goto error;
    }

    /* Unlock */
    result = ngcliRemoteMachineInformationWriteUnlock(
        dstRmInfoMng, log, error);
    rmLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the Remote Machine Information.\n"); 
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
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't write unlock the Remote Machine Information.\n"); 
        }
    }
  
    /* Failed */
    return 0;
}

/**
 * Registers number of MPI CPUs to Remote Machine Information.
 */
int 
ngcliRemoteMachineInformationAppendMPInCPUs(
    ngclRemoteMachineInformation_t *rmInfo,
    char *className,
    int nCPUs,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclMPInCPUsInformation_t *ncpuInfo = NULL;
    static const char fName[] = "ngcliRemoteMachineInformationAppendMPInCPUs";

    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Remote Machine Information is invalid.\n"); 
        return 0;
    }
    if ((className == NULL) || strlen(className) == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Class name is invalid.\n"); 
        return 0;
    }
    if (nCPUs <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "nCPUs is invalid.\n"); 
        return 0;
    }

    ncpuInfo = ngcllMPInCPUsInformationConstruct(className, nCPUs, log, error);
    if (ncpuInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't construct MPI number of CPUs Information.\n"); 
        goto error;
    }
    result = ngcllRemoteMachineInformationRegisterMPInCPUs(
        rmInfo, ncpuInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register MPI number of CPUs.\n"); 
        goto error;
    }

    return 1;
error:
    result = ngcllMPInCPUsInformationDestruct(ncpuInfo, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct MPI number of CPUs Information.\n"); 
    }
    return 0;
}

/**
 * Gets number of MPI CPUs from Remote Machine Information.
 * If not found, return -1.
 */
int
ngcliRemoteMachineInformationGetMPInCPUs(
    ngclRemoteMachineInformation_t *rmInfo,
    char *className,
    ngLog_t *log,
    int *error)
{
    ngclMPInCPUsInformation_t *ncpuInfo = NULL;
    int ncpus = -1;
    static const char fName[] = "ngcliRemoteMachineInformationGetMPInCPUs";

    /* Is context valid? */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Remote Machine Information is invalid.\n"); 
        return -1;
    }
    if ((className == NULL) || strlen(className) == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Class name is invalid.\n"); 
        return -1;
    }

    ncpus = -1;
    SLIST_FOREACH(ncpuInfo, &rmInfo->ngrmi_mpiNcpusList, ngmni_entry) {
        if (strcmp(ncpuInfo->ngmni_className, className) == 0) {
            ncpus = ncpuInfo->ngmni_nCPUs;
            break;
        }
    }

    if (ncpus < 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        return -1;
    }

    return ncpus;
}

/**
 * MPI number of CPUs Information: Construct
 */
static ngclMPInCPUsInformation_t *
ngcllMPInCPUsInformationConstruct(
    char *className,
    int nCPUs,
    ngLog_t *log,
    int *error)
{
    ngclMPInCPUsInformation_t *ncpuInfo = NULL;
    int result;
    static const char fName[] = "ngcllMPInCPUsInformationConstruct";

    assert(className != NULL);
    assert(strlen(className) > 0);
    assert(nCPUs > 0);

    ncpuInfo = NGI_ALLOCATE(ngclMPInCPUsInformation_t, log, error);
    if (ncpuInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate storage for number of CPUs Information.\n"); 
        goto error;
    }

    result = ngcllMPInCPUsInformationInitialize(
        ncpuInfo, className, nCPUs, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize number of CPUs Information.\n"); 
        goto error;
    }
    return ncpuInfo;
error:
    result = ngcllMPInCPUsInformationDestruct(ncpuInfo, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct MPI number of CPUs Information.\n"); 
    }

    return NULL;
}

/**
 * MPI number of CPUs Information: Destruct
 */
static int
ngcllMPInCPUsInformationDestruct(
    ngclMPInCPUsInformation_t *ncpuInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllMPInCPUsInformationDestruct";

    if (ncpuInfo == NULL) {
        /* Do nothing */
        return 1;
    }

    result = ngcllMPInCPUsInformationFinalize(ncpuInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize number of CPUs Information.\n"); 
        error = NULL;
        ret = 1;
    }

    result = NGI_DEALLOCATE(ngclMPInCPUsInformation_t, ncpuInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate storage for number of CPUs Information.\n"); 
        error = NULL;
        ret = 1;
    }

    return ret;
}

/**
 * MPI number of CPUs Information: Initialize
 */
static int
ngcllMPInCPUsInformationInitialize(
    ngclMPInCPUsInformation_t *ncpuInfo,
    char *className,
    int nCPUs,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllMPInCPUsInformationInitialize";

    assert(ncpuInfo != NULL);
    assert(className != NULL);
    assert(strlen(className) > 0);
    assert(nCPUs > 0);

    ngcllMPInCPUsInformationInitializeMember(ncpuInfo);

    ncpuInfo->ngmni_nCPUs = nCPUs;
    ncpuInfo->ngmni_className = ngiStrdup(className, log, error);
    if (ncpuInfo->ngmni_className == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't duplicate the string.\n"); 
        return 0;
    }

    return 1;
}

/**
 * MPI number of CPUs Information: Finalize
 */
static int
ngcllMPInCPUsInformationFinalize(
    ngclMPInCPUsInformation_t *ncpuInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllMPInCPUsInformationFinalize";

    assert(ncpuInfo != NULL);

    result = ngiFree(ncpuInfo->ngmni_className, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't free storage for the string.\n"); 
        error = NULL;
        ret = 0;
    }

    ngcllMPInCPUsInformationInitializeMember(ncpuInfo);

    return ret;
}

static void
ngcllMPInCPUsInformationInitializeMember(
    ngclMPInCPUsInformation_t *ncpuInfo)
{
    assert(ncpuInfo != NULL);
    
    ncpuInfo->ngmni_className = NULL;
    ncpuInfo->ngmni_nCPUs = 0;
}

/**
 * Remote Machine Information: Registers MPI number of CPUs Information
 */
static int
ngcllRemoteMachineInformationRegisterMPInCPUs(
    ngclRemoteMachineInformation_t *rmInfo,
    ngclMPInCPUsInformation_t *ncpuInfo,
    ngLog_t *log,
    int *error)
{
    int ncpus = -1;
    static const char fName[] = "ngcllRemoteMachineInformationRegisterMPInCPUs";

    assert(rmInfo != NULL);
    assert(ncpuInfo != NULL);

    /* Check whether Information whoes classname is "className" is already
     * registered */
    ncpus = ngcliRemoteMachineInformationGetMPInCPUs(
        rmInfo, ncpuInfo->ngmni_className, log, error);
    if (ncpus >= 0) {
        NGI_SET_ERROR(error, NG_ERROR_ALREADY);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "MPI number of CPUs Information whose class name is \"%s\""
            " is already registered.\n", ncpuInfo->ngmni_className); 
        return 0;
    }

    SLIST_INSERT_HEAD(&rmInfo->ngrmi_mpiNcpusList, ncpuInfo, ngmni_entry);

    return 1;
}

/**
 * Remote Machine Information: Registers MPI number of CPUs Information
 */
static int
ngcllRemoteMachineInformationUnregisterAllMPInCPUs(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int ret = 1;
    ngLog_t *log = NULL;
    ngclMPInCPUsInformation_t *ncpuInfo = NULL;
    int result;
    static const char fName[] = "ngcllRemoteMachineInformationUnregisterAllMPInCPUs";

    assert(context != NULL);
    assert(rmInfo != NULL);

    log = context->ngc_log;

    while (!SLIST_EMPTY(&rmInfo->ngrmi_mpiNcpusList)) {
        ncpuInfo = SLIST_FIRST(&rmInfo->ngrmi_mpiNcpusList);
        SLIST_REMOVE(&rmInfo->ngrmi_mpiNcpusList,
            ncpuInfo, ngclMPInCPUsInformation_s, ngmni_entry);
        
        result = ngcllMPInCPUsInformationDestruct(ncpuInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct number of CPUs Information.\n"); 
            error = NULL;
            ret = 0;
        }
    }

    return ret;
}
