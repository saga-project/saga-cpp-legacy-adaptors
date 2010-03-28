#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexGASScopy.c,v $ $Revision: 1.18 $ $Date: 2005/06/16 08:04:18 $";
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ngEx.h"

/*
 * Data structure
 */
typedef struct ngexlGASScopyProcessInfo_s {
    char *nggcpi_srcURL;
    char *nggcpi_destURL;
} ngexlGASScopyProcessInfo_t;

/*
 * prototype declaration for internal functions.
 */
static void ngexlGASScopyInitializeMember(ngexiGASScopyManager_t *);
static void ngexlGASScopyInitializeBeforeTransfer(ngexiGASScopyManager_t *);
static void ngexlGASScopyInitializePointer(ngexiGASScopyManager_t *);
static char *ngexlGASScopyCreateURL(
    ngexiGASScopyManager_t *, char *, char *, ngLog_t *, int *);
static int ngexlGASScopyDestroyURL(
    ngexiGASScopyManager_t *, char *, ngLog_t *, int *);
static int ngexlGASScopyFile(
    ngexiGASScopyManager_t *, char *, char *,
    ngiConnectRetryInformation_t, ngiRandomNumber_t *, ngLog_t *, int *);
static int ngexlGASScopyFileTry(
    ngexiGASScopyManager_t *, char *, char *, ngLog_t *, int *);
static int ngexlGASScopyCreateGlobusURL(
    char *, globus_url_t *, globus_gass_transfer_requestattr_t *,
    globus_gass_copy_attr_t *, ngLog_t *log, int *error);
static int ngexlGASScopyDestroyGlobusURL(
    ngexiGASScopyManager_t *, globus_url_t *, ngLog_t *, int *);
static void ngexlGASScopyCallback(
    void *, globus_gass_copy_handle_t *, globus_object_t *);

static int ngexlGASScopyFileTryIsCopyProcessRequired(
    ngexiGASScopyManager_t *, int *, ngLog_t *, int *);
static int ngexlGASScopyFileTryByProcess(
    ngexiGASScopyManager_t *, char *, char *, ngLog_t *, int *);
static int ngexlGASScopyFileTryByProcessParent(
    ngexiGASScopyManager_t *, char *, char *, ngLog_t *, int *);
static void ngexlGASScopyFileTryByProcessChild(
    char *, int *, int *, char *, char *, char *);
static int ngexlGASScopyFileTryByProcessChildOutput(
    int, int, ngLog_t *, int *);
static void ngexlGASScopyProcessInfoInitializeMember(
    ngexlGASScopyProcessInfo_t *);
static int ngexlGASScopyProcessAnalyzeArgument(
    ngexiContext_t *, ngexlGASScopyProcessInfo_t *, int , char *[], int *);
static int ngexlGASScopyProcessAnalyzeArgumentLogLevel(
    ngexiContext_t *, char *, int *);

/**
 * Construct
 */
ngexiGASScopyManager_t *
ngexiGASScopyConstruct(ngLog_t *log, int *error)
{
    int result;
    ngexiGASScopyManager_t *gcm;
    static const char fName[] = "ngexiGASScopyConstruct";

    /* Allocate */
    gcm = ngexiGASScopyAllocate(log, error);
    if (gcm == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for GASS Copy Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngexiGASScopyInitialize(gcm, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the GASS Copy Manager.\n", fName);
	goto error;
    }

    /* Success */
    return gcm;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngexiGASScopyFree(gcm, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for GASS Copy Manager.\n", fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Destruct
 */
int
ngexiGASScopyDestruct(ngexiGASScopyManager_t *gcm, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngexiGASScopyDestruct";

    /* Finalize */
    result = ngexiGASScopyFinalize(gcm, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the GASS Copy Manager.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngexiGASScopyFree(gcm, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for GASS Copy Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngexiGASScopyManager_t *
ngexiGASScopyAllocate(ngLog_t *log, int *error)
{
    ngexiGASScopyManager_t *gcm;
    static const char fName[] = "ngexiGASScopyAllocate";

    /* Allocate */
    gcm = globus_libc_calloc(1, sizeof (ngexiGASScopyManager_t));
    if (gcm == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for GASS Copy Manager.\n", fName);
	return NULL;
    }

    /* Success */
    return gcm;
}

/**
 * Deallocate
 */
int
ngexiGASScopyFree(ngexiGASScopyManager_t *gcm, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(gcm != NULL);

    /* Deallocate */
    globus_libc_free(gcm);

    /* Success */
    return 1;
}

/*
 * Initialize
 */
int
ngexiGASScopyInitialize(ngexiGASScopyManager_t *gcm, ngLog_t *log, int *error)
{
    int result;
    globus_result_t gResult;
    int mutexInit = 0, condInit = 0, attrInit = 0, handleInit = 0;
    static const char fName[] = "ngexiGASScopyInitialize";

    /* Initialize the members */
    ngexlGASScopyInitializeMember(gcm);

#ifdef NGEXI_AVOID_GASS_API_RETRY_SEGV_BUG
    /* Require copy by process. On Copy Process, set this back to 0 later. */
    gcm->nggcm_gassCopyByProcess = 1;
#endif /* NGEXI_AVOID_GASS_API_RETRY_SEGV_BUG */

    /* Initialize the mutex */
    result = ngiMutexInitialize(&gcm->nggcm_mutex, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex.\n", fName);
	goto error;
    }
    mutexInit = 1;

    /* Initialize the condition variable */
    result = ngiCondInitialize(&gcm->nggcm_cond, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Condition variable.\n", fName);
	goto error;
    }
    condInit = 1;

    /* Initialize the Copy Handle Attribute */
    gResult = globus_gass_copy_handleattr_init(&gcm->nggcm_copy_ha);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_handleattr_init() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the GASS Copy Handle Attribute.\n",
	    fName);
	goto error;
    }
    attrInit = 1;

    /* Initialize the Copy Handle */
    gResult = globus_gass_copy_handle_init(
	&gcm->nggcm_copy_h, &gcm->nggcm_copy_ha);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_handle_init() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the GASS Copy Handle.\n", fName);
	goto error;
    }
    handleInit = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Destroy the Copy Handle */
    if (handleInit != 0) {
	gResult = globus_gass_copy_handle_destroy(&gcm->nggcm_copy_h);
	if (gResult != GLOBUS_SUCCESS) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: globus_gass_copy_handle_destroy() failed.\n", fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	fName, gResult, NULL);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't destroy the GASS Copy Handle.\n", fName);
	}
    }
    handleInit = 0;

    /* Destroy the Copy Handle Attribute */
    if (attrInit != 0) {
	gResult = globus_gass_copy_handleattr_destroy(&gcm->nggcm_copy_ha);
	if (gResult != GLOBUS_SUCCESS) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: globus_gass_copy_handleattr_destroy() failed.\n", fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	fName, gResult, NULL);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't destroy the GASS Copy Handle Attribute.\n",
	    	fName);
	}
    }
    attrInit = 0;

    /* Destroy the condition variable */
    if (condInit != 0) {
	result = ngiCondDestroy(&gcm->nggcm_cond, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't destroy the Condition variable.\n", fName);
	}
    }
    condInit = 0;

    /* Destroy the mutex */
    if (mutexInit != 0) {
	result = ngiMutexDestroy(&gcm->nggcm_mutex, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destroy the Mutex.\n", fName);
	}
    }
    mutexInit = 0;

    /* Failed */
    return 0;
}

/*
 * Finalize
 */
int
ngexiGASScopyFinalize(ngexiGASScopyManager_t *gcm, ngLog_t *log, int *error)
{
    int result;
    globus_result_t gResult;
    static const char fName[] = "ngexiGASScopyFinalize";

    /* Destroy the Copy Handle */
    gResult = globus_gass_copy_handle_destroy(&gcm->nggcm_copy_h);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_handle_destroy() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the GASS Copy Handle.\n", fName);
	return 0;
    }

    /* Destroy the Copy Handle Attribute */
    gResult = globus_gass_copy_handleattr_destroy(&gcm->nggcm_copy_ha);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_handleattr_destroy() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            fName, gResult, NULL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destroy the GASS Copy Handle Attribute.\n",
            fName);
	return 0;
    }

    /* Destroy the condition variable */
    result = ngiCondDestroy(&gcm->nggcm_cond, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destroy the Condition variable.\n", fName);
	return 0;
    }

    /* Destroy the mutex */
    result = ngiMutexDestroy(&gcm->nggcm_mutex, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the Mutex.\n", fName);
	return 0;
    }

    /* Initialize the members */
    ngexlGASScopyInitializeMember(gcm);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
ngexlGASScopyInitializeMember(ngexiGASScopyManager_t *gcm)
{
    /* Initialize the pointers */
    ngexlGASScopyInitializePointer(gcm);

    gcm->nggcm_gassCopyByProcess = 0;
    gcm->nggcm_failedCount = 0;

    /* Initialize the members */
    ngexlGASScopyInitializeBeforeTransfer(gcm);
}

/**
 * Initialize before transfer.
 */
static void
ngexlGASScopyInitializeBeforeTransfer(ngexiGASScopyManager_t *gcm)
{
    gcm->nggcm_done = 0;
    gcm->nggcm_result = 0;
    gcm->nggcm_error = NG_ERROR_NO_ERROR;
}

/**
 * Initialize the pointers.
 */
static void
ngexlGASScopyInitializePointer(ngexiGASScopyManager_t *gcm)
{
    /* Initialize the pointers */
    /* Do nothing */
}

/**
 * Transfer the file.
 */
int
ngexiGASScopyFile(
    ngexiGASScopyManager_t *gcm,
    char *url_prefix,
    char *remote_file,
    char *local_file,
    ngexiGASScopyDirection_t direction,
    ngiConnectRetryInformation_t retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    int result, success;
    char *remote_url;
    char *local_url;
    static const char fName[] = "ngexiGASScopyFile";

    /* Check the arguments */
    assert(gcm != NULL);
    assert(url_prefix != NULL);
    assert(remote_file != NULL);
    assert(local_file != NULL);
    assert(randomSeed != NULL);

    /* log */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: new GASS file transfer requested.\n", fName);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: url_prefix  = \"%s\"\n", fName, url_prefix);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: remote_file = \"%s\"\n", fName, remote_file);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: local_file  = \"%s\"\n", fName, local_file);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: direction   = %d (== \"%s\")\n", fName, direction,
        ((direction == NGEXI_GASS_COPY_REMOTE_TO_LOCAL) ?  "REMOTE_TO_LOCAL"
        : ((direction == NGEXI_GASS_COPY_LOCAL_TO_REMOTE) ?  "LOCAL_TO_REMOTE"
        : "unknown")));

    /* Create the URL of remote file */
    remote_url = ngexlGASScopyCreateURL(
    	gcm, url_prefix, remote_file, log, error);
    if (remote_url == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't create the URL.\n", fName);
        return 0;
    }

    /* Create the URL of local file */
    local_url = ngexlGASScopyCreateURL(
    	gcm, NGEXI_LOCAL_FILE_PREFIX, local_file, log, error);
    if (local_url == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't create the URL.\n", fName);
        return 0;
    }

    if (direction == NGEXI_GASS_COPY_REMOTE_TO_LOCAL)
   	success = ngexlGASScopyFile(
            gcm, remote_url, local_url, retryInfo, randomSeed, log, error);
    else
	success = ngexlGASScopyFile(
            gcm, local_url, remote_url, retryInfo, randomSeed, log, error);

    /* Destroy the URL of remote file */
    result = ngexlGASScopyDestroyURL(gcm, remote_url, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy the URL.\n", fName);
	return 0;
    }
    remote_url = NULL;

    /* Destroy the URL of local file */
    result = ngexlGASScopyDestroyURL(gcm, local_url, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destroy the URL.\n", fName);
	return 0;
    }
    local_url = NULL;

    return success;
}

/**
 * Create the URL.
 */
static char *
ngexlGASScopyCreateURL(
    ngexiGASScopyManager_t *gcm,
    char *url_prefix,
    char *file,
    ngLog_t *log,
    int *error)
{
    char *url;
    char *currentDir = "./";
    static const char fName[] = "ngexlGASScopyCreateURL";

    /* Check the arguments */
    assert(gcm != NULL);
    assert(url_prefix != NULL);
    assert(file != NULL);

    /* Make URL of remote file */
    url = globus_libc_malloc(
        strlen(url_prefix) + strlen(currentDir) + strlen(file) + 1);
    if (url == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for URL.\n", fName);
        return NULL;
    }
    strcpy(url, url_prefix);
    if (file[0] != '/') {
        strcat(url, currentDir);
    }
    strcat(url, file);

    /* Success */
    return url;
}

/**
 * Destroy the URL.
 */
static int
ngexlGASScopyDestroyURL(
    ngexiGASScopyManager_t *gcm,
    char *url,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(gcm != NULL);
    assert(url != NULL);

    /* Deallocate */
    globus_libc_free(url);

    /* Success */
    return 1;
}

/*
 * Transfer the file.
 */
static int
ngexlGASScopyFile(
    ngexiGASScopyManager_t *gcm,
    char *srcURL,
    char *destURL,
    ngiConnectRetryInformation_t retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    struct timeval retryTimeval;
    int result, localError, *lerror;
    ngiConnectRetryStatus_t retryStatus;
    int copySuccess, onceFailed, retryRequired, doRetry;
    static const char fName[] = "ngexlGASScopyFile";

    /* Check the arguments */
    assert(gcm != NULL);
    assert(srcURL != NULL);
    assert(destURL != NULL);
    assert(randomSeed != NULL);

    lerror = &localError;
    NGI_SET_ERROR(lerror, NG_ERROR_NO_ERROR);

    copySuccess = 0;
    onceFailed = 0;
    retryRequired = 0;
    doRetry = 0;

    /* Initialize the Retry Status */
    result = ngiConnectRetryInitialize(
        &retryStatus, &retryInfo, randomSeed, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Retry status initialize failed.\n", fName);
        return 0;
    }

    do {
        copySuccess = 0;
        retryRequired = 0;
        doRetry = 0;
        NGI_SET_ERROR(lerror, NG_ERROR_NO_ERROR);

        /* Copy */
        result = ngexlGASScopyFileTry(gcm, srcURL, destURL, log, lerror);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: GASS copy file failed.\n", fName);

            gcm->nggcm_failedCount++;
            copySuccess = 0;
            onceFailed = 1;
            retryRequired = 1;
        } else {
            copySuccess = 1;
            retryRequired = 0;
        }

        if (retryRequired != 0) {
            /* Get Next Retry */
            result = ngiConnectRetryGetNextRetrySleepTime(
                &retryStatus, &doRetry, &retryTimeval, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Getting next retry time failed.\n", fName);
                return 0;
            }
        }

        if (doRetry != 0) {
            /* Sleep before retry */
            result = ngiSleepTimeval(&retryTimeval, 0, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: sleep failed.\n", fName);
                return 0;
            }

            /* to tell loglevel == Error user */
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Retrying GASS copy file.\n", fName);
        }
    } while (doRetry != 0);

    if (copySuccess == 0) {
        NGI_SET_ERROR(error, *lerror);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: GASS copy file was finally failed.\n", fName);
        return 0;
    }

    if (onceFailed != 0) {
        /* to tell loglevel == Error user */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Retrying GASS copy file was finally successful.\n", fName);
    }

    /* Finalize the Retry Status */
    result = ngiConnectRetryFinalize(&retryStatus, log, error);
    if (result == 0) { 
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Retry status finalize failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/*
 * Try to transfer the file once.
 */
static int
ngexlGASScopyFileTry(
    ngexiGASScopyManager_t *gcm,
    char *srcURL,
    char *destURL,
    ngLog_t *log,
    int *error)
{
    int result, isProcess;
    globus_result_t gResult;
    int srcGlobusUrlAllocated, destGlobusUrlAllocated, mutexLocked;
    static const char fName[] = "ngexlGASScopyFileTry";

    /* Check the arguments */
    assert(gcm != NULL);
    assert(srcURL != NULL);
    assert(destURL != NULL);

    /* Initialize the members */
    ngexlGASScopyInitializeBeforeTransfer(gcm);

    srcGlobusUrlAllocated = 0;
    destGlobusUrlAllocated = 0;
    mutexLocked = 0;
    isProcess = 0;

    /* Avoid GASS API Bug */
    result = ngexlGASScopyFileTryIsCopyProcessRequired(
        gcm, &isProcess, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get whether the GASS copy API or process.\n", fName);
        goto error;
    }

    if (isProcess != 0) {
         /* GASS copy by Process */
         result = ngexlGASScopyFileTryByProcess(
             gcm, srcURL, destURL, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: GASS copy file by process failed.\n", fName);
            goto error;
        }

        /* Success */
        return 1;
    }

    /* log */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: src url  = \"%s\"\n", fName, srcURL);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: dst url  = \"%s\"\n", fName, destURL);

    result = ngexlGASScopyCreateGlobusURL(
	srcURL, &gcm->nggcm_src_url, &gcm->nggcm_src_trans,
	&gcm->nggcm_src_copy, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't create the URL.\n", fName);
	goto error;
    }
    srcGlobusUrlAllocated = 1;

    result = ngexlGASScopyCreateGlobusURL(
	destURL, &gcm->nggcm_dest_url, &gcm->nggcm_dest_trans,
	&gcm->nggcm_dest_copy, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't create the URL.\n", fName);
	goto error;
    }
    destGlobusUrlAllocated = 1;

    gResult = globus_gass_copy_register_url_to_url(
	&gcm->nggcm_copy_h, srcURL, &gcm->nggcm_src_copy,
       	destURL, &gcm->nggcm_dest_copy, ngexlGASScopyCallback, gcm);
    if (gResult != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_register_url_to_url() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't register the callback for GASS Copy.\n", fName);
	goto error;
    }

    /* Lock */
    result = ngiMutexLock(&gcm->nggcm_mutex, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
	goto error;
    }
    mutexLocked = 1;

    while (gcm->nggcm_done == 0) {
    	result = ngiCondWait(&gcm->nggcm_cond, &gcm->nggcm_mutex, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't wait the Cond.\n", fName);
	    goto error;
	}
    }

    /* Unlock */
    result = ngiMutexUnlock(&gcm->nggcm_mutex, log, error);
    mutexLocked = 0;
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
	goto error;
    }

    /* Destroy the URL */
    result = ngexlGASScopyDestroyGlobusURL(
    	gcm, &gcm->nggcm_src_url, log, error);
    srcGlobusUrlAllocated = 0;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the Globus URL.\n", fName);
	goto error;
    }

    /* Destroy the URL */
    result = ngexlGASScopyDestroyGlobusURL(
    	gcm, &gcm->nggcm_dest_url, log, error);
    destGlobusUrlAllocated = 0;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the Globus URL.\n", fName);
	goto error;
    }

    /* Is error occurred? */
    if (gcm->nggcm_result == 0) {
    	NGI_SET_ERROR(error, gcm->nggcm_error);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't transfer the File.\n", fName);
	goto error;
    }
    	
    /* log */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: GASS copy try was successfully done.\n", fName);

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        result = ngiMutexUnlock(&gcm->nggcm_mutex, log, NULL);
        mutexLocked = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
        }
    }

    if (srcGlobusUrlAllocated != 0) {
        /* Destroy the URL */
        srcGlobusUrlAllocated = 0;
        result = ngexlGASScopyDestroyGlobusURL(
            gcm, &gcm->nggcm_src_url, log, NULL);
        srcGlobusUrlAllocated = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Globus URL.\n", fName);
        }
    }

    if (destGlobusUrlAllocated != 0) {
        /* Destroy the URL */
        result = ngexlGASScopyDestroyGlobusURL(
                gcm, &gcm->nggcm_dest_url, log, NULL);
        destGlobusUrlAllocated = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destroy the Globus URL.\n", fName);
        }
    }

    /* log */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: GASS copy try was failed.\n", fName);

    /* Failed */
    return 0;
}

/*
 * Create the URL.
 */
static int
ngexlGASScopyCreateGlobusURL(
    char *url,
    globus_url_t *url_t,
    globus_gass_transfer_requestattr_t *trans,
    globus_gass_copy_attr_t *attr,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    globus_gass_copy_url_mode_t url_m;
    static const char fName[] = "ngexlGASScopyCreateGlobusURL";

    gResult = globus_gass_copy_attr_init(attr);
    if ( gResult != GLOBUS_SUCCESS ) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_attr_init() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the attribute for GASS Copy.\n", fName);
        return 0;
    }

    result = globus_url_parse(url, url_t);
    if (result != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: globus_url_parse failed: %d.\n", fName, result);
	return 0;
    }

    gResult = globus_gass_copy_get_url_mode(url, &url_m);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gass_copy_get_url_mode() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    fName, gResult, NULL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the URL mode.\n", fName);
        return 0;
    }

    if (url_m == GLOBUS_GASS_COPY_URL_MODE_GASS) {
        result = globus_gass_transfer_requestattr_init(trans, url_t->scheme);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
		NULL, "%s: globus_gass_transfer_requestattr_init failed: %d.\n",
		fName, result);
	    return 0;
	}

        result = globus_gass_transfer_requestattr_set_file_mode(
            trans, GLOBUS_GASS_TRANSFER_FILE_MODE_BINARY);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: globus_gass_transfer_requestattr_set_file_mode failed: %d.\n",
		fName, result);
	    return 0;
	}

        gResult = globus_gass_copy_attr_set_gass(attr, trans);
	if (gResult != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: globus_gass_copy_attr_set_gass() failed.\n", fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		fName, gResult, NULL);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't register the callback for GASS Copy.\n",
		fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Destroy the URL.
 */
static int
ngexlGASScopyDestroyGlobusURL(
    ngexiGASScopyManager_t *gcm,
    globus_url_t *url,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlGASScopyDestroyGlobusURL";

    /* Destroy the URL */
    result = globus_url_destroy(url);
    if (result != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destroy the Globus URL.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/*
 * Callback function
 */
static void
ngexlGASScopyCallback(
    void *arg,
    globus_gass_copy_handle_t *handle,
    globus_object_t *errorObject)
{
    ngLog_t *log;
    ngexiContext_t *context;
    ngexiGASScopyManager_t *gcm;
    int result, error, mutexLocked;
    static const char fName[] = "ngexlGASScopyCallback";

    mutexLocked = 0;
    log = NULL;

    context = ngexiContextGet(NULL, NULL);
    if (context == NULL) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Ninf-G Executable context is NULL.\n", fName);
    } else {
        log = context->ngc_log;
    }

    /* Check the arguments */
    if (arg == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Callback argument is NULL.\n", fName);
	return;
    }
    gcm = (ngexiGASScopyManager_t *)arg;

    /* Is error occurred? */
    if (errorObject == GLOBUS_SUCCESS) {
    	gcm->nggcm_result = 1;
	gcm->nggcm_error = NG_ERROR_NO_ERROR;
    } else {
	ngiGlobusErrorByObject(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, fName, errorObject, NULL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't register the callback for GASS Copy.\n", fName);

	gcm->nggcm_result = 0;
	gcm->nggcm_error = NG_ERROR_GLOBUS;
    }

    /* Lock the mutex */
    result = ngiMutexLock(&gcm->nggcm_mutex, log, &error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
	goto error;
    }
    mutexLocked = 1;

    /* Notify done */
    gcm->nggcm_done = 1;

    /* Signal to condition variable */
    result = ngiCondSignal(&gcm->nggcm_cond, log, &error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't signal the Cond.\n", fName);
	goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&gcm->nggcm_mutex, log, &error);
    mutexLocked = 0;
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
	goto error;
    }

    /* Success */
    return;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        /* Unlock the mutex */
        result = ngiMutexUnlock(&gcm->nggcm_mutex, log, NULL);
        mutexLocked = 0;
        if (result == 0) {
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
        }
    }

    /* Failed */
    return;
}

/**
 * Is copy by process required?
 */
static int
ngexlGASScopyFileTryIsCopyProcessRequired(
    ngexiGASScopyManager_t *gcm,
    int *isProcess,
    ngLog_t *log,
    int *error)
{
    int isProcessRequired;

    /* Check the arguments */
    assert(gcm != NULL);
    assert(isProcess != NULL);

    *isProcess = 0;
    isProcessRequired = 0;

    if (gcm->nggcm_gassCopyByProcess != 0) {
        isProcessRequired = 1;
    }

    *isProcess = isProcessRequired;

    /* Success */
    return 1;
}

/**
 * GASS copy by GASS copy process invocation.
 */
static int
ngexlGASScopyFileTryByProcess(
    ngexiGASScopyManager_t *gcm,
    char *srcURL,
    char *destURL,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlGASScopyFileTryByProcess";

    result = ngexlGASScopyFileTryByProcessParent(
        gcm, srcURL, destURL, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: GASS copy file by process failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

static int
ngexlGASScopyFileTryByProcessParent(
    ngexiGASScopyManager_t *gcm,
    char *srcURL,
    char *destURL,
    ngLog_t *log,
    int *error)
{
    ngexiContext_t *context;
    pid_t childPid, returnedPid;
    ngLogInformation_t *logInfo;
    int childStdout[2], childStderr[2];
    char *myName, *logLevelSwitchFormat, *logLevelArg;
    int result, exitStatus, exitCode, logLevelArgLength;
    static const char fName[] = "ngexlGASScopyFileTryByProcessParent";

    exitStatus = 0;
    exitCode = 0;
    logLevelArg = NULL;

    context = ngexiContextGet(NULL, NULL);
    if (context == NULL) {
       NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
       ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Ninf-G Executable context is NULL.\n", fName);
        return 0;
    }

    myName = context->ngc_lmInfo.nglmi_path;

    if (log != NULL) {
        /* Pass loglevel to the GASS copy process */
        logLevelSwitchFormat = "--logLevel=%d,%d,%d,%d,%d";
        logLevelArgLength = strlen(logLevelSwitchFormat)
            + NGI_INT_MAX_DECIMAL_DIGITS * 5 + 1;

        logLevelArg = (char *)globus_libc_calloc(
            logLevelArgLength, sizeof(char));
        if (logLevelArg == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for string.\n", fName);
            return 0;
        }
        logInfo = &log->ngl_info;

        snprintf(logLevelArg, logLevelArgLength, logLevelSwitchFormat,
            logInfo->ngli_level,
            logInfo->ngli_levelGlobusToolkit,
            logInfo->ngli_levelNinfgProtocol,
            logInfo->ngli_levelNinfgInternal,
            logInfo->ngli_levelGrpc);
    }

    /* Create parent and child pipe for child stdout/stderr */
    result = pipe(childStdout);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: creating stdout pipe failed: %s.\n", fName, strerror(errno));
        return 0;
    }
    result = pipe(childStderr);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: creating stderr pipe failed: %s.\n", fName, strerror(errno));
        return 0;
    }

    ngLogPrintf(log,
         NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
         NULL, "%s: Invoking GASS copy process.\n", fName);

    childPid = fork();
    if (childPid < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: fork() failed: %s.\n", fName, strerror(errno));
        return 0;
    }

    if (childPid == 0) {
        /* child */
        ngexlGASScopyFileTryByProcessChild(
            myName, childStdout, childStderr, logLevelArg, srcURL, destURL);

        /* NOT REACHED */
        abort();
    }

    /* parent */

    ngLogPrintf(log,
         NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
         NULL, "%s: GASS copy process pid = %ld.\n",
         fName, (long)childPid);

    /* Close parent pipe for write. */
    result = close(childStdout[1]);
    if (childPid < 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stdout write failed: %s.\n",
            fName, strerror(errno));
    }
    result = close(childStderr[1]);
    if (childPid < 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stderr write failed: %s.\n",
            fName, strerror(errno));
    }

    returnedPid = waitpid(childPid, &exitStatus, 0);
    if (returnedPid < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: waitpid() failed: %ld %s.\n",
            fName, (long)returnedPid, strerror(errno));
    }

    exitCode = exitStatus >> 8;
    ngLogPrintf(log,
         NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
         NULL, "%s: GASS copy process returned by %d (status = 0x%x).\n",
         fName, exitCode, exitStatus);

    /* Get child output */
    result = ngexlGASScopyFileTryByProcessChildOutput(
        childStdout[0], childStderr[0], log, error);
    if (result != 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Output the child output failed.\n", fName);
    }

    /* Close parent pipe for read. */
    result = close(childStdout[0]);
    if (childPid < 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stdout read failed: %s.\n",
            fName, strerror(errno));
    }
    result = close(childStderr[0]);
    if (childPid < 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: closing pipe for stderr read failed: %s.\n",
            fName, strerror(errno));
    }

    if (logLevelArg != NULL) {
        globus_libc_free(logLevelArg);
    }

    if (exitCode != 0) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log,
             NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
             NULL, "%s: GASS copy process returned by abnormal exit (%d).\n",
             fName, exitCode);
        return 0;
    }

    /* Success */
    return 1;
}

static void
ngexlGASScopyFileTryByProcessChild(
    char *myName,
    int *stdoutPipe,
    int *stderrPipe,
    char *logLevelArg,
    char *srcURL,
    char *destURL)
{
    char *gassCopyArg;
    int result;

    gassCopyArg = "--gassCopy=1";

    /* Do not touch any Ninf-G or Globus Toolkit data */

    /* Close child pipe for read. */
    result = close(stdoutPipe[0]);
    if (result != 0) {
        exit(1);
    }
    result = close(stderrPipe[0]);
    if (result != 0) {
        exit(1);
    }

    /* Connect pipe to stdout/stderr */
    result = dup2(stdoutPipe[1], STDOUT_FILENO);
    if (result < 0) {
        exit(1);
    }
    result = dup2(stderrPipe[1], STDERR_FILENO);
    if (result < 0) {
        exit(1);
    }

    /* Close unnecessary duplicated child pipe for write. */
    result = close(stdoutPipe[1]);
    if (result != 0) {
        exit(1);
    }
    result = close(stderrPipe[1]);
    if (result != 0) {
        exit(1);
    }
    
    /**
     * Note : myName, that is Remote Executable argv[0] should be full-path.
     * Remote Executable should be invoked by full-path if the GASS copy
     * by process functionality is required.
     */

    /* exec */
    if (logLevelArg == NULL) {
        result = execlp(myName, myName, gassCopyArg,
            srcURL, destURL, NULL);
    } else {
        result = execlp(myName, myName, gassCopyArg, logLevelArg,
            srcURL, destURL, NULL);
    }

    /* If the exec() was successful, the process will not return here. */

    /* error */
    fprintf(stderr, "execlp failed: %d %s.\n", result, strerror(errno));

    exit(1);
}

static int
ngexlGASScopyFileTryByProcessChildOutput(
    int stdoutFd,
    int stderrFd,
    ngLog_t *log,
    int *error)
{
    FILE *fp;
    ngLogLevel_t loglevel;
    int i, c, finished, fd, cur;
    char *buf, *oldBuf, *p, *target;
    int bufSize, oldBufSize, newBufSize, initialBufSize;
    static const char fName[] = "ngexlGASScopyFileTryByProcessChildOutput";

    buf = NULL;
    bufSize = 0;
    initialBufSize = 512;
    finished = 0;

    for (i = 0; i < 2; i++) {
        switch (i) {
        case 0:
            fd =  stdoutFd;
            target = "stdout";
            break;
        case 1:
            fd = stderrFd;
            target = "stderr";
            break;
        default:
            finished = 1;
            fd = -1;
            target = "NULL";
            break;
        }

        if (finished != 0) {
            break;
        }

        fp = fdopen(fd, "r");
        if (fp == NULL) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: fdopen() for child %s failed: %s.\n",
                fName, target, strerror(errno));
            continue;
        }

        cur = 0;
        p = buf;

        do {
            c = fgetc(fp);

            if (cur >= bufSize) {
                /* Renew buffer */
                oldBuf = buf;
                oldBufSize = bufSize;

                if (bufSize <= 0) {
                    newBufSize = initialBufSize;
                } else {
                    newBufSize = bufSize * 2;
                }

                buf = globus_libc_calloc(newBufSize, sizeof(char));
                if (buf == NULL) {
                    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                        NULL,
                        "%s: Can't allocate the storage for string.\n",
                        fName, target, strerror(errno));
                    return 0;
                }
                bufSize = newBufSize;
                newBufSize = 0;

                if (oldBuf != NULL) {
                    memcpy(buf, oldBuf, oldBufSize);

                    globus_libc_free(oldBuf);
                    oldBuf = NULL;
                    oldBufSize = 0;
                }

                p = buf + cur;
            }

            *p = ((c != EOF) ? c : '\0');

            cur++;
            p++;

        } while (c != EOF);

        /* Do not fclose(fp), because of the error and freeze. */

        assert(buf != NULL);
        if (strlen(buf) == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: No output from GASS copy process %s.\n", fName, target);
        } else {
            /* Select loglevel */
            loglevel = NG_LOG_LEVEL_DEBUG;
            if (log != NULL) {
                loglevel = log->ngl_info.ngli_levelNinfgInternal;
            }
            /* If the user does not expect to get log, disable output */
            if (loglevel == NG_LOG_LEVEL_OFF) {
                loglevel = NG_LOG_LEVEL_DEBUG;
            }

            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: Output from GASS copy process %s (got %d char).\n",
                fName, target, strlen(buf));
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, loglevel, NULL,
                "%s: Output from GASS copy process %s below.\n"
                "%s", fName, target, buf);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, loglevel, NULL,
                "%s: Output from GASS copy process %s finished.\n",
                fName, target);
        }
    }

    if (buf != NULL) {
        globus_libc_free(buf);
        buf = NULL;
    }
    bufSize = 0;

    /* Success */
    return 0;
}

/**
 * GASS copy process main routine.
 * The process is invoked by --gassCopy argument.
 * Thus, this process works like a globus-url-copy command.
 *
 * This function will not return.
 */
int
ngexiGASScopyProcess(
    int argc,
    char *argv[],
    int *error)
{
    ngLog_t *log;
    int doNotDeactivate;
    ngexiContext_t *context;
    ngexiGASScopyManager_t *gcm;
    ngLogInformation_t logInfoTemp;
    ngexlGASScopyProcessInfo_t copyInfo;
    int result, exitCode, globusInitialized, errorEntity, *argError;
    static const char fName[] = "ngexiGASScopyProcess";

    log = NULL;
    gcm = NULL;
    exitCode = 0;
    globusInitialized = 0;
    argError = error;
    NGI_SET_ERROR(argError, NG_ERROR_NO_ERROR);

    doNotDeactivate = 0;
#ifdef NGEXI_AVOID_GASS_API_AVOID_DEACTIVATE_FREEZE
    doNotDeactivate = 1;
#endif /* NGEXI_AVOID_GASS_API_AVOID_DEACTIVATE_FREEZE */

    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Initialize Temporary Log Information */
    result = ngiLogInformationSetDefault(
        &logInfoTemp, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize Log Information.\n", fName);
        goto error;
    }

    /* Construct the temporary Log */
    log = ngiLogConstruct(
        NG_LOG_TYPE_GENERIC, "Executable", &logInfoTemp,
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, error);
    if (log == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Log Manager.\n", fName);
        goto error;
    }

    /* Finalize Temporary Log Information */
    result = ngLogInformationFinalize(&logInfoTemp);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize Log Information.\n", fName);
        goto error;
    }

    /* Get the context struct */
    context = ngexiContextGet(NULL, NULL);
    if (context == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Ninf-G Executable context is NULL.\n", fName);
        goto error;
    }

    /* Clear the context */
    memset(context, 0, sizeof(ngexiContext_t));

    /* Only the ngc_log are used in context */
    context->ngc_log = log;

    ngexlGASScopyProcessInfoInitializeMember(&copyInfo);

    /* Analyze the arguments */
    result = ngexlGASScopyProcessAnalyzeArgument(
        context, &copyInfo, argc, argv, error);
    if (result == 0) {
        log = context->ngc_log;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't analyze the arguments.\n", fName);
        goto error;
    }

    /* Set to new log */
    log = context->ngc_log;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: GASS copy process started. now on the process.\n", fName);

    /* Initialize the Globus Toolkit */
    result = ngexiGlobusInitialize(log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't initialize the Globus Toolkit.\n", fName);
        goto error;
    }
    globusInitialized = 1;

    /* Construct the GASS Copy Manager */
    gcm = ngexiGASScopyConstruct(log, error);
    if (gcm == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the GASS Copy Manager.\n", fName);
        goto error;
    }

    /* This is GASS copy process, and invoking child is unnecessary */
    gcm->nggcm_gassCopyByProcess = 0;

    /* Copy (try once) */
    result = ngexlGASScopyFileTry(
        gcm, copyInfo.nggcpi_srcURL, copyInfo.nggcpi_destURL, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: GASS copy file failed.\n", fName);

        exitCode = 1;
    }

    /* Destruct the GASS Copy Manager */
    assert(gcm != NULL);
    result = ngexiGASScopyDestruct(gcm, log, error);
    gcm = NULL;
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't destruct the GASS Copy Manager.\n", fName);
        goto error;
    }

    /* Finalize the Globus Toolkit */
    assert(globusInitialized != 0);
    if (doNotDeactivate == 0) {
        result = ngexiGlobusFinalize(log, error);
        globusInitialized = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Globus Toolkit.\n", fName);
            goto error;
        }
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: suppress globus_module_deactivate() to avoid freeze.\n",
            fName);
    }

    ngexlGASScopyProcessInfoInitializeMember(&copyInfo);

    /* Success */
    exit(exitCode);

    /* NOT REACHED */
    return exitCode;

    /* Error occurred */
error:
    /* Destruct the GASS Copy Manager */
    if (gcm != NULL) {
        result = ngexiGASScopyDestruct(gcm, log, NULL);
        gcm = NULL;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the GASS Copy Manager.\n", fName);
        }
    }

    /* Finalize the Globus Toolkit */
    if ((globusInitialized != 0) && (doNotDeactivate == 0)) {
        result = ngexiGlobusFinalize(log, NULL);
        globusInitialized = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Globus Toolkit.\n", fName);
        }
    }

    NGI_SET_ERROR(argError, errorEntity);

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
        NULL, "%s: GASS copy process is exiting by error %d.\n",
        fName, errorEntity);

    /* Failed */
    exit(1);

    /* NOT REACHED */
    return 1;
}

static void
ngexlGASScopyProcessInfoInitializeMember(
    ngexlGASScopyProcessInfo_t *copyInfo)
{
    /* Check the arguments */
    assert(copyInfo != NULL);

    copyInfo->nggcpi_srcURL = NULL;
    copyInfo->nggcpi_destURL = NULL;
}

static int
ngexlGASScopyProcessAnalyzeArgument(
    ngexiContext_t *context,
    ngexlGASScopyProcessInfo_t *copyInfo,
    int argc,
    char *argv[],
    int *error)
{
    ngLog_t *log;
    int i, result;
    char *gassCopyArg, *logLevelArg;
    static const char fName[] = "ngexlGASScopyProcessAnalyzeArgument";

    /* Check the arguments */
    assert(context != NULL);
    assert(copyInfo != NULL);

    log = context->ngc_log;

    /* "--gassCopy", "--gassCopy=1" both ok */
    gassCopyArg = "--gassCopy";
    logLevelArg = "--logLevel=";

    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], gassCopyArg, strlen(gassCopyArg)) == 0) {
            /* Do nothing */
            continue;

        } else if (strncmp(argv[i], logLevelArg, strlen(logLevelArg)) == 0) {
            result = ngexlGASScopyProcessAnalyzeArgumentLogLevel(
                context, &argv[i][0] + strlen(logLevelArg), error);
            log = context->ngc_log;
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: %s: parse argument failed.\n", fName, argv[i]);
            }
                
        } else if (strncmp(argv[i], "--", strlen("--")) == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s: Argument is not valid.\n", fName, argv[i]);
            return 0;

        } else {
            break;
        }
    }

    if ((i + 2) != argc) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: %s: The number of arguments are not valid.\n",fName);
        return 0;
    }

    if ((argv[i] == NULL) || (argv[i + 1] == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: The URL argument is not valid.\n", fName);
        return 0;
    }

    copyInfo->nggcpi_srcURL = argv[i];
    copyInfo->nggcpi_destURL = argv[i + 1];

    /* Success */
    return 1;
}

static int
ngexlGASScopyProcessAnalyzeArgumentLogLevel(
    ngexiContext_t *context,
    char *logLevelString,
    int *error)
{
    char *p, *end;
    ngLogLevel_t tmpLogLevel;
    ngLog_t *log, *oldLog, *newLog;
    ngLogInformation_t logInfo;
    int i, result, nCategories, validArgument, tmpLogLevelNo;
    static const char fName[] = "ngexlGASScopyProcessAnalyzeArgumentLogLevel";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    nCategories = 5;
    p = logLevelString;

    /* Initialize Temporary Log Information */
    result = ngiLogInformationSetDefault(
        &logInfo, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize Log Information.\n", fName);
        return 0;
    }

    for (i = 0; i < nCategories; i++) {
        validArgument = 1;

        tmpLogLevelNo = strtol(p, &end, 0);
        if ((tmpLogLevelNo < 0) || (tmpLogLevelNo > 5)) {
            validArgument = 0;
        }

        if (i < nCategories - 1) {
            if (*end != ',') {
                validArgument = 0;
            }
        } else {
            if (*end != '\0') {
                validArgument = 0;
            }
        }

        tmpLogLevel = (ngLogLevel_t)tmpLogLevelNo;

        switch(i) {
        case 0:
            logInfo.ngli_level = tmpLogLevel;
            break;
        case 1:
            logInfo.ngli_levelGlobusToolkit = tmpLogLevel;
            break;
        case 2:
            logInfo.ngli_levelNinfgProtocol = tmpLogLevel;
            break;
        case 3:
            logInfo.ngli_levelNinfgInternal = tmpLogLevel;
            break;
        case 4:
            logInfo.ngli_levelGrpc = tmpLogLevel;
            break;
        default:
            validArgument = 0;
            break;
        }

        if (validArgument == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The logLevel argument is not valid.\n", fName);
            return 0;
        }

        p = end + 1;
    }

    /* Construct the new Log */
    newLog = ngiLogConstruct(
        NG_LOG_TYPE_GENERIC, "Executable", &logInfo,
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, error);
    if (newLog == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Log Manager.\n", fName);
        return 0;
    }

    /* Replace to new Log */
    oldLog = context->ngc_log;
    context->ngc_log = newLog;
    log = newLog;

    /* Finalize Temporary Log Information */
    result = ngLogInformationFinalize(&logInfo);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize Log Information.\n", fName);
        return 0;
    }
    
    /* Destruct the old Log */
    if (oldLog != NULL) {
        result = ngiLogDestruct(oldLog, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Log.\n", fName);
            return 0;
        }
    }
    
    /* Success */
    return 1;
}

