/*
 * $RCSfile: ngemUtility.c,v $ $Revision: 1.11 $ $Date: 2008/03/28 03:52:31 $
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

#include "ngemUtility.h"
#include "ngemLog.h"
#include <unistd.h>

NGI_RCSID_EMBED("$RCSfile: ngemUtility.c,v $ $Revision: 1.11 $ $Date: 2008/03/28 03:52:31 $")

#define NGEML_MAX_ARGS_SIZE 4

static size_t ngemlTokenAnalyzerGetTokenLength(ngemTokenAnalyzer_t *);

/**
 * This function is like popen().
 * Invokes "command", and returns pipes connected to stdin, stdout
 * and stderror of new process.
 */
pid_t
ngemPopen(
    ngemStandardIO_t *io,
    char *command)
{
    char *args[NGEML_MAX_ARGS_SIZE];
    int i = 0;
#if 0
    static const char fName[] = "ngemPopen";
#endif
    
    NGEM_ASSERT(io != NULL);
    NGEM_ASSERT(command != NULL);
    
    i = 0;
    if (strcmp(command, NGEM_SHELL) == 0) {
        args[i++] = NGEM_SHELL;
        args[i++] = NULL;
    } else {
        args[i++] = NGEM_SHELL;
        args[i++] = "-c";
        args[i++] = command;
        args[i++] = NULL;
    }
    NGEM_ASSERT(i <= NGEML_MAX_ARGS_SIZE);

    return ngemPopenArgv(io, args);
}

/**
 * Invokes "command", and sets new process's stdin,
 * stdout and stderr to value of "sio".
 */
pid_t
ngemInvokeProcess(
    ngemStandardIO_t *sio,
    char *command)
{
    char *args[NGEML_MAX_ARGS_SIZE];
    int i;
#if 0
    static const char fName[] = "ngemInvokeProcess";
#endif
    
    NGEM_ASSERT(sio != NULL);
    NGEM_ASSERT(command != NULL);
    
    i = 0;
    if (strcmp(command, NGEM_SHELL) == 0) {
        args[i++] = NGEM_SHELL;
        args[i++] = NULL;
    } else {
        args[i++] = NGEM_SHELL;
        args[i++] = "-c";
        args[i++] = command;
        args[i++] = NULL;
    }
    NGEM_ASSERT(i <= NGEML_MAX_ARGS_SIZE);

    return ngemInvokeProcessArgv(sio, args);
}

/**
 * Invokes new process, and returns pipes connected to stdin, stdout
 * and stderror of new process.
 * "argv" is null-terminated array executable file and arguments.
 */
pid_t
ngemPopenArgv(
    ngemStandardIO_t *io,
    char **argv)
{
    return ngemPopenArgvEx(io, argv, 0, 0, NULL);
}

/**
 * Invokes new process, and sets new process's stdin,
 * stdout and stderr to value of "sio".
 * "argv" is null-terminated array executable file and arguments.
 *
 * If "io" is NULL, new process uses parent's stdin, stdout, stderr.
 */
pid_t
ngemInvokeProcessArgv(
    ngemStandardIO_t *io,
    char **argv)
{
    return ngemInvokeProcessArgvEx(io, argv, 0, 0, NULL);
}

pid_t
ngemInvokeProcessArgvEx(
    ngemStandardIO_t *io,
    char **argv,
    uid_t uid,
    gid_t gid,
    ngemEnvironment_t *env)
{
    pid_t pid = -1;
    int result;
    char *sCall = NULL;
    ngLog_t *log;
    char *str;
    static const char fName[] = "ngemInvokeProcessArgvEx";

    log = ngemLogGetDefault();
    
    NGEM_ASSERT(argv != NULL);

    pid = fork();
    if (pid < 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "fork: %s.\n", strerror(errno));
        goto error;
    }
    if (pid == 0) {
        /* Child Process*/
        if (gid != 0) {
            sCall = "setgid";
            result = setgid(gid);
            if (result < 0) {
                goto child_error;
            }
        }
        if (uid != 0) {
            sCall = "setuid";
            result = setuid(uid);
            if (result < 0) {
                goto child_error;
            }
        }

        if (env != NULL) {
            for (;env->nge_name != NULL;env++) {
                sCall = "ngemStrdupPrintf";
                str = ngemStrdupPrintf("%s=%s", env->nge_name, env->nge_value);
                if (str == NULL) {
                    goto child_error;
                }
                result = putenv(str);
                if (result < 0) {
                    goto child_error;
                }
            }
        }

        /* STDIN */
        if (io != NULL) {
            NGEM_ASSERT(io->ngsio_in != 0);
            NGEM_ASSERT(io->ngsio_in != 1);
            NGEM_ASSERT(io->ngsio_in != 2);

            NGEM_ASSERT(io->ngsio_out != 0);
            NGEM_ASSERT(io->ngsio_out != 1);
            NGEM_ASSERT(io->ngsio_out != 2);

            NGEM_ASSERT(io->ngsio_error != 0);
            NGEM_ASSERT(io->ngsio_error != 1);
            NGEM_ASSERT(io->ngsio_error != 2);

            sCall = "dup2";
            result = dup2(io->ngsio_in, 0);
            if (result < 0) {
                goto child_error;
            }
            sCall = "close";
            result = close(io->ngsio_in);
            if (result < 0) {
                goto child_error;
            }
            /* STDOUT */        
            sCall = "dup2";
            result = dup2(io->ngsio_out, 1);
            if (result < 0) {
                goto child_error;
            }
            sCall = "close";
            result = close(io->ngsio_out);
            if (result < 0) {
                goto child_error;
            }
            /* STDERR */
            sCall = "dup2";
            result = dup2(io->ngsio_error, 2);
            if (result < 0) {
                goto child_error;
            }
            sCall = "close";
            result = close(io->ngsio_error);
            if (result < 0) {
                goto child_error;
            }
        }
        
        sCall = "execvp";
        execvp(argv[0], argv);

child_error:
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "%s: %s\n", argv[0], strerror(errno));
        _exit(1);
        /* NOTREACHED */
    }
    
    return pid;
error:

    return (pid_t)-1;    
}

pid_t
ngemPopenArgvEx(
    ngemStandardIO_t *io,
    char **argv,
    uid_t uid,
    gid_t gid,
    ngemEnvironment_t *env)
{
#define N_PIPE_PAIR 3
#define N_PIPE      ((N_PIPE_PAIR) * 2)
    pid_t pid = -1;
    int pfds[N_PIPE] = {-1. -1. -1. -1. -1. -1,};
    static bool pfdsMapForChild[N_PIPE] = {true, false, false, true, false, true,};
    int i;
    int result;
    ngLog_t *log;
    ngemStandardIO_t stdIO;
    static const char fName[] = "ngemPopenArgvEx";

    log = ngemLogGetDefault();
    
    NGEM_ASSERT(io != NULL);
    NGEM_ASSERT(argv != NULL);

    /* Create PIPE */    
    for (i = 0;i < N_PIPE;i += 2) {
        result = pipe(&pfds[i]);
        if (result < 0) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "pipe: %s.\n", strerror(errno));
            goto error;
        }
    }
    
    /* Set close-on-exec flag */
    for (i = 0;i < N_PIPE;++i) {
        result = ngemFDsetExecOnCloseFlag(pfds[i]);
        if (result == 0) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "Can't set exec-on-close flag.\n");
            goto error;
        }    
    }

    stdIO.ngsio_in    = pfds[0];
    stdIO.ngsio_out   = pfds[3];
    stdIO.ngsio_error = pfds[5];
    
    pid = ngemInvokeProcessArgvEx(&stdIO, argv, uid, gid, env);
    if (pid < 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, 
            "Can't invoke the process.\n");
        goto error;
    }
    
    /* Close pipe for Child */
    for (i = 0;i < N_PIPE;++i) {
        if (pfdsMapForChild[i] == true) {
            result = close(pfds[i]);
            pfds[i] = -1;
            if (result < 0) {
                ngLogError(log, NGEM_LOGCAT_UTILITY, fName, 
                    "close: %s.\n", strerror(errno));
                goto error;
            }
        }
    }
    
    io->ngsio_in    = pfds[1];
    io->ngsio_out   = pfds[2];
    io->ngsio_error = pfds[4];
    
    return pid;
error:
    /* Close all pipe */
    for (i = 0;i < N_PIPE;++i) {
        if (pfds[i] >= 0) {
            result = close(pfds[i]);
            pfds[i] = -1;
            if (result < 0) {
                ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                    "close: %s.\n", strerror(errno));
            }
        }
    }
    return (pid_t)-1;    
}

/**
 * StandardIO: Close
 */
ngemResult_t
ngemStandardIOclose(
    ngemStandardIO_t *sio)
{
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    static const char fName[] = "ngemStandardIOclose";

    log = ngemLogGetDefault();

    NGEM_ASSERT(sio != NULL);

    if (sio->ngsio_in >= 0) {
        result = close(sio->ngsio_in);
        if (result < 0) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "close(%d):%s\n", sio->ngsio_in, strerror(errno));
            ret = NGEM_FAILED;
        }
        sio->ngsio_in = -1;
    }

    if (sio->ngsio_out >= 0) {
        result = close(sio->ngsio_out);
        if (result < 0) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "close(%d):%s\n", sio->ngsio_out, strerror(errno));
            ret = NGEM_FAILED;
        }
        sio->ngsio_out = -1;
    }

    if (sio->ngsio_error >= 0) {
        result = close(sio->ngsio_error);
        if (result < 0) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "close(%d):%s\n", sio->ngsio_error, strerror(errno));
            ret = NGEM_FAILED;
        }
        sio->ngsio_error = -1;
    }
    return ret;
}

/**
 * This function like vprintf(), and returns result as allocated string.
 */
char *
ngemStrdupVprintf(
    const char *fmt, 
    va_list ap)
{
    ngLog_t *log;
#if 0
    static const char fName[] = "ngemStrdupVprintf";
#endif

    log = ngemLogGetDefault();

    return ngiStrdupVprintf(fmt, ap, log, NULL);
}

/**
 * This function like printf(), and returns result as allocated string.
 */
char *
ngemStrdupPrintf(
    const char *fmt, ...)
{
    char *ret = NULL;
    va_list ap;
#if 0
    static const char fName[] = "ngemStrdupPrintf";
#endif

    va_start(ap, fmt);
    ret = ngemStrdupVprintf(fmt, ap);
    va_end(ap);

    return ret;
}

/**
 * Quotes string.
 * WARNING: Must free the return value, after use.
 */
char *
ngemShellQuote(
    const char *string)
{
#define NGEML_SINGLE_QUOTE '\''
#define NGEML_QUOTED_SINGLE_QUOTE "'\\''"
    size_t length = 2;/* first and last quotations */
    const char *p;
    char *q;
    char *new = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemShellQuote";

    log = ngemLogGetDefault();

    NGEM_ASSERT(string != NULL);

    /* Get necessary size */
    p = string;
    while (*p != '\0') {
        if (*p == '\'') {
            length += strlen(NGEML_QUOTED_SINGLE_QUOTE);
        } else {
            length++;
        }
        p++;
    }

    new = ngiMalloc(length + 1, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "Can't allocate storage for string.\n");
        goto error;
    }
    
    /* Copy string */
    p = string;
    q = new;
    *(q++)= NGEML_SINGLE_QUOTE;
    while (*p != '\0') {
        if (*p == '\'') {
            strcpy(q, NGEML_QUOTED_SINGLE_QUOTE);
            q += strlen(NGEML_QUOTED_SINGLE_QUOTE);
        } else {
            *q = *p;
            q++;
        }
        p++;
    }
    *(q++)= NGEML_SINGLE_QUOTE;
    *(q++)= '\0';

    return new;
    
error:
    return NULL;
#undef NGEML_SINGLE_QUOTE
#undef NGEML_QUOTED_SINGLE_QUOTE
}

/* String Buffer */

/**
 * String Buffer: Initialize
 */
ngemResult_t
ngemStringBufferInitialize(
    ngemStringBuffer_t *buf)
{
    ngLog_t *log;
    static const char fName[] = "ngemStringBufferInitialize";

    log = ngemLogGetDefault();

    NGEM_ASSERT(buf != NULL);

    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0;
    buf->ngsb_capacity = NGEM_STRING_BUFFER_INIT_SIZE;

    buf->ngsb_buffer = malloc(buf->ngsb_capacity);
    if (buf->ngsb_buffer == NULL) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "malloc: %s.\n", strerror(errno));
        return NGEM_FAILED;
    }
    strcpy(buf->ngsb_buffer, "");

    return NGEM_SUCCESS;
}

/**
 * String Buffer: Reset
 */
ngemResult_t
ngemStringBufferReset(
    ngemStringBuffer_t *buf)
{
    NGEM_FNAME_TAG(ngemStringBufferReset);

    NGEM_ASSERT(buf != NULL);

    if (buf->ngsb_buffer == NULL) {
        ngemStringBufferFinalize(buf);
        return ngemStringBufferInitialize(buf);
    } else {
        buf->ngsb_length  = 0;
        strcpy(buf->ngsb_buffer, "");
    }

    return NGEM_SUCCESS;
}

/**
 * String Buffer: Finalize
 */
void
ngemStringBufferFinalize(
    ngemStringBuffer_t *buf)
{
    ngLog_t *log;
#if 0
    static const char fName[] = "ngemStringBufferFinalize";
#endif
    NGEM_ASSERT(buf != NULL);

    log = ngemLogGetDefault();
    
    if (buf->ngsb_buffer != NULL) {
        NGEM_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
        ngiFree(buf->ngsb_buffer, log, NULL);
    }
    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0U;
    buf->ngsb_capacity = 0U;

    return;
}

/**
 * String Buffer: Append string.
 */
ngemResult_t
ngemStringBufferAppend(
    ngemStringBuffer_t *buf,
    const char *string)
{
    NGEM_FNAME_TAG(ngemStringBufferAppend);

    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(buf->ngsb_buffer != NULL);
    NGEM_ASSERT(buf->ngsb_length + 1 <= buf->ngsb_capacity);
    NGEM_ASSERT(string != NULL);

    return ngemStringBufferNappend(buf, string, strlen(string));
}

/**
 * String Buffer: Append string. n bytes.
 */
ngemResult_t
ngemStringBufferNappend(
    ngemStringBuffer_t *buf,
    const char *string,
    size_t nString)
{
    char *newBuf = NULL;
    size_t newCap;
    size_t newLength;
    ngLog_t *log;
    static const char fName[] = "ngemStringBufferNappend";

    log = ngemLogGetDefault();

    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(buf->ngsb_buffer != NULL);
    NGEM_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
    NGEM_ASSERT(string != NULL);

    if (nString > 0) {
        newLength = buf->ngsb_length + nString;
        if ((newLength + 1) > buf->ngsb_capacity) {
            /* Grow */
            newCap = NGEM_MAX(buf->ngsb_capacity * 2, newLength + 1);
            newBuf = ngiRealloc(buf->ngsb_buffer, newCap, log, NULL);
            if (newBuf == NULL) {
                ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                    "Can't reallocate storage for buffer,\n");
                return NGEM_FAILED;
            }
            buf->ngsb_buffer = newBuf;
            buf->ngsb_capacity = newCap;
        }

        strncpy(&buf->ngsb_buffer[buf->ngsb_length], string, nString);
        buf->ngsb_buffer[newLength] = '\0';

        buf->ngsb_length = newLength;
    }

    return NGEM_SUCCESS;
}

/**
 * String Buffer: Append string, using way like vprintf().
 */
ngemResult_t
ngemStringBufferVformat(
    ngemStringBuffer_t *buf,
    const char *fmt, 
    va_list ap)
{
    size_t size;
    size_t newCap;
    char *newBuf;
    int nprint;
    va_list aq;
    ngLog_t *log;
    static const char fName[] = "ngemStringBufferVformat";

    log = ngemLogGetDefault();

    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(buf->ngsb_buffer != NULL);
    NGEM_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
    NGEM_ASSERT(NGEM_STRING_IS_NONZERO(fmt));

    for (;;) {
        size = buf->ngsb_capacity - buf->ngsb_length;
#ifdef va_copy
        va_copy(aq, ap);
#else
        memcpy(&aq, &ap, sizeof(va_list));
#endif
        nprint = vsnprintf(
            &buf->ngsb_buffer[buf->ngsb_length], size, fmt, aq);
#ifdef va_copy
        va_end(aq);
#endif
        /* In Tru64:
         * When string is longer than buffer, snprintf() returns size - 1.
         */
        if ((nprint >= 0) && (nprint < size-1)) {
            buf->ngsb_length += nprint;
            break;
        } else {
            if ((nprint < 0) || (nprint == size - 1)) {
                newCap = buf->ngsb_capacity * 2;
            } else {
                newCap = NGEM_MAX(buf->ngsb_capacity * 2,
                    buf->ngsb_length + nprint + 1);
            }

            newBuf = ngiRealloc(buf->ngsb_buffer, newCap, log, NULL);
            if (newBuf == NULL) {
                ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                    "Can't reallocate storage for string,\n");
                buf->ngsb_buffer[buf->ngsb_length] = '\0';
                return NGEM_FAILED;
            }
            buf->ngsb_buffer  = newBuf;
            buf->ngsb_capacity = newCap;
            
            ngLogDebug(log, NGEM_LOGCAT_UTILITY, fName,
                "grows buffer to %lu\n", (unsigned long)newCap);
        }
    }
    
    return NGEM_SUCCESS;
}

/**
 * String Buffer: Append string, using way like printf().
 */
ngemResult_t
ngemStringBufferFormat(
    ngemStringBuffer_t *buf,
    const char *fmt, ...)
{
    va_list ap;
    ngemResult_t result;
#if 0
    static const char fName[] = "ngemStringBufferFormat";
#endif

    va_start(ap, fmt);
    result = ngemStringBufferVformat(buf, fmt, ap);
    va_end(ap);

    return result;
}

/**
 * String Buffer: Returns String.
 */
char *
ngemStringBufferRelease(
    ngemStringBuffer_t *buf)
{
    char *ret;
#if 0
    static const char fName[] = "ngemStringBufferRelease";
#endif

    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(buf->ngsb_buffer != NULL);
    NGEM_ASSERT(buf->ngsb_length < buf->ngsb_capacity);

    ret = buf->ngsb_buffer;
    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0U;
    buf->ngsb_capacity = 0U;

    return ret;
}

/**
 * String Buffer: Get String.
 */
char *
ngemStringBufferGet(
    ngemStringBuffer_t *buf)
{
#if 0
    static const char fName[] = "ngemStringBufferGet";
#endif

    NGEM_ASSERT(buf != NULL);
    NGEM_ASSERT(buf->ngsb_buffer != NULL);
    NGEM_ASSERT(buf->ngsb_length < buf->ngsb_capacity);

    return buf->ngsb_buffer;
}

int
ngemAssertFailed(
    const char *file,
    int line,
    const char *string)
{
    ngLogFatal(ngemLogGetDefault(), NGEM_LOGCAT_UTILITY, NULL,
        "In %s:%d \"%s\" failed.\n", file, line, string);
    abort();
    /* NOTREACHED */
    return 0;
}

void
ngemAbort(
    const char *file,
    int line)
{
    ngLogFatal(ngemLogGetDefault(), NGEM_LOGCAT_UTILITY, NULL,
        "In %s:%d abort.\n", file, line);
    abort();
    return;
}

/**
 * Strip string.
 * Deletes spaces in head or tail.
 */
void
ngemStringStrip(
    char *string)
{
    char *head;
    char *tail;
    char *src;
    char *dest;
#if 0
    static const char fName[] = "ngemStringStrip";
#endif

    NGEM_ASSERT(string != NULL);

    head = string;
    while ((*head != '\0') && isspace((int)*head)) {
        head++;
    }

    tail = &string[strlen(string)];
    while ((tail != string) && isspace((int)*(tail-1))) {
        tail--;
    }

    if (head != string) {
        src  = head;
        dest = string;

        while (src != tail) {
            *dest++ = *src++;
        }
        *dest = '\0';
    } else {
        *tail = '\0';
    }

    return;
}

/**
 * Create pipes with no-blocking flag.
 */
ngemResult_t
ngemNonblockingPipe(
    int pfd[2])
{
    int pipeFd[2] = {-1, -1};
    bool pipeCreated = false;
    ngemResult_t nResult;
    int result;
    int i;
    ngLog_t *log;
    static const char fName[] = "ngemNonblockingPipe";

    log = ngemLogGetDefault();

    result = pipe(pipeFd);
    if (result < 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "pipe: %s.\n", strerror(errno));
        goto error;
    }
    pipeCreated = true;

    for (i = 0;i < 2;++i) {
        nResult = ngemFDsetNonblockFlag(pipeFd[i]);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                "Can't set nonblock flag to fd.\n");
            goto error;
        }
    }

    NGEM_PIPE_IN(pfd)  = NGEM_PIPE_IN(pipeFd);
    NGEM_PIPE_OUT(pfd) = NGEM_PIPE_OUT(pipeFd);

    return NGEM_SUCCESS;
error:
    if (pipeCreated == true) {
        for (i = 0;i < 2;++i) {
            result = close(pipeFd[i]);
            if (result < 0) {
                ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
                    "close: %s.\n", strerror(errno));
            }
        }
        pipeCreated = false;
    }
    return NGEM_FAILED;
}

/**
 * Set non-blocking flag
 */
ngemResult_t
ngemFDsetNonblockFlag(
    int fd)
{
    int fsFlags;
    int result;
    ngLog_t *log;
    NGEM_FNAME(ngemFDsetNonblockFlag);

    log = ngemLogGetDefault();

    fsFlags = fcntl(fd, F_GETFL);
    if (fsFlags < 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "fcntl(F_GETFL): %s.\n", strerror(errno));
        return NGEM_FAILED;
    }
    fsFlags |= O_NONBLOCK;

    result = fcntl(fd, F_SETFL, fsFlags);
    if (result < 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName,
            "fcntl(F_SETFL): %s.\n", strerror(errno));
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

/**
 * Set exec-on-close flag
 */
ngemResult_t
ngemFDsetExecOnCloseFlag(
    int fd)
{
    int fdFlags;
    int result;
    static const char fName[] = "ngemFDsetExecOnCloseFlag";

    NGEM_ASSERT_FD(fd);

    fdFlags = fcntl(fd, F_GETFD);
    if (fdFlags < 0) {
        ngLogError(NULL, fName, "fcntl(F_GETFD): %s.\n", strerror(errno));
        return NGEM_FAILED;
    }    
    fdFlags |= FD_CLOEXEC;

    result = fcntl(fd, F_SETFD, fdFlags);
    if (result < 0) {
        ngLogError(NULL, fName, "fcntl(F_SETFD): %s.\n", strerror(errno));
        return NGEM_FAILED;
    }    
    return NGEM_SUCCESS;
}

/* Token Analyzer */

/**
 * Token Analyzer: Initialize
 */
void
ngemTokenAnalyzerInitialize(
    ngemTokenAnalyzer_t *tokenAnalyzer,
    const char *string)
{
#if 0
    static const char fName[] = "ngemTokenAnalyzerInitialize";
#endif
    NGEM_ASSERT(tokenAnalyzer != NULL);
    NGEM_ASSERT(string != NULL);

    tokenAnalyzer->ngta_target  = string;
    /* Skip space at head */
    while ((*string != '\0') && (isspace((int)*string))) {
        string++;
    }
    tokenAnalyzer->ngta_pointer = string;

    return;
}

/**
 * Token Analyzer: Finalize
 */
void
ngemTokenAnalyzerFinalize(
    ngemTokenAnalyzer_t *tokenAnalyzer)
{
#if 0
    static const char fName[] = "ngemTokenAnalyzerFinalize";
#endif
    NGEM_ASSERT(tokenAnalyzer != NULL);

    tokenAnalyzer->ngta_target  = NULL;
    tokenAnalyzer->ngta_pointer = NULL;

    return;
}

/**
 * Token Analyzer: Get enum
 */
int
ngemTokenAnalyzerGetEnum(
    ngemTokenAnalyzer_t *tokenAnalyzer,
    const char **table,
    size_t tableSize)
{
    size_t length;
    const char *p;
    int i;
    ngLog_t *log;
    static const char fName[] = "ngemTokenAnalyzerGetEnum";

    log = ngemLogGetDefault();

    NGEM_ASSERT(tokenAnalyzer != NULL);
    NGEM_ASSERT(table != NULL);

    length = ngemlTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "There is no more token.\n");
        return -1;
    }
    p = tokenAnalyzer->ngta_pointer;
    for (i = 0;i < tableSize;++i) {
        if (strncmp(p, table[i], length) == 0) {
            ngemTokenAnalyzerNext(tokenAnalyzer);
            return i;
        }
    }
    ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Invalid string.\n");

    return -1;
}

/**
 * Token Analyzer: Get int
 */
int
ngemTokenAnalyzerGetInt(
    ngemTokenAnalyzer_t *tokenAnalyzer,
    int *value)
{
    long lret;
    size_t length;
    char *endp;
    const char *p;
    ngLog_t *log;
    static const char fName[] = "ngemTokenAnalyzerGetInt";

    log = ngemLogGetDefault();

    NGEM_ASSERT(tokenAnalyzer != NULL);
    NGEM_ASSERT(value != NULL);
    
    length = ngemlTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "There is no more token.\n");
        return 0;
    }

    p = tokenAnalyzer->ngta_pointer;
    errno = 0;
    lret = strtol(p, &endp, 0);
    if (errno != 0) {
        ngLogError(NULL, fName, "strtol: %s\n", strerror(errno));
        return 0;
    }
    
    if (endp != (p + length)) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "token %lu, endp %lu\n",
            (unsigned long)length, (unsigned long)(endp - p));
        return 0;
    }

    if ((lret < INT_MIN) || (lret > INT_MAX)) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Overflow.\n");
        return 0;
    }

    ngemTokenAnalyzerNext(tokenAnalyzer);
    *value = (int)lret;

    return 1;
}

/**
 * Token Analyzer: Get string(allocated)
 */
char *
ngemTokenAnalyzerGetString(
    ngemTokenAnalyzer_t *tokenAnalyzer)
{
    char *ret = NULL;
    size_t length;
    ngLog_t *log;
    static const char fName[] = "ngemTokenAnalyzerGetString";

    log = ngemLogGetDefault();

    NGEM_ASSERT(tokenAnalyzer != NULL);

    length = ngemlTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "There is no more token.\n");
        return NULL;
    }

    ret = ngiStrndup(tokenAnalyzer->ngta_pointer, length, log, NULL);
    if (ret == NULL) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't copy string.\n");
        return NULL;
    }
    ngemTokenAnalyzerNext(tokenAnalyzer);

    return ret;
}

/**
 * Token Analyzer: Get next.
 */
ngemResult_t
ngemTokenAnalyzerNext(
    ngemTokenAnalyzer_t *tokenAnalyzer)
{
    const char *p;
    ngLog_t *log;
    static const char fName[] = "ngemTokenAnalyzerNext";

    log = ngemLogGetDefault();

    NGEM_ASSERT(tokenAnalyzer != NULL);

    ngLogDebug(log, NGEM_LOGCAT_UTILITY, fName,
        "rest=\"%s\".\n", tokenAnalyzer->ngta_pointer);

    p = tokenAnalyzer->ngta_pointer;
    while ((*p != '\0') && (!isspace((int)*p))) {
        p++;
    }

    while ((*p != '\0') && (isspace((int)*p))) {
        p++;
    }
    tokenAnalyzer->ngta_pointer = p;

    return *p != '\0'?NGEM_SUCCESS:NGEM_FAILED;
}

/**
 * Token Analyzer: has next?
 */
bool
ngemTokenAnalyzerHasNext(
    ngemTokenAnalyzer_t *tokenAnalyzer)
{
    NGEM_FNAME_TAG(ngemTokenAnalyzerHasNext);

    NGEM_ASSERT(tokenAnalyzer != NULL);
    return (*tokenAnalyzer->ngta_pointer == '\0')?false:true;
}

/*
 * Token Analyzer: get token length.
 */
static size_t
ngemlTokenAnalyzerGetTokenLength(
    ngemTokenAnalyzer_t *tokenAnalyzer)
{
    size_t length = 0;
    const char *p;
    NGEM_FNAME_TAG(ngemlTokenAnalyzerGetTokenLength);

    p = tokenAnalyzer->ngta_pointer;
    while ((*p != '\0') && (!isspace((int)*p))) {
        p++;
        length++;
    }

    return length;
}

/**
 * Analyzes environment variable(NAME=VALUE).
 */
ngemResult_t
ngemEnvironmentAnalyze(
    char *in,
    char **name,
    char **value)
{
    char *tmp;
    char *envVal = NULL;
    char *envName = NULL;
    ngLog_t *log;
    static const char fName[] = "ngemEnvironmentAnalyze";

    log = ngemLogGetDefault();

    NGEM_ASSERT(in != NULL);
    NGEM_ASSERT(name != NULL);
    NGEM_ASSERT(value != NULL);

    if (strlen(in) == 0) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Environment variable is empty string.\n");
        goto error;
    }

    /* Value */
    tmp = strstr(in, "=");
    if (tmp == in) {
        /* "=" appear at first */
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Environment variable's name is empty.\n");
        goto error;
    }

    if (tmp == NULL) {
        /* Name */
        envName= ngiStrdup(in, log, NULL);
        if (envName== NULL) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't copy string.\n");
            goto error;
        }
        envName= ngiStrdup("", log, NULL);
        if (envName== NULL) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't create string.\n");
            goto error;
        }

    } else {
        /* Name */
        envName = ngiStrndup(in, tmp - in, log, NULL);
        if (envName== NULL) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't copy string.\n");
            goto error;
        }
        envVal = ngemShellQuote(tmp + 1);
        if (value == NULL) {
            ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't quote string.\n");
            goto error;
        }
    }

    *name  = envName;
    *value = envVal;

    return NGEM_SUCCESS;
error:
    ngiFree(envName, log, NULL);
    ngiFree(envVal, log, NULL);
    envName = NULL;
    envVal = NULL;

    return NGEM_FAILED;
}

/**
 * Makes temp file.
 */
char *
ngemMakeTempFile(
    const char *tmpDir)
{
    char *filename = NULL;
    int result;
    bool fileCreated = false;
    int fd;
    ngLog_t *log;
    static const char fName[] = "ngemMakeTempFile";

    log = ngemLogGetDefault();

    if (tmpDir == NULL) {
        tmpDir = getenv("TMPDIR");
    } 
    if (tmpDir == NULL) {
        tmpDir = "/tmp";
    } 

    filename = ngemStrdupPrintf("%s/ngem-sshXXXXXX", tmpDir);
    if (filename == NULL) {
        ngLogError(log, NGEM_LOGCAT_UTILITY, fName, "Can't copy the string.\n");
        goto error;
    }

    fd = mkstemp(filename);
    if (fd < 0) {
        ngLogError(NULL, fName, "Can't make temporary file: %s.\n",
            strerror(errno));
        goto error;
    }
    fileCreated = true;

    result = close(fd);
    if (result < 0) {
        ngLogError(NULL, fName, "close: %s\n", strerror(errno));
        goto error;
    }

    return filename;
error:
    if (fileCreated == true) {
        NGEM_ASSERT(filename != NULL);
        result = unlink(filename);
        if (result < 0) {
            ngLogError(NULL, fName, "unlink: %s\n", strerror(errno));
        }
        fileCreated = false;
    }
    ngiFree(filename, log, NULL);
    filename = NULL;

    return NULL;
}

/**
 * Compares string.
 */
bool
ngemStringCompare(
    const char *lhs,
    ssize_t lhsLen,
    const char *rhs,
    ssize_t rhsLen)
{
    if ((lhs == NULL) && (rhs == NULL)) {
        return true;
    }
    if (lhs == NULL) {
        return false;
    }
    if (rhs == NULL) {
        return false;
    }

    if (lhsLen < 0) {
        lhsLen = strlen(lhs);
    }

    if (rhsLen < 0) {
        rhsLen = strlen(rhs);
    }
    if (lhsLen != rhsLen) {
        return false;
    }
    if (strncmp(lhs, rhs, lhsLen) != 0) {
        return false;
    }
    return true;
}
