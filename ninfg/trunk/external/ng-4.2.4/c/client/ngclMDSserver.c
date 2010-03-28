#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclMDSserver.c,v $ $Revision: 1.30 $ $Date: 2006/01/06 03:03:52 $";
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
 * MDS Server Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngcliMDSserverInformationManager_t *
    ngcllMDSserverInformationManagerAllocate(ngclContext_t *, int *);
static int
ngcllMDSserverInformationManagerFree(
    ngclContext_t *,
    ngcliMDSserverInformationManager_t *,
    int *);
static ngcliMDSserverInformationManager_t *
ngcllMDSserverInformationConstruct(
    ngclContext_t *,
    ngclMDSserverInformation_t *,
    int *);
static int
ngcllMDSserverInformationDestruct(
    ngclContext_t *,
    ngcliMDSserverInformationManager_t *,
    int *);
static int
ngcllMDSserverInformationManagerInitialize(
     ngclContext_t *,
     ngcliMDSserverInformationManager_t *,
     ngclMDSserverInformation_t *,
     int *);
static int
ngcllMDSserverInformationManagerFinalize(
    ngclContext_t *,
    ngcliMDSserverInformationManager_t *,
    int *);
static void
ngcllMDSserverInformationManagerInitializeMember(
     ngcliMDSserverInformationManager_t *);
static void
ngcllMDSserverInformationInitializeMember(
    ngclMDSserverInformation_t *);
static void
ngcllMDSserverInformationInitializePointer(
    ngclMDSserverInformation_t *mdsInfo);
static int ngcllMDSserverInformationGetCopy(
    ngclContext_t *, char *, ngclMDSserverInformation_t *, int *);
static int ngcllMDSserverInformationRelease(
    ngclContext_t *, ngclMDSserverInformation_t *, int *);
static int ngcllMDSserverInformationReplace(
    ngclContext_t *, ngcliMDSserverInformationManager_t *,
    ngclMDSserverInformation_t *, int *);

/**
 * Information append at last of the list.
 */
int
ngcliMDSserverInformationCacheRegister(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    static const char fName[] = "ngcliMDSserverInformationCacheRegister";

    log = NULL;
    listLocked = 0;
    mdsInfoMng = NULL;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if ((mdsInfo == NULL) ||
        (mdsInfo->ngmsi_hostName == NULL) ||
        (mdsInfo->ngmsi_voName == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliMDSserverInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock MDSserverInformation list.\n", fName);
        goto error;
    }
    listLocked = 1;

    /* Is MDS Server Information available? */
    mdsInfoMng = ngcliMDSserverInformationCacheGet(
        context, mdsInfo->ngmsi_tagName, mdsInfo->ngmsi_hostName,
        NGCLI_MDS_SERVER_CACHE_GET_MODE_PRECISE, &subError);
    if (mdsInfoMng != NULL) {

        /* Replace the MDS Server Information */
        result = ngcllMDSserverInformationReplace(
            context, mdsInfoMng, mdsInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the MDS Server Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliMDSserverInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't write unlock the MDS server Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (mdsInfoMng == NULL) {
        mdsInfoMng = ngcllMDSserverInformationConstruct(
            context, mdsInfo, error);
        if (mdsInfoMng == NULL) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct MDS Server Information.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    if (listLocked != 0) {
        result = ngcliMDSserverInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't write unlock the MDS server Information list.\n",
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
ngcliMDSserverInformationCacheUnregister(
    ngclContext_t *context,
    char *tagName,
    char *hostName,
    int *error)
{
    int result;
    ngcliMDSserverInformationManager_t *curr;
    static const char fName[] = "ngcliMDSserverInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context Invalid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliMDSserverInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock MDSserverInformation list.\n", fName);
	return 0;
    }

    if ((tagName == NULL) && (hostName == NULL)) {
	/* Delete all information */

	/* Get the data from the head of a list */
        curr = ngcliMDSserverInformationCacheGetNext(context, NULL, error);
	if (curr == NULL) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_DEBUG, NULL,
		"%s: No MDS server Information was registered.\n", fName);
	}

        while (curr != NULL) {
	    /* Destruct the data */
            result = ngcllMDSserverInformationDestruct(context, curr, error);
	    if (result == 0) {
	        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_FATAL, NULL,
		    "%s: Can't destruct MDS server Information.\n", fName);
		goto error;
	    }

	    /* Get next data from the list */
            curr = ngcliMDSserverInformationCacheGetNext(context, NULL, error);
	}
    } else {
        /* Delete specified information */

	/* Get the data from the list by MDS servername */
	curr = ngcliMDSserverInformationCacheGet(
            context, tagName, hostName,
            NGCLI_MDS_SERVER_CACHE_GET_MODE_PRECISE, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: MDS server Information Tag \"%s\" host \"%s\""
                " is not found.\n",
		fName, ((tagName != NULL) ? tagName : "none"),
                ((hostName != NULL) ? hostName : "none"));
	    goto error;
	}

	/* Destruct the data */
        result = ngcllMDSserverInformationDestruct(context, curr, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't destruct MDS server Information.\n", fName);
	    goto error;
	}
    }

    /* Unlock the list */
    result = ngcliMDSserverInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of MDS server Information.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcliMDSserverInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of MDS server Information.\n",
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
ngcliMDSserverInformationManager_t *
ngcliMDSserverInformationCacheGet(
    ngclContext_t *context,
    char *tagName,
    char *hostName,
    ngcliMDSserverInformationCacheGetMode_t mode,
    int *error)
{
    int result;
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    static const char fName[] = "ngcliMDSserverInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if ((tagName == NULL) && (hostName == NULL)) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: tag name and hostname is NULL.\n", fName);
	return NULL;
    }
    assert(mode != NGCLI_MDS_SERVER_CACHE_GET_NONE);

    mdsInfoMng = context->ngc_mdsInfo_head;
    for (; mdsInfoMng != NULL; mdsInfoMng = mdsInfoMng->ngmsim_next) {

        if (tagName != NULL) {
	    if ((mdsInfoMng->ngmsim_info.ngmsi_tagName != NULL) &&
                (strcmp(mdsInfoMng->ngmsim_info.ngmsi_tagName, tagName)
                    == 0)) {
                /* Found */
                return mdsInfoMng;
            }
        } else {
	    assert(mdsInfoMng->ngmsim_info.ngmsi_hostName != NULL);
            if (strcmp(mdsInfoMng->ngmsim_info.ngmsi_hostName, hostName) == 0) {

                /* tagName depend skip */
                if (mode == NGCLI_MDS_SERVER_CACHE_GET_MODE_PRECISE) {
                    if (mdsInfoMng->ngmsim_info.ngmsi_tagName != NULL) {
                        continue;
                    }
                }

                /* Found */
                return mdsInfoMng;
            }
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: MDS server Information is not found by "
        " tag name \"%s\" host name \"%s\".\n",
	fName,
        ((tagName != NULL) ? tagName : "null"),
        ((hostName != NULL) ? hostName : "null"));
    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliMDSserverInformationManager_t *
ngcliMDSserverInformationCacheGetNext(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngcliMDSserverInformationCacheGetNext";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    if (current == NULL) {
	/* Return the first information */
	if (context->ngc_mdsInfo_head != NULL) {
	    assert(context->ngc_mdsInfo_tail != NULL);
            return context->ngc_mdsInfo_head;
	}
    } else {
	/* Return the next information */
	if (current->ngmsim_next != NULL) {
	    return current->ngmsim_next;
	}
    }

    /* Not found */
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last MDS Server Information was reached.\n", fName);
    return NULL;
}

/**
 * Construct.
 */
static ngcliMDSserverInformationManager_t *
ngcllMDSserverInformationConstruct(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int result;
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    static const char fName[] = "ngcllMDSserverInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfo != NULL);

    /* Allocate */
    mdsInfoMng = ngcllMDSserverInformationManagerAllocate(context, error);
    if (mdsInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for MDS server Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllMDSserverInformationManagerInitialize(
        context, mdsInfoMng, mdsInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the MDS server Information.\n",
	    fName);
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterMDSserverInformation(
    	context, mdsInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the MDS server Information for Ninf-G Context.\n",
	    fName);
	goto error;
    }

    /* Success */
    return mdsInfoMng;

    /* Error occurred */
error:
    result = ngcllMDSserverInformationDestruct(context, mdsInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for MDS server Information Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllMDSserverInformationDestruct(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllMDSserverInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterMDSserverInformation(context,
	mdsInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the MDS server Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllMDSserverInformationManagerFinalize(context, mdsInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the MDS server Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllMDSserverInformationManagerFree(context, mdsInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the MDS server Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate the information storage.
 */
static ngcliMDSserverInformationManager_t *
ngcllMDSserverInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    static const char fName[] = "ngcllMDSserverInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    mdsInfoMng = globus_libc_calloc(1,
	sizeof (ngcliMDSserverInformationManager_t));
    if (mdsInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for MDS server Information Manager.\n",
	    fName);
	return NULL;
    }

    return mdsInfoMng;
}

/**
 * Free the information storage.
 */
static int
ngcllMDSserverInformationManagerFree(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    globus_libc_free(mdsInfoMng);

    /* Success */
    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngclMDSserverInformation_t *
ngcliMDSserverInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclMDSserverInformation_t *mdsInfo;
    static const char fName[] = "ngcliMDSserverInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    mdsInfo = globus_libc_calloc(1, sizeof (ngclMDSserverInformation_t));
    if (mdsInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for MDS server Information.\n",
	    fName);
	return NULL;
    }

    return mdsInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliMDSserverInformationFree(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliMDSserverInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (mdsInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(mdsInfo);

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllMDSserverInformationManagerInitialize(
     ngclContext_t *context,
     ngcliMDSserverInformationManager_t *mdsInfoMng,
     ngclMDSserverInformation_t *mdsInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllMDSserverInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);
    assert(mdsInfo != NULL);

    /* reset members */
    ngcllMDSserverInformationManagerInitializeMember(mdsInfoMng);

    mdsInfoMng->ngmsim_next = NULL;

    /* Copy to new information */
    result = ngcliMDSserverInformationCopy(context, mdsInfo,
	&mdsInfoMng->ngmsim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the MDS server Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&mdsInfoMng->ngmsim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Initialize the MDS Access */
    if (context->ngc_mdsAccessEnabled != 0) {
        result = ngcliMDSaccessMDSserverInformationInitialize(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize MDS Access on the MDS Server.\n",
                fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllMDSserverInformationManagerFinalize(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllMDSserverInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    /* Finalize the MDS Access */
    if (context->ngc_mdsAccessEnabled != 0) {
        result = ngcliMDSaccessMDSserverInformationFinalize(
            context, mdsInfoMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize MDS Access on the MDS Server.\n",
                fName);
            /* Not return */
        }
    }

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&mdsInfoMng->ngmsim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclMDSserverInformationRelease(context, 
	&mdsInfoMng->ngmsim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the MDS server Information.\n", fName);
	return 0;
    }

    /* reset members */
    ngcllMDSserverInformationManagerInitializeMember(mdsInfoMng);

    /* Success */
    return 1;
}

/**
 * Initialize Member.
 */
static void
ngcllMDSserverInformationManagerInitializeMember(
     ngcliMDSserverInformationManager_t *mdsInfoMng)
{
    /* Check the arguments */
    assert(mdsInfoMng != NULL);

    ngcllMDSserverInformationInitializeMember(&mdsInfoMng->ngmsim_info);

    mdsInfoMng->ngmsim_next = NULL;

#ifndef NGI_NO_MDS2_MODULE
    mdsInfoMng->ngmsim_ldap = NULL;
#endif /* NGI_NO_MDS2_MODULE */
    mdsInfoMng->ngmsim_ldapInitialized = 0;

#ifndef NGI_NO_MDS4_MODULE
    mdsInfoMng->ngmsim_index = NULL;
#endif /* NGI_NO_MDS4_MODULE */
    mdsInfoMng->ngmsim_indexInitialized = 0;

}

/**
 * Copy the information.
 */
int
ngcliMDSserverInformationCopy(
    ngclContext_t *context,
    ngclMDSserverInformation_t *src,
    ngclMDSserverInformation_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngcliMDSserverInformationCopy";

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

    ngcllMDSserverInformationInitializeMember(dest);

    /* Copy values */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllMDSserverInformationInitializePointer(dest);

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
                "for MDS Server Information.\n", fName); \
            goto error; \
        } \
    } while(0)

    /* Copy the strings */
    NGL_ALLOCATE(src, dest, ngmsi_hostName);
    if (src->ngmsi_tagName != NULL) {
        NGL_ALLOCATE(src, dest, ngmsi_tagName);
    }
    if (src->ngmsi_protocol != NULL) {
        NGL_ALLOCATE(src, dest, ngmsi_protocol);
    }
    if (src->ngmsi_path != NULL) {
        NGL_ALLOCATE(src, dest, ngmsi_path);
    }
    if (src->ngmsi_subject != NULL) {
        NGL_ALLOCATE(src, dest, ngmsi_subject);
    }
    NGL_ALLOCATE(src, dest, ngmsi_voName);
#undef NGL_ALLOCATE

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclMDSserverInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the MDS Server Information.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclMDSserverInformationRelease(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclMDSserverInformationRelease";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllMDSserverInformationRelease(context, mdsInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllMDSserverInformationRelease(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    static const char fName[] = "ngcllMDSserverInformationRelease";

    /* Check the arguments */
    if (mdsInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Deallocate the members */
    if (mdsInfo->ngmsi_hostName != NULL) {
        globus_libc_free(mdsInfo->ngmsi_hostName);
    }

    if (mdsInfo->ngmsi_tagName != NULL) {
        globus_libc_free(mdsInfo->ngmsi_tagName);
    }

    if (mdsInfo->ngmsi_protocol != NULL) {
        globus_libc_free(mdsInfo->ngmsi_protocol);
    }

    if (mdsInfo->ngmsi_path != NULL) {
        globus_libc_free(mdsInfo->ngmsi_path);
    }

    if (mdsInfo->ngmsi_subject != NULL) {
        globus_libc_free(mdsInfo->ngmsi_subject);
    }

    if (mdsInfo->ngmsi_voName != NULL) {
        globus_libc_free(mdsInfo->ngmsi_voName);
    }

    /* Initialize the members */
    ngcllMDSserverInformationInitializeMember(mdsInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliMDSserverInformationInitialize(
    ngclContext_t *context,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliMDSserverInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (mdsInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    ngcllMDSserverInformationInitializeMember(mdsInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllMDSserverInformationInitializeMember(
    ngclMDSserverInformation_t *mdsInfo)
{
    /* Initialize the members */
    ngcllMDSserverInformationInitializePointer(mdsInfo);
    mdsInfo->ngmsi_type = NGCL_MDS_SERVER_TYPE_NONE;
    mdsInfo->ngmsi_portNo = 0;
    mdsInfo->ngmsi_clientTimeout = 0;
    mdsInfo->ngmsi_serverTimeout = 0;
}

/**
 * Initialize the variable of pointers.
 */
static void
ngcllMDSserverInformationInitializePointer(
    ngclMDSserverInformation_t *mdsInfo)
{
    /* Initialize the pointers */
    mdsInfo->ngmsi_hostName = NULL;
    mdsInfo->ngmsi_tagName = NULL;
    mdsInfo->ngmsi_protocol = NULL;
    mdsInfo->ngmsi_path = NULL;
    mdsInfo->ngmsi_subject = NULL;
    mdsInfo->ngmsi_voName = NULL;
}

/**
 * GetCopy
 */
int
ngclMDSserverInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclMDSserverInformationGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result != 1) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllMDSserverInformationGetCopy(context,
	hostName, mdsInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllMDSserverInformationGetCopy(
    ngclContext_t *context,
    char *hostName,
    ngclMDSserverInformation_t *mdsInfo,
    int *error)
{
    int result;
    ngcliMDSserverInformationManager_t *mdsInfoMng;
    static const char fName[] = "ngcllMDSserverInformationGetCopy";

    /* Check the arguments */
    if ((hostName == NULL) || (mdsInfo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

   /* Lock the MDS Server Information */
    result = ngcliMDSserverInformationListReadLock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of MDS Server Information.\n", fName);
        return 0;
    }

    /* Get the MDS Server Information */
    mdsInfoMng = ngcliMDSserverInformationCacheGet(
        context, NULL, hostName,
        NGCLI_MDS_SERVER_CACHE_GET_MODE_MATCH, error);
    if (mdsInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the MDS server Information.\n", fName);
        goto error;
    }

    /* Copy the MDS server Information */
    result = ngcliMDSserverInformationCopy(context,
                    &mdsInfoMng->ngmsim_info, mdsInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't copy the MDS server Information.\n", fName);
        goto error;
    }

    /* Unlock the MDS server Information */
    result = ngcliMDSserverInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS server Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliMDSserverInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of MDS server Information.\n",
            fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Replace the MDS Server Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllMDSserverInformationReplace(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *dstMdsInfoMng,
    ngclMDSserverInformation_t *srcMdsInfo,
    int *error)
{
    ngLog_t *log;
    int result, mdsLocked;
    static const char fName[] = "ngcllMDSserverInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstMdsInfoMng != NULL);
    assert(srcMdsInfo != NULL);

    log = context->ngc_log;
    mdsLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the MDS Server Information for \"%s\".\n",
        fName, srcMdsInfo->ngmsi_hostName);

    /* Finalize the MDS Access */
    result = ngcliMDSaccessMDSserverInformationFinalize(
        context, dstMdsInfoMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize MDS Access on the MDS Server.\n",
            fName);
        goto error;
    }

    /* Lock */
    result = ngcliMDSserverInformationWriteLock(
        dstMdsInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the MDS Server Information.\n",
            fName);
        goto error;
    }
    mdsLocked = 1;

    /* Release the MDS Server Information */
    result = ngcllMDSserverInformationRelease(
        context, &dstMdsInfoMng->ngmsim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Copy the MDS Server Information */
    result = ngcliMDSserverInformationCopy(
        context, srcMdsInfo, &dstMdsInfoMng->ngmsim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Unlock */
    result = ngcliMDSserverInformationWriteUnlock(
        dstMdsInfoMng, log, error);
    mdsLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the MDS Server Information.\n",
            fName);
        goto error;
    }

    /* Initialize the MDS Access */
    result = ngcliMDSaccessMDSserverInformationInitialize(
        context, dstMdsInfoMng, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize MDS Access on the MDS Server.\n",
            fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    if (mdsLocked != 0) {
        result = ngcliMDSserverInformationWriteUnlock(
            dstMdsInfoMng, log, NULL);
        mdsLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the MDS Server Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;
}

