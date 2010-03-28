#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclLocalMachine.c,v $ $Revision: 1.37 $ $Date: 2007/12/26 12:27:17 $";
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
 * Local Machine Information modules for Ninf-G Client.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngclLocalMachineInformationManager_t *
ngcllLocalMachineInformationManagerAllocate(
    ngclContext_t *, int *);
static int
ngcllLocalMachineInformationManagerFree(
    ngclContext_t *,
    ngclLocalMachineInformationManager_t *,
    int *);
static ngclLocalMachineInformationManager_t *
ngcllLocalMachineInformationConstruct(
    ngclContext_t *,
    ngclLocalMachineInformation_t *,
    int *);
static int
ngcllLocalMachineInformationDestruct(
    ngclContext_t *,
    ngclLocalMachineInformationManager_t *,
    int *);
static int
ngcllLocalMachineInformationManagerInitialize(
    ngclContext_t *,
    ngclLocalMachineInformationManager_t *,
    ngclLocalMachineInformation_t *,
    int *);
static int
ngcllLocalMachineInformationManagerFinalize(
    ngclContext_t *,
    ngclLocalMachineInformationManager_t *,
    int *);
static void
ngcllLocalMachineInformationInitializeMember(
    ngclLocalMachineInformation_t *);
static void
ngcllLocalMachineInformationInitializePointer(
    ngclLocalMachineInformation_t *lmInfo);
static int ngcllLocalMachineInformationGetCopy(
    ngclContext_t *, ngclLocalMachineInformation_t *, int *);
static int ngcllLocalMachineInformationRelease(
    ngclContext_t *context, ngclLocalMachineInformation_t *lmInfo, int *error);

/**
 * Information append.
 */
int
ngcliLocalMachineInformationCacheRegister(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    ngclLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcliLocalMachineInformationCacheRegister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (lmInfo == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
	return 0;
    }

    /* Note : No Replace for Local Machine Information. */

    /* Construct */
    lmInfoMng = ngcllLocalMachineInformationConstruct(context, lmInfo, error);
    if (lmInfoMng == NULL) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct Local Machine Information.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Information delete.
 */
int
ngcliLocalMachineInformationCacheUnregister(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationCacheUnregister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Lock the list */
    result = ngcliLocalMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock LocalMachineInformation list.\n", fName);
	return 0;
    }

    /* Is information registered? */
    if (context->ngc_lmInfo == NULL) {
	/* Not registered */
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLocalMachineInformationListWriteUnlock(context,
	    context->ngc_log, NULL);
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister DefaultRemoteMachineInformation list.\n",
	    fName);
	return 0;
    }

    /* Delete */
    result = ngcllLocalMachineInformationDestruct(
	context, context->ngc_lmInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct LocalMachineInformation.\n", fName);
	return 0;
    }

    /* Unlock the list */
    result = ngcliLocalMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock LocalMachineInformation list.\n", fName);
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
ngclLocalMachineInformationManager_t *
ngcliLocalMachineInformationCacheGet(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL, 
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    if (context->ngc_lmInfo == NULL) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get LocalMachineInformation list.\n", fName);
        return NULL;
    }
    return context->ngc_lmInfo;
}

/**
 * Construct.
 */
static ngclLocalMachineInformationManager_t *
ngcllLocalMachineInformationConstruct(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    ngclLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcllLocalMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfo != NULL);

    /* Allocate */
    lmInfoMng = ngcllLocalMachineInformationManagerAllocate(context, error);
    if (lmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Local Machine Information.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllLocalMachineInformationManagerInitialize(
        context, lmInfoMng, lmInfo, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't initialize the Local Machine Information.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclContextRegisterLocalMachineInformation(
    	context, lmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Local Machine Information for Ninf-G Context.\n",
	    fName);
	goto error;
    }

    /* Success */
    return lmInfoMng;

    /* Error occurred */
error:
    result = ngcllLocalMachineInformationDestruct(context, lmInfoMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't free the storage for Local Machine Information Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct.
 */
static int
ngcllLocalMachineInformationDestruct(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfoMng != NULL);

    /* Unregister */
    result = ngclContextUnregisterLocalMachineInformation(context, lmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Local Machine Information.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllLocalMachineInformationManagerFinalize(context, lmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Local Machine Information.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllLocalMachineInformationManagerFree(context, lmInfoMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Local Machine Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}


/**
 * Allocate the information storage.
 */
static ngclLocalMachineInformationManager_t *
ngcllLocalMachineInformationManagerAllocate(
    ngclContext_t *context,
    int *error)
{
    ngclLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcllLocalMachineInformationManagerAllocate";

    /* Check the arguments */
    assert(context != NULL);

    /* Allocate new storage */
    lmInfoMng = globus_libc_calloc(1,
	sizeof (ngclLocalMachineInformationManager_t));
    if (lmInfoMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage "
            "for LocalMachineInformationManager.\n", fName);
	return NULL;
    }

    return lmInfoMng;
}

/**
 * Free the information storage.
 */
static int
ngcllLocalMachineInformationManagerFree(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfoMng != NULL);

    globus_libc_free(lmInfoMng);

    return 1;
}

/**
 * Allocate the information storage. (not Manager)
 */
ngclLocalMachineInformation_t *
ngcliLocalMachineInformationAllocate(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclLocalMachineInformation_t *lmInfo;
    static const char fName[] = "ngcliLocalMachineInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Allocate new storage */
    lmInfo = globus_libc_calloc(1, sizeof (ngclLocalMachineInformation_t));
    if (lmInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage "
            "for LocalMachineInformationManager.\n", fName);
	return NULL;
    }

    return lmInfo;
}

/**
 * Free the information storage. (not Manager)
 */
int
ngcliLocalMachineInformationFree(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    globus_libc_free(lmInfo);

    return 1;
}

/**
 * Initialize.
 */
static int
ngcllLocalMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngclLocalMachineInformationManager_t *lmInfoMng,
     ngclLocalMachineInformation_t *lmInfo,
     int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationManagerInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfoMng != NULL);
    assert(lmInfo != NULL);

    /* Copy to new information */
    result = ngcliLocalMachineInformationCopy(context, lmInfo,
	    &lmInfoMng->nglmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't copy the Local Machine Information.\n", fName);
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&lmInfoMng->nglmim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
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
ngcllLocalMachineInformationManagerFinalize(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationManagerFinalize";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfoMng != NULL);

    /* Destroy the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&lmInfoMng->nglmim_rwlOwn,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Release the information */
    result = ngclLocalMachineInformationRelease(context,
	&lmInfoMng->nglmim_info, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Local Machine Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Copy the information.
 */
int
ngcliLocalMachineInformationCopy(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *src,
    ngclLocalMachineInformation_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationCopy";

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
    ngcllLocalMachineInformationInitializeMember(dest);

    /* Copy the members */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllLocalMachineInformationInitializePointer(dest);

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
                "for Local Machine Information.\n", fName); \
            goto error; \
        } \
    } while(0)

    if (src->nglmi_hostName)
        NGL_ALLOCATE(src, dest, nglmi_hostName);
    if (src->nglmi_gassUrl != NULL)
	NGL_ALLOCATE(src, dest, nglmi_gassUrl);
    if (src->nglmi_tmpDir != NULL)
	NGL_ALLOCATE(src, dest, nglmi_tmpDir);
    if (src->nglmi_invokeServerLog != NULL)
	NGL_ALLOCATE(src, dest, nglmi_invokeServerLog);
#undef NGL_ALLOCATE

    /* Copy the Log Information */
    result = ngLogInformationCopy(
        &src->nglmi_logInfo, &dest->nglmi_logInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the Log Information.\n", fName);
        goto error;
    }

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclLocalMachineInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Local Machine Information.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngclLocalMachineInformationRelease(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclLocalMachineInformationRelease";

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

    result = ngcllLocalMachineInformationRelease(context, lmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllLocalMachineInformationRelease(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationRelease";

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Deallocate the members */
    if (lmInfo->nglmi_hostName != NULL)
        globus_libc_free(lmInfo->nglmi_hostName);
    if (lmInfo->nglmi_gassUrl != NULL)
	globus_libc_free(lmInfo->nglmi_gassUrl);
    if (lmInfo->nglmi_tmpDir != NULL)
	globus_libc_free(lmInfo->nglmi_tmpDir);
    if (lmInfo->nglmi_invokeServerLog != NULL)
	globus_libc_free(lmInfo->nglmi_invokeServerLog);

    /* Release LogInformation */
    result = ngLogInformationRelease(&lmInfo->nglmi_logInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't release Log Information.\n",
            fName);
        return 0;
    }

    /* Initialize the members */
    ngcllLocalMachineInformationInitializeMember(lmInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliLocalMachineInformationInitialize(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    ngcllLocalMachineInformationInitializeMember(lmInfo);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllLocalMachineInformationInitializeMember(
    ngclLocalMachineInformation_t *lmInfo)
{
    /* Initialize the members */
    ngcllLocalMachineInformationInitializePointer(lmInfo);
    ngLogInformationInitialize(&lmInfo->nglmi_logInfo);
    lmInfo->nglmi_saveNsessions = 0;
    lmInfo->nglmi_gassPort = 0;
    lmInfo->nglmi_refreshInterval = 0;
    lmInfo->nglmi_fortranCompatible = 0;
    lmInfo->nglmi_listenPort = 0U;
    lmInfo->nglmi_listenPortAuthOnly = 0U;
    lmInfo->nglmi_listenPortGSI = 0U;
    lmInfo->nglmi_listenPortSSL = 0U;
    lmInfo->nglmi_tcpNodelay = 0;/* false */
}

/**
 * Initialize the pointers.
 */
static void
ngcllLocalMachineInformationInitializePointer(
    ngclLocalMachineInformation_t *lmInfo)
{
    /* Initialize the pointers */
    lmInfo->nglmi_hostName = NULL;
    lmInfo->nglmi_logInfo.ngli_filePath = NULL;
    lmInfo->nglmi_logInfo.ngli_suffix = NULL;
    lmInfo->nglmi_gassUrl = NULL;
    lmInfo->nglmi_tmpDir = NULL;
    lmInfo->nglmi_invokeServerLog = NULL;
}

/**
 * GetCopy
 */
int
ngclLocalMachineInformationGetCopy(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclLocalMachineInformationGetCopy";

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

    result = ngcllLocalMachineInformationGetCopy(
	context, lmInfo, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllLocalMachineInformationGetCopy(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    ngclLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcllLocalMachineInformationGetCopy";

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Lock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadLock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Local Machine Information.\n", fName);
        return 0;
    }

    /* Get the Local Machine Information */
    lmInfoMng = ngcliLocalMachineInformationCacheGet(context, error);
    if (lmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Local Machine Information.\n", fName);
        goto error;
    }

    /* Copy the Local Machine Information */
    result = ngcliLocalMachineInformationCopy(context,
                    &lmInfoMng->nglmim_info, lmInfo, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, error, 
            "%s: Can't copy the Local Machine Information.\n", fName);
        goto error;
    }

    /* Unlock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Local Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result != 1) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Local Machine Information.\n",
            fName);
        return 0;
    }

    /* Failed */
    return 0;
}

