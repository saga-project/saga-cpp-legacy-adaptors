/*
 * $RCSfile: ngclDefaultRemoteMachine.c,v $ $Revision: 1.11 $ $Date: 2008/01/28 06:58:00 $
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
 * Default Remote Machine Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclDefaultRemoteMachine.c,v $ $Revision: 1.11 $ $Date: 2008/01/28 06:58:00 $")

/**
 * Prototype declaration of static functions.
 */
static ngcliRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationConstruct(
    ngclContext_t *,
    ngclRemoteMachineInformation_t *,
    int *);
static int
ngcllDefaultRemoteMachineInformationDestruct(
    ngclContext_t *,
    ngcliRemoteMachineInformationManager_t *,
    int *);
static int
ngcllDefaultRemoteMachineInformationManagerInitialize(
     ngclContext_t *,
     ngcliRemoteMachineInformationManager_t *,
     ngclRemoteMachineInformation_t *,
     int *);
static int
ngcllDefaultRemoteMachineInformationManagerFinalize(
    ngclContext_t *,
    ngcliRemoteMachineInformationManager_t *,
    int *);
static int ngcllDefaultRemoteMachineInformationGetCopy(
    ngclContext_t *, ngclRemoteMachineInformation_t *, int *);
static int
ngcllDefaultRemoteMachineInformationReplace(
    ngclContext_t *,
    ngcliRemoteMachineInformationManager_t *,
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcliDefaultRemoteMachineInformationCacheRegister";

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
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(
        context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Default Remote Machine Information list.\n"); 
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
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't replace the Default Remote Machine Information.\n"); 
            goto error;
        }
    }

    /* Unlock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteUnlock(
        context, log, error);
    listLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Default Remote Machine Information list.\n"); 
        goto error;
    }

    /* Construct */
    if (rmInfoMng == NULL) {
        rmInfoMng = ngcllDefaultRemoteMachineInformationConstruct(
            context, rmInfo, error);
        if ((rmInfoMng == NULL) || (context->ngc_rmInfo_default == NULL)) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Default Remote Machine Information.\n"); 
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
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock"
                " the Default Remote Machine Information list.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock DefaultRemoteMachineInformation list.\n"); 
	return 0;
    }

    /* Is information registered? */
    if (context->ngc_rmInfo_default == NULL) {
	/* Not registered */
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	    context->ngc_log, NULL);
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister DefaultRemoteMachineInformation list.\n"); 
	return 0;
    }

    /* Delete */
    result = ngcllDefaultRemoteMachineInformationDestruct(context,
	context->ngc_rmInfo_default, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct DefaultRemoteMachineInformation.\n"); 
	return 0;
    }

    /* Unlock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock DefaultRemoteMachineInformation list.\n"); 
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
ngcliRemoteMachineInformationManager_t *
ngcliDefaultRemoteMachineInformationCacheGet(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliDefaultRemoteMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    if (context->ngc_rmInfo_default == NULL) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Default Remote Machine Information was not found.\n"); 
        return NULL;
    }

    return context->ngc_rmInfo_default;
}

/**
 * Construct.
 */
static ngcliRemoteMachineInformationManager_t *
ngcllDefaultRemoteMachineInformationConstruct(
    ngclContext_t *context,
    ngclRemoteMachineInformation_t *rmInfo,
    int *error)
{
    int result;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfo != NULL);

    /* Allocate */
    rmInfoMng = NGI_ALLOCATE(ngcliRemoteMachineInformationManager_t, context->ngc_log, error);
    if (rmInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Default Remote Machine Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllDefaultRemoteMachineInformationManagerInitialize(
        context, rmInfoMng, rmInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Default Remote Machine Information.\n"); 
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterDefaultRemoteMachineInformation(
    	context, rmInfoMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Default Remote Machine Information for Ninf-G Context.\n"); 
	goto error;
    }

    /* Success */
    return rmInfoMng;

    /* Error occurred */
error:
    result = ngcllDefaultRemoteMachineInformationDestruct(context,
	rmInfoMng, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the storage for Default Remote Machine Information Manager.\n"); 
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterDefaultRemoteMachineInformation(context,
	rmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Default Remote Machine Information.\n"); 
	return 0;
    }

    /* Finalize */
    result = ngcllDefaultRemoteMachineInformationManagerFinalize(context,
	rmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Default Remote Machine Information.\n"); 
	return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(
        ngcliRemoteMachineInformationManager_t, rmInfoMng, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Default Remote Machine Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize.
 */
static int
ngcllDefaultRemoteMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngcliRemoteMachineInformationManager_t *rmInfoMng,
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Default Remote Machine Information.\n"); 
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

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
static int
ngcllDefaultRemoteMachineInformationManagerFinalize(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] =
        "ngcllDefaultRemoteMachineInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(rmInfoMng != NULL);

    log = context->ngc_log;

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&rmInfoMng->ngrmim_rwlOwn,
	context->ngc_log, error);
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
            "Can't release the Default Remote Machine Information.\n"); 
	return 0;
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
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllDefaultRemoteMachineInformationGetCopy";

    /* Check the arguments */
    if (rmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
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

    /* Find Default Remote Machine Information */
    rmInfoMng = ngcliDefaultRemoteMachineInformationCacheGet(context, error);
    if (rmInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Default Remote Machine Information.\n"); 
        goto error;
    }

    /* Copy the Default Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(context,
                    &(rmInfoMng->ngrmim_info), rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Default Remote Machine Information.\n"); 
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

/**
 * Replace the Default Remote Machine Information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
static int
ngcllDefaultRemoteMachineInformationReplace(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *dstRmInfoMng,
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
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Replace the Default Remote Machine Information.\n"); 

    /* Lock */
    result = ngcliRemoteMachineInformationWriteLock(
        dstRmInfoMng, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the Default Remote Machine Information.\n"); 
        goto error;
    }
    rmLocked = 1;

    /* Release the Default Remote Machine Information */
    result = ngclRemoteMachineInformationRelease(
        context, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Default Remote Machine Information.\n"); 
        goto error;
    }

    /* Copy the Default Remote Machine Information */
    result = ngcliRemoteMachineInformationCopy(
        context, srcRmInfo, &dstRmInfoMng->ngrmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Default Remote Machine Information.\n"); 
        goto error;
    }

    /* Unlock */
    result = ngcliRemoteMachineInformationWriteUnlock(
        dstRmInfoMng, log, error);
    rmLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the Default Remote Machine Information.\n"); 
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
                "Can't write unlock"
                " the Default Remote Machine Information.\n"); 
        }
    }
  
    /* Failed */
    return 0;


}

