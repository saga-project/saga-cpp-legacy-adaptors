/*
 * $RCSfile: ngcpRelayHandler.c,v $ $Revision: 1.5 $ $Date: 2008/03/28 05:12:17 $
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

#include "ngcpRelayHandler.h"

NGI_RCSID_EMBED("$RCSfile: ngcpRelayHandler.c,v $ $Revision: 1.5 $ $Date: 2008/03/28 05:12:17 $")

/**
 * Features
 */
typedef struct ngcplFeatures_s {
    char               *ngf_version;
    NGEM_LIST_OF(char)  ngf_requests;
    NGEM_LIST_OF(char)  ngf_features;
} ngcplFeatures_t;

/* Line Parser */
static ngcpLineParser_t *ngcplLineParserCreate(void);
static void ngcplLineParserDestroy(ngcpLineParser_t *);
static ngemResult_t ngcplLineParserPut(ngcpLineParser_t *, char *, size_t);
static char *ngcplLineParserGet(ngcpLineParser_t *);

static void ngcplNotifyInfoDestroy(ngcpNotifyInfo_t *);

static ngemResult_t ngcplRequestAppendReplyLine(ngcpRequest_t *, const char *, bool *);

static ngcpNotify_t *ngcplNotifyCreate(
    NGEM_LIST_OF(ngcpNotifyInfo_t) *, const char *);
static ngemResult_t ngcplNotifyAppendLine(ngcpNotify_t *, const char *, bool *);

static void ngcplNotifyDestroy(ngcpNotify_t *);

static void ngcplRelayHandlerHeaderReadCallback(
    globus_xio_handle_t,
    globus_result_t,
    globus_byte_t *,
    globus_size_t,
    globus_size_t,
    globus_xio_data_descriptor_t,
    void *);
static void ngcplRelayHandlerDataReadCallback(
    globus_xio_handle_t,
    globus_result_t,
    globus_byte_t *,
    globus_size_t,
    globus_size_t,
    globus_xio_data_descriptor_t,
    void *);
static void ngcplRelayHandlerCancelCallback(
    globus_xio_handle_t,
    globus_result_t,
    void *);
static void ngcplRelayHandlerWriteCallback(
    globus_xio_handle_t,
    globus_result_t,
    globus_byte_t *,
    globus_size_t,
    globus_size_t,
    globus_xio_data_descriptor_t,
    void *);

static ngemResult_t ngcplRelayHandlerHandleReply(ngcpRelayHandler_t *, const char *);
static ngemResult_t ngcplRelayHandlerHandleNotify(ngcpRelayHandler_t *, const char *);

static void ngcplFeaturesInitialize(ngcplFeatures_t *);
static void ngcplFeaturesFinalize(ngcplFeatures_t *);
static ngemResult_t ngcplFeaturesCheckValue(ngcplFeatures_t *, char **, char **);

/**
 * Request: Create
 */
ngcpRequest_t *
ngcpRequestCreate(
    const char *name,
    bool multiLine,
    bool multiLineReply)
{
    ngcpRequest_t *req = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcpRequestCreate);

    log = ngemLogGetDefault();

    req = NGI_ALLOCATE(ngcpRequest_t, log, NULL);
    if (req == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the request.\n");
        goto error;
    }

    /* Request part */
    req->ngr_name      = NULL;
    req->ngr_params    = NULL;
    req->ngr_multiLine = multiLine;
    NGEM_LIST_SET_INVALID_VALUE(&req->ngr_arguments);

    /* Reply part */
    req->ngr_multiLineReply = multiLineReply;
    req->ngr_setReply       = false;
    req->ngr_completedReply = false;
    req->ngr_result         = NGEM_FAILED;
    req->ngr_replyParams    = NULL;
    req->ngr_replyMessage   = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&req->ngr_replyArguments);

    NGEM_LIST_INITIALIZE(char, &req->ngr_arguments);
    NGEM_LIST_INITIALIZE(char, &req->ngr_replyArguments);

    req->ngr_name = ngiStrdup(name, log, NULL);
    if (req->ngr_name == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't duplicate the string.\n");
        goto error;
    }

    return req;
error:
    ngcpRequestDestroy(req);

    return NULL;
}

/**
 * Request: Destroy
 */
void
ngcpRequestDestroy(
    ngcpRequest_t *req)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    ngLog_t *log;
    NGEM_FNAME_TAG(ngcpRequestDestroy);

    log = ngemLogGetDefault();

    if (req == NULL) {
        return;
    }

    NGEM_LIST_FOREACH(char, &req->ngr_arguments, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &req->ngr_arguments);

    NGEM_LIST_FOREACH(char, &req->ngr_replyArguments, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &req->ngr_replyArguments);

    ngiFree(req->ngr_name, log, NULL);
    ngiFree(req->ngr_params, log, NULL);
    ngiFree(req->ngr_replyMessage, log, NULL);
    ngiFree(req->ngr_replyParams, log, NULL);

    req->ngr_name         = NULL;
    req->ngr_params       = NULL;
    req->ngr_replyMessage = NULL;
    req->ngr_replyParams  = NULL;

    NGI_DEALLOCATE(ngcpRequest_t, req, log, NULL);

    return;
}

/**
 * Request: Append argument
 */
ngemResult_t
ngcpRequestAppendArgument(
    ngcpRequest_t *req,
    const char *fmt, ...)
{
    char *ret = NULL;
    va_list ap;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngcpRequestAppendArgument);

    NGEM_ASSERT(req != NULL);
    NGEM_ASSERT(fmt != NULL);

    log = ngemLogGetDefault();

    va_start(ap, fmt);
    ret = ngiStrdupVprintf(fmt, ap, log, NULL);
    va_end(ap);
    if (ret == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't duplicate the string.\n");
        goto error;
    }
    nResult = NGEM_LIST_INSERT_TAIL(char, &req->ngr_arguments, ret);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't insert new item to arguments in the request.\n");
        goto error;
    }

    return NGEM_SUCCESS;
error:

    ngiFree(ret, log, NULL);
    return NGEM_FAILED;
}

/**
 * Request: Append reply line.
 */
static ngemResult_t 
ngcplRequestAppendReplyLine(
    ngcpRequest_t *request,
    const char *line,
    bool *endp)
{
    char *p = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    char *rest = NULL;
    int i;
    ngemResult_t replyResult;
    NGEM_FNAME(ngcplRequestAppendReplyLine);

    NGEM_ASSERT(request != NULL);
    NGEM_ASSERT(endp != NULL);

    log = ngemLogGetDefault();

    if (request->ngr_setReply) {
        /* Options or END */
        NGEM_ASSERT(!request->ngr_completedReply);
        NGEM_ASSERT(request->ngr_multiLineReply);
        NGEM_ASSERT(request->ngr_result == NGEM_SUCCESS);

        if (strcmp(line, "REPLY_END") == 0) {
            request->ngr_completedReply = true;
        } else {
            p = ngiStrdup(line, log, NULL);
            if (p == NULL) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't duplicate the string\n");
                goto error;
            }
            nResult = NGEM_LIST_INSERT_TAIL(char,
                    &request->ngr_replyArguments, p);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't insert the string to list.\n");
                goto error;
            }
            p = NULL;
        }
    } else {
        /* First Line */
        NGEM_ASSERT(!request->ngr_completedReply);
        NGEM_ASSERT(request->ngr_result != NGEM_SUCCESS);

        /* Get length of first word */
        for (i = 0;(line[i] != '\0') && (!isspace((int)line[i]));++i);

        if (ngemStringCompare("S", -1, line, i)) {
            replyResult = NGEM_SUCCESS;
            if (request->ngr_multiLineReply) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Expects multi line reply."
                    " but signal line reply is received.\n");
                goto error;
            }
            request->ngr_completedReply = true;;
        } else
        if (ngemStringCompare("SM", -1, line, i)) {
            replyResult = NGEM_SUCCESS;
            if (!request->ngr_multiLineReply) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Expects signal line reply."
                    " but multi line reply is received.\n");
                goto error;
            }
        } else 
        if (ngemStringCompare("F", -1, line, i)) {
            request->ngr_completedReply = true;;
            replyResult = NGEM_FAILED;
        } else {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Invalid reply: %s\n", line);
            goto error;
        }
        request->ngr_setReply = true;

        /* Skip space */
        for (;(line[i] != '\0') && (isspace((int)line[i]));++i);
        if (strlen(&line[i]) > 0) {
            rest = ngiStrdup(&line[i], log, NULL);
            if (rest == NULL) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't duplicate the string.\n");
                goto error;
            }
        }

        request->ngr_result = replyResult;
        if (replyResult == NGEM_SUCCESS) {
            request->ngr_replyParams  = rest;
        } else {
            request->ngr_replyMessage = rest;
        }
    }

    *endp = request->ngr_completedReply;

    return NGEM_SUCCESS;
error:
    ngiFree(p, log, NULL);    p    = NULL;
    ngiFree(rest, log, NULL); rest = NULL;

    return NGEM_FAILED;
}


/**
 * Relay Handler: Create
 */
ngcpRelayHandler_t *
ngcpRelayHandlerCreate(
    ngcpOptions_t *opts,
    char *relayFile,
    ngcpCommonLock_t *lock)
{
    ngcpRelayHandler_t * relayHandler = NULL;
    ngLog_t *log;
    globus_result_t gResult;
    char *address = NULL;
    char *contactString = NULL;
    ngemResult_t nResult;
    bool self;
    ngcpCommunicationSecurity_t sec = NGCP_COMMUNICATION_SECURITY_NONE;
    NGEM_FNAME(ngcpRelayHandlerCreate);

    log = ngemLogGetDefault();

    relayHandler = NGI_ALLOCATE(ngcpRelayHandler_t, log, NULL);
    if (relayHandler == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the relay handler.\n");
        goto error;
    }

    relayHandler->ngrh_crypt          = opts->ngo_relayCrypt;
    relayHandler->ngrh_handle         = NULL;
    relayHandler->ngrh_lpReply        = NULL;
    relayHandler->ngrh_lpNotify       = NULL;
    relayHandler->ngrh_written        = true;
    relayHandler->ngrh_sentRequest    = NULL;
    relayHandler->ngrh_replyReceived  = false;
    relayHandler->ngrh_receivedNotify = NULL;
    relayHandler->ngrh_notifyReceived = false;
    relayHandler->ngrh_lock           = lock;
    relayHandler->ngrh_rest           = 0;
    relayHandler->ngrh_callbackType   = NGR_PACK_TYPE_REPLY;
    relayHandler->ngrh_available      = false;
    relayHandler->ngrh_gsisshInfo     = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&relayHandler->ngrh_notifyInfos);

    NGEM_LIST_INITIALIZE(ngcpNotifyInfo_t, &relayHandler->ngrh_notifyInfos);

    relayHandler->ngrh_lpReply = ngcplLineParserCreate();
    if (relayHandler->ngrh_lpReply == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't create the line parser for reply.\n");
        goto error;
    }

    relayHandler->ngrh_lpNotify = ngcplLineParserCreate();
    if (relayHandler->ngrh_lpReply == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't create the line parser for notify.\n");
        goto error;
    }

    if (relayHandler->ngrh_crypt) {
        sec = NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY;
    } 

    if (opts->ngo_relayInvokeMethod == NGCP_RELAY_INVOKE_METHOD_GSI_SSH) {
        relayHandler->ngrh_gsisshInfo = ngcpGSISSHinvoke(
            opts->ngo_relayHost, opts->ngo_relayGSISSHcommand,
            &opts->ngo_relayGSISSHoptions, relayFile, &opts->ngo_relayOptions,
            sec, relayHandler->ngrh_lock);
        if (relayHandler->ngrh_gsisshInfo == NULL) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't invoke the communication relay.\n");
            goto error;
        }
        contactString = relayHandler->ngrh_gsisshInfo->nggs_address;
    } else {
        contactString = opts->ngo_relayHost;
    }

    /* Connect */
    nResult = ngcpGlobusXIOconnect(&relayHandler->ngrh_handle,
        contactString, 
        sec, NGCP_COMMUNICATION_AUTH_SELF | NGCP_COMMUNICATION_AUTH_HOST, true);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't connect the Client Relay.\n");
        goto error;
    }

    if (sec != NGCP_COMMUNICATION_SECURITY_NONE)
    {
        nResult = ngcpGlobusXIOpeerNameIsSelf(relayHandler->ngrh_handle, &self);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't connect the Client Relay.\n");
            goto error;
        }
        if (!self) {
            nResult = ngcpGlobusXIOinitDelegation(relayHandler->ngrh_handle);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't init delegation.\n");
                goto error;
            }
        }
    }

    relayHandler->ngrh_available = true;
    gResult = globus_xio_register_read(
        relayHandler->ngrh_handle, (void *)relayHandler->ngrh_header,
        NGR_PACK_HEADER_SIZE, NGR_PACK_HEADER_SIZE, NULL,
        ngcplRelayHandlerHeaderReadCallback, relayHandler);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "globus_xio_register_read", gResult);

        relayHandler->ngrh_available = false;
        relayHandler->ngrh_handle = NULL;
        goto error;
    }

    ngiFree(address, log, NULL);
    address = NULL;
    return relayHandler;
error:

    ngiFree(address, log, NULL);
    address = NULL;

    ngcpRelayHandlerDestroy(relayHandler);

    return NULL;
}

/**
 * Relay Handler: Destroy
 */
void
ngcpRelayHandlerDestroy(
    ngcpRelayHandler_t *relayHandler)
{
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(ngcpNotifyInfo_t) it;
    ngcpNotifyInfo_t *val;
    ngemResult_t nResult;
    bool locked = false;
    globus_result_t gResult;
    NGEM_FNAME(ngcpRelayHandlerDestroy);

    log = ngemLogGetDefault();

    if (relayHandler == NULL) {
        return;
    }

    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
    } else {
        locked = true;
    }
    nResult = ngcpCommonLockSetFinalize(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't set finalize flags.\n");
    }

    if (relayHandler->ngrh_gsisshInfo != NULL) {
        ngcpGSISSHinfoDestroy(relayHandler->ngrh_gsisshInfo);
        relayHandler->ngrh_gsisshInfo = NULL;
    }

    if (relayHandler->ngrh_handle != NULL) {
        gResult = globus_xio_register_close(
           relayHandler->ngrh_handle, NULL, 
           ngcplRelayHandlerCancelCallback, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "globus_xio_register_close", gResult);
        }
        while (relayHandler->ngrh_available) {
            nResult = ngcpCommonLockWait(relayHandler->ngrh_lock);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't wait the relay handler.\n");
                break;
            }
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
        }
    }

    NGEM_LIST_FOREACH(ngcpNotifyInfo_t, &relayHandler->ngrh_notifyInfos, it, val) {
        ngcplNotifyInfoDestroy(val);
    }

    ngcpRequestDestroy(relayHandler->ngrh_sentRequest);
    relayHandler->ngrh_sentRequest = NULL;
    relayHandler->ngrh_replyReceived = false;
    ngcplNotifyDestroy(relayHandler->ngrh_receivedNotify);
    relayHandler->ngrh_receivedNotify = NULL;

    ngcplLineParserDestroy(relayHandler->ngrh_lpReply);
    relayHandler->ngrh_lpReply = NULL;
    ngcplLineParserDestroy(relayHandler->ngrh_lpNotify);
    relayHandler->ngrh_lpNotify = NULL;

    relayHandler->ngrh_crypt = false;

    NGEM_LIST_FINALIZE(ngcpNotifyInfo_t, &relayHandler->ngrh_notifyInfos);
    NGI_DEALLOCATE(ngcpRelayHandler_t, relayHandler, log, NULL);

    return;
}

/**
 * Relay Handler: Send request.
 */
ngemResult_t
ngcpRelayHandlerSendRequest(
    ngcpRelayHandler_t *relayHandler,
    ngcpRequest_t *request)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    ngemResult_t nResult;
    globus_result_t gResult;
    ngLog_t *log;
    ngemStringBuffer_t sBuf = NGEM_STRING_BUFFER_NULL;
    char *sendData;
    bool locked = false;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME(ngcpRelayHandlerSendRequest);

    NGEM_ASSERT(relayHandler != NULL);
    NGEM_ASSERT(request != NULL);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called.\n");

    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    if ((!relayHandler->ngrh_available) ||
        (NGCP_COMMON_LOCK_FINALIZING(relayHandler->ngrh_lock))) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Relay handler is not available.\n");
        goto finalize;
    }

    /* Wait other request */
    while ((relayHandler->ngrh_sentRequest != NULL) &&
           (relayHandler->ngrh_available) &&
           (!NGCP_COMMON_LOCK_FINALIZING(relayHandler->ngrh_lock))) {
        nResult = ngcpCommonLockWait(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't wait the signal for relay handler.\n");
            goto finalize;
        }
    }

    if ((relayHandler->ngrh_available) &&
        (!NGCP_COMMON_LOCK_FINALIZING(relayHandler->ngrh_lock))) {

        nResult = ngemStringBufferInitialize(&sBuf);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't initialize the string buffer.\n");
            goto finalize;
        }
        nResult = NGEM_SUCCESS;
        nResult = nResult &&
            ngemStringBufferFormat(&sBuf, "%s\r\n", request->ngr_name);
        if (request->ngr_multiLine) {
            NGEM_LIST_FOREACH(char, &request->ngr_arguments, it, val) {
                nResult = nResult &&
                    ngemStringBufferFormat(&sBuf, "%s\r\n", val);
            }
            nResult = nResult &&
                ngemStringBufferFormat(&sBuf, "%s_END\r\n", request->ngr_name);
        }
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't append the string the buffer.\n");
            goto finalize;
        }
        sendData = ngemStringBufferGet(&sBuf);

        relayHandler->ngrh_written       = false;
        relayHandler->ngrh_replyReceived = false;

        ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "register write. %p\n", relayHandler);

        gResult = globus_xio_register_write(relayHandler->ngrh_handle,
            (void *)sendData, strlen(sendData), strlen(sendData),
            NULL, ngcplRelayHandlerWriteCallback, relayHandler);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "globus_xio_register_write", gResult);
            goto finalize;
        }

        NGEM_ASSERT(relayHandler->ngrh_sentRequest == NULL);
        relayHandler->ngrh_sentRequest = request;

        while (((!relayHandler->ngrh_written) ||
                (!relayHandler->ngrh_replyReceived))      &&
               (relayHandler->ngrh_available)             &&
               (!NGCP_COMMON_LOCK_FINALIZING(relayHandler->ngrh_lock))) {
            nResult = ngcpCommonLockWait(relayHandler->ngrh_lock);
            if (nResult != NGEM_SUCCESS) {
                ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                    "Can't wait the signal for relay handler.\n");
                goto finalize;
            }
        }
    }

    if (NGCP_COMMON_LOCK_FINALIZING(relayHandler->ngrh_lock)) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Relay handler is finalized.\n");
        goto finalize;
    }

    if (!relayHandler->ngrh_available) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Error is occurred in callback function.\n");
        goto finalize;
    }
    NGEM_ASSERT(relayHandler->ngrh_sentRequest != NULL);
    ret = NGEM_SUCCESS;
finalize:
    relayHandler->ngrh_sentRequest = NULL;
    nResult = ngcpCommonLockBroadcast(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't broadcast the signal.\n");
        ret = NGEM_FAILED;
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    ngemStringBufferFinalize(&sBuf);

    return ret;
}

/**
 * Relay Handler: Register Notify
 */
ngemResult_t
ngcpRelayHandlerRegisterNotify(
    ngcpRelayHandler_t *relayHandler,
    const char *name,
    bool multiLine,
    ngcpNotifyHandler_t handler,
    void *userData)
{
    ngcpNotifyInfo_t *nInfo = NULL;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;;
    ngemResult_t nResult;
    char *nc = NULL;
    bool locked = true;
    NGEM_FNAME(ngcpRelayHandlerRegisterNotify);

    log = ngemLogGetDefault();

    /* Lock */
    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    nc = ngiStrdup(name, log, NULL);
    if (nc == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't duplicate the string.\n");
        goto finalize;
    }

    nInfo = NGI_ALLOCATE(ngcpNotifyInfo_t, log, NULL);
    if (nInfo == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the notify information.\n");
        goto finalize;
    }
    nInfo->ngni_multiLine = multiLine;
    nInfo->ngni_handler   = handler;
    nInfo->ngni_userData  = userData;
    nInfo->ngni_name      = nc;

    nResult = NGEM_LIST_INSERT_TAIL(ngcpNotifyInfo_t,
        &relayHandler->ngrh_notifyInfos, nInfo);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't insert the information to list.\n");
        goto finalize;
    }
    nInfo = NULL;
    nc = NULL;

    ret = NGEM_SUCCESS;
finalize:
    ngiFree(nc, log, NULL);
    NGI_DEALLOCATE(ngcpNotifyInfo_t, nInfo, log, NULL);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 * Line Parser: Create
 */
static ngcpLineParser_t *
ngcplLineParserCreate(void)
{
    ngcpLineParser_t *lp = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcplLineParserCreate);

    log = ngemLogGetDefault();

    lp = NGI_ALLOCATE(ngcpLineParser_t, log, NULL);
    if (lp == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the line parser.\n");
        goto error;
    }

    lp->nglp_buffer   = NULL;
    lp->nglp_head     = 0;
    lp->nglp_tail     = 0;
    lp->nglp_capacity = 0;
    lp->nglp_eof      = false;

    lp->nglp_buffer = ngiMalloc(NGCP_RELAY_HANDLER_BUFFER_SIZE, log, NULL);
    if (lp->nglp_buffer == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the buffer.\n");
        goto error;
    }
    lp->nglp_capacity = NGCP_RELAY_HANDLER_BUFFER_SIZE;

    return lp;
error:
    ngcplLineParserDestroy(lp);
    return NULL;
}

/**
 * Line Parser: Destroy
 */
static void
ngcplLineParserDestroy(
    ngcpLineParser_t *lp)
{
    ngLog_t *log;

    log = ngemLogGetDefault();

    if (lp == NULL) {
        return;
    }

    ngiFree(lp->nglp_buffer, log, NULL);
    lp->nglp_buffer   = NULL;
    lp->nglp_capacity = 0;
    lp->nglp_head     = 0;
    lp->nglp_tail     = 0;
    lp->nglp_eof      = false;

    NGI_DEALLOCATE(ngcpLineParser_t, lp, log, NULL);

    return;
}

/**
 * Line Parser: Put new characters
 */
static ngemResult_t
ngcplLineParserPut(
    ngcpLineParser_t *lp,
    char *data,
    size_t dataSize)
{
    int i;
    ngLog_t *log;
    size_t req;
    char *tmp;
    NGEM_FNAME(ngcplLineParserPut);

    log = ngemLogGetDefault();

    if (dataSize == 0) {
        return NGEM_SUCCESS;
    }

    /* Checks "data" to include '\0'.*/
    for (i = 0;i < dataSize;++i) {
        if (data[i] == '\0') {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "\"Data\" includes a null-character.\n");
            return NGEM_FAILED;
        }
    }

    /* Slides string*/
    if (lp->nglp_head > 0) {
        NGEM_ASSERT(lp->nglp_head <= lp->nglp_tail);
        for (i = 0;lp->nglp_head < lp->nglp_tail;++lp->nglp_head, ++i) {
            lp->nglp_buffer[i] = lp->nglp_buffer[lp->nglp_head];
        }
        lp->nglp_head = 0;
        lp->nglp_tail = i;
    }

    /* Grow Buffer */
    req = lp->nglp_tail + dataSize + 1;
    if (req > lp->nglp_capacity) {
        tmp = ngiRealloc(lp->nglp_buffer, req, log, NULL); 
        if (tmp == NULL) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't allocate storage for the buffer.\n");
            return NGEM_FAILED;
        }
        lp->nglp_buffer   = tmp;
        lp->nglp_capacity = req;
    }

    /* Copy */
    for (i = 0;i < dataSize;++i) {
        lp->nglp_buffer[lp->nglp_tail+i] = data[i];
    }
    lp->nglp_tail += i;
    lp->nglp_buffer[lp->nglp_tail] = '\0';

    return NGEM_SUCCESS;
}

/**
 * Line Parser: Get line.
 */
static char *
ngcplLineParserGet(
    ngcpLineParser_t *lp)
{
    char *ret = NULL;
    char *p;

    NGEM_ASSERT(lp->nglp_buffer[lp->nglp_tail] == '\0');

    ret = &lp->nglp_buffer[lp->nglp_head];

    p = strstr(ret, "\r\n");
    if (p != NULL) {
        *p = '\0';
        lp->nglp_head = (p + strlen("\r\n")) - lp->nglp_buffer;
        NGEM_ASSERT(lp->nglp_head <= lp->nglp_tail);
    } else if (lp->nglp_eof) {
        lp->nglp_head = lp->nglp_tail;
    } else {
        ret = NULL;
    }

    return ret;
}

/**
 * Notify Information: Destroy
 */
static void
ngcplNotifyInfoDestroy(
    ngcpNotifyInfo_t *nInfo)
{
    ngLog_t *log;

    log = ngemLogGetDefault();

    if (nInfo == NULL) {
        return;
    }

    ngiFree(nInfo->ngni_name, log, NULL);
    nInfo->ngni_name      = NULL;
    nInfo->ngni_multiLine = false;
    nInfo->ngni_handler   = NULL;
    nInfo->ngni_userData  = NULL;

    NGI_DEALLOCATE(ngcpNotifyInfo_t, nInfo, log, NULL);

    return;
}

/**
 * Notify: Create
 */
static ngcpNotify_t *
ngcplNotifyCreate(
    NGEM_LIST_OF(ngcpNotifyInfo_t) *nInfo,
    const char *line)
{
    NGEM_LIST_ITERATOR_OF(ngcpNotifyInfo_t) it;
    ngcpNotifyInfo_t *val;
    ngcpNotifyInfo_t *target = NULL;
    ngcpNotify_t *notify = NULL;
    int i;
    ngLog_t *log;
    char *rest = NULL;
    NGEM_FNAME(ngcplNotifyCreate);

    NGEM_ASSERT(nInfo != NULL);
    NGEM_ASSERT(line != NULL);

    log = ngemLogGetDefault();

    /* Parse Line */
    for (i = 0;line[i] != '\0';++i) {
        if (isspace((int)line[i])) {
            break;
        }
    }

    NGEM_LIST_FOREACH(ngcpNotifyInfo_t, nInfo, it, val) {
        if (ngemStringCompare(val->ngni_name, -1, line, i)) {
            target = val;
            break;
        }
    }

    if (target == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Unkown notify: \"%s\"\n", line);
        return NULL;
    }

    for (;line[i] != '\0';++i) {
        if (!isspace((int)line[i])) {
            break;
        }
    }
    if (strlen(&line[i]) > 0) {
        rest = ngiStrdup(&line[i], log, NULL);
        if (rest == NULL) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
    }

    notify = NGI_ALLOCATE(ngcpNotify_t, log, NULL);
    if (notify == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't allocate storage for the notify.\n");
        goto error;
    }
    notify->ngn_info   = target;
    notify->ngn_params = rest;
    rest = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&notify->ngn_arguments);
    if (target->ngni_multiLine) {
        NGEM_LIST_INITIALIZE(char, &notify->ngn_arguments);
    }

     ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
         "Notify Create %p\n", notify);

    return notify;
error:
    ngiFree(rest, log, NULL);

    return NULL;
}

/**
 * Notify: Append line.
 */
static ngemResult_t
ngcplNotifyAppendLine(
    ngcpNotify_t *notify,
    const char *line,
    bool *endp)
{
    char *p = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    char *name;
    size_t len;
    NGEM_FNAME(ngcplNotifyAppendLine);

    log = ngemLogGetDefault();

    NGEM_ASSERT(notify != NULL);
    NGEM_ASSERT(notify->ngn_info != NULL);
    NGEM_ASSERT(notify->ngn_info->ngni_multiLine);
    NGEM_ASSERT(endp != NULL);

    name = notify->ngn_info->ngni_name;
    len  = strlen(name);

    if ((strncmp(line, name, len) == 0) &&
        (strcmp(line + len, "_END") == 0)) {
        *endp = true;
        return NGEM_SUCCESS;
    } else {
        *endp = false;
        p = ngiStrdup(line, log, NULL);
        if (p == NULL) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't duplicate the string.\n");
            goto error;
        }
        nResult = NGEM_LIST_INSERT_TAIL(char,
                &notify->ngn_arguments, p);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't insert the string to list.\n");
            goto error;
        }
        p = NULL;
    }

    return NGEM_SUCCESS;
error:
    ngiFree(p, log, NULL);
    p = NULL;

    return NGEM_FAILED;
}

/**
 * Notify: Destroy
 */
static void
ngcplNotifyDestroy(
    ngcpNotify_t *notify)
{
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    NGEM_FNAME(ngcplNotifyDestroy);

    log = ngemLogGetDefault();

     ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
         "Notify Destroy %p\n", notify);

    if (notify == NULL) {
        return;
    }

    if (notify->ngn_info->ngni_multiLine) {
        NGEM_LIST_FOREACH(char, &notify->ngn_arguments, it, val) {
            NGEM_ASSERT(val != NULL);
            ngiFree(val, log, NULL);
        }
        NGEM_LIST_FINALIZE(char, &notify->ngn_arguments);
    }

    ngiFree(notify->ngn_params, log, NULL);
    NGI_DEALLOCATE(ngcpNotify_t, notify, log, NULL);

    return;
}

/**
 * Relay handler: Callback function for reading packing header.
 */
static void
ngcplRelayHandlerHeaderReadCallback(
    globus_xio_handle_t handle,
    globus_result_t cResult,
    globus_byte_t *buffer,
    globus_size_t bufferSize,
    globus_size_t nRead,
    globus_xio_data_descriptor_t desc,
    void *arg)
{
    ngcpRelayHandler_t *relayHandler = arg;
    ngLog_t *log;
    globus_result_t gResult;
    ngemResult_t nResult;
    uint32_t *packHeader = (uint32_t *)buffer;
    uint32_t type;
    uint32_t size;
    bool cont = false;
    bool locked = false;
    NGEM_FNAME(ngcplRelayHandlerHeaderReadCallback);

    log = ngemLogGetDefault();
    
    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called with %p.\n", arg);

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "Callback is canceled\n");
             goto finalize;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
             ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "EOF\n");
         } else {
             ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "Callback function for reading", cResult);
         }
         goto finalize;
    }

    NGEM_ASSERT(relayHandler->ngrh_handle == handle);
    NGEM_ASSERT(nRead == NGR_PACK_HEADER_SIZE);

    type = ntohl(packHeader[0]);
    size = ntohl(packHeader[1]);

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Pack header [type=%u, size=%u].\n", type, size);

    if ((type != NGR_PACK_TYPE_REPLY) && (type != NGR_PACK_TYPE_NOTIFY)) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Invalid pack type(0x%X).\n", type);
        goto finalize;
    }

    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    relayHandler->ngrh_callbackType = type;

    NGEM_ASSERT(relayHandler->ngrh_rest == 0);
    if (size > NGCP_RELAY_HANDLER_BUFFER_SIZE) {
        relayHandler->ngrh_rest = size - NGCP_RELAY_HANDLER_BUFFER_SIZE;
        size = NGCP_RELAY_HANDLER_BUFFER_SIZE;
    }

    gResult = globus_xio_register_read(handle,
        (void *)relayHandler->ngrh_buffer, size, size,
        NULL, ngcplRelayHandlerDataReadCallback, relayHandler);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "globus_xio_register_read", gResult);
        goto finalize;
    }

    cont = true;
finalize:
    if (!locked) {
        nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't lock the relay handler.\n");
        }
        locked = true;
    }
    if (!cont) {
        relayHandler->ngrh_available = false;
        nResult = ngcpCommonLockBroadcast(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't broadcast the signal.\n");
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
        }
    }
    return;
}

/**
 * Relay Handler: Callback function for reading reply or notify.
 */
static void
ngcplRelayHandlerDataReadCallback(
    globus_xio_handle_t handle,
    globus_result_t cResult,
    globus_byte_t *buffer,
    globus_size_t bufferSize,
    globus_size_t nRead,
    globus_xio_data_descriptor_t desc,
    void *arg)
{
    ngcpRelayHandler_t *relayHandler = arg;
    ngLog_t *log;
    ssize_t size;
    globus_result_t gResult;
    ngemResult_t nResult;
    char *line;
    bool locked = false;
    ngemResult_t (*dataHandler)(ngcpRelayHandler_t *, const char *);
    ngcpLineParser_t *lp;
    bool cont = false;
    NGEM_FNAME(ngcplRelayHandlerDataReadCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called with %p.\n", arg);

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
             ;
         } else {
             ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "Callback function for reading", cResult);
         }
         goto finalize;
    }
    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    NGEM_ASSERT(relayHandler->ngrh_handle == handle);
    dataHandler = NULL;
    lp = NULL;
    switch (relayHandler->ngrh_callbackType) {
    case NGR_PACK_TYPE_REPLY:
        lp = relayHandler->ngrh_lpReply;
        dataHandler = ngcplRelayHandlerHandleReply;
        break;
    case NGR_PACK_TYPE_NOTIFY:
        dataHandler = ngcplRelayHandlerHandleNotify;
        lp = relayHandler->ngrh_lpNotify;
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    nResult = ngcplLineParserPut(lp, (char *)buffer, nRead);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't parse line.\n");
        goto finalize;
    }

    while ((line = ngcplLineParserGet(lp)) != NULL) {
        nResult = dataHandler(relayHandler, line);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't handle the line.\n");
            goto finalize;
        }
    }

    if (relayHandler->ngrh_rest > 0) {
        size = relayHandler->ngrh_rest;
        if (size > NGCP_RELAY_HANDLER_BUFFER_SIZE) {
            relayHandler->ngrh_rest = size - NGCP_RELAY_HANDLER_BUFFER_SIZE;
            size = NGCP_RELAY_HANDLER_BUFFER_SIZE;
        } else {
            relayHandler->ngrh_rest = 0;
        }
        gResult = globus_xio_register_read(
            handle, (void *)relayHandler->ngrh_buffer,
            size, size, NULL,
            ngcplRelayHandlerDataReadCallback, relayHandler);
    } else {
        gResult = globus_xio_register_read(
            handle, (void *)relayHandler->ngrh_header,
            NGR_PACK_HEADER_SIZE, NGR_PACK_HEADER_SIZE, NULL,
            ngcplRelayHandlerHeaderReadCallback, relayHandler);
    }
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "globus_xio_register_read", gResult);
        goto finalize;
    }

    cont = true;
finalize:
    if (!locked) {
        nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't lock the relay handler.\n");
        }
        locked = true;
    }
    if (!cont) {
        relayHandler->ngrh_available = false;
        nResult = ngcpCommonLockBroadcast(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't broadcast the signal.\n");
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
        }
    }
    return;
}

static void
ngcplRelayHandlerWriteCallback(
    globus_xio_handle_t handle,
    globus_result_t cResult,
    globus_byte_t *buffer,
    globus_size_t bufferSize,
    globus_size_t nWrite,
    globus_xio_data_descriptor_t desc,
    void *arg)
{
    bool locked = false;
    bool error = false;
    ngcpRelayHandler_t *relayHandler = arg;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngcplRelayHandlerWriteCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called with %p.\n", arg);

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
             NGEM_ASSERT_NOTREACHED();
         } else {
             ngcpLogGlobusError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "Callback function for reading", cResult);
         }
         error = true;
    }

    if (!error) {
        if (bufferSize != nWrite) {
             ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                 "Can't write data(wants write %lu byte but wrote %lu bytes).\n", 
                 (unsigned long)bufferSize, (unsigned long)nWrite);
             error = true;
        }
    }

    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't lock the relay handler.\n");
    } else {
        locked = true;
    }

    if (error) {
        relayHandler->ngrh_available = false;
    } else {
        relayHandler->ngrh_written = true;
    }

    if (locked) {
        nResult = ngcpCommonLockBroadcast(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't broadcast the signal.\n");
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't unlock the relay handler.\n");
        }
    }
    return;
}

/**
 * Relay Handler: Callback for canceling
 */
static void
ngcplRelayHandlerCancelCallback(
    globus_xio_handle_t handle,
    globus_result_t nResult,
    void *userData)
{
    /* Do nothing */
    return;
}

/**
 * Relay Handler: Handle the reply
 * WARN: Must be locked when called
 */
static ngemResult_t
ngcplRelayHandlerHandleReply(
    ngcpRelayHandler_t *relayHandler,
    const char *line)
{
    ngLog_t *log;
    bool end = false;
    ngemResult_t nResult;
    ngcpRequest_t *req = NULL;
    NGEM_FNAME(ngcplRelayHandlerHandleReply);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called with line: \"%s\".\n", line);

    /* Handling Reply */

    req = relayHandler->ngrh_sentRequest; 
    if (req == NULL) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Invalid data. Now, does not wait the reply.\n");
        goto error;
    }

    nResult = ngcplRequestAppendReplyLine(req, line, &end);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
            "Can't parse the reply.\n");
        goto error;
    }

    if (end) {
        ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName, "Reply end.\n");
        relayHandler->ngrh_replyReceived = true;
        nResult = ngcpCommonLockBroadcast(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't broadcast the signal.\n");
            goto error;
        }
    }

    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}

/**
 * Relay Handler: Handle the notify
 */
static ngemResult_t
ngcplRelayHandlerHandleNotify(
    ngcpRelayHandler_t *relayHandler,
    const char *line)
{
    bool end = false;
    ngcpNotify_t *notify = NULL;
    ngcpNotifyInfo_t *nInfo = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcplRelayHandlerHandleNotify);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
        "Called with line: \"%s\".\n", line);

    notify = relayHandler->ngrh_receivedNotify; 
    if (notify == NULL) {
        /* First Line */
        notify = ngcplNotifyCreate(&relayHandler->ngrh_notifyInfos, line);
        if (notify == NULL) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't create the notify.\n");
            goto error;
        }
        relayHandler->ngrh_receivedNotify = notify;
        nInfo = notify->ngn_info;
        if (!nInfo->ngni_multiLine) {
            end = true;
        }

    } else {
        nResult = ngcplNotifyAppendLine(notify, line, &end);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_RELAY_HANDLER, fName,
                "Can't append line to the notify.\n");
            goto error;
        }
    }
    nInfo = notify->ngn_info;
    if (end) {
        nInfo->ngni_handler(relayHandler, notify, nInfo->ngni_userData);
        ngcplNotifyDestroy(notify);
        relayHandler->ngrh_receivedNotify = NULL;
    }

    return NGEM_SUCCESS;
error:
    return NGEM_FAILED;
}

/**
 * Relay Handler: Send QUERY_FEATURES requests
 */
ngemResult_t
ngcpRelayHandlerSendQueryFeatures(
    ngcpRelayHandler_t *relayHandler,
    char **requiredFeatures,
    char **requiredRequests)
{
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    ngemResult_t nResult;
    bool locked = false;
    ngemOptionAnalyzer_t *oa = NULL;
    ngcpRequest_t *req = NULL;
    ngcplFeatures_t features;
    NGEM_FNAME(ngcpRelayHandlerSendQueryFeatures);

    log = ngemLogGetDefault();

    ngcplFeaturesInitialize(&features);

    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    req = ngcpRequestCreate("QUERY_FEATURES", false, true);
    if (req == NULL) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't create the request.\n");
        goto finalize;
    }

    nResult = ngcpRelayHandlerSendRequest(relayHandler, req);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't send the request.\n");
        goto finalize;
    }
    if (req->ngr_result != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
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
            "protocol_version", ngemOptionAnalyzerSetString, &features.ngf_version, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(NGEM_LIST_OF(char), oa,
            "feature", ngemOptionAnalyzerSetStringList, &features.ngf_features, 0, -1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(NGEM_LIST_OF(char), oa,
            "request", ngemOptionAnalyzerSetStringList, &features.ngf_requests, 0, -1);
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
    /* Check reply */
    nResult = ngcplFeaturesCheckValue(&features, requiredFeatures, requiredRequests);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Features check is failed.\n");
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    if (oa != NULL) {
        ngemOptionAnalyzerDestroy(oa);
    }

    ngcpRequestDestroy(req);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult == 0) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    ngcplFeaturesFinalize(&features);

    return ret;
}

/**
 * Relay Handler: Send EXIT requests
 */
ngemResult_t
ngcpRelayHandlerSendExit(
    ngcpRelayHandler_t *relayHandler)
{
    ngemResult_t nResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    bool locked = false;
    ngcpRequest_t *req = NULL;
    NGEM_FNAME(ngcpRelayHandlerSendExit);

    log = ngemLogGetDefault();

    /* Lock */
    nResult = ngcpCommonLockLock(relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    req = ngcpRequestCreate("EXIT", false, false);
    if (req == NULL) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't create the request.\n");
        goto finalize;
    }

    nResult = ngcpRelayHandlerSendRequest(relayHandler, req);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't send the request.\n");
        goto finalize;
    }

    if (req->ngr_result != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Reply result is \"failed: %s\".\n",
            req->ngr_replyMessage);
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    ngcpRequestDestroy(req);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}


static void
ngcplFeaturesInitialize(
    ngcplFeatures_t *features)
{
    NGEM_ASSERT(features != NULL);

    features->ngf_version  = NULL;
    NGEM_LIST_INITIALIZE(char, &features->ngf_features);
    NGEM_LIST_INITIALIZE(char, &features->ngf_requests);

    return;
}

static void
ngcplFeaturesFinalize(
    ngcplFeatures_t *features)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    ngLog_t *log;

    NGEM_ASSERT(features != NULL);

    log = ngemLogGetDefault();

    ngiFree(features->ngf_version, log, NULL);
    features->ngf_version = NULL;

    NGEM_LIST_FOREACH(char, &features->ngf_features, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &features->ngf_features);

    NGEM_LIST_FOREACH(char, &features->ngf_requests, it, val) {
        ngiFree(val, log, NULL);
    }
    NGEM_LIST_FINALIZE(char, &features->ngf_requests);

    return;
}

static ngemResult_t
ngcplFeaturesCheckValue(
    ngcplFeatures_t *features,
    char **requiredFeatures,
    char **requiredRequests)
{
    ngLog_t *log;
    static const char *protocol_version = "1.0";
    int i;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    bool found;
    NGEM_FNAME(ngcplFeaturesCheckValue);

    NGEM_ASSERT(features != NULL);
    NGEM_ASSERT(features->ngf_version != NULL);

    log = ngemLogGetDefault();

    if (strcmp(features->ngf_version, protocol_version) != 0) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Invalid protocol version \"%s\".\n", features->ngf_version);
        return NGEM_FAILED;
    }

    for (i = 0;requiredFeatures[i] != NULL;i++) {
        found = false;
        NGEM_LIST_FOREACH(char, &features->ngf_features, it, val) {
            if (strcmp(val, requiredFeatures[i]) != 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Client Relay does not have \"%s\" feature. it is required\n",
                requiredFeatures[i]);
            return NGEM_FAILED;
        }
    }

    for (i = 0;requiredRequests[i] != NULL;i++) {
        found = false;
        NGEM_LIST_FOREACH(char, &features->ngf_requests, it, val) {
            if (strcmp(val, requiredRequests[i]) != 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Client Relay does not have \"%s\" requiredFeatures. it is required\n",
                requiredRequests[i]);
            return NGEM_FAILED;
        }
    }

    return NGEM_SUCCESS;
}

