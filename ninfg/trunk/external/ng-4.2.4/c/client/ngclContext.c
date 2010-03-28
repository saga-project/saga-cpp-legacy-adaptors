#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclContext.c,v $ $Revision: 1.142 $ $Date: 2007/12/26 12:27:17 $";
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
 * Module of Context for Ninf-G Client.
 */

#include <stdlib.h>
#include <string.h>
#include "ng.h"
#include <assert.h>

/* Flags for wait sessions */
typedef enum nglWaitMode_e {
    NGL_WAIT_ALL,
    NGL_WAIT_ANY
} nglWaitMode_t;

/**
 * Prototype declaration of internal functions.
 */
static ngclContext_t *ngcllContextAllocate(int *);
static int ngcllContextFree(ngclContext_t *, int *);
static int ngcllContextInitialize1st(ngclContext_t *, int *);
static int ngcllContextInitialize2nd(ngclContext_t *, char *, int *);
static int ngcllContextFinalize1st(ngclContext_t *, int *);
static int ngcllContextFinalize2nd(ngclContext_t *, int *);
static int ngcllContextInitializeRWlock(ngclContext_t *, int *);
static int ngcllContextFinalizeRWlock(ngclContext_t *, int *);
static int ngcllContextInitializeMutexAndCond(ngclContext_t *, int *);
static int ngcllContextFinalizeMutexAndCond(ngclContext_t *, int *);
static int ngcllContextInitializeGASSserver(ngclContext_t *, int *);
static int ngcllContextFinalizeGASSserver(ngclContext_t *, int *);
static int ngcllContextInitializeCommunication(ngclContext_t *, int *);
static int ngcllContextInitializeProtocol(ngclContext_t *, int *);
static int ngcllContextFinalizeProtocol(ngclContext_t *, int *);
static int ngcllContextRegisterCallback(ngclContext_t *, int *);
static int ngcllContextUnregisterCallback(ngclContext_t *, int *);
static void ngcllContextInitializeMember(ngclContext_t *);
static void ngcllContextInitializePointer(ngclContext_t *);
static int ngcllContextSetGlobusHostName(ngclContext_t *, int *);
static int ngcllContextLogOutput(ngclContext_t *, int *);
static int ngcllContextRegisterGASSserverManager(ngclContext_t *,
    ngclGASSserverManager_t *, int *);
static int ngcllContextUnregisterGASSserverManager(ngclContext_t *,
    ngclGASSserverManager_t *, int *);
static int ngcllContextRegisterUserData(
    ngclContext_t *, void *, void (*)(ngclContext_t *), int *);
static int ngcllContextUnregisterUserData(ngclContext_t *, int *);
static int ngcllContextGetUserData(ngclContext_t *, void **, int *);
static int ngcllContextRegisterLocalMachineInformation(
    ngclContext_t *, ngclLocalMachineInformationManager_t *, int *);
static int ngcllContextUnregisterLocalMachineInformation(
    ngclContext_t *, ngclLocalMachineInformationManager_t *, int *);
static int ngcllContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
static int ngcllContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
static int ngcllContextRegisterRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
static int ngcllContextUnregisterRemoteMachineInformation(
    ngclContext_t *, ngclRemoteMachineInformationManager_t *, int *);
static int ngcllContextRegisterRemoteClassInformation(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);
static int ngcllContextUnregisterRemoteClassInformation(
    ngclContext_t *, ngclRemoteClassInformationManager_t *, int *);

static int ngcllContextCondWaitSession(ngclContext_t *, ngLog_t *, int *);
static int ngcllContextCheckSessionDone(
    ngclContext_t *, ngclSession_t **, ngclSession_t **, nglWaitMode_t,
    ngLog_t *, int *);
static int ngcllContextExecuteSession(
    ngclContext_t *, ngclSession_t *, ngLog_t *, int *);

static int ngcllContextCancelAllJobsByForce(ngclContext_t *, int *);

/**
 * Construct
 */
ngclContext_t *
ngclContextConstruct(char *configFile, int *error)
{
    int result;
    ngclContext_t *context;
    int initialize1st_done, register_done;
    static const char fName[] = "ngclContextConstruct";

    initialize1st_done = 0;
    register_done = 0;

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Allocate */
    context = ngcllContextAllocate(error);
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Ninf-G Context.\n", fName);
	return NULL;
    }

    /* Initialize 1st pass */
    result = ngcllContextInitialize1st(context, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Ninf-G Context.\n", fName);
        goto error;
    }
    initialize1st_done = 1;

    /* Register */
    result = ngcliNinfgManagerRegisterContext(context, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Ninf-G Context.\n", fName);
        goto error;
    }
    register_done = 1;

    /* Initialize 2nd pass */
    result = ngcllContextInitialize2nd(context, configFile, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Ninf-G Context.\n", fName);
        goto error;
    }

    /* Success */
    return context;

    /* Error occurred */
error:
    /* Unregister */
    if (register_done != 0) {
        result = ngcliNinfgManagerUnregisterContext(
            context, context->ngc_log, NULL);
        if (result == 0) {
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unregister the Ninf-G Context.\n", fName);
        }
    }

    /* Finalize 2nd pass */
    if (initialize1st_done != 0) {
        result = ngcllContextFinalize2nd(context, NULL);
        if (result == 0) {
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Ninf-G Context.\n", fName);
        }
    }

    /* Free */
    result = ngcllContextFree(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't deallocate the Ninf-G Context.\n", fName);
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngclContextDestruct(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngclContextDestruct";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Finalize 1st pass */
    result = ngcllContextFinalize1st(context, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Ninf-G Context.\n", fName);
        return 0;
    }

    /* Unregister */
    result = ngcliNinfgManagerUnregisterContext(
    	context, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Ninf-G Context.\n", fName);
        return 0;
    }

    /* Finalize 2nd pass */
    result = ngcllContextFinalize2nd(context, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Ninf-G Context.\n", fName);
        return 0;
    }

    /* Deallocate */
    result = ngcllContextFree(context, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Ninf-G Context.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngclContext_t *
ngcllContextAllocate(int *error)
{
    ngclContext_t *context;
    static const char fName[] = "ngcllContextAllocate";

    /* Allocate */
    context = globus_libc_calloc(1, sizeof (ngclContext_t));
    if (context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Ninf-G Context.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return context;
}

/**
 * Deallocate
 */
static int
ngcllContextFree(ngclContext_t *context, int *error)
{
    static const char fName[] = "ngcllContextFree";

    /* Is context valid? */
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is NULL.\n", fName);
        return 0;
    }

    /* Deallocate */
    globus_libc_free(context);

    /* Success */
    return 1;
}

/**
 * Initialize 1st pass (called before the registration of context)
 */
static int
ngcllContextInitialize1st(ngclContext_t *context, int *error)
{
    int result;
    ngLogInformation_t logInfoTemp;
    static const char fName[] = "ngcllContextInitialize1st";

    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the members */
    ngcllContextInitializeMember(context);

    /* Initialize Temporary Log Information */
    result = ngiLogInformationSetDefault(
        &logInfoTemp, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize Log Information.\n", fName);
        return 0;
    }

    /* Construct the temporary Log */
    context->ngc_log = ngiLogConstruct(
        NG_LOG_TYPE_GENERIC, "Client", &logInfoTemp,
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, error);
    if (context->ngc_log == NULL) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Log Manager.\n", fName);
        return 0;
    }

    /* Finalize Temporary Log Information */
    result = ngLogInformationFinalize(&logInfoTemp);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize Log Information.\n", fName);
        return 0;
    }

    /* Initialize the Ninf-G Client Manager */
    result = ngcliNinfgManagerInitialize(context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't initialize the Ninf-G Client Manager.\n", fName);
	return 0;
    }

    /* Create the Ninf-G Context ID */
    result = ngcliNinfgManagerCreateContextID(context->ngc_log, error);
    if (result < 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't create the Ninf-G Context ID.\n", fName);
	return 0;
    }
    context->ngc_ID = result;

    /* Initialize the Read/Write Lock */
    result = ngcllContextInitializeRWlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Locks.\n", fName);
	goto error;
    }

    /* Initialize the Mutex and Condition Variable */
    result = ngcllContextInitializeMutexAndCond(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex and Condition Variable.\n", fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the Ninf-G Client Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Ninf-G Client Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Initialize 2nd pass (called after the registration of context)
 */
static int
ngcllContextInitialize2nd(ngclContext_t *context, char *configFile, int *error)
{
    int result;
    static const char fName[] = "ngcllContextInitialize2nd";

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Read configuration file */
    result = ngcliConfigFileRead(context, configFile, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Configuration file error.\n", fName);
        goto error;
    }


    /* Destruct Temporary Log Manager */
    result = ngiLogDestruct(context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't destruct the Log Manager.\n", fName);
        goto error;
    }

    /* Construct the Log Manager */
    if (context->ngc_lmInfo != NULL) {
	context->ngc_log = ngiLogConstruct(
	    NG_LOG_TYPE_GENERIC, "Client",
            &context->ngc_lmInfo->nglmim_info.nglmi_logInfo,
            NGI_LOG_EXECUTABLE_ID_NOT_APPEND, error);
	if (context->ngc_log == NULL) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't construct the Log Manager.\n", fName);
	    goto error;
	}
    }

    /* Set GLOBUS_HOSTNAME environment variable */
    if (context->ngc_lmInfo != NULL) {
        result = ngcllContextSetGlobusHostName(context, error);
        if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't set GLOBUS_HOSTNAME environment variable.\n",
		fName);
	    goto error;
        }
    }

    /* log */
    result = ngcllContextLogOutput(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't output the log.\n", fName);
	goto error;
    }

    /* Set log to Signal Manager */
    result = ngcliNinfgManagerSignalManagerLogSet(
        context->ngc_log, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the log to Signal Manager.\n", fName);
	goto error;
    }

    /* Initialize the GASS Server */
    result = ngcllContextInitializeGASSserver(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the GASS Server.\n", fName);
	goto error;
    }

    /* Initialize the Communication */
    result = ngcllContextInitializeCommunication(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Communication.\n", fName);
	goto error;
    }

    /* Initialize the Protocol */
    result = ngcllContextInitializeProtocol(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Protocol.\n", fName);
	goto error;
    }

    /* Register the Callback */
    result = ngcllContextRegisterCallback(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Callback.\n", fName);
	goto error;
    }

    /* Initialize MDS server connection */
    result = ngcliMDSaccessInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the MDS server connection.\n", fName);
        goto error;
    }

    /* Initialize the Observe Thread */
    result = ngcliObserveThreadInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Observe Thread.\n", fName);
        goto error;
    }

    /* Initialize the HeartBeat */
    result = ngcliHeartBeatInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the heartbeat.\n", fName);
        goto error;
    }

    /* Initialize the Session timeout */
    result = ngcliSessionTimeoutInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the session timeout.\n", fName);
        goto error;
    }

    /* Initialize the Refresh credentials */
    result = ngcliRefreshCredentialsInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the refresh credentials.\n", fName);
        goto error;
    }

    /* Initialize the job start timeout */
    result = ngcliJobStartTimeoutInitialize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the job start timeout.\n", fName);
        goto error;
    }

    /* Start the Observe Thread */
    result = ngcliObserveThreadStart(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't start the Observe Thread.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the Ninf-G Client Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Ninf-G Client Manager.\n", fName);
    }

    return 0;
}

/**
 * Finalize 1st pass (called before unregistration of context)
 */
static int
ngcllContextFinalize1st(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextFinalize1st";

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Execute the user defined destructor */
    if (context->ngc_userDestructer != NULL) {
	context->ngc_userDestructer(context);
    } else {
	if (context->ngc_userData != NULL) {
	    globus_libc_free(context->ngc_userData);
	}
    }
    context->ngc_userData = NULL;
    context->ngc_userDestructer = NULL;

    /* log for remaining Executables */
    if ((context->ngc_nExecutables > 0) ||
        (context->ngc_executable_head != NULL)) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Some remaining Executables still exists.\n",
            fName);

        /* Cancel all Pre-WS GRAM jobs */
        ngcllContextCancelAllJobsByForce(context, NULL);

        /* Not return */
    }

    /* Stop the Observe Thread */
    result = ngcliObserveThreadStop(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't stop the Observe Thread.\n", fName);
	return 0;
    }

    /* Finalize the job start timeout */
    result = ngcliJobStartTimeoutFinalize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the job start timeout.\n", fName);
	return 0;
    }

    /* Finalize the RefreshCredentials */
    result = ngcliRefreshCredentialsFinalize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the heartbeat.\n", fName);
	return 0;
    }

    /* Finalize the Session timeout */
    result = ngcliSessionTimeoutFinalize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the session timeout.\n", fName);
	return 0;
    }

    /* Finalize the HeartBeat */
    result = ngcliHeartBeatFinalize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the heartbeat.\n", fName);
	return 0;
    }

    /* Finalize the Observe Thread */
    result = ngcliObserveThreadFinalize(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Observe Thread.\n", fName);
	return 0;
    }

    /* Destruct all Invoke Servers */
    result = ngcliInvokeServerDestruct(context, NULL, 1, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Invoke Server.\n", fName);
	return 0;
    }

    /* Finalize MDS server connection */
    result = ngcliMDSaccessFinalize(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the MDS server connection.\n", fName);
	return 0;
    }

    /* Unregister the Callback */
    result = ngcllContextUnregisterCallback(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Callback.\n", fName);
	return 0;
    }

    /* Finalize the Protocol */
    result = ngcllContextFinalizeProtocol(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Protocol.\n", fName);
	return 0;
    }

    /* Finalize the GASS Server */
    result = ngcllContextFinalizeGASSserver(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the GASS Server.\n", fName);
	return 0;
    }

    /* Unregister the all information of Remote Class */
    result = ngcliRemoteClassInformationCacheUnregister(context, NULL, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the Remote Class Information.\n", fName);
	return 0;
    }

    /* Unregister information of Default Remote Machine */
    result = ngcliDefaultRemoteMachineInformationCacheUnregister(context,
	error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister Default Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Unregister the all information of Remote Machine */
    result = ngcliRemoteMachineInformationCacheUnregister(context, NULL, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the Remote Machine Information.\n", fName);
	return 0;
    }

    /* Unregister the all Information of Invoke Server */
    result = ngcliInvokeServerInformationCacheUnregister(context, NULL, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the Invoke Server Information.\n", fName);
	return 0;
    }

    /* Unregister the all Information of MDS Server */
    result = ngcliMDSserverInformationCacheUnregister(
        context, NULL, NULL, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the MDS Server Information.\n", fName);
	return 0;
    }

    /* Unregister the all Information of Local Machine */
    result = ngcliLocalMachineInformationCacheUnregister(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the Local Machine Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize 2nd pass (called after unregistration of context)
 */
static int
ngcllContextFinalize2nd(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextFinalize2nd";

    /* Check the arguments */
    assert(context != NULL);

    /* Unset the log from Signal Manager */
    result = ngcliNinfgManagerSignalManagerLogSet(
        NULL, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the log for Signal Manager.\n", fName);
	return 0;
    }

    /* Destruct the Log */
    if (context->ngc_log != NULL) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
	    "%s: Client log destruct.\n", fName);

	result = ngiLogDestruct(context->ngc_log, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't destruct the Log.\n", fName);
	    return 0;
	}
    }
    context->ngc_log = NULL;

    /* Finalize the Mutex and Condition Variable */
    result = ngcllContextFinalizeMutexAndCond(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Mutex and Condition Variable.\n", fName);
	return 0;
    }

    /* Finalize the Read/Write Lock */
    result = ngcllContextFinalizeRWlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Finalize Ninf-G Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Ninf-G Client Manager.\n", fName);
	return 0;
    }

    /* Initialize the members */
    ngcllContextInitializeMember(context);

    /* Success */
    return 1;
}

/**
 * Initialize the Read/Write Lock.
 */
static int
ngcllContextInitializeRWlock(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextInitializeRWlock";

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(
	&context->ngc_rwlOwn, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for own instance.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for GASS server Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlGassMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for GASS server Manager.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Local Machine Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlLmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Local Machine Information.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for MDS server Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlMdsInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for MDS server Information.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Invoke Server Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlIsInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Invoke Server Information.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Remote Machine Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlRmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Remote Class Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlRcInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Remote Class Information.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Job Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlJobMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Job Manager.\n", fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Invoke Server Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlInvokeMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for "
            "Invoke Server Manager.\n", fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Executable Handle */
    result = ngiRWlockInitialize(
	&context->ngc_rwlExecutable, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Executable Handle.\n",
	    fName);
	return 0;
    }

    /* Initialize the Read/Write Lock for Session Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlSession, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock for Session Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize the Read/Write Lock.
 */
static int
ngcllContextFinalizeRWlock(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextFinalizeRWlock";

    /* Finalize the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(
	&context->ngc_rwlOwn, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for own instance.\n", fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for GASS Server Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlGassMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for GASS server Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Local Machine Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlLmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Local Machine Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for MDS server Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlMdsInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for MDS server Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Invoke Server Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlIsInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Invoke Server Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Remote Machine Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlRmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Remote Machine Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Remote Class Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlRcInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Remote Class Information.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Job Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlJobMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Job Manager.\n", fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Invoke Server Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlInvokeMng, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for "
            "Invoke Server Manager.\n", fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Executable Handle */
    result = ngiRWlockFinalize(
	&context->ngc_rwlExecutable, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Executable Handle.\n",
	    fName);
	return 0;
    }

    /* Finalize the Read/Write Lock for Session Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlSession, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Read/Write Lock for Session Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the Mutex and Condition Variable.
 */
static int
ngcllContextInitializeMutexAndCond(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextInitializeMutexAndCond";

    /* Initialize the Mutex for Executable */
    result = ngiMutexInitialize(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex for Executable.\n", fName);
	return 0;
    }

    /* Initialize the Condition Variable for Executable */
    result = ngiCondInitialize(
    	&context->ngc_condExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Cond for Executable.\n", fName);
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagExecutable = 0;

    /* Initialize the Mutex for Session */
    result = ngiMutexInitialize(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex for Session.\n", fName);
	return 0;
    }

    /* Initialize the Condition Variable for Session */
    result = ngiCondInitialize(
    	&context->ngc_condSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Cond for Session.\n", fName);
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagSession = 0;

    /* Success */
    return 1;
}

/**
 * Finalize the Mutex and Condition Variable.
 */
static int
ngcllContextFinalizeMutexAndCond(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextFinalizeMutexAndCond";

    /* Finalize the Mutex for Executable */
    result = ngiMutexDestroy(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Mutex for Executable.\n", fName);
	return 0;
    }

    /* Finalize the Condition Variable for Executable */
    result = ngiCondDestroy(
    	&context->ngc_condExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Cond for Executable.\n", fName);
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagExecutable = 0;

    /* Finalize the Mutex for Session */
    result = ngiMutexDestroy(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Mutex for Session.\n", fName);
	return 0;
    }

    /* Finalize the Condition Variable for Session */
    result = ngiCondDestroy(
    	&context->ngc_condSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Cond for Session.\n", fName);
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagSession = 0;

    /* Success */
    return 1;
}

/**
 * Initialize the GASS Server.
 */
static int
ngcllContextInitializeGASSserver(ngclContext_t *context, int *error)
{
    int result;
    ngclGASSserverManager_t *gassHttp, *gassHttps;
    static const char fName[] = "ngcllContextInitializeGASSserver";

    /* Start the GASS server (http) */
    gassHttp = ngcliGASSserverConstruct(
	"http", NGCLI_GASS_OPTION, context->ngc_log, error);
    if (gassHttp == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the GASS Server Manager.\n", fName);
	return 0;
    }

    /* Register the GASS Server Information to Ninf-G Context */
    result = ngcllContextRegisterGASSserverManager(context, gassHttp, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the GASS Server Manager.\n", fName);
	goto error1;
    }

    /* Start the GASS server (https) */
    gassHttps = ngcliGASSserverConstruct(
	"https", NGCLI_GASS_OPTION, context->ngc_log, error);
    if (gassHttps == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the GASS Server Manager.\n", fName);
	goto error2;
    }

    /* Register the GASS Server Information to Ninf-G Context */
    result = ngcllContextRegisterGASSserverManager(context, gassHttps, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the GASS Server Manager.\n", fName);
	goto error3;
    }

    /* Success */
    return 1;

    /* Error occurred */
error3:
    /* Destruct the GASS server (https) */
    result = ngcliGASSserverDestruct(gassHttps, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the GASS Server Manager.\n", fName);
	goto error2;
    }
    gassHttps = NULL;

error2:
    /* Unregister the GASS Server Information to Ninf-G Context */
    result = ngcllContextUnregisterGASSserverManager(context, gassHttp, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the GASS Server Manager.\n", fName);
	goto error1;
    }

error1:
    /* Destruct the GASS server (http) */
    result = ngcliGASSserverDestruct(gassHttp, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the GASS Server Manager.\n", fName);
	goto error2;
    }
    gassHttp = NULL;

    return 0;
}

/**
 * Finalize the GASS Server.
 */
static int
ngcllContextFinalizeGASSserver(ngclContext_t *context, int *error)
{
    int result;
    ngclGASSserverManager_t *gassMng, *next;
    static const char fName[] = "ngcllContextFinalizeGASSserver";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the GASS Server Manager */
    gassMng = ngcliContextGASSserverManagerGetNext(context, NULL, error);
    if (gassMng == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: GASS Server manager is not found.\n", fName);
	return 0;
    }

    for (; gassMng != NULL; gassMng = next) {
	/* Get the GASS Server Manager */
	next = ngcliContextGASSserverManagerGetNext(context, gassMng, error);

	/* Unregister the GASS Server Information to Ninf-G Context */
	result = ngcllContextUnregisterGASSserverManager(
	    context, gassMng, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't register the GASS Server Manager.\n", fName);
	    return 0;
	}

	/* Destruct the GASS server */
	result = ngcliGASSserverDestruct(gassMng, context->ngc_log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't construct the GASS Server Manager.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Initialize the Communication.
 */
static int
ngcllContextInitializeCommunication(ngclContext_t *context, int *error)
{
    int result;
    ngclLocalMachineInformation_t *lmInfo = NULL;
    static const char fName[] = "ngcllContextInitializeCommunication";

    assert(context->ngc_lmInfo != NULL);

    lmInfo = &context->ngc_lmInfo->nglmim_info;

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Creating listener for non crypt, authentication only, GSI, SSL.\n",
	fName);

    assert(lmInfo->nglmi_listenPort >= NGI_PORT_MIN);
    assert(lmInfo->nglmi_listenPort <= NGI_PORT_MAX);
    /* Initialize the Communication Manager for non crypt */
    context->ngc_commNone = ngiCommunicationConstructServer(
    	NG_PROTOCOL_CRYPT_NONE, lmInfo->nglmi_tcpNodelay,
        lmInfo->nglmi_listenPort, context->ngc_log, error);
    if (context->ngc_commNone == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Communication Manager for non crypt.\n",
	    fName);
	return 0;
    }

    assert(lmInfo->nglmi_listenPortAuthOnly >= NGI_PORT_MIN);
    assert(lmInfo->nglmi_listenPortAuthOnly <= NGI_PORT_MAX);
    /* Initialize the Communication Manager for authentication only */
    context->ngc_commAuthonly = ngiCommunicationConstructServer(
    	NG_PROTOCOL_CRYPT_AUTHONLY, lmInfo->nglmi_tcpNodelay,
        lmInfo->nglmi_listenPortAuthOnly, context->ngc_log, error);
    if (context->ngc_commAuthonly == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Communication Manager for authentication only.\n",
	    fName);
	goto error1;
    }

    assert(lmInfo->nglmi_listenPortGSI >= NGI_PORT_MIN);
    assert(lmInfo->nglmi_listenPortGSI <= NGI_PORT_MAX);
    /* Initialize the Communication Manager for GSI */
    context->ngc_commGSI = ngiCommunicationConstructServer(
    	NG_PROTOCOL_CRYPT_GSI, lmInfo->nglmi_tcpNodelay,
        lmInfo->nglmi_listenPortGSI, context->ngc_log, error);
    if (context->ngc_commGSI == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Communication Manager for GSI.\n",
	    fName);
	goto error2;
    }

    assert(lmInfo->nglmi_listenPortSSL >= NGI_PORT_MIN);
    assert(lmInfo->nglmi_listenPortSSL <= NGI_PORT_MAX);
    /* Initialize the Communication Manager for SSL */
    context->ngc_commSSL = ngiCommunicationConstructServer(
    	NG_PROTOCOL_CRYPT_SSL, lmInfo->nglmi_tcpNodelay,
        lmInfo->nglmi_listenPortSSL, context->ngc_log, error);
    if (context->ngc_commSSL == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Communication Manager for SSL.\n",
	    fName);
	goto error3;
    }

    /* Success */
    return 1;

    /* Error occurred */
error3:
    /* Destruct the Communication Manager for GSI */
    result = ngiCommunicationDestruct(
    	context->ngc_commGSI, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Communication Manager for GSI.\n",
	    fName);
	return 0;
    }

error2:
    /* Destruct the Communication Manager for authentication only */
    result = ngiCommunicationDestruct(
    	context->ngc_commAuthonly, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Communication Manager for authentication only.\n",
	    fName);
	return 0;
    }

error1:
    /* Destruct the Communication Manager for non crypt */
    result = ngiCommunicationDestruct(
    	context->ngc_commNone, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Communication Manager for non crypt.\n",
	    fName);
	return 0;
    }

    return 0;
}

/**
 * Initialize the Protocol.
 */
static int
ngcllContextInitializeProtocol(ngclContext_t *context, int *error)
{
    int result;
    ngiProtocolAttribute_t protoAttr;
    static const char fName[] = "ngcllContextInitializeProtocol";

    /* Initialize the attribute of Protocol Manager */
    protoAttr.ngpa_protocolType = NG_PROTOCOL_TYPE_BINARY;
    protoAttr.ngpa_architecture = NGI_ARCHITECTURE_ID;
    protoAttr.ngpa_xdr = NG_XDR_USE;
    protoAttr.ngpa_protocolVersion = NGI_PROTOCOL_VERSION;
    protoAttr.ngpa_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_DEFAULT;
    protoAttr.ngpa_contextID = context->ngc_ID;
    protoAttr.ngpa_jobID = NGI_JOB_ID_UNDEFINED;
    protoAttr.ngpa_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    protoAttr.ngpa_tmpDir = NULL;

    /* Initialize the Protocol Manager for non crypt */
    context->ngc_protoNone = ngiProtocolConstruct(
	&protoAttr, context->ngc_commNone,
        ngcliProtocolGetRemoteMethodInformation,  context->ngc_log, error);
    if (context->ngc_protoNone == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Protocol Manager for non crypt.\n",
	    fName);
	return 0;
    }

    /* Initialize the Protocol Manager for authentication only */
    context->ngc_protoAuthonly = ngiProtocolConstruct(
	&protoAttr, context->ngc_commAuthonly,
        ngcliProtocolGetRemoteMethodInformation,  context->ngc_log, error);
    if (context->ngc_protoAuthonly == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Protocol Manager for authentication only.\n",
	    fName);
	return 0;
    }

    /* Initialize the Protocol Manager for GSI */
    context->ngc_protoGSI = ngiProtocolConstruct(
	&protoAttr, context->ngc_commGSI,
        ngcliProtocolGetRemoteMethodInformation,  context->ngc_log, error);
    if (context->ngc_protoGSI == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Protocol Manager for GSI.\n",
	    fName);
	return 0;
    }

    /* Initialize the Protocol Manager for SSL */
    context->ngc_protoSSL = ngiProtocolConstruct(
	&protoAttr, context->ngc_commSSL,
        ngcliProtocolGetRemoteMethodInformation,  context->ngc_log, error);
    if (context->ngc_protoSSL == NULL) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Protocol Manager for SSL.\n",
	    fName);
	return 0;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	context->ngc_protoNone, context, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the User Data.\n", fName);
	return 0;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	context->ngc_protoAuthonly, context, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the User Data.\n", fName);
	return 0;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	context->ngc_protoGSI, context, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the User Data.\n", fName);
	return 0;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	context->ngc_protoSSL, context, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the User Data.\n", fName);
	return 0;
    }
    /* Success */
    return 1;
}

/**
 * Finalize the Protocol.
 */
static int
ngcllContextFinalizeProtocol(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextFinalizeProtocol";

    /* Unregister the User Data */
    result = ngiProtocolUnregisterUserData(
    	context->ngc_protoNone, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the User Data.\n", fName);
	return 0;
    }

    /* Unregister the User Data */
    result = ngiProtocolUnregisterUserData(
    	context->ngc_protoAuthonly, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the User Data.\n", fName);
	return 0;
    }

    /* Unregister the User Data */
    result = ngiProtocolUnregisterUserData(
    	context->ngc_protoGSI, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the User Data.\n", fName);
	return 0;
    }

    /* Unregister the User Data */
    result = ngiProtocolUnregisterUserData(
    	context->ngc_protoSSL, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the User Data.\n", fName);
	return 0;
    }

    /* Destruct the Protocol Manager for non crypt */
    result = ngiProtocolDestruct(
	context->ngc_protoNone, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol Manager for non crypt.\n",
	    fName);
	return 0;
    }

    /* Destruct the Protocol Manager for authentication only */
    result = ngiProtocolDestruct(
	context->ngc_protoAuthonly, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol Manager for authentication only.\n",
	    fName);
	return 0;
    }

    /* Destruct the Protocol Manager for GSI */
    result = ngiProtocolDestruct(
	context->ngc_protoGSI, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol Manager for GSI.\n",
	    fName);
	return 0;
    }

    /* Destruct the Protocol Manager for SSL */
    result = ngiProtocolDestruct(
	context->ngc_protoSSL , context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol Manager for SSL.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register the callback functions.
 */
static int
ngcllContextRegisterCallback(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextRegisterCallback";

    /* Register the callback function */
    result = ngcliGlobusIoTcpRegisterListen(
    	&context->ngc_commNone->ngc_ioHandle, ngcliCallbackAccept, 
	context->ngc_protoNone, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function.\n", fName);
	return 0;
    }

    /* Register the callback function */
    result = ngcliGlobusIoTcpRegisterListen(
    	&context->ngc_commAuthonly->ngc_ioHandle, ngcliCallbackAccept, 
	context->ngc_protoAuthonly, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function.\n", fName);
	return 0;
    }

    /* Register the callback function */
    result = ngcliGlobusIoTcpRegisterListen(
    	&context->ngc_commGSI->ngc_ioHandle, ngcliCallbackAccept, 
	context->ngc_protoGSI, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function.\n", fName);
	return 0;
    }

    /* Register the callback function */
    result = ngcliGlobusIoTcpRegisterListen(
    	&context->ngc_commSSL->ngc_ioHandle, ngcliCallbackAccept, 
	context->ngc_protoSSL, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister the callback functions.
 */
static int
ngcllContextUnregisterCallback(ngclContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngcllContextUnregisterCallback";

    /* Unregister the callback function */
    result = ngcliGlobusIoUnregister(
    	&context->ngc_commNone->ngc_ioHandle, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the callback function.\n", fName);
	return 0;
    }

    /* Unregister the callback function */
    result = ngcliGlobusIoUnregister(
    	&context->ngc_commAuthonly->ngc_ioHandle, context->ngc_log, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the callback function.\n", fName);
	return 0;
    }

    /* Unregister the callback function */
    result = ngcliGlobusIoUnregister(
    	&context->ngc_commGSI->ngc_ioHandle, context->ngc_log, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the callback function.\n", fName);
	return 0;
    }

    /* Unregister the callback function */
    result = ngcliGlobusIoUnregister(
    	&context->ngc_commSSL->ngc_ioHandle, context->ngc_log, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unregister the callback function.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the member.
 */
static void
ngcllContextInitializeMember(ngclContext_t *context)
{
    /* Check the argument */
    assert(context != NULL);

    /* Initialize the pointers */
    ngcllContextInitializePointer(context);
    context->ngc_ID = NGI_CONTEXT_ID_UNDEFINED;
    context->ngc_error = NG_ERROR_NO_ERROR;
    context->ngc_cbError = NG_ERROR_NO_ERROR;
    context->ngc_nJobs = 0;
    context->ngc_jobID = NGI_JOB_ID_MIN;
    context->ngc_nInvokeServers = 0;
    context->ngc_invokeServerID = 0;
    context->ngc_nExecutables = 0;
    context->ngc_executableID = NGI_EXECUTABLE_ID_MIN;
    context->ngc_nSessions = 0;
    context->ngc_sessionID = NGI_SESSION_ID_MIN;
    context->ngc_configFileReadCount = 0;
    context->ngc_configFileReading = 0;
    context->ngc_mdsAccessEnabled = 0;
}

/**
 * Initialize the pointer.
 */
static void
ngcllContextInitializePointer(ngclContext_t *context)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the members */
    context->ngc_next = NULL;
    context->ngc_apiNext = NULL;
    context->ngc_userData = NULL;
    context->ngc_userDestructer = NULL;
    context->ngc_commNone = NULL;
    context->ngc_commAuthonly = NULL;
    context->ngc_commGSI = NULL;
    context->ngc_commSSL = NULL;
    context->ngc_protoNone = NULL;
    context->ngc_protoAuthonly = NULL;
    context->ngc_protoGSI = NULL;
    context->ngc_protoSSL = NULL;
    context->ngc_log = NULL;
    context->ngc_gassMng_head = NULL;
    context->ngc_gassMng_tail = NULL;
    context->ngc_lmInfo = NULL;
    context->ngc_mdsInfo_head = NULL;
    context->ngc_mdsInfo_tail = NULL;
    context->ngc_isInfo_head = NULL;
    context->ngc_isInfo_tail = NULL;
    context->ngc_rmInfo_default = NULL;
    context->ngc_rmInfo_head = NULL;
    context->ngc_rmInfo_tail = NULL;
#if 0 /* Is this necessary? */
    context->ngc_epInfo_head = NULL;
    context->ngc_epInfo_tail = NULL;
#endif
    context->ngc_rcInfo_head = NULL;
    context->ngc_rcInfo_tail = NULL;
    context->ngc_jobMng_head = NULL;
    context->ngc_jobMng_tail = NULL;
    context->ngc_invokeCount_head = NULL;
    context->ngc_invokeCount_tail = NULL;
    context->ngc_invokeMng_head = NULL;
    context->ngc_invokeMng_tail = NULL;
    context->ngc_executable_head = NULL;
    context->ngc_executable_tail = NULL;
    context->ngc_destruction_executable_head = NULL;
    context->ngc_destruction_executable_tail = NULL;
    context->ngc_heartBeat = NULL;
    context->ngc_sessionTimeout = NULL;
    context->ngc_refreshCredentialsInfo = NULL;
    context->ngc_jobStartTimeout = NULL;
}


/**
 * Set GLOBUS_HOST environment variable
 */
static int
ngcllContextSetGlobusHostName(
    ngclContext_t *context,
    int *error)
{
    char hostName[NGI_HOST_NAME_MAX];
    ngclLocalMachineInformation_t *lmInfo = NULL;
    ngLog_t *log = NULL;
    int result;
    static const char fName[] ="ngcllContextSetGlobusHostName";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_lmInfo != NULL);

    lmInfo = &context->ngc_lmInfo->nglmim_info;
    log = context->ngc_log;

    result = ngcliNinfgManagerSetGlobusHostName(
        lmInfo->nglmi_hostName, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't set GLOBUS_HOSTNAME environment variable .\n", fName);
        return 0;
    }

    if (lmInfo->nglmi_hostName == NULL) {
        /* Set Default Hostname*/
        result = globus_libc_gethostname(hostName, NGI_HOST_NAME_MAX);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
            ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't get hostname.\n", fName);
            return 0;
        }
        lmInfo->nglmi_hostName = strdup(hostName);
        if (lmInfo->nglmi_hostName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the string.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Output the information for log file. 
 */
static int
ngcllContextLogOutput(
    ngclContext_t *context,
    int *error)
{
    int result;
    char hostName[NGI_HOST_NAME_MAX], *str;
    char workingDirectory[NGI_DIR_NAME_MAX], *resultPtr;
    static const char fName[] = "ngcllContextLogOutput";

    /* Check the arguments */
    assert(context != NULL);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Client log was created.\n", fName);

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Ninf-G Context is creating.\n", fName);

    /* hostname */
    result = globus_libc_gethostname(hostName, NGI_HOST_NAME_MAX);
    if (result != 0) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get hostname.\n", fName);
        /* not return */
    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: host name is \"%s\".\n", fName, hostName);
    }

    /* pid */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: process id = %ld.\n", fName, (long)getpid());

    /* current working directory */
    resultPtr = getcwd(workingDirectory, NGI_DIR_NAME_MAX);
    if (resultPtr == NULL) {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get current working directory.\n", fName);
        /* not return */
    } else {
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: cwd : \"%s\".\n", fName, workingDirectory);
    }

    /* pthread */
    str = "NonThread";
#ifdef NG_PTHREAD
    str = "Pthread";
#endif /* NG_PTHREAD */

    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: This Client binary is %s version.\n", fName, str);

    /* Success */
    return 1;
}

/**
 * Read the configuration file.
 */
int
ngclContextConfigurationFileRead(
    ngclContext_t *context,
    char *configFile,
    int *error)
{
    int result;
    static const char fName[] = "ngclContextConfigurationFileRead";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Read configuration file */
    result = ngcliConfigFileRead(context, configFile, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Configuration file error.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register the user defined data.
 */
int
ngclContextRegisterUserData(
    ngclContext_t *context,
    void *userData,
    void (*userDestructer)(ngclContext_t *),
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextRegisterUserData";

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

    result = ngcllContextRegisterUserData(context, userData,
	userDestructer, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}


static int
ngcllContextRegisterUserData(
    ngclContext_t *context,
    void *userData,
    void (*userDestructer)(ngclContext_t *),
    int *error)
{
    int result;
    static const char fName[] = "ngcllContextRegisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock this instance */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Are userData and userDestructer not NULL? */
    if ((context->ngc_userData != NULL) ||
    	(context->ngc_userDestructer == NULL)) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data has already registered.\n", fName);
	goto error;
    }

    /* Register the user defined data */
    context->ngc_userData = userData;
    context->ngc_userDestructer = userDestructer;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Unregister the user defined data.
 */
int
ngclContextUnregisterUserData(
    ngclContext_t *context,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextUnregisterUserData";

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

    result = ngcllContextUnregisterUserData(context, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterUserData(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcllContextUnregisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock this instance */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Is userData NULL? */
    if (context->ngc_userData == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data has already registered.\n", fName);
	goto error;
    }

    /* Unregister the user defined data */
    context->ngc_userData = NULL;
    context->ngc_userDestructer = NULL;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Get the user defined data.
 */
int
ngclContextGetUserData(ngclContext_t *context, void **userData, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextGetUserData";

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

    result = ngcllContextGetUserData(context, userData, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextGetUserData(ngclContext_t *context, void **userData, int *error)
{
    int result;
    static const char fName[] = "ngcllContextGetUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (userData == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: The userData is NULL.\n", fName);
        return 0;
    }

    /* Lock this instance */
    result = ngclContextReadLock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Get the user defined data */
    *userData = context->ngc_userData;

    /* Unlock this instance */
    result = ngclContextReadUnlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the error code.
 */
int
ngclContextGetError(ngclContext_t *context, int *error)
{
    int result;
    int retError;
    static const char fName[] = "ngclContextGetError";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return -1;
    }

    /* Lock this instance */
    result = ngclContextReadLock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
	return -1;
    }

    /* Get the error code */
    retError = context->ngc_error;

    /* Unlock this instance */
    result = ngclContextReadUnlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
	return -1;
    }

    /* Success */
    return retError;
}

/**
 * Set the error code.
 */
int
ngcliContextSetError(ngclContext_t *context, int setError, int *error)
{
    int result;
    static const char fName[] = "ngcliContextSetError";

    /* Check the argument */
    assert(context != NULL);

    /* Lock the Ninf-G Context */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
    	return 0;
    }

    /* Set the error */
    context->ngc_error = setError;

    /* Unlock the Ninf-G Context */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the error code, it occurred in callback function.
 */
int
ngcliContextSetCbError(ngclContext_t *context, int setError, int *error)
{
    int result;
    static const char fName[] = "ngcliContextSetCbError";

    /* Check the argument */
    assert(context != NULL);

    /* Lock the Ninf-G Context */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Context.\n", fName);
    	return 0;
    }

    /* Set the error */
    context->ngc_cbError = setError;

    /* Unlock the Ninf-G Context */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninf-G Context.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Is context valid?
 */
int
ngcliContextIsValid(
    ngclContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextIsValid";

    /* Is context NULL? */
    if (context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	"%s: Ninf-G Context is NULL.\n", fName);
	return 0;
    }

    /* Is Context valid? */
    result = ngcliNinfgManagerIsContextValid(context, NULL, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	"%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Ninf-G Context is valid */
    return 1;
}

/**
 * Register the GASS Server Manager to Ninf-G Context.
 */
static int
ngcllContextRegisterGASSserverManager(
    ngclContext_t *context,
    ngclGASSserverManager_t *gassMng,
    int *error)
{
    int result;
    ngclGASSserverManager_t *exist;
    static const char fName[] = "ngcllContextRegisterGASSserverManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(gassMng != NULL);

    /* Lock the list */
    result = ngcliGASSserverManagerListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    /* Is the Manager already registered? */
    exist = ngcliContextGASSserverManagerGet(
	context, gassMng->nggsm_scheme, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: GASS Server Manager of the scheme \"%s\" has already registered.\n",
	    fName, gassMng->nggsm_scheme);
	goto error;
    }

    /* Append at last of the list */
    gassMng->nggsm_next = NULL;
    if (context->ngc_gassMng_head == NULL) {
	/* No Manager is registered */
	assert(context->ngc_gassMng_tail == NULL);
	context->ngc_gassMng_head = gassMng;
	context->ngc_gassMng_tail = gassMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_gassMng_tail != NULL);
	assert(context->ngc_gassMng_tail->nggsm_next == NULL);
	context->ngc_gassMng_tail->nggsm_next = gassMng;
	context->ngc_gassMng_tail = gassMng;
    }

    /* Unlock the list */
    result = ngcliGASSserverManagerListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliGASSserverManagerListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Unregister the GASS Server Manager from Ninf-G Context.
 */
static int
ngcllContextUnregisterGASSserverManager(
    ngclContext_t *context,
    ngclGASSserverManager_t *gassMng,
    int *error)
{
    int result;
    ngclGASSserverManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcllContextUnregisterGASSserverManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(gassMng != NULL);

    /* Lock the list */
    result = ngcliGASSserverManagerListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_gassMng_head;
    curr = context->ngc_gassMng_head;
    for (; curr != gassMng; curr = curr->nggsm_next) {
	if (curr == NULL)
	    goto notFound;
	prev = curr;
	prevPtr = &curr->nggsm_next;
    }

    /* Unlink the list */
    *prevPtr = curr->nggsm_next;
    if (curr->nggsm_next == NULL) {
        context->ngc_gassMng_tail = prev;
    }
    gassMng->nggsm_next = NULL;

    /* Unlock the list */
    result = ngcliGASSserverManagerListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: GASS Server Manager of the scheme \"%s\" is not registered.\n",
	fName, gassMng->nggsm_scheme);

    /* Unlock the list */
    result = ngcliGASSserverManagerListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of GASS Server Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Get the GASS Server Manager by scheme.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclGASSserverManager_t *
ngcliContextGASSserverManagerGet(
    ngclContext_t *context,
    char *scheme,
    int *error)
{
    ngclGASSserverManager_t *gassMng;
#if 0
    static const char fName[] = "ngcliContextGASSserverManagerGet";
#endif

    /* Check the argument */
    assert(context != NULL);
    assert(scheme != NULL);
    assert(scheme[0] != '\0');

    /* Find the GASS Server Manager */
    gassMng = context->ngc_gassMng_head;
    for (; gassMng != NULL; gassMng = gassMng->nggsm_next) {
	assert(gassMng->nggsm_scheme != NULL);
	assert(gassMng->nggsm_scheme[0] != '\0');
	if (strcmp(gassMng->nggsm_scheme, scheme) == 0) {
	    /* Found */
	    return gassMng;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);

    /* Failed */
    return NULL;
}

/**
 * GASS Server Manager: Get the next manager.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclGASSserverManager_t *
ngcliContextGASSserverManagerGetNext(
    ngclContext_t *context,
    ngclGASSserverManager_t *current,
    int *error)
{
    static const char fName[] = "ngcliContextGASSserverManagerGetNext";

    /* Check the arguments */
    assert(context != NULL);

    if (context->ngc_gassMng_head == NULL) {
	/* No manager is registered */
	assert(context->ngc_gassMng_tail == NULL);
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: No GASS Server Manager is registered.\n", fName);
	return NULL;
    } else {
	assert(context->ngc_gassMng_tail != NULL);
    	if (current == NULL) {
	    /* Return the first manager */
	    return context->ngc_gassMng_head;
	} else {
	    /* Return the next manager */
	    if (current->nggsm_next != NULL) {
	        return current->nggsm_next;
	    }
	}
    }

    /* Not found */
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: No GASS Server Manager is found.\n", fName);

    return NULL;
}

/**
 * Register the Local Machine Information to Ninf-G Context.
 */
int
ngclContextRegisterLocalMachineInformation(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextRegisterLocalMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    result = ngcllContextRegisterLocalMachineInformation(context,
	lmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextRegisterLocalMachineInformation(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllContextRegisterLocalMachineInformation";

    /* Lock the list */
    result = ngcliLocalMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write lock the list of Local Machine Information.\n",
	    fName);
	return 0;
    }

    /* Is the information already registered? */
    if (context->ngc_lmInfo != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Local Machine Information has already registered.\n",
	    fName);
	goto error;
    }

    /* Append at last of the list */
    context->ngc_lmInfo = lmInfoMng;

    /* Unlock the list */
    result = ngcliLocalMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Local Machine Information.\n",
	    fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliLocalMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Local Machine Information.\n",
	    fName);
    }

    return 0;
}

/**
 * Unregister the Local Machine Information to Ninf-G Context.
 */
int
ngclContextUnregisterLocalMachineInformation(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextUnregisterLocalMachineInformation";

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

    result = ngcllContextUnregisterLocalMachineInformation(
	context, lmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterLocalMachineInformation(
    ngclContext_t *context,
    ngclLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    static const char fName[] = "ngcllContextUnregisterLocalMachineInformation";

    /* Is the information already registered? */
    if (context->ngc_lmInfo == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: Local Machine Information has not registered.\n",
	    fName);
	goto error;
    }

    /* The information registered will be demanded? */
    if (context->ngc_lmInfo != lmInfoMng) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: The Local Machine Information which registered is not demanded one.\n",
	    fName);
	goto error;
    }

    /* Delete from list */
    context->ngc_lmInfo = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    return 0;
}

/**
 * Register the MDS Server Information to Ninf-G Context.
 */
int
ngcliContextRegisterMDSserverInformation(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    int result;
    char *tagName, *hostName;
    ngcliMDSserverInformationManager_t *exist;
    static const char fName[] = "ngcliContextRegisterMDSserverInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    tagName = NULL;
    hostName = NULL;

    /* Lock the list */
    result = ngcliMDSserverInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of MDS Server Information.\n", fName);
	return 0;
    }

    tagName = mdsInfoMng->ngmsim_info.ngmsi_tagName;
    hostName = mdsInfoMng->ngmsim_info.ngmsi_hostName;

    /* Is the Information already registered? */
    exist = ngcliMDSserverInformationCacheGet(
	context, tagName, hostName,
        NGCLI_MDS_SERVER_CACHE_GET_MODE_PRECISE, error);
    if (exist != NULL) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: MDS Server Information of the tag name \"%s\""
            "host name \"%s\" has already registered.\n",
	    fName, ((tagName != NULL) ? tagName : "null"),
            ((hostName != NULL) ? hostName : "null"));
	goto error;
    }

    /* Append at last of the list */
    mdsInfoMng->ngmsim_next = NULL;
    if (context->ngc_mdsInfo_head == NULL) {
	/* No Information is registered */
	assert(context->ngc_mdsInfo_tail == NULL);
	context->ngc_mdsInfo_head = mdsInfoMng;
	context->ngc_mdsInfo_tail = mdsInfoMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_mdsInfo_tail != NULL);
	assert(context->ngc_mdsInfo_tail->ngmsim_next == NULL);
	context->ngc_mdsInfo_tail->ngmsim_next = mdsInfoMng;
	context->ngc_mdsInfo_tail = mdsInfoMng;
    }

    /* Unlock the list */
    result = ngcliMDSserverInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of MDS Server Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliMDSserverInformationListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of MDS Server Information.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Unregister the MDS Server Information from Ninf-G Context.
 */
int
ngcliContextUnregisterMDSserverInformation(
    ngclContext_t *context,
    ngcliMDSserverInformationManager_t *mdsInfoMng,
    int *error)
{
    ngcliMDSserverInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcliContextUnregisterMDSserverInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(mdsInfoMng != NULL);

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_mdsInfo_head;
    curr = context->ngc_mdsInfo_head;
    for (; curr != NULL; curr = curr->ngmsim_next) {
	if (curr == mdsInfoMng) {
	    /* Unlink the list */
	    *prevPtr = curr->ngmsim_next;
	    if (curr->ngmsim_next == NULL) {
	        context->ngc_mdsInfo_tail = prev;
	    }
	    mdsInfoMng->ngmsim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngmsim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: MDS Server Information of the host name \"%s\" is not registered.\n",
	fName, mdsInfoMng->ngmsim_info.ngmsi_hostName);

    return 0;
}

/**
 * Register the Invoke Server Information to Ninf-G Context.
 */
int
ngcliContextRegisterInvokeServerInformation(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    ngcliInvokeServerInformationManager_t *exist;
    static const char fName[] = "ngcliContextRegisterInvokeServerInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Lock the list */
    result = ngcliInvokeServerInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Invoke Server Information.\n", fName);
	return 0;
    }

    /* Is the Information already registered? */
    exist = ngcliInvokeServerInformationCacheGet(
	context, isInfoMng->ngisim_info.ngisi_type, error);
    if (exist != NULL) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invoke Server Information of the name \"%s\" has already registered.\n",
	    fName, isInfoMng->ngisim_info.ngisi_type);
	goto error;
    }

    /* Append at last of the list */
    isInfoMng->ngisim_next = NULL;
    if (context->ngc_isInfo_head == NULL) {
	/* No Information is registered */
	assert(context->ngc_isInfo_tail == NULL);
	context->ngc_isInfo_head = isInfoMng;
	context->ngc_isInfo_tail = isInfoMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_isInfo_tail != NULL);
	assert(context->ngc_isInfo_tail->ngisim_next == NULL);
	context->ngc_isInfo_tail->ngisim_next = isInfoMng;
	context->ngc_isInfo_tail = isInfoMng;
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Invoke Server Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Invoke Server Information.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Unregister the Invoke Server Information from Ninf-G Context.
 */
int
ngcliContextUnregisterInvokeServerInformation(
    ngclContext_t *context,
    ngcliInvokeServerInformationManager_t *isInfoMng,
    int *error)
{
    ngcliInvokeServerInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcliContextUnregisterInvokeServerInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_isInfo_head;
    curr = context->ngc_isInfo_head;
    for (; curr != NULL; curr = curr->ngisim_next) {
	if (curr == isInfoMng) {
	    /* Unlink the list */
	    *prevPtr = curr->ngisim_next;
	    if (curr->ngisim_next == NULL) {
	        context->ngc_isInfo_tail = prev;
	    }
	    isInfoMng->ngisim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngisim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: Invoke Server Information of the name \"%s\" is not registered.\n",
	fName, isInfoMng->ngisim_info.ngisi_type);

    return 0;
}

/**
 * Register the Default Remote Information to Ninf-G Context.
 */
int
ngclContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
	"ngclContextRegisterDefaultRemoteMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    result = ngcllContextRegisterDefaultRemoteMachineInformation(
	context, rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] =
	"ngcllContextRegisterDefaultRemoteMachineInformation";

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write lock the list of Default Remote Information.\n",
	    fName);
	return 0;
    }

    /* Is the information already registered? */
    if (context->ngc_rmInfo_default != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Default Remote Information has already registered.\n",
	    fName);
	goto error;
    }

    /* Append at last of the list */
    context->ngc_rmInfo_default = rmInfoMng;

    /* Unlock the list */
    result = 
	ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	    context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Default Remote Information.\n",
	    fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = 
	ngcliDefaultRemoteMachineInformationListWriteUnlock(context,
	   context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Default Remote Information.\n",
	    fName);
    }

    return 0;
}

/**
 * Unregister the Default Remote Information to Ninf-G Context.
 */
int
ngclContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
	"ngclContextUnregisterDefaultRemoteMachineInformation";

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

    result = ngcllContextUnregisterDefaultRemoteMachineInformation(
	context, rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    static const char fName[] =
	"ngcllContextUnregisterDefaultRemoteMachineInformation";

    /* Is the information already registered? */
    if (context->ngc_rmInfo_default == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: Default Remote Information has not registered.\n",
	    fName);
	goto error;
    }

    /* The information registered will be demanded? */
    if (context->ngc_rmInfo_default != rmInfoMng) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: The Default Remote Machine Information which registered is not demanded one.\n",
	    fName);
	goto error;
    }

    /* Delete from list */
    context->ngc_rmInfo_default = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    return 0;
}

/**
 * Register the Remote Machine Information to Ninf-G Context.
 */
int
ngclContextRegisterRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextRegisterRemoteMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    result = ngcllContextRegisterRemoteMachineInformation(context,
	rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextRegisterRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    char *hostName, *tagName;
    ngclRemoteMachineInformationManager_t *exist;
    static const char fName[] = "ngcllContextRegisterRemoteMachineInformation";

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write lock the list of Remote Machine Information.\n",
	    fName);
	return 0;
    }

    hostName = rmInfoMng->ngrmim_info.ngrmi_hostName;
    tagName = rmInfoMng->ngrmim_info.ngrmi_tagName;

    /* Is the same host name already registered? */
    exist = ngcliRemoteMachineInformationCacheGetWithTag(
	context, hostName, tagName, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Remote Machine Information of the hostname \"%s\""
	    " tag \"%s\" has already registered.\n",
	    fName, hostName, ((tagName != NULL) ? tagName : ""));
	goto error;
    }

    /* Append at last of the list */
    rmInfoMng->ngrmim_next = NULL;
    if (context->ngc_rmInfo_head == NULL) {
    	/* Information is not registered */
	context->ngc_rmInfo_head = rmInfoMng;
	context->ngc_rmInfo_tail = rmInfoMng;
    } else {
    	/* Any information is registered */
	context->ngc_rmInfo_tail->ngrmim_next = 
	    (struct ngclRemoteMachineInformationManager_s *)rmInfoMng;
	context->ngc_rmInfo_tail = rmInfoMng;
    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
	    fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
	    fName);
    	return 0;
    }

    return 0;
}

/**
 * Unregister the Remote Machine Information to Ninf-G Context.
 */
int
ngclContextUnregisterRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextUnregisterRemoteMachineInformation";

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

    result = ngcllContextUnregisterRemoteMachineInformation(
	context, rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterRemoteMachineInformation(
    ngclContext_t *context,
    ngclRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    ngclRemoteMachineInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] =
	"ngcllContextUnregisterRemoteMachineInformation";

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_rmInfo_head;
    curr = context->ngc_rmInfo_head;
    for (; curr != NULL; curr = curr->ngrmim_next) {
	if (curr == rmInfoMng) {
	    /* Found */
	    *prevPtr = curr->ngrmim_next;
	    if (curr->ngrmim_next == NULL) {
	        context->ngc_rmInfo_tail = prev;
	    }
	    rmInfoMng->ngrmim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngrmim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: Remote Machine Information of the hostname \"%s\" has not registered.\n",
	fName, rmInfoMng->ngrmim_info.ngrmi_hostName);

    return 0;
}

/**
 * Register the Job Management to Ninf-G Context.
 */
int
ngcliContextRegisterJobManager(
    ngclContext_t *context,
    struct ngcliJobManager_s *jobMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextRegisterJobManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);

    /* Lock the list */
    result = ngcliContextJobManagerListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Job Manager.\n",
	    fName);
	return 0;
    }

    /* Append at last of the list */
    jobMng->ngjm_next = NULL;
    if (context->ngc_jobMng_head == NULL) {
    	/* No information is registered yet */
    	assert(context->ngc_jobMng_tail == NULL);
	context->ngc_jobMng_head = jobMng;
	context->ngc_jobMng_tail = jobMng;
    } else {
    	/* Some information are registered */
    	assert(context->ngc_jobMng_tail != NULL);
    	assert(context->ngc_jobMng_tail->ngjm_next == NULL);
	context->ngc_jobMng_tail->ngjm_next = jobMng;
	context->ngc_jobMng_tail = jobMng;
    }

    /* Unlock the list */
    result = ngcliContextJobManagerListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Job Manager.\n",
	    fName);
	return 0;
    }

    return 1;
}

/**
 * Unregister the Job Manager from Ninf-G Context.
 */
int
ngcliContextUnregisterJobManager(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    struct ngcliJobManager_s *prev, *curr;
    ngLog_t *log;
    static const char fName[] = "ngcliContextUnregisterJobManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(context->ngc_jobMng_head != NULL);
    assert(context->ngc_jobMng_tail != NULL);

    /* Initialize the local variable */
    log = context->ngc_log;

    /* Lock the list */
    result = ngcliContextJobManagerListWriteLock(context, log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Job Manager.\n", fName);
	return 0;
    }

    /* Find the Job Manager */
    prev = NULL;
    curr = context->ngc_jobMng_head;
    for (; curr != jobMng; curr = curr->ngjm_next) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Job Manager */
    if (jobMng == context->ngc_jobMng_head)
    	context->ngc_jobMng_head = jobMng->ngjm_next;
    if (jobMng == context->ngc_jobMng_tail)
    	context->ngc_jobMng_tail = prev;
    if (prev != NULL)
    	prev->ngjm_next = jobMng->ngjm_next;
    jobMng->ngjm_next = NULL;

    /* Unlock the list */
    result = ngcliContextJobManagerListWriteUnlock(context, log, error);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Job Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
    	NG_LOG_LEVEL_WARNING, NULL,
	"%s: Job management is not found.\n", fName);

    /* Unlock the list */
    result = ngcliContextJobManagerListWriteUnlock(context, log, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Job Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Get the Job Manager by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliJobManager_t *
ngcliContextGetJobManager(ngclContext_t *context, int id, int *error)
{
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcliContextGetJobManager";

    /* Check the argument */
    assert(context != NULL);

    /* Is ID less than minimum? */
    if (id < NGI_JOB_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Job ID %d is less than %d.\n",
	    fName, id, NGI_JOB_ID_MIN);
	goto error;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_JOB_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Job ID %d is greater than %d.\n",
	    fName, id, NGI_JOB_ID_MAX);
	goto error;
    }

    jobMng = context->ngc_jobMng_head;
    for (; jobMng != NULL; jobMng = jobMng->ngjm_next) {
        assert(jobMng->ngjm_ID >= NGI_JOB_ID_MIN);
        assert(jobMng->ngjm_ID <= NGI_JOB_ID_MAX);
        if (jobMng->ngjm_ID == id) {
            /* Found */
            return jobMng;
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_DEBUG, NULL,
	"%s: Job Manager is not found by ID %d.\n", fName, id);

    /* Error occurred */
error:
    return NULL;
}

/**
 * Get next Job Manager in a list.
 *
 * Return the Job Manager from the top of list, if current is NULL.
 * Return the next Job Manager of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliJobManager_t *
ngcliContextGetNextJobManager(
    ngclContext_t *context,
    ngcliJobManager_t *current,
    int *error)
{
    static const char fName[] = "ngcliContextGetNextJobManager";

    /* Check the argument */
    assert(context != NULL);

    if (current == NULL) {
	/* Return the first Job Manager */
	if (context->ngc_jobMng_head != NULL) {
	    assert(context->ngc_jobMng_tail != NULL);
	    return context->ngc_jobMng_head;
	}
    } else {
	/* Return the next Job Manager */
	if (current->ngjm_next != NULL) {
	    return current->ngjm_next;
	}
    }

    /* The last Job Manager was reached */
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Job Manager was reached.\n", fName);

    return NULL;
}

/**
 * Register the Invoke Server Manager to Ninf-G Context.
 */
int
ngcliContextRegisterInvokeServerManager(
    ngclContext_t *context,
    struct ngcliInvokeServerManager_s *invokeMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextRegisterInvokeServerManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);

    /* Lock the list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Invoke Server Manager.\n",
	    fName);
	return 0;
    }

    /* Append at last of the list */
    invokeMng->ngism_next = NULL;
    if (context->ngc_invokeMng_head == NULL) {
    	/* No information is registered yet */
    	assert(context->ngc_jobMng_tail == NULL);
	context->ngc_invokeMng_head = invokeMng;
	context->ngc_invokeMng_tail = invokeMng;
    } else {
    	/* Some information are registered */
    	assert(context->ngc_invokeMng_tail != NULL);
    	assert(context->ngc_invokeMng_tail->ngism_next == NULL);
	context->ngc_invokeMng_tail->ngism_next = invokeMng;
	context->ngc_invokeMng_tail = invokeMng;
    }

    /* Count up */
    context->ngc_nInvokeServers++;

    /* Unlock the list */
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Invoke Server Manager.\n",
	    fName);
	return 0;
    }

    return 1;
}

/**
 * Unregister the Invoke Server Manager from Ninf-G Context.
 */
int
ngcliContextUnregisterInvokeServerManager(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
    int *error)
{
    int result;
    struct ngcliInvokeServerManager_s *prev, *curr;
    ngLog_t *log;
    static const char fName[] = "ngcliContextUnregisterInvokeServerManager";

    /* Check the arguments */
    assert(context != NULL);
    assert(invokeMng != NULL);
    assert(context->ngc_invokeMng_head != NULL);
    assert(context->ngc_invokeMng_tail != NULL);

    /* Initialize the local variable */
    log = context->ngc_log;

    /* Lock the list */
    result = ngcliContextInvokeServerManagerListWriteLock(
        context, log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Invoke Server Manager.\n", fName);
	return 0;
    }

    /* Find the Invoke Server Manager */
    prev = NULL;
    curr = context->ngc_invokeMng_head;
    for (; curr != invokeMng; curr = curr->ngism_next) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Invoke Server Manager */
    if (invokeMng == context->ngc_invokeMng_head)
    	context->ngc_invokeMng_head = invokeMng->ngism_next;
    if (invokeMng == context->ngc_invokeMng_tail)
    	context->ngc_invokeMng_tail = prev;
    if (prev != NULL)
    	prev->ngism_next = invokeMng->ngism_next;
    invokeMng->ngism_next = NULL;

    /* Count down */
    context->ngc_nInvokeServers--;

    /* Unlock the list */
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, error);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Invoke Server Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	"%s: Invoke Server Manager is not found.\n", fName);

    /* Unlock the list */
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Invoke Server Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Get the Invoke Server Manager by type and count.
 * count == -1 : find the last of "type" Invoke Server.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliInvokeServerManager_t *
ngcliContextGetInvokeServerManager(
    ngclContext_t *context,
    char *type,
    int count,
    int *error)
{
    ngcliInvokeServerManager_t *invokeMng, *returnInvokeMng;
    static const char fName[] = "ngcliContextGetInvokeServerManager";

    /* Check the argument */
    assert(context != NULL);

    /* Check the arguments */
    if (type == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invoke Server type is NULL.\n", fName);
        goto error;
    }

    returnInvokeMng = NULL;
    invokeMng = context->ngc_invokeMng_head;
    for (; invokeMng != NULL; invokeMng = invokeMng->ngism_next) {
        assert(invokeMng->ngism_serverType != NULL);
        if (strcmp(type, invokeMng->ngism_serverType) == 0) {
            if (count == -1) {
                /* Found */
                if (returnInvokeMng == NULL) {
                    returnInvokeMng = invokeMng;

                } else if (invokeMng->ngism_typeCount >=
                    returnInvokeMng->ngism_typeCount) {
                    returnInvokeMng = invokeMng;
                }
            } else {
                if (invokeMng->ngism_typeCount == count) {
                    /* Found */
                    returnInvokeMng = invokeMng;
                    break;
                }
            }
        }
    }

    /* Found */
    if (returnInvokeMng != NULL) {
        return returnInvokeMng;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
	"%s: Invoke Server Manager is not found by type \"%s\" (%d%s).\n",
        fName, type, count, ((count == -1) ? ":last" : ""));

    /* Error occurred */
error:
    return NULL;
}

/**
 * Get next Invoke Server Manager in a list.
 *
 * Return the Job Manager from the top of list, if current is NULL.
 * Return the next Job Manager of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngcliInvokeServerManager_t *
ngcliContextGetNextInvokeServerManager(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *current,
    int *error)
{
    static const char fName[] = "ngcliContextGetNextInvokeServerManager";

    /* Check the argument */
    assert(context != NULL);

    if (current == NULL) {
	/* Return the first Job Manager */
	if (context->ngc_invokeMng_head != NULL) {
	    assert(context->ngc_invokeMng_tail != NULL);
	    return context->ngc_invokeMng_head;
	}
    } else {
	/* Return the next Job Manager */
	if (current->ngism_next != NULL) {
	    return current->ngism_next;
	}
    }

    /* The last Job Manager was reached */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Invoke Server Manager was reached.\n", fName);

    return NULL;
}

/**
 * Register the Remote Class Information to Ninf-G Context.
 */
int
ngclContextRegisterRemoteClassInformation(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextRegisterRemoteClassInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    result = ngcllContextRegisterRemoteClassInformation(context,
	rcInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextRegisterRemoteClassInformation(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int result;
    ngclRemoteClassInformationManager_t *exist;
    static const char fName[] = "ngcllContextRegisterRemoteClassInformation";

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write lock the list of Remote Class Information.\n",
	    fName);
	return 0;
    }

    /* Is the same host name already registered? */
    exist = ngcliRemoteClassInformationCacheGet(
	context, rcInfoMng->ngrcim_info.ngrci_className, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Remote Class Information of the hostname \"%s\" has already registered.\n",
	    fName, rcInfoMng->ngrcim_info.ngrci_className);
	goto error;
    }

    /* Append at last of the list */
    rcInfoMng->ngrcim_next = NULL;
    if (context->ngc_rcInfo_head == NULL) {
    	/* Information is not registered */
	context->ngc_rcInfo_head = rcInfoMng;
	context->ngc_rcInfo_tail = rcInfoMng;
    } else {
    	/* Any information is registered */
	context->ngc_rcInfo_tail->ngrcim_next = 
	    (struct ngclRemoteClassInformationManager_s *)rcInfoMng;
	context->ngc_rcInfo_tail = rcInfoMng;
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
	    fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write unlock the list of Remote Machine Information.\n",
	    fName);
    	return 0;
    }

    return 0;
}

/**
 * Unregister the Remote Class Information to Ninf-G Context.
 */
int
ngclContextUnregisterRemoteClassInformation(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclContextUnregisterRemoteClassInformation";

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

    result = ngcllContextUnregisterRemoteClassInformation(context,
	rcInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterRemoteClassInformation(
    ngclContext_t *context,
    ngclRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    struct ngclRemoteClassInformationManager_s **prevPtr, *prev, *curr;
    static const char fName[] = "ngcllContextUnregisterRemoteClassInformation";

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_rcInfo_head;
    curr = context->ngc_rcInfo_head;
    for (; curr != NULL; curr = curr->ngrcim_next) {
	if (curr == rcInfoMng) {
	    /* Found */
	    *prevPtr = curr->ngrcim_next;
	    if (curr->ngrcim_next == NULL) {
	        context->ngc_rcInfo_tail = prev;
	    }
	    rcInfoMng->ngrcim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngrcim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_ERROR, NULL,
	"%s: Remote Class Information of the hostname \"%s\" has not registered.\n",
	fName, rcInfoMng->ngrcim_info.ngrci_className);

    return 0;
}

/**
 * Executable: Notify the complete Session.
 */
int
ngcliContextNotifyExecutable(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextNotifyExecutable";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngiMutexLock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Write status */
    context->ngc_flagExecutable = 1;

    /* Signal */
    result = ngiCondSignal(
    	&context->ngc_condExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't signal the Condition Variable.\n", fName);
    	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	goto error;
    }
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Executable: Wait until Session was complete.
 */
int
ngcliContextWaitExecutable(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextWaitExecutable";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngiMutexLock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    while (context->ngc_flagExecutable == 0) {
	/* Wait */
	result = ngiCondWait(
    	    &context->ngc_condExecutable, &context->ngc_mutexExecutable,
	    context->ngc_log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Get the all Session list.
 */
ngclSession_t *
ngcliContextGetAllSessionList(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclExecutable_t *executable;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliContextGetAllSessionList";

    /* Check the arguments */
    assert(context != NULL);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return NULL;
    }

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Executable.\n", fName);
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_apiNext = NULL;
    prev = &dummy.ngs_apiNext;
    executable = ngclExecutableGetNext(context, NULL, error);
    while (executable != NULL) {
    	session = ngcliExecutableGetAllSessionList(executable, error);
	/* Is Executable has any Sessions? */
	if (session != NULL) {
	    *prev = session;
	    for (; session->ngs_apiNext != NULL; session = session->ngs_apiNext);
	    prev = &session->ngs_apiNext;
	}
	executable = ngclExecutableGetNext(context, executable, error);
    }

    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
	return NULL;
    }

    /* Success */
    return dummy.ngs_apiNext;
}

/**
 * Get the Session list by ID.
 */
ngclSession_t *
ngcliContextGetSessionList(
    ngclContext_t *context,
    int *sessionID,
    int nSessions,
    int *error)
{
    int result;
    int i;
    int flagSession = 0;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliContextGetSessionList";

    /* Check the arguments */
    assert(context != NULL);
    assert(sessionID != NULL);
    assert(nSessions > 0);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
	goto error;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	goto error;
    }
    flagSession = 1;

    /* Make the list of Session */
    dummy.ngs_apiNext = NULL;
    prev = &dummy.ngs_apiNext;
    for (i = 0; i < nSessions; i++) {
    	/* Get the Session by ID */
	session = ngcliContextGetSession(context, sessionID[i], error);
	if (session == NULL) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session by ID %d.\n",
                fName, sessionID[i]);
	    goto error;
	}

	/* Make the list of Session */
	*prev = session;
	prev = &session->ngs_apiNext;
    }

    /* Unlock the list */
    flagSession = 0;
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	    goto error;
    }

    /* Success */
    return dummy.ngs_apiNext;

    /* Error occurred */
error:
    /* Unlock the list */
    if (flagSession != 0) {
	flagSession = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	}
    }

    /* Failed */
    return NULL;
}

/**
 * Get the Session list for cancel.
 */
ngclSession_t *
ngcliContextGetSessionCancelList(
    ngclContext_t *context,
    int *sessionID,
    int nSessions,
    int *error)
{
    int result;
    int i;
    int flagSession = 0;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliContextGetSessionCancelList";

    /* Check the arguments */
    assert(context != NULL);
    assert(sessionID != NULL);
    assert(nSessions > 0);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
	goto error;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	goto error;
    }
    flagSession = 1;

    /* Make the list of Session */
    dummy.ngs_cancelNext = NULL;
    prev = &dummy.ngs_cancelNext;
    for (i = 0; i < nSessions; i++) {
    	/* Get the Session by ID */
	session = ngcliContextGetSession(context, sessionID[i], error);
	if (session == NULL) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session by ID %d.\n",
                fName, sessionID[i]);
	    goto error;
	}

	/* Make the list of Session */
	*prev = session;
	prev = &session->ngs_cancelNext;
    }

    /* Unlock the list */
    flagSession = 0;
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	    goto error;
    }

    /* Success */
    return dummy.ngs_cancelNext;

    /* Error occurred */
error:
    /* Unlock the list */
    if (flagSession) {
	flagSession = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	}
    }

    /* Failed */
    return NULL;
}

/**
 * Get the Session by ID.
 */
ngclSession_t *
ngcliContextGetSession(
    ngclContext_t *context,
    int sessionID,
    int *error)
{
    int result;
    int flagSession = 0;
    ngclExecutable_t *executable;
    ngclSession_t *session;
    static const char fName[] = "ngcliContextGetSession";

    /* Check the arguments */
    assert(context != NULL);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
	goto error;
    }

    /* Lock the list of Session */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	goto error;
    }
    flagSession = 1;

    /* Get the Session by ID */
    for (executable = context->ngc_executable_head;
    	executable != NULL; executable = executable->nge_next) {
    	/* Get the Session by ID */
	session = ngclExecutableGetSession(executable, sessionID, error);
	if (session != NULL) {
	    goto found;
	}
	/* reset status of executable */
	NGI_SET_ERROR_EXECUTABLE(executable, NG_ERROR_NO_ERROR, NULL);
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    goto error;

    /* Found */
found:
    /* Unlock the list of Session */
    flagSession = 0;
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Session.\n", fName);
	goto error;
    }

    /* Success */
    return session;

    /* Error occurred */
error:
    /* Unlock the list */
    if (flagSession) {
	flagSession = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	}
    }

    /* set error */
    NGI_SET_ERROR_CONTEXT(context, *error, NULL);

    /* Failed */
    return NULL;
}

/**
 * Get the all Session list by waitNext.
 */
ngclSession_t *
ngcliContextGetAllSessionWaitList(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclExecutable_t *executable;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliContextGetAllSessionWaitList";

    /* Check the arguments */
    assert(context != NULL);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Context is not valid.\n", fName);
	return NULL;
    }

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Executable.\n", fName);
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_waitNext = NULL;
    prev = &dummy.ngs_waitNext;
    executable = ngclExecutableGetNext(context, NULL, error);
    while (executable != NULL) {
    	session = ngcliExecutableGetAllSessionWaitList(executable, error);
	/* Is Executable has any Sessions? */
	if (session != NULL) {
	    *prev = session;
	    for (; session->ngs_waitNext != NULL;
                   session = session->ngs_waitNext);
	    prev = &session->ngs_waitNext;
	}
	executable = ngclExecutableGetNext(context, executable, error);
    }

    /* Unlock the list of Executable */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
	return NULL;
    }

    /* Rotate the list of Executable */
    result = ngcliExecutableListRotate(context, context->ngc_log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't rotate the list of Executable.\n", fName);
        return NULL;
    }

    /* Success */
    return dummy.ngs_waitNext;
}

/**
 * Get the all Session list by cancelNext.
 */
ngclSession_t *
ngcliContextGetAllSessionCancelList(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngclExecutable_t *executable;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliContextGetAllSessionCancelList";

    /* Check the arguments */
    assert(context != NULL);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
	return NULL;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_cancelNext = NULL;
    prev = &dummy.ngs_cancelNext;
    executable = ngclExecutableGetNext(context, NULL, error);
    while (executable != NULL) {
    	session = ngcliExecutableGetAllSessionCancelList(executable, error);
	/* Is Executable has any Sessions? */
	if (session != NULL) {
	    *prev = session;
	    for (; session->ngs_cancelNext != NULL;
                   session = session->ngs_cancelNext);
	    prev = &session->ngs_cancelNext;
	}
	executable = ngclExecutableGetNext(context, executable, error);
    }

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Session.\n", fName);
	return NULL;
    }

    /* Success */
    return dummy.ngs_cancelNext;
}

/**
 * Session: Notify the complete Session.
 */
int
ngcliContextNotifySession(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliContextNotifySession";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngiMutexLock(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Write status */
    context->ngc_flagSession = 1;

    /* Signal */
    result = ngiCondBroadcast(
    	&context->ngc_condSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't signal the Condition Variable.\n", fName);
    	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	goto error;
    }
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Session: Wait until all Sessions were complete.
 */
int
ngcliContextWaitAllSession(
    ngclContext_t *context,
    ngclSession_t *list,
    ngLog_t *log,
    int *error)
{
    int result;
    int retResult = 1;
    int sessionLocked = 0;
    ngclSession_t *wait = NULL;
    ngclSession_t *done = NULL;
    static const char fName[] = "ngcliContextWaitAllSession";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngiMutexLock(&context->ngc_mutexSession, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the Mutex.\n", fName);
	result = 0;
	error = NULL;
	goto error;
    }
    sessionLocked = 1;

    wait = list;
    while (1) {
	/* Initialize the variables */
	done = NULL;

	/* Check the all sessions */
	result = ngcllContextCheckSessionDone(
	    context, &wait, &done, NGL_WAIT_ALL, log, error);
	if (result == 0) {
	    retResult = 0;
	    error = NULL;
	    goto error;
	}

    	/* Were any the sessions completed? */
	if (done != NULL) {
	    sessionLocked = 0;
	    result = ngiMutexUnlock(&context->ngc_mutexSession, log, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't unlock the Mutex.\n", fName);
		retResult = 0;
		error = NULL;
		goto error;
	    }

	    /* Perform the next session if any */
	    result = ngcllContextExecuteSession(context, done, log, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't perform the next Session.\n", fName);
		retResult = 0;
		error = NULL;
		/* Not returned */
	    }

	    /* Lock */
	    result = ngiMutexLock(&context->ngc_mutexSession, log, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the Mutex.\n",
		    fName);
		result = 0;
		error = NULL;
		goto error;
	    }
	    sessionLocked = 1;

	    /* Reconfirm the session */
	    continue;
	}

	/* Were all sessions complete? */
	if (wait == NULL)
	    break;

	/* Wait */
	result = ngiCondWait(
	    &context->ngc_condSession, &context->ngc_mutexSession, log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't wait the Condition Variable.\n", fName);
	    result = 0;
	    error = NULL;
	    goto error;
	}
    }

error:
    /* Unlock */
    if (sessionLocked != 0) {
	sessionLocked = 0;
	result = ngiMutexUnlock(&context->ngc_mutexSession, log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the Mutex.\n", fName);
	    result = 0;
	    error = NULL;
	}
    }

    /* Finish */
    return retResult;
}

/**
 * Session: Wait until any Sessions were complete.
 */
int
ngcliContextWaitAnySession(
    ngclContext_t *context,
    ngclSession_t *list,
    ngLog_t *log,
    int *error)
{
    int result;
    int sessionID = -1;
    ngclSession_t *curr;
    static const char fName[] = "ngcliContextWaitAnySession";

    /* Check the arguments */
    assert(context != NULL);

    while (1) {
	/* Is any Session complete? */
	for (curr = list; curr != NULL; curr = curr->ngs_waitNext) {
	    /* Is Session complete? */
	    if (curr->ngs_status == NG_SESSION_STATUS_DONE) {
	    	if (curr->ngs_error == NG_ERROR_NO_ERROR) {
	    	    sessionID = curr->ngs_ID;
		} else {
		    sessionID = -1;
		    NGI_SET_ERROR(error, curr->ngs_error);
		    error = NULL;
		    ngclLogPrintfSession(curr,
			NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
			NULL, "%s: returning with error code %d.\n",
			fName, curr->ngs_error);
		    /* Not returned */
		}

		/* Perform the next Session if any. */
		result = ngcliExecutableExecuteSession(
		    curr->ngs_executable, log, error);
		if (result == 0) {
		    ngclLogPrintfExecutable(
			curr->ngs_executable, NG_LOG_CATEGORY_NINFG_PURE,
			NG_LOG_LEVEL_INFORMATION, NULL,
			"%s: Can't perform the next Session.\n", fName);
		    /* Not returned */
		}
		goto end;
	    }
	}

	/* Wait */
	result = ngcllContextCondWaitSession(context, log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_INFORMATION, NULL,
		"%s: Can't wait the Session.\n", fName);
	    sessionID = -1;
	    goto end;
	}
    }
end:

    /* Success */
    return sessionID;
}

/**
 * Session: Check the status of all/any sessions.
 */
static int
ngcllContextCheckSessionDone(
    ngclContext_t *context,
    ngclSession_t **wait,
    ngclSession_t **done,
    nglWaitMode_t waitMode,
    ngLog_t *log,
    int *error)
{
    int retResult = 1;
    ngclSession_t **prev, *curr;
    static const char fName[] = "ngcllContextCheckSessionDone";

    /* Check the arguments */
    assert(context != NULL);
    assert(wait != NULL);
    assert(done != NULL);

    /* Initialize the variables */
    prev = wait;
    *done = NULL;
    for (curr = *wait; curr != NULL; curr = curr->ngs_waitNext) {
	/* Is Session not complete? */
	if (curr->ngs_status != NG_SESSION_STATUS_DONE) {
	    prev = &curr->ngs_waitNext;
	    continue;
	}

	/* Session was complete */
	*prev = curr->ngs_waitNext;
	assert(curr->ngs_doneNext == NULL);
	*done = curr;
	done = &curr->ngs_doneNext;

	/* Had the error occurred? */
	if (curr->ngs_error != NG_ERROR_NO_ERROR) {
	    retResult = 0;
	    NGI_SET_ERROR(error, curr->ngs_error);
	    error = NULL;
	    ngclLogPrintfSession(curr,
		NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
		NULL, "%s: returning with error code %d.\n",
		fName, curr->ngs_error);
	    break;
	}

	if (waitMode == NGL_WAIT_ANY)
	    break;
    }

    /* Success */
    return retResult;
}

/**
 * Perform the next Session if any.
 */
static int
ngcllContextExecuteSession(
    ngclContext_t *context,
    ngclSession_t *list,
    ngLog_t *log,
    int *error)
{
    int retResult = 1;
    int result;
    ngclSession_t *dummy, **prev;
    static const char fName[] = "ngcllContextExecuteSession";

    /* Check the arguments */
    assert(context != NULL);

    dummy = NULL;
    prev = &dummy;
    for (; list != NULL; list = list->ngs_doneNext) {
	/* Perform the next Session if any */
	result = ngcliExecutableExecuteSession(
	    list->ngs_executable, log, error);
	if (result == 0) {
	    ngclLogPrintfExecutable(list->ngs_executable,
		NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
		"%s: Can't perform the next Session.\n", fName);
	    retResult = 0;
	    error = NULL;
	    /* Not returned */
	}

	/* Unlink the list */
	*prev = NULL;
	prev = &list->ngs_doneNext;
    }

    return retResult;
}

/**
 * CondWait the Session.
 */
static int
ngcllContextCondWaitSession(ngclContext_t *context, ngLog_t *log, int *error)
{
    int result = 1;
    int sessionLocked = 0;
    static const char fName[] = "ngcllContextCondWaitSession";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngiMutexLock(&context->ngc_mutexSession, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the Mutex.\n", fName);
	result = 0;
	error = NULL;
	goto error;
    }
    sessionLocked = 1;

    /* Wait */
    result = ngiCondWait(
	&context->ngc_condSession, &context->ngc_mutexSession, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't wait the Condition Variable.\n", fName);
	result = 0;
	error = NULL;
	goto error;
    }

error:
    /* Unlock */
    if (sessionLocked != 0) {
	sessionLocked = 0;
	result = ngiMutexUnlock(&context->ngc_mutexSession, log, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the Mutex.\n", fName);
	    result = 0;
	    error = NULL;
	}
    }

    /* Success */
    return result;
}

/**
 * Job List: Cancel All jobs.
 */
static int
ngcllContextCancelAllJobsByForce(
    ngclContext_t *context,
    int *error)
{
    int result;
    ngcliJobManager_t *jobMng = NULL;
    static const char fName[] = "ngcllContextCancelAllJobsByForce";

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Lock the list */
    result = ngcliContextJobManagerListReadLock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Job Manager.\n",
	    fName);
	return 0;
    }

    /* Cancel All Jobs */
    for (jobMng = context->ngc_jobMng_head;
         jobMng != NULL; jobMng = jobMng->ngjm_next) {
        result = ngcliJobCancelByForce(jobMng, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't cancel Job.\n",
                fName);
            /* Not return */
        }
    }

    /* Unlock the list */
    result = ngcliContextJobManagerListReadUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Job Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}
