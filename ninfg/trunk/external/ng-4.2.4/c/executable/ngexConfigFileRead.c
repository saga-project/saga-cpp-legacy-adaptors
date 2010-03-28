#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexConfigFileRead.c,v $ $Revision: 1.27 $ $Date: 2007/07/09 07:46:59 $";
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
 * Module of remote executable configuration file reading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#include "ngEx.h"
#include "ngExecutableInternal.h"
#include "ngConfigFile.h"

static int ngexIsSetIPtrue = 1;
#define NGEX_ISSET_IP_TRUE (&ngexIsSetIPtrue)

/**
 * function table for each configfile entity and their function
 */
typedef struct ngexlLocalMachineInfoPair_s {
    ngexiLocalMachineInformation_t nelmip_entities;
    ngexiLocalMachineInformation_t nelmip_isSet;
} ngexlLocalMachineInfoPair_t;

typedef struct ngexlReadingState_s {
    ngexlLocalMachineInfoPair_t ners_lmInfo;
} ngexlReadingState_t;


#define NGEXL_CONFIG_ATTRFUNC_ARG \
    (char *attrName, ngiTokenInfo_t *token, \
    ngexlReadingState_t *readingState, ngexiContext_t *context, \
    int *error)

typedef int (*ngexlAttrFunc_t)NGEXL_CONFIG_ATTRFUNC_ARG;

typedef struct ngexlAttrFuncTable_s {
    char *naft_attrName;
    ngexlAttrFunc_t naft_func;
} ngexlAttrFuncTable_t;

static int ngexlAttrFunc_tmpdir               NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_loglevel             NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_loglevel_gt          NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_loglevel_ngprot      NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_loglevel_ngi         NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_log_filePath         NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_log_suffix           NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_log_nFiles           NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_log_maxFileSize      NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_log_overWrite        NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_enable       NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_filePath     NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_suffix       NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_nFiles       NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_maxFileSize  NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_commLog_overWrite    NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_save_stdout          NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_save_stderr          NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_handling_signals     NGEXL_CONFIG_ATTRFUNC_ARG;
static int ngexlAttrFunc_continueOnError      NGEXL_CONFIG_ATTRFUNC_ARG;


/* table data definition below */
ngexlAttrFuncTable_t execAttrs[] = {
    {"tmp_dir",                 ngexlAttrFunc_tmpdir},

    {"loglevel",                ngexlAttrFunc_loglevel},
    {"loglevel_globusToolkit",  ngexlAttrFunc_loglevel_gt},
    {"loglevel_ninfgProtocol",  ngexlAttrFunc_loglevel_ngprot},
    {"loglevel_ninfgInternal",  ngexlAttrFunc_loglevel_ngi},
    {"log_filePath",            ngexlAttrFunc_log_filePath},
    {"log_suffix",              ngexlAttrFunc_log_suffix},
    {"log_nFiles",              ngexlAttrFunc_log_nFiles},
    {"log_maxFileSize",         ngexlAttrFunc_log_maxFileSize},
    {"log_overwriteDirectory",  ngexlAttrFunc_log_overWrite},

    {"commLog_enable",          ngexlAttrFunc_commLog_enable},
    {"commLog_filePath",        ngexlAttrFunc_commLog_filePath},
    {"commLog_suffix",          ngexlAttrFunc_commLog_suffix},
    {"commLog_nFiles",          ngexlAttrFunc_commLog_nFiles},
    {"commLog_maxFileSize",     ngexlAttrFunc_commLog_maxFileSize},
    {"commLog_overwriteDirectory", ngexlAttrFunc_commLog_overWrite},

    {"save_stdout",             ngexlAttrFunc_save_stdout},
    {"save_stderr",             ngexlAttrFunc_save_stderr},
    {"handling_signals",        ngexlAttrFunc_handling_signals},
    {"continue_on_error",       ngexlAttrFunc_continueOnError},

    {NULL, NULL}
};


/**
 * Conversion table for size
 */
extern ngiUnitConvTable_t ngiSizeUnitTable[];

/* prototypes */
static int ngexlConfigFileParse(ngexiContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo, int *error);
static int ngexlAttrFuncInitialize(ngexiContext_t *context,
    ngexlReadingState_t **readingState, int *error);
static int ngexlAttrFuncFinalize(ngexiContext_t *context,
    ngexlReadingState_t *readingState, int *error);
static int ngexlReadingStateConstruct(ngexiContext_t *context,
    ngexlReadingState_t **readingState, int *error);
static int ngexlReadingStateDestruct(ngexiContext_t *context,
    ngexlReadingState_t *readingState, int *error);
static ngexlAttrFunc_t ngexlGetAttrFuncPtr(
    ngexlAttrFuncTable_t *attrTable, char *attrName);

static int ngexlLocalMachineInfoPairInitialize(ngexiContext_t *context,
    ngexlLocalMachineInfoPair_t *lmInfoPair, int *error);
static int ngexlLocalMachineInfoPairFinalize(ngexiContext_t *context,
    ngexlLocalMachineInfoPair_t *lmInfoPair, int *error);
static int ngexlLocalMachineInformationSetPair(
    ngexiLocalMachineInformation_t *dst, ngexlLocalMachineInfoPair_t *src,
    ngLog_t *log, int *error);
static int ngexlLocalMachineInformationSetDefault(
    ngexiLocalMachineInformation_t *lmInfo, ngLog_t *log, int *error);

/**
 * Functions
 */

/**
 * Read Configuration File
 */
int 
ngexiConfigFileRead(
    ngexiContext_t *context,
    char *sysConfig,
    char *userConfig,
    int *error)
{
    static const char fName[] = "ngexiConfigFileRead";
    ngLog_t *log;
    ngiTokenReadInfo_t *tokenReadInfo;
    int result, subError;

    /* Check the arguments */
    if (context == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    log = context->ngc_log;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    result = ngexlLocalMachineInformationSetDefault(
        &context->ngc_lmInfo, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't set Executable Local Machine Information\n",
            fName);
        return 0;
    }

    /* Read system executable configuration file if available. */
    if (sysConfig != NULL) {
	result = ngiConfigFileOpen(
            sysConfig, &tokenReadInfo, 0, log, &subError);
	if (result == 1) {
	    result = ngexlConfigFileParse(context, tokenReadInfo, error);
	    if (result != 1) {
		ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
		    "%s: Parsing executable configuration file %s fail.\n",
		    fName, sysConfig);
		return 0;
	    }

	    result = ngiConfigFileClose(tokenReadInfo, log, error);
	    if (result != 1) {
		ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
		    "%s: Closing executable configuration file %s fail.\n",
		    fName, sysConfig);
		return 0;
	    }
	} else {
	    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
	    ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL, 
		"%s: No executable configuration file \"%s\"."
                " continue anyway.\n",
		fName, sysConfig);
	}
    }

    /* Read user executable configuration file if available. */
    if (userConfig != NULL) {
	result = ngiConfigFileOpen(
            userConfig, &tokenReadInfo, 0, log, &subError);
	if (result == 1) {
	    result = ngexlConfigFileParse(context, tokenReadInfo, error);
	    if (result != 1) {
		ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
		    "%s: Parsing executable configuration file %s fail.\n",
		    fName, userConfig);
		return 0;
	    }

	    result = ngiConfigFileClose(tokenReadInfo, log, error);
	    if (result != 1) {
		ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
		    "%s: Closing executable configuration file %s fail.\n",
		    fName, userConfig);
		return 0;
	    }
	} else {
	    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
	    ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL, 
		"%s: No executable configuration file \"%s\"."
                " continue anyway.\n",
		fName, userConfig);
	}
    }

    /* Success */
    return 1;
}

static int 
ngexlConfigFileParse(
    ngexiContext_t *context,
    ngiTokenReadInfo_t *tokenReadInfo,
    int *error)
{
    static const char fName[] = "ngexlConfigFileParse";
    ngiTokenInfo_t tokenEntity, *token;
    ngiTokenInfo_t argTokenEntity, *argToken;
    ngexlReadingState_t *readingState;
    char attrName[NGI_CONFIG_LINE_MAX];
    ngexlAttrFuncTable_t *attrTable;
    ngexlAttrFunc_t attrfnc;
    ngLog_t *log;
    int result;

    token = &tokenEntity;
    argToken = &argTokenEntity;
    attrTable = execAttrs;
    log = context->ngc_log;

    result = ngexlAttrFuncInitialize(context, &readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to initialize configfile parsing data structure.\n",
            fName);
        return 0;
    }

    while((result = ngiGetToken(
        tokenReadInfo, token, NGI_GETTOKEN_TOKEN, log, error))
            == NGI_GET_TOKEN_SUCCESS) {
        if (token->nti_type != NGI_TOKEN_ATTR) {
            ngiConfigFileSyntaxError(log, token,
                "Not attribute in executable config file",
                fName, NULL, token->nti_tokenStr, error);
            return 0;
        }

        strncpy(attrName, token->nti_tokenStr, NGI_CONFIG_LINE_MAX);
        
        attrfnc = ngexlGetAttrFuncPtr(attrTable, attrName);
        if (attrfnc == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngiConfigFileSyntaxError(log, token,
                "No such attribute", fName, NULL, attrName, error);
            return 0;
        }

        /* get attribute value (attribute argument) */
        result = ngiGetToken(tokenReadInfo, argToken,
            NGI_GETTOKEN_ARGS, log, error);
        if ((result != NGI_GET_TOKEN_SUCCESS) ||
            (argToken->nti_type != NGI_TOKEN_ATTR)) {
            ngiConfigFileSyntaxError(log, argToken,
                "No argument for the attribute",
                fName, attrName, NULL, error);
            return 0;
        }

        result = (*attrfnc)(attrName, argToken, readingState,
           context, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: attribute \"%s\" process fail.\n",
                fName, attrName);
            return 0;
        }
    }
    if (result != NGI_GET_TOKEN_EOF) {
        ngiConfigFileSyntaxError(log, token,
            "Invalid word", fName, NULL, token->nti_tokenStr, error);
        return 0;
    }

    result = ngexlAttrFuncFinalize(context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to finalize configfile parsing data structure.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * This function gets function pointer corresponding keyword attrName
 * in AttrTable
 */
static ngexlAttrFunc_t
ngexlGetAttrFuncPtr(
    ngexlAttrFuncTable_t *attrTable,
    char *attrName)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int i;

    cmp = (NGI_ATTR_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    for (i = 0; attrTable[i].naft_attrName != NULL; i++) {
        if((*cmp)(attrTable[i].naft_attrName, attrName,
             NGI_CONFIG_LINE_MAX) == 0) {
            /* Success */
            return attrTable[i].naft_func;
        }
    }

    /* Failed */
    return NULL;
}


/*****************************************************************/
/**
 * attribute functions below
 */

/**
 * Initialize
 */
static int 
ngexlAttrFuncInitialize(
    ngexiContext_t *context,
    ngexlReadingState_t **readingState,
    int *error)
{
    static const char fName[] = "ngexlAttrFuncInitialize";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    if ((context == NULL) || (readingState == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Construct the ReadingState */
    result = ngexlReadingStateConstruct(context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to construct configfile parsing data structure.\n",
             fName);
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * Finalize
 */
static int 
ngexlAttrFuncFinalize(
    ngexiContext_t *context,
    ngexlReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngexlAttrFuncFinalize";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    if ((context == NULL) || (readingState == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Register LocalMachineInformation to Context */
    result = ngexlLocalMachineInformationSetPair(
        &context->ngc_lmInfo, &readingState->ners_lmInfo, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Setting Executable Local Machine Information failed.\n",
            fName);
        return 0;
    }

    /* Destruct the ReadingState */
    result = ngexlReadingStateDestruct(context, readingState, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Failed to destruct configfile parsing data structure.\n",
             fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Construct
 */
static int 
ngexlReadingStateConstruct(
    ngexiContext_t *context,
    ngexlReadingState_t **readingState,
    int *error)
{
    static const char fName[] = "ngexlReadingStateConstruct";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    if ((context == NULL) || (readingState == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Allocate */
    *readingState = (ngexlReadingState_t *)
        globus_libc_malloc(sizeof(ngexlReadingState_t));
    if (*readingState == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for string.\n", fName);
        return 0;
    }

    /* Initialize */
    result = ngexlLocalMachineInfoPairInitialize(
        context, &(*readingState)->ners_lmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize Executable Local Machine Information.\n",
            fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Destruct
 */
static int 
ngexlReadingStateDestruct(
    ngexiContext_t *context,
    ngexlReadingState_t *readingState,
    int *error)
{
    static const char fName[] = "ngexlReadingStateDestruct";
    ngLog_t *log;
    int result;

    /* Check the arguments */
    if ((context == NULL) || (readingState == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    log = context->ngc_log;

    /* Finalize */
    result = ngexlLocalMachineInfoPairFinalize(
        context, &readingState->ners_lmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize Executable Local Machine Information.\n",
            fName);
        return 0;
    }

    globus_libc_free(readingState);

    return 1;
}

#define NGEXL_AF_MEMBER_SET_STR(member, arg, entity, isset) \
    { \
        if ((isset).member != NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity).member != NULL) { \
            globus_libc_free((entity).member); \
        } \
         \
        (entity).member = ngiReadStringFromArg((arg)); \
        if ((entity).member == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset).member = NGCL_ISSET_S_TRUE; \
    }

#define NGEXL_AF_MEMBER_SET_BOOL(member, arg, entity, isset) \
    { \
        char *true_false; \
        int result; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        true_false = ngiReadStringFromArg((arg)); \
        if (true_false == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg(true_false, NGI_ATTR_ARG_CASE_SENSITIVE, \
                                                  2, "true", "false"); \
        if ((result <= 0) || (result > 2)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(true_false); \
            return 0; \
        } \
        (entity).member = (result == 1 ? 1 : 0); \
        (isset).member = NGCL_ISSET_I_TRUE; \
        globus_libc_free(true_false); \
    }

#define NGEXL_AF_MEMBER_SET_INT(member, minValue, arg, entity, isset) \
    { \
        int resultValue; \
        int result; \
         \
        resultValue = 0; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadIntFromArg((arg), &resultValue); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity).member = resultValue; \
        (isset).member = NGCL_ISSET_I_TRUE; \
    }

#define NGEXL_AF_MEMBER_SET_LOGLEVEL(member, arg, entity, isset) \
    { \
        char *levelStr; \
        int result; \
         \
        if ((isset).member != (ngLogLevel_t) 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        levelStr = ngiReadStringFromArg((arg)); \
        if (levelStr == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                 fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg(levelStr, NGI_ATTR_ARG_CASE_SENSITIVE, \
            12, "0", "1", "2", "3", "4", "5", \
            "Off", "Fatal", "Error", "Warning", "Information", "Debug"); \
         \
        if ((result >= 1) && (result <= 6)) { \
            (entity).member = (ngLogLevel_t) (result - 1); \
        } else if ((result >= 7) && (result <= 12)) { \
            (entity).member = (ngLogLevel_t) (result - 7); \
        } else { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                 fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(levelStr); \
            return 0; \
        } \
        (isset).member = (ngLogLevel_t) NGCL_ISSET_I_TRUE; \
        globus_libc_free(levelStr); \
    }

#define NGEXL_AF_MEMBER_SET_UNIT( \
    member, minValue, arg, entity, isset, unitTable) \
    { \
        char *unitString; \
        int resultValue; \
        int result; \
         \
        resultValue = 0; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        unitString = ngiReadStringFromArg((arg)); \
        if (unitString == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadUnitNumFromArg(unitString, &resultValue, \
             unitTable, NGI_ATTR_ARG_CASE_SENSITIVE); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(unitString); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            globus_libc_free(unitString); \
            return 0; \
        } \
        (entity).member = resultValue; \
        (isset).member = NGCL_ISSET_I_TRUE; \
        globus_libc_free(unitString); \
    }

#define NGEXL_AFS_MEMBER_SET_STR(member, arg) \
         NGEXL_AF_MEMBER_SET_STR(member, arg, \
             readingState->ners_lmInfo.nelmip_entities, \
             readingState->ners_lmInfo.nelmip_isSet)

#define NGEXL_AFS_MEMBER_SET_INT(member, minValue, arg) \
         NGEXL_AF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->ners_lmInfo.nelmip_entities, \
             readingState->ners_lmInfo.nelmip_isSet)

#define NGEXL_AFS_MEMBER_SET_BOOL(member, arg) \
         NGEXL_AF_MEMBER_SET_BOOL(member, arg, \
             readingState->ners_lmInfo.nelmip_entities, \
             readingState->ners_lmInfo.nelmip_isSet)

#define NGEXL_AFS_MEMBER_SET_LOGLEVEL(member, arg) \
         NGEXL_AF_MEMBER_SET_LOGLEVEL(member, arg, \
             readingState->ners_lmInfo.nelmip_entities, \
             readingState->ners_lmInfo.nelmip_isSet)

#define NGEXL_AFS_MEMBER_SET_UNIT(member, minValue, arg, unitTable) \
         NGEXL_AF_MEMBER_SET_UNIT(member, minValue, arg, \
             readingState->ners_lmInfo.nelmip_entities, \
             readingState->ners_lmInfo.nelmip_isSet, unitTable)


static int 
ngexlAttrFunc_tmpdir               NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_tmpdir";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_tmpDir, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel             NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_log.ngli_level, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel_gt          NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel_gt";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_log.ngli_levelGlobusToolkit, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel_ngprot      NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel_ngprot";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_log.ngli_levelNinfgProtocol, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel_ngi         NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel_ngi";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_log.ngli_levelNinfgInternal, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_filePath         NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_filePath";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_log.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_suffix           NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_suffix";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_log.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_nFiles           NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_nFiles";

    NGEXL_AFS_MEMBER_SET_INT(nglmi_log.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_maxFileSize      NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_maxFileSize";

    NGEXL_AFS_MEMBER_SET_UNIT(
        nglmi_log.ngli_maxFileSize, 0, token->nti_tokenStr, ngiSizeUnitTable)

    return 1;
}

static int 
ngexlAttrFunc_log_overWrite        NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_overWrite";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_log.ngli_overWriteDir, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_enable       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_enable";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_commLog.ngli_enable, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_filePath     NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_filePath";

    NGEXL_AFS_MEMBER_SET_STR(
        nglmi_commLog.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_suffix       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_suffix";

    NGEXL_AFS_MEMBER_SET_STR(
        nglmi_commLog.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_nFiles       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_nFiles";

    NGEXL_AFS_MEMBER_SET_INT(
        nglmi_commLog.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_maxFileSize  NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_maxFileSize";

    NGEXL_AFS_MEMBER_SET_UNIT(
        nglmi_commLog.ngli_maxFileSize, 0,
        token->nti_tokenStr, ngiSizeUnitTable)

    return 1;
}

static int 
ngexlAttrFunc_commLog_overWrite    NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_overWrite";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_commLog.ngli_overWriteDir, token->nti_tokenStr)

    return 1;
}

static int
ngexlAttrFunc_save_stdout          NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_save_stdout";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_saveStdout, token->nti_tokenStr)

    return 1;
}

static int
ngexlAttrFunc_save_stderr          NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_save_stderr";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_saveStderr, token->nti_tokenStr)

    return 1;
}

static int
ngexlAttrFunc_handling_signals     NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_handling_signals";
    int (*cmp)(const char *s1, const char *s2, size_t n);
    ngexlLocalMachineInfoPair_t *lmInfoPair;
    int nSignals, *signalNumbers, found, foundNumber, i, j;
    ngiStringList_t *paramSignalStrings, *cur;
    char **signalNames, *end, *endptr;
    int nParamSignals, *paramTable;
    ngLog_t *log;
    int result;

    log = context->ngc_log;
    lmInfoPair = &readingState->ners_lmInfo;
    paramSignalStrings = NULL;
    nParamSignals = 0;
    paramTable = NULL;
    nSignals = 0;
    signalNames = NULL;
    signalNumbers = NULL;

    cmp = (NGI_ATTR_ARG_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    if (lmInfoPair->nelmip_isSet.nglmi_signals != NULL) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0; 
    }

    paramSignalStrings = ngiReadStringListFromArg(token->nti_tokenStr);
    /* NULL is valid */

    result = ngiStringListCount(paramSignalStrings, &nParamSignals);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Count the string list failed.\n", fName);
        goto error;
    }

    paramTable = globus_libc_calloc(sizeof(int), nParamSignals + 1);
    if (paramTable == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the signal table.\n", fName);
        goto error;
    }

    for (i = 0; i < (nParamSignals + 1); i++) {
        paramTable[i] = 0; /* 0 is to terminate */
    }

    /* Get the signal names */
    result = ngiSignalManagerSignalNamesGet(
        &signalNames, &signalNumbers, &nSignals, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the signal table.\n", fName);
        goto error;
    }

    cur = paramSignalStrings;
    i = 0;
    while (cur != NULL) {
        /* Is attribute value "none"? */
        if ((i == 0) &&
            (nParamSignals == 1) &&
            ((*cmp)(cur->nsl_string, "none", strlen(cur->nsl_string) + 1)
                == 0)) {
            /* Signal is empty */
            break;
        }

        /* Decode the attribute value by name */
        found = 0;
        foundNumber = 0;
        for (j = 0; j < nSignals; j++) {
            /* The case sensitive search. */
            result = (*cmp)(signalNames[j], cur->nsl_string,
                strlen(signalNames[j]) + 1);
            if (result == 0) {
                /* Found */
                found = 1;
                foundNumber = signalNumbers[j];
                break;
            }
        }
        
        /* Decode the attribute value by number */
        if (found != 1) {
            end = &(cur->nsl_string[strlen(cur->nsl_string)]);
            foundNumber = (int)strtol(cur->nsl_string, &endptr, 10);
            if ((foundNumber <= 0) || (endptr != end)) {
                ngiConfigFileSyntaxError(context->ngc_log, token,
                    "invalid signal name: ",
                    fName, attrName, cur->nsl_string, error);
                goto error;
            }
            found = 1;
        }

        /* Is this first appearance? */
        for (j = 0; j < i; j++) {
            if (paramTable[j] == foundNumber) {
                ngiConfigFileSyntaxError(context->ngc_log, token,
                    "signal already appeared: ",
                    fName, attrName, cur->nsl_string, error);
                goto error;
            }
        }
        
        assert(found == 1);
        paramTable[i] = foundNumber;

        cur = cur->nsl_next;
        i++;
    }

    lmInfoPair->nelmip_entities.nglmi_signals = paramTable;
    lmInfoPair->nelmip_isSet.nglmi_signals = NGEX_ISSET_IP_TRUE;
    paramTable = NULL;

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(paramSignalStrings);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the StringList.\n", fName);
            paramSignalStrings = NULL;
            goto error;
        }
        paramSignalStrings = NULL;
    }

    result = ngiSignalManagerSignalNamesDestruct(
        signalNames, signalNumbers, nSignals, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Signal Names.\n", fName);
        signalNames = NULL;
        signalNumbers = NULL;
        goto error;
    }
    signalNames = NULL;
    signalNumbers = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(paramSignalStrings);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the StringList.\n", fName);
            paramSignalStrings = NULL;
        }
    }

    if (paramTable != NULL) {
        globus_libc_free(paramTable);
        paramTable = NULL;
    }

    if ((signalNames != NULL) || (signalNumbers != NULL)) {
        result = ngiSignalManagerSignalNamesDestruct(
            signalNames, signalNumbers, nSignals, log, NULL);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Signal Names.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

static int
ngexlAttrFunc_continueOnError      NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_continueOnError";

    NGEXL_AFS_MEMBER_SET_BOOL(nglmi_continueOnError, token->nti_tokenStr)

    return 1;
}

#undef NGEXL_AFS_MEMBER_SET_STR
#undef NGEXL_AFS_MEMBER_SET_INT
#undef NGEXL_AFS_MEMBER_SET_BOOL
#undef NGEXL_AFS_MEMBER_SET_LOGLEVEL
#undef NGEXL_AFS_MEMBER_SET_UNIT


/**
 * Initialize
 */
static int 
ngexlLocalMachineInfoPairInitialize(
    ngexiContext_t *context,
    ngexlLocalMachineInfoPair_t *lmInfoPair,
    int *error)
{
    static const char fName[] = "ngexlLocalMachineInfoPairInitialize";
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    result = ngexiLocalMachineInformationInitialize(
        &lmInfoPair->nelmip_entities, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information.\n",
            fName);
        return 0;
    }

    result = ngexiLocalMachineInformationInitialize(
        &lmInfoPair->nelmip_isSet, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Local Machine Information.\n",
            fName);
        return 0;
    }

    return 1;
}

/**
 * Finalize
 */
static int 
ngexlLocalMachineInfoPairFinalize(
    ngexiContext_t *context,
    ngexlLocalMachineInfoPair_t *lmInfoPair, 
    int *error)
{
    if (lmInfoPair->nelmip_entities.nglmi_tmpDir != NULL) {
        globus_libc_free(lmInfoPair->nelmip_entities.nglmi_tmpDir);
    }

    ngLogInformationRelease(
        &lmInfoPair->nelmip_entities.nglmi_log, error);
    ngLogInformationRelease(
        &lmInfoPair->nelmip_entities.nglmi_commLog, error);

    return 1;
}

#define NGEXL_LMISP_MEMBER_SET_STR(dst, src, member) \
    { \
        if (src->nelmip_isSet.member != NULL) { \
           if (dst->member != NULL) { \
               globus_libc_free(dst->member); \
           }  \
           dst->member = strdup(src->nelmip_entities.member); \
           if (dst->member == NULL) { \
               NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
               ngLogPrintf(log, \
                   NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, \
                   "%s: Can't allocate the storage for string.\n", \
                    fName); \
               return 0; \
           } \
        } \
    }

#define NGEXL_LMISP_MEMBER_SET_INT(dst, src, member) \
    { \
        if (src->nelmip_isSet.member != 0) { \
           dst->member = src->nelmip_entities.member; \
        } \
    }

/**
 * This function sets dst from src member which isSet flag is true.
 */
static int 
ngexlLocalMachineInformationSetPair(
    ngexiLocalMachineInformation_t *dst, 
    ngexlLocalMachineInfoPair_t *src,
    ngLog_t *log, 
    int *error)
{
    static const char fName[] = "ngexlLocalMachineInformationSetPair";
    int *origTable, *newTable, size, i;

    size = 0;
    origTable = NULL;
    newTable = NULL;

    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_tmpDir)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_enable)

    if (src->nelmip_isSet.nglmi_log.ngli_level != 0) {
        dst->nglmi_log.ngli_level =
            dst->nglmi_log.ngli_levelGlobusToolkit =
            dst->nglmi_log.ngli_levelNinfgProtocol =
            dst->nglmi_log.ngli_levelNinfgInternal =
            src->nelmip_entities.nglmi_log.ngli_level;
    }

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_levelGlobusToolkit)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_levelNinfgProtocol)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_levelNinfgInternal)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_log.ngli_filePath)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_log.ngli_suffix)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_nFiles)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_maxFileSize)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_log.ngli_overWriteDir)

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLog.ngli_enable)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commLog.ngli_filePath)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commLog.ngli_suffix)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLog.ngli_nFiles)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLog.ngli_maxFileSize)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLog.ngli_overWriteDir)

    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_saveStdout)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_saveStderr)

    if (src->nelmip_isSet.nglmi_signals != NULL) {
        /* Clear old data and set new data */
        if (dst->nglmi_signals != NULL) {
            globus_libc_free(dst->nglmi_signals);
            dst->nglmi_signals = NULL;
        }

        dst->nglmi_signals = NULL;
        if (src->nelmip_entities.nglmi_signals != NULL) {
            origTable = src->nelmip_entities.nglmi_signals;
            for (size = 0; origTable[size] != 0; size++);

            newTable = globus_libc_calloc(sizeof(int), size + 1);
            if (newTable == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the signal table.\n", fName);
                return 0;
            }

            for (i = 0; i < (size + 1); i++) {
                newTable[i] = origTable[i];
            }

            dst->nglmi_signals = newTable;
        }
    }

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_continueOnError)

    return 1;
}

static int
ngexlLocalMachineInformationSetDefault(
    ngexiLocalMachineInformation_t *lmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlLocalMachineInformationSetDefault";

    if (lmInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    result = ngiLogInformationSetDefault(
        &lmInfo->nglmi_log, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set default value for Log Information.\n", fName);
        return 0;
    }

    result = ngiLogInformationSetDefault(
        &lmInfo->nglmi_commLog, NG_LOG_TYPE_COMMUNICATION, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set default value for Comm Log Information.\n", fName);
        return 0;
    }

    lmInfo->nglmi_tmpDir = ngiDefaultTemporaryDirectoryNameGet(log, error);

    if (lmInfo->nglmi_tmpDir == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get default temporary directory name.\n",
            fName);
        return 0;
    }

    lmInfo->nglmi_saveStdout = NULL;
    lmInfo->nglmi_saveStderr = NULL;

    lmInfo->nglmi_signals = NULL; /* use default defined on Register */
    lmInfo->nglmi_continueOnError = 0; /* false : immediate exit on error */

    return 1;
}

