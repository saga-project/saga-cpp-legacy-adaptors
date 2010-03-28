/*
 * $RCSfile: ngisContext.c,v $ $Revision: 1.9 $ $Date: 2008/02/26 06:32:39 $
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

/**
 * Context for Invoke Server
 */

#include "ngEnvironment.h"
#ifdef NG_PTHREAD

#include "ngInvokeServer.h"

NGI_RCSID_EMBED("$RCSfile: ngisContext.c,v $ $Revision: 1.9 $ $Date: 2008/02/26 06:32:39 $")


/**
 * Prototype
 */
static int ngislContextInitializeSync(ngisiContext_t *, int *);
static int ngislContextFinalizeSync(ngisiContext_t *, int *);
static void ngislContextInitializeMember(ngisiContext_t *);
static int ngislGlobusGassInitialize(ngisiContext_t *, int *);
static int ngislGlobusGassFinalize(ngisiContext_t *, int *);
static int ngislContextOutputLog(
    ngisiContext_t *, char *, int, char **, int *);

/**
 * Functions
 */

/**
 * Context Initialize
 */
int
ngisiContextInitialize(
    ngisiContext_t *context,
    char *serverType,
    char *logFileName,
    int argc,
    char **argv,
    int *error)
{
    static const char fName[] = "ngisiContextInitialize";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    ngislContextInitializeMember(context);

    /* Create the log */
    result = ngisiLogInitialize(
        context, &context->ngisc_log, logFileName, error);
    if (result == 0) {
        /* No error output */
        return 0;
    }

    context->ngisc_serverType = strdup(serverType);
    if (context->ngisc_serverType == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(serverType) failed.\n");
        return 0;
    }

    /* log */
    result = ngislContextOutputLog(context, logFileName, argc, argv, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Output the Context information failed.\n");
        return 0;
    }

    result = ngisiGlobusInitialize(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Globus Toolkit failed.\n");
        return 0;
    }

    result = ngislContextInitializeSync(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Sync failed.\n");
        return 0;
    }

    result = ngisiRefreshCredentialInitialize(
        context, &context->ngisc_refreshCredential, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Refresh Credential failed.\n");
        return 0;
    }

    /* set buffer */
    setlinebuf(NGISI_FP_REQUEST);
    setlinebuf(NGISI_FP_REPLY);
    setlinebuf(NGISI_FP_NOTIFY);
    
    context->ngisc_maxJobID = 0;

    result = ngisiRequestReaderInitialize(
        context, &context->ngisc_requestReader, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Request Reader failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Context Finalize
 */
int
ngisiContextFinalize(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiContextFinalize";
    char *serverType;
    int result, retResult;

    serverType = context->ngisc_serverType;

    retResult = 1;

    result = ngisiRefreshCredentialFinalize(
        context, &context->ngisc_refreshCredential, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Refresh Credential failed.\n");
        retResult = 0;
    }

    result = ngisiRequestReaderFinalize(
        context, &context->ngisc_requestReader, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the Request Reader failed.\n");
        retResult = 0;
    }

    result = ngisiJobConnectionClosed(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Connection Close for all Jobs were failed.\n");
        retResult = 0;
    }

    result = ngisiJobDestructAll(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Destruct all Jobs failed.\n");
        retResult = 0;
    }
    
    result = ngislContextFinalizeSync(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize Sync failed.\n");
        retResult = 0;
    }

    result = ngisiGlobusFinalize(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize Globus Toolkit failed.\n");
        retResult = 0;
    }

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Invoke Server %s finished.\n", serverType);
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "log output finished.\n");
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Invoke Server %s process exiting.\n", serverType);

    result = ngisiLogFinalize(context, &context->ngisc_log, error);
    if (result == 0) {
        /* No error output */
        retResult = 0;
    }

    serverType = NULL;
    globus_libc_free(context->ngisc_serverType);
    context->ngisc_serverType = NULL;

    ngislContextInitializeMember(context);

    if (retResult == 0) {
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize Context Synchronization
 */
static int
ngislContextInitializeSync(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislContextInitializeSync";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the mutex */
    result = ngisiMutexInitialize(
        context, &context->ngisc_jobMutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Mutex failed.\n");
        return 0;    
    }
    context->ngisc_jobMutexInitialized = 1;

    /* Initialize the cond */
    result = ngisiCondInitialize(
        context, &context->ngisc_jobCond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the Cond failed.\n");
        return 0;    
    }
    context->ngisc_jobCondInitialized = 1;

    /* Success */
    return 1;
}

/**
 * Finalize Context Synchronization
 */
static int
ngislContextFinalizeSync(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngislContextFinalizeSync";
    int result;

    /* Check the arguments */
    assert(context != NULL);

    /* Finalize the mutex */
    if (context->ngisc_jobMutexInitialized != 0) {
        context->ngisc_jobMutexInitialized = 0;
        result = ngisiMutexFinalize(
            context, &context->ngisc_jobMutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Initialize the Mutex failed.\n");
            return 0;    
        }
    }

    /* Finalize the cond */
    if (context->ngisc_jobCondInitialized != 0) {
        context->ngisc_jobCondInitialized = 0;
        result = ngisiCondFinalize(
            context, &context->ngisc_jobCond, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Initialize the Cond failed.\n");
            return 0;    
        }
    }

    /* Success */
    return 1;
}

/**
 * Context Initialize Member
 */
static void
ngislContextInitializeMember(
    ngisiContext_t *context)
{
    /* Check the arguments */
    assert(context != NULL);

    context->ngisc_serverType = NULL;
    context->ngisc_jobMutexInitialized = 0;
    context->ngisc_jobCondInitialized = 0;
    context->ngisc_maxJobID = 0;
    context->ngisc_nJobs = 0;
    context->ngisc_job_head = NULL;
    context->ngisc_gassInitialized = 0;
    context->ngisc_gassURL = NULL;
}

/**
 * Context Output Log
 */
static int
ngislContextOutputLog(
    ngisiContext_t *context,
    char *logFileName,
    int argc,
    char **argv,
    int *error)
{
    static const char fName[] = "ngislContextOutputLog";
    char workingDir[NGISI_PATH_MAX], hostName[NGISI_HOSTNAME_MAX];
    char currentTime[NGISI_LINE_MAX];
    char *resultPtr;
    int i, result;
    time_t curTm;

    /* log */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "Invoke Server %s invoked.\n", context->ngisc_serverType);

    /* log filename */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "logfile = \"%s\".\n", logFileName);

    /* args */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "number of arguments = %d.\n", argc);
    for (i = 0; i < argc; i++) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "arg[%d] = \"%s\".\n", i, argv[i]);
    }

    /*
     * hostname
     * Note: hostname can be changed by GLOBUS_HOSTNAME environment
     * variable.
     */
    result = globus_libc_gethostname(hostName, sizeof(hostName));
    if (result != 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "globus_libc_gethostname() failed.\n");
    } else {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "hostname = \"%s\".\n", hostName);
    }

    /* pid */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "pid = %ld.\n", (long)getpid());

    /* pid */
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "parent pid = %ld.\n", (long)getppid());

    /* cwd */
    resultPtr = getcwd(workingDir, sizeof(workingDir));
    if (resultPtr == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "getcwd() failed.\n");
    } else {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
            "cwd = \"%s\".\n", workingDir);
    }

    /* current time */
    time(&curTm);
    strftime(currentTime, sizeof(currentTime),
        "%a %b %d %T %Y %Z", localtime(&curTm));
    ngisiLogPrintf(context, NGISI_LOG_LEVEL_DEBUG, fName,
        "time %d = \"%s\".\n", curTm, currentTime);

    /* Success */
    return 1;
}

/**
 * Globus Initialize
 */
int
ngisiGlobusInitialize(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiGlobusInitialize";
    int result;

    result = globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d.\n",
            "globus_module_activate()", result);
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "failed by %d because :%s.\n", result,
            globus_gram_protocol_error_string(result));
        return 0;
    }

    result = globus_module_activate(GLOBUS_GASS_SERVER_EZ_MODULE);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d.\n",
            "globus_module_activate()", result);
        return 0;
    }

    result = ngislGlobusGassInitialize(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Initialize the GASS failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Globus Finalize
 */
int
ngisiGlobusFinalize(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiGlobusFinalize";
    int result;

    result = ngislGlobusGassFinalize(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Finalize the GASS failed.\n");
        return 0;
    }

    result = globus_module_deactivate(GLOBUS_GASS_SERVER_EZ_MODULE);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d.\n",
            "globus_module_deactivate()", result);
    }

    result = globus_module_deactivate(GLOBUS_GRAM_CLIENT_MODULE);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "%s failed by %d.\n",
            "globus_module_deactivate()", result);
    }

    /* Success */
    return 1;
}

/**
 * Initialize the GASS server.
 */
static int
ngislGlobusGassInitialize(
    ngisiContext_t *context,
    int *error)
{
    int result;
    char *gassURL;
    static const char fName[] = "ngislGlobusGassInitialize";

    context->ngisc_gassInitialized = 0;

    /* Initialize the GASS server */
    result = globus_gass_server_ez_init(
        &context->ngisc_gassListener, GLOBUS_NULL, NGISI_GASS_SCHEME,
        GLOBUS_NULL, NGISI_GASS_OPTION, GLOBUS_NULL);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "globus_gass_server_ez_init failed by %d.\n", result);
        return 0;
    }

    /* Get the GASS URL */
    gassURL = globus_gass_transfer_listener_get_base_url(
        context->ngisc_gassListener);
    if (gassURL == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Can't get the GASS URL.\n");
        goto error;
    }
    context->ngisc_gassURL = strdup(gassURL);
    if (context->ngisc_gassURL == NULL) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "strdup(gassURL) failed.\n");
        goto error;
    }

    /* Success */
    context->ngisc_gassInitialized = 1;
    return 1;

    /* Error occurred */
error:
    /* Shutdown the GASS server */
    result = globus_gass_server_ez_shutdown(context->ngisc_gassListener);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "globus_gass_server_ez_shutdown failed by %d.\n", result);
    }

    /* Failed */
    return 0;
}

/**
 * Finalize the GASS server.
 */
static int
ngislGlobusGassFinalize(
    ngisiContext_t *context,
    int *error)
{
    int result;
    static const char fName[] = "ngislGlobusGassFinalize";

    if (context->ngisc_gassInitialized == 0) {
        /* Success */
        return 1;
    }
    context->ngisc_gassInitialized = 0;

    /* Free the GASS URL */
    free(context->ngisc_gassURL);

    /* Shutdown the GASS server */
    result = globus_gass_server_ez_shutdown(context->ngisc_gassListener);
    if (result != GLOBUS_SUCCESS) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "globus_gass_server_ez_shutdown failed by %d.\n", result);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Lock the Context Job
 */
int
ngisiContextJobListLock(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiContextJobListLock";
    int result;

    result = ngisiMutexLock(context, &context->ngisc_jobMutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Lock the Job list mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unlock the Context Job
 */
int
ngisiContextJobListUnlock(
    ngisiContext_t *context,
    int *error)
{
    static const char fName[] = "ngisiContextJobListUnlock";
    int result;

    result = ngisiMutexUnlock(context, &context->ngisc_jobMutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Unlock the Job list mutex failed.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Notify the Context Job
 */
int
ngisiContextJobListNotify(
    ngisiContext_t *context,
    int mutexLocked,
    int *error)
{
    static const char fName[] = "ngisiContextJobListNotify";
    int result, doLock;

    doLock = 0;

    if (mutexLocked == 0) {
        result = ngisiMutexLock(context, &context->ngisc_jobMutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Lock the Job list mutex failed.\n");
            return 0;
        }
        doLock = 1;
    }

    result = ngisiCondBroadcast(
        context, &context->ngisc_jobCond, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Notify the Job list failed.\n");
        return 0;
    }

    if (doLock != 0) {
        result = ngisiMutexUnlock(context, &context->ngisc_jobMutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list mutex failed.\n");
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Wait the Context Job
 */
int
ngisiContextJobListWait(
    ngisiContext_t *context,
    int mutexLocked,
    int *error)
{
    static const char fName[] = "ngisiContextJobListWait";
    int result, doLock;

    doLock = 0;

    if (mutexLocked == 0) {
        result = ngisiMutexLock(context, &context->ngisc_jobMutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Lock the Job list mutex failed.\n");
            return 0;
        }
        doLock = 1;
    }

    result = ngisiCondWait(
        context, &context->ngisc_jobCond, &context->ngisc_jobMutex, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
            "Wait the Job list failed.\n");
        return 0;
    }

    if (doLock != 0) {
        result = ngisiMutexUnlock(context, &context->ngisc_jobMutex, error);
        if (result == 0) {
            ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName,
                "Unlock the Job list mutex failed.\n");
            return 0;
        }
    }

    /* Success */
    return 1;
}

#endif /* NG_PTHREAD */

