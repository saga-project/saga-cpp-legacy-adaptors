#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngProtocolXML.c,v $ $Revision: 1.7 $ $Date: 2004/03/11 07:25:27 $";
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
 * XML Protocol part.
 */

#include <string.h>
#include <assert.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
#if 0 /* Temporary comment out */
static int nglProtocolXML_MakeRequestHeader(
    ngiProtocol_t *, char *, int, ngLog_t *, int *);
static int nglProtocolXML_MakeRequestHeaderTerminator(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolXML_SendRequestQueryFunctionInformations(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolXML_SendRequestQueryExecutableInformations(
    ngiProtocol_t *protocol, ngLog_t *log, int *error);
static int nglProtocolXML_SendRequestInvokeSession(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolXML_MakeReplyHeader(
    ngiProtocol_t *, char *, int, int, ngLog_t *, int *);
static int nglProtocolXML_MakeReplyHeaderTerminator(
    ngiProtocol_t *, ngLog_t *, int *);
static int nglProtocolXML_SendReplyQueryFunctionInformations(
    ngiProtocol_t *, char *, ngLog_t *, int *);
static int nglProtocolXML_SendReplyQueryExecutableInformations(
#if 0 /* Is this correct? */
    ngiProtocol_t *, ngclExecutableFunctionInfo_t *, ngLog_t *, int *);
#else
    ngiProtocol_t *, char *, ngLog_t *, int *);
#endif
static int nglProtocolXML_SendReplyResetExecutable(
    ngiProtocol_t *, int, ngLog_t *, int *);
static int nglProtocolXML_SendReplyExitExecutable(
    ngiProtocol_t *, int, ngLog_t *, int *);
static int nglProtocolXML_SendReplyInvokeSession(
    ngiProtocol_t *, int, ngLog_t *, int *);
#endif


#if 0 /* Temporary commented out */
/**
 * XML: Make the Request Header.
 */
static int
nglProtocolXML_MakeRequestHeader(
    ngiProtocol_t *protocol,
    char *requestType,
    int sessionID,
    ngLog_t *log,
    int *error)
{
    int result, nBytes;
    char buf[NGI_PROTOCOL_XML_HEADER_NBYTES];
    static const char fName[] = "nglProtocolXML_MakeRequestHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ingp_stream != NULL);
    assert(requestType != NULL);
    assert((sessionID == NGI_SESSION_ID_UNDEFINED) ||
	   ((sessionID >= NGI_SESSION_ID_MIN) &&
	    (sessionID <= NGI_SESSION_ID_MAX)));

    /* Increment the Sequence No. */
    protocol->ngp_sequenceNo++;
    if (protocol->ngp_sequenceNo > NGI_PROTOCOL_SEQUENCE_NO_MAX)
    	protocol->ngp_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;

    /* Make the header */
    result = snprintf(buf, sizeof (buf),
	"<request type=\"%s\" sequenceNo=\"%d\" contextID=\"%d\" executableID=\"%d\" sessionID=\"%d\">"
	requestType, protocol->ngp_sequenceNo, protocol->ngp_contextId,
	protocol->ngp_executableId, sessionId);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }
    nBytes = result;

    /* Write data to Stream Buffer */
    result = ngiStreamManagerWrite(protocol->ngp_sMng, buf, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Make the terminater for Request Header.
 */
static int
nglProtocolXML_MakeRequestHeaderTerminator(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    char buf[NGI_PROTOCOL_XML_HEADER_NBYTES];
    static const char fName[] = "nglProtocolXML_MakeRequestHeaderTerminator";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the header */
    result = snprintf(buf, sizeof (buf), "</request>");
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }
    nBytes = result;

    /* Write data to Stream Buffer */
    result = ngiStreamBufferWrite(protocol->stream, buf, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }


    /* Success */
    return 1;
}

/**
 * XML: Send request: Query Function Informations.
 */
static int
nglProtocolXML_SendRequestQueryFunctionInformations(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendRequestQueryFunctionInformations";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeRequestHeader(
    	protocol, "queryFunctionInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeRequestHeaderTerminator(
    	protocol, "queryFunctionInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send request: Query Executable Informations.
 */
static int
nglProtocolXML_SendRequestQueryExecutableInformations(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendRequestQueryExecutableInformations";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeRequestHeader(
    	protocol, "queryExecutableInformation", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeRequestHeaderTerminator(
    	protocol, "queryFunctionInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send request: Reset Executable.
 */
static int
nglProtocolXML_SendRequestResetExecutable(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendRequestResetExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeRequestHeader(
    	protocol, "resetExecutable", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeRequestHeaderTerminator(protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send request: Exit Executable.
 */
static int
nglProtocolXML_SendRequestExitExecutable(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendRequestExitExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeRequestHeader(
    	protocol, "exitExecutable", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeRequestHeaderTerminator(protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send request: Invoke Session
 */
static int
nglProtocolXML_SendRequestInvokeSession(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    char buf[NGI_PROTOCOL_XML_INVOKE_SESSION_NBYTES];
    static const char fName[] = "nglProtocolXML_SendRequestInvokeSession";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeRequestHeader(
    	protocol, "exitExecutable", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the parameter */
    result = snprintf(buf, sizeof (buf),
    	"<session methodID=\"%d\"/>", methodID);
    if (result < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't make the parameter.\n", fName);
	abort();
	return 0;
    }

    /* Write data to Stream Buffer */
    result = ngiStreamManagerWrite(protocol->ngp_sMng, buf, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeRequestHeaderTerminator(protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Make the Reply Header.
 */
static int
nglProtocolXML_MakeReplyHeader(
    ngiProtocol_t *protocol,
    char *requestType,
    int sessionID,
    int protoResult,
    ngLog_t *log,
    int *error)
{
    int result, nBytes;
    char buf[NGI_PROTOCOL_XML_HEADER_NBYTES];
    static const char fName[] = "nglProtocolXML_MakeReplyHeader";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ingp_stream != NULL);
    assert(requestType != NULL);
    assert((sessionID == NGI_SESSION_ID_UNDEFINED) ||
	   ((sessionID >= NGI_SESSION_ID_MIN) &&
	    (sessionID <= NGI_SESSION_ID_MAX)));

    /* Make the header */
    result = snprintf(buf, sizeof (buf),
	"<reply type=\"%s\" sequenceNo=\"%d\" contextID=\"%d\" executableID=\"%d\" sessionID=\"%d\" result=\"%d\">"
	requestType, protocol->ngp_sequenceNo, protocol->ngp_contextId,
	protocol->ngp_executableId, sessionId, protoResult);
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }
    nBytes = result;

    /* Write data to Stream Buffer */
    result = ngiStreamManagerWrite(protocol->ngp_sMng, buf, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Make the terminater for Reply Header.
 */
static int
nglProtocolXML_MakeReplyHeaderTerminator(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    char buf[NGI_PROTOCOL_XML_HEADER_NBYTES];
    static const char fName[] = "nglProtocolXML_MakeReplyHeaderTerminator";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the header */
    result = snprintf(buf, sizeof (buf), "</reply>");
    if (result < 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }
    nBytes = result;

    /* Write data to Stream Buffer */
    result = ngiStreamBufferWrite(protocol->stream, buf, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }


    /* Success */
    return 1;
}

/**
 * XML: Send reply: Query Function Informations.
 */
static int
nglProtocolXML_SendReplyQueryFunctionInformations(
    ngiProtocol_t *protocol,
    char *info,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendReplyQueryFunctionInformations";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);
    assert(info != NULL);
    assert(info[0] != '\0');

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeReplyHeader(
    	protocol, "queryFunctionInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Write data to Stream Buffer */
    nBytes = strlen(funInfo);
    result = ngiStreamBufferWrite(protocol->stream, info, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeReplyHeaderTerminator(
    	protocol, "queryFunctionInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send reply: Query Executable Informations.
 */
static int
nglProtocolXML_SendReplyQueryExecutableInformations(
    ngiProtocol_t *protocol,
    ngclExecutableFunctionInformation_t *information,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendReplyQueryExecutableInformations";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);
    assert(info != NULL);
    assert(info[0] != '\0');

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeReplyHeader(
    	protocol, "queryExecutableInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Write data to Stream Buffer */
    nBytes = strlen(funInfo);
    result = ngiStreamBufferWrite(protocol->stream, info, nBytes);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the Protocol Header to Stream Buffer.\n",
	    fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeReplyHeaderTerminator(
    	protocol, "queryExecutableInformations", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent request.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send reply: Reset Executable.
 */
static int
nglProtocolXML_SendReplyResetExecutable(
    ngiProtocol_t *protocol,
    int protoResult,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendReplyResetExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeReplyHeader(
    	protocol, "queryExecutableInformations", protoResult, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeReplyHeaderTerminator(
    	protocol, "resetExecutable", log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent reply.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send reply: Exit Executable.
 */
static int
nglProtocolXML_SendReplyExitExecutable(
    ngiProtocol_t *protocol,
    int protoResult,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendReplyExitExecutable";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeReplyHeader(
    	protocol, "exitExecutable", protoResult, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeReplyHeaderTerminator(protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent reply.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * XML: Send reply: Invoke Session.
 */
static int
nglProtocolXML_SendReplyInvokeSession(
    ngiProtocol_t *protocol,
    int protoResult,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolXML_SendReplyInvokeSession";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);
    assert(protocol->ngp_stream != NULL);

    /* Make the Protocol Header */
    result = nglProtocolXML_MakeReplyHeader(
    	protocol, "invokeSession", protoResult, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the Protocol Header.\n", fName);
	return 0;
    }

    /* Make the terminater of Protocol Header */
    result = nglProtocolXML_MakeReplyHeaderTerminator(
    	protocol, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't make the terminater of Protocol Header.\n", fName);
	return 0;
    }

    /* Send request */
    result = ngiStreamManagerSend(
    	protocol->sMng, protocol->ngp_communication);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't sent reply.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}
#endif /* Temporary commented out */

