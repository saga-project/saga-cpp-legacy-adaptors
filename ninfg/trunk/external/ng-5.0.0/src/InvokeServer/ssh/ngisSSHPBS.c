/*
 * $RCSfile: ngisSSHPBS.c,v $ $Revision: 1.3 $ $Date: 2008/02/27 09:56:32 $
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

#include "ngInvokeServerSSH.h"

NGI_RCSID_EMBED("$RCSfile: ngisSSHPBS.c,v $ $Revision: 1.3 $ $Date: 2008/02/27 09:56:32 $")

#define NGISL_PBS_NODEFILE "$PBS_NODEFILE"

static int ngislSSHjobPBScreateScript(ngisSSHjob_t *);
static int ngislSSHjobPBSwriteScript(ngisSSHjob_t *, FILE *);
static int ngislSSHjobPBSwriteScriptHeader(ngisSSHjob_t *, FILE *);
static int ngislSSHjobPBSwriteResource(ngisSSHjob_t *, FILE *);
static int ngislSSHjobPBSsubmitStart(ngisSSHjob_t *);
static int ngislSSHjobPBSsubmit(ngisSSHjob_t *);
static int ngislSSHjobPBSqueryStatus(ngisSSHjob_t *job);
static void ngislSSHjobPBSsubmitReadCallback(void *,
    ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static void ngislSSHjobPBSqueryStatusReadCallback(void *,
    ngisLineBuffer_t *, char *, ngisCallbackResult_t);

int
ngisSSHjobPBSprepare(ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPBSprepare";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    /* Common? */
    result = ngisSSHjobPrepareCommon(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't prepare.\n");
        return 0;
    }

    result = ngisSSHjobFunctionPush(job, ngislSSHjobPBScreateScript);
    if (result == 0) {
        ngisErrorPrint(log, fName, 
            "Can't register function for create PBS script.\n");
        return 0;
    }

    result = ngisSSHjobFunctionPush(job, ngisSSHjobStagingScript);
    if (result == 0) {
        ngisErrorPrint(log, fName, 
            "Can't register function for transfer PBS script.\n");
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

    result = ngisSSHjobFunctionPush(job, ngislSSHjobPBSsubmitStart);
    if (result == 0) {
        ngisErrorPrint(log, fName, 
            "Can't register function for transfer PBS script.\n");
        return 0;
    }

    return 1;
}

static int
ngislSSHjobPBScreateScript(
    ngisSSHjob_t *job)
{
    char *filename = NULL;
    FILE *fp = NULL;
    int result;
    int ret = 1;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobPBScreateScript";

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

    filename = ngisMakeTempFile(job->ngsj_attributes->ngsja_tmpDir);
    if (filename == NULL) {
        ngisErrorPrint(log, fName, "Can't make temporary file.\n");
        ret = 0;
        goto finalize;
    }
    job->ngsj_localScriptName = filename;

    fp = fopen(filename, "w");
    if (fp == NULL) {
        ngisErrorPrint(log, fName, "Can't create FILE structure.\n");
        ret = 0;
        goto finalize;
    }

    result = ngislSSHjobPBSwriteScript(job, fp);
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

    ngisLogDumpFile(log, NGIS_LOG_LEVEL_DEBUG,
        fName, job->ngsj_localScriptName);

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
ngislSSHjobPBSwriteScript(
    ngisSSHjob_t *job,
    FILE *fp)
{
    ngisSSHjobAttributes_t *attr = NULL;
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngislSSHjobPBSwriteScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log = job->ngsj_log;

    result = ngislSSHjobPBSwriteScriptHeader(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write script header.\n");
        return 0;
    }

    switch (attr->ngsja_jobBackend) {
    case NGIS_BACKEND_NORMAL:
        if (attr->ngsja_count > 1) {
            result = ngisSSHjobWriteArrayJobCommandToScript(
                job, fp, NGISL_PBS_NODEFILE, attr->ngsja_sshPBSrsh);
        } else {
            result = ngisSSHjobWriteSingleJobCommandToScript(job, fp);
        }
        break;
    case NGIS_BACKEND_MPI:
    case NGIS_BACKEND_BLACS:
        result = ngisSSHjobWriteMPICommandToScript(
            job, fp, NGISL_PBS_NODEFILE);
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
ngislSSHjobPBSwriteScriptHeader(
    ngisSSHjob_t *job,
    FILE *fp)
{
    int maxWallTime;
    int maxCpuTime;
    ngisSSHjobAttributes_t *attr;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobPBSwriteScriptHeader";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    /* SHELL */
    result = fprintf(fp, 
        "#! %s\n"
        "#PBS -S %s\n",
        attr->ngsja_sshRemoteSh,
        attr->ngsja_sshRemoteSh);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't print shell to scriptfile.\n");
        return 0;
    }

    result = ngislSSHjobPBSwriteResource(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't print required resources to scriptfile.\n");
        return 0;
    }

    result = fprintf(fp, 
        "#PBS -e %s\n" 
        "#PBS -o %s\n",
        NGIS_DEV_NULL, NGIS_DEV_NULL);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't print output to scriptfile.\n");
        return 0;
    }

    if (attr->ngsja_queueName != NULL) {
        result = fprintf(fp, "#PBS -q %s\n", attr->ngsja_queueName);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print queue name to scriptfile.\n");
            return 0;
        }
    }

    if (attr->ngsja_project != NULL) {
        result = fprintf(fp, "#PBS -A %s\n", attr->ngsja_project);
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
        result = fprintf(fp, "#PBS -l walltime=%d:00\n",
            maxWallTime);
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
        result = fprintf(fp, "#PBS -l pcput=%d:00\n",
            maxCpuTime);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print cpu wall time to scriptfile.\n");
            return 0;
        }
    }

    if (attr->ngsja_maxMemory > 0) {
        result = fprintf(fp, "#PBS -l mem=%dmb\n",
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
ngislSSHjobPBSsubmitStart(
    ngisSSHjob_t *job)
{
    int result;
    static const char fName[] = "ngislSSHjobPBSsubmitStart";

    NGIS_ASSERT(job != NULL);

    job->ngsj_iExecutables = 0;
    result = ngisSSHjobSetStatus(job, NGIS_EXECUTABLE_STATUS_SUBMITTING);
    if (result == 0) {
        ngisErrorPrint(job->ngsj_log, fName,
            "Can't start to submit the job.\n");
        return 0;
    }

    return ngislSSHjobPBSsubmit(job);
}

static int
ngislSSHjobPBSsubmit(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobPBSsubmit";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    job->ngsj_nextReadCallback = ngislSSHjobPBSsubmitReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
        "%s %s || %s %s\n",
        job->ngsj_attributes->ngsja_sshSubmitCommand,
        job->ngsj_remoteScriptName,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't register for sending submit command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;


    return 1;
}

static int
ngislSSHjobPBSwriteResource(
    ngisSSHjob_t *job,
    FILE *fp)
{
    ngisSSHjobAttributes_t *attr;
    int ppn;
    int count;
    int fraction;
    int nNodes;
    int result = -1;
    static const char fName[] = "ngislSSHjobPBSwriteResource";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;

    ppn      = attr->ngsja_sshPBSprocessorsPerNode;
    count    = attr->ngsja_count;
    fraction = count%ppn;
    nNodes   = count/ppn;

    if ((nNodes > 0) && (fraction > 0)) {
        result = fprintf(fp, "#PBS -l nodes=%d:ppn=%d+nodes=1:ppn=%d\n",
            nNodes, ppn, fraction);
    } else if (nNodes > 0) {
        result = fprintf(fp, "#PBS -l nodes=%d:ppn=%d\n",
            nNodes, ppn);
    } else if (fraction > 0) {
        result = fprintf(fp, "#PBS -l nodes=1:ppn=%d\n",
            fraction);
    } else {
        NGIS_ASSERT_NOTREACHED();
    }
    if (result < 0) {
        ngisErrorPrint(job->ngsj_log, fName,
            "Can't print number of executable to scriptfile.\n");
        return 0;
    }
    return 1;
}

static void
ngislSSHjobPBSsubmitReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    char *jobID = NULL;
    int nActive;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobPBSsubmitReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lBuf != NULL);

    log = job->ngsj_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
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
        goto next;
    }

    ngisDebugPrint(log, fName, "Job ID = %s.\n", line);

    jobID = strdup(line);
    if (jobID == NULL) {
        ngisErrorPrint(log, fName, "Can't copy the job ID.\n");
        goto next;
    }

next:
    if (jobID != NULL) {
        job->ngsj_executables[job->ngsj_iExecutables].nge_identifier = jobID;
        jobID = NULL;
        result = ngisSSHjobExecutableSetStatus(
            job, &job->ngsj_executables[job->ngsj_iExecutables],
            NGIS_EXECUTABLE_STATUS_PENDING);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't set status.\n");
            goto next;
        }
    } else {
        result = ngisSSHjobExecutableSetStatus(
            job, &job->ngsj_executables[job->ngsj_iExecutables],
            NGIS_EXECUTABLE_STATUS_FAILED);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't set status.\n");
        }
    }

    job->ngsj_iExecutables++;
    if (job->ngsj_iExecutables < job->ngsj_nExecutables) {
        result = ngislSSHjobPBSsubmit(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't submit next job.\n");
            goto error;
        }
    } else {
        nActive = ngisSSHjobGetNexecutable(job, NGIS_EXECUTABLE_STATUS_PENDING);
        if (nActive == 0) {
            ngisErrorPrint(log, fName, "Can't execute all executables.\n");
            goto error;
        }

        /* Polling Start */
        result = ngisSSHjobPollingStart(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't start status polling.\n");
            goto error;
        }
    }    

    return;
error:
    NGIS_NULL_CHECK_AND_FREE(jobID);

    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobPBSqueryStatus(
    ngisSSHjob_t *job)
{
#if 0
    static const char fName[] = "ngisSSHjobPBSqueryStatus";
#endif

    job->ngsj_iExecutables = 0;

    return ngislSSHjobPBSqueryStatus(job);
}

static int
ngislSSHjobPBSqueryStatus(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    int index;
    ngisExecutable_t *exe;
    ngisLog_t *log;
    int result;
    static const char fName[] = "ngislSSHjobPBSqueryStatus";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    while (job->ngsj_iExecutables < job->ngsj_nExecutables) {
        index = job->ngsj_iExecutables;
        exe   = &job->ngsj_executables[index];
        ngisDebugPrint(log, fName,
            "Executable[%d] is %d\n", index, exe->nge_status);

        if (exe->nge_status <= NGIS_EXECUTABLE_STATUS_ACTIVE) {
            /* Check Status */
            job->ngsj_nextReadCallback = ngislSSHjobPBSqueryStatusReadCallback;
            callback = ngisCallbackWriteFormat(
                job->ngsj_stdio.ngsio_in,
                ngisSSHjobWriteStringCallback, job,
                "%s -f %s | %s 'job_state';%s dummy\n",
                job->ngsj_attributes->ngsja_sshStatusCommand,
                exe->nge_identifier, NGIS_GREP_COMMAND, NGIS_ECHO_COMMAND);
            if (!ngisCallbackIsValid(callback)) {
                ngisErrorPrint(log, fName,
                    "Can't register function for sending"
                    " query status command string.\n");
                return 0;
            }
            break;
        }
        job->ngsj_iExecutables++;
    }

    if (!(job->ngsj_iExecutables < job->ngsj_nExecutables)) {
        ngisDebugPrint(log, fName, "Status checking has finished.\n");
        job->ngsj_iExecutables = 0;
        result = ngisSSHjobFinishQuery(job);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't finish to query the job status.\n");
            return 0;
        }
    }
    return 1;
}

static void
ngislSSHjobPBSqueryStatusReadCallback(
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
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobPBSqueryStatusReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lBuf != NULL);

    log = job->ngsj_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName,
            "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName,
            "Can't read the reply.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Failed? */
    if (strcmp(line, "dummy") == 0) {
        /* Done */
        job->ngsj_executables[0].nge_exitCode = 0;/* Success */
        result = ngisSSHjobExecutableSetStatus(
            job, &job->ngsj_executables[0],
            NGIS_EXECUTABLE_STATUS_EXIT);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't set status.\n");            
            goto error;
        }

        result = ngisSSHjobProcessAfterExit(job);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't process after exit.\n");
            goto error;
        }
        return;
    }

    ngisTokenAnalyzerInitialize(&tokenAnalyzer, line);
    tokenAnalyzerInitialized = 0;

    /* job_state = [STATUS] */
    for (i = 0;i < 2;++i) {
        result = ngisTokenAnalyzerNext(&tokenAnalyzer);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't get status.\n.");
            goto error;
        }
    }
    
    /* Get Status */
    status = ngisTokenAnalyzerGetString(&tokenAnalyzer);
    if (status == NULL) {
        ngisErrorPrint(log, fName,
            "Can't get status.\n");
        goto error;
    }

    ngisDebugPrint(log, fName, "Status string is \"%s\".\n", status);

    /* Check Status */
    switch (*status) {
    case 'Q':
    case 'W':
    case 'T':
        break;
    case 'B':
    case 'E':
    case 'R':
    case 'X':
        /* t(ransfering), r(unning) */
        job->ngsj_executables[0].nge_status
            = NGIS_EXECUTABLE_STATUS_ACTIVE;
        break;
    case 'H':
        /* Do nothing */
        break;
    case 'S':
        break;
    case 'U':
    default:
        /* DONOTHING */;
    }

    free(status);
    status = NULL;

    ngisTokenAnalyzerFinalize(&tokenAnalyzer);
    tokenAnalyzerInitialized = 0;

    result = ngisLineBufferReadLine(job->ngsj_lBufStdout, 
        ngisSSHjobQueryStatusReadDummyCallback, job);
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
        ngisErrorPrint(NULL, fName, "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobPBSdoCancel(
    ngisSSHjob_t *job)
{
    ngisStringBuffer_t sBuf;
    int sBufInitialized = 0;
    char *command = NULL;
    int i;
    int ret = 1;
    int nExe = 0;
    int result;
    ngisCallback_t callback;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPBSdoCancel";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    nExe =  ngisSSHjobGetNexecutable(job, NGIS_EXECUTABLE_STATUS_PENDING);
    nExe += ngisSSHjobGetNexecutable(job, NGIS_EXECUTABLE_STATUS_ACTIVE);
    if (nExe == 0) {
        /* There is no executable. */
        goto finalize;
    }

    result = ngisStringBufferInitialize(&sBuf);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't initialize the string buffer.\n");
        ret = 0;
        goto finalize;
    }
    sBufInitialized = 1;

    result = ngisStringBufferAppend(&sBuf,
        job->ngsj_attributes->ngsja_sshDeleteCommand);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        ret = 0;
        goto finalize;
    }
    for (i = 0;i < job->ngsj_nExecutables;++i) {
        switch (job->ngsj_executables[i].nge_status) {
        case NGIS_EXECUTABLE_STATUS_PENDING:
        case NGIS_EXECUTABLE_STATUS_ACTIVE:
            result = ngisStringBufferFormat(&sBuf,
                " %s", job->ngsj_executables[i].nge_identifier);
            if (result == 0) {
                ngisErrorPrint(log, fName,
                    "Can't append string to the string buffer.\n");
                ret = 0;
                goto finalize;
            }
            break;
        default:
            break;
        }
    }
    result = ngisStringBufferAppend(&sBuf, "\n");
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        ret = 0;
        goto finalize;
    }

    command = ngisStringBufferRelease(&sBuf);
    if (command == NULL) {
        ngisErrorPrint(log, fName,
            "Can't release string from string buffer.\n");
        ret = 0;
        goto finalize;
    }

    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in, ngisSSHjobCancelWriteCallback, job,
        "%s", command);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName, "Can't send command string.\n");
        ret = 0;
        goto finalize;
    }
    job->ngsj_stdinCallback = callback;
    
finalize:
    NGIS_NULL_CHECK_AND_FREE(command);

    if (sBufInitialized != 0) {
        ngisStringBufferFinalize(&sBuf);
        sBufInitialized = 0;
    }
    return ret;
}
