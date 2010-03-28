/*
 * $RCSfile: ngInvokeServer.c,v $ $Revision: 1.5 $ $Date: 2008/02/27 09:56:32 $
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

#include "ngisList.h"
#include "ngisLog.h"
#include "ngInvokeServer.h"

NGI_RCSID_EMBED("$RCSfile: ngInvokeServer.c,v $ $Revision: 1.5 $ $Date: 2008/02/27 09:56:32 $")

#define NGISL_MSG_NOT_INITIALIZED "Invoke Server still has not been initialized.\n"

/* File local variables */
static int nglInvokeServerInitialized = 0;
static ngInvokeServer_t nglInvokeServer;

/* File local functions */
static void nglInvokeServerInitializeMember(ngInvokeServer_t *);

/**
 * This function initializes Invoke Server.
 */
int
ngInvokeServerInitialize(
    ngisJobTypeInformation_t *jobTypeInformation)
{
    int result;
    int callbackManagerInitialized = 0;
    ngisLog_t *newLog = NULL;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngInvokeServerInitialize";
    
    /* Check the arguments */
    NGIS_ASSERT(jobTypeInformation != NULL);
    NGIS_ASSERT(jobTypeInformation->ngjti_size >= sizeof(ngisJob_t));
    NGIS_ASSERT(jobTypeInformation->ngjti_initializer != NULL);
    NGIS_ASSERT(jobTypeInformation->ngjti_finalizer  != NULL);
    NGIS_ASSERT(jobTypeInformation->ngjti_canceler   != NULL);

    if (nglInvokeServerInitialized != 0) {
        ngisErrorPrint(log, fName,
            "Invoke Server has already been initialized.\n");
        return 0;
    }
    nglInvokeServerInitializeMember(&nglInvokeServer);
    nglInvokeServer.ngis_jobTypeInformation = jobTypeInformation;
    nglInvokeServer.ngis_log                = NULL;

    newLog = ngisLogCreate("Invoke Server");
    if (newLog == NULL) {
        ngisErrorPrint(log, fName, 
            "Can't create log for Invoke Server.\n");
        goto error;
    }
    nglInvokeServer.ngis_log = log = newLog;
    nglInvokeServerInitialized = 1;

    /* Initialize Callback Manager */
    result = ngisCallbackManagerInitialize();
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't initialize the Callback Manager.\n");
        goto error;
    }
    callbackManagerInitialized = 1;

    /* Register the input source from Ninf-G Client */
    nglInvokeServer.ngis_protocol = ngisProtocolCreate();
    if (nglInvokeServer.ngis_protocol == NULL) {
        ngisErrorPrint(log, fName, "Can't create the protocol.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    nglInvokeServerInitialized = 0;

    if (nglInvokeServer.ngis_protocol != NULL) {
        result = ngisProtocolDestroy(nglInvokeServer.ngis_protocol);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy the protocol talker.\n");
        }
        nglInvokeServer.ngis_protocol = NULL;
    }
    if (callbackManagerInitialized == 0) {
        result = ngisCallbackManagerFinalize();
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't finalize the Callback Manager.\n");
        }
    }
    if (newLog != NULL) {
        result = ngisLogDestroy(newLog);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy the log.\n");
        }
        newLog = NULL;
        nglInvokeServer.ngis_log = NULL;
    }

    nglInvokeServerInitializeMember(&nglInvokeServer);
    
    /* Failed */
    return 0;
}

/**
 * This function finalizes Invoke Server.
 */
int
ngInvokeServerFinalize()
{
    int result;
    int ret = 1;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngInvokeServerFinalize";

    /* Initialized ? */
    if (nglInvokeServerInitialized == 0) {
        ngisErrorPrint(log, fName, NGISL_MSG_NOT_INITIALIZED);
        return 0;        
    }    
    nglInvokeServerInitialized = 0;

    log = nglInvokeServer.ngis_log;

    result = ngisCallbackManagerFinalize();
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't finalize the Callback Manager.\n");
        ret = 0;
    }

    result = ngisProtocolDestroy(nglInvokeServer.ngis_protocol);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy the protocol talker.\n");
        ret = 0;
    }
    nglInvokeServer.ngis_protocol = NULL;

    result = ngisLogDestroy(nglInvokeServer.ngis_log);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy the log.\n");
        ret = 0;
    }
    nglInvokeServerInitializeMember(&nglInvokeServer);
    
    return ret;
}

/**
 * This function clears ngInvokeServer's members.
 */
static void
nglInvokeServerInitializeMember(
    ngInvokeServer_t *invokeServer)
{
    NGIS_ASSERT(invokeServer != NULL);
    
    invokeServer->ngis_log                = NULL;
    invokeServer->ngis_jobTypeInformation = NULL;
    invokeServer->ngis_protocol           = NULL;
}

/**
 * This function is event loop.
 * It call back the registered function, when it read a line.
 */
int
ngInvokeServerRun()
{
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngInvokeServerRun";

    /* Initialized? */
    if (nglInvokeServerInitialized == 0) {
        ngisErrorPrint(log, fName, NGISL_MSG_NOT_INITIALIZED);
        return 0;        
    }    
    log = nglInvokeServer.ngis_log;

    result = ngisCallbackManagerRun();
    if (result == 0) {
        ngisErrorPrint(log, fName, "Error occurred in event loop.\n");
        return 0;
    }
    return 1;
}

/*============================================================================*/
/* Job                                                                        */
/*============================================================================*/

/**
 * Job: Create
 */
ngisJob_t *
ngisJobCreate(
    ngisProtocol_t *protocol,
    char *requestId,
    ngisOptionContainer_t *options)
{
    ngisJob_t *new = NULL;
    int result;
    ngisLog_t *log = NULL;
    char *requestIDcopy = NULL;
    int registered = 0;
    ngisJobTypeInformation_t *jobTypeInformation;
    static const char fName[] = "ngisJobCreate";

    NGIS_ASSERT(options != NULL);
    NGIS_ASSERT(NGIS_STRING_IS_NONZERO(requestId));

    if (nglInvokeServerInitialized == 0) {
        ngisErrorPrint(log, fName, NGISL_MSG_NOT_INITIALIZED);
        return 0;
    }

    /* Get Job Type Information */
    jobTypeInformation = nglInvokeServer.ngis_jobTypeInformation;

    /* Allocate */
    new = (ngisJob_t *)malloc(jobTypeInformation->ngjti_size);
    if (new == NULL) {
        ngisErrorPrint(log, fName, "malloc: %s.\n", strerror(errno));
        goto error;
    }

    /* Initialize Members */
    new->ngj_jobID            = NULL;
    new->ngj_requestID        = NULL;
    new->ngj_protocol         = protocol;
    new->ngj_status           = NGIS_STATUS_PENDING;
    new->ngj_log              = NULL;
    new->ngj_destroyRequested = 0;

    /* Create Log */
    log = ngisLogCreatef("Job:RequestID-%s", requestId);
    if (log == NULL) {
        ngisErrorPrint(log, fName, "Can't create new log.\n");
        goto error;
    }
    new->ngj_log = log;
    ngisDebugPrint(log, fName, "Job creating...\n");

    /* Copy request ID */
    requestIDcopy = strdup(requestId);
    if (requestIDcopy == NULL) {
        ngisErrorPrint(log, fName, "strdup: %s.\n", strerror(errno));
        goto error;
    }
    new->ngj_requestID = requestIDcopy;

    result = ngisProtocolRegisterJob(protocol, new);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't register to the protocol.\n");
        goto error;
    }
    registered = 1;

    /* Initialize */
    result = jobTypeInformation->ngjti_initializer(new, options);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't initialize the job.\n");
        goto error;
    }

    return new;
error:
    if ((new != NULL) && (registered != 0)) {
        result = ngisProtocolUnregisterJob(protocol, new);
        registered = 0;
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't unregister to the protocol.\n");
        }
    }
    if (log != NULL) {
        result = ngisLogDestroy(log);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy the log.\n");
        }
        log = NULL;
    }

    NGIS_NULL_CHECK_AND_FREE(requestIDcopy);
    NGIS_NULL_CHECK_AND_FREE(new);

    return NULL;
}

/**
 * Job: Destroy
 */
int
ngisJobDestroy(
    ngisJob_t *job)
{
    int result;
    ngisLog_t *log;
    ngisJobTypeInformation_t *jobTypeInformation;
    static const char fName[] = "ngisJobDestroy";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngj_log != NULL);

    log = NULL;

    /* Get Job Type Information */
    jobTypeInformation = nglInvokeServer.ngis_jobTypeInformation;

    if (nglInvokeServerInitialized == 0) {
        ngisErrorPrint(log, fName, NGISL_MSG_NOT_INITIALIZED);
        return 0;
    }
    log = job->ngj_log;
    ngisDebugPrint(log, fName, "Job destroying...\n");

    /* Finalize */
    result = jobTypeInformation->ngjti_finalizer(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't finalize the job.\n");
    }

    if (job->ngj_protocol != NULL) {
        result = ngisProtocolUnregisterJob(job->ngj_protocol, job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't unregister from the protocol.\n");
        }
    }

    /* Destroy Log */
    log = NULL;
    result = ngisLogDestroy(job->ngj_log);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy the log.\n");
    }

    /* Initialize Members */
    NGIS_NULL_CHECK_AND_FREE(job->ngj_jobID);
    NGIS_NULL_CHECK_AND_FREE(job->ngj_requestID);

    job->ngj_protocol         = NULL;
    job->ngj_status           = NGIS_STATUS_DONE;
    job->ngj_log              = NULL;
    job->ngj_destroyRequested = 0;

    NGIS_NULL_CHECK_AND_FREE(job);

    return 1;
}

/**
 * Job: Set status
 */
int
ngisJobSetStatus(
    ngisJob_t *job,
    ngisJobStatus_t status,
    char *message)
{
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisJobSetStatus";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(message != NULL);
    NGIS_JOB_STATUS_ASSERT(status);

    log = job->ngj_log;

    if (!NGIS_STRING_IS_NONZERO(job->ngj_jobID)) {
        ngisErrorPrint(log, fName, "Has not set Job ID.\n");
        return 0;
    }
    
    ngisDebugPrint(log, fName, "%s => %s.\n",
        ngisJobStatusStrings[job->ngj_status],
        ngisJobStatusStrings[status]);
    
    if (job->ngj_status > status) {
        ngisErrorPrint(log, fName, "Invalid the of status change.\n");
        return 0;
    }    
    if (job->ngj_status < status) {
        job->ngj_status = status;
        /* Notify to Ninf-G Client */
        if (job->ngj_protocol != NULL) {
            result = ngisProtocolSendStatusNotify(
                job->ngj_protocol,
                job->ngj_jobID, status, message);
            if (result == 0) {
                ngisErrorPrint(log, fName, 
                    "Can't send status notify to Ninf-G Client.\n");
                return 0;
            }
        }
    }
    
    /* Successful */
    return 1;
}

/**
 * Job: Get status
 */
ngisJobStatus_t
ngisJobGetStatus(
    ngisJob_t *job)
{
#if 0
    static const char fName[] = "ngisJobGetStatus";
#endif

    NGIS_ASSERT(job != NULL);
    NGIS_JOB_STATUS_ASSERT(job->ngj_status);
    
    return job->ngj_status;
}

/**
 * Job: Register Job ID
 */
int
ngisJobRegisterID(
    ngisJob_t *job,
    char *jobID)
{
    ngisProtocol_t *protocol = NULL;
    ngisJob_t *exist = NULL;
    ngisLog_t *log;
    ngisLog_t *newLog = NULL;
    int result;
    static const char fName[] = "ngisJobRegisterID";

    NGIS_ASSERT(job   != NULL);
    NGIS_ASSERT(jobID != NULL);
    NGIS_ASSERT(job->ngj_requestID != NULL);
    NGIS_ASSERT(job->ngj_jobID     == NULL);

    protocol = job->ngj_protocol;
    log      = job->ngj_log;

    if (protocol == NULL) {
        ngisDebugPrint(log, fName, "Can't talk Ninf-G Client.\n");
        /* Success */
        return 1;
    }

    /* Check */
    exist = ngisProtocolFindJob(protocol, jobID);
    if (exist != NULL) {
        ngisErrorPrint(log, fName,
            "\"%s\" is already used as Job ID.\n", jobID);
        return 0;
    }

    /* Register */

    newLog = ngisLogCreatef("Job:%s", jobID);
    if (newLog == NULL) {
        ngisErrorPrint(log, fName, "Can't create a new log.\n");
    } else {
        result = ngisLogDestroy(log);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy a log.\n");
        }
        job->ngj_log = log = newLog;
    }

    job->ngj_jobID = strdup(jobID);
    if (job->ngj_jobID == NULL) {
        ngisErrorPrint(log, fName, "Can't copy string of Job ID.\n");
        return 0;
    }

    result = ngisProtocolSendCreateNotify(
        protocol, 1, job->ngj_requestID, jobID);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't send create notify.\n");

        log = NULL;
        result = ngisProtocolDisable(protocol);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't disable the protocol.\n");
        }
        NGIS_NULL_CHECK_AND_FREE(job->ngj_jobID);

        return 0;
    }
    return 1;
}

/**
 * Job: Cancel
 * This function is interface to function set by user.
 */
int
ngisJobCancel(
    ngisJob_t *job)
{
    int result;
    ngisLog_t *log;
    ngisJobTypeInformation_t *jobTypeInformation;
    static const char fName[] = "ngisJobCancel";

    NGIS_ASSERT(job != NULL);

    log = NULL;
    jobTypeInformation = nglInvokeServer.ngis_jobTypeInformation;

    if (nglInvokeServerInitialized == 0) {
        ngisErrorPrint(log, fName, NGISL_MSG_NOT_INITIALIZED);
        return 0;
    }
    log = job->ngj_log;

    job->ngj_destroyRequested = 1;
    result = jobTypeInformation->ngjti_canceler(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't cancel the job.\n");
        return 0;
    }
    
    return 1;
}
