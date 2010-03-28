/*
 * $RCSfile: ngclContext.c,v $ $Revision: 1.37 $ $Date: 2008/03/27 10:39:24 $
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
 * Module of Context for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclContext.c,v $ $Revision: 1.37 $ $Date: 2008/03/27 10:39:24 $")

/* Flags for wait sessions */
typedef enum ngcllWaitMode_e {
    NGCLL_WAIT_ALL,
    NGCLL_WAIT_ANY
} ngcllWaitMode_t;

/**
 * Prototype declaration of internal functions.
 */
static int ngcllContextInitialize1st(ngclContext_t *, int *);
static int ngcllContextInitialize2nd(ngclContext_t *, char *, int *);
static int ngcllContextFinalize1st(ngclContext_t *, int *);
static int ngcllContextFinalize2nd(ngclContext_t *, int *);
static int ngcllContextInitializeRWlock(ngclContext_t *, int *);
static int ngcllContextFinalizeRWlock(ngclContext_t *, int *);
static int ngcllContextInitializeMutexAndCond(ngclContext_t *, int *);
static int ngcllContextFinalizeMutexAndCond(ngclContext_t *, int *);
static int ngcllContextInitializeCommunication(ngclContext_t *, int *);
static int ngcllContextInitializeProtocol(ngclContext_t *, int *);
static int ngcllContextFinalizeProtocol(ngclContext_t *, int *);
static int ngcllContextRegisterCallback(ngclContext_t *, int *);
static int ngcllContextUnregisterCallback(ngclContext_t *, int *);
static void ngcllContextInitializeMember(ngclContext_t *);
static void ngcllContextInitializePointer(ngclContext_t *);
static int ngcllContextSetHostName(ngclContext_t *, int *);
static int ngcllContextLogOutput(ngclContext_t *, int *);
static int ngcllContextSignalRegister(ngclContext_t *, int *);
static int ngcllContextRegisterUserData(
    ngclContext_t *, void *, void (*)(ngclContext_t *), int *);
static int ngcllContextUnregisterUserData(ngclContext_t *, int *);
static int ngcllContextGetUserData(ngclContext_t *, void **, int *);
static int ngcllContextRegisterLocalMachineInformation(
    ngclContext_t *, ngcliLocalMachineInformationManager_t *, int *);
static int ngcllContextUnregisterLocalMachineInformation(
    ngclContext_t *, ngcliLocalMachineInformationManager_t *, int *);
static int ngcllContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
static int ngcllContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
static int ngcllContextRegisterRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
static int ngcllContextUnregisterRemoteMachineInformation(
    ngclContext_t *, ngcliRemoteMachineInformationManager_t *, int *);
static int ngcllContextRegisterRemoteClassInformation(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);
static int ngcllContextUnregisterRemoteClassInformation(
    ngclContext_t *, ngcliRemoteClassInformationManager_t *, int *);

static int ngcllContextCondWaitSession(ngclContext_t *, ngLog_t *, int *);
static int ngcllContextCheckSessionDone(
    ngclContext_t *, ngclSession_t **, ngclSession_t **, ngcllWaitMode_t,
    ngLog_t *, int *);
static int ngcllContextExecuteSession(
    ngclContext_t *, ngclSession_t *, ngLog_t *, int *);

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
    context = NGI_ALLOCATE(ngclContext_t, NULL, error);
    if (context == NULL) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Ninf-G Context.\n"); 
	return NULL;
    }

    /* Initialize 1st pass */
    result = ngcllContextInitialize1st(context, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Ninf-G Context.\n"); 
        goto error;
    }
    initialize1st_done = 1;

    /* Register */
    result = ngcliNinfgManagerRegisterContext(context, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Ninf-G Context.\n"); 
        goto error;
    }
    register_done = 1;

    /* Initialize 2nd pass */
    result = ngcllContextInitialize2nd(context, configFile, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Ninf-G Context.\n"); 
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
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the Ninf-G Context.\n"); 
        }
    }

    /* Finalize 2nd pass */
    if (initialize1st_done != 0) {
        result = ngcllContextFinalize2nd(context, NULL);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the Ninf-G Context.\n"); 
        }
    }

    /* Free */
    result = NGI_DEALLOCATE(ngclContext_t, context, NULL, NULL);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the Ninf-G Context.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Finalize 1st pass */
    result = ngcllContextFinalize1st(context, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Ninf-G Context.\n"); 
        return 0;
    }

    /* Unregister */
    result = ngcliNinfgManagerUnregisterContext(
    	context, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the Ninf-G Context.\n"); 
        return 0;
    }

    /* Finalize 2nd pass */
    result = ngcllContextFinalize2nd(context, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Ninf-G Context.\n"); 
        return 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngclContext_t, context, NULL, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't deallocate the Ninf-G Context.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize 1st pass (called before the registration of context)
 */
static int
ngcllContextInitialize1st(ngclContext_t *context, int *error)
{
    int result, isPthread;
    static const char fName[] = "ngcllContextInitialize1st";

    /* Check the arguments */
    assert(context != NULL);

#ifdef NG_PTHREAD
    isPthread = 1;
#else /* NG_PTHREAD */
    isPthread = 0;
#endif /* NG_PTHREAD */

    /* Initialize the members */
    ngcllContextInitializeMember(context);

    /* Construct the temporary Log */
    context->ngc_log = NULL;

    /* Initialize the Ninf-G Client Manager */
    result = ngcliNinfgManagerInitialize(context->ngc_log, error);
    if (result == 0) {
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Ninf-G Client Manager.\n"); 
	return 0;
    }

    /* Create the Ninf-G Context ID */
    result = ngcliNinfgManagerCreateContextID(context->ngc_log, error);
    if (result < 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't create the Ninf-G Context ID.\n"); 
	return 0;
    }
    context->ngc_ID = result;

    if (isPthread == 0) {
        /**
         * On NonThread version, Ninf-G Event must be available before
         * ngiCondInitialize().
         */

        /* Initialize the Ninf-G Event */
        context->ngc_event = ngiEventConstruct(NULL, context->ngc_log, error);
        if (context->ngc_event == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Ninf-G Event module.\n"); 
            goto error;
        }
    }

    /* Initialize the Read/Write Lock */
    result = ngcllContextInitializeRWlock(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Read/Write Locks.\n"); 
	goto error;
    }

    /* Initialize the Mutex and Condition Variable */
    result = ngcllContextInitializeMutexAndCond(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Mutex and Condition Variable.\n"); 
	goto error;
    }

    /* Initialize the Listen Callback Waiter */
    result = ngiIOhandleCallbackWaiterInitialize(
        &context->ngc_protoCallbackWaiter,
        context->ngc_event, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the listen callback waiter.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the Ninf-G Client Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Ninf-G Client Manager.\n"); 
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
    int result, isPthread;
    static const char fName[] = "ngcllContextInitialize2nd";

#ifdef NG_PTHREAD
    isPthread = 1;
#else /* NG_PTHREAD */
    isPthread = 0;
#endif /* NG_PTHREAD */

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Read configuration file */
    result = ngcliConfigFileRead(context, configFile, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Configuration file error.\n"); 
        goto error;
    }

    /* Initialize the Log Manager */
    if (context->ngc_lmInfo != NULL) {
        context->ngc_log = ngiLogConstructFromConfig(
            &context->ngc_lmInfo->nglmim_info.nglmi_logInfo,
            &context->ngc_lmInfo->nglmim_info.nglmi_logLevels,
            "Client", NGI_LOG_EXECUTABLE_ID_NOT_APPEND,
            NULL, error);
        if (context->ngc_log == NULL) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Log Manager.\n"); 
            goto error;
        }
    }

    /* Set hostname */
    if (context->ngc_lmInfo != NULL) {
        result = ngcllContextSetHostName(context, error);
        if (result == 0) {
	    ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't set hostname.\n"); 
	    goto error;
        }
    }

    /* log */
    result = ngcllContextLogOutput(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't output the log.\n"); 
	goto error;
    }

    /* Register the signal */
    result = ngcllContextSignalRegister(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the signal.\n"); 
	goto error;
    }

    if (isPthread != 0) {
        assert(context->ngc_event == NULL);

        /**
         * On Pthread version, Event must be created after
         * ngiSignalManagerStart().
         */
        /* Initialize the Ninf-G Event. */
        context->ngc_event = ngiEventConstruct(NULL, context->ngc_log, error);
        if (context->ngc_event == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Ninf-G Event module.\n"); 
            goto error;
        }
    } else {
        assert(context->ngc_event != NULL);

        /* Set log to Ninf-G Event. */
        result = ngiEventLogSet(
            context->ngc_event, context->ngc_log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the log to Ninf-G Event.\n"); 
            goto error;
        }
    }

    /* Initialize the Random Number */
    result = ngiRandomNumberInitialize(
        &context->ngc_randomStatus, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Random Number.\n"); 
	goto error;
    }

    /* Initialize the External Module. */
    context->ngc_externalModuleManager =
	ngiExternalModuleManagerConstruct(
	context->ngc_event, context->ngc_log, error);
    if (context->ngc_externalModuleManager == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the External Module Manager.\n"); 
	goto error;
    }

    /* Initialize the Communication */
    result = ngcllContextInitializeCommunication(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Communication.\n"); 
	goto error;
    }

    /* Initialize the Protocol */
    result = ngcllContextInitializeProtocol(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Protocol.\n"); 
	goto error;
    }

    /* Initialize the Communication Proxy */
    context->ngc_communicationProxyManager = 
        ngcliCommunicationProxyManagerConstruct(
            context, context->ngc_comm->ngc_portNo, error);
    if (context->ngc_communicationProxyManager == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Communication Proxy Manager.\n"); 
	goto error;
    }

    /* Register the Callback */
    result = ngcllContextRegisterCallback(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Callback.\n"); 
	goto error;
    }

    context->ngc_queryManager = ngcliQueryManagerConstruct(context, error);
    if (context->ngc_queryManager == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't construct the Query Manager.\n"); 
        goto error;
    }

    /* Initialize the HeartBeat */
    result = ngcliHeartBeatInitialize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the heartbeat.\n"); 
        goto error;
    }

    /* Initialize the Session timeout */
    result = ngcliSessionTimeoutInitialize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the session timeout.\n"); 
        goto error;
    }

    /* Initialize the Transfer timeout */
    result = ngcliTransferTimeoutInitialize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the transfer timeout.\n"); 
        goto error;
    }

    /* Initialize the job start timeout */
    result = ngcliJobStartTimeoutInitialize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the job start timeout.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the Ninf-G Client Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Ninf-G Client Manager.\n"); 
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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Execute the user defined destructor */
    if (context->ngc_userDestructer != NULL) {
	context->ngc_userDestructer(context);
    } else {
	if (context->ngc_userData != NULL) {
	    ngiFree(context->ngc_userData, context->ngc_log, error);
	}
    }
    context->ngc_userData = NULL;
    context->ngc_userDestructer = NULL;

    /* log for remaining Executables */
    if ((context->ngc_nExecutables > 0) ||
        (context->ngc_executable_head != NULL)) {
    	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Some remaining Executables still exists.\n"); 

        /* Not return */
    }

    /* Finalize the job start timeout */
    result = ngcliJobStartTimeoutFinalize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the job start timeout.\n"); 
	return 0;
    }

    /* Finalize the Transfer timeout */
    result = ngcliTransferTimeoutFinalize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the transfer timeout.\n"); 
	return 0;
    }

    /* Finalize the Session timeout */
    result = ngcliSessionTimeoutFinalize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the session timeout.\n"); 
	return 0;
    }

    /* Finalize the HeartBeat */
    result = ngcliHeartBeatFinalize(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the heartbeat.\n"); 
	return 0;
    }

    /* Destruct all Invoke Servers */
    result = ngcliInvokeServerDestruct(context, NULL, 1, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the Invoke Server.\n"); 
	return 0;
    }

    result = ngcliQueryManagerDestruct(context->ngc_queryManager, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the Query Manager.\n"); 
        return 0;
    }

    /* Unregister the Callback */
    result = ngcllContextUnregisterCallback(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the Callback.\n"); 
	return 0;
    }

    /* Finalize the Communication Proxy */
    result = ngcliCommunicationProxyManagerDestruct(
        context->ngc_communicationProxyManager, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Communication Proxy Manager.\n"); 
        return 0;
    }

    /* Finalize the Protocol */
    result = ngcllContextFinalizeProtocol(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Protocol.\n"); 
	return 0;
    }

    /* Finalize the External Module. */
    result = ngiExternalModuleManagerDestruct(
	context->ngc_externalModuleManager, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the External Module Manager.\n"); 
	return 0;
    }

    /* Finalize the Ninf-G Event. */
    result = ngiEventDestruct(context->ngc_event, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the Event module.\n"); 
	return 0;
    }

    /* Finalize the Random Number */
    result = ngiRandomNumberFinalize(
	&context->ngc_randomStatus, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Random Number.\n"); 
	return 0;
    }

    /* Unregister the all information of Remote Class */
    result = ngcliRemoteClassInformationCacheUnregister(context, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Remote Class Information.\n"); 
	return 0;
    }

    /* Unregister the all information of Executable Path */
    result = ngcliExecutablePathInformationCacheUnregister(context, NULL, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executables Path Information.\n"); 
	return 0;
    }

    /* Unregister information of Default Remote Machine */
    result = ngcliDefaultRemoteMachineInformationCacheUnregister(context,
	error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister Default Remote Machine Information.\n"); 
	return 0;
    }

    /* Unregister the all information of Remote Machine */
    result = ngcliRemoteMachineInformationCacheUnregister(context, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Remote Machine Information.\n"); 
	return 0;
    }

    /* Unregister the all Information of Information Service */
    result = ngcliInformationServiceInformationCacheUnregister(
	context, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Information Service Information.\n"); 
	return 0;
    }

    /* Unregister the all Information of Communication Proxy */
    result = ngcliCommunicationProxyInformationCacheUnregister(
	context, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Communication Proxy Information.\n"); 
	return 0;
    }

    /* Unregister the all Information of Invoke Server */
    result = ngcliInvokeServerInformationCacheUnregister(
	context, NULL, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Invoke Server Information.\n"); 
	return 0;
    }

    /* Unregister the all Information of Local Machine */
    result = ngcliLocalMachineInformationCacheUnregister(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Local Machine Information.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the log for Signal Manager.\n"); 
	return 0;
    }

    /* Destruct the Log */
    if (context->ngc_log != NULL) {
	ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Client log destruct.\n"); 

	result = ngLogDestruct(context->ngc_log, NULL, error);
	if (result == 0) {
	    ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Log.\n"); 
	    return 0;
	}
    }
    context->ngc_log = NULL;

    /* Finalize the Listen Callback Waiter */
    result = ngiIOhandleCallbackWaiterFinalize(
        &context->ngc_protoCallbackWaiter,
        context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the listen callback waiter.\n"); 
	return 0;
    }

    /* Finalize the Mutex and Condition Variable */
    result = ngcllContextFinalizeMutexAndCond(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Mutex and Condition Variable.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock */
    result = ngcllContextFinalizeRWlock(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Read/Write Lock.\n"); 
	return 0;
    }

    /* Finalize Ninf-G Manager */
    result = ngcliNinfgManagerFinalize(context->ngc_log, error);
    if (result == 0) {
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Ninf-G Client Manager.\n"); 
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
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for own instance.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Local Machine Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlLmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Local Machine Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Invoke Server Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlInvokeServerInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Invoke Server Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Communication Proxy Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlCpInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Communication Proxy Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Information Service Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlInfoServiceInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Information Service Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Remote Machine Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlRmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Remote Machine Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Executable Path Information*/
    result = ngiRWlockInitialize(
	&context->ngc_rwlEpInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Remote Machine Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Remote Class Information */
    result = ngiRWlockInitialize(
	&context->ngc_rwlRcInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Remote Class Information.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Job Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlJobMng, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Job Manager.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Invoke Server Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlInvokeMng, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for "
            "Invoke Server Manager.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Executable Handle */
    result = ngiRWlockInitialize(
	&context->ngc_rwlExecutable, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Executable Handle.\n"); 
        goto error;
    }

    /* Initialize the Read/Write Lock for Session Manager */
    result = ngiRWlockInitialize(
	&context->ngc_rwlSession, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock for Session Manager.\n"); 
        goto error;
    }

    /* Success */
    return 1;

error:
    result = ngcllContextFinalizeRWlock(context, NULL);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write locks.\n");
    }
    return 0;
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
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for own instance.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Local Machine Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlLmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Local Machine Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Invoke Server Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlInvokeServerInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Invoke Server Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Communication Proxy Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlCpInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Communication Proxy Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Information Service Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlInfoServiceInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Information Service Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Remote Machine Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlRmInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Remote Machine Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Remote Class Information */
    result = ngiRWlockFinalize(
	&context->ngc_rwlRcInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Remote Class Information.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Job Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlJobMng, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Job Manager.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Invoke Server Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlInvokeMng, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for "
            "Invoke Server Manager.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Executable Handle */
    result = ngiRWlockFinalize(
	&context->ngc_rwlExecutable, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Executable Handle.\n"); 
	return 0;
    }

    /* Finalize the Read/Write Lock for Session Manager */
    result = ngiRWlockFinalize(
	&context->ngc_rwlSession, context->ngc_log, error);
    if (result == 0) {
        ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock for Session Manager.\n"); 
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
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Mutex for Executable.\n"); 
	return 0;
    }

    /* Initialize the Condition Variable for Executable */
    result = ngiCondInitialize(
    	&context->ngc_condExecutable, context->ngc_event,
    	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Cond for Executable.\n"); 
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagExecutable = 0;

    /* Initialize the Mutex for Session */
    result = ngiMutexInitialize(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Mutex for Session.\n"); 
	return 0;
    }

    /* Initialize the Condition Variable for Session */
    result = ngiCondInitialize(
    	&context->ngc_condSession, context->ngc_event,
    	context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Cond for Session.\n"); 
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagSession = 0;

    /* Initialize the Mutex for Configuration File */
    result = ngiMutexInitialize(
    	&context->ngc_mutexConfig, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Mutex for Configuration file.\n"); 
	return 0;
    }

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

    /* Finalize the Mutex for Configuration file */
    result = ngiMutexDestroy(
    	&context->ngc_mutexConfig, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Mutex for Executable.\n"); 
	return 0;
    }

    /* Finalize the Mutex for Executable */
    result = ngiMutexDestroy(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Mutex for Executable.\n"); 
	return 0;
    }

    /* Finalize the Condition Variable for Executable */
    result = ngiCondDestroy(
    	&context->ngc_condExecutable, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Cond for Executable.\n"); 
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagExecutable = 0;

    /* Finalize the Mutex for Session */
    result = ngiMutexDestroy(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Mutex for Session.\n"); 
	return 0;
    }

    /* Finalize the Condition Variable for Session */
    result = ngiCondDestroy(
    	&context->ngc_condSession, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Cond for Session.\n"); 
	return 0;
    }

    /* Initialize the flag */
    context->ngc_flagSession = 0;

    /* Success */
    return 1;
}

/**
 * Initialize the Communication.
 */
static int
ngcllContextInitializeCommunication(ngclContext_t *context, int *error)
{
    ngclLocalMachineInformation_t *lmInfo = NULL;
    static const char fName[] = "ngcllContextInitializeCommunication";

    assert(context->ngc_lmInfo != NULL);

    lmInfo = &context->ngc_lmInfo->nglmim_info;

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Creating listener.\n"); 

    assert(lmInfo->nglmi_listenPort >= NGI_PORT_MIN);
    assert(lmInfo->nglmi_listenPort <= NGI_PORT_MAX);

    /* Initialize the Communication Manager */
    context->ngc_comm = ngiCommunicationConstructServer(
        context->ngc_event,
    	lmInfo->nglmi_listenPort, context->ngc_log,
        error);
    if (context->ngc_comm == NULL) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the Communication Manager.\n"); 
	return 0;
    }

    /* Success */
    return 1;
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
    result = ngiProtocolAttributeInitialize(
        &protoAttr, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't initialize the protocol attribute.\n"); 
	return 0;
    }

    protoAttr.ngpa_architecture = NGI_ARCHITECTURE_ID;
    protoAttr.ngpa_xdr = NG_XDR_USE;
    protoAttr.ngpa_protocolVersion = NGI_PROTOCOL_VERSION;
    protoAttr.ngpa_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_DEFAULT;
    protoAttr.ngpa_simpleAuthNumber = 0;
    protoAttr.ngpa_contextID = context->ngc_ID;
    protoAttr.ngpa_jobID = NGI_JOB_ID_UNDEFINED;
    protoAttr.ngpa_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    protoAttr.ngpa_tmpDir = NULL;
    protoAttr.ngpa_keepConnect = 1;

    /* Initialize the Protocol Manager */
    context->ngc_proto = ngiProtocolConstruct(
	&protoAttr, context->ngc_comm, context->ngc_event,
        NULL,
        context->ngc_log, error);
    if (context->ngc_proto == NULL) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the Protocol Manager.\n"); 
	return 0;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	context->ngc_proto, context, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the User Data.\n"); 
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
    ngLog_t *log;
    static const char fName[] = "ngcllContextFinalizeProtocol";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Close and invoke the Listen callback */
    result = ngiIOhandleClose(
        context->ngc_comm->ngc_ioHandle, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't close the Listener handle.\n"); 
	return 0;
    }

    /* Wait Listen Callback End */
    result = ngiIOhandleCallbackWaiterWait(
        &context->ngc_protoCallbackWaiter, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't wait Listen callback End.\n"); 
	return 0;
    }

    /* Unregister the User Data */
    result = ngiProtocolUnregisterUserData(
    	context->ngc_proto, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the User Data.\n"); 
	return 0;
    }

    /* Destruct the Protocol Manager */
    result = ngiProtocolDestruct(
	context->ngc_proto, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the Protocol Manager.\n"); 
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
    ngLog_t *log;
    static const char fName[] = "ngcllContextRegisterCallback";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Listen Callback Start */
    result = ngiIOhandleCallbackWaiterStart(
        &context->ngc_protoCallbackWaiter, log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the callback function.\n"); 
	return 0;
    }

    /* Register the callback function */
    result = ngiIOhandleTCPlistenerCallbackRegister(
    	context->ngc_comm->ngc_ioHandle,
        ngcliCallbackAccept, 
	context->ngc_proto,
        log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the callback function.\n"); 
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
    result = ngiIOhandleTCPlistenerCallbackUnregister(
        context->ngc_comm->ngc_ioHandle, context->ngc_log, NULL);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the callback function.\n"); 
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
    context->ngc_nExecutables = 0;
    context->ngc_executableID = NGI_EXECUTABLE_ID_MIN;
    context->ngc_nSessions = 0;
    context->ngc_sessionID = NGI_SESSION_ID_MIN;
    context->ngc_configFileReadCount = 0;
    context->ngc_configFileReading = 0;
    context->ngc_randomStatus = 0;

    /* RW locks */
    context->ngc_rwlOwn              = NGI_RWLOCK_NULL;
    context->ngc_rwlLmInfo           = NGI_RWLOCK_NULL;
    context->ngc_rwlInvokeServerInfo = NGI_RWLOCK_NULL;
    context->ngc_rwlCpInfo           = NGI_RWLOCK_NULL;
    context->ngc_rwlInfoServiceInfo  = NGI_RWLOCK_NULL;
    context->ngc_rwlRmInfo           = NGI_RWLOCK_NULL;
    context->ngc_rwlEpInfo           = NGI_RWLOCK_NULL;
    context->ngc_rwlRcInfo           = NGI_RWLOCK_NULL;
    context->ngc_rwlJobMng           = NGI_RWLOCK_NULL;
    context->ngc_rwlInvokeMng        = NGI_RWLOCK_NULL;
    context->ngc_rwlExecutable       = NGI_RWLOCK_NULL;
    context->ngc_rwlSession          = NGI_RWLOCK_NULL;
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
    context->ngc_comm = NULL;
    context->ngc_proto = NULL;
    context->ngc_log = NULL;
    context->ngc_lmInfo = NULL;
    context->ngc_invokeServerInfo_head = NULL;
    context->ngc_invokeServerInfo_tail = NULL;
    context->ngc_cpInfo_head = NULL;
    context->ngc_cpInfo_tail = NULL;
    context->ngc_infoServiceInfo_head = NULL;
    context->ngc_infoServiceInfo_tail = NULL;
    context->ngc_rmInfo_default = NULL;
    context->ngc_rmInfo_head = NULL;
    context->ngc_rmInfo_tail = NULL;
    context->ngc_epInfo_head = NULL;
    context->ngc_epInfo_tail = NULL;
    context->ngc_rcInfo_head = NULL;
    context->ngc_rcInfo_tail = NULL;
    context->ngc_jobMng_head = NULL;
    context->ngc_jobMng_tail = NULL;
    context->ngc_externalModuleManager = NULL;
    context->ngc_invokeMng_head = NULL;
    context->ngc_invokeMng_tail = NULL;
    context->ngc_communicationProxyManager = NULL;
    context->ngc_queryManager = NULL;
    context->ngc_executable_head = NULL;
    context->ngc_executable_tail = NULL;
    context->ngc_destruction_executable_head = NULL;
    context->ngc_destruction_executable_tail = NULL;
    context->ngc_sessionTimeoutHandle = NULL;
    context->ngc_transferTimeoutHandle = NULL;
    context->ngc_jobStartTimeoutHandle = NULL;
}


/**
 * Set hostname
 */
static int
ngcllContextSetHostName(
    ngclContext_t *context,
    int *error)
{
    char hostName[NGI_HOST_NAME_MAX];
    ngclLocalMachineInformation_t *lmInfo = NULL;
    ngLog_t *log = NULL;
    int result;
    static const char fName[] ="ngcllContextSetHostName";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_lmInfo != NULL);

    lmInfo = &context->ngc_lmInfo->nglmim_info;
    log = context->ngc_log;

    if (lmInfo->nglmi_hostName == NULL) {
        /* Set Default Hostname*/
        result = ngiHostnameGet(hostName, NGI_HOST_NAME_MAX, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get hostname.\n"); 
            return 0;
        }

        lmInfo->nglmi_hostName = strdup(hostName);
        if (lmInfo->nglmi_hostName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the string.\n"); 
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

    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Client log was created.\n"); 

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Ninf-G Context is creating.\n"); 

    /* hostname */
    result = ngiHostnameGet(
        hostName, NGI_HOST_NAME_MAX, context->ngc_log, error);
    if (result == 0) {
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get hostname.\n"); 
        /* not return */
    } else {
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "host name is \"%s\".\n", hostName); 
    }

    /* pid */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "process id = %ld.\n", (long)getpid()); 

    /* current working directory */
    resultPtr = getcwd(workingDirectory, NGI_DIR_NAME_MAX);
    if (resultPtr == NULL) {
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get current working directory.\n"); 
        /* not return */
    } else {
        ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "cwd : \"%s\".\n", workingDirectory); 
    }

    /* pthread */
    str = "NonThread";
#ifdef NG_PTHREAD
    str = "Pthread";
#endif /* NG_PTHREAD */

    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "This Client binary is %s version.\n", str); 

    /* Success */
    return 1;
}

/**
 * Register the signal.
 */
static int
ngcllContextSignalRegister(
    ngclContext_t *context,
    int *error)
{
    ngLog_t *log;
    int *signalTable, size, result;
    static const char fName[] = "ngcllContextSignalRegister";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    size = 0;
    signalTable = context->ngc_lmInfo->nglmim_info.nglmi_signals;

    if (signalTable != NULL) {
        /* Find the tail */
        for (; signalTable[size] != 0; size++);
    } else {
        signalTable = NULL; /* NULL is default, size 0 is no-signal */
    }

    result = ngcliNinfgManagerSignalRegister(
        signalTable, size, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the Signal.\n");
        return 0;
    }

    result = ngcliNinfgManagerSignalManagerStart(log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start the Signal Manager.\n");
        return 0;
    }

    result = ngcliNinfgManagerSignalManagerLogSet(
        log, context->ngc_log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the log to Signal Manager.\n"); 
        return 0;
    }

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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Read configuration file */
    result = ngcliConfigFileRead(context, configFile, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Configuration file error.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	return 0;
    }

    /* Are userData and userDestructer not NULL? */
    if ((context->ngc_userData != NULL) ||
    	(context->ngc_userDestructer == NULL)) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "User Data has already registered.\n"); 
	goto error;
    }

    /* Register the user defined data */
    context->ngc_userData = userData;
    context->ngc_userDestructer = userDestructer;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, NULL);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	return 0;
    }

    /* Is userData NULL? */
    if (context->ngc_userData == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "User Data has already registered.\n"); 
	goto error;
    }

    /* Unregister the user defined data */
    context->ngc_userData = NULL;
    context->ngc_userDestructer = NULL;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, NULL);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "The userData is NULL.\n"); 
        return 0;
    }

    /* Lock this instance */
    result = ngclContextReadLock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	return 0;
    }

    /* Get the user defined data */
    *userData = context->ngc_userData;

    /* Unlock this instance */
    result = ngclContextReadUnlock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return -1;
    }

    /* Lock this instance */
    result = ngclContextReadLock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	return -1;
    }

    /* Get the error code */
    retError = context->ngc_error;

    /* Unlock this instance */
    result = ngclContextReadUnlock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Ninf-G Context.\n"); 
    	return 0;
    }

    /* Set the error */
    context->ngc_error = setError;

    /* Unlock the Ninf-G Context */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Ninf-G Context.\n"); 
    	return 0;
    }

    /* Set the error */
    context->ngc_cbError = setError;

    /* Unlock the Ninf-G Context */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Ninf-G Context.\n"); 
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
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is NULL.\n"); 
	return 0;
    }

    /* Is Context valid? */
    result = ngcliNinfgManagerIsContextValid(context, NULL, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Ninf-G Context is valid */
    return 1;
}

/**
 * Register the Local Machine Information to Ninf-G Context.
 */
int
ngcliContextRegisterLocalMachineInformation(
    ngclContext_t *context,
    ngcliLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngcliContextRegisterLocalMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    ngcliLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllContextRegisterLocalMachineInformation";

    /* Lock the list */
    result = ngcliLocalMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the list of Local Machine Information.\n"); 
	return 0;
    }

    /* Is the information already registered? */
    if (context->ngc_lmInfo != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Local Machine Information has already registered.\n"); 
	goto error;
    }

    /* Append at last of the list */
    context->ngc_lmInfo = lmInfoMng;

    /* Unlock the list */
    result = ngcliLocalMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Local Machine Information.\n"); 
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Local Machine Information.\n"); 
    }

    return 0;
}

/**
 * Unregister the Local Machine Information to Ninf-G Context.
 */
int
ngcliContextUnregisterLocalMachineInformation(
    ngclContext_t *context,
    ngcliLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngcliContextUnregisterLocalMachineInformation";

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

    result = ngcllContextUnregisterLocalMachineInformation(
	context, lmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterLocalMachineInformation(
    ngclContext_t *context,
    ngcliLocalMachineInformationManager_t *lmInfoMng,
    int *error)
{
    static const char fName[] = "ngcllContextUnregisterLocalMachineInformation";

    /* Is the information already registered? */
    if (context->ngc_lmInfo == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Local Machine Information has not registered.\n"); 
	goto error;
    }

    /* The information registered will be demanded? */
    if (context->ngc_lmInfo != lmInfoMng) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The Local Machine Information which registered is not demanded one.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Invoke Server Information.\n"); 
	return 0;
    }

    /* Is the Information already registered? */
    exist = ngcliInvokeServerInformationCacheGet(
	context, isInfoMng->ngisim_info.ngisi_type, error);
    if (exist != NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invoke Server Information of the name \"%s\" has already registered.\n",
            isInfoMng->ngisim_info.ngisi_type); 
	goto error;
    }

    /* Append at last of the list */
    isInfoMng->ngisim_next = NULL;
    if (context->ngc_invokeServerInfo_head == NULL) {
	/* No Information is registered */
	assert(context->ngc_invokeServerInfo_tail == NULL);
	context->ngc_invokeServerInfo_head = isInfoMng;
	context->ngc_invokeServerInfo_tail = isInfoMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_invokeServerInfo_tail != NULL);
	assert(context->ngc_invokeServerInfo_tail->ngisim_next == NULL);
	context->ngc_invokeServerInfo_tail->ngisim_next = isInfoMng;
	context->ngc_invokeServerInfo_tail = isInfoMng;
    }

    /* Unlock the list */
    result = ngcliInvokeServerInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Invoke Server Information.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Invoke Server Information.\n"); 
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
    prevPtr = &context->ngc_invokeServerInfo_head;
    curr = context->ngc_invokeServerInfo_head;
    for (; curr != NULL; curr = curr->ngisim_next) {
	if (curr == isInfoMng) {
	    /* Unlink the list */
	    *prevPtr = curr->ngisim_next;
	    if (curr->ngisim_next == NULL) {
	        context->ngc_invokeServerInfo_tail = prev;
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
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Invoke Server Information of the name \"%s\" is not registered.\n",
        isInfoMng->ngisim_info.ngisi_type); 

    return 0;
}


/**
 * Register the Communication Proxy Information to Ninf-G Context.
 */
int
ngcliContextRegisterCommunicationProxyInformation(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *cpInfoMng,
    int *error)
{
    int result;
    ngcliCommunicationProxyInformationManager_t *exist;
    static const char fName[] =
        "ngcliContextRegisterCommunicationProxyInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfoMng != NULL);

    /* Lock the list */
    result = ngcliCommunicationProxyInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Communication Proxy Information.\n"); 
	return 0;
    }

    /* Is the Information already registered? */
    exist = ngcliCommunicationProxyInformationCacheGet(
	context, cpInfoMng->ngcpim_info.ngcpi_type, error);
    if (exist != NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Communication Proxy Information of the name \"%s\" has already registered.\n",
            cpInfoMng->ngcpim_info.ngcpi_type); 
	goto error;
    }

    /* Append at last of the list */
    cpInfoMng->ngcpim_next = NULL;
    if (context->ngc_cpInfo_head == NULL) {
	/* No Information is registered */
	assert(context->ngc_cpInfo_tail == NULL);
	context->ngc_cpInfo_head = cpInfoMng;
	context->ngc_cpInfo_tail = cpInfoMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_cpInfo_tail != NULL);
	assert(context->ngc_cpInfo_tail->ngcpim_next == NULL);
	context->ngc_cpInfo_tail->ngcpim_next = cpInfoMng;
	context->ngc_cpInfo_tail = cpInfoMng;
    }

    /* Unlock the list */
    result = ngcliCommunicationProxyInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Communication Proxy Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliCommunicationProxyInformationListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Communication Proxy Information.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Unregister the Communication Proxy Information from Ninf-G Context.
 */
int
ngcliContextUnregisterCommunicationProxyInformation(
    ngclContext_t *context,
    ngcliCommunicationProxyInformationManager_t *cpInfoMng,
    int *error)
{
    ngcliCommunicationProxyInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcliContextUnregisterCommunicationProxyInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(cpInfoMng != NULL);

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_cpInfo_head;
    curr = context->ngc_cpInfo_head;
    for (; curr != NULL; curr = curr->ngcpim_next) {
	if (curr == cpInfoMng) {
	    /* Unlink the list */
	    *prevPtr = curr->ngcpim_next;
	    if (curr->ngcpim_next == NULL) {
	        context->ngc_cpInfo_tail = prev;
	    }
	    cpInfoMng->ngcpim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngcpim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Communication Proxy Information of the name \"%s\" is not registered.\n",
        cpInfoMng->ngcpim_info.ngcpi_type); 

    return 0;
}

/**
 * Register the Information Service Information to Ninf-G Context.
 */
int
ngcliContextRegisterInformationServiceInformation(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error)
{
    int result;
    ngcliInformationServiceInformationManager_t *exist;
    static const char fName[] = "ngcliContextRegisterInformationServiceInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Lock the list */
    result = ngcliInformationServiceInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Information Service Information.\n"); 
	return 0;
    }

    /* Is the Information already registered? */
    if (isInfoMng->ngisim_info.ngisi_tag != NULL) {
	exist = ngcliInformationServiceInformationCacheGet(
	    context, isInfoMng->ngisim_info.ngisi_tag, error);
	if (exist != NULL) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		"Information Service Information of the tag name \"%s\""
		" has already registered.\n",
		isInfoMng->ngisim_info.ngisi_tag); 
	    goto error;
	}
    }

    /* Append at last of the list */
    isInfoMng->ngisim_next = NULL;
    if (context->ngc_infoServiceInfo_head == NULL) {
	/* No Information is registered */
	assert(context->ngc_infoServiceInfo_tail == NULL);
	context->ngc_infoServiceInfo_head = isInfoMng;
	context->ngc_infoServiceInfo_tail = isInfoMng;
    } else {
	/* Any information is registered */
	assert(context->ngc_infoServiceInfo_tail != NULL);
	assert(context->ngc_infoServiceInfo_tail->ngisim_next == NULL);
	context->ngc_infoServiceInfo_tail->ngisim_next = isInfoMng;
	context->ngc_infoServiceInfo_tail = isInfoMng;
    }

    /* Unlock the list */
    result = ngcliInformationServiceInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Information Service Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliInformationServiceInformationListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Information Service Information.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Unregister the Information Service Information from Ninf-G Context.
 */
int
ngcliContextUnregisterInformationServiceInformation(
    ngclContext_t *context,
    ngcliInformationServiceInformationManager_t *isInfoMng,
    int *error)
{
    ngcliInformationServiceInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] =
	"ngcliContextUnregisterInformationServiceInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(isInfoMng != NULL);

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_infoServiceInfo_head;
    curr = context->ngc_infoServiceInfo_head;
    for (; curr != NULL; curr = curr->ngisim_next) {
	if (curr == isInfoMng) {
	    /* Unlink the list */
	    *prevPtr = curr->ngisim_next;
	    if (curr->ngisim_next == NULL) {
	        context->ngc_infoServiceInfo_tail = prev;
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
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Information Service Information of the tag name \"%s\""
	" is not registered.\n",
        isInfoMng->ngisim_info.ngisi_tag); 

    return 0;
}

/**
 * Register the Default Remote Information to Ninf-G Context.
 */
int
ngcliContextRegisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
	"ngcliContextRegisterDefaultRemoteMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    static const char fName[] =
	"ngcllContextRegisterDefaultRemoteMachineInformation";

    /* Lock the list */
    result = ngcliDefaultRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the list of Default Remote Information.\n"); 
	return 0;
    }

    /* Is the information already registered? */
    if (context->ngc_rmInfo_default != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Default Remote Information has already registered.\n"); 
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Default Remote Information.\n"); 
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Default Remote Information.\n"); 
    }

    return 0;
}

/**
 * Unregister the Default Remote Information to Ninf-G Context.
 */
int
ngcliContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
	"ngcliContextUnregisterDefaultRemoteMachineInformation";

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

    result = ngcllContextUnregisterDefaultRemoteMachineInformation(
	context, rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterDefaultRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    static const char fName[] =
	"ngcllContextUnregisterDefaultRemoteMachineInformation";

    /* Is the information already registered? */
    if (context->ngc_rmInfo_default == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Default Remote Information has not registered.\n"); 
	goto error;
    }

    /* The information registered will be demanded? */
    if (context->ngc_rmInfo_default != rmInfoMng) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The Default Remote Machine Information which registered is not demanded one.\n"); 
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
ngcliContextRegisterRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngcliContextRegisterRemoteMachineInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int result;
    char *hostName, *tagName;
    ngcliRemoteMachineInformationManager_t *exist;
    static const char fName[] = "ngcllContextRegisterRemoteMachineInformation";

    /* Lock the list */
    result = ngcliRemoteMachineInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the list of Remote Machine Information.\n"); 
	return 0;
    }

    hostName = rmInfoMng->ngrmim_info.ngrmi_hostName;
    tagName = rmInfoMng->ngrmim_info.ngrmi_tagName;

    /* Is the same host name already registered? */
    exist = ngcliRemoteMachineInformationCacheGetWithTag(
	context, hostName, tagName, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Remote Machine Information of the hostname \"%s\""
            " tag \"%s\" has already registered.\n",
            hostName, ((tagName != NULL) ? tagName : "")); 
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
	    (ngcliRemoteMachineInformationManager_t *)rmInfoMng;
	context->ngc_rmInfo_tail = rmInfoMng;
    }

    /* Unlock the list */
    result = ngcliRemoteMachineInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
    	return 0;
    }

    return 0;
}

/**
 * Unregister the Remote Machine Information to Ninf-G Context.
 */
int
ngcliContextUnregisterRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
        "ngcliContextUnregisterRemoteMachineInformation";

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

    result = ngcllContextUnregisterRemoteMachineInformation(
	context, rmInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterRemoteMachineInformation(
    ngclContext_t *context,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    int *error)
{
    ngcliRemoteMachineInformationManager_t **prevPtr, *prev, *curr;
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
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Machine Information of the hostname \"%s\" has not registered.\n",
        rmInfoMng->ngrmim_info.ngrmi_hostName); 

    return 0;
}

/**
 * Register the Job Management to Ninf-G Context.
 */
int
ngcliContextRegisterJobManager(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
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
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the list of Job Manager.\n"); 
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
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Job Manager.\n"); 
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
    ngcliJobManager_t *prev, *curr;
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the list of Job Manager.\n"); 
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Job Manager.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Job management is not found.\n"); 

    /* Unlock the list */
    result = ngcliContextJobManagerListWriteUnlock(context, log, NULL);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Job Manager.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is less than %d.\n", id, NGI_JOB_ID_MIN); 
	goto error;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_JOB_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is greater than %d.\n", id, NGI_JOB_ID_MAX); 
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
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Job Manager is not found by ID %d.\n", id); 

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
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Job Manager was reached.\n"); 

    return NULL;
}

/**
 * Register the Invoke Server Manager to Ninf-G Context.
 */
int
ngcliContextRegisterInvokeServerManager(
    ngclContext_t *context,
    ngcliInvokeServerManager_t *invokeMng,
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
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the list of Invoke Server Manager.\n"); 
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
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Invoke Server Manager.\n"); 
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
    ngcliInvokeServerManager_t *prev, *curr;
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the list of Invoke Server Manager.\n"); 
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
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Invoke Server Manager.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogWarnContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Invoke Server Manager is not found.\n"); 

    /* Unlock the list */
    result = ngcliContextInvokeServerManagerListWriteUnlock(
        context, log, NULL);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the list of Invoke Server Manager.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invoke Server type is NULL.\n"); 
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
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Invoke Server Manager is not found by type \"%s\" (%d%s).\n",
        type, count, ((count == -1) ? ":last" : "")); 

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
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Invoke Server Manager was reached.\n"); 

    return NULL;
}

/**
 * Register Executable Path Information.
 */
int
ngcliContextRegisterExecutablePathInformation(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    int result;
    ngcliExecutablePathInformationManager_t *exist;
    static const char fName[] = "ngcliContextRegisterExecutablePathInformation";

    assert(context   != NULL);
    assert(epInfoMng != NULL);

    /* Lock the list */
    result = ngcliExecutablePathInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the list of Executable Path Information.\n"); 
	return 0;
    }

    /* Is the same path name already registered? */
    exist = ngcliExecutablePathInformationCacheGet(context,
	epInfoMng->ngepim_info.ngepi_hostName,
	epInfoMng->ngepim_info.ngepi_className, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable Path Information of the classname \"%s\" on \"%s\" has already registered.\n",
            epInfoMng->ngepim_info.ngepi_className, epInfoMng->ngepim_info.ngepi_hostName); 
	goto error;
    }

    /* Append at last of the list */
    epInfoMng->ngepim_next = NULL;
    if (context->ngc_epInfo_head == NULL) {
    	/* Information is not registered */
	context->ngc_epInfo_head = epInfoMng;
	context->ngc_epInfo_tail = epInfoMng;
    } else {
    	/* Any information is registered */
	context->ngc_epInfo_tail->ngepim_next = epInfoMng;
	context->ngc_epInfo_tail = epInfoMng;
    }

    /* Unlock the list */
    result = ngcliExecutablePathInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Executable Path Information.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngcliExecutablePathInformationListWriteUnlock(context,
	context->ngc_log, NULL);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Executable Path Information.\n"); 
    }

    return 0;
}

/**
 * Unregister the Executable Path Information
 */
int
ngcliContextUnregisterExecutablePathInformation(
    ngclContext_t *context,
    ngcliExecutablePathInformationManager_t *epInfoMng,
    int *error)
{
    ngcliExecutablePathInformationManager_t **prevPtr, *prev, *curr;
    static const char fName[] = "ngcliContextUnregisterExecutablePathInformation";

    assert(context != NULL);
    assert(epInfoMng != NULL);

    /* Delete the data from the list */
    prev = NULL;
    prevPtr = &context->ngc_epInfo_head;
    curr = context->ngc_epInfo_head;
    for (; curr != NULL; curr = curr->ngepim_next) {
	if (curr == epInfoMng) {
            /* Found */
	    *prevPtr = curr->ngepim_next;
            if (curr->ngepim_next == NULL) {
                context->ngc_epInfo_tail = prev;
            }
	    epInfoMng->ngepim_next = NULL;

	    /* Success */
	    return 1;
	}
	/* set prev to current element */
	prev = curr;
	prevPtr = &curr->ngepim_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable Path Information of class \"%s\" on \"%s\" has not registered.\n",
        epInfoMng->ngepim_info.ngepi_className, epInfoMng->ngepim_info.ngepi_hostName); 

    return 0;
}

/**
 * Register the Remote Class Information to Ninf-G Context.
 */
int
ngcliContextRegisterRemoteClassInformation(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngcliContextRegisterRemoteClassInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is context valid? */
    if (context == NULL) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
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
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int result;
    ngcliRemoteClassInformationManager_t *exist;
    static const char fName[] = "ngcllContextRegisterRemoteClassInformation";

    /* Lock the list */
    result = ngcliRemoteClassInformationListWriteLock(context,
	context->ngc_log, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write lock the list of Remote Class Information.\n"); 
	return 0;
    }

    /* Is the same host name already registered? */
    exist = ngcliRemoteClassInformationCacheGet(
	context, rcInfoMng->ngrcim_info.ngrci_className, NULL);
    if (exist != NULL) {
	NGI_SET_ERROR(error, NG_ERROR_ALREADY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Remote Class Information of the hostname \"%s\" has already registered.\n",
            rcInfoMng->ngrcim_info.ngrci_className); 
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
	    (ngcliRemoteClassInformationManager_t *)rcInfoMng;
	context->ngc_rcInfo_tail = rcInfoMng;
    }

    /* Unlock the list */
    result = ngcliRemoteClassInformationListWriteUnlock(context,
	context->ngc_log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_UNLOCK);
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
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
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't write unlock the list of Remote Machine Information.\n"); 
    	return 0;
    }

    return 0;
}

/**
 * Unregister the Remote Class Information to Ninf-G Context.
 */
int
ngcliContextUnregisterRemoteClassInformation(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    int local_error, result;
    static const char fName[] =
        "ngcliContextUnregisterRemoteClassInformation";

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

    result = ngcllContextUnregisterRemoteClassInformation(context,
	rcInfoMng, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllContextUnregisterRemoteClassInformation(
    ngclContext_t *context,
    ngcliRemoteClassInformationManager_t *rcInfoMng,
    int *error)
{
    ngcliRemoteClassInformationManager_t **prevPtr, *prev, *curr;
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
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Class Information of the hostname \"%s\" has not registered.\n",
        rcInfoMng->ngrcim_info.ngrci_className); 

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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Write status */
    context->ngc_flagExecutable = 1;

    /* Signal */
    result = ngiCondSignal(
    	&context->ngc_condExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
    	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	goto error;
    }
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    while (context->ngc_flagExecutable == 0) {
	/* Wait */
	result = ngiCondWait(
    	    &context->ngc_condExecutable, &context->ngc_mutexExecutable,
	    context->ngc_log, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't wait the Condition Variable.\n"); 
	    goto error;
	}
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexExecutable, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return NULL;
    }

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	goto error;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
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
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get the Session by ID %d.\n", sessionID[i]); 
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
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
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
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	goto error;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
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
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get the Session by ID %d.\n", sessionID[i]); 
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
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
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
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	goto error;
    }

    /* Lock the list of Session */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Session.\n"); 
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
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Context is not valid.\n"); 
	return NULL;
    }

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return NULL;
    }

    /* Rotate the list of Executable */
    result = ngcliExecutableListRotate(context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't rotate the list of Executable.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	return NULL;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
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
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Session.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Write status */
    context->ngc_flagSession = 1;

    /* Signal */
    result = ngiCondBroadcast(
    	&context->ngc_condSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
    	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(
    	&context->ngc_mutexSession, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	goto error;
    }
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
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
	    context, &wait, &done, NGCLL_WAIT_ALL, log, error);
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
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't unlock the Mutex.\n"); 
		retResult = 0;
		error = NULL;
		goto error;
	    }

	    /* Perform the next session if any */
	    result = ngcllContextExecuteSession(context, done, log, error);
	    if (result == 0) {
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't perform the next Session.\n"); 
		retResult = 0;
		error = NULL;
		/* Not returned */
	    }

	    /* Lock */
	    result = ngiMutexLock(&context->ngc_mutexSession, log, error);
	    if (result == 0) {
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't lock the Mutex.\n"); 
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
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't wait the Condition Variable.\n"); 
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
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the Mutex.\n"); 
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
		    ngclLogInfoSession(curr, NG_LOGCAT_NINFG_PURE, fName,  
		        "returning with error code %d.\n", curr->ngs_error); 
		    /* Not returned */
		}

		/* Perform the next Session if any. */
		result = ngcliExecutableExecuteSession(
		    curr->ngs_executable, log, error);
		if (result == 0) {
		    ngclLogInfoExecutable( curr->ngs_executable, NG_LOGCAT_NINFG_PURE, fName,  
		        "Can't perform the next Session.\n"); 
		    /* Not returned */
		}
		goto end;
	    }
	}

	/* Wait */
	result = ngcllContextCondWaitSession(context, log, error);
	if (result == 0) {
	    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't wait the Session.\n"); 
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
    ngcllWaitMode_t waitMode,
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
	    ngclLogInfoSession(curr, NG_LOGCAT_NINFG_PURE, fName,  
	        "returning with error code %d.\n", curr->ngs_error); 
	    break;
	}

	if (waitMode == NGCLL_WAIT_ANY)
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
	    ngclLogInfoExecutable(list->ngs_executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't perform the next Session.\n"); 
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
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
	result = 0;
	error = NULL;
	goto error;
    }
    sessionLocked = 1;

    /* Wait */
    result = ngiCondWait(
	&context->ngc_condSession, &context->ngc_mutexSession, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't wait the Condition Variable.\n"); 
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
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the Mutex.\n"); 
	    result = 0;
	    error = NULL;
	}
    }

    /* Success */
    return result;
}

/**
 * Get the random number.
 */
int
ngcliContextRandomNumberGet(
    ngclContext_t *context,
    long *randomNo,
    int *error)
{
    ngLog_t *log;
    int result, locked;
    static const char fName[] = "ngcliContextRandomNumberGet";

    /* Check the arguments */
    assert(context != NULL);
    assert(randomNo != NULL);

    log = context->ngc_log;
    locked = 0;

    /* Lock this instance */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	goto error;
    }
    locked = 1;

    result = ngiRandomNumberGetLong(
        &context->ngc_randomStatus, randomNo, log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Ninf-G Context.\n"); 
	goto error;
    }

    /* Unlock this instance */
    locked = 0;
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Ninf-G Context.\n"); 
	goto error;
    }


    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock this instance */
    if (locked == 0) {
	locked = 0;
	result = ngclContextWriteUnlock(context, NULL);
	if (result == 0) {
	    ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Ninf-G Context.\n"); 
        }
    }

    /* Failed */
    return 0;
}

