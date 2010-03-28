/*
 * $RCSfile: ngrcProtocol.c,v $ $Revision: 1.12 $ $Date: 2008/03/28 03:52:30 $
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
#include "ngrcProtocol.h"

NGI_RCSID_EMBED("$RCSfile: ngrcProtocol.c,v $ $Revision: 1.12 $ $Date: 2008/03/28 03:52:30 $")

static ngemResult_t INITIALIZE_begin(ngemRequestFunctionArgument_t *);
static ngemResult_t INITIALIZE_end(ngemRequestFunctionArgument_t *);

static ngrcInitializeOptions_t *ngrclInitializeOptionsCreate(void);
static void ngrclInitializeOptionsDestroy(ngrcInitializeOptions_t *);
static void ngrclInitializeOptionsReset(ngrcInitializeOptions_t *);

/* Requests */
static ngemRequestInformation_t ngrclProtocolInitializeRequest = {
    "INITIALIZE",
    0,
    true,
    INITIALIZE_begin,
    INITIALIZE_end,
    NULL
};

/**
 * Remote Communication Proxy Protocol: Create
 */
ngrcProtocol_t *
ngrcProtocolCreate(
    ngrcProtocolActions_t *actions,
    void *userData)
{
    ngrcProtocol_t *protocol = NULL;
    ngemProtocol_t *baseProtocol = NULL;
    ngrcInitializeOptions_t *opts = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngrcProtocolCreate);

    log = ngemLogGetDefault();

    protocol = NGI_ALLOCATE(ngrcProtocol_t, log, NULL);
    if (protocol == NULL) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName, "Can't allocate storage for the protocol.\n");
        goto error;
    }

    baseProtocol = ngemProtocolCreate((void *)protocol, "\r\n",
        NGCP_PROTOCOL_VERSION_MAJOR, NGCP_PROTOCOL_VERSION_MINOR);
    if (baseProtocol == NULL) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName, "Can't create the Base Protocol Manager.\n");
        goto error;
    }

    nResult =
        ngemProtocolAppendRequestInfo(baseProtocol, &ngrclProtocolInitializeRequest)         &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolQueryFeaturesRequest)       &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolExitRequest);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
            "Can't append the request information.\n");
        goto error;
    }

    opts = ngrclInitializeOptionsCreate();
    if (opts == NULL) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName, "Can't create the options container.\n");
        goto error;
    }

    protocol->ngp_protocol           = baseProtocol;
    protocol->ngp_actions            = actions;
    protocol->ngp_options            = opts;
    protocol->ngp_initialized        = false;
    protocol->ngp_userData           = userData;

    return protocol;
error:
    nResult = ngrcProtocolDestroy(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
            "Can't destroy the protocol.\n");
    }
    return NULL;
}

/**
 * Remote Communication Proxy Protocol: Destroy
 */
ngemResult_t
ngrcProtocolDestroy(
    ngrcProtocol_t *protocol)
{
    ngLog_t *log = NULL;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngrcProtocolDestroy);

    log = ngemLogGetDefault();

    if (protocol == NULL) {
        return ret;
    }

    result = ngemProtocolDestroy(protocol->ngp_protocol);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
            "Can't destroy the Base Protocol Manager.\n");
        ret = NGEM_FAILED;
    }
    protocol->ngp_protocol = NULL;

    ngrclInitializeOptionsDestroy(protocol->ngp_options);
    protocol->ngp_options = NULL;

    protocol->ngp_protocol = NULL;
    protocol->ngp_actions  = NULL;
    protocol->ngp_options  = NULL;

    NGI_DEALLOCATE(ngrcProtocol_t, protocol, log, NULL);

    return ret;
}

static ngemResult_t
INITIALIZE_begin(
    ngemRequestFunctionArgument_t *arg)
{
    ngemOptionAnalyzer_t *analyzer = arg->ngra_analyzer;
    ngrcProtocol_t *protocol = (ngrcProtocol_t *)arg->ngra_userData;
    ngLog_t *log = NULL;
    ngrcInitializeOptions_t *opts = NULL;
    ngemResult_t nResult;
    char *message = NULL;
    NGEM_FNAME(INITIALIZE_begin);

    log = ngemLogGetDefault();

    if (protocol->ngp_initialized) {
        message = "Communication Proxy is already initialized.";
        goto error;
    }
    protocol->ngp_initialized = true;

    opts = protocol->ngp_options;
    ngrclInitializeOptionsReset(opts);
    nResult =
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, analyzer,
            "hostname",  ngemOptionAnalyzerSetString, &opts->ngio_clientHostname, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "buffer_size",  ngemOptionAnalyzerSetInt, &opts->ngio_bufferSize, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(bool, analyzer,
            "tcp_nodelay",  ngemOptionAnalyzerSetBool, &opts->ngio_tcpNodelay, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryCount",  ngemOptionAnalyzerSetInt, &opts->ngio_retryInfo.ngcri_count,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryBaseInterval",  ngemOptionAnalyzerSetInt, &opts->ngio_retryInfo.ngcri_interval,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(double, analyzer,
            "tcp_connect_retryIncreaseRatio",  ngemOptionAnalyzerSetDouble, &opts->ngio_retryInfo.ngcri_increase, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(int, analyzer,
            "tcp_connect_retryRandom",  ngemOptionAnalyzerSetIbool, &opts->ngio_retryInfo.ngcri_useRandom,   1, 1);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't set options to analyzer.";
        goto error;
    }

    nResult = protocol->ngp_actions->ngpa_initializeBegin(protocol, analyzer);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't start to process INITIALIZE.";
        goto error;
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName, "%s\n", message);
    ngemReplySetError(arg->ngra_reply, message);

    return NGEM_SUCCESS;
}

static ngemResult_t
INITIALIZE_end(
    ngemRequestFunctionArgument_t *arg)
{
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message = NULL;
    char *address = NULL;
    ngrcProtocol_t *protocol = (ngrcProtocol_t *)arg->ngra_userData;
    NGEM_FNAME(INITIALIZE_end);

    log = ngemLogGetDefault();
    reply = arg->ngra_reply;

    if (ngemReplyGetError(reply) == NGEM_SUCCESS) {

        ngemReplySetMultiLine(reply, true);

        nResult = protocol->ngp_actions->ngpa_initializeEnd(
            protocol, protocol->ngp_options,
            &address, ngemReplyGetError(reply));
        if ((nResult != NGEM_SUCCESS) || (address == NULL)) {
            message = "Can't process INITIALIZE.";
            goto error;
        }

        NGEM_ASSERT(address != NULL);
        nResult = ngemReplyAddOption(reply, "address", "%s", address);
        if (nResult != NGEM_SUCCESS) {
            message = "Can't add option to reply.";
            goto error;
        }
        ngiFree(address, log, NULL);
        address = NULL;
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName, "%s\n", message);
    ngemReplySetError(reply, message);
    ngiFree(address, log, NULL);

    return NGEM_SUCCESS;
}

static ngrcInitializeOptions_t *
ngrclInitializeOptionsCreate(void)
{
    ngrcInitializeOptions_t *opts = NULL;
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngrclInitializeOptionsCreate);

    log = ngemLogGetDefault();

    opts = NGI_ALLOCATE(ngrcInitializeOptions_t, log, NULL);
    if (opts == NULL) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for InitializeOptions.\n");
        goto error;
    }

    opts->ngio_clientHostname = NULL;
    opts->ngio_bufferSize = (8 * (1024));
    opts->ngio_tcpNodelay = false;
    opts->ngio_retryInfoInitialized = false;

    result = ngiConnectRetryInformationInitialize(
        &opts->ngio_retryInfo, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
            "Can't initialize connect retry information.\n");
        goto error;
    }
    opts->ngio_retryInfoInitialized = true;

    return opts;
error:
    ngrclInitializeOptionsDestroy(opts);
    opts = NULL;

    return NULL;
}

static void
ngrclInitializeOptionsDestroy(
    ngrcInitializeOptions_t *opts)
{
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngrclInitializeOptionsDestroy);

    if (opts == NULL) {
        return;
    }

    log = ngemLogGetDefault();

    ngrclInitializeOptionsReset(opts);
    
    if (opts->ngio_retryInfoInitialized) {
        result = ngiConnectRetryInformationFinalize(
            &opts->ngio_retryInfo, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_PROTOCOL, fName,
                "Can't finalize connect retry information.\n");
        }
        opts->ngio_retryInfoInitialized = false;
    }

    NGI_DEALLOCATE(ngrcInitializeOptions_t, opts, log, NULL);

    return;
}

static void
ngrclInitializeOptionsReset(
    ngrcInitializeOptions_t *opts)
{
    ngLog_t *log;
    NGEM_FNAME_TAG(ngrclInitializeOptionsReset);

    NGEM_ASSERT(opts != NULL);

    log = ngemLogGetDefault();

    ngiFree(opts->ngio_clientHostname, log, NULL);
    opts->ngio_clientHostname = NULL;
    opts->ngio_bufferSize = 0;
    opts->ngio_tcpNodelay = false;
    opts->ngio_retryInfo.ngcri_count = 0;
    opts->ngio_retryInfo.ngcri_interval = 0;
    opts->ngio_retryInfo.ngcri_increase = 0.0;
    opts->ngio_retryInfo.ngcri_useRandom = 0;

    return;
}

