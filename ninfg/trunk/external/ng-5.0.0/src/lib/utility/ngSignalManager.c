/*
 * $RCSfile: ngSignalManager.c,v $ $Revision: 1.9 $ $Date: 2008/03/06 11:39:18 $
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
 * Module of Signal Manager of Ninf-G Client and Executable.
 */

#include "ngInternal.h"

NGI_RCSID_EMBED("$RCSfile: ngSignalManager.c,v $ $Revision: 1.9 $ $Date: 2008/03/06 11:39:18 $")

#ifdef NGI_SIGNAL_HANDLING_ENABLED

/* How to handle signals */
#define NGL_SIGNAL_HANDLING_PTHREAD   1
#define NGL_SIGNAL_HANDLING_NONTHREAD 2

#ifdef NG_PTHREAD
#  define NGL_SIGNAL_HANDLING NGL_SIGNAL_HANDLING_PTHREAD
#else /* NG_PTHREAD */
#  define NGL_SIGNAL_HANDLING NGL_SIGNAL_HANDLING_NONTHREAD
#endif /* NG_PTHREAD */

#define NGL_SIGNAL_ACTION_MAX_NEST_LEVEL 2
#define NGL_SIGNAL_NAME_MAX 1024

/**
 * Data for managing the Signal Manager.
 */
typedef enum nglSignalHandlingType_e {
    NGL_SIGNAL_HANDLING_TYPE_NONE,
    NGL_SIGNAL_HANDLING_TYPE_PTHREAD,
    NGL_SIGNAL_HANDLING_TYPE_NONTHREAD,
    NGL_SIGNAL_HANDLING_TYPE_NOMORE
} nglSignalHandlingType_t;

typedef struct nglSignalActionLevel_s {
    int ngsal_ninfgHandlerRegistered;
    void (*ngsal_ninfgHandler)(int);

    int ngsal_userHandlerRegistered;
    void (*ngsal_userHandler)(int);

} nglSignalActionLevel_t;

typedef struct nglSignalAction_s {
    struct nglSignalAction_s *ngsa_next;

    int ngsa_signalNumber;
    nglSignalActionLevel_t ngsa_level[NGL_SIGNAL_ACTION_MAX_NEST_LEVEL];

    int ngsa_registered;
    nglSignalHandlingType_t ngsa_registerType;

    int ngsa_oldActionStored;
    struct sigaction ngsa_oldAction;
} nglSignalAction_t;

typedef struct nglSignalManager_s {
    int ngsm_currentID;

    ngLog_t *ngsm_log;

    int ngsm_handlingSIGTSTP;
    nglSignalHandlingType_t ngsm_signalType;

    int ngsm_initialized;
    int ngsm_started;

    int ngsm_oldMaskStored;
    sigset_t ngsm_oldMask;
    sigset_t ngsm_curMask;

    nglSignalAction_t *ngsm_actions;

#ifdef NG_PTHREAD
    pthread_t ngsm_thread;
#endif /* NG_PTHREAD */
    int ngsm_threadContinue;
    int ngsm_threadWorking;
} nglSignalManager_t;

static int nglSignalManagerInitialized = 0;
static nglSignalManager_t nglSignalManager;

#ifdef NG_PTHREAD
static ngiMutex_t nglSignalManagerMutex = NGI_MUTEX_INITIALIZER;
static ngiCond_t nglSignalManagerCond = NGI_COND_INITIALIZER;
#endif /* NG_PTHREAD */

typedef enum nglSignalDefaultActionBehavior_e {
    NGL_SIGNAL_DEFAULT_ACTION_NONE,
    NGL_SIGNAL_DEFAULT_ACTION_EXIT,
    NGL_SIGNAL_DEFAULT_ACTION_CORE,
    NGL_SIGNAL_DEFAULT_ACTION_STOP,
    NGL_SIGNAL_DEFAULT_ACTION_IGNORE,
    NGL_SIGNAL_DEFAULT_ACTION_NOMORE
} nglSignalDefaultActionBehavior_t;

typedef struct nglSignalDefaultAction_s {
    int ngsda_number;
    nglSignalDefaultActionBehavior_t ngsda_behavior;
} nglSignalDefaultAction_t;

/* Default signal behavior, Not all signals can be treated. */
static nglSignalDefaultAction_t nglSignalDefaultActions[] =
{
#ifdef SIGHUP
    {SIGHUP,     NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGINT
    {SIGINT,     NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGQUIT
    {SIGQUIT,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGILL
    {SIGILL,     NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGABRT
    {SIGABRT,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGFPE
    {SIGFPE,     NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGKILL
    {SIGKILL,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGSEGV
    {SIGSEGV,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGPIPE
    {SIGPIPE,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGALRM
    {SIGALRM,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGTERM
    {SIGTERM,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGUSR1
    {SIGUSR1,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGUSR2
    {SIGUSR2,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGCHLD
    {SIGCHLD,    NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGCONT
    {SIGCONT,    NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGSTOP
    {SIGSTOP,    NGL_SIGNAL_DEFAULT_ACTION_STOP},
#endif
#ifdef SIGTSTP
    {SIGTSTP,    NGL_SIGNAL_DEFAULT_ACTION_STOP},
#endif
#ifdef SIGTTIN
    {SIGTTIN,    NGL_SIGNAL_DEFAULT_ACTION_STOP},
#endif
#ifdef SIGTTOU
    {SIGTTOU,    NGL_SIGNAL_DEFAULT_ACTION_STOP},
#endif
#ifdef SIGBUS
    {SIGBUS,     NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGPOLL
    {SIGPOLL,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGPROF
    {SIGPROF,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGSYS
    {SIGSYS,     NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGTRAP
    {SIGTRAP,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGURG
    {SIGURG,     NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGVTALRM
    {SIGVTALRM,  NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGXCPU
    {SIGXCPU,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGXFSZ
    {SIGXFSZ,    NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGIOT
    {SIGIOT,     NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGEMT
    {SIGEMT,     NGL_SIGNAL_DEFAULT_ACTION_CORE},
#endif
#ifdef SIGSTKFLT
    {SIGSTKFLT,  NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGIO
    {SIGIO,      NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGCLD
    {SIGCLD,     NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGPWR
    {SIGPWR,     NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGINFO
    {SIGINFO,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGLOST
    {SIGLOST,    NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGWINCH
    {SIGWINCH,   NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGUNUSED
    {SIGUNUSED,  NGL_SIGNAL_DEFAULT_ACTION_EXIT},
#endif
#ifdef SIGWAITING
    {SIGWAITING, NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGLWP
    {SIGLWP,     NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGFREEZE
    {SIGFREEZE,  NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
#ifdef SIGTHAW
    {SIGTHAW,    NGL_SIGNAL_DEFAULT_ACTION_IGNORE},
#endif
    {0, NGL_SIGNAL_DEFAULT_ACTION_NONE},
};

static int nglSignalsRefusedByThread[] = {
    SIGKILL,
#ifdef SIGSEGV
    SIGSEGV,
#endif
#ifdef SIGABRT
    SIGABRT,
#endif
#ifdef SIGBUS
    SIGBUS,
#endif
#ifdef SIGFPE
    SIGFPE,
#endif
#ifdef SIGILL
    SIGILL,
#endif
#ifdef SIGIOT
    SIGIOT,
#endif
#ifdef SIGPIPE
    SIGPIPE,
#endif
#ifdef SIGEMT
    SIGEMT,
#endif
#ifdef SIGSYS
    SIGSYS,
#endif
#ifdef SIGTRAP
    SIGTRAP,
#endif
#ifdef SIGSTOP
    SIGSTOP,
#endif
#ifdef SIGCONT
    SIGCONT,
#endif
#ifdef SIGWAITING
    SIGWAITING,
#endif
};

typedef struct nglSignalNames_s {
    int ngsn_valid;
    int ngsn_sigNo;
    char *ngsn_sigName;
} nglSignalNames_t;

#define NGL_SIGNAL_NAME_ADD(sig) \
    {1, (sig), ("" # sig)}

/**
 * We do not assure all these signals works.
 * Just listup the signal names
 */
nglSignalNames_t nglSignalNameTable[NGL_SIGNAL_NAME_MAX] =
{
#ifdef SIGHUP
    NGL_SIGNAL_NAME_ADD(SIGHUP),
#endif
#ifdef SIGINT
    NGL_SIGNAL_NAME_ADD(SIGINT),
#endif
#ifdef SIGQUIT
    NGL_SIGNAL_NAME_ADD(SIGQUIT),
#endif
#ifdef SIGILL
    NGL_SIGNAL_NAME_ADD(SIGILL),
#endif
#ifdef SIGABRT
    NGL_SIGNAL_NAME_ADD(SIGABRT),
#endif
#ifdef SIGFPE
    NGL_SIGNAL_NAME_ADD(SIGFPE),
#endif
#ifdef SIGKILL
    NGL_SIGNAL_NAME_ADD(SIGKILL),
#endif
#ifdef SIGSEGV
    NGL_SIGNAL_NAME_ADD(SIGSEGV),
#endif
#ifdef SIGPIPE
    NGL_SIGNAL_NAME_ADD(SIGPIPE),
#endif
#ifdef SIGALRM
    NGL_SIGNAL_NAME_ADD(SIGALRM),
#endif
#ifdef SIGTERM
    NGL_SIGNAL_NAME_ADD(SIGTERM),
#endif
#ifdef SIGUSR1
    NGL_SIGNAL_NAME_ADD(SIGUSR1),
#endif
#ifdef SIGUSR2
    NGL_SIGNAL_NAME_ADD(SIGUSR2),
#endif
#ifdef SIGCHLD
    NGL_SIGNAL_NAME_ADD(SIGCHLD),
#endif
#ifdef SIGCONT
    NGL_SIGNAL_NAME_ADD(SIGCONT),
#endif
#ifdef SIGSTOP
    NGL_SIGNAL_NAME_ADD(SIGSTOP),
#endif
#ifdef SIGTSTP
    NGL_SIGNAL_NAME_ADD(SIGTSTP),
#endif
#ifdef SIGTTIN
    NGL_SIGNAL_NAME_ADD(SIGTTIN),
#endif
#ifdef SIGTTOU
    NGL_SIGNAL_NAME_ADD(SIGTTOU),
#endif
#ifdef SIGBUS
    NGL_SIGNAL_NAME_ADD(SIGBUS),
#endif
#ifdef SIGPOLL
    NGL_SIGNAL_NAME_ADD(SIGPOLL),
#endif
#ifdef SIGPROF
    NGL_SIGNAL_NAME_ADD(SIGPROF),
#endif
#ifdef SIGSYS
    NGL_SIGNAL_NAME_ADD(SIGSYS),
#endif
#ifdef SIGTRAP
    NGL_SIGNAL_NAME_ADD(SIGTRAP),
#endif
#ifdef SIGURG
    NGL_SIGNAL_NAME_ADD(SIGURG),
#endif
#ifdef SIGVTALRM
    NGL_SIGNAL_NAME_ADD(SIGVTALRM),
#endif
#ifdef SIGXCPU
    NGL_SIGNAL_NAME_ADD(SIGXCPU),
#endif
#ifdef SIGXFSZ
    NGL_SIGNAL_NAME_ADD(SIGXFSZ),
#endif
#ifdef SIGIOT
    NGL_SIGNAL_NAME_ADD(SIGIOT),
#endif
#ifdef SIGEMT
    NGL_SIGNAL_NAME_ADD(SIGEMT),
#endif
#ifdef SIGSTKFLT
    NGL_SIGNAL_NAME_ADD(SIGSTKFLT),
#endif
#ifdef SIGIO
    NGL_SIGNAL_NAME_ADD(SIGIO),
#endif
#ifdef SIGCLD
    NGL_SIGNAL_NAME_ADD(SIGCLD),
#endif
#ifdef SIGPWR
    NGL_SIGNAL_NAME_ADD(SIGPWR),
#endif
#ifdef SIGINFO
    NGL_SIGNAL_NAME_ADD(SIGINFO),
#endif
#ifdef SIGLOST
    NGL_SIGNAL_NAME_ADD(SIGLOST),
#endif
#ifdef SIGWINCH
    NGL_SIGNAL_NAME_ADD(SIGWINCH),
#endif
#ifdef SIGUNUSED
    NGL_SIGNAL_NAME_ADD(SIGUNUSED),
#endif
#ifdef SIGWAITING
    NGL_SIGNAL_NAME_ADD(SIGWAITING),
#endif
#ifdef SIGLWP
    NGL_SIGNAL_NAME_ADD(SIGLWP),
#endif
#ifdef SIGFREEZE
    NGL_SIGNAL_NAME_ADD(SIGFREEZE),
#endif
#ifdef SIGTHAW
    NGL_SIGNAL_NAME_ADD(SIGTHAW),
#endif
    {0, 0, NULL},
};

#undef NGL_SIGNAL_NAME_ADD


/**
 * Prototype declaration of internal functions.
 */
static int nglSignalManagerInitializeThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerInitializeNonThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static void nglSignalManagerInitializeMember(
    nglSignalManager_t *sigMng);

static char *nglSignalManagerSignalNameGet(
    int sigNo);

static int nglSignalManagerFinalizeThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerFinalizeNonThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);

static int nglSignalManagerStartThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerStartNonThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);

static int nglSignalManagerStopThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerStopNonThread(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);

static void nglSignalManagerSignalHandlerWorkForSIGTSTP(int sigNo);

static int nglSignalManagerSignalActionFind(
    nglSignalManager_t *sigMng, int sigNo, nglSignalAction_t **action,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionInitialize(
    nglSignalAction_t *action, int sigNo, ngLog_t *log, int *error);
static int nglSignalManagerSignalActionFinalize(
    nglSignalAction_t *action, ngLog_t *log, int *error);
static void nglSignalManagerSignalActionInitializeMember(
    nglSignalAction_t *action);
static int nglSignalManagerSignalActionRegister(
    nglSignalManager_t *sigMng, nglSignalAction_t *action,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionUnregister(
    nglSignalManager_t *sigMng, nglSignalAction_t *action,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionListBlock(
    int how, nglSignalManager_t *sigMng, nglSignalAction_t *newAction,
    sigset_t *argNewSet, sigset_t *argOldSet,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionIsRegisterRequired(
    nglSignalAction_t *action, int *requireRegister,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionStart(
    nglSignalManager_t *sigMng, nglSignalAction_t *action,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionStop(
    nglSignalManager_t *sigMng, nglSignalAction_t *action,
    ngLog_t *log, int *error);
static int nglSignalManagerSignalActionStoreOldActions(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);

static int nglSignalManagerSignalHandlerThreadCreate(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerSignalHandlerThreadDestroy(
    nglSignalManager_t *sigMng, ngLog_t *log, int *error);
static int nglSignalManagerSignalHandlerSigactionRegister(
    int sigNo, void (*handler)(int), ngLog_t *log, int *error);
static int nglSignalManagerSignalHandlerSigactionUnregister(
    int sigNo, struct sigaction *oldAction, int oldActionValid,
    ngLog_t *log, int *error);

#ifdef NG_PTHREAD
static void *nglSignalManagerSignalHandlerThreadSigwaitThread(void *arg);
#endif /* NG_PTHREAD */
static void nglSignalManagerSignalHandlerSigaction(int sigNo);
static int nglSignalManagerSignalHandlerCommon(
    nglSignalAction_t *action, int *performed,
    ngLog_t *log, int *error);

static int nglSignalManagerDefaultActionTypeGet(
    int signalNumber, nglSignalDefaultActionBehavior_t *behavior,
    ngLog_t *log, int *error);

static int nglSignalManagerMutexLock(ngLog_t *log, int *error);
static int nglSignalManagerMutexUnlock(ngLog_t *log, int *error);
static int nglSignalManagerPthreadSigmask(
    int how, sigset_t *newMask, sigset_t *oldMask,
    ngLog_t *log, int *error);


/**
 * SignalManager: Initialize.
 */
int
ngiSignalManagerInitialize(
    int *id,
    ngLog_t *log,
    int *error)
{
    nglSignalManager_t *sigMng;
    int mutexLocked, result, sigTSTP;
    static const char fName[] = "ngiSignalManagerInitialize";

    /* Check the arguments */
    assert(id != NULL);

    mutexLocked = 0;
    sigMng = &nglSignalManager;

    sigTSTP = 0;

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Is Signal Manager already initialized? */
    if (nglSignalManagerInitialized != 0) {
        assert(sigMng->ngsm_initialized == 1);

        /* Already initialized */
        sigMng->ngsm_currentID++;

        if (sigMng->ngsm_currentID >=
            NGL_SIGNAL_ACTION_MAX_NEST_LEVEL) {

            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Signal Manager ID is too large (%d >= %d).\n",
                sigMng->ngsm_currentID, NGL_SIGNAL_ACTION_MAX_NEST_LEVEL); 
            goto error;
        }

        *id = sigMng->ngsm_currentID;

        /* Success */
        goto success;
    }

    /* Initialize the members */
    nglSignalManagerInitializeMember(sigMng);

    sigMng->ngsm_currentID = 0;
    *id = sigMng->ngsm_currentID;

    /* Set handling type */
    sigMng->ngsm_signalType = NGL_SIGNAL_HANDLING_TYPE_NONE;

#if NGL_SIGNAL_HANDLING == NGL_SIGNAL_HANDLING_PTHREAD
    sigMng->ngsm_signalType = NGL_SIGNAL_HANDLING_TYPE_PTHREAD;

#elif NGL_SIGNAL_HANDLING == NGL_SIGNAL_HANDLING_NONTHREAD
    sigMng->ngsm_signalType = NGL_SIGNAL_HANDLING_TYPE_NONTHREAD;

#endif /* NGL_SIGNAL_HANDLING */
    assert(sigMng->ngsm_signalType != NGL_SIGNAL_HANDLING_TYPE_NONE);

    sigMng->ngsm_handlingSIGTSTP = 0;
#ifdef NGL_HANDLING_SIGTSTP
    sigMng->ngsm_handlingSIGTSTP = 1;
#endif /* NGL_HANDLING_SIGTSTP */

    /* Process each types */
    if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {

        result = nglSignalManagerInitializeThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Signal Manager for Thread.\n"); 
            goto error;
        }


    } else if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {

        result = nglSignalManagerInitializeNonThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Signal Manager for Non Thread.\n"); 
            goto error;
        }
    }

    sigMng->ngsm_initialized = 1;
    nglSignalManagerInitialized = 1;

success:

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }

    /* Work for SIGTSTP. */
    if ((sigMng->ngsm_handlingSIGTSTP != 0) &&
        (sigMng->ngsm_currentID == 0)) {
        sigTSTP = SIGTSTP;
        result = ngiSignalManagerSignalHandlerRegister(
            sigMng->ngsm_currentID,
            NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG,
            &sigTSTP, 1,
            nglSignalManagerSignalHandlerWorkForSIGTSTP,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the SIGTSTP.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Initialize Thread.
 */
static int
nglSignalManagerInitializeThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, *table, size;
    static const char fName[] = "nglSignalManagerInitializeThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    result = sigfillset(&sigMng->ngsm_curMask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigfillset()", strerror(errno)); 
        return 0;
    }

    table = nglSignalsRefusedByThread;
    size = NGI_NELEMENTS(nglSignalsRefusedByThread);

    for (i = 0; i < size; i++) {
        result = sigdelset(&sigMng->ngsm_curMask, table[i]);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "%s for %d failed: %s\n",
                "sigdelset()",table[i], strerror(errno)); 
            return 0;
        }
    }

    /* sigmask */
    result = nglSignalManagerPthreadSigmask(
        SIG_SETMASK, &sigMng->ngsm_curMask, &sigMng->ngsm_oldMask,
        log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "sigmask failed.\n"); 
        return 0;
    }
    sigMng->ngsm_oldMaskStored = 1;

    /* Success */
    return 1;
}

/**
 * SignalManager: Initialize NonThread.
 */
static int
nglSignalManagerInitializeNonThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(sigMng != NULL);

    /* Do nothing */

    /* Success */
    return 1;
}

/**
 * SignalManager: Finalize.
 */
int
ngiSignalManagerFinalize(
    int id,
    ngLog_t *log,
    int *error)
{
    int mutexLocked, result;
    nglSignalAction_t *action;
    nglSignalManager_t *sigMng;
    static const char fName[] = "ngiSignalManagerFinalize";

    mutexLocked = 0;
    sigMng = &nglSignalManager;

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Is Signal Manager initialized? */
    if ((nglSignalManagerInitialized == 0) ||
        (sigMng->ngsm_initialized == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager was not initialized.\n"); 
        goto error;
    }

    /* Is id not 0? */
    if (id > 0) {
        if (sigMng->ngsm_currentID <= 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Signal Manager current ID %d is invalid.\n",
                sigMng->ngsm_currentID); 
            goto error;
        }

        sigMng->ngsm_currentID--;

        goto success;
    }

    sigMng->ngsm_initialized = 0;

    /* Process each types */
    if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {

        result = nglSignalManagerFinalizeThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the Signal Manager for Thread.\n"); 
            goto error;
        }

    } else if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {

        result = nglSignalManagerFinalizeNonThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the Signal Manager for Non Thread.\n"); 
            goto error;
        }
    }

    while (sigMng->ngsm_actions != NULL) {
        action = sigMng->ngsm_actions;

        /* Unregister */
        result = nglSignalManagerSignalActionUnregister(
            sigMng, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the Signal Action.\n"); 
                goto error;
        }

        /* Finalize */
        result = nglSignalManagerSignalActionFinalize(
            action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Signal Action.\n"); 
                goto error;
        }

        /* Deallocate */
        result = NGI_DEALLOCATE(nglSignalAction_t, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't deallocate the Signal Action.\n"); 
                goto error;
        }
    }

    /* Initialize the members */
    nglSignalManagerInitializeMember(sigMng);

    nglSignalManagerInitialized = 0;

success:

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Finalize Thread.
 */
static int
nglSignalManagerFinalizeThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglSignalManagerFinalizeThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    /* sigmask */
    if (sigMng->ngsm_oldMaskStored != 0) {
        result = nglSignalManagerPthreadSigmask(
            SIG_SETMASK, &sigMng->ngsm_oldMask, NULL,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "sigmask failed.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Finalize NonThread.
 */
static int
nglSignalManagerFinalizeNonThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(sigMng != NULL);

    /* Success */
    return 1;
}


/**
 * SignalManager: Start.
 */
int
ngiSignalManagerStart(
    int id,
    ngLog_t *log,
    int *error)
{
    nglSignalAction_t *action;
    nglSignalManager_t *sigMng;
    int result, mutexLocked, ignoreSIGPIPE;
    static const char fName[] = "ngiSignalManagerStart";

    mutexLocked = 0;
    sigMng = &nglSignalManager;
    action = NULL;

    ignoreSIGPIPE = 1;
#ifdef NGI_NO_IGNORE_SIGPIPE
    ignoreSIGPIPE = 0;
#endif /* NGI_NO_IGNORE_SIGPIPE */

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Start the Signal Manager by id %d.\n", id); 

    /* Is Signal Manager initialized? */
    if ((nglSignalManagerInitialized == 0) ||
        (sigMng->ngsm_initialized == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager is not initialized.\n"); 
        goto error;
    }

    /* Is id valid? */
    if (id != sigMng->ngsm_currentID) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "ID %d do not match the current Signal Manager ID %d.\n",
            id, sigMng->ngsm_currentID); 
        goto error;
    }

    /* Is id not zero? */
    if (id > 0) {
        if (sigMng->ngsm_currentID <= 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Signal Manager current ID %d is invalid.\n",
                sigMng->ngsm_currentID); 
            goto error;
        }

        if (sigMng->ngsm_started == 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Signal Manager current ID is %d and id is %d,"
                " but not started.\n", sigMng->ngsm_currentID, id); 
            goto error;
        }

        /* Success */
        goto success;
    }

    /* Is already started? */
    if (sigMng->ngsm_started != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager already started.\n"); 
        goto error;
    }

    /* Store old actions */
    result = nglSignalManagerSignalActionStoreOldActions(
        sigMng, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't store old sigactions.\n"); 
        goto error;
    }

    /* Ignore SIGPIPE */
    if (ignoreSIGPIPE != 0) {
        action = NULL;
        result = nglSignalManagerSignalActionFind(
            sigMng, SIGPIPE, &action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't find the Signal Action.\n"); 
                goto error;
        }

        if (action == NULL) {
            ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Set the signal SIGPIPE to SIG_IGN.\n"); 

            result = nglSignalManagerSignalHandlerSigactionRegister(
                SIGPIPE, SIG_IGN, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register the signal handler.\n"); 
                goto error;
            }
        }
    }

    /* Process each types */
    if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {

        result = nglSignalManagerStartThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't start the Signal Manager for Thread.\n"); 
            goto error;
        }

    } else if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {

        result = nglSignalManagerStartNonThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't start the Signal Manager for Non Thread.\n"); 
            goto error;
        }
    }

    sigMng->ngsm_started = 1;

success:

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }


    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Start Thread.
 */
static int
nglSignalManagerStartThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    struct sigaction sigact;
    nglSignalAction_t *action;
    static const char fName[] = "nglSignalManagerStartThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    /* Set signal handler(when ignore) */
    /* If signal handler is SIG_IGN, sigwait() doesn't work */
    sigact.sa_handler = SIG_DFL;
    sigact.sa_flags = 0;
    result = sigemptyset(&sigact.sa_mask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigemptyset()", strerror(errno)); 
        return 0;
    }

    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {
        if ((action->ngsa_oldActionStored != 0) &&
            ((action->ngsa_oldAction.sa_flags & SA_SIGINFO) == 0) &&
            (action->ngsa_oldAction.sa_handler == SIG_IGN)) {

            result = sigaction(
                action->ngsa_signalNumber, &sigact, NULL);
            if (result != 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register the SIG_DFL.\n"); 
                return 0;
            }
        }
    }

    /* Register to system. */
    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {

        result = nglSignalManagerSignalActionStart(
            sigMng, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the signal handler.\n"); 
            return 0;
        }
    }

    /* Create the thread */
    result = nglSignalManagerSignalHandlerThreadCreate(
        sigMng, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't create the sigwait thread.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Start NonThread.
 */
static int
nglSignalManagerStartNonThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    nglSignalAction_t *action;
    static const char fName[] = "nglSignalManagerStartNonThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {

        result = nglSignalManagerSignalActionStart(
            sigMng, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the signal handler.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}


/**
 * SignalManager: Stop.
 */
int
ngiSignalManagerStop(
    int id,
    ngLog_t *log,
    int *error)
{
    int mutexLocked, result;
    nglSignalManager_t *sigMng;
    static const char fName[] = "ngiSignalManagerStop";

    mutexLocked = 0;
    sigMng = &nglSignalManager;

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Stop the Signal Manager by id %d.\n", id); 

    /* Is Signal Manager initialized? */
    if ((nglSignalManagerInitialized == 0) ||
        (sigMng->ngsm_initialized == 0)) {
        /* Not initialized yet */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager was not initialized.\n"); 
        goto error;
    }

    /* Is Signal Manager started? */
    if (sigMng->ngsm_started == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager is not started.\n"); 
        goto error;
    }

    /* Is id valid? */
    if (id != sigMng->ngsm_currentID) {
        /* It will happen on Cascading RPC call, */
        /* and the user forgot to grpc_finalize(). */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "ID %d do not match the current Signal Manager ID %d.\n",
            id, sigMng->ngsm_currentID); 
        /* Not return */
    }

    /* Is id not 0? */
    if (id > 0) {
        if (sigMng->ngsm_currentID <= 0) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Signal Manager current ID %d is invalid.\n",
                sigMng->ngsm_currentID); 
            goto error;
        }

        /* Do nothing */
        goto success;
    }

    sigMng->ngsm_started = 0;

    /* Process each types */
    if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {

        result = nglSignalManagerStopThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't stop the Signal Manager for Thread.\n"); 
            goto error;
        }

    } else if (sigMng->ngsm_signalType == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {

        result = nglSignalManagerStopNonThread(sigMng, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't stop the Signal Manager for Non Thread.\n"); 
            goto error;
        }
    }

success:

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Stop Thread.
 */
static int
nglSignalManagerStopThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    nglSignalAction_t *action;
    static const char fName[] = "nglSignalManagerStopThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    /* Destroy the thread */
    result = nglSignalManagerSignalHandlerThreadDestroy(
        sigMng, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destroy the sigwait thread.\n"); 
        return 0;
    }

    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {

        result = nglSignalManagerSignalActionStop(
            sigMng, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the signal handler.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Stop NonThread.
 */
static int
nglSignalManagerStopNonThread(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    int result;
    nglSignalAction_t *action;
    static const char fName[] = "nglSignalManagerStopNonThread";

    /* Check the arguments */
    assert(sigMng != NULL);

    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {

        result = nglSignalManagerSignalActionStop(
            sigMng, action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the signal handler.\n"); 
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Initialize the member of Signal Manager.
 */
static void
nglSignalManagerInitializeMember(
    nglSignalManager_t *sigMng)
{
    /* Check the arguments */
    assert(sigMng != NULL);

    sigMng->ngsm_currentID = 0;
    sigMng->ngsm_log = NULL;
    sigMng->ngsm_handlingSIGTSTP = 0;
    sigMng->ngsm_initialized = 0;
    sigMng->ngsm_started = 0;

    sigMng->ngsm_threadContinue = 0;
    sigMng->ngsm_threadWorking = 0;

    sigMng->ngsm_actions = NULL;
}

/**
 * SignalManager: Set the log.
 */
int
ngiSignalManagerLogSet(
   int id,
   ngLog_t *target,
   ngLog_t *log,
   int *error)
{
    nglSignalManager_t *sigMng;

    sigMng = &nglSignalManager;

    if (id == 0) {
        sigMng->ngsm_log = target;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Get signal name strings.
 * Note: signal names are static constants.
 */
int
ngiSignalManagerSignalNamesGet(
    char ***signalNames,
    int **signalNumbers,
    int *nSignals,
    ngLog_t *log,
    int *error)
{
    int count, i;
    int *numTable;
    char **nameTable;
    static const char fName[] = "ngiSignalManagerSignalNamesGet";

    /* Check the arguments */
    assert(signalNames != NULL);
    assert(signalNumbers != NULL);
    assert(nSignals != NULL);

    *signalNames = NULL;
    *signalNumbers = NULL;
    *nSignals = 0;

    count = 0;
    nameTable = NULL;
    numTable = NULL;

    for (i = 0; nglSignalNameTable[i].ngsn_valid != 0; i++) {
        count++;
    }

    nameTable = (char **)ngiCalloc(sizeof(char *), count, log, error);
    if (nameTable == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal name table.\n"); 
        return 0;
    }

    numTable = (int *)ngiCalloc(sizeof(int), count, log, error);
    if (numTable == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal number table.\n"); 
        return 0;
    }

    for (i = 0; i < count; i++) {
        nameTable[i] = nglSignalNameTable[i].ngsn_sigName;
        numTable[i] = nglSignalNameTable[i].ngsn_sigNo;
    }

    *nSignals = count;
    *signalNames = nameTable;
    *signalNumbers = numTable;

    return 1;
}

/**
 * SignalManager: Destruct the signal name strings.
 */
int
ngiSignalManagerSignalNamesDestruct(
    char **signalNames,
    int *signalNumbers,
    int nSignals,
    ngLog_t *log,
    int *error)
{

    if (signalNames != NULL) {
        ngiFree(signalNames, log, error);
    }

    if (signalNumbers != NULL) {
        ngiFree(signalNumbers, log, error);
    }

    /* Do not free signal name strings because they are static */

    return 1;
}

/**
 * SignalManager: Get the Signal Name from number.
 */
static char *
nglSignalManagerSignalNameGet(
    int sigNo)
{
    nglSignalNames_t *table;
    int i;

    table = nglSignalNameTable;

    for (i = 0; table[i].ngsn_valid != 0; i++) {
        if (table[i].ngsn_sigNo == sigNo) {
            /* Found. */
            return table[i].ngsn_sigName;
        }
    }

    /* Not found */
    return "-";
}

/**
 * SignalManager: Signal Handler Register.
 */
int
ngiSignalManagerSignalHandlerRegister(
    int id,
    ngiSignalHandlerType_t type,
    int *signals,
    int nSignals,
    void (*handler)(int),
    ngLog_t *log,
    int *error)
{
    int i;
    int sigNo;
    int mutexLocked, result;
    nglSignalAction_t *action;
    nglSignalManager_t *sigMng;
    static const char fName[] = "ngiSignalManagerSignalHandlerRegister";

    sigMng = &nglSignalManager;
    action = NULL;
    sigNo = 0;
    mutexLocked = 0;

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Is Signal Manager initialized? */
    if ((nglSignalManagerInitialized == 0) ||
        (sigMng->ngsm_initialized == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Signal Manager is not initialized.\n"); 
        goto error;
    }

    /* Is id valid? */
    if (id > sigMng->ngsm_currentID) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "ID %d do not match the current Signal Manager ID %d.\n",
            id, sigMng->ngsm_currentID); 
        goto error;
    }

    for (i = 0; i < nSignals; i++) {
        sigNo = signals[i];

        /* Find the signal */
        action = NULL;
        result = nglSignalManagerSignalActionFind(
            sigMng, sigNo, &action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't find the Signal Action.\n"); 
                goto error;
        }

        if (action == NULL) {
            /* Allocate */
            action = NGI_ALLOCATE(nglSignalAction_t, log, error);
            if (action == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't allocate the Signal Action.\n"); 
                    goto error;
            }

            /* Initialize */
            result = nglSignalManagerSignalActionInitialize(
                action, sigNo, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't initialize the Signal Action.\n"); 
                    goto error;
            }

            /* Register */
            result = nglSignalManagerSignalActionRegister(
                sigMng, action, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register the Signal Action.\n"); 
                    goto error;
            }
        }

        assert(action != NULL);

        /* Register to Signal Manager */
        if (type == NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_NINFG) {
            action->ngsa_level[id].ngsal_ninfgHandler = handler;
            action->ngsa_level[id].ngsal_ninfgHandlerRegistered = 1;

        } else if (type == NGI_SIGNAL_MANAGER_SIGNAL_HANDLER_USER) {
            action->ngsa_level[id].ngsal_userHandler = handler;
            action->ngsa_level[id].ngsal_userHandlerRegistered = 1;

        } else {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Invalid register type %d for signal %d (%s).\n",
                type, sigNo, nglSignalManagerSignalNameGet(sigNo)); 
                goto error;
        }

        if (sigMng->ngsm_started != 0) {
            result = nglSignalManagerSignalActionStart(
                sigMng, action, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register the signal handler.\n"); 
                goto error;
            }
        }
    }

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Dummy Signal Handler for SIGTSTP.
 */
static void
nglSignalManagerSignalHandlerWorkForSIGTSTP(
    int sigNo)
{
    /* Do nothing */
}

/**
 * SignalManager: Find the signal action.
 * Lock the SignalManager before use this function.
 */
static int
nglSignalManagerSignalActionFind(
    nglSignalManager_t *sigMng,
    int sigNo,
    nglSignalAction_t **action,
    ngLog_t *log,
    int *error)
{
    nglSignalAction_t *cur;

    /* Check the arguments */
    assert(sigMng != NULL);
    assert(action != NULL);

    *action = NULL;

    cur = NULL;

    /* Find the signal entry */
    cur = sigMng->ngsm_actions;
    for (; cur != NULL; cur = cur->ngsa_next) {
        if (cur->ngsa_signalNumber == sigNo) {
            *action = cur;
            break;
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalAction: Initialize
 */
static int
nglSignalManagerSignalActionInitialize(
    nglSignalAction_t *action,
    int sigNo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(action != NULL);

    /* Initialize the members */
    nglSignalManagerSignalActionInitializeMember(action);

    action->ngsa_signalNumber = sigNo;

    /* Success */
    return 1;
}

/**
 * SignalAction: Finalize
 */
static int
nglSignalManagerSignalActionFinalize(
    nglSignalAction_t *action,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(action != NULL);

    /* Initialize the members */
    nglSignalManagerSignalActionInitializeMember(action);

    /* Success */
    return 1;
}

/**
 * SignalAction: Initialize the members.
 */
static void
nglSignalManagerSignalActionInitializeMember(
    nglSignalAction_t *action)
{
    int i;

    /* Check the arguments */
    assert(action != NULL);

    action->ngsa_next = NULL;
    action->ngsa_signalNumber = 0;

    for (i = 0; i < NGL_SIGNAL_ACTION_MAX_NEST_LEVEL; i++) {
        action->ngsa_level[i].ngsal_ninfgHandlerRegistered = 0;
        action->ngsa_level[i].ngsal_ninfgHandler = SIG_DFL;
        action->ngsa_level[i].ngsal_userHandlerRegistered = 0;
        action->ngsa_level[i].ngsal_userHandler = SIG_DFL;
    }

    memset(&action->ngsa_oldAction, 0, sizeof(action->ngsa_oldAction));
}

/**
 * SignalAction: Register
 * Lock the SignalManager before use this function.
 */
static int
nglSignalManagerSignalActionRegister(
    nglSignalManager_t *sigMng,
    nglSignalAction_t *action,
    ngLog_t *log,
    int *error)
{
    sigset_t oldSet;
    nglSignalAction_t *cur;
    int result, signalBlocked;
    static const char fName[] = "nglSignalManagerSignalActionRegister";

    /* Check the arguments */
    assert(sigMng != NULL);
    assert(action != NULL);

    signalBlocked = 0;

    /* Block the signal on NonThread */
    result = nglSignalManagerSignalActionListBlock(
        SIG_BLOCK, sigMng, action, NULL, &oldSet, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't block the Signal Action list.\n"); 
        goto error;
    }
    signalBlocked = 1;

    cur = NULL;

    if (sigMng->ngsm_actions == NULL) {
        /* Register to empty list */
        sigMng->ngsm_actions = action;

        /* Success */
        goto success;
    }

    /* Find the tail */
    cur = sigMng->ngsm_actions;
    while (cur->ngsa_next != NULL) {
        cur = cur->ngsa_next;
    }

    /* Register */
    cur->ngsa_next = action;

success:
    /* Unblock the signal on NonThread */
    signalBlocked = 0;
    result = nglSignalManagerSignalActionListBlock(
        SIG_SETMASK, sigMng, NULL, &oldSet, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unblock the Signal Action list.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unblock the signal on NonThread */
    if (signalBlocked != 0) {
        signalBlocked = 0;
        result = nglSignalManagerSignalActionListBlock(
            SIG_SETMASK, sigMng, NULL, &oldSet, NULL, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unblock the Signal Action list.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalAction: Unregister
 * Lock the SignalManager before use this function.
 */
static int
nglSignalManagerSignalActionUnregister(
    nglSignalManager_t *sigMng,
    nglSignalAction_t *action,
    ngLog_t *log,
    int *error)
{
    sigset_t oldSet;
    int result, signalBlocked, found;
    nglSignalAction_t *cur, **prevPtr;
    static const char fName[] = "nglSignalManagerSignalActionUnregister";

    /* Check the arguments */
    assert(sigMng != NULL);
    assert(action != NULL);

    signalBlocked = 0;

    /* Block the signal on NonThread */
    result = nglSignalManagerSignalActionListBlock(
        SIG_BLOCK, sigMng, NULL, NULL, &oldSet, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't block the Signal Action list.\n"); 
        goto error;
    }
    signalBlocked = 1;

    /* Delete the data from the list */
    found = 0;
    prevPtr = &sigMng->ngsm_actions;
    cur = sigMng->ngsm_actions;
    for (; cur != NULL; cur = cur->ngsa_next) {
        if (cur == action) {
            /* Found */
            *prevPtr = cur->ngsa_next;
            action->ngsa_next = NULL;
            found = 1;

            break;
        }

        /* set prev to current element */
        prevPtr = &cur->ngsa_next;
    }

    /* Unblock the signal on NonThread */
    signalBlocked = 0;
    result = nglSignalManagerSignalActionListBlock(
        SIG_SETMASK, sigMng, NULL, &oldSet, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unblock the Signal Action list.\n"); 
        goto error;
    }

    if (found == 0) {
        /* Not found */
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unregister the Signal Action.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unblock the signal on NonThread */
    if (signalBlocked != 0) {
        signalBlocked = 0;
        result = nglSignalManagerSignalActionListBlock(
            SIG_SETMASK, sigMng, NULL, &oldSet, NULL, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unblock the Signal Action list.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalAction: Block or Unblock the list.
 * If argNewSet is non-null, argNewSet are used to block signals.
 * In other case, list of signals on Signal Manager and
 * the newAction is blocked.
 * old signal mask are stored in argOldSet.
 */
static int
nglSignalManagerSignalActionListBlock(
    int how,
    nglSignalManager_t *sigMng,
    nglSignalAction_t *newAction,
    sigset_t *argNewSet,
    sigset_t *argOldSet,
    ngLog_t *log,
    int *error)
#ifndef NG_PTHREAD
{
    int result;
    nglSignalAction_t *cur;
    sigset_t createdNewSet, *usedNewSet;
    static const char fName[] = "nglSignalManagerSignalActionListBlock";

    /* Check the arguments */
    assert(sigMng != NULL);

    usedNewSet = NULL;

    if (argNewSet != NULL) {
        usedNewSet = argNewSet;
    } else {
        usedNewSet = &createdNewSet;

        result = sigemptyset(&createdNewSet);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "%s failed: %s\n", "sigemptyset()", strerror(errno)); 
            return 0;
        }
     
        if (newAction != NULL) {
            result = sigaddset(&createdNewSet, newAction->ngsa_signalNumber);
            if (result != 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "%s failed: %s\n", "sigaddset()", strerror(errno)); 
                return 0;
            }
        }
     
        cur = sigMng->ngsm_actions;
        for (; cur != NULL; cur = cur->ngsa_next) {
            result = sigaddset(&createdNewSet, cur->ngsa_signalNumber);
            if (result != 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "%s failed: %s\n", "sigaddset()", strerror(errno)); 
                return 0;
            }
        }
    }

    result = sigprocmask(how, usedNewSet, argOldSet);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigprocmask()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    /* Only effective for NonThread version */

    /* Do nothing */
    return 1;
}
#endif /* NG_PTHREAD */

/**
 * SignalAction: Is register required?
 */
static int
nglSignalManagerSignalActionIsRegisterRequired(
    nglSignalAction_t *action,
    int *requireRegister,
    ngLog_t *log,
    int *error)
{
    int i;
    nglSignalActionLevel_t *cur;

    /* Check the arguments */
    assert(action != NULL);
    assert(requireRegister != NULL);

    *requireRegister = 0;

    /* Is already registered? */
    if (action->ngsa_registered != 0) {
        *requireRegister = 0;

        return 1;
    }

    for (i = 0; i < NGL_SIGNAL_ACTION_MAX_NEST_LEVEL; i++) {
        cur = &(action->ngsa_level[i]);

        if (cur->ngsal_ninfgHandlerRegistered != 0) {
            *requireRegister = 1;
        }

        if (cur->ngsal_userHandlerRegistered != 0) {
            *requireRegister = 1;
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalAction: Start and register the sigaction to system.
 * This function just return if unnecessary.
 */
static int
nglSignalManagerSignalActionStart(
    nglSignalManager_t *sigMng,
    nglSignalAction_t *action,
    ngLog_t *log,
    int *error)
{
    nglSignalHandlingType_t type;
    int sigNo, requireRegister, result, found, size, i;
    static const char fName[] = "nglSignalManagerSignalActionStart";

    /* Check the arguments */
    assert(sigMng != NULL);
    assert(action != NULL);

    sigNo = action->ngsa_signalNumber;

    if (sigNo <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The signal %d is invalid.\n", sigNo); 
        return 0;
    }

    /* Does this signal require register? */
    requireRegister = 0;
    result = nglSignalManagerSignalActionIsRegisterRequired(
        action, &requireRegister, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't check the Signal Action.\n"); 
        return 0;
    }

    if (requireRegister == 0) {
        /* Success */
        return 1;
    }

    type = sigMng->ngsm_signalType;

    if (type == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {

        found = 0;
        size = NGI_NELEMENTS(nglSignalsRefusedByThread);
        for (i = 0; i < size; i++) {
            if (nglSignalsRefusedByThread[i] == sigNo) {
                found = 1;
                break;
            }
        }

        if (found != 0) {
            type = NGL_SIGNAL_HANDLING_TYPE_NONTHREAD;
        }
    }

    if (type == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {
        /* Register to Thread */
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The signal %d (%s) is registered with Thread.\n",
            sigNo, nglSignalManagerSignalNameGet(sigNo)); 

        /* Already treated */
        action->ngsa_registered = 1;
        action->ngsa_registerType = NGL_SIGNAL_HANDLING_TYPE_PTHREAD;

    } else if (type == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The signal %d (%s) is registered with system call.\n",
            sigNo, nglSignalManagerSignalNameGet(sigNo)); 

        result = nglSignalManagerSignalHandlerSigactionRegister(
            sigNo,
            nglSignalManagerSignalHandlerSigaction,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the signal handler.\n"); 
            return 0;
        }

        action->ngsa_registered = 1;
        action->ngsa_registerType = NGL_SIGNAL_HANDLING_TYPE_NONTHREAD;

    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid type %d for signal %d (%s).\n",
            type, sigNo, nglSignalManagerSignalNameGet(sigNo)); 
        return 0;
    }

    /* Success */
    return 1;
}
    

/**
 * SignalAction: Stop and unregister the sigaction from system.
 */
static int
nglSignalManagerSignalActionStop(
    nglSignalManager_t *sigMng,
    nglSignalAction_t *action,
    ngLog_t *log,
    int *error)
{
    int sigNo, result;
    nglSignalHandlingType_t type;
    static const char fName[] = "nglSignalManagerSignalActionStop";

    /* Check the arguments */
    assert(sigMng != NULL);
    assert(action != NULL);

    sigNo = action->ngsa_signalNumber;
    type = action->ngsa_registerType;

    if (action->ngsa_registered == 0) {
        /* Success */
        return 1;
    }

    if (type == NGL_SIGNAL_HANDLING_TYPE_PTHREAD) {
        /* No signal handler registered */

        /* Restore Old actions */
        result = nglSignalManagerSignalHandlerSigactionUnregister(
            sigNo, &action->ngsa_oldAction, action->ngsa_oldActionStored,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the signal handler %d.\n", sigNo); 
            return 0;
        }

        action->ngsa_registerType = NGL_SIGNAL_HANDLING_TYPE_NONE;
        action->ngsa_registered = 0;

    } else if (type == NGL_SIGNAL_HANDLING_TYPE_NONTHREAD) {
        result = nglSignalManagerSignalHandlerSigactionUnregister(
            sigNo, &action->ngsa_oldAction, action->ngsa_oldActionStored,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the signal handler %d.\n", sigNo); 
            return 0;
        }
        action->ngsa_registerType = NGL_SIGNAL_HANDLING_TYPE_NONE;
        action->ngsa_registered = 0;
    } else {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid type %d for signal %d (%s).\n",
            type, sigNo, nglSignalManagerSignalNameGet(sigNo)); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SignalAction: Store old sigactions.
 */
static int
nglSignalManagerSignalActionStoreOldActions(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
{
    struct sigaction *sigact;
    nglSignalAction_t *action;
    int result, requireRegister;
    static const char fName[] = "nglSignalManagerSignalActionStoreOldActions";

    /* Check the arguments */
    assert(sigMng != NULL);

    action = NULL;
    sigact = NULL;

    action = sigMng->ngsm_actions;
    for (; action != NULL; action = action->ngsa_next) {

        /* Does this signal require register? */
        result = nglSignalManagerSignalActionIsRegisterRequired(
            action, &requireRegister, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't check the Signal Action.\n"); 
            return 0;
        }
        if (requireRegister == 0) {
            continue;
        }

        result = sigaction(
            action->ngsa_signalNumber, NULL,
            &action->ngsa_oldAction);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "sigaction failed: %s\n", strerror(errno)); 
            /* Not return */
        }
        action->ngsa_oldActionStored = 1;

        sigact = &action->ngsa_oldAction;
        if (((sigact->sa_flags & SA_SIGINFO) != 0) ||
            (sigact->sa_handler != SIG_DFL)) {
            /* Print warning */
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Ninf-G overwrite your signal handler of signal %d (%s).\n",
                action->ngsa_signalNumber,
                nglSignalManagerSignalNameGet(action->ngsa_signalNumber)); 
        }
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Create the sigwait() thread.
 */
static int
nglSignalManagerSignalHandlerThreadCreate(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "nglSignalManagerSignalHandlerThreadCreate";

    /* Check the arguments */
    assert(sigMng != NULL);

    sigMng->ngsm_threadContinue = 1;

    /* Create the thread */
    result = pthread_create(
        &sigMng->ngsm_thread, NULL,
        nglSignalManagerSignalHandlerThreadSigwaitThread, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigemptyset()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    static const char fName[] = "nglSignalManagerSignalHandlerThreadCreate";

    /* Only effective for Pthread version. */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s not found.\n", "pthread_create()"); 

    /* Failed */
    return 0;
}
#endif /* NG_PTHREAD */

/**
 * SignalManager: Destroy the sigwait() thread.
 */
static int
nglSignalManagerSignalHandlerThreadDestroy(
    nglSignalManager_t *sigMng,
    ngLog_t *log,
    int *error)
#ifdef NG_PTHREAD
{
    int sigNo, result;
    static const char fName[] = "nglSignalManagerSignalHandlerThreadDestroy";

    /* Check the arguments */
    assert(sigMng != NULL);

    sigMng->ngsm_threadContinue = 0;

    if (sigMng->ngsm_threadWorking != 0) {

        sigNo = SIGHUP; /* sigwait thread catches it. */

        /* Kill the thread */
        result = pthread_kill(
            sigMng->ngsm_thread, sigNo);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "%s failed: %s\n", "pthread_kill()", strerror(errno)); 
            return 0;
        }

        /* Mutex is already locked by nglSignalManagerStop() */

        /* Wait the sigwait thread to finish */
        while (sigMng->ngsm_threadWorking != 0) {
            result = ngiCondWait(
                &nglSignalManagerCond, &nglSignalManagerMutex, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't wait condition.\n"); 
                return 0;
            }
        }
    }

    /* Kill the thread */
    result = pthread_join(sigMng->ngsm_thread, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "pthread_join()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    static const char fName[] = "nglSignalManagerSignalHandlerThreadDestroy";

    /* Only effective for Pthread version. */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s not found.\n", "pthread_kill()"); 

    /* Failed */
    return 0;
}
#endif /* NG_PTHREAD */

/**
 * SignalManager: Register the signal handler to sigaction().
 */
static int
nglSignalManagerSignalHandlerSigactionRegister(
    int sigNo,
    void (*handler)(int),
    ngLog_t *log,
    int *error)
{
    int result;
    struct sigaction sigact;
    static const char fName[] =
        "nglSignalManagerSignalHandlerSigactionRegister";

    sigact.sa_handler = handler;
    sigact.sa_flags = 0;

    result = sigemptyset(&sigact.sa_mask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigemptyset()", strerror(errno)); 
        return 0;
    }

    result = sigaction(sigNo, &sigact, NULL);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigaction()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Unregister the signal handler from sigaction().
 */
static int
nglSignalManagerSignalHandlerSigactionUnregister(
    int sigNo,
    struct sigaction *oldAction,
    int oldActionValid,
    ngLog_t *log,
    int *error)
{
    int result;
    struct sigaction *sigact, sigactEntity;
    static const char fName[] =
        "nglSignalManagerSignalHandlerSigactionUnregister";

    sigact = NULL;

    if (oldActionValid != 0) {
        sigact = oldAction;

    } else {
        sigactEntity.sa_handler = SIG_DFL;
        sigactEntity.sa_flags = 0;

        result = sigemptyset(&sigactEntity.sa_mask);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "%s failed: %s\n", "sigemptyset()", strerror(errno)); 
            return 0;
        }
        sigact = &sigactEntity;
    }

    result = sigaction(sigNo, sigact, NULL);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "sigaction()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}

#ifdef NG_PTHREAD
/**
 * SignalManager: Sigwait thread for Thread version signal process.
 */
static void *
nglSignalManagerSignalHandlerThreadSigwaitThread(
    void *arg)
{
    ngLog_t *log;
    int sigNo, performed;
    nglSignalAction_t *action;
    nglSignalManager_t *sigMng;
    nglSignalDefaultActionBehavior_t behavior;
    int mutexLocked, result, *error, errorEntity;
    static const char fName[] =
        "nglSignalManagerSignalHandlerThreadSigwaitThread";

    sigMng = &nglSignalManager;
    log = sigMng->ngsm_log;
    mutexLocked = 0;
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    action = NULL;
    sigMng->ngsm_threadWorking = 1;

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "sigwait thread started.\n"); 

    if (sigMng->ngsm_threadContinue == 0) {
       goto finalize;
    }

    for (;;) {
        /* Wait the signal */
        result = sigwait(&sigMng->ngsm_curMask, &sigNo);
        if (result != 0) {
            if (errno == EINTR) {
                /* gdb throws somehow */
                continue;
            }

            log = sigMng->ngsm_log;
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "%s failed: %s\n", "sigwait()", strerror(errno)); 
            goto finalize;
        }

        log = sigMng->ngsm_log;

        if (sigMng->ngsm_threadContinue == 0) {
            goto finalize;
        }

        /* Log for Debug */
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Catch the signal %d (%s).\n",
            sigNo, nglSignalManagerSignalNameGet(sigNo)); 

        /* Lock the Signal Manager */
        result = nglSignalManagerMutexLock(log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Signal Manager.\n"); 
            goto finalize;
        }
        mutexLocked = 1;

        action = NULL;
        result = nglSignalManagerSignalActionFind(
            sigMng, sigNo, &action, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't find the Signal Action.\n"); 
                goto finalize;
        }

        /* Unlock the Signal Manager */
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
            goto finalize;
        }

        if (action == NULL) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unregistered signal %d (%s) arrived.\n",
                sigNo, nglSignalManagerSignalNameGet(sigNo)); 

            /* Default */
            behavior = NGL_SIGNAL_DEFAULT_ACTION_NONE;
            result = nglSignalManagerDefaultActionTypeGet(
                sigNo, &behavior, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get the type of default signal action"
                    " for signal %d (%s).\n",
                    sigNo, nglSignalManagerSignalNameGet(sigNo)); 
                /* Not return */
            }

            if (behavior == NGL_SIGNAL_DEFAULT_ACTION_EXIT) {
                exit(1);

            } else if (behavior == NGL_SIGNAL_DEFAULT_ACTION_CORE) {
                abort();
            }
            /* Ignore NGL_SIGNAL_DEFAULT_ACTION_IGNORE */

            continue;
        }

        performed = 0;
        result = nglSignalManagerSignalHandlerCommon(
            action, &performed, NULL, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't handle signal.\n"); 
            goto finalize;
        }
    }

finalize:
    /* Thread Exit */

    log = sigMng->ngsm_log;

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "sigwait thread exiting.\n"); 

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        /* Not return */
    }
    mutexLocked = 1;

    sigMng->ngsm_threadWorking = 0;

    result = ngiCondSignal(&nglSignalManagerCond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't signal condition.\n"); 
        /* Not return */
    }
    
    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        /* Not return */
    }

    return NULL;
}
#endif /* NG_PTHREAD */

/**
 * SignalManager: Signal Handler for sigaction().
 */
static void
nglSignalManagerSignalHandlerSigaction(
    int sigNo)
{
    ngLog_t *log;
    int performed, result;
    int *error, errorEntity;
    nglSignalAction_t *action;
    nglSignalManager_t *sigMng;
    static const char fName[] = "nglSignalManagerSignalHandlerSigaction";

    sigMng = &nglSignalManager;
    log = NULL;

    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Catch the signal %d (%s).\n",
        sigNo, nglSignalManagerSignalNameGet(sigNo)); 

    /* Find signal action */

    /**
     * Note : Signal handler must be blocked on NonThread,
     * while Signal Action list is being modified.
     */
    action = NULL;
    result = nglSignalManagerSignalActionFind(
        sigMng, sigNo, &action, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't find the Signal Action.\n"); 
        action = NULL;
    }

    if (action == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The action for signal %d (%s) not found.\n",
            sigNo, nglSignalManagerSignalNameGet(sigNo)); 
        return;
    }

    performed = 0;
    result = nglSignalManagerSignalHandlerCommon(
        action, &performed, NULL, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't handle signal.\n"); 
        return;
    }

    return;
}

/**
 * SignalManager: Signal Handler for all.
 */
static int
nglSignalManagerSignalHandlerCommon(
    nglSignalAction_t *action,
    int *performed,
    ngLog_t *log,
    int *error)
{
    int sigNo, maxNest;
    void (*handler)(int);
    nglSignalManager_t *sigMng;
    nglSignalActionLevel_t *level;
    int i, result, isSet, mutexLocked;
    nglSignalDefaultActionBehavior_t behavior;
    static const char fName[] = "nglSignalManagerSignalHandlerCommon";

    /* Check the arguments */
    assert(action != NULL);
    assert(performed != NULL);

    mutexLocked = 0;
    *performed = 0;
    sigMng = &nglSignalManager;

    maxNest = NGL_SIGNAL_ACTION_MAX_NEST_LEVEL;
    handler = NULL;
    sigNo = 0;

    /* Lock the Signal Manager */
    result = nglSignalManagerMutexLock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Signal Manager.\n"); 
        goto error;
    }
    mutexLocked = 1;

    if (nglSignalManagerInitialized == 0) {
        goto success;
    }

    if (sigMng->ngsm_initialized == 0) {
        goto success;
    }

    if (sigMng->ngsm_started == 0) {
        goto success;
    }

    *performed = 1;

    sigNo = action->ngsa_signalNumber;

    /* SIG_IGN? */
    level = action->ngsa_level;
    for (i = 0; i < maxNest; i++) {
        if ((level[i].ngsal_userHandlerRegistered == 1) &&
            (level[i].ngsal_userHandler == SIG_IGN)) {
            goto success;
        }
    }

    /* SIGTSTP? */
    if ((sigMng->ngsm_handlingSIGTSTP != 0) && (sigNo == SIGTSTP)) {
        isSet = 0;
        for (i = 0; i < maxNest; i++) {
            if ((level[i].ngsal_userHandlerRegistered != 0) &&
                (level[i].ngsal_userHandler != SIG_DFL)) {
                isSet = 1;
            }
        }
        if (isSet == 0) {
            result = kill(getpid(), SIGSTOP);
            if (result < 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "kill failed: %s.\n", strerror(errno)); 
                goto error;
            }
        } else {
            /* Call user's signal handler */
            for (i = 0; i < maxNest; i++) {
                if ((level[i].ngsal_userHandlerRegistered != 0) &&
                    (level[i].ngsal_userHandler != SIG_DFL)) {
                    handler = level[i].ngsal_userHandler;
                    handler(sigNo);
                }
            }
        }
    } else {

        /* Handle system handler */
        for (i = 0; i < maxNest; i++) {
            if ((level[i].ngsal_ninfgHandlerRegistered != 0) &&
                (level[i].ngsal_ninfgHandler != SIG_DFL)) {
                handler = level[i].ngsal_ninfgHandler;
                handler(sigNo);
            }
        }

        /* Handle user handler */
        isSet = 0;
        for (i = 0; i < maxNest; i++) {
            if ((level[i].ngsal_userHandlerRegistered != 0) &&
                (level[i].ngsal_userHandler != SIG_DFL)) {
                handler = level[i].ngsal_userHandler;
                handler(sigNo);
                isSet = 1;
            }
        }

        if (isSet == 0) {
            /* Default */
            behavior = NGL_SIGNAL_DEFAULT_ACTION_NONE;
            result = nglSignalManagerDefaultActionTypeGet(
                sigNo, &behavior, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get the type of default signal action"
                    " for signal %d (%s).\n",
                    sigNo, nglSignalManagerSignalNameGet(sigNo)); 
                return 0;
            }

            if (behavior == NGL_SIGNAL_DEFAULT_ACTION_EXIT) {
                exit(1);

            } else if (behavior == NGL_SIGNAL_DEFAULT_ACTION_CORE) {
                abort();
            }
            /* Ignore NGL_SIGNAL_DEFAULT_ACTION_IGNORE */
        }
    }

success:

    /* Unlock the Signal Manager */
    mutexLocked = 0;
    result = nglSignalManagerMutexUnlock(log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Unlock the Signal Manager */
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = nglSignalManagerMutexUnlock(log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Signal Manager.\n"); 
            return 0;
        }
    }

    /* Failed */
    return 0;
}

/**
 * SignalManager: Get the signal default action type.
 */
static int
nglSignalManagerDefaultActionTypeGet(
    int signalNumber,
    nglSignalDefaultActionBehavior_t *behavior,
    ngLog_t *log,
    int *error)
{
    int i, found;
    nglSignalDefaultAction_t *defaultActions;

    /* Check the arguments */
    assert(behavior != NULL);

    *behavior = NGL_SIGNAL_DEFAULT_ACTION_NONE;
    defaultActions = nglSignalDefaultActions;

    found = 0;
    for (i = 0; defaultActions[i].ngsda_number != 0; i++) {
        if (signalNumber == defaultActions[i].ngsda_number) {
            /* Found */
            *behavior = defaultActions[i].ngsda_behavior;
            found = 1;
            break;
        }
    }

    /* Ignore unknown signal. (It may specified by Ninf-G user.) */
    if (found == 0) {
        *behavior = NGL_SIGNAL_DEFAULT_ACTION_IGNORE;
    }

    /* Success */
    return 1;
}

/**
 * SignalManager: Reset the Signal Mask.
 * Note: This function is called after fork, called by child process.
 */
int
ngiSignalManagerSignalMaskReset()
#ifdef NG_PTHREAD
{
    int result;
    nglSignalManager_t *sigMng;

    sigMng = &nglSignalManager;

    if (sigMng->ngsm_oldMaskStored == 0) {
        /* Do nothing */
        return 1;
    }

    result = pthread_sigmask(
        SIG_SETMASK, &sigMng->ngsm_oldMask , NULL);
    if (result != 0) {
        /* Do not print log */
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    /* Only effective for Pthread version */

    /* Do nothing */
    return 1;
}
#endif /* NG_PTHREAD */


/**
 * SignalManager: Mutex Lock.
 */
static int
nglSignalManagerMutexLock(
    ngLog_t *log,
    int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "nglSignalManagerMutexLock";

    /* Lock the mutex */
    result = ngiMutexLock(&nglSignalManagerMutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock mutex.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    /* Only effective for Pthread version */

    /* Do nothing */
    return 1;
}
#endif /* NG_PTHREAD */

/**
 * SignalManager: Mutex Unlock.
 */
static int
nglSignalManagerMutexUnlock(
    ngLog_t *log,
    int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "nglSignalManagerMutexUnlock";

    /* Unlock the mutex */
    result = ngiMutexUnlock(&nglSignalManagerMutex, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock mutex.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    /* Only effective for Pthread version */

    /* Do nothing */
    return 1;
}
#endif /* NG_PTHREAD */

static int
nglSignalManagerPthreadSigmask(
    int how,
    sigset_t *newMask,
    sigset_t *oldMask,
    ngLog_t *log,
    int *error)
#ifdef NG_PTHREAD
{
    int result;
    static const char fName[] = "nglSignalManagerPthreadSigmask";

    /* Get signal mask */
    result = pthread_sigmask(how, newMask, oldMask);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed: %s\n", "pthread_sigmask()", strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}
#else /* NG_PTHREAD */
{
    static const char fName[] = "nglSignalManagerPthreadSigmask";

    /* Only effective for Pthread version. */

    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s not found.\n", "pthread_sigmask()"); 

    /* Failed */
    return 0;
}
#endif /* NG_PTHREAD */

#else /* NGI_SIGNAL_HANDLING_ENABLED */

/**
 * SignalManager: Initialize. (Just return)
 */
int
ngiSignalManagerInitialize(
    int *id,
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * SignalManager: Finalize. (Just return)
 */
int
ngiSignalManagerFinalize(
    int id,
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * SignalManager: Start. (Just return)
 */
int
ngiSignalManagerStart(
    int id,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSignalManagerStart";

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Start the Signal Manager by id %d.\n", id); 
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Do nothing for signal.\n"); 

    /* Success */
    return 1;
}

/**
 * SignalManager: Stop. (Just return)
 */
int
ngiSignalManagerStop(
    int id,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiSignalManagerStop";

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Stop the Signal Manager by id %d.\n", id); 

    /* Success */
    return 1;
}

/**
 * SignalManager: Set the log. (Just return)
 */
int
ngiSignalManagerLogSet(
   int id,
   ngLog_t *target,
   ngLog_t *log,
   int *error)
{
    /* Success */
    return 1;
}

/**
 * SignalManager: Get signal name strings. (Just return)
 */
int
ngiSignalManagerSignalNamesGet(
    char ***signalNames,
    int **signalNumbers,
    int *nSignals,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(signalNames != NULL);
    assert(signalNumbers != NULL);
    assert(nSignals != NULL);

    *signalNames = NULL;
    *signalNumbers = NULL;
    *nSignals = 0;

    /* Success */
    return 1;
}

/**
 * SignalManager: Destruct the signal name strings. (Just return)
 */
int
ngiSignalManagerSignalNamesDestruct(
    char **signalNames,
    int *signalNumbers,
    int nSignals,
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * SignalManager: Signal Handler Register. (Just return)
 */
int
ngiSignalManagerSignalHandlerRegister(
    int id,
    ngiSignalHandlerType_t type,
    int *signals,
    int nSignals,
    void (*handler)(int),
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * SignalManager: Reset the Signal Mask. (Just return)
 */
int
ngiSignalManagerSignalMaskReset()
{
    /* Success */
    return 1;
}

#endif /* NGI_SIGNAL_HANDLING_ENABLED */

