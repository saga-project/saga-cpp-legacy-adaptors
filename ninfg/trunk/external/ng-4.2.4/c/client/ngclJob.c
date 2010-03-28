#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclJob.c,v $ $Revision: 1.110 $ $Date: 2007/12/26 12:27:17 $";
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
 * Module of Job management for Ninf-G Client.
 */

#include <stdlib.h>
#include <string.h>
#ifndef NG_PTHREAD
# ifdef linux
#  include <sys/poll.h>
# else /* linux */
#  include <poll.h>
#endif /* linux */
#endif /* NG_PTHREAD */
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngcliJobManager_t *ngcllJobAllocate(ngclContext_t *, int *);
static int ngcllJobFree(ngcliJobManager_t *, int *);
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
static int ngcllJobStartByGRAM(ngcliJobManager_t *, int *);
static char *ngcllJobMakeRSL(ngcliJobManager_t *, int *);
static int ngcllJobReleaseRSL(ngcliJobManager_t *, char *, int *);
static int ngcllJobMakeRMcontact(ngcliJobManager_t *, int *);
static int ngcllJobCancelByGRAM(ngcliJobManager_t *, int *, int *);
static int ngcllJobCancelByForceByGRAM(ngcliJobManager_t *, int *);

static int ngcllJobWaitDone(ngcliJobManager_t *, ngLog_t *, int *);
static int ngcllJobWaitDoneTimeout(
    ngcliJobManager_t *, int, ngLog_t *, int *);

static void ngcllJobGRAMcallback(void *, char *, int, int);

static int ngcllJobGRAMcallbackAllow(ngcliJobManager_t *, int *);
static int ngcllJobGRAMcallbackDisallow(ngcliJobManager_t *, int *);

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
    jobMng = ngcllJobAllocate(context, error);
    if (jobMng == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Job Manager.\n",
	    fName);
        goto error;
    }

    /* Initialize */
    result = ngcllJobInitialize(context, jobMng, jobAttr, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Job Manager.\n",
	    fName);
	goto error;
    }
    initialized = 1;

    /* Register */
    result = ngcliContextRegisterJobManager(context, jobMng, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Job Manager to Ninf-G Context.\n",
	    fName);
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
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Job Manager.\n",
                fName);
        }
        initialized = 0;
    }

    /* Deallocate */
    if (jobMng != NULL) {
        result = ngcllJobFree(jobMng, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't deallocate the storage for Job Manager.\n",
                fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't stop the Job.\n", fName);
        /* If ngcliJobStop() fails, ngcliJobDestruct() doesn't fail. */
    }

    /* Get the Ninf-G Context */
    context = jobMng->ngjm_context;

    /* Unegister */
    result = ngcliContextUnregisterJobManager(context, jobMng, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Job Manager.\n",
	    fName);
        error = NULL;
        ret = 0;
    }

    /* Finalize */
    result = ngcllJobFinalize(jobMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Job Manager.\n",
	    fName);
        error = NULL;
        ret = 0;
    }

    /* Deallocate */
    result = ngcllJobFree(jobMng, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Job Manager.\n",
	    fName);
        error = NULL;
        ret = 0;
    }

    return ret;
}

/**
 * Allocate
 */
static ngcliJobManager_t *
ngcllJobAllocate(ngclContext_t *context, int *error)
{
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcllJobAllocate";

    /* Allocate the Job Manager */
    jobMng = globus_libc_calloc(1, sizeof (ngcliJobManager_t));
    if (jobMng == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Job Manager.\n",
	    fName);
	return NULL;
    }

    return jobMng;
}

/**
 * Free
 */
static int
ngcllJobFree(ngcliJobManager_t *jobMng, int *error)
{
    globus_libc_free(jobMng);

    return 1;
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
    	ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Invoke Server Job Information.\n",
	    fName);
	goto error;
    }

    /* Initialize the Mutex, Cond and Read/Write lock */
    result = ngcllJobInitializeMutex(jobMng, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize Mutex, Cond and Read/Write lock for Job Manager.\n",
	    fName);
	goto error;
    }

    /* Create the Job ID */
    jobID = ngcllJobCreateID(context, error);
    if (jobID < 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't create the Job ID.\n", fName);
	goto error;
    }
    jobMng->ngjm_ID = jobID;

    /* Copy the Job Attribute */
    result = ngcliJobAttributeCopy(
    	context, jobAttr, &jobMng->ngjm_attr, error);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't copy the Job Attribute.\n", fName);
	goto error;
    }

    /* Use Invoke Server? */
    jobMng->ngjm_useInvokeServer = 0; /* Use GRAM */
    if ((jobMng->ngjm_attr.ngja_rmInfoExist != 0) &&
        (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_invokeServerType != NULL)) {
        assert(jobMng->ngjm_attr.ngja_isInfoExist != 0);
        assert(jobMng->ngjm_attr.ngja_isInfo.ngisi_type != NULL);
        jobMng->ngjm_useInvokeServer = 1; /* Use Invoke Server */
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngcllJobFinalize(jobMng, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Job Manager.\n",
	    fName);
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

    /* Release the Job Attribute */
    result = ngcliJobAttributeRelease(
	jobMng->ngjm_context, &jobMng->ngjm_attr, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Job Attribute.\n", fName);
        error = NULL;
        ret = 0;
    }

    /* Disallow the callback */
    result = ngcllJobGRAMcallbackDisallow(jobMng, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't disallow the callback.\n", fName);
        error = NULL;
        ret = 0;
    }

    /* Deallocate the Job Contact */
    if (jobMng->ngjm_jobContact != NULL) {
	result = globus_gram_client_job_contact_free(jobMng->ngjm_jobContact);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_JOB_INVOKE);
	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: globus_gram_client_job_contact_free failed by %d: %s.\n",
		fName, result, globus_gram_client_error_string(result));
            error = NULL;
            ret = 0;
	}
    }
    jobMng->ngjm_jobContact = NULL;

    /* Deallocate the Resource Manager Contact */
    if (jobMng->ngjm_rmContact != NULL)
	globus_libc_free(jobMng->ngjm_rmContact);
    jobMng->ngjm_rmContact = NULL;

    /* Finalize the Mutex, Condition variable and Read/Write lock */
    result = ngcllJobFinalizeMutex(jobMng, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Mutex, Condition variable and Read/Write lock.\n",
	    fName);
        error = NULL;
        ret = 0;
    }

    /* Finalize the Invoke Server Job Info */
    result = ngcliInvokeServerJobFinalize(
        context, &jobMng->ngjm_invokeServerInfo, error);
    if (result == 0) {
    	ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Invoke Server Job Information.\n",
	    fName);
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
}

/**
 * Initialize the flags.
 */
static void
ngcllJobInitializeFlag(ngcliJobManager_t *jobMng)
{
    /* Initialize the flags */
    jobMng->ngjm_monMutexInitialized = 0;
    jobMng->ngjm_monCondInitialized = 0;
    jobMng->ngjm_rwlOwnInitialized = 0;
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
    jobMng->ngjm_callbackContact = NULL;
    jobMng->ngjm_jobContact = NULL;
    jobMng->ngjm_rmContact = NULL;
}

/**
 * Initialize the Mutex, Condition variable and Read/Write lock.
 */
static int
ngcllJobInitializeMutex(ngcliJobManager_t *jobMng, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcllJobInitializeMutex";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Initialize the mutex for monitor */
    result = ngiMutexInitialize(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex.\n", fName);
	return 0;
    }
    jobMng->ngjm_monMutexInitialized = 1;

    /* Initialize the condition variable for monitor */
    result = ngiCondInitialize(&jobMng->ngjm_monCond, log, NULL);
    if (result == 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Condition variable.\n", fName);
	return 0;
    }
    jobMng->ngjm_monCondInitialized = 1;

    /* Initialize the Read/Write Lock for this instance */
    result = ngiRWlockInitialize(&jobMng->ngjm_rwlOwn, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write.\n", fName);
	return 0;
    }
    jobMng->ngjm_rwlOwnInitialized = 1;

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
    if (jobMng->ngjm_monMutexInitialized != 0) {
	result = ngiMutexDestroy(&jobMng->ngjm_monMutex, log, NULL);
	if (result == 0) {
	    NGI_SET_ERROR (error, NG_ERROR_THREAD);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Mutex.\n", fName);
	    return 0;
	}
    }
    jobMng->ngjm_monMutexInitialized = 0;

    /* Finalize the condition variable for monitor */
    if (jobMng->ngjm_monCondInitialized != 0) {
	result = ngiCondDestroy(&jobMng->ngjm_monCond, log, NULL);
	if (result == 0) {
	    NGI_SET_ERROR (error, NG_ERROR_THREAD);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Condition variable.\n", fName);
	    return 0;
	}
    }
    jobMng->ngjm_monCondInitialized = 0;

    /* Finalize the Read/Write Lock for this instance */
    if (jobMng->ngjm_rwlOwnInitialized != 0) {
	result = ngiRWlockFinalize(&jobMng->ngjm_rwlOwn, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Read/Write.\n", fName);
	    return 0;
	}
    }
    jobMng->ngjm_rwlOwnInitialized = 0;

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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
	return 0;
    }

    /* Update the status */
    jobMng->ngjm_status = status;

    /* Signal */
    result = ngiCondSignal(&jobMng->ngjm_monCond, log, error);
    if (result == 0) {
    	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't signal the Condition Variable.\n", fName);
	goto error;
    }

    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
	return 0;
    }

    /* Wait the status */
    while (jobMng->ngjm_status < status) {
	result = ngiCondWait(
	    &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, log, error);
	if (result == 0) {
    	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
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
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Job Manager is NULL.\n", fName);
	return 0;
    }

    /* Use Ninf-G Context which contained in Job Manager, if context is NULL */
    if (context == NULL) {
	context = jobMng->ngjm_context;
    }

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    /* Get the Log Manager */
    log = context->ngc_log;

    /* Is ID smaller than minimum? */
    if (jobMng->ngjm_ID < NGI_JOB_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Job ID %d is smaller than minimum %d.\n",
	    fName, jobMng->ngjm_ID, NGI_JOB_ID_MIN);
	return 0;
    }

    /* Is ID greater than maximum? */
    if (jobMng->ngjm_ID > NGI_JOB_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Job ID %d is greater than maximum %d.\n",
	    fName, jobMng->ngjm_ID, NGI_JOB_ID_MAX);
	return 0;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListReadLock(context, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the list of Job Manager.\n", fName);
	return 0;
    }

    /* Is Job Manager exist? */
    curr = ngcliContextGetNextJobManager(context, NULL, error);
    if (curr == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: No Job Manager is registered.\n", fName);
	goto error;
    }

    while (curr != jobMng) {
	curr = ngcliContextGetNextJobManager(context, curr, error);
	if (curr == NULL) {
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Job Manager is not found.\n", fName);
	    goto error;
	}
    }

    /* Unlock the list of Job Manager */
    result = ngcliContextJobManagerListReadUnlock(context, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Job Manager.\n", fName);
	return 0;
    }

    /* Job Manager is valid */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Job Manager */
    result = ngcliContextJobManagerListReadUnlock(context, log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Job Manager.\n", fName);
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
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Job manager.\n", fName);
        return 0;
    }
    locked = 1;

    /* Is already measured? */
    if (jobMng->ngjm_invokeMeasured != 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Job invoke time was already measured.\n", fName);
    } else {
        jobMng->ngjm_invokeMeasured = 1;

        /* Set the end time */
        result = ngiSetEndTime(&jobMng->ngjm_invoke, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the End time.\n", fName);
            goto error;
        }
    }

    /* Unlock the this instance */
    locked = 0;
    result = ngcliJobManagerWriteUnlock(jobMng, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Job manager.\n", fName);
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
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Job manager.\n", fName);
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
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the list of Job.\n",
	    fName);
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
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the list of Job.\n", fName);
	    return -1;
	}

	/* Success */
	return newID;
    }

    /* All ID is used */
    NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_WARNING, error,
	"%s: There is no IDs for Executable to use.\n",
	fName);

    /* Unlock the list of Job */
    result = ngcliContextJobManagerListWriteUnlock(
	context, context->ngc_log, NULL);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't unlock the list of Job.\n",
	    fName);
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
    int gassMngListLocked, rmInfoListLocked, rcInfoListLocked;
    ngclGASSserverManager_t *gassMng;
    ngclRemoteMachineInformationManager_t *rmInfoMng;
    ngclRemoteClassInformationManager_t *rcInfoMng;
    static const char fName[] = "ngcliJobAttributeInitialize";

    /* Check the arguments */
    assert(context != NULL);
    assert(execAttr != NULL);
    assert(jobAttr != NULL);

    /* Initialize the local variables */
    gassMngListLocked = 0;
    rmInfoListLocked = 0;
    rcInfoListLocked = 0;
    jobManager = NULL;
    subject = NULL;
    clientHostName = NULL;
    log = context->ngc_log;

    /* Initialize the members */
    ngcllJobAttributeInitializeMember(jobAttr);

    /* Get the Local Machine Information */
    result = ngclLocalMachineInformationGetCopy(
        context, &jobAttr->ngja_lmInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Local Machine Information.\n", fName);
	goto error;
    }
    jobAttr->ngja_lmInfoExist = 1;

    /* Get the Remote Machine Information */
    result = ngclRemoteMachineInformationGetCopy(
        context, execAttr->ngea_hostName, execAttr->ngea_className,
        &jobAttr->ngja_rmInfo, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Remote Machine Information.\n", fName);
	goto error;
    }
    jobAttr->ngja_rmInfoExist = 1;

    /* Lock the list of GASS Server Manager */
    result = ngcliGASSserverManagerListReadLock(
    	context, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the list of GASS Server Information.\n", fName);
	goto error;
    }
    gassMngListLocked = 1;

    /* Get the GASS Server Manager */
    gassMng = ngcliContextGASSserverManagerGet(
    	context, jobAttr->ngja_rmInfo.ngrmi_gassScheme, error);
    if (gassMng == NULL) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the GASS Server Information.\n", fName);
	goto error;
    }

    /* Duplicate the URL of GASS Server */
    jobAttr->ngja_gassUrl = strdup(gassMng->nggsm_url);
    if (jobAttr->ngja_gassUrl == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get storage for URL of GASS Server.\n", fName);
	goto error;
    }

    /* Unlock the list of GASS Server Manager */
    result = ngcliGASSserverManagerListReadUnlock(
    	context, log, error);
    gassMngListLocked = 0;
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of GASS Server Information.\n", fName);
	goto error;
    }

    /* Lock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadLock(
	context, log, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the list of Remote Machine Information.\n", fName);
	goto error;
    }
    rmInfoListLocked = 1;

    /* Get the Remote Machine Information */
    rmInfoMng = ngcliRemoteMachineInformationCacheGet(
        context, execAttr->ngea_hostName, error);
    if (rmInfoMng == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Remote Machine Information.\n", fName);
	goto error;
    }

    /* Get the Executable Path Information */
    result = ngclExecutablePathInformationGetCopy(
	context, rmInfoMng, execAttr->ngea_className, &jobAttr->ngja_epInfo,
	error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Executable Path Information.\n", fName);
	goto error;
    }
    jobAttr->ngja_epInfoExist = 1;

    /* Lock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadLock(
        context, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Remote Class Information.\n", fName);
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
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the Remote Class Information.\n", fName);
            goto error;
        }
        jobAttr->ngja_rcInfoExist = 1;

    }

    /* Unlock the Remote Class Information */
    result = ngcliRemoteClassInformationListReadUnlock(
        context, log, error);
    rcInfoListLocked = 0;
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Remote Class Information.\n", fName);
        goto error;
    }

    /* Initialize the attribute of job management */
    jobAttr->ngja_portNo   = execAttr->ngea_portNo;
    if (execAttr->ngea_portNo ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_portNo = rmInfoMng->ngrmim_info.ngrmi_portNo;
    }

    jobAttr->ngja_mpiNcpus = rmInfoMng->ngrmim_info.ngrmi_mpiNcpus;
    if (jobAttr->ngja_epInfo.ngepi_mpiNcpus > 0)
	jobAttr->ngja_mpiNcpus = jobAttr->ngja_epInfo.ngepi_mpiNcpus;
    if (execAttr->ngea_mpiNcpus != NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED)
        jobAttr->ngja_mpiNcpus = execAttr->ngea_mpiNcpus;

    if (jobAttr->ngja_rcInfoExist != 0)
        jobAttr->ngja_backend = jobAttr->ngja_rcInfo.ngrci_backend;
    if (jobAttr->ngja_epInfo.ngepi_backend != NG_BACKEND_NORMAL)
    	jobAttr->ngja_backend = jobAttr->ngja_epInfo.ngepi_backend;
    jobAttr->ngja_stagingEnable = jobAttr->ngja_epInfo.ngepi_stagingEnable;
    jobAttr->ngja_invokeNjobs = execAttr->ngea_invokeNjobs;

    jobAttr->ngja_startTimeout = execAttr->ngea_jobStartTimeout;
    if (execAttr->ngea_jobStartTimeout ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_startTimeout =
            rmInfoMng->ngrmim_info.ngrmi_jobStartTimeout;
    }
    jobAttr->ngja_stopTimeout = execAttr->ngea_jobStopTimeout;
    if (execAttr->ngea_jobStopTimeout ==
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        jobAttr->ngja_stopTimeout =
            rmInfoMng->ngrmim_info.ngrmi_jobEndTimeout;
    }

    jobAttr->ngja_forceXDR = rmInfoMng->ngrmim_info.ngrmi_forceXDR;

    /* Duplicate the host name */
    assert(rmInfoMng->ngrmim_info.ngrmi_hostName != NULL);
    jobAttr->ngja_hostName = strdup(rmInfoMng->ngrmim_info.ngrmi_hostName);
    if (jobAttr->ngja_hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't duplicate the host name.\n", fName);
	goto error;
    }

    /* Duplicate the Queue Name */
    if ((execAttr->ngea_queueName != NULL) ||
        (rmInfoMng->ngrmim_info.ngrmi_jobQueue != NULL)) {

        jobAttr->ngja_queueName = strdup(
            (execAttr->ngea_queueName != NULL ?
                execAttr->ngea_queueName :
                rmInfoMng->ngrmim_info.ngrmi_jobQueue));
        if (jobAttr->ngja_queueName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	        NG_LOG_LEVEL_ERROR, NULL,
	        "%s: Can't duplicate the queue name.\n", fName);
	    goto error;
        }
    }

    /* Duplicate the Job Manager */
    jobManager = NULL;
    if (rmInfoMng->ngrmim_info.ngrmi_jobManager != NULL) {
        jobManager = rmInfoMng->ngrmim_info.ngrmi_jobManager;
    }
    if (execAttr->ngea_jobManager != NULL) {
        jobManager = execAttr->ngea_jobManager;
    }
    if (jobManager != NULL) {
        jobAttr->ngja_jobManager = strdup(jobManager);
        if (jobAttr->ngja_jobManager == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the jobmanager.\n", fName);
            goto error;
        }
    }
    jobManager = NULL;

    /* Duplicate the Subject */
    subject = NULL;
    if (rmInfoMng->ngrmim_info.ngrmi_subject != NULL) {
        subject = rmInfoMng->ngrmim_info.ngrmi_subject;
    }
    if (execAttr->ngea_subject != NULL) {
        subject = execAttr->ngea_subject;
    }
    if (subject != NULL) {
        jobAttr->ngja_subject = strdup(subject);
        if (jobAttr->ngja_subject == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the jobmanager.\n", fName);
            goto error;
        }
    }
    subject = NULL;

    /* Duplicate the Client Host name */
    clientHostName = jobAttr->ngja_lmInfo.nglmi_hostName;
    if (rmInfoMng->ngrmim_info.ngrmi_clientHostName != NULL) {
        clientHostName = rmInfoMng->ngrmim_info.ngrmi_clientHostName;
    }
    if (clientHostName != NULL) {
        jobAttr->ngja_clientHostName = strdup(clientHostName);
        if (jobAttr->ngja_clientHostName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the Client host name.\n", fName);
            goto error;
        }
    } else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfContext(context,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Client host name is NULL.\n", fName);
        goto error;
    }
    clientHostName = NULL;

    /* Unlock the Remote Machine Information */
    result = ngcliRemoteMachineInformationListReadUnlock(
	context, log, error);
    rmInfoListLocked = 0;
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Remote Class Information.\n", fName);
	goto error;
    }

    /* Duplicate the Executable Path */
    jobAttr->ngja_executablePath = strdup(jobAttr->ngja_epInfo.ngepi_path);
    if (jobAttr->ngja_executablePath == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't duplicate the host name.\n", fName);
	goto error;
    }

    /* Invoke Server */
    assert(jobAttr->ngja_rmInfoExist == 1);
    if (jobAttr->ngja_rmInfo.ngrmi_invokeServerType != NULL) {
        result = ngclInvokeServerInformationGetCopy(
            context, jobAttr->ngja_rmInfo.ngrmi_invokeServerType,
            &jobAttr->ngja_isInfo, error);
        if (result == 0) {
            ngclLogPrintfContext(context,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get the Invoke Server Information.\n", fName);
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
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: multiple function handle for jobType=mpi is not supported"
            " for one Globus Toolkit job submission.\n", fName);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: executable = %d, mpi nodes = %d\n",
                fName, jobAttr->ngja_invokeNjobs, jobAttr->ngja_mpiNcpus);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of GASS Server Manager */
    if (gassMngListLocked != 0) {
        gassMngListLocked = 0;
        result = ngcliGASSserverManagerListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of GASS Server Information.\n",
                fName);
        }
    }

    /* Unlock the Remote Class Information */
    if (rcInfoListLocked != 0) {
        rcInfoListLocked = 0;
        result = ngcliRemoteClassInformationListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Class Information.\n",
                fName);
        }
    }

    /* Unlock the Remote Machine Information */
    if (rmInfoListLocked != 0) {
        rmInfoListLocked = 0;
        result = ngcliRemoteMachineInformationListReadUnlock(
            context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Remote Class Information.\n",
                fName);
        }
    }

    /* Release */
    result = ngcliJobAttributeRelease(context, jobAttr, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Job Attribute.\n", fName);
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
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Job Attribute.\n", fName);
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

    /* Duplicate the URL of GASS Server */
    if (src->ngja_gassUrl != NULL) {
	dest->ngja_gassUrl = strdup(src->ngja_gassUrl);
	if (dest->ngja_gassUrl == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the URL of GASS Server.\n", fName);
	    goto error;
	}
    }

    /* Duplicate the host name */
    if (src->ngja_hostName != NULL) {
	dest->ngja_hostName = strdup(src->ngja_hostName);
	if (dest->ngja_hostName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the host name.\n", fName);
	    goto error;
	}
    }

    /* Duplicate the queue name */
    if (src->ngja_queueName != NULL) {
        dest->ngja_queueName = strdup(src->ngja_queueName);
        if (dest->ngja_queueName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
               "%s: Can't duplicate the queue name.\n", fName);
            goto error;
        }
    }

    /* Duplicate the Job Manager */
    if (src->ngja_jobManager != NULL) {
	dest->ngja_jobManager = strdup(src->ngja_jobManager);
	if (dest->ngja_jobManager == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the Job Manager.\n", fName);
	    goto error;
        }
    }

    /* Duplicate the Subject */
    if (src->ngja_subject != NULL) {
	dest->ngja_subject = strdup(src->ngja_subject);
	if (dest->ngja_subject == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the Subject.\n", fName);
	    goto error;
        }
    }

    /* Duplicate the Client Host name */
    if (src->ngja_clientHostName != NULL) {
	dest->ngja_clientHostName = strdup(src->ngja_clientHostName);
	if (dest->ngja_clientHostName == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the Client Host name.\n", fName);
	    goto error;
        }
    }

    /* Duplicate the Executable Path */
    if (src->ngja_executablePath != NULL) {
	dest->ngja_executablePath = strdup(src->ngja_executablePath);
	if (dest->ngja_executablePath == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the Executable Path.\n", fName);
	    goto error;
	}
    }

    /* Copy the Local Machine Information */
    if (src->ngja_lmInfoExist != 0) {
	result = ngcliLocalMachineInformationCopy(
	    context, &src->ngja_lmInfo, &dest->ngja_lmInfo, error);
	if (result == 0) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't copy the Local Machine Information.\n", fName);
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
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't copy the Invoke Server Information.\n", fName);
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
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't copy the Remote Machine Information.\n", fName);
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
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't copy the Remote Class Information.\n", fName);
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
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't copy the Executable Path Information.\n", fName);
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
    static const char fName[] = "ngcliJobAttributeRelease";

    /* Check the arguments */
    assert(context != NULL);
    assert(jobAttr != NULL);

    /* Release the Local Machine Information */
    if (jobAttr->ngja_lmInfoExist != 0) {
	result = ngclLocalMachineInformationRelease(
	    context, &jobAttr->ngja_lmInfo, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the Local Machine Information.\n", fName);
	    return 0;
	}
    }
    jobAttr->ngja_lmInfoExist = 0;

    /* Release the Invoke Server Information */
    if (jobAttr->ngja_isInfoExist != 0) {
	result = ngclInvokeServerInformationRelease(
	    context, &jobAttr->ngja_isInfo, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the Invoke Server Information.\n", fName);
	    return 0;
	}
    }
    jobAttr->ngja_isInfoExist = 0;

    /* Release the Remote Machine Information */
    if (jobAttr->ngja_rmInfoExist != 0) {
	result = ngclRemoteMachineInformationRelease(
	    context, &jobAttr->ngja_rmInfo, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the Remote Machine Information.\n", fName);
	    return 0;
	}
    }
    jobAttr->ngja_rmInfoExist = 0;

    /* Release the Remote Class Information */
    if (jobAttr->ngja_rcInfoExist != 0) {
	result = ngclRemoteClassInformationRelease(
	    context, &jobAttr->ngja_rcInfo, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the Remote Class Information.\n", fName);
	    return 0;
	}
    }
    jobAttr->ngja_rcInfoExist = 0;

    /* Release the Executable Path Information */
    if (jobAttr->ngja_epInfoExist != 0) {
	result = ngclExecutablePathInformationRelease(
	    context, &jobAttr->ngja_epInfo, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the Executable Path Information.\n", fName);
	    return 0;
	}
    }
    jobAttr->ngja_epInfoExist = 0;

    /* Deallocate */
    if (jobAttr->ngja_gassUrl != NULL)
    	globus_libc_free(jobAttr->ngja_gassUrl);
    jobAttr->ngja_gassUrl = NULL;

    if (jobAttr->ngja_hostName != NULL)
    	globus_libc_free(jobAttr->ngja_hostName);
    jobAttr->ngja_hostName = NULL;

    if (jobAttr->ngja_jobManager != NULL)
	globus_libc_free(jobAttr->ngja_jobManager);
    jobAttr->ngja_jobManager = NULL;

    if (jobAttr->ngja_subject != NULL)
	globus_libc_free(jobAttr->ngja_subject);
    jobAttr->ngja_subject = NULL;

    if (jobAttr->ngja_clientHostName != NULL)
    	globus_libc_free(jobAttr->ngja_clientHostName);
    jobAttr->ngja_clientHostName = NULL;

    if (jobAttr->ngja_executablePath != NULL)
	globus_libc_free(jobAttr->ngja_executablePath);
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
    jobAttr->ngja_gassUrl = NULL;
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
	ngLogPrintf(jobMng->ngjm_context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Executable.\n", fName);
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
	ngLogPrintf(jobMng->ngjm_context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Executable.\n", fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_FATAL, NULL, "%s: Executable is not found.\n", fName);

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Executable.\n", fName);
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_FATAL, NULL, "%s: Executable is not found.\n", fName);

    /* Unlock the list */
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Executable.\n", fName);
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
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Job ID %d is less than %d.\n",
	    fName, id, NGI_EXECUTABLE_ID_MIN);
	goto error;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_EXECUTABLE_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Job ID %d is greater than %d.\n",
	    fName, id, NGI_EXECUTABLE_ID_MAX);
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
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Executable is not found by ID %d.\n", fName, id);
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
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Executable was reached.\n", fName);

    return NULL;
}

/**
 * Start the remote job.
 */
int
ngcliJobStart(ngcliJobManager_t *jobMng, int *error)
{
    int result;
    static const char fName[] = "ngcliJobStart";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */

    if (jobMng->ngjm_useInvokeServer != 0) {
        result = ngcliInvokeServerJobStart(
            jobMng->ngjm_context, jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't start the job by Invoke Server.\n", fName);
            return 0;
        }
    } else {
        result = ngcllJobStartByGRAM(jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't start the job.\n", fName);
            return 0;
        }
    }

    /* Modify the status */
    jobMng->ngjm_status = NGI_JOB_STATUS_INVOKED;

    /* Success */
    return 1;
}

/**
 * Start the remote job by GRAM.
 */
static int
ngcllJobStartByGRAM(ngcliJobManager_t *jobMng, int *error)
{
    int result;
    char *rsl = NULL;
    ngLog_t *log;
    static const char fName[] = "ngcllJobStartByGRAM";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);

    /* Initialize the local variables */
    log = jobMng->ngjm_context->ngc_log;

    result = ngcliGlobusGRAMclientInitialize(log, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't activate the GRAM_CLIENT module.\n", fName);
	goto error;
    }

    /* Make Resource Manager Contact */
    result = ngcllJobMakeRMcontact(jobMng, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't make the Resource Manager Contact.\n", fName);
	goto error;
    }

    /* Callback allow */
    result = ngcllJobGRAMcallbackAllow(jobMng, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't construct callback contact.\n",
	    fName);
	goto error;
    }

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Callback contact: \"%s\".\n",
        fName, jobMng->ngjm_callbackContact);

    /* Make the RSL */
    rsl = ngcllJobMakeRSL(jobMng, error);
    if (rsl == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't make the RSL.\n", fName);
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(&jobMng->ngjm_invoke, log, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start time.\n", fName);
	goto error;
    }

    /* Invoke the job */
    result = globus_gram_client_job_request(
	jobMng->ngjm_rmContact,
	rsl,
	GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL,
	jobMng->ngjm_callbackContact,
	&jobMng->ngjm_jobContact);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_JOB_INVOKE);
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: globus_gram_client_job_request failed by %d: %s.\n",
	    fName, result, globus_gram_client_error_string(result));
	goto error;
    }

    /* Output the log */
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
        NG_LOG_LEVEL_DEBUG, NULL, "%s: Job Contact: %s.\n",
        fName, jobMng->ngjm_jobContact);

    /* Release the RSL */
    result = ngcllJobReleaseRSL(jobMng, rsl, error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't release the RSL.\n", fName);
	return 0;
    }
    rsl = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Disallow the callback */
    result = ngcllJobGRAMcallbackDisallow(jobMng, NULL);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't disallow the callback.\n", fName);
    }

    /* Release the RSL */
    if (rsl != NULL) {
	result = ngcllJobReleaseRSL(jobMng, rsl, NULL);
	if (result == 0) {
	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't release the RSL.\n", fName);
    	}
    }
    rsl = NULL;

    /* Failure */
    return 0;
}

/**
 * Make the RSL.
 * Note: If you added the new option, Be sure to regist
 * not only MakeRSL, but also Invoke Server JobCreateArgument.
 */
static char *
ngcllJobMakeRSL(ngcliJobManager_t *jobMng, int *error)
{
    int i;
    char *rsl, *crypt, *protocol, *tmp, *env, *p;
    ngclRemoteMachineInformation_t *rmInfo;
    ngclLocalMachineInformation_t *lmInfo;
    struct ngiCommunication_s *comm;
    size_t bufferNbytes, length;
    static const char fName[] = "ngcllJobMakeRSL";

    /* Initialize the local variable */
    rmInfo = &jobMng->ngjm_attr.ngja_rmInfo;
    lmInfo = &jobMng->ngjm_attr.ngja_lmInfo;
    bufferNbytes = NGCLI_RSL_NBYTES;

    /* Calculate the length of strings of debugger */
    if (rmInfo->ngrmi_debug.ngdi_enable) {
      if (rmInfo->ngrmi_debug.ngdi_terminalPath != NULL) {
	    bufferNbytes +=
		strlen(rmInfo->ngrmi_debug.ngdi_terminalPath) + 1;
      }
      if (rmInfo->ngrmi_debug.ngdi_display != NULL) {
	    bufferNbytes += strlen(rmInfo->ngrmi_debug.ngdi_display) + 1;
      }
      if (rmInfo->ngrmi_debug.ngdi_debuggerPath != NULL) {
	    bufferNbytes +=
		strlen(rmInfo->ngrmi_debug.ngdi_debuggerPath) + 1;
      }
    }

    /* Calculate the length of environment variables */
    for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
	bufferNbytes += strlen(rmInfo->ngrmi_environment[i]) + 1;
    }

    /* Calculate the length of RSL extensions */
    for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
	bufferNbytes += strlen(rmInfo->ngrmi_rslExtensions[i]) + 1;
    }

    /* Allocate */
    rsl = globus_libc_malloc(bufferNbytes);
    if (rsl == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for RSL.\n", fName);
	return 0;
    }

    /* Get the Crypt type and Port No. */
    switch (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_crypt) {
    case NG_PROTOCOL_CRYPT_NONE:
	crypt = "none";
	comm = jobMng->ngjm_context->ngc_commNone;
	break;

    case NG_PROTOCOL_CRYPT_AUTHONLY:
	crypt = "authonly";
	comm = jobMng->ngjm_context->ngc_commAuthonly;
	break;

    case NG_PROTOCOL_CRYPT_GSI:
	crypt = "GSI";
	comm = jobMng->ngjm_context->ngc_commGSI;
	break;

    case NG_PROTOCOL_CRYPT_SSL:
	crypt = "SSL";
	comm = jobMng->ngjm_context->ngc_commSSL;
	break;

    default:
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Unknown Crypt Type %s",
	    fName, jobMng->ngjm_attr.ngja_rmInfo.ngrmi_crypt);
	goto error;
    }

    /* Get the protocol */
    switch (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_protocol) {
    case NG_PROTOCOL_TYPE_XML:
	protocol = "XML";
	break;

    case NG_PROTOCOL_TYPE_BINARY:
	protocol = "binary";
	break;

    default:
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Unknown Protocol Type %s",
	    fName, jobMng->ngjm_attr.ngja_rmInfo.ngrmi_protocol);
	goto error;
    }

    /**
     * Make the RSL string.
     */
    /* Initialize the length */
    length = 0;

    /* GASS URL */
    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	"&(rsl_substitution = (NG_GASS_URL %s))",
	jobMng->ngjm_attr.ngja_gassUrl);
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* jobType */
    if ((jobMng->ngjm_attr.ngja_backend == NG_BACKEND_MPI) ||
        (jobMng->ngjm_attr.ngja_backend == NG_BACKEND_BLACS)) {

        /* for MPI */
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(jobType=%s)", "mpi");
        if (length >= (NGCLI_RSL_NBYTES - 1))
            goto overflow;

        /* Count */
        length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
            "(count=%d)", jobMng->ngjm_attr.ngja_mpiNcpus);
        if (length >= (NGCLI_RSL_NBYTES - 1))
            goto overflow;

    } else {
        /* Count */
        length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
            "(count=%d)", jobMng->ngjm_attr.ngja_invokeNjobs);
        if (length >= (NGCLI_RSL_NBYTES - 1))
            goto overflow;
    }

    /* executable */
    if (jobMng->ngjm_attr.ngja_stagingEnable == 0) {
        /* No staging */
        length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
            "(executable=%s)",
	    jobMng->ngjm_attr.ngja_executablePath);
    } else {
        /* staging enabled */
        length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
            "(executable=$(NG_GASS_URL) # %s)",
            jobMng->ngjm_attr.ngja_executablePath);
    }
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* Arguments */
    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	"(arguments= \"--client=%s:%d\" \"--gassServer=\" # %s ",
	jobMng->ngjm_attr.ngja_clientHostName,
	comm->ngc_portNo,
	"$(NG_GASS_URL)");
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
        "\"--crypt=%s\" \"--protocol=%s\" \"--contextID=%d\" \"--jobID=%d\"",
	crypt, protocol,
	jobMng->ngjm_context->ngc_ID,
	jobMng->ngjm_ID);
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* heartBeat */
    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	" \"--heartbeat=%d\"", rmInfo->ngrmi_heartBeat);
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* TCP Connect Retry */
    if (rmInfo->ngrmi_retryInfo.ngcri_count > 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    " \"--connectRetry=%d,%d,%g,%s\"",
            rmInfo->ngrmi_retryInfo.ngcri_count,
            rmInfo->ngrmi_retryInfo.ngcri_interval,
            rmInfo->ngrmi_retryInfo.ngcri_increase,
            (rmInfo->ngrmi_retryInfo.ngcri_useRandom == 1 ?
                "random" : "fixed"));
    }
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* coreDumpSize (output if defined in configuration file) */
    if (rmInfo->ngrmi_coreDumpSize != -2) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    " \"--coreDumpSize=%d\"", rmInfo->ngrmi_coreDumpSize);
    }
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* debug by RemoteExecutable invoked debugger on termital */
    if (rmInfo->ngrmi_debug.ngdi_enable) {
	if (rmInfo->ngrmi_debug.ngdi_terminalPath != NULL) {
	    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		" \"--debugTerminal=%s\"",
		rmInfo->ngrmi_debug.ngdi_terminalPath);
	    if (length >= (NGCLI_RSL_NBYTES - 1))
		goto overflow;
	}

	if (rmInfo->ngrmi_debug.ngdi_display != NULL) {
	    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		" \"--debugDisplay=%s\"", rmInfo->ngrmi_debug.ngdi_display);
	    if (length >= (NGCLI_RSL_NBYTES - 1))
		goto overflow;
	}

	if (rmInfo->ngrmi_debug.ngdi_debuggerPath != NULL) {
	    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		" \"--debugger=%s\"", rmInfo->ngrmi_debug.ngdi_debuggerPath);
	    if (length >= (NGCLI_RSL_NBYTES - 1))
		goto overflow;
	}

	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    " \"--debugEnable=%d\"", rmInfo->ngrmi_debug.ngdi_enable);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* debugBusyLoop */
    if (rmInfo->ngrmi_debugBusyLoop) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: debug_busyLoop was set."
	    " please attach Remote Executable by debugger.\n",
	    fName);
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    " \"--debugBusyLoop=%d\"", rmInfo->ngrmi_debugBusyLoop);
    }
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* tcpNodelay */
    if (lmInfo->nglmi_tcpNodelay != 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    " \"--tcpNodelay=%d\"", 1/* true */);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* End Argument */
    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length, ")");
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* Environment variables */
    if (rmInfo->ngrmi_nEnvironments > 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(environment = ");
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;

	for (i = 0; i < rmInfo->ngrmi_nEnvironments; i++) {
	    env = strdup(rmInfo->ngrmi_environment[i]);
	    if (env == NULL) {
		NGI_SET_ERROR(error, NG_ERROR_MEMORY);
		ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't duplicate the Environment variable.\n",
		    fName);
		goto overflow;
	    }
	    tmp = strchr(env, '=');
	    if (tmp == NULL) {
		length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		    "(%s \"\")", env);
		if (length >= (NGCLI_RSL_NBYTES - 1))
		    goto overflow;
	    } else if (tmp[1] != '\0') {
		*tmp = '\0';
		length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		    "(%s \"%s\")", env, &tmp[1]);
		if (length >= (NGCLI_RSL_NBYTES - 1))
		    goto overflow;
	    } else {
		NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
		ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Environment variable \"%s\": Syntax error.\n",
		    fName);
		goto overflow;
	    }
	    globus_libc_free(env);
	}
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length, ")");
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* redirect stdout, stderr */
    if (rmInfo->ngrmi_redirectEnable != 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(stderr = $(NG_GASS_URL) # /dev/stderr)"
	    "(stdout = $(NG_GASS_URL) # /dev/stdout)");
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* workDirectory */
    if (rmInfo->ngrmi_workDirectory != NULL) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(directory=%s)", rmInfo->ngrmi_workDirectory);

    } else if (jobMng->ngjm_attr.ngja_stagingEnable == 0) {

	/* Make work directory from executable path */
	assert(jobMng->ngjm_attr.ngja_executablePath != NULL);
	tmp = strdup(jobMng->ngjm_attr.ngja_executablePath);
	if (tmp == NULL) {
	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't duplicate the Executable Path.\n", fName);
	    goto error;
	}

	p = strrchr(tmp, '/');
	if (p != NULL) {
	    assert(*p == '/');
	    *p = '\0';
	}

	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(directory=%s)", tmp);

	globus_libc_free(tmp);
    }
    if (length >= (NGCLI_RSL_NBYTES - 1))
	goto overflow;

    /* maxTime */
    if (rmInfo->ngrmi_jobMaxTime >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(maxTime=%d)", rmInfo->ngrmi_jobMaxTime);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* maxWallTime */
    if (rmInfo->ngrmi_jobMaxWallTime >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(maxWallTime=%d)", rmInfo->ngrmi_jobMaxWallTime);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* maxCpuTime */
    if (rmInfo->ngrmi_jobMaxCpuTime >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(maxCpuTime=%d)", rmInfo->ngrmi_jobMaxCpuTime);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* queue */
    if (jobMng->ngjm_attr.ngja_queueName != NULL) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(queue=\"%s\")", jobMng->ngjm_attr.ngja_queueName);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* project */
    if (rmInfo->ngrmi_jobProject != NULL) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(project=\"%s\")", rmInfo->ngrmi_jobProject);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* hostCount */
    if (rmInfo->ngrmi_jobHostCount >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(hostCount=%d)", rmInfo->ngrmi_jobHostCount);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* minMemory */
    if (rmInfo->ngrmi_jobMinMemory >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(minMemory=%d)", rmInfo->ngrmi_jobMinMemory);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* maxMemory */
    if (rmInfo->ngrmi_jobMaxMemory >= 0) {
	length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
	    "(maxMemory=%d)", rmInfo->ngrmi_jobMaxMemory);
	if (length >= (NGCLI_RSL_NBYTES - 1))
	    goto overflow;
    }

    /* RSL extensions */
    if (rmInfo->ngrmi_rslExtensionsSize > 0) {
	for (i = 0; i < rmInfo->ngrmi_rslExtensionsSize; i++) {
	    length += snprintf(&rsl[length], NGCLI_RSL_NBYTES - length,
		"%s", rmInfo->ngrmi_rslExtensions[i]);
	    if (length >= (NGCLI_RSL_NBYTES - 1))
		goto overflow;
        }
    }

    /* Print out */
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,	"%s: RSL: \"%s\".\n", fName, rsl);

    /* Success */
    return rsl;

    /* Error occurred */
overflow:
    /* Overflow */
    NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	"%s: RSL buffer is overflow.\n", fName);

error:
    /* Deallocate */
    globus_libc_free(rsl);

    /* Failed */
    return NULL;
}

/**
 * Release the RSL.
 */
static int
ngcllJobReleaseRSL(ngcliJobManager_t *jobMng, char *rsl, int *error)
{
    /* Deallocate */
    globus_libc_free(rsl);

    /* Success */
    return 1;
}

/**
 * Make the Resource Manager Contact.
 */
static int
ngcllJobMakeRMcontact(ngcliJobManager_t *jobMng, int *error)
{
    char *rmc;
    int bufferNbytes, length;
    ngcliJobAttribute_t *jobAttr;
    static const char fName[] = "ngcllJobMakeRMcontact";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);
    assert(jobMng->ngjm_rmContact == NULL);

    bufferNbytes = 0;
    length = 0;
    rmc = NULL;
    jobAttr = &jobMng->ngjm_attr;

    /* Calculate the size of Resource Manager Contact */
    bufferNbytes = strlen(jobAttr->ngja_hostName) + 1;
    if (jobAttr->ngja_portNo != NGI_PORT_ANY) {
        /* ':' + "65535" + margin */
        bufferNbytes += 8;
    }
    if (jobAttr->ngja_jobManager != NULL) {
        bufferNbytes += strlen(jobAttr->ngja_jobManager) + 1;
        bufferNbytes += 8;    /* margin */
    }
    if (jobAttr->ngja_subject != NULL) {
        bufferNbytes += strlen(jobAttr->ngja_subject) + 1;
        bufferNbytes += 8;    /* margin */
    }

    /* Allocate the storage for Resource Manager Contact */
    rmc = globus_libc_malloc(bufferNbytes);
    if (rmc == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Resource Manager Contact.\n",
            fName);
        return 0;
    }

    length = 0;

    /* Make Resource Manager Contact */

    length += snprintf(&rmc[length], bufferNbytes,
        "%s", jobAttr->ngja_hostName);

    if (jobAttr->ngja_portNo != NGI_PORT_ANY) {
        length += snprintf(&rmc[length], bufferNbytes,
            ":%d", jobAttr->ngja_portNo);
    } else if ((jobAttr->ngja_jobManager == NULL) &&
        (jobAttr->ngja_subject != NULL)) {
        length += snprintf(&rmc[length], bufferNbytes, ":");
    }

    if (jobAttr->ngja_jobManager != NULL) {
        length += snprintf(&rmc[length], bufferNbytes,
            "/%s", jobAttr->ngja_jobManager);
    }

    if (jobAttr->ngja_subject != NULL) {
        length += snprintf(&rmc[length], bufferNbytes,
            ":%s", jobAttr->ngja_subject);
    }

    if (length >= bufferNbytes) {
        globus_libc_free(rmc);
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: RSL buffer is overflow.\n", fName);
        return 0;
    }

    jobMng->ngjm_rmContact = rmc;

    /* log */
    ngcliLogPrintfJob(jobMng,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: RMcontact: \"%s\".\n", fName, jobMng->ngjm_rmContact);

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
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
	NULL, "%s: Set the request for Job cancel.\n", fName);

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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Job is not invoked.\n", fName);

        /* Success */
        return 1;
    }

    /* Cancel request for the Job if necessary */
    if (jobMng->ngjm_useInvokeServer != 0) {
        result = ngcliInvokeServerJobCancel(
            jobMng->ngjm_context, jobMng, &doNotWait, error);
    } else {
        result = ngcllJobCancelByGRAM(jobMng, &doNotWait, error);
    }
    if (result == 0) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't cancel the job.\n", fName);
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
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't wait the Job done.\n", fName);
        error = NULL;
        ret = 0;
    }

    if (jobMng->ngjm_useInvokeServer != 0) {
        /* Tell the Invoke Server to destroy the job */
        result = ngcliInvokeServerJobStop(
            jobMng->ngjm_context, jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't stop the job.\n", fName);
            error = NULL;
            ret = 0;
        }
    } else {
        /* Disallow the callback */
        result = ngcllJobGRAMcallbackDisallow(jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't disallow the callback.\n", fName);
            error = NULL;
            ret = 0;
        }
    }

    return ret;
}

/**
 * Cancel the remote job by GRAM.
 */
static int
ngcllJobCancelByGRAM(
    ngcliJobManager_t *jobMng,
    int *doNotWait,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllJobCancelByGRAM";

    /* Check the arguments */
    assert(jobMng != NULL);
    assert(jobMng->ngjm_context != NULL);
    assert(doNotWait != NULL);

    /* Initialize the local variables */
    log = jobMng->ngjm_context->ngc_log;
    *doNotWait = 0;

    /* Output the log */
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
        NG_LOG_LEVEL_DEBUG, NULL, "%s: Job Contact: %s.\n",
        fName, jobMng->ngjm_jobContact);

    /* Check the arguments */
    assert(jobMng->ngjm_callbackContact != NULL);
    assert(jobMng->ngjm_jobContact != NULL);

    /* Cancel the job */
    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Job is already done. suppress job cancel.\n", fName);

    } else if (jobMng->ngjm_requestCancel == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_DEBUG, NULL,
	    "%s: Job cancel was not requested: All Executables are exited.\n",
            fName);
    } else {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_DEBUG, NULL, "%s: Job Contact: %s.\n",
            fName, jobMng->ngjm_jobContact);
	result = globus_gram_client_job_cancel(jobMng->ngjm_jobContact);
	if (result != GLOBUS_SUCCESS) {
	    ngLogPrintf(log,
		NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_WARNING, NULL,
		"%s: globus_gram_client_job_cancel failed by %d: %s.\n",
		fName, result, globus_gram_client_error_string(result));

            /**
             * Cancel was failed, Thus, wait the job done should not be
             * done, although return code was successful.
             */
            *doNotWait = 1;

	    /* Success */
	    return 1;
	}
    }

    if (jobMng->ngjm_attr.ngja_stopTimeout == 0) {
        /* Do  not wait job done if the stopTimeout == 0 */
        result = globus_gram_client_job_callback_unregister(
                jobMng->ngjm_jobContact, jobMng->ngjm_callbackContact,
                NULL, NULL);
        if (result != GLOBUS_SUCCESS) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: globus_gram_client_job_callback_unregister "
                "failed by %d: %s.\n",
                fName, result, globus_gram_client_error_string(result));
        }
    }

    /* Success */
    return 1;
}

/**
 * Job : Cancel job by force
 */ 
int
ngcliJobCancelByForce(
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcliJobCancelByForce";

    /* Check arguments */
    assert(jobMng != NULL);

    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Job is already done. suppress job cancel.\n", fName);
        goto success;
    }

    /* Do nothing If Invoke Server is used. */
    if (jobMng->ngjm_useInvokeServer == 0) {
        result = ngcllJobCancelByForceByGRAM(jobMng, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng,
                NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't cancel the GRAM job.\n",
                fName);

            goto error;
        }
    }

    /* Success */
success:

    return 1;

    /* Error occurred */
error:
    return 0;
}

/**
 * Job : Cancel GRAM job by force
 */ 
static int
ngcllJobCancelByForceByGRAM(
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllJobCancelByForceByGRAM";

    /* Check arguments */
    assert(jobMng != NULL);

    if (jobMng->ngjm_jobContact == NULL) {
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: Job Contact is invalid.\n",
            fName);
        return 1;
    }

    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
        NG_LOG_LEVEL_DEBUG, NULL, "%s: Cancel the Job: Job Contact: %s.\n",
        fName, jobMng->ngjm_jobContact);

    result = globus_gram_client_job_cancel(jobMng->ngjm_jobContact);
    if (result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngcliLogPrintfJob(jobMng,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: globus_gram_client_job_cancel failed by %d: %s.\n",
            fName, result, globus_gram_client_error_string(result));
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    return 0;
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
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Wait done or failed */
    while ((jobMng->ngjm_status != NGI_JOB_STATUS_DONE) &&
           (jobMng->ngjm_status != NGI_JOB_STATUS_FAILED)) {
        result = ngiCondWait(
            &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
	}
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
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
#ifdef NG_PTHREAD
{
    int result;
    int retResult = 0;
    int remain;
    int timeout;
    static const char fName[] = "ngcllJobWaitDoneTimeout";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't lock the Mutex.\n", fName);
        return 0;
    }

    /* Wait done or failed */
    remain = second;
    while ((jobMng->ngjm_status != NGI_JOB_STATUS_DONE) &&
           (jobMng->ngjm_status != NGI_JOB_STATUS_FAILED)) {
        timeout = 0;
        result = ngiCondTimedWait(
            &jobMng->ngjm_monCond, &jobMng->ngjm_monMutex, 1, &timeout,
            log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable.\n", fName);
            goto error;
	}

        /* Timedout? */
        if (timeout != 0) {
            remain--;
            if (remain <= 0) {
                NGI_SET_ERROR(error, NG_ERROR_TIMEOUT);
                ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Timeout. waited %d seconds.\n", fName, second);
                goto error;
            }
        }
    }

    if ((jobMng->ngjm_status == NGI_JOB_STATUS_DONE) ||
        (jobMng->ngjm_status == NGI_JOB_STATUS_FAILED)) {
        retResult = 1;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
        return 0;
    }

    /* Success */
    return retResult;

    /* Error occurred */
error:
    /* Unlock the mutex */
    result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, NULL);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex.\n", fName);
    }

    /* Failed */
    return 0;
}
#else /* NG_PTHREAD */
{
    int result;
    int remain;
    ngcliJobStatus_t jobStatus;
    static const char fName[] = "ngcllJobWaitDoneTimeout";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Initialize the local variable */
    remain = second;

    /* Wait done */
    while (1) {
        /* Lock the mutex */
        result = ngiMutexLock(&jobMng->ngjm_monMutex, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex.\n", fName);
            return 0;
        }

        /* Copy the status */
        jobStatus = jobMng->ngjm_status;

        /* Unlock the mutex */
        result = ngiMutexUnlock(&jobMng->ngjm_monMutex, log, error);
        if (result == 0) {
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex.\n", fName);
            return 0;
        }

        /* Is job done or failed? */
        if ((jobStatus == NGI_JOB_STATUS_DONE) ||
            (jobStatus == NGI_JOB_STATUS_FAILED)) {
            return 1;
        }

        /* Timeout? */
        if (remain <= 0) {
            NGI_SET_ERROR(error, NG_ERROR_TIMEOUT);
            error = NULL;
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Timeout. waited %d seconds.\n", fName, second);
            return 0;
        }

        /* Wait 1sec */
retry:
        result = poll(NULL, 0, 1000);
        if (result < 0) {
            if (errno == EINTR)
                goto retry;

            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: poll failed: %s.\n", fName, strerror(errno));
            return 0;
        }
        remain--;

        globus_thread_yield();
    }
}
#endif /* NG_PTHREAD */

/**
 * Callback function.
 */
static void
ngcllJobGRAMcallback(
    void *userData,
    char *jobContact,
    int state,
    int errorcode)
{
    int result;
    int error;
    ngcliJobManager_t *jobMng;
    ngcliJobStatus_t status;
    char *msg;
    ngLog_t *log;
    static const char fName[] = "ngcllJobGRAMcallback";

    /* Check the arguments */
    assert(userData != NULL);

    /* Get the Job Manager */
    jobMng = userData;

    /* Is Job Manager valid? */
    result = ngcliJobIsValid(NULL, jobMng, &error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Job is not valid.\n", fName);
	return;
    }

    /* Print the debug message  */
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG,
	NULL, "%s: Job status %d.\n", fName, state);

    /* Get the Log Manager */
    log = jobMng->ngjm_context->ngc_log;

    switch(state)
    {
    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
	status = NGI_JOB_STATUS_PENDING;
	msg = "pending";
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
	status = NGI_JOB_STATUS_ACTIVE;
	msg = "active";

	/* Set the end time */
        result = ngcliJobSetEndTimeOfInvoke(jobMng, log, &error);
        if (result == 0) {
	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't set the End time.\n", fName);
	}
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
	status = NGI_JOB_STATUS_FAILED;
	msg = "failed";

	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    (jobMng->ngjm_requestCancel == 0 ?
                NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_INFORMATION), NULL,
	    "%s: GRAM Job failed because %s (error code %d).\n",
	    fName, globus_gram_client_error_string(errorcode), errorcode);

        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
	status = NGI_JOB_STATUS_DONE;
	msg = "done";
	break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED:
	status = NGI_JOB_STATUS_UNSUBMITTED;
	msg = "unsubmitted";
	break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN:
	status = NGI_JOB_STATUS_STAGE_IN;
	msg = "stage in";
	break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_OUT:
	status = NGI_JOB_STATUS_STAGE_OUT;
	msg = "stage out";
	break;

    default:
	status = NGI_JOB_STATUS_UNKNOWN;
	msg = "Unknown";
	break;
    }

    /* Is job dead? */
    if ((status == NGI_JOB_STATUS_FAILED) ||
        (status == NGI_JOB_STATUS_DONE)) {
	result = ngcliExecutableJobDone(jobMng, log, &error);
	if (result == 0) {
	    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't noify the Job done to Executable.\n", fName);
	    /* Note: Not returned here. Because should execute the
             * ngcliJobNotifyStatus.
	     */
	}
    }

    /* Print the information */
    ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	NG_LOG_LEVEL_INFORMATION, NULL, "%s: Job is %s.\n", fName, msg);

    /* Notify the Job Status */
    result = ngcliJobNotifyStatus(jobMng, status, log, &error);
    if (result == 0) {
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't noify the Job Status.\n", fName);
	return;
    }

    /* Success */
    return;
}

/**
 * Set proxy credential.
 */
int
ngcliJobSetCredential(
    gss_cred_id_t gss_cred, int *error)
{
    int gResult;

    /* Set proxy credential for GRAM */
    gResult = globus_gram_client_set_credentials(gss_cred);

    if (gResult == GLOBUS_SUCCESS) {
        return 1;
    } else {
        /* will never come here */
        return 0;
    }
}

/**
 * Refresh credential of JobManager.
 */
int
ngcliJobRefreshCredential(
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    char *contact;
    static const char fName[] = "ngcliJobRefreshCredential";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Skip Invoke Server */
    if (jobMng->ngjm_useInvokeServer != 0) {

        /* Success */
        return 1;
    }

    contact = jobMng->ngjm_jobContact;
    if (contact == NULL) {
        /* Success */
        return 1;
    }

    /* refresh credential of the JobManager */
    result = globus_gram_client_job_refresh_credentials(
	contact, GSS_C_NO_CREDENTIAL);
    if (result != GLOBUS_SUCCESS) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: Failed to refresh credential for %s.\n", fName,
            contact);
        return 0;
    }

    /* Success */
    return 1;
}

/*
 * Globus Gram Callback: Allow
 */
static int
ngcllJobGRAMcallbackAllow(
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    static const char fName[] = "ngcllJobGRAMcallbackAllow";

    assert(jobMng != NULL);
    assert(jobMng->ngjm_callbackContact == NULL);

    result = globus_gram_client_callback_allow(
	ngcllJobGRAMcallback, jobMng, &jobMng->ngjm_callbackContact);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_JOB_INVOKE);
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: globus_gram_client_callback_allow failed by %d: %s.\n",
	    fName, result, globus_gram_client_error_string(result));
        return 0;
    }

    /* Success */
    return 1;
}

/*
 * Globus Gram Callback: Disallow
 */
static int
ngcllJobGRAMcallbackDisallow(
    ngcliJobManager_t *jobMng,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngcllJobGRAMcallbackDisallow";

    assert(jobMng != NULL);

    if (jobMng->ngjm_callbackContact == NULL) {
        /* Do nothing */
        return 1;
    }

    /* Disallow the callback */
    result = globus_gram_client_callback_disallow(jobMng->ngjm_callbackContact);
    if (result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_JOB_INVOKE);
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_gram_client_callback_disallow failed by %d: %s.\n",
            fName, result, globus_gram_client_error_string(result));
        error = NULL;
        ret = 0;
    }

    /* Free the storage for callback contact */
    globus_free(jobMng->ngjm_callbackContact);
    jobMng->ngjm_callbackContact = NULL;

    return ret;
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
	ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Local Machine Information is not exist.\n", fName);
	return 0;
    }

    /* Success */
    *fortranCompatible = jobMng->ngjm_attr.ngja_lmInfo.nglmi_fortranCompatible;
    return 1;
}
