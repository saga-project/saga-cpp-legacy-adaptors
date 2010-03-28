#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclNinfgManager.c,v $ $Revision: 1.63 $ $Date: 2006/08/17 07:30:46 $";
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
 * Module of Context for Ninf-G Client.
 */

#ifdef sun
#define _POSIX_PTHREAD_SEMANTICS
#endif /* sun */
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "ng.h"
#include <assert.h>


#define NGCLL_GLOBUS_HOSTNAME "GLOBUS_HOSTNAME"
#define NGCLL_GLOBUS_HOSTNAME_MAX (NGI_HOST_NAME_MAX + 15 + 1)
/* 15 is length of NGCLL_GLOBUS_HOSTNAME */
#define NGCLL_GLOBUS_SIGNAL_ACTIONS_CAPACITY_DEFAULT 16

#ifdef NG_PTHREAD
static pthread_mutex_t ngcllMutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* NG_PTHREAD */

static int ngcllInitialized = 0;

/* List of signals catched by Ninf-G Client */
static int ngcllCatchedSignals[] = {
    SIGHUP,
    SIGINT,
    SIGTERM,
};

/* Temporary File */
typedef struct ngcllTemporaryFile_s {
    struct ngcllTemporaryFile_s *ngtf_next;
    /* File name
     * It seems only one element. However, ngtf_fileName elements allocated in
     * fact.
     */
    char ngtf_fileName[1];
} ngcllTemporaryFile_t;

/**
 * Data for managing the Ninf-G Client.
 */
typedef struct ngcllNinfgManager_s {
    ngRWlock_t  ngnm_rwlOwn;    /* Read/Write Lock for this instance */
    int		ngnm_nContexts;	/* The number of existing Contexts */
    int		ngnm_contextID;	/* Ninf-G Context ID */

    char        *ngnm_globusHostName;
    char        ngnm_oldGlobusHostNameIsValid;

    /* List of Ninf-G Context */
    ngclContext_t	*ngnm_context_head;
    ngclContext_t	*ngnm_context_tail;

    /* Temporary file */
    ngcllTemporaryFile_t *ngnm_tmpFile_head;
    ngcllTemporaryFile_t *ngnm_tmpFile_tail;

    int         ngnm_signalManagerID;
} ngcllNinfgManager_t;
static ngcllNinfgManager_t ngcllNinfgManager;

static char ngcllOldGlobusHostName[NGCLL_GLOBUS_HOSTNAME_MAX];

/**
 * Prototype declaration of internal functions.
 */
static void ngcllNinfgManagerInitializeMember(ngcllNinfgManager_t *);
static void ngcllNinfgManagerInitializePointer(ngcllNinfgManager_t *);

static void ngcllNinfgManagerSignalHandler(int);
static void ngcllNinfgManagerAllJobCancel(void);
static int ngcllNinfgManagerJobCancel(ngcliJobManager_t *, ngLog_t *, int *);

static ngclContext_t * ngcllNinfgManagerGetNextContext(
    ngclContext_t *, ngLog_t *, int *);

static int ngcllNinfgManagerMutexLock(ngLog_t *, int *);
static int ngcllNinfgManagerMutexUnlock(ngLog_t *, int *);

static int ngcllTemporaryFileRegister(char *, ngLog_t *, int *);
static int ngcllTemporaryFileUnregister(char *, ngLog_t *, int *);
static ngcllTemporaryFile_t * ngcllTemporaryFileAllocate(
    char *, ngLog_t *, int *);
static int ngcllTemporaryFileFree(ngcllTemporaryFile_t *, ngLog_t *, int *);
static int ngcllTemporaryFileInitialize(
    ngcllTemporaryFile_t *, char *, ngLog_t *, int *);
static int ngcllTemporaryFileFinalize(ngcllTemporaryFile_t *, ngLog_t *, int *);
static ngcllTemporaryFile_t *ngcllTemporaryFileGetNext(
    ngcllTemporaryFile_t *, ngLog_t *, int *);
static void ngcllTemporaryFileDeleteAll(void);


/**
 * Initialize
 */
int
ngcliNinfgManagerInitialize(ngLog_t *log, int *error)
{
    int result, id;
    int rwLockInitialized = 0;
    int signalInitialized = 0;
    int commonInitialized = 0;
    int globusInitialized = 0;
    int locked = 0;
    static const char fName[] = "ngcliNinfgManagerInitialize";

    id = -1;

    /* Lock the Ninf-G Client Manager */
    result = ngcllNinfgManagerMutexLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Client Manager.\n", fName);
	return 0;
    }
    locked = 1;

    /* Has already initialized? */
    if (ngcllInitialized != 0) {
    	/* Already initialized */

	/* Success */
        goto success;
    }

    /* Initialize the members */
    ngcllNinfgManagerInitializeMember(&ngcllNinfgManager);

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(
        &ngcllNinfgManager.ngnm_rwlOwn, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Read/Write Lock.\n",
            fName);
        goto error;
    }
    rwLockInitialized = 1;

    /* Initialize the Signal Manager */
    result = ngiSignalManagerInitialize(&id, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Signal Manager.\n", fName);
        goto error;
    }
    signalInitialized = 1;
    ngcllNinfgManager.ngnm_signalManagerID = id;

    /* Initialize the Common module */
    result = ngcliGlobusCommonInitialize(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Globus Toolkit.\n", fName);
	goto error;
    }
    commonInitialized = 1;

    /* Initialize the Globus Toolkit */
    result = ngcliGlobusInitialize(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Globus Toolkit.\n", fName);
	goto error;
    }
    globusInitialized = 1;

    /* Initialized */
    ngcllInitialized = 1;

success:

    /* Unlock the Ninf-G Client Manager */
    assert(locked != 0);
    locked = 0;
    result = ngcllNinfgManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngcllInitialized = 0;

    if (globusInitialized != 0) { 
        result = ngcliGlobusFinalize(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Globus Toolkit.\n", fName);
        }
    }

    if (signalInitialized != 0) {
        signalInitialized = 0;
        result = ngiSignalManagerFinalize(id, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Signal Manager.\n", fName);
        }
    }

    if (rwLockInitialized != 0) {
        rwLockInitialized = 0;
        result = ngiRWlockFinalize(
            &ngcllNinfgManager.ngnm_rwlOwn, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Read/Write Lock.\n", fName);
        }
    }

    if (locked != 0) {
        locked = 0;

        /* Unlock the Ninf-G Client Manager */
        result = ngcllNinfgManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
        }
    }

    return 0;
}

/**
 * Finalize
 */
int
ngcliNinfgManagerFinalize(ngLog_t *log, int *error)
{
    int result;
    int locked = 0;
    static const char fName[] = "ngcliNinfgManagerFinalize";

    /* Lock the Ninf-G Client Manager */
    result = ngcllNinfgManagerMutexLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Client Manager.\n", fName);
	return 0;
    }
    locked = 1;

    /* Has not initialized? */
    if (ngcllInitialized == 0) {
    	/* Not initialized yet */
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Manager has not initialized yet.\n", fName);
	goto error;
    }

    /* Does Ninf-G Context exist? */
    if (ngcllNinfgManager.ngnm_nContexts > 0) {
    	/* Exist */
	goto success;
    }

    /* Is Ninf-G Context exist? */
    if ((ngcllNinfgManager.ngnm_context_head != NULL) ||
        (ngcllNinfgManager.ngnm_context_tail != NULL)) {
	NGI_SET_ERROR(error, NG_ERROR_EXIST);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Ninf-G Context is exist.\n", fName);
	goto error;
    }

    /* Restore environment variable GLOBUS_HOSTNAME */
    if (ngcllNinfgManager.ngnm_globusHostName != NULL) {
        if (ngcllNinfgManager.ngnm_oldGlobusHostNameIsValid != 0) {
            result = putenv(ngcllOldGlobusHostName);
        } else {
            result = putenv(NGCLL_GLOBUS_HOSTNAME);
        }
        if (result < 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: putenv failed.\n", fName);
            /* Continue because failure of putenv() is not fatal. */
        }
        globus_libc_free(ngcllNinfgManager.ngnm_globusHostName);
    }

    /* Stop the Signal Manager */
    result = ngiSignalManagerStop(
        ngcllNinfgManager.ngnm_signalManagerID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't stop the Signal Manager.\n", fName);
        goto error;
    }

    /* Finalize the Globus Toolkit */
    result = ngcliGlobusFinalize(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Globus Toolkit.\n", fName);
	goto error;
    }

    /* Finalize the Common module */
    result = ngcliGlobusCommonFinalize(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Globus Toolkit.\n", fName);
	goto error;
    }

    /* Finalize the Signal Manager */
    result = ngiSignalManagerFinalize(
        ngcllNinfgManager.ngnm_signalManagerID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Signal Manager.\n", fName);
        goto error;
    }

    /* Finalize the Read/Write Lock for own instance */
    result = ngiRWlockFinalize(&ngcllNinfgManager.ngnm_rwlOwn, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Read/Write Lock for Ninf-G Manager.\n",
            fName);
        goto error;
    }

    /* Finalize the members */
    ngcllNinfgManagerInitializeMember(&ngcllNinfgManager);

    /* Finalized */
    ngcllInitialized = 0;

success:

    /* Unlock the Ninf-G Client Manager */
    assert(locked != 0);
    locked = 0;
    result = ngcllNinfgManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Ninf-G Client Manager */
    if (locked != 0) {
        locked = 0;
        result = ngcllNinfgManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
        }
    }

    return 0;
}

/**
 * Initialize member of Ninf-G Client Manager.
 */
static void
ngcllNinfgManagerInitializeMember(ngcllNinfgManager_t *ninfgMng)
{
    /* Check the arguments */
    assert(ninfgMng != NULL);

    /* Initialize the pointers */
    ngcllNinfgManagerInitializePointer(ninfgMng);

    /* Initialize the members */
    ninfgMng->ngnm_nContexts = 0;
    ninfgMng->ngnm_contextID = NGI_CONTEXT_ID_MIN;
    ninfgMng->ngnm_oldGlobusHostNameIsValid = 0;
    ninfgMng->ngnm_signalManagerID = 0;
}

/**
 * Initialize pointer of Ninf-G Client Manager.
 */
static void
ngcllNinfgManagerInitializePointer(ngcllNinfgManager_t *ninfgMng)
{
    /* Check the arguments */
    assert(ninfgMng != NULL);

    /* Initialize the pointers */
    ninfgMng->ngnm_globusHostName = NULL;

    ninfgMng->ngnm_context_head = NULL;
    ninfgMng->ngnm_context_tail = NULL;

    ninfgMng->ngnm_tmpFile_head = NULL;
    ninfgMng->ngnm_tmpFile_tail = NULL;
}

/**
 * NinfgManager: Start the signal manager.
 */
int
ngcliNinfgManagerSignalManagerStart(
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliNinfgManagerSignalManagerStart";

    result = ngiSignalManagerStart(
        ngcllNinfgManager.ngnm_signalManagerID,
        log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't start the Signal Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NinfgManager: Stop the signal manager.
 */
int
ngcliNinfgManagerSignalManagerStop(
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliNinfgManagerSignalManagerStop";

    result = ngiSignalManagerStop(
        ngcllNinfgManager.ngnm_signalManagerID,
        log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't stop the Signal Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NinfgManager: Set log to the signal manager.
 */
int
ngcliNinfgManagerSignalManagerLogSet(
    ngLog_t *target,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliNinfgManagerSignalManagerLogSet";

    result = ngiSignalManagerLogSet(
        ngcllNinfgManager.ngnm_signalManagerID,
        target, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the log to Signal Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NinfgManager: Register the Ninf-G signal handler.
 * argSignals == NULL : Use default signal.
 * nArgSignals == 0 : No signal will be register.
 */
int
ngcliNinfgManagerSignalRegister(
    int *argSignals,
    int nArgSignals,
    ngLog_t *log,
    int *error)
{
    int nSignals, *signals, result;
    static const char fName[] = "ngcliNinfgManagerSignalRegister";

    nSignals = 0;
    signals = NULL;

    if (argSignals == NULL) {
        nSignals = NGI_NELEMENTS(ngcllCatchedSignals);
        signals = ngcllCatchedSignals;

    } else if (nArgSignals <= 0) {
        /* Register is not performed */
        return 1;

    } else {
        nSignals = nArgSignals;
        signals = argSignals;
    }

    result = ngiSignalManagerSignalHandlerRegister(
        ngcllNinfgManager.ngnm_signalManagerID,
        NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG,
        signals, nSignals,
        ngcllNinfgManagerSignalHandler, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the signals to Signal Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * NinfgManager: Client signal handler.
 */
static void
ngcllNinfgManagerSignalHandler(int sigNo)
{
    /* Delete the temporary files */
    ngcllTemporaryFileDeleteAll();

    /* Cancel the all Jobs */
    ngcllNinfgManagerAllJobCancel();
}

/**
 * NinfgManager: Cancel the all Jobs.
 */
static void
ngcllNinfgManagerAllJobCancel(void)
#ifdef NG_PTHREAD
{
    int result, error;
    ngclContext_t *context;
    ngcliJobManager_t *jobMng;
    static const char fName[] = "ngcllNinfgManagerAllJobCancel";

    /* Lock the Ninf-G Manager */
    result = ngcliNinfgManagerWriteLock(NULL, &error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Ninf-G Manager.\n", fName);
	return;
    }

    context = ngcllNinfgManagerGetNextContext(NULL, NULL, &error);
    while (context != NULL) {
	/* Lock the list of Job Manager */
	result = ngcliContextJobManagerListWriteLock(context, NULL, &error);
	if (result == 0) {
	    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't lock the list of Job Manager.\n", fName);
	    return;
	}

	/* Get the Job Manager */
    	jobMng = ngcliContextGetNextJobManager(context, NULL, &error);
	while (jobMng != NULL) {
	    ngcllNinfgManagerJobCancel(jobMng, NULL, &error);
    	    jobMng = ngcliContextGetNextJobManager(context, jobMng, &error);
	}

	/* Unlock the list */
	result = ngcliContextJobManagerListWriteUnlock(
	    context, context->ngc_log, &error);
	if (result == 0) {
	    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the list of Job Manager.\n", fName);
	    return;
	}

	/* Get the Ninf-G Context */
	context = ngcllNinfgManagerGetNextContext(context, NULL, &error);
    }

    /* Unlock the manager */
    result = ngcliNinfgManagerWriteUnlock(NULL, NULL);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Ninf-G Manager.\n", fName);
        return;
    }
}
#else /* NG_PTHREAD */
{
    int error;
    ngclContext_t *context;
    ngcliJobManager_t *jobMng;

    context = ngcllNinfgManagerGetNextContext(NULL, NULL, &error);
    while (context != NULL) {
	/* Get the Job Manager */
    	jobMng = ngcliContextGetNextJobManager(context, NULL, &error);
	while (jobMng != NULL) {
	    ngcllNinfgManagerJobCancel(jobMng, NULL, &error);
    	    jobMng = ngcliContextGetNextJobManager(context, jobMng, &error);
	}

	/* Get the Ninf-G Context */
	context = ngcllNinfgManagerGetNextContext(context, NULL, &error);
    }
}
#endif /* NG_PTHERAD */

/**
 * Cancel the Job.
 */
static int
ngcllNinfgManagerJobCancel(
    ngcliJobManager_t *jobMng,
    ngLog_t *log,
    int *error)
{
    int result;
    pid_t pid;
    static const char fName[] = "ngcllNinfgManagerJobCancel";

    /* Check the arguments */
    assert(jobMng != NULL);

    /* Is Job contact valid? */
    if (jobMng->ngjm_jobContact == NULL) {
    	/* Success */
	return 1;
    }

    /* Fork the child process */
    pid = fork();
    if (pid < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: fork failed: %s.\n", fName, strerror(errno));
	return 0;
    } 

    /* Is process child? */
    else if (pid == 0) {
    	result = execlp(
	    NGCLI_JOBCANCEL_COMMAND, NGCLI_JOBCANCEL_COMMAND,
	    NGCLI_JOBCANCEL_ARGUMENT, jobMng->ngjm_jobContact, NULL);
	if (result < 0) {
    	    NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: execlp failed: %s.\n", fName, strerror(errno));

	    return 0;
	}
    }

    /* Do not wait the child process */

    /* Success */
    return 1;
}

/**
 * Register
 */
int
ngcliNinfgManagerRegisterContext(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliNinfgManagerRegisterContext";

    /* Check the arguments */
    assert(context != NULL);
    assert(ngcllInitialized != 0);

    /* Lock the manager */
    result = ngcliNinfgManagerWriteLock(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Manager.\n", fName);
	return 0;
    }
    
    /* Increment the counter */
    ngcllNinfgManager.ngnm_nContexts++;

    /* Append at last of the list */
    context->ngc_next = NULL;
    if (ngcllNinfgManager.ngnm_context_head == NULL) {
    	/* No Context is registered */
    	assert(ngcllNinfgManager.ngnm_context_tail == NULL);
	ngcllNinfgManager.ngnm_context_head = context;
	ngcllNinfgManager.ngnm_context_tail = context;
    } else {
	/* Any Context is already registered */
    	assert(ngcllNinfgManager.ngnm_context_tail != NULL);
    	assert(ngcllNinfgManager.ngnm_context_tail->ngc_next == NULL);
	ngcllNinfgManager.ngnm_context_tail->ngc_next = context;
	ngcllNinfgManager.ngnm_context_tail = context;
    }

    /* Unlock the manager */
    result = ngcliNinfgManagerWriteUnlock(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninf-G Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister
 */
int
ngcliNinfgManagerUnregisterContext(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *prev, *curr;
    static const char fName[] = "ngcliNinfgManagerUnregisterContext";

    /* Check the arguments */
    assert(context != NULL);
    assert(ngcllNinfgManager.ngnm_context_head != NULL);
    assert(ngcllNinfgManager.ngnm_context_tail != NULL);

    /* Lock the manager */
    result = ngcliNinfgManagerWriteLock(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Client Manager.\n", fName);
	return 0;
    }
    
    /* Find the Ninf-G Context */
    prev = NULL;
    curr = ngcllNinfgManager.ngnm_context_head;
    for (; curr != context; curr = curr->ngc_next) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Ninf-G Context */
    if (context == ngcllNinfgManager.ngnm_context_head)
    	ngcllNinfgManager.ngnm_context_head = context->ngc_next;
    if (context == ngcllNinfgManager.ngnm_context_tail)
    	ngcllNinfgManager.ngnm_context_tail = prev;
    if (prev != NULL)
    	prev->ngc_next = context->ngc_next;
    context->ngc_next = NULL;

    /* Unlock the manager */
    result = ngcliNinfgManagerWriteUnlock(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Client Manager.\n", fName);
	return 0;
    }

    /* Decrement the number of Ninf-G Contexts */
    ngcllNinfgManager.ngnm_nContexts--;
    
    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	"%s: Ninf-G Context is not found.\n", fName);

    /* Unlock the manager */
    result = ngcliNinfgManagerWriteUnlock(log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninf-G Manager.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Set environment variable GLOBUS_HOSTNAME
 */
int
ngcliNinfgManagerSetGlobusHostName(
    char *globusHostName,
    ngLog_t *log,
    int *error)
{
    int locked = 0;
    char *string = NULL;
    char *old;
    int result;
    static int isSet = 0;
    static const char fName[] = "ngcliNinfgManagerSetGlobusHostName";

    /* Lock the Ninf-G Client Manager */
    result = ngcllNinfgManagerMutexLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Client Manager.\n", fName);
	return 0;
    }
    locked = 1;

    if (isSet != 0) {
        goto success;
    }
    isSet = 1;

    if (globusHostName == NULL) {
        goto success;
    }

    /* Save old environment */
    old = getenv(NGCLL_GLOBUS_HOSTNAME);
    if (old != NULL) {
        if (strlen(old) > NGI_HOST_NAME_MAX) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Environment variable GLOBUS_HOSTNAME is"
                " too long.\n", fName);
            goto error;
        }
        ngcllNinfgManager.ngnm_oldGlobusHostNameIsValid = 1;

        /* storage for old environment variable is
         * ngcllOldGlobusHostName?
         */
        if (ngcllOldGlobusHostName + strlen(NGCLL_GLOBUS_HOSTNAME) + 1/* = */
                != old) {
            sprintf(ngcllOldGlobusHostName, "%s=%s",
                NGCLL_GLOBUS_HOSTNAME, old);
        }
    }

    string = globus_libc_malloc(
        strlen(NGCLL_GLOBUS_HOSTNAME) + 1/* = */ +
        strlen(globusHostName) + 1);
    if (string == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for environment variable "
            "GLOBUS_HOSTNAME.\n", fName);
        goto error;
    }
    sprintf(string, "%s=%s", NGCLL_GLOBUS_HOSTNAME, globusHostName);

    result = putenv(string);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: putenv failed.\n", fName);
        goto error;
    }

    if (ngcllNinfgManager.ngnm_globusHostName != NULL) {
        globus_libc_free(ngcllNinfgManager.ngnm_globusHostName);
        ngcllNinfgManager.ngnm_globusHostName = NULL;
    }
    ngcllNinfgManager.ngnm_globusHostName = string;

success:

    /* Unlock the Ninf-G Client Manager */
    assert(locked != 0);
    locked = 0;
    result = ngcllNinfgManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (locked != 0) {
        locked = 0;

        /* Unlock the Ninf-G Client Manager */
        result = ngcllNinfgManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't unlock the Ninf-G Client Manager.\n", fName);
        }
    }
    return 0;
}

/**
 * Create the identifier of Ninf-G Context.
 */
int
ngcliNinfgManagerCreateContextID(ngLog_t *log, int *error)
{
    int result;
    int newID;
    static const char fName[] = "ngcliNinfgManagerCreateContextID";

    /* Lock the Ninf-G Manager */
    result = ngcliNinfgManagerWriteLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninf-G Manager.\n", fName);
	return -1;
    }

    /* Get the new ID, which is not used. */
    newID = ngcllNinfgManager.ngnm_contextID;

    newID++;
    if (newID > NGI_CONTEXT_ID_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: There is no IDs for Ninf-G Context to use.\n", fName);

        /* Lock the data for Context include Ninf-G Context */
        result = ngcliNinfgManagerWriteUnlock(log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't unlock the list of Ninf-G Context.\n", fName);
            return -1;
        }
    }

    ngcllNinfgManager.ngnm_contextID = newID;

    /* Lock the data for Context include Ninf-G Context */
    result = ngcliNinfgManagerWriteUnlock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Ninf-G Context.\n", fName);
	return -1;
    }

    return newID;
}

/**
 * Is Ninf-G Context valid?
 */
int
ngcliNinfgManagerIsContextValid(
    ngclContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *curr;
    static const char fName[] = "ngcliNinfgManagerIsContextValid";

    /* Lock the list of Ninf-G Context */
    result = ngcliNinfgManagerReadLock(log, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the list of Ninf-G Context.\n", fName);
	return 0;
    }

    /* Is Ninf-G Context exist? */
    curr = ngcllNinfgManagerGetNextContext(NULL, log, error);
    if (curr == NULL) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: No Ninf-G Context is registered.\n", fName);
	goto error;
    }

    while (curr != context) {
	/* Get the next Ninf-G Context */
        curr = ngcllNinfgManagerGetNextContext(curr, log, error);
	if (curr == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Ninf-G Context is not found.\n", fName);
	    goto error;
    	}
    }

    /* Unlock the list of Ninf-G Context */
    result = ngcliNinfgManagerReadUnlock(log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Ninf-G Context.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list of Ninf-G Context */
    result = ngcliNinfgManagerReadUnlock(log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Ninf-G Context.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Get the Ninf-G Context by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclContext_t *
ngcliNinfgManagerGetContext(int id, ngLog_t *log, int *error)
{
    ngclContext_t *context;
    static const char fName[] = "ngcliNinfgManagerGetContext";

    /* Is ID less than minimum? */
    if (id < NGI_CONTEXT_ID_MIN) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context ID number %d is less than %d.\n", fName, NGI_CONTEXT_ID_MIN);
	return NULL;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_CONTEXT_ID_MAX) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context ID number %d is greater than %d.\n",
	    fName, id, NGI_CONTEXT_ID_MAX);
	return NULL;
    }

    context = ngcllNinfgManager.ngnm_context_head;
    for (; context != NULL; context = context->ngc_next) {
    	assert(context->ngc_ID >= NGI_CONTEXT_ID_MIN);
	assert(context->ngc_ID <= NGI_CONTEXT_ID_MAX);
	if (context->ngc_ID == id) {
	    /* Found */
	    return context;
	}
    }

    /* Not found */

    return NULL;
}

/**
 * Get the next manager.
 *
 * Return the manager from the top of the list, if current is NULL.
 * Return the next manager of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclContext_t *
ngcllNinfgManagerGetNextContext(
    ngclContext_t *current,
    ngLog_t *log,
    int *error)
{
#if 0
    static const char fName[] = "ngcllNinfgManagerGetNextContext";
#endif

    if (current == NULL) {
	/* Return the first manager */
	if (ngcllNinfgManager.ngnm_context_head != NULL) {
	    assert(ngcllNinfgManager.ngnm_context_tail != NULL);
	    return ngcllNinfgManager.ngnm_context_head;
	}
    } else {
	/* Return the next manager */
	if (current->ngc_next != NULL) {
	    return current->ngc_next;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);

    return NULL;
}

/**
 * NinfgManager: Set the signal handler
 */
int
ngclNinfgManagerSetSignalHandler(
    int sigNo,
    void (*handler)(int),
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngclNinfgManagerSetSignalHandler";

    /* Register to Signal Manager. */
    result = ngiSignalManagerSignalHandlerRegister(
        ngcllNinfgManager.ngnm_signalManagerID,
        NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_USER,
        &sigNo, 1, handler, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the signal %d to Signal Manager.\n",
            fName, sigNo);
        return 0;
    }

    return 1;
}

/**
 * Wrapper of mutex lock.
 */
static int
ngcllNinfgManagerMutexLock(ngLog_t *log, int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "ngcllNinfgManagerMutexLock";

    result = pthread_mutex_lock(&ngcllMutex);
    if (result != 0) {
    	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: pthread_mutex_lock failed by %d: %s.\n", fName,
	    result, strerror(result));
	return 0;
    }
    return 1;
}
#else /* NG_PTHREAD */
{
    /* if NonThread flavor, do nothing. */
    /* and always success.              */
    return 1;
}
#endif /* NG_PTHREAD */

/**
 * Wrapper of mutex unlock.
 */
static int
ngcllNinfgManagerMutexUnlock(ngLog_t *log, int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "ngcllNinfgManagerMutexUnlock";

    result = pthread_mutex_unlock(&ngcllMutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: pthread_mutex_unlock failed by %d: %s.\n", fName,
	    result, strerror(result));
        return 0;
    }
    return 1;
}
#else /* NG_PTHREAD */
{
    /* if NonThread flavor, do nothing. */
    /* and always success. */
    return 1;
}
#endif /* NG_PTHREAD */

/**
 * Temporary file: Register.
 */
int
ngcliNinfgManagerTemporaryFileRegister(
    char *fileName,
    ngLog_t *log,
    int *error)
{
    int result;
    int mutexLocked = 0;
    static const char fName[] = "ngcliNinfgManagerTemporaryFileRegister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Lock the mutex */
    result = ngcllNinfgManagerMutexLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninfg Manager.\n", fName);
	goto error;
    }
    mutexLocked = 1;

    /* The Ninfg Manager initialized? */
    if (ngcllInitialized == 0) {
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninfg Manager is not initialized.\n", fName);
	goto error;
    }

    /* Register the temporary file */
    result = ngcllTemporaryFileRegister(fileName, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Temporary File.\n", fName);
	goto error;
    }

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngcllNinfgManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninfg Manager.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngcllNinfgManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Ninfg Manager.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Temporary file: Register.
 */
static int
ngcllTemporaryFileRegister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    ngcllTemporaryFile_t *tmpFile;
    static const char fName[] = "ngcllTemporaryFileRegister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Allocate */
    tmpFile = ngcllTemporaryFileAllocate(fileName, log, error);
    if (tmpFile == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Temporary File.\n", fName);
	return 0;
    }

    /* Initialize */
    result = ngcllTemporaryFileInitialize(tmpFile, fileName, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Temporary File.\n", fName);
	return 0;
    }

    /* Append at last of the list */
    tmpFile->ngtf_next = NULL;
    if (ngcllNinfgManager.ngnm_tmpFile_head == NULL) {
	/* No Temporary File is registered */
	assert(ngcllNinfgManager.ngnm_tmpFile_tail == NULL);
	ngcllNinfgManager.ngnm_tmpFile_head = tmpFile;
	ngcllNinfgManager.ngnm_tmpFile_tail = tmpFile;
    } else {
	/* Any Temporary File is already registered */
	assert(ngcllNinfgManager.ngnm_tmpFile_tail != NULL);
	assert(ngcllNinfgManager.ngnm_tmpFile_tail->ngtf_next == NULL);
	ngcllNinfgManager.ngnm_tmpFile_tail->ngtf_next
	    = (struct ngcllTemporaryFile_s *)tmpFile;
	ngcllNinfgManager.ngnm_tmpFile_tail = tmpFile;
    }

    /* Success */
    return 1;
}

/**
 * Temporary file: Unregister.
 */
int
ngcliNinfgManagerTemporaryFileUnregister(
    char *fileName,
    ngLog_t *log,
    int *error)
{
    int result;
    int mutexLocked = 0;
    static const char fName[] = "ngcliNinfgManagerTemporaryFileUnregister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Lock the mutex */
    result = ngcllNinfgManagerMutexLock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Ninfg Manager.\n", fName);
	goto error;
    }

    /* The Ninfg Manager initialized? */
    if (ngcllInitialized == 0) {
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
	    "%s: Ninfg Manager is not initialized.\n", fName);
	goto error;
    }

    /* Unregister the temporary file */
    result = ngcllTemporaryFileUnregister(fileName, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unregister the Temporary File.\n", fName);
	goto error;
    }

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngcllNinfgManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Ninfg Manager.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngcllNinfgManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Ninfg Manager.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Temporary file: Unregister.
 */
static int
ngcllTemporaryFileUnregister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    ngcllTemporaryFile_t *prev, *curr;
    static const char fName[] = "ngcllTemporaryFileUnregister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Get the top of list */	
    prev = NULL;
    curr = ngcllTemporaryFileGetNext(NULL, log, error);
    while (curr != NULL) {
	if (strcmp(&curr->ngtf_fileName[0], fileName) == 0) {
	    /* Found */
	    goto found;
	}
	prev = curr;
	curr = ngcllTemporaryFileGetNext(curr, log, error);
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	"%s: %s: Temporary File is not found.\n", fName, fileName);

    /* Failed */
    return 0;

    /* Found */
found:
    /* Unregister the Temporary File */
    if (curr == ngcllNinfgManager.ngnm_tmpFile_head)
	ngcllNinfgManager.ngnm_tmpFile_head
	    = (ngcllTemporaryFile_t *)curr->ngtf_next;
    if (curr == ngcllNinfgManager.ngnm_tmpFile_tail)
	ngcllNinfgManager.ngnm_tmpFile_tail = prev;
    if (prev != NULL)
	prev->ngtf_next = curr->ngtf_next;
    curr->ngtf_next = NULL;

    /* Finalize */
    result = ngcllTemporaryFileFinalize(curr, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Temporary File.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllTemporaryFileFree(curr, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Temporary File.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Temporary file: Allocate.
 */
static ngcllTemporaryFile_t *
ngcllTemporaryFileAllocate(char *fileName, ngLog_t *log, int *error)
{
    ngcllTemporaryFile_t *tmpFile;
    static const char fName[] = "ngcllTemporaryFileAllocate";

    /* Check the arguments */
    assert(fileName != NULL);
    assert(fileName[0] != '\0');

    /* Allocate */
    tmpFile = globus_libc_calloc(
	1,
	sizeof (ngcllTemporaryFile_t) - sizeof (tmpFile->ngtf_fileName)
	+ strlen(fileName) + 1);
    if (tmpFile == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Temporary File.\n", fName);
	return NULL;
    }

    /* Success */
    return tmpFile;
}

/**
 * Temporary file: Deallocate.
 */
static int
ngcllTemporaryFileFree(ngcllTemporaryFile_t *tmpFile, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(tmpFile != NULL);

    /* Deallocate */
    globus_libc_free(tmpFile);

    /* Success */
    return 1;
}

/**
 * Temporary file: Initialize.
 */
static int
ngcllTemporaryFileInitialize(
    ngcllTemporaryFile_t *tmpFile,
    char *fileName,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(tmpFile != NULL);
    assert(fileName != NULL);
    assert(fileName[0] != '\0');

    /* Initialize */
    tmpFile->ngtf_next = NULL;
    strcpy(&tmpFile->ngtf_fileName[0], fileName);

    /* Success */
    return 1;
}

/**
 * Temporary file: Finalize
 */
static int
ngcllTemporaryFileFinalize(
    ngcllTemporaryFile_t *tmpFile,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(tmpFile != NULL);

    /* Finalize */
    tmpFile->ngtf_next = NULL;

    /* Success */
    return 1;
}

/**
 * Temporary file: Get the next Temporary file.
 */
static ngcllTemporaryFile_t *
ngcllTemporaryFileGetNext(
    ngcllTemporaryFile_t *current,
    ngLog_t *log,
    int *error)
{
    if (current == NULL) {
        /* Return the head */
        if (ngcllNinfgManager.ngnm_tmpFile_head != NULL) {
            assert(ngcllNinfgManager.ngnm_tmpFile_tail != NULL);
            return ngcllNinfgManager.ngnm_tmpFile_head;
        }
    } else {
        /* Return the next */
        if (current->ngtf_next != NULL) {
            assert(ngcllNinfgManager.ngnm_tmpFile_head != NULL);
            assert(ngcllNinfgManager.ngnm_tmpFile_tail != NULL);
            return (ngcllTemporaryFile_t *)current->ngtf_next;
        }
    }

    /* No data */
    return NULL;
}

/**
 * Temporary File: Delete all files.
 */
static void
ngcllTemporaryFileDeleteAll(void)
{
    ngcllTemporaryFile_t *tmpFile;

    /* Is not initialized? */
    if (ngcllInitialized == 0) {
	/* Success */
	return;
    }

    /* Get the top of list */	
    tmpFile = ngcllNinfgManager.ngnm_tmpFile_head;
    while (tmpFile != NULL) {
    	/* Check the data */
	assert(tmpFile->ngtf_fileName[0] != '\0');

	/* Unlink the Temporary File */
	unlink(&tmpFile->ngtf_fileName[0]);

	/* Get the next */
	tmpFile = (ngcllTemporaryFile_t *)tmpFile->ngtf_next;
    }
}

/**
 * Yield for callback
 */
int
ngclNinfgManagerYieldForCallback(void)
{
    globus_thread_yield();

    /* Success */
    return 1;
}

