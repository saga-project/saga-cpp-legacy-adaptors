/*
 * $RCSfile: ngisSSHfileTransfer.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $
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

NGI_RCSID_EMBED("$RCSfile: ngisSSHfileTransfer.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $")

typedef struct ngislSSHjobFileTransferOpenSTDIOresult_s {
    int              ngosr_stderrPipe;
    ngisStandardIO_t ngosr_stdio;
} ngislSSHjobFileTransferOpenSTDIOresult_t;

static void ngislSSHjobStagingWaitCallback(void *, pid_t, int,
    ngisCallbackResult_t);
static int ngislSSHjobFileTransferChmodExecutable(
    ngisSSHjobFileTransfer_t *fileTransfer);
static void ngislSSHjobFileTransferChmodReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

static int ngislSSHjobFileTransferOpenSTDIO(ngisSSHjobFileTransfer_t *,
    const char *, const char *, ngislSSHjobFileTransferOpenSTDIOresult_t *);
static void ngislSSHjobFileTransferKill(void *, ngisCallbackResult_t);

ngisSSHjobFileTransfer_t *
ngisSSHjobFileTransferCreate(
    ngisSSHjob_t *job,
    const char *localFile,
    const char *remoteFile,
    ngisSSHjobFileTransferDirection_t direction)
{
    ngisSSHjobFileTransfer_t *new = NULL;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobFileTransferCreate";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT_STRING(localFile);
    NGIS_ASSERT_STRING(remoteFile);

    log = job->ngsj_log;

    if (job->ngsj_fileTransfer != NULL) {
        ngisErrorPrint(log, fName, "File Transfer already exists.\n");
        goto error;
    }

    new = NGIS_ALLOC(ngisSSHjobFileTransfer_t);
    if (new == NULL) {
        ngisErrorPrint(log, fName,
            "Can't allocate storage for file transfer.\n");
        goto error;
    }

    new->ngsjft_stderr          = -1;
    new->ngsjft_lBufStderr      = NULL;
    new->ngsjft_callback        = NULL;
    new->ngsjft_pid             = -1;
    new->ngsjft_localFilename   = NULL;
    new->ngsjft_remoteFilename  = NULL;
    new->ngsjft_direction       = direction;
    new->ngsjft_job             = job;

    new->ngsjft_localFilename   = strdup(localFile);
    if (new->ngsjft_localFilename == NULL) {
        ngisErrorPrint(log, fName, "Can't copy local filename.\n");
        goto error;
    }

    new->ngsjft_remoteFilename  = strdup(remoteFile);
    if (new->ngsjft_remoteFilename == NULL) {
        ngisErrorPrint(log, fName, "Can't copy remote filename.\n");
        goto error;
    }

    job->ngsj_fileTransfer = new;

    return new;
error:
    if (new != NULL) {
        NGIS_NULL_CHECK_AND_FREE(new->ngsjft_localFilename);
        NGIS_NULL_CHECK_AND_FREE(new->ngsjft_remoteFilename);
        NGIS_FREE(new);
        new = NULL;
    }
    return NULL;
}

int
ngisSSHjobFileTransferDestroy(
    ngisSSHjobFileTransfer_t *fileTransfer)
{
    int ret = 1;
    int result;
    ngisLog_t *log;
    ngisCallback_t callback;
    pid_t *pid_ptr = NULL;
    static const char fName[] = "ngisSSHjobFileTransferDestroy";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT(fileTransfer->ngsjft_job != NULL);

    log = fileTransfer->ngsjft_job->ngsj_log;
    
    /* STDERR */
    if (fileTransfer->ngsjft_lBufStderr != NULL) {
        ngisLineBufferDestroy(fileTransfer->ngsjft_lBufStderr);
        fileTransfer->ngsjft_lBufStderr = NULL;
    }

    if (fileTransfer->ngsjft_stderr >= 0) {
        result = close(fileTransfer->ngsjft_stderr);
        if (result < 0) {
            ngisErrorPrint(log, fName, "close:%s.\n", strerror(errno));
        }
        fileTransfer->ngsjft_stderr = -1;

        ret = 0;
    }

    if (fileTransfer->ngsjft_pid >= 0) {
        ngisWarningPrint(log, fName, "kill(%ld).\n",
            (long)fileTransfer->ngsjft_pid);
        result = kill(fileTransfer->ngsjft_pid, SIGTERM);
        if (result < 0) {
            ngisErrorPrint(log, fName, "kill(%ld):%s.\n",
                (long)fileTransfer->ngsjft_pid, strerror(errno));
            ret = 0;
        }
        pid_ptr = NGIS_ALLOC(pid_t);
        if (pid_ptr == NULL) {
            /* Certainly kill */
            result = kill(fileTransfer->ngsjft_pid, SIGKILL);
            if (result < 0) {
                ngisErrorPrint(log, fName, "kill(%ld):%s.\n",
                    (long)fileTransfer->ngsjft_pid, strerror(errno));
            }
            ret = 0;
        } else {
            *pid_ptr = fileTransfer->ngsjft_pid;
            callback = ngisCallbackSetTimer(
                fileTransfer->ngsjft_job->ngsj_attributes->ngsja_statusPolling,
                ngislSSHjobFileTransferKill, pid_ptr);
            if (!ngisCallbackIsValid(callback)) {
                ngisErrorPrint(log, fName, "Can't set timer for kill.\n");
                free(pid_ptr);
                ret = 0;
            }
            pid_ptr = NULL;
        }

        fileTransfer->ngsjft_pid = -1;
    }

    if (ngisCallbackIsValid(fileTransfer->ngsjft_callback)) {
        ngisCallbackCancel(fileTransfer->ngsjft_callback);
        fileTransfer->ngsjft_callback = NULL;
    }
    free(fileTransfer->ngsjft_localFilename);
    free(fileTransfer->ngsjft_remoteFilename);

    fileTransfer->ngsjft_job->ngsj_fileTransfer = NULL;
    fileTransfer->ngsjft_localFilename   = NULL;
    fileTransfer->ngsjft_remoteFilename  = NULL;
    fileTransfer->ngsjft_job             = NULL;

    NGIS_FREE(fileTransfer);

    return ret;
}

/* ssh REMOTE_HOST 'cat > REMOTE_EXECUTABLE' < LOCAL_EXECUTABLE */

int
ngisSSHjobFileTransferStart(
    ngisSSHjobFileTransfer_t *fileTransfer)
{
    pid_t pid = -1;
    char *command = NULL;
    char *sshCommand = NULL;
    int ret = 1;
    int result = 0;
    ngisCallback_t callback;
    ngisLineBuffer_t *lBuf = NULL;
    ngislSSHjobFileTransferOpenSTDIOresult_t osResult = {-1, {-1, -1, -1}};
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobFileTransferStart";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT(fileTransfer->ngsjft_job != NULL);

    log = fileTransfer->ngsjft_job->ngsj_log;

    /* Open Child Process's STDIO */
    switch (fileTransfer->ngsjft_direction) {
    case NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE:
        result = ngislSSHjobFileTransferOpenSTDIO(fileTransfer,
            fileTransfer->ngsjft_localFilename, NGIS_DEV_NULL, &osResult);
        break;
    case NGIS_SSH_JOB_FILE_TRANSFER_REMOTE_TO_LOCAL:
        result = ngislSSHjobFileTransferOpenSTDIO(fileTransfer,
            NGIS_DEV_NULL, fileTransfer->ngsjft_localFilename, &osResult);
        break;
    case NGIS_SSH_JOB_FILE_CREATE_FILE_IN_REMOTE:
        result = ngislSSHjobFileTransferOpenSTDIO(fileTransfer,
            NGIS_DEV_NULL, NGIS_DEV_NULL, &osResult);
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't open STDIO for file transfer.\n");
        ret = 0;
        goto finalize;
    }

    /* Create Remote Command */
    switch (fileTransfer->ngsjft_direction) {
    case NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE:
        /* Execute chmod after transport */
        command = ngisStrdupPrintf("umask 0077 && %s > %s",
            NGIS_CAT_COMMAND, fileTransfer->ngsjft_remoteFilename);
        break;
    case NGIS_SSH_JOB_FILE_TRANSFER_REMOTE_TO_LOCAL:
        command = ngisStrdupPrintf("%s %s",
            NGIS_CAT_COMMAND, fileTransfer->ngsjft_remoteFilename);
        break;
    case NGIS_SSH_JOB_FILE_CREATE_FILE_IN_REMOTE:
        command = ngisStrdupPrintf("umask 0077 && %s %s > %s",
            NGIS_ECHO_COMMAND, fileTransfer->ngsjft_localFilename,
            fileTransfer->ngsjft_remoteFilename);
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    if (command == NULL) {
        ngisErrorPrint(log, fName, "Can't create command for file transfer.\n");
        ret = 0;
        goto finalize;
    }

    sshCommand = ngisSSHjobCreateSSHcommand(fileTransfer->ngsjft_job, command);
    if (sshCommand == NULL) {
        ngisErrorPrint(log, fName,
            "Can't create ssh command for file transfer.\n");
        ret = 0;
        goto finalize;
    }

    /* Invoke Process */
    pid = ngisInvokeProcess(&osResult.ngosr_stdio, sshCommand);
    if (pid < 0) {
        ngisErrorPrint(log, fName, "Can't invoke Process.\n");
        ret = 0;
        goto finalize;
    }

    /* STDERR */
    lBuf = ngisLineBufferCreate(osResult.ngosr_stderrPipe, "\n");
    if (lBuf == NULL) {
        ngisErrorPrint(log, fName, "Can't register source of stderr.\n");
        ret = 0;
        goto finalize;
    }

    result = ngisLineBufferReadLine(lBuf,
        ngisSSHjobFileTransferErrorCallback, fileTransfer);
    if (lBuf == NULL) {
        ngisErrorPrint(log, fName,
            "Can't register callback for reading STDERR.\n");
        ret = 0;
        goto finalize;
    }

    /* Register function for wait process */
    fileTransfer->ngsjft_pid        = pid;
    fileTransfer->ngsjft_stderr     = osResult.ngosr_stderrPipe;
    fileTransfer->ngsjft_lBufStderr = lBuf;

    callback = ngisCallbackWait(pid,
        ngislSSHjobStagingWaitCallback, fileTransfer);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't register callback for waiting the process.\n");
        ret = 0;
        goto finalize;
    }
    fileTransfer->ngsjft_callback = callback;

    pid = -1;
    osResult.ngosr_stderrPipe = -1;
    lBuf = NULL;

finalize:

    NGIS_NULL_CHECK_AND_FREE(sshCommand);
    NGIS_NULL_CHECK_AND_FREE(command);
    
    if (pid >= 0) {
        result = kill(pid, SIGTERM);
        if (result < 0) {
            ngisErrorPrint(log, fName, "kill(%ld):%s.\n", (long)pid, strerror(errno));
            ret = 0;
        }
        pid = -1;
    }

    if (lBuf != NULL) {
        ngisLineBufferDestroy(lBuf);
        lBuf = NULL;
    }

    result = ngisStandardIOclose(&osResult.ngosr_stdio);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't close standard I/O of child process.\n");
        ret = 0;
    }

    if (osResult.ngosr_stderrPipe >= 0) {
        result = close(osResult.ngosr_stderrPipe);
        if (result < 0) {
            ngisErrorPrint(log, fName, "close:%s.\n", strerror(errno));
            ret = 0;
        }
        osResult.ngosr_stderrPipe = -1;
    }

    return ret;
}

/**
 * SSH job: call when staging has finished
 */
static void
ngislSSHjobStagingWaitCallback(
    void *arg,
    pid_t pid,
    int status,
    ngisCallbackResult_t cResult)
{
    int result;
    ngisSSHjobFileTransfer_t *fileTransfer = arg;
    ngisSSHjob_t *job;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobStagingWaitCallback";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT(fileTransfer->ngsjft_job != NULL);

    job = fileTransfer->ngsjft_job;
    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is Canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisDebugPrint(log, fName, "Can't wait the process.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Set Invalid value */
    fileTransfer->ngsjft_pid = -1;
    fileTransfer->ngsjft_callback = NULL;

    /* STDERR */
    if (fileTransfer->ngsjft_lBufStderr != NULL) {
        ngisLineBufferDestroy(fileTransfer->ngsjft_lBufStderr);
        fileTransfer->ngsjft_lBufStderr = NULL;
    }

    result = close(fileTransfer->ngsjft_stderr);
    if (result < 0) {
        ngisErrorPrint(log, fName, "close:%s.\n", strerror(errno));
        goto error;
    }
    fileTransfer->ngsjft_stderr = -1;

    /* Check Exit Status */
    if ((WIFEXITED(status) == 0) ||
        (WEXITSTATUS(status) != 0)) {
        ngisErrorPrint(log, fName, "Child process for staging failed.\n");
        goto error;
    }

    /* Next */
    switch (fileTransfer->ngsjft_direction) {
    case NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE:
        result = ngislSSHjobFileTransferChmodExecutable(fileTransfer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't change mode of remote file.\n");
            goto error;
        }
        break;
    case NGIS_SSH_JOB_FILE_TRANSFER_REMOTE_TO_LOCAL:
    case NGIS_SSH_JOB_FILE_CREATE_FILE_IN_REMOTE:
        /* File Transfer End */
        result = ngisSSHjobFileTransferDestroy(fileTransfer);
        fileTransfer = NULL;
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
            goto error;
        }
        result = ngisSSHjobFunctionPop(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't execute the next process.\n");
            goto error;
        }
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    return;

error:
    if (fileTransfer != NULL) {
        result = ngisSSHjobFileTransferDestroy(fileTransfer);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
        }
    }

    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL, fName, "The job isn't able to be done.\n");
    }
    return;
}

static int
ngislSSHjobFileTransferChmodExecutable(
    ngisSSHjobFileTransfer_t *fileTransfer)
{
    int result;
    struct stat buf;
    ngisCallback_t callback;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobFileTransferChmodExecutable";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT(fileTransfer->ngsjft_job != NULL);

    log = fileTransfer->ngsjft_job->ngsj_log;

    /* Get source file's permission */
    result = stat(fileTransfer->ngsjft_localFilename, &buf);
    if (result < 0) {
        ngisErrorPrint(log, fName, "stat: %s.\n", strerror(errno));
        return 0;
    }

    /* Next */
    fileTransfer->ngsjft_job->ngsj_nextReadCallback =
        ngislSSHjobFileTransferChmodReadCallback;
    callback = ngisCallbackWriteFormat(
        fileTransfer->ngsjft_job->ngsj_stdio.ngsio_in,
        ngisSSHjobWriteStringCallback, fileTransfer->ngsjft_job,
        "%s 0%lo %s && %s %s || %s %s\n",
        NGIS_CHMOD_COMMAND,
        (unsigned long)(buf.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO)),
        fileTransfer->ngsjft_remoteFilename,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_SUCCESS,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName, "Can't send command string.\n");
        return 0;
    }
    fileTransfer->ngsjft_job->ngsj_stdinCallback = callback;

    return 1;
}

static void
ngislSSHjobFileTransferChmodReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    ngisSSHjobFileTransfer_t *fileTransfer;
    int result;
    int ret = 1;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobFileTransferChmodReadCallback";

    NGIS_ASSERT(job != NULL);

    fileTransfer = job->ngsj_fileTransfer;
    log          = job->ngsj_log;

    ngisDebugPrint(log, fName, "called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        ret = 0;
        goto finalize;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't read the reply.\n");
        ret = 0;
        goto finalize;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    if (strcmp(line, NGIS_SSH_JOB_COMMAND_FAILED) == 0) {
        ngisErrorPrint(log, fName, "Command failed.\n");
        ret = 0;
        goto finalize;
    }

    if (strcmp(line, NGIS_SSH_JOB_COMMAND_SUCCESS) != 0) {
        ngisErrorPrint(log, fName, "Unexpect the reply \"%s\".\n", line);
        ret = 0;
        goto finalize;
    }

finalize:
    result = ngisSSHjobFileTransferDestroy(fileTransfer);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
        ret = 0;
    }

    if (ret != 0) {
        result = ngisSSHjobFunctionPop(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't execute the next process.\n");
            ret = 0;
        }
    }

    if (ret == 0) {
        /* Error */
        result = ngisSSHjobProcessAfterExit(job);
        if (result == 0) {
            ngisErrorPrint(NULL, fName,
                "The job isn't able to be done.\n");
        }
    }
    return;
}


static int
ngislSSHjobFileTransferOpenSTDIO(
    ngisSSHjobFileTransfer_t *fileTransfer,
    const char *in,
    const char *out,
    ngislSSHjobFileTransferOpenSTDIOresult_t *ioResult)
{
    int fdIn = -1;
    int fdOut = -1;
    int pfd[2] = {-1, -1};
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobFileTransferOpenSTDIO";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT_STRING(in);
    NGIS_ASSERT_STRING(out);
    NGIS_ASSERT(ioResult != NULL);

    log = fileTransfer->ngsjft_job->ngsj_log;

    /* STDIN */
    fdIn = open(in, O_RDONLY);
    if (fdIn < 0) {
        ngisErrorPrint(log, fName, "open(%s):%s.\n", in, strerror(errno));
        goto error;
    }

    /* STDOUT */
    fdOut = open(out, O_WRONLY|O_CREAT, 0666);
    if (fdOut < 0) {
        ngisErrorPrint(log, fName,
            "open(%s):%s.\n", out, strerror(errno));
        goto error;
    }

    /* STDERR */
    result = ngisNonblockingPipe(pfd);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't create pipe.\n");
        goto error;
    }

    /* Set exec-on-close */
    result = ngisFDsetExecOnCloseFlag(NGIS_PIPE_IN(pfd));
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set exec-on-close flag.\n");
        goto error;
    }
    ioResult->ngosr_stdio.ngsio_in    = fdIn;
    ioResult->ngosr_stdio.ngsio_out   = fdOut;
    ioResult->ngosr_stdio.ngsio_error = NGIS_PIPE_OUT(pfd);
    ioResult->ngosr_stderrPipe        = NGIS_PIPE_IN(pfd);

    return 1;
error:
    if (fdIn >= 0) {
        result = close(fdIn);
        if (result < 0) {
            ngisErrorPrint(log, fName, "close: %s.\n", strerror(errno));
        }
        fdIn = -1;
    }

    if (fdOut >= 0) {
        result = close(fdOut);
        if (result < 0) {
            ngisErrorPrint(log, fName, "close: %s.\n", strerror(errno));
        }
        fdOut = -1;
    }

    if (NGIS_PIPE_IN(pfd) >= 0) {
        result = close(NGIS_PIPE_IN(pfd));
        if (result < 0) {
            ngisErrorPrint(log, fName, "close: %s.\n", strerror(errno));
        }
        NGIS_PIPE_IN(pfd) = -1;
    }

    if (NGIS_PIPE_OUT(pfd) >= 0) {
        result = close(NGIS_PIPE_OUT(pfd));
        if (result < 0) {
            ngisErrorPrint(log, fName, "close: %s.\n", strerror(errno));
        }
        NGIS_PIPE_OUT(pfd) = -1;
    }
    return 0;
}

void
ngisSSHjobFileTransferErrorCallback(
    void *arg,
    ngisLineBuffer_t *lineBuffer,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjobFileTransfer_t *fileTransfer = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobFileTransferErrorCallback";

    NGIS_ASSERT(fileTransfer != NULL);
    NGIS_ASSERT(lineBuffer != NULL);
    NGIS_ASSERT(lineBuffer == fileTransfer->ngsjft_lBufStderr);

    log = fileTransfer->ngsjft_job->ngsj_log;

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
    ngisDebugPrint(log, fName, "\"%s\" is read.\n", line);

    /* Next Line */
    result = ngisLineBufferReadLine(lineBuffer,
        ngisSSHjobFileTransferErrorCallback, arg);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register callback for reading STDERR.\n");
        goto error;
    }
    
    return;
error:
    ngisLineBufferDestroy(fileTransfer->ngsjft_lBufStderr);
    fileTransfer->ngsjft_lBufStderr = NULL;

    return;
}

static void
ngislSSHjobFileTransferKill(
    void *arg,
    ngisCallbackResult_t cResult)
{
    pid_t *pid_ptr = arg;
    pid_t pid = *pid_ptr;
    int result;
    static const char fName[] = "ngislSSHjobFileTransferKill";

    free(pid_ptr);
    pid_ptr = NULL;

    result = kill(pid, 0);
    if (result >= 0) {
        ngisWarningPrint(NULL, fName, "kill(%ld).\n",
            (long)pid);
        result = kill(pid, SIGKILL);
        if (result < 0) {
            ngisErrorPrint(NULL, fName, "kill(%ld):%s.\n",
                (long)pid, strerror(errno));
        }
    }
    return;
}
