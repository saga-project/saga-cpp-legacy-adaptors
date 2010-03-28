/*
 * $RCSfile: ngUtility.c,v $ $Revision: 1.28 $ $Date: 2008/03/03 05:56:17 $
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
 * Utility module for Ninf-G.
 */

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngUtility.c,v $ $Revision: 1.28 $ $Date: 2008/03/03 05:56:17 $")

/**
 * Prototype declaration of internal functions.
 */
static int nglLineListInitialize(
    ngiLineList_t *lineList, ngLog_t *log, int *error);
static int nglLineListFinalize(
    ngiLineList_t *lineList, ngLog_t *log, int *error);
static void nglLineListInitializeMember(ngiLineList_t *lineList);
static ngiLineListLine_t *nglLineListLineConstruct(
    ngLog_t *log, int *error);
static int nglLineListLineDestruct(
    ngiLineListLine_t *line, ngLog_t *log, int *error);
static int nglLineListLineInitialize(
    ngiLineListLine_t *line, ngLog_t *log, int *error);
static int nglLineListLineFinalize(
    ngiLineListLine_t *line, ngLog_t *log, int *error);
static void nglLineListLineInitializeMember(ngiLineListLine_t *line);
static int nglLineListLineRegister(
    ngiLineList_t *lineList, ngiLineListLine_t *line,
    ngLog_t *log, int *error);
static int nglLineListLineUnregister(
    ngiLineList_t *lineList, ngiLineListLine_t *line,
    ngLog_t *log, int *error);

static void nglConnectRetryInformationInitializeMember(
    ngiConnectRetryInformation_t *retryInfo);
static void nglConnectRetryInitializeMember(
    ngiConnectRetryStatus_t *retryStatus);
static void nglRandomNumberInitializeMember(
    ngiRandomNumber_t *randomNumberStatus);


/**
 * Functions.
 */

/**
 * malloc()
 */
void *
ngiMalloc(
    size_t size,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiMalloc";
    void *obj;

    /* Check the arguments */
    if (size <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid size.\n");
        return NULL;
    }

    /* Allocate */
    obj = malloc(size);
    if (obj == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n", "malloc()", errno, strerror(errno));
        return NULL;
    }

    /* Success */
    return obj;
}

/**
 * calloc()
 */
void *
ngiCalloc(
    size_t nElements,
    size_t elementSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCalloc";
    void *obj;

    /* Check the arguments */
    if (nElements <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid nElements.\n");
        return NULL;
    }

    if (elementSize <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid elementSize.\n");
        return NULL;
    }

    /* Allocate */
    obj = calloc(nElements, elementSize);
    if (obj == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "calloc()", errno, strerror(errno));
        return NULL;
    }

    /* Success */
    return obj;
}

/**
 * realloc()
 */
void *
ngiRealloc(
    void *oldObject,
    size_t newSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRealloc";
    void *obj;

    /* Check the arguments */
    if (newSize <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid size.\n");
        return NULL;
    }

    /* realloc */
    obj = realloc(oldObject, newSize);
    if (obj == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "realloc()", errno, strerror(errno));
        return NULL;
    }

    /* Success */
    return obj;
}

/**
 * free()
 */
int
ngiFree(
    void *target,
    ngLog_t *log,
    int *error)
{

    /* Check the arguments */
    if (target == NULL) {

        /* Success */
        return 1;
    }

    /* Deallocate */
    free(target);

    /* Success */
    return 1;
}

/**
 * strdup()
 */
char *
ngiStrdup(
    const char *src,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiStrdup";
    char *copy;

    /* Check the arguments */
    if (src == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. src is NULL.\n");
        return NULL;
    }

    /* Duplicate */
    copy = strdup(src);
    if (copy == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "strdup()", errno, strerror(errno));
        return NULL;
    }

    /* Success */
    return copy;
}

/**
 * Creates a duplicate of head "n" bytes of string.
 * On success, Null-terminated string is returned.
 * It must be freed using ngiFree() after it is used.
 * On error, NULL is returned.
 */
char *
ngiStrndup(
    const char *src,
    size_t n,
    ngLog_t *log,
    int *error)
{
    char *copy = NULL;
    int i;
    static const char fName[] = "ngiStrndup";

    /* Check the arguments */
    if (src == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. src is NULL.\n");
        return NULL;
    }

    /* Get Size */
    for (i = 0;(i < n) && (src[i] != '\0');++i);
    n = i;

    copy = ngiMalloc(n + 1, log, error);
    if (copy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for a string.\n");
        return NULL;
    }
    
    strncpy(copy, src, n);
    copy[n] = '\0';
    
    return copy;
}

/**
 * Allocate Memory
 * Note: This function is used via Macro NGI_ALLOCATE().
 */
void *
ngiAllocate(
    size_t nObj,
    size_t size,
    const char *typeName,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiAllocate";
    void *obj = NULL;

    /* Check the arguments */
    if ((typeName == NULL) || (strlen(typeName) == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. typeName is NULL.\n");
        return 0;
    }

    if (nObj == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. number of object is 0.\n");
        return 0;
    }

    /* realloc */
    obj = ngiCalloc(nObj, size, log ,error);
    if (obj == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Can't allocate storage for %s.\n", typeName);
        return NULL;
    }

    /* Success */
    return obj;
}

/**
 * Dellocate Memory
 * Note: This function is used via Macro NGI_DEALLOCATE().
 */
int
ngiDeallocate(
    void *ptr,
    const char *typeName,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiDeallocate";

    /* Check the arguments */
    if ((typeName == NULL) || (strlen(typeName) == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. typeName is NULL.\n");
        return 0;
    }

    ngiFree(ptr, log, error);

    /* Success */
    return 1;
}

/**
 * gethostname(),
 * which try to get FQDN and NG_HOSTNAME environment variable.
 */
int
ngiHostnameGet(
    char *name,
    size_t length,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiHostnameGet";
    struct addrinfo addrHints;
    struct addrinfo *addrList;
    char tmpName[NGI_HOST_NAME_MAX], *tmp;
    int result;

    tmp = NULL;
    tmpName[0] = '\0';

    /* Check the arguments */
    if ((name == NULL) || (length == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument.\n");
        return 0;
    }

    /* Clear */
    memset(name, 0, length);

    /* Check the environment variable. */
    tmp = getenv(NGI_HOSTNAME_ENVIRONMENT_NAME);
    if (tmp != NULL) {
        if (strcmp(tmp, "") != 0) {
            if (strlen(tmp) >= length) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "hostname buffer is too short.\n");
                return 0;
            }

            strncpy(name, tmp, length);

            return 1;
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "empty %s environment variable was set.\n",
                NGI_HOSTNAME_ENVIRONMENT_NAME);
        }
    }

    /* Get the hostname, which is sometime not FQDN. */
    result = gethostname(tmpName, sizeof(tmpName));
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "gethostname()", errno, strerror(errno));
        return 0;
    }

    memset(&addrHints, 0, sizeof(addrHints));
    addrHints.ai_family = PF_INET; /* or AF_UNSPEC ? */
    addrHints.ai_socktype = SOCK_STREAM;
    addrHints.ai_protocol = 0;

    /* Get the IP address. */
    addrList = NULL;
    result = getaddrinfo(
        tmpName, NULL, &addrHints, &addrList);
    if ((result != 0) || (addrList == NULL)) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "getaddrinfo()", result, gai_strerror(result));
        return 0;
    }

    /* Get the hostname, which is maybe a FQDN. */
    result = getnameinfo(
        addrList->ai_addr, addrList->ai_addrlen,
        name, length, NULL, 0, NI_NAMEREQD);
    if (result != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "getnameinfo()", result, gai_strerror(result));
        return 0;
    }

    freeaddrinfo(addrList);

    /* Success */
    return 1;
}

/**
 * getpwuid.
 */
int
ngiGetpwuid(
    uid_t uid,
    struct passwd **pwbufp,
    char **bufp,
    ngLog_t *log,
    int *error)
{
    char *buf;
    int result;
    long bufSize;
    struct passwd *pw;
    struct passwd *pwp;
    static const char fName[] = "ngiGetpwuid";

    assert(pwbufp != NULL);
    assert(bufp != NULL);

    pw = NULL;
    pwp = NULL;
    buf = NULL;

    *pwbufp = NULL;
    *bufp = NULL;

    /* Get buffer size */
    bufSize = NGI_GETPWUID_BUFSIZE;

#ifdef HAVE_SYSCONF
    bufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufSize < 0) {
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "sysconf()", errno, strerror(errno));

        bufSize = NGI_GETPWUID_BUFSIZE;
        ngLogInfo(log, NG_LOGCAT_NINFG_LIB, fName,
            "use default buf size for getpwuid_r().\n");
    }
#endif

    /* Allocate the buffer */
    pw = ngiCalloc(1, sizeof (struct passwd), log, error);
    if (pw == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate the storage for struct passwd.\n");
        goto error;
    }
    buf = ngiCalloc(bufSize, 1, log, error);
    if (buf == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate the storage for char buf.\n");
        goto error;
    }

#ifdef NGI_GETPWUID_R_ENABLE
    result = getpwuid_r(uid, pw, buf, bufSize, &pwp);
    if (result != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
             "getpwuid_r()", result, strerror(result));
        goto error;
    }

#else /* NGI_GETPWUID_R_ENABLE */
    result = 0;
    assert(result == 0);

    pwp = getpwuid(uid);
    if (pwp == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "%s failed: %d: %s.\n",
            "getpwuid()", errno, strerror(errno));
        goto error;
    }
    *pw = *pwp;

#endif /* NGI_GETPWUID_R_ENABLE */

    *pwbufp = pw;
    *bufp = buf;

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngiFree(buf, log, NULL);
    buf = NULL;

    ngiFree(pw, log, NULL);
    pw = NULL;

    /* Failed */
    return 0;
}

/**
 * localtime.
 * Note: This function is called from ngLog.c.
 */
int
ngiLocalTime(
    time_t timer,
    struct tm *resultTm,
    ngLog_t *log,
    int *error)
{
    struct tm ltime, *tmpTm;

    assert(resultTm != NULL);

    tmpTm = NULL;

#ifdef NG_PTHREAD

    localtime_r(&timer, &ltime);

#else /* NG_PTHREAD */

    tmpTm = localtime(&timer);

    ltime = *tmpTm;

#endif /* NG_PTHREAD */

    *resultTm = ltime;

    /* Success */
    return 1;
}

/**
 * ngiReleasePasswd
 */
int
ngiReleasePasswd(
    struct passwd *pwbufp,
    char *bufp,
    ngLog_t *log,
    int *error)
{
    assert(pwbufp != NULL);
    assert(bufp != NULL);

    ngiFree(pwbufp, log, error);
    ngiFree(bufp, log, error);

    return 1;
}

/**
 * LineList: Construct.
 */
ngiLineList_t *
ngiLineListConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiLineListConstruct";
    ngiLineList_t *lineList;
    int result;

    lineList = NULL;

    /* Allocate */
    lineList = NGI_ALLOCATE(ngiLineList_t, log, error);
    if (lineList == NULL) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the Line List failed.\n");
        goto error;
    }

    /* Initialize */
    result = nglLineListInitialize(lineList, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Line List failed.\n");
        goto error;
    }

    /* Success */
    return lineList;

    /* Error occurred */
error:
    /* Failed */
    return NULL;
}

/**
 * LineList: Destruct.
 */
int
ngiLineListDestruct(
    ngiLineList_t *lineList,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiLineListDestruct";
    int result;

    /* Check the arguments */
    if (lineList == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Line List is NULL.\n");
        return 0;
    }

    /* Finalize */
    result = nglLineListFinalize(lineList, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Line List failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiLineList_t, lineList, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the Line List failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * LineList: Initialize.
 */
static int
nglLineListInitialize(
    ngiLineList_t *lineList,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(lineList != NULL);

    nglLineListInitializeMember(lineList);

    lineList->ngll_tmpBufferSize = 0;
    lineList->ngll_tmpBuffer = NULL;

    lineList->ngll_nLines = 0;
    lineList->ngll_lines_head = NULL;

    /* Success */
    return 1;
}

/**
 * LineList: Finalize.
 */
static int
nglLineListFinalize(
    ngiLineList_t *lineList,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglLineListFinalize";
    ngiLineListLine_t *line, *nextLine;
    int result;

    line = NULL;

    /* Check the arguments */
    assert(lineList != NULL);

    /* Deallocate the tmp buffer. */
    if (lineList->ngll_tmpBuffer != NULL) {
        ngiFree(lineList->ngll_tmpBuffer, log, error);
    }
    lineList->ngll_tmpBuffer = NULL;
    lineList->ngll_tmpBufferSize = 0;

    /* Deallocate the Lines. */
    line = lineList->ngll_lines_head;
    while (line != NULL) {
        nextLine = line->nglll_next;

        result = nglLineListLineUnregister(
            lineList, line, log, error);
        if (result == 0) {
            ngLogError(log,
                NG_LOGCAT_NINFG_LIB, fName,
                "Unregister the Line failed.\n");
            goto error;
        }

        result = nglLineListLineDestruct(line, log, error);
        if (result == 0) {
            ngLogError(log,
                NG_LOGCAT_NINFG_LIB, fName,
                "Destruct the Line failed.\n");
            goto error;
        }

        line = nextLine;
    }

    nglLineListInitializeMember(lineList);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * LineList: Initialize the members.
 */
static void
nglLineListInitializeMember(
    ngiLineList_t *lineList)
{
    /* Check the arguments */
    assert(lineList != NULL);

    lineList->ngll_tmpBufferSize = 0;
    lineList->ngll_tmpBuffer = NULL;
    lineList->ngll_nLines = 0;
    lineList->ngll_lines_head = NULL;
}

/**
 * LineListLine: Construct.
 */
static ngiLineListLine_t *
nglLineListLineConstruct(
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglLineListLineConstruct";
    ngiLineListLine_t *line;
    int result;

    line = NULL;

    /* Allocate */
    line = NGI_ALLOCATE(ngiLineListLine_t, log, error);
    if (line == NULL) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Allocate the Line failed.\n");
        goto error;
    }

    /* Initialize */
    result = nglLineListLineInitialize(line, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Initialize the Line failed.\n");
        goto error;
    }


    /* Success */
    return line;

    /* Error occurred */
error:
    /* Failed */
    return NULL;
}

/**
 * LineListLine: Destruct.
 */
static int
nglLineListLineDestruct(
    ngiLineListLine_t *line,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglLineListLineDestruct";
    int result;

    /* Check the arguments */
    assert(line != NULL);

    /* Finalize */
    result = nglLineListLineFinalize(line, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Finalize the Line failed.\n");
        goto error;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngiLineListLine_t, line, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Deallocate the Line failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * LineListLine: Initialize.
 */
static int
nglLineListLineInitialize(
    ngiLineListLine_t *line,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(line != NULL);

    nglLineListLineInitializeMember(line);

    /* Success */
    return 1;
}

/**
 * LineListLine: Finalize.
 */
static int
nglLineListLineFinalize(
    ngiLineListLine_t *line,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(line != NULL);

    line->nglll_next = NULL;

    assert(line->nglll_line != NULL);
    ngiFree(line->nglll_line, log, error);
    line->nglll_line = NULL;

    nglLineListLineInitializeMember(line);

    /* Success */
    return 1;
}

/**
 * LineListLine: Initialize the members.
 */
static void
nglLineListLineInitializeMember(
    ngiLineListLine_t *line)
{
    /* Check the arguments */
    assert(line != NULL);

    line->nglll_next = NULL;
    line->nglll_line = NULL;
}

/**
 * LineList: Register LineListLine.
 */
static int
nglLineListLineRegister(
    ngiLineList_t *lineList,
    ngiLineListLine_t *line,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglLineListLineRegister";
    ngiLineListLine_t **tail;
    int count;

    /* Check the arguments */
    assert(lineList != NULL);
    assert(line != NULL);
    assert(line->nglll_next == NULL);

    count = 0;

    /* Find the tail. */
    tail = &lineList->ngll_lines_head;
    while (*tail != NULL) {
        if (*tail == line) {
            NGI_SET_ERROR(error, NG_ERROR_ALREADY);
            ngLogError(log,
                NG_LOGCAT_NINFG_LIB, fName,
                "The Line is already registered.\n");
            goto error;
        }

        tail = &(*tail)->nglll_next;
        count++;
    }

    *tail = line;
    count++;

    lineList->ngll_nLines = count;

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * LineList: Unregister LineListLine.
 */
static int
nglLineListLineUnregister(
    ngiLineList_t *lineList,
    ngiLineListLine_t *line,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglLineListLineUnregister";
    ngiLineListLine_t *cur, **prevPtr;
    int count;

    /* Check the arguments */
    assert(lineList != NULL);
    assert(line != NULL);

    /* Count the number of Lines. */
    count = 0;
    cur = lineList->ngll_lines_head;
    while (cur != NULL) {
        count++;
        cur = cur->nglll_next;
    }
    lineList->ngll_nLines = count;

    /* Delete the data from the list. */
    prevPtr = &lineList->ngll_lines_head;
    cur = lineList->ngll_lines_head;
    for (; cur != NULL; cur = cur->nglll_next) {
        if (cur == line) {
            /* Unlink the list */
            *prevPtr = cur->nglll_next;
            line->nglll_next = NULL;
            count--;
            lineList->ngll_nLines = count;

            /* Success */
            return 1;
        }
        prevPtr = &cur->nglll_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log,
        NG_LOGCAT_NINFG_LIB, fName,
        "The Line is not registered.\n");
    return 0;
}

/**
 * LineList: Append.
 */
int
ngiLineListAppend(
    ngiLineList_t *lineList,
    char *newLine,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiLineListAppend";
    ngiLineListLine_t *appendLine;
    char *appendString;
    int result;

    appendString = NULL;
    appendLine = NULL;

    /* Check the arguments */
    if (lineList == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Line List is NULL.\n");
        goto error;
    }

    if (newLine == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. new Line is NULL.\n");
        goto error;
    }

    /* Duplicate. */
    appendString = ngiStrdup(newLine, log, error);
    if (appendString == NULL) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Duplicate the line string failed.\n");
        goto error;
    }

    /* Construct. */
    appendLine = nglLineListLineConstruct(log, error);
    if (appendLine == NULL) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Construct the Line failed.\n");
        goto error;
    }

    appendLine->nglll_line = appendString;

    /* Register. */
    result = nglLineListLineRegister(lineList, appendLine, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Register the Line failed.\n");
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * LineList: printf.
 */
int
ngiLineListPrintf(
    ngiLineList_t *lineList,
    ngLog_t *log,
    int *error,
    char *format,
    ...)
{
    static const char fName[] = "ngiLineListPrintf";
    int newSize, done, size, length, result;
    char *newBuffer, *buffer;
    va_list ap;

    /* Check the arguments */
    if (lineList == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Line List is NULL.\n");
        return 0;
    }
    if (format == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. format is NULL.\n");
        return 0;
    }

    /* Create the tmp buffer for sprintf. */
    if (lineList->ngll_tmpBuffer == NULL) {
        newSize = NGI_LINE_LIST_LINE_TMP_BUFFER_INITIAL_SIZE;
        newBuffer = ngiCalloc(newSize, sizeof(char), log, error);
        if (newBuffer == NULL) {
            ngLogError(log,
                NG_LOGCAT_NINFG_LIB, fName,
                "Allocate the buffer failed.\n");
            return 0;
        }

        lineList->ngll_tmpBuffer = newBuffer;
        lineList->ngll_tmpBufferSize = newSize;
    }

    va_start(ap, format);

    /* Output the printf format. */
    done = 0;
    while (done == 0) {
        buffer = lineList->ngll_tmpBuffer;
        size = lineList->ngll_tmpBufferSize;

        length = vsnprintf(buffer, size, format, ap);
        if (length < 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log,
                NG_LOGCAT_NINFG_LIB, fName,
                "Print to the buffer failed.\n");
            goto error;
        }
        if ((length >= size) || (length == (size - 1))) {
            done = 0;

            ngiFree(lineList->ngll_tmpBuffer, log, error);
            lineList->ngll_tmpBuffer = NULL;
            lineList->ngll_tmpBufferSize = 0;

            newSize = size * 2;
            /* No length + 1 size are used, to reuse the buffer. */

            newBuffer = ngiCalloc(newSize, sizeof(char), log, error);
            if (newBuffer == NULL) {
                ngLogError(log,
                    NG_LOGCAT_NINFG_LIB, fName,
                    "Allocate the buffer failed.\n");
                return 0;
            }
            lineList->ngll_tmpBuffer = newBuffer;
            lineList->ngll_tmpBufferSize = newSize;
        } else {
            done = 1;
        }
    }

    va_end(ap);

    /* Register the string. */
    result = ngiLineListAppend(
        lineList, lineList->ngll_tmpBuffer, log, error);
    if (result == 0) {
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Append the line to Line List failed.\n");
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * LineList: Get next Line.
 * Note: Returned string must not free(). Just reference.
 * Note: Each LineListLine->nglll_line will not shared and not NULL.
 */
char *
ngiLineListLineGetNext(
    ngiLineList_t *lineList,
    char *line,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiLineListLineGetNext";
    ngiLineListLine_t *cur;
    char *returnLine;

    returnLine = NULL;

    /* Check the arguments */
    if (lineList == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log,
            NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument. Line List is NULL.\n");
        return 0;
    }

    /* IS Head Item required? */
    if (line == NULL) {
        if (lineList->ngll_lines_head == NULL) {
            returnLine = NULL;
        } else {
            returnLine = lineList->ngll_lines_head->nglll_line;
        }
        
        return returnLine;
    }

    /* Find the line. */
    cur = lineList->ngll_lines_head;
    while (cur != NULL) {
        if (cur->nglll_line == line) {
            /* found. */
            if (cur->nglll_next == NULL) {
                returnLine = NULL;
            } else {
                returnLine = cur->nglll_next->nglll_line;
            }

            return returnLine;
        }
        cur = cur->nglll_next;
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngLogError(log,
        NG_LOGCAT_NINFG_LIB, fName,
        "The Line is not found.\n");
    return 0;
}

#ifdef NGI_POLL_ENABLED

/**
 * Sleep by struct timeval
 */
int
ngiSleepTimeval(
    struct timeval *sleepTime,
    int returnByEINTR,
    ngLog_t *log,
    int *error)
{
    int result;
    long sleepMilliSec;
    char tvBuf[NGI_TIMEVAL_STRING_MAX];
    struct timeval timeNow, timeEnd, timeTmp;
    static const char fName[] = "ngiSleepTimeval";

    /* Check the arguments */
    assert(sleepTime != NULL);

    /* log */
    result = ngiTimevalToString(tvBuf, sizeof(tvBuf), sleepTime, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Convert the time to string failed.\n"); 
        return 0;
    }
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Sleeping %s.\n", tvBuf); 

    /* Get finish time */
    result = gettimeofday(&timeNow, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "gettimeofday() failed by %d: %s.\n", errno, strerror(errno)); 
        return 0;
    }

    timeEnd = ngiTimevalAdd(timeNow, *sleepTime);

    /* loop until finish time arrive */
    while(1) {
        /* Check finished */
        result = gettimeofday(&timeNow, NULL);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "gettimeofday() failed by %d: %s.\n", errno, strerror(errno)); 
            return 0;
        }

        if (ngiTimevalCompare(timeNow, timeEnd) >= 0) {
            break;
        }

        /* Get sleep time and sleep */
        timeTmp = ngiTimevalSub(timeEnd, timeNow);
        sleepMilliSec = (timeTmp.tv_sec * 1000) + (timeTmp.tv_usec / 1000);

        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "start poll() for %ld ms.\n", sleepMilliSec); 

        result = poll(NULL, 0, sleepMilliSec);
        if (result < 0) {
            if (errno == EINTR) {
                if (returnByEINTR != 0) {
                    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "return by EINTR.\n"); 
                    break;
                } else {
                    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "poll() returned by EINTR. continue poll().\n"); 
                    continue;
                }
            } else {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "poll() failed by %d: %s.\n", result, strerror(result)); 
                return 0;
            }
        }
    }
    
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "sleep finished. return.\n"); 

    /* Success */
    return 1;
}

#else /* NGI_POLL_ENABLED */
#ifdef NGI_SELECT_ENABLED

/**
 * Sleep by struct timeval
 */
int
ngiSleepTimeval(
    struct timeval *sleepTime,
    int returnByEINTR,
    ngLog_t *log,
    int *error)
{
    int result;
    char tvBuf[NGI_TIMEVAL_STRING_MAX];
    struct timeval timeNow, timeEnd, timeTmp;
    static const char fName[] = "ngiSleepTimeval";

    /* Check the arguments */
    assert(sleepTime != NULL);

    /* log */
    result = ngiTimevalToString(tvBuf, sizeof(tvBuf), sleepTime, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Convert the time to string failed.\n"); 
        return 0;
    }
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Sleeping %s.\n", tvBuf); 

    /* Get finish time */
    result = gettimeofday(&timeNow, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "gettimeofday() failed by %d: %s.\n", errno, strerror(errno)); 
        return 0;
    }

    timeEnd = ngiTimevalAdd(timeNow, *sleepTime);

    /* loop until finish time arrive */
    while(1) {
        /* Check finished */
        result = gettimeofday(&timeNow, NULL);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "gettimeofday() failed by %d: %s.\n", errno, strerror(errno)); 
            return 0;
        }

        if (ngiTimevalCompare(timeNow, timeEnd) >= 0) {
            break;
        }

        /* Get sleep time and sleep */
        timeTmp = ngiTimevalSub(timeEnd, timeNow);

        result = ngiTimevalToString(
            tvBuf, sizeof(tvBuf), &timeTmp, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Convert the time to string failed.\n"); 
            return 0;
        }
        ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
            "start select() for %s.\n", tvBuf); 

        result = select(0, NULL, NULL, NULL, &timeTmp);
        if (result < 0) {
            if (errno == EINTR) {
                if (returnByEINTR != 0) {
                    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "return by EINTR.\n"); 
                    break;
                } else {
                    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "select() returned by EINTR. continue select().\n"); 
                    continue;
                }
            } else {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "select() failed by %d: %s.\n", result, strerror(result)); 
                return 0;
            }
        }
    }
    
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "sleep finished. return.\n"); 

    /* Success */
    return 1;
}

#else /* NGI_SELECT_ENABLED */

/**
 * Sleep by struct timeval
 */
int
ngiSleepTimeval(
    struct timeval *sleepTime,
    int returnByEINTR,
    ngLog_t *log,
    int *error)
{
    int result;
    char tvBuf[NGI_TIMEVAL_STRING_MAX];
    static const char fName[] = "ngiSleepTimeval";

    /* Check the arguments */
    assert(sleepTime != NULL);

    /* log */
    result = ngiTimevalToString(tvBuf, sizeof(tvBuf), sleepTime, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Convert the time to string failed.\n"); 
        return 0;
    }
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Sleeping %s not performed. return immediately.\n", tvBuf); 

    /* Success */
    return 1;
}

#endif /* NGI_SELECT_ENABLED */
#endif /* NGI_POLL_ENABLED */

/**
 * Execution time: Set start time.
 */
int
ngiSetStartTime(ngExecutionTime_t *time, ngLog_t *log, int *error)
{
    int result;
    struct rusage rusage;
    static const char fName[] = "ngiSetStartTime";

    /* Check the arguments */
    assert(time != NULL);

    memset(&rusage, '\0', sizeof(rusage));

    /* Get the real start time */
    result = gettimeofday(&time->nget_real.nget_start, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "gettimeofday failed.\n"); 
	return 0;
    }

    /* Get the cpu start time */
    result = getrusage(RUSAGE_SELF, &rusage);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "getrusage failed.\n"); 
	return 0;
    }
    time->nget_cpu.nget_start = ngiTimevalAdd(
        rusage.ru_utime, rusage.ru_stime);

    /* Success */
    return 1;
}

/**
 * Execution time: Set end time.
 */
int
ngiSetEndTime(ngExecutionTime_t *time, ngLog_t *log, int *error)
{
    int result;
    struct rusage rusage;
    static const char fName[] = "ngiSetEndTime";

    /* Check the arguments */
    assert(time != NULL);

    memset(&rusage, '\0', sizeof(rusage));

    /* Get the real end time */
    result = gettimeofday(&time->nget_real.nget_end, NULL);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "gettimeofday failed.\n"); 
	return 0;
    }

    /* Get the cpu end time */
    result = getrusage(RUSAGE_SELF, &rusage);
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "getrusage failed.\n"); 
	return 0;
    }

    time->nget_cpu.nget_end = ngiTimevalAdd(
        rusage.ru_utime, rusage.ru_stime);

    /* Calculate the real execution time */
    time->nget_real.nget_execution = ngiTimevalSub(
        time->nget_real.nget_end, time->nget_real.nget_start);

    /* Calculate the cpu execution time */
    time->nget_cpu.nget_execution = ngiTimevalSub(
        time->nget_cpu.nget_end, time->nget_cpu.nget_start);

    /* Success */
    return 1;
}

/**
 * Convert string to time.
 */
int
ngiStringToTimeval(
    char *string,
    struct timeval *tv,
    ngLog_t *log,
    int *error)
{
    char *work, *tmp, *curr, *end;
    static const char fName[] = "ngiStringToTimeval";

    /* Check the arguments */
    assert(string != NULL);
    assert(tv != NULL);

    /* Duplicate the string */
    work = strdup(string);
    if (work == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for string.\n"); 
	return 0;
    }
    curr = work;

    /* Get the string of second */
    tmp = strchr(curr, 's');
    if (tmp == NULL)
	goto syntaxError;
    *tmp = '\0';

    /* Convert to second */
    tv->tv_sec = strtol(curr, &end, 0);
    if (end == curr)
	goto syntaxError;

    /* Get the micro second */
    curr = tmp + 1;
    if (*curr == '\0')
	goto syntaxError;
    tmp = strchr(curr, 'u');
    if ((tmp == NULL) || (tmp[1] != 's'))
	goto syntaxError;

    /* Convert to micro second */
    tv->tv_usec = strtol(curr, &end, 0);
    if (end == curr)
	goto syntaxError;

    /* Deallocate the work */
    ngiFree(work, log, error);

    /* Success */
    return 1;

    /* Syntax error */
syntaxError:
    ngiFree(work, log, NULL);

    NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Session Information: Syntax error: %s.\n", string); 
    return 0;
}

/**
 * Convert timeval to string.
 */
int
ngiTimevalToString(
    char *buf,
    int size,
    struct timeval *tv,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiTimevalToString";
    int nBytes;

    /* Check the arguments */
    assert(buf != NULL);
    assert(tv != NULL);

    nBytes = snprintf(buf, size, "%lds %ldus",
        tv->tv_sec, (long)tv->tv_usec);
    if (nBytes >= size) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "String Buffer %d overflow.\n", size); 
        return 0;
    }

    buf[size - 1] = '\0';

    /* Success */
    return 1;
}

/**
 * Debugger Information: Finalize
 */
int
ngiDebuggerInformationFinalize(
    ngDebuggerInformation_t *dbgInfo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(dbgInfo != NULL);

    /* Deallocate the members */
    ngiFree(dbgInfo->ngdi_terminalPath, log, error);
    ngiFree(dbgInfo->ngdi_display, log, error);
    ngiFree(dbgInfo->ngdi_debuggerPath, log, error);

    /* Initialize the members */
    ngiDebuggerInformationInitializeMember(dbgInfo);

    /* Success */
    return 1;
}

/**
 * Debugger Information: Initialize the members.
 */
void
ngiDebuggerInformationInitializeMember(ngDebuggerInformation_t *dbgInfo)
{
    /* Check the argument */
    assert(dbgInfo != NULL);

    /* Initialize the pointers */
    ngiDebuggerInformationInitializePointer(dbgInfo);

    /* Initialize the members */
    dbgInfo->ngdi_enable = 0;
}

/**
 * Debugger Information: Initialize the pointers.
 */
void
ngiDebuggerInformationInitializePointer(ngDebuggerInformation_t *dbgInfo)
{
    /* Check the argument */
    assert(dbgInfo != NULL);

    /* Initialize the pointers */
    dbgInfo->ngdi_display = NULL;
    dbgInfo->ngdi_terminalPath = NULL;
    dbgInfo->ngdi_debuggerPath = NULL;
}

/**
 * Temporary file: Default temporary directory name get
 */
char *
ngiDefaultTemporaryDirectoryNameGet(
    ngLog_t *log,
    int *error)
{
    char *tmpDir;
    char *ret;
    static const char fName[] = "ngiDefaultTemporaryDirectoryNameGet";

    tmpDir = getenv(NGI_ENVIRONMENT_TMPDIR);
    if (tmpDir == NULL) {
	tmpDir = NGI_TMP_DIR;
    }
    /* Print Warning If ${TMPDIR} is empty string. */
    else if (strlen(tmpDir) == 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "the TMPDIR environment variable is empty.\n"); 
    }

    ret = strdup(tmpDir);
    if (ret == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for string.\n"); 

        /* Failed */
	return NULL;
    }

    /* Success */
    return ret;
}

/**
 * Temporary file: Create
 */
char *
ngiTemporaryFileCreate(
    char *prefix,
    ngLog_t *log,
    int *error)
{
    int result;
    int length;
    int fd = -1;
    int prefix_allocated = 0;
    char *tmpFile = NULL;
    static const char fName[] = "ngiTemporaryFileCreate";

    /* Check the argument */
    if (prefix == NULL) {
	prefix = ngiDefaultTemporaryDirectoryNameGet(log, error);
	if (prefix == NULL) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get default temporary directory name.\n"); 
		goto error;
	}
	prefix_allocated = 1;
    }

    /* Allocate */
    length = strlen(prefix) + 1 + strlen(NGI_TMP_FILE) + 1;
    tmpFile = ngiCalloc(length, sizeof(char), log, error);
    if (tmpFile == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Temporary File Name.\n"); 
	goto error;
    }

    /* Copy */
    snprintf(tmpFile, length, "%s/%s", prefix, NGI_TMP_FILE);

    /* Create */
    fd = mkstemp(tmpFile);
    if (fd < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "mkstemp \"%s\" failed: %s.\n", tmpFile, strerror(errno)); 
	goto error;
    }

    /* Close */
    result = close(fd);
    if (result < 0) {
	fd = -1;
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "close failed: %s.\n", strerror(errno)); 
	goto error;
    }

    /* Deallocate prefix if allocated */
    if (prefix_allocated != 0) {
	assert(prefix != NULL);
	ngiFree(prefix, log, error);
	prefix = NULL;
        prefix_allocated = 0;
    }

    /* Success */
    return tmpFile;

    /* Error occurred */
error:
    if (fd >= 0) {
	/* Close the Temporary File */
	result = close(fd);
	if (result < 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "close failed: %s.\n", strerror(errno)); 
	}
	fd = -1;

	/* Unlink the Temporary File */
	result = unlink(tmpFile);
	if (result < 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "unlink failed: %s.\n", strerror(errno)); 
	}
    }

    /* Deallocate the Temporary File Name */
    if (tmpFile != NULL) {
	ngiFree(tmpFile, log, NULL);
	tmpFile = NULL;
    }

    /* Deallocate prefix if allocated */
    if (prefix_allocated != 0) {
	assert(prefix != NULL);
	ngiFree(prefix, log, NULL);
	prefix = NULL;
        prefix_allocated = 0;
    }

    /* Failed */
    return NULL;
}

/**
 * Temporary file name: Destroy
 */
int
ngiTemporaryFileDestroy(char *tmpFile, ngLog_t *log, int *error)
{
    int result;
    int retResult = 1;
    static const char fName[] = "ngiTemporaryFileDestroy";

    /* Check the arguments */
    assert(tmpFile != NULL);

    /* Unlink the Temporary File */
    result = unlink(tmpFile);
    if (result < 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "unlink failed: %s.\n", strerror(errno)); 
	retResult = 0;
    }

    /* Deallocate */
    ngiFree(tmpFile, log, error);

    /* Success */
    return retResult;
}

/**
 * Connect Retry Information : Initialize.
 */
int
ngiConnectRetryInformationInitialize(
    ngiConnectRetryInformation_t *retryInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInformationInitialize";

    /* Check the arguments */
    if (retryInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "retryInfo is NULL.\n"); 
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInformationInitializeMember(retryInfo);

    /* Success */
    return 1;
}

/**
 * Connect Retry Information : Finalize.
 */
int
ngiConnectRetryInformationFinalize(
    ngiConnectRetryInformation_t *retryInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInformationFinalize";

    /* Check the arguments */
    if (retryInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "retryInfo is NULL.\n"); 
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInformationInitializeMember(retryInfo);

    /* Success */
    return 1;
}

static void
nglConnectRetryInformationInitializeMember(
    ngiConnectRetryInformation_t *retryInfo)
{
    /* Check the arguments */
    assert(retryInfo != NULL);

    retryInfo->ngcri_count = 0;
    retryInfo->ngcri_interval = 0;
    retryInfo->ngcri_increase = 0.0;
    retryInfo->ngcri_useRandom = 0;
}

/**
 * Connect Retry : Initialize.
 */
int
ngiConnectRetryInitialize(
    ngiConnectRetryStatus_t *retryStatus,
    ngiConnectRetryInformation_t *retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryInitialize";

    /* Check the arguments */
    if ((retryStatus == NULL) ||
        (retryInfo == NULL) ||
        (randomSeed == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Initialize the members */
    nglConnectRetryInitializeMember(retryStatus);

    /* Copy the Retry Information */
    retryStatus->ngcrs_retryInfo = *retryInfo;

    /* Set to initial state */
    retryStatus->ngcrs_retry = retryInfo->ngcri_count; /* full count */
    retryStatus->ngcrs_nextInterval = (double)retryInfo->ngcri_interval;

    /* Set the Random Number Seed */
    retryStatus->ngcrs_randomSeed = randomSeed;

    /* Success */
    return 1;
}

/**
 * Connect Retry : Finalize.
 */
int
ngiConnectRetryFinalize(
    ngiConnectRetryStatus_t *retryStatus,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConnectRetryFinalize";

    /* Check the arguments */
    if (retryStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Do not free Random Number Seed */

    /* Initialize the members */
    nglConnectRetryInitializeMember(retryStatus);

    /* Success */
    return 1;
}

static void
nglConnectRetryInitializeMember(
    ngiConnectRetryStatus_t *retryStatus)
{
    /* Check the arguments */
    assert(retryStatus != NULL);

    nglConnectRetryInformationInitializeMember(&retryStatus->ngcrs_retryInfo);

    retryStatus->ngcrs_retry = 0;
    retryStatus->ngcrs_nextInterval = 0.0;
    retryStatus->ngcrs_randomSeed = NULL;
}

/**
 * Connect Retry : Get next retry sleep time.
 */
int
ngiConnectRetryGetNextRetrySleepTime(
    ngiConnectRetryStatus_t *retryStatus,
    int *doRetry,
    struct timeval *sleepTime,
    ngLog_t *log,
    int *error)
{
    int result;
    long sleepSec, sleepUsec;
    double nextRetrySec, randomNo;
    static const char fName[] = "ngiConnectRetryGetNextRetrySleepTime";

    *doRetry = 0;

    /* Check the arguments */
    if ((retryStatus == NULL) || (doRetry == NULL) || (sleepTime == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    if (retryStatus->ngcrs_retry > 0) {
        nextRetrySec = retryStatus->ngcrs_nextInterval;
        if (retryStatus->ngcrs_retryInfo.ngcri_useRandom == 1) {

            result = ngiRandomNumberGetDouble(
                retryStatus->ngcrs_randomSeed, &randomNo, log, error);
            if (result == 0) {
                ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Random number generation failed.\n"); 
                return 0;
            }

            nextRetrySec = retryStatus->ngcrs_nextInterval * randomNo;

            /* log */
            ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                "got the random number %g from 0 to %g.\n", nextRetrySec, retryStatus->ngcrs_nextInterval); 
        }

        /* log */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "last %d retry will performed after %g second sleep.\n", retryStatus->ngcrs_retry, nextRetrySec); 

        *doRetry = 1;
        retryStatus->ngcrs_retry--;

        sleepSec = (long)nextRetrySec;
        nextRetrySec -= (double)sleepSec;
        sleepUsec = (long)((double)nextRetrySec * 1000 * 1000);
        nextRetrySec = 0;

        sleepTime->tv_sec = sleepSec; 
        sleepTime->tv_usec = sleepUsec; 

        /* next interval */
        retryStatus->ngcrs_nextInterval *=
            retryStatus->ngcrs_retryInfo.ngcri_increase;

        if (retryStatus->ngcrs_nextInterval < 0) {
            ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Next retry time %g is invalid.\n", retryStatus->ngcrs_nextInterval); 
        }

    } else {
        /* log */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "retry count exhausted. do not retry.\n"); 

        *doRetry = 0;
    }

    /* Success */
    return 1;
}

/**
 * struct timeval operations : Add
 */
struct timeval
ngiTimevalAdd(
    struct timeval tvA,
    struct timeval tvB)
{
    struct timeval tmp;

    tmp.tv_usec = tvA.tv_usec + tvB.tv_usec;
    tmp.tv_sec = tvA.tv_sec + tvB.tv_sec + tmp.tv_usec / (1000 * 1000);
    tmp.tv_usec %= 1000 * 1000;

    return tmp;
}

/**
 * struct timeval operations : Subtract
 */
struct timeval
ngiTimevalSub(
    struct timeval tvA,
    struct timeval tvB)
{
    struct timeval tmp;
    long carry;

    if (tvA.tv_usec < tvB.tv_usec) {
        carry = 1 + (tvB.tv_usec / (1000 * 1000));
        tvA.tv_sec -= carry;
        tvA.tv_usec += carry * 1000 * 1000;
    }

    tmp.tv_sec = tvA.tv_sec - tvB.tv_sec;
    tmp.tv_usec = tvA.tv_usec - tvB.tv_usec;

    return tmp;
}

/**
 * struct timeval operations : Compare
 *   if (tvA <  tvB) return -1;
 *   if (tvA == tvB) return  0;
 *   if (tvA >  tvB) return +1;
 */
int
ngiTimevalCompare(
    struct timeval tvA,
    struct timeval tvB)
{
    if (tvA.tv_sec > tvB.tv_sec) {
        return 1;
    }

    if (tvA.tv_sec < tvB.tv_sec) {
        return -1;
    }

    assert (tvA.tv_sec == tvB.tv_sec);

    if (tvA.tv_usec > tvB.tv_usec) {
        return 1;
    }

    if (tvA.tv_usec < tvB.tv_usec) {
        return -1;
    }

    assert (tvA.tv_usec == tvB.tv_usec);

    return 0;
}

/**
 * struct timeval operations : gettimeofday()
 */
int
ngiGetTimeOfDay(
    struct timeval *tv,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiGetTimeOfDay";
    int result;

    /* Check the arguments */
    if (tv == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "Invalid argument. tv is NULL.\n");
        return 0;
    }

    /* Get finish time */
    result = gettimeofday(tv, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "%s failed by %d: %s.\n",
            "gettimeofday()", errno, strerror(errno)); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocates storage and writes it format string.
 */
char *
ngiStrdupPrintf(
    ngLog_t *log,
    int *error,
    const char *fmt,...)
{
    char *ret = NULL;
    va_list ap;
#if 0
    static const char fName[] = "ngiStrdupPrintf";
#endif

    va_start(ap, fmt);
    ret = ngiStrdupVprintf(fmt, ap, log, error);
    va_end(ap);

    return ret;
}

/**
 * Allocates storage and writes it format string.
 */
char *
ngiStrdupVprintf(
    const char *fmt,
    va_list ap,
    ngLog_t *log,
    int *error)
{
    char *buffer = NULL;
    char *p;
    int n;
    va_list aq;
    size_t size = 128;/* Default size is 128. */
    int result;
    static const char fName[] = "ngiStrdupVprintf";

    if (fmt == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Format string is NULL.\n");
        goto error;
    }

    buffer = ngiMalloc(size, log, error);
    if (buffer == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't allocate storage for buffer.\n");
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
        if ((n < 0) && (errno != 0)) {
            NGI_SET_ERROR(error,  NG_ERROR_SYSCALL);
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                "vsnprintf: %s.\n", strerror(errno));
            goto error;
        }
        
        /* In Tru64.
         * When string is longer than buffer, snprintf() returns size - 1.
         */
        if ((n >= 0) && (n < size - 1)) {
            break;
        } else {
            if ((n < 0) || (n == size - 1)) {
                size *= 2;
            } else {
                size = n + 1;
            }
            
            p = ngiRealloc(buffer, size, log, error);
            if (p == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
                    "Can't reallocate storage for string,\n");
                goto error;
            }
            buffer = p;
            
            ngLogDebug(log, fName, NG_LOGCAT_NINFG_LIB,
                "grows buffer to %lu\n", (unsigned long)size);
        }
    }
    
    return buffer;
    
error:
    result = ngiFree(buffer, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Can't free storage for string,\n");
    }
    buffer = NULL;
    
    return NULL;
}

/**
 * String to integer
 */
int
ngiStringToInt(
    const char *string,
    int *out,
    ngLog_t *log,
    int *error)
{
    char *endp;
    long l;
    static const char fName[] = "ngiStringToInt";

    if (out == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    errno = 0;
    l = strtol(string, &endp, 10);
    if (errno != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "strtol(): %s.\n", strerror(errno));
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        if (errno == ERANGE) {
            if (l == LONG_MIN) {
                NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
            } else if (l == LONG_MAX) { 
                NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            }
        }
        return 0;
    }
    if ((endp == string) || (*endp != '\0')) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "String contains the figure without.");
        return 0;
    }

    if (l > INT_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Value is too large.\n"); 
        return 0;
    }

    if (l < INT_MIN) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Value is too small.\n"); 
        return 0;
    }
    *out = l;

    return 1;
}

#define NGL_MODULUS     2147483647 /* 2^31 - 1 */
#define NGL_MULTIPLIER  48271

/**
 * Random Number.
 * Avoid libc function rand() or random() use,
 *  for to avoid user code influence.
 */
int
ngiRandomNumberInitialize(
    ngiRandomNumber_t *randomNumberStatus,
    ngLog_t *log,
    int *error)
{
    long seedNo;
    static const char fName[] = "ngiRandomNumberInitialize";

    /* Check the arguments */
    if (randomNumberStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Initialize the members */
    nglRandomNumberInitializeMember(randomNumberStatus);

    /* Initialize the seed */
    seedNo = ((long)getpid() + (long)time(NULL)) % NGL_MODULUS;

    *randomNumberStatus = seedNo;

    /* Success */
    return 1;
}

int
ngiRandomNumberFinalize(
    ngiRandomNumber_t *randomNumberStatus,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiRandomNumberFinalize";

    /* Check the arguments */
    if (randomNumberStatus == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Initialize the members */
    nglRandomNumberInitializeMember(randomNumberStatus);

    /* Success */
    return 1;
}

static void
nglRandomNumberInitializeMember(
    ngiRandomNumber_t *randomNumberStatus)
{
    /* Check the arguments */
    assert(randomNumberStatus != NULL);

    *randomNumberStatus = 0;
}


/**
 * Get the Random number by long int.
 */
int
ngiRandomNumberGetLong(
    ngiRandomNumber_t *randomNumberStatus,
    long *randomNo,
    ngLog_t *log,
    int *error)
{
    long seedNo;
    static const char fName[] = "ngiRandomNumberGetLong";

    /* Check the arguments */
    if ((randomNumberStatus == NULL) || (randomNo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    *randomNo = 0;

    seedNo = *randomNumberStatus;
    seedNo = NGL_MULTIPLIER 
        * (seedNo % (NGL_MODULUS / NGL_MULTIPLIER))
        - (NGL_MODULUS % NGL_MULTIPLIER) 
            * (seedNo / (NGL_MODULUS / NGL_MULTIPLIER));
    seedNo = ((seedNo > 0) ? seedNo : seedNo + NGL_MODULUS);
    
    *randomNumberStatus = seedNo;
    *randomNo = seedNo;

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Got the random number (long int)%ld.\n", *randomNo); 
    
    /* Success */
    return 1;
}

/**
 * Get the Random number. random from 0.0 to 1.0 on double.
 */
int
ngiRandomNumberGetDouble(
    ngiRandomNumber_t *randomNumberStatus,
    double *randomNo,
    ngLog_t *log,
    int *error)
{
    int result;
    long randomNoLong;
    static const char fName[] = "ngiRandomNumberGetDouble";

    /* Check the arguments */
    if ((randomNumberStatus == NULL) || (randomNo == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    *randomNo = 0.0;

    randomNoLong = 0;
    result = ngiRandomNumberGetLong(
        randomNumberStatus, &randomNoLong, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Getting random number failed.\n"); 
        return 0;
    }

    *randomNo = ((double)randomNoLong / NGL_MODULUS);

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Got the random number (double)%g.\n", *randomNo); 
    
    /* Success */
    return 1;
}

