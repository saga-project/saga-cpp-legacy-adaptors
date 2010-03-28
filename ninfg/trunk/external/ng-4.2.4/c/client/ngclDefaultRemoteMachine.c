#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclDefaultRemoteMachine.c,v $ $Revision: 1.18 $ $Date: 2005/07/04 08:49:47 $";
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
 * Default Remote Machine Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngclRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationManagerAllocate(
    ngclContext_t *,
    int *);
static int
ngcllDefaultRemoteMachineInformationManagerFree(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static ngclRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationConstruct(
    ngclContext_t *,
    ngclRemoteMachineInformation_t *,
    int *);
static int
ngcllDefaultRemoteMachineInformationDestruct(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static int
ngcllDefaultRemoteMachineInformationManagerInitialize(
     ngclContext_t *,
     ngclRemoteMachineInformationManager_t *,
     ngclRemoteMachineInformation_t *,
     int *);
static int
ngcllDefaultRemoteMachineInformationManagerFinalize(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    int *);
static int ngcllDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
static int
ngcllDefaultRemoteMachineInformationReplace(
    ngclContext_t *,
    ngclRemoteMachineInformationManager_t *,
    ngclRemoteMachineInformation_t *, int *);

/**
 * Information append.
 */
int
ngcliDefaultRemoteMachineInformationCacheRegister(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    ngLog_t *log;
    int result, listLocked, subError;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliDefaultRemoteMachineInformationCacheRegister";

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
        (rmInfo->ngrmi_gassScheme == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Default Remote Machine Information list.\n",
            fName);
        goto error;
    }
    listLocked = 1;

    /* Is Default Remote Machine Information available? */
    rmInfoMng = ngcliDefaultRemoteMachineInformationCacheGet(
        context, &subError);
    if (rmInfoMng != NULL) {

        /* Replace the Default Remote Machine Information */
        result = ngcllDefaultRemoteMachineInformationReplace(
            context, rmInfoMng, rmInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't replace the Default Remote Machine Information.\n",
                fName);
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Default Remote Machine Information list.\n",
            fName);
        goto error;
    }

    /* Construct */
    if (rmInfoMng == NULL) {
        rmInfoMng = ngcllDefaultRemoteMachineInformationConstruct(
            context, rmInfo, error);
        if ((rmInfoMng == NULL) || (context->ngc_rmInfo_default == NULL)) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Default Remote Machine Information.\n",
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
        result = ngcliDefaultRemoteMachineInformationListWriteUnlock(
            context, log, NULL);
        listLocked = 0;
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock"
                " the Default Remote Machine Information list.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Information delete.
 */
int
ngcliDefaultRemoteMachineInformationCacheUnregister(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] =
	"ngcliDefaultRemoteMachineInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock DefaultRemoteMachineInformation list.\n", fName);
	return 0;
    }

    /* Is information registered? */
    if (context->ngc_rmInfo_default == NULL) {
	/* Not registered */
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	    context->ngc_log, NULL);
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister DefaultRemoteMachineInformation list.\n",
	    fName);
	return 0;
    }

    /* Delete */
    result = ngcllDefaultRemoteMachineInformationDestruct(context,
	context->ngc_rmInfo_default, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct DefaultRemoteMachineInformation.\n", fName);
	return 0;
    }

    /* Unlock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock DefaultRemoteMachineInformation list.\n", fName);
	return 0;
    }

    return 1;
}

/**
 * Get the information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclRemoteMachineInformationManager_t *
ngcliDefaultRemoteMachineInformationCacheGet(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliDefaultRemoteMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    if (context->ngc_rmInfo_default == NULL) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: Default Remote Machine Information was not found.\n", fName);
        return NULL;
    }

    return context->ngc_rmInfo_default;
}

/**
 * Construct.
 */
static ngclRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationConstruct(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    /* Allocate */
    rmInfoMng = ngcllDefaultRemoteMachineInformationManagerAllocate(context, error);
    if (rmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Default Remote Machine Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllDefaultRemoteMachineInformationManagerInitialize(
        context, rmInfoMng, rmInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't initialize the Default Remote Machine Information.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclContextRegisterDefaultRemoteMachineInformation(
    	context, rmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Default Remote Machine Information for Ninf-G Context.\n",
	    fName);
	goto error;
    }

    /* Success */
    return rmInfoMng;

    /* Error occurred */
error:
    result = ngcllDefaultRemoteMachineInformationDestruct(context,
	rmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for Default Remote Machine Information Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllDefaultRemoteMachineInformationDestruct(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Unregister */
    result = ngclContextUnregisterDefaultRemoteMachineInformation(context,
	rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Default Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Finalize */
    result = ngcllDefaultRemoteMachineInformationManagerFinalize(context,
	rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Default Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Deallocate */
    result =
	ngcllDefaultRemoteMachineInformationManagerFree(context, rmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Default Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * Allocate the information storage.
 */
static ngclRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] =
        "ngcllDefaultRemoteMachineInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    rmInfoMng = globus_libc_calloc(1,
	sizeof (ngclRemoteMachineInformationManager_t));
    if (rmInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Default Remote Machine Information Manager.\n",
            fName);

	return NULL;
    }

    return rmInfoMng;
}

/**
 * Free the information storage.
 */
static int
ngcllDefaultRemoteMachineInformationManagerFree(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfo,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    globus_libc_free(rmInfo);

    return 1;
}

/**
 * Initialize.
 */
static int
ngcllDefaultRemoteMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngclRemoteMachineInformationManager_t *rmInfoMng,
     ngclRemoteMachineInformation_t *rmInfo,
     int *error)
{
    int result;
    static const char fName[] =
	"ngcllDefaultRemoteMachineInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);
    assert(rmInfo != NULL);

    /* Copy to new information */
    result = ngcliRemoteMachineInformationCopy(context, rmInfo,
	    &rmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the Default Remote Machine Information.\n", fName);
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
    result = ngiRWlockInitialize(&rmInfoMng->ngrmim_rwlExecPath,
        context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Read/Write Lock for list of Executable Path.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllDefaultRemoteMachineInformationManagerFinalize(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] =
        "ngcllDefaultRemoteMachineInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&rmInfoMng->ngrmim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&rmInfoMng->ngrmim_rwlExecPath,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    assert(rmInfoMng->ngrmim_epInfo_head == NULL);
    assert(rmInfoMng->ngrmim_epInfo_tail == NULL);

    /* Release the information */
    result = ngclRemoteMachineInformationRelease(context, 
	&rmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Default Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* reset members */
    if (rmInfoMng->ngrmim_mdsServer != NULL) {
	globus_libc_free(rmInfoMng->ngrmim_mdsServer);
	rmInfoMng->ngrmim_mdsServer = NULL;
    }
    if (rmInfoMng->ngrmim_hostDN != NULL) {
	globus_libc_free(rmInfoMng->ngrmim_hostDN);
	rmInfoMng->ngrmim_hostDN = NULL;
    }
    rmInfoMng->ngrmim_next = NULL;

    /* Success */
    return 1;
}

/**
 * GetCopy
 */
int
ngclDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclDefaultRemoteMachineInformationGetCopy";

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

    result = ngcllDefaultRemoteMachineInformationGetCopy(context,
	rmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationGetCopy";

    /* Check the arguments */
    if (rmInfo == NULL) {
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

    /* Find Default Remote Machine Information */
    rmInfoMng = ngcliDefaultRemoteMachineInformationCacheGet(context, error);
    if (rmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Default Remote Machine Information.\n", fName);
        goto error;
    }

    /* Copy the Default Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(context,
                    &(rmInfoMng->ngrmim_info), rmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't copy the Default Remote Machine Information.\n",
            fName);
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

/**
 * Replace the Default Remote Machine Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllDefaultRemoteMachineInformationReplace(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *dstRmInfoMng,
    ngclRemoteMachineInformation_t *srcRmInfo,
    int *error)
{
    ngLog_t *log;
    int result, rmLocked;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationReplace";

    /* Check the arguments */
    assert(context != NULL);
    assert(dstRmInfoMng != NULL);
    assert(srcRmInfo != NULL);

    log = context->ngc_log;
    rmLocked = 0;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Replace the Default Remote Machine Information.\n",
        fName);

    /* Lock */
    result = ngcliRemoteMachineInformationWriteLock(
        dstRmInfoMng, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Default Remote Machine Information.\n",
            fName);
        goto error;
    }
    rmLocked = 1;

    /* Release the Default Remote Machine Information */
    result = ngclRemoteMachineInformationRelease(
        context, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Default Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Copy the Default Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(
        context, srcRmInfo, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Default Remote Machine Information.\n",
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
            "%s: Can't write unlock the Default Remote Machine Information.\n",
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
                "%s: Can't write unlock"
                " the Default Remote Machine Information.\n",
                fName);
        }
    }
  
    /* Failed */
    return 0;


}

