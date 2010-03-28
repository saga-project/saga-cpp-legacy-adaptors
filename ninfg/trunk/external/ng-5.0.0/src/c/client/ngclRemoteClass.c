/*
 * $RCSfile: ngclRemoteClass.c,v $ $Revision: 1.12 $ $Date: 2008/02/27 05:43:01 $
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
 * Remote Class Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclRemoteClass.c,v $ $Revision: 1.12 $ $Date: 2008/02/27 05:43:01 $")

/**
 * Prototype declaration of static functions.
 */
static ngcliRemoteClassInformationManager_t *
ngcllRemoteClassInformationCacheGet(ngclContext_t *, char *, int, int *);
static ngcliRemoteClassInformationManager_t *
    ngcllRemoteClassInformationConstruct(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationDestruct(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);
static int ngcllRemoteClassInformationManagerInitialize(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationManagerFinalize(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);
static int ngcllRemoteClassInformationRelease(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
static void ngcllRemoteClassInformationInitializeMember(
    ngRemoteClassInformation_t *);
static void ngcllRemoteClassInformationInitializePointer(
    ngRemoteClassInformation_t *);
static int ngcllRemoteClassInformationGetCopy(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, char *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationLockListCopy(
    ngclContext_t *, int, char *, ngRemoteClassInformation_t *, int *, int *);
static int ngcllRemoteClassInformationLockCopy(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationReplace(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *,
    ngRemoteClassInformation_t *, int *);

/**
 * Register to the cache
 * Information append at last of the list.
 */
int
ngcliRemoteClassInformationCacheRegister(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngLog_t *log;
    int result, subError, listLocked;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcliRemoteClassInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    rcInfoMng = NULL;
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
    if ((rcInfo == NULL) ||
        (rcInfo->ngrci_className == NULL) ||
        (rcInfo->ngrci_version == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Remote Class Information list.\n"); 
        goto error;
    }
    listLocked = 1;

    /* Is Remote Class Information available? */
    rcInfoMng = ngcllRemoteClassInformationCacheGet(
        context, rcInfo->ngrci_className, 1, &subError);
    if (rcInfoMng != NULL) {

        /* Replace the Remote Class Information */
        result = ngcllRemoteClassInformationReplace(
            context, rcInfoMng, rcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't replace the Remote Class Information.\n"); 
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the Remote Class Information list.\n"); 
        goto error;
    }

    /* Construct */
    if (rcInfoMng == NULL) {
        rcInfoMng = ngcllRemoteClassInformationConstruct(
            context, rcInfo, error);
        if (rcInfoMng == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Remote Class Information.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliRemoteClassInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't write unlock the Remote Class Information list.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Information delete from the list.
 * if className is NULL, deletes all items.
 */
int
ngcliRemoteClassInformationCacheUnregister(
    ngclContext_t *context,
    char *className,
    int *error)
{
    int result;
    ngcliRemoteClassInformationManager_t *curr;
    static const char fName[] = "ngcliRemoteClassInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock RemoteClassInformation list.\n"); 
	return 0;
    }

    if (className == NULL) {
	/* Delete all information */

        while (1) {
	    /* Get data from the list */
	    curr = ngcliRemoteClassInformationCacheGetNext(
		context, NULL, error);

            if (curr == NULL) {
                break;
            }

	    /* Destruct the data */
	    result = ngcllRemoteClassInformationDestruct(
		context, curr, error);
	    if (result == 0) {
		ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't destruct Remote Class Information.\n"); 
		goto error;
	    }
	}
    } else {
        /* Delete specified information */

	/* Get the data from the list by className */
	curr = ngcllRemoteClassInformationCacheGet(
	    context, className, 1, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Remote Class Information of the classname \"%s\" is not found.\n",
                className); 
	    goto error;
	}

	/* Destruct the data */
	result = ngcllRemoteClassInformationDestruct(context, curr, error);
	if (result == 0) {
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct Remote Class Information.\n"); 
	    goto error;
	}
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't write unlock the list of Remote Class Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't write unlock the list of Remote Class Information.\n"); 
    }
    return 0;
}

/**
 * Get the information by class name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliRemoteClassInformationManager_t *
ngcliRemoteClassInformationCacheGet(
    ngclContext_t *context,
    char *className,
    int *error)
{
    int result;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcliRemoteClassInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    /* Check the arguments */
    if (className == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostname is NULL.\n"); 
	return NULL;
    }

    /* Get Active Remote Class Information. */
    rcInfoMng = ngcllRemoteClassInformationCacheGet(
        context, className, 0, error);
    if (rcInfoMng == NULL) {
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Remote Class Information is not found by class name \"%s\".\n",
            className); 
    }

    return rcInfoMng;
}

/**
 * Get the information by class name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static ngcliRemoteClassInformationManager_t *
ngcllRemoteClassInformationCacheGet(
    ngclContext_t *context,
    char *className,
    int requireInactivated,
    int *error)
{
    int result, found;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationCacheGet";

    found = 0;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    /* Check the arguments */
    if (className == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostname is NULL.\n"); 
	return NULL;
    }

    found = 0;
    rcInfoMng = context->ngc_rcInfo_head;
    for (; rcInfoMng != NULL; rcInfoMng = rcInfoMng->ngrcim_next) {
	assert(rcInfoMng->ngrcim_info.ngrci_className != NULL);
	if (strcmp(rcInfoMng->ngrcim_info.ngrci_className, className) == 0) {

            found = 1;
            break;
	}
    }

    if ((found != 0) && (requireInactivated == 0)) {
        if (rcInfoMng->ngrcim_active == 0) {
            found = 0;
        }
    }

    if (found == 0) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Remote Class Information is not found by class name \"%s\".\n",
            className); 

        return NULL;
    }

    /* Found */
    return rcInfoMng;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliRemoteClassInformationManager_t *
ngcliRemoteClassInformationCacheGetNext(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteClassInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    if (current == NULL) {
	/* Return the first information */
	if (context->ngc_rcInfo_head != NULL) {
            return context->ngc_rcInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngrcim_next != NULL) {
	    return current->ngrcim_next;
	}
    }

    /* Not found */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Remote Class Information was reached.\n"); 
    return NULL;
}

/**
 * Information inactivate from the list.
 * if className is NULL, inactivate all items.
 */
int
ngcliRemoteClassInformationCacheInactivate(
    ngclContext_t *context,
    char *className,
    int *error)
{
    int result;
    ngcliRemoteClassInformationManager_t *curr;
    static const char fName[] = "ngcliRemoteClassInformationCacheInactivate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock RemoteClassInformation list.\n"); 
	return 0;
    }

    if (className == NULL) {
	/* Inactivate all information */

        curr = NULL;
        while (1) {
	    /* Get data from the list */
	    curr = ngcliRemoteClassInformationCacheGetNext(
		context, curr, error);

            if (curr == NULL) {
                break;
            }

            curr->ngrcim_active = 0;
	}
    } else {
        /* Inactivate specified information */

	/* Get the data from the list by className */
	curr = ngcllRemoteClassInformationCacheGet(
	    context, className, 1, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Remote Class Information of the classname \"%s\" is not found.\n",
                className); 
	    goto error;
	}

        curr->ngrcim_active = 0;
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't write unlock the list of Remote Class Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't write unlock the list of Remote Class Information.\n"); 
    }
    return 0;
}

/**
 * Construct.
 */
static ngcliRemoteClassInformationManager_t *
ngcllRemoteClassInformationConstruct(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfo != NULL);

    /* Allocate */
    rcInfoMng = NGI_ALLOCATE(ngcliRemoteClassInformationManager_t,
        context->ngc_log, error);
    if (rcInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Remote Class Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllRemoteClassInformationManagerInitialize(
        context, rcInfoMng, rcInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Remote Class Information.\n"); 
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterRemoteClassInformation(
    	context, rcInfoMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Remote Class Information for Ninf-G Context.\n"); 
	goto error;
    }

    /* Success */
    return rcInfoMng;

    /* Error occurred */
error:
    result = ngcllRemoteClassInformationDestruct(context, rcInfoMng, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the storage for Remote Class Information Manager.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllRemoteClassInformationDestruct(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteClassInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterRemoteClassInformation(context, rcInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Remote Class Information.\n"); 
	return 0;
    }

    /* Finalize */
    result = ngcllRemoteClassInformationManagerFinalize(context, rcInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Remote Class Information.\n"); 
	return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(
        ngcliRemoteClassInformationManager_t, rcInfoMng, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Remote Class Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngRemoteClassInformation_t *
ngcliRemoteClassInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    ngRemoteClassInformation_t *rcInfo;
    static const char fName[] = "ngcliRemoteClassInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Allocate new storage */
    rcInfo = ngiCalloc(1,
	sizeof (ngRemoteClassInformation_t), log, error);
    if (rcInfo == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Remote Class Information.\n"); 
	return NULL;
    }

    return rcInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliRemoteClassInformationFree(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliRemoteClassInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngiFree(rcInfo, log, error);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllRemoteClassInformationManagerInitialize(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteClassInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfoMng != NULL);
    assert(rcInfo != NULL);

    /* Copy to new information */
    result = ngcliRemoteClassInformationCopy(context,
    	rcInfo, &rcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Class Information.\n"); 
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&rcInfoMng->ngrcim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Init member */
    rcInfoMng->ngrcim_next = NULL;
    rcInfoMng->ngrcim_active = 1;

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllRemoteClassInformationManagerFinalize(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteClassInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&rcInfoMng->ngrcim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destroy Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Release the information */
    result = ngclRemoteClassInformationRelease(context,
	&rcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Class Information.\n"); 
	return 0;
    }

    /* clear member of ngcliRemoteClassInformationManager_t */
    rcInfoMng->ngrcim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliRemoteClassInformationCopy(
    ngclContext_t *context,
    ngRemoteClassInformation_t *src,
    ngRemoteClassInformation_t *dest,
    int *error)
{
    int i;
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliRemoteClassInformationCopy";

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
            "RemoteMethod is NULL.\n"); 
        return 0;
    }
    assert(src->ngrci_className != NULL);
    assert(src->ngrci_version != NULL);
    assert(src->ngrci_nMethods > 0);
    assert(src->ngrci_method != NULL);

    /* Initialize the members */
    ngcllRemoteClassInformationInitializeMember(dest);

    /* Copy the members */
    dest->ngrci_backend = src->ngrci_backend;
    dest->ngrci_language = src->ngrci_language;
    dest->ngrci_nMethods = src->ngrci_nMethods;

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = ngiStrdup((src)->member, log, error); \
        if ((dest)->member == NULL) { \
            ngclLogErrorContext(context, \
                NG_LOGCAT_NINFG_PURE, fName, \
                "Can't allocate the storage " \
                "for Remote Class Information.\n"); \
            goto error; \
        } \
    } while(0)

#define  NGL_COPY_STRING_IF_VALID(str, dest, member) \
    do {\
        if ((src)->member != NULL) { \
            NGL_COPY_STRING(str, dest, member); \
        } \
    } while (0)
    
    NGL_COPY_STRING(src, dest, ngrci_className); 
    NGL_COPY_STRING(src, dest, ngrci_version);
    NGL_COPY_STRING_IF_VALID(src, dest, ngrci_description);

#undef NGL_COPY_STRING_IF_VALID
#undef NGL_COPY_STRING

    /* Copy methods */
    dest->ngrci_method = ngcliRemoteMethodInformationAllocate(
        context, src->ngrci_nMethods, error);
    if (dest->ngrci_method == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate RemoteMethodInformation.\n"); 
        goto error;
    }

    for (i = 0; i < src->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationCopy(context,
            &src->ngrci_method[i],
            &dest->ngrci_method[i], error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy RemoteMethodInformation.\n"); 
            goto error;
        }
    }
    assert(dest->ngrci_className != NULL);
    assert(dest->ngrci_version != NULL);
    assert(dest->ngrci_nMethods > 0);
    assert(dest->ngrci_method != NULL);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Release */
    if (dest->ngrci_className != NULL) {
        ngiFree(dest->ngrci_className, log, NULL);
    }
    if (dest->ngrci_version != NULL) {
        ngiFree(dest->ngrci_version, log, NULL);
    }
    if (dest->ngrci_description != NULL) {
        ngiFree(dest->ngrci_description, log, NULL);
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclRemoteClassInformationRelease(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int local_error, result;
    static const char *fName = "ngclRemoteClassInformationRelease";

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

    result = ngcllRemoteClassInformationRelease(context, rcInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRemoteClassInformationRelease(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    ngLog_t *log;
    int result, i;
    static const char *fName = "ngcllRemoteClassInformationRelease";

    /* Check the arguments */
    assert(context != NULL);

    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "rcInfo is NULL.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Release Method Information */
    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationRelease(
            context, &rcInfo->ngrci_method[i], error);
        if (result == 0) {
           ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
               "Can't release RemoteMethodInformation.\n"); 
            return 0;
        }
    } 

    /* Deallocate the members */
    ngiFree(rcInfo->ngrci_className, log, error);
    ngiFree(rcInfo->ngrci_version, log, error);
    ngiFree(rcInfo->ngrci_description, log, error);

    result = ngcliRemoteMethodInformationFree(context,
        rcInfo->ngrci_method, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't free RemoteMethodInformation.\n"); 
        return 0;
    }
    
    /* Initialize members */
    ngcllRemoteClassInformationInitializeMember(rcInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliRemoteClassInformationInitialize(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteClassInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllRemoteClassInformationInitializeMember(rcInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllRemoteClassInformationInitializeMember(
    ngRemoteClassInformation_t *rcInfo)
{
    /* Initialize the members */
    ngcllRemoteClassInformationInitializePointer(rcInfo);
    rcInfo->ngrci_backend = NG_BACKEND_NORMAL;
    rcInfo->ngrci_language = NG_CLASS_LANGUAGE_C;
    rcInfo->ngrci_nMethods = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllRemoteClassInformationInitializePointer(
    ngRemoteClassInformation_t *rcInfo)
{
    /* Initialize the pointers */
    rcInfo->ngrci_className = NULL;
    rcInfo->ngrci_version = NULL;
    rcInfo->ngrci_description = NULL;
    rcInfo->ngrci_method = NULL;
}

/**
 * GetCopy.
 * Note: This function doesn't include retrieving from Executable.
 */
int 
ngclRemoteClassInformationGetCopy(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRemoteClassInformationGetCopy";

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

    result = ngcllRemoteClassInformationGetCopy(context, rmInfoMng,
	className, rcInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

int 
ngcllRemoteClassInformationGetCopy(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result, found;
    static const char fName[] = "ngcllRemoteClassInformationGetCopy";

    /* Check the arguments (rmInfoMng can be NULL) */
    if ((className == NULL) || (rcInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get the Remote Class Information (not found is invalid) */
    result = ngcllRemoteClassInformationLockListCopy(
        context, 1, className, rcInfo, &found, error);
    if ((result == 0) || (found == 0)) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Class Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Lock list and Get.
 */
static int
ngcllRemoteClassInformationLockListCopy(
    ngclContext_t *context,
    int isNotFoundValid,
    char *className,
    ngRemoteClassInformation_t *dstRcInfo,
    int *found,
    int *error)
{
    int result, returnCode;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationLockListCopy";

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(dstRcInfo != NULL);
    assert(found != NULL);

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Remote Class Information.\n"); 
        return 0;
    }

    returnCode = 0;

    /* Find the Remote Class Information */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(context, className, error);
    if (rcInfoMng == NULL) {
        *found = 0; 
        if (isNotFoundValid == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the Remote Class Information.\n"); 
            returnCode = 0;
        } else {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            returnCode = 1;
        }
    } else {
        *found = 1; 

        /* Copy the Remote Class Information */
        result = ngcllRemoteClassInformationLockCopy(
            context, rcInfoMng, dstRcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the Remote Class Information.\n"); 
            returnCode = 0;
        } else {
            returnCode = 1;
        }
    }

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Remote Class Information.\n"); 
        return 0;
    }

    return returnCode;
}

/**
 * Lock and Copy.
 */
static int
ngcllRemoteClassInformationLockCopy(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *srcRcInfoMng,
    ngRemoteClassInformation_t *dstRcInfo,
    int *error)
{
    ngLog_t *log;
    int result, returnCode;
    static const char fName[] = "ngcllRemoteClassInformationLockCopy";

    /* Check the arguments */
    assert(context != NULL);
    assert(srcRcInfoMng != NULL);
    assert(dstRcInfo != NULL);

    log = context->ngc_log;

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationReadLock(srcRcInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Remote Class Information.\n"); 
        return 0;
    }

    returnCode = 0;

    /* Copy the Remote Class Information */
    result = ngcliRemoteClassInformationCopy(
        context, &srcRcInfoMng->ngrcim_info, dstRcInfo, error);
    if (result != 0) {
        returnCode = 1;
    } else {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Class Information.\n"); 
        returnCode = 0;
    }

    result = ngcliRemoteClassInformationReadUnlock(srcRcInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Remote Class Information.\n"); 
        return 0;
    }

    return returnCode;
}

/**
 * Replace the Remote Class Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllRemoteClassInformationReplace(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *dstRcInfoMng,
    ngRemoteClassInformation_t *srcRcInfo,
    int *error)
{
    ngLog_t *log;
    int result, rcLocked;
    static const char fName[] = "ngcllRemoteClassInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstRcInfoMng != NULL);
    assert(srcRcInfo != NULL);

    log = context->ngc_log;
    rcLocked = 0;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Replace the Remote Class Information for \"%s\".\n",
        srcRcInfo->ngrci_className); 

    /* Lock */
    result = ngcliRemoteClassInformationWriteLock(
        dstRcInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the Remote Class Information.\n"); 
        goto error;
    }
    rcLocked = 1;
  
    /* Release the Remote Class Information */
    result = ngcllRemoteClassInformationRelease(
        context, &dstRcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Remote Class Information.\n"); 
        goto error;
    }

    /* Copy the Remote Class Information */
    result = ngcliRemoteClassInformationCopy(
        context, srcRcInfo, &dstRcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Remote Class Information.\n"); 
        goto error;
    }

    dstRcInfoMng->ngrcim_active = 1;

    /* Unlock */
    result = ngcliRemoteClassInformationWriteUnlock(
        dstRcInfoMng, log, error);
    rcLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the Remote Class Information.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (rcLocked != 0) {
        result = ngcliRemoteClassInformationWriteUnlock(
            dstRcInfoMng, log, NULL);
        rcLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't write unlock the Remote Class Information.\n"); 
        }
    }
  
    /* Failed */
    return 0;
}

