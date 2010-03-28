/*
 * $RCSfile: ngisLog.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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

#include "ngisLog.h"
#include "ngisUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngisLog.c,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $")

#define NGISL_LOG_OPEN_MODE "a"

static char *ngislLevelStrings[] = {
    "OFF",
    "ABORT",
    "ERROR",
    "WARNING",
    "DEBUG",
};

typedef struct ngislLogModule_s {
    FILE           *nglm_fp;
    ngisLogLevel_t  nglm_level;
    ngisLog_t       nglm_log;
} ngislLogModule_t;

static ngislLogModule_t ngislLogModule;
static int ngislLogModuleInitialized = 0;

/**
 * Log Module: Initialize
 * This function open log file.
 */
int
ngisLogInitializeModule(
    const char    *logFileName,
    ngisLogLevel_t level)
{
#if 0
    static const char fName[] = "ngisLogInitializeModule";
#endif

    if (ngislLogModuleInitialized != 0) {
        /* No error output */
        return 0;
    }

    ngislLogModule.nglm_fp    = NULL;
    ngislLogModule.nglm_level = NGIS_LOG_LEVEL_OFF;
    ngislLogModule.nglm_log.ngl_moduleName = NULL;

    if (logFileName != NULL) {
        ngislLogModule.nglm_fp = fopen(logFileName, NGISL_LOG_OPEN_MODE);
        if (ngislLogModule.nglm_fp == NULL) {
            /* No error output */
            return 0;
        }
        setvbuf(ngislLogModule.nglm_fp, NULL, _IONBF, 0);
    }
    ngislLogModule.nglm_level = level;
    ngislLogModuleInitialized = 1;
    
    /* Success */
    return 1;
}

/**
 * Log Module: Initialize
 * This function close log file.
 */
int
ngisLogFinalizeModule()
{
#if 0
    static const char fName[] = "ngisLogFinalizeModule";
#endif
    if (ngislLogModuleInitialized == 0) {
        return 0;
    }

    if (ngislLogModule.nglm_fp != NULL) {
        fclose(ngislLogModule.nglm_fp);
        /* IGNORE ERROR */
    }
    ngislLogModule.nglm_fp    = NULL;
    ngislLogModule.nglm_level = NGIS_LOG_LEVEL_OFF;
    ngislLogModule.nglm_log.ngl_moduleName = NULL;

    ngislLogModuleInitialized = 0;
    
    /* Success */
    return 1;
}

ngisLog_t *
ngisLogCreate(
    const char *moduleName)
{
    return ngisLogCreatef("%s", moduleName);
}

ngisLog_t *
ngisLogCreatef(
    const char *format,...)
{
    ngisLog_t *new = NULL;
    char *copyString = NULL;
    va_list ap;
    static const char fName[] = "ngisLogCreatef";

    if (ngislLogModuleInitialized == 0) {
        return NULL;
    }

    new = NGIS_ALLOC(ngisLog_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName, "Can't allocate storage for log.\n");
        goto error;
    }

    va_start(ap, format);
    copyString = ngisStrdupVprintf(format, ap);
    va_end(ap);
    if (copyString == NULL) {
        ngisErrorPrint(NULL, fName, "Can't copy module name.\n");
        goto error;
    }
    new->ngl_moduleName = copyString;

    return new;
error:
    NGIS_NULL_CHECK_AND_FREE(copyString);
    NGIS_NULL_CHECK_AND_FREE(new);

    return NULL;
}

int
ngisLogDestroy(
    ngisLog_t *log)
{
    NGIS_ASSERT(log != NULL);

    if (ngislLogModuleInitialized == 0) {
        return 0;
    }

    NGIS_NULL_CHECK_AND_FREE(log->ngl_moduleName);
    NGIS_NULL_CHECK_AND_FREE(log);

    return 1;
}

int
ngisLogPrintf(
    ngisLog_t *log,
    ngisLogLevel_t level,
    const char *fName,
    const char *format, ...)
{
    va_list ap;
    int result;

    va_start(ap, format);

    result = ngisLogVprintf(
        log, level, fName, format, ap);

    va_end(ap);

    return result;
}

int 
ngisLogDumpFile(
    ngisLog_t *log,
    ngisLogLevel_t level,
    const char *fName,
    const char *filename)
{
    FILE *fp = NULL;
    int c;
    int result;

    NGIS_ASSERT_STRING(filename);

    if (ngislLogModuleInitialized == 0) {
        return 0;
    }

    if (ngislLogModule.nglm_fp == NULL) {
        /* Do nothing */
        return 1;
    }

    if (level > ngislLogModule.nglm_level) {
        /* Do nothing */
        return 1;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        ngisLogPrintf(log, level, fName,
            "FILENAME:%s: %s\n", filename, strerror(errno));
        return 0;
    }

    ngisLogPrintf(log, level, fName, "FILENAME:%s\n", filename);

    while ((c = fgetc(fp)) != EOF) {
        fputc(c, ngislLogModule.nglm_fp);
    }

    result = fclose(fp);
    if (result == EOF) {
        return 0;
    }

    return 1;
}

int
ngisLogVprintf(
    ngisLog_t *log,
    ngisLogLevel_t level,
    const char *fName,
    const char *format,
    va_list ap)
{
    struct tm *tmResult;
    time_t t;
#if 0
    static const char fName[] = "ngisLogVprintf";
#endif
    
    NGIS_ASSERT(level == NGIS_LOG_LEVEL_ABORT   ||
                level == NGIS_LOG_LEVEL_DEBUG   ||
                level == NGIS_LOG_LEVEL_WARNING ||
                level == NGIS_LOG_LEVEL_ERROR);

    /* Check the arguments */
    NGIS_ASSERT(format != NULL);
    
    if (ngislLogModuleInitialized == 0) {
        return 0;
    }

    if (ngislLogModule.nglm_fp == NULL) {
        /* Do nothing */
        return 1;
    }

    if (level > ngislLogModule.nglm_level) {
        /* Do nothing */
        return 1;
    }
    
    /* Make the time */
    t = time(NULL);
    assert(t >= 0);
    /* Doesn't use NGIS_ASSERT to avoid infinite recursive call */
    
    tmResult = localtime(&t);
    if (tmResult == NULL) {
        /* Failed */
        fprintf(ngislLogModule.nglm_fp,
            "%ld: " , (long)t);
    } else {
        fprintf(ngislLogModule.nglm_fp,
            "%d/%d/%d %02d:%02d:%02d :",
            tmResult->tm_year + 1900, tmResult->tm_mon + 1, tmResult->tm_mday,
            tmResult->tm_hour, tmResult->tm_min, tmResult->tm_sec);
    }

    /* Module */
    if ((log != NULL) && (log->ngl_moduleName != NULL)) {
        fprintf(ngislLogModule.nglm_fp, "%s:", log->ngl_moduleName);
    } 

    /* Function */
    if (fName != NULL) {
        fprintf(ngislLogModule.nglm_fp, "%s:", fName);
    }
    
    /* Level */
    fprintf(ngislLogModule.nglm_fp, "[%s]: ", ngislLevelStrings[level]);

    /* Message */
    vfprintf(ngislLogModule.nglm_fp, format, ap);

    fflush(ngislLogModule.nglm_fp);

    /* Success */
    return 1;
}

#define NGISL_LOG_FUNC_DEFINE(funcName, logLevel) \
int                                               \
funcName(                                         \
    ngisLog_t *log,                               \
    const char *fName,                            \
    const char *format, ...)                      \
{                                                 \
    va_list ap;                                   \
    int result;                                   \
                                                  \
    va_start(ap, format);                         \
                                                  \
    result = ngisLogVprintf(                      \
        log, logLevel, fName, format, ap);        \
                                                  \
    va_end(ap);                                   \
                                                  \
    return result;                                \
}                                                 

NGISL_LOG_FUNC_DEFINE(ngisAbortPrint,   NGIS_LOG_LEVEL_ABORT)
NGISL_LOG_FUNC_DEFINE(ngisErrorPrint,   NGIS_LOG_LEVEL_ERROR)
NGISL_LOG_FUNC_DEFINE(ngisWarningPrint, NGIS_LOG_LEVEL_WARNING)
NGISL_LOG_FUNC_DEFINE(ngisDebugPrint,   NGIS_LOG_LEVEL_DEBUG)
