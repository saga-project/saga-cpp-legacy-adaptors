/*
 * $RCSfile: ngemProtocol.h,v $ $Revision: 1.10 $ $Date: 2008/02/25 10:17:27 $
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
#ifndef NGEM_PROTOCOL_H_
#define NGEM_PROTOCOL_H_

#include "ngemEnvironment.h"
#include "ngemType.h"

#include "ngemList.h"
#include "ngemUtility.h"
#include "ngemCallbackManager.h"
#include "ngemLog.h"

/**
 * @file
 */

#define NGEM_LOGCAT_PROTOCOL "EM Protocol"
#define NGEM_LOGCAT_OPTION   "EM Option"

typedef struct ngemOptionAnalyzer_s ngemOptionAnalyzer_t;
typedef struct ngemOptionAction_s   ngemOptionAction_t;

typedef enum ngemOptionError_e {
    NGEM_OPTION_ERROR_NO_ERROR,
    NGEM_OPTION_ERROR_INTERNAL,
    NGEM_OPTION_ERROR_SYNTAX
} ngemOptionError_t;

/** Option Handler */
typedef ngemResult_t (*ngemOptionHandler_t)(
    ngemOptionAnalyzer_t *, void *, char *, char *);

struct ngemOptionAction_s {
    char                *ngoa_name;
    ngemOptionHandler_t  ngoa_handler;
    void                *ngoa_user_data;
    int                  ngoa_min;
    int                  ngoa_max;
    int                  ngoa_count;
};

NGEM_DECLARE_LIST_OF(ngemOptionAction_t);

/**
 * Analyzer of request's options.
 */
struct ngemOptionAnalyzer_s {
    NGEM_LIST_OF(ngemOptionAction_t) ngoa_actions;
    int                              ngoa_nLines;
    char                            *ngoa_curLine;
    bool                             ngoa_printed;
    bool                             ngoa_first;
    ngemStringBuffer_t               ngoa_messages;
    bool                             ngoa_messagesValid;
    ngemOptionError_t                ngoa_error;
};

ngemOptionAnalyzer_t *ngemOptionAnalyzerCreate(void);
ngemResult_t ngemOptionAnalyzerDestroy(ngemOptionAnalyzer_t *);
ngemResult_t ngemOptionAnalyzerReset(ngemOptionAnalyzer_t *);
ngemResult_t ngemOptionAnalyzerSetAction(ngemOptionAnalyzer_t *, char *, ngemOptionHandler_t, void *, int, int);
ngemResult_t ngemOptionAnalyzerAnalyzeLine(ngemOptionAnalyzer_t *, char *);
ngemResult_t ngemOptionAnalyzerAnalyzeEnd(ngemOptionAnalyzer_t *, char **);
ngemResult_t ngemOptionAnalyzerAnalyzeList(ngemOptionAnalyzer_t *, NGEM_LIST_OF(char) *);

ngemResult_t ngemOptionAnalyzerSetShort(ngemOptionAnalyzer_t *, short *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetInt(ngemOptionAnalyzer_t *, int *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetLong(ngemOptionAnalyzer_t *,long *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetUshort(ngemOptionAnalyzer_t *, unsigned short *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetUint(ngemOptionAnalyzer_t *, unsigned int *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetUlong(ngemOptionAnalyzer_t *, unsigned long *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetString(ngemOptionAnalyzer_t *, char **, char *, char *);
ngemResult_t ngemOptionAnalyzerSetStringList(ngemOptionAnalyzer_t *, NGEM_LIST_OF(char) *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetBool(ngemOptionAnalyzer_t *, bool *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetIbool(ngemOptionAnalyzer_t *, int *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetDouble(ngemOptionAnalyzer_t *, double *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetResult(ngemOptionAnalyzer_t *, ngemResult_t *, char *, char *);
ngemResult_t ngemOptionAnalyzerSetIgnore(ngemOptionAnalyzer_t *, void *, char *, char *);

void ngemOptionSyntaxError(ngemOptionAnalyzer_t *, const char *, const char *, ...)
    NG_ATTRIBUTE_PRINTF(3, 4);

/**
 * Option Analyzer: Set callback function for reading option of request.
 */
#define NGEM_OPTION_ANALYZER_SET_ACTION(type, analyzer, name, func, userData, min, max) \
    (((ngemResult_t (*)(ngemOptionAnalyzer_t *,                        \
                      char *, ngemResult_t (*)(ngemOptionAnalyzer_t *, \
                                               type *, char *, char *),\
                      type *, int, int))                               \
      (void (*)(void))ngemOptionAnalyzerSetAction)                     \
      (analyzer, name, func, userData, min, max))

#define NGEM_DECLARE_OPTION_ANALYZER_SET_ENUM(name, type) \
        ngemResult_t name (ngemOptionAnalyzer_t *, type *, char *, char *)

#define NGEM_DEFINE_OPTION_ANALYZER_SET_ENUM(name, type, items, nItems) \
ngemResult_t                                                            \
name (                                                                  \
    ngemOptionAnalyzer_t *oa,                                           \
    type *userData,                                                     \
    char *name,                                                         \
    char *value)                                                        \
{                                                                       \
    int i;                                                              \
    NGEM_FNAME(name);                                                   \
                                                                        \
    NGEM_ASSERT(oa != NULL);                                            \
    NGEM_ASSERT(name != NULL);                                          \
    NGEM_ASSERT(value != NULL);                                         \
                                                                        \
    for (i = 0;i < nItems;++i) {                                        \
        if (strcmp((items)[i], value) == 0) {                           \
            break;                                                      \
        }                                                               \
    }                                                                   \
    if (i == nItems) {                                                  \
        /* not Found */                                                 \
        ngemOptionSyntaxError(oa, fName,                                \
            "\"%s\" is invalid value.\n", value);                       \
        return NGEM_SUCCESS;                                            \
    }                                                                   \
    *userData = (type)i;                                                \
                                                                        \
    return NGEM_SUCCESS;                                                \
}

#ifdef NGEM_LINE_SEPARATOR_DEBUG
#define NGEML_PROTOCOL_LINE_SEPARATOR "\n" /* for Debug */
#else
#define NGEML_PROTOCOL_LINE_SEPARATOR "\r\n"
#endif

typedef struct ngemRequestInformation_s ngemRequestInformation_t;
typedef struct ngemProtocol_s           ngemProtocol_t;

typedef struct ngemReply_s {
    ngemResult_t              ngr_result;
    bool                      ngr_multiLine;
    char                     *ngr_errorMessage;
    NGEM_LIST_OF(char)        ngr_params;
    NGEM_LIST_OF(char)        ngr_options;
} ngemReply_t;

typedef struct ngemNotify_s {
    char                     *ngn_name;
    bool                      ngn_multiLine;
    NGEM_LIST_OF(char)        ngn_params;
    NGEM_LIST_OF(char)        ngn_options;
} ngemNotify_t;
NGEM_DECLARE_LIST_OF(ngemNotify_t);

typedef struct ngemRequestFunctionArgument_s {
    ngemProtocol_t           *ngra_protocol;
    ngemRequestInformation_t *ngra_requestInfo;
    void                     *ngra_userData;
    char                     *ngra_requestName;
    char                    **ngra_params;
    int                       ngra_nParams;
    ngemOptionAnalyzer_t     *ngra_analyzer;
    ngemReply_t              *ngra_reply;
} ngemRequestFunctionArgument_t;

typedef ngemResult_t (*ngemRequestFunction_t)(ngemRequestFunctionArgument_t *);

/**
 * Informations of each request
 */
struct ngemRequestInformation_s {
    char                 *ngri_string;
    int                   ngri_nArguments;
    bool                  ngri_multiLine;
    ngemRequestFunction_t ngri_beginCallback;
    ngemRequestFunction_t ngri_endCallback;
    ngemRequestFunction_t ngri_replyCallback;
};

NGEM_DECLARE_LIST_OF(ngemRequestInformation_t);

/**
 * Protocol Version
 */
typedef struct ngemProtocolVersion_s {
    int ngpv_major;
    int ngpv_minor;
} ngemProtocolVersion_t;

/**
 * Ninf-G Protocol Talker.
 */
struct ngemProtocol_s {
    bool                                    ngp_available;
    ngemStandardIO_t                        ngp_stdio;
    ngemLineBuffer_t                       *ngp_lineBuffer;
    char                                   *ngp_separator;

    ngemProtocolVersion_t                   ngp_version;

    ngemRequestFunctionArgument_t          *ngp_argument;
    ngemReply_t                            *ngp_reply;

    ngemCallback_t                          ngp_replyCallback;
    ngemCallback_t                          ngp_notifyCallback;

    NGEM_LIST_OF(ngemRequestInformation_t)  ngp_requestInfo;
    NGEM_LIST_OF(char)                      ngp_features;
    NGEM_LIST_OF(ngemNotify_t)              ngp_notifyQueue;

    void                                   *ngp_userData;
};

/* Protocol */
ngemProtocol_t *ngemProtocolCreate(void *, const char *, int, int);
ngemResult_t ngemProtocolDestroy(ngemProtocol_t *);

ngemResult_t  ngemProtocolDisable(ngemProtocol_t *);

/* Set Information */
ngemResult_t ngemProtocolAppendRequestInfo(ngemProtocol_t *, ngemRequestInformation_t *);
ngemResult_t ngemProtocolAppendFeature(ngemProtocol_t *, const char *);

/* Reply */
ngemResult_t ngemReplySetError(ngemReply_t *, const char *);
ngemResult_t ngemReplyGetError(ngemReply_t *);
void ngemReplySetMultiLine(ngemReply_t *, bool);
ngemResult_t ngemReplyAddParam(ngemReply_t *, const char *, ...) NG_ATTRIBUTE_PRINTF(2,3);
ngemResult_t ngemReplyAddOption(ngemReply_t *, const char *, const char *,...) NG_ATTRIBUTE_PRINTF(3,4);

/* Notify */
ngemNotify_t *ngemNotifyCreate( const char *, bool);
void ngemNotifyDestroy(ngemNotify_t *);

ngemResult_t ngemNotifyAddParam(ngemNotify_t *, const char *, ...) NG_ATTRIBUTE_PRINTF(2,3);
ngemResult_t ngemNotifyAddOption(ngemNotify_t *, const char *, const char *,...) NG_ATTRIBUTE_PRINTF(3,4);

ngemResult_t ngemProtocolSendNotify(ngemProtocol_t *, ngemNotify_t **);

extern ngemRequestInformation_t ngemProtocolQueryFeaturesRequest;
extern ngemRequestInformation_t ngemProtocolExitRequest;

/* Callback */
ngemResult_t ngemQueryFeaturesBegin(ngemRequestFunctionArgument_t *arg);
ngemResult_t ngemExitAfterReply(ngemRequestFunctionArgument_t *arg);

#endif
