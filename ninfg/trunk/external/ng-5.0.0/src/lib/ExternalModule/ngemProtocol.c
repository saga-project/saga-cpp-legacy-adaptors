/*
 * $RCSfile: ngemProtocol.c,v $ $Revision: 1.14 $ $Date: 2008/03/17 08:58:40 $
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

#include "ngemProtocol.h"
#include "ngemUtility.h"

NGI_RCSID_EMBED("$RCSfile: ngemProtocol.c,v $ $Revision: 1.14 $ $Date: 2008/03/17 08:58:40 $")

/**
 * @file 
 */

/*****************************************************************************/
/* File local functions declaration                                          */
/*****************************************************************************/

/* Protocol */
static void ngemlProtocolRequestCallback(void *, ngemLineBuffer_t *, char *, ngemCallbackResult_t);
static void ngemlProtocolRequestOptionsCallback(void *, ngemLineBuffer_t *, char *, ngemCallbackResult_t);
static ngemRequestFunctionArgument_t * ngemlProtocolAnalyzeRequest(ngemProtocol_t *, char *);
static void ngemlProtocolSendReplyCallback(void *, int, ngemCallbackResult_t);
static void ngemlProtocolReplyOptionCallback(void *, int, ngemCallbackResult_t);

/* Request Function Argument */
static ngemRequestFunctionArgument_t *ngemlRequestFunctionArgumentCreate(ngemProtocol_t *, char *);
static ngemResult_t ngemlRequestFunctionArgumentDestroy(ngemRequestFunctionArgument_t *);

/* Reply */
static ngemResult_t ngemlProtocolSendReply(ngemProtocol_t *, ngemReply_t *);
static ngemReply_t *ngemlReplyCreate(void);
static ngemResult_t ngemlReplyDestroy(ngemReply_t *);

/* Notify */
static ngemResult_t ngemlProtocolSendNotify(ngemProtocol_t *protocol);
static void ngemlProtocolSendNotifyCallback(void *, int, ngemCallbackResult_t);
static void ngemlProtocolNotifyOptionCallback(void *, int, ngemCallbackResult_t);

/* Other */
static ngemResult_t ngemlAddOption(NGEM_LIST_OF(char) *, const char *, const char *, va_list);
static ngemCallback_t ngemlProtocolSend(ngemProtocol_t *, int, const char *, NGEM_LIST_OF(char) *, ngemWriteStringCallbackFunc_t);

/*****************************************************************************/
/* Implementation                                                            */
/*****************************************************************************/

/**
 * Protocol: Create
 */
ngemProtocol_t *
ngemProtocolCreate(
    void *userData,
    const char *separator,
    int majorVersion,
    int minorVersion)
{
    ngemProtocol_t *new = NULL;
    ngemLineBuffer_t *lBuffer = NULL;
    ngemResult_t result;
    ngLog_t *log = NULL;
    char *sep = NULL;
    NGEM_FNAME(ngemProtocolCreate);

    NGEM_ASSERT_STRING(separator);
    NGEM_ASSERT(majorVersion >= 0);
    NGEM_ASSERT(minorVersion >= 0);

    log = ngemLogGetDefault();

    new = NGI_ALLOCATE(ngemProtocol_t, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for protocol.\n");
        goto error;
    }
    
    /* Initialize members */
    new->ngp_available         = true;
    new->ngp_stdio.ngsio_in    = 0;
    new->ngp_stdio.ngsio_out   = 1;
    new->ngp_stdio.ngsio_error = 2;
    new->ngp_lineBuffer        = NULL;
    new->ngp_separator         = NULL;

    new->ngp_version.ngpv_major = majorVersion;
    new->ngp_version.ngpv_minor = minorVersion;

    new->ngp_argument          = NULL;
    new->ngp_reply             = NULL;

    new->ngp_replyCallback     = NULL;
    new->ngp_notifyCallback    = NULL;
    new->ngp_userData          = userData; 
    NGEM_LIST_SET_INVALID_VALUE(&new->ngp_requestInfo);
    NGEM_LIST_SET_INVALID_VALUE(&new->ngp_features);
    NGEM_LIST_SET_INVALID_VALUE(&new->ngp_notifyQueue);

    NGEM_LIST_INITIALIZE(ngemRequestInformation_t, &new->ngp_requestInfo);
    NGEM_LIST_INITIALIZE(char, &new->ngp_features);
    NGEM_LIST_INITIALIZE(ngemNotify_t, &new->ngp_notifyQueue);

    sep = ngiStrdup(separator, log, NULL);
    if (sep == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't copy string.\n");
        goto error;
    }
    new->ngp_separator = sep;

    lBuffer = ngemLineBufferCreate(new->ngp_stdio.ngsio_in, sep);
    if (lBuffer == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, 
            "Can't create line buffer for receiving Invoke Server Protocol.\n");
        goto error;
    }
    new->ngp_lineBuffer = lBuffer;

    result = ngemLineBufferReadLine(
        lBuffer, ngemlProtocolRequestCallback, (void *)new);
    if (result == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't register for reading a line.\n");
        goto error;
    }
    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Creates new protocol.\n");
    
    return new;
error:

    NGEM_NULL_CHECK_AND_DESTROY(lBuffer, ngemLineBufferDestroy);
    ngiFree(sep, log, NULL);
    NGI_DEALLOCATE(ngemProtocol_t, new, log, NULL);

    return NULL;
}

/**
 * Protocol: Destroy
 */
ngemResult_t
ngemProtocolDestroy(
    ngemProtocol_t *protocol)
{
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngemProtocolDestroy);

    log = ngemLogGetDefault();

    if (protocol == NULL) {
        return NGEM_SUCCESS;
    }
   
    nResult = ngemProtocolDisable(protocol);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disable the protocol.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngemlRequestFunctionArgumentDestroy(protocol->ngp_argument);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't destroy the request function argument.\n");
        ret = NGEM_FAILED;
    }
    protocol->ngp_argument = NULL;

    /* Requests */
    if (!NGEM_LIST_IS_INVALID_VALUE(&protocol->ngp_requestInfo)) {
        NGEM_LIST_ITERATOR_OF(ngemRequestInformation_t) it;
        ngemRequestInformation_t *req = NULL;

        NGEM_LIST_ERASE_EACH(ngemRequestInformation_t, &protocol->ngp_requestInfo, it, req) {
            ngiFree(req->ngri_string, log, NULL);
            req->ngri_string        = NULL;
            req->ngri_multiLine     = false;
            req->ngri_nArguments    = 0;
            req->ngri_beginCallback = NULL;
            req->ngri_endCallback   = NULL;
            req->ngri_replyCallback = NULL;
            NGI_DEALLOCATE(ngemRequestInformation_t, req, log, NULL);
        }
        NGEM_LIST_FINALIZE(ngemRequestInformation_t, &protocol->ngp_requestInfo);
    }
    /* Features */
    if (!NGEM_LIST_IS_INVALID_VALUE(&protocol->ngp_features)) {
        NGEM_LIST_ITERATOR_OF(char) it;
        char *feature = NULL;

        NGEM_LIST_ERASE_EACH(char, &protocol->ngp_features, it, feature) {
            ngiFree(feature, log, NULL);
        }
        NGEM_LIST_FINALIZE(char, &protocol->ngp_features);
    }

    ngiFree(protocol->ngp_separator, log, NULL);

    protocol->ngp_available         = false;
    protocol->ngp_stdio.ngsio_in    = 0;
    protocol->ngp_stdio.ngsio_out   = 1;
    protocol->ngp_stdio.ngsio_error = 2;        
    protocol->ngp_separator         = NULL;

    protocol->ngp_version.ngpv_major = 0;
    protocol->ngp_version.ngpv_minor = 0;

    protocol->ngp_lineBuffer        = NULL;

    protocol->ngp_argument          = NULL;
    protocol->ngp_reply             = NULL;

    protocol->ngp_replyCallback     = NULL;
    protocol->ngp_notifyCallback    = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&protocol->ngp_notifyQueue);

    protocol->ngp_userData          = NULL;
    
    NGI_DEALLOCATE(ngemProtocol_t, protocol, log, NULL);
    
    return ret;
}

/**
 * Protocol: Disable
 */
ngemResult_t
ngemProtocolDisable(ngemProtocol_t *protocol)
{
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemResult_t nResult;
    NGEM_LIST_ITERATOR_OF(ngemNotify_t) it;
    ngemNotify_t *notify = NULL;
    int i;
    NGEM_FNAME(ngemProtocolDisable);

    NGEM_ASSERT(protocol != NULL);
    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Disables the protocol.\n");

    /* Unregister Source */
    ngemLineBufferDestroy(protocol->ngp_lineBuffer);
    protocol->ngp_lineBuffer = NULL;

    if (ngemCallbackIsValid(protocol->ngp_replyCallback)) {
        ngemCallbackCancel(protocol->ngp_replyCallback);
    }
    protocol->ngp_replyCallback = NULL;

    if (ngemCallbackIsValid(protocol->ngp_notifyCallback)) {
        ngemCallbackCancel(protocol->ngp_notifyCallback);
    }
    protocol->ngp_notifyCallback = NULL;

    nResult = ngemlReplyDestroy(protocol->ngp_reply);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't destroy the reply.\n");
        ret = NGEM_FAILED;
    }
    protocol->ngp_reply = NULL;

    /* Notify Queue */
    i = 0;
    if (!NGEM_LIST_IS_INVALID_VALUE(&protocol->ngp_notifyQueue)) {
        i++;
        NGEM_LIST_ERASE_EACH(ngemNotify_t, &protocol->ngp_notifyQueue, it, notify) {
            ngemNotifyDestroy(notify);
        }
        NGEM_LIST_FINALIZE(ngemNotify_t, &protocol->ngp_notifyQueue);
    }
    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "%d notifies is destroyed.\n", i);

    protocol->ngp_available = false;

    return ret;
}

/**
 * Protocol: append request
 */
ngemResult_t
ngemProtocolAppendRequestInfo(
    ngemProtocol_t *protocol,
    ngemRequestInformation_t *reqInfo)
{
    ngemRequestInformation_t *new = NULL;
    ngemResult_t nResult;
    char *string = NULL;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngemProtocolAppendRequestInfo);

    log = ngemLogGetDefault();

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT(reqInfo != NULL);

    new = NGI_ALLOCATE(ngemRequestInformation_t, log, NULL);
    if (new == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't allocate the storage for a new request.\n");
        goto error;
    }

    *new = *reqInfo;
    string  = ngiStrdup(reqInfo->ngri_string, log, NULL);
    if (string == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't allocate the storage for string.\n");
        goto error;
    }
    new->ngri_string = string;

    nResult = NGEM_LIST_INSERT_TAIL(
        ngemRequestInformation_t, &protocol->ngp_requestInfo, new);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append new request to requests list.\n");
        goto error;
    }

    return NGEM_SUCCESS;
error:
    ngiFree(string, log, NULL);
    ngiFree(new, log, NULL);

    return NGEM_FAILED;
}

/**
 * Protocol; Append feature (for QUERY_FEATURES)
 */
ngemResult_t
ngemProtocolAppendFeature(
    ngemProtocol_t *protocol,
    const char *feature)
{
    char *featureCopy = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngemProtocolAppendFeature);

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT_STRING(feature);

    log = ngemLogGetDefault();

    featureCopy = ngiStrdup(feature, log, NULL);
    if (featureCopy == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't copy string.\n");
        goto error;
    }

    nResult = NGEM_LIST_INSERT_TAIL(
        char, &protocol->ngp_features, featureCopy);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append new request to requests list.\n");
        goto error;
    }

    return NGEM_SUCCESS;
error:
    ngiFree(featureCopy, log, NULL);

    return NGEM_FAILED;
}

/**
 * Protocol: Callback function on receiving a request from the Ninf-G Client.
 */
static void
ngemlProtocolRequestCallback(
    void *arg,
    ngemLineBuffer_t *lBuffer,
    char *line,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t *protocol = (ngemProtocol_t *)arg;
    ngemRequestFunctionArgument_t *reqArg = NULL;
    ngemOptionAnalyzer_t *analyzer = NULL;
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message;
    NGEM_FNAME(ngemlProtocolRequestCallback);

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT(lBuffer != NULL);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't read the request of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_EOF:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "End of file.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    if (strlen(line) == 0) {
        /* Skip */
        nResult = ngemLineBufferReadLine(
            lBuffer, ngemlProtocolRequestCallback, (void *)protocol);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register for reading a line.\n");
            goto protocol_end;
        }
        return;
    }

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Read \"%s\".\n", line);

    reply = ngemlReplyCreate();
    if (reply == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't create the reply.\n");
        goto protocol_end;
    }
    NGEM_ASSERT(protocol->ngp_reply == NULL);
    protocol->ngp_reply = reply;
    
    /* Command */
    reqArg = ngemlProtocolAnalyzeRequest(protocol, line);
    if (reqArg == NULL) {
        message = "Unknown request";
        goto error;
    }
    NGEM_ASSERT(protocol->ngp_argument == NULL);

    protocol->ngp_argument = reqArg;
    reqArg->ngra_reply = reply;

    if (reqArg->ngra_requestInfo->ngri_multiLine) {
        analyzer = ngemOptionAnalyzerCreate();
        if (analyzer == NULL) {
            message = "Can't create option analyzer.";
            goto error;
        }
        reqArg->ngra_analyzer = analyzer;
    }

    if (reqArg->ngra_requestInfo->ngri_beginCallback != NULL) {
        nResult = reqArg->ngra_requestInfo->ngri_beginCallback(reqArg);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback function failed.\n");
            goto protocol_end;
        }
        if (!protocol->ngp_available) {
            return;
        }
    }

    if (reqArg->ngra_requestInfo->ngri_multiLine) {
        /* Register Callback for reading a line. */
        nResult = ngemLineBufferReadLine(
            lBuffer, ngemlProtocolRequestOptionsCallback,
            (void *)protocol);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register for reading a line.\n");
            goto protocol_end;
        }
        return;
    } else {
        /* Send Reply */
        nResult = ngemlProtocolSendReply(protocol, reply);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't send the reply.\n");
            goto protocol_end;
        }
    }
    return;
error:

    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s\n", message);
    ngemReplySetError(reply, message);
    nResult = ngemlProtocolSendReply(protocol, reply);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't send the reply.\n");
        goto protocol_end;
    }
    return;

protocol_end:

    nResult = ngemProtocolDisable(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
    }

    return;
}

/**
 * Protocol: Callback function on receiving a option from Ninf-G Client.
 */
static void
ngemlProtocolRequestOptionsCallback(
    void *userData,
    ngemLineBuffer_t *lBuffer,
    char *data,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t                *protocol = (ngemProtocol_t *)userData;
    ngemRequestFunctionArgument_t *reqArg   = protocol->ngp_argument;
    ngemOptionAnalyzer_t          *analyzer = reqArg->ngra_analyzer;
    char *reqName = reqArg->ngra_requestName;
    ngemResult_t nResult;
    ngLog_t *log = ngemLogGetDefault();
    char *message = NULL;
    char *msg = NULL;
    NGEM_FNAME(ngemlProtocolRequestOptionsCallback);

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't read the request option of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_EOF:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "End of file.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    if ((strncmp(reqName, data, strlen(reqName)) != 0) ||
        (strcmp("_END", data + strlen(reqName)) != 0)) {
        /* Option Analyze */
        nResult = ngemOptionAnalyzerAnalyzeLine(analyzer, data);
        if (nResult == NGEM_FAILED) {
            message = "Can't analyze options";
            ngemReplySetError(protocol->ngp_reply, message);
            /* Through */
        }

        /* Next */
        nResult = ngemLineBufferReadLine(
            lBuffer, ngemlProtocolRequestOptionsCallback,
            (void *)protocol);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register for reading a line.\n");
            goto protocol_end;
        }
        return;
    }
    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName,
        "Last line of REQUEST.\n");

    /* Request End */
    if (ngemReplyGetError(protocol->ngp_reply) != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Error occurs in processing the lines.\n");
        goto error;
    }

    nResult = ngemOptionAnalyzerAnalyzeEnd(analyzer, &msg);
    if (nResult == NGEM_FAILED) {
        message = "Error on end of options.";
        goto error;
    }

    if (msg != NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "%s", msg);
        ngemReplySetError(protocol->ngp_reply, msg);
        ngiFree(msg, log, NULL);
        goto error;
    }

    if (reqArg->ngra_requestInfo->ngri_endCallback != NULL) {
        nResult = reqArg->ngra_requestInfo->ngri_endCallback(reqArg);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback function failed.\n");
            goto protocol_end;
        }
        if (!protocol->ngp_available) {
            return;
        }
    }

    /* Send Reply */
    nResult = ngemlProtocolSendReply(protocol, protocol->ngp_argument->ngra_reply);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't send the reply.\n");
        goto protocol_end;
    }

    return;
error:
    if (message != NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s\n", message);
        ngemReplySetError(protocol->ngp_reply, message);
    }
    nResult = ngemlProtocolSendReply(protocol, protocol->ngp_argument->ngra_reply);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't send the reply.\n");
        goto protocol_end;
    }

    return;

protocol_end:
    nResult = ngemProtocolDisable(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
    }

    return;
}

/**
 * Protocol: Analyze Request of Invoke Server Protocol.
 */
static ngemRequestFunctionArgument_t *
ngemlProtocolAnalyzeRequest(
    ngemProtocol_t *protocol,
    char *line)
{
    NGEM_FNAME_TAG(ngemlProtocolAnalyzeRequest);

    return ngemlRequestFunctionArgumentCreate(protocol, line);
}

/**
 * Protocol: Send the reply.
 */
static ngemResult_t
ngemlProtocolSendReply(
    ngemProtocol_t *protocol,
    ngemReply_t *reply)
{
    ngemWriteStringCallbackFunc_t func = NULL;
    ngemCallback_t callback;
    ngLog_t *log = NULL;
    char *resultString = NULL;
    NGEM_FNAME(ngemlProtocolSendReply);

    log = ngemLogGetDefault();

    if (reply->ngr_result == NGEM_SUCCESS) {
        /* BEGIN */
        if (reply->ngr_multiLine) {
            resultString = "SM";
            func = ngemlProtocolReplyOptionCallback;
        } else {
            resultString = "S";
            func = ngemlProtocolSendReplyCallback;
        }

        callback = ngemlProtocolSend(
            protocol, protocol->ngp_stdio.ngsio_out,
            resultString, &reply->ngr_params, func);
    } else {
        callback = ngemCallbackWriteFormat(protocol->ngp_stdio.ngsio_out,
            ngemlProtocolSendReplyCallback, (void *)protocol,
            "F %s%s", reply->ngr_errorMessage, protocol->ngp_separator); 
    }
    if (ngemCallbackIsValid(callback) == 0) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register callback for sending reply.\n");
        return NGEM_FAILED;
    }
    protocol->ngp_replyCallback = callback;

    return NGEM_SUCCESS;
}

/**
 * Protocol: Callback function after sending the reply.
 */
static void
ngemlProtocolSendReplyCallback(
    void *userData,
    int   fd,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t *protocol = (ngemProtocol_t *)userData;
    ngLog_t *log = ngemLogGetDefault();
    ngemResult_t nResult;
    ngemRequestFunction_t replyCallback = NULL;
    NGEM_FNAME(ngemlProtocolSendReplyCallback);

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't send the reply of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    case NGEM_CALLBACK_RESULT_EOF:
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    if (protocol->ngp_argument != NULL) {
        replyCallback = protocol->ngp_argument->ngra_requestInfo->ngri_replyCallback;
        if (replyCallback != NULL) {
            nResult = replyCallback(protocol->ngp_argument);
            if (nResult != NGEM_SUCCESS) {
                goto protocol_end;
            }
            if (!protocol->ngp_available) {
                return;
            }
        }

        /* Destroy a argument */
        nResult = ngemlRequestFunctionArgumentDestroy(protocol->ngp_argument);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
                "Can't destroy request function argument.\n");
            /* Through */
        }
        protocol->ngp_argument = NULL;
    }

    nResult = ngemlReplyDestroy(protocol->ngp_reply);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't destroy reply.\n");
        /* Through */
    }
    protocol->ngp_reply = NULL;

    /* Register Callback for reading a line. */
    nResult = ngemLineBufferReadLine(protocol->ngp_lineBuffer,
        ngemlProtocolRequestCallback, (void *)protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register for reading a line.\n");
        goto protocol_end;
    }
    return;

protocol_end:
    nResult = ngemProtocolDisable(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
    }
    return;
}

/**
 * Protocol: Callback function after sending the option of reply.
 */
static void
ngemlProtocolReplyOptionCallback(
    void *userData,
    int   fd,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t *protocol = (ngemProtocol_t *)userData;
    ngLog_t *log = ngemLogGetDefault();
    ngemReply_t *reply;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *line = NULL;
    bool fatal = false;
    ngemWriteStringCallbackFunc_t func = NULL;
    ngemCallback_t callback = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngemlProtocolReplyOptionCallback);

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't send the reply option of Ninf-G Protocol.\n");
        fatal = true;
        goto end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    case NGEM_CALLBACK_RESULT_EOF:
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    reply= protocol->ngp_argument->ngra_reply;
    NGEM_ASSERT(reply->ngr_multiLine == true);

    if (NGEM_LIST_IS_EMPTY(char, &reply->ngr_options)) {
        /* End */
        line = strdup("REPLY_END");
        if (line == NULL) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't allocate storage for the string.\n");
            fatal = true;
            goto end;
        }
        func = ngemlProtocolSendReplyCallback;
    } else {
        it = NGEM_LIST_BEGIN(char, &reply->ngr_options);
        line = NGEM_LIST_GET(char, it);
        NGEM_ASSERT_STRING(line);
        NGEM_LIST_ERASE(char, it);
        func = ngemlProtocolReplyOptionCallback;
    }

    callback = ngemCallbackWriteFormat(protocol->ngp_stdio.ngsio_out,
        func, (void *)protocol, "%s%s", line, protocol->ngp_separator);
    if (ngemCallbackIsValid(callback) == 0) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register callback for sending the reply.\n");
        fatal = true;
        goto end;
    }
end:
    ngiFree(line, log, NULL);

    if (fatal) {
        nResult = ngemProtocolDisable(protocol);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
        }
    }
    return;
}

/**
 * Reply: Create the reply data
 */
static ngemReply_t *
ngemlReplyCreate(void)
{
    ngemReply_t *reply = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngemlReplyCreate);

    log = ngemLogGetDefault();
    
    reply = NGI_ALLOCATE(ngemReply_t, log, NULL);
    if (reply == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't allocate storage for a reply.\n");
        return NULL;
    }

    reply->ngr_result       = NGEM_SUCCESS;
    reply->ngr_multiLine    = false;
    reply->ngr_errorMessage = NULL;
    NGEM_LIST_SET_INVALID_VALUE(&reply->ngr_params);
    NGEM_LIST_SET_INVALID_VALUE(&reply->ngr_options);

    /* LIST */
    NGEM_LIST_INITIALIZE(char ,&reply->ngr_params);
    NGEM_LIST_INITIALIZE(char ,&reply->ngr_options);

    return reply;
}

/**
 * Reply: Destroy the reply data
 */
static ngemResult_t
ngemlReplyDestroy(ngemReply_t *reply)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *option;
    char *param;
    ngLog_t *log;
    NGEM_FNAME_TAG(ngemlReplyDestroy);

    log = ngemLogGetDefault();

    if (reply == NULL) {
        return NGEM_SUCCESS;
    }

    NGEM_LIST_ERASE_EACH(char, &reply->ngr_params, it, param) {
        ngiFree(param, log, NULL);
    }
    NGEM_LIST_ERASE_EACH(char, &reply->ngr_options, it, option) {
        ngiFree(option, log, NULL);
    }

    /* LIST */
    NGEM_LIST_FINALIZE(char ,&reply->ngr_params);
    NGEM_LIST_FINALIZE(char ,&reply->ngr_options);

    ngiFree(reply->ngr_errorMessage, log, NULL);
    NGI_DEALLOCATE(ngemReply_t, reply, log, NULL);

    return NGEM_SUCCESS;
}

/**
 * Reply: Is error?
 */
ngemResult_t
ngemReplyGetError(
    ngemReply_t *reply)
{
    NGEM_FNAME_TAG(ngemReplyGetError);

    return reply->ngr_result;
}

/**
 * Reply: Set error
 */
ngemResult_t
ngemReplySetError(
    ngemReply_t *reply,
    const char *errorMessage)
{
    NGEM_FNAME_TAG(ngemReplySetError);

    NGEM_ASSERT(reply != NULL);
    NGEM_ASSERT((reply->ngr_errorMessage != NULL) || (errorMessage != NULL));

    if (reply->ngr_result == NGEM_FAILED) {
        /* Ignore */
        return NGEM_SUCCESS;
    }
    reply->ngr_result = NGEM_FAILED;
    reply->ngr_errorMessage = strdup(errorMessage);
    if (reply->ngr_errorMessage == NULL) {
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

/**
 * Reply: Set that the reply is multiline.
 */
void
ngemReplySetMultiLine(
    ngemReply_t *reply,
    bool multiLine)
{
    NGEM_FNAME_TAG(ngemReplySetMultiLine);

    NGEM_ASSERT(reply != NULL);

    reply->ngr_multiLine = multiLine;

    return;
}

/**
 * Reply: Add parameter.
 */
ngemResult_t
ngemReplyAddParam(
    ngemReply_t *reply,
    const char *param, ...)
{
    va_list ap;
    char *copy  = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngemReplyAddParam);

    NGEM_ASSERT(reply != NULL);
    NGEM_ASSERT_STRING(param);

    log = ngemLogGetDefault();

    if (reply->ngr_result == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Result of reply already is set to error.\n");
        goto error;
    }
        
    va_start(ap, param);
    copy = ngemStrdupVprintf(param, ap);
    va_end(ap);
    if (copy == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't allocate for storage for string.\n");
        goto error;

    }
    nResult = NGEM_LIST_INSERT_TAIL(char, &reply->ngr_params, copy);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't insert parameter to the reply.\n");
        goto error;
    }
    copy = NULL;
        
    return NGEM_SUCCESS;
error:
    ngiFree(copy, log, NULL);
    return NGEM_FAILED;
}

/**
 * Reply: Add option.
 */
ngemResult_t
ngemReplyAddOption(
    ngemReply_t *reply,
    const char *key,
    const char *value,...)
{
    va_list ap;
    ngemResult_t nResult;
    NGEM_FNAME_TAG(ngemReplyAddOption);

    va_start(ap, value);
    nResult = ngemlAddOption(&reply->ngr_options, key, value, ap);
    va_end(ap);
    return nResult;
}

/**
 * Request Function Argument: Create
 */
static ngemRequestFunctionArgument_t *
ngemlRequestFunctionArgumentCreate(
    ngemProtocol_t *protocol,
    char *line)
{
    NGEM_LIST_ITERATOR_OF(ngemRequestInformation_t) it;
    ngemRequestInformation_t *ri;
    ngemRequestFunctionArgument_t *reqArg = NULL;
    ngemRequestInformation_t *reqInfo = NULL;
    ngLog_t *log = NULL;
    int i;
    ngemTokenAnalyzer_t ta;
    bool taInitialized = false;
    char *reqName = NULL;
    int nParams = 0;
    char **params = NULL;
    NGEM_FNAME(ngemlRequestFunctionArgumentCreate);
    
    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT_STRING(line);

    log = ngemLogGetDefault();

    ngemTokenAnalyzerInitialize(&ta, line);
    taInitialized = true;

    reqName = ngemTokenAnalyzerGetString(&ta);
    if (reqName == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't get the request name.\n");
        goto finalize;
    }
    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Request name is \"%s\".\n", reqName);

    /* Get kind of requests */
    NGEM_LIST_FOREACH(ngemRequestInformation_t, &protocol->ngp_requestInfo, it, ri) {
        if (strcmp(reqName, ri->ngri_string) == 0) {
            reqInfo = ri;
            break;
        }
    }
    if (reqInfo == NULL) {
        /* not Found, */
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s: Unknown request.\n", reqName);
        goto finalize;
    }

    nParams = reqInfo->ngri_nArguments;
    if (nParams > 0) {
        params = NGI_ALLOCATE_ARRAY(char *, nParams, log, NULL);
        for (i = 0;i < nParams;++i) {
            params[i] = NULL;
        }
        for (i = 0;i < nParams;++i) {
            params[i] = ngemTokenAnalyzerGetString(&ta);
            if (params[i] == NULL) {
                ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s: Few parameters.\n", reqName);
                goto finalize;
            }
        }
    }
    if (ngemTokenAnalyzerHasNext(&ta)) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s: Too many parameters.\n", reqName);
        goto finalize;
    }

    reqArg = NGI_ALLOCATE(ngemRequestFunctionArgument_t, log, NULL);
    if (reqArg == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for arguments of request handler.\n");
        goto finalize;
    }
    reqArg->ngra_protocol     = protocol;
    reqArg->ngra_requestInfo  = reqInfo;
    reqArg->ngra_userData     = protocol->ngp_userData;
    reqArg->ngra_requestName  = reqName;
    reqArg->ngra_params       = params;
    reqArg->ngra_nParams      = nParams;
    reqArg->ngra_analyzer     = NULL;
    reqArg->ngra_reply        = NULL;

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "%s request is received.\n", reqName);    

    /* Success */
finalize:
    if (reqArg == NULL) {
        /* Error occurred */
        if (params != NULL) {
            for (i = 0;i < nParams;++i) {
                ngiFree(params[i], log, NULL);
            }
            ngiFree(params, log, NULL);
        }
        ngiFree(reqName, log, NULL);
    }

    if (taInitialized) {
        taInitialized = false;
        ngemTokenAnalyzerFinalize(&ta);
    }

    return reqArg;
}

/**
 * Request Function Argument: Destroy
 */
static ngemResult_t
ngemlRequestFunctionArgumentDestroy(
    ngemRequestFunctionArgument_t *reqArg)
{
    ngemResult_t nResult;
    int i;
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngemlRequestFunctionArgumentDestroy);

    log = ngemLogGetDefault();

    if (reqArg == NULL) {
        return NGEM_SUCCESS;
    }

    nResult = ngemOptionAnalyzerDestroy(reqArg->ngra_analyzer);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't destroy option analyzer.\n");
        ret = nResult;
    }

    /* Params */
    if (reqArg->ngra_params != NULL) {
        NGEM_ASSERT(reqArg->ngra_nParams > 0);
        for (i = 0;i < reqArg->ngra_nParams;++i) {
            ngiFree(reqArg->ngra_params[i], log, NULL);
            reqArg->ngra_params[i] = NULL;
        }
        ngiFree(reqArg->ngra_params, log, NULL);
        reqArg->ngra_params = NULL;
    }
    ngiFree(reqArg->ngra_requestName, log, NULL);
    reqArg->ngra_requestName = NULL;

    reqArg->ngra_protocol     = NULL;
    reqArg->ngra_requestInfo  = NULL;
    reqArg->ngra_userData     = NULL;
    reqArg->ngra_requestName  = NULL;
    reqArg->ngra_params       = NULL;
    reqArg->ngra_nParams      = 0;
    reqArg->ngra_analyzer     = NULL;
    reqArg->ngra_reply        = NULL;

    ngiFree(reqArg, log, NULL);
    reqArg = NULL;

    return ret;
}

/**
 * Notify: Create
 */
ngemNotify_t *
ngemNotifyCreate(
    const char *name,
    bool multiLine)
{
    ngemNotify_t *notify = NULL;
    char *nameCopy = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngemNotifyCreate);

    NGEM_ASSERT_STRING(name);

    log = ngemLogGetDefault();

    notify = NGI_ALLOCATE(ngemNotify_t, log, NULL);
    if (notify == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't allocate storage for the notify.\n");
        goto error;
    }

    notify->ngn_name      = NULL;
    notify->ngn_multiLine = multiLine;
    NGEM_LIST_SET_INVALID_VALUE(&notify->ngn_params);
    NGEM_LIST_SET_INVALID_VALUE(&notify->ngn_options);

    /* LIST */
    NGEM_LIST_INITIALIZE(char ,&notify->ngn_params);
    NGEM_LIST_INITIALIZE(char ,&notify->ngn_options);

    nameCopy = ngiStrdup(name, log, NULL);
    if (nameCopy == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't copy string.\n");
        goto error;
    }
    notify->ngn_name = nameCopy;

    return notify;
error:
    ngiFree(nameCopy, log, NULL);
    NGI_DEALLOCATE(ngemNotify_t, notify, log, NULL);

    return NULL;
}

/**
 * Notify: Destroy
 */
void
ngemNotifyDestroy(
    ngemNotify_t *notify)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *option;
    char *param;
    ngLog_t *log;
    NGEM_FNAME_TAG(ngemNotifyDestroy);

    log = ngemLogGetDefault();

    if (notify == NULL) {
        return;
    }

    /* LIST */
    NGEM_LIST_ERASE_EACH(char, &notify->ngn_params, it, param) {
        ngiFree(param, log, NULL);
    }
    NGEM_LIST_ERASE_EACH(char, &notify->ngn_options, it, option) {
        ngiFree(option, log, NULL);
    }

    NGEM_LIST_FINALIZE(char ,&notify->ngn_params);
    NGEM_LIST_FINALIZE(char ,&notify->ngn_options);

    ngiFree(notify->ngn_name, log, NULL);

    notify->ngn_name      = NULL;
    notify->ngn_multiLine = false;
    NGEM_LIST_SET_INVALID_VALUE(&notify->ngn_params);
    NGEM_LIST_SET_INVALID_VALUE(&notify->ngn_options);

    NGI_DEALLOCATE(ngemNotify_t, notify, log, NULL);
    notify = NULL;

    return;
}

/**
 * Notify: Add parameter.
 */
ngemResult_t
ngemNotifyAddParam(
    ngemNotify_t *notify,
    const char *param,
    ...)
{
    va_list ap;
    char *copy = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngemNotifyAddParam);

    NGEM_ASSERT(notify != NULL);
    NGEM_ASSERT_STRING(param);

    log = ngemLogGetDefault();
        
    va_start(ap, param);
    copy = ngemStrdupVprintf(param, ap);
    va_end(ap);
    if (copy == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't copy string.\n");
    }
    nResult = NGEM_LIST_INSERT_TAIL(char, &notify->ngn_params, copy);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append a parameter to notify.\n");
    }
    return NGEM_SUCCESS;
}

/**
 * Notify: Add option.
 */
ngemResult_t
ngemNotifyAddOption(
    ngemNotify_t *notify,
    const char *key,
    const char *value,...)
{
    va_list ap;
    ngemResult_t nResult;
    NGEM_FNAME_TAG(ngemNotifyAddOption);

    va_start(ap, value);
    nResult = ngemlAddOption(&notify->ngn_options, key, value, ap);
    va_end(ap);
    return nResult;
}

/**
 * Protocol: Send notify
 */
ngemResult_t
ngemProtocolSendNotify(
    ngemProtocol_t *protocol,
    ngemNotify_t **ppnotify)
{
    ngemResult_t nResult;
    ngemNotify_t *notify;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngemProtocolSendNotify);

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT(ppnotify != NULL);
    NGEM_ASSERT(*ppnotify != NULL);

    notify = *ppnotify;
    *ppnotify = NULL;
    log = ngemLogGetDefault();

    if (!protocol->ngp_available) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "This protocol is not available.\n");
        return NGEM_FAILED;
    }

    /* Push to the queue */
    nResult = NGEM_LIST_INSERT_TAIL(ngemNotify_t, &protocol->ngp_notifyQueue, notify);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't insert notify to the queue.\n");
        return NGEM_FAILED;
    }

    if (ngemCallbackIsValid(protocol->ngp_notifyCallback)) {
        /* Delay send */
        return NGEM_SUCCESS;
    }

    NGEM_ASSERT(NGEM_LIST_HEAD(ngemNotify_t, &protocol->ngp_notifyQueue) == notify);

    return ngemlProtocolSendNotify(protocol);
}

/**
 * Protocol: Send notify
 */
static ngemResult_t
ngemlProtocolSendNotify(
    ngemProtocol_t *protocol)
{
    ngemNotify_t *notify = NULL;
    ngemWriteStringCallbackFunc_t func = NULL;
    ngemCallback_t callback;
    ngLog_t *log;
    NGEM_FNAME(ngemlProtocolSendNotify);

    NGEM_ASSERT(protocol != NULL);

    log = ngemLogGetDefault();

    notify = NGEM_LIST_HEAD(ngemNotify_t, &protocol->ngp_notifyQueue);
    NGEM_ASSERT(notify != NULL);

    if (notify->ngn_multiLine) {
        func = ngemlProtocolNotifyOptionCallback;
    } else {
        func = ngemlProtocolSendNotifyCallback;
    }

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Register Callback for Notify.\n");
    callback = ngemlProtocolSend(protocol, protocol->ngp_stdio.ngsio_error,
        notify->ngn_name, &notify->ngn_params, func);
    if (ngemCallbackIsValid(callback) == 0) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't register callback for sending the notify.\n");
        return NGEM_FAILED;
    }
    protocol->ngp_notifyCallback = callback;

    return NGEM_SUCCESS;
}

/**
 * Protocol: Callback function after sending the notify.
 */
static void
ngemlProtocolSendNotifyCallback(
    void *userData,
    int   fd,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t *protocol = (ngemProtocol_t *)userData;
    ngLog_t *log;
    ngemResult_t nResult;
    ngemNotify_t *notify = NULL;
    NGEM_FNAME(ngemlProtocolSendNotifyCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't write the notify of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is canceled.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    case NGEM_CALLBACK_RESULT_EOF:
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    NGEM_ASSERT(protocol->ngp_notifyCallback != NULL);
    protocol->ngp_notifyCallback = NULL;

    /* Destroy the notify */
    NGEM_ASSERT(!NGEM_LIST_IS_EMPTY(ngemNotify_t, &protocol->ngp_notifyQueue));
    notify = NGEM_LIST_HEAD(ngemNotify_t, &protocol->ngp_notifyQueue);
    NGEM_LIST_ERASE_BY_ADDRESS(ngemNotify_t, &protocol->ngp_notifyQueue, notify);
    ngemNotifyDestroy(notify);

    if (NGEM_LIST_IS_EMPTY(ngemNotify_t, &protocol->ngp_notifyQueue)) {
        /* Not continue */
    } else {
        nResult = ngemlProtocolSendNotify(protocol);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't send the notify.\n");
            goto protocol_end;
        }
    }
    return;

protocol_end:
    nResult = ngemProtocolDisable(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
    }
    return;
}

/**
 * Protocol: Callback function after sending the option of reply.
 */
static void
ngemlProtocolNotifyOptionCallback(
    void *userData,
    int   fd,
    ngemCallbackResult_t cResult)
{
    ngemProtocol_t *protocol = (ngemProtocol_t *)userData;
    ngLog_t *log;
    ngemNotify_t *notify = NULL;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *line = NULL;
    ngemWriteStringCallbackFunc_t func = NULL;
    ngemCallback_t callback = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngemlProtocolNotifyOptionCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    switch (cResult) {
    case NGEM_CALLBACK_RESULT_FAILED:
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't write the notify of Ninf-G Protocol.\n");
        goto protocol_end;
    case NGEM_CALLBACK_RESULT_CANCEL:
        ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Callback is option.\n");
        return;
    case NGEM_CALLBACK_RESULT_SUCCESS:
        break;
    case NGEM_CALLBACK_RESULT_EOF:
    default:
        NGEM_ASSERT_NOTREACHED();
    }

    NGEM_ASSERT(protocol->ngp_notifyCallback != NULL);
    protocol->ngp_notifyCallback = NULL;

    NGEM_ASSERT(!NGEM_LIST_IS_EMPTY(ngemNotify_t, &protocol->ngp_notifyQueue));
    notify = NGEM_LIST_HEAD(ngemNotify_t, &protocol->ngp_notifyQueue);
    NGEM_ASSERT(notify->ngn_multiLine == true);

    if (NGEM_LIST_IS_EMPTY(char, &notify->ngn_options)) {
        /* End */
        func = ngemlProtocolSendNotifyCallback;
        line = ngemStrdupPrintf("%s_END", notify->ngn_name);
        if (line == NULL) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't allocate storage for the string.\n");
            goto protocol_end;
        }
    } else {
        it = NGEM_LIST_BEGIN(char, &notify->ngn_options);
        line = NGEM_LIST_GET(char, it);
        NGEM_ASSERT_STRING(line);
        NGEM_LIST_ERASE(char, it);
        func = ngemlProtocolNotifyOptionCallback;
    }

    callback = ngemCallbackWriteFormat(protocol->ngp_stdio.ngsio_error,
        func, (void *)protocol, "%s%s", line, protocol->ngp_separator);
    if (callback == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't register callback for sending the notify.\n");
        goto protocol_end;
    }
    protocol->ngp_notifyCallback = callback;

    ngiFree(line, log, NULL);
    line = NULL;

    return;
protocol_end:
    ngiFree(line, log, NULL);
    line = NULL;

    nResult = ngemProtocolDisable(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disables the protocol.\n");
    }

    return;
}

/**
 * Add option(notify and reply)
 */
static ngemResult_t
ngemlAddOption(
    NGEM_LIST_OF(char) *list,
    const char *key,
    const char *value,
    va_list ap)
{
    char *string = NULL;
    ngemStringBuffer_t sBuffer;
    ngemResult_t nResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    bool sBufferInitialized = false;
    NGEM_FNAME(ngemlAddOption);

    log = ngemLogGetDefault();

    NGEM_ASSERT_STRING(key);
    NGEM_ASSERT_STRING(value);

    nResult = ngemStringBufferInitialize(&sBuffer);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't initialize string buffer.\n");
        goto finalize;
    }
    sBufferInitialized = false;

    nResult = ngemStringBufferFormat(&sBuffer, "%s ", key);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append option's name.\n");
        goto finalize;
    }

    nResult = ngemStringBufferVformat(&sBuffer, value, ap);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append option's value.\n");
        goto finalize;
    }
    string = ngemStringBufferRelease(&sBuffer);
    if (string == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't release string from string buffer.\n");
        goto finalize;
    }

    nResult = NGEM_LIST_INSERT_TAIL(char, list, string);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't append new option to list.\n");
        goto finalize;
    }
    string = NULL;
    
    ret = NGEM_SUCCESS;
finalize:
    if (sBufferInitialized) {
        ngemStringBufferFinalize(&sBuffer);
        sBufferInitialized = false;
    }
    ngiFree(string, log, NULL);

    return ret;
}

/**
 * Protocol: Send
 */
static ngemCallback_t
ngemlProtocolSend(
    ngemProtocol_t *protocol,
    int fd,
    const char *prefix,
    NGEM_LIST_OF(char) *params,
    ngemWriteStringCallbackFunc_t func)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    ngemStringBuffer_t sBuffer;
    char *param = NULL;
    char *paramString = NULL;
    ngemCallback_t callback = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngemlProtocolSend);

    log = ngemLogGetDefault();

    nResult = ngemStringBufferInitialize(&sBuffer);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't initialize string buffer.\n");
        goto finalize;
    }
    NGEM_LIST_FOREACH(char, params, it, param) {
        nResult = ngemStringBufferFormat(&sBuffer, " %s", param);
        if (nResult == NGEM_FAILED) {
            ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
                "Can't append a option.\n");
            goto finalize;
        }
    }
    paramString = ngemStringBufferRelease(&sBuffer);
    if (paramString == NULL) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't release string from string buffer.\n");
        goto finalize;
    }

    /* Send */
    callback = ngemCallbackWriteFormat(fd, func,
        (void *)protocol, "%s%s%s", prefix, paramString, protocol->ngp_separator);
    if (ngemCallbackIsValid(callback) == 0) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName,
            "Can't register callback for sending.\n");
        goto finalize;
    }

finalize:
    ngemStringBufferFinalize(&sBuffer);
    ngiFree(paramString, log, NULL);
    paramString = NULL;
    
    return callback;
}

/* Requests */
ngemRequestInformation_t ngemProtocolQueryFeaturesRequest = {
    "QUERY_FEATURES",
    0,
    false,
    ngemQueryFeaturesBegin,
    NULL,
    NULL
};
ngemRequestInformation_t ngemProtocolExitRequest = {
    "EXIT",
    0,
    false,
    NULL,
    NULL,
    ngemExitAfterReply
};

/**
 * Protocol: Callback for QUERY_FEATURES
 */
ngemResult_t
ngemQueryFeaturesBegin(
    ngemRequestFunctionArgument_t *arg)
{
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message = NULL;
    ngemProtocol_t *protocol = arg->ngra_protocol;
    NGEM_FNAME(ngemQueryFeaturesBegin);

    log = ngemLogGetDefault();
    
    ngemReplySetMultiLine(arg->ngra_reply, true);

    /* Version */
    nResult = ngemReplyAddOption(arg->ngra_reply, "protocol_version", "%d.%d",
        protocol->ngp_version.ngpv_major, protocol->ngp_version.ngpv_minor);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't create the reply.";
        goto error;
    }

    /* Features */
    {
        NGEM_LIST_ITERATOR_OF(char) it;
        char *feature = NULL;

        NGEM_LIST_FOREACH(char, &protocol->ngp_features, it, feature) {
            nResult = ngemReplyAddOption(arg->ngra_reply, "feature", feature);
            if (nResult == NGEM_FAILED) {
                message = "Can't create the reply.";
                goto error;
            }
        }
    }

    /* Requests */
    {
        NGEM_LIST_ITERATOR_OF(ngemRequestInformation_t) it;
        ngemRequestInformation_t *req = NULL;

        NGEM_LIST_FOREACH(ngemRequestInformation_t, &protocol->ngp_requestInfo, it, req) {
            nResult = ngemReplyAddOption(arg->ngra_reply, "request",
                "%s", req->ngri_string);
            if (nResult != NGEM_SUCCESS) {
                message = "Can't create the reply.";
                goto error;
            }
        }
    }

    return NGEM_SUCCESS;
error:
    ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "%s\n", message);
    nResult= ngemReplySetError(arg->ngra_reply, message);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't set error to the reply\n");
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

/**
 * Protocol: Callback for EXIT
 */
ngemResult_t
ngemExitAfterReply(ngemRequestFunctionArgument_t *arg)
{
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngemExitAfterReply);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGEM_LOGCAT_PROTOCOL, fName, "Called.\n");

    nResult = ngemProtocolDisable(arg->ngra_protocol);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_PROTOCOL, fName, "Can't disable the protocol.\n");
    }

    return NGEM_SUCCESS;
}
