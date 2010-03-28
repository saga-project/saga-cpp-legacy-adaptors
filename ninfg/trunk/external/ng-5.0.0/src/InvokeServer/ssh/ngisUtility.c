/*
 * $RCSfile: ngisUtility.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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

#include "ngisUtility.h"
#include "ngInvokeServer.h"

NGI_RCSID_EMBED("$RCSfile: ngisUtility.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $")

#define NGISL_MAX_ARGS_SIZE 4

static size_t ngislTokenAnalyzerGetTokenLength(ngisTokenAnalyzer_t *);

pid_t
ngisPopen(
    ngisStandardIO_t *io,
    char *command)
{
    char *args[NGISL_MAX_ARGS_SIZE];
    int i = 0;
#if 0
    static const char fName[] = "ngisPopen";
#endif
    
    NGIS_ASSERT(io != NULL);
    NGIS_ASSERT(command != NULL);
    
    i = 0;
    if (strcmp(command, NGIS_SHELL) == 0) {
        args[i++] = NGIS_SHELL;
        args[i++] = NULL;
    } else {
        args[i++] = NGIS_SHELL;
        args[i++] = "-c";
        args[i++] = command;
        args[i++] = NULL;
    }
    NGIS_ASSERT(i <= NGISL_MAX_ARGS_SIZE);

    return ngisPopenArgv(io, args);
}

pid_t
ngisInvokeProcess(
    ngisStandardIO_t *sio,
    char *command)
{
    char *args[NGISL_MAX_ARGS_SIZE];
    int i;
#if 0
    static const char fName[] = "ngisInvokeProcess";
#endif
    
    NGIS_ASSERT(sio != NULL);
    NGIS_ASSERT(command != NULL);
    
    i = 0;
    if (strcmp(command, NGIS_SHELL) == 0) {
        args[i++] = NGIS_SHELL;
        args[i++] = NULL;
    } else {
        args[i++] = NGIS_SHELL;
        args[i++] = "-c";
        args[i++] = command;
        args[i++] = NULL;
    }
    NGIS_ASSERT(i <= NGISL_MAX_ARGS_SIZE);

    return ngisInvokeProcessArgv(sio, args);
}

pid_t
ngisPopenArgv(
    ngisStandardIO_t *io,
    char **argv)
{
#define N_PIPE_PAIR 3
#define N_PIPE      ((N_PIPE_PAIR) * 2)
    pid_t pid = -1;
    int pfds[N_PIPE]                   = {-1. -1. -1. -1. -1. -1,};
    static int pfdsMapForChild[N_PIPE] = { 1,  0,  0,  1,  0,  1,};
    int i;
    int result;
    ngisStandardIO_t stdIO;
    static const char fName[] = "ngisPopenArgv";
    
    NGIS_ASSERT(io != NULL);
    NGIS_ASSERT(argv != NULL);

    /* Create PIPE */    
    for (i = 0;i < N_PIPE;i += 2) {
        result = pipe(&pfds[i]);
        if (result < 0) {
            ngisErrorPrint(NULL, fName, "pipe: %s.\n", strerror(errno));
            goto error;
        }
    }
    
    /* Set close-on-exec flag */
    for (i = 0;i < N_PIPE;++i) {
        result = ngisFDsetExecOnCloseFlag(pfds[i]);
        if (result == 0) {
            ngisErrorPrint(NULL, fName, "Can't set exec-on-close flag.\n");
            goto error;
        }    
    }

    stdIO.ngsio_in    = pfds[0];
    stdIO.ngsio_out   = pfds[3];
    stdIO.ngsio_error = pfds[5];
    
    pid = ngisInvokeProcessArgv(&stdIO, argv);
    if (pid < 0) {
        ngisErrorPrint(NULL, fName, "Can't invoke the process.\n");
        goto error;
    }
    
    /* Close pipe for Child */
    for (i = 0;i < N_PIPE;++i) {
        if (pfdsMapForChild[i] != 0) {
            result = close(pfds[i]);
            pfds[i] = -1;
            if (result < 0) {
                ngisErrorPrint(NULL, fName, "close: %s.\n", strerror(errno));
                goto error;
            }
        }
    }
    
    io->ngsio_in    = pfds[1];
    io->ngsio_out   = pfds[2];
    io->ngsio_error = pfds[4];
    
    return pid;
error:
    /* Close pipe for Child */
    for (i = 0;i < N_PIPE;++i) {
        if (pfdsMapForChild[i] != 0) {
            result = close(pfds[i]);
            pfds[i] = -1;
            if (result < 0) {
                ngisErrorPrint(NULL, fName, "close: %s.\n", strerror(errno));
            }
        }
    }
    return (pid_t)-1;    
}

pid_t
ngisInvokeProcessArgv(
    ngisStandardIO_t *io,
    char **argv)
{
    pid_t pid = -1;
    int result;
    char *sCall = NULL;
    static const char fName[] = "ngisInvokeProcessArgv";
    
    NGIS_ASSERT(argv != NULL);

    pid = fork();
    if (pid < 0) {
        ngisErrorPrint(NULL, fName, "fork: %s.\n", strerror(errno));
        goto error;
    }
    if (pid == 0) {
        /* Child Process*/
        /* STDIN */
        if (io != NULL) {
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
        
        sCall = "execv";
        execv(argv[0], argv);

child_error:
        fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
        fflush(stderr);
        _exit(1);
        /* NOTREACHED */
    }
    
    return pid;
error:

    return (pid_t)-1;    
}

/**
 * StandardIO: Close
 */
int
ngisStandardIOclose(
    ngisStandardIO_t *sio)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngisStandardIOclose";

    NGIS_ASSERT(sio != NULL);

    if (sio->ngsio_in >= 0) {
        result = close(sio->ngsio_in);
        if (result < 0) {
            ngisErrorPrint(NULL, fName,
                "close(%d):%s\n", sio->ngsio_in, strerror(errno));
            ret = 0;
        }
        sio->ngsio_in = -1;
    }

    if (sio->ngsio_out >= 0) {
        result = close(sio->ngsio_out);
        if (result < 0) {
            ngisErrorPrint(NULL, fName,
                "close(%d):%s\n", sio->ngsio_out, strerror(errno));
            ret = 0;
        }
        sio->ngsio_out = -1;
    }

    if (sio->ngsio_error >= 0) {
        result = close(sio->ngsio_error);
        if (result < 0) {
            ngisErrorPrint(NULL, fName,
                "close(%d):%s\n", sio->ngsio_error, strerror(errno));
            ret = 0;
        }
        sio->ngsio_error = -1;
    }
    return ret;
}

char *
ngisStrdupVprintf(
    const char *fmt, 
    va_list ap)
{
    char *buffer = NULL;
    char *p;
    int n;
    va_list aq;
    size_t size = 128;/* Default size is 128. */
    static const char fName[] = "ngisStrdupVprintf";

    NGIS_ASSERT(fmt != NULL);

    buffer = malloc(size);
    if (buffer == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        goto error;
    }

    for (;;) {
        errno = 0;
#ifdef va_copy
        va_copy(aq, ap);
#else
        memcpy(&aq, &ap, sizeof(va_list));
#endif
        n = vsnprintf(buffer, size, fmt, aq);
#ifdef va_copy
        va_end(aq);
#endif
        if ((n > -1) && (errno != 0)) {
            ngisErrorPrint(NULL, fName, "vsnprintf: %s.\n", strerror(errno));
            goto error;
        }
        
        if ((n > -1) && (n != size - 1) && (n < size)) {
            break;
        } else {
            if ((n <= -1) || (n == size - 1)) {
                size *= 2;
            } else {
                size = n + 1;
            }
            
            p = realloc(buffer, size);
            if (p == NULL) {
                ngisErrorPrint(NULL, fName, "realloc: %s.\n", strerror(errno));
                goto error;
            }
            buffer = p;
            
            ngisDebugPrint(NULL, fName, "grows buffer to %lu\n",
                (unsigned long)size);
        }
    }
    
    return buffer;
    
error:
    NGIS_NULL_CHECK_AND_FREE(buffer);
    
    return NULL;
}


char *
ngisStrdupPrintf(
    const char *fmt, ...)
{
    char *ret = NULL;
    va_list ap;
#if 0
    static const char fName[] = "ngisStrdupPrintf";
#endif

    va_start(ap, fmt);
    ret = ngisStrdupVprintf(fmt, ap);
    va_end(ap);

    return ret;
}

char *
ngisStrndup(
    const char *string, size_t n)
{
    size_t length;
    char *new = NULL;
    static const char fName[] = "ngisStrndup";
    
    length = NGIS_MIN(strlen(string), n);
    
    new = malloc(length + 1);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        return NULL;
    }
    
    strncpy(new, string, length);
    new[length] = '\0';
    
    return new;
}

char *
ngisShellQuote(
    const char *string)
{
#define NGISL_SINGLE_QUOTE '\''
#define NGISL_QUOTED_SINGLE_QUOTE "'\\''"
    size_t length = 2;/* first and last quotations */
    const char *p;
    char *q;
    char *new = NULL;
    static const char fName[] = "ngisShellQuote";

    NGIS_ASSERT(string != NULL);

    /* Get necessary size */
    p = string;
    while (*p != '\0') {
        if (*p == '\'') {
            length += strlen(NGISL_QUOTED_SINGLE_QUOTE);
        } else {
            length++;
        }
        p++;
    }
    
    new = malloc(length + 1);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        goto error;
    }
    
    /* Copy string */
    p = string;
    q = new;
    *(q++)= NGISL_SINGLE_QUOTE;
    while (*p != '\0') {
        if (*p == '\'') {
            strcpy(q, NGISL_QUOTED_SINGLE_QUOTE);
            q += strlen(NGISL_QUOTED_SINGLE_QUOTE);
        } else {
            *q = *p;
            q++;
        }
        p++;
    }
    *(q++)= NGISL_SINGLE_QUOTE;
    *(q++)= '\0';

    return new;
    
error:
    return NULL;
#undef NGISL_SINGLE_QUOTE
#undef NGISL_QUOTED_SINGLE_QUOTE
}

/* String Buffer */
int
ngisStringBufferInitialize(
    ngisStringBuffer_t *buf)
{
    static const char fName[] = "ngisStringBufferInitialize";

    NGIS_ASSERT(buf != NULL);

    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0;
    buf->ngsb_capacity = NGIS_STRING_BUFFER_INIT_SIZE;

    buf->ngsb_buffer = malloc(buf->ngsb_capacity);
    if (buf->ngsb_buffer == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        return 0;
    }
    return 1;
}

void
ngisStringBufferFinalize(
    ngisStringBuffer_t *buf)
{
#if 0
    static const char fName[] = "ngisStringBufferFinalize";
#endif
    NGIS_ASSERT(buf != NULL);
    
    if (buf->ngsb_buffer != NULL) {
        NGIS_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
        free(buf->ngsb_buffer);
    }
    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0U;
    buf->ngsb_capacity = 0U;

    return;
}

int
ngisStringBufferAppend(
    ngisStringBuffer_t *buf,
    const char *string)
{
    char *newBuf = NULL;
    size_t newCap;
    size_t newLength;
    static const char fName[] = "ngisStringBufferAppend";

    NGIS_ASSERT(buf != NULL);
    NGIS_ASSERT(buf->ngsb_buffer != NULL);
    NGIS_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
    NGIS_ASSERT(string != NULL);

    if (strlen(string) > 0) {
        newLength = buf->ngsb_length + strlen(string);
        if (newLength >= buf->ngsb_capacity) {
            /* Grow */
            newCap = buf->ngsb_capacity * 2;
            newBuf = realloc(buf->ngsb_buffer, newCap);
            if (newBuf == NULL) {
                ngisErrorPrint(NULL, fName,
                    "realloc: %s.\n", strerror(errno));
                return 0;
            }
            buf->ngsb_buffer = newBuf;
            buf->ngsb_capacity = newCap;
        }

        strcpy(&buf->ngsb_buffer[buf->ngsb_length], string);
        buf->ngsb_length = newLength;
    }

    return 1;
}

int
ngisStringBufferVformat(
    ngisStringBuffer_t *buf,
    const char *fmt, 
    va_list ap)
{
    size_t size;
    size_t newCap;
    char *newBuf;
    int nprint;
    va_list aq;
    static const char fName[] = "ngisStringBufferVformat";

    NGIS_ASSERT(buf != NULL);
    NGIS_ASSERT(buf->ngsb_buffer != NULL);
    NGIS_ASSERT(buf->ngsb_length < buf->ngsb_capacity);
    NGIS_ASSERT(NGIS_STRING_IS_NONZERO(fmt));

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
        if ((nprint < 0) || (nprint == size - 1) || (nprint >= size)) {
            if ((nprint < 0) || (nprint == size - 1)) {
                newCap = buf->ngsb_capacity * 2;
            } else {
                newCap = NGIS_MAX(buf->ngsb_capacity * 2,
                    buf->ngsb_length + nprint + 1);
            }
            newBuf = realloc(buf->ngsb_buffer, newCap);
            if (newBuf == NULL) {
                ngisErrorPrint(NULL, fName, "realloc: %s\n", strerror(errno));
                buf->ngsb_buffer[buf->ngsb_length] = '\0';
                return 0;
            }
            buf->ngsb_buffer  = newBuf;
            buf->ngsb_capacity = newCap;
            
            ngisDebugPrint(NULL, fName, "grows buffer to %lu\n",
                (unsigned long)newCap);
        } else {
            buf->ngsb_length += nprint;
            break;
        }
    }
    
    return 1;
}

int
ngisStringBufferFormat(
    ngisStringBuffer_t *buf,
    const char *fmt, ...)
{
    va_list ap;
    int result;
#if 0
    static const char fName[] = "ngisStringBufferFormat";
#endif

    va_start(ap, fmt);
    result = ngisStringBufferVformat(buf, fmt, ap);
    va_end(ap);

    return result;
}

char *
ngisStringBufferRelease(
    ngisStringBuffer_t *buf)
{
    char *ret;
#if 0
    static const char fName[] = "ngisStringBufferRelease";
#endif

    NGIS_ASSERT(buf != NULL);
    NGIS_ASSERT(buf->ngsb_buffer != NULL);
    NGIS_ASSERT(buf->ngsb_length < buf->ngsb_capacity);

    ret = buf->ngsb_buffer;
    buf->ngsb_buffer  = NULL;
    buf->ngsb_length  = 0U;
    buf->ngsb_capacity = 0U;

    return ret;
}

int
ngisAssertFailed(
    const char *file,
    int line,
    const char *string)
{
    ngisAbortPrint(NULL, NULL,
        "In %s:%d \"%s\" failed.\n", file, line, string);
    abort();
    /* NOTREACHED */
    return 0;
}

void
ngisAbort(
    const char *file,
    int line)
{
    ngisAbortPrint(NULL, NULL,
        "In %s:%d abort.\n", file, line);
    abort();
    return;
}

void
ngisStringStrip(
    char *string)
{
    char *head;
    char *tail;
    char *src;
    char *dest;
#if 0
    static const char fName[] = "ngisStringStrip";
#endif

    NGIS_ASSERT(string != NULL);

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

int
ngisNonblockingPipe(
    int pfd[2])
{
    int pipeFd[2] = {-1, -1};
    int pipeCreated = 0;
    int fsFlags;
    int result;
    int i;
    static const char fName[] = "ngisNonblockingPipe";

    result = pipe(pipeFd);
    if (result < 0) {
        ngisErrorPrint(NULL, fName, "pipe: %s.\n", strerror(errno));
        goto error;
    }
    pipeCreated = 1;

    for (i = 0;i < 2;++i) {
        fsFlags = fcntl(pipeFd[i], F_GETFL);
        if (fsFlags < 0) {
            ngisErrorPrint(NULL, fName,
                "fcntl(F_GETFL): %s.\n", strerror(errno));
            goto error;
        }
        fsFlags |= O_NONBLOCK;
        result = fcntl(pipeFd[i], F_SETFL, fsFlags);
        if (result < 0) {
            ngisErrorPrint(NULL, fName,
                "fcntl(F_SETFL): %s.\n", strerror(errno));
            goto error;
        }
    }

    NGIS_PIPE_IN(pfd)  = NGIS_PIPE_IN(pipeFd);
    NGIS_PIPE_OUT(pfd) = NGIS_PIPE_OUT(pipeFd);

    return 1;
error:
    if (pipeCreated != 0) {
        for (i = 0;i < 2;++i) {
            result = close(pipeFd[i]);
            if (result < 0) {
                ngisErrorPrint(NULL, fName, "close: %s.\n", strerror(errno));
            }
        }
        pipeCreated = 0;
    }
    return 0;
}

int
ngisFDsetExecOnCloseFlag(
    int fd)
{
    int fdFlags;
    int result;
    static const char fName[] = "ngisFDsetExecOnCloseFlag";

    NGIS_ASSERT_FD(fd);

    fdFlags = fcntl(fd, F_GETFD);
    if (fdFlags < 0) {
        ngisErrorPrint(NULL, fName, "fcntl(F_GETFD): %s.\n", strerror(errno));
        return 0;
    }    
    fdFlags |= FD_CLOEXEC;

    result = fcntl(fd, F_SETFD, fdFlags);
    if (result < 0) {
        ngisErrorPrint(NULL, fName, "fcntl(F_SETFD): %s.\n", strerror(errno));
        return 0;
    }    
    return 1;
}

/* Token Analyzer */
void
ngisTokenAnalyzerInitialize(
    ngisTokenAnalyzer_t *tokenAnalyzer,
    const char *string)
{
#if 0
    static const char fName[] = "ngisTokenAnalyzerInitialize";
#endif
    NGIS_ASSERT(tokenAnalyzer != NULL);
    NGIS_ASSERT(string != NULL);

    tokenAnalyzer->ngta_target  = string;
    /* Skip space at head */
    while ((*string != '\0') && (isspace((int)*string))) {
        string++;
    }
    tokenAnalyzer->ngta_pointer = string;

    return;
}

void
ngisTokenAnalyzerFinalize(
    ngisTokenAnalyzer_t *tokenAnalyzer)
{
#if 0
    static const char fName[] = "ngisTokenAnalyzerFinalize";
#endif
    NGIS_ASSERT(tokenAnalyzer != NULL);

    tokenAnalyzer->ngta_target  = NULL;
    tokenAnalyzer->ngta_pointer = NULL;

    return;
}

int
ngisTokenAnalyzerGetEnum(
    ngisTokenAnalyzer_t *tokenAnalyzer,
    const char **table,
    size_t tableSize)
{
    size_t length;
    const char *p;
    int i;
    static const char fName[] = "ngisTokenAnalyzerGetEnum";

    NGIS_ASSERT(tokenAnalyzer != NULL);
    NGIS_ASSERT(table != NULL);

    length = ngislTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngisErrorPrint(NULL, fName, "There is no more token.\n");
        return -1;
    }
    p = tokenAnalyzer->ngta_pointer;
    for (i = 0;i < tableSize;++i) {
        if (strncmp(p, table[i], length) == 0) {
            ngisTokenAnalyzerNext(tokenAnalyzer);
            return i;
        }
    }
    ngisErrorPrint(NULL, fName, "Invalid string.\n");

    return -1;
}

int
ngisTokenAnalyzerGetInt(
    ngisTokenAnalyzer_t *tokenAnalyzer, int *value)
{
    long lret;
    size_t length;
    char *endp;
    const char *p;
    static const char fName[] = "ngisTokenAnalyzerGetInt";

    NGIS_ASSERT(tokenAnalyzer != NULL);
    NGIS_ASSERT(value != NULL);
    
    length = ngislTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngisErrorPrint(NULL, fName, "There is no more token.\n");
        return 0;
    }

    p = tokenAnalyzer->ngta_pointer;
    errno = 0;
    lret = strtol(p, &endp, 0);
    if (errno != 0) {
        ngisErrorPrint(NULL, fName, "strtol: %s\n", strerror(errno));
        return 0;
    }
    
    if (endp != (p + length)) {
        ngisErrorPrint(NULL, fName, "token %lu, endp %lu\n",
            (unsigned long)length, (unsigned long)(endp - p));
        return 0;
    }

    if ((lret < INT_MIN) || (lret > INT_MAX)) {
        ngisErrorPrint(NULL, fName, "Overflow.\n");
        return 0;
    }

    ngisTokenAnalyzerNext(tokenAnalyzer);
    *value = (int)lret;

    return 1;
}

char *
ngisTokenAnalyzerGetString(
    ngisTokenAnalyzer_t *tokenAnalyzer)
{
    char *ret = NULL;
    size_t length;
    static const char fName[] = "ngisTokenAnalyzerGetString";

    NGIS_ASSERT(tokenAnalyzer != NULL);

    length = ngislTokenAnalyzerGetTokenLength(tokenAnalyzer);
    if (length == 0) {
        ngisErrorPrint(NULL, fName, "There is no more token.\n");
        return NULL;
    }

    ret = ngisStrndup(tokenAnalyzer->ngta_pointer, length);
    if (ret == NULL) {
        ngisErrorPrint(NULL, fName, "Can't copy string.\n");
        return NULL;
    }
    ngisTokenAnalyzerNext(tokenAnalyzer);

    return ret;
}


int
ngisTokenAnalyzerNext(
    ngisTokenAnalyzer_t *tokenAnalyzer)
{
    const char *p;
    static const char fName[] = "ngisTokenAnalyzerNext";

    NGIS_ASSERT(tokenAnalyzer != NULL);

    ngisDebugPrint(NULL, fName, "rest=\"%s\".\n", tokenAnalyzer->ngta_pointer);

    p = tokenAnalyzer->ngta_pointer;
    while ((*p != '\0') && (!isspace((int)*p))) {
        p++;
    }

    while ((*p != '\0') && (isspace((int)*p))) {
        p++;
    }
    tokenAnalyzer->ngta_pointer = p;

    return *p != '\0';
}

static size_t
ngislTokenAnalyzerGetTokenLength(
    ngisTokenAnalyzer_t *tokenAnalyzer)
{
    size_t length = 0;
    const char *p;
#if 0
    static const char fName[] = "ngislTokenAnalyzerGetTokenLength";
#endif

    p = tokenAnalyzer->ngta_pointer;
    while ((*p != '\0') && (!isspace((int)*p))) {
        p++;
        length++;
    }

    return length;
}

int
ngisEnvironmentAnalyze(
    char *in,
    char **name,
    char **value)
{
    char *tmp;
    char *envVal = NULL;
    char *envName = NULL;
    static const char fName[] = "ngisEnvironmentAnalyze";

    NGIS_ASSERT(in != NULL);
    NGIS_ASSERT(name != NULL);
    NGIS_ASSERT(value != NULL);

    if (strlen(in) == 0) {
        ngisErrorPrint(NULL, fName, "Environment variable is empty string.\n");
        goto error;
    }

    /* Value */
    tmp = strstr(in, "=");
    if (tmp == in) {
        /* "=" appear at first */
        ngisErrorPrint(NULL, fName, "Environment variable's name is empty.\n");
        goto error;
    }

    if (tmp == NULL) {
        /* Name */
        envName= strdup(in);
        if (envName== NULL) {
            ngisErrorPrint(NULL, fName, "Can't copy string.\n");
            goto error;
        }
        envVal = strdup("");
        if (envName== NULL) {
            ngisErrorPrint(NULL, fName, "Can't create string.\n");
            goto error;
        }

    } else {
        /* Name */
        envName = ngisStrndup(in, tmp - in);
        if (envName== NULL) {
            ngisErrorPrint(NULL, fName, "Can't copy string.\n");
            goto error;
        }
        envVal = ngisShellQuote(tmp + 1);
        if (value == NULL) {
            ngisErrorPrint(NULL, fName, "Can't quote string.\n");
            goto error;
        }
    }

    *name  = envName;
    *value = envVal;

    return 1;
error:
    NGIS_NULL_CHECK_AND_FREE(envName);
    NGIS_NULL_CHECK_AND_FREE(envVal);

    return 0;
}

char *
ngisMakeTempFile(
    const char *tmpDir)
{
    char *filename = NULL;
    int result;
    int fileCreated = 0;
    int fd;
    static const char fName[] = "ngisMakeTempFile";

    if (tmpDir == NULL) {
        tmpDir = getenv("TMPDIR");
    } 
    if (tmpDir == NULL) {
        tmpDir = "/tmp";
    } 

    filename = ngisStrdupPrintf("%s/ngis-sshXXXXXX", tmpDir);
    if (filename == NULL) {
        ngisErrorPrint(NULL, fName, "Can't copy the string.\n");
        goto error;
    }

    fd = mkstemp(filename);
    if (fd < 0) {
        ngisErrorPrint(NULL, fName, "Can't make temporary file: %s.\n",
            strerror(errno));
        goto error;
    }
    fileCreated = 1;

    result = close(fd);
    if (result < 0) {
        ngisErrorPrint(NULL, fName, "close: %s\n", strerror(errno));
        goto error;
    }

    return filename;
error:
    if (fileCreated != 0) {
        NGIS_ASSERT(filename != NULL);
        result = unlink(filename);
        if (result < 0) {
            ngisErrorPrint(NULL, fName, "unlink: %s\n", strerror(errno));
        }
        fileCreated = 0;
    }
    NGIS_NULL_CHECK_AND_FREE(filename);

    return NULL;
}
