#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclConfigFileLdif.c,v $ $Revision: 1.34 $ $Date: 2005/06/16 03:36:56 $";
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
 * Module of client configuration local ldif file reading.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "ngConfigFile.h"

static int ngcllLdifFuncHostInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncHostInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncHostInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncHostInfo_mpirun         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncHostInfo_mpicpus        NGCLL_CONFIG_ATTRFUNC_ARG;

static int ngcllLdifFuncFuncInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_funcname       NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_module         NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_entry          NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_path           NGCLL_CONFIG_ATTRFUNC_ARG;
static int ngcllLdifFuncFuncInfo_stub           NGCLL_CONFIG_ATTRFUNC_ARG;

/* table data definition below */
static ngcllAttrFuncTable_t hostInfoAttrs[] = {
    {"GridRPC-Hostname:",      ngcllLdifFuncHostInfo_hostname},
        /* support old notation */
    {"GridRPC-MpirunCommand:", ngcllLdifFuncHostInfo_mpirun},
    {"GridRPC-MpirunNoOfCPUs:",ngcllLdifFuncHostInfo_mpicpus},
    {NULL, NULL}
};

#define NG_STUB_ATTR_NAME "GridRPC-Stub::"
static ngcllAttrFuncTable_t funcInfoAttrs[] = {
    {"GridRPC-Hostname:",      ngcllLdifFuncFuncInfo_hostname},
    {"GridRPC-Funcname:",      ngcllLdifFuncFuncInfo_funcname},
    {"GridRPC-Module:",        ngcllLdifFuncFuncInfo_module},
    {"GridRPC-Entry:",         ngcllLdifFuncFuncInfo_entry},
    {"GridRPC-Path:",          ngcllLdifFuncFuncInfo_path},
    {NG_STUB_ATTR_NAME,        ngcllLdifFuncFuncInfo_stub},
    {NULL, NULL}
};

static ngcllTagFuncTable_t localLdifTable[] = {
  {"HOST_INFO", hostInfoAttrs,
         ngcllLdifFuncHostInfoBegin,    ngcllLdifFuncHostInfoEnd},

  {"FUNCTION_INFO", funcInfoAttrs,
         ngcllLdifFuncFuncInfoBegin,  ngcllLdifFuncFuncInfoEnd},

  {NULL, NULL, NULL, NULL}
};

/**
 * prototypes.
 */
static int
ngcllLocalLdifParse(ngiTokenReadInfo_t *tokenReadInfo,
    ngcllReadingState_t *readingState, ngclContext_t *context, int *error);
static ngcllTagFuncTable_t * ngcllGetLdifTagFuncInfo(
    ngcllTagFuncTable_t *tagTable, char *tagName, int *pos);
static ngcllAttrFunc_t ngcllGetLdifFuncPtr(
    ngcllAttrFuncTable_t *attrTable, char *attrName, int *pos);
static int ngcllCountAttrTableSize(ngcllAttrFuncTable_t *attrTable);
static int ngcllRemoteClassInformationRegisterFromStringList(
    ngclContext_t *context, ngiStringList_t *classInfoStrList, int *error);

static int ngcllBase64Decode(char *inString, char **outData,
    int *outSize, ngLog_t *log, int *error);
static int ngcllBase64DecodeReadChar(char *inString, int *inIndex);
static int ngcllBase64DecodeToCode(int c);

#define NG_B64_PAD       '='
#define NG_B64_PADCODE 0x100  /* 256 */
#define NG_B64_CASE_ERROR  0  /* error */
#define NG_B64_CASE1       1  /*  8bit effective, 16bit EOF PAD */  
#define NG_B64_CASE2       2  /* 16bit effective,  8bit EOF PAD */
#define NG_B64_CASE3       3  /* 24bit effective, no EOF */

/**
 * Functions.
 */
int
ngcllConfigFileReadLocalLdif         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllConfigFileReadLocalLdif";
    ngiTokenReadInfo_t *newTokenReadInfo;
    int result, fileOpen;
    char *ldifFile;
    ngLog_t *log;

    assert(context != NULL);

    log = context->ngc_log;
    ldifFile = NULL;
    newTokenReadInfo = NULL;
    fileOpen = 0;

    ldifFile = ngiReadStringFromArg(token->nti_tokenStr);
    if (ldifFile == NULL) {
       NGI_SET_ERROR(error, NG_ERROR_MEMORY);
       ngLogPrintf(log,
           NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: Can't allocate the storage for string.\n", fName);
        goto error;
    }

    result = ngiConfigFileOpen(ldifFile, &newTokenReadInfo, 1, log, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Opening local LDIF file \"%s\" fail.\n",
            fName, ldifFile);
        goto error;
    }
    fileOpen = 1;

    result = ngcllLocalLdifParse(newTokenReadInfo,
        readingState, context, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Parsing LocalLdif file %s fail.\n",
            fName, ldifFile);
        goto error;
    }

    result = ngiConfigFileClose(newTokenReadInfo, log, error);
    fileOpen = 0;
    newTokenReadInfo = NULL;
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing local LDIF file %s fail.\n",
            fName, ldifFile);
        goto error;
    }

    globus_libc_free(ldifFile);
    ldifFile = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if ((newTokenReadInfo != NULL) && (fileOpen == 1)) {
        result = ngiConfigFileClose(newTokenReadInfo, log, NULL);
        fileOpen = 0;
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Closing local LDIF file %s fail.\n",
                fName, ldifFile);
        }
    }

    if (ldifFile != NULL) {
        globus_libc_free(ldifFile);
        ldifFile = NULL;
    }

    /* Failed */
    return 0;
}

/* enum for where ConfigFileParse reading */
typedef enum ngcllLocalLdifParseReadingType_e {
    NG_READING_FIRST,    /* FIRST state, will come HOST_INFO next */
    NG_READING_GLOBAL,   /* GLOBAL state, next will HOST_ or FUNCTION_ */
    NG_READING_INFO,     /* reading in INFO */
    NG_READING_STUB      /* reading base64 stub information (multi line) */
} ngcllLocalLdifParseReadingType_t;

static int
ngcllLocalLdifParse(
    ngiTokenReadInfo_t *tokenReadInfo,
    ngcllReadingState_t *readingState,
    ngclContext_t *context,
    int *error)
{
    static const char fName[] = "ngcllLocalLdifParse";
    ngiTokenInfo_t tokenEntity, *token;
    ngiTokenInfo_t argTokenEntity, *argToken;
    ngcllLocalLdifParseReadingType_t reading;
    char tagName[NGI_CONFIG_LINE_MAX];
    char attrName[NGI_CONFIG_LINE_MAX];
    ngcllTagFuncTable_t *tagTable, *tagItem;
    ngcllAttrFuncTable_t *attrTable;
    ngcllAttrFunc_t attrfnc;
    ngcllAttrFunc_t tagEndFunc;
    long prevLineNo, curLineNo;
    int curInfoCount, prevInfoCount, maxInfoCount;
    int tagEndFuncNotDone;
    int result;
    ngLog_t *log;

    token = &tokenEntity;
    argToken = &argTokenEntity;
    reading = NG_READING_FIRST;
    tagTable = localLdifTable;
    tagItem = NULL;
    attrTable = NULL;
    tagEndFunc = NULL;
    tagEndFuncNotDone = 0; /* FALSE */
    maxInfoCount = -1;
    prevInfoCount = -1;
    log = context->ngc_log;

    prevLineNo = 0;

    while(ngiGetToken(tokenReadInfo, token, NGI_GETTOKEN_TOKEN,
                                log, error) == NGI_GET_TOKEN_SUCCESS) {
        curLineNo = token->nti_readInfo->ntri_lineno;

        /* Process empty line */
        if (curLineNo > prevLineNo + 1) {
            if (reading == NG_READING_STUB) {
                if (tagEndFuncNotDone) {
                    assert(tagEndFunc != NULL);
                    result = (*tagEndFunc)(tagName, token, readingState,
                                  context, error);
                    if (result != 1) {
                        ngLogPrintf(log,
                            NG_LOG_CATEGORY_NINFG_PURE,
                            NG_LOG_LEVEL_ERROR, NULL,
                            "%s: %s closing tag process fail.\n",
                            fName, tagName);
                        return 0;
                    }
                    tagEndFunc = NULL;
                    tagEndFuncNotDone = 0; /* FALSE */
                    maxInfoCount = -1;
                }
            }

            if (tagEndFuncNotDone) {
                ngiConfigFileSyntaxError(log, token,
                    "Wrong empty line. Not valid local ldif file.",
                     fName, NULL, token->nti_tokenStr, error);
                return 0;
            }

            if (reading != NG_READING_FIRST) {
                reading = NG_READING_GLOBAL;
            }
        }

        if ((reading == NG_READING_FIRST)
            || (reading == NG_READING_GLOBAL)) {
                strncpy(tagName, token->nti_tokenStr, NGI_CONFIG_LINE_MAX);

            tagItem = ngcllGetLdifTagFuncInfo(
                tagTable, tagName, &curInfoCount);

            if (tagItem == NULL) {
                ngiConfigFileSyntaxError(log, token,
                   "No such info in local LDIF",
                    fName, NULL, tagName, error);
                return 0;
            }
            if ((reading == NG_READING_FIRST) && (curInfoCount != 0)) {
                ngiConfigFileSyntaxError(log, token,
                    "first INFO is wrong",
                    fName, NULL, tagName, error);
                return 0;
            }
            reading = NG_READING_INFO;
            prevInfoCount = -1;

            attrTable = tagItem->ntft_attrs;
            assert(attrTable != NULL);
            maxInfoCount = ngcllCountAttrTableSize(attrTable);
            maxInfoCount--; /* Points last index of attrTable */
            if (maxInfoCount < 0) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: AttributeTableSize invalid.\n", fName);
                return 0;
            }

            tagEndFunc = tagItem->ntft_tagEnd;
            assert(tagEndFunc != NULL);
            tagEndFuncNotDone = 1; /* TRUE */

            attrfnc = tagItem->ntft_tagBegin;
            assert(attrfnc != NULL);

            result = (*attrfnc)(tagName, token, readingState, context, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: \"%s\" opening tag process fail.\n", fName, tagName);
                return 0;
            }

        } else if (reading == NG_READING_INFO) {
            strncpy(attrName, token->nti_tokenStr, NGI_CONFIG_LINE_MAX);
            assert(attrTable != NULL);

            attrfnc = ngcllGetLdifFuncPtr(attrTable, attrName,
                &curInfoCount);
            if (attrfnc == NULL) {
                ngiConfigFileSyntaxError(log, token,
                    "No such attribute in local LDIF",
                    fName, attrName, NULL, error);
                return 0;
            }

            /* for to support old mpirun command notation */
            if (attrfnc == ngcllLdifFuncHostInfo_mpicpus) {
                if (curInfoCount == prevInfoCount + 1) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                        "%s: old type ldif file. includes mpirun command.\n",
                        fName);
                } else if (curInfoCount != prevInfoCount + 2) {
                    ngiConfigFileSyntaxError(log, token,
                        "Illegal order of attribute",
                        fName, attrName, NULL, error);
                    return 0;
                }

            } else if (curInfoCount != prevInfoCount + 1) {
                ngiConfigFileSyntaxError(log, token,
                    "Illegal order of attribute",
                    fName, attrName, NULL, error);
                return 0;
            }

            /* get attribute value (attribute argument) */
            result = ngiGetToken(tokenReadInfo, argToken,
                NGI_GETTOKEN_ARGS, log, error);
            if ((result != NGI_GET_TOKEN_SUCCESS)
                        || (argToken->nti_type != NGI_TOKEN_ATTR)) {
                ngiConfigFileSyntaxError(log, token,
                    "No argument for attribute",
                    fName, attrName, NULL, error);
                return 0;
            }

            result = (*attrfnc)(attrName, argToken, readingState,
                context, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: \"%s\" info process fail.\n", fName, attrName);
                return 0;
            }

            if (strncmp(attrName, NG_STUB_ATTR_NAME, NGI_CONFIG_LINE_MAX)
                                                                    == 0) {
                reading = NG_READING_STUB;
            } else {
                if (curInfoCount == maxInfoCount) {
                    assert(tagEndFunc != NULL);
                    result = (*tagEndFunc)(tagName, token, readingState,
                                  context, error);
                    if (result != 1) {
                        ngLogPrintf(log,
                            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                            NULL,
                            "%s: \"%s\" closing tag process fail.\n",
                            fName, tagName);
                        return 0;
                    }
                    tagEndFunc = NULL;
                    tagEndFuncNotDone = 0; /* FALSE */
                    reading = NG_READING_GLOBAL;
                    maxInfoCount = -1;
                }
            }
            prevInfoCount = curInfoCount;

        } else if (reading == NG_READING_STUB) {
            assert(
                strncmp(attrName, NG_STUB_ATTR_NAME, NGI_CONFIG_LINE_MAX) == 0);

            attrfnc = ngcllGetLdifFuncPtr(attrTable, attrName,
                &curInfoCount);
            assert(attrfnc != NULL);

            result = (*attrfnc)(attrName, token, readingState, context, error);
            if (result != 1) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: \"%s\" info process fail.\n", fName, attrName);
                return 0;
            }

        } else {
            /* NOT REACHED */
            abort();
            return 0;
        }
        prevLineNo = curLineNo;
    }

    if (tagEndFuncNotDone) {
        assert(tagEndFunc != NULL);
        result = (*tagEndFunc)(tagName, token, readingState,
                      context, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: \"%s\" closing tag process fail.\n",
                fName, tagName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * This function gets TagTable element from
 *  argument given global variable tagTable[].
 */
static ngcllTagFuncTable_t *
ngcllGetLdifTagFuncInfo(
    ngcllTagFuncTable_t *tagTable,
    char *tagName,
    int *pos)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int i;

    assert(pos != NULL);

    cmp = (NGI_SECTION_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    for (i = 0; tagTable[i].ntft_tagName != NULL; i++) {
        if((*cmp)(
            tagTable[i].ntft_tagName, tagName, NGI_CONFIG_LINE_MAX) == 0) {

            /* found */
            *pos = i;
            return &tagTable[i];
        }
    }

    /* not found */
    *pos = -1;
    return NULL;
}

/**
 * This function gets function pointer corresponding keyword attrName
 * in AttrTable
 */
static ngcllAttrFunc_t
ngcllGetLdifFuncPtr(
    ngcllAttrFuncTable_t *attrTable,
    char *attrName,
    int *pos)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    int i;

    assert(pos != NULL);

    cmp = (NGI_ATTR_NAME_CASE_SENSITIVE ? strncmp : strncasecmp);
    assert(cmp != NULL);

    for (i = 0; attrTable[i].naft_attrName != NULL; i++) {
        if((*cmp)(
            attrTable[i].naft_attrName, attrName, NGI_CONFIG_LINE_MAX) == 0) {

            /* found */
            *pos = i;
            return attrTable[i].naft_func;
        }
    }

    /* not found */
    *pos = -1;
    return NULL;
}

static int
ngcllCountAttrTableSize(ngcllAttrFuncTable_t *attrTable)
{
    int i;

    i = 0;
    while (attrTable[i].naft_attrName != NULL) {
        i++;
    }

    return i;
}

/**
 * attribute functions below
 */

/**
 * template for each Local ldif attributeFunction members
 */

#define NGCLL_LF_MEMBER_SET_STR(member, arg, entity, isset) \
    { \
        if ((isset)->member != NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "info redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        if ((entity)->member != NULL) { \
            globus_libc_free((entity)->member); \
        } \
         \
        (entity)->member = ngiReadStringFromArg((arg)); \
        if ((entity)->member == NULL) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "info argument invalid:", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (isset)->member = NGCL_ISSET_S_TRUE; \
    }

#define NGCLL_LF_MEMBER_SET_INT(member, minValue, arg, entity, isset) \
    { \
        int resultValue; \
        int result; \
         \
        resultValue = 0; \
         \
        if ((isset)->member != 0) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "info redefined", fName, attrName, NULL, error); \
            return 0; \
        } \
         \
        result = ngiReadIntFromArg((arg), &resultValue); \
        if (result != 1) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "info argument invalid (not integer):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        if (resultValue < (minValue)) { \
            ngiConfigFileSyntaxError(context->ngc_log, token, \
                "attribute value invalid (too small):", \
                fName, attrName, token->nti_tokenStr, error); \
            return 0; \
        } \
        (entity)->member = resultValue; \
        (isset)->member = NGCL_ISSET_I_TRUE; \
    }

#define NGCLL_LF_ATTRFUNC_ASSERTS \
    { \
        assert(attrName != NULL); \
        assert(token != NULL); \
        assert(readingState != NULL); \
        assert(context != NULL); \
    }

/**
 * HOST_INFO
 */
static int
ngcllLdifFuncHostInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncHostInfoBegin";
    ngLog_t *log;
    int result;

    NGCLL_LF_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    readingState->nrs_localCurRmInfo = ngcllRemoteMachineInfoPairCreate(
        context, error);
    if (readingState->nrs_localCurRmInfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Remote Machine Info Pair.\n", fName);
        return 0;
    }

    result = ngcllRemoteMachineInfoPairInitialize(context,
        readingState->nrs_localCurRmInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Remote Machine Info Pair.\n", fName);
        return 0;
    }

    return 1;
}

static int
ngcllLdifFuncHostInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncHostInfoEnd";
    ngcllRemoteMachineInfoPair_t  *rmInfoPair, *foundPair;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_LF_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    rmInfoPair = readingState->nrs_localCurRmInfo;

    /* Validity check */
   if (rmInfoPair->nrmip_isSet->ngrmi_hostName == NULL) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "No hostname in local LDIF info:",
            fName, NULL, attrName, error);
        return 0;
    }

    /* Register to appearedLocalRmInfoHosts */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedLocalRmInfoHosts,
        rmInfoPair->nrmip_entities->ngrmi_hostName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedLocalRmInfoHosts),
            rmInfoPair->nrmip_entities->ngrmi_hostName);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register StringList.\n", fName);
            return 0;
        }
    }

    /* Check if already registered */
    foundPair = ngcllRemoteMachineInfoPairGet(
        readingState->nrs_localRmInfos,
        rmInfoPair->nrmip_entities->ngrmi_hostName);
    if (foundPair != NULL) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Already registered host %s in local ldif. ignoreing.\n",
            fName, rmInfoPair->nrmip_entities->ngrmi_hostName);

        result = ngcllRemoteMachineInfoPairDestruct(
            context, rmInfoPair, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't destruct the Remote Machine Info Pair.\n", fName);
            return 0;
        }
    } else {
        /* Register */
        result = ngcllRemoteMachineInfoPairRegister(
            &(readingState->nrs_localRmInfos), rmInfoPair);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register the Remote Machine Info Pair.\n", fName);
            return 0;
        }
    }
    readingState->nrs_localCurRmInfo = NULL;

    /* Success */
    return 1;
}

/**
 * template for each RemoteMachineInfo members
 */

#define NGCLL_LFH_MEMBER_SET_STR(member, arg) \
         NGCLL_LF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_localCurRmInfo->nrmip_entities, \
             readingState->nrs_localCurRmInfo->nrmip_isSet)

#define NGCLL_LFH_MEMBER_SET_INT(member, minValue, arg) \
         NGCLL_LF_MEMBER_SET_INT(member, minValue, arg, \
             readingState->nrs_localCurRmInfo->nrmip_entities, \
             readingState->nrs_localCurRmInfo->nrmip_isSet)

static int
ngcllLdifFuncHostInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncHostInfo_hostname";

    NGCLL_LF_ATTRFUNC_ASSERTS
    NGCLL_LFH_MEMBER_SET_STR(ngrmi_hostName, token->nti_tokenStr)

    return 1;
}

static int
ngcllLdifFuncHostInfo_mpirun         NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncHostInfo_mpirun";

    NGCLL_LF_ATTRFUNC_ASSERTS

    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s:%s:line %ld: %s is an obsolete attribute.  Ignored.\n",
        fName,
        token->nti_readInfo->ntri_filename,
        token->nti_readInfo->ntri_lineno,
        attrName);

    return 1;
}

static int
ngcllLdifFuncHostInfo_mpicpus        NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncHostInfo_mpicpus";

    NGCLL_LF_ATTRFUNC_ASSERTS
    NGCLL_LFH_MEMBER_SET_INT(ngrmi_mpiNcpus, 0, token->nti_tokenStr)

    return 1;
}

/**
 * FUNCTION_INFO
 */
static int
ngcllLdifFuncFuncInfoBegin           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfoBegin";
    ngLog_t *log;
    int result;

    NGCLL_LF_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    readingState->nrs_localCurEpInfo = ngcllExecutablePathInfoPairCreate(
        context, error);
    if (readingState->nrs_localCurEpInfo == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the Executable Path Info Pair.\n", fName);
        return 0;
    }

    result = ngcllExecutablePathInfoPairInitialize(context,
        readingState->nrs_localCurEpInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't initialize the Executable Path Info Pair.\n", fName);
        return 0;
    }
    assert(readingState->nrs_localCurEpInfoStub == NULL);

    /* Success */
    return 1;
}

static int
ngcllLdifFuncFuncInfoEnd             NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfoEnd";
    ngcllExecutablePathInfoPair_t *epInfoPair, *foundPair;
    char *currentClass;
    int result, isRegistered;
    ngLog_t *log;

    NGCLL_LF_ATTRFUNC_ASSERTS

    log = context->ngc_log;
    epInfoPair = readingState->nrs_localCurEpInfo;

    /* Validity check */
    if (epInfoPair->nepip_isSet->ngepi_hostName == NULL) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "No hostname in local LDIF info:",
            fName, NULL, attrName, error);
        return 0;
    }
    if (epInfoPair->nepip_isSet->ngepi_className == NULL) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "No function name in local LDIF info:",
            fName, NULL, attrName, error);
        return 0;
    }

    /* Register to appearedRmInfoHostsInEp */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedRmInfoHostsInEp,
        epInfoPair->nepip_entities->ngepi_hostName,
        NGI_HOSTNAME_CASE_SENSITIVE, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }
    if (!isRegistered) {
        result = ngiStringListRegister(
            &(readingState->nrs_appearedRmInfoHostsInEp),
            epInfoPair->nepip_entities->ngepi_hostName);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register to StringList.\n", fName);
            return 0;
        }
    }

    /* Check if already registered */
    foundPair = ngcllExecutablePathInfoPairGet(
        readingState->nrs_localEpInfos,
        epInfoPair->nepip_entities->ngepi_hostName,
        epInfoPair->nepip_entities->ngepi_className);
    if (foundPair != NULL) {
        ngiConfigFileSyntaxError(context->ngc_log, token,
            "Already registered class in the host:",
            fName, NULL, epInfoPair->nepip_entities->ngepi_className, error);
        return 0;
    }

    /* Register */
    result = ngcllExecutablePathInfoPairRegister(
        &(readingState->nrs_localEpInfos), epInfoPair);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the Executable Path Info Pair.\n", fName);
        return 0;
    }
    readingState->nrs_localCurEpInfo = NULL;

    /* Treat Stub info */

    currentClass = epInfoPair->nepip_entities->ngepi_className;

    /* Check if RemoteClassInformation was registered */
    result = ngiStringListCheckIncludeSameString(
        readingState->nrs_appearedClasses, currentClass, 1, &isRegistered);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't check the StringList.\n", fName);
        return 0;
    }

    if (!isRegistered) {
        /* Not registered yet, then register */
        result = ngcllRemoteClassInformationRegisterFromStringList(
            context, readingState->nrs_localCurEpInfoStub, error);
        if (result != 1) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register th Remote Class Information.\n", fName);
            return 0;
        }

        result = ngiStringListRegister(
            &(readingState->nrs_appearedClasses), currentClass);
        if (result != 1) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
                "%s: Can't register to StringList.\n", fName);
            return 0;
        }
            
    }
    /* if already registered, then do nothing */

    /* Destruct temporary string list to store base64 string */
    result = ngiStringListDestruct(readingState->nrs_localCurEpInfoStub);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't destruct the StringList.\n", fName);
        return 0;
    }
    readingState->nrs_localCurEpInfoStub = NULL;

    /* Success */
    return 1;
}

/**
 * template for each RemoteMachineInfo members
 */

#define NGCLL_LFF_MEMBER_SET_STR(member, arg) \
         NGCLL_LF_MEMBER_SET_STR(member, arg, \
             readingState->nrs_localCurEpInfo->nepip_entities, \
             readingState->nrs_localCurEpInfo->nepip_isSet)

static int
ngcllLdifFuncFuncInfo_hostname       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfo_hostname";

    NGCLL_LF_ATTRFUNC_ASSERTS
    NGCLL_LFF_MEMBER_SET_STR(ngepi_hostName, token->nti_tokenStr)

    return 1;
}

static int
ngcllLdifFuncFuncInfo_funcname       NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfo_funcname";

    NGCLL_LF_ATTRFUNC_ASSERTS
    NGCLL_LFF_MEMBER_SET_STR(ngepi_className, token->nti_tokenStr)

    return 1;
}

static int
ngcllLdifFuncFuncInfo_module         NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_LF_ATTRFUNC_ASSERTS

    /* Do nothing */
    /* check validity for funcname and module correspondence suppressed.  */

    return 1;
}

static int
ngcllLdifFuncFuncInfo_entry          NGCLL_CONFIG_ATTRFUNC_ARG
{
    NGCLL_LF_ATTRFUNC_ASSERTS

    /* Do nothing */
    /* check validity for funcname and entry correspondence suppressed.  */

    return 1;
}

static int
ngcllLdifFuncFuncInfo_path           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfo_path";

    NGCLL_LF_ATTRFUNC_ASSERTS
    NGCLL_LFF_MEMBER_SET_STR(ngepi_path, token->nti_tokenStr)

    return 1;
}

static int
ngcllLdifFuncFuncInfo_stub           NGCLL_CONFIG_ATTRFUNC_ARG
{
    static const char fName[] = "ngcllLdifFuncFuncInfo_stub";
    char *stubStringPart;
    ngLog_t *log;
    int result;

    NGCLL_LF_ATTRFUNC_ASSERTS

    log = context->ngc_log;

    stubStringPart = ngiReadStringFromArg(token->nti_tokenStr);
    if (stubStringPart == NULL) {
        ngiConfigFileSyntaxError(log, token,
            "no stub info:", fName, NULL, token->nti_tokenStr, error);
        return 0;
    }

    result = ngiStringListRegister(
        &(readingState->nrs_localCurEpInfoStub), stubStringPart);
    globus_libc_free(stubStringPart);
    if (result != 1) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't register the StringList.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * utility functions
 */

static int
ngcllRemoteClassInformationRegisterFromStringList(
    ngclContext_t *context,
    ngiStringList_t *classInfoStringList,
    int *error)
{
    static const char fName[] =
        "ngcllRemoteClassInformationRegisterFromStringList";
    char *classInfoBuffer, *b64Buffer;
    ngRemoteClassInformation_t *rcInfo;
    int classInfoBufferSize;
    ngLog_t *log;
    int result;

    log = context->ngc_log;

    /* merge from StringList to one string */
    b64Buffer = ngiStringListMergeToString(classInfoStringList);
    if (b64Buffer == NULL) {
       ngLogPrintf(log,
           NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: Can't allocate the storage for string list.\n", fName);
        return 0;
    }

    /* Decode base64 */
    result = ngcllBase64Decode(b64Buffer,
        &classInfoBuffer, &classInfoBufferSize, log, error);
    if (result != 1) {
        ngLogPrintf(log,
           NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: decoding base64 for Remote Class Information failed.\n", fName);
        return 0;
    }
    assert(classInfoBuffer != NULL);
    assert(classInfoBufferSize != 0);

    /* Generate RemoteClassInformation */
    rcInfo = ngcliRemoteClassInformationGenerate(
       context, classInfoBuffer, classInfoBufferSize, error);
    if (rcInfo == NULL) {
        ngLogPrintf(log,
           NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: Generate the Remote Class Information failed.\n", fName);
        return 0;
    }

    /* Register RemoteClassInformation */
    result = ngcliRemoteClassInformationCacheRegister(context, rcInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
           NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: Can't register the Remote Class Information.\n", fName);
        return 0;
    }

    /* Release RemoteClassInformation */
    result = ngclRemoteClassInformationRelease(context, rcInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Remote Class Information.\n", fName);
        return 0;
    }

    /* Free RemoteClassInformation */
    result = ngcliRemoteClassInformationFree(context, rcInfo, error);
    if (result != 1) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the Remote Class Information.\n", fName);
        return 0;
    }

    globus_libc_free(b64Buffer);
    globus_libc_free(classInfoBuffer);

    return 1;
}

/**
 * ngcllBase64Decode decodes base64 encoded string.
 * This implementation mallocs string for result and store to outData.
 * If error occur, return NULL.
 */
static int 
ngcllBase64Decode(
    char *inString,
    char **outData,
    int *outSize,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcllBase64Decode";
    char *outBuf;
    int outIndex, inIndex;
    int in1, in2, in3, in4;
    int out1, out2, out3;
    long resultChars; /* includes 3 characters */
    int proc;

    /* Check the arguments */
    if ((inString == NULL) || (outData == NULL) || (outSize == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* malloc buffer to return : same size as inString (little verbose) */
    outBuf = (char *)globus_libc_malloc(sizeof(char) * (strlen(inString) + 1));
    if (outBuf == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for base64 decode string.\n",
            fName);
        return 0;
    }

    *outData = NULL;
    *outSize = 0;

    inIndex = outIndex = 0;

    while(1) {
        proc = NG_B64_CASE_ERROR;

        in1 = ngcllBase64DecodeReadChar(inString, &inIndex);
        in2 = ngcllBase64DecodeReadChar(inString, &inIndex);
        in3 = ngcllBase64DecodeReadChar(inString, &inIndex);
        in4 = ngcllBase64DecodeReadChar(inString, &inIndex);

        if (in1 == EOF) {
            /* Decoding successful */
            break;
        }
        if ((in2 == EOF) || (in3 == EOF) || (in4 == EOF)) {
            globus_libc_free(outBuf);
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: unexpected EOF in base64 string.\n", fName);
            return 0;
        }
        if (in2 == NG_B64_PAD) {
            globus_libc_free(outBuf);
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: unexpected character in base64 string.\n", fName);
            return 0;
        }
        if (in3 == NG_B64_PAD) {
            proc = NG_B64_CASE1;
        } else if (in4 == NG_B64_PAD) {
            proc = NG_B64_CASE2;
        } else {
            proc = NG_B64_CASE3;
        }

        /* Convert from character code to decode table index number */
        in1 = ngcllBase64DecodeToCode(in1);
        in2 = ngcllBase64DecodeToCode(in2);
        in3 = ngcllBase64DecodeToCode(in3);
        in4 = ngcllBase64DecodeToCode(in4);

        /* Check error */
        if ((in1 < 0) || (in2 < 0) || (in3 < 0) || (in4 < 0)) {
            globus_libc_free(outBuf);
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: unexpected character in base64 string.\n", fName);
            return 0;
        }

        resultChars = ((in1 & 0x3f) << (3 * 6))
                    | ((in2 & 0x3f) << (2 * 6))
                    | ((in3 & 0x3f) << (1 * 6))
                    | ((in4 & 0x3f) << (0 * 6));

        out1 = (resultChars >> (2 * 8)) & 0xff;
        out2 = (resultChars >> (1 * 8)) & 0xff;
        out3 = (resultChars >> (0 * 8)) & 0xff;

        switch (proc) {
        case NG_B64_CASE_ERROR:
            globus_libc_free(outBuf);
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: unexpected character in base64 string.\n", fName);
            return 0;

        case NG_B64_CASE1:
            outBuf[outIndex++] = out1;
            break;

        case NG_B64_CASE2:
            outBuf[outIndex++] = out1;
            outBuf[outIndex++] = out2;
            break;

        case NG_B64_CASE3:
            outBuf[outIndex++] = out1;
            outBuf[outIndex++] = out2;
            outBuf[outIndex++] = out3;
            break;
        }
        if ((proc == NG_B64_CASE1) || (proc == NG_B64_CASE2)) {
            /* Decoding successful */
            break;
        }
    }
    outBuf[outIndex] = '\0'; /* effective for non-NULL terminated string */

    /* Success */
    *outData = outBuf;
    *outSize = outIndex;
    return 1;
}

/**
 * Return next valid base64 character in inString.
 * space, tab, return is skipped.
 * If end of string reached, then return EOF.
 */
static int 
ngcllBase64DecodeReadChar(char *inString, int *inIndex)
{
    int c;

    while ((c = inString[*inIndex]) != '\0') {
        (*inIndex)++;
        if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r')) {
            continue;
        }

        return c;
    }

    return EOF;
}

/**
 * Convert from character code to base64 decode table index number
 */
static int
ngcllBase64DecodeToCode(int c)
{
    int base;

    if (c >= 'A' && c <= 'Z') {
        return c - 'A';  /* 0 to 25 */
    }
    base = 'Z' - 'A' + 1;

    if (c >= 'a' && c <= 'z') {
        return c - 'a' + base; /* 26 to 51 */
    }
    base += 'z' - 'a' + 1;

    if (c >= '0' && c <= '9') {
        return c - '0' + base; /* 52 to 61 */
    }
    base += '9' - '0' + 1;

    if (c == '+') {
        return base; /* 62 */
    }
    base++;

    if (c == '/') {
        return base; /* 63 */
    }

    if (c == NG_B64_PAD) { /* '=' */
        return NG_B64_PADCODE;
    }

    /* decode failure */
    return -1;
}

