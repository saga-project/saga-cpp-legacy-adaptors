/*
 * $RCSfile: ngInvokeServerSSH.c,v $ $Revision: 1.4 $ $Date: 2008/03/03 09:11:21 $
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
#include "ngisUtility.h"
#include "ngInvokeServer.h"
#include "ngInvokeServerSSH.h"

NGI_RCSID_EMBED("$RCSfile: ngInvokeServerSSH.c,v $ $Revision: 1.4 $ $Date: 2008/03/03 09:11:21 $")

/* File local functions */

/* SSH job - Job Interface */
static int ngislSSHjobInitialize(ngisJob_t *, ngisOptionContainer_t *);
static int ngislSSHjobFinalize(ngisJob_t *);
static int ngislSSHjobCancel(ngisJob_t* );

static int ngislSSHjobDisable(ngisSSHjob_t *);

static int ngislSSHjobQueryStatus(ngisSSHjob_t *);
static void ngislSSHjobQueryStatusReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

static void ngislSSHjobExecutableDoneReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static int ngislSSHjobTransferStdout(ngisSSHjob_t *);
static int ngislSSHjobTransferStderr(ngisSSHjob_t *);
static int ngislSSHjobTransferStdouterr(
    ngisSSHjob_t *, ngisSSHjobTransferTarget_t);

static void ngislSSHjobInitializeMember(ngisSSHjob_t *);
static pid_t ngislSSHjobSSHpopen(
    ngisSSHjob_t *job, char *, ngisStandardIO_t *);
static pid_t ngislSSHjobExecuteSSH(ngisSSHjob_t *);
static int ngislSSHjobDoCancel(ngisSSHjob_t *job);
static void ngislSSHjobWarnIgnoredOptions(
    ngisSSHjob_t *, ngisOptionContainer_t *);

/* Executables */
static ngisExecutable_t *ngislExecutableCreate(int, ngisLog_t *);
static void ngislExecutableDestroy(ngisExecutable_t *, int);

static ngisJobTypeInformation_t ngislSSHjobTypeInformation = {
    sizeof(ngisSSHjob_t),
    ngislSSHjobInitialize,
    ngislSSHjobFinalize,
    ngislSSHjobCancel
};

static char *ngislExecutableStatusString[] = {
    "UNSUBMITTED",
    "SUBMITTING",
    "PENDING",
    "ACTIVE",
    "EXIT",
    "FAILED",
    "STAGEOUT",
    "FAILED_STAGEOUT",
    "DONE",
};

int
main(int argc, char *argv[])
{
    int result;
    char *logFileName = NGIS_DEV_NULL;
    int invokeServerInitialized = 0;
    int exitCode = 0;/* Success */
    int opt;
    struct sigaction sa;    
    int debug = 0;
    int i;
    static const int ignoreSignals[] = {
        SIGPIPE, 
        SIGINT,
        SIGTSTP, 
    };
    static const char fName[] = "main";

    /* Options analyze */
    while ((opt = getopt(argc, argv, "l:d")) >= 0) {
        switch (opt) {
        case 'l':
            /* LOG */
            logFileName = optarg;
            break;
        case 'd':
            debug = 1;
            break;
        case '?':
        default:
            /* Ignore arguments */
            ;
        }
    }
    
    result = ngisLogInitializeModule(logFileName, NGIS_LOG_LEVEL_DEBUG);
    if (result == 0) {
        return 1;
    }

    ngisDebugPrint(NULL, fName, "#========================================#\n");
    ngisDebugPrint(NULL, fName, "#      Invoke Server SSH Started         #\n");
    ngisDebugPrint(NULL, fName, "#========================================#\n");
    
    /* Signal */
    sa.sa_flags   = 0;
    sa.sa_handler = SIG_IGN;
    result = sigemptyset(&sa.sa_mask);
    if (result < 0) {
        ngisErrorPrint(NULL, fName, "sigemptyset: %s\n", strerror(errno));
        exitCode = 1;
        goto finalize;
    }

    for (i = 0;i < NGIS_NELEMENTS(ignoreSignals);++i) {
        result = sigaction(ignoreSignals[i], &sa, NULL);
        if (result < 0) {
            ngisErrorPrint(NULL, fName, "sigaction: %s\n", strerror(errno));
            exitCode = 1;
            goto finalize;
        }    
    }
    
    /* Invoke Server */
    result = ngInvokeServerInitialize(&ngislSSHjobTypeInformation);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "Can't Initialize Invoke Server.\n");
        exitCode = 1;
        goto finalize;
    }
    invokeServerInitialized = 1;
    
    result = ngInvokeServerRun();
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "Invoke Server can't run.\n");
        exitCode = 1;
        goto finalize;
    }
    
finalize:
    if (invokeServerInitialized != 0) {
        result = ngInvokeServerFinalize();
        if (result == 0) {
            ngisErrorPrint(NULL, fName, "Can't finalize Invoke Server.\n");
            exitCode = 1;
        }
    }

    ngisDebugPrint(NULL, fName, "#========================================#\n");
    ngisDebugPrint(NULL, fName, "#      Invoke Server SSH Exit            #\n");
    ngisDebugPrint(NULL, fName, "#========================================#\n");
    
    ngisLogFinalizeModule();
    /* Ignore result */    
    
    return 0;
}

/**
 * SSH Job: Initialize
 */
static int
ngislSSHjobInitialize(
    ngisJob_t *job,
    ngisOptionContainer_t *opts)
{
    ngisSSHjob_t *sshJob = (ngisSSHjob_t *)job;
    ngisSSHjobAttributes_t *attr = NULL;
    char *jobID = NULL;
    int result;
    pid_t pid = -1;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngislSSHjobInitialize";
    
    NGIS_ASSERT(sshJob != NULL);
    NGIS_ASSERT(opts   != NULL);

    log = job->ngj_log;
    
    ngislSSHjobInitializeMember(sshJob);

    /* Function Queue */
    NGIS_LIST_INITIALIZE(ngisSSHjobFunc_t, &sshJob->ngsj_funcQueue);
    
    /* Create SSH Job Attribute from Invoke Server Options */
    attr = ngisSSHjobAttributesCreate(opts);
    if (attr == NULL) {
        ngisErrorPrint(log, fName, "Can't create SSH job attributes.\n");
        goto error;
    }
    sshJob->ngsj_attributes = attr;
    ngislSSHjobWarnIgnoredOptions(sshJob, opts);

    /* Set JobManager*/
    if ((attr->ngsja_jobManager == NULL) ||
        (strcmp(attr->ngsja_jobManager, "jobmanager-fork") == 0)) {
        sshJob->ngsj_jobManagerType = NGIS_SSH_JOB_MANAGER_NORMAL;
    } else if (strcmp(attr->ngsja_jobManager, "jobmanager-sge") == 0) {
        sshJob->ngsj_jobManagerType = NGIS_SSH_JOB_MANAGER_SGE;
    } else if (strcmp(attr->ngsja_jobManager, "jobmanager-pbs") == 0) {
        sshJob->ngsj_jobManagerType = NGIS_SSH_JOB_MANAGER_PBS;
    } else {
        ngisErrorPrint(log, fName, "Invalid job manager.\n");
        goto error;
    }
    
    /* Number of the job of queuing system */
    switch (attr->ngsja_jobBackend) {
    case NGIS_BACKEND_NORMAL:
        switch(sshJob->ngsj_jobManagerType) {
        case NGIS_SSH_JOB_MANAGER_PBS:
        case NGIS_SSH_JOB_MANAGER_SGE:
            sshJob->ngsj_nExecutables = 1;
            break;
        case NGIS_SSH_JOB_MANAGER_NORMAL:
            sshJob->ngsj_nExecutables = attr->ngsja_count;
            break;
        }
        break;
    case NGIS_BACKEND_MPI:
    case NGIS_BACKEND_BLACS:
        sshJob->ngsj_nExecutables = 1;
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Create Executable Handle */
    sshJob->ngsj_executables =
        ngislExecutableCreate(sshJob->ngsj_nExecutables, log);
    if (sshJob->ngsj_executables == NULL) {
        ngisErrorPrint(log, fName, "Can't create the executables.\n");
        goto error;
    }

    /* Execute SSH */
    pid = ngislSSHjobExecuteSSH(sshJob);
    if (pid < 0) {
        ngisErrorPrint(log, fName, "Can't execute SSH.\n");
        goto error;
    }

    /* Create Line Buffer */
    sshJob->ngsj_lBufStdout =
        ngisLineBufferCreate(sshJob->ngsj_stdio.ngsio_out, "\n");
    if (sshJob->ngsj_lBufStdout == NULL) {
        ngisErrorPrint(log, fName,
            "Can't create line buffer for the STDOUT.\n");
        goto error;
    }

    sshJob->ngsj_lBufStderr =
        ngisLineBufferCreate(sshJob->ngsj_stdio.ngsio_error, "\n");
    if (sshJob->ngsj_lBufStderr == NULL) {
        ngisErrorPrint(log, fName,
            "Can't create line buffer for the STDERR.\n");
        goto error;
    }

    /* Register function for reading STDERR */
    result = ngisLineBufferReadLine(
        sshJob->ngsj_lBufStderr, ngisSSHjobErrorCallback, sshJob);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register callback for reading STDERR.\n");
        goto error;
    }

    /* Prepare submitting the job */
    switch (sshJob->ngsj_jobManagerType) {
    case NGIS_SSH_JOB_MANAGER_NORMAL:
        result = ngisSSHjobPrepare(sshJob);
        break;
    case NGIS_SSH_JOB_MANAGER_SGE:
        result = ngisSSHjobSGEprepare(sshJob);
        break;
    case NGIS_SSH_JOB_MANAGER_PBS:
        result = ngisSSHjobPBSprepare(sshJob);
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't prepare.\n");
        goto error;
    }

    /* Job ID is process ID of SSH */
    jobID = ngisStrdupPrintf("%ld", (long)pid);
    if (jobID == NULL) {
        ngisErrorPrint(log, fName, "Can't create Job ID.\n");
        goto error;
    }

    result = ngisJobRegisterID(job, jobID);    
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register Job ID to protocol.\n");
        goto error;
    }

    result = ngisSSHjobFunctionPop(sshJob);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't execute the next process.\n");
        goto error;
    }

    NGIS_NULL_CHECK_AND_FREE(jobID);
 
    return 1;

    /* Error occurred */
error:
    NGIS_NULL_CHECK_AND_FREE(jobID);

    result = ngislSSHjobFinalize(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't finalize the ssh job.\n");
    }
    return 0;
}

/**
 * SSH Job: Finalize
 */
static int
ngislSSHjobFinalize(
    ngisJob_t *job)
{
    int result;
    int ret = 1;
    ngisLog_t *log;
    ngisSSHjob_t *sshJob = (ngisSSHjob_t *)job;
    static const char fName[] = "ngislSSHjobFinalize";
    
    NGIS_ASSERT(sshJob != NULL);

    log = job->ngj_log;

    result = ngislSSHjobDisable(sshJob);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't disable the SSH job.");
    }
    
    /* Destroy Executables */
    if (sshJob->ngsj_executables != NULL) {
        ngislExecutableDestroy(
            sshJob->ngsj_executables, sshJob->ngsj_nExecutables);
        sshJob->ngsj_executables = NULL;
    }

    NGIS_NULL_CHECK_AND_DESTROY(
        sshJob->ngsj_attributes, ngisSSHjobAttributesDestroy);
    NGIS_NULL_CHECK_AND_FREE(sshJob->ngsj_remoteExecutablePath);
    NGIS_NULL_CHECK_AND_FREE(sshJob->ngsj_remoteAuthNumberPath);
    NGIS_NULL_CHECK_AND_FREE(sshJob->ngsj_remoteTempdir);
    NGIS_NULL_CHECK_AND_FREE(sshJob->ngsj_remoteTempdirBase);
    NGIS_NULL_CHECK_AND_FREE(sshJob->ngsj_pidRemoteSh);

    if (!NGIS_LIST_IS_INVALID_VALUE(&sshJob->ngsj_funcQueue)) {
        NGIS_LIST_FINALIZE(ngisSSHjobFunc_t, &sshJob->ngsj_funcQueue);
    }

    ngislSSHjobInitializeMember(sshJob);

    return ret;
}

static int
ngislSSHjobDisable(
    ngisSSHjob_t *job)
{
    int result;
    int ret = 1;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobDisable";
    
    NGIS_ASSERT(job != NULL);

    log = ((ngisJob_t *)job)->ngj_log;
    
    /* Stop file transfer */
    if (job->ngsj_fileTransfer != NULL) {
        result = ngisSSHjobFileTransferDestroy(job->ngsj_fileTransfer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
            ret = 0;
        }
    }

    NGIS_NULL_CHECK_AND_DESTROY(job->ngsj_timerCallback, ngisCallbackCancel);
    NGIS_NULL_CHECK_AND_DESTROY(job->ngsj_stdinCallback, ngisCallbackCancel);
    NGIS_NULL_CHECK_AND_FREE(job->ngsj_command);
    NGIS_NULL_CHECK_AND_DESTROY(job->ngsj_lBufStdout, ngisLineBufferDestroy);
    NGIS_NULL_CHECK_AND_DESTROY(job->ngsj_lBufStderr, ngisLineBufferDestroy);

    result = ngisStandardIOclose(&job->ngsj_stdio);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't close the STDIO\n");
        ret = 0;
    }

    /* Remove Script File */
#define NGISL_SSH_JOB_FILE_DESTROY(filename) do {               \
    char *scriptName = (filename);                              \
    if (scriptName != NULL) {                                   \
        ngisDebugPrint(log, fName, "Unlink %s.\n", scriptName); \
        result = unlink(scriptName);                            \
        if (result < 0) {                                       \
            ngisErrorPrint(log, fName,                          \
                "unlink(%s): %s", scriptName, strerror(errno)); \
            ret = 0;                                            \
        }                                                       \
    }                                                           \
    NGIS_NULL_CHECK_AND_FREE(filename);                         \
}while (0)

    NGISL_SSH_JOB_FILE_DESTROY(job->ngsj_localScriptNameForArrayJob);
    NGISL_SSH_JOB_FILE_DESTROY(job->ngsj_localScriptName);

#undef NGISL_SSH_JOB_FILE_DESTROY

    return ret;
}

/**
 * SSH Job: Callback function called when a line is read from standard error
 */
void
ngisSSHjobErrorCallback(
    void *arg,
    ngisLineBuffer_t *lineBuffer,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisSSHjobErrorCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lineBuffer != NULL);
    NGIS_ASSERT(lineBuffer == job->ngsj_lBufStderr);

    log = job->ngsj_log;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisDebugPrint(log, fName, "Called with EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Called because callback was canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName,
            "Called because error occurred on reading from STDERR.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    NGIS_ASSERT(line != NULL);
    ngisErrorPrint(log, fName, "\"%s\".\n", line);

    /* Next Line */
    result = ngisLineBufferReadLine(lineBuffer, ngisSSHjobErrorCallback, job);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register callback for reading STDERR.\n");
        goto error;
    }
    
    return;
error:
    ngisLineBufferDestroy(job->ngsj_lBufStderr);
    job->ngsj_lBufStderr = NULL;

    return;
}

/**
 * SSH Job: Cancel
 */
static int
ngislSSHjobCancel(
    ngisJob_t* job)
{
    ngisSSHjob_t *sshJob = (ngisSSHjob_t *)job;
    int result = 0;
    ngisExecutableStatus_t status;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobCancel";    

    NGIS_ASSERT(sshJob != NULL);

    log = sshJob->ngsj_log;
    
    ngisDebugPrint(log, fName, "called.\n");
    sshJob->ngsj_cancelCalled = 0;

    status = ngisSSHjobGetStatus(sshJob);
    switch (status) {
    case NGIS_EXECUTABLE_STATUS_UNSUBMITTED:
        result = ngisSSHjobDone(sshJob);
        if (result == 0) {
            ngisErrorPrint(NULL, fName,
                "The job isn't able to be done.\n");
        }
        break;
    case NGIS_EXECUTABLE_STATUS_SUBMITTING:
        ngisDebugPrint(log, fName, "Delayed cancel.\n");
        sshJob->ngsj_cancelCalled = 1;
        break;
    case NGIS_EXECUTABLE_STATUS_PENDING:
    case NGIS_EXECUTABLE_STATUS_ACTIVE:
        if (sshJob->ngsj_statusChecking == 0) {
            switch (sshJob->ngsj_jobManagerType) {
            case NGIS_SSH_JOB_MANAGER_NORMAL:
                result = ngislSSHjobDoCancel(sshJob);
                break;
            case NGIS_SSH_JOB_MANAGER_SGE:
                result = ngisSSHjobSGEdoCancel(sshJob);
                break;
            case NGIS_SSH_JOB_MANAGER_PBS:
                result = ngisSSHjobPBSdoCancel(sshJob);
                break;
            default:
                NGIS_ASSERT_NOTREACHED();
            }
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't cancel the job.\n");
                return 0;
            }
        } else {
            ngisDebugPrint(log, fName, "Delayed cancel.\n");
            sshJob->ngsj_cancelCalled = 1;
        }
        break;
    case NGIS_EXECUTABLE_STATUS_DONE:
    case NGIS_EXECUTABLE_STATUS_FAILED:
    case NGIS_EXECUTABLE_STATUS_STAGEOUT:
    case NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT:
    case NGIS_EXECUTABLE_STATUS_EXIT:
        break;
    }

    return 1; 
}

static int
ngislSSHjobDoCancel(
    ngisSSHjob_t* job)
{
    ngisStringBuffer_t sBuf;
    int sBufInitialized = 0;
    char *command = NULL;
    int i;
    int ret = 1;
    int nActive = 0;
    int result;
    ngisCallback_t callback;
    ngisLog_t *log;
    ngisExecutable_t *exe;
    static const char fName[] = "ngislSSHjobDoCancel";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    nActive = ngisSSHjobGetNexecutable(job, NGIS_EXECUTABLE_STATUS_ACTIVE);
    if (nActive == 0) {
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

    result = ngisStringBufferAppend(&sBuf, NGIS_KILL_COMMAND);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        ret = 0;
        goto finalize;
    }
    for (i = 0;i < job->ngsj_nExecutables;++i) {
        exe = &job->ngsj_executables[i];
        if (exe->nge_status == NGIS_EXECUTABLE_STATUS_ACTIVE) {
            result = ngisStringBufferFormat(&sBuf, " %s", exe->nge_identifier);
            if (result == 0) {
                ngisErrorPrint(log, fName,
                    "Can't append string to the string buffer.\n");
                ret = 0;
                goto finalize;
            }
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
            "Can't append string to the string buffer.\n");
        ret = 0;
        goto finalize;
    }

    callback = ngisCallbackWriteFormat(job->ngsj_stdio.ngsio_in,
        ngisSSHjobCancelWriteCallback, job, "%s", command);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't register function for sending command string.\n");
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

/**
 * SSH job: done
 */
int
ngisSSHjobDone(
    ngisSSHjob_t *job)
{
    int i;
    int result;
    ngisLog_t *log;
    ngisExecutable_t *exe = NULL;
    static const char fName[] = "ngisSSHjobDone";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;
    ngisDebugPrint(log, fName, "called.\n");

    for (i = 0;i < job->ngsj_nExecutables;++i) {
        exe = &job->ngsj_executables[i];
        switch (exe->nge_status) {
        case NGIS_EXECUTABLE_STATUS_FAILED:
        case NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT:
            break;
        default:
            result = ngisSSHjobExecutableSetStatus(
                job, exe, NGIS_EXECUTABLE_STATUS_DONE);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't set status.\n");            
            }
            break;
        }
    }

    if (((ngisJob_t *)job)->ngj_destroyRequested != 0) {
        result = ngisJobDestroy((ngisJob_t *)job);
        if (result == 0) {
            ngisErrorPrint(NULL, fName, "Can't destroy the job.\n");            
            /* through */
        }
        job = NULL;
    } else {
        /* Disable */
        result = ngislSSHjobDisable(job);
        if (result == 0) {
            ngisErrorPrint(NULL, fName, "Can't disable the job.\n");            
            /* through */
        }
    }
    return 1;
}

/**
 * SSH job: Start status polling
 */
int
ngisSSHjobPollingStart(
    ngisSSHjob_t *job)
{
    int interval;
    ngisCallback_t callback;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPollingStart";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    interval = job->ngsj_attributes->ngsja_statusPolling;
    if (interval <= 0) {
        interval = NGIS_SSH_JOB_DEFAULT_POLLING_TIME;
    }
    callback = ngisCallbackSetTimer(interval, ngisSSHjobPollingCallback, job);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't register callback for status polling.\n");
        return 0;
    }
    job->ngsj_timerCallback = callback;

    return 1;
}

/**
 * SSH job: Callback for polling
 */
void
ngisSSHjobPollingCallback(
    void *arg,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPollingCallback";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;
    job->ngsj_timerCallback = NULL;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Failed.\n");
        goto error;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Next */
    result = ngisSSHjobPollingStart(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't continue status polling.\n");
        goto error;
    }

    if (job->ngsj_statusChecking != 0) {
        ngisWarningPrint(log, fName, 
            "Previous status checking still has not finished. "
            "Skip checking of this time.\n");
        /* DONOTHING */
        return;
    } else {
        /* Check Start */
        ngisSSHjobFunctionReset(job);
        job->ngsj_statusChecking = 1;
        job->ngsj_iExecutables = 0;
        switch (job->ngsj_jobManagerType) {
        case NGIS_SSH_JOB_MANAGER_NORMAL:
            result = ngislSSHjobQueryStatus(job);
            break;
        case NGIS_SSH_JOB_MANAGER_SGE:
            result = ngisSSHjobSGEqueryStatus(job);
            break;
        case NGIS_SSH_JOB_MANAGER_PBS:
            result = ngisSSHjobPBSqueryStatus(job);
            break;
        default:
            NGIS_ASSERT_NOTREACHED();
        }
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't query status.\n");
            goto error;
        }
    }
    
    return;
error:
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "The job isn't able to be done.\n");
    }
    return;
}

/**
 * SSH job: Finish to query the job status
 */
int
ngisSSHjobFinishQuery(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisSSHjobFinishQuery";

    log = job->ngsj_log;

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_statusChecking != 0);

    job->ngsj_statusChecking = 0;
    if (job->ngsj_cancelCalled != 0) {
        result = ngisJobCancel((ngisJob_t *)job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't cancel the job.\n");
            return 0;
        }
    }
    return 1;
}

/**
 * SSH job: Query status
 */
static int
ngislSSHjobQueryStatus(
    ngisSSHjob_t *job)
{
    int index;
    ngisExecutable_t *exe;
    ngisCallback_t callback;
    ngisLog_t *log = NULL;
    int result;
    static const char fName[] = "ngislSSHjobQueryStatus";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Index is %d/%d\n",
        job->ngsj_iExecutables, job->ngsj_nExecutables);

    while (job->ngsj_iExecutables < job->ngsj_nExecutables) {
        index =  job->ngsj_iExecutables;
        exe   = &job->ngsj_executables[index];
        ngisDebugPrint(log, fName, "Executable[%d] is %s\n",
            index, ngislExecutableStatusString[exe->nge_status]);
        if (exe->nge_status == NGIS_EXECUTABLE_STATUS_ACTIVE) {
            /* Check Status */
            job->ngsj_nextReadCallback = ngislSSHjobQueryStatusReadCallback;
            callback = ngisCallbackWriteFormat(
                job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
                "%s %s 2> %s && %s %s || %s %s\n",
                NGIS_QUERY_COMMAND, exe->nge_identifier, NGIS_DEV_NULL,
                NGIS_ECHO_COMMAND, NGIS_SSH_JOB_NONE,
                NGIS_ECHO_COMMAND, NGIS_SSH_JOB_EXIT);
            if (!ngisCallbackIsValid(callback)) {
                ngisErrorPrint(log, fName,
                    "Can't write command invoking a executable.\n");
            }
            break;
        }
        job->ngsj_iExecutables++;
    }

    if (!(job->ngsj_iExecutables < job->ngsj_nExecutables)) {
        ngisDebugPrint(log, fName, "Status checking has finished.\n");
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
ngislSSHjobQueryStatusReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    ngisCallback_t callback;
    int result;
    char *command;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobQueryStatusReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lBuf != NULL);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisDebugPrint(log, fName, "Can't read the data.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    if (strcmp(line, NGIS_SSH_JOB_NONE) == 0) {
        /* Check next executable */
        job->ngsj_iExecutables++;
        result = ngislSSHjobQueryStatus(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't query the status.\n");
            goto error;
        }
    } else if (strcmp(line, NGIS_SSH_JOB_EXIT) == 0) {
        /* Wait the child process and get exit status */
        job->ngsj_nextReadCallback = ngislSSHjobExecutableDoneReadCallback;
        callback = ngisCallbackWriteFormat(
            job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
            "wait %s;%s $?\n",
            job->ngsj_executables[job->ngsj_iExecutables].nge_identifier,
            NGIS_ECHO_COMMAND);
        if (!ngisCallbackIsValid(callback)) {
            ngisErrorPrint(log, fName, 
                "Can't write command getting exit code.\n");
            goto error;
        }
        command = NULL;
    } else {
        NGIS_ASSERT_NOTREACHED();
    }

    return;

error:
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

/**
 * SSH job: Callback function for "Query exit status"
 */
static void
ngislSSHjobExecutableDoneReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int exitStatus;
    int result;
    int nExit;
    ngisLog_t *log;
    ngisTokenAnalyzer_t tokenAnalyzer;
    int tokenAnalyzerInitialized = 0;
    ngisExecutable_t *exe = NULL;
    static const char fName[] = "ngislSSHjobExecutableDoneReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(lBuf != NULL);
    
    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisDebugPrint(log, fName, "Can't read the reply.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    NGIS_ASSERT(line != NULL);

    ngisTokenAnalyzerInitialize(&tokenAnalyzer, line);
    tokenAnalyzerInitialized = 1;

    /* Get Exit Status */
    result = ngisTokenAnalyzerGetInt(&tokenAnalyzer, &exitStatus);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't get exit status.\n");
        goto error;
    }

    /* Check End */
    result = ngisTokenAnalyzerNext(&tokenAnalyzer);
    if (result != 0) {
        ngisErrorPrint(log, fName, "Unnecessary token.\n.");
        goto error;
    }
    tokenAnalyzerInitialized = 0;
    ngisTokenAnalyzerFinalize(&tokenAnalyzer);

    ngisDebugPrint(log, fName, "Exit status is \"%d\".\n", exitStatus);

    /* Set status */
    exe = &job->ngsj_executables[job->ngsj_iExecutables];
    exe->nge_exitCode = exitStatus;
    result = ngisSSHjobExecutableSetStatus(
        job, exe, NGIS_EXECUTABLE_STATUS_EXIT);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set status.\n");
        goto error;
    }

    nExit = ngisSSHjobGetNexecutable(job, NGIS_EXECUTABLE_STATUS_EXIT);
    if (nExit >= job->ngsj_nExecutables) {
        ngisDebugPrint(log, fName, "All Executable exit.\n");
        result = ngisSSHjobProcessAfterExit(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't process after exit.\n");
            goto error;
        }
    } else {
        ngisDebugPrint(log, fName, "Check status of the next executable.\n");
        job->ngsj_iExecutables++;
        result = ngislSSHjobQueryStatus(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't query status.\n");
            goto error;
        }
    }
    return;

error:
    if (tokenAnalyzerInitialized != 0) {
        tokenAnalyzerInitialized = 0;
        ngisTokenAnalyzerFinalize(&tokenAnalyzer);
    }

    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobProcessAfterExit(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log = NULL;
    int i;
    ngisExecutable_t *exe = NULL;
    ngisExecutableStatus_t jobStatus;
    static const char fName[] = "ngisSSHjobProcessAfterExit";
    
    NGIS_ASSERT(job != NULL);

    log  = job->ngsj_log;
    jobStatus = ngisSSHjobGetStatus(job);

    ngisSSHjobFunctionReset(job);
    job->ngsj_iExecutables = 0;

    /* Stop polling */
    if (ngisCallbackIsValid(job->ngsj_timerCallback)) {
        ngisCallbackCancel(job->ngsj_timerCallback);
        job->ngsj_timerCallback = NULL;
    }

    if (job->ngsj_afterExit != 0) {
        goto error;
    }
    job->ngsj_afterExit = 1;

    if ((job->ngsj_attributes->ngsja_redirectEnable != 0) &&
        (jobStatus > NGIS_EXECUTABLE_STATUS_SUBMITTING)   &&
        (jobStatus < NGIS_EXECUTABLE_STATUS_STAGEOUT)) {

        for (i = 0;i < job->ngsj_nExecutables;++i) {
            exe = &job->ngsj_executables[i];
            if (exe->nge_status == NGIS_EXECUTABLE_STATUS_FAILED) {
                result = ngisSSHjobExecutableSetStatus(
                    job, exe, NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT);
            } else {
                result = ngisSSHjobExecutableSetStatus(
                    job, exe, NGIS_EXECUTABLE_STATUS_STAGEOUT);
            }
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't set status.\n");            
                goto error;
            }
        }

        /* Redirect outerr */
        result = ngisSSHjobFunctionPush(job, ngislSSHjobTransferStdout);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register the function for transfer STDOUT.\n");
            goto error;
        }
        result = ngisSSHjobFunctionPush(job, ngislSSHjobTransferStderr);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register the function for transfer STDERR.\n");
            goto error;
        }
    } 

    result = ngisSSHjobFunctionPush(job, ngisSSHjobDone);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register the function for transfer job done.\n");
        goto error;
    }

    result = ngisSSHjobFunctionPop(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Fails the registered function.\n");
        return 0;
    }
    
    return 1;
error:
    result = ngisSSHjobDone(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "Can't done the job.\n");
    }
    return 0;
}

static int
ngislSSHjobTransferStdouterr(
    ngisSSHjob_t *job,
    ngisSSHjobTransferTarget_t targetType)
{
    ngisSSHjobFileTransfer_t *fileTransfer = NULL;
    int ret = 1;
    int result;
    char *remoteFileName = NULL;
    char *localFileName = NULL;
    char *remoteFileNameFormat = NULL;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobTransferStdouterr";
    
    NGIS_ASSERT(job != NULL);

    log  = job->ngsj_log;

    switch (targetType) {
    case NGIS_SSH_JOB_TRANSFER_STDOUT:
        remoteFileNameFormat = "%s/stdout.*";
        localFileName        = job->ngsj_attributes->ngsja_stdoutFile;
        break;
    case NGIS_SSH_JOB_TRANSFER_STDERR:
        remoteFileNameFormat = "%s/stderr.*";
        localFileName        = job->ngsj_attributes->ngsja_stderrFile;
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    remoteFileName = ngisStrdupPrintf(
        remoteFileNameFormat, job->ngsj_remoteTempdir);
    if (remoteFileName == NULL) {
        ngisErrorPrint(log, fName,
            "Can't copy string of temporary filename.\n");
        ret = 0;
        goto finalize;
    }

    fileTransfer = ngisSSHjobFileTransferCreate(
        job, localFileName, remoteFileName,
        NGIS_SSH_JOB_FILE_TRANSFER_REMOTE_TO_LOCAL);
    if (fileTransfer == NULL) {
        ngisErrorPrint(log, fName, "Can't create file transfer.\n");
        ret = 0;
        goto finalize;
    }
    result = ngisSSHjobFileTransferStart(fileTransfer);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't start file transfer.\n");
        ret = 0;
        goto finalize;
    }
finalize:
    NGIS_NULL_CHECK_AND_FREE(remoteFileName);

    if ((ret == 0) && (fileTransfer != NULL)) {
        result = ngisSSHjobFileTransferDestroy(fileTransfer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
        }
    }
    return ret;
}

static int
ngislSSHjobTransferStdout(ngisSSHjob_t *job)
{
    int result;
    static const char fName[] = "ngislSSHjobTransferStdout";
    
    NGIS_ASSERT(job != NULL);

    result = ngislSSHjobTransferStdouterr(job, NGIS_SSH_JOB_TRANSFER_STDOUT);
    if (result == 0) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't transfer STDOUT.\n");
    }
    return result;
}

static int
ngislSSHjobTransferStderr(ngisSSHjob_t *job)
{
    int result;
    static const char fName[] = "ngislSSHjobTransferStderr";

    NGIS_ASSERT(job != NULL);

    result = ngislSSHjobTransferStdouterr(job, NGIS_SSH_JOB_TRANSFER_STDERR);
    if (result == 0) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't transfer STDERR.\n");
    }
    return result;
}

/**
 * SSH job: Popen
 * If command is NULL, "/bin/sh" used as command.
 */
static pid_t
ngislSSHjobSSHpopen(
    ngisSSHjob_t *job,
    char *command,
    ngisStandardIO_t* sio)
{
    char *sshCommand = NULL;
    pid_t ret = -1;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobSSHpopen";
    
    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(sio != NULL);

    log = job->ngsj_log;

    sshCommand = ngisSSHjobCreateSSHcommand(job, command);
    if (sshCommand == NULL) {
        ngisErrorPrint(log, fName, "Can't create SSH command.\n");
        goto finalize;
    }

    ret = ngisPopen(sio, sshCommand);
    if (ret < 0) {
        ngisErrorPrint(log, fName, "Can't execute SSH command.\n");
        goto finalize;
    }

finalize:
    NGIS_NULL_CHECK_AND_FREE(sshCommand);

    return ret;
}

/**
 * SSH job: Execute SSH
 */
static pid_t
ngislSSHjobExecuteSSH(
    ngisSSHjob_t *job)
{
    pid_t pid = -1;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobExecuteSSH";
    
    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    pid = ngislSSHjobSSHpopen(job, NULL, &job->ngsj_stdio);
    if (pid < 0) {
        ngisErrorPrint(log, fName, "Can't execute SSH.\n");
        return -1;
    }
    
    ngisDebugPrint(log, fName, "Child Process's STDIN  is %d.\n",
        job->ngsj_stdio.ngsio_in);
    ngisDebugPrint(log, fName, "Child Process's STDOUT is %d.\n",
        job->ngsj_stdio.ngsio_out);
    ngisDebugPrint(log, fName, "Child Process's STDERR is %d.\n",
        job->ngsj_stdio.ngsio_error);

    return pid;
}

static void
ngislSSHjobInitializeMember(
    ngisSSHjob_t *job)
{
    NGIS_ASSERT(job != NULL);

    job->ngsj_executables        = NULL;
    job->ngsj_nExecutables       = 0;

    job->ngsj_remoteExecutablePath = NULL;
    job->ngsj_remoteAuthNumberPath = NULL;
    job->ngsj_remoteTempdir        = NULL;
    job->ngsj_remoteTempdirBase    = NULL;
    job->ngsj_stdio.ngsio_in       = -1;
    job->ngsj_stdio.ngsio_out      = -1;
    job->ngsj_stdio.ngsio_error    = -1;
    job->ngsj_stdinCallback        = NULL;
    job->ngsj_lBufStdout           = NULL;
    job->ngsj_lBufStderr           = NULL;
    job->ngsj_attributes           = NULL;
    job->ngsj_timerCallback        = NULL;
    job->ngsj_statusChecking       = 0;
    job->ngsj_pidRemoteSh          = NULL;
    job->ngsj_fileTransfer         = NULL;
    job->ngsj_cancelCalled         = 0;
    job->ngsj_afterExit            = 0;
    job->ngsj_jobManagerType       = NGIS_SSH_JOB_MANAGER_NORMAL;

    /* for Queueing System */
    job->ngsj_localScriptName             = NULL;
    job->ngsj_remoteScriptName            = NULL;
    job->ngsj_localScriptNameForArrayJob  = NULL;
    job->ngsj_remoteScriptNameForArrayJob = NULL;

    NGIS_LIST_SET_INVALID_VALUE(&job->ngsj_funcQueue);

    /* Temporary */
    job->ngsj_iExecutables     = 0;
    job->ngsj_command          = NULL;
    job->ngsj_nextReadCallback = NULL;
    job->ngsj_iterEnv          = NULL;
    
    return;
}

/**
 * Executable: Create
 */
static ngisExecutable_t *
ngislExecutableCreate(
    int count,
    ngisLog_t *log)
{
    ngisExecutable_t *new = NULL;
    int i;
    static const char fName[] = "ngislExecutableCreate";

    NGIS_ASSERT(count > 0);
    NGIS_ASSERT(log != NULL);

    new = NGIS_ALLOC_ARRAY(ngisExecutable_t, count);
    if (new == NULL) {
        ngisErrorPrint(log, fName, "Can't allocate storage for executables.\n");
        return NULL;
    }

    /* Initialize */
    for (i = 0;i < count;++i) {
        new[i].nge_identifier = NULL;
        new[i].nge_status     = NGIS_EXECUTABLE_STATUS_UNSUBMITTED;
        new[i].nge_tmpStatus  = NGIS_EXECUTABLE_STATUS_UNSUBMITTED;
        new[i].nge_stdout     = NULL;
        new[i].nge_stderr     = NULL;
        new[i].nge_exitCode   = -1;
    }
    return new;
}

/**
 * Executable: Destroy
 */
static void
ngislExecutableDestroy(
    ngisExecutable_t *executables,
    int count)
{
    int i;
#if 0
    static const char fName[] = "ngislExecutableDestroy";
#endif

    NGIS_ASSERT(executables != NULL);
    NGIS_ASSERT(count > 0);

    /* Finalize */
    for (i = 0;i < count;++i) {
        executables[i].nge_status     = NGIS_EXECUTABLE_STATUS_UNSUBMITTED;
        executables[i].nge_tmpStatus  = NGIS_EXECUTABLE_STATUS_UNSUBMITTED;
        executables[i].nge_exitCode   = 0;
        NGIS_NULL_CHECK_AND_FREE(executables[i].nge_identifier);
        NGIS_NULL_CHECK_AND_FREE(executables[i].nge_stdout);
        NGIS_NULL_CHECK_AND_FREE(executables[i].nge_stderr);
    }
    NGIS_FREE(executables);

    return;
}

int
ngisSSHjobFunctionPop(
    ngisSSHjob_t *job)
{
    ngisSSHjobFunc_t *func = NULL;
    NGIS_LIST_ITERATOR_OF(ngisSSHjobFunc_t) it;
    ngisLog_t *log;
    int result;
    static const char fName[] = "ngisSSHjobFunctionPop";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    it = NGIS_LIST_BEGIN(ngisSSHjobFunc_t, &job->ngsj_funcQueue);
    if (NGIS_LIST_IS_EMPTY(ngisSSHjobFunc_t, &job->ngsj_funcQueue)) {
        ngisDebugPrint(log, fName, "Do nothing.\n");
        return 1;
    }
    func = NGIS_LIST_GET(ngisSSHjobFunc_t, it);
    NGIS_LIST_ERASE(ngisSSHjobFunc_t, it);

    NGIS_ASSERT(job->ngsj_iExecutables == 0);

    result = func(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Fails the register function.\n");
    }
    return result;
}

void
ngisSSHjobFunctionReset(
    ngisSSHjob_t *job)
{
    NGIS_LIST_ITERATOR_OF(ngisSSHjobFunc_t) it;
#if 0
    static const char fName[] = "ngisSSHjobFunctionReset";
#endif

    NGIS_ASSERT(job != NULL);

    for (;;) {
        it = NGIS_LIST_BEGIN(ngisSSHjobFunc_t, &job->ngsj_funcQueue);
        if (it == NGIS_LIST_END(ngisSSHjobFunc_t, &job->ngsj_funcQueue)) {
            break;
        }
        it = NGIS_LIST_ERASE(ngisSSHjobFunc_t, it);
    }
    return;
}

int
ngisSSHjobFunctionPush(
    ngisSSHjob_t *job,
    ngisSSHjobFunc_t *func)
{
    int result;
    static const char fName[] = "ngisSSHjobFunctionPush";

    result = NGIS_LIST_INSERT_TAIL(
        ngisSSHjobFunc_t, &job->ngsj_funcQueue, func);
    if (result == 0) {
        ngisErrorPrint(job->ngsj_log, fName,
            "Can't insert the function to job function queue.\n");
        return 0;
    }
    return 1;
}

/**
 * SSH Job: Write String Callback Function
 */
void
ngisSSHjobWriteStringCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLineBufferCallbackFunc_t func;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobWriteStringCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fd >= 0);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    /* Clear Callback of STDIN */
    job->ngsj_stdinCallback = NULL;

    /* Get the callback function for reading */
    func = job->ngsj_nextReadCallback;
    if (func == NULL) {
        ngisErrorPrint(log, fName,
            "The next callback function for reading is invalid.\n");
        goto error;
    }
    job->ngsj_nextReadCallback = NULL;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't write the string.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    result = ngisLineBufferReadLine(job->ngsj_lBufStdout, func, job);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register callback for reading a next line.\n");
        goto error;
    }
    return;
error:
    /* Job Done */
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "The job isn't able to be done.\n");
    }
    return;
}

/**
 * SSH Job: Write String Callback Function
 */
void
ngisSSHjobCancelWriteCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobCancelWriteCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fd >= 0);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't write the string.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    return;
error:

    /* Job Done */
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "The job isn't able to be done.\n");
    }
    return;
}

int
ngisSSHjobSetStatus(
    ngisSSHjob_t *job,
    ngisExecutableStatus_t status)
{
    int i;
    int ret = 1;
    int nChanged = 0;
    int result;
    ngisExecutableStatus_t jobStatus;
    ngisLog_t *log;
    ngisExecutable_t *exe = NULL;
    static const char fName[] = "ngisSSHjobSetStatus";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;
    jobStatus = ngisSSHjobGetStatus(job);

    ngisDebugPrint(log, fName, "Job status change: %s => %s.\n",
        ngislExecutableStatusString[jobStatus],
        ngislExecutableStatusString[status]);

    /* Set status each executable */
    for (i = 0;i < job->ngsj_nExecutables;++i) {
        exe = &job->ngsj_executables[i];
        if (exe->nge_status < status) {
            nChanged++;
            result = ngisSSHjobExecutableSetStatus(job, exe, status);
            if (result == 0) {
                ngisErrorPrint(log, fName,
                    "Can't set status of executable[%d].\n", i);
                ret = 0;
            }
        }
    }
    if (nChanged == 0) {
        ngisErrorPrint(log, fName, "Invalid status.\n");
        ret = 0;
    }
    return ret;
}

int
ngisSSHjobExecutableSetStatus(
    ngisSSHjob_t *job,
    ngisExecutable_t *executable,
    ngisExecutableStatus_t status)
{
    int result;
    ngisExecutableStatus_t jobStatus;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngisSSHjobExecutableSetStatus";
    static const ngisJobStatus_t statusMap[] = {
        NGIS_STATUS_PENDING /* NGIS_EXECUTABLE_STATUS_UNSUBMITTED */,
        NGIS_STATUS_PENDING /* NGIS_EXECUTABLE_STATUS_SUBMITTING */,
        NGIS_STATUS_PENDING /* NGIS_EXECUTABLE_STATUS_PENDING */,
        NGIS_STATUS_ACTIVE  /* NGIS_EXECUTABLE_STATUS_ACTIVE */,
        NGIS_STATUS_ACTIVE  /* NGIS_EXECUTABLE_STATUS_EXIT */,
        NGIS_STATUS_FAILED  /* NGIS_EXECUTABLE_STATUS_FAILED */,
        NGIS_STATUS_ACTIVE  /* NGIS_EXECUTABLE_STATUS_STAGEOUT */,
        NGIS_STATUS_FAILED  /* NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT */,
        NGIS_STATUS_DONE    /* NGIS_EXECUTABLE_STATUS_DONE */
    };

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(executable != NULL);
    
    log = job->ngsj_log;

    ngisDebugPrint(log, fName,
        "Executable status change: %s => %s.\n",
        ngislExecutableStatusString[executable->nge_status],
        ngislExecutableStatusString[status]);

    if (executable->nge_status >= status) {
        ngisErrorPrint(log, fName, "Invalid status.\n");
        return 0;
    }
    executable->nge_status = status;

    /* Delayed Cancel */
    jobStatus = ngisSSHjobGetStatus(job);
    switch (jobStatus) {
    case NGIS_EXECUTABLE_STATUS_PENDING:
    case NGIS_EXECUTABLE_STATUS_ACTIVE:
        if (job->ngsj_cancelCalled != 0) {
            result = ngisJobCancel((ngisJob_t *)job);
            if (result == 0) {
                ngisErrorPrint(log, fName, "Can't cancel the job.\n");
                return 0;
            }
        }
        break;
    default:
        break;
    }

    /* Set Status to Handle */
    result = ngisJobSetStatus((ngisJob_t *)job, statusMap[jobStatus], "");
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set status to handle.\n");
        return 0;
    }

    return 1;
}

ngisExecutableStatus_t
ngisSSHjobGetStatus(
    ngisSSHjob_t *job)
{
    int i;
    ngisExecutableStatus_t ret = NGIS_EXECUTABLE_STATUS_DONE;
#if 0
    static const char fName = "ngisSSHjobGetStatus";
#endif
    NGIS_ASSERT(job != NULL);

    for (i = 0;i < job->ngsj_nExecutables;++i) {
        ret = NGIS_MIN(ret, job->ngsj_executables[i].nge_status);
    }
    return ret;
}

int
ngisSSHjobGetNexecutable(
    ngisSSHjob_t *job,
    ngisExecutableStatus_t status)
{
    int count = 0;
    int i;
#if 0
    static const char fName = "ngisSSHjobGetNexecutable";
#endif

    NGIS_ASSERT(job != NULL);

    for (i = 0;i < job->ngsj_nExecutables;++i) {
        if (job->ngsj_executables[i].nge_status == status) {
            count++;
        }
    }

    return count;
}

static void
ngislSSHjobWarnIgnoredOptions(
    ngisSSHjob_t *job,
    ngisOptionContainer_t *opts)
{
    ngisOption_t it;
    ngisOption_t last;    
    static const char fName[] = "ngislSSHjobWarnIgnoredOptions";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(opts != NULL);

    it = ngisOptionContainerBegin(opts);
    last = ngisOptionContainerEnd(opts);
    while (it != last) {
        ngisWarningPrint(NULL, fName,
            "Unknown option \"%s\".\n", ngisOptionName(it));
        it = ngisOptionNext(it);
    }
    return;
}

