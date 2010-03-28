#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngisJob.c,v $ $Revision: 1.8 $ $Date: 2006/08/21 02:19:05 $";
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
 * Job for Invoke Server.
 */

#ifdef NG_PTHREAD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include "ngInvokeServer.h"

/**
 * Prototype
 */
static ngisiJob_t *ngislJobAllocate(ngisiContext_t *, int *);
static int ngislJobFree(ngisiContext_t *, ngisiJob_t *, int *);
static int ngislJobInitialize(
    ngisiContext_t *, ngisiJob_t *, char *, ngisiCreateAttr_t *, int *);
static int ngislJobFinalize(
    ngisiContext_t *, ngisiJob_t *, int *);
static int ngislJobInitializeSync(ngisiContext_t *, ngisiJob_t *, int *);
static int ngislJobFinalizeSync(ngisiContext_t *, ngisiJob_t *, int *);
static void ngislJobInitializeMember(ngisiJob_t *);
static int ngislJobRegister(
    ngisiContext_t *, ngisiJob_t *, int *);
static int ngislJobUnregister(
    ngisiContext_t *, ngisiJob_t *, int *);

static int ngislJobAttributeInitialize(
    ngisiContext_t *, ngisiJob_t *, ngisiJobAttribute_t *,
    ngisiCreateAttr_t *, int *);
static int ngislJobAttributeFinalize(
    ngisiContext_t *, ngisiJob_t *, ngisiJobAttribute_t *, int *);
static int ngislJobAttributeGetEnum(char *, int, ...);
static int ngislJobAttributeGetStrArray(
    ngisiContext_t *, ngisiJob_t *, ngisiCreateAttr_t *,
    char *, int *, char ***, int *);
static void ngislJobAttributeInitializeMember(ngisiJobAttribute_t *);
static void ngislJobAttributeSetDefault(ngisiJobAttribute_t *);

static int ngislJobListLock(ngisiContext_t *, int *);
static int ngislJobListUnlock(ngisiContext_t *, int *);
static int ngislJobLock(ngisiContext_t *, ngisiJob_t *, int *);
static int ngislJobUnlock(ngisiContext_t *, ngisiJob_t *, int *);
 
static int ngislJobMakeRMcontact(
    ngisiContext_t *, ngisiJob_t *, char **, int *);
static int ngislJobMakeRSL(
    ngisiContext_t *, ngisiJob_t *, char **, int *);

static void ngislJobGRAMcallback(
    void *, char *, int, int);
static int ngislJobNotifyStatus(
    ngisiContext_t *, ngisiJob_t *, ngisiJobStatus_t, char *, int *);
static int ngislJobFindByJobID(
    ngisiContext_t *, char *, ngisiJob_t **, int *, int *);

static int ngislJobRefreshCredentialAcquire(
    ngisiContext_t *context, gss_cred_id_t *gssCred, int *error);
static int ngislJobRefreshCredentialSet(
    ngisiContext_t *context, gss_cred_id_t gssCred, int *error);
static int ngislJobRefreshCredentialOneJob(
    ngisiContext_t *context, ngisiJob_t *job, int *error);

static int ngislLogPrintfJob(
    ngisiJob_t *, ngisiLogLevel_t, const char *, char *, ...);

/**
 * Job Construct
 */
ngisiJob_t *
ngisiJobConstruct(
    ngisiContext_t *context,
    char *requestID,
    ngisiCreateAttr_t *createAttr,
    int *error)
{
    static const char fName[] = "ngisiJobConstruct";
    int result, listLocked;
    ngisiJob_t *newJob;

    /* Check the arguments */
    assert(context != NULL);
    assert(createAttr != NULL);

    listLocked = 0;
    newJob = NULL;

    if (requestID == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Request ID is NULL.\n");
        goto error;
    }

    result = ngislJobListLock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list failed.\n");
        goto error;
    }
    listLocked = 1;

    if (context->ngisc_nJobs >= NGISI_JOB_NUM_MAX) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Exceeds the Job limit. max = %d, current = %d.\n",
            NGISI_JOB_NUM_MAX, context->ngisc_nJobs);
        goto error;
    }

    newJob = ngislJobAllocate(context, error);
    if (newJob == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the Job failed.\n");
        goto error;
    }

    result = ngislJobInitialize(
        context, newJob, requestID, createAttr, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Job failed.\n");
        goto error;
    }

    result = ngislJobRegister(context, newJob, error);
    if (result == 0) {
        ngislLogPrintfJob(newJob, NGISI_LOG_LEVEL_ERROR, fName,
            "Register the Job failed.\n");
        goto error;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Currently %d jobs are available.\n",
        context->ngisc_nJobs);

    result = ngislJobListUnlock(context, error);
    listLocked = 0;
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list failed.\n");
        goto error;
    }

    /* Success */
    return newJob;

    /* Error occurred */
error:

    if (listLocked != 0) {
        listLocked = 0;
        result = ngislJobListUnlock(context, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list failed.\n");
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Job Destruct
 */
int
ngisiJobDestruct(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngisiJobDestruct";
    int result, listLocked, jobLocked;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    listLocked = 0;
    jobLocked = 0;

    result = ngislJobListLock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list failed.\n");
        goto error;
    }
    listLocked = 1;

    /* Wait the Job unlocked */
    result = ngislJobLock(context, job, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job failed.\n");
        goto error;
    }
    jobLocked = 1;

    result = ngislJobUnlock(context, job, error);
    jobLocked = 0;
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job failed.\n");
        goto error;
    }

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "Destructing the job.\n");

    result = ngislJobUnregister(context, job, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unregister the Job failed.\n");
        goto error;
    }

    result = ngislJobFinalize(
        context, job, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Job failed.\n");
        goto error;
    }

    result = ngislJobFree(context, job, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Free the Job failed.\n");
        goto error;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Currently %d jobs are available.\n",
        context->ngisc_nJobs);

    result = ngislJobListUnlock(context, error);
    listLocked = 0;
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (jobLocked != 0) {
        jobLocked = 0;
        result = ngislJobUnlock(context, job, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job failed.\n");
        }
    }

    if (listLocked != 0) {
        listLocked = 0;
        result = ngislJobListUnlock(context, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Destruct All the Job after EXIT.
 */
int
ngisiJobDestructAll(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiJobDestructAll";

    /* Check the arguments */
    assert(context != NULL);

    if (context->ngisc_job_head == NULL) {
        /* log */
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "All jobs were destructed. Do nothing.\n");

        /* Success */
        return 1;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Currently %d jobs are available.\n",
        context->ngisc_nJobs);

    /* WARNING */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
        "Not all the jobs were destructed.\n");
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
        "Destructing All jobs unimplemented yet.\n");

    /* Success */
    return 1;
}


/**
 * Job Allocate
 */
static ngisiJob_t *
ngislJobAllocate(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislJobAllocate";
    ngisiJob_t *newJob;

    /* Check the arguments */
    assert(context != NULL);

    newJob = globus_libc_calloc(1, sizeof(ngisiJob_t));
    if (newJob == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the Job failed.\n");
        return NULL;
    }

    /* Success */
    return newJob;
}

/**
 * Job Free
 */
static int
ngislJobFree(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    free(job);

    /* Success */
    return 1;
}
    

/**
 * Job Initialize
 */
static int
ngislJobInitialize(
    ngisiContext_t *context,
    ngisiJob_t *job,
    char *requestID,
    ngisiCreateAttr_t *createAttr,
    int *error)
{
    static const char fName[] = "ngislJobInitialize";
    char newJobIDstr[NGISI_ID_STR_MAX];
    int newJobID, result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(createAttr != NULL);

    newJobID = 0;
    newJobIDstr[0] = '\0';

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Creating new Job for RequestID %s.\n", requestID);

    ngislJobInitializeMember(job);

    job->ngisj_context = context;
    job->ngisj_next = NULL;

    result = ngislJobInitializeSync(context, job, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Job Sync failed.\n");
        return 0;
    }

    job->ngisj_requestID = strdup(requestID);
    if (job->ngisj_requestID == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(Job requestID) failed.\n");
        return 0;
    }

    context->ngisc_maxJobID++;
    newJobID = context->ngisc_maxJobID;

    job->ngisj_internalJobID = newJobID;
    snprintf(newJobIDstr, sizeof(newJobIDstr), "J0x%03x", newJobID);
    job->ngisj_jobID = strdup(newJobIDstr);
    if (job->ngisj_jobID == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(new JobID) failed.\n");
        return 0;
    }

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "New JobID %s for RequestID %s created.\n",
        job->ngisj_jobID, job->ngisj_requestID);

    result = ngislJobAttributeInitialize(
        context, job, &job->ngisj_attr, createAttr, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Job Attribute failed.\n");
        return 0;
    }

    job->ngisj_status = NGISI_JOB_STATUS_INITIALIZED;

    /* Success */
    return 1;
}

/**
 * Job Finalize
 */
static int
ngislJobFinalize(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobFinalize";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    result = ngislJobAttributeFinalize(
        context, job, &job->ngisj_attr, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Job Attribute failed.\n");
        return 0;
    }

    if (job->ngisj_callbackContact != NULL) {
        result = globus_gram_client_job_contact_free(
            job->ngisj_callbackContact);
        if (result != GLOBUS_SUCCESS) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "%s failed by %d: %s.\n",
                "globus_gram_client_job_contact_free()",
                result, globus_gram_client_error_string(result));
            return 0;
        }
    }
    job->ngisj_callbackContact = NULL;

    if (job->ngisj_jobContact != NULL) {
        result = globus_gram_client_job_contact_free(
            job->ngisj_jobContact);
        if (result != GLOBUS_SUCCESS) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "%s failed by %d: %s.\n",
                "globus_gram_client_job_contact_free()",
                result, globus_gram_client_error_string(result));
            return 0;
        }
    }
    job->ngisj_jobContact = NULL;

    result = ngislJobFinalizeSync(context, job, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Job Sync failed.\n");
        return 0;
    }

#define NGISL_JOB_DEALLOCATE(member) \
    { \
        if ((member) != NULL) { \
            globus_libc_free(member); \
        } \
        member = NULL; \
    }

    NGISL_JOB_DEALLOCATE(job->ngisj_requestID)
    NGISL_JOB_DEALLOCATE(job->ngisj_jobID)
    NGISL_JOB_DEALLOCATE(job->ngisj_rmContact)
    NGISL_JOB_DEALLOCATE(job->ngisj_rsl)
#undef NGISL_JOB_DEALLOCATE

    /* Success */
    return 1;
}

/**
 * Initialize Job Synchronization
 */
static int
ngislJobInitializeSync(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobInitializeSync";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    result = ngisiMutexInitialize(
        context, &job->ngisj_mutex, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Job mutex failed.\n");
        return 0;
    }
    job->ngisj_mutexInitialized = 1;

    /* Success */
    return 1;
}

/**
 * Finalize Job Synchronization
 */
static int
ngislJobFinalizeSync(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobFinalizeSync";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    if (job->ngisj_mutexInitialized != 0) {
        job->ngisj_mutexInitialized = 0;
        result = ngisiMutexFinalize(
            context, &job->ngisj_mutex, error);
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Finalize the Job mutex failed.\n");
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Job Initialize Member
 */
static void
ngislJobInitializeMember(
    ngisiJob_t *job)
{
    /* Check the arguments */
    assert(job != NULL);
    
    ngislJobAttributeInitializeMember(&job->ngisj_attr);

    job->ngisj_context = NULL;
    job->ngisj_next = NULL;
    job->ngisj_mutexInitialized = 0;
    job->ngisj_internalJobID = 0;
    job->ngisj_requestID = NULL;
    job->ngisj_jobID = NULL;
    job->ngisj_status = NGISI_JOB_STATUS_UNDEFINED;
    job->ngisj_destructableStatus = 0;
    job->ngisj_destructableProtocol = 0;
    job->ngisj_rmContact = NULL;
    job->ngisj_rsl = NULL;
    job->ngisj_callbackContact = NULL;
    job->ngisj_jobContact = NULL;
}

/**
 * Job Register
 * Note : Lock the Job List before use this function.
 */
static int
ngislJobRegister(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    ngisiJob_t **jobPtr;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    if (context->ngisc_job_head == NULL) {
        context->ngisc_job_head = job;
    } else {
        /* Add to tail */
        jobPtr = &context->ngisc_job_head;
        while ((*jobPtr)->ngisj_next != NULL) {
            jobPtr = &(*jobPtr)->ngisj_next;
        }
        (*jobPtr)->ngisj_next = job;
    }

    context->ngisc_nJobs++;

    /* Success */
    return 1;
}

/**
 * Job Unregister
 * Note : Lock the Job List before use this function.
 */
static int
ngislJobUnregister(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobUnregister";
    ngisiJob_t **jobPtr;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    jobPtr = &context->ngisc_job_head;
    while (*jobPtr != NULL) {
        if (*jobPtr == job) {
            /* Found */
            *jobPtr = (*jobPtr)->ngisj_next;

            context->ngisc_nJobs--;

            /* Success */
            return 1;
        }
        jobPtr = &(*jobPtr)->ngisj_next;
    }

    /* Not found */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
        "Job not found.\n");

    return 0;
}

#define NGISL_JOB_ATTR_SET_STR( \
    jobAttr, createAttr, member, attrName, require) \
    { \
        int macroResult; \
        char *macroValue, *macroStr; \
         \
        assert((jobAttr) != NULL); \
        assert((createAttr) != NULL); \
        assert((attrName) != NULL); \
         \
        macroResult = ngisiCreateAttrGet( \
            context, (createAttr), (attrName), 0, \
            &macroValue, error); \
        if (macroResult == 0) { \
            if ((require) != 0) { \
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                    "Create Attribute \"%s\" (required) not found.\n", \
                    (attrName)); \
                goto error; \
            } \
        } else { \
            if (macroValue != NULL) { \
                macroStr = strdup(macroValue); \
                if (macroStr == NULL) { \
                    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                        "strdup(Create Attr \"%s\" Value) failed.\n", \
                        (attrName)); \
                    goto error; \
                } \
                (jobAttr)->member = macroStr; \
            } \
        } \
    }

#define NGISL_JOB_ATTR_SET_INT( \
    jobAttr, createAttr, member, attrName, require) \
    { \
        int macroResult, macroValueInt; \
        char *macroValue; \
         \
        assert((jobAttr) != NULL); \
        assert((createAttr) != NULL); \
        assert((attrName) != NULL); \
         \
        macroResult = ngisiCreateAttrGet( \
            context, (createAttr), (attrName), 0, \
            &macroValue, error); \
        if (macroResult == 0) { \
            if ((require) != 0) { \
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                    "Create Attribute \"%s\" (required) not found.\n", \
                    (attrName)); \
                goto error; \
            } \
        } else { \
            macroValueInt = (int)strtol(macroValue, NULL, 10); \
             \
            (jobAttr)->member = macroValueInt; \
        } \
    }

#define NGISL_JOB_ATTR_SET_BOOL( \
    jobAttr, createAttr, member, attrName, require) \
    { \
        int macroResult; \
        char *macroValue; \
         \
        assert((jobAttr) != NULL); \
        assert((createAttr) != NULL); \
        assert((attrName) != NULL); \
         \
        macroResult = ngisiCreateAttrGet( \
            context, (createAttr), (attrName), 0, \
            &macroValue, error); \
        if (macroResult == 0) { \
            if ((require) != 0) { \
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                    "Create Attribute \"%s\" (required) not found.\n", \
                    (attrName)); \
                goto error; \
            } \
        } else { \
            macroResult = ngislJobAttributeGetEnum( \
                macroValue, 2, "true", "false"); \
            if ((macroResult < 1) || (macroResult > 2)) { \
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                    "Invalid value %s for Create Attribute \"%s\".\n", \
                    ((macroValue != NULL) ? macroValue : "NULL"), \
                    (attrName)); \
                goto error; \
            } \
              (jobAttr)->member = ((macroResult == 1) ? 1: 0); \
        } \
    }

#define NGISL_JOB_ATTR_SET_STR_ARRAY( \
    jobAttr, createAttr, nMembers, member, attrName, require) \
    { \
        int macroResult; \
         \
        assert((jobAttr) != NULL); \
        assert((createAttr) != NULL); \
        assert((attrName) != NULL); \
         \
        macroResult = ngislJobAttributeGetStrArray( \
            context, job, (createAttr), (attrName), \
                &((jobAttr)->nMembers), &((jobAttr)->member), error); \
        if (macroResult == 0) { \
            if ((require) != 0) { \
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName, \
                    "Create Attribute \"%s\" (required) not found.\n", \
                    (attrName)); \
                goto error; \
            } \
        } \
    }

/**
 * Job Attribute Initialize
 */
static int
ngislJobAttributeInitialize(
    ngisiContext_t *context,
    ngisiJob_t *job,
    ngisiJobAttribute_t *jobAttr,
    ngisiCreateAttr_t *createAttr,
    int *error)
{
    static const char fName[] = "ngislJobAttributeInitialize";
    int result, i, remainCount, useRedirectFile;
    char **remainTable;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(jobAttr != NULL);
    assert(createAttr != NULL);

    ngislJobAttributeInitializeMember(jobAttr);

    ngislJobAttributeSetDefault(jobAttr);

    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_hostName, "hostname", 1)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_portNo, "port", 1)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_jobManager, "jobmanager", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_subject, "subject", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_clientName, "client_name", 1)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_exePath, "executable_path", 1)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_backend, "backend", 1)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_count, "count", 1)
    NGISL_JOB_ATTR_SET_BOOL(
        jobAttr, createAttr, ngisja_staging, "staging", 1)
    NGISL_JOB_ATTR_SET_STR_ARRAY(
        jobAttr, createAttr, ngisja_nArgs, ngisja_args, "argument", 1)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_workDirectory, "work_directory", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_gassURL, "gass_url", 0)
    NGISL_JOB_ATTR_SET_BOOL(
        jobAttr, createAttr, ngisja_redirect, "redirect_enable", 1)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_stdoutFile, "stdout_file", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_stderrFile, "stderr_file", 0)
    NGISL_JOB_ATTR_SET_STR_ARRAY(
        jobAttr, createAttr, ngisja_nEnv, ngisja_env, "environment", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_pollingInterval, "status_polling", 1)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_refreshCred, "refresh_credential", 1)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_maxTime, "max_time", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_maxWallTime, "max_wall_time", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_maxCpuTime, "max_cpu_time", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_queue, "queue_name", 0)
    NGISL_JOB_ATTR_SET_STR(
        jobAttr, createAttr, ngisja_project, "project", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_hostCount, "host_count", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_minMemory, "min_memory", 0)
    NGISL_JOB_ATTR_SET_INT(
        jobAttr, createAttr, ngisja_maxMemory, "max_memory", 0)
    NGISL_JOB_ATTR_SET_STR_ARRAY(
        jobAttr, createAttr,
        ngisja_rslExtensionsSize, ngisja_rslExtensions, "rsl_extensions", 0)

    /* Warning the unsupported JOB_CREATE attributes */
    result = ngisiCreateAttrGetRemain(
        context, createAttr, &remainCount, &remainTable, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Get the Create Attribute remaining item failed.\n");
        goto error;
    }

    if (remainCount > 0) {
        assert(remainTable != NULL);

        for (i = 0; i < remainCount; i++) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_WARNING, fName,
                "Unknown Attr \"%s\".\n", remainTable[i]);
            free(remainTable[i]);
            remainTable[i] = NULL;
        }
        free(remainTable);
        remainTable = NULL;
    }

    /* Output Warning */

    useRedirectFile = 0;
#ifdef NGISI_USE_REDIRECT_FILE
    useRedirectFile = 1;
#endif /* NGISI_USE_REDIRECT_FILE */

    if (jobAttr->ngisja_redirect != 0) {
        if (jobAttr->ngisja_gassURL == NULL) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "No GASS. redirect stdout/stderr requires GASS.\n");
            goto error;
        }

        if (useRedirectFile != 0) {
            if (jobAttr->ngisja_stdoutFile == NULL) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                    "redirect stdout/stderr enabled, but no stdout file.\n");
                goto error;
            }

            if (jobAttr->ngisja_stderrFile == NULL) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                    "redirect stdout/stderr enabled, but no stderr file.\n");
                goto error;
            }
        } else {
            if (jobAttr->ngisja_stdoutFile != NULL) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_WARNING, fName,
                    "saving stdout into file is not supported.\n");
            }
     
            if (jobAttr->ngisja_stderrFile != NULL) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_WARNING, fName,
                    "saving stderr into file is not supported.\n");
            }
        }
    }

    if ((jobAttr->ngisja_staging != 0) &&
        (jobAttr->ngisja_gassURL == NULL)) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "No GASS. redirect stdout/stderr requires GASS.\n");
        goto error;
    }

    if (jobAttr->ngisja_pollingInterval > 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_WARNING, fName,
            "polling is not used to check the job status.\n");
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
        "Job Attribute was not created.\n");

    /* Failed */
    return 0;
}

#undef NGISJ_JOB_ATTR_SET_STR
#undef NGISL_JOB_ATTR_SET_INT
#undef NGISL_JOB_ATTR_SET_BOOL
#undef NGISL_JOB_ATTR_SET_STR_ARRAY

/**
 * Job Attribute Get Enum
 */
static int
ngislJobAttributeGetEnum(
    char *str,
    int num,
    ...)
{
    char *candidate;
    va_list args;
    int i, len;

    /* Check the arguments */
    assert(str != NULL);

    len = strlen(str);

    va_start(args, num);
    for (i = 1; i <= num; i++) {
        candidate = (char *)va_arg(args, char *);
        if (strncmp(str, candidate, len + 1) == 0) {
            va_end(args);
            return i;
        }
    }
    va_end(args);

    return 0;
}

/**
 * Job Attribute Get String Array
 */
static int
ngislJobAttributeGetStrArray(
    ngisiContext_t *context,
    ngisiJob_t *job,
    ngisiCreateAttr_t *createAttr,
    char *attrName,
    int *nMembers,
    char ***member,
    int *error)
{
    static const char fName[] = "ngislJobAttributeGetStrArray";
    char **table, *value, *str;
    int nAttrs, result, i;

    /* Check the arguments */
    assert(context != NULL);
    assert(createAttr != NULL);
    assert(attrName != NULL);
    assert(nMembers != NULL);
    assert(member != NULL);

    *nMembers = 0;
    *member = NULL;

    nAttrs = 0;

    /* Get the count of attributes */
    result = ngisiCreateAttrGetCount(
        context, createAttr, attrName, &nAttrs, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Get Create Attr \"%s\" count failed.\n",
            attrName);
        return 0;
    }

    if (nAttrs <= 0) {
        /* Not available */
        return 0;
    }

    /* Create table */
    table = (char **)globus_libc_calloc(nAttrs, sizeof(char *));
    if (table == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the string array failed.\n");
        return 0;
    }

    for (i = 0; i < nAttrs; i++) {
        result = ngisiCreateAttrGet(
            context, createAttr, attrName, i, &value, error);
        if ((value == NULL) || (result == 0)) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Get the Create Attr \"%s\" [%d] failed.\n",
                attrName, i);
            return 0;
        }

        str = strdup(value);
        if (str == NULL) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "strdup() failed.\n");
            return 0;
        }

        table[i] = str;
    }

    *nMembers = nAttrs;
    *member = table;

    /* Success */
    return 1;
}

/**
 * Job Attribute Finalize
 */
static int
ngislJobAttributeFinalize(
    ngisiContext_t *context,
    ngisiJob_t *job,
    ngisiJobAttribute_t *jobAttr,
    int *error)
{
    int i;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(jobAttr != NULL);

#define NGISL_JOB_DEALLOCATE(member) \
    { \
        if ((member) != NULL) { \
            globus_libc_free(member); \
        } \
        member = NULL; \
    }

    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_hostName)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_jobManager)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_subject)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_clientName)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_exePath)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_backend)

    for (i = 0; i < jobAttr->ngisja_nArgs; i++) {
        NGISL_JOB_DEALLOCATE(jobAttr->ngisja_args[i])
    }
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_args)

    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_workDirectory)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_gassURL)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_queue)
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_project)

    for (i = 0; i < jobAttr->ngisja_nEnv; i++) {
        NGISL_JOB_DEALLOCATE(jobAttr->ngisja_env[i])
    }
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_env)

    for (i = 0; i < jobAttr->ngisja_rslExtensionsSize; i++) {
        NGISL_JOB_DEALLOCATE(jobAttr->ngisja_rslExtensions[i])
    }
    NGISL_JOB_DEALLOCATE(jobAttr->ngisja_rslExtensions)

#undef NGISL_JOB_DEALLOCATE

    ngislJobAttributeInitializeMember(jobAttr);

    /* Success */
    return 1;
}

/**
 * Job Attribute Initialize Member
 */
static void
ngislJobAttributeInitializeMember(
    ngisiJobAttribute_t *jobAttr)
{
    /* Check the arguments */
    assert(jobAttr != NULL);

    jobAttr->ngisja_hostName = NULL;
    jobAttr->ngisja_portNo = 0;
    jobAttr->ngisja_jobManager = NULL;
    jobAttr->ngisja_subject = NULL;
    jobAttr->ngisja_clientName = NULL;
    jobAttr->ngisja_exePath = NULL;
    jobAttr->ngisja_backend = NULL;
    jobAttr->ngisja_count = 0;
    jobAttr->ngisja_staging = 0;
    jobAttr->ngisja_nArgs = 0;
    jobAttr->ngisja_args = NULL;
    jobAttr->ngisja_workDirectory = NULL;
    jobAttr->ngisja_gassURL = NULL;
    jobAttr->ngisja_redirect = 0;
    jobAttr->ngisja_stdoutFile = NULL;
    jobAttr->ngisja_stderrFile = NULL;
    jobAttr->ngisja_nEnv = 0;
    jobAttr->ngisja_env = NULL;
    jobAttr->ngisja_pollingInterval = 0;
    jobAttr->ngisja_refreshCred = 0;
    jobAttr->ngisja_maxTime = 0;
    jobAttr->ngisja_maxWallTime = 0;
    jobAttr->ngisja_maxCpuTime = 0;
    jobAttr->ngisja_queue = NULL;
    jobAttr->ngisja_project = NULL;
    jobAttr->ngisja_hostCount = 0;
    jobAttr->ngisja_minMemory = 0;
    jobAttr->ngisja_maxMemory = 0;
    jobAttr->ngisja_rslExtensionsSize = 0;
    jobAttr->ngisja_rslExtensions = NULL;
}

/**
 * Job Attribute SetDefault
 */
static void
ngislJobAttributeSetDefault(
    ngisiJobAttribute_t *jobAttr)
{
    /* Check the arguments */
    assert(jobAttr != NULL);

    jobAttr->ngisja_hostName = NULL;  /* no default */
    jobAttr->ngisja_portNo = 0;
    jobAttr->ngisja_jobManager = NULL;
    jobAttr->ngisja_subject = NULL;
    jobAttr->ngisja_clientName = NULL;
    jobAttr->ngisja_exePath = NULL;   /* no default */
    jobAttr->ngisja_backend = NULL;
    jobAttr->ngisja_count = 0;        /* no default */
    jobAttr->ngisja_staging = 0;
    jobAttr->ngisja_nArgs = 0;        /* no default */
    jobAttr->ngisja_args = NULL;      /* no default */
    jobAttr->ngisja_workDirectory = NULL;
    jobAttr->ngisja_gassURL = NULL;
    jobAttr->ngisja_redirect = 0;
    jobAttr->ngisja_stdoutFile = NULL;
    jobAttr->ngisja_stderrFile = NULL;
    jobAttr->ngisja_nEnv = 0;
    jobAttr->ngisja_env = NULL;
    jobAttr->ngisja_pollingInterval = 0;
    jobAttr->ngisja_refreshCred = 0;
    jobAttr->ngisja_maxTime = -1;
    jobAttr->ngisja_maxWallTime = -1;
    jobAttr->ngisja_maxCpuTime = -1;
    jobAttr->ngisja_queue = NULL;
    jobAttr->ngisja_project = NULL;
    jobAttr->ngisja_hostCount = -1;
    jobAttr->ngisja_minMemory = -1;
    jobAttr->ngisja_maxMemory = -1;
    jobAttr->ngisja_rslExtensionsSize = 0;
    jobAttr->ngisja_rslExtensions = NULL;
}

/**
 * Lock the list of the Job
 */
static int
ngislJobListLock(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislJobListLock";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    /* Lock */
    result = ngisiContextJobListLock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unlock the list of the Job
 */
static int
ngislJobListUnlock(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislJobListUnlock";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    /* Unlock */
    result = ngisiContextJobListUnlock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Lock the Job
 */
static int
ngislJobLock(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobLock";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    /* Lock */
    result = ngisiMutexLock(context, &job->ngisj_mutex, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unlock the Job
 */
static int
ngislJobUnlock(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobUnlock";
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    /* Unlock */
    result = ngisiMutexUnlock(context, &job->ngisj_mutex, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the JobID
 * Note : jobID is copied.
 * Thus, the caller should release the jobID after use.
 */
int
ngisiJobGetJobID(
    ngisiContext_t *context,
    ngisiJob_t *job,
    char **jobID,
    int *error)
{
    static const char fName[] = "ngisiJobGetJobID";

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(jobID != NULL);

    *jobID = NULL;

    assert(job->ngisj_jobID != NULL);
    *jobID = strdup(job->ngisj_jobID);
    if (*jobID == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Get the Job ID failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Job Start
 */
int
ngisiJobStart(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngisiJobStart";
    int result, callbackAllowed, notifyDone;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    callbackAllowed = 0;
    notifyDone = 0;

    /* Set the interval if necessary */
    result = ngisiRefreshCredentialUpdateIntervalSet(
        context, &context->ngisc_refreshCredential,
        job->ngisj_attr.ngisja_refreshCred, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Set the refresh credential interval failed.\n");
        goto error;
    }

    result = ngislJobMakeRMcontact(
        context, job, &job->ngisj_rmContact, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Make the RMcontact failed.\n");
        goto error;
    }

    result = ngislJobMakeRSL(
        context, job, &job->ngisj_rsl, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Make the RSL failed.\n");
        goto error;
    }

    result = globus_gram_client_callback_allow(
        ngislJobGRAMcallback, job, &job->ngisj_callbackContact);
    if (result != GLOBUS_SUCCESS) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d: %s.\n",
            "globus_gram_client_callback_allow()", result,
            globus_gram_client_error_string(result));
        goto error;
    }
    callbackAllowed = 1;

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "GRAM Callback Contact: \"%s\".\n", job->ngisj_callbackContact);

    result = globus_gram_client_job_request(
        job->ngisj_rmContact,
        job->ngisj_rsl,
        GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL,
        job->ngisj_callbackContact,
        &job->ngisj_jobContact);
    if (result != GLOBUS_SUCCESS) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d: %s.\n",
            "globus_gram_client_job_request()",
            result, globus_gram_client_error_string(result));
        notifyDone = 1;
        goto error;
    } 

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "GRAM Job Contact: \"%s\".\n", job->ngisj_jobContact);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Disallow the callback */
    if (callbackAllowed != 0) {
        callbackAllowed = 0;
        result = globus_gram_client_callback_disallow(
            job->ngisj_callbackContact);
        if (result != GLOBUS_SUCCESS) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "%s failed by %d: %s.\n",
                "globus_gram_client_callback_disallow()",
                result, globus_gram_client_error_string(result));
        }
    }

    if (job->ngisj_callbackContact != NULL) {
        result = globus_gram_client_job_contact_free(
            job->ngisj_callbackContact);
        if (result != GLOBUS_SUCCESS) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "%s failed by %d: %s.\n",
                "globus_gram_client_job_contact_free()",
                result, globus_gram_client_error_string(result));
        }
        job->ngisj_callbackContact = NULL;
    }

    /* Notify Done */
    if (notifyDone != 0) {
        result = ngislJobNotifyStatus(context, job,
            NGISI_JOB_STATUS_FAILED, NULL, NULL);
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Notify Done failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Job make Resource Manager Contact
 */
static int
ngislJobMakeRMcontact(
    ngisiContext_t *context,
    ngisiJob_t *job,
    char **rmContact,
    int *error)
{
    static const char fName[] = "ngislJobMakeRMcontact";
    char rmContactBuf[NGISI_RSL_STR_MAX];
    ngisiJobAttribute_t *jobAttr;
    size_t length;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(rmContact != NULL);

    jobAttr = &job->ngisj_attr;
    
    *rmContact = NULL;

    length = 0;

    length += snprintf(&rmContactBuf[length], NGISI_RSL_STR_MAX - length,
        "%s", jobAttr->ngisja_hostName);

    if (jobAttr->ngisja_portNo != 0) {
        length += snprintf(&rmContactBuf[length], NGISI_RSL_STR_MAX - length,
            ":%d", jobAttr->ngisja_portNo);
    } else if ((jobAttr->ngisja_jobManager == NULL) &&
        (jobAttr->ngisja_subject != NULL)) {
        length += snprintf(&rmContactBuf[length], NGISI_RSL_STR_MAX - length,
            ":");
    }

    if (jobAttr->ngisja_jobManager != NULL) {
        length += snprintf(&rmContactBuf[length], NGISI_RSL_STR_MAX - length,
            "/%s", jobAttr->ngisja_jobManager);
    }

    if (jobAttr->ngisja_subject != NULL) {
        length += snprintf(&rmContactBuf[length], NGISI_RSL_STR_MAX - length,
            ":%s", jobAttr->ngisja_subject);
    }

    *rmContact = strdup(rmContactBuf);
    if (*rmContact == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(rmContact) failed.\n");
        return 0;
    }    

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "RM Contact: \"%s\".\n", *rmContact);

    /* Success */
    return 1;
}
    
/**
 * Job make RSL
 */
static int
ngislJobMakeRSL(
    ngisiContext_t *context,
    ngisiJob_t *job,
    char **rsl,
    int *error)
{
    static const char fName[] = "ngislJobMakeRSL";
    int i, gassURL, useRedirectFile;
    char *stdoutFile, *stderrFile;
    ngisiJobAttribute_t *jobAttr;
    size_t bufferNbytes, length;
    char *rslBuf, *tmp, *env;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(rsl != NULL);

    jobAttr = &job->ngisj_attr;

    *rsl = NULL;
    rslBuf = NULL;
    gassURL = 0;

    useRedirectFile = 0;
#ifdef NGISI_USE_REDIRECT_FILE
    useRedirectFile = 1;
#endif /* NGISI_USE_REDIRECT_FILE */

    bufferNbytes = NGISI_RSL_STR_MAX;

    /* Calculate the length */
    bufferNbytes += ((jobAttr->ngisja_exePath != NULL) ?
        strlen(jobAttr->ngisja_exePath) : 0);

    bufferNbytes += ((jobAttr->ngisja_backend != NULL) ?
        strlen(jobAttr->ngisja_backend) : 0);

    for (i = 0; i < jobAttr->ngisja_nArgs; i++) {
        bufferNbytes += strlen(jobAttr->ngisja_args[i]);
    }

    bufferNbytes += ((jobAttr->ngisja_workDirectory != NULL) ?
        strlen(jobAttr->ngisja_workDirectory) : 0);

    bufferNbytes += ((jobAttr->ngisja_gassURL != NULL) ?
        strlen(jobAttr->ngisja_gassURL) : 0);

    bufferNbytes += ((jobAttr->ngisja_queue != NULL) ?
        strlen(jobAttr->ngisja_queue) : 0);

    bufferNbytes += ((jobAttr->ngisja_project != NULL) ?
        strlen(jobAttr->ngisja_project) : 0);

    for (i = 0; i < jobAttr->ngisja_nEnv; i++) {
        bufferNbytes += strlen(jobAttr->ngisja_env[i]);
    }

    for (i = 0; i < jobAttr->ngisja_rslExtensionsSize; i++) {
        bufferNbytes += strlen(jobAttr->ngisja_rslExtensions[i]);
    }

    /* Allocate */
    rslBuf = (char *)globus_libc_malloc(bufferNbytes);
    if (rslBuf == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Allocate the RSL buffer failed.\n");
        return 0;
    }

    length = 0;

    /* first "&" */
    length += snprintf(&rslBuf[length], bufferNbytes - length,
        "&");
    if (length >= (bufferNbytes - 1)) {
        goto overflow;
    }

    /* GASS URL */
    gassURL = 0;
    if (jobAttr->ngisja_gassURL != NULL) {
        gassURL = 1;
    }

    if (gassURL != 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(rsl_substitution = (NG_GASS_URL %s))",
            jobAttr->ngisja_gassURL);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* jobType */
    if ((jobAttr->ngisja_backend != NULL) &&
        ((strcmp(jobAttr->ngisja_backend, "MPI") == 0) ||
        (strcmp(jobAttr->ngisja_backend, "BLACS") == 0))) {
        
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(jobType=%s)", "mpi");
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* Count */
    length += snprintf(&rslBuf[length], bufferNbytes - length,
        "(count=%d)", jobAttr->ngisja_count);
    if (length >= (bufferNbytes - 1)) {
        goto overflow;
    }

    /* executable */
    if ((jobAttr->ngisja_staging != 0) && (gassURL == 0)) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "staging enabled, GASS URL not prepared.\n");
        goto error;
    }

    length += snprintf(&rslBuf[length], bufferNbytes - length,
        ((jobAttr->ngisja_staging == 0) ?
        "(executable=%s)" : "(executable=$(NG_GASS_URL) # %s)"),
        jobAttr->ngisja_exePath);
    if (length >= (bufferNbytes - 1)) {
        goto overflow;
    }

    /* Arguments */
    length += snprintf(&rslBuf[length], bufferNbytes - length,
        "(arguments=");
    if (length >= (bufferNbytes - 1)) {
        goto overflow;
    }

    for (i = 0; i < jobAttr->ngisja_nArgs; i++) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "\"%s\"%s", jobAttr->ngisja_args[i],
            ((i < (jobAttr->ngisja_nArgs - 1)) ? " " : ""));
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    length += snprintf(&rslBuf[length], bufferNbytes - length,
        ")");
    if (length >= (bufferNbytes - 1)) {
        goto overflow;
    }

    /* Environment variables */
    if (jobAttr->ngisja_nEnv > 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(environment = ");
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }

        for (i = 0; i < jobAttr->ngisja_nEnv; i++) {
            /* Replace ENV=value -> ("ENV" "value") */
            env = strdup(jobAttr->ngisja_env[i]);
            if (env == NULL) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                    "strdup(env %s) failed.\n", jobAttr->ngisja_env[i]);
                goto error;
            }
            tmp = strchr(env, '=');
            if (tmp == NULL) {
                length += snprintf(&rslBuf[length], bufferNbytes - length,
                    "(%s \"\")", env);
                if (length >= (bufferNbytes - 1)) {
                    goto overflow;
                }
            } else if (tmp[1] != '\0') {
                *tmp = '\0';
                length += snprintf(&rslBuf[length], bufferNbytes - length,
                    "(%s \"%s\")", env, &tmp[1]);
                if (length >= (bufferNbytes - 1)) {
                    goto overflow;
                }
            } else {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                    "invalid syntax for env \"%s\".\n",
                    jobAttr->ngisja_env[i]);
                goto error;
            }
            globus_libc_free(env);
        }

        length += snprintf(&rslBuf[length], bufferNbytes - length,
            ")");
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* redirect stdout/stderr */
    if (jobAttr->ngisja_redirect != 0) {
        if (gassURL == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "GASS URL is NULL, but redirect stdout/stderr are set.\n");
            goto error;
        }

        if (useRedirectFile != 0) {
            stdoutFile = jobAttr->ngisja_stdoutFile;
            stderrFile = jobAttr->ngisja_stderrFile;
        } else {
            stdoutFile = "/dev/stdout";
            stderrFile = "/dev/stderr";
        }
        assert(stdoutFile != NULL);
        assert(stderrFile != NULL);

        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(stderr = $(NG_GASS_URL) # %s)"
            "(stdout = $(NG_GASS_URL) # %s)",
            stderrFile, stdoutFile);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* workDirectory */
    if (jobAttr->ngisja_workDirectory != NULL) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(directory=%s)", jobAttr->ngisja_workDirectory);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* maxTime */
    if (jobAttr->ngisja_maxTime >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(maxTime=%d)", jobAttr->ngisja_maxTime);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* maxWallTime */
    if (jobAttr->ngisja_maxWallTime >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(maxWallTime=%d)", jobAttr->ngisja_maxWallTime);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* maxCpuTime */
    if (jobAttr->ngisja_maxCpuTime >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(maxCpuTime=%d)", jobAttr->ngisja_maxCpuTime);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* queue */
    if (jobAttr->ngisja_queue != NULL) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(queue=\"%s\")", jobAttr->ngisja_queue);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* project */
    if (jobAttr->ngisja_project != NULL) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(project=\"%s\")", jobAttr->ngisja_project);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* hostCount */
    if (jobAttr->ngisja_hostCount >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(hostCount=%d)", jobAttr->ngisja_hostCount);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* minMemory */
    if (jobAttr->ngisja_minMemory >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(minMemory=%d)", jobAttr->ngisja_minMemory);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* maxMemory */
    if (jobAttr->ngisja_maxMemory >= 0) {
        length += snprintf(&rslBuf[length], bufferNbytes - length,
            "(maxMemory=%d)", jobAttr->ngisja_maxMemory);
        if (length >= (bufferNbytes - 1)) {
            goto overflow;
        }
    }

    /* RSL extensions */
    if (jobAttr->ngisja_rslExtensionsSize > 0) {
        for (i = 0; i < jobAttr->ngisja_rslExtensionsSize; i++) {
            length += snprintf(&rslBuf[length], bufferNbytes - length,
                "%s", jobAttr->ngisja_rslExtensions[i]);
            if (length >= (bufferNbytes - 1)) {
                goto overflow;
            }
        }
    }

    *rsl = strdup(rslBuf);
    if (*rsl == NULL) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(RSL) failed.\n");
        goto error;
    }

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "RSL: \"%s\".\n", *rsl);

    /* Success */
    return 1;

    /* Overflow */
overflow:

    /* Error occurred */
error:

    if (rslBuf != NULL) {
        globus_libc_free(rslBuf);
        rslBuf = NULL;
    }

    /* Failed */
    return 0;
}
    
/**
 * Job GRAM Callback function
 */
static void
ngislJobGRAMcallback(
    void *userData,
    char *jobContact,
    int state,
    int errorCode)
{
    static const char fName[] = "ngislJobGRAMcallback";
    int result, doNotify, *error, errorEntity;
    ngisiJobStatus_t newStatus;
    ngisiContext_t *context;
    char *statusString;
    ngisiJob_t *job;

    /* Check the arguments */
    assert(userData != NULL);

    job = (ngisiJob_t *)userData;
    context = job->ngisj_context;
    error = &errorEntity;

    newStatus = NGISI_JOB_STATUS_UNDEFINED;
    statusString = "unknown";
    doNotify = 0;

    switch(state) {
    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
        newStatus = NGISI_JOB_STATUS_PENDING;
        statusString = "PENDING";
        doNotify = 1;
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
        newStatus = NGISI_JOB_STATUS_ACTIVE;
        statusString = "ACTIVE";
        doNotify = 1;
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
        newStatus = NGISI_JOB_STATUS_FAILED;
        statusString = "FAILED";
        doNotify = 1;
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
        newStatus = NGISI_JOB_STATUS_DONE;
        statusString = "DONE";
        doNotify = 1;
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_UNSUBMITTED:
        statusString = "UNSUBMITTED";
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_IN:
        statusString = "STAGE_IN";
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_STAGE_OUT:
        statusString = "STAGE_OUT";
        break;

    default:
        break;
    }

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "Got the Job status \"%s\". status%s changed.\n",
        statusString, ((doNotify != 0) ? "" : " not"));

    if (state == GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "GRAM job failed because %s (error code %d).\n",
            globus_gram_client_error_string(errorCode), errorCode);
    }

    if (doNotify != 0) {
        result = ngislJobNotifyStatus(
            context, job, newStatus, NULL, error);
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Notify the Job Status failed.\n");
        }
    }

    /* Success */
    return;
}

/**
 * Job Notify Status
 */
static int
ngislJobNotifyStatus(
    ngisiContext_t *context,
    ngisiJob_t *job,
    ngisiJobStatus_t newStatus,
    char *message,
    int *error)
{
    static const char fName[] = "ngislJobNotifyStatus";
    int result, jobLocked, destructable, jobDestructed;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);
    assert(newStatus > NGISI_JOB_STATUS_UNDEFINED);
    assert(newStatus < NGISI_JOB_STATUS_NOMORE);

    jobLocked = 0;
    destructable = 0;
    jobDestructed = 0;

    result = ngislJobLock(context, job, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job failed.\n");
        goto error;
    }
    jobLocked = 1;

    job->ngisj_status = newStatus;

    result = ngisiNotifyJobStatus(
        context, job->ngisj_jobID, newStatus, message, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Notify Job Status failed.\n");
        goto error;
    }

    if ((newStatus == NGISI_JOB_STATUS_FAILED) ||
        (newStatus == NGISI_JOB_STATUS_DONE)) {
        /* log */
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
            "The Job become destructable by status.\n");

        if (job->ngisj_callbackContact != NULL) {
            result = globus_gram_client_callback_disallow(
                job->ngisj_callbackContact);
            if (result != GLOBUS_SUCCESS) {
                ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                    "%s failed by %d: %s.\n",
                    "globus_gram_client_callback_disallow()",
                    result, globus_gram_client_error_string(result));
                goto error;
            }
        }

        job->ngisj_destructableStatus = 1;
    }

    if ((job->ngisj_destructableStatus != 0) &&
        (job->ngisj_destructableProtocol != 0)) {
        /* log */
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
            "The Job is destructable (both status and protocol).\n");

        destructable = 1;
    }

    result = ngislJobUnlock(context, job, error);
    jobLocked = 0;
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job failed.\n");
        goto error;
    }

    if (destructable != 0) {
        jobDestructed = 1;
        result = ngisiJobDestruct(context, job, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Destruct the Job failed.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (jobLocked != 0) {
        assert(jobDestructed == 0);
        result = ngislJobUnlock(context, job, NULL);
        jobLocked = 0;
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Job Protocol JOB_DESTROY
 */
int
ngisiJobProtocolDestroy(
    ngisiContext_t *context,
    char *jobID,
    int *error)
{
    static const char fName[] = "ngisiJobProtocolDestroy";
    int result, jobLocked, destructable, jobDestructed;
    ngisiJob_t *job;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobID != NULL);

    job = NULL;
    jobLocked = 0;
    destructable = 0;
    jobDestructed = 0;

    result = ngislJobFindByJobID(context, jobID, &job, &jobLocked, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Job ID %s not found.\n", jobID);
        goto error;
    }

    if ((job == NULL) || (jobLocked == 0)) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Job ID %s not found.\n", jobID);
        goto error;
    }

    /* log */
    ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
        "The Job become destructable by protocol.\n");

    job->ngisj_destructableProtocol = 1;

    if ((job->ngisj_destructableStatus != 0) &&
        (job->ngisj_destructableProtocol != 0)) {
        /* log */
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_DEBUG, fName,
            "The Job is destructable (both status and protocol).\n");

        destructable = 1;
    }

    result = ngislJobUnlock(context, job, error);
    jobLocked = 0;
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job failed.\n");
        goto error;
    }

    if (destructable != 0) {
        jobDestructed = 1;
        result = ngisiJobDestruct(context, job, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Destruct the Job failed.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (jobLocked != 0) {
        assert(jobDestructed == 0);
        result = ngislJobUnlock(context, job, NULL);
        jobLocked = 0;
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job failed.\n");
        }
    }

    /* Failed */
    return 0;
}
 
/**
 * Job Protocol JOB_STATUS
 */
int
ngisiJobProtocolStatus(
    ngisiContext_t *context,
    char *jobID,
    ngisiJobStatus_t *curStatus,
    int *error)
{
    static const char fName[] = "ngisiJobProtocolStatus";
    int jobLocked, result;
    ngisiJob_t *job;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobID != NULL);
    assert(curStatus != NULL);

    *curStatus = NGISI_JOB_STATUS_UNDEFINED;

    job = NULL;
    jobLocked = 0;

    result = ngislJobFindByJobID(context, jobID, &job, &jobLocked, error);
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Job ID %s not found.\n", jobID);
        goto error;
    }

    if ((job == NULL) || (jobLocked == 0)) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Job ID %s not found.\n", jobID);
        goto error;
    }

    *curStatus = job->ngisj_status;

    result = ngislJobUnlock(context, job, error);
    jobLocked = 0;
    if (result == 0) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (jobLocked != 0) {
        result = ngislJobUnlock(context, job, NULL);
        jobLocked = 0;
        if (result == 0) {
            ngislLogPrintfJob(job, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job failed.\n");
        }
    }

    /* Failed */
    return 0;
}
 
/**
 * Find the Job by JobID
 */
static int
ngislJobFindByJobID(
    ngisiContext_t *context,
    char *jobID,
    ngisiJob_t **job,
    int *jobLocked,
    int *error)
{
    static const char fName[] = "ngislJobFindByJobID";
    int result, found, listLocked;
    ngisiJob_t *curJob;

    /* Check the arguments */
    assert(context != NULL);
    assert(jobID != NULL);
    assert(job != NULL);
    assert(jobLocked != NULL);

    *job = NULL;
    *jobLocked = 0;

    curJob = NULL;
    listLocked = 0;
    found = 0;

    result = ngislJobListLock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list failed.\n");
        goto error;
    }
    listLocked = 1;

    found = 0;
    for (curJob = context->ngisc_job_head; curJob != NULL;
        curJob = curJob->ngisj_next) {
        assert(curJob->ngisj_jobID != NULL);
        if (strcmp(jobID, curJob->ngisj_jobID) == 0) {
            found = 1;
            break;
        }
    }

    if ((found != 0) && (curJob != NULL)) {
        result = ngislJobLock(context, curJob, error);
        if (result == 0) {
            ngislLogPrintfJob(curJob, NGISI_LOG_LEVEL_ERROR, fName,
                "Lock the Job failed.\n");
            goto error;
        }

        *jobLocked = 1;
        *job = curJob;
    }

    result = ngislJobListUnlock(context, error);
    listLocked = 0;
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list failed.\n");
        goto error;
    }

    if ((found == 0) || (curJob == NULL)) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "The JobID %s was not found.\n", jobID);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (listLocked != 0) {
        result = ngislJobListUnlock(context, error);
        listLocked = 0;
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list failed.\n");
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Connection to Client closed.
 */
int
ngisiJobConnectionClosed(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiJobConnectionClosed";

    /* Check the arguments */
    assert(context != NULL);

    if (context->ngisc_job_head == NULL) {
        /* log */
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "All jobs were destructed. Do nothing.\n");

        /* Success */
        return 1;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_WARNING, fName,
        "Connection close to All jobs unimplemented yet.\n");

    /* Success */
    return 1;
}

/**
 * Refresh Credential.
 */
int
ngisiJobRefreshCredential(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiJobRefreshCredential";
    static gss_cred_id_t gssCredential = GSS_C_NO_CREDENTIAL;
    int listLocked, result;
    ngisiJob_t *cur;

    /* Check the arguments */
    assert(context != NULL);

    listLocked = 0;
    cur = NULL;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "refreshing the credentials.\n");

    /* acquire credential */
    result = ngislJobRefreshCredentialAcquire(
        context, &gssCredential, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Can't acquire proxy credential.\n");
        goto error;
    }

    /* Reset credential */
    result = ngislJobRefreshCredentialSet(
        context, gssCredential, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Can't set proxy credential.\n");
        goto error;
    }

    result = ngislJobListLock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list failed.\n");
        goto error;
    }
    listLocked = 1;

    /* Refresh Credential for each Job */
    for (cur = context->ngisc_job_head; cur != NULL; cur = cur->ngisj_next) {
        result = ngislJobRefreshCredentialOneJob(context, cur, error);
        if (result == 0) {
            ngislLogPrintfJob(cur, NGISI_LOG_LEVEL_ERROR, fName,
                "Failed to update the credential.\n");
            /* Not Return */
        }
    }

    listLocked = 0;
    result = ngislJobListUnlock(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list failed.\n");
        goto error;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "refreshing the credentials done.\n");

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (listLocked != 0) {
        listLocked = 0;
        result = ngislJobListUnlock(context, NULL);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list failed.\n");
        }
    }

    /* Failed */
    return 0;
}

/**
 * Acquire credential.
 */
static int
ngislJobRefreshCredentialAcquire(
    ngisiContext_t *context,
    gss_cred_id_t *gssCred,
    int *error)
{
    static const char fName[] = "ngislJobRefreshCredentialAcquire";
    OM_uint32 major;
    OM_uint32 minor;
    char *message;

    /* Check the arguments */
    assert(context != NULL);

    message = NULL;

    /* acquire credential */
    major = globus_gss_assist_acquire_cred(
        &minor, GSS_C_BOTH, gssCred);
    if (major != GSS_S_COMPLETE) {
        globus_gss_assist_display_status_str(
            &message, "", major, minor, 0);

        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Failed to refresh credentials(%s).\n", message);

        return 0;
    }
    
    /* Success */
    return 1;
}
    
/**
 * Set credential to GRAM.
 */
static int
ngislJobRefreshCredentialSet(
    ngisiContext_t *context,
    gss_cred_id_t gssCred,
    int *error)
{
    int gResult;

    /* Check the arguments */
    assert(context != NULL);

    /* Set proxy credential for GRAM */
    gResult = globus_gram_client_set_credentials(gssCred);

    if (gResult != GLOBUS_SUCCESS) {
        /* will never come here */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Refresh the credential of Job.
 */
static int
ngislJobRefreshCredentialOneJob(
    ngisiContext_t *context,
    ngisiJob_t *job,
    int *error)
{
    static const char fName[] = "ngislJobRefreshCredentialOneJob";
    ngisiJobStatus_t status;
    char *contact;
    int result;

    /* Check the arguments */
    assert(context != NULL);
    assert(job != NULL);

    status = job->ngisj_status;

    if ((status < NGISI_JOB_STATUS_PENDING) ||
        (status > NGISI_JOB_STATUS_ACTIVE)) {
        /* Success */
        return 1;
    }

    contact = job->ngisj_jobContact;
    if (contact == NULL) {
        /* Success */
        return 1;
    }

    /* Refresh credential of the Job. */
    result = globus_gram_client_job_refresh_credentials(
        contact, GSS_C_NO_CREDENTIAL);
    if (result != GLOBUS_SUCCESS) {
        ngislLogPrintfJob(job, NGISI_LOG_LEVEL_WARNING, fName,
            "Failed to refresh credential for %s.\n", contact);
    }

    /* Success */
    return 1;
}
 
/**
 * log
 */
static int
ngislLogPrintfJob(
    ngisiJob_t *job,
    ngisiLogLevel_t level,
    const char *functionName,
    char *format,
    ...)
{
    ngisiContext_t *context;
    char buf[NGISI_ID_STR_MAX];
    va_list ap;
    int result;

    /* Check the arguments */
    assert(job != NULL);
    assert(job->ngisj_context != NULL);

    context = job->ngisj_context;

    snprintf(buf, sizeof(buf), "Job %s", 
        ((job->ngisj_jobID != NULL) ? job->ngisj_jobID : "undef"));

    va_start(ap, format);

    result = ngisiLogVprintfInternal(
        context, level, functionName, buf, format, ap);

    va_end(ap);

    return result;
}

#endif /* NG_PTHREAD */

