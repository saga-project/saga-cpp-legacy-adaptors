#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclProtocol.c,v $ $Revision: 1.109 $ $Date: 2007/12/26 12:27:17 $";
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define NGI_REQUIRE_ARCHITECTURE_IDS
#include "ng.h"
#undef NGI_REQUIRE_ARCHITECTURE_IDS
#include "ngXML.h"

/**
 * Prototype declaration of static functions.
 */
static void ngcllCallbackNegotiationFromExecutable(
    void *, globus_io_handle_t *, globus_result_t);
static void ngcllCallbackNegotiationResult(
    void *, globus_io_handle_t *, globus_result_t);
static void ngcllCallbackProtocol(
    void *, globus_io_handle_t *, globus_result_t);

static int ngcllProtocolProcess(
    ngclContext_t *, ngclExecutable_t *, ngiProtocol_t *,
    ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolProcessReply(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolProcessNotify(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyQueryFunctionInformation(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#if 0 /* Temporary comment out */
static int ngcllProtocolReplyQueryExecutableInformation(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#endif
static int ngcllProtocolReplyResetExecutable(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolReplyExitExecutable(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyInvokeSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolRequestTransferArgumentData(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngcllProtocolReplyTransferArgumentData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyTransferResultData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyCancelSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolRequestPullBackSession(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngcllProtocolReplyPullBackSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyTransferCallbackArgumentData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolReplyTransferCallbackResultData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolNotifyCalculationEnd(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolNotifyInvokeCallback(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

/**
 * Callback function for accept.
 */
void
ngcliCallbackAccept(
    void *cbArg,
    globus_io_handle_t *ioHandle,
    globus_result_t gResult)
{
    int result;
    int error;
    ngiProtocol_t *protocol, *newProto;
    ngiCommunication_t *comm, *newComm;
    ngclContext_t *context;
    static const char fName[] = "ngcliCallbackAccept";

    /* Check and get the arguments */
    assert(cbArg != NULL);
    protocol = (ngiProtocol_t *)cbArg;
    comm = protocol->ngp_communication;
    assert(ioHandle != NULL);
    assert(ioHandle == &comm->ngc_ioHandle);
    newProto = NULL;
    newComm = NULL;

    /* Get the Ninf-G Context */
    context = ngiProtocolGetUserData(protocol, NULL, &error);
    if (context == NULL) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context is NULL.\n", fName);
	return;
    }
    
    /* Is the callback canceled? */
    result = ngiGlobusIsCallbackCancel(gResult, context->ngc_log, &error);
    if (result == 1) {
    	/**
	 * Callback was canceled
	 */
	/* Print the information message */
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
	    NULL, "%s: Callback was canceled.\n", fName);

	/* Success */
	return;
    }

    /* Did the error occur? */
    if (gResult != GLOBUS_SUCCESS) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback error was occurred.\n", fName);
	goto error;
    }

    /* log */
    ngclLogPrintfContext(context,
        NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: New TCP connection request.\n", fName);

    /* Construct the new Communication */
    newComm = ngiCommunicationConstructAccept(comm, context->ngc_log, &error);
    if (newComm == NULL) {
	if (error == NG_ERROR_DISCONNECT)
	    goto end;

    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Communication.\n", fName);
	goto error;
    }

    /* Construct the new Protocol */
    newProto = ngiProtocolConstruct(
    	&protocol->ngp_attr, newComm,
        ngcliProtocolGetRemoteMethodInformation, context->ngc_log, &error);
    if (newProto == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Protocol.\n", fName);
	goto error;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	newProto, context, context->ngc_log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the User Data to Protocol.\n", fName);
	goto error;
    }

    /* Register the callback function for Negotiation Information */
    result = ngcliGlobusIoRegisterSelect(
	&newComm->ngc_ioHandle, ngcllCallbackNegotiationFromExecutable,
	newProto, context->ngc_log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the callback function for Select.\n", fName);
	goto error;
    }

end:
    /* Register the callback function for accept */
    result = ngcliGlobusIoTcpRegisterListen(
    	&comm->ngc_ioHandle, ngcliCallbackAccept, protocol,
	context->ngc_log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the callback function for Accept.\n", fName);
	return;
    }

    /* Success */
    return;

error:

    /* Destruct Protocol (and Communication) */
    if (newProto != NULL) {
	assert(newComm != NULL);

	result = ngiProtocolDestruct(newProto, context->ngc_log, NULL);
	newProto = NULL;
	newComm = NULL;
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol.\n", fName);
	}
    } 

    /* Destruct Communication */
    if (newComm != NULL) {
	result = ngiCommunicationDestruct(newComm, context->ngc_log, NULL);
	newComm = NULL;
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Communication.\n", fName);
	}
    }

    /* Set the error code */
    result = ngcliContextSetError(context, error, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the error code to the Ninf-G Context.\n",
	    fName);
    }

    /* Failed */
    return;
}

/**
 * Callback function for Negotiation Information.
 */
static void
ngcllCallbackNegotiationFromExecutable(
    void *cbArg,
    globus_io_handle_t *ioHandle,
    globus_result_t gResult)
{
    int result;
    int error;
    int lockJobList = 0;
    int lockExecutableList = 0;
    ngiCommunication_t *comm;
    ngiProtocol_t *protocol;
    ngclContext_t *context;
    ngclExecutable_t *executable = NULL;
    ngLog_t *log = NULL;
    ngcliJobManager_t *jobMng;
    ngiProtocolNegotiationFromExecutable_t *nego;
    long negoOpt[3+3];/* Zlib(3) + Division(3) */
#define NGL_NOPTIONS (sizeof (negoOpt) / sizeof (negoOpt[0]))
    int nNegoOpts;
    ngiByteStreamConversion_t conv;
    int protocolRegistered;
    int ioCallbackNeedFinish;
    static const char fName[] = "ngcllCallbackNegotiationFromExecutable";

    /* Check and get the arguments */
    assert(cbArg != NULL);
    protocol = (ngiProtocol_t *)cbArg;
    assert(protocol->ngp_communication);
    comm = protocol->ngp_communication;
    assert(ioHandle != NULL);
    assert(ioHandle == &comm->ngc_ioHandle);

    /* Initialize variable */
    protocolRegistered = 0;
    ioCallbackNeedFinish = 0;
    nego = NULL;

    /* Get the Ninf-G Context */
    context = ngiProtocolGetUserData(protocol, NULL, &error);
    if (context == NULL) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context is NULL.\n", fName);
	goto end;
    }

    /* Is the callback canceled? */
    result = ngiGlobusIsCallbackCancel(gResult, context->ngc_log, &error);
    if (result == 1) {
    	/**
	 * Callback was canceled
	 */
	/* Print the information message */
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
	    NULL, "%s: Callback was canceled.\n", fName);

	/* Success */
	goto end;
    }

    /* Did the error occur? */
    if (gResult != GLOBUS_SUCCESS) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback error was occurred.\n", fName);
        goto end;
    }

    /* Get the Negotiation Information */
    nego = ngiProtocolReceiveNegotiationFromExecutable(
    	protocol, context->ngc_log, &error);
    if (nego == NULL) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the Negotiation Information.\n", fName);
	goto end;
    }

    /**
     * Check the Negotiation Information.
     */

    /* Set the Version Number of a partner's Protocol */
    result = ngiProtocolSetProtocolVersionOfPartner(
	protocol, nego->ngpnfe_protocolVersion, context->ngc_log, &error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the version number of a partner's protocol.\n",
	    fName);
	goto end;
    }

    /* Get the Ninf-G Context */
    if (nego->ngpnfe_contextID != context->ngc_ID) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Context ID of received %d and Ninf-G Context %d are difference.\n",
	    fName, nego->ngpnfe_contextID, context->ngc_ID);
	goto end;
    }
    log = context->ngc_log;

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListWriteLock(context, log, &error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the list of Job Manager.\n", fName);
        goto error;
    }
    lockJobList = 1;

    /* Lock the list of Executable */
    result = ngclExecutableListWriteLock(context, &error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't lock the list of Executable.\n", fName);
        goto error;
    }
    lockExecutableList = 1;

    /* Get the Job Manager */
    jobMng = ngcliContextGetJobManager(context, nego->ngpnfe_jobID, &error);
    if (jobMng == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Job Manager is not found by ID %d.\n",
	    fName, nego->ngpnfe_jobID);
	goto error;
    }

    /* Set the end time */
    result = ngcliJobSetEndTimeOfInvoke(jobMng, log, &error);
    if (result == 0) {
        ngcliLogPrintfJob(jobMng, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End time.\n", fName);
    }

    /* Get the Executable */
    executable = ngcliJobGetNextExecutable(jobMng, NULL, &error);
    if (executable == NULL) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: No executable is registered.\n", fName);
	goto error;
    }
    while (executable->nge_status != NG_EXECUTABLE_STATUS_INITIALIZED) {
	executable = ngcliJobGetNextExecutable(jobMng, executable, &error);
	if (executable == NULL) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: No executable is found.\n", fName);
	    goto error;
	}
    }

    /* I/O Callback Start */
    result = ngcliExecutableIoCallbackStart(executable, log, &error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't start Executable I/O callback. \n", fName);
        goto error;
    }
    ioCallbackNeedFinish = 1;

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
	nego->ngpnfe_architectureID, NULL, &error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: The architecture code %d is not valid.\n",
	    fName, nego->ngpnfe_architectureID);
	goto end;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfe_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Protocol Version is not equal. My version is %#x and partner's version is %#x.\n",
            fName, NGI_PROTOCOL_VERSION, nego->ngpnfe_protocolVersion);
	/* This is not a error. Continue the process. */
    }

    /* Register the Communication Log */
    if (executable->nge_commLog != NULL) {
        result = ngiCommunicationLogRegister(
            comm, executable->nge_commLog, log, &error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the Communication Log.\n", fName);
            goto error;
        }
    }

    /* Check forceXDR. if it's set, set architecture ID to 0 */
    if (jobMng->ngjm_attr.ngja_forceXDR) {
        result = ngiProtocolSetArchitectureID(
            protocol, NGI_ARCHITECTURE_UNDEFINED, log, &error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't set the architecture ID.\n", fName);
            goto error;
        }
    }

    /* Is architecture same? */
    if ((nego->ngpnfe_architectureID == NGI_ARCHITECTURE_ID) &&
       (nego->ngpnfe_architectureID != NGI_ARCHITECTURE_UNDEFINED) &&
       (protocol->ngp_attr.ngpa_architecture != NGI_ARCHITECTURE_UNDEFINED)) {
	result = ngiProtocolSetXDR(protocol, NG_XDR_NOT_USE, log, &error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't set the XDR operation.\n", fName);
	    goto error;
	}
    }

    /* Set the Executable ID */
    protocol->ngp_attr.ngpa_executableID = executable->nge_ID;
    result = ngiProtocolSetExecutableID(
    	protocol, executable->nge_ID, log, &error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't set the Executable ID.\n", fName);
	goto error;
    }

    /* Connecting */
    result = ngcliExecutableConnecting(executable, protocol, log, &error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable can't process the Connecting.\n", fName);
	goto error;
    }
    protocolRegistered = 1;

    /* Unlock the list of Executable */
    result = ngclExecutableListWriteUnlock(context, &error);
    lockExecutableList = 0;
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't unlock the list of Executable.\n", fName);
        goto error;
    }

    /* Set the Conversion Method */
#ifndef NGI_NO_ZLIB
    conv.ngbsc_zlib =
	(jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionType ==
	 NG_COMPRESSION_TYPE_ZLIB) ? 1 : 0;
    conv.ngbsc_zlibThreshold =
	jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionThresholdNbytes;
#else /* NGI_NO_ZLIB */
    conv.ngbsc_zlib = 0;
    conv.ngbsc_zlibThreshold = 0;
#endif /* NGI_NO_ZLIB */
    conv.ngbsc_argumentBlockSize = 
        jobMng->ngjm_attr.ngja_rmInfo.ngrmi_argumentBlockSize;

    if (NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(
	    nego->ngpnfe_protocolVersion)) {
	result = ngiProtocolSetConversionMethod(
	    protocol, &conv,
	    &nego->ngpnfe_conversionMethod[0], nego->ngpnfe_nConversions,
	    log, &error);
    } else {
	result = ngiProtocolSetConversionMethod(
	    protocol, &conv, NULL, 0, log, &error);
    }
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Can't set the Conversion Method.\n", fName);
        goto error;
    }

    /* Unlock the list of Job Manager */
    result = ngcliContextJobManagerListWriteUnlock(context, log, &error);
    lockJobList = 0;
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the list of Job Manager.\n", fName);
        goto error;
    }

    /* Reply the result to Ninf-G Executable */
    result = ngiProtocolSendNegotiationResult(
    	protocol, NGI_PROTOCOL_RESULT_OK, context->ngc_log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't reply the result of Negotiation.\n", fName);
	goto error;
    }

    /* Is zlib enable? */
    nNegoOpts = 0;
#ifdef NGI_NO_ZLIB
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_RAW;
    negoOpt[nNegoOpts++] = 0;
#else /* NGI_NO_ZLIB */
    if (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionType ==
	NG_COMPRESSION_TYPE_RAW) {
	/* CAUTION:
	 * This option RAW is not necessary. However, remote executable it
	 * version 2.1.0 or before are require any options. Therefore, option
	 * RAW is send to remote executable.
	 */
	/* Initialize the Negotiation Options */
	negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_RAW;
	negoOpt[nNegoOpts++] = 0;
    } else {
	/* Initialize the Negotiation Options */
	negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_ZLIB;
	negoOpt[nNegoOpts++] = 1;
	negoOpt[nNegoOpts++] =
	    jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionThresholdNbytes;
    }
#endif /* NGI_NO_ZLIB */
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_DIVIDE;
    negoOpt[nNegoOpts++] = 1;
    negoOpt[nNegoOpts++] =
        jobMng->ngjm_attr.ngja_rmInfo.ngrmi_argumentBlockSize;

    assert(nNegoOpts <= NGL_NOPTIONS);

    /* Send the Negotiation Information to Ninf-G Executable */
    result = ngiProtocolSendNegotiationFromClient(
	protocol, &negoOpt[0], nNegoOpts, context->ngc_log, &error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't send the Negotiation Information.\n", fName);
	goto error;
    }

    /* Register the callback for Negotiation Result */
    result = ngcliGlobusIoRegisterSelect(
    	ioHandle, ngcllCallbackNegotiationResult, executable,
	context->ngc_log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function for Negotiation Result.\n",
	    fName);
	goto error;
    }
    ioCallbackNeedFinish = 0;

    /* Success */
    goto end;

    /* Error occurred */
error:
    /* Unregister the Communication Log */
    if ((executable != NULL) && (executable->nge_commLog != NULL)) {
        result = ngiCommunicationLogUnregister(comm, log, &error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the Communication Log.\n", fName);
        }
    }

    /* Unlock the list of Executable */
    if (lockExecutableList != 0) {
        lockExecutableList = 0;
        result = ngclExecutableListWriteUnlock(context, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't unlock the list of Executable.\n", fName);
        }
    }

    /* set Unusable to Executable */
    if (executable != NULL) {
	result = ngcliExecutableUnusable(executable, NG_ERROR_PROTOCOL, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Something wrong was happened in negotiation.\n", fName);
        }
    }

    /* Unlock the list of Job Manager */
    if (lockJobList != 0) {
        lockJobList = 0;
        result = ngcliContextJobManagerListWriteUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the list of Job Manager.\n", fName);
        }
    }


    /* Set the error code */
    result = ngcliContextSetError(context, error, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the error code to the Ninf-G Context.\n",
	    fName);
    }

end:
    /* Release the Negotiation Information */
    if (nego != NULL) {
	assert(context != NULL);
	result = ngiProtocolReleaseData(nego, log, NULL);
	nego = NULL;
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't release the Negotiation Information.\n", fName);
	}
    }

    /* Destruct Protocol */
    if (protocolRegistered == 0) {
	result = ngiProtocolDestruct(protocol, log, NULL);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Protocol.\n", fName);
	}
    } 

    /* Finish I/O callback */
    if (ioCallbackNeedFinish != 0) {
        result = ngcliExecutableIoCallbackFinish(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finished Executable I/O callback.\n", fName);
        }
    }

#undef NGL_NOPTIONS 
}

#if 0 /* Is this necessary? Use ngiProtocolIsArchitectureValid instead */
/**
 * Is architecture ID valid?
 */
static int
ngcllProtocolIsArchitectureValid(long archID, ngLog_t *log, int *error)
{
    int i;
    static const char fName[] = "ngcllProtocolIsArchitectureValid";

    for (i = 0; i < NGI_NARCHITECTUREIDS; i++) {
    	if (ngiArchitectureIDs[i] == archID) {
	    /* It is defined */
	    return 1;
	}
    }

    /* The architecture ID is not defined */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
    	"%s: The architecture ID %d is not defined.\n", fName);

    return 0;
}
#endif

/**
 * Callback function for Negotiation Result.
 */
static void
ngcllCallbackNegotiationResult(
    void *cbArg,
    globus_io_handle_t *ioHandle,
    globus_result_t gResult)
{
    int result;
    int error;
    ngclContext_t *context;
    int doQueryFunctionInformation;
    ngLog_t *log = NULL;
    ngclExecutable_t *executable;
    ngiProtocol_t *protocol;
    ngiProtocolNegotiationResult_t negoResult;
    static const char fName[] = "ngcllCallbackNegotiationResult";
    int ioCallbackNeedFinish;

    /* Initialize variable */
    ioCallbackNeedFinish = 1;;

    /* Check and get the arguments */
    assert(cbArg != NULL);
    executable = (ngclExecutable_t *)cbArg;
    assert(executable->nge_context != NULL);
    context = executable->nge_context;
    assert(executable->nge_protocol != NULL);
    protocol = executable->nge_protocol;
    assert(protocol->ngp_communication != NULL);
    assert(ioHandle != NULL);
    assert(ioHandle == &protocol->ngp_communication->ngc_ioHandle);

    /* Is Executable valid */
    result = ngcliExecutableIsValid(context, executable, &error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Executable is not valid.\n", fName);
	return;
    }

    /* Get the log */
    log = context->ngc_log;

    /* Is the callback canceled? */
    result = ngiGlobusIsCallbackCancel(gResult, log, &error);
    if (result == 1) {
    	/**
	 * Callback was canceled
	 */
	/* Print the information message */
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: Callback was canceled.\n", fName);

	/* Success */
        goto finish;
    }

    /* Did the error occur? */
    if (gResult != GLOBUS_SUCCESS) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback error was occurred.\n", fName);

        /* Failed */
        goto finish;
    }

    /* Get the Negotiation Information */
    result = ngiProtocolReceiveNegotiationResult(
    	protocol, &negoResult, log, &error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	     NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the Negotiation Result.\n", fName);

        /* Failed */
        goto finish;
    }

    /* Is negotiation success? */
    if (negoResult.ngpnr_result != NGI_PROTOCOL_RESULT_OK) {
    	/* Failed */
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Negotiation failed.\n", fName);
	goto error;
    }

    /* Initialize HeartBeat last arrived time */
    result = ngcliExecutableHeartBeatTimeInitialize(executable, &error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable can't set heartbeat arrived time.\n", fName);
        goto error;
    }

    /* Connected */
    result = ngcliExecutableConnected(executable, log, &error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable can't process the Connected.\n", fName);
	goto error;
    }

    /* Treat Remote Class Information Exist or not */
    result = ngcliExecutableRemoteClassInformationCheck(
        executable, &doQueryFunctionInformation, &error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't check the RemoteClassInformation.\n", fName);
        goto error;
    }

    /* Invoke the waiting session */
    result = ngcliExecutableExecuteSession(executable, log, &error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't invoke the waiting Session.\n", fName);
        goto error;
    }

    /* Idle */
    if (doQueryFunctionInformation == 0) {
        result = ngcliExecutableIdle(executable, log, &error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Executable can't process the Idle.\n", fName);
            goto error;
        }
    }

    /* Register the callback for Protocol */
    result = ngcliGlobusIoRegisterSelect(
    	ioHandle, ngcllCallbackProtocol, executable, log, &error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function for Protocol.\n", fName);
	goto error;
    }
    ioCallbackNeedFinish = 0;

finish:

    if (ioCallbackNeedFinish != 0) {
        /* Finish I/O callback */
        ioCallbackNeedFinish = 0;
        result = ngcliExecutableIoCallbackFinish(executable, log, &error);
        if (result == 0) {
            /* Error occurred */
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finished Executable I/O callback.\n", fName);
            goto error;
        }
    }

    /* Success */
    return;

    /* Error occurred */
error:
    /* Set the error code */
    result = ngcliContextSetError(context, error, NULL);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the error code to the Ninf-G Context.\n",
            fName);
    }

    /* Finish I/O callback */
    if (ioCallbackNeedFinish != 0) { 
        result = ngcliExecutableIoCallbackFinish(executable, log, NULL);
        if (result == 0) {
            /* Error occurred */
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finished Executable I/O callback.\n", fName);
        }
    }

    /* Failed */
    return;
}

/**
 * Callback function for Protocol.
 */
static void
ngcllCallbackProtocol(
    void *cbArg,
    globus_io_handle_t *ioHandle,
    globus_result_t gResult)
{
    int result;
    int error;
    int received;
    int type;
    ngiProtocolReceiveMode_t mode;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngiProtocol_t *proto;
    ngiProtocolHeader_t head;
    static const char fName[] = "ngcllCallbackProtocol";
    int ioCallbackNeedFinish;

    /* Initialize variable */
    ioCallbackNeedFinish = 1;

    /* Check and get the arguments */
    assert(cbArg != NULL);
    executable = (ngclExecutable_t *)cbArg;

    /* Is Executable valid */
    result = ngcliExecutableIsValid(NULL, executable, &error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Executable is not valid.\n", fName);
	return;
    }
    assert(executable->nge_context != NULL);
    assert(executable->nge_protocol != NULL);
    context = executable->nge_context;
    proto = executable->nge_protocol;
    assert(proto->ngp_communication != NULL);

    /* Output the log */
    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
    	NG_LOG_LEVEL_DEBUG, NULL, "%s: Start.\n", fName);
    
    /* Is the callback canceled? */
    result = ngiGlobusIsCallbackCancel(gResult, context->ngc_log, &error);
    if (result == 1) {
    	/**
	 * Callback was canceled
	 */
	/* Print the information message */
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: Callback was canceled.\n", fName);

	/* Success */
        goto finish;
    }

    /* Did the error occur? */
    if (gResult != GLOBUS_SUCCESS) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback error was occurred.\n", fName);
        goto finish;
    }

    mode = NGI_PROTOCOL_RECEIVE_MODE_WAIT;
    received = -1;

    /* Set the context ID to protocol */
    result = ngiProtocolSetIDofContext(proto, context->ngc_ID,
        context->ngc_log, &error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set context ID to protocol.\n", fName);
        goto error;
    }

    /* Set the executable ID to protocol */
    result = ngiProtocolSetIDofExecutable(proto, executable->nge_ID,
        context->ngc_log, &error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set executable ID to protocol.\n", fName);
        goto error;
    }

    while (1) {
        /* Receive the Protocol */
        result = ngiProtocolReceive(proto, &head, mode, &received,
            context->ngc_log, &error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
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
        result = ngcllProtocolProcess(
            context, executable, proto, &head, context->ngc_log, &error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Process the Protocol.\n", fName);
            goto error;
        }
 
        /* Release the Session ID of Protocol */
        result = ngiProtocolReleaseIDofSession(proto,
            context->ngc_log, &error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't Release the Session ID of Protocol.\n", fName);
            goto error;
        }

	/* Is callback register? */
	type = head.ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;
	if ((type == NGI_PROTOCOL_REQUEST_TYPE_REPLY) &&
	    ((head.ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK) ==
	      NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE)) {
	    /* Success */
            goto finish;
	}
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(proto);

    /* Output the log */
    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
    	NG_LOG_LEVEL_DEBUG, NULL, "%s: Register the callback.\n", fName);

    /* Register the callback for Protocol */
    result = ngcliGlobusIoRegisterSelect(
	ioHandle, ngcllCallbackProtocol, executable, context->ngc_log, &error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't register the callback function for Protocol.\n", fName);
	goto error;
    }
    ioCallbackNeedFinish = 0;

finish:

    /* Finish I/O callback */
    if (ioCallbackNeedFinish != 0) {
        ioCallbackNeedFinish = 0;
        result = ngcliExecutableIoCallbackFinish(executable, context->ngc_log,
            &error);
        if (result == 0) {
            /* Error occurred */
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finished Executable I/O callback.\n", fName);
            goto error;
        }
    }

    /* Success */
    return;
    
    /* Error occurred */
error:
    /* Make the Executable unusable */
    result = ngcliExecutableUnusable(executable, error, NULL);
    if (result == 0) {
        ngclLogPrintfExecutable(executable,
            NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Failed to set the executable unusable.\n",
            fName);
    }

    /* Set the error code */
    result = ngcliContextSetError(context, error, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the error code to the Ninf-G Context.\n",
	    fName);
    }

    /* Finish I/O callback */
    if (ioCallbackNeedFinish != 0) {
        result = ngcliExecutableIoCallbackFinish(executable, context->ngc_log,
            NULL);
        if (result == 0) {
            /* Error occurred */
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finished Executable I/O callback.\n", fName);
        }
    }

    /* Failed */
    return;
}

/**
 * Process the Protocol
 */
static int
ngcllProtocolProcess(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclSession_t *session = NULL;
    static const char fName[] = "ngcllProtocolProcess";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Session */
    if (header->ngph_sessionID != NGI_SESSION_ID_UNDEFINED) {
        session = ngclSessionGet(
            context, executable, header->ngph_sessionID, error);
        if (session == NULL) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Session is not found by ID %d.\n",
                fName, header->ngph_sessionID);
            goto error;
        }
    }

    /* Is Request Type valid? */
    switch (header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REPLY:
        result = ngcllProtocolProcessReply(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't process the Reply.\n", fName);
            goto error;
        }
        break;

    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
        result = ngcllProtocolProcessNotify(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't process the Notify.\n", fName);
            goto error;
        }
        break;

    default:
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request Type %d is not valid.\n",
            fName,
	    header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGI_SET_CB_ERROR_CONTEXT(context, NG_ERROR_PROTOCOL, NULL);
#if 0 /* Temporary comment out */
    NGI_SET_CB_ERROR_EXECUTABLE(executable, NG_ERROR_PROTOCOL, NULL);
    NGI_SET_CB_ERROR_SESSION(session, NG_ERROR_PROTOOCL, NULL);
#endif /* Temporary comment out */
    return 0;
}

/**
 * Process the Reply.
 */
static int
ngcllProtocolProcessReply(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int reply;

    static const char fName[] = "ngcllProtocolProcessReply";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Reply Code */
    reply = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (reply) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
        result = ngcllProtocolReplyQueryFunctionInformation(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Query Function Information.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
        result = ngcllProtocolReplyResetExecutable(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Reset Executable.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
        result = ngcllProtocolReplyExitExecutable(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Reset Executable.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
        result = ngcllProtocolReplyInvokeSession(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Invoke Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
        result = ngcllProtocolReplyCancelSession(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Cancel Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        result = ngcllProtocolReplyPullBackSession(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Pull Back Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        result = ngcllProtocolReplyTransferArgumentData(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Transfer Argument Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        result = ngcllProtocolReplyTransferResultData(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Transfer Result Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = ngcllProtocolReplyTransferCallbackArgumentData(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Transfer Callback Argument Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = ngcllProtocolReplyTransferCallbackResultData(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Reply Transfer Callback Result Data.\n",
                fName);
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Request Code %d is not valid.\n", fName, reply); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Notify.
 */
static int
ngcllProtocolProcessNotify(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int notify;
    static const char fName[] = "ngcllProtocolProcessNotify";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Reply Code */
    notify = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (notify) {
    case NGI_PROTOCOL_NOTIFY_CODE_I_AM_ALIVE:
        result = ngcliExecutableHeartBeatArrive(executable, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Notify heartbeat.\n", fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_NOTIFY_CODE_CALCULATION_END:
        assert(session != NULL);
        result = ngcllProtocolNotifyCalculationEnd(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Notify Calculation End.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK:
        result = ngcllProtocolNotifyInvokeCallback(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't Process the Notify Invoke Callback.\n",
                fName);
            return 0;
        }
        break;

    default:
        assert(session != NULL);
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
#if 0 /* Temporary comment out */
        NGI_SET_CB_ERROR_EXECUTABLE(executable, NG_ERROR_PROTOCOL);
        NGI_SET_CB_ERROR_SESSION(session, NG_ERROR_PROTOCOL);
#endif /* Temporary comment out */
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unknown Notify %#x.\n", fName, notify);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Send request: Query Function Information.
 */
int
ngcliProtocolRequestQueryFunctionInformation(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestQueryFunctionInformation";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Set the request */
    result = ngcliExecutableStatusSet(executable,
        NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED,
        log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL, 
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n",
	    fName);
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestQueryFunctionInformation(protocol, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't send the Request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Query Function Information.
 */
static int
ngcllProtocolReplyQueryFunctionInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    char *rcInfoString;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyQueryFunctionInformation";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(head != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does QUERY FUNCTION INFORMATION requested? */
    if (executable->nge_status !=
        NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not requested the QUERY FUNCTION INFORMATION.\n",
            fName);
        goto error;
    }

    /* Get the received Remote Class Information (XML string) */
    if (protocol->ngp_rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Remote Class Information was not available.\n",
            fName);
        goto error;
    }
    rcInfoString = protocol->ngp_rcInfo;
    protocol->ngp_rcInfo = NULL;

    /* Register Remote Class Information */
    result = ngcliExecutableRemoteClassInformationArrived(
        executable, rcInfoString, error);
    if (result  == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Register Remote Class Information failed.\n",
            fName);
        goto error;
    }

    /* Release Remote Class Information (XML string) */
    globus_libc_free(rcInfoString);

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Invoke the waiting session */
    result = ngcliExecutableExecuteSession(executable, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't invoke the waiting Session.\n", fName);
        goto error;
    }

    /* Idle */
    result = ngcliExecutableIdle(executable, log, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable can't process the Idle.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

#if 0 /* Temporary comment out */
/**
 * Send request: Query Executable Information.
 */
int
ngcliProtocolRequestQueryExecutableInformation(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * Receive reply: Query Executable Information.
 */
static int
ngcllProtocolReplyQueryExecutableInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    char *funcInfo;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyQueryExecutableInformation";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(head != NULL);
    assert(head->ngp_nBytes > 0);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does QUERY FUNCTION INFORMATION requested? */
    if (executable->nge_status !=
        NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not requested the "
            "QUERY EXECUTABLE INFORMATION.\n",
            fName);
        goto error;
    }

    /* Receive the Function Information */
    result = ngiReceiveReplyQueryExecutableInformation(
    	protocol, head, funcInfo, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't receive the Function Information.\n", fName);
	goto error;
    }

    /* Parse XML and register to cache */
    result = XXX();
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: \n", fName);
	goto error;
    }

    /* Release Function Information */
    result = ngiProtocolRelease(funcInfo, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the Function Information.\n", fName);
	goto error;
    }

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE,
        log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n", fName);
	goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Release Function Information */
    result = ngiProtocolRelease(funcInfo, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the Function Information.\n", fName);
	return 0;
    }

    /* Failed */
    return 0;
}
#endif /* Temporary comment out */

/**
 * Send request: Reset Executable.
 */
int
ngcliProtocolRequestResetExecutable(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestResetExecutable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Set the request */
    result = ngcliExecutableStatusSet(
    	executable, NG_EXECUTABLE_STATUS_RESET_REQUESTED, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL, 
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n",
	    fName);
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestResetExecutable(protocol, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't send the Request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Reset Executable.
 */
static int
ngcllProtocolReplyResetExecutable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyResetExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Check the Protocol Header */
    if (header->ngph_nBytes != 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Protocol: Parameter length %d is not zero.\n",
            fName, header->ngph_nBytes);
	return 0;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does EXECUTABLE RESET requested? */
    if (executable->nge_status != NG_EXECUTABLE_STATUS_RESET_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not requested the EXECUTABLE RESET.\n",
            fName);
        goto error;
    }

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_RESET_DONE, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL, 
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Exit Executable.
 */
int
ngcliProtocolRequestExitExecutable(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestExitExecutable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Set the request */
    result = ngcliExecutableStatusSet(
    	executable, NG_EXECUTABLE_STATUS_EXIT_REQUESTED, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL, 
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n",
	    fName);
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestExitExecutable(protocol, log, error);
    if (result == 0) {
    	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't send the Request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Exit Executable.
 */
static int
ngcllProtocolReplyExitExecutable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyExitExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Check the Protocol Header */
    if (header->ngph_nBytes != 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Protocol: Parameter length %d is not zero.\n",
            fName, header->ngph_nBytes);
        return 0;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does EXECUTABLE EXIT requested? */
    if (executable->nge_status != NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not requested the EXECUTABLE EXIT.\n",
            fName);
        goto error;
    }

    /* Save the result */
    executable->nge_requestResult = header->ngph_result;

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_EXIT_DONE, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Invoke Session.
 */
int
ngcliProtocolRequestInvokeSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestInvokeSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: INVOKE SESSION.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	return 0;
    }

    /* Invoke Session */
    result = ngiSendRequestInvokeSession(
	protocol, session->ngs_ID, session->ngs_rmInfo.ngrmi_methodID,
        log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Invoke Session.
 */
static int
ngcllProtocolReplyInvokeSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyInvokeSession";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does INVOKE SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_INVOKE_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the INVOKE SESSION.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: INVOKE SESSION.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
    } else {
        /* Transfer Argument Data */
        result = ngcllProtocolRequestTransferArgumentData(
            session, protocol, log, error);
    }
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Argument Data.
 */
static int
ngcllProtocolRequestTransferArgumentData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    int heartbeatModeChanged;
    static const char fName[] = "ngcllProtocolRequestTransferArgumentData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);
    assert(session->ngs_arg != NULL);

    heartbeatModeChanged = 0;

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: TRANSFER ARGUMENT DATA.\n", fName);

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStart(
        session->ngs_executable, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't change the heartbeat mode to start data transfer.\n",
            fName);
	goto error;
    }
    heartbeatModeChanged = 1;

    /* Start the measurement */
    result = ngiProtocolSessionInformationStartMeasurement(
	protocol, session->ngs_arg->nga_nArguments, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't start the measurement.\n", fName);
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_transferArgument, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the Start time.\n", fName);
	goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSARG_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Set the argument to protocol*/
    result = ngiProtocolSetArgument(protocol, session->ngs_arg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the argument.\n", fName);
	goto error;
    }

    /* Send request */
    result = ngiSendRequestTransferArgumentData(
	protocol, session->ngs_ID, session->ngs_arg, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't send the request.\n", fName);
	goto error;
    }

    /* Change the Heart Beat check mode */
    heartbeatModeChanged = 0;
    result = ngcliExecutableHeartBeatDataTransferStop(
        session->ngs_executable, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't change the heartbeat mode to stop data transfer.\n",
            fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Change the Heart Beat check mode */
    if (heartbeatModeChanged != 0) {
        heartbeatModeChanged = 0;
        result = ngcliExecutableHeartBeatDataTransferStop(
            session->ngs_executable, error);
        if (result == 0) {
            ngclLogPrintfSession(session,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't change the heartbeat mode to stop data transfer.\n",
                fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Argument Data.
 */
static int
ngcllProtocolReplyTransferArgumentData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyTransferArgumentData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does TRANSFER ARGUMENT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_TRANSARG_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the TRANSFER ARGUMENT DATA.\n",
            fName);
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_transferArgument, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End time.\n", fName);
        goto error;
    }

    /* check result */
    if (header->ngph_result == NGI_PROTOCOL_RESULT_BAD_ARGUMENT) {
        /* Unlock the Executable with send */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        /* disable the session */
        result = ngcliSessionUnusable(executable, session,
            NG_ERROR_INVALID_ARGUMENT, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invalid result data.\n",
                fName);
        }

        return 1;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSARG_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the request.\n", fName);
            goto error;
        }
    } else {
        /* Set the start time */
        result = ngiSetStartTime(
            &session->ngs_executionTime.ngs_calculation, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the start time.\n", fName);
            goto error;
        }
        
        /* Set the status */
        result = ngcliSessionStatusSet(
       	    session, NG_SESSION_STATUS_CALCULATE_EXECUTING, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the status.\n", fName);
            goto error;
        }

        /* Unlock the Executable with send */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify Calculation End.
 */
static int
ngcllProtocolNotifyCalculationEnd(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    int sendEnable = 0;
    static const char fName[] = "ngcllProtocolNotifyCalculationEnd";

    /* Check the arguments */
    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_calculation, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End time.\n", fName);
	goto error;
    }

    /* Output the log */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "Receive the Reply Calculation End.\n", fName);

    /* Is not Session calculating? */
    if (session->ngs_status != NG_SESSION_STATUS_CALCULATE_EXECUTING)
        goto success;

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CALCULATE_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_transferResult, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the start time.\n", fName);
	goto error;
    }

    /* Try lock for send */
    result = ngcliExecutableLockTrySend(executable, &sendEnable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "Can't lock for send.\n", fName);
        goto error;
    }

success:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }

    if (sendEnable != 0) {
        /* Send request */
        result = ngcliProtocolRequestTransferResultData(
            session, protocol, log, error);
       if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the request.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Result Data.
 */
int
ngcliProtocolRequestTransferResultData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestTransferResultData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: TRANSFER RESULT DATA.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSRES_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	return 0;
    }

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStart(
        session->ngs_executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to start data transfer.\n",
            fName);
	return 0;
    }
 
    /* Send the request */
    result = ngiSendRequestTransferResultData(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Transfer Result Data.
 */
static int
ngcllProtocolReplyTransferResultData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyTransferResultData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStop(
        executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to stop data transfer.\n",
            fName);
        goto error;
    }

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does TRANSFER RESULT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_TRANSRES_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the TRANSRES.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: TRANSFER RESULT DATA.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSRES_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
        goto error;
    }

    /* Release the argument from protocol */
    result = ngiProtocolReleaseArgument(executable->nge_protocol, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the argument.\n", fName);
        goto error;
    }

    /* Destruct the argument */
    if (session->ngs_arg) {
	result = ngiArgumentDestruct(session->ngs_arg, log, error);
	session->ngs_arg = NULL;
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destruct the argument.\n", fName);
	    return 0;
	}
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_transferResult, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the End time.\n", fName);
        goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Pull back session */
    result = ngcllProtocolRequestPullBackSession(
        session, protocol, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Cancel Session.
 */
int
ngcliProtocolRequestCancelSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestCancelSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: CANCEL SESSION.\n", fName);

    /* Set the request */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CANCEL_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiSendRequestCancelSession(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Cancel Session.
 */
static int
ngcllProtocolReplyCancelSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyCancelSession";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does CANCEL SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_CANCEL_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the CANCEL.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: TRANSFER ARGUMENT DATA.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CANCEL_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Pull Back Session */
    result = ngcllProtocolRequestPullBackSession(
        session, protocol, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Pull Back Session.
 */
static int
ngcllProtocolRequestPullBackSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllProtocolRequestPullBackSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: PULL BACK SESSION.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_PULLBACK_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestPullBackSession(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't send the request.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Pull Back Session.
 */
static int
ngcllProtocolReplyPullBackSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyPullBackSession";

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does PULL BACK SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_PULLBACK_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the PULLBACK SESSION.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: PULLBACK SESSION.\n", fName);

    /* Get the Session Information from protocol */
    result = ngiProtocolGetSessionInfo(
        protocol, &session->ngs_info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't get the Session Information from protocol.\n", fName);
        goto error;
    }

    /* Finish the measurement */
    result = ngiProtocolSessionInformationFinishMeasurement(
	protocol, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finish the measurement.\n", fName);
	goto error;
    }

    /* Set the execution time */
    result = ngcliSessionSetExecutionTime(
        context, executable->nge_jobMng, executable, session, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't set the Execution time.\n", fName);
        goto error;
    }

    /* Session is done */
    result = ngcliSessionStatusSetDone(session, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify Invoke Callback.
 */
static int
ngcllProtocolNotifyInvokeCallback(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int executableLocked = 0;
    int sendEnable;
    int result;
    static const char fName[] = "ngcllProtocolNotifyInvokeCallback";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Set the callback ID to Protocol */
        result = ngiProtocolSetCallbackID(protocol, NGI_CALLBACK_ID_UNDEFINED,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the callback ID.\n", fName);
            return 0;
        }

        /* Set the sequence number to Protocol */
        result = ngiProtocolSetSequenceNoOfCallback(protocol,
            NGI_PROTOCOL_SEQUENCE_NO_UNDEFINED, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the sequence number.\n", fName);
            return 0;
        }

        /* It was canceled. */
        return 1;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive notify: INVOKE CALLBACK.\n", fName);

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackTransferArgument, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the start time.\n", fName);
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Increment the number of times to which the callback was called */
    session->ngs_executionTime.ngs_callbackNtimesCalled++;

    /* Try lock for send */
    result = ngcliExecutableLockTrySend(executable, &sendEnable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "Can't lock for send.\n", fName);
        goto error;
    }

    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }

    if (sendEnable != 0) {
        /* Change the Heart Beat check mode */
        result = ngcliExecutableHeartBeatDataTransferStart(
            executable, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't change the heartbeat mode to start data transfer.\n",
                fName);
            goto error;
        }
 
        /* Send request */
        result = ngiSendRequestTransferCallbackArgumentData(
            protocol, session->ngs_ID, header->ngph_sequenceNo, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't send the request.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Callback Argument Data.
 */
int
ngcliProtocolRequestTransferCallbackArgumentData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    long sequenceNo;
    int result;
    static const char fName[] =
        "ngcliProtocolRequestTransferCallbackArgumentData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Send request: TRANSFER CALLBACK ARGUMENT DATA.\n", fName);

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStart(
        session->ngs_executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to start data transfer.\n",
            fName);
        return 0;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSRES_REQUESTED, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	return 0;
    }

    /* Get the Sequence number */
    sequenceNo = ngiProtocolGetSequenceNoOfCallback(protocol, log, error);
    if (sequenceNo == NGI_CALLBACK_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
        return 0;
    }

    /* Send request */
    result = ngiSendRequestTransferCallbackArgumentData(
        protocol, session->ngs_ID, sequenceNo, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the request.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Transfer Callback Argument Data.
 */
static int
ngcllProtocolReplyTransferCallbackArgumentData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg, *callbackArg;
    int callbackID;
    int executableLocked = 0;
    int i, count = 0;
    int result;
    static const char fName[] =
        "ngcllProtocolReplyTransferCallbackArgumentData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStop(
        executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to stop data transfer.\n",
            fName);
        goto error;
    }

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does INVOKE CALLBACK received? */
    if (session->ngs_status != NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the TRANSFER CALLBACK ARGUMENT DATA.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: TRANSFER CALLBACK ARGUMENT DATA.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSARG_DONE, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the session status.\n", fName);
	goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackTransferArgument, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End time.\n", fName);
	goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackTransferArgumentReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferArgumentReal,
	    session->ngs_executionTime.ngs_callbackTransferArgument.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackTransferArgumentCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferArgumentCPU,
	    session->ngs_executionTime.ngs_callbackTransferArgument.nget_cpu.
		nget_execution);

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackCalculation, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the start time.\n", fName);
	goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_EXECUTING, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
	goto error;
    }

    /* Get the callback ID */
    callbackID = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackID == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the callback ID.\n", fName);
	goto error;
    }

    /* Get the argument of callback */
    callbackArg = ngiProtocolGetCallbackArgument(protocol, log, error);
    if (callbackArg == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the argument of callback.\n", fName);
	goto error;
    }

    /* Get the argument */
    arg = ngiProtocolGetArgument(protocol, log, error);
    if (arg == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the argument.\n", fName);
        goto error;
    }

    for (i = 0; i < arg->nga_nArguments; i++) {
        if (arg->nga_argument[i].ngae_dataType ==
            NG_ARGUMENT_DATA_TYPE_CALLBACK) {
            if (count == callbackID) {
                break;
            }
            count += 1;
        }
    }
    if ((i == arg->nga_nArguments) && (count != callbackID)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback ID %d is not valid.\n", fName, callbackID);
        goto error;
    }

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        goto error;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
        callbackArg, arg,
        rmInfo->ngrmi_arguments[i].ngai_callback->ngrmi_arguments, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't initialize the Subscript Value of Argument.\n", fName);
        goto error;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(callbackArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Subscript Value of Argument is not valid.\n", fName);
        goto error;
    }

    /* Allocate the storage for Argument Data of OUT mode */
    result = ngiArgumentAllocateDataStorage(callbackArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't allocate the storage for Argument Data.\n", fName);
        goto error;
    }

    /* Invoke the Callback function */
    result = ngcliSessionInvokeCallback(
        arg->nga_argument[i].ngae_pointer.ngap_function,
        callbackArg, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't invoke the callback.\n", fName);
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_EXECUTE_DONE, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackCalculation, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End time.\n", fName);
        goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackCalculationReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackCalculationReal,
	    session->ngs_executionTime.ngs_callbackCalculation.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackCalculationCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackCalculationCPU,
	    session->ngs_executionTime.ngs_callbackCalculation.nget_cpu.
		nget_execution);

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackTransferResult, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the start time.\n", fName);
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSRES_REQUESTED, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStart(
        executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to start data transfer.\n",
            fName);
        goto error;
    }
 
    /* Send request */
    result = ngiSendRequestTransferCallbackResultData(
        protocol, session->ngs_ID, callbackArg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the request.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Callback Result Data.
 */
static int
ngcllProtocolReplyTransferCallbackResultData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngiArgument_t *callbackArg;
    int executableLocked = 0;
    int result;
    static const char fName[] = "ngcllProtocolReplyTransferCallbackResultData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Change the Heart Beat check mode */
    result = ngcliExecutableHeartBeatDataTransferStop(
        executable, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't change the heartbeat mode to stop data transfer.\n",
            fName);
        goto error;
    }

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not sending.\n", fName);
        goto error;
    }

    /* Does TRANSFER CALLBACK RESULT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_CB_TRANSRES_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not requested the TRANSFER CALLBACK RESULT DATA.\n",
            fName);
        goto error;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Receive reply: TRANSFER CALLBACK RESULT DATA.\n", fName);

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSRES_DONE, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackTransferResult, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End time.\n", fName);
        goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackTransferResultReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferResultReal,
	    session->ngs_executionTime.ngs_callbackTransferResult.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackTransferResultCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferResultCPU,
	    session->ngs_executionTime.ngs_callbackTransferResult.nget_cpu.
		nget_execution);

    /* Get the argument of callback */
    callbackArg = ngiProtocolGetCallbackArgument(protocol, log, error);
    if (callbackArg == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the argument of callback.\n", fName);
        goto error;
    }

    /* Release the argument of callback */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't Release the argument of callback.\n", fName);
        goto error;
    }

    /* Destruct the Argument */
    if (callbackArg != NULL) {
        result = ngiArgumentDestruct(callbackArg, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the argument.\n", fName);
            goto error;
        }
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't release the callback ID to protocol.\n", fName);
        goto error;
    }

    /* Release the Sequence number to protocol */
    result = ngiProtocolReleaseSequenceNoOfCallback(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Sequence number to protocol.\n", fName);
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CALCULATE_EXECUTING, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the status.\n", fName);
        goto error;
    }

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the request.\n", fName);
            goto error;
        }
    } else {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the Remote Method Information
 */
ngRemoteMethodInformation_t *
ngcliProtocolGetRemoteMethodInformation(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngclSession_t *session; 
    ngRemoteMethodInformation_t *rmInfo;
    int result;
    static const char fName[] = "ngcliProtocolGetRemoteMethodInformation";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Get the Ninf-G Context */
    context = ngcliNinfgManagerGetContext(
        (int)protocol->ngp_contextID, log, error);
    if (context == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Context is not found by ID %ld.\n",
            fName, protocol->ngp_contextID);
        return NULL;
    }

    /* Lock the list */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't lock the list of Executable.\n", fName);
        return NULL;
    }

    /* Get */
    executable = ngclExecutableGet(context, (int)protocol->ngp_executableID,
        error);
    if (executable == NULL) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
             NG_LOG_LEVEL_ERROR, NULL,
             "%s: Executable is not found by ID %ld.\n",
             fName, protocol->ngp_executableID);
        goto error1;
    }

    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
        return NULL;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't lock the list of Session.\n", fName);
        return NULL;
    }

    /* Get the Session */
    session = ngclSessionGet(
        context, executable, (int)protocol->ngp_sessionID, error);
    if (session == NULL) {
        ngclLogPrintfExecutable(executable,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not found by ID %d.\n",
            fName, protocol->ngp_sessionID);
        goto error2;
    }
    rmInfo = &session->ngs_rmInfo;

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
        return NULL;
    }

    /* Success */
    return rmInfo;

/* Error occurred */
error1:
    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't unlock the list of Executable.\n", fName);
    }
    return NULL;

error2:
    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't unlock the list of Session.\n", fName);
    }
    return NULL;
}
