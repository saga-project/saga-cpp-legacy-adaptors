/*
 * $RCSfile: ngrcSocket.c,v $ $Revision: 1.5 $ $Date: 2008/02/25 10:17:26 $
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

#include "ngemEnvironment.h"
#include "ngUtility.h"
#include "ngemUtility.h"
#include "ngemLog.h"
#include "ngrcGT.h"

NGI_RCSID_EMBED("$RCSfile: ngrcSocket.c,v $ $Revision: 1.5 $ $Date: 2008/02/25 10:17:26 $")

static ngrcSocket_t *ngrclSocketCreate(int);

/**
 * Socket: Create listener.
 */
ngrcSocket_t *
ngrcSocketCreateListener(
    bool tcpNodelay)
{
    ngLog_t *log;
    int result;
    int fd = -1;
    struct sockaddr_in addr; 
    ngrcSocket_t *sock = NULL;
    int nodelay;
    NGEM_FNAME(ngrcSocketCreateListener);

    log = ngemLogGetDefault();

    /* TODO: Local socket */
    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "socket: %s.\n", strerror(errno));
        goto error;
    }
    memset(&addr, '\0', sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = htons(0); /* any */

    result = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (result < 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "bind: %s.\n", strerror(errno));
        goto error;
    }

    if (tcpNodelay) {
        nodelay = 1;
        result = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "setsockopt(TCP_NODELAY): %s.\n", strerror(errno));
            goto error;
        }
    }

    /* backlog is 1 because Remote Communication Proxy handles
     * a connection only.*/
    result = listen(fd, 1);
    if (result < 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "listen: %s.\n", strerror(errno));
        goto error;
    }

    sock = ngrclSocketCreate(fd);
    if (sock == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create socket.\n");
        goto error;
    }

    return sock;
error:
    if (fd >= 0) {
        result = close(fd);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
        }
    }

    return NULL;
}

/**
 * Socket(Listener only): Get contact string.
 */
char *
ngrcSocketGetContactString(
    ngrcSocket_t *sock)
{
    ngLog_t *log;
    struct sockaddr_in addr; 
    char *ret = NULL;
    int result;
    ngiSockLen_t addr_len = sizeof(addr);
    NGEM_FNAME(ngrcSocketGetContactString);

    NGEM_ASSERT(sock != NULL);

    log = ngemLogGetDefault();

    /* TODO: Local socket */
    result = getsockname(sock->ngs_fd, (struct sockaddr *)&addr, &addr_len);
    if (result < 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "getsockname: %s\n", strerror(errno));
        return NULL;
    }

    NGEM_ASSERT(addr.sin_family == AF_INET);

    /* Remote Communication Proxy uses local address only. */
    ret = ngemStrdupPrintf("ng_tcp://localhost:%d/",
        ntohs(addr.sin_port));
    if (ret == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create address string.\n");
        return NULL;
    }
    return ret;
}

/**
 * Socket: accept
 */
ngrcSocket_t *
ngrcSocketAccept(
    ngrcSocket_t *listener,
    bool *canceled)
{
    ngrcSocket_t *sock = NULL;
    int fd = -1;
    fd_set rfds;
    int result;
    ngLog_t *log;
    int fdmax;
    struct sockaddr_in addr; 
    ngiSockLen_t addr_len = sizeof(addr);
    NGEM_FNAME(ngrcSocketAccept);

    NGEM_ASSERT(listener != NULL);
    NGEM_ASSERT(canceled != NULL);

    *canceled = false;

    log = ngemLogGetDefault();

    while (fd < 0) {
        FD_ZERO(&rfds);
        FD_SET(listener->ngs_fd, &rfds);
        FD_SET(NGEM_PIPE_IN(listener->ngs_pipe), &rfds);

        fdmax = NGEM_MAX(listener->ngs_fd, NGEM_PIPE_IN(listener->ngs_pipe)) + 1;

        result = select(fdmax, &rfds, NULL, NULL, NULL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "select: %s\n", strerror(errno));
            goto error;
        }

        if (FD_ISSET(NGEM_PIPE_IN(listener->ngs_pipe), &rfds)) {
            /* Not continue */
            *canceled = true;
            return NULL;
        }

        if (FD_ISSET(listener->ngs_fd, &rfds)) {
            fd = accept(listener->ngs_fd, (struct sockaddr *)&addr, &addr_len);
            if (result < 0) {
                if (errno == EAGAIN) {continue;}
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "accept: %s\n", strerror(errno));
                if (errno == EBADF) {
                    return NULL;
                }
                continue;
            }
        }
    }

    sock = ngrclSocketCreate(fd);
    if (sock == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create socket.\n");
        goto error;
    }

    return sock;
error:
    if (fd >= 0) {
        result = close(fd);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
        }
    }

    return NULL;
}

/**
 * Socket: duplicate
 */
ngrcSocket_t *
ngrcSocketDup(
    ngrcSocket_t *src)
{
    ngLog_t *log;
    ngrcSocket_t *sock = NULL;
    int fd = -1;
    int result;
    NGEM_FNAME(ngrcSocketDup);

    log = ngemLogGetDefault();

    fd = dup(src->ngs_fd);
    if (fd < 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "dup: %s.\n", strerror(errno));
        goto error;
    }

    sock = ngrclSocketCreate(fd);
    if (sock == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create socket.\n");
        goto error;
    }

    return sock;
error:
    if (fd >= 0) {
        result = close(fd);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
        }
    }

    return NULL;
}

/**
 * Socket: Read
 * if nread == 0, EOF or Canceled.
 */
ngemResult_t
ngrcSocketRead(
    ngrcSocket_t *sock,
    void *buf,
    size_t size,
    size_t *nread)
{
    fd_set rfds;
    int result;
    ngLog_t *log;
    int fdmax;
    ssize_t nr = -1;
    NGEM_FNAME(ngrcSocketRead);

    NGEM_ASSERT(sock != NULL);
    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(size > 0);
    NGEM_ASSERT(nread != NULL);

    *nread = 0;

    log = ngemLogGetDefault();

    while (nr < 0) {
        FD_ZERO(&rfds);
        FD_SET(sock->ngs_fd, &rfds);
        FD_SET(NGEM_PIPE_IN(sock->ngs_pipe), &rfds);

        fdmax = NGEM_MAX(sock->ngs_fd, NGEM_PIPE_IN(sock->ngs_pipe)) + 1;

        result = select(fdmax, &rfds, NULL, NULL, NULL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "select: %s\n", strerror(errno));
            return NGEM_FAILED;
        }

        if (FD_ISSET(NGEM_PIPE_IN(sock->ngs_pipe), &rfds)) {
            /* Not continue */
            return NGEM_SUCCESS;
        }

        if (FD_ISSET(sock->ngs_fd, &rfds)) {
            nr = read(sock->ngs_fd, buf, size);
            if (nr < 0) {
                if (errno == EAGAIN) {continue;}
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "read: %s\n", strerror(errno));
                return NGEM_SUCCESS;
            }
        }
    }
    *nread = nr;
    return NGEM_SUCCESS;
}

/**
 * Socket:Write 
 * if nwrite < size, Canceled.
 */
ngemResult_t
ngrcSocketWrite(
    ngrcSocket_t *sock,
    void *buf,
    size_t size,
    size_t *nwrite)
{
    fd_set rfds;
    fd_set wfds;
    int result;
    ngLog_t *log;
    int fdmax;
    ssize_t nw = 0;
    size_t sum = 0;
    NGEM_FNAME(ngrcSocketWrite);

    NGEM_ASSERT(sock != NULL);
    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(size > 0);
    NGEM_ASSERT(nwrite != NULL);

    *nwrite = 0;

    log = ngemLogGetDefault();

    while (sum < size) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sock->ngs_fd, &wfds);
        FD_SET(NGEM_PIPE_IN(sock->ngs_pipe), &rfds);

        fdmax = NGEM_MAX(sock->ngs_fd, NGEM_PIPE_IN(sock->ngs_pipe)) + 1;

        result = select(fdmax, &rfds, &wfds, NULL, NULL);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "select: %s\n", strerror(errno));
            return NGEM_FAILED;
        }

        if (FD_ISSET(NGEM_PIPE_IN(sock->ngs_pipe), &rfds)) {
            /* Not continue */
            *nwrite = sum;
            return NGEM_SUCCESS;
        }

        if (FD_ISSET(sock->ngs_fd, &wfds)) {
            nw = write(sock->ngs_fd, ((char *)buf)+sum, size-sum);
            if (nw < 0) {
                if (errno == EAGAIN) {continue;}
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "write: %s\n", strerror(errno));
                return NGEM_FAILED;
            }
            sum += nw;
            NGEM_ASSERT(sum <= size);
        }
    }
    *nwrite = sum;

    return NGEM_SUCCESS;
}

/**
 * Socket: Cancels operation(read and write).
 */
ngemResult_t 
ngrcSocketCancel(
    ngrcSocket_t *sock)
{
    int result;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngrcSocketCancel);

    log = ngemLogGetDefault();

    result = write(NGEM_PIPE_OUT(sock->ngs_pipe), "C", 1);
    if (result <= 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "write: %s\n", strerror(errno));
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Socket: Destroy
 */
ngemResult_t
ngrcSocketDestroy(
    ngrcSocket_t *sock)
{
    ngemResult_t ret = NGEM_SUCCESS;
    int result;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngrcSocketDestroy);

    log = ngemLogGetDefault();

    if (sock == NULL) {
        return NGEM_SUCCESS;
    }
 
    if (sock->ngs_fd >= 0) {
        result = close(sock->ngs_fd);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
            ret = NGEM_FAILED;
        }
        sock->ngs_fd = -1;
    }

    if (sock->ngs_pipe[0] >= 0) {
        result = close(sock->ngs_pipe[0]);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
            ret = NGEM_FAILED;
        }
        sock->ngs_pipe[0] = -1;
    }

    if (sock->ngs_pipe[1] >= 0) {
        result = close(sock->ngs_pipe[1]);
        if (result < 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "close: %s.\n", strerror(errno));
            ret = NGEM_FAILED;
        }
        sock->ngs_pipe[1] = -1;
    }

    NGI_DEALLOCATE(ngrcSocket_t, sock, log, NULL);

    return ret;
}

/*
 * Socket: Create(common) 
 */
static ngrcSocket_t *
ngrclSocketCreate(int fd)
{
    ngLog_t *log;
    ngemResult_t nResult;
    ngrcSocket_t *sock = NULL;
    NGEM_FNAME(ngrclSocketCreate);

    log = ngemLogGetDefault();

    sock = NGI_ALLOCATE(ngrcSocket_t, log, NULL);
    if (sock == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't allocate the storage for socket.\n.");
        goto error;
    }
    sock->ngs_fd      = fd; 
    sock->ngs_pipe[0] = -1;
    sock->ngs_pipe[1] = -1;

    nResult = ngemFDsetNonblockFlag(sock->ngs_fd);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't set non-block flag to fd.\n");
        goto error;
    }

    nResult = ngemNonblockingPipe(sock->ngs_pipe);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create the pipe.\n");
        goto error;
    }

    return sock;
error:
    nResult = ngrcSocketDestroy(sock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the socket.\n");
    }
    sock = NULL;
    return NULL;
}
