#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexProtocol.c,v $ $Revision: 1.60 $ $Date: 2008/07/02 09:29:38 $";
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
 * Module of Protocol manager for Ninf-G Client.
 */

#include <stdlib.h>
#include "ngEx.h"

/**
 * Prototype declaration of static functions.
 */
static int ngexlProtocolNegotiationFromClient(
    ngexiContext_t *, ngLog_t *, int *);
static int ngexlProtocolNegotiationResult(
    ngexiContext_t *, ngLog_t *, int *);

static void ngexlCallbackProtocol(
    void *, globus_io_handle_t *, globus_result_t);
static int ngexlProtocolProcess(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);

static int ngexlProtocolProcessRequest(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestQueryFunctionInformation(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestQueryExecutableInformation(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestResetExecutable(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolReplyResetExecutable(
    ngexiContext_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngexlProtocolRequestExitExecutable(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolReplyExitExecutable(
    ngexiContext_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngexlProtocolRequestInvokeSession(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
#if 0 /* Temporary commented out */
static int ngexlProtocolRequestSuspendSession(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestResumeSession(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
#endif
static int ngexlProtocolRequestCancelSession(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolReplyCancelSession(
    ngexiContext_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngexlProtocolRequestPullBackSession(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolSessionInformationConvert(
    ngiProtocol_t *, ngexiSessionInformation_t *, char *, int,
    ngLog_t *, int *);
static int ngexlProtocolCompressionInformationConvert(
    ngiProtocol_t *, ngCompressionInformation_t *, int, char *, int, int *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferArgumentData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferResultData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferCallbackArgumentData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferCallbackResultData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);

static int ngexlProtocolReceive(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngiProtocolReceiveMode_t, int *, int *);
static int ngexlSendReplyQueryFunctionInformation(
    ngexiContext_t *, ngiProtocol_t *, int, char *, int *);
#if 0 /* Temporary commented out */
static int ngexlSendReplyQueryExecutableInformation(
    ngexiContext_t *, ngiProtocol_t *, int, char *, int *);
#endif
static int ngexlSendReplyResetExecutable(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
static int ngexlSendReplyExitExecutable(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
static int ngexlSendReplyInvokeSession(
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
#if 0 /* Temporary commented out */
static int ngexlSendReplySuspendSession(
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
static int ngexlSendReplyResumeSession(
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
#endif
static int ngexlSendReplyCancelSession(
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
static int ngexlSendReplyPullBackSession(
    ngexiContext_t *, ngiProtocol_t *, int, int, char *, int *);
static int ngexlSendReplyTransferArgumentData(
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
static int ngexlSendReplyTransferResultData(
    ngexiContext_t *, ngiProtocol_t *, int, int, ngiArgument_t *, int *);
static int ngexlSendReplyTransferResultDataNG(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
static int ngexlSendReplyTransferCallbackArgumentData(
    ngexiContext_t *, ngiProtocol_t *, int, int, ngiArgument_t *, int *);
static int ngexlSendReplyTransferCallbackResultData(
    ngexiContext_t *, ngiProtocol_t *, int, int, long, int *);
static int ngexlSendNotifyIamAlive(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
static int ngexlSendNotifyCalculationEnd(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
static int ngexlSendNotifyInvokeCallback(
    ngexiContext_t *, ngiProtocol_t *, int, long, int *);

static int ngexlProtocolCreateTemporaryFilesForWorkArguments(
    ngiProtocol_t *, ngiArgument_t *, ngLog_t *, int *);
static int ngexlProtocolDestroyTemporaryFiles(
    ngiProtocol_t *, ngiArgument_t *, ngLog_t *, int *);
static int ngexlFileCopyClientToRemote(
    ngexiContext_t *, ngiArgument_t *, ngLog_t *, int *);
static int ngexlFileCopyRemoteToClient(
    ngexiContext_t *, ngiArgument_t *, ngLog_t *, int *);

/**
 * Negotiation
 */
int
ngexiProcessNegotiation(ngexiContext_t *context, ngLog_t *log, int *error)
{
    int result;
    long negoOpt[2+2];
#define NGL_NOPTIONS (sizeof (negoOpt) / sizeof (negoOpt[0]))
    int nNegoOpts;
    static const char fName[] = "ngexiProcessNegotiation";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Negotiation start.\n", fName);

    /* Initialize the Negotiation Options */
    nNegoOpts = 0;
#ifdef NGI_NO_ZLIB
    /* CAUTION:
    * This option RAW is not necessary. However, client it version 2.1.0 or
    * before are require any options. Therefore, option RAW is send to client.
    */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_RAW;
    negoOpt[nNegoOpts++] = 0;
#else /* NGI_NO_ZLIB */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_ZLIB;
    negoOpt[nNegoOpts++] = 0;
#endif /* NGI_NO_ZLIB */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_DIVIDE;
    negoOpt[nNegoOpts++] = 0;
    assert(nNegoOpts <= NGL_NOPTIONS);

    /* Send the Negotiation Information */
    result = ngiProtocolSendNegotiationFromExecutable(
    	context->ngc_protocol, &negoOpt[0], nNegoOpts, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the Negotiation Information.\n", fName);
	return 0;
    }

    /* Receive the Negotiation Result */
    result = ngexlProtocolNegotiationResult(context, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Negotiation failed.\n", fName);
	return 0;
    }

    /* Process the Negotiation Information */
    result = ngexlProtocolNegotiationFromClient(context, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Negotiation failed.\n", fName);
	goto error;
    }

    /* Reply the result to Ninf-G Client */
    result = ngiProtocolSendNegotiationResult(
    	context->ngc_protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't reply the result of Negotiation.\n", fName);
	return 0;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Negotiation was successful.\n", fName);

    /* Start sending heartbeat */
    result = ngexiHeartBeatStart(context, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: heartbeat start failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Reply the result to Ninf-G Client */
    result = ngiProtocolSendNegotiationResult(
	context->ngc_protocol, NGI_PROTOCOL_RESULT_NG, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't reply the result of Negotiation.\n", fName);
	return 0;
    }

    return 0;
#undef NGL_NOPTIONS 
}

/**
 * Process the Negotiation Information from Client.
 */
static int
ngexlProtocolNegotiationFromClient(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiProtocolNegotiationFromClient_t *nego;
    ngiByteStreamConversion_t conv;
    static const char fName[] = "ngexlProtocolNegotiationFromClient";

    /* Check and get the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    /* Get the Negotiation Information */
    nego = ngiProtocolReceiveNegotiationFromClient(
    	context->ngc_protocol, log, error);
    if (nego == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the Negotiation Information.\n", fName);
	return 0;
    }

    /**
     * Check the Negotiation Information.
     */

    /* Set the Version Number of a partner's Protocol */
    result = ngiProtocolSetProtocolVersionOfPartner(
	context->ngc_protocol, nego->ngpnfc_protocolVersion,
	context->ngc_log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the version number of a partner's protocol.\n",
	    fName);
	goto release;
    }

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
	nego->ngpnfc_architectureID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: The architecture code %d is not valid.\n",
	    fName, nego->ngpnfc_architectureID);
	goto release;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfc_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Protocol Version is not equal. My version is %#x and partner's version is %#x.\n",
            fName, NGI_PROTOCOL_VERSION, nego->ngpnfc_protocolVersion);
	/* This is not a error. Continue the process. */
    }

    /* Is Context ID valid? */
    if (nego->ngpnfc_contextID != context->ngc_commInfo.ngci_contextID) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Context ID %d is not equal saved one %d.\n",
	    fName, nego->ngpnfc_contextID);
	goto release;
    }
    log = context->ngc_log;

    /* Is Executable ID smaller the minimum? */
    if (nego->ngpnfc_executableID < NGI_EXECUTABLE_ID_MIN) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable ID %d is smaller than minimum %d.\n",
	    fName, nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MIN);
	goto error;
    }

    /* Is Executable ID greater the maximum? */
    if (nego->ngpnfc_executableID > NGI_EXECUTABLE_ID_MAX) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable ID %d is greater than minimum %d.\n",
	    fName, nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MAX);
	goto error;
    }

    /* Get the Executable ID */
    context->ngc_commInfo.ngci_executableID = nego->ngpnfc_executableID;

    /* Is architecture same? */
    if ((nego->ngpnfc_architectureID == NGI_ARCHITECTURE_ID) &&
	(nego->ngpnfc_architectureID != NGI_ARCHITECTURE_UNDEFINED) &&
	(context->ngc_protocol->ngp_attr.ngpa_architecture != NGI_ARCHITECTURE_UNDEFINED)) {
	result = ngiProtocolSetXDR(
    	    context->ngc_protocol, NG_XDR_NOT_USE, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't set the XDR operation.\n", fName);
	    return 0;
	}
    }

    /* Set the Executable ID */
    result = ngiProtocolSetExecutableID(
    	context->ngc_protocol, nego->ngpnfc_executableID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't set the Executable ID.\n", fName);
	return 0;
    }

    /* Set the Conversion Method */
#ifndef NGI_NO_ZLIB
    conv.ngbsc_zlib = 1;
#else /* !NGI_NO_ZLIB */
    conv.ngbsc_zlib = 0;
#endif /* !NGI_NO_ZLIB */
    conv.ngbsc_zlibThreshold = 0;
    conv.ngbsc_argumentBlockSize = 0;
    if (NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(
	    nego->ngpnfc_protocolVersion)) {
	result = ngiProtocolSetConversionMethod(
	    context->ngc_protocol, &conv,
	    &nego->ngpnfc_conversionMethod[0], nego->ngpnfc_nConversions,
	    log, error);
    } else {
	result = ngiProtocolSetConversionMethod(
	    context->ngc_protocol, &conv, NULL, 0, log, error);
    }
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't set the Conversion Method.\n", fName);
        goto error;
    }

    /* Release the Negotiation Information */
    result = ngiProtocolReleaseData(nego, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't release the Negotiation Information.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
release:
error:
    result = ngiProtocolReleaseData(nego, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't release the Negotiation Information.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Process the Negotiation Result.
 */
static int
ngexlProtocolNegotiationResult(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiProtocolNegotiationResult_t negoResult;
    static const char fName[] = "ngexlProtocolNegotiationResult";

    /* Check and get the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    /* Get the Negotiation Result */
    result = ngiProtocolReceiveNegotiationResult(
    	context->ngc_protocol,
	&negoResult, context->ngc_log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the Negotiation Result.\n", fName);
	return 0;
    }

    /* Is negotiation success? */
    if (negoResult.ngpnr_result != NGI_PROTOCOL_RESULT_OK) {
    	/* Failed */
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Negotiation failed.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register Protocol callback function into Globus I/O.
 */
int
ngexiProtocolRegisterCallback(
    ngexiContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngexiProtocolRegisterCallback";

    /* Check the argument */
    assert(context != NULL);

    log = context->ngc_log;

    /* Register the callback for Protocol */
    result = ngexiGlobusIoRegisterSelect(
        &context->ngc_communication->ngc_ioHandle,
        ngexlCallbackProtocol,
        context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't register the callback function.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Callback function for Protocol.
 */
static void
ngexlCallbackProtocol(
    void *cbArg,
    globus_io_handle_t *ioHandle,
    globus_result_t gResult)
{
    int result;
    ngLog_t *log;
    int received, type, request;
    int *error, errorEntity;
    ngexiContext_t *context;
    ngiProtocol_t *protocol;
    ngiCommunication_t *comm;
    ngiProtocolHeader_t head;
    ngiProtocolReceiveMode_t mode;
    static const char fName[] = "ngexlCallbackProtocol";

    /* Check and get the arguments */
    assert(cbArg != NULL);
    assert(ioHandle != NULL);

    context = (ngexiContext_t *)cbArg;
    protocol = context->ngc_protocol;
    comm = context->ngc_communication;
    log = context->ngc_log;
    error = &errorEntity;

    assert(protocol != NULL);
    assert(comm != NULL);
    assert(ioHandle == &comm->ngc_ioHandle);

    /* Output the log */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
        NG_LOG_LEVEL_DEBUG, NULL, "%s: Start.\n", fName);

    /* Is the callback canceled? */
    result = ngiGlobusIsCallbackCancel(gResult, log, error);
    if (result == 1) {
        /**
         * Callback was canceled
         */
        /* Print the information message */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Callback was canceled.\n", fName);

        /* Success */
        return;
    }

    /* Did the error occur? */
    if (gResult != GLOBUS_SUCCESS) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback error was occurred.\n", fName);
        return;
    }

    mode = NGI_PROTOCOL_RECEIVE_MODE_WAIT;
    received = -1;

    /* Set the context ID to protocol */
    result = ngiProtocolSetIDofContext(protocol,
        protocol->ngp_attr.ngpa_contextID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set context ID to protocol.\n", fName);
        goto error;
    }

    /* Set the executable ID to protocol */
    result = ngiProtocolSetIDofExecutable(protocol,
        protocol->ngp_attr.ngpa_executableID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set executable ID to protocol.\n", fName);
        goto error;
    }

    while (1) {

        /* Calls ngiSetEndTime() only in some case.(not every case) */
        result = ngiSetStartTime(&context->ngc_communicationTime, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set start time of communication.\n", fName);
            goto error;
        }

        /* Receive the Protocol */
        result = ngexlProtocolReceive(
            context, protocol, &head, mode, &received, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Receiving protocol failed.\n", fName);
            goto error;
        }

        if (received == 0) {
            break;
        }

        if (mode == NGI_PROTOCOL_RECEIVE_MODE_WAIT) {
            mode = NGI_PROTOCOL_RECEIVE_MODE_NOWAIT;
        }

        /* Process the Protocol */
        result = ngexlProtocolProcess(context, protocol, &head, log, error);
        if (result == 0) {
            ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Process the Protocol.\n", fName);
            goto error;
        }

        /* Release the Session ID of Protocol */
        result = ngiProtocolReleaseIDofSession(protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Release the Session ID of Protocol.\n", fName);
            goto error;
        }

        /* Is callback register? */
        type = head.ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;
        request = head.ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;
        if ((type == NGI_PROTOCOL_REQUEST_TYPE_REQUEST) &&
            (request == NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE)) {

            /* Success */
            return;
        }
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(protocol);

    /* Output the log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Register the callback.\n", fName);

    /* Register the callback for Protocol */
    result = ngexiGlobusIoRegisterSelect(
        ioHandle, ngexlCallbackProtocol, context, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't register the callback function for Protocol.\n", fName);
        goto error;
    }

    /* Success */
    return;

    /* Error occurred */
error:
    /* Set the I/O callback error code */
    result = ngexiContextSetCbError(context, *error, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the cb error code to the Ninf-G Executable.\n",
            fName);
    }

    /* Make the Executable unusable */
    result = ngexiContextUnusable(context, *error, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Failed to set the executable unusable.\n",
            fName);
    }
}

/**
 * Process the Protocol
 */
static int
ngexlProtocolProcess(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int type;
    int result;
    static const char fName[] = "ngexlProtocolProcess";

    /* Check the argument */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    type = header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;

    /* Is Request Type valid? */
    switch (type) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
        result = ngexlProtocolProcessRequest(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't process the Request.\n", fName);
            goto error;
        }
        break;

    default:
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request Type %d is not valid.\n",
            fName, type); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Process the Request.
 */
static int
ngexlProtocolProcessRequest(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int request;

    static const char fName[] = "ngexlProtocolProcessRequest";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Request Code */
    request = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (request) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
        result = ngexlProtocolRequestQueryFunctionInformation(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Query Function Information.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
        result = ngexlProtocolRequestQueryExecutableInformation(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Query Executable Information.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
        result = ngexlProtocolRequestResetExecutable(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Reset Executable.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
        result = ngexlProtocolRequestExitExecutable(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Reset Executable.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
        result = ngexlProtocolRequestInvokeSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Invoke Session.\n",
                fName);
            return 0;
        }
        break;

#if 0 /* Temporary commented out */
    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
        result = ngexlProtocolRequestSuspendSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Suspend Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
        result = ngexlProtocolRequestResumeSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Suspend Session.\n",
                fName);
            return 0;
        }
        break;
#endif

    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
        result = ngexlProtocolRequestCancelSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Cancel Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        result = ngexlProtocolRequestPullBackSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Pull Back Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        result = ngexlProtocolRequestTransferArgumentData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Transfer Argument Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        result = ngexlProtocolRequestTransferResultData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Transfer Result Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = ngexlProtocolRequestTransferCallbackArgumentData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Transfer Callback Argument Data.\n"
,
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = ngexlProtocolRequestTransferCallbackResultData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Request Transfer Callback Result Data.\n",
                fName);
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Request Code %d is not valid.\n", fName, request); 
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * Process the Reply, previously got and stored to status.
 */
int
ngexiProtocolReplyByStatus(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngexiExecutableStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiProtocolReplyByStatus";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    switch (status) {
    case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
        result = ngexlProtocolReplyCancelSession(
            context, protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Cancel Session.\n",
                fName);
            return 0;
        }
        break;

    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
        result = ngexlProtocolReplyResetExecutable(
            context, protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Reset Executable.\n",
                fName);
            return 0;
        }
        break;

    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
        result = ngexlProtocolReplyExitExecutable(
            context, protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Exit Executable.\n",
                fName);
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unexpected state: %d.\n", fName, status);
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * Process the Query Function Information.
 */
static int
ngexlProtocolRequestQueryFunctionInformation(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    char *funcInfo;
    static const char fName[] = "ngexlProtocolRequestQueryFunctionInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(head != NULL);
    assert(context->ngc_rcInfo != NULL);

    funcInfo = NULL;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Function Information was queried.\n", fName);

    /* Get the function information */
    result = ngexiPrintRemoteClassInformationToBuffer(
        context, context->ngc_rcInfo, &funcInfo, log, error);
    if ((result == 0) || (funcInfo == NULL)) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get Remote Class Information string.\n", fName);
	goto error;
    }

    /* Send reply */
    result = ngexlSendReplyQueryFunctionInformation(
    	context, protocol, NGI_PROTOCOL_RESULT_OK, funcInfo, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send reply.\n", fName);
	goto error;
    }

    /* Release the function information */
    globus_libc_free(funcInfo);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the function information */
    if (funcInfo != NULL) {
        globus_libc_free(funcInfo);
    }

    /* Failed */
    return 0;
}

/**
 * Process the Query Executable Information.
 */
static int
ngexlProtocolRequestQueryExecutableInformation(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
#if 0 /* Temporary commented out */
    int result;
    char *info;
    static const char fName[] = "ngexlProtocolRequestQueryExecutableInformation";

    /* Get the function information */

    /* Send reply */
    result = ngexlSendReplyQueryExecutableInformation(
    	context, protocol, NGI_PROTOCOL_RESULT_OK, info, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send reply.\n", fName);
	goto error;
    }
    /* Release the function information */

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the function information */
#endif
    return 0;
}

/**
 * Process the Reset Executable.
 */
static int
ngexlProtocolRequestResetExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolRequestResetExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Reset was requested.\n", fName);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyResetExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    return 0;
}

/**
 * Process the Reply Reset Executable.
 */
static int
ngexlProtocolReplyResetExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolReplyResetExecutable";

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Reset was performed.\n", fName);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyResetExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send reply Reset Executable.\n", fName);
        return 0;
    }

    return 1;
}

/**
 * Process the Exit Executable.
 */
static int
ngexlProtocolRequestExitExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolRequestExitExecutable";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_IDLE,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Exit was requested.\n", fName);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyExitExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    return 0;
}

/**
 * Process the Reply Exit Executable.
 */
static int
ngexlProtocolReplyExitExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolReplyExitExecutable";

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Exit is now on the process.\n", fName);

    /* Stop sending heartbeat */
    result = ngexiHeartBeatStop(context, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: stop heartbeat failed.\n", fName);
        /* not return, continue exiting */
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_END, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyExitExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send reply Exit Executable.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Invoke Session.
 */
static int
ngexlProtocolRequestInvokeSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    long methodID;
    int result;
    static const char fName[] = "ngexlProtocolRequestInvokeSession";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_IDLE,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        goto error;
    }

    /* Get the Method ID from protocol */
    methodID = ngiProtocolGetMethodID(protocol, log, error);
    if (methodID == NGI_METHOD_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Invalid Method ID.\n", fName);
        goto error;
    }

    /* Get the Remote Method Information */
    rmInfo = ngexiRemoteMethodInformationGet(
        context, protocol->ngp_methodID, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Method ID %d.\n", fName, methodID);
        goto error;
    }

    /* Release the Method ID of protocol */
    result = ngiProtocolReleaseMethodID(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't release the Method ID.\n", fName);
        goto error;
    }

    /* Initialize the Session Information */
    result = ngexiContextInitializeSessionInformation(context, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Session Information.\n", fName);
	return 0;
    }

    /* Start the measurement */
    result = ngiProtocolSessionInformationStartMeasurement(
	protocol, rmInfo->ngrmi_nArguments, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't start the measurement.\n", fName);
	return 0;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_INVOKED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Session Invoke was requested.\n", fName);
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: SessionID = %d, MethodID = %d.\n",
        fName, header->ngph_sessionID, methodID);

    ngexiContextSetSessionID(context, header->ngph_sessionID);
    ngexiContextSetRemoteMethodID(context, methodID);

    /* Send reply */
    result = ngexlSendReplyInvokeSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    result = ngexiContextSetMethodTransferArgumentStartTime(
        context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the Start Time.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyInvokeSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

#if 0 /* Temporary commented out */
/**
 * Process the Suspend Session.
 */
static int
ngexlProtocolRequestSuspendSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    int methodID;
    ngRemoteMethodInformationGet *rmInfo;
    static const char fName[] = "ngexlProtocolRequestSuspendSession";
    static const ngexiExecutableStatus_t reqStatus[] = {
	NGEXI_EXECUTABLE_STATUS_CALCULATING,
	NGEXI_EXECUTABLE_STATUS_SUSPENDED,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(head != NULL);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus, NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid state.\n", fName);
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_SUSPENDED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplySuspendSession(
	context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplySuspendSession(
	context, protocol, NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
	return 0;
    }

    return 0;
}

/**
 * Process the Resume Session.
 */
static int
ngexlProtocolRequestResumeSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    int methodID;
    ngRemoteMethodInformationGet *rmInfo;
    static const char fName[] = "ngexlProtocolRequestResumeSession";
    static const ngexiExecutableStatus_t reqStatus[] = {
	NGEXI_EXECUTABLE_STATUS_CALCULATING,
	NGEXI_EXECUTABLE_STATUS_SUSPENDED,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(head != NULL);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus, NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid state.\n", fName);
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplyResumeSession(
	protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyResumeSession(
	protocol, NGI_PROTOCOL_RESULT_OK, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
	return 0;
    }

    return 0;
}
#endif /* Temporary commented out */

/**
 * Process the Cancel Session.
 */
static int
ngexlProtocolRequestCancelSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    char *string;
    ngexiExecutableStatus_t status;
    static const char fName[] = "ngexlProtocolRequestCancelSession";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the status of Executable */
    status = ngexiContextExecutableStatusGet(context, error);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Session cancel was requested.\n", fName);

    /* Can execute request? */
    if (status == NGEXI_EXECUTABLE_STATUS_IDLE) {
        /* Get the Status String */
        string = ngexiContextExecutableStatusStringGet(status, log, NULL);
        if (string == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
                NULL, "%s: Invalid state %d.\n", fName, status);
            goto error;
        }

        /* Set the error code */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state: Now %s.\n", fName, string);
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyCancelSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    return 0;
}

/**
 * Process the Reply Cancel Session.
 */
static int
ngexlProtocolReplyCancelSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolReplyCancelSession";

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_PULL_WAIT, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyCancelSession(
	context, protocol, context->ngc_sessionID,
	NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send reply Cancel Session.\n", fName);
        return 0;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Session cancel was performed.\n", fName);

    return 1;
}

/**
 * Process the Pull Back Session.
 */
static int
ngexlProtocolRequestPullBackSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    char *buf = NULL;
    int nBytes;
    int sentReply = 0;
    ngexiSessionInformation_t sessInfo;
    static const char fName[] = "ngexlProtocolRequestPullBackSession";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_PULL_WAIT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Invalid state.\n", fName);
        goto error;
    }

    /* Get the Session Information */
    ngexiSessionInformationGet(context, &sessInfo);

    /* Allocate the storage for XML of Session Information */
    nBytes = 1024 * 4 + 1024 * sessInfo.ngsi_nCompressionInformations;
    buf = globus_libc_malloc(nBytes);
    if (buf == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for XML.\n", fName);
	goto error;
    }

    /* Make the Session Information */
    result = ngexlProtocolSessionInformationConvert(
	protocol, &sessInfo, buf, nBytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert the Session Information.\n", fName);
        goto error;
    }

    /* Finish the measurement */
    result = ngiProtocolSessionInformationFinishMeasurement(
	protocol, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finish the measurement.\n", fName);
        goto error;
    }

    /* Release the Session Information */
    result = ngexiContextReleaseSessionInformation(context, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the Session Information.\n", fName);
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Send reply */
    sentReply = 1;
    result = ngexlSendReplyPullBackSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, buf, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Session %d was finished.\n",
        fName, header->ngph_sessionID);

    globus_libc_free(buf);
    buf = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (buf != NULL)
	globus_libc_free(buf);

    if (sentReply == 0) {
        /* Send reply */
        result = ngexlSendReplyPullBackSession(
            context, protocol, header->ngph_sessionID,
            NGI_PROTOCOL_RESULT_NG, NULL, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't send the reply.\n", fName);
            return 0;
        }
    }

    return 0;
}

/**
 * Pull Back Session: Convert Session Information.
 */
static int
ngexlProtocolSessionInformationConvert(
    ngiProtocol_t *protocol,
    ngexiSessionInformation_t *sessInfo,
    char *buf,
    int nBytes,
    ngLog_t *log,
    int *error)
{
    int i;
    int result;
    int wNbytes;
    long protocolVersion;
    static const char fName[] = "ngexlProtocolSessionInformationConvert";

    /* Check the arguments */
    assert(sessInfo != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    wNbytes = 0;
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "<sessionInformation>\n");

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        " <realTime \n");

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferArgument=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferArgument.nget_real.nget_execution.tv_sec,
        sessInfo->ngsi_transferArgument.nget_real.nget_execution.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferFileClientToRemote=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferFileClientToRemote.nget_real.nget_execution.tv_sec,
        sessInfo->ngsi_transferFileClientToRemote.nget_real.nget_execution.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  calculation=\"%lds %ldus\"\n",
        sessInfo->ngsi_calculation.nget_real.nget_execution.tv_sec,
        sessInfo->ngsi_calculation.nget_real.nget_execution.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferResult=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferResult.nget_real.nget_execution.tv_sec,
        sessInfo->ngsi_transferResult.nget_real.nget_execution.tv_usec);

    /* Is support the data conversion? */
    if (NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(protocolVersion)) {
	wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
	    "  transferFileRemoteToClient=\"%lds %ldus\"\n",
	    sessInfo->ngsi_transferFileRemoteToClient.nget_real.nget_execution.tv_sec,
	    sessInfo->ngsi_transferFileRemoteToClient.nget_real.nget_execution.tv_usec);
    }

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackTransferArgument=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackTransferArgumentReal.tv_sec,
        sessInfo->ngsi_sumCallbackTransferArgumentReal.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackCalculation=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackCalculationReal.tv_sec,
        sessInfo->ngsi_sumCallbackCalculationReal.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackTransferResult=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackTransferResultReal.tv_sec,
        sessInfo->ngsi_sumCallbackTransferResultReal.tv_usec);

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        " />\n");

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        " <CPUtime \n");

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferArgument=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferArgument.nget_cpu.nget_execution.tv_sec,
        sessInfo->ngsi_transferArgument.nget_cpu.nget_execution.tv_usec);

    /* Is support the data conversion? */
    if (protocolVersion >= NGI_PROTOCOL_VERSION_SUPPORT_THE_CONVERSION_METHOD) {
	wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferFileClientToRemote=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferFileClientToRemote.nget_cpu.nget_execution.tv_sec,
        sessInfo->ngsi_transferFileClientToRemote.nget_cpu.nget_execution.tv_usec);
    }

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  calculation=\"%lds %ldus\"\n",
        sessInfo->ngsi_calculation.nget_cpu.nget_execution.tv_sec,
        sessInfo->ngsi_calculation.nget_cpu.nget_execution.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferResult=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferResult.nget_cpu.nget_execution.tv_sec,
        sessInfo->ngsi_transferResult.nget_cpu.nget_execution.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  transferFileRemoteToClient=\"%lds %ldus\"\n",
        sessInfo->ngsi_transferFileRemoteToClient.nget_cpu.nget_execution.tv_sec,
        sessInfo->ngsi_transferFileRemoteToClient.nget_cpu.nget_execution.tv_usec);

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackTransferArgument=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackTransferArgumentCPU.tv_sec,
        sessInfo->ngsi_sumCallbackTransferArgumentCPU.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackCalculation=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackCalculationCPU.tv_sec,
        sessInfo->ngsi_sumCallbackCalculationCPU.tv_usec);
    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "  callbackTransferResult=\"%lds %ldus\"\n",
        sessInfo->ngsi_sumCallbackTransferResultCPU.tv_sec,
        sessInfo->ngsi_sumCallbackTransferResultCPU.tv_usec);

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        " />\n");

    /* Is compression time supported? */
    if (NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_CALLBACK_NTIMES_CALLED(
	 protocolVersion)) {
	wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
	    " <callbackInformation numberOfTimesWhichCalled=\"%d\"/>\n",
	    sessInfo->ngsi_callbackNtimesCalled);
    }

    if ((NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME(
	 protocolVersion)) &&
	(sessInfo->ngsi_nCompressionInformations > 0)) {
	for (i = 0; i < sessInfo->ngsi_nCompressionInformations; i++) {
	    result = ngexlProtocolCompressionInformationConvert(
		protocol, &sessInfo->ngsi_compressionInformation[i],
		i + 1, buf, nBytes, &wNbytes, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't make the Compression Information.\n", fName);
		return 0;
	    }
	}
    }

    wNbytes += snprintf(&buf[wNbytes], nBytes - wNbytes,
        "</sessionInformation>\n");

    if (wNbytes >= nBytes) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Session Information: Buffer overflow.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Pull Back Session: Convert Session Information.
 */
static int
ngexlProtocolCompressionInformationConvert(
    ngiProtocol_t *protocol,
    ngCompressionInformation_t *comp,
    int argNo,
    char *buf,
    int nBytes,
    int *wNbytes,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(comp != NULL);
    assert(argNo > 0);
    assert(buf != NULL);
    assert(nBytes > 0);
    assert(wNbytes != NULL);
    assert(*wNbytes >= 0);

    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	" <compressionInformation \n");
    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	"  argumentNo=\"%d\"\n", argNo);

    /* Compression */
    if (comp->ngci_out.ngcic_compression.ngcie_measured == 0) {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  beforeCompressionLength=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  afterCompressionLength=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  compressionRealTime=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  compressionCPUtime=\"\"\n");
    } else {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  beforeCompressionLength=\"%lu\"\n",
	    (unsigned long)comp->ngci_out.ngcic_compression.ngcie_lengthRaw);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  afterCompressionLength=\"%lu\"\n",
	    (unsigned long)
            comp->ngci_out.ngcic_compression.ngcie_lengthCompressed);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  compressionRealTime=\"%lds %ldus\"\n",
	    comp->ngci_out.ngcic_compression.ngcie_executionRealTime.tv_sec,
	    comp->ngci_out.ngcic_compression.ngcie_executionRealTime.tv_usec);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  compressionCPUtime=\"%lds %ldus\"\n",
	    comp->ngci_out.ngcic_compression.ngcie_executionCPUtime.tv_sec,
	    comp->ngci_out.ngcic_compression.ngcie_executionCPUtime.tv_usec);
    }

    /* Decompression */
    if (comp->ngci_in.ngcic_decompression.ngcie_measured == 0) {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  beforeDecompressionLength=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  afterDecompressionLength=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionRealTime=\"\"\n");
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionCPUtime=\"\"\n");
    } else {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  beforeDecompressionLength=\"%lu\"\n",
	    (unsigned long)comp->ngci_in.ngcic_decompression.ngcie_lengthRaw);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  afterDecompressionLength=\"%lu\"\n",
	    (unsigned long)
	    comp->ngci_in.ngcic_decompression.ngcie_lengthCompressed);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionRealTime=\"%lds %ldus\"\n",
	    comp->ngci_in.ngcic_decompression.ngcie_executionRealTime.tv_sec,
	    comp->ngci_in.ngcic_decompression.ngcie_executionRealTime.tv_usec);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionCPUtime=\"%lds %ldus\"\n",
	    comp->ngci_in.ngcic_decompression.ngcie_executionCPUtime.tv_sec,
	    comp->ngci_in.ngcic_decompression.ngcie_executionCPUtime.tv_usec);
    }

    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	" />\n");

    /* Success */
    return 1;
}

/**
 * Process the Transfer Argument Data.
 */
static int
ngexlProtocolRequestTransferArgumentData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngiArgument_t *arg;
    ngRemoteMethodInformation_t *rmInfo;
    int methodID;
    int result;
    long protocolVersion;
    static const char fName[] = "ngexlProtocolRequestTransferArgumentData";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_INVOKED,
        NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferArgumentData started.\n", fName);

    /* Get Protocol Version */
    result = ngiProtocolGetProtocolVersionOfPartner(
        protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
        goto error;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Method ID %d.\n", fName, methodID);
        goto error;
    }

    /* Get the Argument */
    arg = ngiProtocolGetArgument(protocol, log, error);
    if (arg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Argument.\n", fName);
        goto error;
    }

    /* Release the Argument */
    result = ngiProtocolReleaseArgument(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't release the Argument.\n", fName);
        goto error;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
        arg, arg, rmInfo->ngrmi_arguments, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't initialize the Subscript Value of Argument.\n", fName);
        goto error;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Subscript Value of Argument is not valid.\n", fName);
        goto error;
    }

    /* Allocate the storage for Argument Data of OUT mode */
    result = ngiArgumentAllocateDataStorage(arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for Argument Data.\n", fName);
        goto error;
    }
    if (NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(protocolVersion)) {
        result = ngexlProtocolCreateTemporaryFilesForWorkArguments(protocol, arg, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't create the temporary file for Argument Data.\n", fName);
            goto error;
        }
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        goto error;
    }

    if (!NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(protocolVersion)) {
        /* Set the start time of file transfer */
        result = ngexiContextSetMethodTransferFileClientToRemoteStartTime(
            context, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Start Time.\n", fName);
            goto error;
        }

        /* Copy the file from client */
        result = ngexlFileCopyClientToRemote(context, arg, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the file from client.\n", fName);
            goto errorBadArgument;
        }

        /* Set the end time of file transfer */
        result = ngexiContextSetMethodTransferFileClientToRemoteEndTime(
            context, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the End Time.\n", fName);
            goto error;
        }
    }

    /* Send reply */
    result = ngexlSendReplyTransferArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    /* Set end time of Callback Transfer Argument */
    result = ngexiContextSetMethodTransferArgumentEndTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End Time of Callback Transfer Argument.\n",
            fName); 
        goto error;
    }

    ngexiContextSetMethodArgument(context, arg);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferArgumentData finished.\n", fName);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    result = ngexlSendReplyTransferArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_NG, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    return 0;

    /* Bad Argument */
errorBadArgument:
    /* Set the end time of file transfer */
    result = ngexiContextSetMethodTransferFileClientToRemoteEndTime(
	context, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
    }

    /* Send reply */
    result = ngexlSendReplyTransferArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_BAD_ARGUMENT, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    assert(error != NULL);
    if ((*error == NG_ERROR_COMMUNICATION) ||
        (*error == NG_ERROR_DISCONNECT)) {
        /* If the connection was closed, No more I/O callback will invoked */

        /* Executable should exit by Unusable */
        return 0;
    }

    /* Executable will not exit */
    return 1;
}

/**
 * Send Notify : Calculation End
 */
int
ngexiProtocolNotifyCalculationEnd(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    int sessionID;
    static const char fName[] = "ngexiProtocolNotifyCalculationEnd";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    /* Set start time */
    result = ngexiContextSetMethodTransferResultStartTime(
        context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the Start Time.\n", fName);
        return 0;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATION_END, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Get sessionID */
    sessionID = ngexiContextGetSessionID(context);

    /* Send Notify Calculation End */
    result = ngexlSendNotifyCalculationEnd(
        context, protocol, sessionID, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send notify Calculation End.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Transfer Result Data.
 */
static int
ngexlProtocolRequestTransferResultData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int methodID;
    int replied = 0;
    long protocolVersion;
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg;
    static const char fName[] = "ngexlProtocolRequestTransferResultData";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CALCULATION_END,
        NGEXI_EXECUTABLE_STATUS_TRANSFER_RESULT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferResultData started.\n", fName);

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
        goto error;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        goto error;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Method ID %d.\n", fName, methodID);
        goto error;
    }

    /* Get the arguments */
    ngexiContextGetMethodArgument(context, &arg);

    if (!NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(protocolVersion)) {
        /* Set the start time of file transfer */
        result = ngexiContextSetMethodTransferFileRemoteToClientStartTime(
            context, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Start Time.\n", fName);
            goto error;
        }

        /* Copy the file to client */
        result = ngexlFileCopyRemoteToClient(context, arg, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the file to client.\n", fName);
            goto errorBadArgument;
        }

        /* Set the end time of file transfer */
        result = ngexiContextSetMethodTransferFileRemoteToClientEndTime(
            context, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the End Time.\n", fName);
            goto error;
        }
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_PULL_WAIT, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplyTransferResultData(
        context, protocol, 
        ngexiContextGetSessionID(context),
        NGI_PROTOCOL_RESULT_OK, arg, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        goto error;
    }
    replied = 1;

    /* Set end time of transfer Argument */
    result = ngexiContextSetMethodTransferResultEndTime(
        context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End Time of Callback Transfer Argument.\n",
            fName); 
        goto error;
    }

    /* Destroy the temporary files */
    result = ngexlProtocolDestroyTemporaryFiles(protocol, arg, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Destroy temporary files.\n", fName);
        goto error;
    }

    /* Make the Session Information */
    result = ngexiContextMakeSessionInformation(context, arg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't make the Session Information.\n", fName);
	goto error;
    }

    /* Release the arguments */
    result = ngexiContextReleaseMethodArgument(context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Argument Data.\n", fName);
        goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferResultData finished.\n", fName);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    if (replied == 1)
	return 0;

    result = ngexlSendReplyTransferResultDataNG(
	context, protocol, ngexiContextGetSessionID(context), NULL);
    if (result != 0) {
	replied = 1;
    } else {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
    }

    /* set error code */
    *error = NG_ERROR_INVALID_ARGUMENT;

    /* Failed */
    return 0;

    /* Bad Argument */
errorBadArgument:

    /* Set the end time of file transfer */
    result = ngexiContextSetMethodTransferFileRemoteToClientEndTime(
	context, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
    }

    /* Send reply */
    if (replied == 1)
	return 0;

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
    }

    result = ngexlSendReplyTransferArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_BAD_ARGUMENT, NULL);
    if (result != 0) {
	replied = 1;
    } else {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        return 0;
    }

    assert(error != NULL);
    if ((*error == NG_ERROR_COMMUNICATION) ||
        (*error == NG_ERROR_DISCONNECT)) {
        /* If the connection was closed, No more I/O callback will invoked */

        /* Executable should exit by Unusable */
        return 0;
    }

    /* Executable will not exit */
    return 1;
}

/**
 * Send Notify : Invoke Callback
 */
int
ngexiProtocolNotifyInvokeCallback(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    long callbackID,
    ngLog_t *log,
    int *error)
{
    int result;
    int sessionID;
    static const char fName[] = "ngexiProtocolNotifyInvokeCallback";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CB_NOTIFY, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Get sessionID */
    sessionID = ngexiContextGetSessionID(context);

    /* Send Notify Invoke Callback */
    result = ngexlSendNotifyInvokeCallback(
        context, protocol, sessionID, callbackID, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send Invoke Callback.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Transfer Callback Argument Data.
 */
static int
ngexlProtocolRequestTransferCallbackArgumentData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngiArgument_t *arg;
    int replied = 0;
    int result;
    static const char fName[] =
        "ngexlProtocolRequestTransferCallbackArgumentData";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CB_NOTIFY,
        NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_ARGUMENT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferCallbackArgumentData started.\n", fName);

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid state.\n", fName);
	goto error;
    }

    /* Get the Argument for Callback*/
    ngexiContextGetCallbackArgument(context, &arg);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CB_WAIT, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyTransferCallbackArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, arg, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the reply.\n", fName);
        goto error;
    }
    replied = 1;

    result = ngiSetEndTime(&context->ngc_communicationTime, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set start time of communication.\n", fName);
        goto error;
    }
    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentReal = 
        ngiTimevalAdd(
            context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentReal,
            context->ngc_communicationTime.nget_real.nget_execution);
    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentCPU = 
        ngiTimevalAdd(
            context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentCPU,
            context->ngc_communicationTime.nget_cpu.nget_execution);

    /* Set start time of Callback */
    result = ngexiContextSetCallbackStartTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the Start Time of Callback.\n", fName);
        return 0;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferCallbackArgumentData finished.\n", fName);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    if (replied == 0) {
        result = ngexlSendReplyTransferResultDataNG(
            context, protocol, header->ngph_sessionID, NULL);
        if (result != 0) {
            replied = 1;
        } else {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the reply.\n", fName);
        }
    }

    return 0;
}

/**
 * Process the Transfer Callback Result Data.
 */
static int
ngexlProtocolRequestTransferCallbackResultData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    int methodID;
    int replied = 0;
    int result;
    static const char fName[] =
        "ngexlProtocolRequestTransferCallbackResultData";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CB_WAIT,
        NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_RESULT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferCallbackResultData started.\n", fName);

    /* Set end time of Callback */
    result = ngexiContextSetCallbackEndTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End Time of Callback.\n", fName);
        return 0;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid Method ID %d.\n", fName, methodID);
	goto error;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid state.\n", fName);
	goto error;
    }

    /* Send reply */
    result = ngexlSendReplyTransferCallbackResultData(
	context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, header->ngph_sequenceNo, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the reply.\n", fName);
	goto error;
    }
    replied = 1;

    result = ngiSetEndTime(&context->ngc_communicationTime, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set start time of communication.\n", fName);
        goto error;
    }

    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultReal = 
        ngiTimevalAdd(
            context->ngc_sessionInfo.ngsi_sumCallbackTransferResultReal,
            context->ngc_communicationTime.nget_real.nget_execution);
    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultCPU = 
        ngiTimevalAdd(
            context->ngc_sessionInfo.ngsi_sumCallbackTransferResultCPU,
            context->ngc_communicationTime.nget_cpu.nget_execution);

    ngexiContextReleaseCallbackArgument(context, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't Release the argument of callback.\n", fName);
	goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: TransferCallbackResultData finished.\n", fName);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    if (replied == 0) {
        result = ngexlSendReplyTransferResultDataNG(
            context, protocol, header->ngph_sessionID, NULL);
        if (result != 0) {
            replied = 1;
        } else {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the reply.\n", fName);
        }
    }

    return 0;
}

/**
 * Send Notify : I am Alive
 */
int
ngexiProtocolNotifyIamAlive(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiProtocolNotifyIamAlive";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    /* Send Notify I am Alive */
    result = ngexlSendNotifyIamAlive(
         context, protocol, NGI_SESSION_ID_UNDEFINED, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send heartbeat.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * Get the Remote Method Information
 */
ngRemoteMethodInformation_t *
ngexiProtocolGetRemoteMethodInformation(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngexiContext_t *context;
    int methodID;
    static const char fName[] = "ngexiProtocolGetRemoteMethodInformation";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Get the Context */
    context = ngexiContextGet(log, error);
    assert(context != NULL);

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid Method ID %d.\n", fName, methodID);
	return NULL;
    }

    return rmInfo;
}


/**
 * Block SIGALRM to send heartbeat when sending.
 */
#define NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK(context, blockType, error) \
    { \
        int macroResult; \
        macroResult = ngexiHeartBeatSendBlock( \
            context, blockType, error); \
        if (macroResult == 0) { \
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL, \
                NG_LOG_LEVEL_ERROR, NULL, \
                "%s: Can't block signal.\n", fName); \
            return 0; \
        } \
    }

#define NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error) \
        NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
            context, NGEXI_HEARTBEAT_SEND_BLOCK, error)

#define NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error) \
        NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK, error)

/**
 * Receive : Receive protocol
 */
static int
ngexlProtocolReceive(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngiProtocolReceiveMode_t mode,
    int *received,
    int *error)
{
    int result;
    static const char fName[] = "ngexlProtocolReceive";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiProtocolReceive(
        protocol, head, mode, received, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't receive request.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;

}

/**
 * SendReply : QueryFunctionInformation
 */
static int
ngexlSendReplyQueryFunctionInformation(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int protoResult,
    char *info,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyQueryFunctionInformation";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyQueryFunctionInformation(
        protocol, protoResult, info, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

#if 0 /* Temporary commented out */
/**
 * SendReply : QueryExecutableInformation
 */
static int
ngexlSendReplyQueryExecutableInformation(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int protoResult,
    char *info,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyQueryExecutableInformation";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyQueryExecutableInformation(
        protocol, protoResult, info, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}
#endif

/**
 * SendReply : ResetExecutable
 */
static int
ngexlSendReplyResetExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyResetExecutable";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyResetExecutable(
        protocol, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : ExitExecutable
 */
static int
ngexlSendReplyExitExecutable(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyExitExecutable";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyExitExecutable(
        protocol, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : InvokeSession
 */
static int
ngexlSendReplyInvokeSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyInvokeSession";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyInvokeSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}


#if 0 /* Temporary commented out */
/**
 * SendReply : SuspendSession 
 */
static int
ngexlSendReplySuspendSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplySuspendSession";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplySuspendSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : ResumeSession
 */
static int
ngexlSendReplyResumeSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyResumeSession";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyResumeSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}
#endif

/**
 * SendReply : CancelSession
 */
static int
ngexlSendReplyCancelSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyCancelSession";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyCancelSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : PullBackSession
 */
static int
ngexlSendReplyPullBackSession(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    char *sessionInfo,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyPullBackSession";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyPullBackSession(
        protocol, sessionID, protoResult, sessionInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : TransferArgumentData
 */
static int
ngexlSendReplyTransferArgumentData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferArgumentData";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyTransferArgumentData(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : TransferResultData
 */
static int
ngexlSendReplyTransferResultData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    ngiArgument_t *arg,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferResultData";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyTransferResultData(
        protocol, sessionID, protoResult, arg, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : TransferResultDataNG
 */
static int
ngexlSendReplyTransferResultDataNG(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferResultDataNG";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyTransferResultDataNG(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : TransferCallbackArgumentData
 */
static int
ngexlSendReplyTransferCallbackArgumentData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    ngiArgument_t *arg,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferCallbackArgumentData";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyTransferCallbackArgumentData(
        protocol, sessionID, protoResult, arg, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : TransferCallbackResultData
 */
static int
ngexlSendReplyTransferCallbackResultData(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    long sequenceNo,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferCallbackResultData";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendReplyTransferCallbackResultData(
        protocol, sessionID, protoResult, sequenceNo, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendNotify : IamAlive
 */
static int
ngexlSendNotifyIamAlive(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendNotifyIamAlive";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendNotifyIamAlive(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send notify.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendNotify : CalculationEnd
 */
static int
ngexlSendNotifyCalculationEnd(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendNotifyCalculationEnd";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendNotifyCalculationEnd(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send notify.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

/**
 * SendNotify : InvokeCallback
 */
static int
ngexlSendNotifyInvokeCallback(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int sessionID,
    long callbackID,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendNotifyInvokeCallback";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK(context, error)

    result = ngiSendNotifyInvokeCallback(
        protocol, sessionID, callbackID, context->ngc_log, error);
    if (result == 0) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send notify.\n", fName);
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK(context, error)

    /* Failed */
    return 0;
}

#undef NGEXL_HEARTBEAT_SIGNAL_BLOCK
#undef NGEXL_HEARTBEAT_SIGNAL_UNBLOCK
#undef NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK

/**
 * Create Temporary File for WORK arguments
 */
static int
ngexlProtocolCreateTemporaryFilesForWorkArguments(
    ngiProtocol_t *protocol,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, j;
    long nFiles;
    char **tmpFileTable;
    char *tmpFile;
    ngiArgumentElement_t *argElement;
    static const char fName[] =
        "ngexlProtocolCreateTemporaryFilesForWorkArguments";

    for (i = 0; i < arg->nga_nArguments; i++) {
        argElement = &arg->nga_argument[i];

        if (arg->nga_argument[i].ngae_dataType != NG_ARGUMENT_DATA_TYPE_FILENAME) {
            continue;
        }

        /* No elements? */
        nFiles = argElement->ngae_nElements;
        if (nFiles <= 0) {
            continue;
        }

        /* OUT and WORK */
        if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_WORK)) {

            /* Allocate tmpFileTable */
            tmpFileTable = (char **)globus_libc_calloc(
                sizeof(char *), nFiles);
            if (tmpFileTable == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't create the Temporary File Name Table.\n", fName);
                goto error;
            }
            /* Register tmpFileTable */
            assert(argElement->ngae_tmpFileNameTable == NULL);
            argElement->ngae_tmpFileNameTable = tmpFileTable;

            /* Initialize tmpFileTable */
            for (j = 0; j < nFiles; j++) {
                tmpFileTable[j] = NULL;
            }

            for (j = 0; j < nFiles; j++) {
                /* Create the temporary file name */
                tmpFile = ngiTemporaryFileCreate(
                    protocol->ngp_attr.ngpa_tmpDir, log, error);
                if (tmpFile == NULL) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't create the Temporary File Name.\n", fName);
                    goto error;
                }
                tmpFileTable[j] = tmpFile;
            }
        }

        for (j = 0; j < nFiles; j++) {
            tmpFile = argElement->ngae_tmpFileNameTable[j];
            if ((tmpFile != NULL) &&
                (strcmp(tmpFile, "") != 0)) {
                /* Register the temporary file */
                result = ngexiTemporaryFileRegister(tmpFile, log, error);
                if (result == 0) {
                    ngLogPrintf(log,
                        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't register the Temporary File.\n", fName);
                    goto error;
                }
            }
        }

        assert(argElement->ngae_nDimensions >= 0);
        if (argElement->ngae_nDimensions == 0) {
            assert(nFiles == 1);
            argElement->ngae_pointer.ngap_fileName =
                argElement->ngae_tmpFileNameTable[0];
        } else {
            argElement->ngae_pointer.ngap_fileNameArray =
                argElement->ngae_tmpFileNameTable;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngexlProtocolDestroyTemporaryFiles(protocol, arg, log, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Destroy temporary files.\n", fName);
    }
    /* Failed */
    return 0;
}

/**
 * Destroy Temporary File
 */
static int
ngexlProtocolDestroyTemporaryFiles(
    ngiProtocol_t *protocol,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, j;
    int ret = 1;
    long nFiles;
    char **tmpFileTable;
    char *tmpFile;
    ngiArgumentElement_t *argElement;
    static const char fName[] = "ngexlProtocolDestroyTemporaryFiles";

    for (i = 0; i < arg->nga_nArguments; i++) {
        argElement = &arg->nga_argument[i];

        if (arg->nga_argument[i].ngae_dataType != NG_ARGUMENT_DATA_TYPE_FILENAME) {
            continue;
        }

        /* No elements? */
        nFiles = argElement->ngae_nElements;
        if (nFiles <= 0) {
            continue;
        }

        /* Is Table null? */
        if (argElement->ngae_tmpFileNameTable == NULL) {
            continue;
        }

        tmpFileTable = argElement->ngae_tmpFileNameTable;
        for (j = 0; j < nFiles; j++) {

            tmpFile = tmpFileTable[j];
            if ((tmpFile == NULL) ||  
                (strcmp(tmpFile, "") == 0)) {
                continue;
            }
            /* Register the temporary file */
            result = ngexiTemporaryFileUnregister(tmpFile, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't unregister the Temporary File.\n", fName);
                error = NULL;
                ret = 0;
            }
            /* Destroy the temporary file name */
            result = ngiTemporaryFileDestroy(tmpFile, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destroy the Temporary File Name.\n", fName);
                error = NULL;
                ret = 0;
            }
            tmpFile = NULL;
        }

        /* Free tmpFileTable */
        globus_libc_free(tmpFileTable);

        argElement->ngae_pointer.ngap_void = NULL;
        argElement->ngae_tmpFileNameTable = NULL;
    }

    return ret;
}

/**
 * File transfer from client to remote.
 */
static int
ngexlFileCopyClientToRemote(
    ngexiContext_t *context,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, j, nFiles;
    char *fileName, *tmpFile;
    char **fileNameTable, **tmpFileTable;
    ngiArgumentElement_t *argElement;
    static const char fName[] = "ngexlFileCopyClientToRemote";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);

    for (i = 0; i < arg->nga_nArguments; i++) {
        argElement = &arg->nga_argument[i];

        if (argElement->ngae_dataType != NG_ARGUMENT_DATA_TYPE_FILENAME) {
            continue;
        }

        nFiles = argElement->ngae_nElements;

        /* Check size */
        if (nFiles <= 0) {
            continue;
        }

        /* Set fileNameTable */
        fileNameTable = argElement->ngae_pointer.ngap_fileNameArray;

        /* Check table */
        if (fileNameTable == NULL) {
            continue;
        }

        /* Allocate tmpFileTable */
        tmpFileTable = (char **)globus_libc_calloc(
            nFiles, sizeof(char *));
        if (tmpFileTable == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't create the Temporary File Name Table.\n", fName);
            return 0;
        }

        /* Initialize tmpFileTable */
        for (j = 0; j < nFiles; j++) {
            tmpFileTable[j] = NULL;
        }

        /* Register tmpFileTable */
        assert(argElement->ngae_tmpFileNameTable == NULL);
        argElement->ngae_tmpFileNameTable = tmpFileTable;

        for (j = 0; j < nFiles; j++) {

            /* Get the file name of client */
            fileName = fileNameTable[j];

            /* Is filename NULL or zero length string ("")? */
            if (fileName == NULL) {
                tmpFileTable[j] = NULL;
                continue;
            } else if (fileName[0] == '\0') {
                tmpFileTable[j] = "";
                continue;
            }
            
            /* Create the temporary file name */
            tmpFile = ngiTemporaryFileCreate(
                context->ngc_lmInfo.nglmi_tmpDir, log, error);
            if (tmpFile == NULL) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't create the Temporary File Name.\n", fName);
                return 0;
            }

            /* Register the temporary file */
            result = ngexiTemporaryFileRegister(tmpFile, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't register the Temporary File.\n", fName);
                return 0;
            }
            
            tmpFileTable[j] = tmpFile;

            /* Is argument IN or INOUT? */
            if ((argElement->ngae_ioMode != NG_ARGUMENT_IO_MODE_IN) &&
                (argElement->ngae_ioMode != NG_ARGUMENT_IO_MODE_INOUT))
                continue;

            /* Transfer the file */
            result = ngexiGASScopyFile(
                context->ngc_gassCopy, context->ngc_commInfo.ngci_gassServer,
                fileName, tmpFile, NGEXI_GASS_COPY_REMOTE_TO_LOCAL,
                context->ngc_retryInfo, &context->ngc_random, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't transfer the file.\n", fName);
                return 0;
            }
        }
    }

    /* Success */
    return 1;
}

/**
 * File transfer from remote to client.
 */
static int
ngexlFileCopyRemoteToClient(
    ngexiContext_t *context,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, j, nFiles;
    char *fileName, *tmpFile;
    char **fileNameTable, **tmpFileTable;
    ngiArgumentElement_t *argElement;
    static const char fName[] = "ngexlFileCopyRemoteToClient";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);

    fileNameTable = NULL;
    tmpFileTable = NULL;

    for (i = 0; i < arg->nga_nArguments; i++) {
        argElement = &arg->nga_argument[i];

        if (argElement->ngae_dataType != NG_ARGUMENT_DATA_TYPE_FILENAME) {
            continue;
        }

        nFiles = argElement->ngae_nElements;

        /* Check size */
        if (nFiles <= 0) {
            continue;
        }

        /* Set fileNameTable */
        fileNameTable = argElement->ngae_pointer.ngap_fileNameArray;

        /* Set tmpFileTable */
        tmpFileTable = argElement->ngae_tmpFileNameTable;

        /* Check table */
        if (fileNameTable == NULL) {
            continue;
        }

        /* Unregister tmpFileTable */
        argElement->ngae_tmpFileNameTable = NULL;

        for (j = 0; j < nFiles; j++) {

            /* Get the file name */
            fileName = fileNameTable[j];
            tmpFile = tmpFileTable[j];

            /* Is tmpFile NULL or zero length string ("")? */
            if (tmpFile == NULL) {
                continue;
            } else if (tmpFile[0] == '\0') {
                tmpFileTable[j] = NULL;
                continue;
            }
            assert(fileName != NULL);
         
            /* Is argument OUT or INOUT? */
            if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_OUT) ||
                (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_INOUT)) {

                /* Transfer the file */
                result = ngexiGASScopyFile(
                    context->ngc_gassCopy,
                    context->ngc_commInfo.ngci_gassServer,
                    fileName, tmpFile, NGEXI_GASS_COPY_LOCAL_TO_REMOTE,
                    context->ngc_retryInfo, &context->ngc_random, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't transfer the file.\n", fName);
                    return 0;
                }
            }
         
            /* Unregister the temporary file */
            result = ngexiTemporaryFileUnregister(tmpFile, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't unregister the Temporary File.\n", fName);
                return 0;
            }
         
            /* Destroy the temporary file name */
            result = ngiTemporaryFileDestroy(tmpFile, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destroy the Temporary File Name.\n", fName);
                return 0;
            }
        }

        /* Destroy tmpFileTable */
        globus_libc_free(tmpFileTable);
    }

    /* Success */
    return 1;
}
