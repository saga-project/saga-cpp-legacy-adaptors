/*
 * $RCSfile: ngexConfigFileRead.c,v $ $Revision: 1.18 $ $Date: 2008/02/25 10:05:25 $
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
 * Module of remote executable configuration file reading.
 */

#include "ngEx.h"
#include "ngConfigFile.h"

NGI_RCSID_EMBED("$RCSfile: ngexConfigFileRead.c,v $ $Revision: 1.18 $ $Date: 2008/02/25 10:05:25 $")

static int ngexlIsSetIPtrue = 1;
#define NGEXL_ISSET_IP_TRUE (&ngexlIsSetIPtrue)

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
static int ngexlAttrFunc_commProxyLogFilePath NGEXL_CONFIG_ATTRFUNC_ARG;


/* table data definition below */
ngexlAttrFuncTable_t execAttrs[] = {
    {"tmp_dir",                 ngexlAttrFunc_tmpdir},

    {"loglevel",                ngexlAttrFunc_loglevel},
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

    {"communication_proxy_log_filePath", ngexlAttrFunc_commProxyLogFilePath},

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
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;
    NGI_SET_ERROR(&subError, NG_ERROR_NO_ERROR);

    result = ngexlLocalMachineInformationSetDefault(
        &context->ngc_lmInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set Executable Local Machine Information\n"); 
        return 0;
    }

    /* Read system executable configuration file if available. */
    if (sysConfig != NULL) {
	result = ngiConfigFileOpen(
            sysConfig, &tokenReadInfo, 0, log, &subError);
	if (result != 0) {
	    result = ngexlConfigFileParse(context, tokenReadInfo, error);
            if (result == 0) {
		ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
		    "Parsing executable configuration file %s fail.\n", sysConfig); 
		return 0;
	    }

	    result = ngiConfigFileClose(tokenReadInfo, log, error);
	    if (result == 0) {
		ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
		    "Closing executable configuration file %s fail.\n", sysConfig); 
		return 0;
	    }
	} else {
	    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
	    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "No executable configuration file \"%s\"."
                " continue anyway.\n", sysConfig); 
	}
    }

    /* Read user executable configuration file if available. */
    if (userConfig != NULL) {
	result = ngiConfigFileOpen(
            userConfig, &tokenReadInfo, 0, log, &subError);
	if (result != 0) {
	    result = ngexlConfigFileParse(context, tokenReadInfo, error);
	    if (result == 0) {
		ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
		    "Parsing executable configuration file %s fail.\n", userConfig); 
		return 0;
	    }

	    result = ngiConfigFileClose(tokenReadInfo, log, error);
	    if (result == 0) {
		ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
		    "Closing executable configuration file %s fail.\n", userConfig); 
		return 0;
	    }
	} else {
	    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
	    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "No executable configuration file \"%s\"."
                " continue anyway.\n", userConfig); 
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
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to initialize configfile parsing data structure.\n"); 
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
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "attribute \"%s\" process fail.\n", attrName); 
            return 0;
        }
    }
    if (result != NGI_GET_TOKEN_EOF) {
        ngiConfigFileSyntaxError(log, token,
            "Invalid word", fName, NULL, token->nti_tokenStr, error);
        return 0;
    }

    result = ngexlAttrFuncFinalize(context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to finalize configfile parsing data structure.\n"); 
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
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Construct the ReadingState */
    result = ngexlReadingStateConstruct(context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to construct configfile parsing data structure.\n"); 
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
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Register LocalMachineInformation to Context */
    result = ngexlLocalMachineInformationSetPair(
        &context->ngc_lmInfo, &readingState->ners_lmInfo, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Setting Executable Local Machine Information failed.\n"); 
        return 0;
    }

    /* Destruct the ReadingState */
    result = ngexlReadingStateDestruct(context, readingState, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to destruct configfile parsing data structure.\n"); 
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
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Allocate */
    *readingState = (ngexlReadingState_t *)
        ngiCalloc(1, sizeof(ngexlReadingState_t), log, error);
    if (*readingState == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for string.\n"); 
        return 0;
    }

    /* Initialize */
    result = ngexlLocalMachineInfoPairInitialize(
        context, &(*readingState)->ners_lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize Executable Local Machine Information.\n"); 
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
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Finalize */
    result = ngexlLocalMachineInfoPairFinalize(
        context, &readingState->ners_lmInfo, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize Executable Local Machine Information.\n"); 
        return 0;
    }

    ngiFree(readingState, log, error);

    return 1;
}

#define NGEXL_AF_MEMBER_SET_STR(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
         \
        macroLog = context->ngc_log; \
         \
        if ((isset).member != NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity).member != NULL) { \
            ngiFree((entity).member, macroLog, error); \
        } \
         \
        (entity).member = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if ((entity).member == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset).member = NGI_ISSET_S_TRUE; \
    }

#define NGEXL_AF_MEMBER_SET_BOOL(member, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
        char *true_false; \
        int result; \
         \
        macroLog = context->ngc_log; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        true_false = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if (true_false == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadEnumFromArg( \
            true_false, NGI_ATTR_ARG_CASE_SENSITIVE, \
            2, "true", "false"); \
        if (!((result >= 1) && (result <= 2))) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid [true/false]:", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(true_false, macroLog, error); \
            return 0; \
        } \
        if (result == 1) { \
            (entity).member = 1; \
        } else if (result == 2) { \
            (entity).member = 0; \
        } else { \
            abort(); \
        } \
        (isset).member = NGI_ISSET_I_TRUE; \
        ngiFree(true_false, macroLog, error); \
    }

#define NGEXL_AF_MEMBER_SET_INT(member, minValue, arg, entity, isset) \
    { \
        ngLog_t *macroLog; \
        int resultValue; \
        int result; \
         \
        macroLog = context->ngc_log; \
        resultValue = 0; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadIntFromArg((arg), &resultValue); \
        if (result == 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity).member = resultValue; \
        (isset).member = NGI_ISSET_I_TRUE; \
    }

#define NGEXL_AF_MEMBER_SET_LOGLEVEL(member, arg, entity, isset) \
    { \
        char *levelStr; \
        int result; \
        ngLog_t *log;\
         \
        log = context->ngc_log;\
         \
        if ((isset).member != (ngLogLevel_t) 0) { \
            ngiConfigFileSyntaxError(log, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        levelStr = ngiReadStringFromArg((arg), log, error); \
        if (levelStr == NULL) { \
            ngiConfigFileSyntaxError(log, token, \
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
            ngiFree(levelStr, log, error); \
            return 0; \
        } \
        (isset).member = (ngLogLevel_t) NGI_ISSET_I_TRUE; \
        ngiFree(levelStr, log, error); \
    }

#define NGEXL_AF_MEMBER_SET_UNIT( \
    member, minValue, arg, entity, isset, unitTable) \
    { \
        ngLog_t *macroLog; \
        char *unitString; \
        int resultValue; \
        int result; \
         \
        macroLog = context->ngc_log; \
        resultValue = 0; \
         \
        if ((isset).member != 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        unitString = ngiReadStringFromArg( \
            (arg), macroLog, error); \
        if (unitString == NULL) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
         \
        result = ngiReadUnitNumFromArg(unitString, &resultValue, \
             unitTable, NGI_ATTR_ARG_CASE_SENSITIVE); \
        if (result == 0) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(unitString, macroLog, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(macroLog, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            ngiFree(unitString, macroLog, error); \
            return 0; \
        } \
        (entity).member = resultValue; \
        (isset).member = NGI_ISSET_I_TRUE; \
        ngiFree(unitString, macroLog, error); \
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
        nglmi_logLevels.nglli_level, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel_ngprot      NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel_ngprot";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_ninfgProtocol, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_loglevel_ngi         NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_loglevel_ngi";

    NGEXL_AFS_MEMBER_SET_LOGLEVEL(
        nglmi_logLevels.nglli_ninfgInternal, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_filePath         NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_filePath";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_logInfo.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_suffix           NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_suffix";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_logInfo.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_nFiles           NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_nFiles";

    NGEXL_AFS_MEMBER_SET_INT(
        nglmi_logInfo.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_log_maxFileSize      NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_maxFileSize";

    NGEXL_AFS_MEMBER_SET_UNIT(
        nglmi_logInfo.ngli_maxFileSize, 0, token->nti_tokenStr,
        ngiSizeUnitTable)

    return 1;
}

static int 
ngexlAttrFunc_log_overWrite        NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_log_overWrite";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_logInfo.ngli_overWriteDir, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_enable       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_enable";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_commLogEnable, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_filePath     NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_filePath";

    NGEXL_AFS_MEMBER_SET_STR(
        nglmi_commLogInfo.ngli_filePath, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_suffix       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_suffix";

    NGEXL_AFS_MEMBER_SET_STR(
        nglmi_commLogInfo.ngli_suffix, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_nFiles       NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_nFiles";

    NGEXL_AFS_MEMBER_SET_INT(
        nglmi_commLogInfo.ngli_nFiles, 0, token->nti_tokenStr)

    return 1;
}

static int 
ngexlAttrFunc_commLog_maxFileSize  NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_maxFileSize";

    NGEXL_AFS_MEMBER_SET_UNIT(
        nglmi_commLogInfo.ngli_maxFileSize, 0,
        token->nti_tokenStr, ngiSizeUnitTable)

    return 1;
}

static int 
ngexlAttrFunc_commLog_overWrite    NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commLog_overWrite";

    NGEXL_AFS_MEMBER_SET_BOOL(
        nglmi_commLogInfo.ngli_overWriteDir, token->nti_tokenStr)

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
        ngiConfigFileSyntaxError(log, token,
            "attribute redefined", fName, attrName, NULL, error);
        return 0; 
    }

    paramSignalStrings = ngiReadStringListFromArg(
        token->nti_tokenStr, log, error);
    /* NULL is valid */

    result = ngiStringListCount(
        paramSignalStrings, &nParamSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Count the string list failed.\n"); 
        goto error;
    }

    paramTable = ngiCalloc(sizeof(int), nParamSignals + 1, log, error);
    if (paramTable == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal table.\n"); 
        goto error;
    }

    for (i = 0; i < (nParamSignals + 1); i++) {
        paramTable[i] = 0; /* 0 is to terminate */
    }

    /* Get the signal names */
    result = ngiSignalManagerSignalNamesGet(
        &signalNames, &signalNumbers, &nSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the signal table.\n"); 
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
        if (found == 0) {
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
    lmInfoPair->nelmip_isSet.nglmi_signals = NGEXL_ISSET_IP_TRUE;
    paramTable = NULL;

    if (paramSignalStrings != NULL) {
        result = ngiStringListDestruct(
            paramSignalStrings, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            paramSignalStrings = NULL;
            goto error;
        }
        paramSignalStrings = NULL;
    }

    result = ngiSignalManagerSignalNamesDestruct(
        signalNames, signalNumbers, nSignals, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct the Signal Names.\n"); 
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
        result = ngiStringListDestruct(
            paramSignalStrings, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the StringList.\n"); 
            paramSignalStrings = NULL;
        }
    }

    if (paramTable != NULL) {
        ngiFree(paramTable, log, NULL);
        paramTable = NULL;
    }

    if ((signalNames != NULL) || (signalNumbers != NULL)) {
        result = ngiSignalManagerSignalNamesDestruct(
            signalNames, signalNumbers, nSignals, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Signal Names.\n"); 
        }
    }

    /* Failed */
    return 0;
}

static int
ngexlAttrFunc_commProxyLogFilePath NGEXL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngexlAttrFunc_commProxyLogFilePath";

    NGEXL_AFS_MEMBER_SET_STR(nglmi_commProxyLogFilePath, token->nti_tokenStr)

    return 1;
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
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information.\n"); 
        return 0;
    }

    result = ngexiLocalMachineInformationInitialize(
        &lmInfoPair->nelmip_isSet, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Local Machine Information.\n"); 
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
    ngLog_t *log;
    int result;
    static const char fName[] = "ngexlLocalMachineInfoPairFinalize";

    log = context->ngc_log;

    result = ngexiLocalMachineInformationFinalize(
        &lmInfoPair->nelmip_entities, NULL, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't finalize the local machine information.\n");
        return 0;
    }

    return 1;
}

#define NGEXL_LMISP_MEMBER_SET_STR(dst, src, member) \
    { \
        if (src->nelmip_isSet.member != NULL) { \
           if (dst->member != NULL) { \
               ngiFree(dst->member, log, error); \
           } \
           dst->member = strdup(src->nelmip_entities.member); \
           if (dst->member == NULL) { \
               NGI_SET_ERROR(error, NG_ERROR_MEMORY); \
               ngLogError(log, \
                   NG_LOGCAT_NINFG_PURE, fName,  \
                   "Can't allocate the storage for string.\n"); \
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

    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_filePath)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_logInfo.ngli_suffix)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_nFiles)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_maxFileSize)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logInfo.ngli_overWriteDir)

    if (src->nelmip_isSet.nglmi_logLevels.nglli_level != 0) {
        dst->nglmi_logLevels.nglli_level =
        dst->nglmi_logLevels.nglli_ninfgProtocol =
        dst->nglmi_logLevels.nglli_ninfgInternal =
        dst->nglmi_logLevels.nglli_grpc =
            src->nelmip_entities.nglmi_logLevels.nglli_level;
    }

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_ninfgProtocol)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_ninfgInternal)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_logLevels.nglli_grpc)

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLogEnable)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commLogInfo.ngli_filePath)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commLogInfo.ngli_suffix)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLogInfo.ngli_nFiles)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLogInfo.ngli_maxFileSize)
    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_commLogInfo.ngli_overWriteDir)

    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_saveStdout)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_saveStderr)

    if (src->nelmip_isSet.nglmi_signals != NULL) {
        /* Clear old data and set new data */
        if (dst->nglmi_signals != NULL) {
            ngiFree(dst->nglmi_signals, log, error);
            dst->nglmi_signals = NULL;
        }

        dst->nglmi_signals = NULL;
        if (src->nelmip_entities.nglmi_signals != NULL) {
            origTable = src->nelmip_entities.nglmi_signals;
            for (size = 0; origTable[size] != 0; size++);

            newTable = ngiCalloc(sizeof(int), size + 1, log, error);
            if (newTable == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't allocate the signal table.\n"); 
                return 0;
            }

            for (i = 0; i < (size + 1); i++) {
                newTable[i] = origTable[i];
            }

            dst->nglmi_signals = newTable;
        }
    }

    NGEXL_LMISP_MEMBER_SET_INT(dst, src, nglmi_continueOnError)
    NGEXL_LMISP_MEMBER_SET_STR(dst, src, nglmi_commProxyLogFilePath)

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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    result = ngiLogInformationSetDefault(&lmInfo->nglmi_logInfo, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set default value to the log information.\n"); 
        return 0;
    }

    result = ngiLogLevelInformationSetDefault(
        &lmInfo->nglmi_logLevels, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set default value to the log level information.\n"); 
        return 0;
    }

    lmInfo->nglmi_commLogEnable = 0;

    result = ngiLogInformationSetDefault(&lmInfo->nglmi_commLogInfo, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set default value to the communication log information.\n"); 
        return 0;
    }

    lmInfo->nglmi_tmpDir = ngiDefaultTemporaryDirectoryNameGet(log, error);

    if (lmInfo->nglmi_tmpDir == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get default temporary directory name.\n"); 
        return 0;
    }

    lmInfo->nglmi_saveStdout = NULL;
    lmInfo->nglmi_saveStderr = NULL;

    lmInfo->nglmi_signals = NULL; /* use default defined on Register */
    lmInfo->nglmi_continueOnError = 0; /* false : immediate exit on error */
    lmInfo->nglmi_commProxyLogFilePath = NULL;

    return 1;
}

