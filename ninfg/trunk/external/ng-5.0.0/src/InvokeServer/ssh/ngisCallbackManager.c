/*
 * $RCSfile: ngisCallbackManager.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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

NGI_RCSID_EMBED("$RCSfile: ngisCallbackManager.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $")

#define NGISL_CALLBACK_MANAGER_MAX_UNUSED_COUNT  64
#define NGISL_READ_CALLBACK_BUFFER_SIZE         (32U * 1024U)

typedef enum ngisCallbackType_e {
    NGISL_CALLBACK_TYPE_UNUSED,
    NGISL_CALLBACK_TYPE_READ,
    NGISL_CALLBACK_TYPE_WRITE,
    NGISL_CALLBACK_TYPE_TIMER,
    NGISL_CALLBACK_TYPE_WAIT
} ngisCallbackType_t;

typedef struct ngislReadCallback_s {
    void                  *ngrc_arg;
    ngisReadCallbackFunc_t ngrc_callback;
    int                    ngrc_fd;
} ngislReadCallback_t;

typedef struct ngislWriteCallback_s {
    void                    *ngwc_arg;
    ngisWriteCallbackFunc_t  ngwc_callback;
    int                      ngwc_fd;
    void                    *ngwc_data;
    size_t                   ngwc_size;
    size_t                   ngwc_nWrite;
} ngislWriteCallback_t;

typedef struct ngislTimerCallback_s {
    void                   *ngtc_arg;
    ngisTimerCallbackFunc_t ngtc_callback;
    time_t                  ngtc_time;
} ngislTimerCallback_t;

typedef struct ngislWaitCallback_s {
    void                  *ngwc_arg;
    ngisWaitCallbackFunc_t ngwc_callback;
    pid_t                  ngwc_pid;
} ngislWaitCallback_t;

struct ngislCallbackEntity_s {
    ngisCallbackType_t ngc_type;
    union {
        ngislReadCallback_t  ngd_read;
        ngislWriteCallback_t ngd_write;
        ngislTimerCallback_t ngd_timer;
        ngislWaitCallback_t  ngd_wait;
    } ngc_dummy;
};

#define ngc_read  ngc_dummy.ngd_read
#define ngc_write ngc_dummy.ngd_write
#define ngc_timer ngc_dummy.ngd_timer
#define ngc_wait  ngc_dummy.ngd_wait

typedef struct ngislCallbackManager_s {
    int                                 ngcm_running;
    int                                 ngcm_nUnused;
    NGIS_LIST_OF(ngislCallbackEntity_t) ngcm_listValid;
    NGIS_LIST_OF(ngislCallbackEntity_t) ngcm_listUnused;
    ngisLog_t                          *ngcm_log;
} ngislCallbackManager_t;

typedef struct ngislWriteStringCallbackArgument_s {
    ngisWriteStringCallbackFunc_t  ngwsca_func;
    void                          *ngwsca_arg;
} ngislWriteStringCallbackArgument_t;

/* File local variables */
static ngislCallbackManager_t ngislCallbackManager;
static int ngislCallbackManagerInitialized = 0;
static int ngislCallbackManagerProcessExitNotifyFd = -1;

/* File local functions */
static void ngislCallbackManagerSigchildHandler(int);

static int ngislCallbackManagerLoop(int);

static int ngislCallbackManagerCheckWaitCallback(int);
static void ngislReadCallbackRead(ngisCallback_t);
static void ngislWriteCallbackWrite(ngisCallback_t);
static void ngislTimerCallbackCall(ngisCallback_t);

static ngislCallbackEntity_t *ngislCallbackEntityCreate();
static void ngislCallbackEntityDestroy(ngislCallbackEntity_t *);

static void ngislCallbackUnused(ngisCallback_t);
static ngisCallback_t ngislCallbackGet();
static void ngislCallbackEntitySetTypeRead(ngislCallbackEntity_t *,
    int, ngisReadCallbackFunc_t, void *arg);
static void ngislCallbackEntitySetTypeWrite(ngislCallbackEntity_t *,
    int, ngisWriteCallbackFunc_t, void *, size_t, void *);
static void ngislCallbackEntitySetTypeTimer(ngislCallbackEntity_t *,
    int, ngisTimerCallbackFunc_t, void *);
static void ngislCallbackEntitySetTypeWait(ngislCallbackEntity_t *,
    pid_t, ngisWaitCallbackFunc_t, void *);

static void ngisWriteStringCallback(
    void *, int, void *, size_t, size_t, ngisCallbackResult_t);

int
ngisCallbackManagerInitialize()
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    static const char fName[] = "ngisCallbackManagerInitialize";

    if (ngislCallbackManagerInitialized != 0) {
        ngisErrorPrint(mng->ngcm_log, fName,
            "Callback manager is already initialized.\n");
        return 0;
    }

    mng->ngcm_nUnused = 0;
    mng->ngcm_running = 0;
    NGIS_LIST_SET_INVALID_VALUE(&mng->ngcm_listValid);
    NGIS_LIST_SET_INVALID_VALUE(&mng->ngcm_listUnused);
    mng->ngcm_log     = NULL;

    NGIS_LIST_INITIALIZE(ngislCallbackEntity_t,  &mng->ngcm_listValid);
    NGIS_LIST_INITIALIZE(ngislCallbackEntity_t,  &mng->ngcm_listUnused);

    mng->ngcm_log = ngisLogCreate("Callback Manager");
    if (mng->ngcm_log == NULL) {
        ngisErrorPrint(NULL, fName, "Can't create the log.\n");
        goto error;
    }
    ngislCallbackManagerInitialized = 1;
    
    return 1;
error:
    NGIS_LIST_FINALIZE(ngislCallbackEntity_t,  &mng->ngcm_listValid);
    NGIS_LIST_FINALIZE(ngislCallbackEntity_t,  &mng->ngcm_listUnused);

    return 0;
}

int
ngisCallbackManagerFinalize()
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngislCallbackEntity_t *entity = NULL;
    int ret = 1;
    int result;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) it;
    static const char fName[] = "ngisCallbackManagerFinalize";

    if (ngislCallbackManagerInitialized == 0) {
        ngisErrorPrint(NULL, fName,
            "Callback manager is not still initialized.\n");
        return 0;
    }
    
    if (mng->ngcm_running != 0) {
        ngisErrorPrint(mng->ngcm_log, fName,
            "Callback manager is running.\n");
        return 0;
    }

    while (!NGIS_LIST_IS_EMPTY(ngislCallbackEntity_t, &mng->ngcm_listValid)) {
        it = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listValid);
        entity = NGIS_LIST_GET(ngislCallbackEntity_t, it);
        ngislCallbackEntityDestroy(entity);
        NGIS_LIST_ERASE(ngislCallbackEntity_t, it);
    }
    NGIS_LIST_FINALIZE(ngislCallbackEntity_t, &mng->ngcm_listValid);

    while (!NGIS_LIST_IS_EMPTY(ngislCallbackEntity_t, &mng->ngcm_listUnused)) {
        it = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listUnused);
        entity = NGIS_LIST_GET(ngislCallbackEntity_t, it);
        ngislCallbackEntityDestroy(entity);
        NGIS_LIST_ERASE(ngislCallbackEntity_t, it);
    }
    NGIS_LIST_FINALIZE(ngislCallbackEntity_t, &mng->ngcm_listUnused);

    result = ngisLogDestroy(mng->ngcm_log);
    if (result == 0) {
        ngisErrorPrint(mng->ngcm_log, fName,
            "Can't destroy the log.\n");
        ret = 0;
    }

    mng->ngcm_nUnused = 0;
    mng->ngcm_running = 0;
    mng->ngcm_log     = NULL;
    ngislCallbackManagerInitialized = 0;
    
    return ret;
}

/**
 * Callback Manager: SigchildHandler
 * Handling the SIGCHLD
 */
static void
ngislCallbackManagerSigchildHandler(
    int signum)
{
    ssize_t nw;
    int result;
#if 0
    static const char fName[] = "ngislCallbackManagerSigchildHandler";
#endif

    if (ngislCallbackManagerProcessExitNotifyFd < 0) {
        return;
    }

    nw = write(ngislCallbackManagerProcessExitNotifyFd, "c", 1);
    if (nw < 0) {
        result = close(ngislCallbackManagerProcessExitNotifyFd);
        if (result < 0 ){
            /* Do nothing */;
        }
        ngislCallbackManagerProcessExitNotifyFd = -1;
    }
    return;
}

int
ngisCallbackManagerRun()
{
    int pfd[2] = {-1, -1};
    int ret = 1;
    int isSignalSet = 0;
    int result;
    int i;
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    struct sigaction sa;
    struct sigaction saOld;
    ngisLog_t *log;
    static const char fName[] = "ngisCallbackManagerRun";

    if (ngislCallbackManagerInitialized == 0) {
        ngisErrorPrint(NULL, fName,
            "Callback manager is not still initialized.\n");
        return 0;
    }
    mng->ngcm_running = 1;
    log = mng->ngcm_log;

    /* Prepare */
    result = ngisNonblockingPipe(pfd);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't create pipes.\n");
        ret = 0;
        goto finalize;
    }

    for (i = 0;i < 2;++i) {
        result = ngisFDsetExecOnCloseFlag(pfd[i]);
        if (result == 0) {
        ngisErrorPrint(log, fName, "Can't set exec-on-close flag\n");
            ret = 0;
            goto finalize;
        }
    }
    ngislCallbackManagerProcessExitNotifyFd = NGIS_PIPE_OUT(pfd);

    /* Set the Signal handler */
    sa.sa_flags   = SA_NOCLDSTOP;
    sa.sa_handler = ngislCallbackManagerSigchildHandler;
    sigemptyset(&sa.sa_mask);
    result = sigaction(SIGCHLD, &sa, &saOld);
    if (result < 0) {
        ngisErrorPrint(log, fName, "sigaction: %s\n", strerror(errno));
        ret = 0;
        goto finalize;
    }
    isSignalSet = 1;

    /* Loop */
    result = ngislCallbackManagerLoop(NGIS_PIPE_IN(pfd));
    if (result == 0) {
        ngisErrorPrint(log, fName, "Error occurred in event loop.\n");
        ret = 0;
        goto finalize;
    }

    /* After process */
finalize:
    if (isSignalSet != 0) {
        /* Reset the Signal handlers */
        result = sigaction(SIGCHLD, &saOld, NULL);
        if (result < 0) {
            ngisErrorPrint(log, fName, "sigaction: %s\n", strerror(errno));
            ret = 0;
        }
    }

    ngislCallbackManagerProcessExitNotifyFd = 0;

    for (i = 0;i < 2;++i) {
        if (pfd[i] >= 0) {
            result = close(pfd[i]);
            if (result < 0) {
                ngisErrorPrint(log, fName, "close: %s\n", strerror(errno));
                ret = 0;
            }
        }
    }
    mng->ngcm_running = 0;

    return ret;
}

static int
ngislCallbackManagerLoop(
    int fd)
{
    int max = 0;
    fd_set fdsRead;
    fd_set fdsWrite;
    struct timeval tv;
    struct timeval *ptv;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) it;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) last;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) itTmp;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) itValid;
    NGIS_LIST_OF(ngislCallbackEntity_t) listTmp;
    ngislCallbackEntity_t *entity = NULL;
    time_t now;
    time_t nextTime;
    int result;
    int ret = 1;
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngisLog_t *log;
    static const char fName[] = "ngislCallbackManagerLoop";

    NGIS_ASSERT(ngislCallbackManagerInitialized != 0);

    log = mng->ngcm_log;

    /* List Init */
    NGIS_LIST_INITIALIZE(ngislCallbackEntity_t, &listTmp);

    while (!NGIS_LIST_IS_EMPTY(ngislCallbackEntity_t, &mng->ngcm_listValid)) {
        /* Merge */
        max = 0;
        FD_ZERO(&fdsRead);
        FD_ZERO(&fdsWrite);

        ptv = NULL;
        nextTime = -1;

        /* Set FD for signal(Wait) */
        FD_SET(fd, &fdsRead);
        max = fd;

        it = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listValid);
        last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listValid); 
        while (it != last) {
            entity = NGIS_LIST_GET(ngislCallbackEntity_t, it);
            switch (entity->ngc_type) {
            case NGISL_CALLBACK_TYPE_READ:
                ngisDebugPrint(log, fName,
                    "Checks fd %d for reading.\n", entity->ngc_read.ngrc_fd);
                FD_SET(entity->ngc_read.ngrc_fd, &fdsRead);
                max = NGIS_MAX(entity->ngc_read.ngrc_fd, max);
                break;
            case NGISL_CALLBACK_TYPE_WRITE:
                ngisDebugPrint(log, fName,
                    "Checks fd %d for writing.\n", entity->ngc_write.ngwc_fd);
                FD_SET(entity->ngc_write.ngwc_fd, &fdsWrite);
                max = NGIS_MAX(entity->ngc_write.ngwc_fd, max);
                break;
            case NGISL_CALLBACK_TYPE_TIMER:
                ngisDebugPrint(log, fName,
                    "Checks timer(%ld) .\n", (long)entity->ngc_timer.ngtc_time);
                if (nextTime < 0) {
                    nextTime = entity->ngc_timer.ngtc_time;
                } else {
                    nextTime = NGIS_MIN(nextTime, entity->ngc_timer.ngtc_time);
                }
                break;
            case NGISL_CALLBACK_TYPE_WAIT:
            case NGISL_CALLBACK_TYPE_UNUSED:
                /* Do nothing */
                break;
            default:
                NGIS_ASSERT_NOTREACHED();
            }
            it = NGIS_LIST_NEXT(ngislCallbackEntity_t, it);
        }

        /* Set Time val */
        if (nextTime >= 0) {
            now = time(NULL);
            NGIS_ASSERT(now >= 0);
            ptv = &tv;
            tv.tv_sec  = NGIS_MAX(nextTime - now, 0);
            tv.tv_usec = 0;
            ngisDebugPrint(log, fName, "Wait %lds.\n", tv.tv_sec);
        }
        max++;

        ngisDebugPrint(log, fName, "Enters in select() (max = %d).\n", max);
        result = select(max, &fdsRead, &fdsWrite, NULL, ptv);
        ngisDebugPrint(log, fName, "Leaves in select().\n");
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            ngisErrorPrint(log, fName, "select: %s.\n", strerror(errno));
            ret = 0;
            goto finalize;
        }

        /* Wait Check */
        if (FD_ISSET(fd, &fdsRead)) {
            result = ngislCallbackManagerCheckWaitCallback(fd);
            if (result == 0) {
                ngisErrorPrint(log, fName,
                    "Error occurred during checking finished process.\n");
                ret = 0;
                goto finalize;
            }
        }

        /* Slice */
        itTmp = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &listTmp);
        it    = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listValid);
        last  = NGIS_LIST_END  (ngislCallbackEntity_t, &mng->ngcm_listValid);
        NGIS_LIST_SPLICE2(ngislCallbackEntity_t, itTmp, it, last);
        itValid = last;

        /* Checking */
        now = time(NULL);
        while (!NGIS_LIST_IS_EMPTY(ngislCallbackEntity_t, &listTmp)) {
            it = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &listTmp);
            NGIS_LIST_SPLICE1(ngislCallbackEntity_t, itValid, it);
            
            entity = NGIS_LIST_GET(ngislCallbackEntity_t, it);
            switch (entity->ngc_type) {
            case NGISL_CALLBACK_TYPE_READ:
                ngisDebugPrint(log, fName,
                    "Checking fd %d for reading.\n", entity->ngc_read.ngrc_fd);
                if (FD_ISSET(entity->ngc_read.ngrc_fd, &fdsRead)) {
                    ngislReadCallbackRead(it);
                }
                break;
            case NGISL_CALLBACK_TYPE_WRITE:
                ngisDebugPrint(log, fName,
                    "Checking fd %d for writing.\n", entity->ngc_write.ngwc_fd);
                if (FD_ISSET(entity->ngc_write.ngwc_fd, &fdsWrite)) {
                    ngislWriteCallbackWrite(it);
                }
                break;
            case NGISL_CALLBACK_TYPE_TIMER:
                ngisDebugPrint(log, fName,
                    "Checking timer(Timer = %ld, Now = %ld).\n",
                    (long)entity->ngc_timer.ngtc_time, (long)now);
                if (entity->ngc_timer.ngtc_time <= now) {
                    ngislTimerCallbackCall(it);
                }
                break;
            case NGISL_CALLBACK_TYPE_WAIT:
                ;/* Do nothing */
                break;
            case NGISL_CALLBACK_TYPE_UNUSED:
            default:
                NGIS_ASSERT_NOTREACHED();
            }
        }
    }
finalize:

    NGIS_LIST_FINALIZE(ngislCallbackEntity_t, &listTmp);

    return ret;
}

static int
ngislCallbackManagerCheckWaitCallback(
    int fd)
{
    ssize_t nread;
    char c = 'e';
    int status;
    ngislCallbackEntity_t *entity;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) it;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) last;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) itTmp;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) itValid;
    NGIS_LIST_OF(ngislCallbackEntity_t) listTmp;
    ngisWaitCallbackFunc_t func;
    void *arg;
    pid_t pid;
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    int ret = 1;
    ngisLog_t *log;
    static const char fName[] = "ngislCallbackManagerCheckWaitCallback";

    NGIS_ASSERT(ngislCallbackManagerInitialized != 0);
    NGIS_ASSERT(fd >= 0);

    log = mng->ngcm_log;

    ngisDebugPrint(log, fName, "Called.\n");

    NGIS_LIST_INITIALIZE(ngislCallbackEntity_t, &listTmp);

    /* Read 1 bytes */
    do {
        nread = read(fd, &c, 1);
        if ((nread < 0) && (errno != EINTR)) {
            if (errno != EAGAIN) {
                ret = 0;
                ngisErrorPrint(log, fName,
                    "read: %s.\n", strerror(errno));
            } 
            goto finalize;
        }
    } while (nread < 0);

    if (nread == 0) {
        ngisErrorPrint(log ,fName, "Unexpected EOF.\n");
        ret = 0;
        goto finalize;
    }

    for (;;) {
        /* Wait Child Process */
        errno = 0;
        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == ECHILD) {
                break;
            }
            ngisErrorPrint(log, fName, "waitpid: %s.\n", strerror(errno));
            ret = 0;
            goto finalize;
        }

        if (pid == 0) {
            break;
        }
        ngisDebugPrint(log, fName, "The finished process's id %ld.\n", (long)pid);

        /* Slice */
        itTmp = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &listTmp);
        it    = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listValid);
        last  = NGIS_LIST_END  (ngislCallbackEntity_t, &mng->ngcm_listValid);
        NGIS_LIST_SPLICE2(ngislCallbackEntity_t, itTmp, it, last);
        itValid = last;

        /* Check Process Waiter */
        while (!NGIS_LIST_IS_EMPTY(ngislCallbackEntity_t, &listTmp)) {
            it = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &listTmp);
            NGIS_LIST_SPLICE1(ngislCallbackEntity_t, itValid, it);

            entity = NGIS_LIST_GET(ngislCallbackEntity_t, it);
            NGIS_ASSERT(entity != NULL);
            if ((entity->ngc_type == NGISL_CALLBACK_TYPE_WAIT) &&
                (entity->ngc_wait.ngwc_pid == pid)) {
                func = entity->ngc_wait.ngwc_callback;
                arg  = entity->ngc_wait.ngwc_arg;

                ngislCallbackUnused(it);
                
                /* Callback */
                func(arg, pid, status, NGIS_CALLBACK_RESULT_SUCCESS);
            }
        }
        /* Not error, if callback isn't found */
    }

finalize:
    NGIS_LIST_INITIALIZE(ngislCallbackEntity_t, &listTmp);

    return ret;
}

int
ngisCallbackIsValid(
    ngisCallback_t callback)
{
    ngislCallbackManager_t *cm = &ngislCallbackManager;
    ngisCallback_t last = NULL;
    ngislCallbackEntity_t *entity = NULL;
#if 0
    static const char fName[] = "ngisCallbackIsValid";
#endif
    if (!NGIS_LIST_ITERATOR_IS_VALID(ngislCallbackEntity_t, callback)) {
        return 0;
    }

    last = NGIS_LIST_END(ngislCallbackEntity_t, &cm->ngcm_listUnused);
    if (callback == last) {
        return 0;
    }

    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    if (entity->ngc_type == NGISL_CALLBACK_TYPE_UNUSED) {
        return 0;
    }
    return 1;
}

static void
ngislReadCallbackRead(
    ngisCallback_t callback)
{
    ngislCallbackEntity_t *entity;
    ngisReadCallbackFunc_t func;
    ngisCallbackResult_t cResult;
    char buffer[NGISL_READ_CALLBACK_BUFFER_SIZE] = "";
    void *data;
    ssize_t nr;
    size_t nRead = 0;
    void *arg;
    int fd;
    ngisLog_t *log;
    static const char fName[] = "ngislReadCallbackRead";

    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    log    = ngislCallbackManager.ngcm_log;

    NGIS_ASSERT(entity->ngc_type == NGISL_CALLBACK_TYPE_READ);

    func    = entity->ngc_read.ngrc_callback;
    arg     = entity->ngc_read.ngrc_arg;
    fd      = entity->ngc_read.ngrc_fd;
    data    = NULL;
    nRead   = 0;
    cResult = NGIS_CALLBACK_RESULT_SUCCESS;

    do {
        nr = read(fd, buffer, NGISL_READ_CALLBACK_BUFFER_SIZE);
    } while ((nr < 0) && (errno == EINTR));

    if (nr < 0) {
        if (errno == EAGAIN) {
            return;
        }
        ngisErrorPrint(log, fName, "read: %s", strerror(errno));
        /* Error */
        cResult = NGIS_CALLBACK_RESULT_FAILED;
    } else if (nr == 0) {
        /* EOF */
        cResult = NGIS_CALLBACK_RESULT_EOF;
    } else {
        /* Read Success */
        data  = buffer;
        nRead = nr;
    }
    ngislCallbackUnused(callback);

    func(arg, fd, data, nRead, cResult);

    return;
}

static void
ngislWriteCallbackWrite(
    ngisCallback_t callback)
{
    ngislCallbackEntity_t *entity;
    ngisWriteCallbackFunc_t func;
    ngisCallbackResult_t cResult;
    char *data;
    ssize_t nw;
    size_t nWrite;
    size_t size;
    void *arg;
    int fd;
    ngisLog_t *log;
    static const char fName[] = "ngislWriteCallbackWrite";

    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    log    = ngislCallbackManager.ngcm_log;

    NGIS_ASSERT(entity->ngc_type == NGISL_CALLBACK_TYPE_WRITE);

    func    = entity->ngc_write.ngwc_callback;
    arg     = entity->ngc_write.ngwc_arg;
    fd      = entity->ngc_write.ngwc_fd;
    data    = (char *)entity->ngc_write.ngwc_data;
    size    = entity->ngc_write.ngwc_size;
    nWrite  = entity->ngc_write.ngwc_nWrite;
    cResult = NGIS_CALLBACK_RESULT_SUCCESS;

    do {
        NGIS_ASSERT(size > nWrite);
        nw = write(fd, &data[nWrite], size - nWrite);
    } while ((nw < 0) && (errno == EINTR));

    if (nw < 0) {
        if (errno == EAGAIN) {
            return;
        }
        /* Error */
        ngisErrorPrint(log, fName, "write: %s.\n", strerror(errno));
        cResult = NGIS_CALLBACK_RESULT_FAILED;
    } else if (nw == 0) {
        cResult = NGIS_CALLBACK_RESULT_EOF;
    } else {
        /* Write Success */
        nWrite = nWrite + nw;
        entity->ngc_write.ngwc_nWrite = nWrite;
        if (nWrite < size) {
            /* Continue */
            return;
        }
    }
    ngislCallbackUnused(callback);
    func(arg, fd, data, size, nWrite, cResult);

    return;
}

static void
ngislTimerCallbackCall(
    ngisCallback_t callback)
{
    ngisTimerCallbackFunc_t func;
    void *arg;
    ngislCallbackEntity_t *entity;
#if 0
    static const char fName[] = "ngislTimerCallbackCall";
#endif

    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);

    func = entity->ngc_timer.ngtc_callback;
    arg  = entity->ngc_timer.ngtc_arg;

    ngislCallbackUnused(callback);

    func(arg, NGIS_CALLBACK_RESULT_SUCCESS);

    return;
}

static ngislCallbackEntity_t *
ngislCallbackEntityCreate()
{
    ngislCallbackEntity_t *new = NULL;
    static const char fName[] = "ngislCallbackEntityCreate";

    new = NGIS_ALLOC(ngislCallbackEntity_t);
    if (new == NULL) {
        ngisErrorPrint(ngislCallbackManager.ngcm_log, fName,
            "Can't allocate storage for callback entity.\n");

        return NULL;
    }
    memset(new, '\0', sizeof(*new));
    new->ngc_type = NGISL_CALLBACK_TYPE_UNUSED;

    return new;
}

static void
ngislCallbackEntityDestroy(
    ngislCallbackEntity_t *callback)
{
#if 0
    static const char fName[] = "ngislCallbackEntityDestroy";
#endif

    NGIS_ASSERT(callback != NULL);

    memset(callback, '\0', sizeof(*callback));
    callback->ngc_type = NGISL_CALLBACK_TYPE_UNUSED;
    NGIS_FREE(callback);

    return;
}

ngisCallback_t
ngisCallbackRead(
    int fd,
    ngisReadCallbackFunc_t func,
    void *arg)
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngislCallbackEntity_t *entity;
    ngisCallback_t callback;
    ngisCallback_t last;
    ngisLog_t *log;
    static const char fName[] = "ngisCallbackRead";

    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(func != NULL);

    log = ngislCallbackManager.ngcm_log;
    ngisDebugPrint(log, fName,
        "Registers callback for reading from fd %d\n", fd);

    last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);
    callback = ngislCallbackGet();
    if (callback == last) {
        ngisErrorPrint(log, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    ngislCallbackEntitySetTypeRead(entity, fd, func, arg);

    return callback;
}

ngisCallback_t
ngisCallbackWrite(
    int fd,
    ngisWriteCallbackFunc_t func,
    void *data,
    size_t size,
    void *arg)
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngisCallback_t callback;
    ngisCallback_t last;
    ngisLog_t *log;
    ngislCallbackEntity_t *entity;
    static const char fName[] = "ngisCallbackWrite";

    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(func != NULL);

    log = ngislCallbackManager.ngcm_log;
    ngisDebugPrint(log, fName,
        "Registers callback for writing to fd %d\n", fd);

    last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);
    callback = ngislCallbackGet();
    if (callback == last) {
        ngisErrorPrint(log, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    ngislCallbackEntitySetTypeWrite(entity, fd, func, data, size, arg);

    return callback;
}

ngisCallback_t
ngisCallbackWriteVformat(
    int fd,
    ngisWriteStringCallbackFunc_t func,
    void *arg, 
    const char *format,
    va_list ap)
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    char *string = NULL;
    ngisCallback_t callback = NULL;
    ngisCallback_t last;
    ngislWriteStringCallbackArgument_t *callbackArgument = NULL;
    ngisLog_t *log;
    static const char fName[] = "ngisCallbackWriteVformat";

    log = ngislCallbackManager.ngcm_log;
    last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);

    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(func != NULL);
    NGIS_ASSERT(NGIS_STRING_IS_NONZERO(format));

    callbackArgument = NGIS_ALLOC(ngislWriteStringCallbackArgument_t);
    if (callbackArgument == NULL) {
        ngisErrorPrint(log, fName,
            "Can't allocate for storage for a argument of callback.\n");
        goto error;
    }
    callbackArgument->ngwsca_func = func;
    callbackArgument->ngwsca_arg  = arg;

    string = ngisStrdupVprintf(format, ap);
    if (string == NULL) {
        ngisErrorPrint(log, fName,
            "Can't allocate storage for the string.\n");
        goto error;
    }    
    ngisDebugPrint(log, fName, "Writing string \"%s\".\n", string);

    callback = ngisCallbackWrite(
        fd, ngisWriteStringCallback, string, strlen(string), callbackArgument);
    if (!ngisCallbackIsValid(callback)) {
        ngisErrorPrint(log, fName,
            "Can't register callback for writing string.\n");
        goto error;
    }
    return callback;

error:
    NGIS_NULL_CHECK_AND_FREE(callbackArgument);
    NGIS_NULL_CHECK_AND_FREE(string);

    return last;
}

ngisCallback_t
ngisCallbackWriteFormat(
    int fd,
    ngisWriteStringCallbackFunc_t func,
    void *arg, 
    const char *format,
    ...)
{
    ngisCallback_t callback;
    va_list ap;
#if 0
    static const char fName[] = "ngisCallbackWriteFormat";
#endif

    va_start(ap, format);
    callback = ngisCallbackWriteVformat(fd, func, arg, format, ap);
    va_end(ap);

    return callback;
}

static void
ngisWriteStringCallback(
    void *arg,
    int fd,
    void *string,
    size_t bufferSize,
    size_t nWrite,
    ngisCallbackResult_t cResult)
{
    ngislWriteStringCallbackArgument_t *callbackArgument = arg;
#if 0
    static const char fName[] = "ngisWriteStringCallback";
#endif

    free(string);

    callbackArgument = arg;
    callbackArgument->ngwsca_func(callbackArgument->ngwsca_arg, fd, cResult);
    free(arg);

    return;
}

ngisCallback_t
ngisCallbackSetTimer(
    int waitTime,
    ngisTimerCallbackFunc_t func,
    void *arg)
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngisCallback_t callback;
    ngisCallback_t last;
    ngislCallbackEntity_t *entity;
    ngisLog_t *log;
    static const char fName[] = "ngisCallbackSetTimer";

    log = ngislCallbackManager.ngcm_log;
    last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);

    NGIS_ASSERT(waitTime >= 0);
    NGIS_ASSERT(func != NULL);

    ngisDebugPrint(log, fName, "Callback after %ds.\n", waitTime);

    callback = ngislCallbackGet();
    if (callback == last) {
        ngisErrorPrint(log, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    ngislCallbackEntitySetTypeTimer(entity, waitTime, func, arg);

    return callback;
}

ngisCallback_t
ngisCallbackWait(
    pid_t pid,
    ngisWaitCallbackFunc_t func,
    void *arg)
{
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    ngisCallback_t callback;
    ngisCallback_t last;
    ngislCallbackEntity_t *entity = NULL;
    static const char fName[] = "ngisCallbackWait";

    callback = ngislCallbackGet();
    last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);
    if (callback == last) {
        ngisErrorPrint(NULL, fName, "Can't get callback.\n");
        return last;
    }
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    ngislCallbackEntitySetTypeWait(entity, pid, func, arg);

    return callback;
}

void
ngisCallbackCancel(
    ngisCallback_t callback)
{
    ngislCallbackEntity_t copy;
    ngislCallbackEntity_t *entity;
    ngisLog_t *log;
    static const char fName[] = "ngisCallbackCancel";

    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);
    copy   = *entity;
    log    = ngislCallbackManager.ngcm_log;

    ngislCallbackUnused(callback);

    switch (copy.ngc_type) {
    case NGISL_CALLBACK_TYPE_READ:
        ngisDebugPrint(log, fName,
            "Cancel reading(fd=%d).\n", copy.ngc_read.ngrc_fd);
        copy.ngc_read.ngrc_callback(
            copy.ngc_read.ngrc_arg,
            copy.ngc_read.ngrc_fd,
            NULL, 0,
            NGIS_CALLBACK_RESULT_CANCEL);
        break;
    case NGISL_CALLBACK_TYPE_WRITE:
        ngisDebugPrint(log, fName,
            "Cancel writing(fd=%d).\n", copy.ngc_write.ngwc_fd);
        copy.ngc_write.ngwc_callback(
            copy.ngc_write.ngwc_arg,
            copy.ngc_write.ngwc_fd,
            copy.ngc_write.ngwc_data,
            copy.ngc_write.ngwc_nWrite,
            copy.ngc_write.ngwc_size,
            NGIS_CALLBACK_RESULT_CANCEL);
        break;
    case NGISL_CALLBACK_TYPE_TIMER:
        ngisDebugPrint(log, fName,
            "Cancel Timer(time=%ld).\n", (long)entity->ngc_timer.ngtc_time);
        copy.ngc_timer.ngtc_callback(
            copy.ngc_timer.ngtc_arg,
            NGIS_CALLBACK_RESULT_CANCEL);
        break;
    case NGISL_CALLBACK_TYPE_WAIT:
        ngisDebugPrint(log, fName,
            "Cancel waiting process(pid=%ld).\n", (long)copy.ngc_wait.ngwc_pid);
        copy.ngc_wait.ngwc_callback(
            copy.ngc_wait.ngwc_arg,
            copy.ngc_wait.ngwc_pid, 0,
            NGIS_CALLBACK_RESULT_CANCEL);
        break;
    case NGISL_CALLBACK_TYPE_UNUSED:
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    return;
}

static void
ngislCallbackUnused(
    ngisCallback_t callback)
{
    ngislCallbackEntity_t *entity;
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) last;
    ngisLog_t *log;
    static const char fName[] = "ngislCallbackUnused";

    NGIS_ASSERT(callback != NULL);

    log    = ngislCallbackManager.ngcm_log;
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);

    switch (entity->ngc_type) {
    case NGISL_CALLBACK_TYPE_READ:
        ngisDebugPrint(log, fName,
            "Unused Read Callback(fd=%d).\n", entity->ngc_read.ngrc_fd);
        break;
    case NGISL_CALLBACK_TYPE_WRITE:
        ngisDebugPrint(log, fName,
            "Unused Write Callback(fd=%d).\n", entity->ngc_write.ngwc_fd);
        break;
    case NGISL_CALLBACK_TYPE_TIMER:
        ngisDebugPrint(log, fName,
            "Unused Timer Callback(time=%ld).\n", (long)entity->ngc_timer.ngtc_time);
        break;
    case NGISL_CALLBACK_TYPE_WAIT:
        ngisDebugPrint(log, fName,
            "Unused Wait Callback(pid=%ld).\n", (long)entity->ngc_wait.ngwc_pid);
        break;
    default:
        NGIS_ASSERT_NOTREACHED();
    }
    
    if (mng->ngcm_nUnused < NGISL_CALLBACK_MANAGER_MAX_UNUSED_COUNT ) {
        /* Move list of unused callbacks */
        mng->ngcm_nUnused++;

        memset(entity, '\0', sizeof(*entity));
        entity->ngc_type = NGISL_CALLBACK_TYPE_UNUSED;

        last = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);
        NGIS_LIST_SPLICE1(ngislCallbackEntity_t, last, callback);
    } else {
        /* Remove */
        ngislCallbackEntityDestroy(entity);
        NGIS_LIST_ERASE(ngislCallbackEntity_t, callback);
    }

    return;
}

/**
 * Return last of list of unused callback, If error is occurred.
 */
static ngisCallback_t
ngislCallbackGet()
{
    ngislCallbackEntity_t *entity = NULL;
    ngislCallbackEntity_t *newEntity = NULL;
    ngislCallbackManager_t *mng = &ngislCallbackManager;
    int result;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) first;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) last;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) it;
    NGIS_LIST_ITERATOR_OF(ngislCallbackEntity_t) callback;
    
    first = NGIS_LIST_BEGIN(ngislCallbackEntity_t, &mng->ngcm_listUnused);
    last  = NGIS_LIST_END(ngislCallbackEntity_t, &mng->ngcm_listUnused);
    callback = last;
    if (first == last) {
        /* Empty */
        newEntity = ngislCallbackEntityCreate();
        if (newEntity == NULL) {
            goto finalize;
        }

        result = NGIS_LIST_INSERT_HEAD(ngislCallbackEntity_t,
            &mng->ngcm_listValid, newEntity);
        if (result == 0) {
            goto finalize;
        }
        newEntity = NULL;
    } else {
        NGIS_ASSERT(mng->ngcm_nUnused > 0);
        mng->ngcm_nUnused--;

        it = NGIS_LIST_BEGIN(ngislCallbackEntity_t,
            &mng->ngcm_listValid);
        NGIS_LIST_SPLICE1(ngislCallbackEntity_t, it, first);
    }

    /* Check Only */
    callback = NGIS_LIST_BEGIN(ngislCallbackEntity_t,
            &mng->ngcm_listValid);
    entity = NGIS_LIST_GET(ngislCallbackEntity_t, callback);

    NGIS_ASSERT(entity != NULL);
    NGIS_ASSERT(entity->ngc_type == NGISL_CALLBACK_TYPE_UNUSED);

finalize:
    if (newEntity != NULL) {
        ngislCallbackEntityDestroy(newEntity);
    }
    return callback;
}

static void
ngislCallbackEntitySetTypeRead(
    ngislCallbackEntity_t *entity,
    int fd,
    ngisReadCallbackFunc_t func,
    void *arg)
{
    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(func != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGISL_CALLBACK_TYPE_READ;
    entity->ngc_read.ngrc_fd       = fd;
    entity->ngc_read.ngrc_callback = func;
    entity->ngc_read.ngrc_arg      = arg;

    return;
}

static void
ngislCallbackEntitySetTypeWrite(
    ngislCallbackEntity_t *entity,
    int fd,
    ngisWriteCallbackFunc_t func,
    void *data,
    size_t size,
    void *arg)
{
    NGIS_ASSERT(entity != NULL);
    NGIS_ASSERT(fd >= 0);
    NGIS_ASSERT(func != NULL);
    NGIS_ASSERT(size > 0);
    NGIS_ASSERT(data != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGISL_CALLBACK_TYPE_WRITE;
    entity->ngc_write.ngwc_fd       = fd;
    entity->ngc_write.ngwc_callback = func;
    entity->ngc_write.ngwc_arg      = arg;
    entity->ngc_write.ngwc_nWrite   = 0U;
    entity->ngc_write.ngwc_size     = size;
    entity->ngc_write.ngwc_data     = data;

    return;
}

static void
ngislCallbackEntitySetTypeTimer(
    ngislCallbackEntity_t *entity,
    int waitTime,
    ngisTimerCallbackFunc_t func,
    void *arg)
{
    time_t now;
    ngisLog_t *log;
    static const char fName[] = "ngislCallbackEntitySetTypeTimer";

    NGIS_ASSERT(entity != NULL);
    NGIS_ASSERT(waitTime >= 0);
    NGIS_ASSERT(func != NULL);

    log = ngislCallbackManager.ngcm_log;
    now = time(NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGISL_CALLBACK_TYPE_TIMER;
    entity->ngc_timer.ngtc_time     = now + waitTime;
    entity->ngc_timer.ngtc_callback = func;
    entity->ngc_timer.ngtc_arg      = arg;

    ngisDebugPrint(log, fName, "waitTime    = %d.\n", waitTime);
    ngisDebugPrint(log, fName, "timeoutTime = %ld.\n",
        (long)entity->ngc_timer.ngtc_time);

    NGIS_ASSERT(entity->ngc_timer.ngtc_time >= now);

    return;
}

static void
ngislCallbackEntitySetTypeWait(
    ngislCallbackEntity_t *entity,
    pid_t pid,
    ngisWaitCallbackFunc_t func,
    void * arg)
{
#if 0
    static const char fName[] = "ngislCallbackEntitySetTypeWait";
#endif
    NGIS_ASSERT(entity != NULL);
    NGIS_ASSERT(pid > 0);
    NGIS_ASSERT(func != NULL);

    memset(entity, '\0', sizeof(*entity));

    entity->ngc_type = NGISL_CALLBACK_TYPE_WAIT;
    entity->ngc_wait.ngwc_pid      = pid;
    entity->ngc_wait.ngwc_callback = func;
    entity->ngc_wait.ngwc_arg      = arg;

    return;
}
