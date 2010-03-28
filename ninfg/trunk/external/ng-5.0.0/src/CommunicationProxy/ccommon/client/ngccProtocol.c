/*
 * $RCSfile: ngccProtocol.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $
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

#include "ngemUtility.h"
#include "ngccProtocol.h"

NGI_RCSID_EMBED("$RCSfile: ngccProtocol.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $")

/* Functions */
static ngemResult_t ngcclInitializeBegin(ngemRequestFunctionArgument_t *);
static ngemResult_t ngcclInitializeEnd(ngemRequestFunctionArgument_t *);

static ngemResult_t ngcclPrepareCommunicationBegin(ngemRequestFunctionArgument_t *);
static ngemResult_t ngcclPrepareCommunicationEnd(ngemRequestFunctionArgument_t *);
static ngemResult_t ngcclPrepareCommunicationAfterReply(ngemRequestFunctionArgument_t *);

static ngemResult_t ngcclProtocolRegisterPrepareCommunicationOptions(ngccProtocol_t *, ngccPrepareCommunicationOptions_t *);

static ngccInitializeOptions_t *ngcclInitializeOptionsCreate(void);
static void ngcclInitializeOptionsDestroy(ngccInitializeOptions_t *);
static void ngcclInitializeOptionsReset(ngccInitializeOptions_t *);

static ngccPrepareCommunicationOptions_t *ngcclPrepareCommunicationOptionsCreate(void);
static ngemResult_t ngcclPrepareCommunicationOptionsDestroy(ngccPrepareCommunicationOptions_t *);

/* Requests */
static ngemRequestInformation_t ngcclProtocolInitializeRequest = {
    "INITIALIZE",
    0,
    true,
    ngcclInitializeBegin,
    ngcclInitializeEnd,
    NULL
};

static ngemRequestInformation_t ngcclProtocolPrepareCommunicationRequest = {
    "PREPARE_COMMUNICATION",
    0,
    true,
    ngcclPrepareCommunicationBegin,
    ngcclPrepareCommunicationEnd,
    ngcclPrepareCommunicationAfterReply
};

/**
 * Client Communication Protocol: Create
 */
ngccProtocol_t *
ngccProtocolCreate(
    ngccProtocolInformation_t *info,
    void *userData)
{
    ngccProtocol_t *protocol = NULL;
    ngemProtocol_t *baseProtocol = NULL;
    ngccInitializeOptions_t *opts = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngccProtocolCreate);

    log = ngemLogGetDefault();

    protocol = NGI_ALLOCATE(ngccProtocol_t, log, NULL);
    if (protocol == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for the protocol.\n");
        goto error;
    }

    /* Initialize Member */
    protocol->ngp_protocol        = NULL;
    protocol->ngp_info            = NULL;
    protocol->ngp_initOpts        = NULL;
    protocol->ngp_prepareCommOpts = NULL;
    protocol->ngp_initialized     = false;
    protocol->ngp_userData        = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&protocol->ngp_prepareCommOptsList);

    NGEM_LIST_INITIALIZE(ngccPrepareCommunicationOptions_t, &protocol->ngp_prepareCommOptsList);

    baseProtocol = ngemProtocolCreate((void *)protocol, "\r\n",
        NGCP_PROTOCOL_VERSION_MAJOR, NGCP_PROTOCOL_VERSION_MINOR);
    if (baseProtocol == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't create the Base Protocol Manager.\n");
        goto error;
    }

    if (info->ngpi_enableRelay) {
        nResult = ngemProtocolAppendFeature(
            baseProtocol, "RELAY_COMMUNICATION_PROXY");
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
                "Can't append the feature.\n");
            goto error;
        }
    }

    nResult =
        ngemProtocolAppendRequestInfo(baseProtocol, &ngcclProtocolInitializeRequest)           &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngcclProtocolPrepareCommunicationRequest) &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolQueryFeaturesRequest)         &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolExitRequest);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't append the request information.\n");
        goto error;
    }

    opts = ngcclInitializeOptionsCreate();
    if (opts == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "Can't create the options container.\n");
        goto error;
    }

    protocol->ngp_protocol             = baseProtocol;
    protocol->ngp_info                 = info;
    protocol->ngp_initOpts             = opts;
    protocol->ngp_initialized          = false;
    protocol->ngp_userData             = userData;

    return protocol;
error:
    nResult = ngccProtocolDestroy(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't destroy the protocol.\n");
    }

    return NULL;
}

/**
 * Client Communication Protocol: Destroy
 */
ngemResult_t
ngccProtocolDestroy(
    ngccProtocol_t *protocol)
{
    ngLog_t *log = NULL;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_LIST_ITERATOR_OF(ngccPrepareCommunicationOptions_t) it;
    ngccPrepareCommunicationOptions_t *val = NULL;
    NGEM_FNAME(ngccProtocolDestroy);

    log = ngemLogGetDefault();

    if (protocol == NULL) {
        /* No Error */
        return NGEM_SUCCESS;
    }

    result = ngemProtocolDestroy(protocol->ngp_protocol);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't destroy the Base Protocol Manager.\n");
        ret = NGEM_FAILED;
    }
    protocol->ngp_protocol = NULL;

    /* List Destroy */
    NGEM_LIST_ERASE_EACH(ngccPrepareCommunicationOptions_t,
        &protocol->ngp_prepareCommOptsList, it, val) {
        if (val != protocol->ngp_prepareCommOpts) {
            ngcclPrepareCommunicationOptionsDestroy(val);
        }
    }
    NGEM_LIST_FINALIZE(ngccPrepareCommunicationOptions_t,
        &protocol->ngp_prepareCommOptsList);

    ngcclPrepareCommunicationOptionsDestroy(protocol->ngp_prepareCommOpts);
    ngcclInitializeOptionsDestroy(protocol->ngp_initOpts);

    protocol->ngp_initOpts        = NULL;
    protocol->ngp_prepareCommOpts = NULL;
    protocol->ngp_protocol        = NULL;

    NGI_DEALLOCATE(ngccProtocol_t, protocol, log, NULL);

    return ret;
}

/**
 * Client Communication Protocol: Send COMMUNICATION_REPLY notify.
 */
ngemResult_t
ngccProtocolSendCommunicationReplyNotify(
    ngccProtocol_t *protocol,
    ngccPrepareCommunicationOptions_t *pCommOpts)
{
    ngLog_t *log = NULL;
    bool sentNotify = false;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemResult_t nResult;
    NGEM_FNAME(ngccProtocolSendCommunicationReplyNotify);

    log = ngemLogGetDefault();
    
    nResult = ngemNotifyAddOption(
        pCommOpts->ngpo_notify, "request_id", "%d", pCommOpts->ngpo_requestID);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't add option to notify.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }
    if (pCommOpts->ngpo_result == NGEM_SUCCESS) {
        nResult = ngemNotifyAddOption(
            pCommOpts->ngpo_notify, "result", "S");
    } else {
        nResult =
            ngemNotifyAddOption(pCommOpts->ngpo_notify, "result", "F") &&
            ngemNotifyAddOption(pCommOpts->ngpo_notify, "message", "%s", 
                (pCommOpts->ngpo_message!= NULL)?pCommOpts->ngpo_message:"Unknown error");
    }
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't add option to notify.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    sentNotify = true;
    nResult = ngemProtocolSendNotify(
        protocol->ngp_protocol, &pCommOpts->ngpo_notify);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't send the COMMUNICATION_REPLY notify.\n");
        goto finalize;
    }

finalize:
    if (ret == NGEM_FAILED) {
        if (!sentNotify) {
            nResult = ngemProtocolDisable(protocol->ngp_protocol);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
                    "Can't disable the protocol.\n");
            }
        }
    }

    nResult = NGEM_LIST_ERASE_BY_ADDRESS(ngccPrepareCommunicationOptions_t,
        &protocol->ngp_prepareCommOptsList, pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't remove PREPARE_COMMUNICATION's option from list.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngcclPrepareCommunicationOptionsDestroy(pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't destroy PREPARE_COMMUNICATION's option.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * PREPARE_COMMUNICATION's options: Set error to the COMMUNICATION_REPLY notify.
 */
ngemResult_t
ngccPrepareCommunicationOptionsSetError(
    ngccPrepareCommunicationOptions_t *pCommOpts,
    const char *errorMessage)
{
    ngLog_t *log = NULL;
    NGEM_FNAME(ngccPrepareCommunicationOptionsSetError);

    log = ngemLogGetDefault();

    NGEM_ASSERT(pCommOpts != NULL);
    NGEM_ASSERT_STRING(errorMessage);

    if (pCommOpts->ngpo_result != NGEM_SUCCESS) {
        return NGEM_SUCCESS;
    }
    NGEM_ASSERT(pCommOpts->ngpo_message == NULL);

    pCommOpts->ngpo_result = NGEM_FAILED;
    pCommOpts->ngpo_message = ngiStrdup(errorMessage, log, NULL);
    if (pCommOpts->ngpo_message == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "Can't copy string.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Callback function on receiving begin of INITIALIZE request.
 */
static ngemResult_t
ngcclInitializeBegin(
    ngemRequestFunctionArgument_t *arg)
{
    ngemOptionAnalyzer_t *analyzer = arg->ngra_analyzer;
    ngccProtocol_t *protocol = (ngccProtocol_t *)arg->ngra_userData;
    ngLog_t *log = NULL;
    ngccInitializeOptions_t *opts = NULL;
    ngemResult_t nResult;
    char *message = NULL;
    NGEM_FNAME(ngcclInitializeBegin);

    log = ngemLogGetDefault();

    /* Already initialized check */
    if (protocol->ngp_initialized) {
        message = "Communication Proxy is already initialized.";
        goto error;
    }
    protocol->ngp_initialized = true;

    /* Register callback for getting "listen_port" option */
    opts = protocol->ngp_initOpts;
    ngcclInitializeOptionsReset(opts);
    nResult =
        NGEM_OPTION_ANALYZER_SET_ACTION(unsigned short, analyzer,
            "listen_port",  ngemOptionAnalyzerSetUshort, &opts->ngio_clientPort,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "buffer_size",  ngemOptionAnalyzerSetInt, &opts->ngio_bufferSize,   0, 1);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't set options to analyzer.";
        goto error;
    }

    /* Call user's callback function */
    nResult = protocol->ngp_info->ngpi_initializeBegin(protocol, analyzer);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't start to process INITIALIZE.";
        goto error;
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "%s\n", message);
    nResult = ngemReplySetError(arg->ngra_reply, message);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't set error to the reply.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Callback function after receiving INITIALIZE request.
 */
static ngemResult_t
ngcclInitializeEnd(
    ngemRequestFunctionArgument_t *arg)
{
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message = NULL;
    ngccProtocol_t *protocol = (ngccProtocol_t *)arg->ngra_userData;
    NGEM_FNAME(ngcclInitializeEnd);

    log = ngemLogGetDefault();
    reply = arg->ngra_reply;

    if (ngemReplyGetError(reply) == NGEM_SUCCESS) {
        /* Call user's callback function */
        nResult = protocol->ngp_info->ngpi_initializeEnd(
            protocol, protocol->ngp_initOpts,
            reply, ngemReplyGetError(reply));
        if (nResult != NGEM_SUCCESS) {
            message = "Can't process INITIALIZE.";
            goto error;
        }
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "%s\n", message);
    nResult = ngemReplySetError(reply, message);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't set error to the reply.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Callback function on receiving begin of PREPARE_COMMUNICATION request.
 */
static ngemResult_t
ngcclPrepareCommunicationBegin(
    ngemRequestFunctionArgument_t *arg)
{
    ngemOptionAnalyzer_t *analyzer = arg->ngra_analyzer;
    ngccProtocol_t *protocol = (ngccProtocol_t *)arg->ngra_userData;
    ngLog_t *log = NULL;
    ngccPrepareCommunicationOptions_t *pCommOpts = NULL;
    ngemResult_t nResult;
    char *message = NULL;
    NGEM_FNAME(ngcclPrepareCommunicationBegin);

    log = ngemLogGetDefault();

    pCommOpts = ngcclPrepareCommunicationOptionsCreate();
    if (pCommOpts == NULL) {
        message = "Can't allocate storage for the PREPARE_COMMUNICATION's options.";
        goto error;
    }
    NGEM_ASSERT(protocol->ngp_prepareCommOpts == NULL); 
    protocol->ngp_prepareCommOpts = pCommOpts;

    nResult = NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "request_id",  ngemOptionAnalyzerSetInt, &pCommOpts->ngpo_requestID, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(bool, analyzer,
            "tcp_nodelay",  ngemOptionAnalyzerSetBool, &pCommOpts->ngpo_tcpNodelay,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryCount",  ngemOptionAnalyzerSetInt, &pCommOpts->ngpo_retryInfo.ngcri_count,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryBaseInterval",  ngemOptionAnalyzerSetInt, &pCommOpts->ngpo_retryInfo.ngcri_interval,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(double, analyzer,
            "tcp_connect_retryIncreaseRatio",  ngemOptionAnalyzerSetDouble, &pCommOpts->ngpo_retryInfo.ngcri_increase, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryRandom",  ngemOptionAnalyzerSetIbool, &pCommOpts->ngpo_retryInfo.ngcri_useRandom,   1, 1);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't set options to analyzer.";
        goto error;
    }

    nResult = protocol->ngp_info->ngpi_prepareCommunicationBegin(protocol, pCommOpts, analyzer);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't start to process PREPARE_COMMUNICATION.";
        goto error;
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "%s\n", message);
    nResult = ngemReplySetError(arg->ngra_reply, message);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't set error to the reply.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}


/**
 * Callback function after receiving PREPARE_COMMUNICATION request.
 */
static ngemResult_t
ngcclPrepareCommunicationEnd(
    ngemRequestFunctionArgument_t *arg)
{
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message = NULL;
    ngccProtocol_t *protocol = (ngccProtocol_t *)arg->ngra_userData;
    ngccPrepareCommunicationOptions_t *pCommOpts = NULL;
    NGEM_FNAME(ngcclPrepareCommunicationEnd);

    log = ngemLogGetDefault();
    reply = arg->ngra_reply;

    pCommOpts = protocol->ngp_prepareCommOpts;

    if (reply->ngr_result == NGEM_SUCCESS) {
        /* Register Protocol's List */
        nResult = ngcclProtocolRegisterPrepareCommunicationOptions(
            protocol, pCommOpts);
        if (nResult != NGEM_SUCCESS) {
            message = "Can't register prepare communication options to the protocol.";
            goto error;
        }
    } else {
        goto error;
    }

    return NGEM_SUCCESS;
error:
    if (message != NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "%s\n", message);
        nResult = ngemReplySetError(reply, message);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
                "Can't set error to the reply.\n");
            return NGEM_FAILED;
        }
    }

    return NGEM_SUCCESS;
}

static ngemResult_t
ngcclPrepareCommunicationAfterReply(
    ngemRequestFunctionArgument_t *arg)
{
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message = NULL;
    ngccProtocol_t *protocol = (ngccProtocol_t *)arg->ngra_userData;
    ngccPrepareCommunicationOptions_t *pCommOpts = NULL;
    NGEM_FNAME(ngcclPrepareCommunicationAfterReply);

    /* Reset current PREPARE_COMMUNICATION Options */
    pCommOpts = protocol->ngp_prepareCommOpts;
    protocol->ngp_prepareCommOpts = NULL;
    reply = arg->ngra_reply;

    if (ngemReplyGetError(reply) == NGEM_SUCCESS) {
        /* Callback user's function */
        nResult = protocol->ngp_info->ngpi_prepareCommunicationEnd(
            protocol, pCommOpts, reply, ngemReplyGetError(reply));
        if (nResult != NGEM_SUCCESS) {
            message = "Can't process PREPARE_COMMUNICATION.";
            goto error;
        }
    } else {
        nResult = ngcclPrepareCommunicationOptionsDestroy(pCommOpts);
        if (nResult != NGEM_SUCCESS) {
            message = "Can't destroy PREPARE_COMMUNICATION options.";
            goto error;
        }
    }

    return NGEM_SUCCESS;
error:
    ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName, "%s\n", message);

    return NGEM_SUCCESS;
}

/**
 * Callback function after sending PREPARE_COMMUNICATION reply.
 */
static ngemResult_t
ngcclProtocolRegisterPrepareCommunicationOptions(
    ngccProtocol_t *protocol,
    ngccPrepareCommunicationOptions_t *pCommOpts)
{
    ngLog_t *log = NULL;
    int requestID;
    ngemResult_t nResult;
    NGEM_LIST_ITERATOR_OF(ngccPrepareCommunicationOptions_t) it;
    ngccPrepareCommunicationOptions_t *val;
    NGEM_FNAME(ngcclProtocolRegisterPrepareCommunicationOptions);

    log = ngemLogGetDefault();

    /* Check ID */
    requestID = pCommOpts->ngpo_requestID;
    if (requestID <= 0) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Invalid request ID \"%d\".\n", requestID);
        goto error;
    }

    NGEM_LIST_FOREACH(ngccPrepareCommunicationOptions_t,
        &protocol->ngp_prepareCommOptsList, it, val) {
        if (val->ngpo_requestID == requestID) {
            ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
                "Request ID \"%d\" is already used.\n", requestID);
            goto error;
        }
    }

    nResult = NGEM_LIST_INSERT_TAIL(ngccPrepareCommunicationOptions_t,
        &protocol->ngp_prepareCommOptsList, pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't insert new PrepareCommunicationInfo to list.\n");
        goto error;
    }

    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}

/**
 * INITIALIZE request's options: Create
 */
static ngccInitializeOptions_t *
ngcclInitializeOptionsCreate(void)
{
    ngccInitializeOptions_t *opts = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcclInitializeOptionsCreate);

    log = ngemLogGetDefault();

    opts = NGI_ALLOCATE(ngccInitializeOptions_t, log, NULL);
    if (opts == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for InitializeOptions.\n");
        return NULL;
    }

    opts->ngio_clientPort = 0U;

    return opts;
}

/**
 * INITIALIZE request's options: Destroy
 */
static void
ngcclInitializeOptionsDestroy(
    ngccInitializeOptions_t *opts)
{
    ngLog_t *log;
    NGEM_FNAME_TAG(ngcclInitializeOptionsDestroy);

    if (opts == NULL) {
        return ;
    }

    log = ngemLogGetDefault();
    
    ngcclInitializeOptionsReset(opts);
    NGI_DEALLOCATE(ngccInitializeOptions_t, opts, log, NULL);

    return;
}

/**
 * INITIALIZE request's options: reset
 */
static void
ngcclInitializeOptionsReset(
    ngccInitializeOptions_t *opts)
{
    ngLog_t *log;
    NGEM_FNAME_TAG(ngcclInitializeOptionsReset);

    NGEM_ASSERT(opts != NULL);

    log = ngemLogGetDefault();

    opts->ngio_clientPort = 0U;

    return;
}

/**
 * PREPARE_COMMUNICATION's options: Create
 */
static ngccPrepareCommunicationOptions_t *
ngcclPrepareCommunicationOptionsCreate(void)
{
    ngccPrepareCommunicationOptions_t *pCommOpts = NULL;
    ngLog_t *log = NULL;
    ngemNotify_t *notify = NULL;
    ngemResult_t nResult;
    int result;
    NGEM_FNAME(ngcclPrepareCommunicationOptionsCreate);

    log = ngemLogGetDefault();

    pCommOpts = NGI_ALLOCATE(ngccPrepareCommunicationOptions_t, log, NULL);
    if (pCommOpts == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for the PREPARE_COMMUNICATION's options.\n");
        goto error;
    }

    notify = ngemNotifyCreate("COMMUNICATION_REPLY", true);
    if (notify == NULL) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't create COMMUNICATION_REPLY notify.\n");
        goto error;
    }

    pCommOpts->ngpo_requestID  = -1;
    pCommOpts->ngpo_userData   = NULL;
    pCommOpts->ngpo_notify     = notify;
    pCommOpts->ngpo_result     = NGEM_SUCCESS;
    pCommOpts->ngpo_message    = NULL;
    pCommOpts->ngpo_tcpNodelay = false;
    pCommOpts->ngpo_retryInfoInitialized = false;

    result = ngiConnectRetryInformationInitialize(
        &pCommOpts->ngpo_retryInfo, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't initialize connect retry information.\n");
        goto error;
    }
    pCommOpts->ngpo_retryInfoInitialized = true;

    return pCommOpts;
error:

    nResult = ngcclPrepareCommunicationOptionsDestroy(pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't destroy PREPARE_COMMUNICATION's options.\n");
    }

    return NULL;
}

/**
 * PREPARE_COMMUNICATION's options: Destroy
 */
static ngemResult_t
ngcclPrepareCommunicationOptionsDestroy(
    ngccPrepareCommunicationOptions_t *pCommOpts)
{
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    int result;
    NGEM_FNAME(ngcclPrepareCommunicationOptionsDestroy);

    log = ngemLogGetDefault();

    if (pCommOpts == NULL) {
        return NGEM_SUCCESS;
    }

    ngemNotifyDestroy(pCommOpts->ngpo_notify);
    ngiFree(pCommOpts->ngpo_message, log, NULL);

    if (pCommOpts->ngpo_retryInfoInitialized) {
        result = ngiConnectRetryInformationFinalize(
            &pCommOpts->ngpo_retryInfo, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
                "Can't finalize connect retry information.\n");
            ret = NGEM_FAILED;
        }
    }

    pCommOpts->ngpo_requestID = 0;
    pCommOpts->ngpo_userData  = NULL;
    pCommOpts->ngpo_notify    = NULL;
    pCommOpts->ngpo_tcpNodelay = false;
    pCommOpts->ngpo_retryInfoInitialized = false;

    result = NGI_DEALLOCATE(ngccPrepareCommunicationOptions_t,
        pCommOpts, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_PROTOCOL, fName,
            "Can't free storage for the PREPARE_COMMUNICATION's options.\n");
        ret = NGEM_FAILED;
    }
    return ret;
}
