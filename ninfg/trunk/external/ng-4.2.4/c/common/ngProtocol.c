#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngProtocol.c,v $ $Revision: 1.96 $ $Date: 2007/07/10 07:51:48 $";
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
 */

#include <string.h>
#include <assert.h>
#define NGI_REQUIRE_ARCHITECTURE_IDS
#include "ng.h"
#undef NGI_REQUIRE_ARCHITECTURE_IDS
#include "ngXML.h"

/**
 * Prototype declaration of static functions.
 */
static void nglProtocolInitializeMember(ngiProtocol_t *);
static void nglProtocolInitializePointer(ngiProtocol_t *);
static int nglProtocolInitializeDataSize( ngiProtocol_t *, ngLog_t *, int *);
static void nglProtocolAttributeInitializeMember(
    ngiProtocolAttribute_t *protoAttr);
static void nglProtocolAttributeInitializePointer(
    ngiProtocolAttribute_t *protoAttr);

static int nglProtocolSessionInformationParse(
    ngiProtocol_t *, ngiXMLelement_t *, ngSessionInformationExecutable_t *,
    ngSessionInformationExecutable_t *, int *,
    ngCompressionInformation_t *, int, ngLog_t *, int *);
static int nglProtocolSessionInformationConvertToTime(
    ngiProtocol_t *, ngiXMLelement_t *, ngSessionInformationExecutable_t *,
    ngLog_t *, int *);
static int nglProtocolCompressionInformationParse(
    ngiProtocol_t *, ngiXMLelement_t *,
    ngCompressionInformation_t *, ngLog_t *, int *);
static int nglProtocolXMLstringToTime(
    ngiXMLelement_t *, char *, struct timeval *, ngLog_t *, int *);
static int nglProtocolCompressionInformationSkipAttribute(
    ngiXMLelement_t *, char *[], ngLog_t *, int *);

/**
 * Construct
 */
ngiProtocol_t *
ngiProtocolConstruct(
    ngiProtocolAttribute_t *protoAttr,
    ngiCommunication_t *comm,
    ngRemoteMethodInformation_t *getRemoteMethodInformation(),
    ngLog_t *log,
    int *error)
{
    int result;
    ngiProtocol_t *proto;
    static const char fName[] = "ngiProtocolConstruct";

    /* Check the arguments */
    assert(protoAttr != NULL);
    assert(protoAttr->ngpa_contextID >= NGI_CONTEXT_ID_MIN);
    assert((protoAttr->ngpa_executableID >= NGI_EXECUTABLE_ID_MIN) ||
           (protoAttr->ngpa_executableID == NGI_EXECUTABLE_ID_UNDEFINED));

    /* Allocate */
    proto = ngiProtocolAllocate(log, error);
    if (proto == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Protocol Manager.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiProtocolInitialize(proto, protoAttr, comm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Protocol Manager.\n", fName);
	goto error;
    }

    /* Initialize the functions pointer for receive*/
    proto->ngp_getRemoteMethodInfo = getRemoteMethodInformation;

    /* Success */
    return proto;

    /* Error occurred */
error:
    result = ngiProtocolFree(proto, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Protocol Manager.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngiProtocolDestruct(ngiProtocol_t *proto, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiProtocolDestruct";

    /* Check the arguments */
    assert(proto != NULL);

    /* Finalize */
    result = ngiProtocolFinalize(proto, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Protocol Manager.\n", fName);
	return 0;
    }

    result = ngiProtocolFree(proto, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't deallocate the storage for Protocol Manager.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngiProtocol_t *
ngiProtocolAllocate(ngLog_t *log, int *error)
{
    ngiProtocol_t *proto;
    static const char fName[] = "ngiProtocolAllocate";

    proto = globus_libc_calloc(1, sizeof (ngiProtocol_t));
    if (proto == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "Can't allocate the storage for Protocol.\n", fName, NULL);
	return NULL;
    }

    /* Success */
    return proto;
}

/**
 * Deallocate
 */
int
ngiProtocolFree(ngiProtocol_t *proto, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(proto != NULL);

    /* Deallocate */
    globus_libc_free(proto);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngiProtocolInitialize(
    ngiProtocol_t *protocol,
    ngiProtocolAttribute_t *protoAttr,
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolInitialize";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protoAttr != NULL);
    assert(comm != NULL);

    /* Initialize the members */
    nglProtocolInitializeMember(protocol);

    /* Copy the Communication Manager */
    protocol->ngp_communication = comm;

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&protocol->ngp_netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Initialize the Session Information */
    result = ngiSessionInformationInitialize(
	&protocol->ngp_sessionInfo, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the Session Information.\n", fName);
	return 0;
    }

    /* Initialize the Mutex for send */
    result = ngiMutexInitialize(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the Mutex for protocol send.\n",
            fName);
        return 0;
    }

    /* Copy the attribute */
    protocol->ngp_attr = *protoAttr;
    if (protoAttr->ngpa_tmpDir != NULL) {
        protocol->ngp_attr.ngpa_tmpDir = strdup(protoAttr->ngpa_tmpDir);
        if (protocol->ngp_attr.ngpa_tmpDir == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get storage for temporary directory string.\n",
                fName);
            return 0;
        }
    }

    /* Construct the Stream Buffer for receive */
    protocol->ngp_sReceive = ngiMemoryStreamManagerConstruct(
    	NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (protocol->ngp_sReceive == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct Stream Buffer.\n", fName);
	return 0;
    }

    /* Get the data size */
    result = nglProtocolInitializeDataSize(protocol, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the Data Size.\n", fName);
	return 0;
    }

    /* Initialize the functions */
    result = ngiProtocolBinary_InitializeFunction(protocol, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize function.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglProtocolInitializeMember(ngiProtocol_t *protocol)
{
    /* Check the arguments */
    assert(protocol);

    /* Initialize the pointers */
    nglProtocolInitializePointer(protocol);

    /* Initialize the IDs */
    ngiProtocolInitializeID(protocol);

    /* Initialize the members */
    protocol->ngp_protocolVersionOfPartner = 0;
    protocol->ngp_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
    protocol->ngp_notifySeqNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
    protocol->ngp_byteStreamConversion.ngbsc_zlib = 0;
    protocol->ngp_byteStreamConversion.ngbsc_zlibThreshold = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglProtocolInitializePointer(ngiProtocol_t *protocol)
{
    /* Check the arguments */
    assert(protocol);

    /* Initialize the pointers */
    protocol->ngp_communication = NULL;
    protocol->ngp_userData = NULL;
    protocol->ngp_sReceive = NULL;
    protocol->ngp_rcInfo = NULL;
}

/**
 * Initialize the data size.
 */
static int
nglProtocolInitializeDataSize(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    NET_Communicator netComm;
    static const char fName[] = "nglProtocolInitializeDataSize";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize the NET Communicator.\n", fName);
        return 0;
    }

    /* Get the size of data of Native */
    protocol->ngp_nativeDataSize.ngds_char     = sizeof (char);
    protocol->ngp_nativeDataSize.ngds_short    = sizeof (short);
    protocol->ngp_nativeDataSize.ngds_int      = sizeof (int);
    protocol->ngp_nativeDataSize.ngds_long     = sizeof (long);
    protocol->ngp_nativeDataSize.ngds_float    = sizeof (float);
    protocol->ngp_nativeDataSize.ngds_double   = sizeof (double);
    protocol->ngp_nativeDataSize.ngds_scomplex = sizeof (scomplex);
    protocol->ngp_nativeDataSize.ngds_dcomplex = sizeof (dcomplex);

    /* Copy the data size */
    ngiNetCommunicatorCopyDataSize(
        &netComm, &protocol->ngp_xdrDataSize);

    /* Set the XDR operation */
    result = ngiProtocolSetXDR(
	protocol, protocol->ngp_attr.ngpa_xdr, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't set the XDR operation.\n", fName);
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
 * Finalize
 */
int
ngiProtocolFinalize(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiProtocolFinalize";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protocol->ngp_communication != NULL);

    /* Initialize the Protocol Attribute */
    if (protocol->ngp_attr.ngpa_tmpDir != NULL) {
        globus_libc_free(protocol->ngp_attr.ngpa_tmpDir);
        protocol->ngp_attr.ngpa_tmpDir = NULL;
    }
    nglProtocolAttributeInitializeMember(&protocol->ngp_attr);

    /* Destruct the Stread Buffer */
    if (protocol->ngp_sReceive != NULL) {
	result = ngiStreamManagerDestruct(protocol->ngp_sReceive, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destruct the Stream Buffer.\n", fName);
	    return 0;
	}
    }
    protocol->ngp_sReceive = NULL;

    /* Finalize the Mutex for send */
    result = ngiMutexDestroy(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the Mutex for protocol send\n",
            fName);
        return 0;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&protocol->ngp_netComm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't finalize the NET Communicator.\n", fName);
        return 0;
    }

    /* Finalize the Session Information */
    result = ngiSessionInformationFinalize(
	&protocol->ngp_sessionInfo, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't finalize the Session Information.\n", fName);
	return 0;
    }

    /* Release the Communication Manager */
    if (protocol->ngp_communication != NULL) {
	result = ngiCommunicationDestruct(
	    protocol->ngp_communication, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destruct the Communication.\n", fName);
	    return 0;
	}
    }
    protocol->ngp_communication = NULL;

    /* Success */
    return 1;
}

/**
 * Register the User Data.
 */
int
ngiProtocolRegisterUserData(
    ngiProtocol_t *protocol,
    void *userData,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolRegisterUserData";

    /* Check the arguments */
    assert(protocol);

    /* Is data NULL? */
    if (userData == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data is NULL.\n", fName);
	return 0;
    }

    /* Is User Data already registered? */
    if (protocol->ngp_userData != NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_EXIST);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data is already registered.\n", fName);
	return 0;
    }

    /* Register the User Data */
    protocol->ngp_userData = userData;

    /* Success */
    return 1;
}

/**
 * Unregister the User Data.
 */
int
ngiProtocolUnregisterUserData(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolUnregisterUserData";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Is User Data registered? */
    if (protocol->ngp_userData == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data is not registered.\n", fName);
	return 0;
    }

    /* Unregister the User Data */
    protocol->ngp_userData = NULL;

    /* Success */
    return 1;
}

/**
 * Get the User Data.
 */
void *
ngiProtocolGetUserData(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolGetUserData";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Is User Data registered? */
    if (protocol->ngp_userData == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: User Data is not registered.\n", fName);
	return NULL;
    }

    /* Success */
    return protocol->ngp_userData;
}

/**
 * Protocol Attribute: Set the XDR operation.
 */
int
ngiProtocolSetXDR(
    ngiProtocol_t *protocol,
    ngXDR_t xdr,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    /* Set the XDR operation */
    protocol->ngp_attr.ngpa_xdr = xdr;

    /* Success */
    return 1;
}

/**
 * Protocol Attribute: Set the Executable ID.
 */
int
ngiProtocolSetExecutableID(
    ngiProtocol_t *protocol,
    int executableID,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    /* Set the XDR operation */
    protocol->ngp_attr.ngpa_executableID = executableID;

    /* Success */
    return 1;
}

/**
 * Protocol Attribute: Set the Architecture ID.
 */
int
ngiProtocolSetArchitectureID(
    ngiProtocol_t *protocol,
    u_long architectureID,
    ngLog_t *log,
    int *error)
{
#if 0
    static const char fName[] = "ngiProtocolSetArchitectureID";
#endif

    /* Check the arguments */
    assert(protocol != NULL);

    /* Set the Architecture ID */
    protocol->ngp_attr.ngpa_architecture = architectureID;

    /* Success */
    return 1;
}

/**
 * Protocol Attribute: Initialize the members.
 */
static void
nglProtocolAttributeInitializeMember(ngiProtocolAttribute_t *protoAttr)
{
    /* Check the arguments */
    assert(protoAttr != NULL);

    /* Initialize the pointers */
    nglProtocolAttributeInitializePointer(protoAttr);

    /* Initialize the members */
    /* Do nothing */
}

/**
 * Protocol Attribute: Initialize the pointers.
 */
static void
nglProtocolAttributeInitializePointer(ngiProtocolAttribute_t *protoAttr)
{
    /* Check the arguments */
    assert(protoAttr != NULL);

    /* Do nothing */
}

/**
 * Set the Version Number of a partner's Protocol.
 */
int
ngiProtocolSetProtocolVersionOfPartner(
    ngiProtocol_t *protocol,
    long version,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolSetProtocolVersionOfPartner";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Check the version number */
    if (version < NGI_PROTOCOL_VERSION_MINIMUM) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Invalid protocol version (%#x) of partner.\n",
	    fName, version);
	return 0;
    }

    protocol->ngp_protocolVersionOfPartner = version;

    /* Success */
    return 1;
}

/**
 * Get the Version Number of a partner's Protocol.
 */
int
ngiProtocolGetProtocolVersionOfPartner(
    ngiProtocol_t *protocol,
    long *version,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(version != NULL);

    *version = protocol->ngp_protocolVersionOfPartner;

    /* Success */
    return 1;
}

/**
 * Update the Sequence Number.
 */
int
ngiProtocolUpdateSequenceNo(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *protoHead,
    ngLog_t *log,
    int *error)
{
    int seqNo;
    static const char fName[] = "ngiProtocolUpdateSequenceNo";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protoHead != NULL);

    switch (protoHead->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
    case NGI_PROTOCOL_REQUEST_TYPE_REPLY:
        seqNo = protocol->ngp_sequenceNo;
	break;

    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
	seqNo = protocol->ngp_notifySeqNo;
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Request Type %d is not valid.\n",
            fName,
	    protoHead->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT); 
        return 0;
    }

    switch (protoHead->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
        /* Increment the Sequence No. */
        seqNo++;
        if (seqNo > NGI_PROTOCOL_SEQUENCE_NO_MAX)
            seqNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
	break;

    default:
    	/* Do no thing */
	break;
    }

    /* Is Sequence Number valid? */
    if (protoHead->ngph_sequenceNo != seqNo) {
	NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Sequence Number is not valid: Expected: %d, Received: %d\n",
	    fName, seqNo, protoHead->ngph_sequenceNo);
	return 0;
    }

    switch (protoHead->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REQUEST:
        protocol->ngp_sequenceNo = seqNo;
	break;

    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
	protocol->ngp_notifySeqNo = seqNo;
        break;

    default:
    	/* Do nothing */
	break;
    }

    /* Success */
    return 1;
}

/**
 * Is architecture ID valid?
 */
int
ngiProtocolIsArchitectureValid(long archID, ngLog_t *log, int *error)
{
    int i;
    static const char fName[] = "ngiProtocolIsArchitectureValid";

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

#if 0 /* Temporary commented out */
/**
 * Release Argument Data.
 */
int
ngiProtocolReleaseArgumentData(ngiArgumentPointer_t *argP, int nArgs)
{
    int i;

    /* Check the arguments */
    assert(argP != NULL);
    assert(nArgs > 0);

    /* Deallocate the data */
    for (i = 0; i < nArgs; i++) {
	globus_libc_free(argP->ngap_void);
	argP->ngap_void = NULL;
    }

    /* Deallocate the Argument Pointer */
    globus_libc_free(argP);
    argP = NULL;
}
#endif /* Temporary commented out */

/**
 * Release any data it allocated by above allocator.
 */
int
ngiProtocolReleaseData(void *data, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(data != NULL);

    /* Deallocate */
    globus_libc_free(data);

    /* Success */
    return 1;
}

#if 0 /* Is this necessary? */
/**
 * Read the information for Protocol Header.
 */
static int
nglProtocolReadHeader(
    ngiProtocol_t *protocol,
    ngiStreamBuffer_t *stream,
    ngiStreamBuffer_t **newStream,
    ngiProtocolHeader *head,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglProtocolReadHeader";

    /* Read the Request Code */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_requestCode, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Request Code.\n", fName);
	return 0;
    }

    /* Read the Sequence number */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_sequenceNo, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Sequence number.\n", fName);
	return 0;
    }

    /* Read the Context ID */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_contextID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Context ID.\n", fName);
	return 0;
    }

    /* Read the Executable ID */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_executableID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Executable ID.\n", fName);
	return 0;
    }

    /* Read the Session ID */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_sessionID, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Session ID.\n", fName);
	return 0;
    }

    /* Read the Not Used1 */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_notUsed1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Not Used1.\n", fName);
	return 0;
    }

    /* Read the Result */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_result, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the Result.\n", fName);
	return 0;
    }

    /* Read the number of bytes of parameter */
    result = ngiStreamManagerReadLong(
    	protocol->ngp_sMng, head->ngph_nBytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the number of bytes of parameter.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}
#endif /* Is this necessary? */

#if 0 /* Is this necessary? */
/**
 * Get the number of bytes of XDR or Native Data.
 */
int
ngiProtocolGetDataSize(
    ngiProtocol_t *protocol,
    ngArgumentDataType_t dataType,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    size_t nBytes;
    NET_Communicator netComm;
    static const char fName[] = "ngiProtocolGetDataSize";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(nBytes != NULL);

    /* Use XDR ? */
    if (protocol->ngp_attr.ngpa_xdr == NG_XDR_USE) {
	/* Use XDR */
	result = ngiNetCommunicatorGetDataSize(
	    &netComm, dataType, nBytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't get the Size Of Data.\n", fName);
	    return 0;
	}
    } else {
	/* Not use XDR */
	result = ngiGetDataSize(dataType, nBytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't get the Size Of Data.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}
#endif

/**
 * Get the number of bytes of native data.
 */
int
ngiGetDataSize(
    ngArgumentDataType_t dataType,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiGetDataSize";

    /* Check the arguments */
    assert(nBytes != NULL);

    switch (dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
    	*nBytes = sizeof (char);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_SHORT:
    	*nBytes = sizeof (short);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_INT:
    	*nBytes = sizeof (int);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_LONG:
    	*nBytes = sizeof (long);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_FLOAT:
    	*nBytes = sizeof (float);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
    	*nBytes = sizeof (double);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
    	*nBytes = sizeof (scomplex);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
    	*nBytes = sizeof (dcomplex);
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_STRING:
    case NG_ARGUMENT_DATA_TYPE_FILENAME:
	*nBytes = 0;
    	return 1;
    case NG_ARGUMENT_DATA_TYPE_CALLBACK:
    	*nBytes = sizeof (void (*)(void));
    	return 1;

    default:
	/* Do nothing */
	break;
    }

    /* Unknown data type */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
    	"%s: Unknown data type %d\n", fName, dataType);
    abort();
    return 0;
}

/**
 * Receive the Data.
 */
int
ngiProtocolReceive(
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngiProtocolReceiveMode_t mode,
    int *received,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolReceive";

    /* Check the argument */
    assert(protocol != NULL);
    assert(header != NULL);

    /* Check the Protocol type */
    if (protocol->ngp_attr.ngpa_protocolType == NG_PROTOCOL_TYPE_BINARY) {
        /* Receive the Data of Binary */
        result = ngiProtocolBinary_Receive(
            protocol, header, mode, received, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Data.\n", fName);
            return 0;
        }
    } else {
        /* Receive the Data of XML */
#if 0
        result = ngiProtocolXML_receive(protocol);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't receive the Data.\n", fName);
            return 0;
        }
#endif
    }

    /* Success */
    return 1;
}

/**
 * Set the Context ID to Protocol
 */
int
ngiProtocolSetIDofContext(
    ngiProtocol_t *protocol,
    long contextID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolSetIDofContext";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Is Executable ID of Protocol valid? */
    if (protocol->ngp_contextID != NGI_CONTEXT_ID_UNDEFINED) {
        NGI_SET_ERROR(error,  NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Context ID of Protocol is not valid.: %d\n",
            fName, protocol->ngp_contextID);
        return 0;
    }  

    /* Set the Context ID */
    protocol->ngp_contextID = contextID;

    /* Success */
    return 1;
}

/**
 * Set the Executable ID to Protocol
 */
int
ngiProtocolSetIDofExecutable(
    ngiProtocol_t *protocol,
    long executableID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolSetIDofExecutable";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Is Executable ID of Protocol valid? */
    if (protocol->ngp_executableID != NGI_EXECUTABLE_ID_UNDEFINED) {
        NGI_SET_ERROR(error,  NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Executable ID of Protocol is not valid.: %d\n",
            fName, protocol->ngp_executableID);
        return 0;
    }  

    /* Set the Executable ID */
    protocol->ngp_executableID = executableID;

    /* Success */
    return 1;
}

/**
 * Set the Session ID to Protocol
 */
int
ngiProtocolSetIDofSession(
    ngiProtocol_t *protocol,
    long sessionID,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiProtocolSetIDofSession";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Is Session ID of Protocol valid? */
    if (protocol->ngp_sessionID != NGI_SESSION_ID_UNDEFINED) {
        NGI_SET_ERROR(error,  NG_ERROR_PROTOCOL);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Session ID of Protocol is not valid.: %d\n",
            fName, protocol->ngp_sessionID);
        return 0;
    }  

    /* Set the Session ID */
    protocol->ngp_sessionID = sessionID;

    /* Success */
    return 1;
}

/**
 * Release the Session ID of Protocol
 */
int
ngiProtocolReleaseIDofSession(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    /* Initialize the Session ID */
    protocol->ngp_sessionID = NGI_SESSION_ID_UNDEFINED;

    /* Success */
    return 1;
}

/**
 * Initialize the IDs
 */
void
ngiProtocolInitializeID(ngiProtocol_t *protocol)
{
    /* Check the arguments */
    assert(protocol != NULL);

    /* Initialize the ID */
    protocol->ngp_contextID    = NGI_CONTEXT_ID_UNDEFINED;
    protocol->ngp_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    protocol->ngp_sessionID    = NGI_SESSION_ID_UNDEFINED;
    protocol->ngp_methodID     = NGI_METHOD_ID_UNDEFINED;
}

/**
 * Set Method ID to Protocol
 */
int
ngiProtocolSetMethodID(
    ngiProtocol_t *protocol,
    long methodID,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_methodID = methodID;

    return 1;
}

/**
 * Get Method ID from Protocol
 */
long
ngiProtocolGetMethodID(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    return protocol->ngp_methodID;
}

/**
 * Release Method ID of Protocol
 */
int
ngiProtocolReleaseMethodID(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_methodID = NGI_METHOD_ID_UNDEFINED;

    return 1;
}

/**
 * Set Argument to Protocol
 */
int
ngiProtocolSetArgument(
    ngiProtocol_t *protocol,
    ngiArgument_t *argument,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(argument != NULL);

    protocol->ngp_argument = argument;

    return 1;
}

/**
 * Get Argument from Protocol
 */
ngiArgument_t *
ngiProtocolGetArgument(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    return protocol->ngp_argument;
}

/**
 * Release Argument from Protocol
 */
int
ngiProtocolReleaseArgument(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_argument = NULL;

    return 1;
}

/**
 * Protocol: get number of arguments sent.
 */
int
ngiProtocolGetNargumentsSent(
    ngiProtocol_t *protocol,
    ngiArgument_t *arg,
    int when,
    int *nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    int isSent;
    static const char fName[] = "ngiProtocolGetNargumentsSent";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(arg != NULL);
    assert(nArguments != NULL);

    /* Calculate number of arguments */
    *nArguments = 0;
    for (i = 0; i < arg->nga_nArguments; i++) {
        /* Get get whether argument element is sent or not. */
        result = ngiProtocolArgumentElementIsSent(
            protocol, &arg->nga_argument[i], when,
            &isSent, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get whether argument element is sent or not.\n", fName);
            return 0;
        }
        if (isSent != 0) {
            (*nArguments)++;
        }
    }

    /* Success */
    return 1;
}

int
ngiProtocolArgumentElementIsSent(
    ngiProtocol_t *protocol,
    ngiArgumentElement_t *argElement,
    int when, 
    int *isSent,
    ngLog_t *log,
    int *error)
{
    long version;
    int result;
    static const char fName[] = "ngiProtocolArgumentElementIsSent";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(argElement != NULL);
    assert(isSent != NULL);

    result = ngiProtocolGetProtocolVersionOfPartner(
        protocol, &version, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get protocol version of partner.\n",
	    fName);
        return 0;
    }

    /* Is argument FILENAME? */
    *isSent = 0;
    switch (when) {
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) {
            *isSent = 1;
            break;
        }
        /* BREAKTHROUGH */
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        /* Is argument IN or INOUT? */
        if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_IN) ||
            (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_INOUT)) {
            *isSent = 1;
        }
        break;
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        if ((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) &&
            (!NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(version))) {
            break;
        }
        /* BREAKTHROUGH */
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        /* Is argument OUT or INOUT? */
        if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_OUT) ||
            (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_INOUT)) {
            *isSent = 1;
        }
        break;
    default:
        assert(0);
    }
    return 1;
}

/**
 * Get Session information from Protocol
 * Start the measurement.
 */
int
ngiProtocolSessionInformationStartMeasurement(
    ngiProtocol_t *protocol,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolSessionInformationStartMeasurement";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(nArguments >= 0);

    /* Start the measurement */
    result = ngiSessionInformationStartMeasurement(
	&protocol->ngp_sessionInfo, nArguments, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't start the measurement.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finish the measurement.
 */
int
ngiProtocolSessionInformationFinishMeasurement(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
	"ngiProtocolSessionInformationFinishMeasurement";

    /* Check the arguments */
    assert(protocol != NULL);

    /* Finish the measurement */
    result = ngiSessionInformationFinishMeasurement(
	&protocol->ngp_sessionInfo, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't finish the measurement.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the Session information from Protocol
 */
int
ngiProtocolGetSessionInfo(
    ngiProtocol_t *protocol,
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    int i;
    static const char fName[] = "ngiProtocolGetSessionInfo";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(info != NULL);

    *info = protocol->ngp_sessionInfo;

    /* Allocate the storage for Compression Information */
    if (info->ngsi_nCompressionInformations > 0) {
	info->ngsi_compressionInformation = globus_libc_calloc(
	    info->ngsi_nCompressionInformations,
	    sizeof (*info->ngsi_compressionInformation));
	if (info->ngsi_compressionInformation == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Can't allocate the storage for Compression Information.\n",
		fName);
	    return 0;
	}
    }

    /* Copy the Compression Informations */
    for (i = 0; i < info->ngsi_nCompressionInformations; i++)
	info->ngsi_compressionInformation[i] =
	    protocol->ngp_sessionInfo.ngsi_compressionInformation[i];

    /* Success */
    return 1;
}

/**
 * Release the Session Information.
 */
int
ngiProtocolReleaseSessionInfo(
    ngiProtocol_t *protocol,
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(info != NULL);

    if (info->ngsi_compressionInformation != NULL)
	globus_libc_free(info->ngsi_compressionInformation);
    info->ngsi_nCompressionInformations = 0;
    info->ngsi_compressionInformation = NULL;

    /* Success */
    return 1;
}

/**
 * Set Sequence number of Callback to Protocol
 */
int
ngiProtocolSetSequenceNoOfCallback(
    ngiProtocol_t *protocol,
    int sequenceNo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_sequenceNoOfCallback = sequenceNo;

    return 1;
}

/**
 * Get Sequence number from Protocol
 */
long
ngiProtocolGetSequenceNoOfCallback(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    return protocol->ngp_sequenceNoOfCallback;
}

/**
 * Release Sequence number of Protocol
 */
int
ngiProtocolReleaseSequenceNoOfCallback(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_sequenceNoOfCallback = NGI_PROTOCOL_SEQUENCE_NO_DEFAULT;

    return 1;
}

/**
 * Set Callback ID to Protocol
 */
int
ngiProtocolSetCallbackID(
    ngiProtocol_t *protocol,
    int callbackID,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_callbackID = callbackID;

    return 1;
}

/**
 * Get Callback ID from Protocol
 */
long
ngiProtocolGetCallbackID(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    return protocol->ngp_callbackID;
}

/**
 * Release Callback ID of Protocol
 */
int
ngiProtocolReleaseCallbackID(ngiProtocol_t *protocol, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_callbackID = NGI_CALLBACK_ID_UNDEFINED;

    return 1;
}

/**
 * Set Argument of Callback to Protocol
 */
int
ngiProtocolSetCallbackArgument(
    ngiProtocol_t *protocol,
    ngiArgument_t *argument,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(argument != NULL);

    protocol->ngp_callbackArg = argument;

    return 1;
}

/**
 * Get Argument of Callback from Protocol
 */
ngiArgument_t *
ngiProtocolGetCallbackArgument(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    return protocol->ngp_callbackArg;
}

/**
 * Release Argument of Callback from Protocol
 */
int
ngiProtocolReleaseCallbackArgument(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_callbackArg = NULL;

    return 1;
}

/**
 * Set the Conversion Method for byte stream.
 */
int
ngiProtocolSetConversionMethod(
    ngiProtocol_t *protocol,
    ngiByteStreamConversion_t *ownSetup,
    long *negoOption,
    int nNegoOptions,
    ngLog_t *log,
    int *error)
{
    int i;
    int nConvOptions;
    int convMethod;
    int pairArgumentBlockSize;
    ngiByteStreamConversion_t conv;
    static const char fName[] = "ngiProtocolSetConversionMethod";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(ownSetup != NULL);
    assert(((negoOption != NULL) && (nNegoOptions >= 0)) ||
	   ((negoOption == NULL) && (nNegoOptions <= 0)));

    /* Initialize the Byte Stream Conversion */
    conv.ngbsc_zlib = 0;			/* Disable */
    conv.ngbsc_zlibThreshold = ownSetup->ngbsc_zlibThreshold;
    conv.ngbsc_argumentBlockSize = 0;        /* Disable */

    /* Analyze the negotiation option */
    for (i = 0; i < nNegoOptions;) {
        convMethod = negoOption[i++];
        if (i >= nNegoOptions) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't get number of options.\n",
                fName);
            return 0;
        }
        nConvOptions = negoOption[i++];
        if (i + nConvOptions > nNegoOptions) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invalid number of options (%d).\n",
                fName, nConvOptions);
            return 0;
        }

	switch (convMethod) {
	case NGI_BYTE_STREAM_CONVERSION_RAW:
	    if (nConvOptions != 0) {
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Invalid number of options (%d).\n",
		    fName, nConvOptions);
		return 0;
	    }
	    break;

	case NGI_BYTE_STREAM_CONVERSION_ZLIB:
	    if ((nConvOptions < 0) || (nConvOptions > 1)) {
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Invalid number of options (%d).\n",
		    fName, nConvOptions);
		return 0;
	    }

            /* Enable */
            conv.ngbsc_zlib = ownSetup->ngbsc_zlib;

	    /* Is threshold specified? */
	    if (nConvOptions >= 1) {
		conv.ngbsc_zlibThreshold = negoOption[i++];
	    }

            /* Is division size use an own default value? */
            if (conv.ngbsc_zlibThreshold <= 0) {
                conv.ngbsc_zlibThreshold = ownSetup->ngbsc_zlibThreshold;
            }
	    break;
        case NGI_BYTE_STREAM_CONVERSION_DIVIDE:
	    if ((nConvOptions < 0) || (nConvOptions > 1)) {
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Invalid number of options (%d).\n",
		    fName, nConvOptions);
		return 0;
	    }

            /* Enable */
            conv.ngbsc_argumentBlockSize = 
                ownSetup->ngbsc_argumentBlockSize;

	    /* Is division size specified? */
	    if (nConvOptions == 1) {
                pairArgumentBlockSize = negoOption[i++];
                if (pairArgumentBlockSize > 0) {
                    conv.ngbsc_argumentBlockSize =
                                    pairArgumentBlockSize;
                }
	    }
	    break;
        default:
            /* Ignore Unknown Conversion Method */
            i += nConvOptions;

            break;
	}
    }

    /* Setup the conversion method */
    protocol->ngp_byteStreamConversion = conv;

    /* Success */
    return 1;
}

#define NGL_XML_SESSION_INFORMATION		"sessionInformation"
#define NGL_XML_REAL_TIME			"realTime"
#define NGL_XML_CPU_TIME			"CPUtime"
#define NGL_XML_TRANSFER_ARGUMENT		"transferArgument"
#define NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE	"transferFileClientToRemote"
#define NGL_XML_CALCULATION			"calculation"
#define NGL_XML_TRANSFER_RESULT			"transferResult"
#define NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT	"transferFileRemoteToClient"
#define NGL_XML_CALLBACK_TRANSFER_ARGUMENT	"callbackTransferArgument"
#define NGL_XML_CALLBACK_CALCULATION		"callbackCalculation"
#define NGL_XML_CALLBACK_TRANSFER_RESULT	"callbackTransferResult"

#define NGL_XML_CALLBACK_INFORMATION		"callbackInformation"
#define NGL_XML_CALLBACK_NTIMES_CALLED		"numberOfTimesWhichCalled"

#define NGL_XML_COMPRESSION_INFORMATION		"compressionInformation"
#define NGL_XML_COMPRESSION_ARGUMENT_NO		"argumentNo"

#define NGL_XML_BEFORE_COMPRESSION_LENGTH	"beforeCompressionLength"
#define NGL_XML_AFTER_COMPRESSION_LENGTH	"afterCompressionLength"
#define NGL_XML_COMPRESSION_REAL_TIME		"compressionRealTime"
#define NGL_XML_COMPRESSION_CPU_TIME		"compressionCPUtime"

#define NGL_XML_BEFORE_DECOMPRESSION_LENGTH	"beforeDecompressionLength"
#define NGL_XML_AFTER_DECOMPRESSION_LENGTH	"afterDecompressionLength"
#define NGL_XML_DECOMPRESSION_REAL_TIME		"decompressionRealTime"
#define NGL_XML_DECOMPRESSION_CPU_TIME		"decompressionCPUtime"

/**
 * Session Information: Convert
 */
int
ngiProtocolSessionInformationConvert(
    ngiProtocol_t *protocol,
    char *buf,
    ngSessionInformationExecutable_t *siReal,
    ngSessionInformationExecutable_t *siCPU,
    int *nTimesCalled,
    ngCompressionInformation_t *compInfo,
    int nCompInfos,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t nBytes;
    ngiXMLparser_t *parser;
    ngiXMLelement_t *element;
    static const char fName[] = "ngiProtocolSessionInformationConvert";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(buf != NULL);
    assert(buf[0] != '\0');
    nBytes = strlen(buf);
    assert(nBytes > 0);

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for XMLparser.\n", fName);
        return 0;
    }

    /* Parse XML */
    result = ngiXMLparserParse(parser, buf, nBytes, 1, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: XML parse fail.\n", fName);
        return 0;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get root element.\n", fName);
        return 0;
    }

    /* Get Session Information element tree */
    element = ngiXMLelementGetNext(
	element, NULL, NGL_XML_SESSION_INFORMATION, log, error);
    if (element == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Tag <sessionInformation> is not found.\n", fName);
        return 0;
    }

    /* Convert XML element tree to RemoteClassInformation */
    result = nglProtocolSessionInformationParse(
	protocol, element, siReal, siCPU, nTimesCalled,
	compInfo, nCompInfos, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Session Information: Parse Error.\n", fName);
        return 0;
    }

    /* Destruct XML parser */
    result = ngiXMLparserDestruct(parser, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct XML parser\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Session Information: Parse
 */
static int
nglProtocolSessionInformationParse(
    ngiProtocol_t *protocol,
    ngiXMLelement_t *element,
    ngSessionInformationExecutable_t *siReal,
    ngSessionInformationExecutable_t *siCPU,
    int *nTimesCalled,
    ngCompressionInformation_t *compInfo,
    int nCompInfos,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiXMLelement_t *childElement;
    long protocolVersion;
    static const char fName[] = "nglProtocolSessionInformationParse";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(element != NULL);
    assert(siReal != NULL);
    assert(siCPU != NULL);
    assert(nTimesCalled != NULL);
    assert(((compInfo == NULL) && (nCompInfos == 0)) ||
	   ((compInfo != NULL) && (nCompInfos > 0)));

    childElement = ngiXMLelementGetNext(
        element, NULL, NGL_XML_REAL_TIME, log, error);
    if (childElement == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Tag <%s> is not found.\n", fName, NGL_XML_REAL_TIME);
	return 0;
    }

    result = nglProtocolSessionInformationConvertToTime(
	protocol, childElement, siReal, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't convert the real time.\n", fName);
        return 0;
    }    

    childElement = ngiXMLelementGetNext(
        element, childElement, NGL_XML_CPU_TIME, log, error);
    if (childElement == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Tag <%s> is not found.\n", fName, NGL_XML_CPU_TIME);
	return 0;
    }

    result = nglProtocolSessionInformationConvertToTime(
	protocol, childElement, siCPU, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't convert the CPU time.\n", fName);
        return 0;
    }

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    if (NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_CALLBACK_NTIMES_CALLED(
	protocolVersion)) {
	char *pvalue;
	char *pend;

	childElement = ngiXMLelementGetNext(
	    element, childElement, NGL_XML_CALLBACK_INFORMATION, log, error);
	if (childElement == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Tag <%s> is not found.\n", fName,
		NGL_XML_COMPRESSION_INFORMATION);
	    return 0;
	}

	/* Get the argumentNo */
	pvalue = ngiXMLattributeGetValue(
	    childElement, NGL_XML_CALLBACK_NTIMES_CALLED, log, error);
	if (pvalue == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Attribute is not found.\n",
		fName, NGL_XML_COMPRESSION_ARGUMENT_NO);
	    return 0;
	}
	*nTimesCalled = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Value is not found.\n",
		fName, NGL_XML_COMPRESSION_ARGUMENT_NO);
	}

    }

    /* Is support the data conversion? */
    if (NGI_PROTOCOL_IS_SUPPORT_SESSION_INFORMATION_COMPRESSION_TIME(
	protocolVersion)) {
	int i;

	for (i = 0; i < nCompInfos; i++) {
	    childElement = ngiXMLelementGetNext(
		element, childElement, NGL_XML_COMPRESSION_INFORMATION,
		log, error);
	    if (childElement == NULL) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_INFORMATION,
		    NULL, "%s: Tag <%s> is not found.\n", fName,
		    NGL_XML_COMPRESSION_INFORMATION);
		break;
	    }

	    result = nglProtocolCompressionInformationParse(
		protocol, childElement, &compInfo[i], log, error);
	    if (result == 0) {
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		    NULL,
		    "%s: Can't convert the Compression Information.\n", fName);
		return 0;
	    }
	}
    }

    /* Success */
    return 1;
}

/**
 * Session Information: Convert string to time.
 */
static int
nglProtocolSessionInformationConvertToTime(
    ngiProtocol_t *protocol,
    ngiXMLelement_t *element,
    ngSessionInformationExecutable_t *siExec,
    ngLog_t *log,
    int *error)
{
    int result;
    long protocolVersion;
    static const char fName[] = "nglProtocolSessionInformationConvertToTime";

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    /* Method: Transfer Argument Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_TRANSFER_ARGUMENT,
	&siExec->ngsie_transferArgument, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_TRANSFER_ARGUMENT);
        return 0;
    }

    /* Is support the data conversion? */
    if (protocolVersion >= NGI_PROTOCOL_VERSION_SUPPORT_THE_CONVERSION_METHOD) {
	/* Method: Transfer File from Client to Remote */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE,
	    &siExec->ngsie_transferFileClientToRemote, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE);
	    return 0;
	}
    }

    /* Method: Calculation */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALCULATION,
	&siExec->ngsie_calculation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_CALCULATION);
        return 0;
    }

    /* Method: Tansfer Result Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_TRANSFER_RESULT,
	&siExec->ngsie_transferResult, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_TRANSFER_RESULT);
        return 0;
    }

    /* Is support the data conversion? */
    if (protocolVersion >= NGI_PROTOCOL_VERSION_SUPPORT_THE_CONVERSION_METHOD) {
	/* Method: Transfer File from Remote to Client */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT,
	    &siExec->ngsie_transferFileRemoteToClient, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT);
	    return 0;
	}
    }

    /* Callback: Transfer Argument */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_TRANSFER_ARGUMENT,
	&siExec->ngsie_callbackTransferArgument, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_CALLBACK_TRANSFER_ARGUMENT);
        return 0;
    }

    /* Callback: Calculation */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_CALCULATION,
	&siExec->ngsie_callbackCalculation, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_CALLBACK_CALCULATION);
        return 0;
    }

    /* Callback: Transfer Result Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_TRANSFER_RESULT,
	&siExec->ngsie_callbackTransferResult, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session Information: Can't get the XML attribute \"%s\"\n",
	    fName, NGL_XML_CALLBACK_TRANSFER_RESULT);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Parse the Compression Information.
 */
static int
nglProtocolCompressionInformationParse(
    ngiProtocol_t *protocol,
    ngiXMLelement_t *element,
    ngCompressionInformation_t *compInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    char *pvalue;
    char *pend;
    long value;
    static const char fName[] = "nglProtocolCompressionInformationParse";

    static const char *compAttrs[] = {
	NGL_XML_AFTER_COMPRESSION_LENGTH,
	NGL_XML_COMPRESSION_REAL_TIME,
	NGL_XML_COMPRESSION_CPU_TIME,
	NULL,
    };
    static const char *decompAttrs[] = {
	NGL_XML_AFTER_DECOMPRESSION_LENGTH,
	NGL_XML_DECOMPRESSION_REAL_TIME,
	NGL_XML_DECOMPRESSION_CPU_TIME,
	NULL,
    };

    /* Check the arguments */
    assert(protocol != NULL);
    assert(element != NULL);
    assert(compInfo != NULL);

    /* Initialize the temporary information */
    /* Get the argumentNo */
    pvalue = ngiXMLattributeGetValue(
	element, NGL_XML_COMPRESSION_ARGUMENT_NO, log, error);
    if (pvalue == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: %s: Attribute is not found.\n",
	    fName, NGL_XML_COMPRESSION_ARGUMENT_NO);
	return 0;
    }
    value = strtol(pvalue, &pend, 0);
    if (pend == pvalue) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: %s: Value is not found.\n",
	    fName, NGL_XML_COMPRESSION_ARGUMENT_NO);
    }

    /* Get the beforeCompressionLength */
    pvalue = ngiXMLattributeGetValue(
	element, NGL_XML_BEFORE_COMPRESSION_LENGTH, log, error);
    if (pvalue == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: %s: Attribute is not found.\n",
	    fName, NGL_XML_BEFORE_COMPRESSION_LENGTH);
	return 0;
    }
    /* Not measured? */
    if (pvalue[0] == '\0') {
	memset(&compInfo->ngci_out.ngcic_compression, 0,
	    sizeof (compInfo->ngci_out.ngcic_compression));

	/* Skip the attributes */
	result = nglProtocolCompressionInformationSkipAttribute(
	    element, (char **)compAttrs, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't skip the attributes.\n", fName);
	    return 0;
	}
    } else {
	compInfo->ngci_out.ngcic_compression.ngcie_measured = 1;
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Value is not found.\n",
		fName, NGL_XML_BEFORE_COMPRESSION_LENGTH);
	    return 0;
	}
	compInfo->ngci_out.ngcic_compression.ngcie_lengthRaw = value;

	/* Get the afterCompressionLength */
	pvalue = ngiXMLattributeGetValue(
	    element, NGL_XML_AFTER_COMPRESSION_LENGTH, log, error);
	if (pvalue == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Attribute is not found.\n",
		fName, NGL_XML_AFTER_COMPRESSION_LENGTH);
	    return 0;
	}
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Value is not found.\n",
		fName, NGL_XML_BEFORE_COMPRESSION_LENGTH);
	    return 0;
	}
	compInfo->ngci_out.ngcic_compression.ngcie_lengthCompressed =
	    value;

	/* Get the compressionRealTime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_COMPRESSION_REAL_TIME,
	    &compInfo->ngci_out.ngcic_compression.ngcie_executionRealTime,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_COMPRESSION_REAL_TIME);
	    return 0;
	}

	/* Get the compressionCPUtime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_COMPRESSION_CPU_TIME,
	    &compInfo->ngci_out.ngcic_compression.ngcie_executionCPUtime,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_COMPRESSION_CPU_TIME);
	    return 0;
	}
    }

    /* Get the beforeDecompressionLength */
    pvalue = ngiXMLattributeGetValue(
	element, NGL_XML_BEFORE_DECOMPRESSION_LENGTH, log, error);
    if (pvalue == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: %s: Attribute is not found.\n",
	    fName, NGL_XML_BEFORE_DECOMPRESSION_LENGTH);
	return 0;
    }
    /* Not measured? */
    if (pvalue[0] == '\0') {
	memset(&compInfo->ngci_in.ngcic_decompression, 0,
	    sizeof (compInfo->ngci_in.ngcic_decompression));

	/* Skip the attributes */
	result = nglProtocolCompressionInformationSkipAttribute(
	    element, (char **)decompAttrs, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't skip the attributes.\n", fName);
	    return 0;
	}
    } else {
	compInfo->ngci_in.ngcic_decompression.ngcie_measured = 1;
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Value is not found.\n",
		fName, NGL_XML_BEFORE_DECOMPRESSION_LENGTH);
	    return 0;
	}
	compInfo->ngci_in.ngcic_decompression.ngcie_lengthCompressed = value;

	/* Get the afterCompressionLength */
	pvalue = ngiXMLattributeGetValue(
	    element, NGL_XML_AFTER_DECOMPRESSION_LENGTH, log, error);
	if (pvalue == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Attribute is not found.\n",
		fName, NGL_XML_AFTER_DECOMPRESSION_LENGTH);
	    return 0;
	}
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: %s: Value is not found.\n",
		fName, NGL_XML_BEFORE_DECOMPRESSION_LENGTH);
	    return 0;
	}
	compInfo->ngci_in.ngcic_decompression.ngcie_lengthRaw =
	    value;

	/* Get the decompressionRealTime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_DECOMPRESSION_REAL_TIME,
	    &compInfo->ngci_in.ngcic_decompression.ngcie_executionRealTime,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_DECOMPRESSION_REAL_TIME);
	    return 0;
	}

	/* Get the decompressionCPUtime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_DECOMPRESSION_CPU_TIME,
	    &compInfo->ngci_in.ngcic_decompression.ngcie_executionCPUtime,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session Information: Can't get the XML attribute \"%s\"\n",
		fName, NGL_XML_DECOMPRESSION_CPU_TIME);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Session Information: Convert string to time.
 */
static int
nglProtocolXMLstringToTime(
    ngiXMLelement_t *element, 
    char *attribute,
    struct timeval *tv,
    ngLog_t *log,
    int *error)
{
    int result;
    char *value;
    static const char fName[] = "nglProtocolXMLstringToTime";

    /* Get the value of attribute */
    value = ngiXMLattributeGetValue(element, attribute, log, error);
    if (value == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the value of attribute \"%s\"\n",
	    fName, attribute);
	return 0;
    }

    /* Convert string to time */
    result = ngiStringToTime(value, tv, log, error);
    if (result ==  0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't convert the string to time.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Skip the attributes.
 */
static int
nglProtocolCompressionInformationSkipAttribute(
    ngiXMLelement_t *element,
    char *attributes[],
    ngLog_t *log,
    int *error)
{
    int i;
    char *pvalue;
    static const char fName[] =
	"nglProtocolCompressionInformationSkipAttribute";

    /* Check the arguments */
    assert(element != NULL);
    assert(attributes != NULL);

    for (i = 0; attributes[i] != NULL; i++) {
	/* Get the next attribute */
	pvalue = ngiXMLattributeGetValue(
	    element, attributes[i], log, error);
	if (pvalue == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL, "%s: %s: Attribute is not found.\n",
		fName, attributes[i]);
	    return 0;
	}
	if (pvalue[0] != '\0') {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL, "%s: %s: Value is found.\n",
		fName, attributes[i]);
	    return 0;
	}
    }

    /* Success */
    return 1;
}
