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

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>

#include "ngInvokeServerSSH.h"

static int ngislSSHjobDemandHome(ngisSSHjob_t *);
static void ngislSSHjobDemandHomeReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static int ngislSSHjobDemandPid(ngisSSHjob_t *);
static void ngislSSHjobDemandPidReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
static int ngislSSHjobMakeTempdir(ngisSSHjob_t *);
static int ngislSSHjobSetTempfileName(ngisSSHjob_t *);
static int  ngislSSHjobStagingExecutable(ngisSSHjob_t *);
static int ngislSSHjobUlimitMaxMemory(ngisSSHjob_t *);
static int ngislSSHjobChangeDirectory(ngisSSHjob_t *);
static int ngislSSHjobSetEnvironment(ngisSSHjob_t *);
static void ngislSSHjobSetEnvironmentWriteCallback(
    void *, int, ngisCallbackResult_t);
static int ngislSSHjobInvokeExecutableStart(ngisSSHjob_t *);
static int ngislSSHjobInvokeExecutable(ngisSSHjob_t *);
static void ngislSSHjobInvokeExecutableReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

static void ngislSSHjobCommandReadCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

int
ngisSSHjobPrepare(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPrepare";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    result = ngisSSHjobPrepareCommon(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't prepare(common).\n");
        return 0;
    }

    /* CHDIR */
    if (job->ngsj_attributes->ngsja_workDirectory != NULL) {
        result = ngisSSHjobFunctionPush(job, ngislSSHjobChangeDirectory);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for change"
                " working directory in remote host.\n");
            return 0;
        }
    }
    /* ulimit (max_memory) */
    if (job->ngsj_attributes->ngsja_maxMemory > 0) {
        result = ngisSSHjobFunctionPush(job, ngislSSHjobUlimitMaxMemory);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for set"
                " ma memory of remote job.\n");
            return 0;
        }
    }

    job->ngsj_iterEnv =
        NGIS_LIST_BEGIN(char, &job->ngsj_attributes->ngsja_environments);
    result = ngisSSHjobFunctionPush(job, ngislSSHjobSetEnvironment);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for setting"
            " environment variable in remote host.\n");
        return 0;
    }

    /* Invoke */
    result = ngisSSHjobFunctionPush(job, ngislSSHjobInvokeExecutableStart);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for Invoke Executable.\n");
    }

    return 1;
}

int
ngisSSHjobPrepareCommon(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobPrepareCommon";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    result = ngisSSHjobFunctionPush(job, ngislSSHjobDemandHome);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for demanding remote"
            " temporary directory.\n");
        return 0;
    }

    /* PID */
    result = ngisSSHjobFunctionPush(job, ngislSSHjobDemandPid);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for demanding remote  shell's pid.\n");
        return 0;
    }

    /* Use temporary files in remote host? */
    if ((job->ngsj_attributes->ngsja_staging != 0)                 ||
        (job->ngsj_attributes->ngsja_redirectEnable != 0)          ||
        (job->ngsj_jobManagerType != NGIS_SSH_JOB_MANAGER_NORMAL)) {

        /* Make tempdir */
        result = ngisSSHjobFunctionPush(job, ngislSSHjobMakeTempdir);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for demanding remote  shell's pid.\n");
            return 0;
        }

    }

    /* Staging */
    if (job->ngsj_attributes->ngsja_staging != 0) {
        /* Copy */
        result = ngisSSHjobFunctionPush(job, ngislSSHjobStagingExecutable);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't register function for staging  the executable.\n.");
            return 0;
        }
    }
    return 1;
}

static int
ngislSSHjobDemandHome(
    ngisSSHjob_t *job)
{
    int result;
    ngisCallback_t callback;
    ngisLog_t *log;
    char *tmpdir = NULL;
    static const char fName[] = "ngislSSHjobDemandHome";

    NGIS_ASSERT(job != NULL);    
    NGIS_ASSERT(job->ngsj_attributes != NULL);
    NGIS_ASSERT(job->ngsj_remoteTempdir == NULL);
    NGIS_ASSERT(job->ngsj_remoteTempdirBase == NULL);

    log = job->ngsj_log;
    tmpdir = job->ngsj_attributes->ngsja_sshRemoteTempdir;

    ngisDebugPrint(log, fName, "Called.\n");

    /* Check */
    if (tmpdir[0] == '/') {
        /* Full Path */
        job->ngsj_remoteTempdirBase = strdup(tmpdir);
        if (job->ngsj_remoteTempdirBase == NULL) {
            ngisErrorPrint(log, fName,
                "Can't copy string of remote temporary directory.\n");
            return 0;
        } 
        result = ngisSSHjobFunctionPop(job);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't call registered function.\n");
            return 0;
        }
    } else {
        /* Path related $HOME */
        job->ngsj_nextReadCallback = ngislSSHjobDemandHomeReadCallback;
        callback = ngisCallbackWriteFormat(
            job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
            "%s $HOME\n", NGIS_ECHO_COMMAND);
        if (!ngisCallbackIsValid(callback)) {
            ngisErrorPrint(log, fName, "Can't send command string.\n");
            return 0;
        }
        job->ngsj_stdinCallback = callback;
    }

    return 1;
}

static void
ngislSSHjobDemandHomeReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    ngisSSHjobAttributes_t *attr = NULL;
    int result;
    ngisLog_t *log;
    char *tmpdir = NULL;
    static const char fName[] = "ngislSSHjobDemandHomeReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't read the line.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    ngisDebugPrint(log, fName, "Read (%s).\n", line);

    if ((strlen(line) == 0) && (line[0] != '/')){
        ngisErrorPrint(log, fName, "$HOME is invalid string.\n.");
        goto error;
    }

    tmpdir = job->ngsj_attributes->ngsja_sshRemoteTempdir;
    NGIS_ASSERT(tmpdir[0] != '/');
    if (strncmp(tmpdir, "~/", strlen("~/")) == 0) {
        tmpdir += strlen("~/");
    }

    /* Get temporary directory */
    job->ngsj_remoteTempdirBase = ngisStrdupPrintf("%s/%s", line, tmpdir);
    if (attr->ngsja_sshRemoteTempdir == NULL) {
        ngisErrorPrint(log, fName,
            "Can't allocate storage for name of temporary "
            "directory.\n.");
        goto error;
    }

    /* Next */
    result = ngisSSHjobFunctionPop(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't execute the next process.\n");
        goto error;
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
 * SSH Job: Demand process id to remote host.
 */
static int
ngislSSHjobDemandPid(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    static const char fName[] = "ngislSSHjobDemandPid";

    NGIS_ASSERT(job != NULL);

    job->ngsj_nextReadCallback = ngislSSHjobDemandPidReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in,
        ngisSSHjobWriteStringCallback, job,
        "%s $$\n", NGIS_ECHO_COMMAND);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;

    return 1;
}

/**
 * SSH Job: Process the reply of demand of process id
 */
void
ngislSSHjobDemandPidReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    ngisSSHjobAttributes_t *attr = NULL;
    int result;
    char *p;
    ngisLog_t *log = NULL;
    static const char fName[] = "ngislSSHjobDemandPidReadCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);
    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't read the line.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName, "Unexcept EOF.\n");
        goto error;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Process ID is already set? */
    if (job->ngsj_pidRemoteSh != NULL) {
        ngisErrorPrint(log, fName, "pid of remote sh  is already set\n.");
        goto error;
    }

    /* Get Remote SH's PID */

    /* Check Number */
    if (strlen(line) == 0) {
            ngisErrorPrint(log, fName,
                "Invalid value \"%s\" as process id\n.", line);
            goto error;
    }

    for (p = line;*p != '\0';++p) {
        if (!isdigit((int)*p)) {
            ngisErrorPrint(log, fName,
                "Invalid value \"%s\" as process id\n.", line);
            goto error;
        }
    }
    job->ngsj_pidRemoteSh = strdup(line);
    if (job->ngsj_pidRemoteSh == NULL) {
        ngisErrorPrint(log, fName, "Can't copy string.\n");
        goto error;
    }

    /* Next */
    result = ngisSSHjobFunctionPop(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't execute the next process.\n");
        goto error;
    }

    return;
error:
    /* Job Done */
    result = ngisSSHjobProcessAfterExit(job);
    if (result == 0) {
        ngisErrorPrint(NULL,
            fName, "The job isn't able to be done.\n");
    }
    return;
}

static int
ngislSSHjobMakeTempdir(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    char *tempdir = NULL;
    char *trapCommand = NULL;
    char *quotedCommand = NULL;
    ngisLog_t *log = NULL;
    int ret = 0;
    int result;
    static const char fName[] = "ngislSSHjobMakeTempdir";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    NGIS_ASSERT_STRING(job->ngsj_pidRemoteSh);

    /* Create temporary directory name */
    tempdir = ngisStrdupPrintf("%s/ngissshtmp.%s.%s",
        job->ngsj_remoteTempdirBase,
        job->ngsj_attributes->ngsja_hostname,
        job->ngsj_pidRemoteSh);
    if (tempdir == NULL) {
        ngisErrorPrint(log, fName,
            "Can't create name of temporary directory in  remote host.\n");
        goto finalize;
    }
    job->ngsj_remoteTempdir = tempdir;
    tempdir = NULL;

    /* Set Tempfile*/
    result = ngislSSHjobSetTempfileName(job);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't register function for demanding remote  shell's pid.\n");
        goto finalize;
    }

    /* Create Trap Command */
    trapCommand = ngisStrdupPrintf("%s -rf %s", NGIS_RM_COMMAND,
        job->ngsj_remoteTempdir);
    if (trapCommand == NULL) {
        ngisErrorPrint(log, fName, "Can't create command string.\n");
        goto finalize;
    }
    quotedCommand = ngisShellQuote(trapCommand);
    if (quotedCommand == NULL) {
        ngisErrorPrint(log, fName, "Can't quote command string.\n");
        goto finalize;
    }

    /* Send command */
    job->ngsj_nextReadCallback = ngislSSHjobCommandReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in,
        ngisSSHjobWriteStringCallback, job,
        "%s %s && trap %s 0 && %s %s || %s %s\n",
        NGIS_MKDIR_COMMAND, job->ngsj_remoteTempdir, quotedCommand,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_SUCCESS,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName, "Can't send command string.\n");
        goto finalize;
    }
    job->ngsj_stdinCallback = callback;

    ret = 1;
finalize:
    NGIS_NULL_CHECK_AND_FREE(tempdir);
    NGIS_NULL_CHECK_AND_FREE(trapCommand);
    NGIS_NULL_CHECK_AND_FREE(quotedCommand);

    return ret;
}

static int
ngislSSHjobSetTempfileName(
    ngisSSHjob_t *job)
{
    int i;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobSetTempfileName";

    NGIS_ASSERT(job != NULL);
    log = job->ngsj_log;

    if (job->ngsj_attributes->ngsja_redirectEnable != 0) {
        for (i = 0;i < job->ngsj_nExecutables;++i) {
            job->ngsj_executables[i].nge_stdout =
                ngisStrdupPrintf("%s/stdout.%d", job->ngsj_remoteTempdir, i);
            if (job->ngsj_executables[i].nge_stdout == NULL) {
                ngisErrorPrint(log, fName,
                    "Can't create string of tempfile for stdout.\n");
                return 0;
            }
            job->ngsj_executables[i].nge_stderr =
                ngisStrdupPrintf("%s/stderr.%d", job->ngsj_remoteTempdir, i);
            if (job->ngsj_executables[i].nge_stderr == NULL) {
                ngisErrorPrint(log, fName,
                    "Can't create string of tempfile for stderr.\n");
                return 0;
            }
        }
    }

    if (job->ngsj_attributes->ngsja_staging != 0) {
        job->ngsj_remoteExecutablePath = 
            ngisStrdupPrintf("%s/stub", job->ngsj_remoteTempdir);
        if (job->ngsj_remoteExecutablePath == NULL) {
            ngisErrorPrint(log, fName,
                "Can't create string of tempfile for staging.\n");
            return 0;
        }
    }

    return 1;
}

static int
ngislSSHjobStagingExecutable(
    ngisSSHjob_t *job)
{
    ngisSSHjobFileTransfer_t *fileTransfer = NULL;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobStagingExecutable";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    fileTransfer = ngisSSHjobFileTransferCreate(
        job, job->ngsj_attributes->ngsja_executablePath,
        job->ngsj_remoteExecutablePath,
        NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE);
    if (fileTransfer == NULL) {
        ngisErrorPrint(log, fName, "Can't create File Transfer.\n");
        goto error;
    }
    result = ngisSSHjobFileTransferStart(fileTransfer);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't start file transfer.\n");
        goto error;
    }
    return 1;

error:
    result = ngisSSHjobFileTransferDestroy(fileTransfer);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't destroy file transfer.\n");
    }
    return 0;
}

static int
ngislSSHjobInvokeExecutableStart(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobInvokeExecutableStart";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    result = ngisSSHjobSetStatus(job, NGIS_EXECUTABLE_STATUS_SUBMITTING);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set the job status.\n");
        return 0;
    }
    job->ngsj_iExecutables = 0;
    result = ngislSSHjobInvokeExecutable(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't invoke Executable.\n");
        return 0;
    }
    return 1;
}

static int
ngislSSHjobInvokeExecutable(
    ngisSSHjob_t *job)
{
    char *out = NGIS_DEV_NULL;
    char *err = NGIS_DEV_NULL;
    char *in  = NGIS_DEV_NULL;
    ngisCallback_t callback;
    int index;
    ngisLog_t *log = NULL;
    int ret = 1;
    char *MPIcommand = NULL;
    static const char fName[] = "ngislSSHjobInvokeExecutable";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_iExecutables >= 0);
    NGIS_ASSERT(job->ngsj_iExecutables < job->ngsj_nExecutables);

    index = job->ngsj_iExecutables;
    log   = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called\n");

    if (job->ngsj_command == NULL) {
        NGIS_ASSERT(index == 0);
        /* Create Command String */
        job->ngsj_command = ngisSSHjobCreateExecutableCommand(job);
        if (job->ngsj_command == NULL) {
            ngisErrorPrint(log, fName,
                "Can't create command string for invoke executables.\n");
            ret = 0;
            goto finalize;
        }
    }

    if (job->ngsj_attributes->ngsja_redirectEnable != 0) {
        out = job->ngsj_executables[index].nge_stdout;
        err = job->ngsj_executables[index].nge_stderr;

        NGIS_ASSERT(NGIS_STRING_IS_NONZERO(out));
        NGIS_ASSERT(NGIS_STRING_IS_NONZERO(err));
    }

    /* Write Command */
    job->ngsj_nextReadCallback = ngislSSHjobInvokeExecutableReadCallback;
    switch (job->ngsj_attributes->ngsja_jobBackend) {
    case NGIS_BACKEND_MPI:
    case NGIS_BACKEND_BLACS:
        NGIS_ASSERT(index == 0);
        MPIcommand = ngisSSHjobCreateMPIcommand(job, NULL);
        if (MPIcommand == NULL) {
            ngisErrorPrint(log, fName, "Can't create MPI command.\n");
            ret = 0;
            goto finalize;
        }
        callback = ngisCallbackWriteFormat(
            job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
            "%s > %s 2> %s < %s & %s $!\n",
            MPIcommand, out, err, in, NGIS_ECHO_COMMAND);
        break;
    case NGIS_BACKEND_NORMAL:
        callback = ngisCallbackWriteFormat(
            job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job,
            "%s > %s 2> %s < %s & %s $!\n",
            job->ngsj_command, out, err, in, NGIS_ECHO_COMMAND);
        break;
    default:
        callback = NULL;
        NGIS_ASSERT_NOTREACHED();
    }
    if (!ngisCallbackIsValid(callback)) {
        ngisDebugPrint(log, fName,
            "Can't write command invoking a executable.\n");
        ret = 0;
        goto finalize;
    }

finalize:
    NGIS_NULL_CHECK_AND_FREE(MPIcommand);

    return ret;
}

static int
ngislSSHjobUlimitMaxMemory(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    static const char fName[] = "ngislSSHjobUlimitMaxMemory";

    NGIS_ASSERT(job != NULL);

    job->ngsj_nextReadCallback = ngislSSHjobCommandReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job, 
        "%s -d %d && %s %s || %s %s\n", 
        NGIS_ULIMIT_COMMAND,
        job->ngsj_attributes->ngsja_maxMemory * 1024, 
        /* Attribute's unit is MB, ulimit's unit is kB  */
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_SUCCESS,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;

    return 1;
}

static int
ngislSSHjobChangeDirectory(
    ngisSSHjob_t *job)
{
    ngisCallback_t callback;
    static const char fName[] = "ngislSSHjobChangeDirectory";

    NGIS_ASSERT(job != NULL);

    /* CD */
    job->ngsj_nextReadCallback = ngislSSHjobCommandReadCallback;
    callback = ngisCallbackWriteFormat(
        job->ngsj_stdio.ngsio_in, ngisSSHjobWriteStringCallback, job, 
        "cd %s && %s %s || %s %s\n", 
        job->ngsj_attributes->ngsja_workDirectory,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_SUCCESS,
        NGIS_ECHO_COMMAND, NGIS_SSH_JOB_COMMAND_FAILED);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't send command string.\n");
        return 0;
    }
    job->ngsj_stdinCallback = callback;

    return 1;
}

static int
ngislSSHjobSetEnvironment(
    ngisSSHjob_t *job)
{
    char *name = NULL;
    char *value = NULL;
    int ret = 1;
    ngisCallback_t callback;
    NGIS_LIST_OF(char) *list;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobSetEnvironment";

    NGIS_ASSERT(job != NULL);
    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");

    list = &job->ngsj_attributes->ngsja_environments;
    if (job->ngsj_iterEnv == NGIS_LIST_END(char, list)) {
        /* Next */
        job->ngsj_iterEnv = NULL;
        return ngisSSHjobFunctionPop(job);
    }

    if (!NGIS_LIST_ITERATOR_IS_VALID(char, job->ngsj_iterEnv)) {
        ngisErrorPrint(log, fName, "Invalid list iterator.\n");
        ret = 0;
        goto finalize;
    }

    /* Analyze environment */
    result = ngisEnvironmentAnalyze(
        NGIS_LIST_GET(char, job->ngsj_iterEnv), &name, &value);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't analyze Environment.\n");
        ret = 0;
        goto finalize;
    }

    callback = ngisCallbackWriteFormat(job->ngsj_stdio.ngsio_in,
        ngislSSHjobSetEnvironmentWriteCallback, job,
        "%s=%s;export %s\n", name, value, name);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName, "Can't send command string.\n");
        ret = 0;
        goto finalize;
    }
    job->ngsj_stdinCallback = callback;

finalize:
    NGIS_NULL_CHECK_AND_FREE(name);
    NGIS_NULL_CHECK_AND_FREE(value);

    return ret;
}

static void
ngislSSHjobSetEnvironmentWriteCallback(
    void *arg,
    int fd,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobSetEnvironmentWriteCallback";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fd >= 0);

    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "Called.\n");
    job->ngsj_stdinCallback = NULL;

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName, "Callback is canceled.\n");
        return;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName, "Can't write the string.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_EOF:
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    /* Next */
    job->ngsj_iterEnv = NGIS_LIST_NEXT(char, job->ngsj_iterEnv);

    result = ngislSSHjobSetEnvironment(job);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set environment variable.\n");
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

static void
ngislSSHjobInvokeExecutableReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    char *pid = NULL;
    int result;
    ngisTokenAnalyzer_t tokenAnalyzer;
    int tokenAnalyzerInitialized = 0;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobInvokeExecutableReadCallback";

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

    ngisTokenAnalyzerInitialize(&tokenAnalyzer, line);
    tokenAnalyzerInitialized = 0;
    
    /* Get pid */
    pid = ngisTokenAnalyzerGetString(&tokenAnalyzer);
    if (pid == NULL) {
        ngisErrorPrint(log, fName, "Can't get pid.\n");
        goto next;
    }

    /* Check End */
    result = ngisTokenAnalyzerNext(&tokenAnalyzer);
    if (result != 0) {
        ngisErrorPrint(log, fName, "Unnecessary token.\n.");
        goto next;
    }
    ngisTokenAnalyzerFinalize(&tokenAnalyzer);
    tokenAnalyzerInitialized = 0;

next:
    if (pid != NULL) {
        /* Set PID and status */
        job->ngsj_executables[job->ngsj_iExecutables].nge_identifier = pid;
        pid = NULL;
        result = ngisSSHjobExecutableSetStatus(job, 
            &job->ngsj_executables[job->ngsj_iExecutables],
             NGIS_EXECUTABLE_STATUS_ACTIVE);
        if (result == 0) {
            ngisDebugPrint(log, fName, "Can't set the job status.\n");
            goto error;
        }
    } else {
        result = ngisSSHjobExecutableSetStatus(job, 
            &job->ngsj_executables[job->ngsj_iExecutables],
             NGIS_EXECUTABLE_STATUS_FAILED);
        if (result == 0) {
            ngisDebugPrint(log, fName, "Can't set the job status.\n");
            goto error;
        }
    }

    job->ngsj_iExecutables++;
    if (job->ngsj_iExecutables < job->ngsj_nExecutables) {
        /* Continue Loop */
        result = ngislSSHjobInvokeExecutable(arg);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't invoke the next executable.\n.");
            goto error;
        }
    } else {
        /* Next */
        job->ngsj_iExecutables = 0;

        /* Polling Start */
        result = ngisSSHjobPollingStart(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't start status polling.\n");
            goto error;
        }

    }
    
    return;
error:
    NGIS_NULL_CHECK_AND_FREE(pid);

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


static void
ngislSSHjobCommandReadCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngislSSHjobCommandReadCallback";

    NGIS_ASSERT(job != NULL);
    log = job->ngsj_log;

    ngisDebugPrint(log, fName, "called.\n");

    switch (cResult) {
    case NGIS_CALLBACK_RESULT_EOF:
        ngisErrorPrint(log, fName,
            "Unexcept EOF.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_CANCEL:
        ngisDebugPrint(log, fName,
            "Callback is canceled.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_FAILED:
        ngisErrorPrint(log, fName,
            "Can't read the reply.\n");
        goto error;
    case NGIS_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }

    if (strcmp(line, NGIS_SSH_JOB_COMMAND_FAILED) == 0) {
        ngisErrorPrint(log, fName,
            "command failed.\n");
        /* Failed */
        goto error;
    }

    if (strcmp(line, NGIS_SSH_JOB_COMMAND_SUCCESS) != 0) {
        ngisErrorPrint(log, fName,
            "Unexpect the reply \"%s\".\n", line);
        goto error;
    }

    /* Next */
    result = ngisSSHjobFunctionPop(job);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't execute the next process.\n");
        goto error;
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
