/*
 * $RCSfile: ngLogInformation.c,v $ $Revision: 1.5 $ $Date: 2008/02/07 06:22:03 $
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
 * Module of log information in configuration file.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngLogInformation.c,v $ $Revision: 1.5 $ $Date: 2008/02/07 06:22:03 $")

/**
 * Log Information: Initialize.
 */
int
ngiLogInformationInitialize(
    ngLogInformation_t *logInfo,
    ngLog_t *log,
    int *error)
{
    ngiLogInformationInitializeMember(logInfo);

    return 1;
}

/**
 * Log Information: Finalize.
 */
int
ngiLogInformationFinalize(
    ngLogInformation_t *logInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiLogInformationFinalize";

    result = ngiFree(logInfo->ngli_filePath, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for string.\n");
        error = NULL;
        ret = 0;
    }

    result = ngiFree(logInfo->ngli_suffix, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for string.\n");
        error = NULL;
        ret = 0;
    }
    ngiLogInformationInitializeMember(logInfo);

    return ret;
}

/**
 * Log Information: Set default value.
 * Warning: If logInfo has valid string, it leaks.
 */
int
ngiLogInformationSetDefault(
    ngLogInformation_t *logInfo,
    ngLog_t *log,
    int *error)
{
    logInfo->ngli_filePath     = NULL;
    logInfo->ngli_maxFileSize  = 0;
    logInfo->ngli_nFiles       = 1;
    logInfo->ngli_overWriteDir = 0;/* false */
    logInfo->ngli_suffix       = NULL;

    return 1;
}

/**
 * Log Information: Copy
 */
int
ngiLogInformationCopy(
    ngLogInformation_t *src, 
    ngLogInformation_t *dest,
    ngLog_t *log,
    int *error)
{
    char *filePath = NULL;
    char *suffix = NULL;
    int result;
    static const char fName[] = "ngiLogInformationCopy";

    if (src->ngli_filePath != NULL) {
        filePath = ngiStrdup(src->ngli_filePath, log, error);
        if (filePath == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate storage for string.\n");
            goto error;
        }
    }

    if (src->ngli_suffix != NULL) {
        suffix = ngiStrdup(src->ngli_suffix, log, error);
        if (suffix == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't allocate storage for string.\n");
            goto error;
        }
    }

    if (src->ngli_filePath != dest->ngli_filePath) {
        result = ngiFree(dest->ngli_filePath, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't deallocate storage for string.\n");
            /* Through */
        }
    }

    if (src->ngli_suffix != dest->ngli_suffix) {
        result = ngiFree(dest->ngli_suffix, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't deallocate storage for string.\n");
            /* Through */
        }
    }

    dest->ngli_filePath     = filePath;
    dest->ngli_maxFileSize  = src->ngli_maxFileSize;
    dest->ngli_nFiles       = src->ngli_nFiles;
    dest->ngli_overWriteDir = src->ngli_overWriteDir;
    dest->ngli_suffix       = suffix;

    return 1;

error:
    result = ngiFree(filePath, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for string.\n");
        /* Through */
    }
    filePath = NULL;


    result = ngiFree(suffix, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't deallocate storage for string.\n");
        /* Through */
    }
    suffix = NULL;

    return 0;
}

/**
 * Log Information: Zero clear.
 * Warning: If logInfo has valid string, it leaks.
 */
void 
ngiLogInformationInitializeMember(
    ngLogInformation_t *logInfo)
{
    logInfo->ngli_filePath     = NULL;
    logInfo->ngli_maxFileSize  = 0;
    logInfo->ngli_nFiles       = 0;
    logInfo->ngli_overWriteDir = 0;/* false */
    logInfo->ngli_suffix       = NULL;

    return;
}

/**
 * Log Level Information: Initialize.
 */
int
ngiLogLevelInformationInitialize(
    ngLogLevelInformation_t *logLevels,
    ngLog_t *log,
    int *error)
{
    ngiLogLevelInformationInitializeMember(logLevels);

    return 1;
}

/**
 * Log Level Information: Finalize.
 */
int
ngiLogLevelInformationFinalize(
    ngLogLevelInformation_t *logLevels,
    ngLog_t *log,
    int *error)
{
    ngiLogLevelInformationInitializeMember(logLevels);

    return 1;
}

/**
 * Log Level Information: Set default value.
 */
int
ngiLogLevelInformationSetDefault(
    ngLogLevelInformation_t *logLevels,
    ngLog_t *log,
    int *error)
{
    ngLogLevel_t level;

    level = ngLogGetLogLevelFromEnv();

    logLevels->nglli_level         = level;
    logLevels->nglli_ninfgProtocol = level;
    logLevels->nglli_ninfgInternal = level;
    logLevels->nglli_grpc          = level;

    return 1;
}

/**
 * Log Level Information: Copy
 */
int
ngiLogLevelInformationCopy(
    ngLogLevelInformation_t *src,
    ngLogLevelInformation_t *dest,
    ngLog_t *log,
    int *error)
{
    dest->nglli_level         = src->nglli_level;
    dest->nglli_ninfgProtocol = src->nglli_ninfgProtocol;
    dest->nglli_ninfgInternal = src->nglli_ninfgInternal;
    dest->nglli_grpc          = src->nglli_grpc;

    return 1;
}

/**
 * Log Level Information: Zero clear.
 * Warning: If logInfo has valid string, it leaks.
 */
void 
ngiLogLevelInformationInitializeMember(
    ngLogLevelInformation_t *logLevels)
{
    logLevels->nglli_level         = NG_LOG_LEVEL_OFF;
    logLevels->nglli_ninfgProtocol = NG_LOG_LEVEL_OFF;
    logLevels->nglli_ninfgInternal = NG_LOG_LEVEL_OFF;
    logLevels->nglli_grpc          = NG_LOG_LEVEL_OFF;

    return;
}

/**
 * Log: Construct from ngLogInformation_t and ngLogLevelInformation_t.
 */
ngLog_t *
ngiLogConstructFromConfig(
    ngLogInformation_t *logInfo,
    ngLogLevelInformation_t *logLevels,
    const char *appname,
    int executableID,
    ngLog_t *log,
    int *error)
{
    int result;
    int logCargInitialized = 0;
    int i = 0;
    ngLog_t *ret = NULL;
    ngLogConstructArgument_t logCarg;
    static const char fName[] = "ngiLogConstructFromConfig";
    
    result = ngLogConstructArgumentInitialize(&logCarg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize log construct argument.\n"); 
        error = NULL;
        goto finalize;
    }
    logCargInitialized = 1;

    if (logInfo->ngli_filePath == NULL) {
        logCarg.nglca_output = NG_LOG_STDERR;
    } else {
        logCarg.nglca_output       = NG_LOG_FILE;
        logCarg.nglca_nFiles       = logInfo->ngli_nFiles;
        logCarg.nglca_maxFileSize  = logInfo->ngli_maxFileSize;
        logCarg.nglca_overWriteDir = logInfo->ngli_overWriteDir;
        logCarg.nglca_appending    = 0;/* false */

        logCarg.nglca_filePath =
            ngiStrdup(logInfo->ngli_filePath, log, error);
        if (logCarg.nglca_filePath == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy string.\n"); 
            error = NULL;
            goto finalize;
        }
        if (logInfo->ngli_suffix != NULL) {
            logCarg.nglca_suffix =
                ngiStrdup(logInfo->ngli_suffix, log, error);
            if (logCarg.nglca_suffix == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy string.\n"); 
                error = NULL;
                goto finalize;
            }
        }
    }

#define NGL_NLOGCATEGORIES 4
    /* Categories */
    logCarg.nglca_categories = NGI_ALLOCATE_ARRAY(
        ngLogCategory_t, NGL_NLOGCATEGORIES + 1, log, error);
    if (logCarg.nglca_categories == NULL) {
        ret = 0;
        error = NULL;
        goto finalize;
    }
    logCarg.nglca_categories[i].nglc_string = NG_LOGCAT_NINFG_PROTOCOL;
    logCarg.nglca_categories[i].nglc_level = logLevels->nglli_ninfgProtocol;
    i++;

    logCarg.nglca_categories[i].nglc_string = NG_LOGCAT_NINFG_PURE;
    logCarg.nglca_categories[i].nglc_level = logLevels->nglli_ninfgInternal;
    i++;

    logCarg.nglca_categories[i].nglc_string = NG_LOGCAT_NINFG_GRPC;
    logCarg.nglca_categories[i].nglc_level = logLevels->nglli_grpc;
    i++;

    logCarg.nglca_categories[i].nglc_string = NG_LOGCAT_DEFAULT;
    logCarg.nglca_categories[i].nglc_level = logLevels->nglli_level;
    i++;

    logCarg.nglca_categories[i].nglc_string = NULL;
    logCarg.nglca_categories[i].nglc_level = NG_LOG_LEVEL_OFF;
    assert(i == NGL_NLOGCATEGORIES);
#undef NGL_NLOGCATEGORIES

    ret = ngLogConstructForExecutable(
        appname, executableID, &logCarg, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't construct the Log Manager.\n"); 
        error = NULL;
        goto finalize;
    }

finalize:
    if (logCargInitialized != 0) {
        result = ngLogConstructArgumentFinalize(&logCarg, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize log construct argument.\n"); 
            error = NULL;
        }
        logCargInitialized = 0;
    }
    return ret;
}
