#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclRemoteClass.c,v $ $Revision: 1.31 $ $Date: 2006/01/06 03:03:52 $";
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
 * Remote Class Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngclRemoteClassInformationManager_t *
    ngcllRemoteClassInformationManagerAllocate(ngclContext_t *, int *);
static int ngcllRemoteClassInformationManagerFree(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
static ngclRemoteClassInformationManager_t *
    ngcllRemoteClassInformationConstruct(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationDestruct(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
static int ngcllRemoteClassInformationManagerInitialize(
    ngclContext_t *, ngclRemoteClassInformationManager_t *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationManagerFinalize(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
static int ngcllRemoteClassInformationRelease(
    ngclContext_t *, ngRemoteClassInformation_t *, int *);
static void ngcllRemoteClassInformationInitializeMember(
    ngRemoteClassInformation_t *);
static void ngcllRemoteClassInformationInitializePointer(
    ngRemoteClassInformation_t *);
static int ngcllRemoteClassInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, char *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationLockListCopy(
    ngclContext_t *, int, char *, ngRemoteClassInformation_t *, int *, int *);
static int ngcllRemoteClassInformationLockCopy(
    ngclContext_t *, ngclRemoteClassInformationManager_t *,
    ngRemoteClassInformation_t *, int *);
static int ngcllRemoteClassInformationReplace(
    ngclContext_t *, ngclRemoteClassInformationManager_t *,
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
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcliRemoteClassInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    rcInfoMng = NULL;
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
    if ((rcInfo == NULL) ||
        (rcInfo->ngrci_className == NULL) ||
        (rcInfo->ngrci_version == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Remote Class Information list.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Is Remote Class Information available? */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(
        context, rcInfo->ngrci_className, &subError);
    if (rcInfoMng != NULL) {

        /* Replace the Remote Class Information */
        result = ngcllRemoteClassInformationReplace(
            context, rcInfoMng, rcInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the Remote Class Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't write unlock the Remote Class Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (rcInfoMng == NULL) {
        rcInfoMng = ngcllRemoteClassInformationConstruct(
            context, rcInfo, error);
        if (rcInfoMng == NULL) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Remote Class Information.\n",
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
        result = ngcliRemoteClassInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't write unlock the Remote Class Information list.\n",
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
ngcliRemoteClassInformationCacheUnregister(
    ngclContext_t *context,
    char *className,
    int *error)
{
    int result;
    ngclRemoteClassInformationManager_t *curr;
    static const char fName[] = "ngcliRemoteClassInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock RemoteClassInformation list.\n", fName);
	return 0;
    }

    if (className == NULL) {
	/* Delete all information */

	/* Get the data from the head of list */
        curr = ngcliRemoteClassInformationCacheGetNext(context, NULL, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_WARNING, NULL,
		"%s: No Remote Class Information was registered.\n", fName);
	}

        while (curr != NULL) {
	    /* Destruct the data */
	    result = ngcllRemoteClassInformationDestruct(
		context, curr, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_FATAL, NULL,
		    "%s: Can't destruct Remote Class Information.\n", fName);
		goto error;
	    }

	    /* Get next data from the list */
	    curr = ngcliRemoteClassInformationCacheGetNext(
		context, NULL, error);
	}
    } else {
        /* Delete specified information */

	/* Get the data from the list by className */
	curr = ngcliRemoteClassInformationCacheGet(context, className, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Remote Class Information of the classname \"%s\" is not found.\n",
		fName, className);
	    goto error;
	}

	/* Destruct the data */
	result = ngcllRemoteClassInformationDestruct(context, curr, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't destruct Remote Class Information.\n", fName);
	    goto error;
	}
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Class Information.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Class Information.\n",
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
ngclRemoteClassInformationManager_t *
ngcliRemoteClassInformationCacheGet(
    ngclContext_t *context,
    char *className,
    int *error)
{
    int result;
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcliRemoteClassInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    /* Check the arguments */
    if (className == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: hostname is NULL.\n", fName);
	return NULL;
    }

    rcInfoMng = context->ngc_rcInfo_head;
    for (; rcInfoMng != NULL; rcInfoMng = rcInfoMng->ngrcim_next) {
	assert(rcInfoMng->ngrcim_info.ngrci_className != NULL);
	if (strcmp(rcInfoMng->ngrcim_info.ngrci_className, className) == 0) {
	    /* Found */
	    return rcInfoMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Remote Class Information is not found by class name \"%s\".\n",
	fName, className);

    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclRemoteClassInformationManager_t *
ngcliRemoteClassInformationCacheGetNext(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliRemoteClassInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
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
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Remote Class Information was reached.\n", fName);
    return NULL;
}

/**
 * Construct.
 */
static ngclRemoteClassInformationManager_t *
ngcllRemoteClassInformationConstruct(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result;
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfo != NULL);

    /* Allocate */
    rcInfoMng = ngcllRemoteClassInformationManagerAllocate(context, error);
    if (rcInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Class Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllRemoteClassInformationManagerInitialize(
        context, rcInfoMng, rcInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, 
	    "%s: Can't initialize the Remote Class Information.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclContextRegisterRemoteClassInformation(
    	context, rcInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Remote Class Information for Ninf-G Context.\n",
	    fName);
	goto error;
    }

    /* Success */
    return rcInfoMng;

    /* Error occurred */
error:
    result = ngcllRemoteClassInformationDestruct(context, rcInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for Remote Class Information Manager.\n",
	    fName);
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
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllRemoteClassInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfoMng != NULL);

    /* Unregister */
    result = ngclContextUnregisterRemoteClassInformation(context, rcInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Remote Class Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllRemoteClassInformationManagerFinalize(context, rcInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Remote Class Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllRemoteClassInformationManagerFree(context, rcInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Remote Class Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * Allocate the information storage.
 */
static ngclRemoteClassInformationManager_t *
ngcllRemoteClassInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    rcInfoMng = globus_libc_calloc(
	1, sizeof (ngclRemoteClassInformationManager_t));
    if (rcInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Class Information Manager.\n", 
	    fName);
	return NULL;
    }

    return rcInfoMng;
}

/**
 * Free the information storage.
 */
static int
ngcllRemoteClassInformationManagerFree(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(rcInfoMng != NULL);

    globus_libc_free(rcInfoMng);

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
    ngRemoteClassInformation_t *rcInfo;
    static const char fName[] = "ngcliRemoteClassInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    rcInfo = globus_libc_calloc(1,
	sizeof (ngRemoteClassInformation_t));
    if (rcInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Remote Class Information.\n", 
	    fName);
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
    static const char fName[] = "ngcliRemoteClassInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(rcInfo);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllRemoteClassInformationManagerInitialize(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the Remote Class Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&rcInfoMng->ngrcim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't initialize Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Init member */
    rcInfoMng->ngrcim_next = NULL;

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllRemoteClassInformationManagerFinalize(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
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
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclRemoteClassInformationRelease(context,
	&rcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Remote Class Information.\n", fName);
	return 0;
    }

    /* clear member of ngclRemoteClassInformationManager_t */
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
    static const char fName[] = "ngcliRemoteClassInformationCopy";

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
            NG_LOG_LEVEL_ERROR, NULL, "%s: RemoteMethod is NULL.\n", fName);
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
#define NGL_ALLOCATE(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = strdup((src)->member); \
        if ((dest)->member == NULL) { \
            NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
            ngclLogPrintfContext(context, \
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, \
                "%s: Can't allocate the storage " \
                "for Remote Class Information.\n", fName); \
            goto error; \
        } \
    } while(0)

    NGL_ALLOCATE(src, dest, ngrci_className);
    NGL_ALLOCATE(src, dest, ngrci_version);
    if (src->ngrci_description != NULL) {
	NGL_ALLOCATE(src, dest, ngrci_description);
    }
#undef NGL_ALLOCATE

    /* Copy methods */
    dest->ngrci_method = ngcliRemoteMethodInformationAllocate(
        context, src->ngrci_nMethods, error);
    if (dest->ngrci_method == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate RemoteMethodInformation.\n", fName);
        goto error;
    }

    for (i = 0; i < src->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationCopy(context,
            &src->ngrci_method[i],
            &dest->ngrci_method[i], error);
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy RemoteMethodInformation.\n", fName);
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
        globus_libc_free(dest->ngrci_className);
    }
    if (dest->ngrci_version != NULL) {
        globus_libc_free(dest->ngrci_version);
    }
    if (dest->ngrci_description != NULL) {
        globus_libc_free(dest->ngrci_description);
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
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
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
    int result, i;
    static const char *fName = "ngcllRemoteClassInformationRelease";

    /* Check the arguments */
    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: rcInfo is NULL.\n", fName);
        return 0;
    }
    assert(rcInfo->ngrci_className != NULL);
    assert(rcInfo->ngrci_version != NULL);
    assert(rcInfo->ngrci_nMethods > 0);
    assert(rcInfo->ngrci_method != NULL);

    /* Release Method Information */
    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
        result = ngcliRemoteMethodInformationRelease(
            context, &rcInfo->ngrci_method[i], error);
        if (result != 1) {
           ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
           "%s: Can't release RemoteMethodInformation.\n", fName);
            return 0;
        }
    } 

    /* Deallocate the members */
    globus_libc_free(rcInfo->ngrci_className);
    globus_libc_free(rcInfo->ngrci_version);
    if (rcInfo->ngrci_description != NULL) {
        globus_libc_free(rcInfo->ngrci_description);
    }

    result = ngcliRemoteMethodInformationFree(
        context, rcInfo->ngrci_method, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't free RemoteMethodInformation.\n",
            fName);
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
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
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
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
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
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    char *className,
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int result, found;
    static const char fName[] = "ngcllRemoteClassInformationGetCopy";

    /* Check the arguments (rmInfoMng can be NULL) */
    if ((className == NULL) || (rcInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Find the Remote Class Information (not found is valid) */
    result = ngcllRemoteClassInformationLockListCopy(
        context, 1, className, rcInfo, &found, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Remote Class Information.\n", fName);
        return 0;
    }
    if (found == 1) {
        /* Success */
        return 1;
    }

    /* Get Remote Class Information from MDS */
    result = ngcliMDSaccessRemoteExecutableInformationGet(
                                 context, rmInfoMng, className, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to access MDS server.\n", fName);
        return 0;
    }

    /* Get the Remote Class Information (not found is invalid) */
    result = ngcllRemoteClassInformationLockListCopy(
        context, 1, className, rcInfo, &found, error);
    if ((result != 1) || (found != 1)) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Remote Class Information.\n", fName);
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
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllRemoteClassInformationLockListCopy";

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(dstRcInfo != NULL);
    assert(found != NULL);

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Class Information.\n", fName);
        return 0;
    }

    returnCode = 0;

    /* Find the Remote Class Information */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(context, className, error);
    if (rcInfoMng == NULL) {
        *found = 0; 
        if (isNotFoundValid != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Remote Class Information.\n", fName);
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
        if (result != 1) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the Remote Class Information.\n", fName);
            returnCode = 0;
        } else {
            returnCode = 1;
        }
    }

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Class Information.\n", fName);
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
    ngclRemoteClassInformationManager_t *srcRcInfoMng,
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
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error,
            "%s: Can't lock the Remote Class Information.\n", fName);
        return 0;
    }

    returnCode = 0;

    /* Copy the Remote Class Information */
    result = ngcliRemoteClassInformationCopy(
        context, &srcRcInfoMng->ngrcim_info, dstRcInfo, error);
    if (result == 1) {
        returnCode = 1;
    } else {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Remote Class Information.\n", fName);
        returnCode = 0;
    }

    result = ngcliRemoteClassInformationReadUnlock(srcRcInfoMng, log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't unlock the Remote Class Information.\n", fName);
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
    ngclRemoteClassInformationManager_t *dstRcInfoMng,
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
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the Remote Class Information for \"%s\".\n",
        fName, srcRcInfo->ngrci_className);

    /* Lock */
    result = ngcliRemoteClassInformationWriteLock(
        dstRcInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Remote Class Information.\n",
            fName);
        goto error;
    }
    rcLocked = 1;
  
    /* Release the Remote Class Information */
    result = ngcllRemoteClassInformationRelease(
        context, &dstRcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Class Information.\n",
            fName);
        goto error;
    }

    /* Copy the Remote Class Information */
    result = ngcliRemoteClassInformationCopy(
        context, srcRcInfo, &dstRcInfoMng->ngrcim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Remote Class Information.\n",
            fName);
        goto error;
    }

    /* Unlock */
    result = ngcliRemoteClassInformationWriteUnlock(
        dstRcInfoMng, log, error);
    rcLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Remote Class Information.\n",
            fName);
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
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Remote Class Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;
}

