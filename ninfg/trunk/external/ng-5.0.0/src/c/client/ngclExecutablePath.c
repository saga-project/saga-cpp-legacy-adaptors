/*
 * $RCSfile: ngclExecutablePath.c,v $ $Revision: 1.14 $ $Date: 2008/03/03 05:56:16 $
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
 * Executable Path Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclExecutablePath.c,v $ $Revision: 1.14 $ $Date: 2008/03/03 05:56:16 $")

/**
 * Prototype declaration of static functions.
 */
static ngcliExecutablePathInformationManager_t *
    ngcllExecutablePathInformationCacheGet(
    ngclContext_t *, char *, char *, int, int *);
static ngcliExecutablePathInformationManager_t *
    ngcllExecutablePathInformationConstruct(
    ngclContext_t *, ngclExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationDestruct(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);
static int ngcllExecutablePathInformationManagerInitialize(
     ngclContext_t *, ngcliExecutablePathInformationManager_t *,
     ngclExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationManagerFinalize(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);
static void ngcllExecutablePathInformationInitializeMember(
    ngclExecutablePathInformation_t *);
static void ngcllExecutablePathInformationInitializePointer(
    ngclExecutablePathInformation_t *);
static int ngcllExecutablePathInformationLockListCopy(
    ngclContext_t *, char *, char *,
    ngclExecutablePathInformation_t *, int *, int *);
static int ngcllExecutablePathInformationLockCopy(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *,
    ngclExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationGetCopy(
    ngclContext_t *, char *, char *,
    ngclExecutablePathInformation_t *, int *error);
static int ngcllExecutablePathInformationRelease(
    ngclContext_t *, ngclExecutablePathInformation_t *, int *error);
static int ngcllExecutablePathInformationReplace(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *,
    ngclExecutablePathInformation_t *, int *);

/**
 * Information append at last of the list.
 */
int
ngcliExecutablePathInformationCacheRegister(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    int ret = 0;
    static const char fName[] = "ngcliExecutablePathInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    epInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    log = context->ngc_log;

    if ((epInfo == NULL) ||
        (epInfo->ngepi_hostName == NULL) ||
        (epInfo->ngepi_className == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid argument.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable Path Information list.\n"); 
        goto finalize;
    }
    listLocked = 1;

    /* Is Executable Path Information available? */
    epInfoMng = ngcllExecutablePathInformationCacheGet(
        context, epInfo->ngepi_hostName, epInfo->ngepi_className,
        1, &subError);
    if (epInfoMng != NULL) {
        /* Replace the Executable Path Information */
        result = ngcllExecutablePathInformationReplace(
            context, epInfoMng, epInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't replace the Executable Path Information.\n"); 
            goto finalize;
        }
    }

    /* Construct */
    if (epInfoMng == NULL) {
        epInfoMng = ngcllExecutablePathInformationConstruct(
            context, epInfo, error);
        if (epInfoMng == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Executable Path Information.\n"); 
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliExecutablePathInformationListWriteUnlock(
            context, log, error);
        if (result == 0) {
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't write unlock the Executable Path Information list.\n");
            error = NULL;
            ret = 0;
        }
        listLocked = 0;
    }
    return ret;
}

/**
 * Information delete from the list.
 */
int
ngcliExecutablePathInformationCacheUnregister(
    ngclContext_t *context,
    char *hostName,
    char *className,
    int *error)
{
    int result;
    int ret = 0;
    ngcliExecutablePathInformationManager_t *curr;
    static const char fName[] = "ngcliExecutablePathInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    if (((hostName != NULL) && (className == NULL)) ||
        ((hostName == NULL) && (className != NULL)) ) {
        /**
         * If both hostName and className are NULL,
         * Delete all items
         */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostName or className is NULL.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock ExecutablePathInformation list.\n"); 
	return 0;
    }

    if (className == NULL) {
        assert(hostName == NULL);
	/* Delete all information */
        while (1) {
            /* Get data from a head of list */
            curr = ngcliExecutablePathInformationCacheGetNext(context,
                NULL, error);
            if (curr == NULL) {
                break;
            }
	    /* Destruct data */
	    result = ngcllExecutablePathInformationDestruct(
		context, curr, error);
	    if (result == 0) {
		ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't destruct Executable Path Information.\n"); 
                goto finalize;
	    }
	}
    } else {
        /* Delete specified information */

	/* Get data from list by classname */
	curr = ngcllExecutablePathInformationCacheGet(context,
	    hostName, className, 1, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Executable Path Information(classname: \"%s\", hostName = \"%s\" is not found.\n",
                className, hostName); 
	    goto finalize;
	}

	/* Destruct data */
	result = ngcllExecutablePathInformationDestruct(context, curr, error);
	if (result == 0) {
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct Executable Path Information.\n"); 
	    goto finalize;
	}
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    result = ngcliExecutablePathInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Executable Path Information.\n"); 
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * Get the information by host name, class name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliExecutablePathInformationManager_t *
ngcliExecutablePathInformationCacheGet(
    ngclContext_t *context,
    char *hostName,
    char *className,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcliExecutablePathInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    /* Check the arguments */
    if ((hostName == NULL) || (className == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Get Active Remote Executable Path Information. */
    epInfoMng = ngcllExecutablePathInformationCacheGet(
        context, hostName, className, 0, error);
    if (epInfoMng == NULL) {
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable Path Information is not found by "
            "host name \"%s\" and class name \"%s\".\n",
            hostName, className); 
    }

    return epInfoMng;
}

/**
 * Get the information by host name, class name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static ngcliExecutablePathInformationManager_t *
ngcllExecutablePathInformationCacheGet(
    ngclContext_t *context,
    char *hostName,
    char *className,
    int requireInactivated,
    int *error)
{
    int result, found;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationCacheGet";

    found = 0;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    /* Check the arguments */
    if ((hostName == NULL) || (className == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    found = 0;
    epInfoMng = context->ngc_epInfo_head;
    for (; epInfoMng != NULL; epInfoMng = epInfoMng->ngepim_next) {
	assert(epInfoMng->ngepim_info.ngepi_className != NULL);
	assert(epInfoMng->ngepim_info.ngepi_hostName != NULL);
	if ((strcmp(epInfoMng->ngepim_info.ngepi_className, className) == 0) &&
	    (strcmp(epInfoMng->ngepim_info.ngepi_hostName,  hostName) == 0)) {

	    found = 1;
	    break;
	}
    }

    if ((found != 0) && (requireInactivated == 0)) {
        if (epInfoMng->ngepim_active == 0) {
            found = 0;
        }
    }

    if (found == 0) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable Path Information is not found by "
            "host name \"%s\" and class name \"%s\".\n",
            hostName, className); 
        return NULL;
    }

    /* Found */
    return epInfoMng;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliExecutablePathInformationManager_t *
ngcliExecutablePathInformationCacheGetNext(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    if (current == NULL) {
	/* Return the first information */
	if (context->ngc_epInfo_head != NULL) {
            return context->ngc_epInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngepim_next != NULL) {
	    return current->ngepim_next;
	}
    }

    /* The last information was reached */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Executable Path Information was reached.\n"); 
    return NULL;
}

/**
 * Information inactivate from the list.
 */
int
ngcliExecutablePathInformationCacheInactivate(
    ngclContext_t *context,
    char *hostName,
    char *className,
    int *error)
{
    int result;
    int ret = 0;
    ngcliExecutablePathInformationManager_t *curr;
    static const char fName[] = "ngcliExecutablePathInformationCacheInactivate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    if (((hostName != NULL) && (className == NULL)) ||
        ((hostName == NULL) && (className != NULL)) ) {
        /**
         * If both hostName and className are NULL,
         * Inactivate all items
         */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostName or className is NULL.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock ExecutablePathInformation list.\n"); 
	return 0;
    }

    if (className == NULL) {
        assert(hostName == NULL);
	/* Inactivate all information */
        curr = NULL;
        while (1) {
            /* Get data from a head of list */
            curr = ngcliExecutablePathInformationCacheGetNext(
                context, curr, error);

            if (curr == NULL) {
                break;
            }

	    curr->ngepim_active = 0;
	}
    } else {
        /* Inactivate specified information */

	/* Get data from list by classname */
	curr = ngcllExecutablePathInformationCacheGet(context,
	    hostName, className, 1, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Executable Path Information(classname: \"%s\", hostName = \"%s\" is not found.\n",
                className, hostName); 
	    goto finalize;
	}

	curr->ngepim_active = 0;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }
    result = ngcliExecutablePathInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Executable Path Information.\n"); 
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * Construct.
 */
static ngcliExecutablePathInformationManager_t *
ngcllExecutablePathInformationConstruct(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(epInfo != NULL);

    /* Allocate */
    epInfoMng = NGI_ALLOCATE(ngcliExecutablePathInformationManager_t,
        context->ngc_log, error);
    if (epInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Executable Path Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllExecutablePathInformationManagerInitialize(
        context, epInfoMng, epInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Executable Path Information.\n"); 
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterExecutablePathInformation(
    	context, epInfoMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Executable Path Information.\n"); 
	goto error;
    }

    /* Success */
    return epInfoMng;

    /* Error occurred */
error:
    result = ngcllExecutablePathInformationDestruct(context, epInfoMng, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the storage for Executable Path Information Manager.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllExecutablePathInformationDestruct(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutablePathInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(epInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterExecutablePathInformation(
	context, epInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executable Path Information.\n"); 
	return 0;
    }

    /* Finalize */
    result = ngcllExecutablePathInformationManagerFinalize(context, epInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Executable Path Information.\n"); 
	return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliExecutablePathInformationManager_t, epInfoMng,
        context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Executable Path Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage.
 */
ngclExecutablePathInformation_t *
ngcliExecutablePathInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclExecutablePathInformation_t *epInfo;
    static const char fName[] = "ngcliExecutablePathInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Allocate new storage */
    epInfo = NGI_ALLOCATE(ngclExecutablePathInformation_t,
        context->ngc_log, error);
    if (epInfo == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the Executable Path Information.\n"); 
	return NULL;
    }

    /* Success */
    return epInfo;
}

/**
 * Deallocate.
 */
int
ngcliExecutablePathInformationFree(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngclExecutablePathInformation_t, epInfo,
        context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Executable Path Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllExecutablePathInformationManagerInitialize(
     ngclContext_t *context,
     ngcliExecutablePathInformationManager_t *epInfoMng,
     ngclExecutablePathInformation_t *epInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllExecutablePathInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(epInfoMng != NULL);
    assert(epInfo != NULL);

    /* Copy to new information */
    result = ngcliExecutablePathInformationCopy(context, epInfo,
	&epInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Information.\n"); 
        return 0;
    }

    /* Initialize the Read/Write lock for own instance */
    result = ngiRWlockInitialize(&epInfoMng->ngepim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* set next to NULL */
    epInfoMng->ngepim_next = NULL;

    epInfoMng->ngepim_active = 1;

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllExecutablePathInformationManagerFinalize(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutablePathInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(epInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&epInfoMng->ngepim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destroy Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Release the information */
    result = ngclExecutablePathInformationRelease(context,
	&epInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Information.\n"); 
	return 0;
    }

    /* reset members */
    epInfoMng->ngepim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliExecutablePathInformationCopy(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *src,
    ngclExecutablePathInformation_t *dest,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliExecutablePathInformationCopy";

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
    ngcllExecutablePathInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllExecutablePathInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = ngiStrdup((src)->member, log, error); \
        if ((dest)->member == NULL) { \
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName, \
                "Can't allocate the storage " \
                "for Executable Path Information.\n"); \
            goto error; \
        } \
    } while(0)

#define  NGL_COPY_STRING_IF_VALID(str, dest, member) \
    do { \
        if ((src)->member != NULL) { \
            NGL_COPY_STRING(str, dest, member); \
        } \
    } while (0)
    
    NGL_COPY_STRING(src, dest, ngepi_hostName);
    NGL_COPY_STRING(src, dest, ngepi_className);
    NGL_COPY_STRING_IF_VALID(src, dest, ngepi_path);

#undef NGL_COPY_STRING_IF_VALID
#undef NGL_COPY_STRING

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclExecutablePathInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Information.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclExecutablePathInformationRelease(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutablePathInformationRelease";

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

    result = ngcllExecutablePathInformationRelease(
	context, epInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutablePathInformationRelease(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    ngLog_t *log;
    static const char fName[] = "ngcllExecutablePathInformationRelease";

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Deallocate the members */
    ngiFree(epInfo->ngepi_hostName, log, error);
    ngiFree(epInfo->ngepi_className, log, error);
    ngiFree(epInfo->ngepi_path, log, error);

    /* Initialize the members */
    ngcllExecutablePathInformationInitializeMember(epInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliExecutablePathInformationInitialize(
    ngclContext_t *context,
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllExecutablePathInformationInitializeMember(epInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllExecutablePathInformationInitializeMember(
    ngclExecutablePathInformation_t *epInfo)
{
    /* Initialize the members */
    ngcllExecutablePathInformationInitializePointer(epInfo);
    epInfo->ngepi_stagingEnable = 0;
    epInfo->ngepi_backend = NG_BACKEND_NORMAL;
    epInfo->ngepi_sessionTimeout = 0;
    epInfo->ngepi_transferTimeout_argument = 0;
    epInfo->ngepi_transferTimeout_result = 0;
    epInfo->ngepi_transferTimeout_cbArgument = 0;
    epInfo->ngepi_transferTimeout_cbResult = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllExecutablePathInformationInitializePointer(
    ngclExecutablePathInformation_t *epInfo)
{
    /* Initialize the pointers */
    epInfo->ngepi_hostName = NULL;
    epInfo->ngepi_className = NULL;
    epInfo->ngepi_path = NULL;
}

/**
 * GetCopy.
 */
int
ngclExecutablePathInformationGetCopy(
    ngclContext_t *context,
    char *hostName, 
    char *className, 
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutablePathInformationGetCopy";

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

    result = ngcllExecutablePathInformationGetCopy(context, hostName,
	className, epInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

/**
 * GetCopy.
 * This function query information to Information Service if necessary
 */
int
ngcliExecutablePathInformationGetCopyWithQuery(
    ngclContext_t *context,
    char *hostName, 
    char *className, 
    char *tag, 
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    int found = 0;
    static const char fName[] = "ngcliExecutablePathInformationGetCopyWithQuery";

    /* Check the arguments */
    assert(context   != NULL);
    assert(hostName  != NULL);
    assert(className != NULL);
    assert(epInfo    != NULL);

    result = ngcllExecutablePathInformationLockListCopy(
        context, hostName, className, epInfo, &found, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Information.\n"); 
        return 0;
    }
    if (found != 0) {
        /* Found! */
        return 1;
    }

    result = ngcliQueryManagerQuery(context->ngc_queryManager,
        hostName, className, tag, epInfo, NULL, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't query information to the Information Service.\n"); 
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "The Executable Path Information(hostname = \"%s\""
            " classname = \"%s\") is not found.\n", hostName, className); 
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        return 0;
    }

    return 1;
}

static int
ngcllExecutablePathInformationGetCopy(
    ngclContext_t *context,
    char *hostName, 
    char *className, 
    ngclExecutablePathInformation_t *epInfo,
    int *error)
{
    int result, found;
    static const char fName[] = "ngcllExecutablePathInformationGetCopy";

    /* Check the arguments */
    if ((hostName == NULL) || (className == NULL) || (epInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Find the Executable Path Information (not found is valid) */
    result = ngcllExecutablePathInformationLockListCopy(
        context, hostName, className, epInfo, &found, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Information.\n"); 
        return 0;
    }
    if (found == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Lock list and Get.
 */
static int
ngcllExecutablePathInformationLockListCopy(
    ngclContext_t *context,
    char *hostName,
    char *className,
    ngclExecutablePathInformation_t *dstEpInfo,
    int *found,
    int *error)
{
    int result, returnCode;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationLockListCopy";

    /* Check the arguments */
    assert(context   != NULL);
    assert(hostName  != NULL);
    assert(className != NULL);
    assert(dstEpInfo != NULL);
    assert(found     != NULL);

    /* Lock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable Path Information.\n"); 
        return 0;
    }

    returnCode = 0;

    /* Find the Executable Path Information */
    epInfoMng = ngcliExecutablePathInformationCacheGet(
        context, hostName, className, error);
    if (epInfoMng == NULL) {
        *found = 0; 
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
        returnCode = 1;
    } else {
        assert(epInfoMng->ngepim_info.ngepi_path != NULL);
        *found = 1; 

        /* Copy the Executable Path Information */
        result = ngcllExecutablePathInformationLockCopy(
            context, epInfoMng, dstEpInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the Executable Path Information.\n"); 
            returnCode = 0;
        } else {
            returnCode = 1;
        }
    }

    /* Unlock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable Path Information.\n"); 
        return 0;
    }

    return returnCode;
}

/**
 * Lock and Copy.
 */
static int
ngcllExecutablePathInformationLockCopy(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *srcEpInfoMng,
    ngclExecutablePathInformation_t *dstEpInfo,
    int *error)
{
    ngLog_t *log;
    int result, returnCode;
    static const char fName[] = "ngcllExecutablePathInformationLockCopy";

    /* Check the arguments */
    assert(context != NULL);
    assert(srcEpInfoMng != NULL);
    assert(dstEpInfo != NULL);

    log = context->ngc_log;

    /* Lock the Executable Path Information */
    result = ngcliExecutablePathInformationReadLock(srcEpInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the Executable Path Information.\n"); 
        return 0;
    }

    returnCode = 0;

    /* Copy the Executable Path Information */
    result = ngcliExecutablePathInformationCopy(
        context, &srcEpInfoMng->ngepim_info, dstEpInfo, error);
    if (result != 0) {
        returnCode = 1;
    } else {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Information.\n"); 
        returnCode = 0;
    }

    result = ngcliExecutablePathInformationReadUnlock(srcEpInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the Executable Path Information.\n"); 
        return 0;
    }

    return returnCode;
}

/**
 * Replace the Executable Path Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllExecutablePathInformationReplace(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *dstEpInfoMng,
    ngclExecutablePathInformation_t *srcEpInfo,
    int *error)
{
    ngLog_t *log;
    int result, epLocked;
    static const char fName[] = "ngcllExecutablePathInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstEpInfoMng != NULL);
    assert(srcEpInfo != NULL);

    log = context->ngc_log;
    epLocked = 0;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Replace the Executable Path Information for"
        " \"%s\" on \"%s\".\n",
        srcEpInfo->ngepi_className, srcEpInfo->ngepi_hostName); 

    /* Lock */
    result = ngcliExecutablePathInformationWriteLock(
        dstEpInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the Executable Path Information.\n"); 
        goto error;
    }
    epLocked = 1;

    /* Release the Executable Path Information */
    result = ngclExecutablePathInformationRelease(
        context, &dstEpInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Information.\n"); 
        goto error;
    }

    /* Copy the Executable Path Information */
    result = ngcliExecutablePathInformationCopy(
        context, srcEpInfo, &dstEpInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Executable Path Information.\n"); 
        goto error;
    }

    dstEpInfoMng->ngepim_active = 1;

    /* Unlock */
    result = ngcliExecutablePathInformationWriteUnlock(
        dstEpInfoMng, log, error);
    epLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the Executable Path Information.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (epLocked != 0) {
        result = ngcliExecutablePathInformationWriteUnlock(
            dstEpInfoMng, log, NULL);
        epLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't write unlock the Executable Path Information.\n"); 
        }
    }
  
    /* Failed */
    return 0;
}

