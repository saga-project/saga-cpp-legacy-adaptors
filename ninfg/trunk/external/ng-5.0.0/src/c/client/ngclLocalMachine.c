/*
 * $RCSfile: ngclLocalMachine.c,v $ $Revision: 1.13 $ $Date: 2008/01/28 06:58:00 $
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
 * Local Machine Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclLocalMachine.c,v $ $Revision: 1.13 $ $Date: 2008/01/28 06:58:00 $")

/**
 * Prototype declaration of static functions.
 */
static ngcliLocalMachineInformationManager_t *
ngcllLocalMachineInformationConstruct(
    ngclContext_t *,
    ngclLocalMachineInformation_t *,
    int *);
static int
ngcllLocalMachineInformationDestruct(
    ngclContext_t *,
    ngcliLocalMachineInformationManager_t *,
    int *);
static int
ngcllLocalMachineInformationManagerInitialize(
    ngclContext_t *,
    ngcliLocalMachineInformationManager_t *,
    ngclLocalMachineInformation_t *,
    int *);
static int
ngcllLocalMachineInformationManagerFinalize(
    ngclContext_t *,
    ngcliLocalMachineInformationManager_t *,
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
    ngcliLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcliLocalMachineInformationCacheRegister";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (lmInfo == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
	return 0;
    }

    /* Note : No Replace for Local Machine Information. */

    /* Construct */
    lmInfoMng = ngcllLocalMachineInformationConstruct(context, lmInfo, error);
    if (lmInfoMng == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't construct Local Machine Information.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Lock the list */
    result = ngcliLocalMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock LocalMachineInformation list.\n"); 
	return 0;
    }

    /* Is information registered? */
    if (context->ngc_lmInfo == NULL) {
	/* Not registered */
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngcliLocalMachineInformationListWriteUnlock(context,
	    context->ngc_log, NULL);
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister DefaultRemoteMachineInformation list.\n"); 
	return 0;
    }

    /* Delete */
    result = ngcllLocalMachineInformationDestruct(
	context, context->ngc_lmInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct LocalMachineInformation.\n"); 
	return 0;
    }

    /* Unlock the list */
    result = ngcliLocalMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock LocalMachineInformation list.\n"); 
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
ngcliLocalMachineInformationManager_t *
ngcliLocalMachineInformationCacheGet(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliLocalMachineInformationCacheGet";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    if (context->ngc_lmInfo == NULL) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't get LocalMachineInformation list.\n"); 
        return NULL;
    }
    return context->ngc_lmInfo;
}

/**
 * Construct.
 */
static ngcliLocalMachineInformationManager_t *
ngcllLocalMachineInformationConstruct(
    ngclContext_t *context,
    ngclLocalMachineInformation_t *lmInfo,
    int *error)
{
    int result;
    ngcliLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcllLocalMachineInformationConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfo != NULL);

    /* Allocate */
    lmInfoMng = NGI_ALLOCATE(ngcliLocalMachineInformationManager_t,
        context->ngc_log, error);
    if (lmInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Local Machine Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllLocalMachineInformationManagerInitialize(
        context, lmInfoMng, lmInfo, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Local Machine Information.\n"); 
	goto error;
    }

    /* Register */
    result = ngcliContextRegisterLocalMachineInformation(
    	context, lmInfoMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Local Machine Information for Ninf-G Context.\n"); 
	goto error;
    }

    /* Success */
    return lmInfoMng;

    /* Error occurred */
error:
    result = ngcllLocalMachineInformationDestruct(context, lmInfoMng, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the storage for Local Machine Information Manager.\n"); 
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
    ngcliLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllLocalMachineInformationDestruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(lmInfoMng != NULL);

    /* Unregister */
    result = ngcliContextUnregisterLocalMachineInformation(context, lmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Local Machine Information.\n"); 
	return 0;
    }

    /* Finalize */
    result = ngcllLocalMachineInformationManagerFinalize(context, lmInfoMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Local Machine Information.\n"); 
	return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliLocalMachineInformationManager_t,
            lmInfoMng, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Local Machine Information.\n"); 
	return 0;
    }

    /* Success */
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
    ngLog_t *log;
    ngclLocalMachineInformation_t *lmInfo;
    static const char fName[] = "ngcliLocalMachineInformationAllocate";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Allocate new storage */
    lmInfo = ngiCalloc(
        1, sizeof (ngclLocalMachineInformation_t), log, error);
    if (lmInfo == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage "
            "for LocalMachineInformationManager.\n"); 
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
    ngLog_t *log;
    static const char fName[] = "ngcliLocalMachineInformationFree";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngiFree(lmInfo, log, error);

    return 1;
}

/**
 * Initialize.
 */
static int
ngcllLocalMachineInformationManagerInitialize(
     ngclContext_t *context,
     ngcliLocalMachineInformationManager_t *lmInfoMng,
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Local Machine Information.\n"); 
        return 0;
    }

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(&lmInfoMng->nglmim_rwlOwn,
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
ngcllLocalMachineInformationManagerFinalize(
    ngclContext_t *context,
    ngcliLocalMachineInformationManager_t *lmInfoMng,
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
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destroy Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Release the information */
    result = ngclLocalMachineInformationRelease(context,
	&lmInfoMng->nglmim_info, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Information.\n"); 
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
    ngLog_t *log;
    int *signalTable, *newSignalTable, i, size;
    static const char fName[] = "ngcliLocalMachineInformationCopy";

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
    ngcllLocalMachineInformationInitializeMember(dest);

    /* Copy the members */
    *dest = *src;

    /* Clear pointers for to error-release work fine */
    ngcllLocalMachineInformationInitializePointer(dest);

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        if ((src)->member != NULL) {\
            assert((src)->member != NULL); \
            (dest)->member = ngiStrdup((src)->member, log, error); \
            if ((dest)->member == NULL) { \
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName, \
                    "Can't allocate the storage " \
                    "for Local Machine Information.\n"); \
                    goto error; \
            } \
        } \
    } while(0)
    
    NGL_COPY_STRING(src, dest, nglmi_hostName); 
    NGL_COPY_STRING(src, dest, nglmi_tmpDir);
    NGL_COPY_STRING(src, dest, nglmi_invokeServerLog);
    NGL_COPY_STRING(src, dest, nglmi_commProxyLog);
    NGL_COPY_STRING(src, dest, nglmi_infoServiceLog);
#undef NGL_COPY_STRING

    /* Copy the Log Information */
    result = ngiLogInformationCopy(
        &src->nglmi_logInfo, &dest->nglmi_logInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Log Information.\n"); 
        goto error;
    }

    result = ngiLogLevelInformationCopy(
        &src->nglmi_logLevels, &dest->nglmi_logLevels, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't copy the Log Level.\n"); 
        goto error;
    }

    size = 0;
    signalTable = src->nglmi_signals;
    newSignalTable = NULL;
    if (signalTable != NULL) {
        for (; signalTable[size] != 0; size++);

        newSignalTable = ngiCalloc(sizeof(int), size + 1, log, error);
        if (newSignalTable == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the storage of signal table.\n"); 
            goto error;
        }

        for (i = 0; i < (size + 1); i++) {
            newSignalTable[i] = signalTable[i];
        }
    }
    dest->nglmi_signals = newSignalTable;

    return 1;

    /* Error occurred */
error:

    /* Release */
    result = ngclLocalMachineInformationRelease(context, dest, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Local Machine Information.\n"); 
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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
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
    ngLog_t *log;
    int ret = 1;
    static const char fName[] = "ngcllLocalMachineInformationRelease";

    /* Check the arguments */
    assert(context != NULL);

    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Deallocate the members */
    if (lmInfo->nglmi_hostName != NULL)
        ngiFree(lmInfo->nglmi_hostName, log, error);
    if (lmInfo->nglmi_tmpDir != NULL)
	ngiFree(lmInfo->nglmi_tmpDir, log, error);
    if (lmInfo->nglmi_invokeServerLog != NULL)
	ngiFree(lmInfo->nglmi_invokeServerLog, log, error);
    if (lmInfo->nglmi_commProxyLog != NULL)
	ngiFree(lmInfo->nglmi_commProxyLog, log, error);
    if (lmInfo->nglmi_infoServiceLog != NULL)
	ngiFree(lmInfo->nglmi_infoServiceLog, log, error);
    if (lmInfo->nglmi_signals != NULL)
	ngiFree(lmInfo->nglmi_signals, log, error);

    /* Release Log Information */
    result = ngiLogInformationFinalize(&lmInfo->nglmi_logInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release Log Information.\n"); 
        error = NULL;
        ret = 0;
    }

    result = ngiLogLevelInformationFinalize(&lmInfo->nglmi_logLevels, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release Log Levels Information.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Initialize the members */
    ngcllLocalMachineInformationInitializeMember(lmInfo);

    /* Success */
    return ret;
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
    ngLog_t *log;
    static const char fName[] = "ngcliLocalMachineInformationInitialize";

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }
    log = context->ngc_log;

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllLocalMachineInformationInitializeMember(lmInfo);

    result = ngiLogInformationInitialize(&lmInfo->nglmi_logInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the log information.\n"); 
        return 0;
    }
    result = ngiLogLevelInformationInitialize(&lmInfo->nglmi_logLevels, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the log levels.\n"); 
        return 0;
    }

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

    ngiLogInformationInitializeMember(&lmInfo->nglmi_logInfo);
    ngiLogLevelInformationInitializeMember(&lmInfo->nglmi_logLevels);

    lmInfo->nglmi_saveNsessions = 0;
    lmInfo->nglmi_refreshInterval = 0;
    lmInfo->nglmi_fortranCompatible = 0;
    lmInfo->nglmi_listenPort = 0;
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
    lmInfo->nglmi_tmpDir = NULL;
    lmInfo->nglmi_invokeServerLog = NULL;
    lmInfo->nglmi_commProxyLog = NULL;
    lmInfo->nglmi_infoServiceLog = NULL;
    lmInfo->nglmi_signals = NULL;
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
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
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
    ngcliLocalMachineInformationManager_t *lmInfoMng;
    static const char fName[] = "ngcllLocalMachineInformationGetCopy";

    /* Check the arguments */
    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Lock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Local Machine Information.\n"); 
        return 0;
    }

    /* Get the Local Machine Information */
    lmInfoMng = ngcliLocalMachineInformationCacheGet(context, error);
    if (lmInfoMng == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Local Machine Information.\n"); 
        goto error;
    }

    /* Copy the Local Machine Information */
    result = ngcliLocalMachineInformationCopy(context,
                    &lmInfoMng->nglmim_info, lmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy the Local Machine Information.\n"); 
        goto error;
    }

    /* Unlock the Local Machine Information */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Local Machine Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngcliLocalMachineInformationListReadUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Local Machine Information.\n"); 
        return 0;
    }

    /* Failed */
    return 0;
}

