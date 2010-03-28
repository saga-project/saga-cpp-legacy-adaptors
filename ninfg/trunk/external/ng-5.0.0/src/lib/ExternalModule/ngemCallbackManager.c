/*
 * $RCSfile: ngemCallbackManager.c,v $ $Revision: 1.8 $ $Date: 2008/03/17 08:58:40 $
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

#include "ngemList.h"
#include "ngemUtility.h"
#include "ngemLog.h"
#include "ngemCallbackManager.h"

NGI_RCSID_EMBED("$RCSfile: ngemCallbackManager.c,v $ $Revision: 1.8 $ $Date: 2008/03/17 08:58:40 $")

#define NGEML_CALLBACK_MANAGER_MAX_UNUSED_COUNT  64
#define NGEML_READ_CALLBACK_BUFFER_SIZE         (32U * 1024U)

typedef enum ngemlCallbackType_e {
    NGEML_CALLBACK_TYPE_UNUSED,
    NGEML_CALLBACK_TYPE_READ,
    NGEML_CALLBACK_TYPE_WRITE,
    NGEML_CALLBACK_TYPE_TIMER,
    NGEML_CALLBACK_TYPE_WAIT
} ngemlCallbackType_t;

typedef struct ngemlReadCallback_s {
    void                  *ngrc_arg;
    ngemReadCallbackFunc_t ngrc_callback;
    int                    ngrc_fd;
} ngemlReadCallback_t;

typedef struct ngemlWriteCallback_s {
    void                    *ngwc_arg;
    ngemWriteCallbackFunc_t  ngwc_callback;
    int                      ngwc_fd;
    void                    *ngwc_data;
    size_t                   ngwc_size;
    size_t                   ngwc_nWrite;
} ngemlWriteCallback_t;

typedef struct ngemlTimerCallback_s {
    void                   *ngtc_arg;
    ngemTimerCallbackFunc_t ngtc_callback;
    time_t                  ngtc_time;
} ngemlTimerCallback_t;

/**
 * Callback for terminating the process.
 */
typedef struct ngemlWaitCallback_s {
    void                  *ngwc_arg;
    ngemWaitCallbackFunc_t ngwc_callback;
    pid_t                  ngwc_pid;
} ngemlWaitCallback_t;

struct ngemlCallbackEntity_s {
    ngemlCallbackType_t ngc_type;
    bool                ngc_daemon;
    union {
        ngemlReadCallback_t  ngd_read;
        ngemlWriteCallback_t ngd_write;
        ngemlTimerCallback_t ngd_timer;
        ngemlWaitCallback_t  ngd_wait;
    } ngc_dummy;
};

#define ngc_read  ngc_dummy.ngd_read
#define ngc_write ngc_dummy.ngd_write
#define ngc_timer ngc_dummy.ngd_timer
#define ngc_wait  ngc_dummy.ngd_wait

typedef struct ngemlCallbackManager_s {
    bool                                ngcm_running;
    int                                 ngcm_nUnused;
    NGEM_LIST_OF(ngemlCallbackEntity_t) ngcm_listValid;
    NGEM_LIST_OF(ngemlCallbackEntity_t) ngcm_listUnused;
} ngemlCallbackManager_t;

typedef struct ngemlWriteStringCallbackArgument_s {
    ngemWriteStringCallbackFunc_t  ngwsca_func;
    void                          *ngwsca_arg;
} ngemlWriteStringCallbackArgument_t;

/* File local variables */
static ngemlCallbackManager_t ngemlCallbackManager;
static bool ngemlCallbackManagerInitialized = false;
static int ngemlCallbackManagerProcessExitNotifyFd = -1;
static pid_t ngemlCallbackManagerMainThread = -1;

/* File local functions */
static void ngemlCallbackManagerSigchildHandler(int);

static ngemResult_t ngemlCallbackManagerLoop(int);

static ngemResult_t ngemlCallbackManagerCheckWaitCallback(int);
static void ngemlReadCallbackRead(ngemCallback_t);
static void ngemlWriteCallbackWrite(ngemCallback_t);
static void ngemlTimerCallbackCall(ngemCallback_t);

static ngemlCallbackEntity_t *ngemlCallbackEntityCreate(void);
static void ngemlCallbackEntityDestroy(ngemlCallbackEntity_t *);

static void ngemlCallbackUnused(ngemCallback_t);
static ngemCallback_t ngemlCallbackGet(void);
static void ngemlCallbackEntitySetTypeRead(ngemlCallbackEntity_t *,
    int, ngemReadCallbackFunc_t, void *arg);
static void ngemlCallbackEntitySetTypeWrite(ngemlCallbackEntity_t *,
    int, ngemWriteCallbackFunc_t, void *, size_t, void *);
static void ngemlCallbackEntitySetTypeTimer(ngemlCallbackEntity_t *,
    int, ngemTimerCallbackFunc_t, void *);
static void ngemlCallbackEntitySetTypeWait(ngemlCallbackEntity_t *,
    pid_t, ngemWaitCallbackFunc_t, void *);

static void ngemWriteStringCallback(
    void *, int, void *, size_t, size_t, ngemCallbackResult_t);

/**
 * Callback Manager: Initialize
 */
ngemResult_t
ngemCallbackManagerInitialize()
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackManagerInitialize";

    log = ngemLogGetDefault();

    if (ngemlCallbackManagerInitialized == true) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Callback manager is already initialized.\n");
        return NGEM_FAILED;
    }

    mng->ngcm_nUnused = 0;
    mng->ngcm_running = false;
    NGEM_LIST_SET_INVALID_VALUE(&mng->ngcm_listValid);
    NGEM_LIST_SET_INVALID_VALUE(&mng->ngcm_listUnused);

    NGEM_LIST_INITIALIZE(ngemlCallbackEntity_t,  &mng->ngcm_listValid);
    NGEM_LIST_INITIALIZE(ngemlCallbackEntity_t,  &mng->ngcm_listUnused);

    ngemlCallbackManagerInitialized = true;
    
    return NGEM_SUCCESS;
}

/**
 * Callback Manager: Finalize
 */
ngemResult_t
ngemCallbackManagerFinalize()
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemlCallbackEntity_t *entity = NULL;
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) it;
    int i = 0;
    static const char fName[] = "ngemCallbackManagerFinalize";

    log = ngemLogGetDefault();

    if (ngemlCallbackManagerInitialized == false) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Callback manager is not still initialized.\n");
        return NGEM_FAILED;
    }
    
    if (mng->ngcm_running == true) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Callback manager is running.\n");
        return NGEM_FAILED;
    }

    NGEM_LIST_ERASE_EACH(ngemlCallbackEntity_t, &mng->ngcm_listValid, it, entity) {
        ngemlCallbackEntityDestroy(entity);
        i++;
    }
    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Valid entities = %d\n", i);
    NGEM_LIST_FINALIZE(ngemlCallbackEntity_t, &mng->ngcm_listValid);

    i = 0;
    NGEM_LIST_ERASE_EACH(ngemlCallbackEntity_t, &mng->ngcm_listUnused, it, entity) {
        ngemlCallbackEntityDestroy(entity);
        i++;
    }
    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Unused entities = %d\n", i);
    NGEM_LIST_FINALIZE(ngemlCallbackEntity_t, &mng->ngcm_listUnused);

    mng->ngcm_nUnused = 0;
    mng->ngcm_running = false;
    ngemlCallbackManagerInitialized = false;
    
    return NGEM_SUCCESS;
}

/**
 * Callback Manager: SigchildHandler
 * Handling the SIGCHLD
 */
static void
ngemlCallbackManagerSigchildHandler(
    int signum)
{
    ssize_t nw;
    int result;
    pid_t pid;
#if 0
    static const char fName[] = "ngemlCallbackManagerSigchildHandler";
#endif

    if (ngemlCallbackManagerProcessExitNotifyFd < 0) {
        return;
    }

    if (ngemlCallbackManagerMainThread == getpid()) {
        nw = write(ngemlCallbackManagerProcessExitNotifyFd, "c", 1);
        if (nw < 0) {
            result = close(ngemlCallbackManagerProcessExitNotifyFd);
            if (result < 0 ){
                /* Do nothing */;
            }
            ngemlCallbackManagerProcessExitNotifyFd = -1;
        }
    } else {
        while (1) {
            pid = waitpid(-1, NULL, WNOHANG);
            if (pid < 0) {
                if (errno == EINTR) {
                    continue;
                }
            }
            break;
        }
    }
    return;
}

/**
 * Callback Manager: Enter event loop
 */
ngemResult_t
ngemCallbackManagerRun()
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    int pfd[2] = {-1, -1};
    ngemResult_t ret = NGEM_SUCCESS;
    bool isSignalSet = false;
    ngemResult_t nResult;
    int result;
    int i;
    struct sigaction sa;
    struct sigaction saOld;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackManagerRun";

    log = ngemLogGetDefault();

    if (ngemlCallbackManagerInitialized == false) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Callback manager is not still initialized.\n");
        return NGEM_FAILED;
    }
    mng->ngcm_running = true;

    /* Prepare */
    nResult = ngemNonblockingPipe(pfd);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "Can't create pipes.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    for (i = 0;i < 2;++i) {
        nResult = ngemFDsetExecOnCloseFlag(pfd[i]);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
                "Can't set exec-on-close flag\n");
            ret = NGEM_FAILED;
            goto finalize;
        }
    }

    ngemlCallbackManagerMainThread = getpid();
    ngemlCallbackManagerProcessExitNotifyFd = NGEM_PIPE_OUT(pfd);

    /* Set the Signal handler */
    sa.sa_flags   = SA_NOCLDSTOP;
    sa.sa_handler = ngemlCallbackManagerSigchildHandler;
    sigemptyset(&sa.sa_mask);
    result = sigaction(SIGCHLD, &sa, &saOld);
    if (result < 0) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "sigaction: %s\n", strerror(errno));
        ret = NGEM_FAILED;
        goto finalize;
    }
    isSignalSet = true;

    /* Loop */
    nResult = ngemlCallbackManagerLoop(NGEM_PIPE_IN(pfd));
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Error occurred in event loop.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* After process */
finalize:
    if (isSignalSet == true) {
        /* Restores the signal handler */
        result = sigaction(SIGCHLD, &saOld, NULL);
        if (result < 0) {
            ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, 
                "sigaction: %s\n", strerror(errno));
            ret = NGEM_FAILED;
        }
    }
    ngemlCallbackManagerProcessExitNotifyFd = -1;

    for (i = 0;i < 2;++i) {
        if (pfd[i] >= 0) {
            result = close(pfd[i]);
            if (result < 0) {
                ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, 
                    "close: %s\n", strerror(errno));
                ret = NGEM_FAILED;
            }
        }
    }
    mng->ngcm_running = false;

    return ret;
}

/**
 * Callback Manager: Loop
 */
static ngemResult_t
ngemlCallbackManagerLoop(
    int fd)
{
    int max = 0;
    fd_set fdsRead;
    fd_set fdsWrite;
    struct timeval tv;
    struct timeval *ptv;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) it;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) last;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) itTmp;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) itValid;
    NGEM_LIST_OF(ngemlCallbackEntity_t) listTmp;
    ngemlCallbackEntity_t *entity = NULL;
    time_t now;
    time_t nextTime;
    int result;
    int count;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngLog_t *log;
    static const char fName[] = "ngemlCallbackManagerLoop";

    NGEM_ASSERT(ngemlCallbackManagerInitialized == true);

    log = ngemLogGetDefault();

    /* List Init */
    NGEM_LIST_INITIALIZE(ngemlCallbackEntity_t, &listTmp);

    while (1) {
        count = 0;
        NGEM_LIST_FOREACH(ngemlCallbackEntity_t, &mng->ngcm_listValid, it, entity) {
            if (!entity->ngc_daemon) {
                count++;
            }
        }
        if (count == 0) {
            break;
        }
        /* Merge */
        max = 0;
        FD_ZERO(&fdsRead);
        FD_ZERO(&fdsWrite);

        ptv = NULL;
        nextTime = -1;

        /* Set FD for signal(Wait) */
        FD_SET(fd, &fdsRead);
        max = fd;

        NGEM_LIST_FOREACH(ngemlCallbackEntity_t, &mng->ngcm_listValid, it, entity) {
            switch (entity->ngc_type) {
            case NGEML_CALLBACK_TYPE_READ:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checks fd %d for reading.\n", entity->ngc_read.ngrc_fd);
                FD_SET(entity->ngc_read.ngrc_fd, &fdsRead);
                max = NGEM_MAX(entity->ngc_read.ngrc_fd, max);
                break;
            case NGEML_CALLBACK_TYPE_WRITE:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checks fd %d for writing.\n", entity->ngc_write.ngwc_fd);
                FD_SET(entity->ngc_write.ngwc_fd, &fdsWrite);
                max = NGEM_MAX(entity->ngc_write.ngwc_fd, max);
                break;
            case NGEML_CALLBACK_TYPE_TIMER:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checks timer(%ld) .\n", (long)entity->ngc_timer.ngtc_time);
                if (nextTime < 0) {
                    nextTime = entity->ngc_timer.ngtc_time;
                } else {
                    nextTime = NGEM_MIN(nextTime, entity->ngc_timer.ngtc_time);
                }
                break;
            case NGEML_CALLBACK_TYPE_WAIT:
            case NGEML_CALLBACK_TYPE_UNUSED:
                /* Do nothing */
                break;
            default:
                NGEM_ASSERT_NOTREACHED();
            }
        }

        /* Set Time val */
        if (nextTime >= 0) {
            now = time(NULL);
            NGEM_ASSERT(now >= 0);
            ptv = &tv;
            tv.tv_sec  = NGEM_MAX(nextTime - now, 0);
            tv.tv_usec = 0;
            ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Wait %lds.\n", tv.tv_sec);
        }
        max++;

        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Enters in select() (max = %d).\n", max);
        result = select(max, &fdsRead, &fdsWrite, NULL, ptv);
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Leaves in select().\n");
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
                "select: %s.\n", strerror(errno));
            ret = NGEM_FAILED;
            goto finalize;
        }

        /* Wait Check */
        if (FD_ISSET(fd, &fdsRead)) {
            nResult = ngemlCallbackManagerCheckWaitCallback(fd);
            if (nResult == NGEM_FAILED) {
                ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Error occurred during checking finished process.\n");
                ret = NGEM_FAILED;
                goto finalize;
            }
        }

        /* Slice */
        itTmp = NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &listTmp);
        it    = NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &mng->ngcm_listValid);
        last  = NGEM_LIST_END  (ngemlCallbackEntity_t, &mng->ngcm_listValid);
        NGEM_LIST_SPLICE2(ngemlCallbackEntity_t, itTmp, it, last);
        itValid = last;

        /* Checking */
        now = time(NULL);
        while (!NGEM_LIST_IS_EMPTY(ngemlCallbackEntity_t, &listTmp)) {
            it = NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &listTmp);
            NGEM_LIST_SPLICE1(ngemlCallbackEntity_t, itValid, it);
            
            entity = NGEM_LIST_GET(ngemlCallbackEntity_t, it);
            switch (entity->ngc_type) {
            case NGEML_CALLBACK_TYPE_READ:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checking fd %d for reading.\n", entity->ngc_read.ngrc_fd);
                if (FD_ISSET(entity->ngc_read.ngrc_fd, &fdsRead)) {
                    ngemlReadCallbackRead(it);
                }
                break;
            case NGEML_CALLBACK_TYPE_WRITE:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checking fd %d for writing.\n", entity->ngc_write.ngwc_fd);
                if (FD_ISSET(entity->ngc_write.ngwc_fd, &fdsWrite)) {
                    ngemlWriteCallbackWrite(it);
                }
                break;
            case NGEML_CALLBACK_TYPE_TIMER:
                ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "Checking timer(Timer = %ld, Now = %ld).\n",
                    (long)entity->ngc_timer.ngtc_time, (long)now);
                if (entity->ngc_timer.ngtc_time <= now) {
                    ngemlTimerCallbackCall(it);
                }
                break;
            case NGEML_CALLBACK_TYPE_WAIT:
                ;/* Do nothing */
                break;
            case NGEML_CALLBACK_TYPE_UNUSED:
            default:
                NGEM_ASSERT_NOTREACHED();
            }
        }
    }

finalize:
    NGEM_LIST_FINALIZE(ngemlCallbackEntity_t, &listTmp);

    return ret;
}

/**
 * Callback Manager: Check terminate process.
 */
static ngemResult_t
ngemlCallbackManagerCheckWaitCallback(
    int fd)
{
    ssize_t nread;
    char c = 'e';
    int status;
    ngemlCallbackEntity_t *entity;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) it;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) itValid;
    NGEM_LIST_OF(ngemlCallbackEntity_t) listTmp;
    ngemWaitCallbackFunc_t func;
    void *arg;
    pid_t pid;
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    static const char fName[] = "ngemlCallbackManagerCheckWaitCallback";

    NGEM_ASSERT(ngemlCallbackManagerInitialized != false);
    NGEM_ASSERT(fd >= 0);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Called.\n");

    NGEM_LIST_INITIALIZE(ngemlCallbackEntity_t, &listTmp);

    /* Read 1 bytes */
    do {
        nread = read(fd, &c, 1);
        if ((nread < 0) && (errno != EINTR)) {
            if (errno != EAGAIN) {
                ret = NGEM_FAILED;
                ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
                    "read: %s.\n", strerror(errno));
            } 
            goto finalize;
        }
    } while (nread < 0);

    if (nread == 0) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER ,fName, "Unexpected EOF.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    for (;;) {
        /* Wait Child Process */
        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == ECHILD) {
                break;
            }
            ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "waitpid: %s.\n", strerror(errno));
            ret = NGEM_FAILED;
            goto finalize;
        }

        if (pid == 0) {
            break;
        }
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "The finished process's id %ld.\n", (long)pid);

        /* Move */
        NGEM_LIST_SPLICE2(ngemlCallbackEntity_t,
            NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &listTmp),
            NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &mng->ngcm_listValid),
            NGEM_LIST_END  (ngemlCallbackEntity_t, &mng->ngcm_listValid));
        itValid = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listValid);

        /* Check Process Waiter */
        while (!NGEM_LIST_IS_EMPTY(ngemlCallbackEntity_t, &listTmp)) {
            it = NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &listTmp);
            NGEM_LIST_SPLICE1(ngemlCallbackEntity_t, itValid, it);

            entity = NGEM_LIST_GET(ngemlCallbackEntity_t, it);
            NGEM_ASSERT(entity != NULL);
            if ((entity->ngc_type == NGEML_CALLBACK_TYPE_WAIT) &&
                (entity->ngc_wait.ngwc_pid == pid)) {
                func = entity->ngc_wait.ngwc_callback;
                arg  = entity->ngc_wait.ngwc_arg;

                ngemlCallbackUnused(it);
                
                /* Callback */
                func(arg, pid, status, NGEM_CALLBACK_RESULT_SUCCESS);
            }
        }
        /* Not error, if callback isn't found */
    }

finalize:
    NGEM_LIST_FINALIZE(ngemlCallbackEntity_t, &listTmp);

    return ret;
}

/**
 * Callback: Is valid?
 */
bool
ngemCallbackIsValid(
    ngemCallback_t callback)
{
    ngemlCallbackManager_t *cm = &ngemlCallbackManager;
    ngemCallback_t last = NULL;
    ngemlCallbackEntity_t *entity = NULL;
#if 0
    static const char fName[] = "ngemCallbackIsValid";
#endif
    if (!NGEM_LIST_ITERATOR_IS_VALID(ngemlCallbackEntity_t, callback)) {
        return false;
    }

    last = NGEM_LIST_END(ngemlCallbackEntity_t, &cm->ngcm_listUnused);
    if (callback == last) {
        return false;
    }

    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    if (entity->ngc_type == NGEML_CALLBACK_TYPE_UNUSED) {
        return false;
    }
    return true;
}

/**
 * Callback: call back for reading
 */
static void
ngemlReadCallbackRead(
    ngemCallback_t callback)
{
    ngemlCallbackEntity_t *entity;
    ngemReadCallbackFunc_t func;
    ngemCallbackResult_t cResult;
    char buffer[NGEML_READ_CALLBACK_BUFFER_SIZE] = "";
    void *data;
    ssize_t nr;
    size_t nRead = 0;
    void *arg;
    int fd;
    ngLog_t *log;
    static const char fName[] = "ngemlReadCallbackRead";

    log    = ngemLogGetDefault();
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);

    NGEM_ASSERT(entity->ngc_type == NGEML_CALLBACK_TYPE_READ);

    func    = entity->ngc_read.ngrc_callback;
    arg     = entity->ngc_read.ngrc_arg;
    fd      = entity->ngc_read.ngrc_fd;
    data    = NULL;
    nRead   = 0;
    cResult = NGEM_CALLBACK_RESULT_SUCCESS;

    do {
        nr = read(fd, buffer, NGEML_READ_CALLBACK_BUFFER_SIZE);
    } while ((nr < 0) && (errno == EINTR));

    if (nr < 0) {
        if (errno == EAGAIN) {
            return;
        }
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "read: %s", strerror(errno));
        /* Error */
        cResult = NGEM_CALLBACK_RESULT_FAILED;
    } else if (nr == 0) {
        /* EOF */
        cResult = NGEM_CALLBACK_RESULT_EOF;
    } else {
        /* Read Success */
        data  = buffer;
        nRead = nr;
    }
    ngemlCallbackUnused(callback);

    func(arg, fd, data, nRead, cResult);

    return;
}

/**
 * Callback: call back for writing
 */
static void
ngemlWriteCallbackWrite(
    ngemCallback_t callback)
{
    ngemlCallbackEntity_t *entity;
    ngemWriteCallbackFunc_t func;
    ngemCallbackResult_t cResult;
    char *data;
    ssize_t nw;
    size_t nWrite;
    size_t size;
    void *arg;
    int fd;
    ngLog_t *log;
    static const char fName[] = "ngemlWriteCallbackWrite";

    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    log    = ngemLogGetDefault();

    NGEM_ASSERT(entity->ngc_type == NGEML_CALLBACK_TYPE_WRITE);

    func    = entity->ngc_write.ngwc_callback;
    arg     = entity->ngc_write.ngwc_arg;
    fd      = entity->ngc_write.ngwc_fd;
    data    = (char *)entity->ngc_write.ngwc_data;
    size    = entity->ngc_write.ngwc_size;
    nWrite  = entity->ngc_write.ngwc_nWrite;
    cResult = NGEM_CALLBACK_RESULT_SUCCESS;

    do {
        NGEM_ASSERT(size > nWrite);
        nw = write(fd, &data[nWrite], size - nWrite);
    } while ((nw < 0) && (errno == EINTR));

    if (nw < 0) {
        if (errno == EAGAIN) {
            return;
        }
        /* Error */
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "write: %s.\n", strerror(errno));
        cResult = NGEM_CALLBACK_RESULT_FAILED;
    } else if (nw == 0) {
        cResult = NGEM_CALLBACK_RESULT_EOF;
    } else {
        /* Write Success */
        nWrite = nWrite + nw;
        entity->ngc_write.ngwc_nWrite = nWrite;
        if (nWrite < size) {
            /* Continue */
            return;
        }
    }
    ngemlCallbackUnused(callback);
    func(arg, fd, data, size, nWrite, cResult);

    return;
}

/**
 * Callback: call back for writing
 */
static void
ngemlTimerCallbackCall(
    ngemCallback_t callback)
{
    ngemTimerCallbackFunc_t func;
    void *arg;
    ngemlCallbackEntity_t *entity;
#if 0
    static const char fName[] = "ngemlTimerCallbackCall";
#endif

    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);

    func = entity->ngc_timer.ngtc_callback;
    arg  = entity->ngc_timer.ngtc_arg;

    ngemlCallbackUnused(callback);

    func(arg, NGEM_CALLBACK_RESULT_SUCCESS);

    return;
}

/**
 * Callback Entity: Create
 */
static ngemlCallbackEntity_t *
ngemlCallbackEntityCreate(void)
{
    ngemlCallbackEntity_t *new = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemlCallbackEntityCreate";

    log = ngemLogGetDefault();

    new = NGI_ALLOCATE(ngemlCallbackEntity_t, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Can't allocate storage for callback entity.\n");

        return NULL;
    }
    memset(new, '\0', sizeof(*new));
    new->ngc_type   = NGEML_CALLBACK_TYPE_UNUSED;
    new->ngc_daemon = false;

    return new;
}

/**
 * Callback Entity: Destroy
 */
static void
ngemlCallbackEntityDestroy(
    ngemlCallbackEntity_t *callback)
{
    ngLog_t *log;
#if 0
    static const char fName[] = "ngemlCallbackEntityDestroy";
#endif
    log = ngemLogGetDefault();

    if (callback == NULL) {
        return;
    }

    memset(callback, '\0', sizeof(*callback));
    callback->ngc_type = NGEML_CALLBACK_TYPE_UNUSED;
    NGI_DEALLOCATE(ngemlCallbackEntity_t, callback, log, NULL);

    return;
}

/**
 * Callback: Register callback function for reading.
 */
ngemCallback_t
ngemCallbackRead(
    int fd,
    ngemReadCallbackFunc_t func,
    void *arg)
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemlCallbackEntity_t *entity;
    ngemCallback_t callback;
    ngemCallback_t last;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackRead";

    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(func != NULL);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
        "Registers callback for reading from fd %d\n", fd);

    last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
    callback = ngemlCallbackGet();
    if (callback == last) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    ngemlCallbackEntitySetTypeRead(entity, fd, func, arg);

    return callback;
}

/**
 * Callback: Register callback function for writing.
 */
ngemCallback_t
ngemCallbackWrite(
    int fd,
    ngemWriteCallbackFunc_t func,
    void *data,
    size_t size,
    void *arg)
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemCallback_t callback;
    ngemCallback_t last;
    ngLog_t *log;
    ngemlCallbackEntity_t *entity;
    static const char fName[] = "ngemCallbackWrite";

    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(func != NULL);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
        "Registers callback for writing to fd %d\n", fd);

    last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
    callback = ngemlCallbackGet();
    if (callback == last) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    ngemlCallbackEntitySetTypeWrite(entity, fd, func, data, size, arg);

    return callback;
}

/**
 * Callback: Register callback function for writing(vprintf).
 */
ngemCallback_t
ngemCallbackWriteVformat(
    int fd,
    ngemWriteStringCallbackFunc_t func,
    void *arg, 
    const char *format,
    va_list ap)
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    char *string = NULL;
    ngemCallback_t callback = NULL;
    ngemCallback_t last;
    ngemlWriteStringCallbackArgument_t *callbackArgument = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackWriteVformat";

    log = ngemLogGetDefault();
    last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);

    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(func != NULL);
    NGEM_ASSERT_STRING(format);

    callbackArgument = NGI_ALLOCATE(ngemlWriteStringCallbackArgument_t, log, NULL);
    if (callbackArgument == NULL) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Can't allocate for storage for a argument of callback.\n");
        goto error;
    }
    callbackArgument->ngwsca_func = func;
    callbackArgument->ngwsca_arg  = arg;

    string = ngemStrdupVprintf(format, ap);
    if (string == NULL) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Can't allocate storage for the string.\n");
        goto error;
    }    
    ngLogDebug(log, NGEM_LOGCAT_CALLBACK, fName, "Writing string \"%s\".\n", string);

    callback = ngemCallbackWrite(
        fd, ngemWriteStringCallback, string, strlen(string), callbackArgument);
    if (!ngemCallbackIsValid(callback)) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Can't register callback for writing string.\n");
        goto error;
    }
    return callback;

error:
    NGI_DEALLOCATE(ngemlWriteStringCallbackArgument_t, callbackArgument, log, NULL);
    ngiFree(string, log, NULL);

    return last;
}

/**
 * Callback: Register callback function for writing(printf).
 */
ngemCallback_t
ngemCallbackWriteFormat(
    int fd,
    ngemWriteStringCallbackFunc_t func,
    void *arg, 
    const char *format,
    ...)
{
    ngemCallback_t callback;
    va_list ap;
#if 0
    static const char fName[] = "ngemCallbackWriteFormat";
#endif

    va_start(ap, format);
    callback = ngemCallbackWriteVformat(fd, func, arg, format, ap);
    va_end(ap);

    return callback;
}

/**
 * Callback: Callback function for writing string.
 */
static void
ngemWriteStringCallback(
    void *arg,
    int fd,
    void *string,
    size_t bufferSize,
    size_t nWrite,
    ngemCallbackResult_t cResult)
{
    ngLog_t *log;
    ngemlWriteStringCallbackArgument_t *callbackArgument = arg;
#if 0
    static const char fName[] = "ngemWriteStringCallback";
#endif

    log = ngemLogGetDefault();

    ngiFree(string, log, NULL);

    callbackArgument = arg;
    callbackArgument->ngwsca_func(callbackArgument->ngwsca_arg, fd, cResult);

    ngiFree(arg, log, NULL);

    return;
}

/**
 * Callback: Register callback for timer.
 */
ngemCallback_t
ngemCallbackSetTimer(
    int waitTime,
    ngemTimerCallbackFunc_t func,
    void *arg)
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemCallback_t callback;
    ngemCallback_t last;
    ngemlCallbackEntity_t *entity;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackSetTimer";

    log = ngemLogGetDefault();
    last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);

    NGEM_ASSERT(waitTime >= 0);
    NGEM_ASSERT(func != NULL);

    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "Callback after %ds.\n", waitTime);

    callback = ngemlCallbackGet();
    if (callback == last) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    ngemlCallbackEntitySetTypeTimer(entity, waitTime, func, arg);

    return callback;
}

/**
 * Callback: Register callback for waiting a child process.
 */
ngemCallback_t
ngemCallbackWait(
    pid_t pid,
    ngemWaitCallbackFunc_t func,
    void *arg)
{
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemCallback_t callback;
    ngemCallback_t last;
    ngemlCallbackEntity_t *entity = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackWait";

    log = ngemLogGetDefault();

    callback = ngemlCallbackGet();
    last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
    if (callback == last) {
        ngLogError(log, NGEM_LOGCAT_LINEBUFFER, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    ngemlCallbackEntitySetTypeWait(entity, pid, func, arg);

    return callback;
}

/**
 * Callback: Cancel
 */
void
ngemCallbackCancel(
    ngemCallback_t callback)
{
    ngemlCallbackEntity_t copy;
    ngemlCallbackEntity_t *entity;
    ngLog_t *log;
    static const char fName[] = "ngemCallbackCancel";

    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    copy   = *entity;
    log    = ngemLogGetDefault();

    ngemlCallbackUnused(callback);

    switch (copy.ngc_type) {
    case NGEML_CALLBACK_TYPE_READ:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Cancel reading(fd=%d).\n", copy.ngc_read.ngrc_fd);
        copy.ngc_read.ngrc_callback(
            copy.ngc_read.ngrc_arg,
            copy.ngc_read.ngrc_fd,
            NULL, 0,
            NGEM_CALLBACK_RESULT_CANCEL);
        break;
    case NGEML_CALLBACK_TYPE_WRITE:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Cancel writing(fd=%d).\n", copy.ngc_write.ngwc_fd);
        copy.ngc_write.ngwc_callback(
            copy.ngc_write.ngwc_arg,
            copy.ngc_write.ngwc_fd,
            copy.ngc_write.ngwc_data,
            copy.ngc_write.ngwc_nWrite,
            copy.ngc_write.ngwc_size,
            NGEM_CALLBACK_RESULT_CANCEL);
        break;
    case NGEML_CALLBACK_TYPE_TIMER:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Cancel Timer(time=%ld).\n", (long)entity->ngc_timer.ngtc_time);
        copy.ngc_timer.ngtc_callback(
            copy.ngc_timer.ngtc_arg,
            NGEM_CALLBACK_RESULT_CANCEL);
        break;
    case NGEML_CALLBACK_TYPE_WAIT:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Cancel waiting process(pid=%ld).\n", (long)copy.ngc_wait.ngwc_pid);
        copy.ngc_wait.ngwc_callback(
            copy.ngc_wait.ngwc_arg,
            copy.ngc_wait.ngwc_pid, 0,
            NGEM_CALLBACK_RESULT_CANCEL);
        break;
    case NGEML_CALLBACK_TYPE_UNUSED:
    default:
        NGEM_ASSERT_NOTREACHED();
    }
    return;
}

/**
 * Callback: Set unused.
 */
static void
ngemlCallbackUnused(
    ngemCallback_t callback)
{
    ngemlCallbackEntity_t *entity;
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) last;
    ngLog_t *log;
    static const char fName[] = "ngemlCallbackUnused";

    NGEM_ASSERT(callback != NULL);

    log    = ngemLogGetDefault();
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);

    switch (entity->ngc_type) {
    case NGEML_CALLBACK_TYPE_READ:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Unused Read Callback(fd=%d).\n", entity->ngc_read.ngrc_fd);
        break;
    case NGEML_CALLBACK_TYPE_WRITE:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Unused Write Callback(fd=%d).\n", entity->ngc_write.ngwc_fd);
        break;
    case NGEML_CALLBACK_TYPE_TIMER:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Unused Timer Callback(time=%ld).\n", (long)entity->ngc_timer.ngtc_time);
        break;
    case NGEML_CALLBACK_TYPE_WAIT:
        ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName,
            "Unused Wait Callback(pid=%ld).\n", (long)entity->ngc_wait.ngwc_pid);
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }
    
    if (mng->ngcm_nUnused < NGEML_CALLBACK_MANAGER_MAX_UNUSED_COUNT ) {
        /* Move list of unused callbacks */
        mng->ngcm_nUnused++;

        memset(entity, '\0', sizeof(*entity));
        entity->ngc_type = NGEML_CALLBACK_TYPE_UNUSED;
        entity->ngc_daemon = false;

        last = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
        NGEM_LIST_SPLICE1(ngemlCallbackEntity_t, last, callback);
    } else {
        /* Remove */
        ngemlCallbackEntityDestroy(entity);
        NGEM_LIST_ERASE(ngemlCallbackEntity_t, callback);
    }

    return;
}

/**
 * Return last of list of unused callback, If error is occurred.
 */
static ngemCallback_t
ngemlCallbackGet(void)
{
    ngemlCallbackEntity_t *entity = NULL;
    ngemlCallbackEntity_t *newEntity = NULL;
    ngemlCallbackManager_t *mng = &ngemlCallbackManager;
    ngemResult_t nResult;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) first;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) last;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) it;
    NGEM_LIST_ITERATOR_OF(ngemlCallbackEntity_t) callback;
    
    first = NGEM_LIST_BEGIN(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
    last  = NGEM_LIST_END(ngemlCallbackEntity_t, &mng->ngcm_listUnused);
    callback = last;
    if (first == last) {
        /* Empty */
        newEntity = ngemlCallbackEntityCreate();
        if (newEntity == NULL) {
            goto finalize;
        }

        nResult = NGEM_LIST_INSERT_HEAD(ngemlCallbackEntity_t,
            &mng->ngcm_listValid, newEntity);
        if (nResult == NGEM_FAILED) {
            goto finalize;
        }
        newEntity = NULL;
    } else {
        NGEM_ASSERT(mng->ngcm_nUnused > 0);
        mng->ngcm_nUnused--;

        it = NGEM_LIST_BEGIN(ngemlCallbackEntity_t,
            &mng->ngcm_listValid);
        NGEM_LIST_SPLICE1(ngemlCallbackEntity_t, it, first);
    }

    /* Check Only */
    callback = NGEM_LIST_BEGIN(ngemlCallbackEntity_t,
            &mng->ngcm_listValid);
    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);

    NGEM_ASSERT(entity != NULL);
    NGEM_ASSERT(entity->ngc_type == NGEML_CALLBACK_TYPE_UNUSED);

finalize:
    ngemlCallbackEntityDestroy(newEntity);
    return callback;
}

/**
 * Callback entity: Set type "read".
 */
static void
ngemlCallbackEntitySetTypeRead(
    ngemlCallbackEntity_t *entity,
    int fd,
    ngemReadCallbackFunc_t func,
    void *arg)
{
    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(func != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGEML_CALLBACK_TYPE_READ;
    entity->ngc_read.ngrc_fd       = fd;
    entity->ngc_read.ngrc_callback = func;
    entity->ngc_read.ngrc_arg      = arg;

    return;
}

/**
 * Callback entity: Set type "write".
 */
static void
ngemlCallbackEntitySetTypeWrite(
    ngemlCallbackEntity_t *entity,
    int fd,
    ngemWriteCallbackFunc_t func,
    void *data,
    size_t size,
    void *arg)
{
    NGEM_ASSERT(entity != NULL);
    NGEM_ASSERT(fd >= 0);
    NGEM_ASSERT(func != NULL);
    NGEM_ASSERT(size > 0);
    NGEM_ASSERT(data != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGEML_CALLBACK_TYPE_WRITE;
    entity->ngc_write.ngwc_fd       = fd;
    entity->ngc_write.ngwc_callback = func;
    entity->ngc_write.ngwc_arg      = arg;
    entity->ngc_write.ngwc_nWrite   = 0U;
    entity->ngc_write.ngwc_size     = size;
    entity->ngc_write.ngwc_data     = data;

    return;
}

/**
 * Callback entity: Set type "timer".
 */
static void
ngemlCallbackEntitySetTypeTimer(
    ngemlCallbackEntity_t *entity,
    int waitTime,
    ngemTimerCallbackFunc_t func,
    void *arg)
{
    time_t now;
    ngLog_t *log;
    static const char fName[] = "ngemlCallbackEntitySetTypeTimer";

    NGEM_ASSERT(entity != NULL);
    NGEM_ASSERT(waitTime >= 0);
    NGEM_ASSERT(func != NULL);

    log = ngemLogGetDefault();
    now = time(NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGEML_CALLBACK_TYPE_TIMER;
    entity->ngc_timer.ngtc_time     = now + waitTime;
    entity->ngc_timer.ngtc_callback = func;
    entity->ngc_timer.ngtc_arg      = arg;

    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "waitTime    = %d.\n", waitTime);
    ngLogDebug(log, NGEM_LOGCAT_LINEBUFFER, fName, "timeoutTime = %ld.\n",
        (long)entity->ngc_timer.ngtc_time);

    NGEM_ASSERT(entity->ngc_timer.ngtc_time >= now);

    return;
}

/**
 * Callback entity: Set type "wait".
 */
static void
ngemlCallbackEntitySetTypeWait(
    ngemlCallbackEntity_t *entity,
    pid_t pid,
    ngemWaitCallbackFunc_t func,
    void * arg)
{
#if 0
    static const char fName[] = "ngemlCallbackEntitySetTypeWait";
#endif
    NGEM_ASSERT(entity != NULL);
    NGEM_ASSERT(pid > 0);
    NGEM_ASSERT(func != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGEML_CALLBACK_TYPE_WAIT;
    entity->ngc_wait.ngwc_pid      = pid;
    entity->ngc_wait.ngwc_callback = func;
    entity->ngc_wait.ngwc_arg      = arg;

    return;
}

void
ngemCallbackSetDaemon(
    ngemCallback_t callback)
{
    ngemlCallbackEntity_t *entity;

    NGEM_ASSERT(callback != NULL);

    entity = NGEM_LIST_GET(ngemlCallbackEntity_t, callback);
    NGEM_ASSERT(entity != NULL);

    entity->ngc_daemon = true;

    return;
}
