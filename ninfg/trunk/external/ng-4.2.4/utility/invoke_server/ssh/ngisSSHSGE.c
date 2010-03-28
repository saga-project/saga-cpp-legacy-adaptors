#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile$ $Revision$ $Date$";
#endif /* NGIS_NO_WARN_RCSID */
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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "ngInvokeServerSSH.h"

static int ngisSSHjobSGEcreateScript(ngisSSHjob_t *);
static int ngisSSHjobSGEwriteScript(ngisSSHjob_t *, FILE *);
static int ngisSSHjobSGEwriteScriptHeader(ngisSSHjob_t *, FILE *);
static int ngisSSHjobSGEsubmit(ngisSSHjob_t *);
static void ngislSSHjobSGEsubmitReadCallback(void *, ngisLineBuffer_t *,
    char *, ngisCallbackResult_t);
static void ngislSSHjobSGEqueryStatusReadCallback(void *,
    ngisLineBuffer_t *, char *, ngisCallbackResult_t);

/**
 * SSH job: SGE: Prepare to submit the Job
 */
int
ngisSSHjobSGEprepare(ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobSGEprepare";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    /* Common? */
    result = ngisSSHjobPrepareCommon(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't prepare(common).\n");
        return 0;
    }

    result = ngisSSHjobFunctionPush(job, ngisSSHjobSGEcreateScript);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for create SGE script.\n");
        return 0;
    }

    result = ngisSSHjobFunctionPush(job, ngisSSHjobStagingScript);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for transfer SGE script.\n");
        return 0;
    }

    if ((job->ngsj_attributes->ngsja_jobBackend == NGIS_BACKEND_NORMAL) && 
        (job->ngsj_attributes->ngsja_count > 1)) {
        /* For Array Job */
        result = ngisSSHjobFunctionPush(job,
            ngisSSHjobStagingScriptForArrayJob);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function "
                "for transfer SGE script for array job.\n");
            return 0;
        }
    }

    result = ngisSSHjobFunctionPush(job, ngisSSHjobSGEsubmit);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for transfer SGE script.\n");
        return 0;
    }

    return 1;
}

static int
ngisSSHjobSGEcreateScript(
    ngisSSHjob_t *job)
{
    FILE *fp = NULL;
    int result;
    int ret = 1;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobSGEcreateScript";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    if ((job->ngsj_attributes->ngsja_jobBackend == NGIS_BACKEND_NORMAL) && 
        (job->ngsj_attributes->ngsja_count > 1)) {
        /* For Array Job */
        result = ngisSSHjobCreateScriptForArrayJob(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't create script for array job.\n");
            ret = 0;
            goto finalize;
        }
    }

    job->ngsj_localScriptName =
        ngisMakeTempFile(job->ngsj_attributes->ngsja_tmpDir);
    if (job->ngsj_localScriptName == NULL) {
        ngisErrorPrint(log, fName, "Can't make temporary file.\n");
        ret = 0;
        goto finalize;
    }

    fp = fopen(job->ngsj_localScriptName, "w");
    if (fp == NULL) {
        ngisErrorPrint(log, fName, "Can't create FILE structure.\n");
        ret = 0;
        goto finalize;
    }

    result = ngisSSHjobSGEwriteScript(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write script to file.\n");
        ret = 0;
        goto finalize;
    }
    
finalize:
    /* Close */
    if (fp != NULL) {
        result = fclose(fp);
        if (result != 0) {
            ngisErrorPrint(log, fName, "fclose: %s.\n", strerror(errno));
            ret = 0;
        }
        fp = NULL;
    }

    ngisLogDumpFile(log, 
        NGIS_LOG_LEVEL_DEBUG, fName, job->ngsj_localScriptName);

    if (ret != 0) {
        result = ngisSSHjobFunctionPop(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't execute the next process.\n");
            ret = 0;
        }
    }

    return ret;
}

static int
ngisSSHjobSGEwriteScript(
    ngisSSHjob_t *job,
    FILE *fp)
{
    ngisSSHjobAttributes_t *attr = NULL;
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisSSHjobSGEwriteScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log = job->ngsj_log;

    result = ngisSSHjobSGEwriteScriptHeader(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write script header.\n");
        return 0;
    }

    switch (attr->ngsja_jobBackend) {
    case NGIS_BACKEND_NORMAL:
        if (attr->ngsja_count > 1) {
            result = ngisSSHjobWriteArrayJobCommandToScript(
                job, fp, "$TMPDIR/machines", "$TMPDIR/rsh");
        } else {
            result = ngisSSHjobWriteSingleJobCommandToScript(job, fp);
        }
        break;
    case NGIS_BACKEND_MPI:
    case NGIS_BACKEND_BLACS:
        result = ngisSSHjobWriteMPICommandToScript(
            job, fp, "$TMPDIR/machines");
        break;
    default:
        result = 0;
        NGIS_ASSERT_NOTREACHED();
    }
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't print command string.\n");
        return 0;
    }

    return 1;
}

static int
ngisSSHjobSGEwriteScriptHeader(
    ngisSSHjob_t *job,
    FILE *fp)
{
    int maxWallTime;
    int maxCpuTime;
    ngisSSHjobAttributes_t *attr;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobSGEwriteScriptHeader";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    /* SHELL */
    result = fprintf(fp, 
        "#! %s\n"
        "#$ -S %s\n",
        attr->ngsja_sshRemoteSh,
        attr->ngsja_sshRemoteSh);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't print shell to scriptfile.\n");
        return 0;
    }

    /* COUNT */
    switch (attr->ngsja_jobBackend) {
    case NGIS_BACKEND_NORMAL:
        if (attr->ngsja_count == 1) {
            break;
        }
        /* BREAKTHROUGH */
    case NGIS_BACKEND_BLACS:
    case NGIS_BACKEND_MPI:
        result = fprintf(fp, "#$ -pe %s %d\n",
                attr->ngsja_sshSGEparallelEnvironment,
                attr->ngsja_count);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print number of executable to scriptfile.\n");
            return 0;
        }
        break;
    }

    result = fprintf(fp, 
        "#$ -e %s\n" 
        "#$ -o %s\n",
        NGIS_DEV_NULL, NGIS_DEV_NULL);

    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't print output to scriptfile.\n");
        return 0;
    }

    if (attr->ngsja_queueName != NULL) {
        result = fprintf(fp, "#$ -q %s\n", attr->ngsja_queueName);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print queue name to scriptfile.\n");
            return 0;
        }
    }

    if (attr->ngsja_project != NULL) {
        result = fprintf(fp, "#$ -P %s\n", attr->ngsja_project);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print project name to scriptfile.\n");
            return 0;
        }
    }

    /* Max Wall Time */
    maxWallTime = 0;
    if (attr->ngsja_maxWallTime > 0) {
        maxWallTime = attr->ngsja_maxWallTime;
    } else if (attr->ngsja_maxTime > 0) {
        maxWallTime = attr->ngsja_maxTime;
    }
    if (maxWallTime > 0) {
        result = fprintf(fp, "#$ -l h_rt=%02d:%02d:00\n",
            maxWallTime/60, maxWallTime%60);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print max wall time to scriptfile.\n");
            return 0;
        }
    }
    /* Max CPU Time */
    maxCpuTime = 0;
    if (attr->ngsja_maxCpuTime > 0) {
        maxCpuTime = attr->ngsja_maxCpuTime;
    } else if (attr->ngsja_maxTime > 0) {
        maxCpuTime = attr->ngsja_maxTime;
    }
    if (maxCpuTime > 0) {
        result = fprintf(fp, "#$ -l h_cpu=%02d:%02d:00\n",
            maxCpuTime/60, maxCpuTime%60);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print cpu wall time to scriptfile.\n");
            return 0;
        }
    }

    if (attr->ngsja_maxMemory > 0) {
        result = fprintf(fp, "#$ -l h_data=%dM\n",
            attr->ngsja_maxMemory);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print max memory to scriptfile.\n");
            return 0;
        }
    }
    result = fprintf(fp, "\n");

    return 1;
}

static int
ngisSSHjobSGEsubmit(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobSGEsubmit";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    result = ngisSSHjobSetStatus(job, NGIS_EXECUTABLE_STATUS_SUBMITTING);
    if (result == 0) {
        ngisDebugPrint(log, fName, "Can't set the job status.\n");
        return 0;
    }

    job->ngsj_nextReadCallback = ngislSSHjobSGEsubmitReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
        "%s %s || %s %s\n",
        job->ngsj_attributes->ngsja_sshSubmitCommand,
        job->ngsj_remoteScriptName,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName, "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;

    return 1;
}

static void
ngislSSHjobSGEsubmitReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    char *jobID = NULL;
    char *p;
    int i;
    ngisTokenAnalyzer_t tokenAnalyzer;
    int tokenAnalyzerInitialized = 0;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobSGEsubmitReadCallback";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't read the reply.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Failed? */
    if (strcmp(line, NGIS_SSH_JOB_COMMAND_FAILED) == 0) {
        ngisErrorPrint(log, fName, "Can't submit the job.\n");
        goto error;
    }

    ngisTokenAnalyzerInitialize(&tokenAnalyzer, line);
    tokenAnalyzerInitialized = 0;

    /* Get Third (JobID) */
    for (i = 0;i < 2;++i) {
        result = ngisTokenAnalyzerNext(&tokenAnalyzer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't get Job ID.\n.");
            goto error;
        }
    }
    
    /* Get JobID*/
    jobID = ngisTokenAnalyzerGetString(&tokenAnalyzer);
    if (jobID == NULL) {
        ngisErrorPrint(log, fName, "Can't get pid.\n");
        goto error;
    }
    /* Erase Task Number */
    /* When array job, JOB_ID.1-TASK_NUMBER:1 */
    for (p = jobID;*p != '\0';++p) {
        if (!isdigit((int)*p)) {
            *p = '\0';
            break;
        }
    }
    if (strlen(jobID) == 0) {
        ngisErrorPrint(log, fName, "Can't get pid.\n");
        goto error;
    }

    ngisDebugPrint(log, fName, "Job ID = %s.\n", jobID);

    job->ngsj_executables[0].nge_identifier = jobID;
    jobID = NULL;
    result = ngisSSHjobSetStatus(job, NGIS_EXECUTABLE_STATUS_PENDING);
    if (result == 0) {
        ngisDebugPrint(log, fName, "Can't set the job status.\n");
        goto error;
    }

    ngisTokenAnalyzerFinalize(&tokenAnalyzer);
    tokenAnalyzerInitialized = 0;

    /* Polling Start */
    result = ngisSSHjobPollingStart(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't start status polling.\n");
        goto error;
    }
    
    return;

error:
    NGIS_NULL_CHECK_AND_FREE(jobID);

    if (tokenAnalyzerInitialized != 0) {
        ngisTokenAnalyzerFinalize(&tokenAnalyzer);
        tokenAnalyzerInitialized = 0;
    }
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobSGEqueryStatus(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    static const char fName[] = "ngisSSHjobSGEqueryStatus";

    NGIS_ASSERT(job != NULL);

    job->ngsj_executables[0].nge_tmpStatus = NGIS_EXECUTABLE_STATUS_EXIT;
    job->ngsj_nextReadCallback = ngislSSHjobSGEqueryStatusReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in,
        ngisSSHjobWriteStringCallback, job,
        "%s | %s '^[ \t]*%s[ \t]';echo dummy\n",
        job->ngsj_attributes->ngsja_sshStatusCommand,
        NGIS_GREP_COMMAND, job->ngsj_executables[0].nge_identifier);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;

    return 1;
}

static void
ngislSSHjobSGEqueryStatusReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    int i;
    ngisTokenAnalyzer_t tokenAnalyzer;
    int tokenAnalyzerInitialized = 0;
    char *status = NULL;
    char *p = NULL;
    ngisLog_t *log;
    ngisExecutable_t *exe;
    ngisExecutableStatus_t exeStatus;
    static const char fName[] = "ngislSSHjobSGEqueryStatusReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lBuf != NULL);

    log = job->ngsj_log;
    exe = &job->ngsj_executables[0];

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't read the reply.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    ngisDebugPrint(log, fName, "Line %s.\n", line);

    /* Read Dummy */
    if (strcmp(line, "dummy") == 0) {
        if (exe->nge_tmpStatus > ngisSSHjobGetStatus(job)) {
            result = ngisSSHjobExecutableSetStatus(
                job, exe, exe->nge_tmpStatus);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't set status.\n");
                goto error;
            }

            if (exe->nge_tmpStatus == NGIS_EXECUTABLE_STATUS_EXIT) {
                exe->nge_exitCode = 0;/* Success */
                result = ngisSSHjobProcessAfterExit(job);
                if (result == 0) {
                    ngisErrorPrint(log, fName, "Can't process after exit.\n");
                    goto error;
                }
            }
        }
        result = ngisSSHjobFinishQuery(job);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't finish to query the job status.\n");
            goto error;
        }

        return;
    }

    ngisTokenAnalyzerInitialize(&tokenAnalyzer, line);
    tokenAnalyzerInitialized = 0;

    /* Get fifth (status) */
    for (i = 0;i < 4;++i) {
        result = ngisTokenAnalyzerNext(&tokenAnalyzer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't get status.\n.");
            goto error;
        }
    }
    
    /* Get Status */
    status = ngisTokenAnalyzerGetString(&tokenAnalyzer);
    if (status == NULL) {
        ngisErrorPrint(log, fName, "Can't get status.\n");
        goto error;
    }

    ngisDebugPrint(log, fName, "Status string is \"%s\".\n", status);

    /* Check Status */
    exeStatus = ngisSSHjobGetStatus(job);
    for (p = status;*p != '\0';++p) {
        switch (*p) {
        case 'q':
        case 'h':
        case 'w':
        case 'R':
            /* Do nothing */
            /* q h(old), w(aiting) R(estarted) */
            goto status_break;
        case 's':
        case 'S':
        case 'T':
            /* s(uspended) S(uspended) T(hreshold) */
            goto status_break;
        case 't':
        case 'r':
            /* t(ransfering), r(unning) */
            exeStatus = NGIS_EXECUTABLE_STATUS_ACTIVE;
            goto status_break;
        case 'E':
        case 'd':
            /* E(rror), d(eletion) */
            exeStatus = NGIS_EXECUTABLE_STATUS_FAILED;
            goto status_break;
        default:
            /* DONOTHING */;
        }
    }
status_break:
    free(status);
    status = NULL;

    exe->nge_tmpStatus = NGIS_MIN(exe->nge_tmpStatus, exeStatus);

    ngisTokenAnalyzerFinalize(&tokenAnalyzer);
    tokenAnalyzerInitialized = 0;

    result = ngisLineBufferReadLine(job->ngsj_lBufStdout, 
        ngislSSHjobSGEqueryStatusReadCallback, job);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register the function for reading the line.\n");
        goto error;
    }

    return;
error:
    NGIS_NULL_CHECK_AND_FREE(status);

    if (tokenAnalyzerInitialized != 0) {
        ngisTokenAnalyzerFinalize(&tokenAnalyzer);
        tokenAnalyzerInitialized = 0;
    }
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobSGEdoCancel(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobSGEdoCancel";

    log = job->ngsj_log;

    if (job->ngsj_executables[0].nge_identifier == NULL) {
        ngisErrorPrint(log, fName, "Can't cancel the job.\n");
        return 0;
    }

    callback = ngisCallbackWriteFormat(
    job->ngsj_stdio.ngsio_in, ngisSSHjobCancelWriteCallback, job,
        "%s %s > %s\n",
        job->ngsj_attributes->ngsja_sshDeleteCommand,
        job->ngsj_executables[0].nge_identifier, NGIS_DEV_NULL);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;
    
    return 1;
}
