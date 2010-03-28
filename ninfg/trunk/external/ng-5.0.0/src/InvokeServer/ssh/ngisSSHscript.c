/*
 * $RCSfile: ngisSSHscript.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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

NGI_RCSID_EMBED("$RCSfile: ngisSSHscript.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $")

static int ngislSSHjobTransferScript(
    ngisSSHjob_t *, const char *, const char *);
static int ngislSSHjobWriteBodyToScript(
    ngisSSHjob_t *job, FILE *fp, const char *, const char *);

int
ngisSSHjobWriteEnvironmentsToScript(
    ngisSSHjob_t *job,
    FILE *fp)
{
    ngisSSHjobAttributes_t *attr;
    char *name = NULL;
    char *value = NULL;
    NGIS_LIST_ITERATOR_OF(char) it;
    NGIS_LIST_ITERATOR_OF(char) last;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobWriteEnvironmentsToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    /* Environment */
    it = NGIS_LIST_BEGIN(char, &attr->ngsja_environments);
    last = NGIS_LIST_END(char, &attr->ngsja_environments);
    while (it != last) {
        result = ngisEnvironmentAnalyze(
            NGIS_LIST_GET(char, it), &name, &value);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't analyze environment.\n");
            goto error;
        }

        result = fprintf(fp, "%s=%s;export %s\n", name, value, name);
        if (result < 0) {
            ngisErrorPrint(log, fName,
                "Can't print environment variable to scriptfile.\n");
            goto error;
        }

        NGIS_NULL_CHECK_AND_FREE(value);
        NGIS_NULL_CHECK_AND_FREE(name);
        
        it = NGIS_LIST_NEXT(char, it);
    }
    return 1;

error:
    NGIS_NULL_CHECK_AND_FREE(name);
    NGIS_NULL_CHECK_AND_FREE(value);

    return 0;
}

int
ngisSSHjobWriteWorkDirectoryToScript(
    ngisSSHjob_t *job,
    FILE *fp)
{
    ngisSSHjobAttributes_t *attr;
    int result;
    static const char fName[] = "ngisSSHjobWriteWorkDirectoryToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;

    /* Change Directory */
    if (attr->ngsja_workDirectory != NULL) {
        result = fprintf(fp, 
            "\n"
            "cd %s\n"
            "if test $? -ne 0\n"
            "then\n"
            "     %s \"$1: cd $work: failed\" >&2\n"
            "    exit 1\n"
            "fi\n",
            attr->ngsja_workDirectory, NGIS_ECHO_COMMAND);
        if (result < 0) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't print working directory to scriptfile.\n");
            return 0;
        }
    }
    return 1;
}

void
ngisSSHjobQueryStatusReadDummyCallback(
    void *arg,
    ngisLineBuffer_t *lBuf,
    char *line,
    ngisCallbackResult_t cResult)
{
    ngisSSHjob_t *job = arg;
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobQueryStatusReadDummyCallback";

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
    if (strcmp(line, "dummy") != 0) {
        ngisErrorPrint(log, fName, "Invalid reply.\n");
        goto error;
    }

    switch (job->ngsj_executables[0].nge_status) {
    case NGIS_EXECUTABLE_STATUS_UNSUBMITTED:
    case NGIS_EXECUTABLE_STATUS_SUBMITTING:
        NGIS_ASSERT_NOTREACHED();
        break;
    case NGIS_EXECUTABLE_STATUS_PENDING:
    case NGIS_EXECUTABLE_STATUS_ACTIVE:
        job->ngsj_statusChecking = 0;
        break;
    case NGIS_EXECUTABLE_STATUS_EXIT:
        result = ngisSSHjobProcessAfterExit(job);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't process after exit.\n");
            goto error;
        }
        break;
    case NGIS_EXECUTABLE_STATUS_FAILED:
        goto error;
    case NGIS_EXECUTABLE_STATUS_STAGEOUT:
    case NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT:
    case NGIS_EXECUTABLE_STATUS_DONE:
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

int
ngisSSHjobStagingScript(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobStagingScript";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    /* Create Name */
    job->ngsj_remoteScriptName =
        ngisStrdupPrintf("%s/script.sh", job->ngsj_remoteTempdir);
    if (job->ngsj_remoteScriptName == NULL) {
        ngisErrorPrint(log, fName,
            "Can't create string of the remote script file name.\n");
        return 0;
    }

    result = ngislSSHjobTransferScript(
        job, job->ngsj_localScriptName, job->ngsj_remoteScriptName);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't transfer script.\n");
        return 0;
    }
    return 1;
}

int
ngisSSHjobStagingScriptForArrayJob(
    ngisSSHjob_t *job)
{
    int result;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobStagingScriptForArrayJob";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    result = ngislSSHjobTransferScript(job,
        job->ngsj_localScriptNameForArrayJob,
        job->ngsj_remoteScriptNameForArrayJob);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't transfer script.\n");
        return 0;
    }
    return 1;
}

static int
ngislSSHjobTransferScript(
    ngisSSHjob_t *job,
    const char *local,
    const char *remote)
{
    ngisSSHjobFileTransfer_t *fileTransfer = NULL;
    int ret = 0;
    ngisLog_t *log;
    int result;
    static const char fName[] = "ngislSSHjobTransferScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT_STRING(local);
    NGIS_ASSERT_STRING(remote);

    log = job->ngsj_log;

    fileTransfer = ngisSSHjobFileTransferCreate(
        job, local, remote,
        NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE);
    if (fileTransfer == NULL) {
        ngisErrorPrint(log, fName, "Can't create File Transfer.\n");
        goto finalize;
    }

    result = ngisSSHjobFileTransferStart(fileTransfer);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't start file transfer.\n");
        goto finalize;
    }
    fileTransfer = NULL;

    ret = 1;
finalize:
    if (fileTransfer != NULL) {
        result = ngisSSHjobFileTransferDestroy(fileTransfer);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't destroy file transfer.\n");
        }
    }
    return ret;
}

int
ngisSSHjobCreateScriptForArrayJob(
    ngisSSHjob_t *job)
{
    char *filename = NULL;
    ngisLog_t *log = NULL;
    int ret = 0;
    FILE *fp = NULL;
    int result;
    char *out = NULL;
    char *err = NULL;
    static const char fName[] = "ngisSSHjobCreateScriptForArrayJob";

    NGIS_ASSERT(job != NULL);

    log = job->ngsj_log;

    filename = ngisMakeTempFile(job->ngsj_attributes->ngsja_tmpDir);
    if (filename == NULL) {
        ngisErrorPrint(log, fName, "Can't make temporary file.\n");
        goto finalize;
    }
    job->ngsj_localScriptNameForArrayJob = filename;

    fp = fopen(filename, "w");
    if (fp == NULL) {
        ngisErrorPrint(log, fName, "Can't create FILE structure.\n");
        goto finalize;
    }

    result = fprintf(fp, "#! %s\n", job->ngsj_attributes->ngsja_sshRemoteSh);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't write shell to script.\n");
        goto finalize;
    }

    /* STDOUTERR */
    if (job->ngsj_attributes->ngsja_redirectEnable != 0) {
        out = ngisStrdupPrintf("%s/stdout.$1", job->ngsj_remoteTempdir);
        if (out == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDOUT.\n");
            goto finalize;
        }
        err = ngisStrdupPrintf("%s/stderr.$1", job->ngsj_remoteTempdir);
        if (err == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDERR.\n");
            goto finalize;
        }
    }

    result = ngislSSHjobWriteBodyToScript(job, fp, out, err);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write script to file.\n");
        goto finalize;
    }
    
    /* Success */
    ret = 1;
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
    NGIS_NULL_CHECK_AND_FREE(out);
    NGIS_NULL_CHECK_AND_FREE(err);

    ngisDebugPrint(log, fName, "Sub Script for array job.\n");
    ngisLogDumpFile(log, NGIS_LOG_LEVEL_DEBUG, fName, filename);

    return ret;
}

static int
ngislSSHjobWriteBodyToScript(
    ngisSSHjob_t *job, 
    FILE *fp,
    const char *out,
    const char *err)
{
    ngisLog_t *log = NULL;
    int result;
    static const char fName[] = "ngislSSHjobWriteBodyToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    log = job->ngsj_log;

    if (job->ngsj_command == NULL) {
        job->ngsj_command = ngisSSHjobCreateExecutableCommand(job);
        if (job->ngsj_command == NULL) {
            ngisErrorPrint(log, fName, "Can't create command string.\n");
            return 0;
        }
    }

    result = ngisSSHjobWriteEnvironmentsToScript(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write environments to script.\n");
        return 0;
    }

    result = ngisSSHjobWriteWorkDirectoryToScript(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write work directory to script.\n");
        return 0;
    }

    /* STDOUTERR */
    if (out != NULL) {
        NGIS_ASSERT(out != NULL);
        NGIS_ASSERT(err != NULL);
        result = fprintf(fp, "\n%s > %s 2> %s\n", job->ngsj_command, out, err);
    } else {
        NGIS_ASSERT(out == NULL);
        NGIS_ASSERT(err == NULL);
        result = fprintf(fp, "\n%s\n", job->ngsj_command);
    }
    if (result< 0) {
        ngisErrorPrint(log, fName, "Can't print command string.\n");
        return 0;
    }

    return 1;
}

int
ngisSSHjobWriteSingleJobCommandToScript(
    ngisSSHjob_t *job, 
    FILE *fp)
{
    ngisLog_t *log = NULL;
    int result;
    char *out = NULL;
    char *err = NULL;
    ngisSSHjobAttributes_t *attr = NULL;
    int ret = 0;
    static const char fName[] = "ngisSSHjobWriteSingleJobCommandToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);

    attr = job->ngsj_attributes;
    log = job->ngsj_log;

    /* STDOUTERR */
    if (attr->ngsja_redirectEnable != 0) {
        out = ngisStrdupPrintf("%s/stdout.0", job->ngsj_remoteTempdir);
        if (out == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDOUT.\n");
            goto finalize;
        }
        err = ngisStrdupPrintf("%s/stderr.0", job->ngsj_remoteTempdir);
        if (err == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDERR.\n");
            goto finalize;
        }
    }

    result = ngislSSHjobWriteBodyToScript(job, fp, out, err);
    if (result == 0) {
        ngisErrorPrint(job->ngsj_log, fName, "Can't write body to script.\n");
        goto finalize;
    }

    /* Success */
    ret = 1;
finalize:
    NGIS_NULL_CHECK_AND_FREE(out);
    NGIS_NULL_CHECK_AND_FREE(err);
    return ret;
}

int
ngisSSHjobWriteArrayJobCommandToScript(
    ngisSSHjob_t *job,
    FILE *fp,
    const char *machinefile, 
    const char *rsh)
{
    ngisSSHjobAttributes_t *attr;
    int result;
    static const char fName[] = "ngisSSHjobWriteArrayJobCommandToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT_STRING(machinefile);
    NGIS_ASSERT_STRING(rsh);

    attr = job->ngsj_attributes;

    if (job->ngsj_remoteScriptNameForArrayJob == NULL ) {
        job->ngsj_remoteScriptNameForArrayJob =
            ngisStrdupPrintf("%s/subscript.sh", job->ngsj_remoteTempdir);
        if (job->ngsj_remoteScriptNameForArrayJob == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of the remote script file name.\n");
            return 0;
        }
    }

    /* Array Job */
    result = fprintf(fp,
        "hosts=`cat %s 2> %s`\n"
        "if test -z \"$hosts\"; then\n"
        "   hosts=$HOSTNAME\n"
        "fi\n"
        "counter=0\n"
        "while test $counter -lt %d; do\n"
        "   for host in $hosts; do\n"
        "       if test $counter -lt %d; then\n"
        "           %s $host \"%s %s $counter\" &\n"
        "           counter=`expr $counter + 1`\n"
        "       else\n"
        "           break\n"
        "       fi\n"
        "   done\n"
        "done\n"
        "wait\n",
        machinefile, NGIS_DEV_NULL, attr->ngsja_count, attr->ngsja_count,
        rsh, attr->ngsja_sshRemoteSh,  job->ngsj_remoteScriptNameForArrayJob);
    if (result < 0) {
        ngisErrorPrint(job->ngsj_log, fName,
            "Can't write command for array job to script.\n");
        return 0;
    }

    /* Success */
    return 1;
}

int
ngisSSHjobWriteMPICommandToScript(
    ngisSSHjob_t *job,
    FILE *fp,
    const char *machinefile)
{
    char *command = NULL;
    ngisLog_t *log = NULL;
    int result;
    int ret = 0;
    char *out = NULL;
    char *err = NULL;
    static const char fName[] = "ngisSSHjobWriteMPICommandToScript";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(fp != NULL);
    NGIS_ASSERT_STRING(machinefile);

    log = job->ngsj_log;

    if (job->ngsj_command == NULL) {
        job->ngsj_command = ngisSSHjobCreateExecutableCommand(job);
        if (job->ngsj_command == NULL) {
            ngisErrorPrint(log, fName, "Can't create command string.\n");
            goto finalize;
        }
    }

    /* STDOUTERR */
    if (job->ngsj_attributes->ngsja_redirectEnable != 0) {
        out = ngisStrdupPrintf("%s/stdout.0", job->ngsj_remoteTempdir);
        if (out == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDOUT.\n");
            goto finalize;
        }
        err = ngisStrdupPrintf("%s/stderr.0", job->ngsj_remoteTempdir);
        if (err == NULL) {
            ngisErrorPrint(job->ngsj_log, fName,
                "Can't create string of STDERR.\n");
            goto finalize;
        }
    }

    result = ngisSSHjobWriteEnvironmentsToScript(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write environments to script.\n");
        goto finalize;
    }

    result = ngisSSHjobWriteWorkDirectoryToScript(job, fp);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't write work directory to script.\n");
        goto finalize;
    }

    command = ngisSSHjobCreateMPIcommand(job, machinefile);
    if (command == NULL) {
        ngisErrorPrint(log, fName, "Can't create MPI command.\n");
        goto finalize;
    }

    result = fprintf(fp, "\n%s < %s > %s 2> %s\n", command, NGIS_DEV_NULL, 
        out != NULL?out:NGIS_DEV_NULL,
        err != NULL?err:NGIS_DEV_NULL);
    if (result < 0) {
        ngisErrorPrint(log, fName, "Can't write work directory to script.\n");
        goto finalize;
    }

    ret = 1;
finalize:
    NGIS_NULL_CHECK_AND_FREE(out);
    NGIS_NULL_CHECK_AND_FREE(err);
    NGIS_NULL_CHECK_AND_FREE(command);

    return ret;
}
