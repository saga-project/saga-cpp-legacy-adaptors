/*
 * $RCSfile$ $Revision$ $Date$
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
#ifndef NG_INVOKE_SEVER_SSH_H
#define NG_INVOKE_SEVER_SSH_H

#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "ngInvokeServer.h"

#define NGIS_ECHO_COMMAND        "/bin/echo"
#define NGIS_GREP_COMMAND        "/bin/grep"
#define NGIS_CHMOD_COMMAND       "/bin/chmod"
#define NGIS_MKDIR_COMMAND       "/bin/mkdir"
#define NGIS_CAT_COMMAND         "/bin/cat"
#define NGIS_RM_COMMAND          "/bin/rm -f"
#define NGIS_KILL_COMMAND        "/bin/kill"
#define NGIS_QUERY_COMMAND       "/bin/kill -0"

#define NGIS_DEFAULT_REMOTE_TEMPDIR "~/"

#define NGIS_DEFAULT_SSH_COMMAND    "/usr/bin/ssh"
#define NGIS_DEFAULT_MPI_COMMAND    "mpirun"
#define NGIS_DEFAULT_SUBMIT_COMMAND "qsub"
#define NGIS_DEFAULT_STATUS_COMMAND "qstat"
#define NGIS_DEFAULT_DELETE_COMMAND "qdel"
#define NGIS_ULIMIT_COMMAND         "ulimit"
#define NGIS_MPI_DEFAULT_N_PROCESSORS_OPTION "-np %d"
#define NGIS_MPI_DEFAULT_MACHINEFILE_OPTION  "-machinefile %s"

#define NGIS_DEV_NULL            "/dev/null"

#define NGIS_SSH_JOB_ACTIVE      "JOB_ACTIVE"
#define NGIS_SSH_JOB_CHMOD_DONE  "JOB_CHMOD_DONE"
#define NGIS_SSH_JOB_TMPDIR      "JOB_TMPDIR"
#define NGIS_SSH_JOB_PID         "JOB_PID"
#define NGIS_SSH_JOB_STDOUT      "JOB_STDOUT"
#define NGIS_SSH_JOB_STDERR      "JOB_STDERR"
#define NGIS_SSH_JOB_EXIT        "JOB_EXIT"
#define NGIS_SSH_JOB_NONE        "JOB_NONE"
#define NGIS_SSH_JOB_DONE        "JOB_DONE"
#define NGIS_SSH_JOB_FAILED      "JOB_FAILED"

#define NGIS_SSH_JOB_COMMAND_SUCCESS "SUCCESS"
#define NGIS_SSH_JOB_COMMAND_FAILED  "FAILED"

#define NGIS_DEFAULT_SGE_PE "*mpi*"

#define NGIS_SSH_JOB_DEFAULT_POLLING_TIME 3

/* Types */
typedef struct ngisSSHjob_s           ngisSSHjob_t;
typedef struct ngisExecutable_s       ngisExecutable_t;
typedef int (ngisSSHjobFunc_t)(ngisSSHjob_t *);/* Not Pointer */

NGIS_DECLARE_LIST_OF(ngisSSHjobFunc_t);

/**
 * Status of Remote Executable
 */
typedef enum ngisExecutableStatus_e {
    NGIS_EXECUTABLE_STATUS_UNSUBMITTED,
    NGIS_EXECUTABLE_STATUS_SUBMITTING,
    NGIS_EXECUTABLE_STATUS_PENDING,
    NGIS_EXECUTABLE_STATUS_ACTIVE,
    NGIS_EXECUTABLE_STATUS_EXIT,
    NGIS_EXECUTABLE_STATUS_FAILED,
    NGIS_EXECUTABLE_STATUS_STAGEOUT,
    NGIS_EXECUTABLE_STATUS_FAILED_STAGEOUT,
    NGIS_EXECUTABLE_STATUS_DONE
} ngisExecutableStatus_t;

typedef enum ngisSSHjobTransferTarget_s {
    NGIS_SSH_JOB_TRANSFER_STDOUT,
    NGIS_SSH_JOB_TRANSFER_STDERR
} ngisSSHjobTransferTarget_t;

typedef enum ngisSSHjobManagerType_s {
    NGIS_SSH_JOB_MANAGER_NORMAL,
    NGIS_SSH_JOB_MANAGER_SGE,
    NGIS_SSH_JOB_MANAGER_PBS
} ngisSSHjobManagerType_t;

/**
 * File Transfer Direction
 */
typedef enum ngisSSHjobFileTransferDirection_s {
    NGIS_SSH_JOB_FILE_TRANSFER_REMOTE_TO_LOCAL,
    NGIS_SSH_JOB_FILE_TRANSFER_LOCAL_TO_REMOTE
} ngisSSHjobFileTransferDirection_t;

/**
 * Attributes of SSH job
 */
typedef struct ngisSSHjobAttributes_s {
    ngisJobAttributes_t *ngsja_attributes;
    char                *ngsja_sshCommand;
    char                *ngsja_sshRemoteSh;
    char                *ngsja_sshUser;
    NGIS_LIST_OF(char)   ngsja_sshOptions;
    char                *ngsja_sshMPIcommand;
    char                *ngsja_sshRemoteTempdir;
    char                *ngsja_sshSubmitCommand;
    char                *ngsja_sshStatusCommand;
    char                *ngsja_sshDeleteCommand;
    NGIS_LIST_OF(char)   ngsja_sshMPIoptions;
    char                *ngsja_sshMPInProcessorsOption;
    char                *ngsja_sshMPImachinefileOption;
    char                *ngsja_sshSGEparallelEnvironment;
    int                  ngsja_sshPBSprocessorsPerNode;
    char                *ngsja_sshPBSrsh;
} ngisSSHjobAttributes_t;

#define ngsja_hostname               ngsja_attributes->ngja_hostname
#define ngsja_port                   ngsja_attributes->ngja_port
#define ngsja_jobManager             ngsja_attributes->ngja_jobManager
#define ngsja_clientHostname         ngsja_attributes->ngja_clientHostname
#define ngsja_executablePath         ngsja_attributes->ngja_executablePath
#define ngsja_jobBackend             ngsja_attributes->ngja_jobBackend
#define ngsja_arguments              ngsja_attributes->ngja_arguments
#define ngsja_argumentsInitialized    ngsja_attributes->ngja_argumentsInitialized
#define ngsja_workDirectory          ngsja_attributes->ngja_workDirectory
#define ngsja_environments           ngsja_attributes->ngja_environments
#define ngsja_environmentsInitialized \
    ngsja_attributes->ngja_environmentsInitialized
#define ngsja_statusPolling          ngsja_attributes->ngja_statusPolling
#define ngsja_refreshCredential      ngsja_attributes->ngja_refreshCredential
#define ngsja_count                  ngsja_attributes->ngja_count
#define ngsja_staging                ngsja_attributes->ngja_staging
#define ngsja_redirectEnable         ngsja_attributes->ngja_redirectEnable
#define ngsja_stdoutFile             ngsja_attributes->ngja_stdoutFile
#define ngsja_stderrFile             ngsja_attributes->ngja_stderrFile
#define ngsja_maxTime                ngsja_attributes->ngja_maxTime
#define ngsja_maxWallTime            ngsja_attributes->ngja_maxWallTime
#define ngsja_maxCpuTime             ngsja_attributes->ngja_maxCpuTime
#define ngsja_queueName              ngsja_attributes->ngja_queueName
#define ngsja_project                ngsja_attributes->ngja_project
#define ngsja_hostCount              ngsja_attributes->ngja_hostCount
#define ngsja_minMemory              ngsja_attributes->ngja_minMemory
#define ngsja_maxMemory              ngsja_attributes->ngja_maxMemory
#define ngsja_tmpDir                 ngsja_attributes->ngja_tmpDir

/**
 * Executable
 */
struct ngisExecutable_s {
    char                   *nge_identifier;
    ngisExecutableStatus_t  nge_status;
    ngisExecutableStatus_t  nge_tmpStatus;/* For SGE */
    char                   *nge_stdout;
    char                   *nge_stderr;
    int                     nge_exitCode;
};

typedef struct ngisSSHjobFileTransfer_s {
    int                               ngsjft_stderr;
    ngisLineBuffer_t                 *ngsjft_lBufStderr;
    ngisCallback_t                    ngsjft_callback;
    pid_t                             ngsjft_pid;
    char                             *ngsjft_localFilename;
    char                             *ngsjft_remoteFilename;
    ngisSSHjobFileTransferDirection_t ngsjft_direction;
    ngisSSHjob_t                     *ngsjft_job;
} ngisSSHjobFileTransfer_t;

/**
 * Job using SSH
 */
struct ngisSSHjob_s {
    ngisJob_t                       ngsj_base;

    /* Executables */
    ngisExecutable_t               *ngsj_executables;
    int                             ngsj_nExecutables;

    /* IO */
    ngisStandardIO_t                ngsj_stdio;
    ngisCallback_t                  ngsj_stdinCallback;
    ngisLineBuffer_t               *ngsj_lBufStdout;
    ngisLineBuffer_t               *ngsj_lBufStderr;

    /* Timer */
    ngisCallback_t                  ngsj_timerCallback;
    int                             ngsj_statusChecking;

    /* Attribute */
    ngisSSHjobAttributes_t         *ngsj_attributes;
    
    /* Other */
    char                           *ngsj_remoteExecutablePath;
    char                           *ngsj_remoteTempdir;
    char                           *ngsj_remoteTempdirBase;
    char                           *ngsj_pidRemoteSh;
    NGIS_LIST_OF(ngisSSHjobFunc_t)  ngsj_funcQueue;
    int                             ngsj_cancelCalled;
    ngisSSHjobManagerType_t         ngsj_jobManagerType;
    int                             ngsj_afterExit;

    /* For Queueing System */
    char                           *ngsj_localScriptName;
    char                           *ngsj_remoteScriptName;
    char                           *ngsj_localScriptNameForArrayJob;
    char                           *ngsj_remoteScriptNameForArrayJob;

    /* Use Temporary */
    char                           *ngsj_command;
    int                             ngsj_iExecutables;
    ngisLineBufferCallbackFunc_t    ngsj_nextReadCallback;
    NGIS_LIST_ITERATOR_OF(char)     ngsj_iterEnv;

    ngisSSHjobFileTransfer_t       *ngsj_fileTransfer;
};

#define ngsj_log    ngsj_base.ngj_log
#define ngsj_handle ngsj_base.ngj_handle

/* Job Attributes */
ngisSSHjobAttributes_t *ngisSSHjobAttributesCreate(ngisOptionContainer_t *);
void ngisSSHjobAttributesDestroy(ngisSSHjobAttributes_t *);

/* Create Command */
char *ngisSSHjobCreateSSHcommand(ngisSSHjob_t *, char *);
char *ngisSSHjobCreateExecutableCommand(ngisSSHjob_t *);
char *ngisSSHjobCreateMPIcommand(ngisSSHjob_t *, const char *);

/* Function Queue */
int ngisSSHjobFunctionPop(ngisSSHjob_t *job);
int ngisSSHjobFunctionPush(ngisSSHjob_t *job, ngisSSHjobFunc_t *func);
void ngisSSHjobFunctionReset(ngisSSHjob_t *job);

int ngisSSHjobDone(ngisSSHjob_t *);
int ngisSSHjobPollingStart(ngisSSHjob_t *);

/* Assistant Function */
void ngisSSHjobWriteStringCallback(void *, int, ngisCallbackResult_t);
void ngisSSHjobCancelWriteCallback(void *, int, ngisCallbackResult_t);

/* Prepare */
int ngisSSHjobPrepare(ngisSSHjob_t *);
int ngisSSHjobPrepareCommon(ngisSSHjob_t *);

/* For query */
int ngisSSHjobFinishQuery(ngisSSHjob_t *);

/* */
int ngisSSHjobProcessAfterExit(ngisSSHjob_t *);

/* Callback  */
void ngisSSHjobErrorCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

void ngisSSHjobPollingCallback(void *arg, ngisCallbackResult_t);
void ngisSSHjobFileTransferErrorCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);

/* File Transfer */
ngisSSHjobFileTransfer_t *ngisSSHjobFileTransferCreate(ngisSSHjob_t *,
    const char *, const char *, ngisSSHjobFileTransferDirection_t);
int ngisSSHjobFileTransferDestroy(ngisSSHjobFileTransfer_t *);
int ngisSSHjobFileTransferStart(ngisSSHjobFileTransfer_t *);

/* Script */
int ngisSSHjobWriteEnvironmentsToScript(ngisSSHjob_t *, FILE *);
int ngisSSHjobWriteWorkDirectoryToScript(ngisSSHjob_t *, FILE *);
void ngisSSHjobQueryStatusReadDummyCallback(
    void *, ngisLineBuffer_t *, char *, ngisCallbackResult_t);
int ngisSSHjobStagingScript(ngisSSHjob_t *job);
int ngisSSHjobStagingScriptForArrayJob(ngisSSHjob_t *job);
int ngisSSHjobCreateScriptForArrayJob(ngisSSHjob_t *);
int ngisSSHjobWriteSingleJobCommandToScript(ngisSSHjob_t *, FILE *);
int ngisSSHjobWriteArrayJobCommandToScript(
    ngisSSHjob_t *, FILE* fp, const char *, const char *);
int ngisSSHjobWriteMPICommandToScript(ngisSSHjob_t *, FILE *, const char *);

/* SGE */
int ngisSSHjobSGEqueryStatus(ngisSSHjob_t *);
int ngisSSHjobSGEprepare(ngisSSHjob_t *);
int ngisSSHjobSGEdoCancel(ngisSSHjob_t *);

/* PBS */
int ngisSSHjobPBSqueryStatus(ngisSSHjob_t *);
int ngisSSHjobPBSprepare(ngisSSHjob_t *);
int ngisSSHjobPBSdoCancel(ngisSSHjob_t *);

int ngisSSHjobExecutableSetStatus(ngisSSHjob_t *,
    ngisExecutable_t *, ngisExecutableStatus_t);
ngisExecutableStatus_t ngisSSHjobGetStatus(ngisSSHjob_t *);
int ngisSSHjobSetStatus( ngisSSHjob_t *, ngisExecutableStatus_t);
int ngisSSHjobGetNexecutable(ngisSSHjob_t *, ngisExecutableStatus_t);
#endif /* NG_INVOKE_SEVER_SSH_H */
