#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngLog.c,v $ $Revision: 1.58 $ $Date: 2006/09/13 09:09:13 $";
#endif /* NG_OS_IRIX */
/* 
 * $AIST_Release: 4.2.4 $
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
 * Module of Log for Ninf-G.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef sun
#ifndef _REENTRANT
#define _REENTRANT
#endif /* _REENTRANT */
#define _POSIX_PTHREAD_SEMANTICS
#include <time.h>
#else /* sun */
#include <time.h>
#endif /* sun */
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static ngLog_t *nglLogAllocate(int *);
static int nglLogFree(ngLog_t *, int *);
static int nglLogInitialize(
    ngLog_t *, ngLogType_t, char *, ngLogInformation_t *, int, int *);
static void nglLogInitializeMember(ngLog_t *);
static void nglLogInitializeFlag(ngLog_t *);
static void nglLogInitializePointer(ngLog_t *);
static int nglLogFinalize(ngLog_t *, int *);
static int nglLogInitializeFileFormat(ngLog_t *, int, int *);
static int nglLogCurrentTimeStringGenerate(
    char *, int, int *, ngLog_t *, int *);
static int nglLogCreateLogDirectory(char *, int, int *);
static int nglLogIsDirectoryAvailable(char *, int, int *, int *);
static int nglLogNewFile(ngLog_t *, int *);
static int nglLogCreateFile(ngLog_t *, int *);
static int nglLogCloseFile(ngLog_t *, int *);
static int nglLogCreateFileName(ngLog_t *, int *);
static void nglLogInformationInitializeMember(ngLogInformation_t *);
static void nglLogInformationInitializePointer(ngLogInformation_t *);
static int nglLogInformationGetDefaultLoglevel(
    ngLogType_t, ngLogLevel_t *, int *);
static int nglLogDoesPrint(ngLog_t *, ngLogCategory_t, ngLogLevel_t);
static char *nglLogGetLevel(ngLogLevel_t);

static char *nglLogGetStringOfDateAndTime(ngLog_t *, int *);
static int nglLogReleaseStringOfDateAndTime(char *, int *);

static int nglCommLogDump(ngLog_t *, u_char *, size_t, int *);

/**
 * Construct
 */
ngLog_t *
ngiLogConstruct(
    ngLogType_t logType,
    char *message,
    ngLogInformation_t *logInfo,
    int executableID,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngiLogConstruct";

    /* Check the arguments */
    assert((logType == NG_LOG_TYPE_GENERIC) ||
           (logType == NG_LOG_TYPE_COMMUNICATION));

    /* Is communication log disable? */
    if ((logType == NG_LOG_TYPE_COMMUNICATION) &&
        (logInfo->ngli_enable == 0)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING,
            NULL, "%s: Communication log was disabled.\n", fName);
        return NULL;
    }

    /* Allocate */
    log = nglLogAllocate(error);
    if (log == NULL) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Log.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = nglLogInitialize(
        log, logType, message, logInfo, executableID, error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Log.\n", fName);
	goto error;
    }

    /* Success */
    return log;

    /* Error occurred */
error:
    /* Deallocate */
    result = nglLogFree(log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Log.\n", fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Destruct
 */
int
ngiLogDestruct(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiLogDestruct";

    /* Finalize */
    result = nglLogFinalize(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Log.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = nglLogFree(log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the Log.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
static ngLog_t *
nglLogAllocate(int *error)
{
    ngLog_t *log;
    static const char fName[] = "nglLogAllocate";

    /* Allocate */
    log = globus_libc_calloc(1, sizeof (ngLog_t));
    if (log == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	fprintf(stderr, "%s: Can't allocate the storage for Log.\n", fName);
	return NULL;
    }

    /* Success */
    return log;
}

/**
 * Free
 */
static int
nglLogFree(ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(log != NULL);

    /* Deallocate */
    globus_libc_free(log);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
static int
nglLogInitialize(
    ngLog_t *log,
    ngLogType_t logType,
    char *message,
    ngLogInformation_t *logInfo,
    int executableID,
    int *error)
{
    int result;
    char hostName[NGI_HOST_NAME_MAX];
    static const char fName[] = "nglLogInitialize";

    /* Check the arguments */
    assert((logType == NG_LOG_TYPE_GENERIC) ||
           (logType == NG_LOG_TYPE_COMMUNICATION));

    /* Initialize the members */
    nglLogInitializeMember(log);

    /* Save the log type */
    log->ngl_logType = logType;

    /* Initialize the Log output status */
    log->ngl_currentNo = 0;
    log->ngl_outputNbytes = 0;

    /* Initialize the Mutex */
    result = ngiMutexInitialize(&log->ngl_mutex, NULL, error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Mutex.\n", fName);
	return 0;
    }
    log->ngl_mutexInitialized = 1;

    /* Copy the Log Information */
    result = ngLogInformationCopy(logInfo, &log->ngl_info, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't copy the Log Information.\n", fName);
        return 0;
    }

    /* Is Log rotation size default? */
    if (((logInfo->ngli_nFiles == 0) || (logInfo->ngli_nFiles > 1)) &&
        (logInfo->ngli_maxFileSize == 0)) {
        /* Set default rotation size */
        log->ngl_info.ngli_maxFileSize = NG_LOG_ROTATE_MAX_FILE_SIZE;
    }

    /* Copy the message */
    if (message == NULL) {
	log->ngl_message = strdup("");
    } else {
	log->ngl_message = strdup(message);
    }
    if (log->ngl_message == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't duplicate the message.\n", fName);
	return 0;
    }

    /* Get the host name */
    result = gethostname(hostName, sizeof (hostName));
    if (result < 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the host name.\n", fName);
	return 0;
    }

    /* Copy the host name */
    log->ngl_hostName = strdup(hostName);
    if (log->ngl_hostName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't duplicate the host name.\n", fName);
	return 0;
    }

    /* Get the pid */
    log->ngl_pid = getpid();

    /* Is file path not specified? */
    if (logInfo->ngli_filePath == NULL) {
	/* Use stderr */
	log->ngl_stream = stderr;

	/* Success */
	return 1;
    }

    /* Initialize the Log File */
    result = nglLogInitializeFileFormat(log, executableID, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize Log File.\n", fName);
	return 0;
    }

    /* Create the Log File */
    result = nglLogCreateFile(log, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't create the Log File.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglLogInitializeMember(ngLog_t *log)
{
    /* Check the arguments */
    assert(log != NULL);

    /* Initialize the Log Information */
    nglLogInformationInitializeMember(&log->ngl_info);

    /* Initialize the flags and pointers */
    nglLogInitializeFlag(log);
    nglLogInitializePointer(log);

    /* Initialize the members */
    log->ngl_logType = NG_LOG_TYPE_GENERIC;
    log->ngl_currentNo = 0;
    log->ngl_fileNameBufferSize = 0;
    log->ngl_executableID = 0;
    log->ngl_nDigits = 0;
    log->ngl_pid = 0;
    log->ngl_outputNbytes = 0;
}

/**
 * Initialize the flags.
 */
static void
nglLogInitializeFlag(ngLog_t *log)
{
    /* Check the arguments */
    assert(log != NULL);

    /* Initialize the flags */
    log->ngl_mutexInitialized = 0;
    log->ngl_requireExecutableID = 0;
    log->ngl_requireFileNumber = 0;
    log->ngl_startTimeInitialized = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglLogInitializePointer(ngLog_t *log)
{
    /* Check the arguments */
    assert(log != NULL);

    /* Initialize the pointers */
    log->ngl_stream = NULL;
    log->ngl_fileFormat = NULL;
    log->ngl_fileName = NULL;
    log->ngl_hostName = NULL;
    log->ngl_message = NULL;
}

/**
 * Finalize
 */
static int
nglLogFinalize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "nglLogFinalize";

    /* Close the log file */
    if (log->ngl_stream != stderr) {
	result = nglLogCloseFile(log, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't close the Log File.\n", fName);
	    return 0;
	}
    }

    /* Deallocate the file format */
    if (log->ngl_fileFormat != NULL)
	globus_libc_free(log->ngl_fileFormat);
    log->ngl_fileFormat = NULL;

    /* Deallocate the file name */
    if (log->ngl_fileName != NULL)
	globus_libc_free(log->ngl_fileName);
    log->ngl_fileName = NULL;

    /* Deallocate the host name */
    if (log->ngl_hostName != NULL)
	globus_libc_free(log->ngl_hostName);
    log->ngl_hostName = NULL;

    /* Deallocate the message */
    if (log->ngl_message != NULL)
	globus_libc_free(log->ngl_message);
    log->ngl_message = NULL;

    /* Release the Log Information */
    result = ngLogInformationRelease(&log->ngl_info, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Log Information.\n", fName);
	return 0;
    }

    /* Finalize the Mutex */
    if (log->ngl_mutexInitialized != 0) {
	result = ngiMutexDestroy(&log->ngl_mutex, NULL, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't finalize the Mutex.\n", fName);
	    return 0;
	}
    }
    log->ngl_mutexInitialized = 0;

    /* Initialize the members */
    nglLogInitializeMember(log);

    return 1;
}

/**
 * Initialize the Log File.
 */
static int
nglLogInitializeFileFormat(
    ngLog_t *log,
    int executableID,
    int *error)
{
    int result;
    char *src, *dst;
    ngLogInformation_t *logInfo;
    int prevPercent, requireSeparator;
    char fileFormat[NGI_FILE_NAME_MAX];
    int srcCur, dstSize, dstCur, length;
    static const char fName[] = "nglLogInitializeFileFormat";

    /* Check the arguments */
    assert(log != NULL);

    dstCur = 0;
    dstSize = 0;

    logInfo = &log->ngl_info;

    /* Is number of bytes of File Path greater equal than maximum? */
    length = strlen(logInfo->ngli_filePath);
    if (length >= (sizeof (fileFormat) - NGI_LOG_FILE_NAME_MARGIN)) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Log File Path length %d greater equal than maximum %d.\n",
	    fName, length,
	    sizeof (fileFormat) - NGI_LOG_FILE_NAME_MARGIN);
	return 0;
    }

    /* Set the format flags : file number */
    log->ngl_requireFileNumber = 0;
    if ((logInfo->ngli_nFiles == 0) || (logInfo->ngli_nFiles > 1)) {
        log->ngl_requireFileNumber = 1;
    }

    /* Set the format flags : executable ID */
    if (executableID == NGI_LOG_EXECUTABLE_ID_NOT_APPEND) {
        log->ngl_requireExecutableID = 0;

    } else if (executableID == NGI_LOG_EXECUTABLE_ID_UNDEFINED) {
        log->ngl_requireExecutableID = 1;
        log->ngl_executableID = NGI_LOG_EXECUTABLE_ID_UNDEFINED;

    } else {
        log->ngl_requireExecutableID = 1;
        log->ngl_executableID = executableID;
    }

    /* Get the digits of file number */
    log->ngl_nDigits =
        (logInfo->ngli_nFiles > 1000000000) ? 10 :
        (logInfo->ngli_nFiles >  100000000) ?  9 :
        (logInfo->ngli_nFiles >   10000000) ?  8 :
        (logInfo->ngli_nFiles >    1000000) ?  7 :
        (logInfo->ngli_nFiles >     100000) ?  6 :
        (logInfo->ngli_nFiles >      10000) ?  5 :
        (logInfo->ngli_nFiles >       1000) ?  4 :
        (logInfo->ngli_nFiles >        100) ?  3 :
        (logInfo->ngli_nFiles >         10) ?  2 : 1;

    /* Zero clear the fileFormat */
    memset(fileFormat, 0, sizeof(fileFormat));

    /* Decode and Generate fileFormat from filePath */
    src = logInfo->ngli_filePath;
    srcCur = 0;

    dst = fileFormat;
    dstCur = 0;
    dstSize = sizeof(fileFormat);

    prevPercent = 0;
    while (src[srcCur] != '\0') {
        if (prevPercent == 1) {
            switch (src[srcCur]) {
            case 't' : /* "%t" time */
                result = nglLogCurrentTimeStringGenerate(
                    &dst[dstCur], dstSize - dstCur, &length, NULL, error);
                if (result == 0) {
                    ngLogPrintf(NULL,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Making time string failed.\n",
                        fName);
                    return 0;
                }

                dstCur += length;
                break;

            case 'h' : /* "%h" hostname */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%s", log->ngl_hostName);
                break;

            case 'p' : /* "%p" process id */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%ld", (long)log->ngl_pid);
                break;

            case '%' : /* "%%" just % */
                /* "output "%%" for to give "%" to filename */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%%%%");
                break;

            default:
                /* "output given "%?" to filename */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%%%%%c", src[srcCur]);
                break;
            }

            if (dstCur >= dstSize) {
                break;
            }

            prevPercent = 0;
            srcCur++;

        } else if (src[srcCur] == '%') {
            prevPercent = 1;
            srcCur++;

        } else {
            dst[dstCur] = src[srcCur];
            dstCur++;
            srcCur++;
            if (dstCur >= dstSize) {
                break;
            }
        }
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* output final "%" */
    if (prevPercent != 0) {
        dstCur += snprintf(&dst[dstCur], dstSize - dstCur, "%%%%");
        prevPercent = 0;
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* Output executable ID specifier */
    if (log->ngl_requireExecutableID != 0) {
        requireSeparator = 0;
        if ((log->ngl_requireFileNumber != 0) &&
            (logInfo->ngli_suffix != NULL)) {
            requireSeparator = 1;
        }

        /* executable ID may number or host-pid */
        dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
            "-execID-%%s%s", ((requireSeparator != 0) ? "-" : ""));
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* File number */
    if (log->ngl_requireFileNumber != 0) {
        dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
            "%s%%0%dd", (logInfo->ngli_suffix == NULL ? "." : ""),
            log->ngl_nDigits);
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* Suffix */
    if (logInfo->ngli_suffix != NULL) {
        dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
            ".%s", logInfo->ngli_suffix);
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* Terminate fileFormat string */
    dst[dstCur] = '\0';

    /* Copy the fileFormat */
    log->ngl_fileFormat = strdup(fileFormat);
    if (log->ngl_fileFormat == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Log File Path.\n", fName);
        return 0;
    }

    /* Get the file name buffer size */
    log->ngl_fileNameBufferSize =
        strlen(fileFormat)
        + strlen(NGI_LOG_EXECUTABLE_ID_UNDEF_FORMAT)
        + strlen(log->ngl_hostName) + NGI_INT_MAX_DECIMAL_DIGITS * 2 /* pid */
        + strlen(NGI_LOG_EXECUTABLE_ID_DEFINED_FORMAT)
        + NGI_INT_MAX_DECIMAL_DIGITS /* executable ID */
        + NGI_INT_MAX_DECIMAL_DIGITS /* unlimited nFiles */
        + 1;

    /* Allocate the storage for Log File Name */
    log->ngl_fileName = globus_libc_calloc(1, log->ngl_fileNameBufferSize);
    if (log->ngl_fileName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Log File Name.\n", fName);
        return 0;
    }

    /* Create the log file directory */
    result = nglLogCreateLogDirectory(
        fileFormat, logInfo->ngli_overWriteDir, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the log directory from format \"%s\".\n",
            fName, fileFormat);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
overflow:
    /* Overflow */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogPrintf(NULL,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Log File Format length %d greater equal than maximum %d.\n",
        fName, dstCur, dstSize);

    /* Failed */
    return 0;
}

/**
 * Generate the current time string.
 */
static int
nglLogCurrentTimeStringGenerate(
    char *buf,
    int size,
    int *writeSize,
    ngLog_t *log,
    int *error)
{
    int result;
    struct tm tm;
    struct timeval tv;
    static const char fName[] = "nglLogCurrentTimeStringGenerate";

    /* Check the arguments */
    assert(writeSize != NULL);

    /* Get the current time */
    result = gettimeofday(&tv, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: gettimeofday failed: %s.\n", fName, strerror(errno));
        return 0;
    }

    /* Make the time */
    gmtime_r(&tv.tv_sec, &tm);

    *writeSize = snprintf(buf, size,
        "%02d%02d%02d-%02d%02d%02d-%04ld",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        tv.tv_usec / 1000);

    /* Success */
    return 1;
}

/**
 * Log Information: Initialize
 */
int
ngLogInformationInitialize(ngLogInformation_t *logInfo)
{
    /* Check the arguments */
    assert(logInfo != NULL);

    /* Initialize the pointers */
    nglLogInformationInitializeMember(logInfo);

    /* Success */
    return 1;
}

/**
 * Log Information: Finalize
 */
int
ngLogInformationFinalize(ngLogInformation_t *logInfo)
{
    int result;
    static const char fName[] = "ngLogInformationFinalize";

    /* Check the arguments */
    assert(logInfo != NULL);

    /* Release the Log Information */
    result = ngLogInformationRelease(logInfo, NULL);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Log Information.\n", fName);
	return 0;
    }

    /* Initialize the pointers */
    nglLogInformationInitializeMember(logInfo);

    /* Success */
    return 1;
}

/**
 * Log Information: Initialize the members.
 */
static void
nglLogInformationInitializeMember(ngLogInformation_t *logInfo)
{
    /* Check the arguments */
    assert(logInfo != NULL);

    /* Initialize the pointers */
    nglLogInformationInitializePointer(logInfo);

    /* Initialize the members */
    logInfo->ngli_enable = 0;
    logInfo->ngli_level = NG_LOG_LEVEL_OFF;
    logInfo->ngli_levelGlobusToolkit = NG_LOG_LEVEL_OFF;
    logInfo->ngli_levelNinfgProtocol = NG_LOG_LEVEL_OFF;
    logInfo->ngli_levelNinfgInternal = NG_LOG_LEVEL_OFF;
    logInfo->ngli_levelGrpc = NG_LOG_LEVEL_OFF;
    logInfo->ngli_nFiles = 0;
    logInfo->ngli_maxFileSize = 0;
    logInfo->ngli_overWriteDir = 0;
}

/**
 * Log Information: Initialize the pointers.
 */
static void
nglLogInformationInitializePointer(ngLogInformation_t *logInfo)
{
    /* Check the arguments */
    assert(logInfo != NULL);

    /* Initialize the pointers */
    logInfo->ngli_filePath = NULL;
    logInfo->ngli_suffix = NULL;
}

/**
 * Log Information: Copy
 */
int
ngLogInformationCopy(
    ngLogInformation_t *src,
    ngLogInformation_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngLogInformationCopy";

    /* Check the arguments */
    assert(src != NULL);
    assert(dest != NULL);

    /* Copy the members */
    *dest = *src;

    /* Initialize the pointers */
    nglLogInformationInitializePointer(dest);

    /* Duplicate the File Path */
    if (src->ngli_filePath != NULL) {
	dest->ngli_filePath = strdup(src->ngli_filePath);
	if (dest->ngli_filePath == NULL) {
    	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL,
		"%s: Can't allocate the storage for File Path.\n", fName);
	    goto error;
	}
    }

    /* Duplicate the Suffix */
    if (src->ngli_suffix != NULL) {
	dest->ngli_suffix = strdup(src->ngli_suffix);
	if (dest->ngli_suffix == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Can't allocate the storage for Suffix.\n", fName);
	    goto error;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the Log Information */
    result = ngLogInformationRelease(dest, NULL);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Log Information.\n", fName);
	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Log Information: Release
 */
int
ngLogInformationRelease(ngLogInformation_t *logInfo, int *error)
{
    /* Check the arguments */
    assert(logInfo != NULL);

    /* Deallocate the File Path */
    if (logInfo->ngli_filePath != NULL) {
    	globus_libc_free(logInfo->ngli_filePath);
	logInfo->ngli_filePath = NULL;
    }

    /* Deallocate the Suffix */
    if (logInfo->ngli_suffix != NULL) {
    	globus_libc_free(logInfo->ngli_suffix);
	logInfo->ngli_suffix = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Log Information: Set Default
 */
int
ngiLogInformationSetDefault(
    ngLogInformation_t *logInfo,
    ngLogType_t logType,
    int *error)
{
    int result;
    ngLogLevel_t logLevel;
    static const char fName[] = "ngiLogInformationSetDefault";

    /* Check the arguments */
    assert(logInfo != NULL);

    logLevel = NG_LOG_LEVEL_OFF;

    /* Initialize */
    result = ngLogInformationInitialize(logInfo);
    if (result == 0) {
        /* Note: This situation will not happen. */
        fprintf(stderr, "%s: Can't initialize Log Information.\n", fName);
	return 0;
    }

    /* Get the default loglevel */
    result = nglLogInformationGetDefaultLoglevel(logType, &logLevel, error);
    if (result == 0) {
        /* Note: This situation will not happen. */
        fprintf(stderr, "%s: Can't get the default loglevel.\n", fName);
	return 0;
    }

    logInfo->ngli_enable = (logType == NG_LOG_TYPE_GENERIC ? 1 : 0);
    logInfo->ngli_level              = logLevel;
    logInfo->ngli_levelGlobusToolkit = logLevel;
    logInfo->ngli_levelNinfgProtocol = logLevel;
    logInfo->ngli_levelNinfgInternal = logLevel;
    logInfo->ngli_levelGrpc          = logLevel;
    logInfo->ngli_filePath           = NULL; /* for stderr */
    logInfo->ngli_suffix             = NULL;
    logInfo->ngli_nFiles             = 1;
    logInfo->ngli_maxFileSize        = 0; /* 1M/infinite */
    logInfo->ngli_overWriteDir       = 0; /* false */

    /* Success */
    return 1;
}

/**
 * Log Information: Get Default loglevel
 */
static int
nglLogInformationGetDefaultLoglevel(
    ngLogType_t logType,
    ngLogLevel_t *logLevel,
    int *error)
{
    int tmp, i;
    char *logLevelString, *end;
    char *logLevelTable[] = {
        "Off", "Fatal", "Error", "Warning", "Information", "Debug"};
    static const char fName[] = "nglLogInformationGetDefaultLoglevel";

    /* Check the arguments */
    assert(logLevel != NULL);

    *logLevel = NG_LOG_LEVEL_OFF;
    logLevelString = NULL;

    if (logType == NG_LOG_TYPE_COMMUNICATION) {
        *logLevel = NGI_LOG_DEFAULT_COMMUNICATION_LOGLEVEL;
        return 1;
    }

    assert(logType == NG_LOG_TYPE_GENERIC);

    /* Get default loglevel from environment variable */
    logLevelString = getenv(NGI_ENVIRONMENT_LOG_LEVEL);
    if (logLevelString == NULL) {
        /* No environment variable was set */
        *logLevel = NGI_LOG_DEFAULT_GENERIC_LOGLEVEL;
        return 1;
    }

    assert(logLevelString != NULL);

    /* Try decode by number */
    tmp = strtol(logLevelString, &end, 10);
    if ((tmp >= 0) && (tmp <= 5) && (*end == '\0')) {
        *logLevel = (ngLogLevel_t)tmp;
        return 1;
    }

    /* Try decode by string */
    for (i = 0; i <= 5; i++) {
        if (strcmp(logLevelString, logLevelTable[i]) == 0) {
            /* found */
            *logLevel = (ngLogLevel_t)i;
            return 1;
        }
    }

    *logLevel = NGI_LOG_DEFAULT_GENERIC_LOGLEVEL;

    /* Not found */
    fprintf(stderr,
        "%s: Invalid environment variable \"$%s=%s\" ignored.\n",
        fName, NGI_ENVIRONMENT_LOG_LEVEL, logLevelString);

    /* Success */
    return 1;
}

/**
 * Log File: Create Log directory
 */
static int
nglLogCreateLogDirectory(char *fileFormat, int overWrite, int *error)
{
    int result, dirAvailable, errorNumber;
    char *dir, *dirPart, *sep, *cur;
    static const char fName[] = "nglLogCreateLogDirectory";

    /* Check the arguments */
    assert(fileFormat != NULL);

    dir = NULL;
    dirPart = NULL;
    dirAvailable = 0;

    /* Is directory specified? */
    if (strchr(fileFormat, '/') == NULL) {

        /* Success */
        return 1;
    }

    /* Duplicate the dir name */
    dir = strdup(fileFormat);
    dirPart = strdup(fileFormat);

    if ((dir == NULL) || (dirPart == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for directory name.\n", fName);
        goto error;
    }

    sep = strrchr(dir, '/');
    if (sep == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't find the \"/\" from string \"%s\".\n", fName, dir);
        goto error;
    }

    /* Terminate the dir path */
    *sep = '\0';

    /* Check the dir available and writable */
    result = nglLogIsDirectoryAvailable(dir, 1, &dirAvailable, error);
    if (result == 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Checking directory \"%s\" failed.\n", fName, dir);
        goto error;
    }

    if (dirAvailable != 0) {
        if (overWrite == 0) {
            NGI_SET_ERROR(error, NG_ERROR_EXIST);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't overwriteDirectory \"%s\".\n",
                fName, dir);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: overwriteDirectory disabled"
                " by Ninf-G configuration file.\n",
                fName);
            goto error;
        }
        goto success;
    }

    /* Create the Log directory */
    ngLogPrintf(NULL,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Creating directory \"%s\".\n", fName, dir);

    cur = dir;

    /* Skip top directory */
    while (*cur == '/') {
        cur++;
    }

    do {
        /* Skip '/' */
        while (*cur == '/') {
            cur++;
        }
    
        cur = strchr(cur, '/');

        if (cur != NULL) {
            /* Copy a part of the dir */
            *cur = '\0';
            strcpy(dirPart, dir);
            *cur = '/';

        } else {
            /* Copy the dir */
            strcpy(dirPart, dir);
        }

        /* Check the dirPart available */
        dirAvailable = 0;
        result = nglLogIsDirectoryAvailable(
            dirPart, 0, &dirAvailable, error);
        if (result == 0) {
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Checking directory \"%s\" failed.\n", fName, dirPart);
            goto error;
        }

        if (dirAvailable != 0) {
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: The directory \"%s\" available.\n", fName, dirPart);
            continue;
        }

        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: mkdir(\"%s\").\n", fName, dirPart);

        /* Make the directory */
        result = mkdir(dirPart, S_IRWXU | S_IRWXG | S_IRWXO);
        if (result != 0) {
            errorNumber = errno;
            if ((errorNumber == EEXIST) && (overWrite != 0)) {
                ngLogPrintf(NULL,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
                    NULL, "%s: mkdir \"%s\" failed : %s, continue.\n",
                    fName, dirPart, strerror(errorNumber));
            } else {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogPrintf(NULL,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: mkdir \"%s\" failed : %s.\n",
                    fName, dirPart, strerror(errorNumber));
                goto error;
            }
        }

        /* Check the created directory writable */
        dirAvailable = 0;
        result = nglLogIsDirectoryAvailable(
            dirPart, 1, &dirAvailable, error);
        if (result == 0) {
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Checking created directory \"%s\" failed.\n",
                fName, dirPart);
            goto error;
        }

        if (dirAvailable == 0) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: The created directory \"%s\" not available.\n",
                fName, dirPart);
            goto error;
        }

    } while (cur != NULL);

success:

    if (dir != NULL) {
        free(dir);
        dir = NULL;
    }

    if (dirPart != NULL) {
        free(dirPart);
        dirPart = NULL;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate */
    if (dir != NULL) {
        free(dir);
        dir = NULL;
    }

    if (dirPart != NULL) {
        free(dirPart);
        dirPart = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * Log File: Check the directory available
 */
static int
nglLogIsDirectoryAvailable(
    char *dir,
    int requireWrite,
    int *isAvailable,
    int *error)
{
    struct stat stBuf;
    int result, errorNumber, accessFlag;
    static const char fName[] = "nglLogIsDirectoryAvailable";

    /* Check the arguments */
    assert(dir != NULL);
    assert(isAvailable != NULL);

    *isAvailable = 0;

    /* Check the dir available */
    result = stat(dir, &stBuf);
    if (result != 0) {
        if (errno == ENOENT) {
            *isAvailable = 0;
        } else {
            errorNumber = errno;
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: \"%s\": %s.\n",
                fName, dir, strerror(errorNumber));
            return 0;
        }
    } else {
        if (!(stBuf.st_mode & S_IFDIR)) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: \"%s\" is not the directory.\n", fName, dir);
            return 0;
        }
        *isAvailable = 1;
    }

    ngLogPrintf(NULL,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The directory \"%s\" is%s available by stat() check.\n",
        fName, dir, (*isAvailable == 0 ? " not" : ""));

    /* Check the permission */
    if (*isAvailable != 0) {
        accessFlag = ((requireWrite != 0) ? W_OK | X_OK : X_OK);
        result = access(dir, accessFlag);
        if (result != 0) {
            if (errno == ENOENT) {
                *isAvailable = 0;
            } else {
                errorNumber = errno;
                NGI_SET_ERROR(error, NG_ERROR_FILE);
                ngLogPrintf(NULL,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: \"%s\": %s.\n",
                    fName, dir, strerror(errorNumber));
                return 0;
            }
        } else {
            *isAvailable = 1;
        }
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: The directory \"%s\" is%s available by access() check.\n",
            fName, dir, (*isAvailable == 0 ? " not" : ""));
    }

    /* Success */
    return 1;
}

/**
 * Log File: New file
 */
static int
nglLogNewFile(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "nglLogNewFile";

    /* Is an output stream a file? */
    if (log->ngl_stream == stderr) {
        /* Success */
        return 1;
    }

    /* Is file size unlimited? */
    if (log->ngl_info.ngli_maxFileSize == 0) {
        /* (log->ngl_info.ngli_nFiles == 1) */

        /* Success */ 
        return 1;
    }

    /* Is overflow the log file? */
    if (log->ngl_outputNbytes < log->ngl_info.ngli_maxFileSize) {
        /* Success */
        return 1;
    }
    log->ngl_outputNbytes = 0;

    /* Log file was overflowed. Create a new file */
    result = nglLogCreateFile(log, error);
    if (result == 0) {
        log->ngl_stream = stderr;
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Log File.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Log File: Create
 */
static int
nglLogCreateFile(ngLog_t *log, int *error)
{
    int result;
    FILE *stream;
    static const char fName[] = "nglLogCreateFile";

    /* Check the arguments */
    assert(log != NULL);

    /* Close the current file */
    result = nglLogCloseFile(log, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't close the Log File.\n", fName);
	return 0;
    }

    /* Make the File Name */
    result = nglLogCreateFileName(log, error);
    if (result == 0) {
	ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
    	    "%s: Creating new file name failed.\n", fName);
	return 0;
    }

    /* Create the file */
    stream = fopen(log->ngl_fileName, "w");
    if (stream == NULL) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't create the Log File \"%s\".\n",
            fName, log->ngl_fileName);
        return 0;
    }
    log->ngl_stream = stream;

    /* Disable buffering */
    setvbuf(stream, NULL, _IONBF, 0);

    /* Success */
    return 1;
}

/**
 * Log File: Close
 */
static int
nglLogCloseFile(ngLog_t *log, int *error)
{
    int result;
    FILE *stream;
    static const char fName[] = "nglLogCloseFile";

    stream = log->ngl_stream;
    if (stream != NULL) {
	log->ngl_stream = NULL;
    	result = fclose(stream);
	if (result != 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't close the Log File.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}


/**
 * Log File: Create file name
 */
static int
nglLogCreateFileName(ngLog_t *log, int *error)
{
    int result, maxSize;
    char tmpBuf[NGI_FILE_NAME_MAX];
    static const char fName[] = "nglLogCreateFileName";

    /* Check the arguments */
    assert(log != NULL);

    /* Prepare Executable ID string  */
    if (log->ngl_requireExecutableID != 0) {
        if (log->ngl_executableID == NGI_LOG_EXECUTABLE_ID_UNDEFINED) {
            result = snprintf(tmpBuf, sizeof(tmpBuf),
                NGI_LOG_EXECUTABLE_ID_UNDEF_FORMAT,
                log->ngl_hostName, (long)log->ngl_pid);
        } else {
            result = snprintf(tmpBuf, sizeof(tmpBuf),
                NGI_LOG_EXECUTABLE_ID_DEFINED_FORMAT,
                log->ngl_executableID);
        }

        if (result >= sizeof(tmpBuf)) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: host and pid string is overflow the buffer.\n", fName);
            return 0;
        }
    }

    maxSize = log->ngl_fileNameBufferSize;

    /* Generate the file name */
    if (log->ngl_requireExecutableID != 0) {
        if (log->ngl_requireFileNumber != 0) {
            result = snprintf(log->ngl_fileName, maxSize,
                log->ngl_fileFormat, tmpBuf, log->ngl_currentNo);
        } else {
            result = snprintf(log->ngl_fileName, maxSize,
                log->ngl_fileFormat, tmpBuf);
        }
    } else {
        if (log->ngl_requireFileNumber != 0) {
            result = snprintf(log->ngl_fileName, maxSize,
                log->ngl_fileFormat, log->ngl_currentNo);
        } else {
            result = snprintf(log->ngl_fileName, maxSize,
                log->ngl_fileFormat);
        }
    }

    if (result >= maxSize) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: File Name is overflow the buffer.\n", fName);
        return 0;
    }

    /* Increment the File No. */
    log->ngl_currentNo++;
    if ((log->ngl_info.ngli_nFiles != 0) &&
        (log->ngl_currentNo >= log->ngl_info.ngli_nFiles)) {
    	log->ngl_currentNo = 0;
    }

    /* Success */
    return 1;
}

/**
 * Log File: The Executable ID was resolved. Rename filename.
 */
int
ngiLogExecutableIDchanged(
    ngLog_t *log,
    int newExecutableID,
    int *error)
{
    int result;
    char *oldName;
    static const char fName[] = "ngiLogExecutableIDchanged";

    oldName = NULL;

    if ((log == NULL) || (log->ngl_requireExecutableID == 0)) {
        /* Do nothing */

        /* Success */
        return 1;
    }

    /* Save the old file name */
    oldName = strdup(log->ngl_fileName);
    if (oldName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for old file name.\n", fName);
        goto error;
    }

    /* Set the new Executable ID */
    log->ngl_executableID = newExecutableID;

    /* Reset the file number counter */
    log->ngl_currentNo = 0;

    /* Make the File Name */
    result = nglLogCreateFileName(log, error);
    if (result == 0) {
	ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
    	    "%s: Creating new file name failed.\n", fName);
        goto error;
    }

    result = rename(oldName, log->ngl_fileName);
    if (result != 0) {
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
    	    "%s: Renaming log file \"%s\" to \"%s\" failed.\n",
            fName, oldName, log->ngl_fileName);
        /* Not fail */
    }

    if (oldName != NULL) {
        free(oldName);
        oldName = NULL;
    }
    
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate */
    if (oldName != NULL) {
        free(oldName);
        oldName = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * Printf
 */
int
ngLogPrintf(
    ngLog_t *log,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    ...)
{
    va_list ap;
    int result;
    static const char fName[] = "ngLogPrintf";

    va_start(ap, format);
    
    /* Print out */
    result = ngiLogVprintf(log, category, level, error, NULL, format, ap);
    if (result == 0) {
	fprintf(stderr, "%s: ngiLogVprintf failed.\n", fName);
	goto error;
    }

    va_end(ap);

    /* Success */
    return 1;

    /* Error occurred */
error:
    va_end(ap);

    return 0;
}

/**
 * Vprintf
 */
int
ngLogVprintf(
    ngLog_t *log,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    va_list ap)
{
    return ngiLogVprintf(log, category, level, error, NULL, format, ap);
}

/**
 * Vprintf
 */
int
ngiLogVprintf(
    ngLog_t *log,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *message,
    char *format,
    va_list ap)
{
    int result;
    int locked = 0;
    char *date = NULL;
    char *sLevel;
    ngLog_t logDefault;
    static const char fName[] = "ngiLogVprintf";

    /* Get the Execution time */
    if (log != NULL)  {
        if (log->ngl_startTimeInitialized == 0) {
            result = ngiSetStartTime(&log->ngl_execTime, NULL, error);
            if (result == 0) {
                fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
                goto error;
            }
        }
        log->ngl_startTimeInitialized = 1;
        result = ngiSetEndTime(&log->ngl_execTime, NULL, error);
        if (result == 0) {
            fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
            goto error;
        }
    
        /* Copy the current time to start time */
        log->ngl_execTime.nget_real.nget_start
            = log->ngl_execTime.nget_real.nget_end;
    }

    if (log == NULL) {
        /* Initialize Default Log Information */
        result = ngiLogInformationSetDefault(
            &logDefault.ngl_info, NG_LOG_TYPE_GENERIC, error);
        if (result == 0) {
            fprintf(stderr, "%s: Can't initialize Log Information.\n",
                fName);
            goto error;
        }

	/* Does print this message? */
	if (nglLogDoesPrint(&logDefault, category, level) == 0) {
	    /* This message does not print out */
	    goto success;
	}

        /* Get the date */
        date = nglLogGetStringOfDateAndTime(log, error);
        if (date == NULL) {
            fprintf(stderr, "%s: Can't get the string of date and time.\n",
                fName);
            goto error;
        }

	/* Get the level */
	sLevel = nglLogGetLevel(level);

    	/* Print out to stderr */
	fprintf(stderr, "%s: %s: ", date, sLevel);
#if defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG)
	if (nglLogDoesPrint(&logDefault, category, NG_LOG_LEVEL_DEBUG)) {
	    /* Print globus_thread_t.                       *
	     * this depend implementation of thread library.*/
	    fprintf(stderr, "Thread %lu: ",
	            (unsigned long)globus_thread_self());
	}
#endif /* defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG) */
	if (message != NULL)
	    fprintf(stderr, message);
	vfprintf(stderr, format, ap);
    } else {
        /* Check the arguments */
        assert(log->ngl_logType == NG_LOG_TYPE_GENERIC);

	/* Does print this message? */
	if (nglLogDoesPrint(log, category, level) == 0) {
	    /* This message does not print out */
	    goto success;
	}

        /* Get the date */
        date = nglLogGetStringOfDateAndTime(log, error);
        if (date == NULL) {
            fprintf(stderr, "%s: Can't get the string of date and time.\n",
                fName);
            goto error;
        }

	/* Get the level */
	sLevel = nglLogGetLevel(level);

	/* Lock the Log Manager */
	result = ngiMutexLock(&log->ngl_mutex, NULL, error);
	if (result == 0) {
	    fprintf(stderr, "%s: Can't lock the mutex.\n", fName);
	    goto error;
	}
        locked = 1;

    	/* Print out to log file */
	log->ngl_outputNbytes += fprintf(log->ngl_stream, "%s: %s: %s: %s: ",
	    date, log->ngl_message, log->ngl_hostName, sLevel);
#if defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG)
	if (nglLogDoesPrint(log, category, NG_LOG_LEVEL_DEBUG)) {
	    /* Print globus_thread_t.                       *
	     * this depend implementation of thread library.*/
	    log->ngl_outputNbytes +=
	        fprintf(log->ngl_stream, "Thread %lu: ",
	                (unsigned long)globus_thread_self());
	}
#endif /* defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG) */
	if (message != NULL)
	    log->ngl_outputNbytes += fprintf(log->ngl_stream, message);
	log->ngl_outputNbytes += vfprintf(log->ngl_stream, format, ap);

        /* It will change to a new file, when log is over */
        result = nglLogNewFile(log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't create the new log file.\n", fName);
            goto error;
        }

	/* Unlock the Log Manager */
        locked = 0;
	result = ngiMutexUnlock(&log->ngl_mutex, NULL, error);
	if (result == 0) {
	    fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
	    goto error;
	}
    }
    /* Success */
success:
    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, error);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, NULL);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
        }
    }

    /* Unlock the Log Manager */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&log->ngl_mutex, NULL, NULL);
        if (result == 0) {
            fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Does print out this message?
 */
static int
nglLogDoesPrint(ngLog_t *log, ngLogCategory_t category, ngLogLevel_t level)
{
    ngLogLevel_t tmpLevel;
    static const char fName[] = "nglLogDoesPrint";

    /* Check the arguments */
    assert(log != NULL);

    switch (category) {
    case NG_LOG_CATEGORY_GLOBUS_TOOLKIT:
	tmpLevel = log->ngl_info.ngli_levelGlobusToolkit;
	break;

    case NG_LOG_CATEGORY_NINFG_PROTOCOL:
	tmpLevel = log->ngl_info.ngli_levelNinfgProtocol;
	break;

    case NG_LOG_CATEGORY_NINFG_PURE:
	tmpLevel = log->ngl_info.ngli_levelNinfgInternal;
	break;

    case NG_LOG_CATEGORY_NINFG_GRPC:
	tmpLevel = log->ngl_info.ngli_levelGrpc;
	break;

    default:
	fprintf(stderr, "%s: Unknown category %d.\n", fName, category);
	tmpLevel = log->ngl_info.ngli_level;
	break;
    }

    return level <= tmpLevel;
}

/**
 * Get the level of log.
 */
static char *
nglLogGetLevel(ngLogLevel_t level)
{
    char *sLevel;
    static const char fName[] = "nglLogGetLevel";

    switch (level) {
    case NG_LOG_LEVEL_FATAL:
	sLevel = "Fatal";
	break;

    case NG_LOG_LEVEL_ERROR:
	sLevel = "Error";
	break;

    case NG_LOG_LEVEL_WARNING:
	sLevel = "Warning";
	break;

    case NG_LOG_LEVEL_INFORMATION:
	sLevel = "Information";
	break;

    case NG_LOG_LEVEL_DEBUG:
	sLevel = "Debug";
	break;

    default:
	fprintf(stderr, "%s: Unknown level %d.\n", fName, level);
	sLevel = "Unknown";
	break;
    }

    return sLevel;
}

/**
 * Print the communication log of send data.
 */
int
ngiCommLogSendIovec(
    ngLog_t *commLog,
    struct iovec *iov,
    size_t nIovs,
    int *error)
{
    int result;
    int i;
    size_t nBytes;
    int locked = 0;
    char *date = NULL;
    static const char fName[] = "ngiCommLogSendIovec";

    /* Is log disable? */
    if (commLog == NULL) {
        /* Success */
        return 1;
    }

    /* Check the arguments */
    assert(commLog->ngl_logType == NG_LOG_TYPE_COMMUNICATION);

    /* Calculate the number of bytes */
    nBytes = 0;
    for (i = 0; i < nIovs; i++)
	nBytes += iov[i].iov_len;

    /* Get the Execution time */
    if (commLog != NULL) {
        if (commLog->ngl_startTimeInitialized == 0) {
            result = ngiSetStartTime(&commLog->ngl_execTime, NULL, error);
            if (result == 0) {
                fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
                goto error;
            }
        }
        commLog->ngl_startTimeInitialized = 1;
        result = ngiSetEndTime(&commLog->ngl_execTime, NULL, error);
        if (result == 0) {
            fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
            goto error;
        }
    
        /* Copy the current time to start time */
        commLog->ngl_execTime.nget_real.nget_start
            = commLog->ngl_execTime.nget_real.nget_end;
    }

    /* Get the date */
    date = nglLogGetStringOfDateAndTime(commLog, error);
    if (date == NULL) {
        fprintf(stderr, "%s: Can't get the string of date and time.\n", fName);
        goto error;
    }

    /* Lock the Log Manager */
    result = ngiMutexLock(&commLog->ngl_mutex, NULL, error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't lock the mutex.\n", fName);
        goto error;
    }
    locked = 1;

    /* Print out the Communication log */
    commLog->ngl_outputNbytes += fprintf(
        commLog->ngl_stream, "%s  %s  (%3lds %3ldms %3ldus)  Send: %lubytes\n",
        date, commLog->ngl_message,
        commLog->ngl_execTime.nget_real.nget_execution.tv_sec,
	commLog->ngl_execTime.nget_real.nget_execution.tv_usec / 1000,
        commLog->ngl_execTime.nget_real.nget_execution.tv_usec % 1000,
        (unsigned long)nBytes);

    /* Print the communication data */
    for (i = 0; i < nIovs; i++) {
        result = nglCommLogDump(
            commLog, (u_char *)iov[i].iov_base, iov[i].iov_len, error);
        if (result < 0) {
            fprintf(stderr, "%s: Can't print the communication log.\n", fName);
            goto error;
        }
        commLog->ngl_outputNbytes += result;
    }

    /* It will change to a new file, when log is over */
    result = nglLogNewFile(commLog, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create the new log file.\n", fName);
        goto error;
    }

    /* Unlock the Log Manager */
    locked = 0;
    result = ngiMutexUnlock(&commLog->ngl_mutex, NULL, error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
        goto error;
    }

    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, error);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, NULL);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
        }
    }

    /* Unlock the Log Manager */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&commLog->ngl_mutex, NULL, NULL);
        if (result == 0) {
            fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
        }
    }

    /* Failed */
    return 0;
}
/**
 * Print the communication log of receive data.
 */
int
ngiCommLogReceive(
    ngLog_t *commLog,
    u_char *buf,
    size_t nBytes,
    int *error)
{
    int result;
    int locked = 0;
    char *date = NULL;
    static const char fName[] = "ngiCommLogReceive";

    /* Is log disable? */
    if (commLog == NULL) {
        /* Success */
        return 1;
    }

    /* Check the arguments */
    assert(commLog->ngl_logType == NG_LOG_TYPE_COMMUNICATION);

    /* Get the Execution time */
    if (commLog != NULL) {
        if (commLog->ngl_startTimeInitialized == 0) {
            result = ngiSetStartTime(&commLog->ngl_execTime, NULL, error);
            if (result == 0) {
                fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
                goto error;
            }
        }
        commLog->ngl_startTimeInitialized = 1;
        result = ngiSetEndTime(&commLog->ngl_execTime, NULL, error);
        if (result == 0) {
            fprintf(stderr, "%s: Can't get the Execution time.\n", fName);
            goto error;
        }
    
        /* Copy the current time to start time */
        commLog->ngl_execTime.nget_real.nget_start
            = commLog->ngl_execTime.nget_real.nget_end;
    }

    /* Get the date */
    date = nglLogGetStringOfDateAndTime(commLog, error);
    if (date == NULL) {
        fprintf(stderr, "%s: Can't get the string of date and time.\n", fName);
        goto error;
    }

    /* Lock the Log Manager */
    result = ngiMutexLock(&commLog->ngl_mutex, NULL, error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't lock the mutex.\n", fName);
        goto error;
    }
    locked = 1;

    /* Print out the Communication log */
    commLog->ngl_outputNbytes += fprintf(
        commLog->ngl_stream, "%s  %s  (%3lds %3ldms %3ldus)  Receive: %lubytes\n",
        date, commLog->ngl_message,
        commLog->ngl_execTime.nget_real.nget_execution.tv_sec,
        commLog->ngl_execTime.nget_real.nget_execution.tv_usec / 1000,
        commLog->ngl_execTime.nget_real.nget_execution.tv_usec % 1000,
        (unsigned long)nBytes);

    /* Print the communication data */
    result = nglCommLogDump(commLog, buf, nBytes, error);
    if (result < 0) {
        fprintf(stderr, "%s: Can't print the communication log.\n", fName);
        goto error;
    }
    commLog->ngl_outputNbytes += result;

    /* It will change to a new file, when log is over */
    result = nglLogNewFile(commLog, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't create the new log file.\n", fName);
        goto error;
    }

    /* Unlock the Log Manager */
    locked = 0;
    result = ngiMutexUnlock(&commLog->ngl_mutex, NULL, error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
        goto error;
    }

    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, error);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the date */
    if (date != NULL) {
        result = nglLogReleaseStringOfDateAndTime(date, NULL);
        date = NULL;
        if (result == 0) {
            fprintf(stderr,
                "%s: Can't release the string of date and time.\n", fName);
        }
    }

    /* Unlock the Log Manager */
    if (locked != 0) {
        locked = 0;
        result = ngiMutexUnlock(&commLog->ngl_mutex, NULL, NULL);
        if (result == 0) {
            fprintf(stderr, "%s: Can't unlock the mutex.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the string date and time.
 */
static char *
nglLogGetStringOfDateAndTime(ngLog_t *log, int *error)
{
    int result;
    struct timeval tv;
    long msec, usec;
    char *date, *tmp;
    char sDate[128];
    static const char fName[] = "nglLogGetStringOfDateAndTime";

    /* Clear the string */
    memset(&sDate[0], '\0', sizeof (sDate));

    /* Get current time */
    if (log != NULL) {
        tv = log->ngl_execTime.nget_real.nget_end;
    } else {
        result = gettimeofday(&tv, NULL);
        if (result != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            fprintf(stderr, "%s: gettimeofday failed: %s.\n",
                fName, strerror(errno));
            return 0;
        }
    }
    msec = tv.tv_usec / 1000;
    usec = tv.tv_usec % 1000;

    /* Note:  POSIX ctime_r() function does not take a buflen parameter. */
    (void)ctime_r(&tv.tv_sec, &sDate[0]);

    /* Delete the '\n' */
    tmp = strchr(sDate, '\n');
    if (tmp != NULL) {
	tmp[0] = '\0';
    }

    /* Append the msec */
    sprintf(&sDate[strlen(sDate)], " %3ldms %3ldus", msec, usec);

    /* Allocate the storage for string */
    date = globus_libc_calloc(1, strlen(sDate) + 1);
    if (date == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        fprintf(stderr, "%s: Can't allocate the storage for string.\n", fName);
        return NULL;
    }

    /* Copy the string */
    strcpy(date, sDate);

    /* Success */
    return date;
}

/**
 * Release the string of date and time.
 */
static int
nglLogReleaseStringOfDateAndTime(char *date, int *error)
{
    /* Check the arguments */
    assert(date != NULL);

    /* Free */
    globus_libc_free(date);

    /* Success */
    return 1;
}

/**
 * Dump the communication data.
 */
static int
nglCommLogDump(
    ngLog_t *commLog,
    u_char *buf,
    size_t nBytes,
    int *error)
{
    int i, curr;
    int retNbytes = 0;
    int remain, space;
    char c[16];

    for (i = 0; (i < nBytes) && ((nBytes - i) >= 16); i += 16) {
        c[ 0] = isprint(buf[i+ 0]) ? buf[i+ 0] : '.';
        c[ 1] = isprint(buf[i+ 1]) ? buf[i+ 1] : '.';
        c[ 2] = isprint(buf[i+ 2]) ? buf[i+ 2] : '.';
        c[ 3] = isprint(buf[i+ 3]) ? buf[i+ 3] : '.';
        c[ 4] = isprint(buf[i+ 4]) ? buf[i+ 4] : '.';
        c[ 5] = isprint(buf[i+ 5]) ? buf[i+ 5] : '.';
        c[ 6] = isprint(buf[i+ 6]) ? buf[i+ 6] : '.';
        c[ 7] = isprint(buf[i+ 7]) ? buf[i+ 7] : '.';
        c[ 8] = isprint(buf[i+ 8]) ? buf[i+ 8] : '.';
        c[ 9] = isprint(buf[i+ 9]) ? buf[i+ 9] : '.';
        c[10] = isprint(buf[i+10]) ? buf[i+10] : '.';
        c[11] = isprint(buf[i+11]) ? buf[i+11] : '.';
        c[12] = isprint(buf[i+12]) ? buf[i+12] : '.';
        c[13] = isprint(buf[i+13]) ? buf[i+13] : '.';
        c[14] = isprint(buf[i+14]) ? buf[i+14] : '.';
        c[15] = isprint(buf[i+15]) ? buf[i+15] : '.'; 

        retNbytes += fprintf(commLog->ngl_stream,
            "%10x  %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x    %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
            i,
            buf[i+ 0], buf[i+ 1], buf[i+ 2], buf[i+ 3], 
            buf[i+ 4], buf[i+ 5], buf[i+ 6], buf[i+ 7], 
            buf[i+ 8], buf[i+ 9], buf[i+10], buf[i+11], 
            buf[i+12], buf[i+13], buf[i+14], buf[i+15], 
            c[ 0], c[ 1], c[ 2], c[ 3], c[ 4], c[ 5], c[ 6], c[ 7],
            c[ 8], c[ 9], c[10], c[11], c[12], c[13], c[14], c[15]);
    }

    remain = nBytes - i;
    if (remain <= 0) {
        /* Success */
        return retNbytes;
    }

    curr = i;
    retNbytes += fprintf(commLog->ngl_stream, "%10x  ", i);
    for (; (i < nBytes) && ((nBytes - i) >= 4); i += 4) {
        retNbytes += fprintf(commLog->ngl_stream,
            "%02x%02x%02x%02x ",
            buf[i+ 0], buf[i+ 1], buf[i+ 2], buf[i+ 3]);
    }
    for (; i < nBytes; i++) {
        retNbytes += fprintf(commLog->ngl_stream, "%02x", buf[i]);
    }

    space = ((16 - remain) * 2) + (((16 - remain) / 4) - 1) + 4;
    for (i = 0; i < space; i++) {
        fputc(' ', commLog->ngl_stream);
        retNbytes++;
    }

    for (i = curr; i < nBytes; i++) {
        fputc(isprint(buf[i]) ? buf[i] : '.', commLog->ngl_stream);
        retNbytes++;
    }

    fputc('\n', commLog->ngl_stream);
    retNbytes++;

    /* Success */
    return retNbytes;
}
