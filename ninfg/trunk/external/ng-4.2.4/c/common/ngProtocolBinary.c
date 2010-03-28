#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngProtocolBinary.c,v $ $Revision: 1.72 $ $Date: 2006/08/29 02:59:00 $";
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
 * Module for managing Protocol for Ninf-G Client/Executable.
 * Binary Protocol part.
 */

#include <string.h>
#include <assert.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngiProtocolNegotiationFromExecutable_t *
    nglProtocolAllocateNegotiationFromExecutable(int, ngLog_t *, int *);
static ngiProtocolNegotiationFromClient_t *
    nglProtocolAllocateNegotiationFromClient(int, ngLog_t *, int *);

static int nglProtocolBinary_MakeRequestHeader(
    ngiProtocol_t *, ngiStreamManager_t *, int, int, long,
    ngLog_t *, int *);
static int nglProtocolBinary_MakeReplyHeader(
    ngiProtocol_t *, ngiStreamManager_t *, int, int, int, long,
    ngLog_t *, int *error);
static int nglProtocolBinary_MakeNotifyHeader(
    ngiProtocol_t *, ngiStreamManager_t *, int, int, long,
    ngLog_t *, int *);

static int
nglProtocolBinary_TransferArgumentData(
    ngiProtocol_t *, int, int, int, 
    ngCompressionInformation_t *,
    ngiArgument_t *, ngLog_t *, int *error);

static int nglProtocolBinary_SendRequest(
    ngiProtocol_t *, int, int, long, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglProtocolBinary_SendRequestCommon(
    ngiProtocol_t *, int, int, ngLog_t *, int *);
static int nglProtocolBinary_SendRequestInvokeSession(
    ngiProtocol_t *, int, long, ngLog_t *, int *);
static int nglProtocolBinary_SendRequestTransferArgumentData(
    ngiProtocol_t *, int, ngiArgument_t *, ngLog_t *, int *);
static int nglProtocolBinary_SendRequestTransferCallbackArgumentData(
    ngiProtocol_t *, int, long, ngLog_t *, int *);
static int nglProtocolBinary_SendRequestTransferCallbackResultData(
    ngiProtocol_t *, int, ngiArgument_t *, ngLog_t *, int *);

static int nglProtocolBinary_SendReply(
    ngiProtocol_t *, int, int, int, long, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglProtocolBinary_SendReplyCommon(
    ngiProtocol_t *, int, int, int, ngLog_t *, int *);
static int nglProtocolBinary_SendReplyQueryFunctionInformation(
    ngiProtocol_t *, int, char *, ngLog_t *, int *);
#if 0 /* Temporary comment out */
static int nglProtocolBinary_SendReplyQueryExecutableInformation(
    ngiProtocol_t *, int, char *, ngLog_t *, int *);
#endif /* Temporary comment out */
static int nglProtocolBinary_SendReplyPullBackSession(
    ngiProtocol_t *, int, int, char *, ngLog_t *, int *);
static int nglProtocolBinary_SendReplyTransferResultData(
    ngiProtocol_t *, int, int, ngiArgument_t *, ngLog_t *, int *);
static int nglProtocolBinary_SendReplyTransferCallbackArgumentData(
    ngiProtocol_t *, int, int, ngiArgument_t *, ngLog_t *, int *);
static int nglProtocolBinary_SendReplyTransferCallbackResultData(
    ngiProtocol_t *, int, int, long, ngLog_t *, int *);

static int nglProtocolBinary_SendNotify(
    ngiProtocol_t *, int, int, long, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglProtocolBinary_SendNotifyCommon(
    ngiProtocol_t *, int, int, ngLog_t *, int *);
static int nglProtocolBinary_SendNotifyInvokeCallback(
    ngiProtocol_t *, int, long, ngLog_t *, int *);

static int
nglProtocolBinary_MakeArgumentsData(
    ngiProtocol_t *, ngiArgument_t *, 
    ngCompressionInformation_t *, 
    ngRemoteMethodInformation_t *, int,
    int* , ngiStreamManager_t **, ngLog_t *, int *);
static int nglProtocolBinary_MakeArgumentData(
    ngiProtocol_t *, ngiArgumentElement_t *, ngCompressionInformation_t *,
    ngRemoteMethodInformation_t *, int, long, ngiStreamManager_t **,
    ngLog_t *, int *);
static ngCompressionInformationComplex_t* nglCompressionInfomationGetComplex(
    ngCompressionInformation_t *, int, int);
static int nglProtocolBinary_MakeArgumentDataOfSkip(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    ngLog_t *, int *);

static int nglProtocolBinary_checkHeader(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveRequest(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveReply(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveNotify(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveRequestInvokeSession(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveRequestTransferArgumentData(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveRequestTransferCallbackArgumentData(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveRequestTransferCallbackResultData(
    ngiProtocol_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReceiveReplyQueryFunctionInformation(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#if 0
static int nglProtocolBinary_ReceiveReplyQueryExecutableInformation(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#endif
static int nglProtocolBinary_ReceiveReplyPullBackSession(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveReplyTransferResultData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveReplyTransferCallbackArgumentData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReceiveReplyTransferCallbackResultData(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReceiveNotifyInvokeCallback(
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReceiveArgumentDataOfSkip(
    ngiArgumentElement_t *, ngiProtocol_t *, ngLog_t *, int *);

static int nglProtocolBinary_WriteNativeData(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, ngiDataDirect_t, size_t *, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_WriteNativeString(
    ngiProtocol_t *, ngiStreamManager_t *, char *, long,
    ngiProtocolContainLength_t, size_t *, ngLog_t *, int *);
static int nglProtocolBinary_WriteXDRstring(
    ngiProtocol_t *, ngiStreamManager_t *, char *, long,
    ngiProtocolContainLength_t, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReadXDRdataFromStream(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReadXDRdataWithReceive(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReadNativeDataFromStream(
    ngiProtocol_t *, ngiStreamManager_t *, ngArgumentDataType_t,
    void *, int, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReadNativeDataWithReceive(
    ngiProtocol_t *, ngArgumentDataType_t, void *, int, size_t *,
    ngLog_t *, int *);

static int nglProtocolBinary_ReadNativeStringFromStream(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReadNativeStringWithReceive(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);

static int nglProtocolBinary_ReadXDRstringFromStream(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);
static int nglProtocolBinary_ReadXDRstringWithReceive(
    ngiProtocol_t *, ngiStreamManager_t *, size_t, ngiProtocolContainLength_t,
    char **, size_t *, size_t *, ngLog_t *, int *);

#if defined(NG_OS_IRIX) || (__INTEL_COMPILER)
#define nglCheckSizetInvalid(expr) (0)
#else /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */
#define nglCheckSizetInvalid(expr) (expr)
#endif /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */

/**
 * Binary: Initialize the functions for Protocol Binary.
 */
int
ngiProtocolBinary_InitializeFunction(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{

    /* Check the arguments */
    assert(protocol != NULL);

    /* Send request */
    protocol->ngp_SendRequestCommon =
	nglProtocolBinary_SendRequestCommon;
    protocol->ngp_SendRequestInvokeSession =
    	nglProtocolBinary_SendRequestInvokeSession;
    protocol->ngp_SendRequestTransferArgumentData =
    	nglProtocolBinary_SendRequestTransferArgumentData;
    protocol->ngp_SendRequestTransferCallbackArgumentData = 
        nglProtocolBinary_SendRequestTransferCallbackArgumentData;
    protocol->ngp_SendRequestTransferCallbackResultData = 
        nglProtocolBinary_SendRequestTransferCallbackResultData;

    /* Send reply */
    protocol->ngp_SendReplyCommon =
        nglProtocolBinary_SendReplyCommon;
    protocol->ngp_SendReplyQueryFunctionInformation =
        nglProtocolBinary_SendReplyQueryFunctionInformation;
#if 0
    protocol->ngp_SendReplyQueryExecutableInformation =
        nglProtocolBinary_SendReplyQueryExecutableInformation;
#endif /* 0 */
    protocol->ngp_SendReplyPullBackSession =
    	nglProtocolBinary_SendReplyPullBackSession;
    protocol->ngp_SendReplyTransferResultData =
    	nglProtocolBinary_SendReplyTransferResultData;
    protocol->ngp_SendReplyTransferCallbackArgumentData =
        nglProtocolBinary_SendReplyTransferCallbackArgumentData;
    protocol->ngp_SendReplyTransferCallbackResultData = 
        nglProtocolBinary_SendReplyTransferCallbackResultData;

    /* Send notify */
    protocol->ngp_SendNotifyCommon = nglProtocolBinary_SendNotifyCommon;
    protocol->ngp_SendNotifyInvokeCallback =
        nglProtocolBinary_SendNotifyInvokeCallback;

    /* Success */
    return 1;
}

/**
 * Binary: Send the Negotiation Information from Executable.
 */
int
ngiProtocolSendNegotiationFromExecutable(
    ngiProtocol_t *protocol,
    long *option,
    int nOptions,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    ngiStreamManager_t *sendStream;
    long item[NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_NITEMS];
    static const char fName[] = "ngiProtocolSendNegotiationFromExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Make the Negotiation Information */
    index = 0;
    item[index++] = protocol->ngp_attr.ngpa_architecture;
    item[index++] = protocol->ngp_attr.ngpa_protocolVersion;
    item[index++] = protocol->ngp_attr.ngpa_contextID;
    item[index++] = protocol->ngp_attr.ngpa_jobID;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = protocol->ngp_xdrDataSize.ngds_long * nOptions;

    /* Did the storage of Negotiation Item overflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Negotiation Item overflowed.\n", fName);
	return 0;
    }

    /* Write the Negotiation Item to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sendStream, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], index, NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Negotiation Items to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Write the Conversion Methods to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sendStream, NG_ARGUMENT_DATA_TYPE_LONG, option, nOptions,
	NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Conversion Methods to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Send the Negotiation Information */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the Negotiation Information.\n", fName);
	return 0;
    }

    /* Destruct the Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send the Negotiation Information from Client.
 */
int
ngiProtocolSendNegotiationFromClient(
    ngiProtocol_t *protocol,
    long *option,
    int nOptions,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    ngiStreamManager_t *sendStream;
    long item[NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_NITEMS];
    static const char fName[] = "ngiProtocolSendNegotiationFromClient";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Make the Negotiation Information */
    index = 0;
    item[index++] = protocol->ngp_attr.ngpa_architecture;
    item[index++] = protocol->ngp_attr.ngpa_protocolVersion;
    item[index++] = protocol->ngp_attr.ngpa_contextID;
    item[index++] = protocol->ngp_attr.ngpa_executableID;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = protocol->ngp_xdrDataSize.ngds_long * nOptions;

    /* Did the storage of Negotiation Item overflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Negotiation Item overflowed.\n", fName);
	return 0;
    }

    /* Write the Negotiation Item to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sendStream, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], index, NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Negotiation Items to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Write the Conversion Methods to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sendStream, NG_ARGUMENT_DATA_TYPE_LONG, option, nOptions,
	NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Conversion Methods to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Send the Negotiation Information */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the Negotiation Information.\n", fName);
	return 0;
    }

    /* Destruct the Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send the Negotiation Result.
 */
int
ngiProtocolSendNegotiationResult(
    ngiProtocol_t *protocol,
    int negoResult,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamManager_t *sendStream;
    static const char fName[] = "ngiProtocolSendNegotiationResult";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Write the Negotiation Result to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sendStream, NG_ARGUMENT_DATA_TYPE_INT,
	&negoResult, 1, NULL, NULL, log, error);

    /* Send the Negotiation Result */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send the Negotiation Result.\n", fName);
	return 0;
    }

    /* Destruct the Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive the information for Negotiation.
 */
ngiProtocolNegotiationFromExecutable_t *
ngiProtocolReceiveNegotiationFromExecutable(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t requireNbytes, readNbytesFromStream;
    int index;
    int nConversions;
    long item[NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_NITEMS];
    ngiProtocolNegotiationFromExecutable_t *nego;
    static const char fName[] = "ngiProtocolReceiveNegotiationFromExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Get the number of bytes of Negotiation Items */
    requireNbytes = protocol->ngp_xdrDataSize.ngds_long * NGI_NELEMENTS(item);

    /* Receive the Negotiation Items */
    result = ngiStreamManagerReceiveFull(
    	protocol->ngp_sReceive, protocol->ngp_communication, requireNbytes,
	log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't receive the Negotiation Information.\n", fName);
	return 0;
    }

    /* Read the Negotiation Items from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
    	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't read the Negotiation Information.\n", fName);
	return 0;
    }

    /* Allocate the storage for Negotiation Information */
    nConversions =
    	item[NGI_PROTOCOL_NEGOTIATION_FROM_EXECUTABLE_INDEX_NCONVERSIONS]
	/ protocol->ngp_xdrDataSize.ngds_long;
    nego = nglProtocolAllocateNegotiationFromExecutable(
    	nConversions, log, error);
    if (nego == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Negotiation Information.\n",
	    fName);
	return 0;
    }

    /* Copy the item to Negotiation Information */
    index = 0;
    nego->ngpnfe_architectureID  = item[index++];
    nego->ngpnfe_protocolVersion = item[index++];
    nego->ngpnfe_contextID       = item[index++];
    nego->ngpnfe_jobID           = item[index++];
    nego->ngpnfe_notUsed1        = item[index++];
    nego->ngpnfe_notUsed2        = item[index++];
    nego->ngpnfe_notUsed3        = item[index++];
    nego->ngpnfe_nConversions    = nConversions;
    index++;

    /* Did the storage of Negotiation Item underflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: The storage of Negotiation Item underflowed.\n", fName);
	goto error;
    }

    /* Read the Conversion Method from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
    	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nego->ngpnfe_conversionMethod[0], nego->ngpnfe_nConversions,
	&readNbytesFromStream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't read the Conversion Method.\n", fName);
	goto error;
    }

    /* Destroy the Read Data */
    result = ngiStreamManagerDestroyReadData(
	protocol->ngp_sReceive, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't destroy the Read Data.\n", fName);
	goto error;
    }

    /* Success */
    return nego;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngiProtocolReleaseData(nego, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for Negotiation Information.\n",
	    fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Allocate the Negotiation Information from Executable.
 */
static ngiProtocolNegotiationFromExecutable_t *
nglProtocolAllocateNegotiationFromExecutable(
    int nConversions,
    ngLog_t *log,
    int *error)
{
    ngiProtocolNegotiationFromExecutable_t *nego;
    static const char fName[] = "nglProtocolAllocateNegotiationFromExecutable";

    /* Check the arguments */
    assert(nConversions >= 0);

    /* Allocate */
    nego = globus_libc_calloc(1,
	sizeof (ngiProtocolNegotiationFromExecutable_t)
        - sizeof (nego->ngpnfe_conversionMethod)
	+ (sizeof (nego->ngpnfe_conversionMethod[0]) * nConversions));
    if (nego == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Negotiation Information.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return nego;
}

/**
 * Receive the Negotiation Information from Client.
 */
ngiProtocolNegotiationFromClient_t *
ngiProtocolReceiveNegotiationFromClient(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t requireNbytes, readNbytesFromStream;
    int index;
    int nConversions;
    long item[NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_NITEMS];
    ngiProtocolNegotiationFromClient_t *nego;
    static const char fName[] = "ngiProtocolReceiveNegotiationFromClient";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Get the number of bytes of Negotiation Items */
    requireNbytes = protocol->ngp_xdrDataSize.ngds_long * NGI_NELEMENTS(item);

    /* Receive the Negotiation Items */
    result = ngiStreamManagerReceiveFull(
    	protocol->ngp_sReceive, protocol->ngp_communication, requireNbytes,
	log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't receive the Negotiation Information.\n", fName);
	return 0;
    }

    /* Read the Negotiation Items from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
    	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't read the Negotiation Information.\n", fName);
	return 0;
    }

    /* Allocate the storage for Negotiation Information */
    nConversions = 
    	item[NGI_PROTOCOL_NEGOTIATION_FROM_CLIENT_INDEX_NCONVERSIONS]
	/ protocol->ngp_xdrDataSize.ngds_long;
    nego = nglProtocolAllocateNegotiationFromClient(nConversions, log, error);
    if (nego == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Negotiation Information.\n",
	    fName);
	return 0;
    }

    /* Copy the item to Negotiation Information */
    index = 0;
    nego->ngpnfc_architectureID  = item[index++];
    nego->ngpnfc_protocolVersion = item[index++];
    nego->ngpnfc_contextID       = item[index++];
    nego->ngpnfc_executableID    = item[index++];
    nego->ngpnfc_notUsed1        = item[index++];
    nego->ngpnfc_notUsed2        = item[index++];
    nego->ngpnfc_notUsed3        = item[index++];
    nego->ngpnfc_nConversions    = nConversions;
    index++;

    /* Did the storage of Negotiation Item underflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Negotiation Item underflowed.\n", fName);
	goto error;
    }

    /* Read the Conversion Method from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
    	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nego->ngpnfc_conversionMethod[0], nego->ngpnfc_nConversions,
	&readNbytesFromStream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't read the Conversion Method.\n", fName);
	goto error;
    }

    /* Destroy the Read Data */
    result = ngiStreamManagerDestroyReadData(
	protocol->ngp_sReceive, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't destroy the Read Data.\n", fName);
	goto error;
    }

    /* Success */
    return nego;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngiProtocolReleaseData(nego, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for Negotiation Information.\n",
	    fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Allocate the Negotiation Information from Client.
 */
static ngiProtocolNegotiationFromClient_t *
nglProtocolAllocateNegotiationFromClient(
    int nConversions,
    ngLog_t *log,
    int *error)
{
    ngiProtocolNegotiationFromClient_t *nego;
    static const char fName[] = "nglProtocolAllocateNegotiationFromClient";

    /* Check the arguments */
    assert(nConversions >= 0);

    /* Allocate */
    nego = globus_libc_calloc(1,
	sizeof (ngiProtocolNegotiationFromClient_t)
        - sizeof (nego->ngpnfc_conversionMethod)
	+ (sizeof (nego->ngpnfc_conversionMethod[0]) * nConversions));
    if (nego == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Negotiation Information.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return nego;
}

/**
 * Receive the result of Negotiation.
 */
int
ngiProtocolReceiveNegotiationResult(
    ngiProtocol_t *protocol,
    ngiProtocolNegotiationResult_t *nego,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t requireNbytes, readNbytesFromStream;
    static const char fName[] = "ngiProtocolReceiveNegotiationResult";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Get the number of bytes of Negotiation Items */
    requireNbytes = protocol->ngp_xdrDataSize.ngds_long;

    /* Receive the Negotiation Items */
    result = ngiStreamManagerReceiveFull(
    	protocol->ngp_sReceive, protocol->ngp_communication, requireNbytes,
	log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't receive the Negotiation Information.\n", fName);
	return 0;
    }

    /* Read the Negotiation Items from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
    	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nego->ngpnr_result, 1,	&readNbytesFromStream, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't read the Negotiation Information.\n", fName);
	return 0;
    }

    /* Destroy the Read Data */
    result = ngiStreamManagerDestroyReadData(
	protocol->ngp_sReceive, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't destroy the Read Data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Make the Protocol Header.
 */
static int
nglProtocolBinary_MakeRequestHeader(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    int requestCode,
    int sessionID,
    long paramNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    u_long reqCode;
    long item[NGI_PROTOCOL_HEADER_NITEMS];
    static const char fName[] = "nglProtocolBinary_MakeRequestHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert((requestCode & NGI_PROTOCOL_REQUEST_TYPE_MASK) == 0);
    assert((sessionID == NGI_SESSION_ID_UNDEFINED) ||
	   ((sessionID >= NGI_SESSION_ID_MIN) &&
	    (sessionID <= NGI_SESSION_ID_MAX)));

    /**
     * Make the header.
     */
    /* Request Code */
    reqCode = requestCode;
    reqCode |= NGI_PROTOCOL_REQUEST_TYPE_REQUEST << NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;

    /* Increment the Sequence No. */
    protocol->ngp_sequenceNo++;
    if (protocol->ngp_sequenceNo > NGI_PROTOCOL_SEQUENCE_NO_MAX)
    	protocol->ngp_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;

    index = 0;
    item[index++] = reqCode;
    item[index++] = protocol->ngp_sequenceNo;
    item[index++] = protocol->ngp_attr.ngpa_contextID;
    item[index++] = protocol->ngp_attr.ngpa_executableID;
    item[index++] = sessionID;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = paramNbytes;

    /* Did the storage of Header Item overflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Header Item overflowed.\n", fName);
	return 0;
    }

    /* Write the Header Item to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], index, NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Header Items to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Make the Protocol Header.
 */
static int
nglProtocolBinary_MakeReplyHeader(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    int requestCode,
    int sessionID,
    int protoResult,
    long paramNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    u_long reqCode;
    long item[NGI_PROTOCOL_HEADER_NITEMS];
    static const char fName[] = "nglProtocolBinary_MakeReplyHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert((requestCode & NGI_PROTOCOL_REQUEST_TYPE_MASK) == 0);
    assert((sessionID == NGI_SESSION_ID_UNDEFINED) ||
	   ((sessionID >= NGI_SESSION_ID_MIN) &&
	    (sessionID <= NGI_SESSION_ID_MAX)));

    /**
     * Make the header.
     */
    /* Request Code */
    reqCode = requestCode;
    reqCode |= NGI_PROTOCOL_REQUEST_TYPE_REPLY << NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;

    index = 0;
    item[index++] = reqCode;
    item[index++] = protocol->ngp_sequenceNo;
    item[index++] = protocol->ngp_attr.ngpa_contextID;
    item[index++] = protocol->ngp_attr.ngpa_executableID;
    item[index++] = sessionID;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = protoResult;
    item[index++] = paramNbytes;

    /* Did the storage of Header Item overflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Header Item overflowed.\n", fName);
	return 0;
    }

    /* Write the Header Item to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], index, NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Header Items to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Make the Notify Header.
 */
static int
nglProtocolBinary_MakeNotifyHeader(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    int requestCode,
    int sessionID,
    long paramNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    u_long reqCode;
    long item[NGI_PROTOCOL_HEADER_NITEMS];
    static const char fName[] = "nglProtocolBinary_MakeNotifyHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert((requestCode & NGI_PROTOCOL_REQUEST_TYPE_MASK) == 0);
    assert((sessionID == NGI_SESSION_ID_UNDEFINED) ||
	   ((sessionID >= NGI_SESSION_ID_MIN) &&
	    (sessionID <= NGI_SESSION_ID_MAX)));

    /**
     * Make the header.
     */
    /* Request Code */
    reqCode = requestCode;
    reqCode |= NGI_PROTOCOL_REQUEST_TYPE_NOTIFY << NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;

    /* Increment the Sequence No. */
    protocol->ngp_notifySeqNo++;
    if (protocol->ngp_notifySeqNo > NGI_PROTOCOL_SEQUENCE_NO_MAX)
    	protocol->ngp_notifySeqNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;

    index = 0;
    item[index++] = reqCode;
    item[index++] = protocol->ngp_notifySeqNo;
    item[index++] = protocol->ngp_attr.ngpa_contextID;
    item[index++] = protocol->ngp_attr.ngpa_executableID;
    item[index++] = sessionID;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = NGI_PROTOCOL_NOT_USED;
    item[index++] = paramNbytes;

    /* Did the storage of Header Item overflow? */
    if (index > NGI_NELEMENTS(item)) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: The storage of Header Item overflowed.\n", fName);
	return 0;
    }

    /* Write the Header Item to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
    	protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], index, NULL, NULL, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the Header Items to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send request.
 */
static int
nglProtocolBinary_SendRequest(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    long paramNbytes,
    ngiStreamManager_t *paramStream,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamManager_t *sendStream;
    static const char fName[] = "nglProtocolBinary_SendRequest";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Make the Protocol Header */
    result = nglProtocolBinary_MakeRequestHeader(
    	protocol, sendStream, requestCode, sessionID, paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Append the parameter */
    if (paramStream != NULL) {
        result = ngiStreamManagerAppend(sendStream, paramStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append the Stream Manager.\n", fName);
            return 0;
        }
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send request.\n",
	    fName);
	return 0;
    }

    /* Destruct the all Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send request: common.
 */
static int
nglProtocolBinary_SendRequestCommon(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolBinary_SendRequestCommon";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send the Header */
    result = nglProtocolBinary_SendRequest(
        protocol, requestCode, sessionID, 0, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send request.\n", fName);
        goto error;
    }
    
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Transfer Argument or Result Data.
 */
static int
nglProtocolBinary_TransferArgumentData(
    ngiProtocol_t *protocol,
    int when,
    int protoResult,
    int sessionID,
    ngCompressionInformation_t *compInfo,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiStreamManager_t *paramStream;
    int nArg;
    long nArguments;
    size_t argNbytes = 0;
    long argumentNbytes = 0;
    int sentHeader = 0;
    int sendMutexLocked = 0;
    int result;
    int ret = 0;
    int i;
    int isTooLarge;
    long callbackID;
    long sequenceNo; 
    static const char fName[] =
        "nglProtocolBinary_TransferArgumentData";

    /* Check Argument */
    assert(protocol != NULL);
    assert(arg != NULL);

    /* Lock the SendMutex */
    assert(sendMutexLocked == 0);
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        goto finalize;
    }
    sendMutexLocked = 1;

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        goto finalize;
    }

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        goto finalize;
    }

    /* Write Callback Infomation */
    switch(when) {
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        /* Get the callback ID to Protocol */
        callbackID = ngiProtocolGetCallbackID(protocol, log, error);
        if (callbackID == NGI_CALLBACK_ID_UNDEFINED) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: callback ID %d is undefined.\n", fName, callbackID);
            goto finalize;
        }
        /* Write the Callback ID */
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
            &callbackID, 1, NULL, NULL, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't write the Callback ID.\n", fName);
            goto finalize;
        }

        /* Get the Sequence number */
        sequenceNo = ngiProtocolGetSequenceNoOfCallback(
            protocol, log, error);
        if (sequenceNo == NGI_CALLBACK_ID_UNDEFINED) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get the Sequence number.\n", fName);
            goto finalize;
        }
        /* Write the Sequence number */
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
            &sequenceNo, 1, NULL, NULL, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't write the Sequence number.\n", fName);
            goto finalize;
        }
        break;
    default:
        /* Do nothing */
        break;
    }

    /* Get number of arguments */
    result = ngiProtocolGetNargumentsSent(
        protocol, arg, when, &nArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get number of arguments sent.\n", fName);
        goto finalize;
    }
    nArguments = nArg;

    /* Write number of arguments */
    result = ngiProtocolBinary_WriteXDRdata(
        protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG,
        &nArguments, 1, NULL, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't write the number of arguments.\n", fName);
        goto finalize;
    }

    i = 0;
    do {
        /* Make the argument data */
        result = nglProtocolBinary_MakeArgumentsData(
            protocol, arg, compInfo, rmInfo, when, &i,
            &paramStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't make the arguments data.\n", fName);
            goto finalize;
        }

        /* The argument is divided */
        if (sentHeader == 0) {
            sentHeader = 1;

            argumentNbytes = -1;
            if (i == arg->nga_nArguments) {
                /* Get Stream Manager Size */
                result = ngiStreamManagerGetTotalBytesOfReadableData(
                        paramStream, &isTooLarge, &argNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s:Can't get total bytes of readable data "
                        "from the StreamManager.\n", fName);
                    goto finalize;
                }
                if (isTooLarge != 0) {
                    NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR,
                        NULL, "%s:Arguments's size is too large.\n", fName);
                    goto finalize;
                }
                argumentNbytes = argNbytes;
            }

            switch (when) {
            case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
            case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
                /* Send the Request Header */
                result = nglProtocolBinary_SendRequest(
                    protocol, when, sessionID, argumentNbytes, NULL, log,
                    error);
                break;
            case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
            case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
                /* Send the Reply Header */
                result = nglProtocolBinary_SendReply(
                    protocol, when, sessionID, protoResult, argumentNbytes,
                    NULL, log, error);
                break;
            default:
                assert(0);
            }
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't send the Request.\n", fName);
                goto finalize;
            }
        }

        /* Send the Argument data */
        if (paramStream != NULL) {
            result = ngiStreamManagerSend(
                paramStream, protocol->ngp_communication, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't send argument data.\n", fName);
                goto finalize;
            }

            /* Destruct the Stream Manager */
            result = ngiStreamManagerDestruct(paramStream, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destruct the Stream Manager.\n", fName);
                goto finalize;
            }
            paramStream = NULL;
        }
    } while (i < arg->nga_nArguments);

    ret = 1;
finalize:
    /* Unlock the SendMutex */

    /* If succeed, Mutex has been locked. */
    assert(!((ret != 0) && (sendMutexLocked == 0)));

    if (sendMutexLocked != 0) { 
        sendMutexLocked = 0;
        result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
            ret = 0;
        }
    }

    return ret;
}

/**
 * Binary: Send request: Invoke Session.
 */
static int
nglProtocolBinary_SendRequestInvokeSession(
    ngiProtocol_t *protocol,
    int sessionID,
    long methodID,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t paramNbytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] = "nglProtocolBinary_SendRequestInvokeSession";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Write the Method ID */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&methodID, 1, NULL, &paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Method ID.\n", fName);
	return 0;
    }

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendRequest(
    	protocol, NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION,
        sessionID, paramNbytes, paramStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Request.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send request: Transfer Argument Data.
 */
static int
nglProtocolBinary_SendRequestTransferArgumentData(
    ngiProtocol_t *protocol,
    int sessionID,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "nglProtocolBinary_SendRequestTransferArgumentData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(((protocol->ngp_sessionInfo.ngsi_nCompressionInformations == 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation == NULL)) ||
	   ((protocol->ngp_sessionInfo.ngsi_nCompressionInformations > 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation != NULL)));
    result = nglProtocolBinary_TransferArgumentData(
        protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA,
        0, sessionID, 
        protocol->ngp_sessionInfo.ngsi_compressionInformation,
        arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't transfer argument data.\n", fName);
        return 0;
    }

    return 1;
}

/**
 * Binary: Send request: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_SendRequestTransferCallbackArgumentData(
    ngiProtocol_t *protocol,
    int sessionID,
    long sequenceNo,
    ngLog_t *log,
    int *error)
{
    long sequenceNoOfProtocol;
    long callbackID;
    int result;
    size_t paramNbytes, nBytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] =
        "nglProtocolBinary_SendRequestTransferCallbackArgumentData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    paramNbytes = 0;

    /* Get the callback ID to Protocol */
    callbackID = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackID == NGI_CALLBACK_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: callback ID %d is undefined.\n", fName, callbackID);
        return 0;
    }

    /* Write the Callback ID */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&callbackID, 1, NULL, &nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Callback ID.\n", fName);
	return 0;
    }
    paramNbytes += nBytes;

    /* Get the Sequence number */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
        return 0;
    }

    /* Write the Sequence number */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&sequenceNo, 1, NULL, &nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Sequence number.\n", fName);
	return 0;
    }
    paramNbytes += nBytes;

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendRequest(
    	protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA,
        sessionID, paramNbytes, paramStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Request.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send request: Transfer Callback Result Data.
 */
static int
nglProtocolBinary_SendRequestTransferCallbackResultData(
    ngiProtocol_t *protocol,
    int sessionID,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "nglProtocolBinary_SendRequestTransferCallbackResultData";

    result = nglProtocolBinary_TransferArgumentData(
        protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA,
        0, sessionID, NULL, arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't transfer callback's result data.\n", fName);
        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send reply.
 */
static int
nglProtocolBinary_SendReply(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    int protoResult,
    long paramNbytes,
    ngiStreamManager_t *paramStream,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamManager_t *sendStream;
    static const char fName[] = "nglProtocolBinary_SendReply";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Make the Protocol Header */
    result = nglProtocolBinary_MakeReplyHeader(
    	protocol, sendStream, requestCode, sessionID, protoResult,
        paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Append the parameter */
    if (paramStream != NULL) {
        result = ngiStreamManagerAppend(sendStream, paramStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append the Stream Manager.\n", fName);
            return 0;
        }
    }

    /* Send reply */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send reply.\n",
	    fName);
	return 0;
    }

    /* Destruct the all Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send reply: common.
 */
static int
nglProtocolBinary_SendReplyCommon(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    int protoResult,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolBinary_SendReplyCommon";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send the Header */
    result = nglProtocolBinary_SendReply(
        protocol, requestCode, sessionID, protoResult, 0, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send reply.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send reply: Query Function Information.
 */
static int
nglProtocolBinary_SendReplyQueryFunctionInformation(
    ngiProtocol_t *protocol,
    int protoResult,
    char *funcInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    long strNbytes;
    size_t paramNbytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] = "nglProtocolBinary_SendReplyQueryFunctionInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(funcInfo != NULL);
    assert(funcInfo[0] != '\0');

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    strNbytes = strlen(funcInfo);
    assert(strNbytes > 0);

    /* Write the information */
    result = nglProtocolBinary_WriteXDRstring(
        protocol, paramStream, funcInfo, strNbytes,
        NGI_PROTOCOL_NOT_CONTAIN_LENGTH, &paramNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't write to the Stream Manager.\n", fName);
        return 0;
    }

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendReply(
        protocol, NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION,
        NGI_SESSION_ID_UNDEFINED, protoResult, paramNbytes, paramStream,
        log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Reply.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

#if 0 /* Temporary comment out */
/**
 * Binary: Send reply: Query Executable Information.
 */
static int
nglProtocolBinary_SendReplyQueryExecutableInformation(
    ngiProtocol_t *protocol,
    int protoResult,
    char *info,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t paramNbytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] = "nglProtocolBinary_SendReplyQueryExecutableInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);
    assert(info != NULL);
    assert(info[0] != '\0');

    /* Construct the Stream Manager */
    paramStream = ngiStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Write data to Stream Manager */
    paramNbytes = strlen(funInfo);
    result = ngiStreamManagerWrite(paramStream, info, paramNbytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't write the function information to Stream Manager.\n",
	    fName);
	return 0;
    }

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendReply(
    	protocol, NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION,
        NGI_SESSION_ID_UNDEFINED, protoResult, paramNbytes, paramStream,
        log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Reply.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}
#endif /* Temporary commented out */

/**
 * Binary: Send reply: Pull Back Session.
 */
static int
nglProtocolBinary_SendReplyPullBackSession(
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    char *sessionInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    long strNbytes;
    size_t paramNbytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] = "nglProtocolBinary_SendReplyPullBackSession";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(sessionInfo != NULL);
    assert(sessionInfo[0] != '\0');

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    strNbytes = strlen(sessionInfo);
    assert(strNbytes > 0);

    /* Write the information */
    result = nglProtocolBinary_WriteXDRstring(
	protocol, paramStream, sessionInfo, strNbytes,
        NGI_PROTOCOL_NOT_CONTAIN_LENGTH, &paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write to the Stream Manager.\n", fName);
	return 0;
    }

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendReply(
    	protocol, NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION,
        sessionID, protoResult, paramNbytes, paramStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Reply.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send reply: Transfer Result Data.
 */
static int
nglProtocolBinary_SendReplyTransferResultData(
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "nglProtocolBinary_SendReplyTransferResultData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(((protocol->ngp_sessionInfo.ngsi_nCompressionInformations == 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation == NULL)) ||
	   ((protocol->ngp_sessionInfo.ngsi_nCompressionInformations > 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation != NULL)));

    result = nglProtocolBinary_TransferArgumentData(
        protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA,
        protoResult, sessionID, 
        protocol->ngp_sessionInfo.ngsi_compressionInformation,
        arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't transfer result data.\n", fName);
        /* Failed */
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send reply: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_SendReplyTransferCallbackArgumentData(
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "nglProtocolBinary_SendReplyTransferCallbackArgumentData";

    result = nglProtocolBinary_TransferArgumentData(
        protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA,
        0, sessionID, NULL, arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't transfer callback's result data.\n", fName);
        /* Failed */
        return 0;
    }
    /* Success */
    return 1;
}

/**
 * Binary: Send reply: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_SendReplyTransferCallbackResultData(
    ngiProtocol_t *protocol,
    int sessionID,
    int protoResult,
    long sequenceNo,
    ngLog_t *log,
    int *error)
{
    long callbackID;
    long sequenceNoOfProtocol;
    int result;
    size_t paramNbytes, nBytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] =
        "nglProtocolBinary_SendReplyTransferCallbackResultData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol != NULL);
    assert(((protocol->ngp_sessionInfo.ngsi_nCompressionInformations == 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation == NULL)) ||
	   ((protocol->ngp_sessionInfo.ngsi_nCompressionInformations > 0) &&
	    (protocol->ngp_sessionInfo.ngsi_compressionInformation != NULL)));

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    paramNbytes = 0;

    /* Get the callback ID to Protocol */
    callbackID = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackID == NGI_CALLBACK_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: callback ID %d is undefined.\n", fName, callbackID);
        return 0;
    }

    /* Write the Callback ID */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&callbackID, 1, NULL, &nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Callback ID.\n", fName);
	return 0;
    }
    paramNbytes += nBytes;

    /* Get the Sequence number */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
        return 0;
    }

    /* Write the Sequence number */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&sequenceNoOfProtocol, 1, NULL, &nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Sequence number.\n", fName);
	return 0;
    }
    paramNbytes += nBytes;

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendReply(
    	protocol, NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA,
        sessionID, protoResult, paramNbytes, paramStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Reply.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send notify.
 */
static int
nglProtocolBinary_SendNotify(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    long paramNbytes,
    ngiStreamManager_t *paramStream,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamManager_t *sendStream;
    static const char fName[] = "nglProtocolBinary_SendNotify";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    sendStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (sendStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Make the Protocol Header */
    result = nglProtocolBinary_MakeNotifyHeader(
    	protocol, sendStream, requestCode, sessionID, paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Set the Sequence number to Protocol */
    if (requestCode == NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK) {
        result = ngiProtocolSetSequenceNoOfCallback(
            protocol, protocol->ngp_notifySeqNo, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the callback ID.\n", fName);
            return 0;
        }
    }

    /* Append the parameter */
    if (paramStream != NULL) {
        result = ngiStreamManagerAppend(sendStream, paramStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append the Stream Manager.\n", fName);
            return 0;
        }
    }

    /* Send notify */
    result = ngiStreamManagerSend(
    	sendStream, protocol->ngp_communication, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't send request.\n",
	    fName);
	return 0;
    }

    /* Destruct the all Stream Manager */
    result = ngiStreamManagerDestruct(sendStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Send notify: common.
 */
static int
nglProtocolBinary_SendNotifyCommon(
    ngiProtocol_t *protocol,
    int requestCode,
    int sessionID,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolBinary_SendNotifyCommon";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send the Header */
    result = nglProtocolBinary_SendNotify(
        protocol, requestCode, sessionID, 0, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send notify.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Send notify: Invoke Callback.
 */
static int
nglProtocolBinary_SendNotifyInvokeCallback(
    ngiProtocol_t *protocol,
    int sessionID,
    long callbackID,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t paramNbytes;
    ngiStreamManager_t *paramStream;
    static const char fName[] = "nglProtocolBinary_SendNotifyInvokeCallback";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Construct the Stream Manager */
    paramStream = ngiMemoryStreamManagerConstruct(
        NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (paramStream == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct Stream Manager.\n", fName);
        return 0;
    }

    /* Write the Callback ID */
    result = ngiProtocolBinary_WriteXDRdata(
	protocol, paramStream, NG_ARGUMENT_DATA_TYPE_LONG, 
	&callbackID, 1, NULL, &paramNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Callback ID.\n", fName);
	return 0;
    }

    /* Lock the SendMutex */
    result = ngiMutexLock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't lock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Send with Header */
    result = nglProtocolBinary_SendNotify(
    	protocol,  NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK,
        sessionID, paramNbytes, paramStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't send the Notify.\n", fName);
        goto error;
    }

    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the SendMutex */
    result = ngiMutexUnlock(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s:Can't unlock the mutex for protocol send\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Make arguments
 */
static int
nglProtocolBinary_MakeArgumentsData(
    ngiProtocol_t *protocol,
    ngiArgument_t *arg,
    ngCompressionInformation_t *compInfo,
    ngRemoteMethodInformation_t *rmInfo,
    int when,
    int* argumentNumber,
    ngiStreamManager_t **sMng,
    ngLog_t *log,
    int *error)
{
    int i;
    int result;
    int isSent;
    static const char fName[] = "nglProtocolBinary_MakeArgumentsData";

    /* Check arguments */
    assert(protocol != NULL);
    assert(arg != NULL);
    assert(rmInfo != NULL);
    assert(argumentNumber != NULL);
    assert(*argumentNumber >= 0);
    assert(sMng != NULL);
    
    i = *argumentNumber;
    for (;i < arg->nga_nArguments;i++) {
        /* Get get whether argument element is sent or not. */
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i], when, &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }

        if (isSent == 0) {
            /* Doesn't send */
            continue;
        }

        /* Make the argument data */
        result = nglProtocolBinary_MakeArgumentData(
            protocol, &arg->nga_argument[i], compInfo, rmInfo, when, i + 1,
            sMng, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't make the argument data.\n", fName);
            return 0;
        }

        if (arg->nga_argument[i].ngae_typeOfDivision != NGI_TYPE_OF_DIVISION_NONE) {
            /* The argument is divided */
            break;
        }
    }
    *argumentNumber = i;

    return 1;
}


/**
 * Binary: Make argument data.
 */
static int
nglProtocolBinary_MakeArgumentData(
    ngiProtocol_t *protocol,
    ngiArgumentElement_t *argElement,
    ngCompressionInformation_t *compInfo,
    ngRemoteMethodInformation_t *rmInfo,
    int when,
    long argumentNumber,
    ngiStreamManager_t **sMng,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    int index;
    size_t nBytes;
    size_t paramNbytes;
    long networkNbytes;
    ngiStreamManager_t *smArgHeader = NULL;
    ngiStreamManager_t *smArgument = NULL;
    ngiStreamManager_t *smSkipInfo = NULL;
    long conversion = NGI_CONVERSION_RAW;
    long nElements;
    long prevTypeOfDivision;
    long item[NGI_PROTOCOL_ARGUMENT_DATA_NITEMS];
    int isTooLarge;
    ngCompressionInformationComplex_t *compComplex = NULL;
    static const char fName[] = "nglProtocolBinary_MakeArgumentData";

    /* Check arguments */
    assert(protocol != NULL);
    assert(argElement != NULL);
    assert(rmInfo != NULL);
    assert(argumentNumber > 0);
    assert(sMng != NULL);

    nElements = argElement->ngae_nElements;
    prevTypeOfDivision = argElement->ngae_typeOfDivision;

    compComplex= nglCompressionInfomationGetComplex(
        compInfo, when, argumentNumber);/* Doesn't failed*/

    /* Encode the argument */
    result = ngiArgumentEncode(
	argElement, compComplex,
        protocol, &smArgument, when, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't encode the Argument Data.\n", fName);
	return 0;
    }

    /* First Call */
    if (prevTypeOfDivision == NGI_TYPE_OF_DIVISION_NONE) {
        /* Get size */
        if (argElement->ngae_sMngRemain == NULL) {
            result = ngiStreamManagerGetTotalBytesOfReadableData(
                smArgument, &isTooLarge, &paramNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s:Can't get total bytes of readable data from the StreamManager.\n", fName);
                goto error;
            }
            if (isTooLarge != 0) {
                NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s:Argument's size is too large.\n", fName);
            }
            networkNbytes = paramNbytes;
        } else {
            /* Argument is divided */
            networkNbytes = -1;
        }

        /* Check the Shrink */
        if ((rmInfo->ngrmi_shrink == 1) &&
            (argElement->ngae_subscript != NULL)) {

            /* Shrink */
            smSkipInfo = ngiMemoryStreamManagerConstruct(
                NGI_PROTOCOL_STREAM_NBYTES, log, error);
            if (smSkipInfo == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't construct the Stream Manager.\n", fName);
                goto error;
            }

            result = nglProtocolBinary_MakeArgumentDataOfSkip(
                argElement, protocol, smSkipInfo, log, error);
            if (result ==0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't send the Argument Data of Skip.\n", fName);
                return 0;
            }

            nElements = 1;
            for (i = 0; i < argElement->ngae_nDimensions; i++) {
                nElements *= 
                    (argElement->ngae_subscript[i].ngsv_end -
                     argElement->ngae_subscript[i].ngsv_start + 
                     argElement->ngae_subscript[i].ngsv_skip - 1) /
                    argElement->ngae_subscript[i].ngsv_skip;
            }

            if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
                conversion = NGI_CONVERSION_XDR_SKIP;
            } else {
                conversion = NGI_CONVERSION_SKIP;
            }

            if (smArgument != NULL) {
                /* Append the information of skip */
                result = ngiStreamManagerAppend(smSkipInfo, smArgument, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't append the Stream Manager.\n", fName);
                    goto error;
                }
                smArgument = smSkipInfo;
                smSkipInfo = NULL;
            }
        }

        /* Make Argument Header */
        smArgHeader = ngiMemoryStreamManagerConstruct(
            NGI_PROTOCOL_STREAM_NBYTES, log, error);
        if (smArgHeader == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Stream Manager.\n", fName);
            goto error;
        }

        index = 0;
        item[index++] = argumentNumber;
        item[index++] = argElement->ngae_dataType;
        item[index++] = argElement->ngae_ioMode;
        item[index++] = conversion;
        item[index++] = nElements;
        item[index++] = networkNbytes;
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, smArgHeader, NG_ARGUMENT_DATA_TYPE_LONG,
            &item[0], index, NULL, &nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the Header Items to Stream Manager.\n", fName);
            goto error;
        }

        if (*sMng != NULL) {
            /* Append the argument header */
            result = ngiStreamManagerAppend(*sMng, smArgHeader, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't append the Stream Manager.\n", fName);
                goto error;
            }
        } else {
            *sMng = smArgHeader;
        }
        smArgHeader = NULL;
    }

    /* Argument Stream Manager is valid? */
    if (smArgument != NULL) {
        if (*sMng != NULL) {
            /* Append the argument data */
            result = ngiStreamManagerAppend(*sMng, smArgument, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't append the Stream Manager.\n", fName);
                goto error;
            }
        } else {
            *sMng = smArgument;
        }
        smArgument = NULL;
    }

    /* Success */
    return 1;
error:
    if (smArgHeader != NULL) {
        /* Destruct the Stream Manager */
        result = ngiStreamManagerDestruct(smArgHeader, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        }
        smArgHeader = NULL;
    }
    if (smSkipInfo != NULL) {
        /* Destruct the Stream Manager */
        result = ngiStreamManagerDestruct(smSkipInfo, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        }
        smSkipInfo = NULL;
    }
    if (smArgument != NULL) {
        /* Destruct the Stream Manager */
        result = ngiStreamManagerDestruct(smArgument, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Stream Manager.\n", fName);
        }
        smArgument = NULL;
    }
    return 0;
}

/**
 * Get Compression Infomation Element
 */
static ngCompressionInformationComplex_t *
nglCompressionInfomationGetComplex(
    ngCompressionInformation_t *compInfo,
    int when, 
    int argumentNumber)
{
    ngCompressionInformationComplex_t* ret = NULL;

    /* Check arguments */
    assert(argumentNumber > 0);

    if (compInfo != NULL) {
        switch(when) {
        case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
            ret = &compInfo[argumentNumber - 1].ngci_in;
            break;
        case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
            ret = &compInfo[argumentNumber - 1].ngci_out;
            break;
        case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
            break;
        default:
            /* Not reached */
            abort();
        }
    }
    return ret;
}

/**
 * Binary: Send: Argument Data of Skip
 */
static int
nglProtocolBinary_MakeArgumentDataOfSkip(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sSkipInfo,
    ngLog_t *log,
    int *error)
{
    size_t nBytes;
    long *skipInfo;
    int index;
    int i;
    int result;
    static const char fName[] = "nglProtocolBinary_MakeArgumentDataOfSkip";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sSkipInfo != NULL);

    /* Write the Skip Information to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
        protocol, sSkipInfo, NG_ARGUMENT_DATA_TYPE_INT,
        &argElement->ngae_nDimensions, 1, NULL, &nBytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the Skip Information to Stream Manager.\n",
            fName);
        return 0;
    }

    /* Allocate the data buffer */
    skipInfo = globus_libc_malloc(
        argElement->ngae_nDimensions * sizeof(long) * 4);
    if (skipInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for skip Information.\n", fName);
        return 0;
    }
    for (i = 0, index = 0; i < argElement->ngae_nDimensions; i++) {
        skipInfo[index++] = argElement->ngae_subscript[i].ngsv_size;
        skipInfo[index++] = argElement->ngae_subscript[i].ngsv_start;
        skipInfo[index++] = argElement->ngae_subscript[i].ngsv_end;
        skipInfo[index++] = argElement->ngae_subscript[i].ngsv_skip;
    }

    /* Write the Skip Information to Stream Manager */
    result = ngiProtocolBinary_WriteXDRdata(
        protocol, sSkipInfo, NG_ARGUMENT_DATA_TYPE_LONG,
        &skipInfo[0], index, NULL, &nBytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the Shrink Information to Stream Manager.\n",
            fName);

        if (skipInfo != NULL) {
            globus_libc_free(skipInfo);
            skipInfo = NULL;
        }
        return 0;
    }

    if (skipInfo != NULL) {
        globus_libc_free(skipInfo);
        skipInfo = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive Data.
 */
int
ngiProtocolBinary_Receive(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngiProtocolReceiveMode_t mode,
    int *received,
    ngLog_t *log,
    int *error)
{
    int result;
    long reqType;
    long reqCode;
    static const char fName[] = "ngiProtocolBinary_Receive";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Receive the header */
    result = ngiProtocolBinary_ReceiveHeader(protocol, header, mode, received,
        log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't receive the Protocol Header.\n", fName);
        return 0;
    }

    if (received != NULL) {
        if (*received == 0) {
            return 1;
        }
    }

    /* Is Protocol header valid? */
    result = nglProtocolBinary_checkHeader(protocol, header, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Protocol Header is not valid.\n", fName);
        return 0;
    }

    /* Update the Sequence No. */
    result = ngiProtocolUpdateSequenceNo(protocol, header, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Protocol Sequence No. is not valid.\n", fName);
        return 0;
    }

    /* Does all data receive?
     * All data is received when it corresponds to below:
     *   - Request is not TRANSFER ARGUMENT DATA
     *   - Reply is not TRANSFER RESULT DATA
     */
    reqType = header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT;
    reqCode = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;
    if (((reqType == NGI_PROTOCOL_REQUEST_TYPE_REQUEST) &&
	 (reqCode == NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA)) ||
	((reqType == NGI_PROTOCOL_REQUEST_TYPE_REPLY) &&
	 (reqCode == NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA)) ||
	((reqType == NGI_PROTOCOL_REQUEST_TYPE_REPLY) &&
	 (reqCode == NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA)) ||
	((reqType == NGI_PROTOCOL_REQUEST_TYPE_REQUEST) &&
	 (reqCode == NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA))) {
	 /* Do nothing */
    } else {
	/* Receive the Parameter */
	result = ngiStreamManagerReceive(
	    protocol->ngp_sReceive, protocol->ngp_communication,
	    header->ngph_nBytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't receive the Protocol Parameter.\n", fName);
	    return 0;
	}
    }

    switch (header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
        result = nglProtocolBinary_ReceiveRequest(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request.\n", fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_TYPE_REPLY:
        result = nglProtocolBinary_ReceiveReply(protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Reply.\n", fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
        result = nglProtocolBinary_ReceiveNotify(protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Notify.\n", fName);
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request Type %d is not valid.\n",
            fName,
	    header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive the Protocol Header.
 */
int
ngiProtocolBinary_ReceiveHeader(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *protoHeader,
    ngiProtocolReceiveMode_t mode,
    int *received,
    ngLog_t *log,
    int *error)
{
    int result;
    int index;
    size_t requireNbytes, receiveNbytes, readNbytesFromStream;
    long item[NGI_PROTOCOL_HEADER_NITEMS];
    static const char fName[] = "ngiProtocolBinary_ReceiveHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protoHeader != NULL);

    if (received != NULL) {
        *received = 0;
    }

    /* Destroy the data it was readed */
    result = ngiStreamManagerDestroyReadData(
        protocol->ngp_sReceive, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destroy the data it was readed.\n", fName);
        return 0;
    }

    /* Get the number of bytes of Header Items */
    requireNbytes = protocol->ngp_xdrDataSize.ngds_long * NGI_NELEMENTS(item);

    /* Receive the Header Items */
    if (mode == NGI_PROTOCOL_RECEIVE_MODE_WAIT) {
        result = ngiStreamManagerReceive(
            protocol->ngp_sReceive, protocol->ngp_communication, requireNbytes,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Protocol Header.\n", fName);
            return 0;
        }
    } else {
        assert(received != NULL);
        *received = 0;
        result = ngiStreamManagerReceiveTry(
            protocol->ngp_sReceive, protocol->ngp_communication,
            requireNbytes, &receiveNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Protocol Header.\n", fName);
            return 0;
        }

        if (receiveNbytes < requireNbytes) {
            /* Success */
            return 1;
        }
        *received = 1;
    }

    /* Read the Protocol Header from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataFromStream(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Protocol Header.\n", fName);
        return 0;
    }

    /* Copy the item to Header Information */
    index = 0;
    protoHeader->ngph_requestCode  = item[index++];
    protoHeader->ngph_sequenceNo   = item[index++];
    protoHeader->ngph_contextID    = item[index++];
    protoHeader->ngph_executableID = item[index++];
    protoHeader->ngph_sessionID    = item[index++];
    protoHeader->ngph_notUsed1     = item[index++];
    protoHeader->ngph_result       = item[index++];
    protoHeader->ngph_nBytes       = item[index++];

    /* Did the storage of Header Item underflow? */
    if (index > NGI_NELEMENTS(item)) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
            NULL, "%s: The storage of Header Item underflowed.\n", fName);
        return 0;
    }

    if (received != NULL) {
        *received = 1;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Check the Protocol Header.
 */
int
nglProtocolBinary_checkHeader(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolBinary_checkHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(header != NULL);

#if 0
    /* Is Request Type valid? */
    if ((header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) !=
        NGI_PROTOCOL_REQUEST_TYPE_REQUEST) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request type %d is not REQUEST %d.\n", fName,
            header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT,
            NGI_PROTOCOL_REQUEST_TYPE_REQUEST);
        return 0;
    }
#endif

    /* Is Context ID valid? */
    if (header->ngph_contextID != protocol->ngp_contextID) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Context ID %d is not valid.\n",
            fName, header->ngph_contextID);
        return 0;
    }

    /* Is Executable ID valid? */
    if (header->ngph_executableID != protocol->ngp_executableID) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable ID %d is not valid.\n",
            fName, header->ngph_executableID);
        return 0;
    }

    /* Is Session ID valid? */
    if ((header->ngph_sessionID != NGI_SESSION_ID_UNDEFINED) &&
	((header->ngph_sessionID < NGI_SESSION_ID_MIN) ||
	 (header->ngph_sessionID > NGI_SESSION_ID_MAX))) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Session ID %d is not valid.\n",
            fName, header->ngph_sessionID);
        return 0;
    }

    /* Check the session ID */
    if (protocol->ngp_sessionID == NGI_SESSION_ID_UNDEFINED) {
        /* Set the session ID to protocol */
        result = ngiProtocolSetIDofSession(protocol, header->ngph_sessionID,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set session ID to protocol.\n", fName);
            return 0;
        }
    }

    /* Is Not Used1 valid? */
    if (header->ngph_notUsed1 != NGI_PROTOCOL_NOT_USED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Not Used1 %d is not valid.\n",
            fName, header->ngph_notUsed1);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive the Request.
 */
int
nglProtocolBinary_ReceiveRequest(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int request;
    int result;
    static const char fName[] = "nglProtocolBinary_ReceiveRequest";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Request Code */
    request = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    /* Check the Request Code */
    switch (request) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
        result = nglProtocolBinary_ReceiveRequestInvokeSession(
            protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Invoke Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        result = nglProtocolBinary_ReceiveRequestTransferArgumentData(
            protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Transfer Argument Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = nglProtocolBinary_ReceiveRequestTransferCallbackArgumentData(
            protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Transfer Argument Data for Callback.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = nglProtocolBinary_ReceiveRequestTransferCallbackResultData(
            protocol, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Transfer Result Data for Callback.\n",
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
 * Binary: Receive the Reply.
 */
int
nglProtocolBinary_ReceiveReply(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int request;
    int result;
    static const char fName[] = "nglProtocolBinary_ReceiveReply";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Check result */
    if (header->ngph_result < 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: Remote executable failed process requested by client.\n",
            fName);
    }

    /* Get the Request Code */
    request = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    /* Check the Request Code */
    switch (request) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
        result = nglProtocolBinary_ReceiveReplyQueryFunctionInformation(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Reply of Query Function Information.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
# if 0
        result = nglProtocolBinary_ReceiveReplyQueryExecutableInformation(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Reply of Query Executable Information.\n",
                fName);
            return 0;
        }
        break;
#endif

    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        result = nglProtocolBinary_ReceiveReplyPullBackSession(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Reply of Pull Back Session.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        result = nglProtocolBinary_ReceiveReplyTransferResultData(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Reply of Transfer Result Data.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = nglProtocolBinary_ReceiveReplyTransferCallbackArgumentData(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Transfer Argument Data for Callback.\n",
                fName);
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = nglProtocolBinary_ReceiveReplyTransferCallbackResultData(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Request of Transfer Result Data for Callback.\n",
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
 * Binary: Receive the Notify.
 */
int
nglProtocolBinary_ReceiveNotify(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int notify;
    int result;
    static const char fName[] = "nglProtocolBinary_ReceiveNotify";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Notify Code */
    notify = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    /* Check the Notify Code */
    switch (notify) {
    case NGI_PROTOCOL_NOTIFY_CODE_I_AM_ALIVE:
    case NGI_PROTOCOL_NOTIFY_CODE_CALCULATION_END:
        break;

    case NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK:
        result = nglProtocolBinary_ReceiveNotifyInvokeCallback(
            protocol, header, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Notify of Invoke Callback.\n",
                fName);
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Notify Code %d is not valid.\n", fName, notify); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive Request: Invoke Session.
 */
int
nglProtocolBinary_ReceiveRequestInvokeSession(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    long methodID;
    int result;
    size_t readNbytesFromStream;
    static const char fName[] =
        "nglProtocolBinary_ReceiveRequestInvokeSession";

    /* Check the argument */
    assert(protocol != NULL);

    /* Read the Method ID */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&methodID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Method ID.\n", fName);
        return 0;
    }

    /* Set the Method ID to Protocol */
    result = ngiProtocolSetMethodID(protocol, methodID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the Method ID.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive Request: Transfer Argument Data.
 */
int
nglProtocolBinary_ReceiveRequestTransferArgumentData(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg;
    ngiProtocolArgumentData_t argHead;
    long nArgs, item[NGI_PROTOCOL_ARGUMENT_DATA_NITEMS];
    int i, index;
    int result;
    long protocolVersion;
    size_t readNbytesFromStream;
    int isSent;
    static const char fName[] =
       "nglProtocolBinary_ReceiveRequestTransferArgumentData";

    /* Check the argument */
    assert(protocol != NULL);

   /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
        protocol, &protocolVersion, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the version number of partner's protocol.\n",
            fName);
        return 0;
    }

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        return 0;
    }

    /* Construct the Argument */
    arg = ngiArgumentConstruct(
        rmInfo->ngrmi_arguments, rmInfo->ngrmi_nArguments, log, error);
    if (arg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the Argument.\n", fName);
        return 0;
    }

    /* Allocate the Subscript Value of Argument */
    for (i = 0; i < arg->nga_nArguments; i++) {
	if (arg->nga_argument[i].ngae_nDimensions == 0) {
	    continue;
	}
        arg->nga_argument[i].ngae_subscript = ngiSubscriptValueAllocate(
            arg->nga_argument[i].ngae_nDimensions, log, error);
        if (arg->nga_argument[i].ngae_subscript == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't allocate the Subscript Value of Argument.\n",
                fName);
            return 0;
        }
    }

    /* Read the number of arguments */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nArgs, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Number Of Arguments.\n", fName);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Argument count %d is not valid.\n", fName, nArgs);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs > arg->nga_nArguments) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Argument count %d over the Method Info Argument Information.\n",
            fName, nArgs);
        return 0;
    }

    /* Receive the argument */
    for (i = 0; i < arg->nga_nArguments; i++) {
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i],
            NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA,
            &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }

        if (isSent == 0) {
            continue;
        }

        /* Read the Argument Header Items from Stream Manager */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	    &item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Argument Data.\n", fName);
            return 0;
        }

        /* Copy the item to Argument Header Information */
        index = 0;
        argHead.ngpad_no        = item[index++];
        argHead.ngpad_type      = item[index++];
        argHead.ngpad_ioMode    = item[index++];
        argHead.ngpad_encode    = item[index++];
        argHead.ngpad_nElements = item[index++];
        argHead.ngpad_nBytes    = item[index++];

        /* Did the storage of Argument Data Item overflow? */
        if (index > NGI_NELEMENTS(item)) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: The storage of Argument Data Item overflowed.\n", fName);
            return 0;
        }

        if (argHead.ngpad_no != (i + 1)) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Argument No. is not valid: Require %d, receive %d.\n",
                fName, i + 1, argHead.ngpad_no);
            return 0;
        }

        if (argHead.ngpad_type != arg->nga_argument[i].ngae_dataType) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Data type is not valid: Require %d, receive %d.\n",
                fName, arg->nga_argument[i].ngae_dataType,
                argHead.ngpad_type);
            return 0;
        }

        if (argHead.ngpad_ioMode != arg->nga_argument[i].ngae_ioMode) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: I/O mode is not valid: Require %d, receive %d.\n",
                fName, arg->nga_argument[i].ngae_ioMode,
                argHead.ngpad_ioMode);
            return 0;
        }

        /* Check whether it is Skip data */
        if ((argHead.ngpad_encode == NGI_CONVERSION_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
            result = nglProtocolBinary_ReceiveArgumentDataOfSkip(
                &arg->nga_argument[i], protocol, log, error);
            if (result ==0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't receive the Argument Data of Skip.\n", fName);
                return 0;
            }

        }

        /* Decode the argument data */
        result = ngiArgumentDecode(
            &arg->nga_argument[i],
	    &protocol->ngp_sessionInfo.ngsi_compressionInformation[i].ngci_in,
	    1, protocol, &argHead, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't decode the Argument Data.\n", fName);
            return 0;
        }

        /* Initialize the argument data */
        result = ngiArgumentElementInitializeData(
            &arg->nga_argument[i], arg->nga_argument[i].ngae_pointer,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Argument Data.\n", fName);
            return 0;
        }
    }

    /* Free the Subscript Value */
    for (i = 0; i < arg->nga_nArguments; i++) {
        if (arg->nga_argument[i].ngae_subscript != NULL) {
            result = ngiSubscriptValueFree(
                arg->nga_argument[i].ngae_subscript, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR,	NULL,
                    "%s: Can't construct the Subscript Value.\n", fName);
                return 0;
            }
            arg->nga_argument[i].ngae_subscript = NULL;
        }
    }

    /* Set the Argument */
    result = ngiProtocolSetArgument(protocol, arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't set the Argument Data.\n", fName);
        return 0;
    }

    /* Success */
    return 1;   
}

/**
 * Binary: Receive request: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_ReceiveRequestTransferCallbackArgumentData(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    size_t readNbytesFromStream;
    long sequenceNo, sequenceNoOfProtocol;
    long callbackID, callbackIDofProtocol;
    int result;
    static const char fName[] =
        "nglProtocolBinary_ReceiveRequestTransferCallbackArgumentData";

    /* Check the argument */
    assert(protocol != NULL);

    /* Read the Callback ID */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&callbackID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Callback ID.\n", fName);
        return 0;
    }
    /* Read the Sequence number */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&sequenceNo, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Sequence number.\n", fName);
        return 0;
    }

    /* Get the callback ID */
    callbackIDofProtocol = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackIDofProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the callback ID.\n", fName);
	return 0;
    }
    /* Check the Callback ID */
    if (callbackID != callbackIDofProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received callback ID is not valid.\n", fName);
	return 0;
    }

    /* Get the sequence number of Callback */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the sequence number.\n", fName);
	return 0;
    }
    /* Check the Sequence number of Callback */
    if (sequenceNo != sequenceNoOfProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received Sequence number is not valid.\n", fName);
	return 0;
    }

    /* Success */
    return 1;   
}

/**
 * Binary: Receive request: Transfer Callback Result Data.
 */
static int
nglProtocolBinary_ReceiveRequestTransferCallbackResultData(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo, *rmInfoCallback;
    ngiArgument_t *arg;
    ngiProtocolArgumentData_t argHead;
    long nArgs, item[NGI_PROTOCOL_ARGUMENT_DATA_NITEMS];
    long sequenceNo, sequenceNoOfProtocol;
    long callbackID, callbackIDofProtocol;
    int index;
    int count = 0;
    int i;
    int result;
    int isSent;
    size_t readNbytesFromStream;
    static const char fName[] =
        "nglProtocolBinary_ReceiveRequestTransferCallbackResultData";

    /* Check the argument */
    assert(protocol != NULL);

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        return 0;
    }

    /* Read the Callback ID */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&callbackID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Callback ID.\n", fName);
        return 0;
    }
    /* Read the Sequence number */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&sequenceNo, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Sequence number.\n", fName);
        return 0;
    }

    /* Get the callback ID */
    callbackIDofProtocol = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackIDofProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the callback ID.\n", fName);
	return 0;
    }
    /* Check the Callback ID */
    if (callbackID != callbackIDofProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received callback ID is not valid.\n", fName);
	return 0;
    }

    /* Get the Sequence number */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
	return 0;
    }
    /* Check the Sequence number */
    if (sequenceNo != sequenceNoOfProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received Sequence number is not valid.\n", fName);
	return 0;
    }

    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
        if (rmInfo->ngrmi_arguments[i].ngai_dataType ==
            NG_ARGUMENT_DATA_TYPE_CALLBACK) {
            if (count == callbackIDofProtocol) {
                break;
            }
            count += 1;
        }
    }
    if ((i == rmInfo->ngrmi_nArguments) && (count != callbackIDofProtocol)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback ID %d is not valid.\n", fName, callbackIDofProtocol);
        return 0;
    }
    rmInfoCallback = rmInfo->ngrmi_arguments[i].ngai_callback;

    /* Get the Argument of Callback */
    arg = ngiProtocolGetCallbackArgument(protocol, log, error);
    if (arg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Argument from protocol.\n", fName);
        return 0;
    }

    /* Read the number of arguments */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nArgs, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read the Number Of Arguments.\n", fName);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Argument count %d is not valid.\n", fName, nArgs);
	return 0;
    }

    /* Is argument count valid? */
    if (nArgs > rmInfoCallback->ngrmi_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Argument count %d over the Method Info Argument Information.\n",
	    fName, nArgs);
	return 0;
    }

    /* Is argument count zero? */
    if (nArgs == 0) {
	/* Success */
	return 1;
    }

    /* Receive the argument */
    for (i = 0; i < rmInfoCallback->ngrmi_nArguments; i++) {
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i],
            NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA,
            &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }

        if (isSent == 0) {
            /* Doesn't receive */
            continue;
        }

	/* Receive the items */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	    &item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Argument Data.\n", fName);
            return 0;
        }

        /* Copy the item to Argument Header Information */
        index = 0;
        argHead.ngpad_no        = item[index++];
        argHead.ngpad_type      = item[index++];
        argHead.ngpad_ioMode    = item[index++];
        argHead.ngpad_encode    = item[index++];
        argHead.ngpad_nElements = item[index++];
        argHead.ngpad_nBytes    = item[index++];

        /* Did the storage of Argument Data Item overflow? */
        if (index > NGI_NELEMENTS(item)) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: The storage of Argument Data Item overflowed.\n", fName);
            return 0;
        }

        if (argHead.ngpad_no != (i + 1)) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Argument No. is not valid: Require %d, receive %d.\n",
                fName, i, argHead.ngpad_no);
            return 0;
        }

        if (argHead.ngpad_type !=
            rmInfoCallback->ngrmi_arguments[i].ngai_dataType) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Data type is not valid: Require %d, receive %d.\n",
                fName, rmInfoCallback->ngrmi_arguments[i].ngai_dataType,
                argHead.ngpad_type);
            return 0;
        }

        if (argHead.ngpad_ioMode !=
            rmInfoCallback->ngrmi_arguments[i].ngai_ioMode) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: I/O mode is not valid: Require %d, receive %d.\n",
                fName, rmInfoCallback->ngrmi_arguments[i].ngai_ioMode,
                argHead.ngpad_ioMode);
            return 0;
        }

        /* Check whether it is Skip data */
        if ((argHead.ngpad_encode == NGI_CONVERSION_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
            result = nglProtocolBinary_ReceiveArgumentDataOfSkip(
                &arg->nga_argument[i], protocol, log, error);
            if (result ==0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't receive the Argument Data of Skip.\n", fName);
                return 0;
            }

        }

        /* Decode the argument data */
        result = ngiArgumentDecode(
            &arg->nga_argument[i], NULL, 0, protocol, &argHead, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't decode the Argument Data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive reply: Query Function Information.
 */
static int
nglProtocolBinary_ReceiveReplyQueryFunctionInformation(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    char *buf = NULL;
    size_t nBytes, readNbytesFromStream;
    static const char fName[] = "nglProtocolBinary_ReceiveReplyQueryFunctionInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_sReceive != NULL);
    assert(header != NULL);

    /* Receive the Session Information */
    result = ngiProtocolBinary_ReadXDRstring(
        protocol, protocol->ngp_sReceive, header->ngph_nBytes,
        NGI_PROTOCOL_NOT_CONTAIN_LENGTH, &buf, &nBytes,
        &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't receive the Session Information.\n",
            fName);
        return 0;
    }

    /* Store the Remote Class Information */
    if (protocol->ngp_rcInfo != NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_WARNING, NULL,
            "%s: Remote Class Information is already set.\n",
            fName);
    }
    protocol->ngp_rcInfo = buf;

    /* Success */
    return 1;
}

#if 0 /* Temporary commented out */
/**
 * Binary: Receive reply: Query Executable Information.
 */
static int
nglProtocolBinary_ReceiveReplyQueryExecutableInformation(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    char *buf;
    static const char fName[] = "nglProtocolBinary_ReceiveReplyQueryExecutableInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(information != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_sMng != NULL);
    assert(information != NULL);

    /* Allocate the storage */
    buf = globus_libc_calloc(1, head->ngph_nBytes);
    if (buf == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Receive Data.\n",
	    fName);
	return 0;
    }

    /* Allocate the Executable Information */
    *information = globus_lib_calloc(
    	1, sizeof (ngclExecutableFunctionInformation_t);
    if (*information == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Function Information.\n",
	    fName);
	return 0;
    }

    /* Receive the Parameter */
    result = ngiCommunicationReceive(
    	protocol->ngp_communication, *funcInfo, head->ngph_nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't receive the Function Information.\n", fName);
	goto error;
    }

    /* Parse and Analyze */

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Free */
    globus_libc_free(*funcInfo);

    return 0;
}
#endif /* Temporary commented out */

/**
 * Binary: Receive reply: Pull Back Session.
 */
int
nglProtocolBinary_ReceiveReplyPullBackSession(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    char *buf = NULL;
    size_t nBytes, readNbytesFromStream;
    int result;
    static const char fName[] =
        "nglProtocolBinary_ReceiveReplyPullBackSession";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Receive the Session Information */
    result = ngiProtocolBinary_ReadXDRstring(
	protocol, protocol->ngp_sReceive, header->ngph_nBytes,
	NGI_PROTOCOL_NOT_CONTAIN_LENGTH, &buf, &nBytes,
        &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't receive the Session Information.\n",
            fName);
        goto error;
    }

    assert(buf[nBytes] == '\0');
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
	NULL, "%s: Session Information as below.\n\"%s\"\n", fName, buf);

    /* Convert the Session Information */
    result = ngiProtocolSessionInformationConvert(
        protocol, buf,
        &protocol->ngp_sessionInfo.ngsi_executableRealTime,
        &protocol->ngp_sessionInfo.ngsi_executableCPUtime,
	&protocol->ngp_sessionInfo.ngsi_executableCallbackNtimesCalled,
	protocol->ngp_sessionInfo.ngsi_compressionInformation,
	protocol->ngp_sessionInfo.ngsi_nCompressionInformations,
        log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, 
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't convert the Session Information.\n",
            fName);
        goto error;
    }

    /* Release the string */
    result = ngiProtocolBinary_ReleaseString(protocol, buf, log, error);
    buf = NULL;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the string.\n", fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    globus_libc_free(buf);

    /* Failed */
    return 0;
}

/**
 * Binary: Receive reply: Transfer Result Data.
 */
int
nglProtocolBinary_ReceiveReplyTransferResultData(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg;
    ngiProtocolArgumentData_t argHead;
    long nArgs, item[NGI_PROTOCOL_ARGUMENT_DATA_NITEMS];
    long protocolVersion;
    int i, index;
    int result;
    size_t readNbytesFromStream;
    int isSent;
    static const char fName[] =
        "nglProtocolBinary_ReceiveReplyTransferResultData";

   /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
        protocol, &protocolVersion, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the version number of partner's protocol.\n",
            fName);
        return 0;
    }

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        return 0;
    }

    /* Get the Argument */
    arg = ngiProtocolGetArgument(protocol, log, error);
    if (arg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Argument from protocol.\n", fName);
        return 0;
    }

    /* Read the number of arguments */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nArgs, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read the Number Of Arguments.\n", fName);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Argument count %d is not valid.\n", fName, nArgs);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs > rmInfo->ngrmi_nArguments) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Argument count %d over the Method Info Argument Information.\n",
            fName, nArgs);
        return 0;
    }

    /* Is argument count zero? */
    if (nArgs == 0) {
        /* Success */
        return 1;
    }

    /* Receive the argument */
    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i],
            NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA,
            &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }

        if (isSent == 0) {
            continue;
        }

	/* Receive the items */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	    &item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Argument Data.\n", fName);
            return 0;
        }

        /* Copy the item to Argument Header Information */
        index = 0;
        argHead.ngpad_no        = item[index++];
        argHead.ngpad_type      = item[index++];
        argHead.ngpad_ioMode    = item[index++];
        argHead.ngpad_encode    = item[index++];
        argHead.ngpad_nElements = item[index++];
        argHead.ngpad_nBytes    = item[index++];

        /* Did the storage of Argument Data Item overflow? */
        if (index > NGI_NELEMENTS(item)) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: The storage of Argument Data Item overflowed.\n", fName);
            return 0;
        }

        if (argHead.ngpad_no != (i + 1)) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Argument No. is not valid: Require %d, receive %d.\n",
                fName, i, argHead.ngpad_no);
            return 0;
        }

        if (argHead.ngpad_type != rmInfo->ngrmi_arguments[i].ngai_dataType) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Data type is not valid: Require %d, receive %d.\n",
                fName, rmInfo->ngrmi_arguments[i].ngai_dataType,
                argHead.ngpad_type);
            return 0;
        }

        if (argHead.ngpad_ioMode != rmInfo->ngrmi_arguments[i].ngai_ioMode) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: I/O mode is not valid: Require %d, receive %d.\n",
                fName, rmInfo->ngrmi_arguments[i].ngai_ioMode,
                argHead.ngpad_ioMode);
            return 0;
        }

        /* Check whether it is Skip data */
        if ((argHead.ngpad_encode == NGI_CONVERSION_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
            result = nglProtocolBinary_ReceiveArgumentDataOfSkip(
                &arg->nga_argument[i], protocol, log, error);
            if (result ==0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't receive the Argument Data of Skip.\n", fName);
                return 0;
            }

        }

        /* Decode the argument data */
        result = ngiArgumentDecode(
            &arg->nga_argument[i],
	    &protocol->ngp_sessionInfo.ngsi_compressionInformation[i].ngci_out,
	    0, protocol, &argHead, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't decode the Argument Data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive reply: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_ReceiveReplyTransferCallbackArgumentData(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg, *referArg;
    ngiProtocolArgumentData_t argHead;
    long nArgs, item[NGI_PROTOCOL_ARGUMENT_DATA_NITEMS];
    long sequenceNo, sequenceNoOfProtocol;
    long callbackID, callbackIDofProtocol;
    int index;
    int i, count = 0;
    int result;
    size_t readNbytesFromStream;
    int isSent;
    static const char fName[] =
        "nglProtocolBinary_ReceiveReplyTransferCallbackArgumentData";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        return 0;
    }

    /* Read the Callback ID */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&callbackID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Callback ID.\n", fName);
        return 0;
    }

    /* Read the Sequence number */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&sequenceNo, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Sequence number.\n", fName);
        return 0;
    }

    /* Get the callback ID */
    callbackIDofProtocol = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackIDofProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the callback ID.\n", fName);
	return 0;
    }
    /* Check the Callback ID */
    if (callbackID != callbackIDofProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received callback ID is not valid.\n", fName);
	return 0;
    }

    /* Get the Sequence number */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
	return 0;
    }
    /* Check the Sequence number */
    if (sequenceNo != sequenceNoOfProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received Sequence number is not valid.\n", fName);
	return 0;
    }

    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
        if (rmInfo->ngrmi_arguments[i].ngai_dataType ==
            NG_ARGUMENT_DATA_TYPE_CALLBACK) {
            if (count == callbackIDofProtocol) {
                break;
            }
            count += 1;
        }
    }
    if ((i == rmInfo->ngrmi_nArguments) && (count != callbackIDofProtocol)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback ID %d is not valid.\n", fName, callbackIDofProtocol);
        return 0;
    }

    /* Get the Argument */
    referArg = ngiProtocolGetArgument(protocol, log, error);
    if (referArg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Argument from protocol.\n", fName);
        return 0;
    }

    /* Construct the Argument */
    arg = ngiArgumentConstruct(
        rmInfo->ngrmi_arguments[i].ngai_callback->ngrmi_arguments,
        rmInfo->ngrmi_arguments[i].ngai_callback->ngrmi_nArguments,
        log, error);
    if (arg == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the Argument.\n", fName);
        return 0;
    }

    /* Allocate the Subscript Value of Argument */
    for (i = 0; i < arg->nga_nArguments; i++) {
	if (arg->nga_argument[i].ngae_nDimensions == 0) {
	    continue;
	}
        arg->nga_argument[i].ngae_subscript = ngiSubscriptValueAllocate(
            arg->nga_argument[i].ngae_nDimensions, log, error);
        if (arg->nga_argument[i].ngae_subscript == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't allocate the Subscript Value of Argument.\n",
                fName);
            return 0;
        }
    }

    /* Read the number of arguments */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&nArgs, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Number Of Arguments.\n", fName);
        return 0;
    }

    /* Is argument count valid? */
    if (nArgs < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Argument count %d is not valid.\n", fName, nArgs);
	return 0;
    }

    /* Is argument count valid? */
    if (nArgs > arg->nga_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Argument count %d over the Method Info Argument Information.\n",
	    fName, nArgs);
	return 0;
    }

    /* Receive the argument */
    for (i = 0; i < arg->nga_nArguments; i++) {
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i],
            NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA,
            &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }

        if (isSent == 0) {
            /* Doesn't receive */
            continue;
        }

        /* Read the Argument Header Items */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	    &item[0], NGI_NELEMENTS(item), &readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Argument Data.\n", fName);
            return 0;
        }

        /* Copy the item to Argument Header Information */
        index = 0;
        argHead.ngpad_no        = item[index++];
        argHead.ngpad_type      = item[index++];
        argHead.ngpad_ioMode    = item[index++];
        argHead.ngpad_encode    = item[index++];
        argHead.ngpad_nElements = item[index++];
        argHead.ngpad_nBytes    = item[index++];

        /* Did the storage of Argument Data Item overflow? */
        if (index > NGI_NELEMENTS(item)) {
            NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: The storage of Argument Data Item overflowed.\n", fName);
            return 0;
        }

        if (argHead.ngpad_no != (i + 1)) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Argument No. is not valid: Require %d, receive %d.\n",
                fName, i, argHead.ngpad_no);
            return 0;
        }

        if (argHead.ngpad_type != arg->nga_argument[i].ngae_dataType) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Data type is not valid: Require %d, receive %d.\n",
                fName, arg->nga_argument[i].ngae_dataType,
                argHead.ngpad_type);
            return 0;
        }

        if (argHead.ngpad_ioMode != arg->nga_argument[i].ngae_ioMode) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: I/O mode is not valid: Require %d, receive %d.\n",
                fName, arg->nga_argument[i].ngae_ioMode,
                argHead.ngpad_ioMode);
            return 0;
        }

        /* Check whether it is Skip data */
        if ((argHead.ngpad_encode == NGI_CONVERSION_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
            (argHead.ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
            result = nglProtocolBinary_ReceiveArgumentDataOfSkip(
                &arg->nga_argument[i], protocol, log, error);
            if (result ==0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't receive the Argument Data of Skip.\n", fName);
                return 0;
            }

        }

        /* Decode the argument data */
        result = ngiArgumentDecode(
            &arg->nga_argument[i], NULL, 1, protocol, &argHead, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't decode the Argument Data.\n", fName);
            return 0;
        }

        /* Initialize the argument data */
        result = ngiArgumentElementInitializeData(
            &arg->nga_argument[i], arg->nga_argument[i].ngae_pointer,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Argument Data.\n", fName);
            return 0;
        }
    }

    /* Free the Subscript Value */
    for (i = 0; i < arg->nga_nArguments; i++) {
        if (arg->nga_argument[i].ngae_subscript != NULL) {
            result = ngiSubscriptValueFree(
                arg->nga_argument[i].ngae_subscript, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR,	NULL,
                    "%s: Can't construct the Subscript Value.\n", fName);
                return 0;
            }
            arg->nga_argument[i].ngae_subscript = NULL;
        }
    }

    /* Set the Argument of Callback */
    result = ngiProtocolSetCallbackArgument(protocol, arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't set the Argument Data of Callback.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive reply: Transfer Callback Argument Data.
 */
static int
nglProtocolBinary_ReceiveReplyTransferCallbackResultData(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    size_t readNbytesFromStream;
    long sequenceNo, sequenceNoOfProtocol;
    long callbackID, callbackIDofProtocol;
    int result;
    static const char fName[] =
        "nglProtocolBinary_ReceiveReplyTransferCallbackResultData";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Read the Callback ID */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&callbackID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Callback ID.\n", fName);
        return 0;
    }

    /* Read the Sequence number */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&sequenceNo, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Sequence number.\n", fName);
        return 0;
    }

    /* Get the callback ID */
    callbackIDofProtocol = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackIDofProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the callback ID.\n", fName);
	return 0;
    }

    /* Check the Callback ID */
    if (callbackID != callbackIDofProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received callback ID is not valid.\n", fName);
	return 0;
    }

    /* Get the Sequence number */
    sequenceNoOfProtocol = ngiProtocolGetSequenceNoOfCallback(
        protocol, log, error);
    if (sequenceNoOfProtocol == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the Sequence number.\n", fName);
	return 0;
    }

    /* Check the Sequence number */
    if (sequenceNo != sequenceNoOfProtocol) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Received Sequence number is not valid.\n", fName);
	return 0;
    }

    /* Success */
    return 1;   
}

/**
 * Binary: Receive notify: Invoke Callback
 */
static int
nglProtocolBinary_ReceiveNotifyInvokeCallback(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    size_t readNbytesFromStream;
    long callbackID;
    int result;
    static const char fName[] =
        "nglProtocolBinary_ReceiveNotifyInvokeCallback";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Set the callback ID to Protocol */
    result = ngiProtocolSetCallbackID(protocol, NGI_CALLBACK_ID_UNDEFINED,
        log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the callback ID.\n", fName);
        return 0;
    }

    /* Set the sequence number to Protocol */
    result = ngiProtocolSetSequenceNoOfCallback(protocol,
        NGI_PROTOCOL_SEQUENCE_NO_UNDEFINED, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the sequence number.\n", fName);
        return 0;
    }

    /* Read the Callback ID */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
	&callbackID, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't read the Method ID.\n", fName);
        return 0;
    }

    /* Set the Sequence number to Protocol */
    result = ngiProtocolSetSequenceNoOfCallback(protocol,
        header->ngph_sequenceNo, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the Sequence number.\n", fName);
        return 0;
    }

    /* Set the callback ID to Protocol */
    result = ngiProtocolSetCallbackID(protocol, callbackID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the callback ID.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Receive: Argument Data of Skip
 */
static int
nglProtocolBinary_ReceiveArgumentDataOfSkip(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    long *skipInfo;
    int nDim;
    size_t readNbytesFromStream;
    int index;
    int i;
    int result;
    static const char fName[] = "nglProtocolBinary_ReceiveArgumentDataOfSkip";

    /* Read the Skip Information from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_INT,
        &nDim, 1, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read the Skip Information.\n", fName);
        return 0;
    }

    /* Allocate the data buffer */
    skipInfo = globus_libc_malloc(nDim * sizeof(long) * 4);
    if (skipInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for skip Information.\n", fName);
        return 0;
    }

    /* Read the Skip Information from Stream Manager */
    result = nglProtocolBinary_ReadXDRdataWithReceive(
        protocol, protocol->ngp_sReceive, NG_ARGUMENT_DATA_TYPE_LONG,
        &skipInfo[0], nDim * 4, &readNbytesFromStream, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read the Skip Information.\n", fName);
        return 0;
    }

    /* Set the value of Subscript */
    for (i = 0, index = 0; i < argElement->ngae_nDimensions; i++) {
        argElement->ngae_subscript[i].ngsv_size = skipInfo[index++];
        argElement->ngae_subscript[i].ngsv_start = skipInfo[index++];
        argElement->ngae_subscript[i].ngsv_end = skipInfo[index++];
        argElement->ngae_subscript[i].ngsv_skip = skipInfo[index++];
    }

    /* Check the value of subscript */
    for (i = 0; i < argElement->ngae_nDimensions; i++) {
        if ((argElement->ngae_subscript[i].ngsv_start >
            argElement->ngae_subscript[i].ngsv_size)) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: The value of start of subscript is larger than size.(start = %d, size = %d)\n",
                fName, argElement->ngae_subscript[i].ngsv_start,
                argElement->ngae_subscript[i].ngsv_size);
            return 0;
        } else if (argElement->ngae_subscript[i].ngsv_start >
            argElement->ngae_subscript[i].ngsv_end) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: The value of start of subscript is larger than end.(start = %d, end = %d)\n",
                fName, argElement->ngae_subscript[i].ngsv_start,
                argElement->ngae_subscript[i].ngsv_end);
            return 0;
        } else if (argElement->ngae_subscript[i].ngsv_end >
            argElement->ngae_subscript[i].ngsv_size) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: The value of end of subscript is larger than size.(end = %d, size = %d)\n",
                fName, argElement->ngae_subscript[i].ngsv_end,
                argElement->ngae_subscript[i].ngsv_size);
            return 0;
        }
    }

    /* Get the value of total size of subscript */
    for (i = 1,
        argElement->ngae_subscript[0].ngsv_totalSize =
            argElement->ngae_subscript[0].ngsv_size;
        i < argElement->ngae_nDimensions; i++) {
        argElement->ngae_subscript[i].ngsv_totalSize = 
            argElement->ngae_subscript[i].ngsv_size *
            argElement->ngae_subscript[i - 1].ngsv_totalSize;
    }

    globus_libc_free(skipInfo);
    skipInfo = NULL;

    /* Success */
    return 1;
}

/**
 * Binary: Write data.
 */
int
ngiProtocolBinary_WriteData(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    ngiDataDirect_t direct,
    size_t *networkSizeofData,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_WriteData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(data != NULL);
    assert(nElements > 0);

    /* Use XDR ? */
    if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
	result = ngiProtocolBinary_WriteXDRdata(
	    protocol, sMng, dataType, data, nElements,
	    networkSizeofData, networkNbytes, log, error);
    } else {
	result = nglProtocolBinary_WriteNativeData(
	    protocol, sMng, dataType, data, nElements, direct,
	    networkSizeofData, networkNbytes, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Write native data
 */
static int
nglProtocolBinary_WriteNativeData(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    ngiDataDirect_t direct,
    size_t *networkSizeofData,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t nBytes;
    static const char fName[] = "nglProtocolBinary_WriteNativeData";

    /* Get sizeof data */
    result = ngiGetDataSize(dataType, &nBytes, log, error);
    if ((result == 0) || nglCheckSizetInvalid(nBytes < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the sizeof data.\n", fName);
	return 0;
    }
    if (networkSizeofData != NULL)
	*networkSizeofData = nBytes;
    if (networkNbytes != NULL)
	*networkNbytes = nBytes * nElements;

    /* Calculate the number of bytes of data */
    nBytes *= nElements;

    /* Write to Stream Manager */
    if (direct == NGI_DATA_DIRECTLY)
	result = ngiStreamManagerWriteDirectly(sMng, data, nBytes, log, error);
    else
	result = ngiStreamManagerWrite(sMng, data, nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't write to the Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Write encode XDR data.
 */
int
ngiProtocolBinary_WriteXDRdata(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *networkSizeofData,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t canWriteNbytes;
    int canWriteNelements;
    size_t sizeofNativeData;	/* Size of native */
    size_t sizeofNetData;	/* Size of network */
    void *buf;			/* Buffer of Stream Manager */
    char *tmpData;
    NET_Communicator netComm;
    static const char fName[] = "ngiProtocolBinary_WriteXDRdata";

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Get the size of native */
    result = ngiGetDataSize(dataType, &sizeofNativeData, log, error);
    if ((result == 0) || nglCheckSizetInvalid(sizeofNativeData < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the sizeof data of native.\n", fName);
	goto error;
    }

    /* Get the size of data */
    result = ngiNetCommunicatorGetDataSize(
    	&netComm, dataType, &sizeofNetData, log, error);
    if ((result == 0) || nglCheckSizetInvalid(sizeofNetData < 0)) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the sizeof data.\n", fName);
	goto error;
    }

    /* Set Arguments to data size */
    if (networkSizeofData != NULL) {
	*networkSizeofData = sizeofNetData;
    }
    if (networkNbytes != NULL) {
	*networkNbytes = sizeofNetData * nElements;
    }

    for (; 0 < nElements; nElements -= canWriteNelements) {
	/* Get the buffer of Stream Manager */
	result = ngiStreamManagerGetWritableBuffer(
	    sMng, &buf, sizeofNetData, &canWriteNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't get the Writable Buffer from Stream Manager.\n",
		fName);
	    goto error;
	}

	/* Calculate the number of elements which can write */
	canWriteNelements = canWriteNbytes / sizeofNetData;
	if (canWriteNelements > nElements)
	    canWriteNelements = nElements;
	canWriteNbytes = sizeofNetData * canWriteNelements;

	/* Encode to XDR */
	result = ngiNetCommunicatorWriteArray(
	    &netComm, dataType, data, canWriteNelements,
	    buf, canWriteNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    	NG_LOG_LEVEL_ERROR, NULL, "%s Can't encode to XDR.\n", fName);
	    goto error;
	}

	/* Write to Stream Buffer */
	result = ngiStreamManagerWriteBuffer(sMng, canWriteNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't write the data to Stream Buffer.\n", fName);
	    goto error;
	}

	/* Increment the pointer of data */
	tmpData = data;
	tmpData += sizeofNativeData * canWriteNelements;
	data = tmpData;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Write string.
 */
int
ngiProtocolBinary_WriteString(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    char *string,
    long strNbytes,
    ngiProtocolContainLength_t containLength,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_WriteString";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(((string != NULL) && (strNbytes >= 0)) ||
           ((string == NULL) && (strNbytes <= 0)));
    assert(networkNbytes != NULL);

    /* Use XDR? */
    if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
	result = nglProtocolBinary_WriteXDRstring(
	    protocol, sMng, string, strNbytes, containLength,
            networkNbytes, log, error);
    } else {
	result = nglProtocolBinary_WriteNativeString(
	    protocol, sMng, string, strNbytes, containLength,
            networkNbytes, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Write native string.
 */
static int
nglProtocolBinary_WriteNativeString(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    char *string,
    long strNbytes,
    ngiProtocolContainLength_t containLength,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t sizeofLength;
    static const char fName[] = "nglProtocolBinary_WriteNativeString";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(((string != NULL) && (strNbytes >= 0)) ||
           ((string == NULL) && (strNbytes <= 0)));
    assert(networkNbytes != NULL);

    /* If string is not NULL, then increment the strNbytes.
     * Increment the strNbytes for string terminater of '\0'.
     */
    if (string != NULL) {
	++strNbytes;
    }

    /* Initialize the variables */
    *networkNbytes = strNbytes;

    /* Is length written in? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &strNbytes, 1,
            NULL, &sizeofLength, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the length.\n", fName);
            return 0;
        }
        *networkNbytes += sizeofLength;
    }

    /* Is string NULL? */
    if (string == NULL) {
	/* Success */
	return 1;
    }

    /* Write to Stream Manager */
    result = ngiStreamManagerWrite(sMng, string, strNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Write XDR string.
 */
static int
nglProtocolBinary_WriteXDRstring(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    char *string,
    long strNbytes,
    ngiProtocolContainLength_t containLength,
    size_t *networkNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    void *xdrBuff = NULL;
    size_t xdrNbytes, sizeofLength;
    NET_Communicator netComm;
    static const char fName[] = "nglProtocolBinary_WriteXDRstring";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(((string != NULL) && (strNbytes >= 0)) ||
	((string == NULL) && (strNbytes <= 0)));
    assert(networkNbytes != NULL);

    /* Is string NULL and length written in? */
    if ((string == NULL) && (containLength == NGI_PROTOCOL_CONTAIN_LENGTH)) {
        strNbytes = 0;
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &strNbytes, 1,
            NULL, &sizeofLength, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the length.\n", fName);
            goto error;
        }
        *networkNbytes = sizeofLength;

	/* Success */
	return 1;
    }

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Encode from native string to XDR */
    result = ngiNetCommunicatorWriteString(
	&netComm, string, strNbytes, &xdrBuff, &xdrNbytes,
	log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't encode the string to XDR.\n", fName);
	goto error;
    }
    *networkNbytes = xdrNbytes;

    /* Is length written in? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
        strNbytes = (long)xdrNbytes;
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &strNbytes, 1,
            NULL, &sizeofLength, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the length.\n", fName);
            goto error;
        }
        *networkNbytes += sizeofLength;
    }

    /* Write to Stream Manager */
    result = ngiStreamManagerWrite(sMng, xdrBuff, xdrNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't write the data to Stream Manager.\n", fName);
        goto error;
    }

    /* Release the storage for XDR */
    result = ngiNetCommunicatorReleaseXDRbuffer(
	&netComm, xdrBuff, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the XDR string.\n", fName);
	goto error;
    }

   /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the storage for XDR */
    if (xdrBuff != NULL) {
	result = ngiNetCommunicatorReleaseXDRbuffer(
	    &netComm, xdrBuff, log, error);
	xdrBuff = NULL;
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't release the XDR string.\n", fName);
	}
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Read data.
 */
int
ngiProtocolBinary_ReadData(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(data != NULL);
    assert(readNbytesFromStream != NULL);

    /* Is number of elements zero? */
    if (nElements <= 0) {
    	/* Success */
	*readNbytesFromStream = 0;
	return 1;
    }

    /* Use XDR ? */
    if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
	/* Use XDR */
	result = ngiProtocolBinary_ReadXDRdata(
	    protocol, sMng, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    } else {
	result = ngiProtocolBinary_ReadNativeData(
	    protocol, sMng, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read XDR data.
 */
int
ngiProtocolBinary_ReadXDRdata(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadXDRdata";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(data != NULL);
    assert(readNbytesFromStream != NULL);

    /* Is number of elements zero? */
    if (nElements <= 0) {
    	/* Success */
	*readNbytesFromStream = 0;
	return 1;
    }

    if (sMng != NULL) {
	/* Read from Stream */
	result = nglProtocolBinary_ReadXDRdataFromStream(
	    protocol, sMng, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    } else {
	/* Read from Socket */
	result = nglProtocolBinary_ReadXDRdataWithReceive(
	    protocol, protocol->ngp_sReceive, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read native data.
 */
int
ngiProtocolBinary_ReadNativeData(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadNativeData";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(data != NULL);
    assert(readNbytesFromStream != NULL);

    /* Is number of elements zero? */
    if (nElements <= 0) {
    	/* Success */
	*readNbytesFromStream = 0;
	return 1;
    }

    if (sMng != NULL) {
	/* Read from Stream */
	result = nglProtocolBinary_ReadNativeDataFromStream(
	    protocol, sMng, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    } else {
	result = nglProtocolBinary_ReadNativeDataWithReceive(
	    protocol, dataType, data, nElements,
	    readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read XDR data with receive.
 */
static int
nglProtocolBinary_ReadXDRdataWithReceive(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t sizeofNetData;		/* Size of network */
    static const char fName[] = "nglProtocolBinary_ReadXDRdataWithReceive";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(data != NULL);
    assert(readNbytesFromStream != NULL);

    /* Is number of elements zero? */
    if (nElements <= 0) {
    	/* Success */
	*readNbytesFromStream = 0;
	return 1;
    }

    /* Get the size of network */
    result = ngiNetCommunicatorGetDataSize(
    	&protocol->ngp_netComm, dataType, &sizeofNetData, log, error);
    if ((result == 0) || nglCheckSizetInvalid(sizeofNetData < 0)) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the sizeof data of network.\n", fName);
	return 0;
    }

    /* Receive the XDR data */
    result = ngiStreamManagerReceive(
	sMng, protocol->ngp_communication,
	sizeofNetData * nElements, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't receive the Protocol Parameter.\n", fName);
	return 0;
    }

    /* Read XDR data */
    result = nglProtocolBinary_ReadXDRdataFromStream(
	protocol, sMng, dataType, data, nElements,
	readNbytesFromStream, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't encode to XDR.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read data: Native data from Stream.
 */
static int
nglProtocolBinary_ReadNativeDataFromStream(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t nBytes;
    static const char fName[] = "nglProtocolBinary_ReadNativeDataFromStream";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(data != NULL);
    assert(nElements > 0);
    assert(readNbytesFromStream != NULL);

    /* Get sizeof data */
    result = ngiGetDataSize(dataType, &nBytes, log, error);
    if ((result == 0) || nglCheckSizetInvalid(nBytes < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the sizeof data.\n", fName);
	return 0;
    }

    /* Calculate the number of bytes of data */
    nBytes *= nElements;
    *readNbytesFromStream = nBytes;

    /* Read to Stream Manager */
    result = ngiStreamManagerRead(sMng, data, nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't read from the Stream Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
* Binary: Read data: Native data
*/
static int
nglProtocolBinary_ReadNativeDataWithReceive(
    ngiProtocol_t *protocol,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromSocket,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t nBytes;
    static const char fName[] = "nglProtocolBinary_ReadNativeDataWithReceive";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(data != NULL);
    assert(nElements > 0);
    assert(readNbytesFromSocket != NULL);

    /* Get sizeof data */
    result = ngiGetDataSize(dataType, &nBytes, log, error);
    if ((result == 0) || nglCheckSizetInvalid(nBytes < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the sizeof data.\n", fName);
	return 0;
    }

    /* Calculate the number of bytes of data */
    nBytes *= nElements;

    /* Receive the data from socket */
    result = ngiCommunicationReceive(
	protocol->ngp_communication, data, nBytes, nBytes, readNbytesFromSocket,
	log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't receive the data from socket.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read data: Encode XDR
 */
static int
nglProtocolBinary_ReadXDRdataFromStream(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngArgumentDataType_t dataType,
    void *data,
    int nElements,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t canReadNbytes;
    int canReadNelements;
    size_t requireNbytes;
    size_t sizeofNativeData;	/* Size of native */
    size_t sizeofNetData;		/* Size of network */
    void *buf;		/* Buffer of Stream Manager */
    char *tmpData;
    char tmpBuf[sizeof (ngiArgumentData_t)];
    NET_Communicator netComm;
    static const char fName[] = "nglProtocolBinary_ReadXDRdataFromStream";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(data != NULL);
    assert(nElements > 0);
    assert(readNbytesFromStream != NULL);

    /* Initialize the variables */
    *readNbytesFromStream = 0;

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Get the size of native */
    result = ngiGetDataSize(dataType, &sizeofNativeData, log, error);
    if ((result == 0) || nglCheckSizetInvalid(sizeofNativeData < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the sizeof data of native.\n", fName);
	goto error;
    }

    /* Get the size of network */
    result = ngiNetCommunicatorGetDataSize(
    	&netComm, dataType, &sizeofNetData, log, error);
    if ((result == 0) || nglCheckSizetInvalid(sizeofNetData < 0)) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the sizeof data of network.\n", fName);
	goto error;
    }

    tmpData = data;
    for (; 0 < nElements; nElements -= canReadNelements) {
	/* Get the buffer of Stream Manager */
	result = ngiStreamManagerGetReadableBuffer(
	    sMng, &buf, 1, &canReadNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Readable Buffer from Stream Manager.\n",
		fName);
	    goto error;
	}

        /* Is greater equal than size of data? */
	if (canReadNbytes >= sizeofNetData) {
	    /* Calculate the number of elements */
	    canReadNelements = canReadNbytes / sizeofNetData;
	    if (canReadNelements > nElements)
		canReadNelements = nElements;
	    canReadNbytes = sizeofNetData * canReadNelements;

	    /* Decode from XDR to native */
	    result = ngiNetCommunicatorReadArray(
		&netComm, dataType, buf, canReadNbytes,
		tmpData, canReadNelements, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't read the data from Net Communicator.\n",
		    fName);
		goto error;
	    }

	    /* Read from Stream Buffer */
	    result = ngiStreamManagerReadBuffer(
		sMng, canReadNbytes, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't read the data from Stream Buffer.\n", fName);
		goto error;
	    }
	    *readNbytesFromStream += canReadNbytes;

	    /* Increment the pointer of data */
	    tmpData += sizeofNativeData * canReadNelements;
	} else {
	    /* Read from Stream Manager to temporary buffer */
	    result = ngiStreamManagerRead(
		sMng, &tmpBuf[0], canReadNbytes, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't read the data from Stream Manager.\n",
		    fName);
		goto error;
	    }
	    assert(canReadNbytes < sizeofNetData);
	    *readNbytesFromStream += canReadNbytes;
	    requireNbytes = sizeofNetData - canReadNbytes;

	    /* Get the buffer of Stream Manager */
	    result = ngiStreamManagerGetReadableBuffer(
		sMng, &buf, 1, &canReadNbytes, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't get the Readable Buffer from Stream Manager.\n",
		    fName);
		goto error;
	    }

	    /* Is there enough data? */
	    if (canReadNbytes < requireNbytes) {
	    	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Stream Manager is underflow.\n", fName);
		goto error;
	    }

	    /* Read from Stream Manager to temporary buffer */
	    result = ngiStreamManagerRead(
		sMng, &tmpBuf[sizeofNetData - requireNbytes], requireNbytes,
		log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't read the data from Stream Manager.\n",
		    fName);
		goto error;
	    }
	    *readNbytesFromStream += requireNbytes;

	    /* Decode from XDR to native */
	    canReadNelements = 1;
	    result = ngiNetCommunicatorReadArray(
		&netComm, dataType, &tmpBuf[0], sizeofNetData,
		tmpData, canReadNelements, log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't read the data from Net Communicator.\n",
		    fName);
		goto error;
	    }

	    /* Increment the pointer of data */
	    tmpData += sizeofNativeData;
	}
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Read string.
 */
int
ngiProtocolBinary_ReadString(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadString";

    /* Check the argument */
    assert(protocol != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromStream != NULL);

    /* Use XDR? */
    if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
	result = ngiProtocolBinary_ReadXDRstring(
	    protocol, sMng, nBytes, containLength, string, strNbytes,
	    readNbytesFromStream, log, error);
    } else {
	result = ngiProtocolBinary_ReadNativeString(
	    protocol, sMng, nBytes, containLength, string, strNbytes, 
	    readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read XDR string.
 */
int
ngiProtocolBinary_ReadXDRstring(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadXDRstring";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(string != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromStream != NULL);

    if (sMng != NULL) {
	/* Read from Stream */
	result = nglProtocolBinary_ReadXDRstringFromStream(
	    protocol, sMng, nBytes, containLength, string,
	    strNbytes, readNbytesFromStream, log, error);
    } else {
	/* Read from Socket */
	result = nglProtocolBinary_ReadXDRstringWithReceive(
	    protocol, protocol->ngp_sReceive, nBytes, containLength, string,
	    strNbytes, readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read native string.
 */
int
ngiProtocolBinary_ReadNativeString(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolBinary_ReadNativeString";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(string != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromStream != NULL);

    if (sMng != NULL) {
	/* Read from Stream */
	result = nglProtocolBinary_ReadNativeStringFromStream(
	    protocol, sMng, nBytes, containLength, string,
	    strNbytes, readNbytesFromStream, log, error);
    } else {
	/* Read from Socket */
	result = nglProtocolBinary_ReadNativeStringWithReceive(
	    protocol, protocol->ngp_sReceive, nBytes, containLength, string,
	    strNbytes, readNbytesFromStream, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read data.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Binary: Read native string from Stream.
 */
static int
nglProtocolBinary_ReadNativeStringFromStream(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromSocket,
    ngLog_t *log,
    int *error)
{
    int result;
    long stringLength;
    size_t receiveNbytes;
    static const char fName[] = "nglProtocolBinary_ReadNativeStringFromStream";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromSocket != NULL);

    /* Initialize the variables */
    *string = NULL;
    *strNbytes = 0;
    *readNbytesFromSocket = 0;

    /* Is length contain? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
	/* Read the length of string */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &stringLength, 1,
	    readNbytesFromSocket, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the length of string.\n", fName);
            goto error;
        }
        nBytes = stringLength;

	/* Is string length equal zero? */
	if (nBytes == 0) {
	    /* Success */
	    return 1;
	}
    }

    /* Increment the byte count */
    *strNbytes = nBytes;

    /* Allocate the storage for string */
    *string = globus_libc_malloc(nBytes + 1);
    if (*string == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for string.\n", fName);
	goto error;
    }

    /* Is string length equal zero? */
    receiveNbytes = nBytes;
    if (nBytes > 0) {
	/* Read the string from Stream */
	result = ngiStreamManagerRead(sMng, *string, nBytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't receive the string from socket.\n", fName);
	    goto error;
	}
    }
    (*string)[receiveNbytes] = '\0';
    *readNbytesFromSocket += receiveNbytes;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the storage for string */
    if (*string != NULL)
	globus_libc_free(*string);
    *string = NULL;

    /* Failed */
    return 0;
}


/**
 * Binary: Read native string with receive.
 */
static int
nglProtocolBinary_ReadNativeStringWithReceive(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromSocket,
    ngLog_t *log,
    int *error)
{
    int result;
    long stringLength;
    size_t receiveNbytes;
    static const char fName[] = "nglProtocolBinary_ReadNativeStringWithReceive";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromSocket != NULL);

    /* Initialize the variables */
    *string = NULL;
    *strNbytes = 0;
    *readNbytesFromSocket = 0;

    /* Is length contain? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
	/* Read the length of string */
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &stringLength, 1,
	    readNbytesFromSocket, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the length of string.\n", fName);
            goto error;
        }
        nBytes = stringLength;

	/* Is string length equal zero? */
	if (nBytes == 0) {
	    /* Success */
	    return 1;
	}
    }

    /* Increment the byte count */
    *strNbytes = nBytes;

    /* Allocate the storage for string */
    *string = globus_libc_malloc(nBytes + 1);
    if (*string == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for string.\n", fName);
	goto error;
    }

    /* Is string length equal zero? */
    receiveNbytes = 0;
    if (nBytes > 0) {
	/* Read the string from socket */
	result = ngiCommunicationReceive(
	    protocol->ngp_communication, *string, nBytes, nBytes,
	    &receiveNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't receive the string from socket.\n", fName);
	    goto error;
	}
    }
    (*string)[receiveNbytes] = '\0';
    *readNbytesFromSocket += receiveNbytes;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the storage for string */
    if (*string != NULL)
	globus_libc_free(*string);
    *string = NULL;

    /* Failed */
    return 0;
}

/**
 * Binary: Read XDR string with receive.
 */
static int
nglProtocolBinary_ReadXDRstringWithReceive(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    long stringLength;
    void *xdrBuff = NULL;
    size_t xdrNbytes;
    size_t receiveNbytes;
    char *tmp = NULL;
    NET_Communicator netComm;
    static const char fName[] = "nglProtocolBinary_ReadXDRstringWithReceive";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromStream != NULL);

    /* Initialize the variables */
    *string = NULL;
    *strNbytes = 0;
    *readNbytesFromStream = 0;

   /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Is length contain? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
        result = nglProtocolBinary_ReadXDRdataWithReceive(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &stringLength, 1,
	    readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the length of string.\n", fName);
            goto error;
        }
        nBytes = stringLength;

	/* Is string length equal zero? */
	if (nBytes == 0) {
	    goto success;
	}
    }

    /* Allocate the storage for XDR */
    xdrNbytes = nBytes;
    xdrBuff = globus_libc_malloc(xdrNbytes + 1);
    if (xdrBuff == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for XDR.\n", fName);
	goto error;
    }

    /* Read XDR data from Stream Manager */
    /* Is string length equal zero? */
    if (nBytes > 0) {
	receiveNbytes = 0;
	result = ngiCommunicationReceive(
	    protocol->ngp_communication, xdrBuff, xdrNbytes, xdrNbytes,
	    &receiveNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't receive the data.\n", fName);
	    goto error;
	}
	if (xdrNbytes != receiveNbytes) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Data length is not correct: require = %d, receive = %d).\n",
		fName, xdrNbytes, receiveNbytes);
	    goto error;
	}
	*readNbytesFromStream += receiveNbytes;
	
	/* Decode from XDR to native string */
	result = ngiNetCommunicatorReadString(
	    &netComm, xdrBuff, xdrNbytes, &tmp, strNbytes,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't decode the string from XDR to native.\n", fName);
	    goto error;
	}
	((char *)xdrBuff)[receiveNbytes] = '\0';
    }

    /* Copy the string */
    *string = globus_libc_malloc(*strNbytes + 1);
    if (*string == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't duplicate the string.\n", fName);
	goto error;
    }
    strcpy(*string, tmp);

    /* Release the string */
    ngiNetCommunicatorReleaseString(
	&netComm, tmp, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the string.\n", fName);
	goto error;
    }

    /* Deallocate the storage for XDR */
    globus_libc_free(xdrBuff);
    xdrBuff = NULL;

success:
    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate the storage for XDR */
    if (xdrBuff != NULL)
	globus_libc_free(xdrBuff);
    xdrBuff = NULL;

    if (*string != NULL) {
	ngiProtocolBinary_ReleaseString(protocol, *string, log, NULL);
	*string = NULL;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Read XDR string.
 */
static int
nglProtocolBinary_ReadXDRstringFromStream(
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytes,
    ngiProtocolContainLength_t containLength,
    char **string,
    size_t *strNbytes,
    size_t *readNbytesFromStream,
    ngLog_t *log,
    int *error)
{
    int result;
    long stringLength;
    void *xdrBuff = NULL;
    size_t xdrNbytes;
    char *tmp = NULL;
    NET_Communicator netComm;
    static const char fName[] = "nglProtocolBinary_ReadXDRstringFromStream";

    /* Check the argument */
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(string != NULL);
    assert(strNbytes != NULL);
    assert(readNbytesFromStream != NULL);

    /* Initialize the variables */
    *string = NULL;
    *strNbytes = 0;
    *readNbytesFromStream = 0;

   /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Is length contain? */
    if (containLength == NGI_PROTOCOL_CONTAIN_LENGTH) {
        result = nglProtocolBinary_ReadXDRdataFromStream(
            protocol, sMng, NG_ARGUMENT_DATA_TYPE_LONG, &stringLength, 1,
	    readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the length of string.\n", fName);
            goto error;
        }
        nBytes = stringLength;

	/* Is string length equal zero? */
	if (nBytes == 0) {
	    goto success;
	}
    }

    /* Allocate the storage for XDR */
    xdrNbytes = nBytes;
    xdrBuff = globus_libc_malloc(xdrNbytes);
    if (xdrBuff == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for XDR.\n", fName);
	goto error;
    }

    /* Read XDR data from Stream Manager */
    result = ngiStreamManagerRead(sMng, xdrBuff, xdrNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the XDR data from Stream Manager.\n", fName);
	goto error;
    }

    /* Decode from XDR to native string */
    result = ngiNetCommunicatorReadString(
	&netComm, xdrBuff, xdrNbytes, &tmp, strNbytes,
	log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't decode the string from XDR to native.\n", fName);
	goto error;
    }

    /* Copy the string */
    *string = globus_libc_malloc(*strNbytes + 1);
    if (*string == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't duplicate the string.\n", fName);
	goto error;
    }
    strcpy(*string, tmp);

    /* Release the string */
    ngiNetCommunicatorReleaseString(
	&netComm, tmp, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the string.\n", fName);
	goto error;
    }

    /* Deallocate the storage for XDR */
    globus_libc_free(xdrBuff);
    xdrBuff = NULL;

success:
    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate the storage for XDR */
    if (xdrBuff != NULL)
	globus_libc_free(xdrBuff);
    xdrBuff = NULL;

    if (*string != NULL) {
	ngiProtocolBinary_ReleaseString(protocol, *string, log, NULL);
	*string = NULL;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Binary: Release the string.
 */
int
ngiProtocolBinary_ReleaseString(
    ngiProtocol_t *protocol,
    char *string,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(string != NULL);

    /* Deallocate the string */
    globus_libc_free(string);

    /* Success */
    return 1;
}
