/*
 * $RCSfile: ngisUtility.h,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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
#ifndef _NGIS_UTILITY_H_
#define _NGIS_UTILITY_H_

#include "ngEnvironment.h"

/* MACROS */
#define NGIS_NELEMENTS(array) (sizeof (array) / sizeof (array[0]))

#define NGIS_MAX(lhs, rhs) ((lhs) > (rhs)?(lhs):(rhs))
#define NGIS_MIN(lhs, rhs) ((lhs) < (rhs)?(lhs):(rhs))

#define NGIS_STRING_IS_NONZERO(string) \
    (((string) != NULL) && (strlen(string) > 0))

#define NGIS_PIPE_IN(pipe)   ((pipe)[0])
#define NGIS_PIPE_OUT(pipe)  ((pipe)[1])

#define NGIS_ASSERT(cond) \
    ((void)((cond) || ngisAssertFailed(__FILE__, __LINE__, #cond)))
#define NGIS_ABORT()             ngisAbort(__FILE__, __LINE__)
#define NGIS_ASSERT_NOTREACHED() NGIS_ABORT() 
#define NGIS_ASSERT_STRING(string) NGIS_ASSERT(NGIS_STRING_IS_NONZERO(string))
#define NGIS_ASSERT_FD(fd)         NGIS_ASSERT(fd)

#define NGIS_ALLOC(type)            ((type *)malloc(sizeof(type)))
#define NGIS_ALLOC_ARRAY(type, num) \
    (NGIS_ASSERT((num) > 0), (type *)malloc(sizeof(type) * (num)))
#define NGIS_FREE(pointer)          (free(pointer))

#define NGIS_NULL_CHECK_AND_FREE(pointer) \
    do {                                  \
        if ((pointer) != NULL) {          \
            free(pointer);                \
            (pointer) = NULL;             \
        }                                 \
    } while (0)

#define NGIS_NULL_CHECK_AND_DESTROY(pointer, destructor) \
    do {                                  \
        if ((pointer) != NULL) {          \
            (destructor)(pointer);        \
            (pointer) = NULL;             \
        }                                 \
    } while (0)

/* FD set(STDIN, STDOUT, STDERR) */
typedef struct ngisStandardIO_s {
    int ngsio_in;
    int ngsio_out;
    int ngsio_error;
} ngisStandardIO_t;

typedef struct ngisStringBuffer_s {
    char  *ngsb_buffer;
    size_t ngsb_length;
    size_t ngsb_capacity;
} ngisStringBuffer_t;

typedef struct ngisTokenAnalyzer_s {
    const char *ngta_target;
    const char *ngta_pointer;
} ngisTokenAnalyzer_t;

#define NGIS_SHELL "/bin/sh"
#define NGIS_STRING_BUFFER_INIT_SIZE 128

/* Invoke Process */
pid_t ngisPopen(ngisStandardIO_t *, char *);
pid_t ngisPopenArgv(ngisStandardIO_t *io, char **);
pid_t ngisInvokeProcess(ngisStandardIO_t *, char *);
pid_t ngisInvokeProcessArgv( ngisStandardIO_t *, char **);

/* PIPE */
int ngisNonblockingPipe(int[2]);
int ngisFDsetExecOnCloseFlag(int);

/* For string */
char *ngisStrdupPrintf(const char *, ...) NG_ATTRIBUTE_PRINTF(1, 2);
char *ngisStrdupVprintf(const char *, va_list);
char *ngisStrndup(const char *, size_t);
char *ngisShellQuote(const char *);
void ngisStringStrip(char *);

/* String Buffer */
int ngisStringBufferInitialize(ngisStringBuffer_t *);
void ngisStringBufferFinalize(ngisStringBuffer_t *);

int ngisStringBufferAppend(ngisStringBuffer_t *, const char *);
int ngisStringBufferFormat(ngisStringBuffer_t *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(2, 3);
int ngisStringBufferVformat(ngisStringBuffer_t *, const char *, va_list);

char *ngisStringBufferRelease(ngisStringBuffer_t *);

/* Token Analyzer */
void ngisTokenAnalyzerInitialize(ngisTokenAnalyzer_t *, const char *);
void ngisTokenAnalyzerFinalize(ngisTokenAnalyzer_t *);

int ngisTokenAnalyzerGetEnum(ngisTokenAnalyzer_t *, const char **, size_t);
int ngisTokenAnalyzerGetInt(ngisTokenAnalyzer_t *, int *);
char *ngisTokenAnalyzerGetString(ngisTokenAnalyzer_t *);
int ngisTokenAnalyzerNext(ngisTokenAnalyzer_t *);

/* Assert */
int ngisAssertFailed(const char *, int, const char *);
void ngisAbort(const char *, int);

/* Others */
int ngisEnvironmentAnalyze(char *in, char **name, char **value);
char *ngisMakeTempFile(const char *);
int ngisStandardIOclose(ngisStandardIO_t *);

#endif /* _NGIS_UTILITY_H_ */
