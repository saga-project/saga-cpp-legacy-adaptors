/*
 * $RCSfile: ngclJob.c,v $ $Revision: 1.26 $ $Date: 2008/03/28 09:32:36 $
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
 * Module of Job management for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclJob.c,v $ $Revision: 1.26 $ $Date: 2008/03/28 09:32:36 $")

/**
 * Prototype declaration of static functions.
 */
static int ngcllJobInitialize(
    ngclContext_t *, ngcliJobManager_t *, ngcliJobAttribute_t *, int *);
static int ngcllJobFinalize(ngcliJobManager_t *, int *);
static void ngcllJobInitializeMember(ngcliJobManager_t *);
static void ngcllJobInitializeFlag(ngcliJobManager_t *);
static void ngcllJobInitializePointer(ngcliJobManager_t *);
static int ngcllJobInitializeMutex(ngcliJobManager_t *, ngLog_t *, int *);
static int ngcllJobFinalizeMutex(ngcliJobManager_t *, ngLog_t *, int *);
static int ngcllJobCreateID(ngclContext_t *, int *);
static void ngcllJobAttributeInitializeMember(ngcliJobAttribute_t *);
static void ngcllJobAttributeInitializeFlag(ngcliJobAttribute_t *);
static void ngcllJobAttributeInitializePointer(ngcliJobAttribute_t *);

static int ngcllJobWaitDone(ngcliJobManager_t *, ngLog_t *, int *);
static int ngcllJobWaitDoneTimeout(
    ngcliJobManager_t *, int, ngLog_t *, int *);

/**
 * Construct
 */
ngcliJobManager_t *
ngcliJobConstruct(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngcliJobAttribute_t *jobAttr,
    int *error)
{
    int result;
    int initialized = 0;
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcliJobConstruct";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobAttr != NULL);

    /* Allocate */
    jobMng = NGI_ALLOCATE(ngcliJobManager_t, context->ngc_log, error);
    if (jobMng == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't allocate the storage for Job Manager.\n"); 
        goto error;
    }

    /* Initialize */
    result = ngcllJobInitialize(context, jobMng, jobAttr, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Job Manager.\n"); 
	goto error;
    }
    initialized = 1;

    /* Register */
    result = ngcliContextRegisterJobManager(context, jobMng, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the Job Manager to Ninf-G Context.\n"); 
	goto error;
    }

    /* Success */
    return jobMng;

    /* Error occurred */
error:
    /* Finalize */
    if (initialized != 0) {
        result = ngcllJobFinalize(jobMng, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the Job Manager.\n"); 
        }
        initialized = 0;
    }

    /* Deallocate */
    if (jobMng != NULL) {
        result = NGI_DEALLOCATE(
            ngcliJobManager_t, jobMng, context->ngc_log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't deallocate the storage for Job Manager.\n"); 
        }
        jobMng = NULL;
    }
    return NULL;
}

/**
 * Destruct
 */
int
ngcliJobDestruct(ngcliJobManager_t *jobMng, int *error)
{
    int result;
    int ret = 1;
    ngclContext_t *context;
    static const char fName[] = "ngcliJobDestruct";

    /* Check the arguments */
    assert(jobMng != NULL);

    assert(jobMng->ngjm_executable_head == NULL);
    assert(jobMng->ngjm_executable_tail == NULL);

    /* Stop the Job */
    result = ngcliJobStop(jobMng, NULL);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't stop the Job.\n"); 
        /* If ngcliJobStop() fails, ngcliJobDestruct() doesn't fail. */
    }

    /* Get the Ninf-G Context */
    context = jobMng->ngjm_context;

    /* Unegister */
    result = ngcliContextUnregisterJobManager(context, jobMng, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the Job Manager.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Finalize */
    result = ngcllJobFinalize(jobMng, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Job Manager.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngcliJobManager_t, jobMng, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the storage for Job Manager.\n"); 
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * Initialize
 */
static int
ngcllJobInitialize(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    ngcliJobAttribute_t *jobAttr,
    int *error)
{
    long randomNo;
    int result;
    int jobID;
    static const char fName[] = "ngcllJobInitialize";
    
    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng  != NULL);
    assert(jobAttr != NULL);
    assert(jobAttr->ngja_hostName != NULL);
    assert(jobAttr->ngja_hostName[0] != '\0');
    assert(jobAttr->ngja_portNo >= NGI_PORT_MIN);
    assert(jobAttr->ngja_portNo <= NGI_PORT_MAX);
    assert(jobAttr->ngja_executablePath != NULL);
    assert(jobAttr->ngja_executablePath[0] != '\0');
    assert(jobAttr->ngja_invokeNjobs > 0);
    assert(jobAttr->ngja_startTimeout >= 0);

    /* Initialize the members */
    ngcllJobInitializeMember(jobMng);
    jobMng->ngjm_nExecutables = jobAttr->ngja_invokeNjobs;
    jobMng->ngjm_status = NGI_JOB_STATUS_INITIALIZING;
    jobMng->ngjm_context = context;

    /* Initialize the Invoke Server Job Info */
    result = ngcliInvokeServerJobInitialize(
        context, &jobMng->ngjm_invokeServerInfo, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Invoke Server Job Information.\n"); 
	goto error;
    }

    /* Initialize the Mutex, Cond and Read/Write lock */
    result = ngcllJobInitializeMutex(jobMng, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize Mutex, Cond and Read/Write lock for Job Manager.\n"); 
	goto error;
    }

    /* Create the Job ID */
    jobID = ngcllJobCreateID(context, error);
    if (jobID < 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't create the Job ID.\n"); 
	goto error;
    }
    jobMng->ngjm_ID = jobID;

    /* Copy the Job Attribute */
    result = ngcliJobAttributeCopy(
    	context, jobAttr, &jobMng->ngjm_attr, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't copy the Job Attribute.\n"); 
	goto error;
    }

    /* Use Invoke Server? */
    jobMng->ngjm_useInvokeServer = 0;
    if ((jobMng->ngjm_attr.ngja_rmInfoExist != 0) &&
        (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_invokeServerType != NULL)) {
        assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
        assert(jobMng->ngjm_attr.ngja_isInfo.ngisi_type != NULL);
        jobMng->ngjm_useInvokeServer = 1; /* Use Invoke Server */
    }
    if (jobMng->ngjm_useInvokeServer == 0) {
        /* Note : internal job system is not ready. */

        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Invoke Server is not used.\n"); 
	goto error;
    }

    /* Decide Simple Auth Number. */
    randomNo = 0;
    result = ngcliContextRandomNumberGet(context, &randomNo, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't get the Random Number.\n"); 
	goto error;
    }
    randomNo = ((randomNo - NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN) %
	(NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX
        - NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN))
	+ NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN;
    jobMng->ngjm_simpleAuthNumber = (int)randomNo;

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcllJobFinalize(jobMng, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't Finalize the Job Manager.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Finalize
 */
static int
ngcllJobFinalize(ngcliJobManager_t *jobMng, int *error)
{
    int result;
    int ret = 1;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngcllJobFinalize";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */
    context = jobMng->ngjm_context;
    log = context->ngc_log;

    /* Communication Proxy Unref */
    if (jobMng->ngjm_commProxyID > 0) {
        result = ngcliCommunicationProxyManagerUnref(
            jobMng->ngjm_context->ngc_communicationProxyManager,
            jobMng->ngjm_commProxyID, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unref the Communication Proxy.\n"); 
            error = NULL;
            ret = 0;
        }
    }
    if (jobMng->ngjm_clientCommunicationProxyInfo != NULL) {
        result = ngiLineListDestruct(
            jobMng->ngjm_clientCommunicationProxyInfo, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct client communication proxy information.\n"); 
            error = NULL;
            ret = 0;
        }
    }

    /* Release the Job Attribute */
    result = ngcliJobAttributeRelease(
	jobMng->ngjm_context, &jobMng->ngjm_attr, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't release the Job Attribute.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Finalize the Mutex, Condition variable and Read/Write lock */
    result = ngcllJobFinalizeMutex(jobMng, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Mutex, Condition variable and Read/Write lock.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Finalize the Invoke Server Job Info */
    result = ngcliInvokeServerJobFinalize(
        context, &jobMng->ngjm_invokeServerInfo, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Invoke Server Job Information.\n"); 
        error = NULL;
        ret = 0;
    }

    /* Initialize the members */
    ngcllJobInitializeMember(jobMng);
    jobMng->ngjm_status = NGI_JOB_STATUS_DESTROYED;

    return ret;
}

/**
 * Initialize the members.
 */
static void
ngcllJobInitializeMember(ngcliJobManager_t *jobMng)
{
    /* Initialize the flags and pointers */
    ngcllJobInitializeFlag(jobMng);
    ngcllJobInitializePointer(jobMng);

    /* Initialize the members */
    jobMng->ngjm_nExecutables = 0;
    jobMng->ngjm_requestCancel = 0;
    jobMng->ngjm_invokeMeasured = 0;

    jobMng->ngjm_monMutex = NGI_MUTEX_NULL;
    jobMng->ngjm_monCond  = NGI_COND_NULL;
    jobMng->ngjm_rwlOwn   = NGI_RWLOCK_NULL;
    jobMng->ngjm_commProxyID = 0;
}

/**
 * Initialize the flags.
 */
static void
ngcllJobInitializeFlag(ngcliJobManager_t *jobMng)
{
    /* Initialize the flags */
    jobMng->ngjm_useInvokeServer = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllJobInitializePointer(ngcliJobManager_t *jobMng)
{
    /* Initialize the pointers */
    jobMng->ngjm_next = NULL;
    jobMng->ngjm_context = NULL;
    jobMng->ngjm_executable_head = NULL;
    jobMng->ngjm_executable_tail = NULL;
    jobMng->ngjm_destruction_executable_head = NULL;
    jobMng->ngjm_destruction_executable_tail = NULL;
    jobMng->ngjm_clientCommunicationProxyInfo = NULL;
}

/**
 * Initialize the Mutex, Condition variable and Read/Write lock.
 */
static int
ngcllJobInitializeMutex(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result;
    ngclContext_t *context;
    static const char fName[] = "ngcllJobInitializeMutex";

    /* Check the arguments */
    assert(jobMng != NULL);

    context = jobMng->ngjm_context;

    /* Initialize the mutex for monitor */
    result = ngiMutexInitialize(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Mutex.\n"); 
	return 0;
    }

    /* Initialize the condition variable for monitor */
    result = ngiCondInitialize(
        &jobMng->ngjm_monCond, context->ngc_event, log, NULL);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Condition variable.\n"); 
	return 0;
    }

    /* Initialize the Read/Write Lock for this instance */
    result = ngiRWlockInitialize(&jobMng->ngjm_rwlOwn, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Read/Write.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize the Mutex, Condition variable and Read/Write lock.
 */
static int
ngcllJobFinalizeMutex(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcllJobFinalizeMutex";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Finalize the mutex for monitor */
    result = ngiMutexDestroy(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
        NGI_SET_ERROR (error, NG_ERROR_THREAD);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Mutex.\n"); 
        return 0;
    }

    /* Finalize the condition variable for monitor */
    result = ngiCondDestroy(&jobMng->ngjm_monCond, log, NULL);
    if (result == 0) {
        NGI_SET_ERROR (error, NG_ERROR_THREAD);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Condition variable.\n"); 
        return 0;
    }

    /* Finalize the Read/Write Lock for this instance */
    result = ngiRWlockFinalize(&jobMng->ngjm_rwlOwn, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Read/Write.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Notify the Job status.
 */
int
ngcliJobNotifyStatus(
    ngcliJobManager_t *jobMng,
    ngcliJobStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliJobNotifyStatus";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Lock */
    result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
	return 0;
    }

    /* Update the status */
    jobMng->ngjm_status = status;

    /* Signal */
    result = ngiCondSignal(&jobMng->ngjm_monCond, log, error);
    if (result == 0) {
    	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't signal the Condition Variable.\n"); 
	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Wait any status.
 */
int
ngcliJobWaitStatus(
    ngcliJobManager_t *jobMng,
    ngcliJobStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliJobWaitStatus";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Lock */
    result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Mutex.\n"); 
	return 0;
    }

    /* Wait the status */
    while (jobMng->ngjm_status < status) {
	result = ngiCondWait(
	    &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, log, error);
	if (result == 0) {
    	    ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
    	        "Can't wait the Condition Variable.\n"); 
	    goto error;
	}
    }

    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Is Job Manager valid?
 */
int
ngcliJobIsValid(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    ngLog_t *log;
    ngcliJobManager_t *curr;
    static const char fName[] = "ngcliJobIsValid";

    /* Is Job Manager NULL? */
    if (jobMng == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job Manager is NULL.\n"); 
	return 0;
    }

    /* Use Ninf-G Context which contained in Job Manager, if context is NULL */
    if (context == NULL) {
	context = jobMng->ngjm_context;
    }

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Get the Log Manager */
    log = context->ngc_log;

    /* Is ID smaller than minimum? */
    if (jobMng->ngjm_ID < NGI_JOB_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is smaller than minimum %d.\n",
            jobMng->ngjm_ID, NGI_JOB_ID_MIN); 
	return 0;
    }

    /* Is ID greater than maximum? */
    if (jobMng->ngjm_ID > NGI_JOB_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is greater than maximum %d.\n",
            jobMng->ngjm_ID, NGI_JOB_ID_MAX); 
	return 0;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Job Manager.\n"); 
	return 0;
    }

    /* Is Job Manager exist? */
    curr = ngcliContextGetNextJobManager(context, NULL, error);
    if (curr == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "No Job Manager is registered.\n"); 
	goto error;
    }

    while (curr != jobMng) {
	curr = ngcliContextGetNextJobManager(context, curr, error);
	if (curr == NULL) {
    	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	        "Job Manager is not found.\n"); 
	    goto error;
	}
    }

    /* Unlock the list of Job Manager */
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Job Manager.\n"); 
	return 0;
    }

    /* Job Manager is valid */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Job Manager */
    result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Job Manager.\n"); 
	return 0;
    }

    return 0;
}


/**
 * Set the end time of Job invoke.
 */
int
ngcliJobSetEndTimeOfInvoke(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result;
    int locked = 0;
    static const char fName[] = "ngcliJobSetEndTimeOfInvoke";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Lock the this instance */
    result = ngcliJobManagerWriteLock(jobMng, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Job manager.\n"); 
        return 0;
    }
    locked = 1;

    /* Is already measured? */
    if (jobMng->ngjm_invokeMeasured != 0) {
        ngcliLogInfoJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Job invoke time was already measured.\n"); 
    } else {
        jobMng->ngjm_invokeMeasured = 1;

        /* Set the end time */
        result = ngiSetEndTime(&jobMng->ngjm_invoke, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the End time.\n"); 
            goto error;
        }
    }

    /* Unlock the this instance */
    locked = 0;
    result = ngcliJobManagerWriteUnlock(jobMng, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Job manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the this instance */
    if (locked != 0) {
        locked = 0;
        result = ngcliJobManagerWriteUnlock(jobMng, log, NULL);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Job manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Create the Job ID.
 */
static int
ngcllJobCreateID(ngclContext_t *context, int *error)
{
    int result;
    int newID;
    ngcliJobManager_t *tmpJob;
    static const char fName[] = "ngcllJobCreateID";

    /* Lock the list of Job */
    result = ngcliContextJobManagerListWriteLock(
	context, context->ngc_log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Job.\n"); 
	return -1;
    }

    /**
     * Get the new ID, which is not used.
     */
    newID = context->ngc_jobID;

    newID++;
    if (newID > NGI_JOB_ID_MAX)
	newID = NGI_JOB_ID_MIN;

    /* Find the not used ID */
    while (newID != context->ngc_jobID) {
	tmpJob = ngcliContextGetJobManager(context, newID, error);
	if (tmpJob != NULL) {
	    /* newID is inuse */
	    newID++;
	    if (newID > NGI_JOB_ID_MAX)
		newID = NGI_JOB_ID_MIN;

	    continue;
	}

	/* Update the ID */
	context->ngc_jobID = newID;

	/* Unlock the list of Job */
	result = ngcliContextJobManagerListWriteUnlock(
	    context, context->ngc_log, NULL);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't unlock the list of Job.\n"); 
	    return -1;
	}

	/* Success */
	return newID;
    }

    /* All ID is used */
    NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,
        "There is no IDs for Executable to use.\n"); 

    /* Unlock the list of Job */
    result = ngcliContextJobManagerListWriteUnlock(
	context, context->ngc_log, NULL);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Job.\n"); 
	return -1;
    }

    /* Failed */
    return -1;
}

/**
 * Job Attribute: 
 */
int
ngcliJobAttributeInitialize(
    ngclContext_t *context,
    ngclExecutableAttribute_t *execAttr,
    ngcliJobAttribute_t *jobAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    char *jobManager, *subject, *clientHostName;
    char *queueName;
    int rcInfoListLocked;
    ngclRemoteMachineInformation_t *rmInfo;
    ngcliRemoteClassInformationManager_t *rcInfoMng;
    int sub_error;
    int ncpus;
    static const char fName[] = "ngcliJobAttributeInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(execAttr != NULL);
    assert(jobAttr != NULL);

    /* Initialize the local variables */
    rcInfoListLocked = 0;
    jobManager = NULL;
    queueName = NULL;
    subject = NULL;
    clientHostName = NULL;
    log = context->ngc_log;

    /* Initialize the members */
    ngcllJobAttributeInitializeMember(jobAttr);

    /* Get the Local Machine Information */
    result = ngclLocalMachineInformationGetCopy(
        context, &jobAttr->ngja_lmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Local Machine Information.\n"); 
	goto error;
    }
    jobAttr->ngja_lmInfoExist = 1;

    /* Get the Remote Machine Information */
    result = ngclRemoteMachineInformationGetCopy(
        context, execAttr->ngea_hostName, &jobAttr->ngja_rmInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Remote Machine Information.\n"); 
	goto error;
    }
    rmInfo = &jobAttr->ngja_rmInfo;
    jobAttr->ngja_rmInfoExist = 1;

    /* Get the Executable Path Information */
    result = ngcliExecutablePathInformationGetCopyWithQuery(context,
        rmInfo->ngrmi_hostName, execAttr->ngea_className,
        rmInfo->ngrmi_infoServiceTag,
        &jobAttr->ngja_epInfo, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Executable Path Information.\n"); 
	goto error;
    }
    jobAttr->ngja_epInfoExist = 1;

    {
        /* Lock the Remote Class Information */
        result = ngcliRemoteClassInformationListReadLock(
            context, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the list of Remote Class Information.\n"); 
            goto error;
        }
        rcInfoListLocked = 1;

        /* Get the Remote Class Information (not found is valid) */
        rcInfoMng = ngcliRemoteClassInformationCacheGet(
            context, execAttr->ngea_className, error);
        if (rcInfoMng != NULL) {
            result = ngcliRemoteClassInformationCopy(context,
                &rcInfoMng->ngrcim_info, &jobAttr->ngja_rcInfo, error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy the Remote Class Information.\n"); 
                goto error;
            }
            jobAttr->ngja_rcInfoExist = 1;

        }

        /* Unlock the Remote Class Information */
        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, error);
        rcInfoListLocked = 0;
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Class Information.\n"); 
            goto error;
        }
    }

    /* Initialize the attribute of job management */
    jobAttr->ngja_portNo = execAttr->ngea_portNo;
    if (execAttr->ngea_portNo ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_portNo = rmInfo->ngrmi_portNo;
    }

    /* TODO: priority level is no problem? */
    jobAttr->ngja_mpiNcpus = rmInfo->ngrmi_mpiNcpus;
    NGI_SET_ERROR(&sub_error, NG_ERROR_NO_ERROR);
    ncpus = ngcliRemoteMachineInformationGetMPInCPUs(rmInfo, 
        execAttr->ngea_className, log, &sub_error);
    if (ncpus < 0) {
        if (sub_error != NG_ERROR_NOT_EXIST) {
            assert(sub_error != NG_ERROR_NO_ERROR);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can'get MPI number of CPUs from Remote Machine Information.\n"); 
            goto error;
        }
    } else {
        /* Found */
        jobAttr->ngja_mpiNcpus = ncpus;
    }


    if (execAttr->ngea_mpiNcpus != NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_mpiNcpus = execAttr->ngea_mpiNcpus;
    }

    if (jobAttr->ngja_rcInfoExist != 0) {
        jobAttr->ngja_backend = jobAttr->ngja_rcInfo.ngrci_backend;
    }
    if (jobAttr->ngja_epInfo.ngepi_backend != NG_BACKEND_NORMAL) {
    	jobAttr->ngja_backend = jobAttr->ngja_epInfo.ngepi_backend;
    }

    if (jobAttr->ngja_backend != NG_BACKEND_NORMAL) {
        if (jobAttr->ngja_mpiNcpus <= 0) {
            NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "No of CPUs (%d) for MPI function \"%s\" hostname \"%s\""
                " is invalid.\n",
                jobAttr->ngja_mpiNcpus,
                execAttr->ngea_className, execAttr->ngea_hostName); 
            goto error;
        }
    }

    jobAttr->ngja_stagingEnable = jobAttr->ngja_epInfo.ngepi_stagingEnable;
    jobAttr->ngja_invokeNjobs = execAttr->ngea_invokeNjobs;

    jobAttr->ngja_startTimeout = execAttr->ngea_jobStartTimeout;
    if (execAttr->ngea_jobStartTimeout ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_startTimeout = rmInfo->ngrmi_jobStartTimeout;
    }
    jobAttr->ngja_stopTimeout = execAttr->ngea_jobStopTimeout;
    if (execAttr->ngea_jobStopTimeout ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_stopTimeout = rmInfo->ngrmi_jobEndTimeout;
    }

    jobAttr->ngja_forceXDR = rmInfo->ngrmi_forceXDR;

    /* Duplicate the host name */
    assert(rmInfo->ngrmi_hostName != NULL);
    jobAttr->ngja_hostName = ngiStrdup(rmInfo->ngrmi_hostName, log, error);
    if (jobAttr->ngja_hostName == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't duplicate the host name.\n"); 
	goto error;
    }

    /* Duplicate the Queue Name */
    queueName = NULL;
    if (rmInfo->ngrmi_jobQueue != NULL) {
        queueName = rmInfo->ngrmi_jobQueue;
    }
    if (execAttr->ngea_queueName != NULL) {
        queueName = execAttr->ngea_queueName;
    }
    if (queueName != NULL) {
        jobAttr->ngja_queueName = ngiStrdup(queueName, log, error);
        if (jobAttr->ngja_queueName == NULL) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the queue name.\n"); 
	    goto error;
        }
    }
    queueName = NULL;

    /* Duplicate the Job Manager */
    jobManager = NULL;
    if (rmInfo->ngrmi_jobManager != NULL) {
        jobManager = rmInfo->ngrmi_jobManager;
    }
    if (execAttr->ngea_jobManager != NULL) {
        jobManager = execAttr->ngea_jobManager;
    }
    if (jobManager != NULL) {
        jobAttr->ngja_jobManager = ngiStrdup(jobManager, log, error);
        if (jobAttr->ngja_jobManager == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the jobmanager.\n"); 
            goto error;
        }
    }
    jobManager = NULL;

    /* Duplicate the Subject */
    subject = NULL;
    if (rmInfo->ngrmi_subject != NULL) {
        subject = rmInfo->ngrmi_subject;
    }
    if (execAttr->ngea_subject != NULL) {
        subject = execAttr->ngea_subject;
    }
    if (subject != NULL) {
        jobAttr->ngja_subject = ngiStrdup(subject, log, error);
        if (jobAttr->ngja_subject == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the jobmanager.\n"); 
            goto error;
        }
    }
    subject = NULL;

    /* Duplicate the Client Host name */
    clientHostName = jobAttr->ngja_lmInfo.nglmi_hostName;
    if (rmInfo->ngrmi_clientHostName != NULL) {
        clientHostName = rmInfo->ngrmi_clientHostName;
    }
    if (clientHostName != NULL) {
        jobAttr->ngja_clientHostName = ngiStrdup(clientHostName, log, error);
        if (jobAttr->ngja_clientHostName == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the Client host name.\n"); 
            goto error;
        }
    } else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Client host name is NULL.\n"); 
        goto error;
    }
    clientHostName = NULL;

    /* Duplicate the Executable Path */
    jobAttr->ngja_executablePath =
        ngiStrdup(jobAttr->ngja_epInfo.ngepi_path, log, error);
    if (jobAttr->ngja_executablePath == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't duplicate the host name.\n"); 
	goto error;
    }

    /* Invoke Server */
    assert(jobAttr->ngja_rmInfoExist == 1);
    if (jobAttr->ngja_rmInfo.ngrmi_invokeServerType != NULL) {
        result = ngclInvokeServerInformationGetCopy(
            context, jobAttr->ngja_rmInfo.ngrmi_invokeServerType,
            &jobAttr->ngja_isInfo, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the Invoke Server Information.\n"); 
            goto error;
        }
        
        jobAttr->ngja_isInfoExist = 1;
    }

    /* Check the multipleness */
    if ((jobAttr->ngja_invokeNjobs > 1) &&
        (((jobAttr->ngja_backend == NG_BACKEND_MPI) ||
          (jobAttr->ngja_backend == NG_BACKEND_BLACS))) &&
         (jobAttr->ngja_mpiNcpus > 1)) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "multiple function handle for jobType=mpi is not supported"
            " for one job submission.\n"); 
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "executable = %d, mpi nodes = %d\n",
            jobAttr->ngja_invokeNjobs, jobAttr->ngja_mpiNcpus); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Remote Class Information */
    if (rcInfoListLocked != 0) {
        rcInfoListLocked = 0;
        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Remote Class Information.\n"); 
        }
    }

    /* Release */
    result = ngcliJobAttributeRelease(context, jobAttr, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Job Attribute.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Job Attribute: Finalize
 */
int
ngcliJobAttributeFinalize(
    ngclContext_t *context,
    ngcliJobAttribute_t *jobAttr,
    int *error)
{
    int result;
    static const char fName[] = "ngcliJobAttributeFinalize";

    /* Release */
    result = ngcliJobAttributeRelease(context, jobAttr, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't release the Job Attribute.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Job Attribute: Copy
 */
int
ngcliJobAttributeCopy(
    ngclContext_t *context,
    ngcliJobAttribute_t *src,
    ngcliJobAttribute_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngcliJobAttributeCopy";

    /* Copy the Job Attribute */
    *dest = *src;

    /* Initialize the flags and pointers */
    ngcllJobAttributeInitializeFlag(dest);
    ngcllJobAttributeInitializePointer(dest);

    /* Duplicate the host name */
    if (src->ngja_hostName != NULL) {
	dest->ngja_hostName = strdup(src->ngja_hostName);
	if (dest->ngja_hostName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the host name.\n"); 
	    goto error;
	}
    }

    /* Duplicate the queue name */
    if (src->ngja_queueName != NULL) {
        dest->ngja_queueName = strdup(src->ngja_queueName);
        if (dest->ngja_queueName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't duplicate the queue name.\n"); 
            goto error;
        }
    }

    /* Duplicate the Job Manager */
    if (src->ngja_jobManager != NULL) {
	dest->ngja_jobManager = strdup(src->ngja_jobManager);
	if (dest->ngja_jobManager == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the Job Manager.\n"); 
	    goto error;
        }
    }

    /* Duplicate the Subject */
    if (src->ngja_subject != NULL) {
	dest->ngja_subject = strdup(src->ngja_subject);
	if (dest->ngja_subject == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the Subject.\n"); 
	    goto error;
        }
    }

    /* Duplicate the Client Host name */
    if (src->ngja_clientHostName != NULL) {
	dest->ngja_clientHostName = strdup(src->ngja_clientHostName);
	if (dest->ngja_clientHostName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the Client Host name.\n"); 
	    goto error;
        }
    }

    /* Duplicate the Executable Path */
    if (src->ngja_executablePath != NULL) {
	dest->ngja_executablePath = strdup(src->ngja_executablePath);
	if (dest->ngja_executablePath == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't duplicate the Executable Path.\n"); 
	    goto error;
	}
    }

    /* Copy the Local Machine Information */
    if (src->ngja_lmInfoExist != 0) {
	result = ngcliLocalMachineInformationCopy(
	    context, &src->ngja_lmInfo, &dest->ngja_lmInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't copy the Local Machine Information.\n"); 
	    goto error;
	}
	dest->ngja_lmInfoExist = 1;
    }

    /* Copy the Invoke Server Information */
    if (src->ngja_isInfoExist != 0) {
	result = ngcliInvokeServerInformationCopy(
	    context, &src->ngja_isInfo, &dest->ngja_isInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't copy the Invoke Server Information.\n"); 
	    goto error;
	}
	dest->ngja_isInfoExist = 1;
    }

    /* Copy the Remote Machine Information */
    if (src->ngja_rmInfoExist != 0) {
	result = ngcliRemoteMachineInformationCopy(
	    context, &src->ngja_rmInfo, &dest->ngja_rmInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't copy the Remote Machine Information.\n"); 
	    goto error;
	}
	dest->ngja_rmInfoExist = 1;
    }

    /* Copy the Remote Class Information */
    if (src->ngja_rcInfoExist != 0) {
	result = ngcliRemoteClassInformationCopy(
	    context, &src->ngja_rcInfo, &dest->ngja_rcInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't copy the Remote Class Information.\n"); 
	    goto error;
	}
	dest->ngja_rcInfoExist = 1;
    }

    /* Copy the Executable Path Information */
    if (src->ngja_epInfoExist != 0) {
	result = ngcliExecutablePathInformationCopy(
	    context, &src->ngja_epInfo, &dest->ngja_epInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't copy the Executable Path Information.\n"); 
	    goto error;
	}
	dest->ngja_epInfoExist = 1;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngcliJobAttributeRelease(context, dest, NULL);

    /* Failed */
    return 0;
}

/**
 * Job Attribute: Release
 */
int
ngcliJobAttributeRelease(
    ngclContext_t *context,
    ngcliJobAttribute_t *jobAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcliJobAttributeRelease";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobAttr != NULL);

    log = context->ngc_log;

    /* Release the Local Machine Information */
    if (jobAttr->ngja_lmInfoExist != 0) {
	result = ngclLocalMachineInformationRelease(
	    context, &jobAttr->ngja_lmInfo, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Local Machine Information.\n"); 
	    return 0;
	}
    }
    jobAttr->ngja_lmInfoExist = 0;

    /* Release the Invoke Server Information */
    if (jobAttr->ngja_isInfoExist != 0) {
	result = ngclInvokeServerInformationRelease(
	    context, &jobAttr->ngja_isInfo, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Invoke Server Information.\n"); 
	    return 0;
	}
    }
    jobAttr->ngja_isInfoExist = 0;

    /* Release the Remote Machine Information */
    if (jobAttr->ngja_rmInfoExist != 0) {
	result = ngclRemoteMachineInformationRelease(
	    context, &jobAttr->ngja_rmInfo, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Remote Machine Information.\n"); 
	    return 0;
	}
    }
    jobAttr->ngja_rmInfoExist = 0;

    /* Release the Remote Class Information */
    if (jobAttr->ngja_rcInfoExist != 0) {
	result = ngclRemoteClassInformationRelease(
	    context, &jobAttr->ngja_rcInfo, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Remote Class Information.\n"); 
	    return 0;
	}
    }
    jobAttr->ngja_rcInfoExist = 0;

    /* Release the Executable Path Information */
    if (jobAttr->ngja_epInfoExist != 0) {
	result = ngclExecutablePathInformationRelease(
	    context, &jobAttr->ngja_epInfo, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the Executable Path Information.\n"); 
	    return 0;
	}
    }
    jobAttr->ngja_epInfoExist = 0;

    if (jobAttr->ngja_hostName != NULL)
    	ngiFree(jobAttr->ngja_hostName, log, error);
    jobAttr->ngja_hostName = NULL;

    if (jobAttr->ngja_jobManager != NULL)
	ngiFree(jobAttr->ngja_jobManager, log, error);
    jobAttr->ngja_jobManager = NULL;

    if (jobAttr->ngja_subject != NULL)
	ngiFree(jobAttr->ngja_subject, log, error);
    jobAttr->ngja_subject = NULL;

    if (jobAttr->ngja_clientHostName != NULL)
    	ngiFree(jobAttr->ngja_clientHostName, log, error);
    jobAttr->ngja_clientHostName = NULL;

    if (jobAttr->ngja_executablePath != NULL)
	ngiFree(jobAttr->ngja_executablePath, log, error);
    jobAttr->ngja_executablePath = NULL;

    /* Success */
    return 1;
}

/**
 * Job Attribute: Initialize the members.
 */
static void
ngcllJobAttributeInitializeMember(ngcliJobAttribute_t *jobAttr)
{
    /* Check the argument */
    assert(jobAttr != NULL);

    /* Initialize the flags and pointers */
    ngcllJobAttributeInitializeFlag(jobAttr);
    ngcllJobAttributeInitializePointer(jobAttr);

    /* Initialize the members */
    jobAttr->ngja_portNo = NGI_PORT_INVALID;
    jobAttr->ngja_mpiNcpus = 0;
    jobAttr->ngja_backend = NG_BACKEND_NORMAL;
    jobAttr->ngja_stagingEnable = 0;
    jobAttr->ngja_invokeNjobs = -1;
    jobAttr->ngja_startTimeout = -1;
    jobAttr->ngja_stopTimeout = -1;

    jobAttr->ngja_forceXDR = 0;
}

/**
 * Job Attribute: Initialize the flags.
 */
static void
ngcllJobAttributeInitializeFlag(ngcliJobAttribute_t *jobAttr)
{
    /* Check the argument */
    assert(jobAttr != NULL);

    /* Initialize the flags */
    jobAttr->ngja_lmInfoExist = 0;
    jobAttr->ngja_isInfoExist = 0;
    jobAttr->ngja_rmInfoExist = 0;
    jobAttr->ngja_rcInfoExist = 0;
    jobAttr->ngja_epInfoExist = 0;
}

/**
 * Job Attribute: Initialize the pointers.
 */
static void
ngcllJobAttributeInitializePointer(ngcliJobAttribute_t *jobAttr)
{
    /* Check the arguments */
    assert(jobAttr);

    /* Initialize the pointers */
    jobAttr->ngja_hostName = NULL;
    jobAttr->ngja_jobManager = NULL;
    jobAttr->ngja_subject = NULL;
    jobAttr->ngja_clientHostName = NULL;
    jobAttr->ngja_executablePath = NULL;
    jobAttr->ngja_queueName = NULL;
}

/**
 * Executable: Register
 */
int
ngcliJobRegisterExecutable(
    ngcliJobManager_t *jobMng,
    ngclExecutable_t *executable,
    int nExecutables,
    int *error)
{
    int i;
    int result;
    ngclContext_t *context;
    static const char fName[] = "ngcliJobRegisterExecutable";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(executable != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */
    context = jobMng->ngjm_context;

    /* Set the Job Manager */
    for (i = 0; i < (nExecutables - 1); i++) {
	assert(executable[i].nge_jobMng == NULL);
	assert(executable[i].nge_jobNext == NULL);
	executable[i].nge_jobMng = jobMng;
	executable[i].nge_jobNext = &(executable[i + 1]);
    }
    executable[i].nge_jobMng = jobMng;
    executable[i].nge_jobNext = NULL;

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngLogFatal(jobMng->ngjm_context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* No Executable is registered yes */
    if (jobMng->ngjm_executable_head == NULL) {
	assert(jobMng->ngjm_executable_tail == NULL);

	/* Append at top of the list */
	jobMng->ngjm_executable_head = &executable[0];
	jobMng->ngjm_executable_tail = &executable[nExecutables - 1];
    } else {
	/* Append at last of the list */
	jobMng->ngjm_executable_tail->nge_jobNext = &executable[0];
	jobMng->ngjm_executable_tail = &executable[nExecutables - 1];
    }

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngLogFatal(jobMng->ngjm_context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    return 1;
}

/**
 * Executable: Unregister.
 */
int
ngcliJobUnregisterExecutable(
    ngcliJobManager_t *jobMng,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *prev, *curr;
    static const char fName[] = "ngcliJobUnregisterExecutable";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);
    assert(jobMng->ngjm_executable_head != NULL);
    assert(jobMng->ngjm_executable_tail != NULL);
    assert(executable != NULL);

    /* Initialize the local variables */
    context = jobMng->ngjm_context;

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
        return 0;
    }

    /* Find the Executable */
    prev = NULL;
    curr = jobMng->ngjm_executable_head;
    for (; curr != executable; curr = curr->nge_jobNext) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Executable */
    if (executable == jobMng->ngjm_executable_head)
    	jobMng->ngjm_executable_head = executable->nge_jobNext;
    if (executable == jobMng->ngjm_executable_tail)
    	jobMng->ngjm_executable_tail = prev;
    if (prev != NULL)
    	prev->nge_jobNext = executable->nge_jobNext;
    executable->nge_jobNext = NULL;

    /* Append Executable Handle to Destruction List */

    /* No Executable is registered */
    if (jobMng->ngjm_destruction_executable_head == NULL) {
	assert(jobMng->ngjm_destruction_executable_tail == NULL);

	/* Append at top of the list */
	jobMng->ngjm_destruction_executable_head = executable;
	jobMng->ngjm_destruction_executable_tail = executable;
    } else {
	assert(jobMng->ngjm_destruction_executable_tail != NULL);

	/* Append at last of the list */
	jobMng->ngjm_destruction_executable_tail->nge_jobNext = executable;
	jobMng->ngjm_destruction_executable_tail = executable;
    }

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found.\n"); 

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Executable: Unregister From Destruction List.
 */
int
ngcliJobUnregisterExecutableFromDestructionList(
    ngcliJobManager_t *jobMng,
    ngclExecutable_t *executable,
    int *isLastExecutable,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *prev, *curr;
    static const char fName[] =
        "ngcliJobUnregisterExecutableFromDestructionList";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);
    assert(executable != NULL);
    assert(isLastExecutable != NULL);

    /* Initialize the local variables */
    context = jobMng->ngjm_context;
    *isLastExecutable = 0;

    /* Lock the list */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the list of Executable.\n"); 
	return 0;
    }

    /* Find the Executable */
    prev = NULL;
    curr = jobMng->ngjm_destruction_executable_head;
    for (; curr != executable; curr = curr->nge_jobNext) {
        if (curr == NULL)
            goto notFound;
        prev = curr;
    }

    /* Unregister the Executable */
    if (executable == jobMng->ngjm_destruction_executable_head) {
        assert(prev == NULL);
        jobMng->ngjm_destruction_executable_head = executable->nge_jobNext;
    }
    if (executable == jobMng->ngjm_destruction_executable_tail)
        jobMng->ngjm_destruction_executable_tail = prev;
    if (prev != NULL) {
        assert(executable != jobMng->ngjm_destruction_executable_head);
        prev->nge_jobNext = executable->nge_jobNext;
    }
    executable->nge_jobNext = NULL;

    if (jobMng->ngjm_executable_head == NULL &&
        jobMng->ngjm_destruction_executable_head == NULL) {
        assert(jobMng->ngjm_executable_tail == NULL);
        assert(jobMng->ngjm_destruction_executable_tail == NULL);
        /* If list is empty*/
        *isLastExecutable = 1;
    }

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found.\n"); 

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogFatalJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the list of Executable.\n"); 
    }

    return 0;
}

/**
 * Get the Executable by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclExecutable_t *
ngcliJobGetExecutable(ngcliJobManager_t *jobMng, int id, int *error)
{
    ngclContext_t *context;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcliJobGetExecutable";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Get the Ninf-G Context */
    context = jobMng->ngjm_context;

    /* Is ID less than minimum? */
    if (id < NGI_EXECUTABLE_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is less than %d.\n", id, NGI_EXECUTABLE_ID_MIN); 
	goto error;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_EXECUTABLE_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Job ID %d is greater than %d.\n", id, NGI_EXECUTABLE_ID_MAX); 
	goto error;
    }

    executable = jobMng->ngjm_executable_head;
    for (; executable != NULL; executable = executable->nge_jobNext) {
	assert(executable->nge_ID >= NGI_EXECUTABLE_ID_MIN);
	assert(executable->nge_ID <= NGI_EXECUTABLE_ID_MAX);
	if (executable->nge_ID == id) {
	    /* Found */
	    return executable;
	}
    }

error:
    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable is not found by ID %d.\n", id); 
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
ngcliJobGetNextExecutable(
    ngcliJobManager_t *jobMng,
    ngclExecutable_t *current,
    int *error)
{
    static const char fName[] = "ngcliJobGetNextExecutable";

    /* Check the arguments */
    assert(jobMng != NULL);

    if (current == NULL) {
	/* Return the first Executable */
	if (jobMng->ngjm_executable_head != NULL) {
	    assert(jobMng->ngjm_executable_tail != NULL);
	    return jobMng->ngjm_executable_head;
	}
    } else {
	/* Return the next Executable */
	if (current->nge_jobNext != NULL) {
	    return current->nge_jobNext;
	}
    }

    /* The last Executable was reached */
    ngcliLogInfoJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Executable was reached.\n"); 

    return NULL;
}

/**
 * Start the remote job.
 */
int
ngcliJobStart(ngcliJobManager_t *jobMng, int *error)
{
    int result;
    ngiLineList_t *paramList;
    ngclRemoteMachineInformation_t *rmInfo;
    static const char fName[] = "ngcliJobStart";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */
    paramList = NULL;
    rmInfo = &jobMng->ngjm_attr.ngja_rmInfo;

    if (rmInfo->ngrmi_commProxyType != NULL) {
        jobMng->ngjm_commProxyID =
            ngcliCommunicationProxyManagerPrepareCommunication(
                jobMng->ngjm_context->ngc_communicationProxyManager, 
                rmInfo, &paramList, error);
        if (jobMng->ngjm_commProxyID < 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "The Communication Proxy can't prepare communication.\n"); 
            return 0;
        }
        jobMng->ngjm_clientCommunicationProxyInfo = paramList;
    }

    if (jobMng->ngjm_useInvokeServer != 0) {
        result = ngcliInvokeServerJobStart(
            jobMng->ngjm_context, jobMng, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't start the job by Invoke Server.\n"); 
            return 0;
        }
    } else {
        /* Note : internal job system is not ready. */
        
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Invoke Server was not set for the server %s.\n",
            jobMng->ngjm_attr.ngja_hostName); 
        return 0;
    }

    /* Modify the status */
    jobMng->ngjm_status = NGI_JOB_STATUS_INVOKED;

    /* Success */
    return 1;
}

/**
 * Set the request for Job cancel.
 */
int
ngcliJobRequestCancel(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    static const char fName[] = "ngcliJobRequestCancel";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Print the information */
    ngcliLogDebugJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
        "Set the request for Job cancel.\n"); 

    /* Set the request */
    jobMng->ngjm_requestCancel = 1;

    /* Success */
    return 1;
}

/**
 * Stop the remote job.
 */
int
ngcliJobStop(
    ngcliJobManager_t *jobMng,
    int *error)
{
    ngLog_t *log;
    int result, doNotWait;
    int ret = 1;
    static const char fName[] = "ngcliJobStop";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */
    log = jobMng->ngjm_context->ngc_log;
    doNotWait = 0;

    /* Is job invoked? */
    if (jobMng->ngjm_status < NGI_JOB_STATUS_INVOKED) {
        ngcliLogDebugJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Job is not invoked.\n"); 

        /* Success */
        return 1;
    }

    /* Cancel request for the Job if necessary */
    if (jobMng->ngjm_useInvokeServer != 0) {
        result = ngcliInvokeServerJobCancel(
            jobMng->ngjm_context, jobMng, &doNotWait, error);
    } else {
        /* Note : internal job system is not ready. */

        result = 0;
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "No cancel for non Invoke Server job..\n"); 
    }
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't cancel the job.\n"); 
        error = NULL;
        ret = 0;
        doNotWait = 1;
    }

    /* Wait for the end of job */
    if (doNotWait != 0) {
        result = 1;
    } else if (jobMng->ngjm_attr.ngja_stopTimeout < 0) {
        result = ngcllJobWaitDone(jobMng, log, error);
    } else if (jobMng->ngjm_attr.ngja_stopTimeout > 0) {
        result = ngcllJobWaitDoneTimeout(
            jobMng, jobMng->ngjm_attr.ngja_stopTimeout, log, error);
    } else {
        /* unregister was already performed for stopTimeout == 0 */
        result = 1;
    }
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait the Job done.\n"); 
        error = NULL;
        ret = 0;
    }

    if (jobMng->ngjm_useInvokeServer != 0) {
        /* Tell the Invoke Server to destroy the job */
        result = ngcliInvokeServerJobStop(
            jobMng->ngjm_context, jobMng, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't stop the job.\n"); 
            error = NULL;
            ret = 0;
        }
    } else {
        /* Note : internal job system is not ready. */

        result = 0;
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "No cancel for non Invoke Server job..\n"); 
    }

    return ret;
}

/**
 * Wait done.
 */
static int
ngcllJobWaitDone(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcllJobWaitDone";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        return 0;
    }

    /* Wait done or failed */
    while ((jobMng->ngjm_status != NGI_JOB_STATUS_DONE) &&
           (jobMng->ngjm_status != NGI_JOB_STATUS_FAILED)) {
        result = ngiCondWait(
            &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
	}
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Wait done with timeout.
 */
static int
ngcllJobWaitDoneTimeout(
    ngcliJobManager_t *jobMng,
    int second,
    ngLog_t *log,
    int *error)
{
    time_t timeEnd;
    int result;
    int retResult = 0;
    int remain;
    int timeout;
    static const char fName[] = "ngcllJobWaitDoneTimeout";

    /* Check the arguments */
    assert(jobMng != NULL);

    timeEnd = time(NULL) + second;
    remain = second;

    /* Lock the mutex */
    result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        return 0;
    }

    /* Wait done or failed */
    while ((jobMng->ngjm_status != NGI_JOB_STATUS_DONE) &&
           (jobMng->ngjm_status != NGI_JOB_STATUS_FAILED)) {

        timeout = 0;
        result = ngiCondTimedWait(
            &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, remain, &timeout,
            log, error);
        if (result == 0) {
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable.\n"); 
            goto error;
	}

        /* Timedout? */
        if (timeout != 0) {
            NGI_SET_ERROR(error, NG_ERROR_TIMEOUT);
            ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
                "Timeout. waited %d seconds.\n", second); 
            goto error;
        }

        remain = timeEnd - time(NULL);
        if (remain < 0) {
            remain = 0;
        }
    }

    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        retResult = 1;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        return 0;
    }

    /* Success */
    return retResult;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
    }

    /* Failed */
    return 0;
}

/**
 * Get Local Machine Information: fortran_compatible.
 */
int
ngcliJobGetLocalMachineInformationFortranCompatible(
    ngcliJobManager_t *jobMng,
    int *fortranCompatible,
    int *error)
{
    static const char fName[] = "ngcliJobGetLocalMachineInformationFortranCompatible";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(fortranCompatible != NULL);

    /* Is Local Machine Information exist? */
    if (jobMng->ngjm_attr.ngja_lmInfoExist == 0) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
	    "Local Machine Information is not exist.\n"); 
	return 0;
    }

    /* Success */
    *fortranCompatible = jobMng->ngjm_attr.ngja_lmInfo.nglmi_fortranCompatible;
    return 1;
}

