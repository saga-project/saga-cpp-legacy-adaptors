/*
 * $RCSfile: ngexProtocol.c,v $ $Revision: 1.30 $ $Date: 2008/03/28 09:36:20 $
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
 * Module of Protocol manager for Ninf-G Client.
 */

#include "ngEx.h"

NGI_RCSID_EMBED("$RCSfile: ngexProtocol.c,v $ $Revision: 1.30 $ $Date: 2008/03/28 09:36:20 $")

/**
 * Prototype declaration of static functions.
 */
static int ngexlProtocolNegotiationFromClient(
    ngexiContext_t *, ngLog_t *, int *);
static int ngexlProtocolNegotiationResult(
    ngexiContext_t *, ngLog_t *, int *);
static int ngexlProtocolNegotiationFromClientSecondConnect(
    ngexiContext_t *, ngLog_t *, int *);

static int ngexlCallbackProtocol(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngexlProtocolProcess(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    int *, ngLog_t *, int *);

static int ngexlProtocolProcessRequest(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    int *, ngLog_t *, int *);
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
    int *, ngLog_t *, int *);
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
static int ngexlProtocolRequestTransferArgumentData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    int *, ngLog_t *, int *);
static int ngexlProtocolRequestTransferResultData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferCallbackArgumentData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    ngLog_t *, int *);
static int ngexlProtocolRequestTransferCallbackResultData(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    int *, ngLog_t *, int *);
static int ngexlProtocolRequestConnectionClose(
    ngexiContext_t *, ngiProtocol_t *, ngiProtocolHeader_t *,
    int *, ngLog_t *, int *);

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
    ngexiContext_t *, ngiProtocol_t *, int, int, int *);
static int ngexlSendReplyConnectionClose(
    ngexiContext_t *, ngiProtocol_t *, int, int *);
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
    ngiProtocol_t *protocol;
    static const char fName[] = "ngexiProcessNegotiation";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    protocol = context->ngc_protocol;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Negotiation start.\n"); 

    /* Initialize the Negotiation Options */
    nNegoOpts = 0;
#ifndef NGI_ZLIB_ENABLED
    /* CAUTION:
    * This option RAW is not necessary. However, client it version 2.1.0 or
    * before are require any options. Therefore, option RAW is send to client.
    */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_RAW;
    negoOpt[nNegoOpts++] = 0;
#else /* NGI_ZLIB_ENABLED */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_ZLIB;
    negoOpt[nNegoOpts++] = 0;
#endif /* NGI_ZLIB_ENABLED */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_DIVIDE;
    negoOpt[nNegoOpts++] = 0;
    assert(nNegoOpts <= NGL_NOPTIONS);

    /* Send the Negotiation Information */
    result = ngiProtocolSendNegotiationFromExecutable(
    	protocol, &negoOpt[0], nNegoOpts, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the Negotiation Information.\n"); 
	return 0;
    }

    /* Receive the Negotiation Result */
    result = ngexlProtocolNegotiationResult(context, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Negotiation failed.\n"); 
	return 0;
    }

    /* Process the Negotiation Information */
    result = ngexlProtocolNegotiationFromClient(context, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Negotiation failed.\n"); 
	goto error;
    }

    /* Reply the result to Ninf-G Client */
    result = ngiProtocolSendNegotiationResult(
    	protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't reply the result of Negotiation.\n"); 
	return 0;
    }

    /* Increment the connect count. */
    result = ngiProtocolConnected(protocol, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't increment the connection count on protocol. \n");
        goto error;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Negotiation was successful.\n"); 

    /* Start sending heartbeat */
    result = ngexiHeartBeatStart(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "heartbeat start failed.\n"); 
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
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't reply the result of Negotiation.\n"); 
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
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't receive the Negotiation Information.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the version number of a partner's protocol.\n"); 
	goto release;
    }

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
	nego->ngpnfc_architectureID, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "The architecture code %lu is not valid.\n",
            nego->ngpnfc_architectureID); 
	goto release;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfc_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol Version is not equal."
            " My version is %#x and partner's version is %#lx.\n",
            NGI_PROTOCOL_VERSION, nego->ngpnfc_protocolVersion); 
	/* This is not a error. Continue the process. */
    }

    /* Is Context ID valid? */
    if (nego->ngpnfc_contextID != context->ngc_commInfo.ngci_contextID) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Context ID %ld is not equal saved one %d.\n",
            nego->ngpnfc_contextID, context->ngc_commInfo.ngci_contextID); 
	goto release;
    }
    log = context->ngc_log;

    /* Is Executable ID smaller than minimum? */
    if (nego->ngpnfc_executableID < NGI_EXECUTABLE_ID_MIN) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable ID %ld is smaller than minimum %d.\n",
            nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MIN); 
	goto error;
    }

    /* Is Executable ID greater than maximum? */
    if (nego->ngpnfc_executableID > NGI_EXECUTABLE_ID_MAX) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable ID %ld is greater than minimum %d.\n",
            nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MAX); 
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
	    ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't set the XDR operation.\n"); 
	    return 0;
	}
    }

    /* Set the Executable ID */
    result = ngiProtocolSetExecutableID(
    	context->ngc_protocol, nego->ngpnfc_executableID, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the Executable ID.\n"); 
	return 0;
    }

    /* Set the Conversion Method */
#ifdef NGI_ZLIB_ENABLED
    conv.ngbsc_zlib = 1;
#else /* NGI_ZLIB_ENABLED */
    conv.ngbsc_zlib = 0;
#endif /* NGI_ZLIB_ENABLED */
    conv.ngbsc_zlibThreshold = 0;
    conv.ngbsc_argumentBlockSize = 0;

    result = ngiProtocolSetConversionMethod(
	context->ngc_protocol, &conv,
	&nego->ngpnfc_conversionMethod[0], nego->ngpnfc_nConversions,
	log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Conversion Method.\n"); 
        goto error;
    }

    /* Release the Negotiation Information */
    result = ngiProtocolReleaseData(nego, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't release the Negotiation Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
release:
error:
    result = ngiProtocolReleaseData(nego, log, NULL);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't release the Negotiation Information.\n"); 
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
    ngiProtocol_t *protocol;
    ngiProtocolNegotiationResult_t negoResult;
    static const char fName[] = "ngexlProtocolNegotiationResult";

    /* Check and get the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    protocol = context->ngc_protocol;

    /* Get the Negotiation Result */
    result = ngiProtocolReceiveNegotiationResult(
    	protocol, &negoResult, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't receive the Negotiation Result.\n"); 
	return 0;
    }

    /* Is negotiation success? */
    if (negoResult.ngpnr_result != NGI_PROTOCOL_RESULT_OK) {
    	/* Failed */
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Negotiation failed.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Negotiation for Second Connect.
 */
int
ngexiProcessNegotiationSecondConnect(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiProtocol_t *protocol;
    static const char fName[] = "ngexiProcessNegotiationSecondConnect";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    protocol = context->ngc_protocol;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Negotiation for second connect start.\n"); 

    /* Send the Negotiation Information */
    result = ngiProtocolSendNegotiationFromExecutable(
    	protocol, NULL, 0, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the Negotiation Information.\n"); 
	return 0;
    }

    /* Receive the Negotiation Result */
    result = ngexlProtocolNegotiationResult(context, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Negotiation failed.\n"); 
	return 0;
    }

    /* Process the Negotiation Information */
    result = ngexlProtocolNegotiationFromClientSecondConnect(
        context, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Negotiation failed.\n"); 
	goto error;
    }

    /* Reply the result to Ninf-G Client */
    result = ngiProtocolSendNegotiationResult(
    	protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't reply the result of Negotiation.\n"); 
	return 0;
    }

    /* Increment the connect count. */
    result = ngiProtocolConnected(protocol, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't increment the connection count on protocol. \n");
	return 0;
    }

    /* Register the Communication Log */
    if (context->ngc_commLog != NULL) {
        result = ngiCommunicationLogRegister(
            context->ngc_communication, context->ngc_commLog,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Can't register the Communication Log.\n");
            goto error;
        }
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Negotiation for second connect was successful.\n"); 

    /* Register the I/O Callback */
    result = ngexiProtocolRegisterCallback(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register I/O callback.\n"); 
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
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't reply the result of Negotiation.\n"); 
	return 0;
    }

    return 0;
#undef NGL_NOPTIONS 
}

/**
 * Process the Negotiation Information from Client for Second Connect.
 */
static int
ngexlProtocolNegotiationFromClientSecondConnect(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiProtocolNegotiationFromClient_t *nego;
    static const char fName[] =
        "ngexlProtocolNegotiationFromClientSecondConnect";

    /* Check and get the arguments */
    assert(context != NULL);
    assert(context->ngc_protocol != NULL);

    /* Get the Negotiation Information */
    nego = ngiProtocolReceiveNegotiationFromClient(
    	context->ngc_protocol, log, error);
    if (nego == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't receive the Negotiation Information.\n"); 
	return 0;
    }

    /**
     * Check the Negotiation Information.
     */

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
	nego->ngpnfc_architectureID, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "The architecture code %lu is not valid.\n",
            nego->ngpnfc_architectureID); 
	goto release;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfc_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol Version is not equal."
            " My version is %#x and partner's version is %#lx.\n",
            NGI_PROTOCOL_VERSION, nego->ngpnfc_protocolVersion); 
	/* This is not a error. Continue the process. */
    }

    /* Is Context ID valid? */
    if (nego->ngpnfc_contextID != context->ngc_commInfo.ngci_contextID) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Context ID %ld is not equal saved one %d.\n",
            nego->ngpnfc_contextID, context->ngc_commInfo.ngci_contextID); 
	goto release;
    }
    log = context->ngc_log;

    /* Is Executable ID smaller than minimum? */
    if (nego->ngpnfc_executableID < NGI_EXECUTABLE_ID_MIN) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable ID %ld is smaller than minimum %d.\n",
            nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MIN); 
	goto error;
    }

    /* Is Executable ID greater than maximum? */
    if (nego->ngpnfc_executableID > NGI_EXECUTABLE_ID_MAX) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable ID %ld is greater than minimum %d.\n",
            nego->ngpnfc_executableID, NGI_EXECUTABLE_ID_MAX); 
	goto error;
    }

    /* Release the Negotiation Information */
    result = ngiProtocolReleaseData(nego, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't release the Negotiation Information.\n"); 
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
release:
error:
    result = ngiProtocolReleaseData(nego, log, NULL);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't release the Negotiation Information.\n"); 
	return 0;
    }

    return 0;
}

/**
 * Register Protocol callback function into I/O handle.
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

    /* Wait I/O Callback End to ensure the previous connection closed. */
    result = ngiIOhandleCallbackWaiterWait(
        &context->ngc_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait I/O callback. \n"); 
        return 0;
    }

    /* I/O Callback Start */
    result = ngiIOhandleCallbackWaiterStart(
        &context->ngc_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start I/O callback. \n"); 
        return 0;
    }

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
        context->ngc_communication->ngc_ioHandle,
        ngexlCallbackProtocol,
        context, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't register the callback function.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Callback function for Protocol.
 */
static int
ngexlCallbackProtocol(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    ngLog_t *log;
    int received;
    int callbackEnd;
    int result, finalCallback;
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
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    callbackEnd = 1;

    assert(protocol != NULL);
    assert(comm != NULL);
    assert(ioHandle == comm->ngc_ioHandle);

    /* Output the log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Start.\n"); 

    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,
                "Callback handle was closed.\n");

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,
                "Callback was canceled.\n");
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Invalid handle state %d.\n", argState);
            goto error;
        }

        goto finish;
    }

    mode = NGI_PROTOCOL_RECEIVE_MODE_WAIT;
    received = -1;

    /* Set the context ID to protocol */
    result = ngiProtocolSetIDofContext(protocol,
        protocol->ngp_attr.ngpa_contextID, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set context ID to protocol.\n"); 
        goto error;
    }

    /* Set the executable ID to protocol */
    result = ngiProtocolSetIDofExecutable(protocol,
        protocol->ngp_attr.ngpa_executableID, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set executable ID to protocol.\n"); 
        goto error;
    }

    while (1) {
        /* Receive the Protocol */
        result = ngexlProtocolReceive(
            context, protocol, &head, mode, &received, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Receiving protocol failed.\n"); 
            goto error;
        }

        if (received == 0) {
            break;
        }

        if (mode == NGI_PROTOCOL_RECEIVE_MODE_WAIT) {
            mode = NGI_PROTOCOL_RECEIVE_MODE_NOWAIT;
        }

        /* Process the Protocol */
        result = ngexlProtocolProcess(
	    context, protocol, &head, &finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Protocol.\n"); 
            goto error;
        }

        /* Release the Session ID of Protocol */
        result = ngiProtocolReleaseIDofSession(protocol, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Release the Session ID of Protocol.\n"); 
            goto error;
        }

        /* Is callback register? */
        if (finalCallback != 0) {

            /* Initialize the IDs  */
            ngiProtocolInitializeID(protocol);

            /* Success */
            goto finish;
        }
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(protocol);

    /* Output the log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Register the callback.\n"); 

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
        ioHandle, ngexlCallbackProtocol, context, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't register the callback function for Protocol.\n"); 
        goto error;
    }
    callbackEnd = 0;

finish:

    /* I/O Callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &context->ngc_ioCallbackWaiter, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't end the I/O callback.\n");
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Set the I/O callback error code */
    result = ngexiContextSetCbError(context, *error, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the cb error code to the Ninf-G Executable.\n"); 
    }

    /* Make the Executable unusable */
    result = ngexiContextUnusable(context, *error, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Failed to set the executable unusable.\n"); 
    }

    /* I/O Callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &context->ngc_ioCallbackWaiter, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't end the I/O callback.\n");
        }
    }

    return 0;
}

/**
 * Process the Protocol
 */
static int
ngexlProtocolProcess(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
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
    assert(finalCallback != NULL);

    *finalCallback = 0;

    type = header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;

    /* Is Request Type valid? */
    switch (type) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
        result = ngexlProtocolProcessRequest(
            context, protocol, header, finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't process the Request.\n"); 
            goto error;
        }
        break;

    default:
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Request Type %d is not valid.\n", type); 
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
    int *finalCallback,
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
    assert(finalCallback != NULL);

    /* Get the Request Code */
    request = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (request) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
        result = ngexlProtocolRequestQueryFunctionInformation(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Query Function Information.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
        result = ngexlProtocolRequestQueryExecutableInformation(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Query Executable Information.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
        result = ngexlProtocolRequestResetExecutable(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Reset Executable.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
        result = ngexlProtocolRequestExitExecutable(
            context, protocol, header, finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Reset Executable.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
        result = ngexlProtocolRequestInvokeSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Invoke Session.\n"); 
            return 0;
        }
        break;

#if 0 /* Temporary commented out */
    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
        result = ngexlProtocolRequestSuspendSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Suspend Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
        result = ngexlProtocolRequestResumeSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Suspend Session.\n"); 
            return 0;
        }
        break;
#endif

    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
        result = ngexlProtocolRequestCancelSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Cancel Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        result = ngexlProtocolRequestPullBackSession(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Pull Back Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        result = ngexlProtocolRequestTransferArgumentData(
            context, protocol, header, finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Transfer Argument Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        result = ngexlProtocolRequestTransferResultData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Transfer Result Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = ngexlProtocolRequestTransferCallbackArgumentData(
            context, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Transfer Callback Argument Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = ngexlProtocolRequestTransferCallbackResultData(
            context, protocol, header, finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Transfer Callback Result Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_CONNECTION_CLOSE:
        result = ngexlProtocolRequestConnectionClose(
            context, protocol, header, finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Request Connection Close.\n"); 
            return 0;
        }
        break;


    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Request Code %d is not valid.\n", request); 
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
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Cancel Session.\n"); 
            return 0;
        }
        break;

    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
        result = ngexlProtocolReplyResetExecutable(
            context, protocol, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Reset Executable.\n"); 
            return 0;
        }
        break;

    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
        result = ngexlProtocolReplyExitExecutable(
            context, protocol, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Exit Executable.\n"); 
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unexpected state: %d.\n", status); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Function Information was queried.\n"); 

    /* Get the function information */
    result = ngexiPrintRemoteClassInformationToBuffer(
        context, context->ngc_rcInfo, &funcInfo, log, error);
    if ((result == 0) || (funcInfo == NULL)) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get Remote Class Information string.\n"); 
	goto error;
    }

    /* Send reply */
    result = ngexlSendReplyQueryFunctionInformation(
    	context, protocol, NGI_PROTOCOL_RESULT_OK, funcInfo, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send reply.\n"); 
	goto error;
    }

    /* Release the function information */
    ngiFree(funcInfo, log, error);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the function information */
    if (funcInfo != NULL) {
        ngiFree(funcInfo, log, NULL);
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
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send reply.\n"); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Reset was requested.\n"); 

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Reset was performed.\n"); 

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyResetExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send reply Reset Executable.\n"); 
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
    int *finalCallback,
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
    assert(finalCallback != NULL);

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Exit was requested.\n"); 

    *finalCallback = 1;

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Exit is now on the process.\n"); 

    /* Stop sending heartbeat */
    result = ngexiHeartBeatStop(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "stop heartbeat failed.\n"); 
        /* not return, continue exiting */
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_END, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyExitExecutable(
        context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send reply Exit Executable.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        goto error;
    }

    /* Get the Method ID from protocol */
    methodID = ngiProtocolGetMethodID(protocol, log, error);
    if (methodID == NGI_METHOD_ID_UNDEFINED) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Invalid Method ID.\n"); 
        goto error;
    }

    /* Get the Remote Method Information */
    rmInfo = ngexiRemoteMethodInformationGet(
        context, protocol->ngp_methodID, log, error);
    if (rmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid Method ID %ld.\n", methodID); 
        goto error;
    }

    /* Release the Method ID of protocol */
    result = ngiProtocolReleaseMethodID(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the Method ID.\n"); 
        goto error;
    }

    /* Initialize the Session Information */
    result = ngexiContextInitializeSessionInformation(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Session Information.\n"); 
	return 0;
    }

    /* Start the measurement */
    result = ngiProtocolSessionInformationStartMeasurement(
	protocol, rmInfo->ngrmi_nArguments, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the measurement.\n"); 
	return 0;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_INVOKED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Session Invoke was requested.\n"); 
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "SessionID = %ld, MethodID = %ld.\n", header->ngph_sessionID, methodID); 

    ngexiContextSetSessionID(context, header->ngph_sessionID);
    ngexiContextSetRemoteMethodID(context, methodID);

    /* Send reply */
    result = ngexlSendReplyInvokeSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
        return 0;
    }

    result = ngexiContextSetMethodTransferArgumentStartTime(
        context, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start Time.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid state.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_SUSPENDED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplySuspendSession(
	context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid state.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplyResumeSession(
	protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Session cancel was requested.\n"); 

    /* Can execute request? */
    if (status == NGEXI_EXECUTABLE_STATUS_IDLE) {
        /* Get the Status String */
        string = ngexiContextExecutableStatusStringGet(status, log, NULL);
        if (string == NULL) {
            ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Invalid state %d.\n", status); 
            goto error;
        }

        /* Set the error code */
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state: Now %s.\n", string); 
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyCancelSession(
	context, protocol, context->ngc_sessionID,
	NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send reply Cancel Session.\n"); 
        return 0;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Session cancel was performed.\n"); 

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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Invalid state.\n"); 
        goto error;
    }

    /* Get the Session Information */
    ngexiSessionInformationGet(context, &sessInfo);

    /* Allocate the storage for XML of Session Information */
    nBytes = 1024 * 4 + 1024 * sessInfo.ngsi_nCompressionInformations;
    buf = ngiCalloc(nBytes, sizeof(char), log, error);
    if (buf == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't allocate the storage for XML.\n"); 
	goto error;
    }

    /* Make the Session Information */
    result = ngexlProtocolSessionInformationConvert(
	protocol, &sessInfo, buf, nBytes, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't convert the Session Information.\n"); 
        goto error;
    }

    /* Finish the measurement */
    result = ngiProtocolSessionInformationFinishMeasurement(
	protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finish the measurement.\n"); 
        goto error;
    }

    /* Release the Session Information */
    result = ngexiContextReleaseSessionInformation(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't release the Session Information.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Send reply */
    sentReply = 1;
    result = ngexlSendReplyPullBackSession(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, buf, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
        goto error;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Session %ld was finished.\n", header->ngph_sessionID); 

    ngiFree(buf, log, error);
    buf = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (buf != NULL)
	ngiFree(buf, log, NULL);

    if (sentReply == 0) {
        /* Send reply */
        result = ngexlSendReplyPullBackSession(
            context, protocol, header->ngph_sessionID,
            NGI_PROTOCOL_RESULT_NG, NULL, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the reply.\n"); 
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
    ngSessionInformationExecutable_t siReal;
    ngSessionInformationExecutable_t siCPU;
    int result;
    static const char fName[] = "ngexlProtocolSessionInformationConvert";

    /* Check the arguments */
    assert(sessInfo != NULL);
    assert(buf != NULL);
    assert(nBytes > 0);

    memset(&siReal, '\0', sizeof(siReal));
    memset(&siCPU, '\0', sizeof(siCPU));

    siReal.ngsie_transferArgument           = sessInfo->ngsi_transferArgument.nget_real.nget_execution;
    siReal.ngsie_transferFileClientToRemote = sessInfo->ngsi_transferFileClientToRemote.nget_real.nget_execution;
    siReal.ngsie_calculation                = sessInfo->ngsi_calculation.nget_real.nget_execution;
    siReal.ngsie_transferResult             = sessInfo->ngsi_transferResult.nget_real.nget_execution;
    siReal.ngsie_transferFileRemoteToClient = sessInfo->ngsi_transferFileRemoteToClient.nget_real.nget_execution;
    siReal.ngsie_callbackTransferArgument   = sessInfo->ngsi_callbackTransferArgument.nget_real.nget_execution;
    siReal.ngsie_callbackCalculation        = sessInfo->ngsi_callbackCalculation.nget_real.nget_execution;
    siReal.ngsie_callbackTransferResult     = sessInfo->ngsi_callbackTransferResult.nget_real.nget_execution;

    siCPU.ngsie_transferArgument           = sessInfo->ngsi_transferArgument.nget_cpu.nget_execution;
    siCPU.ngsie_transferFileClientToRemote = sessInfo->ngsi_transferFileClientToRemote.nget_cpu.nget_execution;
    siCPU.ngsie_calculation                = sessInfo->ngsi_calculation.nget_cpu.nget_execution;
    siCPU.ngsie_transferResult             = sessInfo->ngsi_transferResult.nget_cpu.nget_execution;
    siCPU.ngsie_transferFileRemoteToClient = sessInfo->ngsi_transferFileRemoteToClient.nget_cpu.nget_execution;
    siCPU.ngsie_callbackTransferArgument   = sessInfo->ngsi_callbackTransferArgument.nget_cpu.nget_execution;
    siCPU.ngsie_callbackCalculation        = sessInfo->ngsi_callbackCalculation.nget_cpu.nget_execution;
    siCPU.ngsie_callbackTransferResult     = sessInfo->ngsi_callbackTransferResult.nget_cpu.nget_execution;

    result = ngiProtocolGetXMLstringFromSessionInformation(
        protocol, &siReal, &siCPU, sessInfo->ngsi_callbackNtimesCalled,
        sessInfo->ngsi_compressionInformation, sessInfo->ngsi_nCompressionInformations,
        buf, (size_t)nBytes, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get XML string from the Session Information.\n"); 
        return 0;
    }

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
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    ngiArgument_t *arg;
    ngRemoteMethodInformation_t *rmInfo;
    int methodID;
    long protocolVersion;
    int result, doClose, replied;
    static const char fName[] = "ngexlProtocolRequestTransferArgumentData";
    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_INVOKED,
        NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT,
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    replied = 0;

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferArgumentData started.\n"); 

    /* Get Protocol Version */
    result = ngiProtocolGetProtocolVersionOfPartner(
        protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the version number of partner's protocol.\n"); 
        goto error;
    }

    /* Get the Keep Connect */
    result = ngiProtocolGetConnectionClose(protocol, &doClose, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the connection close or not.\n"); 
        goto error;
    }

    /* Release the Keep Connect */
    result = ngiProtocolReleaseConnectionClose(protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't release the connection close or not.\n"); 
        goto error;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid Method ID %d.\n", methodID); 
        goto error;
    }

    /* Get the Argument */
    arg = ngiProtocolGetArgument(protocol, log, error);
    if (arg == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't get the Argument.\n"); 
        goto error;
    }

    /* Release the Argument */
    result = ngiProtocolReleaseArgument(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the Argument.\n"); 
        goto error;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
        arg, arg, rmInfo->ngrmi_arguments, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the Subscript Value of Argument.\n"); 
        goto error;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Subscript Value of Argument is not valid.\n"); 
        goto error;
    }

    /* Allocate the storage for Argument Data of OUT mode */
    result = ngiArgumentAllocateDataStorage(arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't allocate the storage for Argument Data.\n"); 
        goto error;
    }
    result = ngexlProtocolCreateTemporaryFilesForWorkArguments(
        protocol, arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't create the temporary file for Argument Data.\n"); 
        goto error;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        goto error;
    }

    /* Send reply */
    replied = 1;
    result = ngexlSendReplyTransferArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
        return 0;
    }

    ngexiContextSetMethodArgument(context, arg);

    /* Make the Session Information */
    result = ngexiContextMakeSessionInformation(context, arg, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't make the Session Information.\n"); 
	goto error;
    }

    /* Connection close */
    if (doClose != 0) {
        result = ngexiContextConnectionClose(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't close the connection.\n"); 
            goto error;
        }
        *finalCallback = 1;
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferArgumentData finished.\n"); 

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Send reply */
    if (replied == 0) {
        result = ngexlSendReplyTransferArgumentData(
            context, protocol, header->ngph_sessionID,
            NGI_PROTOCOL_RESULT_NG, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the reply.\n"); 
        }
    }

    return 0;
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
    int connectPerformed;
    int transferResultDataArrived, closed;
    static const char fName[] = "ngexiProtocolNotifyCalculationEnd";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    transferResultDataArrived = 0;
    closed = 0;

    /* Set start time */
    result = ngexiContextSetMethodTransferResultStartTime(
        context, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start Time.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATION_END, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Get sessionID */
    sessionID = ngexiContextGetSessionID(context);

    /* Start the Transfer Result Data wait. */
    result = ngexiContextAfterCloseWaitStart(
        context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start the Transfer Result Data wait.\n"); 
        goto error;
    }


    do {

        /* Establish the connection if not connected */
        result = ngexiContextConnectionEstablishAndLock(
            context, &connectPerformed, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Establish the connection failed.\n"); 
            goto error;
        }
     
        /* Send Notify Calculation End */
        result = ngexlSendNotifyCalculationEnd(
            context, protocol, sessionID, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send notify Calculation End.\n"); 
            goto error;
        }
     
        /* Unlock the connection */
        result = ngexiContextConnectionUnlock(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unlock the connection failed.\n"); 
            goto error;
        }

        closed = 0;

        /* Wait the Transfer Result Data or Connection Close. */
        result = ngexiContextAfterCloseWait(
            context, NGEXI_AFTER_CLOSE_TYPE_CALCULATION_END,
            &closed, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't wait the Transfer Result Data.\n"); 
            goto error;
        }

        transferResultDataArrived = 1;
        if (closed != 0) {
            transferResultDataArrived = 0;

            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Notify CALCULATION_END was sent."
                " but connection was closed.\n"); 
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Reconnect and retry sending CALCULATION_END.\n");
        }

    } while (transferResultDataArrived == 0);

    /* End the Transfer Result Data wait. */
    result = ngexiContextAfterCloseWaitEnd(
        context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't end the Transfer Result Data wait.\n"); 
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
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferResultData started.\n"); 

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the version number of partner's protocol.\n"); 
        goto error;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        goto error;
    }

    /* Transfer Result Data arrived. */
    result = ngexiContextAfterCloseArrived(
        context, NGEXI_AFTER_CLOSE_TYPE_CALCULATION_END, 1, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Result Data arrival to context.\n"); 
        goto error;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid Method ID %d.\n", methodID); 
        goto error;
    }

    /* Get the arguments */
    ngexiContextGetMethodArgument(context, &arg);

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_TRANSFER_RESULT, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_PULL_WAIT, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Send reply */
    result = ngexlSendReplyTransferResultData(
        context, protocol, 
        ngexiContextGetSessionID(context),
        NGI_PROTOCOL_RESULT_OK, arg, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
        goto error;
    }
    replied = 1;

    /* Destroy the temporary files */
    result = ngexlProtocolDestroyTemporaryFiles(protocol, arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't Destroy temporary files.\n"); 
        goto error;
    }

    /* Release the Session Information */
    result = ngexiContextReleaseSessionInformation(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
	    "Can't release the Session Information.\n");
	goto error;
    }

    /* Make the Session Information */
    result = ngexiContextMakeSessionInformation(context, arg, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't make the Session Information.\n"); 
	goto error;
    }

    /* Release the arguments */
    result = ngexiContextReleaseMethodArgument(context, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Argument Data.\n"); 
        goto error;
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferResultData finished.\n"); 

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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
    }

    /* set error code */
    *error = NG_ERROR_INVALID_ARGUMENT;

    /* Failed */
    return 0;
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
    int connectPerformed;
    int transferCallbackArgumentDataArrived, closed;
    static const char fName[] = "ngexiProtocolNotifyInvokeCallback";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    transferCallbackArgumentDataArrived = 0;
    closed = 0;

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CB_NOTIFY, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Get sessionID */
    sessionID = ngexiContextGetSessionID(context);

    /* Start the Transfer Callback Argument Data wait. */
    result = ngexiContextAfterCloseWaitStart(
        context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start the Transfer Callback Argument Data wait.\n"); 
        goto error;
    }

    do {

        /* Establish the connection if not connected */
        result = ngexiContextConnectionEstablishAndLock(
            context, &connectPerformed, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Establish the connection failed.\n"); 
            return 0;
        }
     
        /* Send Notify Invoke Callback */
        result = ngexlSendNotifyInvokeCallback(
            context, protocol, sessionID, callbackID, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send Invoke Callback.\n"); 
            return 0;
        }
     
        /* Unlock the connection */
        result = ngexiContextConnectionUnlock(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unlock the connection failed.\n"); 
            return 0;
        }

        closed = 0;

        /* Wait the Transfer Callback Argument Data or Connection Close. */
        result = ngexiContextAfterCloseWait(
            context, NGEXI_AFTER_CLOSE_TYPE_INVOKE_CALLBACK,
            &closed, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't wait the Transfer Callback Argument Data.\n"); 
            goto error;
        }

        transferCallbackArgumentDataArrived = 1;
        if (closed != 0) {
            transferCallbackArgumentDataArrived = 0;

            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Notify INVOKE_CALLBACK was sent."
                " but connection was closed.\n"); 
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Reconnect and retry sending INVOKE_CALLBACK.\n"); 
        }

    } while (transferCallbackArgumentDataArrived == 0);

    /* End the Transfer Callback Argument Data wait. */
    result = ngexiContextAfterCloseWaitEnd(
        context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't end the Transfer Callback Argument Data wait.\n"); 
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
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferCallbackArgumentData started.\n"); 

    /* Set start time of Callback Transfer Argument */
    result = ngexiContextSetCallbackTransferArgumentStartTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start Time of Callback Transfer Argument.\n"); 
        return 0;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid state.\n"); 
	goto error;
    }

    /* Transfer Callback Argument Data arrived. */
    result = ngexiContextAfterCloseArrived(
        context, NGEXI_AFTER_CLOSE_TYPE_INVOKE_CALLBACK, 1, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Callback Argument Data arrival"
            " to context.\n"); 
        goto error;
    }

    /* Get the Argument for Callback*/
    ngexiContextGetCallbackArgument(context, &arg);

    /* Set end time of Callback Transfer Argument */
    result = ngexiContextSetCallbackTransferArgumentEndTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End Time of Callback Transfer Argument.\n"); 
        return 0;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CB_WAIT, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
        return 0;
    }

    /* Send reply */
    result = ngexlSendReplyTransferCallbackArgumentData(
        context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, arg, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the reply.\n"); 
        goto error;
    }
    replied = 1;

    /* Set start time of Callback */
    result = ngexiContextSetCallbackStartTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start Time of Callback.\n"); 
        return 0;
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferCallbackArgumentData finished.\n"); 

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
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the reply.\n"); 
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
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    int methodID;
    int result, doClose, replied;
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
    assert(finalCallback != NULL);

    replied = 0;

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferCallbackResultData started.\n"); 

    /* Set end time of Callback */
    result = ngexiContextSetCallbackEndTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End Time of Callback.\n"); 
        return 0;
    }

    /* Set start time of Callback Transfer Result */
    result = ngexiContextSetCallbackTransferResultStartTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Start Time of Callback Transfer Result.\n"); 
        return 0;
    }

    /* Get the Keep Connect */
    result = ngiProtocolGetConnectionClose(protocol, &doClose, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the connection close or not.\n"); 
        goto error;
    }

    /* Release the Keep Connect */
    result = ngiProtocolReleaseConnectionClose(protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't release the connection close or not.\n"); 
        goto error;
    }

    /* Get the Remote Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid Method ID %d.\n", methodID); 
	goto error;
    }

    /* Can execute request? */
    result = ngexiContextExecutableStatusCheck(
	context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid state.\n"); 
	goto error;
    }

    /* Set end time of Callback Transfer Result */
    result = ngexiContextSetCallbackTransferResultEndTime(
        context, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End Time of Callback Transfer Result.\n"); 
        return 0;
    }

    /* Send reply */
    replied = 1;
    result = ngexlSendReplyTransferCallbackResultData(
	context, protocol, header->ngph_sessionID,
        NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the reply.\n"); 
	goto error;
    }

    ngexiContextReleaseCallbackArgument(context, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't Release the argument of callback.\n"); 
	goto error;
    }

    /* Connection close */
    if (doClose != 0) {
        result = ngexiContextConnectionClose(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't close the connection.\n"); 
            goto error;
        }
        *finalCallback = 1;
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "TransferCallbackResultData finished.\n"); 

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_CALCULATING, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
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
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the reply.\n"); 
        }
    }

    return 0;
}

/**
 * Process the Connection Close.
 */
static int
ngexlProtocolRequestConnectionClose(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    int result, replied;
    static const char fName[] = "ngexlProtocolRequestConnectionClose";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    replied = 0;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Connection Close requested.\n"); 

    /* Send reply */
    replied = 1;
    result = ngexlSendReplyConnectionClose(
    	context, protocol, NGI_PROTOCOL_RESULT_OK, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send reply Connection Close.\n"); 
	goto error;
    }

    /* Connection close */
    result = ngexiContextConnectionClose(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't close the connection.\n"); 
        goto error;
    }

    /* Transfer Result Data or Transfer Callback Argument Data not arrived. */
    result = ngexiContextAfterCloseArrived(
        context, NGEXI_AFTER_CLOSE_TYPE_NONE, 0, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Result Data arrival to context.\n"); 
        goto error;
    }

    *finalCallback = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (replied == 0) {
        result = ngexlSendReplyConnectionClose(
        	context, protocol, NGI_PROTOCOL_RESULT_NG, NULL);
        if (result == 0) {
        	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        	    "Can't send reply Connection Close.\n"); 
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
    int result, connectPerformed;
    static const char fName[] = "ngexiProtocolNotifyIamAlive";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);

    /* Establish the connection if not connected */
    result = ngexiContextConnectionEstablishAndLock(
        context, &connectPerformed, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Establish the connection failed.\n"); 
        return 0;
    }

    /* Send Notify I am Alive */
    result = ngexlSendNotifyIamAlive(
         context, protocol, NGI_SESSION_ID_UNDEFINED, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't send heartbeat.\n"); 
        return 0;
    }

    /* Wait the Connection Close or Cancel Session. */
    if (connectPerformed != 0) {
        result = ngexiContextConnectionCloseWait(context, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Wait the connection close failed.\n"); 
            return 0;
        }
    }

    /* Unlock the connection */
    result = ngexiContextConnectionUnlock(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unlock the connection failed.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid Method ID %d.\n", methodID); 
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
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName, \
                "Can't block signal.\n"); \
            return 0; \
        } \
    }

#define NGEXL_HEARTBEAT_SIGNAL_BLOCK_READ(context, error) \
    NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
        context, NGEXI_HEARTBEAT_SEND_BLOCK_READ, error)

#define NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_READ(context, error) \
    NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK_READ, error)

#define NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error) \
    NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
        context, NGEXI_HEARTBEAT_SEND_BLOCK_WRITE, error)

#define NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error) \
    NGEXL_HEARTBEAT_SIGNAL_SET_BLOCK( \
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK_WRITE, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_READ(context, error)

    result = ngiProtocolReceive(
        protocol, head, mode, received, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't receive request.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_READ(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_READ(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyQueryFunctionInformation(
        protocol, protoResult, info, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyQueryExecutableInformation(
        protocol, protoResult, info, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyResetExecutable(
        protocol, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyExitExecutable(
        protocol, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyInvokeSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplySuspendSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyResumeSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyCancelSession(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyPullBackSession(
        protocol, sessionID, protoResult, sessionInfo, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyTransferArgumentData(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyTransferResultData(
        protocol, sessionID, protoResult, arg, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyTransferResultDataNG(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyTransferCallbackArgumentData(
        protocol, sessionID, protoResult, arg, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyTransferCallbackResultData";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyTransferCallbackResultData(
        protocol, sessionID, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Failed */
    return 0;
}

/**
 * SendReply : ConnectionClose
 */
static int
ngexlSendReplyConnectionClose(
    ngexiContext_t *context,
    ngiProtocol_t *protocol,
    int protoResult,
    int *error)
{   
    int result;
    static const char fName[] = "ngexlSendReplyConnectionClose";

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendReplyConnectionClose(
        protocol, protoResult, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send reply.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendNotifyIamAlive(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send notify.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendNotifyCalculationEnd(
        protocol, sessionID, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send notify.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

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

    NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE(context, error)

    result = ngiSendNotifyInvokeCallback(
        protocol, sessionID, callbackID, context->ngc_log, error);
    if (result == 0) {
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send notify.\n"); 
        goto error;
    }

    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE(context, error)

    /* Failed */
    return 0;
}

#undef NGEXL_HEARTBEAT_SIGNAL_BLOCK_READ
#undef NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_READ
#undef NGEXL_HEARTBEAT_SIGNAL_BLOCK_WRITE
#undef NGEXL_HEARTBEAT_SIGNAL_UNBLOCK_WRITE
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
            tmpFileTable = (char **)ngiCalloc(
                nFiles, sizeof(char *), log, error);
            if (tmpFileTable == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't create the Temporary File Name Table.\n"); 
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
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "Can't create the Temporary File Name.\n"); 
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
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                        "Can't register the Temporary File.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't Destroy temporary files.\n"); 
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
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't unregister the Temporary File.\n"); 
                error = NULL;
                ret = 0;
            }
            /* Destroy the temporary file name */
            result = ngiTemporaryFileDestroy(tmpFile, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't destroy the Temporary File Name.\n"); 
                error = NULL;
                ret = 0;
            }
            tmpFile = NULL;
        }

        /* Free tmpFileTable */
        ngiFree(tmpFileTable, log, error);

        argElement->ngae_pointer.ngap_void = NULL;
        argElement->ngae_tmpFileNameTable = NULL;
    }

    return ret;
}

