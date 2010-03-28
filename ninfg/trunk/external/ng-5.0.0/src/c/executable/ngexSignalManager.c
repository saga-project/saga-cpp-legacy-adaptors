/*
 * $RCSfile: ngexSignalManager.c,v $ $Revision: 1.6 $ $Date: 2007/10/17 11:24:20 $
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
 * Module of Signal Manager.
 */

#include "ngEx.h"

NGI_RCSID_EMBED("$RCSfile: ngexSignalManager.c,v $ $Revision: 1.6 $ $Date: 2007/10/17 11:24:20 $") 

#ifdef NG_PTHREAD
static pthread_mutex_t ngexlSignalMutex = PTHREAD_MUTEX_INITIALIZER;
#endif /* NG_PTHREAD */

/* Temporary File */
typedef struct ngexlTemporaryFile_s {
    struct ngexlTemporaryFile_s *ngtf_next;
    /* File name
     * It seems only one element. However, ngtf_fileName elements allocated in
     * fact.
     */
    char ngtf_fileName[1];
} ngexlTemporaryFile_t;

static int ngexlSignalList[] = {
    SIGHUP,
    SIGINT,
    SIGTERM,
};

/* Signal Manager */
typedef struct ngexlSignalManager_s {
    int ngsm_initialized;

    int ngsm_signalManagerID;

    /* Temporary file */
    ngexlTemporaryFile_t *ngsm_tmpFile_head;
    ngexlTemporaryFile_t *ngsm_tmpFile_tail;
} ngexlSignalManager_t;
static ngexlSignalManager_t ngexlSignalManager;

/**
 * Prototype declaration of static functions.
 */
static void ngexlSignalManagerInitializeMember(ngexlSignalManager_t *);
static void ngexlSignalManagerInitializePointer(ngexlSignalManager_t *);
static void ngexlSignalHandler(int);
static int ngexlTemporaryFileRegister(char *, ngLog_t *, int *);
static int ngexlTemporaryFileUnregister(char *, ngLog_t *, int *);
static ngexlTemporaryFile_t * ngexlTemporaryFileAllocate(
    char *, ngLog_t *, int *);
static int ngexlTemporaryFileFree(ngexlTemporaryFile_t *, ngLog_t *, int *);
static int ngexlTemporaryFileInitialize(
    ngexlTemporaryFile_t *, char *, ngLog_t *, int *);
static int ngexlTemporaryFileFinalize(ngexlTemporaryFile_t *, ngLog_t *, int *);
static ngexlTemporaryFile_t *ngexlTemporaryFileGetNext(
    ngexlTemporaryFile_t *, ngLog_t *, int *);
static void ngexlTemporaryFileDeleteAll(void);
static int ngexlSignalManagerMutexLock(ngLog_t *log, int *error);
static int ngexlSignalManagerMutexUnlock(ngLog_t *log, int *error);

/**
 * Signal Manager: Initialize.
 */
int
ngexiSignalManagerInitialize(ngLog_t *log, int *error)
{
    int result, id;
    int mutexLocked = 0;
    static const char fName[] = "ngexiSignalManagerInitialize";

    id = -1;

    /* Lock the mutex */
    result = ngexlSignalManagerMutexLock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Signal Manager.\n"); 
	goto error;
    }
    mutexLocked = 1;

    if (ngexlSignalManager.ngsm_initialized != 0) {
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Signal Manager is already initialized.\n"); 
	goto error;
    }

    /* Initialize the members */
    ngexlSignalManagerInitializeMember(&ngexlSignalManager);

    /* Register the function at exit */
    result = atexit(ngexlTemporaryFileDeleteAll);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "atexit failed.\n"); 
	goto error;
    }

    /* Initialize the Signal Manager */
    result = ngiSignalManagerInitialize(&id, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Signal Manager.\n"); 
        goto error;
    }
    ngexlSignalManager.ngsm_signalManagerID = id;

    /* Initialized */
    ngexlSignalManager.ngsm_initialized = 1;

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngexlSignalManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Signal Manager.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
	mutexLocked = 0;
        result = ngexlSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Signal Manager: Finalize.
 */
int
ngexiSignalManagerFinalize(ngLog_t *log, int *error)
{
    int result;
    int mutexLocked = 0;
    static const char fName[] = "ngexiSignalManagerFinalize";

    /* Lock the mutex */
    result = ngexlSignalManagerMutexLock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Signal Manager.\n"); 
	goto error;
    }
    mutexLocked = 1;

    if (ngexlSignalManager.ngsm_initialized == 0) {
	NGI_SET_ERROR(error, NG_ERROR_FINALIZE);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Signal Manager is not initialized.\n"); 
	goto error;
    }

    /* Finalized */
    ngexlSignalManager.ngsm_initialized = 0;

    /* Finalize the Signal Manager */
    result = ngiSignalManagerStop(
	ngexlSignalManager.ngsm_signalManagerID, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't stop the Signal Manager.\n"); 
        goto error;
    }

    /* Finalize the Signal Manager */
    result = ngiSignalManagerFinalize(
	ngexlSignalManager.ngsm_signalManagerID, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Signal Manager.\n"); 
        goto error;
    }

    /* Initialize the members */
    ngexlSignalManagerInitializeMember(&ngexlSignalManager);

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngexlSignalManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Signal Manager.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
	mutexLocked = 0;
        result = ngexlSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Signal Manager: Initialize the members.
 */
static void
ngexlSignalManagerInitializeMember(ngexlSignalManager_t *sigMng)
{
    /* Initialize the pointers */
    ngexlSignalManagerInitializePointer(sigMng);

    /* Initialize the members */
    sigMng->ngsm_initialized = 0;
    sigMng->ngsm_signalManagerID = 0;
}

/**
 * Signal Manager: Initialize the pointers.
 */
static void
ngexlSignalManagerInitializePointer(ngexlSignalManager_t *sigMng)
{
    sigMng->ngsm_tmpFile_head = NULL;
    sigMng->ngsm_tmpFile_tail = NULL;
}

/**
 * SignalManager: Start the signal manager.
 */
int
ngexiSignalManagerStart(
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiSignalManagerStart";

    result = ngiSignalManagerStart(
        ngexlSignalManager.ngsm_signalManagerID,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start the Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;

}

/**
 * SignalManager: Stop the signal manager.
 */
int
ngexiSignalManagerStop(
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiSignalManagerStop";

    result = ngiSignalManagerStop(
        ngexlSignalManager.ngsm_signalManagerID,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't stop the Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Set the log.
 */
int
ngexiSignalManagerLogSet(
    ngLog_t *target,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiSignalManagerLogSet";

    result = ngiSignalManagerLogSet(
        ngexlSignalManager.ngsm_signalManagerID,
        target, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the log to Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Register the Ninf-G Executable signal handler.
 * argSignals == NULL : Use default signal.
 * nArgSignals == 0 : No signal will be register.
 */
int
ngexiSignalManagerRegister(
    int *argSignals,
    int nArgSignals,
    ngLog_t *log,
    int *error)
{
    int nSignals, *signals, result;
    static const char fName[] = "ngexiSignalManagerRegister";

    nSignals = 0;
    signals = NULL;

    if (argSignals == NULL) {
        nSignals = NGI_NELEMENTS(ngexlSignalList);
        signals = ngexlSignalList;

    } else if (nArgSignals <= 0) {
        /* Register is not performed */
        return 1;

    } else {
        nSignals = nArgSignals;
        signals = argSignals;
    }

    result = ngiSignalManagerSignalHandlerRegister(
        ngexlSignalManager.ngsm_signalManagerID,
        NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG,
        signals, nSignals,
        ngexlSignalHandler, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the signals to Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Signal handler.
 */
static void
ngexlSignalHandler(int sigNo)
{
    /* Delete the temporary files */
    ngexlTemporaryFileDeleteAll();
}

/**
 * Temporary file: Register.
 */
int
ngexiTemporaryFileRegister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    int mutexLocked = 0;
    static const char fName[] = "ngexiTemporaryFileRegister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Lock the mutex */
    result = ngexlSignalManagerMutexLock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Signal Manager.\n"); 
	goto error;
    }
    mutexLocked = 1;

    /* The Signal Manager initialized? */
    if (ngexlSignalManager.ngsm_initialized == 0) {
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Signal Manager is not initialized.\n"); 
	goto error;
    }

    /* Register the temporary file */
    result = ngexlTemporaryFileRegister(fileName, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't register the Temporary File.\n"); 
	goto error;
    }

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngexlSignalManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Signal Manager.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngexlSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Temporary file: Register.
 */
static int
ngexlTemporaryFileRegister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    ngexlTemporaryFile_t *tmpFile;
    static const char fName[] = "ngexlTemporaryFileRegister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Allocate */
    tmpFile = ngexlTemporaryFileAllocate(fileName, log, error);
    if (tmpFile == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Temporary File.\n"); 
	return 0;
    }

    /* Initialize */
    result = ngexlTemporaryFileInitialize(tmpFile, fileName, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Temporary File.\n"); 
	return 0;
    }

    /* Append at last of the list */
    tmpFile->ngtf_next = NULL;
    if (ngexlSignalManager.ngsm_tmpFile_head == NULL) {
	/* No Temporary File is registered */
	assert(ngexlSignalManager.ngsm_tmpFile_tail == NULL);
	ngexlSignalManager.ngsm_tmpFile_head = tmpFile;
	ngexlSignalManager.ngsm_tmpFile_tail = tmpFile;
    } else {
	/* Any Temporary File is already registered */
	assert(ngexlSignalManager.ngsm_tmpFile_tail != NULL);
	assert(ngexlSignalManager.ngsm_tmpFile_tail->ngtf_next == NULL);
	ngexlSignalManager.ngsm_tmpFile_tail->ngtf_next
	    = (ngexlTemporaryFile_t *)tmpFile;
	ngexlSignalManager.ngsm_tmpFile_tail = tmpFile;
    }

    /* Success */
    return 1;
}

/**
 * Temporary file: Unregister.
 */
int
ngexiTemporaryFileUnregister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    int mutexLocked = 0;
    static const char fName[] = "ngexiTemporaryFileUnregister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Lock the mutex */
    result = ngexlSignalManagerMutexLock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't lock the Signal Manager.\n"); 
	goto error;
    }

    /* The Signal Manager initialized? */
    if (ngexlSignalManager.ngsm_initialized == 0) {
	NGI_SET_ERROR(error, NG_ERROR_INITIALIZE);
	ngLogDebug(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Signal Manager is not initialized.\n"); 
	goto error;
    }

    /* Unregister the temporary file */
    result = ngexlTemporaryFileUnregister(fileName, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unregister the Temporary File.\n"); 
	goto error;
    }

    /* Unlock the mutex */
    mutexLocked = 0;
    result = ngexlSignalManagerMutexUnlock(log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't unlock the Signal Manager.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngexlSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Temporary file: Unregister.
 */
static int
ngexlTemporaryFileUnregister(char *fileName, ngLog_t *log, int *error)
{
    int result;
    ngexlTemporaryFile_t *prev, *curr;
    static const char fName[] = "ngexlTemporaryFileUnregister";

    /* Check the arguments */
    assert(fileName != NULL);

    /* Get the top of list */	
    prev = NULL;
    curr = ngexlTemporaryFileGetNext(NULL, log, error);
    while (curr != NULL) {
	if (strcmp(&curr->ngtf_fileName[0], fileName) == 0) {
	    /* Found */
	    goto found;
	}
	prev = curr;
	curr = ngexlTemporaryFileGetNext(curr, log, error);
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s: Temporary File is not found.\n", fileName); 

    /* Failed */
    return 0;

    /* Found */
found:
    /* Unregister the Temporary File */
    if (curr == ngexlSignalManager.ngsm_tmpFile_head)
	ngexlSignalManager.ngsm_tmpFile_head
	    = (ngexlTemporaryFile_t *)curr->ngtf_next;
    if (curr == ngexlSignalManager.ngsm_tmpFile_tail)
	ngexlSignalManager.ngsm_tmpFile_tail = prev;
    if (prev != NULL)
	prev->ngtf_next = curr->ngtf_next;
    curr->ngtf_next = NULL;

    /* Finalize */
    result = ngexlTemporaryFileFinalize(curr, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the Temporary File.\n"); 
	return 0;
    }

    /* Deallocate */
    result = ngexlTemporaryFileFree(curr, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the storage for Temporary File.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Temporary file: Allocate.
 */
static ngexlTemporaryFile_t *
ngexlTemporaryFileAllocate(
    char *fileName,
    ngLog_t *log,
    int *error)
{
    ngexlTemporaryFile_t *tmpFile;
    static const char fName[] = "ngexlTemporaryFileAllocate";

    /* Check the arguments */
    assert(fileName != NULL);
    assert(fileName[0] != '\0');

    /* Allocate */
    tmpFile = ngiCalloc(
	sizeof (ngexlTemporaryFile_t) - sizeof (tmpFile->ngtf_fileName)
	+ strlen(fileName) + 1, sizeof(char), log, error);
    if (tmpFile == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Temporary File.\n"); 
	return NULL;
    }

    /* Success */
    return tmpFile;
}

/**
 * Temporary file: Deallocate.
 */
static int
ngexlTemporaryFileFree(
    ngexlTemporaryFile_t *tmpFile,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(tmpFile != NULL);

    /* Deallocate */
    ngiFree(tmpFile, log, error);

    /* Success */
    return 1;
}

/**
 * Temporary file: Initialize.
 */
static int
ngexlTemporaryFileInitialize(
    ngexlTemporaryFile_t *tmpFile,
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
ngexlTemporaryFileFinalize(
    ngexlTemporaryFile_t *tmpFile,
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
static ngexlTemporaryFile_t *
ngexlTemporaryFileGetNext(
    ngexlTemporaryFile_t *current,
    ngLog_t *log,
    int *error)
{
    if (current == NULL) {
        /* Return the head */
        if (ngexlSignalManager.ngsm_tmpFile_head != NULL) {
            assert(ngexlSignalManager.ngsm_tmpFile_tail != NULL);
            return ngexlSignalManager.ngsm_tmpFile_head;
        }
    } else {
        /* Return the next */
        if (current->ngtf_next != NULL) {
            assert(ngexlSignalManager.ngsm_tmpFile_head != NULL);
            assert(ngexlSignalManager.ngsm_tmpFile_tail != NULL);
            return (ngexlTemporaryFile_t *)current->ngtf_next;
        }
    }

    /* No data */
    return NULL;
}

/**
 * Temporary File: Delete all files.
 */
static void
ngexlTemporaryFileDeleteAll(void)
{
    ngexlTemporaryFile_t *tmpFile;

    /* Is not initialized? */
    if (ngexlSignalManager.ngsm_initialized == 0) {
	/* Success */
	return;
    }

    /* Get the top of list */	
    tmpFile = ngexlSignalManager.ngsm_tmpFile_head;
    while (tmpFile != NULL) {
    	/* Check the data */
	assert(tmpFile->ngtf_fileName[0] != '\0');

	/* Unlink the Temporary File */
	unlink(&tmpFile->ngtf_fileName[0]);

	/* Get the next */
	tmpFile = (ngexlTemporaryFile_t *)tmpFile->ngtf_next;
    }
}

static int
ngexlSignalManagerMutexLock(ngLog_t *log, int *error)
{
#ifdef NG_PTHREAD
    int result;
    static const char fName[] = "ngexlSignalManagerMutexLock";

    /* Lock the mutex */
    result = pthread_mutex_lock(&ngexlSignalMutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "pthread_mutex_lock failed: %s.\n", strerror(result)); 
        /* Failed */
        return 0;
    }
#endif /* NG_PTHREAD */
    /* Success */
    return 1;
}
static int
ngexlSignalManagerMutexUnlock(ngLog_t *log, int *error)
{
#ifdef NG_PTHREAD
    int result;
    static const char fName[] = "ngexlSignalManagerMutexUnlock";

    /* Unlock the mutex */
    result = pthread_mutex_unlock(&ngexlSignalMutex);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_THREAD);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "pthread_mutex_unlock failed: %s.\n", strerror(result)); 
        /* Failed */
        return 0;
    }
#endif /* NG_PTHREAD */
    /* Success */
    return 1;
}
