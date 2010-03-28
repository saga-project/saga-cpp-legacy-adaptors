/*
 * $RCSfile: ngLog.c,v $ $Revision: 1.27 $ $Date: 2008/02/25 05:21:47 $
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

#include "ngUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngLog.c,v $ $Revision: 1.27 $ $Date: 2008/02/25 05:21:47 $")

#define NGL_LOG_BASE_EXECUTABLE_ID_UNDEF_FORMAT   "-execID-undefined-%s-pid-%ld"
#define NGL_LOG_BASE_EXECUTABLE_ID_DEFINED_FORMAT "-execID-%d"

#define NGL_LOG_DATE_AND_TIME_STRING_MAX 128

#define NGL_LOG_BASE_REQUIRE_FILE_NUMBER(log) \
    (((log)->nglb_arg.nglca_nFiles > 1) || ((log)->nglb_arg.nglca_nFiles == 0))

static const char *nglLogLevelStringTable[] = {
    "Off",
    "Fatal",
    "Error",
    "Warning",
    "Information",
    "Debug"
};

#define NGL_LOG_LEVEL_IS_VALID(loglevel)        \
   (((loglevel) == NG_LOG_LEVEL_OFF)         || \
    ((loglevel) == NG_LOG_LEVEL_FATAL)       || \
    ((loglevel) == NG_LOG_LEVEL_ERROR)       || \
    ((loglevel) == NG_LOG_LEVEL_WARNING)     || \
    ((loglevel) == NG_LOG_LEVEL_INFORMATION) || \
    ((loglevel) == NG_LOG_LEVEL_DEBUG))

/* File Local Functions */

/* Log Information */
static void nglLogInformationInitializeMember(ngLogConstructArgument_t *);

/* Log */
static int nglLogInitialize(
    ngLog_t *, const char *, int, ngLogConstructArgument_t *, ngLog_t *, int *);
static int nglLogFinalize(ngLog_t *, ngLog_t *, int *);
static void nglLogInitializeMember(ngLog_t *);
static ngLog_t *nglLogGetDefault(void);

/* Communication Log */
static int nglCommLogInitialize(ngCommLog_t *, ngCommLogPairInfo_t *pair,
    int, ngLogConstructArgument_t *, ngLog_t *, int *);
static int nglCommLogFinalize(ngCommLog_t *, ngLog_t *, int *);
static void nglCommLogInitializeMember(ngCommLog_t *);
static int nglCommLogPrint(
    ngCommLog_t *, char *, char *, size_t, ngLog_t *, int *);
static int nglCommLogDump(ngCommLog_t *, char *, size_t, int *);

/* Log Base */
static int nglLogBaseInitialize(
    ngLogBase_t *, const char *, int, ngLogConstructArgument_t *, ngLog_t *, int *);
static int nglLogBaseFinalize(ngLogBase_t *, ngLog_t *, int *);
static void nglLogBaseInitializeMember(ngLogBase_t *);
static int nglLogBaseInitializeFileFormat(ngLogBase_t *, ngLog_t *, int *);
static int nglLogBaseNewFile(ngLogBase_t *, ngLog_t *, int *);
static int nglLogBaseCreateFile(ngLogBase_t *, ngLog_t *, int *);
static int nglLogBaseCloseFile(ngLogBase_t *, ngLog_t *, int *);
static int nglLogBaseCreateFileName(ngLogBase_t *, ngLog_t *, int *);
static char *nglLogBaseProcessLogFileFormat(ngLogBase_t *, const char *, ngLog_t *, int *);
static int nglLogBaseDoesPrint(ngLogBase_t *, const char *, ngLogLevel_t);
static ngLogLevel_t nglLogBaseGetLogLevel(ngLogBase_t *, const char *);
static int nglLogBaseExecutableIDchanged(ngLogBase_t *, int, ngLog_t *, int *);

/* Other */
static int nglLogCreateLogDirectory(char *, int, ngLog_t *, int *);
static int nglLogIsDirectoryAvailable(char *, int *, ngLog_t *, int *);
static const char *nglLogLevelToString(ngLogLevel_t);
static int nglTimeValToString(struct timeval *, char *, size_t, int *error);
static int nglGetStringOfDateAndTime(char *, size_t, int *);
static int nglLogCurrentTimeStringGenerate(char *, int, int *, ngLog_t *, int *);

/**
 * Log Information: Initialize
 */
int
ngLogConstructArgumentInitialize(
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngLogConstructArgumentInitialize";

    if (logCarg == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Invalid argument.\n");
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        return 0;
    }
    nglLogInformationInitializeMember(logCarg);

    logCarg->nglca_nFiles = 1;

    return 1;
}

/**
 * Log Information: Finalize
 */
int
ngLogConstructArgumentFinalize(
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int ret = 1;
    int result;
    static const char fName[] = "ngLogConstructArgumentFinalize";

    if (logCarg == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Invalid argument.\n");
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    if (logCarg->nglca_filePath != NULL) {
        result = ngiFree(logCarg->nglca_filePath, log, error);
        if (result == 0) {
            ret = 0;
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
        logCarg->nglca_filePath = NULL;
    }

    if (logCarg->nglca_suffix != NULL) {
        result = ngiFree(logCarg->nglca_suffix, log, error);
        if (result == 0) {
            ret = 0;
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
        logCarg->nglca_suffix = NULL;
    }

    if (logCarg->nglca_categories != NULL) {
        result = ngiFree(logCarg->nglca_categories, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free array.\n");
            /* Through  */
        }
    }

    nglLogInformationInitializeMember(logCarg);

    return ret;
}


/**
 * Log Information: Initialize Member
 */
void
nglLogInformationInitializeMember(
    ngLogConstructArgument_t *logCarg)
{
    assert(logCarg != NULL);

    logCarg->nglca_output       = NG_LOG_NONE;
    logCarg->nglca_filePath     = NULL;
    logCarg->nglca_suffix       = NULL;
    logCarg->nglca_nFiles       = 0;
    logCarg->nglca_maxFileSize  = 0;
    logCarg->nglca_overWriteDir = 0;/* false */
    logCarg->nglca_appending    = 0;/* false */
    logCarg->nglca_categories   = NULL;
}

/**
 * Log Information: copy
 */
int
ngLogConstructArgumentCopy(
    ngLogConstructArgument_t *dest,
    ngLogConstructArgument_t *src,
    ngLog_t *log,
    int *error)
{
    char *filePath = NULL;
    char *suffix   = NULL;
    ngLogCategory_t *categories = NULL;
    int result;
    int i;
    static const char fName[] = "ngLogConstructArgumentCopy";

    if (src->nglca_filePath != NULL) {
        filePath = ngiStrdup(src->nglca_filePath, log, error);
        if (filePath == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't copy string.\n");
            goto error;
        }
    }

    if (src->nglca_suffix != NULL) {
        suffix = ngiStrdup(src->nglca_suffix, log, error);
        if (suffix == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't copy string.\n");
            goto error;
        }
    }

    if (src->nglca_categories != NULL) {
        i = 0;
        while(src->nglca_categories[i++].nglc_string != NULL);

        categories = ngiCalloc(i, sizeof(ngLogCategory_t), log, error);
        if (categories == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't allocate storage for categories array.\n");
            goto error;
        }
        /* Copy */
        for(i = 0;src->nglca_categories[i].nglc_string != NULL;++i) {
            categories[i] = src->nglca_categories[i];
        }
        categories[i].nglc_string = NULL;
        categories[i].nglc_level  = NG_LOG_LEVEL_OFF;/* Is ignored */
    }

    if (dest->nglca_filePath != NULL) {
        result = ngiFree(dest->nglca_filePath, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
    }

    if (dest->nglca_suffix != NULL) {
        result = ngiFree(dest->nglca_suffix, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
    }

    if (dest->nglca_categories != NULL) {
        result = ngiFree(dest->nglca_categories, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free array.\n");
            /* Through  */
        }
    }

    dest->nglca_output       = src->nglca_output;
    dest->nglca_nFiles       = src->nglca_nFiles;
    dest->nglca_filePath     = filePath; /* deep copy */
    dest->nglca_suffix       = suffix; /* deep copy */
    dest->nglca_maxFileSize  = src->nglca_maxFileSize;
    dest->nglca_overWriteDir = src->nglca_overWriteDir;
    dest->nglca_appending    = src->nglca_appending;
    dest->nglca_categories   = categories;/* deep copy */

    return 1;
error:
    if (suffix != NULL) {
        result = ngiFree(suffix, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
    }
    if (filePath != NULL) {
    result = ngiFree(filePath, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free string.\n");
            /* Through  */
        }
    }

    if (categories != NULL) {
    result = ngiFree(categories, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't free array.\n");
            /* Through  */
        }
    }

    return 0;
}

/**
 * Log: Construct
 */
ngLog_t *
ngLogConstruct(
    const char *appName,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    ngLog_t *newLog;
    static const char fName[] = "ngLogConstruct";

    newLog = ngLogConstructForExecutable(appName, 
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, logCarg, log, error);
    if (newLog == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't construct new log.\n");
        return NULL;
    }
    return newLog;
}

/**
 * Log: Construct for Executable
 */
ngLog_t *
ngLogConstructForExecutable(
    const char *appName,
    int executableID,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int result;
    ngLog_t *newLog = NULL;
    static const char fName[] = "ngLogConstructForExecutable";

    /* Allocate */
    newLog = NGI_ALLOCATE(ngLog_t, log, error);
    if (newLog == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't allocate the storage for Log.\n");
        goto error;
    }

    /* Initialize */
    result = nglLogInitialize(
        newLog, appName, executableID, logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't initialize the Log.\n");
	goto error;
    }

    /* Success */
    return newLog;

    /* Error occurred */
error:
    /* Deallocate */
    if (newLog != NULL) {
        result = NGI_DEALLOCATE(ngLog_t, newLog, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't deallocate the storage for Log.\n");
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Log: Destruct
 */
int
ngLogDestruct(
    ngLog_t *tgtLog,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngLogDestruct";

    /* Finalize */
    result = nglLogFinalize(tgtLog, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't finalize the Log.\n" );
        ret = 0;
        error = NULL;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngLog_t, tgtLog, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't deallocate the Log.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Log: Initialize
 */
static int
nglLogInitialize(
    ngLog_t *newLog,
    const char *appName,
    int executableID,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglLogInitialize";

    if (newLog == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Invalid log.\n");
        return 0;
    }
    nglLogInitializeMember(newLog);

    result = nglLogBaseInitialize(
        &newLog->ngl_base, appName, executableID, logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't initialize the log base.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Log: Finalize
 */
static int
nglLogFinalize(
    ngLog_t *tgtLog,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "nglLogFinalize";

    assert(tgtLog != NULL);

    /* Finalize the Mutex */
    result = nglLogBaseFinalize(&tgtLog->ngl_base, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't finalize the log base.\n");
        ret = 0;
        error = NULL;
    }

    /* Initialize the members */
    nglLogInitializeMember(tgtLog);

    return ret;
}

/**
 * Log: Initialize member
 */
static void
nglLogInitializeMember(
    ngLog_t *log)
{
#if 0
    static const char fName[] = "nglLogInitializeMember";
#endif
    nglLogBaseInitializeMember(&log->ngl_base);

    return;
}

void
ngLogPrintf(
    ngLog_t *log,
    const char *category,
    ngLogLevel_t loglevel,
    const char *object,
    const char *funcName,
    const char *format,
    ...)
{
    va_list ap;
#if 0
    static const char fName[] = "ngLogPrintf";
#endif

    va_start(ap, format);
    ngLogVprintf(log, category, loglevel, object, funcName, format, ap);
    va_end(ap);

    return;
}

void
ngLogVprintf(
    ngLog_t      *log,
    const char   *category,
    ngLogLevel_t  loglevel,
    const char   *object,
    const char   *funcName,
    const char   *format,
    va_list ap)
{
    const char *sLevel;
    int error;
    char sDate[NGL_LOG_DATE_AND_TIME_STRING_MAX] = "";
    int result;
    int locked = 0;
    int n;
    static const char fName[] = "ngLogVprintf";

    /* Tries to output log as much as possible */
    if (category == NULL) {
        category = "NULL";
    }
    if (!NGL_LOG_LEVEL_IS_VALID(loglevel)) {
        loglevel = NG_LOG_LEVEL_FATAL;
    }

    if (format == NULL) {
        format = "(No message)\n";
    }

    if (log == NULL) {
        log = nglLogGetDefault();
        /* nglLogGetDefault() doesn't return NULL. */
        assert(log != NULL);
    }

    if (log->ngl_base.nglb_stream == NULL) {
        /* Do nothing */
        /* Success */
        return;
    }

    /* Does print this message? */
    if (!nglLogBaseDoesPrint(&log->ngl_base, category, loglevel)) {
        /* This message does not print out */
        return;
    }

    result = ngiMutexLock(&log->ngl_base.nglb_mutex, NULL, &error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't lock the log's mutex.\n", fName);
        /* Through */
    } else {
        locked = 1;
    }

    /* Get the level String */
    sLevel = nglLogLevelToString(loglevel);
    /* It will change to a new file, when log is over */
    result = nglLogBaseNewFile(&log->ngl_base, NULL, &error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't create the new file for the log.\n", fName);
        goto error;
    }

    /* Get the date */
    result = nglGetStringOfDateAndTime(sDate,
        NGL_LOG_DATE_AND_TIME_STRING_MAX, &error);
    if (result == 0) {
        fprintf(stderr, "%s: Can't get the string of date and time.\n", fName);
        goto error;
    }

    /* Log Output */
    n = log->ngl_base.nglb_outputNbytes;

    n += fprintf(log->ngl_base.nglb_stream, "%s", sDate);
    if (log->ngl_base.nglb_appName != NULL) {
        n += fprintf(log->ngl_base.nglb_stream, ": %s", log->ngl_base.nglb_appName);
    }
    if (log->ngl_base.nglb_hostName != NULL) {
        n += fprintf(log->ngl_base.nglb_stream, ": %s", log->ngl_base.nglb_hostName);
    }
    n += fprintf(log->ngl_base.nglb_stream, ": %s", sLevel);
#if defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG)
    if (nglLogBaseDoesPrint(&log->ngl_base, category, NG_LOG_LEVEL_DEBUG)) {
        /* Print pthread_t.                             *
         * this depend implementation of thread library.*/
        n += fprintf(log->ngl_base.nglb_stream, ": Thread %lu",
                    (unsigned long)pthread_self());
    }
#endif /* defined(NG_PTHREAD) && !defined(NGI_NO_THREAD_T_CASTABLE_LONG) */

    n += fprintf(log->ngl_base.nglb_stream, ": %s", category);

    if (object != NULL) {
        n += fprintf(log->ngl_base.nglb_stream, ": %s", object);
    }
    n +=  fprintf(log->ngl_base.nglb_stream, ": %s: ", funcName);
    n += vfprintf(log->ngl_base.nglb_stream, format, ap);

    log->ngl_base.nglb_outputNbytes = n;

    if (locked != 0) {
        result = ngiMutexUnlock(&log->ngl_base.nglb_mutex, NULL, &error);
        if (result == 0) {
            fprintf(stderr, "%s: Can't unlock the log's mutex.\n", fName);
        }
        locked = 0;
    }

    /* Success */
    return;

error:
    return;
}

#define NGL_LOG_FUNC_DEFINE(func, level)                        \
void                                                            \
func(                                                           \
    ngLog_t *log,                                               \
    const char *category,                                       \
    const char *funcName,                                       \
    const char *fmt,                                            \
    ...)                                                        \
{                                                               \
    va_list ap;                                                 \
                                                                \
    va_start(ap, fmt);                                          \
    ngLogVprintf(log, category, level, NULL, funcName, fmt, ap);\
    va_end(ap);                                                 \
                                                                \
    return;                                                     \
}

NGL_LOG_FUNC_DEFINE(ngLogDebug, NG_LOG_LEVEL_DEBUG)
NGL_LOG_FUNC_DEFINE(ngLogInfo,  NG_LOG_LEVEL_INFORMATION)
NGL_LOG_FUNC_DEFINE(ngLogWarn,  NG_LOG_LEVEL_WARNING)
NGL_LOG_FUNC_DEFINE(ngLogError, NG_LOG_LEVEL_ERROR)
NGL_LOG_FUNC_DEFINE(ngLogFatal, NG_LOG_LEVEL_FATAL)

#undef NGL_LOG_FUNC_DEFINE

/**
 * Log File: The Executable ID was resolved. Rename filename.
 */
int
ngiLogExecutableIDchanged(
    ngLog_t *log,
    int newExecutableID,
    int *error)
{
#if 0
    static const char fName[] = "ngiLogExecutableIDchanged";
#endif
    
    return nglLogBaseExecutableIDchanged(
        &log->ngl_base, newExecutableID, log, error);
}

static int nglDefaultLogIsNull = 0;
static ngiMutex_t defaultLogMutex = NGI_MUTEX_INITIALIZER;
/**
 * Log: set default log to null
 */
int
ngLogSetDefaultNull(
    void)
{
    int result;
#if 0
    static const char fName[] = "ngLogSetDefaultNull";
#endif

    result = ngiMutexLock(&defaultLogMutex, NULL, NULL);
    if (result == 0) {
        return 0;
    }
    nglDefaultLogIsNull = 1;

    result = ngiMutexUnlock(&defaultLogMutex, NULL, NULL);
    if (result == 0) {
        return 0;
    }
    return 1;
}

/**
 * Log: get default log(stderr or null)
 */
static ngLog_t *
nglLogGetDefault(void)
{
    ngLog_t *log = NULL;
    int locked = 0;
    int result;
    static int first = 0;
    static ngLog_t def = {{
        NGI_MUTEX_INITIALIZER,                     /* nglb_mutex */
        {NG_LOG_STDERR, NULL, NULL, 0, 0, 0, 0, NULL},/* nglb_arg */
        NG_LOG_LEVEL_ERROR,                        /* nglb_defaultLevel */
        NULL,                                      /* nglb_stream */
        NULL,                                      /* nglb_fileFormat */
        0,                                         /* nglb_currentNo */
        NULL,                                      /* nglb_fileName */
        0,                                         /* nglb_fileNameBufferSize */
        0,                                         /* nglb_nFigures */
        NULL,                                      /* nglb_appName */
        0,                                         /* nglb_outputNbytes */
        NULL,                                      /* nglb_hostName */
        0l,                                        /* nglb_pid */
        0}};                                       /* nglb_executableID */
    static ngLog_t nullLog = {{
        NGI_MUTEX_INITIALIZER,                     /* nglb_mutex */
        {NG_LOG_NONE, NULL, NULL, 0, 0, 0, 0, NULL},  /* nglb_arg */
        NG_LOG_LEVEL_ERROR,                        /* nglb_defaultLevel */
        NULL,                                      /* nglb_stream */
        NULL,                                      /* nglb_fileFormat */
        0,                                         /* nglb_currentNo */
        NULL,                                      /* nglb_fileName */
        0,                                         /* nglb_fileNameBufferSize */
        0,                                         /* nglb_nFigures */
        NULL,                                      /* nglb_appName */
        0,                                         /* nglb_outputNbytes */
        NULL,                                      /* nglb_hostName */
        0l,                                        /* nglb_pid */
        0}};                                       /* nglb_executableID */
    /* Lock */
    result = ngiMutexLock(&defaultLogMutex, NULL, NULL);
    if (result != 0) {
        locked = 1;
    }

    if (first == 0) {
        first = 1;
        def.ngl_base.nglb_stream = stderr;
        def.ngl_base.nglb_defaultLevel = ngLogGetLogLevelFromEnv();
    }
    if (nglDefaultLogIsNull != 0) {
        log = &nullLog;
    } else {
        log = &def;
    }

    /* Unlock */
    if (locked != 0) {
        result = ngiMutexUnlock(&defaultLogMutex, NULL, NULL);
        /* Ignore Error */
    }

    return log;
}

/**
 * Communication Log; Construct
 */
ngCommLog_t *
ngCommLogConstruct(
    ngCommLogPairInfo_t *pair,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    ngCommLog_t *commLog;
    static const char fName[] = "ngCommLogConstruct";

    commLog = ngCommLogConstructForExecutable(pair, 
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, logCarg, log, error);
    if (commLog == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't construct new communication log.\n");
        return NULL;
    }
    return commLog;
}

/**
 * Communication Log: Construct for Executable
 */
ngCommLog_t *
ngCommLogConstructForExecutable(
    ngCommLogPairInfo_t *pair,
    int executableID,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int result;
    ngCommLog_t *commLog = NULL;
    static const char fName[] = "ngCommLogConstructForExecutable";

    /* Allocate */
    commLog = NGI_ALLOCATE(ngCommLog_t, log, error);
    if (commLog == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't allocate the storage for communication log.\n");
        goto error;
    }

    /* Initialize */
    result = nglCommLogInitialize(
        commLog, pair, executableID, logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't initialize the communication log.\n");
	goto error;
    }

    /* Success */
    return commLog;

    /* Error occurred */
error:
    /* Deallocate */
    if (commLog != NULL) {
        result = NGI_DEALLOCATE(ngCommLog_t, commLog, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't deallocate the storage for communication log.\n");
        }
    }

    /* Failed */
    return NULL;
}

/**
 * Communication Log; Destruct
 */
int
ngCommLogDestruct(
    ngCommLog_t *commLog,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngCommLogDestruct";

    /* Finalize */
    result = nglCommLogFinalize(commLog, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't finalize the communication log.\n" );
        ret = 0;
        error = NULL;
    }

    /* Deallocate */
    result = NGI_DEALLOCATE(ngCommLog_t, commLog, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't deallocate the communication log.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Communication Log: Initialize
 */
static int
nglCommLogInitialize(
    ngCommLog_t *commLog,
    ngCommLogPairInfo_t *pair,
    int executableID,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int result;
    ngCommLogPairInfo_t *dst = NULL;
    static const char fName[] = "nglCommLogInitialize";

    if (commLog == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Invalid communication log.\n");
        return 0;
    }
    nglCommLogInitializeMember(commLog);

    result = nglLogBaseInitialize(
        &commLog->ngcl_base, NULL, executableID, logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't initialize the log base.\n");
        goto error;
    }

    commLog->ngcl_firstTime = 1;/* true */
    dst = &commLog->ngcl_pairInfo;

    dst->ngcp_localAppName   = ngiStrdup(pair->ngcp_localAppName, log, error);
    dst->ngcp_localHostname  = ngiStrdup(pair->ngcp_localHostname, log, error);
    dst->ngcp_remoteAppName  = ngiStrdup(pair->ngcp_remoteAppName, log, error);
    dst->ngcp_remoteHostname = ngiStrdup(pair->ngcp_remoteHostname, log, error);
    if ((dst->ngcp_localAppName   == NULL) ||
        (dst->ngcp_localHostname  == NULL) ||
        (dst->ngcp_remoteAppName  == NULL) ||
        (dst->ngcp_remoteHostname == NULL)) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Can't copy the string.\n");
        goto error;
    }

    /* Success */
    return 1;
error:
    result = nglCommLogFinalize(commLog, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't finalize the communication log.\n");
    }
    return 0;
}

/**
 * Communication Log: Finalize
 */
static int
nglCommLogFinalize(
    ngCommLog_t *commLog,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    ngCommLogPairInfo_t *pair = NULL;
    static const char fName[] = "nglCommLogFinalize";

    assert(commLog != NULL);

    commLog->ngcl_firstTime = 1;/* true */
    pair = &commLog->ngcl_pairInfo;

    if (pair->ngcp_localAppName != NULL) {
        result = ngiFree(pair->ngcp_localAppName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
    }
    pair->ngcp_localAppName = NULL;

    if (pair->ngcp_localHostname != NULL) {
        result = ngiFree(pair->ngcp_localHostname, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
    }
    pair->ngcp_localHostname = NULL;

    if (pair->ngcp_remoteAppName != NULL) {
        result = ngiFree(pair->ngcp_remoteAppName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
    }
    pair->ngcp_remoteAppName = NULL;

    if (pair->ngcp_remoteHostname != NULL) {
        result = ngiFree(pair->ngcp_remoteHostname, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
    }
    pair->ngcp_remoteHostname = NULL;

    /* Finalize the log base*/
    result = nglLogBaseFinalize(&commLog->ngcl_base, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't finalize the log base.\n");
        ret = 0;
        error = NULL;
    }

    /* Initialize the members */
    nglCommLogInitializeMember(commLog);

    return ret;
}

/**
 * Communication Log: Initialize member
 */
static void
nglCommLogInitializeMember(
    ngCommLog_t *commLog)
{
#if 0
    static const char fName[] = "nglCommLogInitializeMember";
#endif
    nglLogBaseInitializeMember(&commLog->ngcl_base);

    commLog->ngcl_pairInfo.ngcp_localAppName   = NULL;
    commLog->ngcl_pairInfo.ngcp_localHostname  = NULL;
    commLog->ngcl_pairInfo.ngcp_remoteAppName  = NULL;
    commLog->ngcl_pairInfo.ngcp_remoteHostname = NULL;

    commLog->ngcl_execTime.nget_real.nget_start.tv_sec      = 0;
    commLog->ngcl_execTime.nget_real.nget_start.tv_usec     = 0;
    commLog->ngcl_execTime.nget_real.nget_end.tv_sec        = 0;
    commLog->ngcl_execTime.nget_real.nget_end.tv_usec       = 0;
    commLog->ngcl_execTime.nget_real.nget_execution.tv_sec  = 0;
    commLog->ngcl_execTime.nget_real.nget_execution.tv_usec = 0;

    commLog->ngcl_execTime.nget_cpu.nget_start.tv_sec      = 0;
    commLog->ngcl_execTime.nget_cpu.nget_start.tv_usec     = 0;
    commLog->ngcl_execTime.nget_cpu.nget_end.tv_sec        = 0;
    commLog->ngcl_execTime.nget_cpu.nget_end.tv_usec       = 0;
    commLog->ngcl_execTime.nget_cpu.nget_execution.tv_sec  = 0;
    commLog->ngcl_execTime.nget_cpu.nget_execution.tv_usec = 0;

    commLog->ngcl_firstTime = 0;

    return;
}

/**
 * Communication Log; Send
 */
int
ngCommLogSend(
    ngCommLog_t *commLog,
    char *data,
    size_t size,
    ngLog_t *log,
    int *error)
{
#if 0
    static const char fName[] = "ngCommLogSend";
#endif

    return nglCommLogPrint(commLog, "Send", data, size, log, error);
}
    
/**
 * Communication Log; Receive
 */
int
ngCommLogReceive(
    ngCommLog_t *commLog,
    char *data,
    size_t size,
    ngLog_t *log,
    int *error)
{
#if 0
    static const char fName[] = "ngCommLogReceive";
#endif

    return nglCommLogPrint(commLog, "Receive", data, size, log, error);
}

/**
 * Communication Log; print
 */
static int
nglCommLogPrint(
    ngCommLog_t *commLog,
    char *method,
    char *data,
    size_t size,
    ngLog_t *log,
    int *error)
{
    int result;
    struct timeval tv;
    char sDate[NGL_LOG_DATE_AND_TIME_STRING_MAX] = "";
    int locked = 0;
    int n;
    int ret = 1;
    static const char fName[] = "nglCommLogPrint";

    /* Is log disable? */
    if (commLog == NULL) {
        /* Success */
        return 1;
    }

    result = ngiMutexLock(&commLog->ngcl_base.nglb_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't lock the mutex.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    locked = 1;

    /* Get the Execution time */
    result = ngiSetEndTime(&commLog->ngcl_execTime, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't get the Execution time.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Copy the current time to start time */
    commLog->ngcl_execTime.nget_real.nget_start
        = commLog->ngcl_execTime.nget_real.nget_end;

    /* Get the date */
    result = nglGetStringOfDateAndTime(sDate,
        NGL_LOG_DATE_AND_TIME_STRING_MAX, error);
    result = nglTimeValToString(&commLog->ngcl_execTime.nget_real.nget_start,
        sDate, NGL_LOG_DATE_AND_TIME_STRING_MAX, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't get the string of date and time.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* It will change to a new file, when log is over */
    result = nglLogBaseNewFile(&commLog->ngcl_base, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't create the new log file.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Print out the Communication log */
    n = 0;
    n += fprintf(commLog->ngcl_base.nglb_stream, "%s  ", sDate);
    n += fprintf(commLog->ngcl_base.nglb_stream, 
        "%s: %s, %s: %s  ",
        commLog->ngcl_pairInfo.ngcp_localAppName,
        commLog->ngcl_pairInfo.ngcp_localHostname,
        commLog->ngcl_pairInfo.ngcp_remoteAppName,
        commLog->ngcl_pairInfo.ngcp_remoteHostname);
    if (commLog->ngcl_firstTime != 0) {
        commLog->ngcl_firstTime = 0;
    } else {
        tv = commLog->ngcl_execTime.nget_real.nget_execution;
        n += fprintf(commLog->ngcl_base.nglb_stream, 
            "(%3lds %3ldms %3ldus) ",
            tv.tv_sec,
            (long)tv.tv_usec / 1000,
            (long)tv.tv_usec % 1000);
    }

    n += fprintf(commLog->ngcl_base.nglb_stream, 
        "%s: %lubytes\n", method, (unsigned long)size);

    /* Print the communication data */
    result = nglCommLogDump(commLog, data, size, error);
    if (result < 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't print the communication log.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }
    n += result;
    commLog->ngcl_base.nglb_outputNbytes += n;

finalize:
    /* Unlock the Log Manager */
    if (locked != 0) {
        result = ngiMutexUnlock(&commLog->ngcl_base.nglb_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't unlock the mutex.\n");
            ret = 0;
            error = NULL;
        }
        locked = 0;
    }

    /* Success */
    return ret;
}

/**
 * Dump the communication data.
 */
static int
nglCommLogDump(
    ngCommLog_t *commLog,
    char *buf,
    size_t nBytes,
    int *error)
{
    int i;
    int curr;
    int j;
    int retNbytes = 0;
    int remain, space;
    unsigned char *ubuf = (unsigned char *)buf;
    char c[16];

    for (i = 0; (i < nBytes) && ((nBytes - i) >= 16); i += 16) {
        for (j = 0;j < 16;++j) {
            c[j] = isprint(ubuf[i+j]) ? ubuf[i+j] : '.';
        }

        retNbytes += fprintf(commLog->ngcl_base.nglb_stream,
            "%10x  %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x"
            "    %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
            i,
            ubuf[i+ 0], ubuf[i+ 1], ubuf[i+ 2], ubuf[i+ 3], 
            ubuf[i+ 4], ubuf[i+ 5], ubuf[i+ 6], ubuf[i+ 7], 
            ubuf[i+ 8], ubuf[i+ 9], ubuf[i+10], ubuf[i+11], 
            ubuf[i+12], ubuf[i+13], ubuf[i+14], ubuf[i+15], 
            c[ 0], c[ 1], c[ 2], c[ 3], c[ 4], c[ 5], c[ 6], c[ 7],
            c[ 8], c[ 9], c[10], c[11], c[12], c[13], c[14], c[15]);
    }

    remain = nBytes - i;
    if (remain <= 0) {
        /* Success */
        return retNbytes;
    }

    curr = i;
    retNbytes += fprintf(commLog->ngcl_base.nglb_stream, "%10x  ", i);
    for (; (i < nBytes) && ((nBytes - i) >= 4); i += 4) {
        retNbytes += fprintf(commLog->ngcl_base.nglb_stream,
            "%02x%02x%02x%02x ",
            ubuf[i+ 0], ubuf[i+ 1], ubuf[i+ 2], ubuf[i+ 3]);
    }
    for (; i < nBytes; i++) {
        retNbytes += fprintf(commLog->ngcl_base.nglb_stream, "%02x", ubuf[i]);
    }

    space = ((16 - remain) * 2) + (((16 - remain) / 4) - 1) + 4;
    for (i = 0; i < space; i++) {
        fputc(' ', commLog->ngcl_base.nglb_stream);
        retNbytes++;
    }

    for (i = curr; i < nBytes; i++) {
        fputc(isprint(ubuf[i]) ? ubuf[i] : '.', commLog->ngcl_base.nglb_stream);
        retNbytes++;
    }

    fputc('\n', commLog->ngcl_base.nglb_stream);
    retNbytes++;

    /* Success */
    return retNbytes;
}

/**
 * Log Base: Initialize
 */
static int
nglLogBaseInitialize(
    ngLogBase_t *logBase,
    const char *appName,
    int executableID,
    ngLogConstructArgument_t *logCarg,
    ngLog_t *log,
    int *error)
{
    int result;
    char hostName[NGI_HOST_NAME_MAX];
    static const char fName[] = "nglLogBaseInitialize";

    if (logBase == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Invalid log.\n");
        return 0;
    }
    nglLogBaseInitializeMember(logBase);

    if (logCarg == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName, "Invalid log info.\n");
        return 0;
    }

    if ((logCarg->nglca_output == NG_LOG_FILE) &&
        (logCarg->nglca_filePath == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Output destination is file, but file name is not specified.\n");
        return 0;
    }

    result = ngLogConstructArgumentInitialize(&logBase->nglb_arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't initialize the log information.\n");
        goto error;
    }

    result = ngLogConstructArgumentCopy(&logBase->nglb_arg, logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't copy the log information.\n");
        goto error;
    }

    /* Initialize the Log output status */
    logBase->nglb_currentNo = 0;
    logBase->nglb_outputNbytes = 0;

    result = ngiMutexInitialize(&logBase->nglb_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't initialize the Mutex.\n");
        goto error;
    }

    /* Is Log rotation size default? */
    if ((NGL_LOG_BASE_REQUIRE_FILE_NUMBER(logBase)) &&
        (logCarg->nglca_maxFileSize == 0)) {
        logBase->nglb_arg.nglca_maxFileSize = NG_LOG_DEFAULT_ROTATION_SIZE;
    }
    if ((logCarg->nglca_appending != 0) &&
        (logCarg->nglca_maxFileSize != 0)) {
        ngLogWarn(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't specify opening file for appending on max file size is not unlimited.\n");
        logCarg->nglca_appending = 0;
    }


    /* Copy the application name */
    if (appName != NULL) {
	logBase->nglb_appName = ngiStrdup(appName, log, error);
        if (logBase->nglb_appName == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't duplicate the application name.\n");
            goto error;
        }
    }

    /* Get the host name */
    result = ngiHostnameGet(hostName, sizeof(hostName), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't get the host name.\n");
        goto error;
    }
    logBase->nglb_hostName = ngiStrdup(hostName, log, error);
    if (logBase->nglb_hostName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't duplicate the host name.\n");
        goto error;
    }

    logBase->nglb_pid = getpid();

    logBase->nglb_defaultLevel = ngLogGetLogLevelFromEnv();
    logBase->nglb_defaultLevel = nglLogBaseGetLogLevel(logBase, NG_LOGCAT_DEFAULT);
    logBase->nglb_executableID = executableID;

    /* Is file path not specified? */
    switch (logCarg->nglca_output) {
    case NG_LOG_NONE:
        break;
    case NG_LOG_STDERR:
	logBase->nglb_stream = stderr;
        break;
    case NG_LOG_FILE:
        /* Initialize the Log File */
        result = nglLogBaseInitializeFileFormat(logBase, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't initialize Log File.\n");
            goto error;
        }

        /* Create the log file directory */
        result = nglLogCreateLogDirectory(
            logBase->nglb_fileFormat, logCarg->nglca_overWriteDir, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't create the log directory from format \"%s\".\n",
                logBase->nglb_fileFormat);
            return 0;
        }

        /* Create the Log File */
        result = nglLogBaseCreateFile(logBase, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't create the Log File.\n");
            goto error;
        }
        break;
    }
    /* Success */
    return 1;
error:

    result = nglLogBaseFinalize(logBase, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't finalize the log base.\n");
    }

    return 0;
}

/**
 * Log Base: Finalize
 */
static int
nglLogBaseFinalize(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "nglLogBaseFinalize";

    assert(logBase != NULL);

    /* Close the log file */
    if ((logBase->nglb_stream != stderr) &&
        (logBase->nglb_stream != NULL)) {
	result = nglLogBaseCloseFile(logBase,log, error);
	if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't close the Log File.\n");
            ret = 0;
            error = NULL;
	}
    }

    /* Deallocate the file format */
    if (logBase->nglb_fileFormat != NULL) {
        result = ngiFree(logBase->nglb_fileFormat, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't free the storage for log file format.\n");
            ret = 0;
            error = NULL;
        }
    }
    logBase->nglb_fileFormat = NULL;

    /* Deallocate the file name */
    if (logBase->nglb_fileName != NULL) {
	result = ngiFree(logBase->nglb_fileName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't free the storage for log file name.\n");
            ret = 0;
            error = NULL;
        }
    }
    logBase->nglb_fileName = NULL;

    /* Deallocate the host name */
    if (logBase->nglb_hostName != NULL) {
	result = ngiFree(logBase->nglb_hostName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't free the storage for hostname.\n");
            ret = 0;
            error = NULL;
        }
    }
    logBase->nglb_hostName = NULL;

    /* Deallocate the message */
    if (logBase->nglb_appName != NULL) {
        result = ngiFree(logBase->nglb_appName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't free the storage for application name.\n");
            ret = 0;
            error = NULL;
        }
    }
    logBase->nglb_appName = NULL;

    /* Release the Log Information */
    result = ngLogConstructArgumentFinalize(&logBase->nglb_arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't release the Log Information.\n");
        ret = 0;
        error = NULL;
    }

    /* Finalize the Mutex */
    result = ngiMutexDestroy(&logBase->nglb_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't finalize the Mutex.\n");
        ret = 0;
        error = NULL;
    }

    /* Initialize the members */
    nglLogBaseInitializeMember(logBase);

    return ret;
}

/**
 * Log Base: Initialize member
 */
static void
nglLogBaseInitializeMember(
    ngLogBase_t *logBase)
{
    nglLogInformationInitializeMember(&logBase->nglb_arg);

    logBase->nglb_defaultLevel        = NG_LOG_LEVEL_OFF;
    logBase->nglb_stream              = NULL;
    logBase->nglb_fileFormat          = NULL;
    logBase->nglb_currentNo           = 0;
    logBase->nglb_fileName            = NULL;
    logBase->nglb_fileNameBufferSize  = 0U;
    logBase->nglb_nFigures            = 0;
    logBase->nglb_appName             = NULL;
    logBase->nglb_outputNbytes        = 0U;

    logBase->nglb_hostName            = NULL;
    logBase->nglb_pid                 = 0;
    
    logBase->nglb_executableID        = 0;

    return;
}

/**
 * Log: Initialize the File.
 */
static int
nglLogBaseInitializeFileFormat(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    ngLogConstructArgument_t *logCarg = NULL;
    int n;
    static const char fName[] = "nglLogBaseInitializeFileFormat";

    /* Check the arguments */
    assert(logBase != NULL);
    assert(logBase->nglb_fileFormat == NULL);
    assert(logBase->nglb_fileName == NULL);

    logCarg = &logBase->nglb_arg;

    /* Get the digits of file number */
    n = logCarg->nglca_nFiles;

    logBase->nglb_nFigures = 1;
    while (n > 10) {
        logBase->nglb_nFigures++;
        n /= 10;
    }

    logBase->nglb_fileFormat = nglLogBaseProcessLogFileFormat(
        logBase, logCarg->nglca_filePath, log, error);
    if (logBase->nglb_fileFormat == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't process log file path.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Log Base: New file
 */
static int
nglLogBaseNewFile(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglLogBaseNewFile";

    /* Is an output stream a file? */
    if ((logBase->nglb_stream == stderr) || (logBase->nglb_stream == NULL)) {
        /* Success */
        return 1;
    }

    /* Is file size unlimited? */
    if (logBase->nglb_arg.nglca_maxFileSize == 0) {

        /* Success */ 
        return 1;
    }

    /* Is overflow the logBase file? */
    if (logBase->nglb_outputNbytes < logBase->nglb_arg.nglca_maxFileSize) {
        /* Success */
        return 1;
    }
    logBase->nglb_outputNbytes = 0;

    /* logBase file was overflowed. Create a new file */
    result = nglLogBaseCreateFile(logBase, log, error);
    if (result == 0) {
        logBase->nglb_stream = stderr;
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't create the Log File.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Log Base: Create File
 */
static int
nglLogBaseCreateFile(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    int result;
    FILE *stream;
    char *mode = "w";
    static const char fName[] = "nglLogBaseCreateFile";

    /* Check the arguments */
    assert(logBase != NULL);

    /* Close the current file */
    result = nglLogBaseCloseFile(logBase, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
	    "Can't close the Log File \"%s\".\n", logBase->nglb_fileName);
	return 0;
    }

    /* Make the File Name */
    result = nglLogBaseCreateFileName(logBase, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
    	    "Creating new file name failed.\n");
	return 0;
    }

    if ((logBase->nglb_arg.nglca_appending != 0) &&
        (logBase->nglb_arg.nglca_maxFileSize == 0)) {
        mode = "a";
    }

    /* Create the file */
    stream = fopen(logBase->nglb_fileName, mode);
    if (stream == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't create the Log File \"%s\".\n", logBase->nglb_fileName);
        return 0;
    }
    logBase->nglb_stream = stream;

    /* Disable buffering */
    setvbuf(stream, NULL, _IONBF, 0);

    /* Success */
    return 1;
}

/**
 * Log Base: Close File
 */
static int
nglLogBaseCloseFile(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    int result;
    FILE *stream;
    static const char fName[] = "nglLogBaseCloseFile";

    assert(logBase != NULL);
    assert(logBase->nglb_stream != stderr);

    stream = logBase->nglb_stream;
    if (stream != NULL) {
	logBase->nglb_stream = NULL;
    	result = fclose(stream);
	if (result != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
		"Can't close the Log File.\n");
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Log Base: Create file name
 */
static int
nglLogBaseCreateFileName(
    ngLogBase_t *logBase,
    ngLog_t *log,
    int *error)
{
    int result;
    int bufCur;
    size_t bufLen;
    static const char fName[] = "nglLogBaseCreateFileName";

    /* Check the arguments */
    assert(logBase != NULL);

    if (logBase->nglb_fileName == NULL) {
        /* Get the file name buffer size */
        logBase->nglb_fileNameBufferSize =
            strlen(logBase->nglb_fileFormat)
            + strlen(NGL_LOG_BASE_EXECUTABLE_ID_UNDEF_FORMAT)
            + strlen(logBase->nglb_hostName) + NGI_INT_MAX_DECIMAL_DIGITS * 2 /* pid */
            + strlen(NGL_LOG_BASE_EXECUTABLE_ID_DEFINED_FORMAT)
            + NGI_INT_MAX_DECIMAL_DIGITS /* executable ID */
            + NGI_INT_MAX_DECIMAL_DIGITS /* unlimited nFiles */
            + 1;

        /* Allocate the storage for Log File Name */
        logBase->nglb_fileName = ngiMalloc(logBase->nglb_fileNameBufferSize, log, error);
        if (logBase->nglb_fileName == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't allocate the storage for Log File Name.\n");
            return 0;
        }
        logBase->nglb_fileName[0] = '\0';
    }

    strcpy(logBase->nglb_fileName, logBase->nglb_fileFormat);
    bufCur = strlen(logBase->nglb_fileFormat);
    bufLen = logBase->nglb_fileNameBufferSize;

    /* Append Executable ID string  */
    switch (logBase->nglb_executableID) {
    case NGI_LOG_EXECUTABLE_ID_NOT_APPEND:
        result = 1;
        /* Do nothing */
        break;
    case NGI_LOG_EXECUTABLE_ID_UNDEFINED:
        result = snprintf(&logBase->nglb_fileName[bufCur], bufLen - bufCur,
            NGL_LOG_BASE_EXECUTABLE_ID_UNDEF_FORMAT,
            logBase->nglb_hostName, (long)logBase->nglb_pid);
        bufCur += result;
        break;
    default:
        assert(logBase->nglb_executableID >= 0);

        result = snprintf(&logBase->nglb_fileName[bufCur], bufLen - bufCur,
            NGL_LOG_BASE_EXECUTABLE_ID_DEFINED_FORMAT, logBase->nglb_executableID);
        bufCur += result;
    }
    assert(result > 0);
    assert(bufCur < logBase->nglb_fileNameBufferSize);

    /* Append Number */
    if (NGL_LOG_BASE_REQUIRE_FILE_NUMBER(logBase)) {
        result = snprintf(&logBase->nglb_fileName[bufCur], bufLen - bufCur,
            "-%0*d", logBase->nglb_nFigures, logBase->nglb_currentNo);
        bufCur += result;
    }
    assert(result > 0);
    assert(bufCur < logBase->nglb_fileNameBufferSize);

    if (logBase->nglb_arg.nglca_suffix != NULL) {
        result = snprintf(&logBase->nglb_fileName[bufCur], bufLen - bufCur,
            ".%s", logBase->nglb_arg.nglca_suffix);
        bufCur += result;
    }
    assert(result > 0);
    assert(bufCur < logBase->nglb_fileNameBufferSize);

    /* Increment the File No. */
    logBase->nglb_currentNo++;
    if ((logBase->nglb_arg.nglca_nFiles != 0) &&
        (logBase->nglb_currentNo >= logBase->nglb_arg.nglca_nFiles)) {
    	logBase->nglb_currentNo = 0;
    }

    /* Success */
    return 1;
}

/**
 * Log Base: Process log file format
 */
static char *
nglLogBaseProcessLogFileFormat(
    ngLogBase_t *logBase,
    const char *logFileFormat,
    ngLog_t *log,
    int *error)
{
    int srcCur = 0;
    int dstCur = 0;
    char *dst;
    size_t dstSize;
    int length;
    char fileFormat[NGI_FILE_NAME_MAX];
    char *ret = NULL;
    int prevPercent = 0;/* true */
    int result;
    static const char fName[] = "nglLogBaseProcessLogFileFormat";

    dst = fileFormat;
    dstSize = sizeof(fileFormat);

    /* Zero clear the fileFormat */
    memset(fileFormat, 0, sizeof(fileFormat));

    for (;logFileFormat[srcCur] != '\0';srcCur++) {
        if (prevPercent != 0/*false*/) {
            switch (logFileFormat[srcCur]) {
            case 't' : /* "%t" time */
                result = nglLogCurrentTimeStringGenerate(
                    &dst[dstCur], dstSize - dstCur, &length, NULL, error);
                if (result == 0) {
                    ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                        "Making time string failed.\n");
                    goto error;
                }

                dstCur += length;
                break;

            case 'h' : /* "%h" hostname */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%s", logBase->nglb_hostName);
                break;

            case 'p' : /* "%p" process id */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%ld", (long)logBase->nglb_pid);
                break;

            case '%' : /* "%%" just % */
                /* "output "%%" for to give "%" to filename */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%%%%");
                break;

            default:
                /* "output given "%?" to filename */
                dstCur += snprintf(&dst[dstCur], dstSize - dstCur,
                    "%%%%%c", logFileFormat[srcCur]);
                break;
            }
            prevPercent = 0;

        } else if (logFileFormat[srcCur] == '%') {
            prevPercent = 1;

        } else {
            dst[dstCur] = logFileFormat[srcCur];
            dstCur++;
        }
        if (dstCur >= dstSize) {
            goto overflow;
        }
    }

    /* output final "%" */
    if (prevPercent != 0/*false*/) {
        dstCur += snprintf(&dst[dstCur], dstSize - dstCur, "%%%%");
        prevPercent = 0;/* false */
    }
    if (dstCur >= dstSize) {
        goto overflow;
    }

    /* Copy the fileFormat */
    ret = ngiStrdup(fileFormat, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't allocate the storage for Log File Path.\n");
        goto error;
    }

    /* Success */
    return ret;
overflow:

    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
        "Log File Format length %d greater equal than maximum %lu.\n",
        dstCur, (unsigned long)dstSize);
    /* Error */
error:
    return NULL;
}

/**
 * Log Base: does print
 */
static int
nglLogBaseDoesPrint(
    ngLogBase_t *logBase,
    const char  *category,
    ngLogLevel_t loglevel)
{
#if 0
    static const char fName[] = "nglLogBaseDoesPrint";
#endif

    /* Check the arguments */
    assert(logBase != NULL);
    assert(category != NULL);

    return loglevel <= nglLogBaseGetLogLevel(logBase, category);
}

/**
 * Log Base: Get Log Level
 */
static ngLogLevel_t
nglLogBaseGetLogLevel(
    ngLogBase_t *logBase,
    const char  *category)
{
    ngLogCategory_t *cat = NULL;
#if 0
    static const char fName[] = "nglLogBaseGetLogLevel";
#endif

    /* Check the arguments */
    assert(logBase != NULL);
    assert(category != NULL);

    cat = logBase->nglb_arg.nglca_categories;
    if (cat != NULL) {
        for (;cat->nglc_string != NULL;cat++) {
            if (strcmp(cat->nglc_string, category) == 0) {
                return cat->nglc_level;
            }
        }
    }

    return logBase->nglb_defaultLevel;
}

/**
 * Log File: The Executable ID was resolved. Rename filename.
 */
static int
nglLogBaseExecutableIDchanged(
    ngLogBase_t *logBase,
    int newExecutableID,
    ngLog_t *log,
    int *error)
{
    int result;
    char *oldName = NULL;
    int ret = 1;
    static const char fName[] = "nglLogBaseExecutableIDchanged";

    oldName = NULL;

    if ((logBase == NULL) ||
        (logBase->nglb_executableID == NGI_LOG_EXECUTABLE_ID_NOT_APPEND) ||
        (logBase->nglb_executableID == newExecutableID)) {
        /* Do nothing */
        /* Success */
        return 1;
    }

    if ((logBase->nglb_stream == NULL) ||
        (logBase->nglb_stream == stderr)) {
        /* Do nothing */
        /* Success */
        return 1;
    }


    /* Save the old file name */
    oldName = ngiStrdup(logBase->nglb_fileName, log, error);
    if (oldName == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't allocate the storage for old file name.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    /* Set the new Executable ID */
    logBase->nglb_executableID = newExecutableID;

    /* Reset the file number counter */
    logBase->nglb_currentNo = 0;

    /* Make the File Name */
    result = nglLogBaseCreateFileName(logBase, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
    	    "Creating new file name failed.\n");
        ret = 0;
        error = NULL;
        goto finalize;
    }

    result = rename(oldName, logBase->nglb_fileName);
    if (result != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
    	    "Renaming log file \"%s\" to \"%s\" failed.\n",
            oldName, logBase->nglb_fileName);
        /* Not fail */
    }

finalize:
    if (oldName != NULL) {
        result = ngiFree(oldName, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free the storage for string.\n");
            ret = 0;
            error = NULL;
        }
        oldName = NULL;
    }
    
    return ret;
}

/**
 * Create Log Directory
 */
static int
nglLogCreateLogDirectory(
    char *fileFormat,
    int overWrite,
    ngLog_t *log,
    int *error)
{
    int result;
    int dirAvailable = 0;
    char *dir = NULL;
    char *sep = NULL;
    char *cur = NULL;
    int ret = 0;
    int hasMake = 0;
    static const char fName[] = "nglLogCreateLogDirectory";

    /* Check the arguments */
    assert(fileFormat != NULL);

    /* Is directory specified? */
    if (strchr(fileFormat, '/') == NULL) {
        /* Success */
        return 1;
    }

    /* Duplicate the dir name */
    dir = ngiStrdup(fileFormat, log, error);
    if (dir == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Can't allocate the storage for directory name.\n");
        goto finalize;
    }

    sep = strrchr(dir, '/');
    assert(sep != NULL);
    *sep = '\0';

    /* Create the Log directory */
    ngLogDebug(log, NG_LOGCAT_NINFG_LOG, fName,
        "Creating directory \"%s\".\n", dir);

    cur = dir;
    do {
        /* Skip '/' */
        for (;*cur == '/'; cur++);
        if (*cur == '\0') {
            break;
        }
    
        cur = strchr(cur, '/');
        if (cur != NULL) {
            *cur = '\0';
        }

        ngLogDebug(log, NG_LOGCAT_NINFG_LOG, fName, "mkdir(\"%s\").\n", dir);

        /* Make the directory */
        hasMake = 0;
        result = mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO);
        if (result != 0) {
            if (errno != EEXIST) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                    "mkdir \"%s\" failed : %s.\n",
                    dir, strerror(errno));
                goto finalize;
            }
            ngLogInfo(log, NG_LOGCAT_NINFG_LOG, fName,
                "mkdir \"%s\" failed : %s, continue.\n",
                dir, strerror(EEXIST));
        } else {
            hasMake = 1;
        }
        if (cur != NULL) {
            /* Restore */
            *cur = '/';
        }
    } while (cur != NULL);

    if ((hasMake == 0) && (overWrite == 0)) {
            NGI_SET_ERROR(error, NG_ERROR_EXIST);
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't overwriteDirectory \"%s\".\n"
                " overwriteDirectory disabled"
                " by Ninf-G configuration file.\n", dir);
            goto finalize;
    }

    /* Check the dir available and writable */
    result = nglLogIsDirectoryAvailable(dir, &dirAvailable, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "Checking directory \"%s\" failed.\n", dir);
        goto finalize;
    }

    if (dirAvailable == 0) {
        NGI_SET_ERROR(error, NG_ERROR_FILE);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "\"%s\" is not available.\n", dir);
        goto finalize;
    }

    ret = 1;
finalize:

    if (dir != NULL) {
        result = ngiFree(dir, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "Can't free buffer of dir.\n");
        }
        dir = NULL;
    }

    return ret;
}

/**
 * Log File: Check the directory available
 */
static int
nglLogIsDirectoryAvailable(
    char *dir,
    int *isAvailable,
    ngLog_t *log,
    int *error)
{
    struct stat stBuf;
    int result, accessFlag;
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
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                "\"%s\": %s.\n", dir, strerror(errno));
            return 0;
        }
    } else {
        if (!(stBuf.st_mode & S_IFDIR)) {
            ngLogInfo(log, NG_LOGCAT_NINFG_LOG, fName,
                "\"%s\" is not the directory.\n", dir);
            return 0;
        }
        *isAvailable = 1;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_LOG, fName,
        "The directory \"%s\" is%s available by stat() check.\n",
        dir, (*isAvailable == 0 ? " not" : ""));

    /* Check the permission */
    if (*isAvailable != 0) {
        accessFlag = W_OK | X_OK;
        result = access(dir, accessFlag);
        if (result != 0) {
            if (errno == ENOENT) {
                *isAvailable = 0;
            } else {
                NGI_SET_ERROR(error, NG_ERROR_FILE);
                ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
                    "\"%s\": %s.\n", dir, strerror(errno));
                return 0;
            }
        } else {
            *isAvailable = 1;
        }
        ngLogDebug(log, NG_LOGCAT_NINFG_LOG, fName,
            "The directory \"%s\" is%s available by access() check.\n",
            dir, (*isAvailable == 0 ? " not" : ""));
    }

    /* Success */
    return 1;
}

/**
 * Log level to string
 */
static const char *
nglLogLevelToString(
    ngLogLevel_t loglevel)
{
#if 0
    static const char fName[] = "nglLogLevelToString";
#endif

    if (NGL_LOG_LEVEL_IS_VALID(loglevel)) {
        return nglLogLevelStringTable[loglevel];
    }
    return "Unknown";
}

/**
 * Get the string date and time.
 */
static int
nglTimeValToString(
    struct timeval *tv,
    char *buf,
    size_t size,
    int *error)
{
    struct tm ltime;
    time_t sec;
    long msec;
    long usec;
    int result;

    assert(tv != NULL);
    assert(buf != NULL);
    assert(size >= 32);/* 32 is string length.*/

    sec = tv->tv_sec;

    result = ngiLocalTime(sec, &ltime, NULL, error);
    if (result == 0) {
        /* TODO: Error Handling */
        return 0;
    }

    msec = tv->tv_usec/1000;
    usec = tv->tv_usec%1000;

    /* Append the msec */
    /* Requires buffer whose length is 32. */
    result = snprintf(buf, size,
             "%4d/%02d/%02d %2d:%02d:%02d %3ldms %3ldus",
           /*  4 1  2 1  2 1 2 1  2 1  2 1 3   3  3   2 1(\0) = 32 */
             ltime.tm_year + 1900, ltime.tm_mon+1, ltime.tm_mday,
             ltime.tm_hour, ltime.tm_min, ltime.tm_sec,
             msec, usec);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        return 0;
    }
    assert(result < size);

    return 1;
}

/**
 * Get string of date and time.
 */
static int
nglGetStringOfDateAndTime(
    char *buf,
    size_t size,
    int *error)
{
    int result;
    struct timeval tv;
    static const char fName[] = "nglGetStringOfDateAndTime";

    assert(buf != NULL);
    assert(size > 0);

    /* Get current time */
    result = gettimeofday(&tv, NULL);
    if (result != 0) {
        /* TODO: Error Handling */
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        fprintf(stderr, "%s: gettimeofday failed: %s.\n",
            fName, strerror(errno));
        return 0;
    }
    result = nglTimeValToString(&tv, buf, size, error);
    if (result == 0) {
        /* TODO: Error Handling */
        fprintf(stderr, "%s\n", "ERROR");
        return 0;
    }

    return 1;
}

/**
 * Log: Current Time String Generate 
 * This function is for log filename
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
    time_t sec;
    struct tm tm;
    struct timeval tv;
    static const char fName[] = "nglLogCurrentTimeStringGenerate";

    /* Check the arguments */
    assert(writeSize != NULL);

    /* Get the current time */
    result = gettimeofday(&tv, NULL);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
        ngLogError(log, NG_LOGCAT_NINFG_LOG, fName,
            "gettimeofday failed: %s.\n", strerror(errno));
        return 0;
    }

    sec = tv.tv_sec;

    result = ngiLocalTime(sec, &tm, log, error);
    if (result == 0) {
        return 0;
    }

    *writeSize = snprintf(buf, size,
        "%02d%02d%02d-%02d%02d%02d-%03ld",
        /* 2 + 2 + 2+1+ 2 + 2 + 2+1+ */
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec,
        (long)tv.tv_usec / 1000);

    /* Success */
    return 1;
}

/**
 * Log Information: Get Default loglevel
 */
ngLogLevel_t
ngLogGetLogLevelFromEnv()
{
    long tmp;
    char *end = NULL;
    char *logLevelString = NULL;
    int i;
    ngLogLevel_t level = NG_LOG_LEVEL_ERROR;/* Default */

    /* Get default loglevel from environment variable */
    logLevelString = getenv(NGI_ENVIRONMENT_LOG_LEVEL);
    if (logLevelString == NULL) {
        goto end;
    }

    /* Try decode by string */
    for (i = 0; i < NGI_NELEMENTS(nglLogLevelStringTable); i++) {
        if (strcmp(logLevelString, nglLogLevelStringTable[i]) == 0) {
            level = (ngLogLevel_t)i;
            goto end;
        }
    }

    /* Try decode by number */
    tmp = strtol(logLevelString, &end, 10);
    if ((tmp < 0) && (logLevelString == end) && (*end != '\0')) {
        goto end;
    }
    if (tmp >= NGI_NELEMENTS(nglLogLevelStringTable)) {
        tmp = NGI_NELEMENTS(nglLogLevelStringTable) - 1;
    }
    level = (ngLogLevel_t)tmp;

    /* Success */
end:
    return level;
}

