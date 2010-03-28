#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclExecutablePath.c,v $ $Revision: 1.27 $ $Date: 2006/01/06 03:03:52 $";
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
 * Executable Path Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngcliExecutablePathInformationManager_t *
    ngcllExecutablePathInformationManagerAllocate(
    ngclContext_t *, int *);
static int ngcllExecutablePathInformationManagerFree(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);
static ngcliExecutablePathInformationManager_t *
ngcllExecutablePathInformationConstruct(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationDestruct(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *, int *);
static int ngcllExecutablePathInformationManagerInitialize(
     ngclContext_t *, ngcliExecutablePathInformationManager_t *,
     ngcliExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationManagerFinalize(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *, int *);
static void ngcllExecutablePathInformationInitializeMember(
    ngcliExecutablePathInformation_t *);
static void ngcllExecutablePathInformationInitializePointer(
    ngcliExecutablePathInformation_t *);
static int ngcllExecutablePathInformationLockListCopy(
    ngclContext_t *, int, ngclRemoteMachineInformationManager_t *,
    char *, ngcliExecutablePathInformation_t *, int *, int *);
static int ngcllExecutablePathInformationLockCopy(
    ngclContext_t *, ngcliExecutablePathInformationManager_t *,
    ngcliExecutablePathInformation_t *, int *);
static int ngcllExecutablePathInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, char *,
    ngcliExecutablePathInformation_t *, int *error);
static int ngcllExecutablePathInformationRelease(
    ngclContext_t *, ngcliExecutablePathInformation_t *, int *error);
static int ngcllExecutablePathInformationReplace(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *,
    ngcliExecutablePathInformationManager_t *,
    ngcliExecutablePathInformation_t *, int *);

/**
 * Information append at last of the list.
 */
int
ngcliExecutablePathInformationCacheRegister(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcliExecutablePathInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    epInfoMng = NULL;
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
    if ((rmInfoMng == NULL) ||
        (epInfo == NULL) ||
        (epInfo->ngepi_hostName == NULL) ||
        (epInfo->ngepi_className == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(
        rmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable Path Information list.\n",
            fName);
        goto error;
    }
    listLocked = 1;

    /* Is Executable Path Information available? */
    epInfoMng = ngcliExecutablePathInformationCacheGet(
        context, rmInfoMng, epInfo->ngepi_className, &subError);
    if (epInfoMng != NULL) {

        /* Replace the Executable Path Information */
        result = ngcllExecutablePathInformationReplace(
            context, rmInfoMng, epInfoMng, epInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the Executable Path Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliExecutablePathInformationListWriteUnlock(
        rmInfoMng, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't write unlock the Executable Path Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (epInfoMng == NULL) {
        epInfoMng = ngcllExecutablePathInformationConstruct(
            context, rmInfoMng, epInfo, error);
        if (epInfoMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Executable Path Information.\n",
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
        result = ngcliExecutablePathInformationListWriteUnlock(
            rmInfoMng, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't write unlock"
                " the Executable Path Information list.\n",
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
ngcliExecutablePathInformationCacheUnregister(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *curr;
    static const char fName[] = "ngcliExecutablePathInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Check the arguments */
    if (rmInfoMng == NULL) {
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
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock ExecutablePathInformation list.\n", fName);
	return 0;
    }

    if (className == NULL) {
	/* Delete all information */

	/* Get data from a head of list */
	curr = ngcliExecutablePathInformationCacheGetNext(context,
	    rmInfoMng, NULL, error);
	if (curr == NULL) {
            /* No  Executable Path Information was registered */
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_DEBUG, NULL,
		"%s: No Executable Path Information was registered for %s.\n",
		fName, rmInfoMng->ngrmim_info.ngrmi_hostName);
	}

        while (curr != NULL) {
	    /* Destruct data */
	    result = ngcllExecutablePathInformationDestruct(
		context, rmInfoMng, curr, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_FATAL, NULL,
		    "%s: Can't destruct Executable Path Information.\n", fName);
		goto error;
	    }

	    /* Get next data from list */
	    curr = ngcliExecutablePathInformationCacheGetNext(
		context, rmInfoMng, NULL, error);
	}
    } else {
        /* Delete specified information */

	/* Get data from list by classname */
	curr = ngcliExecutablePathInformationCacheGet(context,
	    rmInfoMng, className, error);

	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Executable Path Information of the classname \"%s\" is not found.\n",
		fName, className);
	    goto error;
	}

	/* Destruct data */
	result = ngcllExecutablePathInformationDestruct(context,
	    rmInfoMng, curr, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't destruct Executable Path Information.\n",
		fName);
	    goto error;
	}
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
    result = ngcliExecutablePathInformationListWriteUnlock(rmInfoMng,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Executable Path Information.\n",
	    fName);
	return 0;
    }

    return 0;
}

/**
 * Get the information by class name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliExecutablePathInformationManager_t *
ngcliExecutablePathInformationCacheGet(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcliExecutablePathInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    /* Check the arguments */
    if ((rmInfoMng == NULL) || (className == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    epInfoMng = rmInfoMng->ngrmim_epInfo_head;
    for (; epInfoMng != NULL; epInfoMng = epInfoMng->ngepim_next) {
	assert(epInfoMng->ngepim_info.ngepi_className != NULL);
	if (strcmp(epInfoMng->ngepim_info.ngepi_className, className) == 0) {
	    /* Found */
	    return epInfoMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL, 
	"%s: Executable Path Information is not found by class name \"%s\".\n",
	fName, className);
    return NULL;
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    if (current == NULL) {
	/* Return the first information */
	if (rmInfoMng->ngrmim_epInfo_head != NULL) {
            return rmInfoMng->ngrmim_epInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngepim_next != NULL) {
	    return current->ngepim_next;
	}
    }

    /* The last information was reached */
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL, 
	"%s: The last Executable Path Information was reached.\n",
	fName);
    return NULL;
}

/**
 * Construct.
 */
static ngcliExecutablePathInformationManager_t *
ngcllExecutablePathInformationConstruct(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(epInfo != NULL);

    /* Allocate */
    epInfoMng = ngcllExecutablePathInformationManagerAllocate(context, error);
    if (epInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Executable Path Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllExecutablePathInformationManagerInitialize(
        context, epInfoMng, epInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't initialize the Executable Path Information.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclRemoteMachineInformationRegisterExecutablePathInformation(
    	context, rmInfoMng, epInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Executable Path Information for Remote Machine Information.\n",
	    fName);
	goto error;
    }

    /* Success */
    return epInfoMng;

    /* Error occurred */
error:
    result = ngcllExecutablePathInformationDestruct(context,
	rmInfoMng, epInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for Executable Path Information Manager.\n",
	    fName);
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutablePathInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(epInfoMng != NULL);

    /* Unregister */
    result = ngclRemoteMachineInformationUnregisterExecutablePathInformation(
	context, rmInfoMng, epInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Executable Path Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllExecutablePathInformationManagerFinalize(context, epInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Executable Path Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllExecutablePathInformationManagerFree(context, epInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Executable Path Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * Allocate the information storage.
 */
static ngcliExecutablePathInformationManager_t *
ngcllExecutablePathInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    epInfoMng = globus_libc_calloc(1,
	sizeof (ngcliExecutablePathInformationManager_t));
    if (epInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the Executable Path Information.\n", fName);
	return NULL;
    }

    /* Success */
    return epInfoMng;
}

/**
 * Deallocate.
 */
static int
ngcllExecutablePathInformationManagerFree(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(epInfoMng != NULL);

    globus_libc_free(epInfoMng);

    /* Success */
    return 1;
}

/**
 * Allocate the information storage.
 */
ngcliExecutablePathInformation_t *
ngcliExecutablePathInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngcliExecutablePathInformation_t *epInfo;
    static const char fName[] = "ngcliExecutablePathInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    epInfo = globus_libc_calloc(1, sizeof (ngcliExecutablePathInformation_t));
    if (epInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the Executable Path Information.\n", fName);
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
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(epInfo);

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
     ngcliExecutablePathInformation_t *epInfo,
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the Executable Path Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write lock for own instance */
    result = ngiRWlockInitialize(&epInfoMng->ngepim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* set next to NULL */
    epInfoMng->ngepim_next = NULL;

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
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclExecutablePathInformationRelease(context,
	&epInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Executable Path Information.\n", fName);
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
    ngcliExecutablePathInformation_t *src,
    ngcliExecutablePathInformation_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationCopy";

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
    ngcllExecutablePathInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllExecutablePathInformationInitializePointer(dest);

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
                "for Executable Path Information.\n", fName); \
	    goto error; \
        } \
    } while(0)

    NGL_ALLOCATE(src, dest, ngepi_hostName);
    NGL_ALLOCATE(src, dest, ngepi_className);
    if (src->ngepi_path != NULL) {
        NGL_ALLOCATE(src, dest, ngepi_path);
    }
#undef NGL_ALLOCATE

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclExecutablePathInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Executable Path Information.\n", fName);
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
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutablePathInformationRelease";

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

    result = ngcllExecutablePathInformationRelease(
	context, epInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutablePathInformationRelease(
    ngclContext_t *context,
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    static const char fName[] = "ngcllExecutablePathInformationRelease";

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Deallocate the members */
    if (epInfo->ngepi_hostName != NULL) {
        globus_libc_free(epInfo->ngepi_hostName);
    }

    if (epInfo->ngepi_className != NULL) {
        globus_libc_free(epInfo->ngepi_className);
    }

    if (epInfo->ngepi_path != NULL) {
        globus_libc_free(epInfo->ngepi_path);
    }

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
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutablePathInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (epInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
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
    ngcliExecutablePathInformation_t *epInfo)
{
    /* Initialize the members */
    ngcllExecutablePathInformationInitializePointer(epInfo);
    epInfo->ngepi_stagingEnable = 0;
    epInfo->ngepi_backend = NG_BACKEND_NORMAL;
    epInfo->ngepi_mpiNcpus = 0;
    epInfo->ngepi_sessionTimeout = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllExecutablePathInformationInitializePointer(
    ngcliExecutablePathInformation_t *epInfo)
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, 
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutablePathInformationGetCopy";

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

    result = ngcllExecutablePathInformationGetCopy(context, rmInfoMng,
	className, epInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutablePathInformationGetCopy(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className, 
    ngcliExecutablePathInformation_t *epInfo,
    int *error)
{
    int result, found;
    static const char fName[] = "ngcllExecutablePathInformationGetCopy";

    /* Check the arguments */
    if ((rmInfoMng == NULL) || (className == NULL) || (epInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Find the Executable Path Information (not found is valid) */
    result = ngcllExecutablePathInformationLockListCopy(
        context, 1, rmInfoMng, className, epInfo, &found, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Executable Path Information.\n", fName);
        return 0;
    }
    if (found == 1) {
        /* Success */
        return 1;
    }

    /* Get Executable Path Information from MDS */
    result = ngcliMDSaccessRemoteExecutableInformationGet(
                                 context, rmInfoMng, className, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to access MDS server.\n", fName);
        return 0;
    }

    /* Get the Executable Path Information (not found is invalid) */
    result = ngcllExecutablePathInformationLockListCopy(
        context, 1,rmInfoMng,  className, epInfo, &found, error);
    if ((result != 1) || (found != 1)) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Executable Path Information.\n", fName);
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
    int isNotFoundValid,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    ngcliExecutablePathInformation_t *dstEpInfo,
    int *found,
    int *error)
{
    int result, returnCode;
    ngcliExecutablePathInformationManager_t *epInfoMng;
    static const char fName[] = "ngcllExecutablePathInformationLockListCopy";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(className != NULL);
    assert(dstEpInfo != NULL);
    assert(found != NULL);

    /* Lock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadLock(
        rmInfoMng, context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Executable Path Information.\n", fName);
        return 0;
    }

    returnCode = 0;

    /* Find the Executable Path Information */
    epInfoMng = ngcliExecutablePathInformationCacheGet(
        context, rmInfoMng, className, error);
    if ((epInfoMng == NULL) || (epInfoMng->ngepim_info.ngepi_path == NULL)) {
        *found = 0; 
        if (isNotFoundValid != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Executable Path Information.\n", fName);
            returnCode = 0;
        } else {
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            returnCode = 1;
        }
    } else {
        *found = 1; 

        /* Copy the Executable Path Information */
        result = ngcllExecutablePathInformationLockCopy(
            context, epInfoMng, dstEpInfo, error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the Executable Path Information.\n", fName);
            returnCode = 0;
        } else {
            returnCode = 1;
        }
    }

    /* Unlock the Executable Path Information */
    result = ngcliExecutablePathInformationListReadUnlock(
        rmInfoMng, context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Executable Path Information.\n",
             fName);
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
    ngcliExecutablePathInformation_t *dstEpInfo,
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
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error,
            "%s: Can't lock the Executable Path Information.\n", fName);
        return 0;
    }

    returnCode = 0;

    /* Copy the Executable Path Information */
    result = ngcliExecutablePathInformationCopy(
        context, &srcEpInfoMng->ngepim_info, dstEpInfo, error);
    if (result == 1) {
        returnCode = 1;
    } else {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Executable Path Information.\n", fName);
        returnCode = 0;
    }

    result = ngcliExecutablePathInformationReadUnlock(srcEpInfoMng, log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't unlock the Executable Path Information.\n", fName);
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    ngcliExecutablePathInformationManager_t *dstEpInfoMng,
    ngcliExecutablePathInformation_t *srcEpInfo,
    int *error)
{
    ngLog_t *log;
    int result, epLocked;
    static const char fName[] = "ngcllExecutablePathInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(dstEpInfoMng != NULL);
    assert(srcEpInfo != NULL);

    log = context->ngc_log;
    epLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the Executable Path Information for"
        " \"%s\" on \"%s\".\n",
        fName, srcEpInfo->ngepi_className, srcEpInfo->ngepi_hostName);

    /* Lock */
    result = ngcliExecutablePathInformationWriteLock(
        dstEpInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Executable Path Information.\n",
            fName);
        goto error;
    }
    epLocked = 1;

    /* Release the Executable Path Information */
    result = ngclExecutablePathInformationRelease(
        context, &dstEpInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Executable Path Information.\n",
            fName);
        goto error;
    }

    /* Copy the Executable Path Information */
    result = ngcliExecutablePathInformationCopy(
        context, srcEpInfo, &dstEpInfoMng->ngepim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Executable Path Information.\n",
            fName);
        goto error;
    }

    /* Unlock */
    result = ngcliExecutablePathInformationWriteUnlock(
        dstEpInfoMng, log, error);
    epLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Executable Path Information.\n",
            fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Executable Path Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;
}

