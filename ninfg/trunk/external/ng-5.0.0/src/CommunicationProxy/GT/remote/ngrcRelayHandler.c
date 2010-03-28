/*
 * $RCSfile: ngrcRelayHandler.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $
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

#include "ngrcGT.h"

NGI_RCSID_EMBED("$RCSfile: ngrcRelayHandler.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $")

static ngemResult_t ngrclRelayHandlerSendQueryFeatures(ngrcRelayHandler_t *);
static ngemResult_t ngrclRelayHandlerSendInitialize(
    ngrcRelayHandler_t *, ngcpOptions_t *, ngrcInitializeOptions_t *iOpts, char *, bool, char **);

ngrcRelayHandler_t *
ngrcRelayHandlerCreate(
    ngcpOptions_t *opts,
    ngrcInitializeOptions_t *iOpts,
    char *clientRelayHost,
    bool clientRelayCrypt)
{
    ngrcRelayHandler_t *relayHandler = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngrcRelayHandlerCreate);

    log = ngemLogGetDefault();

    relayHandler = NGI_ALLOCATE(ngrcRelayHandler_t, log, NULL);
    if (relayHandler == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't allocate storage for the relay handler.\n");
        goto error;
    }

    relayHandler->ngrh_lock = NGCP_COMMON_LOCK_NULL;
    relayHandler->ngrh_relayHandler = NULL;

    nResult = ngcpCommonLockInitialize(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't initialize the lock.\n");
        goto error;
    }

    relayHandler->ngrh_relayHandler = ngcpRelayHandlerCreate(
        opts, "ng_remote_relay.GT", &relayHandler->ngrh_lock);
    if (relayHandler->ngrh_relayHandler == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create a new relay handler.\n");
        goto error;
    }
    nResult = ngrclRelayHandlerSendQueryFeatures(relayHandler);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't send QUERY_FEATURES.\n");
        goto error;
    }

    nResult = ngrclRelayHandlerSendInitialize(
        relayHandler, opts, iOpts, clientRelayHost,
        clientRelayCrypt, &relayHandler->ngrh_address);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't send QUERY_FEATURES.\n");
        goto error;
    }

    return relayHandler;
error:
    ngrcRelayHandlerDestroy(relayHandler);
    relayHandler = NULL;

    return NULL;
}

void
ngrcRelayHandlerDestroy(
    ngrcRelayHandler_t *relayHandler)
{
    ngLog_t *log;
    ngemResult_t nResult;
    bool locked = false;
    NGEM_FNAME(ngrcRelayHandlerDestroy);

    log = ngemLogGetDefault();

    if (relayHandler == NULL) {
        /* Do nothing */
        return;
    }

    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't lock the the Client Relay Handler.\n");
    } else {
        locked = true;
    }

    if (relayHandler->ngrh_relayHandler != NULL) {
        nResult = ngcpRelayHandlerSendExit(relayHandler->ngrh_relayHandler);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't send EXIT request.\n");
        }
    }

    ngcpRelayHandlerDestroy(relayHandler->ngrh_relayHandler);

    if (locked) {
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't lock the the Client Relay Handler.\n");
        }
        locked = false;
    }

    nResult = ngcpCommonLockFinalize(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't finalize the relay handler.\n");
    }


    ngiFree(relayHandler->ngrh_address, log, NULL);

    relayHandler->ngrh_relayHandler  = NULL;
    relayHandler->ngrh_lock          = NGCP_COMMON_LOCK_NULL;
    relayHandler->ngrh_address       = NULL;

    nResult = NGI_DEALLOCATE(ngrcRelayHandler_t, relayHandler, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't deallocate storage for the relay handler.\n");
    }

    return;
}

/**
 * Relay Handler: Send QUERY_FEATURES requests
 */
static ngemResult_t
ngrclRelayHandlerSendQueryFeatures(
    ngrcRelayHandler_t *relayHandler)
{
    static char *features[] = {NULL};
    static char *requests[] = {
        "QUERY_FEATURES", "INITIALIZE", "EXIT", NULL};

    return ngcpRelayHandlerSendQueryFeatures(relayHandler->ngrh_relayHandler, features, requests);
}

/**
 * Relay Handler: Send INITIALIZE requests
 */
static ngemResult_t
ngrclRelayHandlerSendInitialize(
    ngrcRelayHandler_t *relayHandler,
    ngcpOptions_t *opts,
    ngrcInitializeOptions_t *iOpts,
    char *clientRelayHost,
    bool clientRelayCrypt,
    char **address)
{
    ngemResult_t nResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    ngcpRequest_t *req = NULL;
    ngemOptionAnalyzer_t *oa = NULL;
    NGEM_FNAME(ngrclRelayHandlerSendInitialize);

    log = ngemLogGetDefault();

    *address = NULL;

    req = ngcpRequestCreate("INITIALIZE", true, true);
    if (req == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create the request.\n");
        goto finalize;
    }

    nResult =
        ngcpRequestAppendArgument(req, "%s %s",
            NGCP_OPTION_CONTACT_STRING, opts->ngo_contactString) &&
        ngcpRequestAppendArgument(req, "%s %s",
            NGCP_OPTION_COMMUNICATION_SECURITY,
            ngcpCommunicationSecurityString[opts->ngo_communicationSecurity]) &&
        ngcpRequestAppendArgument(req, "hostname %s", iOpts->ngio_clientHostname) &&
        ngcpRequestAppendArgument(req, "buffer_size %d", iOpts->ngio_bufferSize) &&
        ngcpRequestAppendArgument(req, "tcp_nodelay %s", iOpts->ngio_tcpNodelay?"true":"false") &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryCount %d", iOpts->ngio_retryInfo.ngcri_count) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryBaseInterval %d", iOpts->ngio_retryInfo.ngcri_interval) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryIncreaseRatio %lf", iOpts->ngio_retryInfo.ngcri_increase) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryRandom %s", iOpts->ngio_retryInfo.ngcri_count?"true":"false") &&
        ngcpRequestAppendArgument(req, "%s true", NGRC_OPTION_REMOTE_RELAY);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't set options to the request.\n");
        goto finalize;
    }
    if (clientRelayHost != NULL) {
        nResult = 
            ngcpRequestAppendArgument(req, "%s %s", NGCP_OPTION_CLIENT_RELAY_HOST, clientRelayHost) &&
            ngcpRequestAppendArgument(req, "%s %s", NGCP_OPTION_CLIENT_RELAY_CRYPT, clientRelayCrypt?"true":"false");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't set options to the request.\n");
        goto finalize;
    }
    }

    nResult = ngcpRelayHandlerSendRequest(relayHandler->ngrh_relayHandler, req);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't send the request.\n");
        goto finalize;
    }

    if (req->ngr_result != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Reply result is \"failed: %s\".\n",
            req->ngr_replyMessage);
        goto finalize;
    }

    oa = ngemOptionAnalyzerCreate();
    if (oa == NULL) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't create option analyzer.\n");
        goto finalize;
    }

    /* Setting */
    nResult = 
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, oa,
            "address", ngemOptionAnalyzerSetString, address, 1, 1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't set action to the options analyzer.\n");
        goto finalize;
    }
    nResult = ngemOptionAnalyzerAnalyzeList(oa, &req->ngr_replyArguments);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't analyzer the line list.\n");
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    ngemOptionAnalyzerDestroy(oa);
    oa = NULL;
    ngcpRequestDestroy(req);
    req = NULL;

    return ret;
}
