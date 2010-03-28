/*
 * $RCSfile: ngclExecutable.c,v $ $Revision: 1.25 $ $Date: 2008/01/28 06:58:00 $
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
 * Module of Executable for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclExecutable.c,v $ $Revision: 1.25 $ $Date: 2008/01/28 06:58:00 $")

/**
 * Prototype declaration of static functions.
 */
static ngclExecutable_t * ngcllExecutableAllocate(ngclContext_t *, int, int *);
static int ngcllExecutableFree(
    ngclExecutable_t *, ngLog_t *, int *);
static ngclExecutable_t *ngcllExecutableConstruct(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableDestruct(ngclExecutable_t *, int, int *);
static ngclExecutable_t *ngcllExecutableConstructInternal(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableDestructInternal(ngclExecutable_t *, int, int *);
static int ngcllExecutableInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableFinalize(ngclExecutable_t *, int *, int *);
static int ngcllExecutableInitializeExecutable(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableIsExecutableTypeMPI(
    ngclContext_t *, ngclExecutableAttribute_t *, int *, int *);
static int ngcllExecutableGetRemoteInformation(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *,
    ngLog_t *, int *);
static int ngcllExecutableFindRemoteInformation(
    ngclContext_t *, char *, char *,
    ngcliRemoteMachineInformationManager_t **, int *,
    ngclExecutablePathInformation_t *, ngLog_t *, int *);
static int
ngcllExecutableGetRemoteClassInformation(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *,
    ngcliRemoteMachineInformationManager_t *, ngLog_t *, int *);
static int ngcllExecutableRemoteClassInformationCacheGet(
    ngclContext_t *, ngclExecutable_t *, char *, int, int *, ngLog_t *, int *);
static int ngcllExecutableCopyMemberFromRemoteMachineInformation(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *,
    ngcliRemoteMachineInformationManager_t *, ngLog_t *, int *);
static int ngcllExecutableCopyMemberFromExecutablePathInformation(
    ngclContext_t *, ngclExecutable_t *, ngclExecutableAttribute_t *,
    ngcliRemoteMachineInformationManager_t *, ngLog_t *, int *);
static int ngcllExecutableExit(ngclExecutable_t *, ngLog_t *, int *);
static void ngcllExecutableInitializeMember(ngclExecutable_t *);
static void ngcllExecutableInitializeFlag(ngclExecutable_t *);
static void ngcllExecutableInitializePointer(ngclExecutable_t *);
static int ngcllExecutableInitializeMutex(
    ngclExecutable_t *, ngLog_t *, int *);
static int ngcllExecutableFinalizeMutex(ngclExecutable_t *, ngLog_t *, int *);
static int ngcllExecutableAttributeInitialize( 
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
static void ngcllExecutableAttributeInitializeMember(
    ngclExecutableAttribute_t *);
static void ngcllExecutableAttributeInitializePointer(
    ngclExecutableAttribute_t *);
static int ngcllExecutableAttributeFinalize(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableCheckAttribute(
    ngclContext_t *, ngclExecutableAttribute_t *, int *);
static int ngcllExecutableCreateID(ngclContext_t *, int *);
static void ngcllExecutableUserListInitializeMember(ngclExecutableList_t *);
static void ngcllExecutableUserListInitializePointer(ngclExecutableList_t *);
static int ngcllExecutableRegister(
    ngclContext_t *, ngclExecutable_t *, int, int *);
static int ngcllExecutableUnregisterFromDestructionList(
    ngclExecutable_t *, int *);
static int ngcllExecutableRegisterUserData(
    ngclExecutable_t *, void *, void (*)(ngclExecutable_t *), int *);
static int ngcllExecutableUnregisterUserData(ngclExecutable_t *, int *);
static int ngcllExecutableGetUserData(ngclExecutable_t *, void **, int *);
static int ngcllExecutableStatusCheckTransition(
    ngclExecutable_t *, ngclExecutableStatus_t, int *);
static int ngcllExecutableUserListInitialize( 
    ngclContext_t *, ngclExecutableList_t *, int *);
static int ngcllExecutableUserListFinalize(
    ngclContext_t *, ngclExecutableList_t *, int *);
static int ngcllExecutableUserListRegister(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
static int ngcllExecutableUserListUnregister(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
static ngclExecutable_t *ngcllExecutableUserListGetNext(
    ngclContext_t *, ngclExecutableList_t *, ngclExecutable_t *, int *);
static ngclExecutable_t *ngcllExecutableMultiHandleListGetNext(
    ngclContext_t *, ngclExecutable_t *, int *);
static int ngcllExecutableCopy(
    ngclContext_t *, ngclExecutable_t *, ngclExecutable_t *, int *); 
static ngclExecutable_t * ngcllExecutableGet(ngclContext_t *, int, int *);
static ngclExecutable_t * ngcllExecutableGetNext(
    ngclContext_t *, ngclExecutable_t *, int *);
static ngclExecutable_t *ngcllExecutableListAndDestructionListGetNext(
    ngclContext_t *, ngclExecutable_t *, int *);
static ngclSession_t * ngcllExecutableGetSession(
    ngclExecutable_t *, int, int *);
static int ngcllExecutableGetInformation(
    ngclExecutable_t *, ngclExecutableInformation_t *, int *);
static int ngcllExecutableReset(ngclExecutable_t *, int *);
static char * ngcllExecutableStatusToString(ngclExecutableStatus_t);

/**
 * Construct
 */
ngclExecutable_t *
ngclExecutableConstruct(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int local_error, result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngclExecutableConstruct";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    executable = ngcllExecutableConstruct(context, execAttr, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return executable;
}

/**
 * Destruct
 */
int
ngclExecutableDestruct(
    ngclExecutable_t *executable,
    int sessionRemains,
    int *error)
{
    int local_error, result;
    ngclContext_t *context;
    static const char fName[] = "ngclExecutableDestruct";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable is not valid.\n"); 
        return 0;
    }

    context = executable->nge_context;

    result = ngcllExecutableDestruct(executable, sessionRemains, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

/**
 * Construct
 */
static ngclExecutable_t *
ngcllExecutableConstruct(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int result, required, doMultipleConstruct, nHandles, i;
    ngclExecutable_t *executable, *tmpExecutable;
    ngclExecutable_t *prevExecutable, *nextExecutable;
    static const char fName[] = "ngcllExecutableConstruct";

    /* Initialize the local variables */
    doMultipleConstruct = 0;
    tmpExecutable = NULL;

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Are value of attributes valid? */
    result = ngcllExecutableCheckAttribute(context, execAttr, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid attribute of Executable.\n"); 
	return NULL;
    }

    if (execAttr->ngea_invokeNjobs <= 1) {
        doMultipleConstruct = 0;
    } else {
        result = ngcllExecutableIsExecutableTypeMPI(
            context, execAttr, &required, error);
        if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get whether job is MPI or not.\n"); 
            return NULL;
        }
        if (required != 0) {
            doMultipleConstruct = 1;
        }
    }

    if (doMultipleConstruct == 0) {
        /* Construct the Executable */
        executable = ngcllExecutableConstructInternal(
            context, execAttr, error);
        if (executable == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Executable.\n"); 
            goto error;
        }
    } else {
        assert(execAttr->ngea_invokeNjobs > 1);
        
        nHandles = execAttr->ngea_invokeNjobs;
        execAttr->ngea_invokeNjobs = 1;

        executable = NULL; /* for head */
        prevExecutable = NULL;
        for (i = 0; i < nHandles; i++) {
            /* Construct the Executable */
            tmpExecutable = ngcllExecutableConstructInternal(
                context, execAttr, error);
            if (tmpExecutable == NULL) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't construct the Executable.\n"); 
                goto error;
            }

            if (executable == NULL) {
                executable = tmpExecutable;
            }

            /* Make the list */
            if (prevExecutable != NULL) {
                prevExecutable->nge_multiHandleNext = tmpExecutable;
            }
            
            prevExecutable = tmpExecutable;
        }
        tmpExecutable->nge_multiHandleNext = NULL;

        execAttr->ngea_invokeNjobs = nHandles;
    }

    /* Success */
    return executable;

    /* Error occurred */
error:
    /* Destruct the created Executable */
    if (executable != NULL) {
        tmpExecutable = executable;
        while (tmpExecutable != NULL) {
            nextExecutable = tmpExecutable->nge_multiHandleNext;

            result = ngcllExecutableDestructInternal(
                tmpExecutable, 0, NULL);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't destruct the Executables.\n"); 
            }

            tmpExecutable = nextExecutable;
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Destruct
 */
static int
ngcllExecutableDestruct(
    ngclExecutable_t *executable,
    int sessionRemains,
    int *error)
{
    ngclContext_t *context;
    int result;
    static const char fName[] = "ngcllExecutableDestruct";

    /* Check the arguments */
    assert(executable != NULL);

    context = executable->nge_context;

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    result = ngcllExecutableDestructInternal(
	executable, sessionRemains, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the Executables.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Construct
 */
static ngclExecutable_t *
ngcllExecutableConstructInternal(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int i;
    int result;
    ngLog_t *log;
    int initialized = 0;
    int registered = 0;
    int jobStartTimeoutStarted = 0;
    int isLastExecutable = 0;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllExecutableConstructInternal";

    /* Check the arguments */
    assert(context != NULL);
    assert(execAttr != NULL);

    log = context->ngc_log;

    /* Allocate */
    executable = ngcllExecutableAllocate(
	context, execAttr->ngea_invokeNjobs, error);
    if (executable == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Executable.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllExecutableInitialize(context, executable, execAttr, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Executable.\n"); 
	goto error;
    }
    initialized = 1;

    /* Register */
    result = ngclExecutableRegister(
	context, executable, execAttr->ngea_invokeNjobs, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't register the Executable.\n"); 
	goto error;
    }
    registered = 1;

    /* Notify to HeartBeat */
    result = ngcliHeartBeatIntervalChange(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't notify interval change for heartbeat.\n"); 
	goto error;
    }

    /* Job Start Timeout Start */

    result = ngcliJobStartTimeoutJobStart(
        context, executable->nge_jobMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Job Start Timeout.\n"); 
	goto error;
    }
    jobStartTimeoutStarted = 1;

    /* Start the Job */
    result = ngcliJobStart(executable->nge_jobMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Job.\n"); 
	goto error;
    }

    /* Success */
    return executable;

    /* Error occurred */
error:
    /* Job Start Timeout Stop */
    if (jobStartTimeoutStarted != 0) {
	for (i = 0; i < execAttr->ngea_invokeNjobs; i++) {
            result = ngcliJobStartTimeoutJobStarted(context, &executable[i], NULL);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't stop the Job Start Timeout.\n"); 
            }
        }
    }

    /* Unregister */
    if (registered != 0) {
        registered = 0;
	for (i = 0; i < execAttr->ngea_invokeNjobs; i++) {
	    result = ngclExecutableUnregister(&executable[i], NULL);
	    if (result == 0) {
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't unregister the Executable.\n"); 
	    }
	}
    }

    /* Finalize */
    if (initialized != 0) {
        initialized = 0;
	for (i = 0; i < execAttr->ngea_invokeNjobs; i++) {
	    result = ngcllExecutableFinalize(&executable[i], &isLastExecutable, 
                    NULL);
	    if (result == 0) {
		ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't finalize the Executable.\n"); 
	    }
	}
    }

    result = ngcllExecutableFree(executable, log, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the storage for Executable.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
static int
ngcllExecutableDestructInternal(
    ngclExecutable_t *executable,
    int sessionRemains,
    int *error)
{
    int result;
    int retResult = 1;
    int isLastExecutable = 0;
    ngclContext_t *context;
    ngcliJobManager_t *jobMng;
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableDestructInternal";

    /* Check the arguments */
    assert(executable != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;
    jobMng = executable->nge_jobMng;
    log = context->ngc_log;

    /* Has Executable done? */
    if (executable->nge_status == NG_EXECUTABLE_STATUS_DONE) {
	; /* Do nothing */
    }

    /* Has Executable exiting? */
    else if (executable->nge_status >= NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
	retResult = 0;
	error = NULL;
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid state: Current status is %d.\n", executable->nge_status); 
    }

    /* Has Executable connected? */
    else if (executable->nge_status < NG_EXECUTABLE_STATUS_CONNECTED) {
	ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not connected.\n"); 

	goto unusable;
    }

    /* grpc_wait*() was not called? */
    else if (executable->nge_session_head != NULL) {
	ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Some sessions are still remaining on this Executable.\n"); 

unusable:
	/* drop sessions and cancel the job */
	result = ngcliExecutableUnusable(
            executable, NG_ERROR_INVALID_STATE, error);
        if (result == 0) {
	    retResult = 0;
	    error = NULL;
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Executable unusable.\n"); 
        }

        ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "made the Executable unusable.\n"); 

	/* Wait until the number of Sessions is set to 0 */
	result = ngcliExecutableNsessionsWaitUntilZero(executable, log, error);
	if (result == 0) {
	    retResult = 0;
	    error = NULL;
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't wait until the number of Sessions is set to 0.\n"); 
	}
    }

    else if (sessionRemains != 0) {
	goto jobRequestCancel;
    }

    else {
	/* Exit the Remote Executable */
	result = ngcllExecutableExit(executable, log, error);
	if ((result != 0) &&
	    (executable->nge_requestResult != NGI_PROTOCOL_RESULT_OK)) {
	    goto jobRequestCancel;
	} else if (result == 0) {
	    retResult = 0;
	    error = NULL;
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't exit the Remote Executable.\n"); 

jobRequestCancel:
	    /* Request job cancel */
	    result = ngcliJobRequestCancel(jobMng, log, error);
	    if (result == 0) {
		retResult = 0;
		error = NULL;
		ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
		    "Can't request the cancel.\n"); 
	    }
	}
    }

    /* Unregister */
    result = ngclExecutableUnregister(executable, error);
    if (result == 0) {
	retResult = 0;
	error = NULL;
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executable.\n"); 
    }

    /* Finalize */
    result = ngcllExecutableFinalize(executable, &isLastExecutable, error);
    if (result == 0) {
	retResult = 0;
	error = NULL;
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Executable.\n"); 
    }

    /* Deallocate */
    if (isLastExecutable != 0) {
        result = ngcllExecutableFree(executable, log, error);
        if (result == 0) {
            retResult = 0;
            error = NULL;
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't deallocate the storage for Executable.\n"); 
        }
    }

    /* Success */
    return retResult;
}

/**
 * Exit the Remote Executable.
 */
static int
ngcllExecutableExit(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutableExit";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);

    /* Print the information */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Exit the Executable.\n"); 

    /* Lock the Executable with send */
    result = ngcliExecutableLockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        return 0;
    }

    /* Unlock the Executable */
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        return 0;
    }

    /* Send the request of Exit Executable */
    result = ngcliProtocolRequestExitExecutable(
    	executable, executable->nge_protocol, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't send the EXIT.\n"); 
        return 0;
    }

    /* Wait the Reply */
    result = ngcliExecutableStatusWaitExited(executable, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't wait the EXIT reply.\n"); 
        return 0;
    }

    /* Set the status */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_DONE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Wait the Executable lock free, to destroy the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        return 0;
    }

    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
static ngclExecutable_t *
ngcllExecutableAllocate(ngclContext_t *context, int nExecutables, int *error)
{
    int result;
    ngLog_t *log;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllExecutableAllocate";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (nExecutables <= 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "The number of Executables (%d) is smaller equal zero.\n",
	    nExecutables); 
	return NULL;
    }

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return NULL;
    }

    /* Allocate */
    executable = ngiCalloc(
	nExecutables, sizeof (ngclExecutable_t), log, error);
    if (executable == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Executable.\n"); 
	return NULL;
    }

    /* Success */
    return executable;
}

/**
 * Deallocate
 */
static int
ngcllExecutableFree(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllExecutableFree";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (executable == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is NULL.\n"); 
	return 0;
    }

    /* Is top NULL? */
    if (executable->nge_top == NULL) {
	/* Deallocate */
	ngiFree(executable->nge_top, log, error);

	/* Success */
	return 1;
    }

    /* Deallocate */
    ngiFree(executable->nge_top, log, error);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngclExecutableInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllExecutableInitialize(context, executable, execAttr, error);
}

static int
ngcllExecutableInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int result;
    int i;
    ngcliJobManager_t *jobMng;
    ngclExecutableAttribute_t tmpAttr;
    ngcliJobAttribute_t jobAttr;
    static const char fName[] = "ngcllExecutableInitialize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Is Executable valid? */
    if (executable == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is NULL.\n"); 
	return 0;
    }

    for (i = 0; i < execAttr->ngea_invokeNjobs; i++) {
	/* Set the pointer of top element */
	executable[i].nge_top = &executable[0];

	/* Initialize */
	result = ngcllExecutableInitializeExecutable(
	    context, &executable[i], execAttr, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't initialize the Executable.\n"); 
	    goto error;
	}
    }

    /* Set the pointers of multiHandleNext */
    for (i = 0; i < execAttr->ngea_invokeNjobs - 1; i++) {
        executable[i].nge_multiHandleNext = &executable[i + 1];
    }
    executable[i].nge_multiHandleNext = NULL;

    /* Make the attribute for the Job */
    tmpAttr = *execAttr;
    if (executable->nge_tagName != NULL) {
	tmpAttr.ngea_hostName = executable->nge_tagName;
    } else {
	tmpAttr.ngea_hostName = executable->nge_hostName;
    }
    result = ngcliJobAttributeInitialize(context, &tmpAttr, &jobAttr, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't make the Attribute of Job.\n"); 
	goto error;
    }

    /* Construct the Job Manager */
    jobMng = ngcliJobConstruct(context, executable, &jobAttr, error);
    if (jobMng == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't construct the Job Manager.\n"); 
	goto error;
    }

    /* Release the attribute for the Job */
    result = ngcliJobAttributeFinalize(context, &jobAttr, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't release the Attribute of Job.\n"); 
	goto error;
    }

    /* Register to the Job Manager */
    result = ngcliJobRegisterExecutable(jobMng, executable,
        execAttr->ngea_invokeNjobs, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't register the Executable into the Job Manager.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    return 0;
}

/**
 * Initialize
 */
static int
ngcllExecutableInitializeExecutable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    char host[NGI_HOST_NAME_MAX];
    ngLogConstructArgument_t logCarg;
    ngCommLogPairInfo_t pairInfo;
    int commLogInfoInitialized = 0;
    int logCargInitialized = 0;
    static const char fName[] = "ngcllExecutableInitializeExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(execAttr != NULL);

    /* Initialize the local variables */
    log = context->ngc_log;

    /* Initialize the members */
    ngcllExecutableInitializeMember(executable);
    executable->nge_context = context;
    executable->nge_waitArgumentTransfer = execAttr->ngea_waitArgumentTransfer;

    /* Initialize the mutex, cond and Read/Write Lock */
    result = ngcllExecutableInitializeMutex(executable, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the mutex, cond and Read/Write Lock.\n"); 
        goto error;
    }

    /* Initialize the I/O Callback Waiter */
    result = ngiIOhandleCallbackWaiterInitialize(
        &executable->nge_ioCallbackWaiter,
        context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the I/O callback waiter.\n"); 
        goto error;
    }

    /* Create the identifier of Executable */
    result = ngcllExecutableCreateID(context, error);
    if (result < 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't assign the ID to Executable.\n"); 
	goto error;
    }
    executable->nge_ID = result;

    result = ngiLogInformationInitialize(
        &executable->nge_commLogInfo, log ,error);
    if (result < 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the communication log information.\n"); 
	goto error;
    }
    commLogInfoInitialized = 1;

    /* Get the Remote Information */
    result = ngcllExecutableGetRemoteInformation(
        context, executable, execAttr, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Remote Information.\n"); 
	goto error;
    }

    /* Construct the Communication Log */
    if (executable->nge_commLogEnable != 0) {

        result = ngLogConstructArgumentInitialize(&logCarg, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize log construct argument.\n"); 
            goto error;
        }
        logCargInitialized = 1;

        if (executable->nge_commLogInfo.ngli_filePath == NULL) {
            logCarg.nglca_output       = NG_LOG_STDERR;
        } else {
            logCarg.nglca_output       = NG_LOG_FILE;
            logCarg.nglca_nFiles       = executable->nge_commLogInfo.ngli_nFiles;
            logCarg.nglca_maxFileSize  = executable->nge_commLogInfo.ngli_maxFileSize;
            logCarg.nglca_overWriteDir = executable->nge_commLogInfo.ngli_overWriteDir;
            logCarg.nglca_appending    = 0;/* false */

            logCarg.nglca_filePath = ngiStrdup(executable->nge_commLogInfo.ngli_filePath, log, error);
            if (logCarg.nglca_filePath == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, "Can't copy string.\n");
                goto error;
            }

            if (executable->nge_commLogInfo.ngli_suffix != NULL) {
                logCarg.nglca_suffix = ngiStrdup(executable->nge_commLogInfo.ngli_suffix, log, error);
                if (logCarg.nglca_suffix == NULL) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't copy string.\n");
                    goto error;
                }
            }
        }

        /* Create the message */
        result = ngiHostnameGet(&host[0], sizeof (host), log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the host name.\n"); 
            return 0;
        }
        pairInfo.ngcp_localAppName   = "Client";
        pairInfo.ngcp_localHostname  = host;
        pairInfo.ngcp_remoteAppName  = "Executable";
        pairInfo.ngcp_remoteHostname = executable->nge_hostName;

	/* Construct the Communication Log */
        executable->nge_commLog = ngCommLogConstructForExecutable(&pairInfo,
            executable->nge_ID, &logCarg, log, error);
        if (executable->nge_commLog == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't create the Communication Log.\n"); 
            goto error;
        }

        result = ngLogConstructArgumentFinalize(&logCarg, log, error);
        logCargInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the host name.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (logCargInitialized != 0) {
        result = ngLogConstructArgumentFinalize(&logCarg, log, error);
        logCargInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the host name.\n"); 
        }
    }

    /* Destruct the Communication Log */
    if (executable->nge_commLog != NULL) {
        result = ngCommLogDestruct(executable->nge_commLog, log, NULL);
        executable->nge_commLog = NULL;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't destruct the Communication Log.\n"); 
        }
    }

    if (commLogInfoInitialized != 0) {
        commLogInfoInitialized = 0;
        result = ngiLogInformationFinalize(
           &executable->nge_commLogInfo, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't release the Communication Log Information.\n"); 
        }
    }

    /* Release the Remote Class Information */
    if (executable->nge_rcInfoExist != 0) {
        executable->nge_rcInfoExist = 0;
	result = ngclRemoteClassInformationRelease(
	    context, &executable->nge_rcInfo, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Remote Class Information.\n"); 
	}
    }

    /* Finalize the Read/Write Lock */
    result = ngcllExecutableFinalizeMutex(executable, log, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the mutex, cond, and Read/Write Lock.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Check whether the MPI is required or not
 */
static int
ngcllExecutableIsExecutableTypeMPI(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *required,
    int *error)
{
    ngLog_t *log;
    char *hostName;
    int result, subError;
    ngBackend_t backend;
    int rmInfoLocked, rcInfoLocked;
    ngclExecutablePathInformation_t epInfo;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllExecutableIsExecutableTypeMPI";

    /* Check the arguments */
    assert(context != NULL);
    assert(execAttr != NULL);
    assert(required != NULL);

    /* Initialize the local variables */
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);
    rmInfoLocked = 0;
    rcInfoLocked = 0;
    backend = NG_BACKEND_NORMAL;
    log = context->ngc_log;

    *required = 0;
    hostName = execAttr->ngea_hostName;

    /* Find the Remote Information */
    result = ngcllExecutableFindRemoteInformation(
        context, hostName, execAttr->ngea_className,
        &rmInfoMng, &rmInfoLocked, &epInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't find the Remote Information "
            "for host \"%s\" class \"%s\".\n",
            ((hostName != NULL) ? hostName : "NULL"),
            execAttr->ngea_className);
        goto error;
    }
    assert(rmInfoMng != NULL);
    assert(rmInfoLocked != 0);

    /* Check if MPI is set in ExecutablePathInformation */
    backend = epInfo.ngepi_backend;
    if((backend == NG_BACKEND_MPI) || (backend == NG_BACKEND_BLACS)) {
        *required = 1;
    }

    /* Release */
    result = ngclExecutablePathInformationRelease(
        context, &epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Information.\n"); 
        goto error;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
        context, log, error);
    rmInfoLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Remote Machine Information.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if (rmInfoLocked != 0) {
        rmInfoLocked = 0;
        result = ngcliRemoteMachineInformationListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Machine Information.\n"); 
        }
    }

    /* Unlock the Remote Class Information */
    if (rcInfoLocked != 0) {
        rcInfoLocked = 0;
        result = ngcliRemoteClassInformationListReadUnlock(
            context, context->ngc_log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Class Information.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the information of Remote Machine and Remote Class
 */
static int
ngcllExecutableGetRemoteInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    ngLog_t *log,
    int *error)
{
    int result;
    int rmInfoLocked = 0;
    ngcliRemoteMachineInformationManager_t *rmInfoMng;
    static const char fName[] = "ngcllExecutableGetRemoteInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(execAttr != NULL);

    /* Set the start time */
    result = ngiSetStartTime(
	&executable->nge_executionTime.nge_queryRemoteMachineInformation, 
	log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the Start time.\n"); 
	goto error;
    }

    /* Find the Remote Information */
    result = ngcllExecutableFindRemoteInformation(
        context, execAttr->ngea_hostName, execAttr->ngea_className,
        &rmInfoMng, &rmInfoLocked, NULL, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't find the Remote Information "
            "for host \"%s\" class \"%s\".\n",
            execAttr->ngea_hostName, execAttr->ngea_className); 
	goto error;
    }
    assert(rmInfoMng != NULL);
    assert(rmInfoLocked != 0);

    /* Copy the hostname and necessary members */
    result = ngcllExecutableCopyMemberFromRemoteMachineInformation(
	context, executable, execAttr, rmInfoMng, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't copy the Remote Machine Information member.\n"); 
	goto error;
    }

    /* Copy the necessary members from Executable Path Information */
    result = ngcllExecutableCopyMemberFromExecutablePathInformation(
	context, executable, execAttr, rmInfoMng, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't copy the Executable Path Information member.\n"); 
	goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
	&executable->nge_executionTime.nge_queryRemoteMachineInformation, 
	log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the End time.\n"); 
	goto error;
    }

    /* Get the Remote Class Information */
    result = ngcllExecutableGetRemoteClassInformation(
	context, executable, execAttr, rmInfoMng, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't get the Remote Class Information.\n"); 
	goto error;
    }

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
	context, log, error);
    rmInfoLocked = 0;
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Remote Machine Information.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if (rmInfoLocked != 0) {
	rmInfoLocked = 0;
	result = ngcliRemoteMachineInformationListReadUnlock(
	    context, log, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Remote Machine Information.\n"); 
	}
    }

    /* Failed */
    return 0;
}

/**
 * Find the information of Remote Machine and Executable Path by Class Name.
 */
static int
ngcllExecutableFindRemoteInformation(
    ngclContext_t *context,
    char *hostNameTagName,
    char *className,
    ngcliRemoteMachineInformationManager_t **rmInfoMng,
    int *rmInfoLocked,
    ngclExecutablePathInformation_t *epInfoArg,
    ngLog_t *log,
    int *error)
{
    char *hostName;
    int result, subError;
    ngclExecutablePathInformation_t epInfoEntity, *epInfo;
    static const char fName[] = "ngcllExecutableFindRemoteInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(className != NULL);
    assert(rmInfoMng != NULL);
    assert(rmInfoLocked != NULL);

    /* Initialize the local variables */
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);
    *rmInfoMng = NULL;
    *rmInfoLocked = 0;
    hostName = NULL;

    epInfo = ((epInfoArg != NULL) ? epInfoArg : &epInfoEntity);
    assert(epInfo != NULL);

    /* Lock the Remote Machine Information List */
    result = ngcliRemoteMachineInformationListReadLock(context, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Remote Machine Information.\n"); 
	goto error;
    }
    *rmInfoLocked = 1;

    if (hostNameTagName != NULL) {
        /* Get the Remote Machine Information */
        *rmInfoMng = ngcliRemoteMachineInformationCacheGet(
            context, hostNameTagName, error);
        if (*rmInfoMng == NULL) {
            *rmInfoMng = ngcliDefaultRemoteMachineInformationCacheGet(
                context, error);
        }
        if (*rmInfoMng == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the Remote Machine Information "
                "for host \"%s\".\n", hostNameTagName); 
        }

        if (epInfoArg != NULL) {
            hostName = (*rmInfoMng)->ngrmim_info.ngrmi_hostName;

            /* Get the Executable Path Information */
            result = ngcliExecutablePathInformationGetCopyWithQuery(
                context, hostName, className, 
                (*rmInfoMng)->ngrmim_info.ngrmi_infoServiceTag,
                epInfo, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get the Executable Path Information "
                    "for host \"%s\" class \"%s\".\n", hostName, className); 
                goto error;
            }
        }

    } else {
        /* Get the top of list */
        *rmInfoMng = ngcliRemoteMachineInformationCacheGetNext(
            context, NULL, NULL);

        while (*rmInfoMng != NULL) {
            hostName = (*rmInfoMng)->ngrmim_info.ngrmi_hostName;

            /* Get the Executable Path Information */
            result = ngcliExecutablePathInformationGetCopyWithQuery(
                context, hostName, className, 
                (*rmInfoMng)->ngrmim_info.ngrmi_infoServiceTag,
                epInfo, &subError);
            if (result != 0) {
                ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Remote Class Information \"%s\" was found on "
                    "host \"%s\".\n", className, hostName);
                if (epInfoArg == NULL) {
                    /* Release */
                    result = ngclExecutablePathInformationRelease(
                        context, epInfo, error);
                    if (result == 0) {
                        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                            "Can't release the "
                            "ExecutablePathInformation.\n"); 
                       goto error;
                   }
                }

                /* Success */
                return 1;
            }
     
            /* Get the next */
            *rmInfoMng = ngcliRemoteMachineInformationCacheGetNext(
                context, *rmInfoMng, NULL);
        }
     
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "%s: Can't find the remote class.\n", className); 
        goto error;
    }

    /**
     * Not unlock RemoteMachineInformation list
     * Because caller uses found rmInfoMng.
     * caller will unlock.
     */

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Machine Information */
    if (*rmInfoLocked != 0) {
	*rmInfoLocked = 0;
	result = ngcliRemoteMachineInformationListReadUnlock(
	    context, log, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Remote Machine Information.\n"); 
	}
    }

    /* Failed */
    return 0;
}

/**
 * Get the Remote Class Information.
 */
static int
ngcllExecutableGetRemoteClassInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    ngLog_t *log,
    int *error)
{
    int result, found;
    static const char fName[] = "ngcllExecutableGetRemoteClassInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(execAttr != NULL);
    assert(rmInfoMng != NULL);

    /* Set the Remote Class Name */
    assert(execAttr->ngea_className != NULL);
    assert(executable->nge_rcInfoName == NULL);
    executable->nge_rcInfoName = strdup(execAttr->ngea_className);
    if (executable->nge_rcInfoName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for class name.\n"); 
        return 0;
    }

    /* Set the start time */
    result = ngiSetStartTime(
	&executable->nge_executionTime.nge_queryRemoteClassInformation, 
	log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the Start time.\n"); 
        return 0;
    }

    /* TODO: Does not locked? */
    /* Get the Remote Class Information (not found is valid) */
    result = ngcllExecutableRemoteClassInformationCacheGet(
        context, executable, execAttr->ngea_className, 0, &found, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't get Remote Class Information.\n"); 
    }

    /* Set the end time */
    result = ngiSetEndTime(
	&executable->nge_executionTime.nge_queryRemoteClassInformation, 
	log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the End time.\n"); 
	return 0;
    }

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Class Information \"%s\"%s found.\n",
        execAttr->ngea_className, (found != 0 ? "": " not")); 

    /* Success */
    return 1;
}

/**
 * CacheGet the Remote Class Information.
 * The Information is stored to executable->nge_rcInfo.
 */
static int
ngcllExecutableRemoteClassInformationCacheGet(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    char *className,
    int notFoundInvalid,
    int *found,
    ngLog_t *log,
    int *error)
{
    int result, subError;
    int rcInfoLocked;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcllExecutableRemoteClassInformationCacheGet";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(found != NULL);

    /* Initialize the local variables */
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);
    rcInfoLocked = 0;
    *found = 0;

    /* Is Remote Class Information already Exist? */
    if (executable->nge_rcInfoExist != 0) {
        *found = 1;

        /* Success */
        return 1;
    }

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(
        context, context->ngc_log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Remote Class Information.\n"); 
        return 0;
    }
    rcInfoLocked = 1;

    /* Get the Remote Class Information (not found is valid) */
    rcInfoMng = ngcliRemoteClassInformationCacheGet(
        context, className, &subError);
    if (rcInfoMng != NULL) {
        *found = 1;
        result = ngcliRemoteClassInformationCopy(context,
            &rcInfoMng->ngrcim_info, &executable->nge_rcInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the Remote Class Information.\n"); 
            goto error;
        }
        executable->nge_rcInfoExist = 1;
    } else {
        *found = 0;
        if (notFoundInvalid != 0) {
            NGI_SET_ERROR(error, subError);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't find the Remote Class Information.\n"); 
            goto error;
        }
    }

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(
        context, context->ngc_log, error);
    rcInfoLocked = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Remote Class Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Remote Class Information */
    if (rcInfoLocked != 0) {
    rcInfoLocked = 0;
    result = ngcliRemoteClassInformationListReadUnlock(
        context, context->ngc_log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Class Information.\n"); 
            return 0;
        }
    }

    /* Failed */
    return 0;
}
/**
 * Copy Executable members necessary from Remote Machine Information.
 */
static int
ngcllExecutableCopyMemberFromRemoteMachineInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "ngcllExecutableCopyMemberFromRemoteMachineInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(execAttr != NULL);
    assert(rmInfoMng != NULL);
    assert(rmInfoMng->ngrmim_info.ngrmi_heartBeat >= 0);
    assert(rmInfoMng->ngrmim_info.ngrmi_heartBeatTimeoutCount >= 0);

    /* Copy the host name */
    if (rmInfoMng->ngrmim_info.ngrmi_hostName != NULL) {
        executable->nge_hostName = strdup(rmInfoMng->ngrmim_info.ngrmi_hostName);
    } else if (execAttr->ngea_hostName != NULL) {
        executable->nge_hostName = strdup(execAttr->ngea_hostName);
    } else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't get valid host name.\n"); 
        return 0;
    }
    if (executable->nge_hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for host name.\n"); 
	return 0;
    }

    /* Copy the tag name */
    if (rmInfoMng->ngrmim_info.ngrmi_tagName != NULL) {
        executable->nge_tagName = strdup(
            rmInfoMng->ngrmim_info.ngrmi_tagName);
        if (executable->nge_tagName == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the storage for tag name.\n"); 
            return 0;
        }
    }

    if (execAttr->ngea_waitArgumentTransfer ==
        NG_ARGUMENT_TRANSFER_UNDEFINED) {
        executable->nge_waitArgumentTransfer =
            rmInfoMng->ngrmim_info.ngrmi_argumentTransfer;
    }

    executable->nge_heartBeatInterval = rmInfoMng->ngrmim_info.ngrmi_heartBeat;
    executable->nge_heartBeatTimeout =
        rmInfoMng->ngrmim_info.ngrmi_heartBeat *
        rmInfoMng->ngrmim_info.ngrmi_heartBeatTimeoutCount;

    /* Copy the Log Information */
    executable->nge_commLogEnable = 
        rmInfoMng->ngrmim_info.ngrmi_commLogEnable;
    if (executable->nge_commLogEnable != 0) {
        result = ngiLogInformationCopy(
            &rmInfoMng->ngrmim_info.ngrmi_commLogInfo,
            &executable->nge_commLogInfo, log ,error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the Communication Log Information.\n"); 
            return 0;
        }
    }

    executable->nge_keepConnect = rmInfoMng->ngrmim_info.ngrmi_keepConnect;

    /* Success */
    return 1;
}

/**
 * Copy Executable members necessary from Executable Path Information
 */
static int
ngcllExecutableCopyMemberFromExecutablePathInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclExecutableAttribute_t *execAttr,
    ngcliRemoteMachineInformationManager_t *rmInfoMng,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclExecutablePathInformation_t epInfo;
    static const char fName[] =
        "ngcllExecutableCopyMemberFromExecutablePathInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(execAttr != NULL);
    assert(rmInfoMng != NULL);

    /* Get the Executable Path Information */
    result = ngcliExecutablePathInformationGetCopyWithQuery(
        context, executable->nge_hostName,
        execAttr->ngea_className, 
        rmInfoMng->ngrmim_info.ngrmi_infoServiceTag,
        &epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Executable Path Information.\n"); 
        return 0;
    }

    /* Copy necessary members */
    executable->nge_sessionTimeout = epInfo.ngepi_sessionTimeout;
    executable->nge_transferTimeout_argument =
        epInfo.ngepi_transferTimeout_argument;
    executable->nge_transferTimeout_result =
        epInfo.ngepi_transferTimeout_result;
    executable->nge_transferTimeout_cbArgument =
        epInfo.ngepi_transferTimeout_cbArgument;
    executable->nge_transferTimeout_cbResult =
        epInfo.ngepi_transferTimeout_cbResult;

    /* Release */
    result = ngclExecutablePathInformationRelease(
        context, &epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Executable Path Information.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize
 */
int
ngclExecutableFinalize(ngclExecutable_t *executable, 
    int *isLastExecutable, int *error)
{

    /* just wrap for user */
    return ngcllExecutableFinalize(executable, isLastExecutable, error);
}

static int
ngcllExecutableFinalize(ngclExecutable_t *executable, 
    int *isLastExecutable, int *error)
{
    int result;
    ngclContext_t *context;
    ngcliJobManager_t *jobMng;
    ngLog_t *log;
    int retResult;
    static const char fName[] = "ngcllExecutableFinalize";

    /* initialize variable */
    retResult = 1; /* Success */

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    context = executable->nge_context;
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Is Executable valid? */
    if (executable->nge_top == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Pointer of top is NULL.\n"); 
	return 0;
    }

    if (isLastExecutable == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid argument.\n"); 
	return 0;
    }
    *isLastExecutable = 0;

    /* Initialize the local variables */
    log = context->ngc_log;
    jobMng = executable->nge_jobMng;

    /* Unregister from the Job Manager */
    /* Must do before waiting I/O Callback finished. */
    result = ngcliJobUnregisterExecutable(jobMng, executable, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executable from the Job Manager.\n"); 

        /* Failed */
        retResult = 0;
    }

    /* Connection Close */
    if ((executable->nge_protocol != NULL) &&
        (executable->nge_protocol->ngp_communication != NULL)) {

        result = ngiCommunicationClose(
            executable->nge_protocol->ngp_communication,
            log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't close communication.\n"); 

            /* Failed */
            retResult = 0;
        }
    }

    /* Wait I/O Callback End */
    result = ngiIOhandleCallbackWaiterWait(
        &executable->nge_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait the I/O callback End.\n"); 

        /* Failed */
        retResult = 0;
    }

    /* Destruct the Protocol */
    if (executable->nge_protocol != NULL) {
    	result = ngiProtocolDestruct(executable->nge_protocol, log, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Protocol.\n"); 
            /* Failed */
            retResult = 0;
	}
    }
    executable->nge_protocol = NULL;

    /* Destruct the Communication Log */
    if (executable->nge_commLog != NULL) {
        result = ngCommLogDestruct(executable->nge_commLog, log, error);
        executable->nge_commLog = NULL;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't destruct the Communication Log.\n"); 
            /* Failed */
            retResult = 0;
        }
    }

    /* Release LogInformation */
    result = ngiLogInformationFinalize(
       &executable->nge_commLogInfo, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Communication Log Information.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Release the Remote Class Information */
    if (executable->nge_rcInfoExist != 0) {
        executable->nge_rcInfoExist = 0;
	result = ngclRemoteClassInformationRelease(
	    context, &executable->nge_rcInfo, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Remote Class Information.\n"); 
            /* Failed */
            retResult = 0;
	}
    }

    /* Execute the user defined destructor */
    if (executable->nge_userDestructer != NULL) {
        executable->nge_userDestructer(executable);
    } else {
        if (executable->nge_userData != NULL) {
            ngiFree(executable->nge_userData, log, error);
        }
    }
    executable->nge_userData = NULL;
    executable->nge_userDestructer = NULL;

    /* Deallocate the host name of remote machine */
    if (executable->nge_hostName != NULL)
	ngiFree(executable->nge_hostName, log, error);
    executable->nge_hostName = NULL;

    /* Deallocate the class name */
    if (executable->nge_rcInfoName != NULL)
	ngiFree(executable->nge_rcInfoName, log, error);
    executable->nge_rcInfoName = NULL;

    /* Finalize the I/O Callback Waiter */
    result = ngiIOhandleCallbackWaiterFinalize(
        &executable->nge_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the I/O callback waiter.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Finalize the Read/Write Lock */
    result = ngcllExecutableFinalizeMutex(executable, log, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the mutex, cond, and Read/Write Lock.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Unregister Executable from Destruction List in the Context */
    result = ngcllExecutableUnregisterFromDestructionList(executable, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executable from the Context.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Initialize the members */
    ngcllExecutableInitializeMember(executable);

    /* Unregister Executable from Destruction List in the Job Manager */
    result = ngcliJobUnregisterExecutableFromDestructionList(jobMng, executable,
       isLastExecutable, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Executable from the Job Manager.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Destruct the Job Manager */
    if (*isLastExecutable != 0) {
        result = ngcliJobDestruct(jobMng, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Job Manager.\n"); 
            /* Failed */
            retResult = 0;
        }
    }

    /* Notify to HeartBeat */
    result = ngcliHeartBeatIntervalChange(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't notify interval change for heartbeat.\n"); 
        /* Failed */
        retResult = 0;
    }

    /* Success */
    return retResult;
}

/**
 * Initialize the members.
 */
static void
ngcllExecutableInitializeMember(ngclExecutable_t *executable)
{
    /* Initialize the flags and pointers */
    ngcllExecutableInitializeFlag(executable);
    ngcllExecutableInitializePointer(executable);

    /* Initialize the members */
    ngiLogInformationInitializeMember(&executable->nge_commLogInfo);

    executable->nge_nSessions.ngens_nSessions = 0;
    executable->nge_ID = NGI_EXECUTABLE_ID_UNDEFINED;
    executable->nge_status = NG_EXECUTABLE_STATUS_INITIALIZED;
    executable->nge_locked = 0;
    executable->nge_sending = 0;
    executable->nge_waitArgumentTransfer = NG_ARGUMENT_TRANSFER_WAIT;
    executable->nge_error = NG_ERROR_NO_ERROR;
    executable->nge_cbError = NG_ERROR_NO_ERROR;
    executable->nge_heartBeatInterval = 0;
    executable->nge_heartBeatTimeout = 0;
    executable->nge_heartBeatLastReceived = 0;
    executable->nge_heartBeatStatus = NG_HEART_BEAT_STATUS_OK;
    executable->nge_sessionTimeout = 0;
    executable->nge_transferTimeout_argument = 0;
    executable->nge_transferTimeout_result = 0;
    executable->nge_transferTimeout_cbArgument = 0;
    executable->nge_transferTimeout_cbResult = 0;
    executable->nge_jobStartTimeoutTime = 0;
    executable->nge_keepConnect = 0;
    executable->nge_connecting = 0;
    executable->nge_connectionCloseRequested = 0;

    executable->nge_nSessions.ngens_mutex = NGI_MUTEX_NULL;
    executable->nge_nSessions.ngens_cond  = NGI_COND_NULL;
    executable->nge_mutex = NGI_MUTEX_NULL;
    executable->nge_cond  = NGI_COND_NULL;
    executable->nge_rwlOwn = NGI_RWLOCK_NULL;
}

/**
 * Initialize the flags.
 */
static void
ngcllExecutableInitializeFlag(ngclExecutable_t *executable)
{
    executable->nge_rcInfoExist = 0;
    executable->nge_commLogEnable = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllExecutableInitializePointer(ngclExecutable_t *executable)
{
    executable->nge_next = NULL;
    /* comment out because nge_jobNext is used 
     * after this function is called in ngcllExecutableFinalize.
     *
     * executable->nge_jobNext = NULL;
     */
    executable->nge_apiNext = NULL;
    executable->nge_multiHandleNext = NULL;
    executable->nge_context = NULL;
    executable->nge_jobMng = NULL;
    executable->nge_session_head = NULL;
    executable->nge_session_tail = NULL;
    executable->nge_protocol = NULL;
    executable->nge_userData = NULL;
    executable->nge_userDestructer = NULL;
    executable->nge_hostName = NULL;
    executable->nge_tagName = NULL;
    executable->nge_rcInfoName = NULL;
    executable->nge_commLog = NULL;
}

/**
 * Initialize mutex, cond and Read/Write lock.
 */
static int
ngcllExecutableInitializeMutex(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *context;
    static const char fName[] = "ngcllExecutableInitializeMutex";

    /* Check the arguments */
    assert(executable != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;

    /* Initialize the mutex */
    result = ngiMutexInitialize(
        &executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Mutex.\n"); 
	return 0;
    }

    /* Initialize the cond */
    result = ngiCondInitialize(
        &executable->nge_nSessions.ngens_cond,
        context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Condition variable.\n"); 
        return 0;
    }

    /* Initialize the mutex */
    result = ngiMutexInitialize(
        &executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Mutex.\n"); 
	return 0;
    }

    /* Initialize the cond */
    result = ngiCondInitialize(
        &executable->nge_cond, context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Condition variable.\n"); 
        return 0;
    }

    /* Initialize the Read/Write Lock */
    result = ngiRWlockInitialize(
        &executable->nge_rwlOwn, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write Lock.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize mutex, cond and Read/Write lock.
 */
static int
ngcllExecutableFinalizeMutex(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *context;
    static const char fName[] = "ngcllExecutableFinalizeMutex";

    /* Check the arguments */
    assert(executable != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;

    /* Finalize the mutex */
    result = ngiMutexDestroy(
        &executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Mutex.\n"); 
        return 0;
    }

    /* Finalize the cond */
    result = ngiCondDestroy(
        &executable->nge_nSessions.ngens_cond, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Condition variable.\n"); 
        return 0;
    }

    /* Finalize the mutex */
    result = ngiMutexDestroy(
        &executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Mutex.\n"); 
        return 0;
    }

    /* Finalize the cond */
    result = ngiCondDestroy(
        &executable->nge_cond, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Condition variable.\n"); 
        return 0;
    }

    /* Finalize the Read/Write Lock */
    result = ngiRWlockFinalize(
        &executable->nge_rwlOwn, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Read/Write Lock.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register
 */
int
ngclExecutableRegister(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int nExecutables,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableRegister";

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

    result = ngcllExecutableRegister(context, executable,
	nExecutables, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableRegister(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int nExecutables,
    int *error)
{
    int i;
    int result;
    static const char fName[] = "ngcllExecutableRegister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the data for Executable contained in Ninf-G Context.\n"); 
	return 0;
    }

    /* Make the list */
    for (i = 0; i < (nExecutables - 1); i++) {
	executable[i].nge_next = &executable[i + 1];
    }
    executable[i].nge_next = NULL;

    /* No Executable is not registered */
    if (context->ngc_executable_head == NULL) {
	assert(context->ngc_executable_tail == NULL);

	/* Append at top of the list */
	context->ngc_executable_head = &executable[0];
	context->ngc_executable_tail = &executable[nExecutables - 1];
    }

    /* Append at last of the list */
    else {
        context->ngc_executable_tail->nge_next = &executable[0];
        context->ngc_executable_tail = &executable[nExecutables - 1];
    }

    /* Unlock the data for Executable include Ninf-G Context */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the data for Executable contained in Ninf-G Context.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister
 */
int
ngclExecutableUnregister(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *prev, *curr;
    static const char fName[] = "ngclExecutableUnregister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Executable is not valid.\n"); 
        return 0;
    }
    context = executable->nge_context;

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Find the Executable */
    prev = NULL;
    curr = context->ngc_executable_head;
    for (; curr != executable; curr = curr->nge_next) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Executable */
    if (executable == context->ngc_executable_head)
	context->ngc_executable_head = executable->nge_next;
    if (executable == context->ngc_executable_tail)
    	context->ngc_executable_tail = prev;
    if (prev != NULL)
    	prev->nge_next = executable->nge_next;
    executable->nge_next = NULL;

    /* Append to Destruction list */

    /* No Executable is registered */
    if (context->ngc_destruction_executable_head == NULL) {
	assert(context->ngc_destruction_executable_tail == NULL);

	/* Append at top of the list */
	context->ngc_destruction_executable_head = executable;
	context->ngc_destruction_executable_tail = executable;
    }

    /* Append at last of the list */
    else {
	assert(context->ngc_destruction_executable_tail != NULL);

        context->ngc_destruction_executable_tail->nge_next = executable;
        context->ngc_destruction_executable_tail = executable;
    }

    /* Unlock the data for Executable include Ninf-G Context */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found.\n"); 

    /* Unlock the data for Executable include Ninf-G Context */
    result = ngclExecutableListWriteUnlock(context, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list Executable.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Rotate the list of Executable.
 *
 * Note:
 * Write lock the list before using this function, and unlock the list after
 * use.
 */
int
ngcliExecutableListRotate(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcliExecutableListRotate";

    /* Check the arguments */
    assert(context != NULL);

    /* Lock the list of Executable */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Get the Executable */
    executable = context->ngc_executable_head;
    if ((executable == NULL) ||
        (context->ngc_executable_tail == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "No executables were registered.\n"); 
        goto error;
    }

    /* Is only one Executable registered? */
    if (executable == context->ngc_executable_tail) {
        /* Do nothing. Success */
        goto success;
    }

    /* Register the head */
    context->ngc_executable_head = executable->nge_next;
    executable->nge_next = NULL;
    context->ngc_executable_tail->nge_next = executable;
    context->ngc_executable_tail = executable;

success:
    /* Unlock the list of Executable */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Executable */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
    }

    /* Failed */
    return 0;
}

static int
ngcllExecutableUnregisterFromDestructionList(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *prev, *curr;
    static const char fName[] = "ngcllExecutableUnregisterFromDestructionList";

    /* argument check */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    context = executable->nge_context;

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Find the Executable in Destruction List */
    prev = NULL;
    curr = context->ngc_destruction_executable_head;
    for (; curr != executable; curr = curr->nge_next) {
        if (curr == NULL) {
            goto notFound;
        }
        prev = curr;
    }
    assert(curr != NULL);

    /* Unregister the Executable */
    if (executable == context->ngc_destruction_executable_head) {
        assert(prev == NULL);
        context->ngc_destruction_executable_head = executable->nge_next;
    }
    if (executable == context->ngc_destruction_executable_tail)
        context->ngc_destruction_executable_tail = prev;
    if (prev != NULL) {
        assert(executable != context->ngc_destruction_executable_head);
        prev->nge_next = executable->nge_next;
    }
    executable->nge_next = NULL;

    /* Unlock the data for Executable include Ninf-G Context */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found.\n"); 

    /* Unlock the data for Executable include Ninf-G Context */
    result = ngclExecutableListWriteUnlock(context, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list Executable.\n"); 
    }

    return 0;
}

/**
 * Register the user defined data.
 */
int
ngclExecutableRegisterUserData(
    ngclExecutable_t *executable,
    void *userData,
    void (*userDestructer)(ngclExecutable_t *),
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableRegisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllExecutableRegisterUserData(
	executable, userData, userDestructer, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableRegisterUserData(
    ngclExecutable_t *executable,
    void *userData,
    void (*userDestructer)(ngclExecutable_t *),
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutableRegisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock this instance */
    result = ngclExecutableWriteLock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Executable.\n"); 
	return 0;
    }

    /* Register the user defined data */
    executable->nge_userData = userData;
    executable->nge_userDestructer = userDestructer;

    /* Unlock this instance */
    result = ngclExecutableWriteUnlock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister the user defined data.
 */
int
ngclExecutableUnregisterUserData(
    ngclExecutable_t *executable,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableUnregisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllExecutableUnregisterUserData(executable, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableUnregisterUserData(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutableUnregisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock this instance */
    result = ngclExecutableWriteLock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Executable.\n"); 
	return 0;
    }

    /* Unregister the user defined data */
    executable->nge_userData = NULL;
    executable->nge_userDestructer = NULL;

    /* Unlock this instance */
    result = ngclExecutableWriteUnlock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the user defined data.
 */
int
ngclExecutableGetUserData(
    ngclExecutable_t *executable,
    void **userData,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableGetUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    /* Check the arguments */
    if (userData == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "The userData is NULL.\n"); 
        return 0;
    }

    result = ngcllExecutableGetUserData(executable, userData, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableGetUserData(
    ngclExecutable_t *executable,
    void **userData,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutableGetUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);
    assert(userData != NULL);

    /* Lock this instance */
    result = ngclExecutableReadLock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Executable.\n"); 
	return 0;
    }

    /* Get the user defined data */
    *userData = executable->nge_userData;

    /* Unlock this instance */
    result = ngclExecutableReadUnlock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Connecting.
 */
int
ngcliExecutableConnecting(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableConnecting";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(executable->nge_protocol == NULL);

    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "TCP Connection established. Performing Ninf-G Negotiation.\n"); 

    /* Save the Protocol */
    executable->nge_protocol = protocol;

    /* Update the status */
    result = ngcliExecutableStatusSet(
	executable, NG_EXECUTABLE_STATUS_CONNECTING, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 

	executable->nge_protocol = NULL;

        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Connected.
 */
int
ngcliExecutableConnected(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableConnected";

    /* Check the arguments */
    assert(executable != NULL);

    /* Set the status */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_CONNECTED, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the status.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Idle.
 */
int
ngcliExecutableIdle(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableIdle";

    /* Check the arguments */
    assert(executable != NULL);

    /* Set the status */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_IDLE, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the status.\n"); 
        return 0;
    }

    result = ngcliJobStartTimeoutJobStarted(executable->nge_context,
        executable, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't stop job start timeout.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Job was done.
 */
int
ngcliExecutableJobDone(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result, retResult;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    int jobManagerLocked, executableListLocked;
    static const char fName[] = "ngcliExecutableJobDone";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    retResult = 1;
    jobManagerLocked = 0;
    executableListLocked = 0;

    /* Initialize the local variable */
    context = jobMng->ngjm_context;

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Job Manager.\n"); 
	return 0;
    }
    jobManagerLocked = 1;

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	goto error;
    }
    executableListLocked = 1;

    /* Get the Executable */
    executable = ngcliJobGetNextExecutable(jobMng, NULL, NULL);
    while (executable != NULL) {
	/* Has requested Exit? */
	if (executable->nge_status < NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Unexpected job_done was occurred.\n"); 

            /* Set the executable unusable */
            result = ngcliExecutableUnusable(
                executable, NG_ERROR_JOB_DEAD, NULL);
            if (result == 0) {
                retResult = 0;
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL,
                    fName,  "Failed to set the executable unusable.\n"); 
            }
        }

	/* Get next Executable */
	executable = ngcliJobGetNextExecutable(jobMng, executable, NULL);
    }
        
    /* Unlock the list of Executable */
    executableListLocked = 0;
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	goto error;
    }

    /* Unlock the list of Job Manager */
    jobManagerLocked = 0;
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Job Manager.\n"); 
	return 0;
    }

    return retResult;

    /* Error occurred */
error:
    if (executableListLocked != 0) {
	result = ngclExecutableListReadUnlock(context, NULL);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Executable.\n"); 
	}
    }

    if (jobManagerLocked != 0) {
        result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
        if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Job Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Execute the waiting Session.
 */
int
ngcliExecutableExecuteSession(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    int listLocked = 0;
    int executableLocked = 0;
    int executableLockedWithSend = 0;
    ngclContext_t *context;
    ngclSession_t *session;
    static const char fName[] = "ngcliExecutableExecuteSession";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Start.\n"); 

    /* Initialize the variables */
    context = executable->nge_context;

    /* Executable has not connected? */
    if (executable->nge_protocol == NULL) {
	ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable has not connected.\n"); 
	goto success;
    }

    /* Lock the Session list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
        goto error;
    }
    listLocked = 1;

    /* Lock the Executable */
    result = ngcliExecutableLockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLockedWithSend = 1;

    /* Check the Executable */
    if (executable->nge_status
        == NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST) {

        executableLocked = 1;
        executableLockedWithSend = 0;

        /* Unlock the Executable */
        if (executableLocked != 0) {
            executableLocked = 0;
            result = ngcliExecutableUnlock(executable, log, error);
            if (result == 0) {
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't unlock the Executable.\n"); 
                goto error;
            }
        }

        /* Query Function Information */
        result = ngcliProtocolRequestQueryFunctionInformation(
            executable, executable->nge_protocol, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't send the QUERY FUNCTION INFORMATION.\n"); 
            goto error;
        }
        goto success;
    }

    /* Find the session */
    for (session = NULL;
	 (session = ngclSessionGetNext(executable, session, error)) != NULL;) {
	switch (session->ngs_status) {
	case NG_SESSION_STATUS_WAIT_CONNECT:
	case NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE:
	case NG_SESSION_STATUS_CALCULATE_DONE:
	case NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED:
	    goto next;
	    break;

	default:
	    break;
	}
    }
next:

    /* Is no session found? */
    if (session == NULL) {
        /* Print the debug message */
        ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "No Session was found for invoke.\n"); 
        goto success;
    }

    /*
     * Perform the Session.
     */
    executableLocked = 1;
    executableLockedWithSend = 0;

    /* Unlock the Executable */
    if (executableLocked != 0) {
	executableLocked = 0;
	result = ngcliExecutableUnlock(executable, log, error);
	if (result == 0) {
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the Executable.\n"); 
	    goto error;
	}
    }

    switch (session->ngs_status) {
    case NG_SESSION_STATUS_WAIT_CONNECT:
    case NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE:
	/* 2004/11/18 asou
	 * This process has been performed when negotiation was completed or
	 * Session was done. 
	 */
	/* Invoke Session */
	result = ngcliProtocolRequestInvokeSession(
	    session, executable->nge_protocol, log, error);
	if (result == 0) {
	    ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't send the INVOKE SESSION.\n"); 
	    goto error;
	}
	break;

    case NG_SESSION_STATUS_CALCULATE_DONE:
	/* 2004/11/18 asou
	 * I can't imagine when this process was performed. Is this process
	 * never performed?
	 */
	/* Transfer Result Data */
	result = ngcliProtocolRequestTransferResultData(
	    session, executable->nge_protocol, log, error);
	if (result == 0) {
	    ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't send the TRANSFER RESULT DATA.\n"); 
	    goto error;
	}
	break;

    case NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED:
	/* 2004/11/18 asou
	 * May be this process has been performed when receive the
	 * Function Information of Callback Function.
	 */
	/* Transfer Callback Argument Data */
	result = ngcliProtocolRequestTransferCallbackArgumentData(
	    session, executable->nge_protocol, log, error);
	if (result == 0) {
	    ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't send the TRANSFER CALLBACK ARGUMENT DATA.\n"); 
	    goto error;
	}
	break;

    default:
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid Session status (%s).\n",
            ngcliSessionStatusToString(session->ngs_status)); 
	assert(0); /* Abort here when debugging */
	break;

    }

success:
    /* Unlock the Executable with send */
    if (executableLockedWithSend != 0) {
        executableLockedWithSend = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
            goto error;
        }
    }

    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    /* Unlock the list */
    if (listLocked != 0) {
	listLocked = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	    return 0;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable with send */
    if ((executableLockedWithSend != 0) || executableLocked != 0) {
        executableLockedWithSend = 0;
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
        }
    }

    /* Unlock the list */
    if (listLocked != 0) {
	listLocked = 0;
	result = ngclSessionListReadUnlock(context, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	}
    }

    /* Failed */
    return 0;
}

/**
 * Lock the Executable.
 */
int
ngcliExecutableLock(ngclExecutable_t *executable, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableLock";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Wait unlock */
    while (executable->nge_locked != 0) {
        result = ngiCondWait(
            &executable->nge_cond, &executable->nge_mutex, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
        }
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Lock the Executable.\n"); 

    /* Lock the Executable */
    executable->nge_locked = 1;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * TryLock the Executable.
 */
int
ngcliExecutableTryLock(
    ngclExecutable_t *executable,
    int *locked,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableTryLock";

    /* Check the arguments */
    assert(executable != NULL);
    assert(locked != NULL);

    *locked = 0;

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* locked? */
    if (executable->nge_locked == 0) {
        /* Print the debug message */
        ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Locked the Executable.\n"); 

        *locked = 1;

        /* Lock the Executable */
        executable->nge_locked = 1;

    } else {
        /* Print the debug message */
        ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Not locked the Executable.\n"); 

        *locked = 0;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unlock the Executable.
 */
int
ngcliExecutableUnlock(ngclExecutable_t *executable, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableUnlock";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Is Executable locked? */
    if (executable->nge_locked == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not locked.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Unlock the Executable.\n"); 

    /* Unlock the Executable */
    executable->nge_locked = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&executable->nge_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Lock the Executable with send.
 */
int
ngcliExecutableLockWithSend(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableLockWithSend";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Wait unlock */
    while ((executable->nge_locked != 0) || (executable->nge_sending != 0)) {
        result = ngiCondWait(
            &executable->nge_cond, &executable->nge_mutex, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
        }
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Lock the Executable with send.\n"); 

    /* Lock the Executable with send*/
    executable->nge_locked = 1;
    executable->nge_sending = 1;

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Unlock the Executable with send.
 */
int
ngcliExecutableUnlockWithSend(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableUnlockWithSend";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Is Executable locked? */
    if (executable->nge_locked == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not locked.\n"); 
        goto error;
    }

    /* Is Executable sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Unlock the Executable with send.\n"); 

    /* Unlock the Executable */
    executable->nge_locked = 0;
    executable->nge_sending = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&executable->nge_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Unlock the Executable with all.
 */
int
ngcliExecutableUnlockWithAll(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableUnlockWithAll";

    /* Check the arguments */
    assert(executable != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Start.\n"); 

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Is Executable locked? */
    if (executable->nge_locked == 0) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_LOCKED);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not locked.\n"); 
        goto error;
    }

    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Unlock the Executable with all.\n"); 

    /* Unlock the Executable */
    executable->nge_locked = 0;
    executable->nge_sending = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&executable->nge_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * If transmission is possible, it will set to sending.
 */
int
ngcliExecutableLockTrySend(
    ngclExecutable_t *executable,
    int *success,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableLockTrySend";

    /* Check the arguments */
    assert(executable != NULL);
    assert(success != NULL);

    /* Initialize the variables */
    *success = 0;

    /* Lock the mutex */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Is Executable sending? */
    if (executable->nge_sending != 0) {
        /* Print the debug message */
        ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the transmitting permission.\n"); 
    } else {
        /* Print the debug message */
        ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Get the transmitting permission.\n"); 

        *success = 1;
        executable->nge_sending = 1;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Increment the number of Sessions.
 */
int
ngcliExecutableNsessionsIncrement(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableNsessionsIncrement";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock */
    result = ngiMutexLock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Increment the number of Sessions */
    assert(executable->nge_nSessions.ngens_nSessions >= 0);
    executable->nge_nSessions.ngens_nSessions++;

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Decrement the number of Sessions.
 */
int
ngcliExecutableNsessionsDecrement(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    int retResult = 1;
    static const char fName[] = "ngcliExecutableNsessionsDecrement";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock */
    result = ngiMutexLock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Decrement the number of Sessions */
    assert(executable->nge_nSessions.ngens_nSessions > 0);
    executable->nge_nSessions.ngens_nSessions--;

    /* Notify signal */
    result = ngiCondBroadcast(
	&executable->nge_nSessions.ngens_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	retResult = 0;
	error = NULL;
	goto error;
    }

error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
	retResult = 0;
	error = NULL;
    }

    /* Success */
    return retResult;
}

/**
 * Wait until the number of Sessions is set to 0.
 */
int
ngcliExecutableNsessionsWaitUntilZero(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    int retResult = 1;
    static const char fName[] = "ngcliExecutableNsessionsWaitUntilZero";

    /* Check the arguments */
    assert(executable != NULL);

    /* Lock */
    result = ngiMutexLock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        return 0;
    }

    /* Wait the status */
    while (executable->nge_nSessions.ngens_nSessions > 0) {
        result = ngiCondWait(
            &executable->nge_nSessions.ngens_cond,
	    &executable->nge_nSessions.ngens_mutex,
	    log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
	    retResult = 0;
	    error = NULL;
            goto error;
        }
    }

error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_nSessions.ngens_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        retResult = 0;
	error = NULL;
    }

    /* Finish */
    return retResult;
}
/**
 * Set the status.
 */
int
ngcliExecutableStatusSet(
    ngclExecutable_t *executable,
    ngclExecutableStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result, retResult;
    static const char fName[] = "ngcliExecutableStatusSet";

    /* Check the arguments */
    assert(executable != NULL);

    retResult = 1;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Check the status transition */
    result = ngcllExecutableStatusCheckTransition(executable, status, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable Status transition invalid.\n"); 

        status = NG_EXECUTABLE_STATUS_DONE;
        retResult = 0;
    }

    /* log */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "status \"%s\" -> \"%s\"\n",
        ngcllExecutableStatusToString(executable->nge_status),
        ngcllExecutableStatusToString(status)); 

    /* Set the status */
    executable->nge_status = status;

    /* Notify signal */
    result = ngiCondBroadcast(&executable->nge_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return retResult;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Check the status transition.
 */
static int
ngcllExecutableStatusCheckTransition(
    ngclExecutable_t *executable,
    ngclExecutableStatus_t nextStatus,
    int *error)
{
    int retResult;
    ngclExecutableStatus_t currentStatus;
    static const char fName[] = "ngcllExecutableStatusCheckTransition";

    /* Check the arguments */
    assert(executable != NULL);

    currentStatus = executable->nge_status;
    retResult = 1;

    if (nextStatus == NG_EXECUTABLE_STATUS_DONE) {
        /* Success */
        return 1;
    }

    switch (currentStatus) {
    case NG_EXECUTABLE_STATUS_INITIALIZED:
        if (nextStatus != NG_EXECUTABLE_STATUS_CONNECTING) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_CONNECTING:
        if (nextStatus != NG_EXECUTABLE_STATUS_CONNECTED) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_CONNECTED:
        switch(nextStatus) {
        case NG_EXECUTABLE_STATUS_IDLE:
        case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST:
            break;
        default:
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_IDLE:
        switch(nextStatus) {
        case NG_EXECUTABLE_STATUS_RESET_REQUESTED:
        case NG_EXECUTABLE_STATUS_EXIT_REQUESTED:
            break;
        default:
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST:
        if (nextStatus !=
            NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED:
        if (nextStatus !=
            NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_DONE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_DONE:
        if (nextStatus != NG_EXECUTABLE_STATUS_IDLE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_REQUESTED:
        if (nextStatus !=
            NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE:
        if (nextStatus != NG_EXECUTABLE_STATUS_IDLE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_RESET_REQUESTED:
        if (nextStatus != NG_EXECUTABLE_STATUS_RESET_DONE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_RESET_DONE:
        if (nextStatus != NG_EXECUTABLE_STATUS_IDLE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_EXIT_REQUESTED:
        if (nextStatus != NG_EXECUTABLE_STATUS_EXIT_DONE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_EXIT_DONE:
        if (nextStatus != NG_EXECUTABLE_STATUS_DONE) {
            retResult = 0;
        }
        break;
    case NG_EXECUTABLE_STATUS_DONE:
        retResult = 0;
        break;
    default:
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state %d.\n", currentStatus); 
        break;
    }

    /* Report the error */
    if (retResult == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state transition (%d to %d).\n",
            currentStatus, nextStatus); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Wait the status.
 */
int
ngcliExecutableStatusWait(
    ngclExecutable_t *executable,
    ngclExecutableStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    int errorCause;
    static const char fName[] = "ngcliExecutableStatusWait";

    /* Check the arguments */
    assert(executable != NULL);

    errorCause = NG_ERROR_NO_ERROR;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        return 0;
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Waiting for the state of Executable, grow into %s.\n",
        ngcllExecutableStatusToString(status)); 

    /* Wait the status */
    while ((executable->nge_status != status) &&
           (executable->nge_status != NG_EXECUTABLE_STATUS_DONE)) {
        result = ngiCondWait(
            &executable->nge_cond, &executable->nge_mutex, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
        }
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "The state of Executable grew into %s.\n",
        ngcllExecutableStatusToString(status)); 

    /* Is error occurred? */
    if (executable->nge_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_error);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Is error occurred in callback? */
    if (executable->nge_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_cbError);
        executable->nge_error = executable->nge_cbError;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Unexpected DONE is error */
    if (executable->nge_status != status) {
        assert(executable->nge_status == NG_EXECUTABLE_STATUS_DONE);

        errorCause = NG_ERROR_INVALID_STATE;
        NGI_SET_ERROR(error, errorCause);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable become unexpected state: DONE.\n"); 
        goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Wait the status with timeout.
 */
int
ngcliExecutableStatusWaitTimeout(
    ngclExecutable_t *executable,
    ngclExecutableStatus_t status,
    int second,
    ngLog_t *log,
    int *error)
{
    time_t timeEnd;
    int result, remain;
    int timeout;
    int errorCause;
    static const char fName[] = "ngcliExecutableStatusWaitTimeout";

    /* Check the arguments */
    assert(executable != NULL);

    timeEnd = time(NULL) + second;
    remain = second;
    errorCause = NG_ERROR_NO_ERROR;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Waiting for the state of Executable, grow into %s.\n",
        ngcllExecutableStatusToString(status)); 

    /* Wait the status */
    while ((executable->nge_status != status) &&
	   (executable->nge_status != NG_EXECUTABLE_STATUS_DONE)) {

	timeout = 0;
	result = ngiCondTimedWait(
	    &executable->nge_cond, &executable->nge_mutex, remain, &timeout,
	    log, error);
	if (result == 0) {
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't wait the Condition Variable.\n"); 
	    goto error;
	}

 	/* Timedout? */
	if (timeout != 0) {
            errorCause = NG_ERROR_TIMEOUT;
	    NGI_SET_ERROR(error, errorCause);
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Timeout. waited %d seconds\n", second); 
	    goto error;
	}

        remain = timeEnd - time(NULL);
        if (remain < 0) {
            remain = 0;
        }
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "The state of Executable grew into %s.\n",
        ngcllExecutableStatusToString(status)); 

    /* Is error occurred? */
    if (executable->nge_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_error);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Is error occurred in callback? */
    if (executable->nge_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_cbError);
        executable->nge_error = executable->nge_cbError;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Unexpected DONE is error */
    if (executable->nge_status != status) {
        assert(executable->nge_status == NG_EXECUTABLE_STATUS_DONE);

        errorCause = NG_ERROR_INVALID_STATE;
        NGI_SET_ERROR(error, errorCause);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable become unexpected state: DONE.\n"); 
        goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Wait connected.
 * This function isn't used.
 */
int
ngcliExecutableStatusWaitConnected(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    ngcliJobManager_t *jobMng;
    int timeout;
    static const char fName[] = "ngcliExecutableStatusWaitConnected";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_jobMng != NULL);

    jobMng = executable->nge_jobMng;
    timeout = jobMng->ngjm_attr.ngja_startTimeout;

    /* 
     * job start timeout is measured with Time Event.
     * Thus this function doesn't measure job start timeout.
     */

    /* Wait Executable */
    result = ngcliExecutableStatusWait(
        executable, NG_EXECUTABLE_STATUS_CONNECTED, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not connected.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Wait Idle.
 */
int
ngcliExecutableStatusWaitIdle(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    int timeout;
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcliExecutableStatusWaitIdle";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_jobMng != NULL);

    jobMng = executable->nge_jobMng;
    timeout = jobMng->ngjm_attr.ngja_startTimeout;

    /* 
     * job start timeout is measured with Time Event.
     * Thus this function doesn't measure job start timeout.
     */

    /* Wait Executable */
    result = ngcliExecutableStatusWait(
        executable, NG_EXECUTABLE_STATUS_IDLE, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not connected.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Wait exited.
 */
int
ngcliExecutableStatusWaitExited(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcliExecutableStatusWaitExited";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_jobMng != NULL);

    jobMng = executable->nge_jobMng;

    /* Wait Executable */
    if ((jobMng->ngjm_attr.ngja_stopTimeout < 0) ||
        (jobMng->ngjm_attr.ngja_stopTimeout == 0)) {
        /* When Timeout is 0 or negative value, 
         * Wait forever.
         */
        result = ngcliExecutableStatusWait(
            executable, NG_EXECUTABLE_STATUS_EXIT_DONE, log, error);
    } else {
        result = ngcliExecutableStatusWaitTimeout(
            executable, NG_EXECUTABLE_STATUS_EXIT_DONE,
            jobMng->ngjm_attr.ngja_stopTimeout, log, error);
    }
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable not reported as exited.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the Executable Remote Class Information available.
 */
int
ngcliExecutableRemoteClassInformationNotify(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableRemoteClassInformationNotify";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_rcInfoExist != 0);

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* log */
    ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Class Information is arrived.\n"); 

    /* Notify signal */
    result = ngiCondBroadcast(&executable->nge_cond, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Wait the Remote Class Information ready.
 */
int
ngcliExecutableRemoteClassInformationWait(
    ngclExecutable_t *executable,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableRemoteClassInformationWait";

    /* Check the arguments */
    assert(executable != NULL);

    /* Is Remote Class Information already Exist? */
    if (executable->nge_rcInfoExist != 0) {

        /* Success */
        return 1;
    }

    /* Wait Executable */
    result = ngcliExecutableStatusWaitIdle(executable, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not connected.\n"); 
        return 0;
    }

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        return 0;
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Waiting to arrive Remote Class Information.\n"); 

    /* Wait the RemoteClassInformation */
    while ((executable->nge_rcInfoExist == 0) &&
           (executable->nge_status != NG_EXECUTABLE_STATUS_DONE)) {
        result = ngiCondWait(
            &executable->nge_cond, &executable->nge_mutex, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
        }
    }

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Finished waiting Remote Class Information.\n"); 

    /* Is error occurred? */
    if (executable->nge_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_error);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting Remote Class Information.\n"); 
        goto error;
    }

    /* Is error occurred in callback? */
    if (executable->nge_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_cbError);
        executable->nge_error = executable->nge_cbError;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting Remote Class Information.\n"); 
        goto error;
    }

    /* Unexpected DONE is error */
    if (executable->nge_rcInfoExist == 0) {
        assert(executable->nge_status == NG_EXECUTABLE_STATUS_DONE);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable become unexpected state: DONE.\n"); 
        goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, NULL);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Set the Executable unusable
 */
int
ngcliExecutableUnusable(
    ngclExecutable_t *executable,
    int errorCause,
    int *error)
{
    ngLog_t *log;
    int result, retResult, executableLocked;
    ngclSession_t *session;
    ngclContext_t *context;
    ngiCommunication_t *comm;
    static const char fName[] = "ngcliExecutableUnusable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;
    log = executable->nge_context->ngc_log;
    retResult = 1;
    executableLocked = 0;
    comm = NULL;

    /* log */
    ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Setting this executable unusable.\n"); 

    /* Try Lock the Executable (It's ok, although lock was not present.) */
    result = ngcliExecutableTryLock(executable, &executableLocked, log, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
    }

    /* Get the Communication */
    comm = NULL;
    if ((executable->nge_protocol != NULL) &&
        (executable->nge_protocol->ngp_communication != NULL)) {
        comm = executable->nge_protocol->ngp_communication;
    }

    if (comm != NULL) { 
        ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Closing Communication.\n"); 

        /* Close the connection and finish I/O callback */
        result = ngiCommunicationClose(comm, log, error);
        if (result == 0) {
            retResult = 0;
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't close communication.\n"); 
        }
    }

    if (executableLocked == 0) {
        /* Lock the Executable */
        result = ngcliExecutableLock(executable, log, error);
        if (result == 0) {
            retResult = 0;
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
        }
        executableLocked = 1;
    }

    /* Check error */
    if (errorCause == NG_ERROR_NO_ERROR) {
        ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Setting error to NO_ERROR.\n"); 
    }

    /* Set the error */
    result = ngcliExecutableSetError(executable, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the error.\n"); 
    }

    /* Set the callback error */
    result = ngcliExecutableSetCbError(
        executable, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the callback error.\n"); 
    }

    /* Executable is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_DONE, log, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
    }

    /* Request job cancel */
    if (executable->nge_jobMng != NULL) {
        ngcliLogInfoJob(executable->nge_jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "requesting to perform job cancel on destruct.\n"); 
     
        result = ngcliJobRequestCancel(executable->nge_jobMng, log, error);
        if (result == 0) {
            retResult = 0;
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't request the cancel.\n"); 
        }
    } else {
        ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "job manager was NULL.\n"); 
    }

    /* Unlock the Executable */
    if (executableLocked != 0) {
        result = ngcliExecutableUnlockWithAll(executable, log, error);
        executableLocked = 0;
        if (result == 0) {
            retResult = 0;
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Set unusable for sessions */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
    }

    session = NULL; /* retrieve head item */
    while ((session = ngclSessionGetNext(executable, session, error)) != NULL) {

        result = ngcliSessionUnusable(executable, session, errorCause, error);
        if (result == 0) {
            retResult = 0;
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the Session unusable.\n"); 
            break;
        }
    }

    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        retResult = 0;
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
    }

    return retResult;
}

#if 0 /* Is this necessary? Use GetCopy instead. */
/**
 * Get the error code.
 */
int
ngclExecutableGetError(ngclExecutable_t *executable, int *error)
{
    int result;
    int error;
    static const char fName[] = "ngclExecutableGetError";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is executable valid? */
    result = ngcliExecutableIsValid(executable, error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable is not valid.\n"); 
        return -1;
    }

    /* Lock this instance */
    result = ngclExecutableReadLock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the Executable.\n"); 
	return -1;
    }

    /* Get the user defined data */
    error = executable->nge_error;

    /* Unlock this instance */
    result = ngclExecutableReadUnlock(executable, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unlock the Executable.\n"); 
	return -1;
    }

    /* Success */
    return error;
}
#endif /* 0 */

/**
 * Set the error code.
 */
int
ngcliExecutableSetError(ngclExecutable_t *executable, int setError, int *error)
{
    int result;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngcliExecutableSetError";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;
    log = context->ngc_log;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Set the user defined data */
    executable->nge_error = setError;

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the error code, it occurred in callback function.
 */
int
ngcliExecutableSetCbError(
    ngclExecutable_t *executable,
    int cbError,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngcliExecutableSetCbError";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;
    log = context->ngc_log;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Set the user defined data */
    executable->nge_cbError = cbError;

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the error code, it occurred in callback function.
 */
int
ngcliExecutableGetCbError(
    ngclExecutable_t *executable,
    int *cbError,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngcliExecutableGetCbError";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;
    log = context->ngc_log;

    /* Lock */
    result = ngiMutexLock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
    	return 0;
    }

    /* Set the user defined data */
    *cbError = executable->nge_cbError;

    /* Unlock */
    result = ngiMutexUnlock(&executable->nge_mutex, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Is Executable valid?
 */
int
ngcliExecutableIsValid(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclExecutable_t *curr;
    static const char fName[] = "ngcliExecutableIsValid";

    /* Is Executable NULL? */
    if (executable == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is NULL.\n"); 
	return 0;
    }

    /* Use Ninf-G Context which contained in Executable, if context is NULL */
    if (context == NULL) {
	context = executable->nge_context;
    }

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Is ID smaller than minimum? */
    if (executable->nge_ID < NGI_EXECUTABLE_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable ID %d is smaller than minimum %d.\n",
            executable->nge_ID, NGI_EXECUTABLE_ID_MIN); 
	return 0;
    }

    /* Is ID greater than maximum? */
    if (executable->nge_ID > NGI_EXECUTABLE_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable ID %d is greater than maximum %d.\n",
            executable->nge_ID, NGI_EXECUTABLE_ID_MAX); 
	return 0;
    }

    /* Lock the list of Executable */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Is Executable exist? */
    curr = ngcllExecutableListAndDestructionListGetNext(context, NULL, error);
    if (curr == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "No Executable is registered.\n"); 
	goto error;
    }

    while (curr != executable) {
	curr = ngcllExecutableListAndDestructionListGetNext(context, curr, error);
	if (curr == NULL) {
    	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	        "Executable is not found.\n"); 
	    goto error;
	}
    }

    /* Unlock the list of Executable */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Executable is valid */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Executable */
    result = ngclExecutableListReadUnlock(context, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Is Executable available to create session?
 */
int
ngcliExecutableIsAvailable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    ngclExecutableStatus_t status;
    static const char fName[] = "ngcliExecutableIsAvailable";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    /* Is error occurred? */
    if (executable->nge_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, executable->nge_error);
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable Error (%d) has already occurred.\n", executable->nge_error); 
        return 0;
    }

    status = executable->nge_status;
    if (status == NG_EXECUTABLE_STATUS_DONE) {
        NGI_SET_ERROR(error, NG_ERROR_JOB_DEAD);
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is done.\n"); 

        /* Not available to create session */
        return 0;
    } else if ((status == NG_EXECUTABLE_STATUS_EXIT_REQUESTED) ||
        (status == NG_EXECUTABLE_STATUS_EXIT_DONE)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not available to use anymore.\n"); 

        /* Not available to create session */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * User list: Initialize
 */
int
ngclExecutableUserListInitialize(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableUserListInitialize";

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

    result = ngcllExecutableUserListInitialize(context, list, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableUserListInitialize(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    int *error)
{
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableUserListInitialize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    log = context->ngc_log;

    /* Is list valid? */
    if (list == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "List is NULL.\n"); 
	return 0;
    }

    /* Initialize the members */
    ngcllExecutableUserListInitializeMember(list);

    /* Success */
    return 1;
}

/**
 * User list: Finalize
 */
int
ngclExecutableUserListFinalize(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableUserListFinalize";

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

    result = ngcllExecutableUserListFinalize(context, list, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableUserListFinalize(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    int *error)
{
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableUserListFinalize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    log = context->ngc_log;

    /* Is list valid? */
    if (list == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "List is NULL.\n"); 
	return 0;
    }

    /* Are any Executables exist? */
    if ((list->ngel_head != NULL) || (list->ngel_tail != NULL)) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Any Executables were exist.\n"); 
	return 0;
    }

    /* Initialize the members */
    ngcllExecutableUserListInitializeMember(list);

    /* Success */
    return 1;
}

/**
 * User list: Initialize the members.
 */
static void
ngcllExecutableUserListInitializeMember(ngclExecutableList_t *list)
{
    /* Initialize the pointers */
    ngcllExecutableUserListInitializePointer(list);
}

/**
 * User list: Initialize the pointers.
 */
static void
ngcllExecutableUserListInitializePointer(ngclExecutableList_t *list)
{
    /* Initialize the pointers */
    list->ngel_head = NULL;
    list->ngel_tail = NULL;
}

/**
 * User list: Register
 */
int
ngclExecutableUserListRegister(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *executable,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableUserListRegister";

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

    result = ngcllExecutableUserListRegister(
	context, list, executable, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableUserListRegister(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableUserListRegister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    log = context->ngc_log;

    /* Is list valid? */
    if (list == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "List is NULL.\n"); 
	return 0;
    }

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
        return 0;
    }

    /* Is user list valid? */
    if (executable->nge_apiNext != NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "User list is not valid.\n"); 
	return 0;
    }

    /* Executable is not registered */
    if (list->ngel_head == NULL) {
    	/* Is tail valid? */
	if (list->ngel_tail != NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Tail is not NULL.\n"); 
	    return 0;
	}

	/* Register */
	list->ngel_head = executable;
	list->ngel_tail = executable;
	executable->nge_apiNext = NULL;
    }

    /* Append at last of list */
    else {
    	/* Is tail valid? */
    	if (list->ngel_tail == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Tail is NULL.\n"); 
	    return 0;
	}

	/* Is user list valid? */
	if (list->ngel_tail->nge_apiNext != NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "User list is not valid.\n"); 
	    return 0;
	}

	/* Register */
	list->ngel_tail->nge_apiNext = executable;
	list->ngel_tail = executable;
	executable->nge_apiNext = NULL;
    }

    /* Success */
    return 1;
}

/**
 * User list: Unregister
 */
int
ngclExecutableUserListUnregister(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *executable,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableUserListUnregister";

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

    result = ngcllExecutableUserListUnregister(
	context, list, executable, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableUserListUnregister(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclExecutable_t *prev, *curr;
    static const char fName[] = "ngcllExecutableUserListUnregister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    log = context->ngc_log;

    /* Is list valid? */
    if (list == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "List is NULL.\n"); 
	return 0;
    }

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
        return 0;
    }

    /* Are any Executables exist? */
    if ((list->ngel_head == NULL) || (list->ngel_tail == NULL)) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not exist.\n"); 
	return 0;
    }

    /* Find the Executable */
    prev = NULL;
    curr = list->ngel_head;
    for (; curr != executable; curr = curr->nge_apiNext) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Executable */
    if (executable == list->ngel_head)
	list->ngel_head = executable->nge_apiNext;
    if (executable == list->ngel_tail)
    	list->ngel_tail = prev;
    if (prev != NULL)
    	prev->nge_apiNext = executable->nge_apiNext;
    executable->nge_apiNext = NULL;

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found.\n"); 
    return 0;
}

/**
 * User list: Get the next Executable.
 *
 * Return the Executable from the top of list, if current is NULL.
 * Return the next Executable of current, if current is not NULL.
 */
ngclExecutable_t *
ngclExecutableUserListGetNext(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *current,
    int *error)
{
    int local_error, result;
    ngclExecutable_t* executable;
    static const char fName[] = "ngclExecutableUserListGetNext";

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

    executable = ngcllExecutableUserListGetNext(
	context, list, current, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return executable;
}

static ngclExecutable_t *
ngcllExecutableUserListGetNext(
    ngclContext_t *context,
    ngclExecutableList_t *list,
    ngclExecutable_t *current,
    int *error)
{
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableUserListGetNext";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    log = context->ngc_log;

    /* Is list valid? */
    if (list == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "List is NULL.\n"); 
	return NULL;
    }

    /* Get the executable */
    return (current == NULL) ? list->ngel_head : current->nge_apiNext;
}

/**
 * multi handle list: Get the next Executable.
 */
ngclExecutable_t *
ngclExecutableMultiHandleListGetNext(
    ngclContext_t *context,
    ngclExecutable_t *current,
    int *error)
{
    int local_error, result;
    ngclExecutable_t* executable;
    static const char fName[] = "ngclExecutableMultiHandleListGetNext";

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

    executable = ngcllExecutableMultiHandleListGetNext(
	context, current, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return executable;
}

static ngclExecutable_t *
ngcllExecutableMultiHandleListGetNext(
    ngclContext_t *context,
    ngclExecutable_t *current,
    int *error)
{
    static const char fName[] = "ngcllExecutableMultiHandleListGetNext";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (current == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "current executable is NULL.\n"); 
	return NULL;
    }

    /* Get the executable */
    return current->nge_multiHandleNext;
}

/**
 * Get the copy of Executable by ID.
 */
int
ngclExecutableGetCopy(
    ngclContext_t *context,
    int executableID,
    ngclExecutable_t *executable,
    int *error)
{
    int local_error, result;
    ngclExecutable_t *exec;
    static const char fName[] = "ngclExecutableGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        NGI_SET_ERROR(error, local_error);
        return 0;
    }

    /* Lock the list */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Get */
    exec = ngclExecutableGet(context, executableID, error);
    if (exec == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Executable is not found.\n"); 
	goto error;
    }

    /* Copy */
    result = ngclExecutableCopy(context, exec, executable, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't copy the Executable.\n"); 
	goto error;
    }

    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Copy the Executable.
 */
int
ngclExecutableCopy(
    ngclContext_t *context,
    ngclExecutable_t *src,
    ngclExecutable_t *dest,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableCopy";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
	return 0;
    }

    result = ngcllExecutableCopy(context, src, dest, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableCopy(
    ngclContext_t *context,
    ngclExecutable_t *src,
    ngclExecutable_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngcllExecutableCopy";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, src, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Executable is not valid.\n"); 
	return 0;
    }

    /* Copy */
    *dest = *src;

    /* Clear the pointers */
    ngcllExecutableInitializePointer(dest);

    /* Success */
    return 1;
}

/**
 * Release the Executable.
 */
int
ngclExecutableRelease(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Do nothing */
    return 1;
}

/**
 * Get the Executable by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclExecutable_t *
ngclExecutableGet(ngclContext_t *context, int executableID, int *error)
{
    int local_error, result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngclExecutableGet";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    executable = ngcllExecutableGet(context, executableID, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return executable;
}

static ngclExecutable_t *
ngcllExecutableGet(ngclContext_t *context, int executableID, int *error)
{
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllExecutableGet";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is ID less than minimum? */
    if (executableID < NGI_EXECUTABLE_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "ID number %d is less than %d.\n", executableID, NGI_EXECUTABLE_ID_MIN); 
	goto error;
    }

    /* Is ID greater than maximum? */
    if (executableID > NGI_EXECUTABLE_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "ID number %d is greater than %d.\n", executableID, NGI_EXECUTABLE_ID_MAX); 
	goto error;
    }

    executable = context->ngc_executable_head;
    for (; executable != NULL; executable = executable->nge_next) {
	assert(executable->nge_ID >= NGI_EXECUTABLE_ID_MIN);
	assert(executable->nge_ID <= NGI_EXECUTABLE_ID_MAX);
	if (executable->nge_ID == executableID) {
	    /* Found */
	    return executable;
	}
    }

error:
    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    return NULL;
}

/**
 * Get the next Executable.
 *
 * Return the Executable from the top of list, if current is NULL.
 * Return the next Executable of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclExecutable_t *
ngclExecutableGetNext(
    ngclContext_t *context,
    ngclExecutable_t *current,
    int *error)
{
    int local_error, result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngclExecutableGetNext";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    executable = ngcllExecutableGetNext(context, current, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return executable;
}

static ngclExecutable_t *
ngcllExecutableGetNext(
    ngclContext_t *context,
    ngclExecutable_t *current,
    int *error)
{
    static const char fName[] = "ngcllExecutableGetNext";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (current == NULL) {
	/* Return the first Executable */
	if (context->ngc_executable_head != NULL) {
	    assert(context->ngc_executable_tail != NULL);
	    return context->ngc_executable_head;
	}
    } else {
	/* Return the next Executable */
	if (current->nge_next != NULL) {
	    return current->nge_next;
	}
    }

    /* The last Executable was reached */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Executable was reached.\n"); 

    return NULL;
}

static ngclExecutable_t *
ngcllExecutableListAndDestructionListGetNext(
    ngclContext_t *context,
    ngclExecutable_t *current,
    int *error)
{
    static const char fName[] = "ngcllExecutableListAndDestructionListGetNext";

    assert(context != NULL);

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (current == NULL) {
	/* Return the first Executable */
	if (context->ngc_executable_head != NULL) {
	    assert(context->ngc_executable_tail != NULL);
	    return context->ngc_executable_head;
	}
    } else {
        /* Return the next Executable */
        if (current->nge_next != NULL) {
            return current->nge_next;
        }
    }

    if (current == context->ngc_executable_tail && 
        context->ngc_destruction_executable_head != NULL) {
            assert(context->ngc_destruction_executable_tail != NULL);
            return context->ngc_destruction_executable_head;
    }

    /* The last Executable was reached */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Executable Memory Area was reached.\n"); 

    return NULL;
}

/**
 * Get the all Session list.
 */
ngclSession_t *
ngcliExecutableGetAllSessionList(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliExecutableGetAllSessionList";

    /* Check the arguments */

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
	return NULL;
    }
    context = executable->nge_context;

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_apiNext = NULL;
    prev = &dummy.ngs_apiNext;
    session = ngclSessionGetNext(executable, NULL, error);
    while (session != NULL) {
	/* Make the list of Session */
	*prev = session;
	prev = &session->ngs_apiNext;

	/* Get the next Session */
	session = ngclSessionGetNext(executable, session, error);
    }

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	    return NULL;
    }

    /* Success */
    return dummy.ngs_apiNext;
}

/**
 * Get the all Cancel Session list.
 */
ngclSession_t *
ngcliExecutableGetAllSessionCancelList(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliExecutableGetAllSessionCancelList";

    /* Check the arguments */

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	return NULL;
    }
    context = executable->nge_context;

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_cancelNext = NULL;
    prev = &dummy.ngs_cancelNext;
    session = ngclSessionGetNext(executable, NULL, error);
    while (session != NULL) {
	/* Make the list of Session */
	*prev = session;
	prev = &session->ngs_cancelNext;

	/* Get the next Session */
	session = ngclSessionGetNext(executable, session, error);
    }

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	    return NULL;
    }

    /* Success */
    return dummy.ngs_cancelNext;
}

/**
 * Get the Session list by ID.
 */
ngclSession_t *
ngcliExecutableGetSessionList(
    ngclExecutable_t *executable,
    int *sessionID,
    int nSessions,
    int *error)
{
    int result;
    int i;
    int flagSession = 0;
    ngclContext_t *context;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliExecutableGetSessionList";

    /* Check the arguments */
    assert(sessionID != NULL);
    assert(nSessions > 0);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
	return NULL;
    }
    context = executable->nge_context;

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	goto error;
    }
    flagSession = 1;

    /* Make the list of Session */
    dummy.ngs_apiNext = NULL;
    prev = &dummy.ngs_apiNext;
    for (i = 0; i < nSessions; i++) {
    	/* Get the Session by ID */
	session = ngclExecutableGetSession(executable, sessionID[i], error);
	if (session == NULL) {
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
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
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	    goto error;
    }

    /* Success */
    return dummy.ngs_apiNext;

    /* Error occurred */
error:
    /* Unlock the list */
    if (flagSession) {
	flagSession = 0;
	result = ngclSessionListReadUnlock(context, NULL);
	if (result == 0) {
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't lock the list of Session.\n"); 
	}
    }

    /* Failed */
    return NULL;
}

/**
 * Get the Session list by ID.
 */
ngclSession_t *
ngcliExecutableGetSessionCancelList(
    ngclExecutable_t *executable,
    int *sessionID,
    int nSessions,
    int *error)
{
    int result;
    int i;
    int flagSession = 0;
    ngclContext_t *context;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliExecutableGetSessionCancelList";

    /* Check the arguments */
    assert(sessionID != NULL);
    assert(nSessions > 0);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
	ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
	return NULL;
    }
    context = executable->nge_context;

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	goto error;
    }
    flagSession = 1;

    /* Make the list of Session */
    dummy.ngs_cancelNext = NULL;
    prev = &dummy.ngs_cancelNext;
    for (i = 0; i < nSessions; i++) {
    	/* Get the Session by ID */
	session = ngclExecutableGetSession(executable, sessionID[i], error);
	if (session == NULL) {
	    ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't the Session by ID %d.\n", sessionID[i]); 
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
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
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
	result = ngclSessionListReadUnlock(context, NULL);
	if (result == 0) {
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't lock the list of Session.\n"); 
	}
    }

    /* Failed */
    return NULL;
}

/**
 * Get the Session by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclSession_t *
ngclExecutableGetSession(
    ngclExecutable_t *executable,
    int sessionID,
    int *error)
{
    int local_error, result;
    ngclSession_t *session;
    static const char fName[] = "ngclExecutableGetSession";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    session = ngcllExecutableGetSession(executable, sessionID, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return session;
}

static ngclSession_t *
ngcllExecutableGetSession(
    ngclExecutable_t *executable,
    int sessionID,
    int *error)
{
    ngclContext_t *context;
    ngclSession_t *session;
    static const char fName[] = "ngcllExecutableGetSession";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    context = executable->nge_context;

    /* Is ID less than minimum? */
    if (sessionID < NGI_SESSION_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "ID number %d is less than %d.\n", sessionID, NGI_SESSION_ID_MIN); 
	goto error;
    }

    /* Is ID greater than maximum? */
    if (sessionID > NGI_SESSION_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "ID number %d is greater than %d.\n", sessionID, NGI_SESSION_ID_MAX); 
	goto error;
    }

    session = executable->nge_session_head;
    for (; session != NULL; session = session->ngs_next) {
	assert(session->ngs_ID >= NGI_SESSION_ID_MIN);
	assert(session->ngs_ID <= NGI_SESSION_ID_MAX);
	if (session->ngs_ID == sessionID) {
	    /* Found */
	    return session;
	}
    }

error:
    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    return NULL;
}

/**
 * Get the all Session list by waitNext.
 */
ngclSession_t *
ngcliExecutableGetAllSessionWaitList(
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclSession_t dummy, **prev, *session;
    static const char fName[] = "ngcliExecutableGetAllSessionWaitList";

    /* Check the arguments */
    assert(executable != NULL);

    /* Initialize the local variables */
    context = executable->nge_context;

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Session.\n"); 
	return NULL;
    }

    /* Make the list of Session */
    dummy.ngs_waitNext = NULL;
    prev = &dummy.ngs_waitNext;
    session = ngcliSessionGetNext(executable, NULL, error);
    while (session != NULL) {
	/* Make the list of Session */
	*prev = session;
	prev = &session->ngs_waitNext;

	/* Get the next Session */
	session = ngcliSessionGetNext(executable, session, error);
    }

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
	    ngclLogFatalExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Session.\n"); 
	    return NULL;
    }

    /* Success */
    return dummy.ngs_waitNext;
}

/**
 * ExecutableAttribute Initialize
 */
int
ngclExecutableAttributeInitialize(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllExecutableAttributeInitialize(context, execAttr, error);
}

static int
ngcllExecutableAttributeInitialize(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableAttributeInitialize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (execAttr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The Executable Attribute is NULL.\n"); 
        return 0;
    }

    ngcllExecutableAttributeInitializeMember(execAttr);

    /* Success */
    return 1;
}

static void
ngcllExecutableAttributeInitializeMember(
    ngclExecutableAttribute_t *execAttr)
{
    assert(execAttr != NULL);

    ngcllExecutableAttributeInitializePointer(execAttr);

    execAttr->ngea_portNo =
        execAttr->ngea_invokeNjobs =
        execAttr->ngea_jobStartTimeout =
        execAttr->ngea_jobStopTimeout =
        execAttr->ngea_mpiNcpus =
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED;
    execAttr->ngea_waitArgumentTransfer = NG_ARGUMENT_TRANSFER_UNDEFINED;
}

static void
ngcllExecutableAttributeInitializePointer(
    ngclExecutableAttribute_t *execAttr)
{
    assert(execAttr != NULL);

    execAttr->ngea_hostName = NULL;
    execAttr->ngea_jobManager = NULL;
    execAttr->ngea_subject = NULL;
    execAttr->ngea_className = NULL;
    execAttr->ngea_queueName = NULL;
}

/**
 * ExecutableAttribute Finalize
 */
int
ngclExecutableAttributeFinalize(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllExecutableAttributeFinalize(context, execAttr, error);
}

static int
ngcllExecutableAttributeFinalize(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllExecutableAttributeFinalize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (execAttr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The Executable Attribute is NULL.\n"); 
        return 0;
    }

    if (execAttr->ngea_hostName != NULL) {
        ngiFree(execAttr->ngea_hostName, log, error);
    }
    execAttr->ngea_hostName = NULL;

    if (execAttr->ngea_jobManager != NULL) {
        ngiFree(execAttr->ngea_jobManager, log, error);
    }
    execAttr->ngea_jobManager = NULL;

    if (execAttr->ngea_subject != NULL) {
        ngiFree(execAttr->ngea_subject, log, error);
    }
    execAttr->ngea_subject = NULL;

    if (execAttr->ngea_className != NULL) {
        ngiFree(execAttr->ngea_className, log, error);
    }
    execAttr->ngea_className = NULL;

    if (execAttr->ngea_queueName != NULL) {
        ngiFree(execAttr->ngea_queueName, log, error);
    }
    execAttr->ngea_queueName = NULL;

    ngcllExecutableAttributeInitializeMember(execAttr);

    /* Success */
    return 1;
}

/**
 * It checks whether attributes is valid.
 */
static int
ngcllExecutableCheckAttribute(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    int *error)
{
    static const char fName[] = "ngcllExecutableCheckAttribute";
    static const char eMsg[] = "Invalid attribute";

    if (execAttr == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: attribute is NULL.\n", eMsg); 
	return 0;
    }

    /**
     * Note: Undefined is not allowed for member
     *        ngea_className, ngea_invokeNjobs.
     */

    /* Is host name valid? */
    if ((execAttr->ngea_hostName != NULL) &&
	(execAttr->ngea_hostName[0] == '\0')) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Length of host name is less equal zero.\n", eMsg); 
	return 0;
    }

    /* Is port number less than minimum? */
    if ((execAttr->ngea_portNo != NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) &&
        (execAttr->ngea_portNo < NGI_PORT_MIN)) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Port number of remote machine %d is less than minimum %d.\n",
            eMsg, execAttr->ngea_portNo, NGI_PORT_MIN); 
	return 0;
    }

    /* Is port number greater than maximum? */
    if (execAttr->ngea_portNo > NGI_PORT_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Port number of remote machine %d is greater than maximum %d.\n",
            eMsg, execAttr->ngea_portNo, NGI_PORT_MAX); 
	return 0;
    }

    /* execAttr->ngea_jobManager can be both NULL or not. */
    /* execAttr->ngea_subject can be both NULL or not. */

    /* Is class name of remote class NULL? */
    if (execAttr->ngea_className == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Class name of remote class is NULL.\n", eMsg); 
	return 0;
    }

    /* Is length of class name less equal zero? */
    if (execAttr->ngea_className[0] == '\0') {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Length of class name is less equal zero.\n", eMsg); 
	return 0;
    }

    /* Is number of jobs which invoke defined? */
    if (execAttr->ngea_invokeNjobs ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Number of jobs which invoke is undefined.\n", eMsg); 
	return 0;
    }

    /* Is number of jobs which invoke less equal zero? */
    if (execAttr->ngea_invokeNjobs <= 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Number of jobs which invoke %d is less equal zero.\n",
            eMsg, execAttr->ngea_invokeNjobs); 
	return 0;
    }

    /* Is timeout time at start less than zero? */
    if ((execAttr->ngea_jobStartTimeout !=
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) &&
        (execAttr->ngea_jobStartTimeout < 0)) {

	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Timeout time %d at the time of a Job start is less than zero\n",
            eMsg, execAttr->ngea_jobStartTimeout); 
	return 0;
    }

    /* Is timeout time at stop less than zero? */
    if ((execAttr->ngea_jobStopTimeout !=
            NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) &&
        (execAttr->ngea_jobStopTimeout !=
            NGCL_EXECUTABLE_ATTRIBUTE_JOB_STOP_TIMEOUT_WAIT_FOREVER) &&
        (execAttr->ngea_jobStopTimeout < 0)) {

        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s: Timeout time %d at the time of a Job stop is less than zero\n",
            eMsg, execAttr->ngea_jobStopTimeout); 
        return 0;
    }

    /* Is argument transfer valid? */
    switch (execAttr->ngea_waitArgumentTransfer) {
    case NG_ARGUMENT_TRANSFER_UNDEFINED:
    case NG_ARGUMENT_TRANSFER_WAIT:
    case NG_ARGUMENT_TRANSFER_NOWAIT:
    case NG_ARGUMENT_TRANSFER_COPY:
	break;	/* Do nothing */

    default:
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Wait for completion of transfer argument.\n", eMsg); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Create the Executable ID.
 */
static int
ngcllExecutableCreateID(ngclContext_t *context, int *error)
{
    int result;
    int newID;
    static const char fName[] = "ngcllExecutableCreateID";

    /* Lock this instance */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Ninf-G Context.\n"); 
        return -1;
    }

    /* Get the new ID, which is not used. */
    newID = context->ngc_executableID;

    newID++;
    if (newID > NGI_EXECUTABLE_ID_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "There is no IDs for Ninf-G Executable to use.\n"); 

        /* Unlock this instance */
        result = ngclContextWriteUnlock(context, error);
        if (result == 0) {
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Ninf-G Context.\n"); 
        }
        return -1;
    }

    context->ngc_executableID = newID;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Ninf-G Context.\n"); 
        return -1;
    }

    /* Success */
    return newID;
}

/**
 * Executable: Check if the Remote Class Information exists.
 * If not, set the executable status to Query Function Information.
 */
int
ngcliExecutableRemoteClassInformationCheck(
    ngclExecutable_t *executable,
    int *doQueryFunctionInformation,
    int *error)
{
    ngLog_t *log;
    int result, found;
    ngclContext_t *context;
    static const char fName[] = "ngcliExecutableRemoteClassInformationCheck";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);
    assert(executable->nge_rcInfoName != NULL);
    assert(doQueryFunctionInformation != NULL);

    *doQueryFunctionInformation = 0;

    /* Is Remote Class Information already Exist? */
    if (executable->nge_rcInfoExist != 0) {

        /* Success */
        return 1;
    }

    /* Initialize the local variables */
    context = executable->nge_context;
    log = context->ngc_log;

    /* Set the start time */
    result = ngiSetStartTime(
        &executable->nge_executionTime.nge_queryRemoteClassInformation, 
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start time.\n"); 
        return 0;
    }

    /* Retry to get the Remote Class Information (not found is valid) */
    result = ngcllExecutableRemoteClassInformationCacheGet(
        context, executable, executable->nge_rcInfoName,
        0, &found, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get Remote Class Information.\n"); 
        return 0;
    }

    /* Was Remote Class Information found? */
    if (found != 0) {
        assert(executable->nge_rcInfoExist != 0);

        /* Set the end time */
        result = ngiSetEndTime(
            &executable->nge_executionTime.nge_queryRemoteClassInformation, 
            log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the End time.\n"); 
            return 0;
        }

        /* Notify */
        result = ngcliExecutableRemoteClassInformationNotify(
            executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't notify the Remote Class Information available.\n"); 
            return 0;
        }

        /* Success */
        return 1;
    }

    *doQueryFunctionInformation = 1;

    /* Reserve to request QueryFunctionInformation */
    result = ngcliExecutableStatusSet(executable,
        NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST,
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Executable can't process the Remote Class request.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Executable: Remote Class Information was arrived
 *   via Query Function Information
 */
int
ngcliExecutableRemoteClassInformationArrived(
    ngclExecutable_t *executable,
    char *rcInfoString,
    int *error)
{
    int result;
    ngLog_t *log;
    int found,  subError;
    ngclContext_t *context;
    ngRemoteClassInformation_t rcInfo;
    static const char fName[] = "ngcliExecutableRemoteClassInformationArrived";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);
    assert(executable->nge_rcInfoName != NULL);
    assert(rcInfoString != NULL);

    /* Initialize the local variables */
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);
    context = executable->nge_context;
    log = context->ngc_log;

    result = ngcliRemoteClassInformationInitialize(context,
        &rcInfo, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Remote Class Information.\n"); 
        return 0;
    }

    /* Generate the Remote Class Information */
    result = ngcliRemoteClassInformationGenerate(
       context, rcInfoString, strlen(rcInfoString), &rcInfo, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Generate the Remote Class Information failed.\n"); 
        return 0;
    }

    /* Retry to get the Remote Class Information (not found is valid) */
    result = ngcllExecutableRemoteClassInformationCacheGet(
        context, executable, executable->nge_rcInfoName,
        0, &found, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get Remote Class Information.\n"); 
        return 0;
    }

    /* Was Remote Class Information found? */
    if (found == 0) {
        /* Register the Remote Class Information */
        result = ngcliRemoteClassInformationCacheRegister(
            context, &rcInfo, &subError);
        if (result == 0) {
            ngclLogWarnExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Remote Class Information.\n"); 
        }

        result = ngcliRemoteClassInformationCopy(context,
            &rcInfo, &executable->nge_rcInfo, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy the Remote Class Information.\n"); 
            return 0;
        }
        executable->nge_rcInfoExist = 1;
    }

    /* Release the Remote Class Information */
    result = ngclRemoteClassInformationRelease(context, &rcInfo, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,
            "Can't release the Remote Class Information.\n"); 
        return 0;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &executable->nge_executionTime.nge_queryRemoteClassInformation, 
        log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End time.\n"); 
        return 0;
    }

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_DONE,
        log, error); 
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Notify */
    result = ngcliExecutableRemoteClassInformationNotify(
        executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't notify the Remote Class Information available.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Executable: Get the Information.
 */
int
ngclExecutableGetInformation(
    ngclExecutable_t *executable,
    ngclExecutableInformation_t *information,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableGetInformation";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
	return 0;
    }

    /* Check the arguments */
    if (information == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "The Executable Information is NULL.\n"); 
        return 0;
    }

    result = ngcllExecutableGetInformation(
	executable, information, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableGetInformation(
    ngclExecutable_t *executable,
    ngclExecutableInformation_t *information,
    int *error)
{
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllExecutableGetInformation";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (executable != NULL);

    /* Get the log */
    log = executable->nge_context->ngc_log;

    /* Print the information */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Get Executable Information.\n"); 

    /* Lock the Executable with send */
    result = ngcliExecutableLockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        return 0;
    }

#if 0 /* Temporary comment out */
    ngclExecutableInformation_t *info;

    /* Unlock the Executable */
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        return 0;
    }

    /* Send the request of QueryExecutableInformation */
    result = ngcliProtocolRequestQueryExecutableInformation(
        executable, executable->nge_protocol, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send the Query Executable Information.\n"); 
        return 0;
    }

    /* Wait the Reply */
    result = ngcliExecutableStatusWait(executable,
        NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait the Query Executable Information reply.\n"); 
        return 0;
    }

    /**
     * How can I get the ExecutableInformation info?
     */

    /* Copy the Executable Information */
    *information = *info;

    /* Release the Executable Information */
    result = ngiProtocolReleaseData(info, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the Executable Information.\n"); 
        return 0;
    }

    /* Set the status */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_IDLE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }
#endif /* Temporary comment out */

    /* Success */
    return 1;
}

/**
 * Executable: Reset
 */
int
ngclExecutableReset(
    ngclExecutable_t *executable,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclExecutableReset";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Executable is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
	return 0;
    }

    result = ngcllExecutableReset(executable, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllExecutableReset(
    ngclExecutable_t *executable,
    int *error)
{
    ngLog_t *log;
    int result;
    static const char fName[] = "ngcllExecutableReset";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (executable != NULL);

    /* Get the log */
    log = executable->nge_context->ngc_log;

    /* Print the information */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Reset Executable.\n"); 

    /* Lock the Executable with send */
    result = ngcliExecutableLockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        return 0;
    }

    /* Unlock the Executable */
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        return 0;
    }

    /* Send the request of reset */
    result = ngcliProtocolRequestResetExecutable(
        executable, executable->nge_protocol, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send the Reset Executable.\n"); 
        return 0;
    }

    /* Wait the Reply */
    result = ngcliExecutableStatusWait(executable,
        NG_EXECUTABLE_STATUS_RESET_DONE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait the Reset Executable reply.\n"); 
        return 0;
    }

    /* Set the status */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_IDLE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Executable: Initialize HeartBeat last arrived time.
 */
int
ngcliExecutableHeartBeatTimeInitialize(
    ngclExecutable_t *executable,
    int *error)
{
    /* Check the arguments */
    assert(executable != NULL);

    executable->nge_heartBeatLastReceived = time(NULL);
    executable->nge_heartBeatStatus = NG_HEART_BEAT_STATUS_OK;

    /* Success */
    return 1;
}

/**
 * Executable: HeartBeat arrived
 */
int
ngcliExecutableHeartBeatArrive(
    ngclExecutable_t *executable,
    int *error)
{
    /* Check the arguments */
    assert(executable != NULL);

    /* No log output, because chunk callback is called many times. */

    executable->nge_heartBeatLastReceived = time(NULL);

    /* Success */
    return 1;
}

/**
 * Executable status to string.
 */
static char *
ngcllExecutableStatusToString(ngclExecutableStatus_t status)
{
    switch (status) {

    case NG_EXECUTABLE_STATUS_INITIALIZED:
        return "INITIALIZED";

    case NG_EXECUTABLE_STATUS_CONNECTING:
        return "CONNECTING";

    case NG_EXECUTABLE_STATUS_CONNECTED:
        return "CONNECTED";

    case NG_EXECUTABLE_STATUS_IDLE:
        return "IDLE";

    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_WANT_REQUEST:
        return "QUERY FUNCTION INFORMATION WANT REQUEST";

    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED:
        return "QUERY FUNCTION INFORMATION REQUESTED";

    case NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_DONE:
        return "QUERY FUNCTION INFORMATION DONE";

    case NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_REQUESTED:
        return "QUERY EXECUTABLE INFORMATION REQUESTED";

    case NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE:
        return "QUERY EXECUTABLE INFORMATION DONE";

    case NG_EXECUTABLE_STATUS_RESET_REQUESTED:
        return "RESET REQUESTED";

    case NG_EXECUTABLE_STATUS_RESET_DONE:
        return "RESET DONE";

    case NG_EXECUTABLE_STATUS_EXIT_REQUESTED:
        return "EXIT REQUESTED";

    case NG_EXECUTABLE_STATUS_EXIT_DONE:
        return "EXIT DONE";

    case NG_EXECUTABLE_STATUS_DONE:
        return "DONE";
    }

    return "Unknown status";
}

/**
 * Is Executable IDLE?
 */
int
ngclExecutableIsIdle(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngLog_t *log;
    ngclExecutableStatus_t status;
    static const char fName[] = "ngclExecutableIsIdle";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    log = context->ngc_log;

    /* yield to accept connectback from Ninf-G Executable */
    result = ngiThreadYield(context->ngc_event, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(
            executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Thread yield failed.\n"); 
        return 0;
    }

    /* Check its status */
    status = executable->nge_status;
    if (status == NG_EXECUTABLE_STATUS_IDLE) {
        /* the executable is ready */
        return 1;
    } else {
        /* the executable is not ready */
        return 0;
    }
}

/**
 * get executable ID
 */
int
ngclExecutableGetID(
    ngclExecutable_t *executable,
    int *error)
{
    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    assert(executable != NULL);

    /* return ID of executable */
    return executable->nge_ID;
}

/**
 * Get Local Machine Information: fortran_compatible.
 */
int
ngcliExecutableGetLocalMachineInformationFortranCompatible(
    ngclExecutable_t *executable,
    int *fortranCompatible,
    int *error)
{
    int result;
    static const char fName[] = "ngcliExecutableGetLocalMachineInformationFortranCompatible";

    /* Check the arguments */
    assert(executable != NULL);
    assert(fortranCompatible != NULL);

    /* Get Local Machine Information: fortran_compatible */
    result = ngcliJobGetLocalMachineInformationFortranCompatible(
	executable->nge_jobMng, fortranCompatible, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't get the Fortran Compatible.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}
