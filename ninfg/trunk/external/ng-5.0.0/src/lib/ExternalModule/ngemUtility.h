/*
 * $RCSfile: ngemUtility.h,v $ $Revision: 1.4 $ $Date: 2008/02/25 05:21:47 $
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
#ifndef _NGEM_UTILITY_H_
#define _NGEM_UTILITY_H_

#include "ngemEnvironment.h"
#include "ngemType.h"

/* MACROS */
#define NGEM_LOGCAT_UTILITY "EM Utility"

#define NGEM_MAX(lhs, rhs) ((lhs) > (rhs)?(lhs):(rhs))
#define NGEM_MIN(lhs, rhs) ((lhs) < (rhs)?(lhs):(rhs))

#define NGEM_STRING_IS_NONZERO(string) \
    (((string) != NULL) && (strlen(string) > 0))

#define NGEM_PIPE_IN(pipe)   ((pipe)[0])
#define NGEM_PIPE_OUT(pipe)  ((pipe)[1])

#define NGEM_ASSERT(cond) \
    ((void)((cond) || ngemAssertFailed(__FILE__, __LINE__, #cond)))
#define NGEM_ABORT()             ngemAbort(__FILE__, __LINE__)
#define NGEM_ASSERT_NOTREACHED()   NGEM_ABORT() 
#define NGEM_ASSERT_STRING(string) NGEM_ASSERT(NGEM_STRING_IS_NONZERO(string))
#define NGEM_ASSERT_FD(fd)         NGEM_ASSERT(fd >= 0)

#define NGEM_NULL_CHECK_AND_DESTROY(pointer, destructor) \
    do {                                  \
        if ((pointer) != NULL) {          \
            (destructor)(pointer);        \
            (pointer) = NULL;             \
        }                                 \
    } while (0)

/* For fName */
/* WARNING: This macro must be last in variable declaration */
#define NGEM_FNAME(func_name) static const char fName[] = #func_name
#define NGEM_FNAME_TAG(fName) /* EMPTY */

/* FD set(STDIN, STDOUT, STDERR) */
typedef struct ngemStandardIO_s {
    int ngsio_in;
    int ngsio_out;
    int ngsio_error;
} ngemStandardIO_t;

typedef struct ngemStringBuffer_s {
    char  *ngsb_buffer;
    size_t ngsb_length;
    size_t ngsb_capacity;
} ngemStringBuffer_t;
#define NGEM_STRING_BUFFER_NULL {NULL, 0, 0}

typedef struct ngemTokenAnalyzer_s {
    const char *ngta_target;
    const char *ngta_pointer;
} ngemTokenAnalyzer_t;

#define NGEM_SHELL "/bin/sh"
#define NGEM_STRING_BUFFER_INIT_SIZE 128

typedef struct ngemEnvironment_s {
    char *nge_name;
    char *nge_value;
} ngemEnvironment_t;

/* Invoke Process */
pid_t ngemPopen(ngemStandardIO_t *, char *);
pid_t ngemPopenArgv(ngemStandardIO_t *, char **);
pid_t ngemPopenArgvEx(ngemStandardIO_t *, char **, uid_t, gid_t, ngemEnvironment_t *);
pid_t ngemInvokeProcess(ngemStandardIO_t *, char *);
pid_t ngemInvokeProcessArgv(ngemStandardIO_t *, char **);
pid_t ngemInvokeProcessArgvEx(ngemStandardIO_t *, char **, uid_t, gid_t, ngemEnvironment_t *);

/* PIPE */
ngemResult_t ngemNonblockingPipe(int[2]);

ngemResult_t ngemFDsetExecOnCloseFlag(int);
ngemResult_t ngemFDsetNonblockFlag(int);

/* For string */
char *ngemStrdupPrintf(const char *, ...) NG_ATTRIBUTE_PRINTF(1, 2);
char *ngemStrdupVprintf(const char *, va_list);
char *ngemShellQuote(const char *);
void ngemStringStrip(char *);

/* String Buffer */
ngemResult_t ngemStringBufferInitialize(ngemStringBuffer_t *);
ngemResult_t ngemStringBufferReset(ngemStringBuffer_t *);
void ngemStringBufferFinalize(ngemStringBuffer_t *);

ngemResult_t ngemStringBufferAppend(ngemStringBuffer_t *, const char *);
ngemResult_t ngemStringBufferNappend(ngemStringBuffer_t *, const char *, size_t);
ngemResult_t ngemStringBufferFormat(ngemStringBuffer_t *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(2, 3);
ngemResult_t ngemStringBufferVformat(ngemStringBuffer_t *, const char *, va_list);

char *ngemStringBufferRelease(ngemStringBuffer_t *);
char *ngemStringBufferGet(ngemStringBuffer_t *);

/* Token Analyzer */
void ngemTokenAnalyzerInitialize(ngemTokenAnalyzer_t *, const char *);
void ngemTokenAnalyzerFinalize(ngemTokenAnalyzer_t *);

int ngemTokenAnalyzerGetEnum(ngemTokenAnalyzer_t *, const char **, size_t);
int ngemTokenAnalyzerGetInt(ngemTokenAnalyzer_t *, int *);
char *ngemTokenAnalyzerGetString(ngemTokenAnalyzer_t *);
ngemResult_t ngemTokenAnalyzerNext(ngemTokenAnalyzer_t *);
bool ngemTokenAnalyzerHasNext(ngemTokenAnalyzer_t *);

/* Assert */
int ngemAssertFailed(const char *, int, const char *);
void ngemAbort(const char *, int);

/* Others */
ngemResult_t ngemEnvironmentAnalyze(char *in, char **name, char **value);
char *ngemMakeTempFile(const char *);
ngemResult_t ngemStandardIOclose(ngemStandardIO_t *);
bool ngemStringCompare(const char *, ssize_t, const char *, ssize_t);

#endif /* _NGEM_UTILITY_H_ */
