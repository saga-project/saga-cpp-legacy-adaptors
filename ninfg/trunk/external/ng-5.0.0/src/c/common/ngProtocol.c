/*
 * $RCSfile: ngProtocol.c,v $ $Revision: 1.23 $ $Date: 2008/03/28 09:26:28 $
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
 * Module for managing Protocol for Ninf-G Client/Executable.
 */

#define NGI_REQUIRE_ARCHITECTURE_IDS
#include "ngInternal.h"
#undef NGI_REQUIRE_ARCHITECTURE_IDS
#include "ngXML.h"

NGI_RCSID_EMBED("$RCSfile: ngProtocol.c,v $ $Revision: 1.23 $ $Date: 2008/03/28 09:26:28 $")

/**
 * Prototype declaration of static functions.
 */
static void nglProtocolInitializeMember(ngiProtocol_t *);
static void nglProtocolInitializePointer(ngiProtocol_t *);
static int nglProtocolInitializeDataSize( ngiProtocol_t *, ngLog_t *, int *);
static void nglProtocolAttributeInitializeMember(
    ngiProtocolAttribute_t *);
static void nglProtocolAttributeInitializePointer(
    ngiProtocolAttribute_t *);
static void nglProtocolTransferAttributeInitializeMember(
    ngiProtocolTransferAttribute_t *);

static int nglProtocolSessionInformationParse(
    ngiProtocol_t *, ngiXMLitem_t *, ngSessionInformationExecutable_t *,
    ngSessionInformationExecutable_t *, int *,
    ngCompressionInformation_t *, int, ngLog_t *, int *);
static int nglProtocolSessionInformationConvertToTime(
    ngiProtocol_t *, ngiXMLitem_t *, ngSessionInformationExecutable_t *,
    ngLog_t *, int *);
static int nglProtocolCompressionInformationParse(
    ngiProtocol_t *, ngiXMLitem_t *,
    ngCompressionInformation_t *, ngLog_t *, int *);
static int nglProtocolXMLstringToTime(
    ngiXMLitem_t *, char *, struct timeval *, ngLog_t *, int *);
static int nglProtocolCompressionInformationSkipAttribute(
    ngiXMLitem_t *, char *[], ngLog_t *, int *);

static int nglProtocolGetXMLstringFromCompressionInformation(
    ngiProtocol_t *, ngCompressionInformation_t *, int, char *,
    int, int *, ngLog_t *, int *);

/**
 * Construct
 */
ngiProtocol_t *
ngiProtocolConstruct(
    ngiProtocolAttribute_t *protoAttr,
    ngiCommunication_t *comm,
    ngiEvent_t *event,
    ngRemoteMethodInformation_t *getRemoteMethodInformation(struct ngiProtocol_s *, ngLog_t *, int *),
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
    proto = NGI_ALLOCATE(ngiProtocol_t, log, error);
    if (proto == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Protocol Manager.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngiProtocolInitialize(
        proto, protoAttr, comm, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Protocol Manager.\n"); 
	goto error;
    }

    /* Initialize the functions pointer for receive*/
    proto->ngp_getRemoteMethodInfo = getRemoteMethodInformation;

    /* Success */
    return proto;

    /* Error occurred */
error:
    result = NGI_DEALLOCATE(ngiProtocol_t, proto, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the storage for Protocol Manager.\n"); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Protocol Manager.\n"); 
	return 0;
    }

    result = NGI_DEALLOCATE(ngiProtocol_t, proto, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the storage for Protocol Manager.\n"); 
	return 0;
    }

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
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiProtocolInitialize";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(protoAttr != NULL);
    assert(comm != NULL);
    assert(event != NULL);

    /* Initialize the members */
    nglProtocolInitializeMember(protocol);

    /* Copy the Communication Manager */
    protocol->ngp_communication = comm;

    /* Copy the Event */
    protocol->ngp_event = event;

    /* Initialize the NET Communicator */
    result = ngiNetCommunicatorInitialize(&protocol->ngp_netComm, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the NET Communicator.\n"); 
        return 0;
    }

    /* Initialize the Session Information */
    result = ngiSessionInformationInitialize(
	&protocol->ngp_sessionInfo, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't initialize the Session Information.\n"); 
	return 0;
    }

    /* Initialize the Mutex for send */
    result = ngiMutexInitialize(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the Mutex for protocol send.\n"); 
        return 0;
    }

    /* Copy the attribute */
    protocol->ngp_attr = *protoAttr;
    if (protoAttr->ngpa_tmpDir != NULL) {
        protocol->ngp_attr.ngpa_tmpDir = strdup(protoAttr->ngpa_tmpDir);
        if (protocol->ngp_attr.ngpa_tmpDir == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't get storage for temporary directory string.\n"); 
            return 0;
        }
    }

    /* Construct the Stream Buffer for receive */
    protocol->ngp_sReceive = ngiMemoryStreamManagerConstruct(
    	NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (protocol->ngp_sReceive == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't construct Stream Buffer.\n"); 
	return 0;
    }

    /* Get the data size */
    result = nglProtocolInitializeDataSize(protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the Data Size.\n"); 
	return 0;
    }

    /* Initialize the functions */
    result = ngiProtocolBinary_InitializeFunction(protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't initialize function.\n"); 
	return 0;
    }

    protocol->ngp_connectCount = NGI_CONNECT_COUNT_MIN;
    protocol->ngp_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
    if (protoAttr->ngpa_sequenceNo != NGI_PROTOCOL_SEQUENCE_NO_DEFAULT) {
        protocol->ngp_sequenceNo = protoAttr->ngpa_sequenceNo;
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

    /* Initialize the Transfer Attr */
    nglProtocolTransferAttributeInitializeMember(&protocol->ngp_transferAttr);

    /* Initialize the members */
    protocol->ngp_protocolVersionOfPartner = 0;
    protocol->ngp_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
    protocol->ngp_notifySeqNo = NGI_PROTOCOL_SEQUENCE_NO_MIN;
    protocol->ngp_connectCount = NGI_CONNECT_COUNT_MIN;
    protocol->ngp_doClose = 0;
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
    protocol->ngp_event = NULL;
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the NET Communicator.\n"); 
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
	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't set the XDR operation.\n"); 
	goto error;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't finalize the NET Communicator.\n"); 
        return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&netComm, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't finalize the NET Communicator.\n"); 
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

    /* Initialize the Protocol Attribute */
    if (protocol->ngp_attr.ngpa_tmpDir != NULL) {
        ngiFree(protocol->ngp_attr.ngpa_tmpDir, log, error);
        protocol->ngp_attr.ngpa_tmpDir = NULL;
    }
    nglProtocolAttributeInitializeMember(&protocol->ngp_attr);

    nglProtocolTransferAttributeInitializeMember(&protocol->ngp_transferAttr);

    /* Destruct the Stread Buffer */
    if (protocol->ngp_sReceive != NULL) {
	result = ngiStreamManagerDestruct(protocol->ngp_sReceive, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't destruct the Stream Buffer.\n"); 
	    return 0;
	}
    }
    protocol->ngp_sReceive = NULL;

    /* Finalize the Mutex for send */
    result = ngiMutexDestroy(&protocol->ngp_sendMutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Mutex for protocol send\n"); 
        return 0;
    }

    /* Finalize the NET Communicator */
    result = ngiNetCommunicatorFinalize(&protocol->ngp_netComm, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't finalize the NET Communicator.\n"); 
        return 0;
    }

    /* Finalize the Session Information */
    result = ngiSessionInformationFinalize(
	&protocol->ngp_sessionInfo, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't finalize the Session Information.\n"); 
	return 0;
    }

    /* Release the Communication Manager */
    if (protocol->ngp_communication != NULL) {
	result = ngiCommunicationDestruct(
	    protocol->ngp_communication, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't destruct the Communication.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "User Data is NULL.\n"); 
	return 0;
    }

    /* Is User Data already registered? */
    if (protocol->ngp_userData != NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_EXIST);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "User Data is already registered.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "User Data is not registered.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "User Data is not registered.\n"); 
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
 * Protocol Attribute: Set the Simple Auth Number.
 */
int
ngiProtocolSetSimpleAuthNumber(
    ngiProtocol_t *protocol,
    int authNumber,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_attr.ngpa_simpleAuthNumber = authNumber;

    /* Success */
    return 1;
}

/**
 * Protocol Attribute: Set the Job ID.
 */
int
ngiProtocolSetJobID(
    ngiProtocol_t *protocol,
    int jobID,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_attr.ngpa_jobID = jobID;

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

    protocol->ngp_attr.ngpa_executableID = executableID;

    /* Success */
    return 1;
}

/**
 * Protocol Attribute: Set the Keep Connect.
 */
int
ngiProtocolSetKeepConnect(
    ngiProtocol_t *protocol,
    int keepConnect,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_attr.ngpa_keepConnect = keepConnect;

    /* Success */
    return 1;
}

/**
 * ProtocolAttribute: Initialize.
 */
int
ngiProtocolAttributeInitialize(
    ngiProtocolAttribute_t *protoAttr,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protoAttr != NULL);

    nglProtocolAttributeInitializeMember(protoAttr);

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
    protoAttr->ngpa_architecture = 0;
    protoAttr->ngpa_xdr = NG_XDR_NOT_USE;
    protoAttr->ngpa_protocolVersion = 0;
    protoAttr->ngpa_sequenceNo = 0;
    protoAttr->ngpa_simpleAuthNumber = 0;
    protoAttr->ngpa_contextID = 0;
    protoAttr->ngpa_jobID = 0;
    protoAttr->ngpa_executableID = 0;
    protoAttr->ngpa_keepConnect = 0;
}

/**
 * Protocol Attribute: Initialize the pointers.
 */
static void
nglProtocolAttributeInitializePointer(ngiProtocolAttribute_t *protoAttr)
{
    /* Check the arguments */
    assert(protoAttr != NULL);

    protoAttr->ngpa_tmpDir = NULL;
}

/**
 * Initialize the transfer attribute.
 */
int
ngiProtocolTransferAttributeInitialize(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    nglProtocolTransferAttributeInitializeMember(
        &protocol->ngp_transferAttr);

    /* Success */
    return 1;
}

/**
 * Initialize the transfer attribute members.
 */
static void
nglProtocolTransferAttributeInitializeMember(
    ngiProtocolTransferAttribute_t *transferAttr)
{
    /* Check the arguments */
    assert(transferAttr != NULL);

    transferAttr->ngpta_valid = 0;
    transferAttr->ngpta_contextID    = NGI_CONTEXT_ID_UNDEFINED;
    transferAttr->ngpta_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    transferAttr->ngpta_sessionID    = NGI_SESSION_ID_UNDEFINED;
}

/**
 * Set the transfer attribute.
 */
int
ngiProtocolTransferAttributeSet(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    ngiProtocolTransferAttribute_t *transferAttr;

    /* Check the arguments */
    assert(protocol != NULL);

    transferAttr = &protocol->ngp_transferAttr;

    transferAttr->ngpta_valid = 1;
    transferAttr->ngpta_contextID = protocol->ngp_contextID;
    transferAttr->ngpta_executableID = protocol->ngp_executableID;
    transferAttr->ngpta_sessionID = protocol->ngp_sessionID;

    /* Success */
    return 1;
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Invalid protocol version (%#lx) of partner.\n", version); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Request Type %ld is not valid.\n",
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Sequence Number is not valid: Expected: %d, Received: %ld\n",
            seqNo, protoHead->ngph_sequenceNo); 
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
    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "The architecture ID %ld is not defined.\n", archID);

    return 0;
}

/**
 * Release any data it allocated by above allocator.
 */
int
ngiProtocolReleaseData(void *data, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(data != NULL);

    /* Deallocate */
    ngiFree(data, log, error);

    /* Success */
    return 1;
}

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
    ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Unknown data type %d\n", dataType); 
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

    /* Receive the Data of Binary */
    result = ngiProtocolBinary_Receive(
        protocol, header, mode, received, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't receive the Data.\n"); 
        return 0;
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Context ID of Protocol is not valid.: %ld\n",
            protocol->ngp_contextID); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable ID of Protocol is not valid.: %ld\n",
            protocol->ngp_executableID); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session ID of Protocol is not valid.: %ld\n",
            protocol->ngp_sessionID); 
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
 * Set Communication.
 */
int
ngiProtocolSetCommunication(
    ngiProtocol_t *protocol,
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_communication = comm;

    return 1;
}


/**
 * Increment the connect count.
 */
int
ngiProtocolConnected(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_connectCount++;

    /* Reset the count */
    if (protocol->ngp_connectCount >= NGI_CONNECT_COUNT_MAX) {
        protocol->ngp_connectCount = 1;
    }

    return 1;
}


/**
 * Set Connection Close to Protocol
 */
int
ngiProtocolSetConnectionClose(
    ngiProtocol_t *protocol,
    int doClose,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_doClose = doClose;

    return 1;
}

/**
 * Get Connection Close on Protocol
 */
int
ngiProtocolGetConnectionClose(
    ngiProtocol_t *protocol,
    int *doClose,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);
    assert(doClose != NULL);

    *doClose = protocol->ngp_doClose;

    return 1;
}

/**
 * Release Connection Close on Protocol
 */
int
ngiProtocolReleaseConnectionClose(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(protocol != NULL);

    protocol->ngp_doClose = 0;

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
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't get whether argument element is sent or not.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get protocol version of partner.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't start the measurement.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't finish the measurement.\n"); 
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
	info->ngsi_compressionInformation = ngiCalloc(
	    info->ngsi_nCompressionInformations,
	    sizeof (*info->ngsi_compressionInformation), log, error);
	if (info->ngsi_compressionInformation == NULL) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't allocate the storage for Compression Information.\n"); 
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
	ngiFree(info->ngsi_compressionInformation, log, error);
    info->ngsi_nCompressionInformations = 0;
    info->ngsi_compressionInformation = NULL;

    /* Success */
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
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't get number of options.\n"); 
            return 0;
        }
        nConvOptions = negoOption[i++];
        if (i + nConvOptions > nNegoOptions) {
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Invalid number of options (%d).\n", nConvOptions); 
            return 0;
        }

	switch (convMethod) {
	case NGI_BYTE_STREAM_CONVERSION_RAW:
	    if (nConvOptions != 0) {
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
		ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
		    "Invalid number of options (%d).\n", nConvOptions); 
		return 0;
	    }
	    break;

	case NGI_BYTE_STREAM_CONVERSION_ZLIB:
	    if ((nConvOptions < 0) || (nConvOptions > 1)) {
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
		ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
		    "Invalid number of options (%d).\n", nConvOptions); 
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
		ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
		    "Invalid number of options (%d).\n", nConvOptions); 
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

#define NGL_XML_NS_SESSION_INFORMATION          "http://ninf.apgrid.org/2007/11/SessionInformation"
#define NGL_XML_PREFIX_SESSION_INFORMATION      "si"

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
 * Protocol: Get  Session Information From XML string
 */
int
ngiProtocolGetSessionInformationFromXML(
    ngiProtocol_t *protocol,
    char *xmlString,
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
    ngiXMLitem_t *element;
    static const char fName[] = "ngiProtocolGetSessionInformationFromXML";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(xmlString != NULL);
    nBytes = strlen(xmlString);
    assert(nBytes > 0);

    /* Construct XML parser */
    parser = ngiXMLparserConstruct(log, error);
    if (parser == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for XMLparser.\n"); 
        return 0;
    }

    /* Parse XML */
    result = ngiXMLparserParse(parser, xmlString, nBytes, 1, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "XML parse fail.\n"); 
        return 0;
    }

    /* Get root element tree */
    element = ngiXMLparserGetRootElement(parser, log, error);
    if (element == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't get root element.\n"); 
        return 0;
    }

    /* Convert XML element tree to RemoteClassInformation */
    result = nglProtocolSessionInformationParse(
	protocol, element, siReal, siCPU, nTimesCalled,
	compInfo, nCompInfos, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Parse Error.\n"); 
        return 0;
    }

    /* Destruct XML parser */
    result = ngiXMLparserDestruct(parser, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't destruct XML parser\n"); 
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
    ngiXMLitem_t *element,
    ngSessionInformationExecutable_t *siReal,
    ngSessionInformationExecutable_t *siCPU,
    int *nTimesCalled,
    ngCompressionInformation_t *compInfo,
    int nCompInfos,
    ngLog_t *log,
    int *error)
{
    int result, i;
    ngiXMLitem_t *childElement;
    long protocolVersion;
    char *si = NGL_XML_NS_SESSION_INFORMATION;
    static const char fName[] = "nglProtocolSessionInformationParse";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(element != NULL);
    assert(siReal != NULL);
    assert(siCPU != NULL);
    assert(nTimesCalled != NULL);
    assert(((compInfo == NULL) && (nCompInfos == 0)) ||
	   ((compInfo != NULL) && (nCompInfos > 0)));

    if (!NGI_XML_ELEMENT_IS(element, si, NGL_XML_SESSION_INFORMATION)) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Unexpect tag item, %s is expected.\n", NGL_XML_SESSION_INFORMATION); 
	return 0;
    }

    childElement = ngiXMLelementGetFirstChild(element, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, si, NGL_XML_REAL_TIME))) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Tag <%s> is not found.\n", NGL_XML_REAL_TIME); 
	return 0;
    }

    result = nglProtocolSessionInformationConvertToTime(
	protocol, childElement, siReal, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't convert the real time.\n"); 
        return 0;
    }    

    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, si, NGL_XML_CPU_TIME))) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Tag <%s> is not found.\n", NGL_XML_CPU_TIME); 
	return 0;
    }

    result = nglProtocolSessionInformationConvertToTime(
	protocol, childElement, siCPU, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't convert the CPU time.\n"); 
        return 0;
    }

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the version number of partner's protocol.\n"); 
	return 0;
    }

    childElement = ngiXMLitemGetNext(childElement, 1, log, error);
    if ((childElement == NULL) ||
        (!NGI_XML_ELEMENT_IS(childElement, si, NGL_XML_CALLBACK_INFORMATION))) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Tag <%s> is not found.\n", NGL_XML_CALLBACK_INFORMATION); 
        return 0;
    }

    /* Get the argumentNo */
    result = ngiXMLelementGetAttributeValueAsInt(
        childElement, NULL, NGL_XML_CALLBACK_NTIMES_CALLED, 0, INT_MAX, nTimesCalled, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't get value of %s attribute.\n", NGL_XML_CALLBACK_NTIMES_CALLED); 
        return 0;
    }

    for (i = 0; i < nCompInfos; i++) {
        childElement = ngiXMLitemGetNext(childElement, 1, log, error);
        if ((childElement == NULL) ||
                (NGI_XML_ELEMENT_IS(childElement, si, NGL_XML_COMPRESSION_INFORMATION))) {
    	ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Tag <%s> is not found.\n",
    	    NGL_XML_COMPRESSION_INFORMATION); 
    	break;
        }

        result = nglProtocolCompressionInformationParse(
    	protocol, childElement, &compInfo[i], log, error);
        if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't convert the Compression Information.\n"); 
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
nglProtocolSessionInformationConvertToTime(
    ngiProtocol_t *protocol,
    ngiXMLitem_t *element,
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
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the version number of partner's protocol.\n"); 
	return 0;
    }

    /* Method: Transfer Argument Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_TRANSFER_ARGUMENT,
	&siExec->ngsie_transferArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_TRANSFER_ARGUMENT); 
        return 0;
    }

    /* Method: Transfer File from Client to Remote */
    result = nglProtocolXMLstringToTime(
        element, NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE,
        &siExec->ngsie_transferFileClientToRemote, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE); 
        return 0;
    }

    /* Method: Calculation */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALCULATION,
	&siExec->ngsie_calculation, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_CALCULATION); 
        return 0;
    }

    /* Method: Tansfer Result Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_TRANSFER_RESULT,
	&siExec->ngsie_transferResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_TRANSFER_RESULT); 
        return 0;
    }

    /* Method: Transfer File from Remote to Client */
    result = nglProtocolXMLstringToTime(
        element, NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT,
        &siExec->ngsie_transferFileRemoteToClient, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT); 
        return 0;
    }

    /* Callback: Transfer Argument */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_TRANSFER_ARGUMENT,
	&siExec->ngsie_callbackTransferArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_CALLBACK_TRANSFER_ARGUMENT); 
        return 0;
    }

    /* Callback: Calculation */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_CALCULATION,
	&siExec->ngsie_callbackCalculation, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_CALLBACK_CALCULATION); 
        return 0;
    }

    /* Callback: Transfer Result Data */
    result = nglProtocolXMLstringToTime(
	element, NGL_XML_CALLBACK_TRANSFER_RESULT,
	&siExec->ngsie_callbackTransferResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Session Information: Can't get the XML attribute \"%s\"\n",
            NGL_XML_CALLBACK_TRANSFER_RESULT); 
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
    ngiXMLitem_t *element,
    ngCompressionInformation_t *compInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    char *pvalue;
    char *pend;
    int value;
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
    result = ngiXMLelementGetAttributeValueAsInt(element, NULL,
        NGL_XML_COMPRESSION_ARGUMENT_NO, 0, INT_MAX, &value, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get value of %s attribute.\n",
            NGL_XML_COMPRESSION_ARGUMENT_NO);
	return 0;
    }
    /* NGL_XML_COMPRESSION_ARGUMENT_NO is not used */

    /* Get the beforeCompressionLength */
    pvalue = ngiXMLelementGetAttribute(element, NULL,
        NGL_XML_BEFORE_COMPRESSION_LENGTH, log, error);
    if (pvalue == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "%s: Attribute is not found.\n", NGL_XML_BEFORE_COMPRESSION_LENGTH); 
	return 0;
    }
    /* Not measured? */
    if (strlen(pvalue) == 0) {
	memset(&compInfo->ngci_out.ngcic_compression, 0,
	    sizeof (compInfo->ngci_out.ngcic_compression));

	/* Skip the attributes */
	result = nglProtocolCompressionInformationSkipAttribute(
	    element, (char **)compAttrs, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't skip the attributes.\n"); 
	    return 0;
	}
    } else {
	compInfo->ngci_out.ngcic_compression.ngcie_measured = 1;
        result = ngiStringToInt(pvalue, &value, log, error);
        if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Can't get integer value from string.\n",
                NGL_XML_BEFORE_COMPRESSION_LENGTH); 
	    return 0;
        }
	compInfo->ngci_out.ngcic_compression.ngcie_lengthRaw = value;

	/* Get the afterCompressionLength */
        result = ngiXMLelementGetAttributeValueAsInt(element, NULL,
            NGL_XML_AFTER_COMPRESSION_LENGTH, 0, INT_MAX, &value, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't get value of %s attribute.\n",
                NGL_XML_AFTER_COMPRESSION_LENGTH);
	    return 0;
	}
	compInfo->ngci_out.ngcic_compression.ngcie_lengthCompressed = value;

	/* Get the compressionRealTime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_COMPRESSION_REAL_TIME,
	    &compInfo->ngci_out.ngcic_compression.ngcie_executionRealTime,
	    log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Session Information: Can't get the XML attribute \"%s\"\n",
                NGL_XML_COMPRESSION_REAL_TIME); 
	    return 0;
	}

	/* Get the compressionCPUtime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_COMPRESSION_CPU_TIME,
	    &compInfo->ngci_out.ngcic_compression.ngcie_executionCPUtime,
	    log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Session Information: Can't get the XML attribute \"%s\"\n",
                NGL_XML_COMPRESSION_CPU_TIME); 
	    return 0;
	}
    }

    /* Get the beforeDecompressionLength */
    pvalue = ngiXMLelementGetAttribute(
	element, NULL, NGL_XML_BEFORE_DECOMPRESSION_LENGTH, log, error);
    if (pvalue == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "%s: Attribute is not found.\n", NGL_XML_BEFORE_DECOMPRESSION_LENGTH); 
	return 0;
    }
    /* Not measured? */
    if (strlen(pvalue) == 0) {
	memset(&compInfo->ngci_in.ngcic_decompression, 0,
	    sizeof (compInfo->ngci_in.ngcic_decompression));

	/* Skip the attributes */
	result = nglProtocolCompressionInformationSkipAttribute(
	    element, (char **)decompAttrs, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't skip the attributes.\n"); 
	    return 0;
	}
    } else {
	compInfo->ngci_in.ngcic_decompression.ngcie_measured = 1;
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Value is not found.\n", NGL_XML_BEFORE_DECOMPRESSION_LENGTH); 
	    return 0;
	}
	compInfo->ngci_in.ngcic_decompression.ngcie_lengthCompressed = value;

	/* Get the afterCompressionLength */
	pvalue = ngiXMLelementGetAttribute(element, NULL,
            NGL_XML_AFTER_DECOMPRESSION_LENGTH, log, error);
	if (pvalue == NULL) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Attribute is not found.\n", NGL_XML_AFTER_DECOMPRESSION_LENGTH); 
	    return 0;
	}
	value = strtol(pvalue, &pend, 0);
	if (pend == pvalue) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Value is not found.\n", NGL_XML_BEFORE_DECOMPRESSION_LENGTH); 
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
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Session Information: Can't get the XML attribute \"%s\"\n",
                NGL_XML_DECOMPRESSION_REAL_TIME); 
	    return 0;
	}

	/* Get the decompressionCPUtime */
	result = nglProtocolXMLstringToTime(
	    element, NGL_XML_DECOMPRESSION_CPU_TIME,
	    &compInfo->ngci_in.ngcic_decompression.ngcie_executionCPUtime,
	    log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Session Information: Can't get the XML attribute \"%s\"\n",
                NGL_XML_DECOMPRESSION_CPU_TIME); 
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
    ngiXMLitem_t *element, 
    char *attribute,
    struct timeval *tv,
    ngLog_t *log,
    int *error)
{
    int result;
    char *value;
    static const char fName[] = "nglProtocolXMLstringToTime";

    /* Get the value of attribute */
    value = ngiXMLelementGetAttribute(element, NULL, attribute, log, error);
    if (value == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the value of attribute \"%s\"\n", attribute); 
	return 0;
    }

    /* Convert string to time */
    result = ngiStringToTimeval(value, tv, log, error);
    if (result ==  0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't convert the string to time.\n"); 
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
    ngiXMLitem_t *element,
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
	pvalue = ngiXMLelementGetAttribute(
	    element, NULL, attributes[i], log, error);
	if (pvalue == NULL) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Attribute is not found.\n", attributes[i]); 
	    return 0;
	}
        /* value is not empty */
	if (strlen(pvalue) > 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "%s: Value is found.\n", attributes[i]); 
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Protocol: Get XML from session information.
 */
int
ngiProtocolGetXMLstringFromSessionInformation(
    ngiProtocol_t *protocol,
    ngSessionInformationExecutable_t *siReal,
    ngSessionInformationExecutable_t *siCPU,
    int nTimesCalled,
    ngCompressionInformation_t *compInfo,
    int nCompInfos,
    char *buf,
    size_t buflen,
    ngLog_t *log,
    int *error)
{
    long protocolVersion;
    int result;
    int wNbytes = 0;
    int i;
    int ret = 1;
    char tvBuf[NGI_TIMEVAL_STRING_MAX];
    char *prefix = NGL_XML_PREFIX_SESSION_INFORMATION;
    struct {
        char                             *ngl_tagName;
        ngSessionInformationExecutable_t *ngl_si;
    } times[3];
    static const char fName[] = "ngiProtocolGetXMLstringFromSessionInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(siReal != NULL);
    assert(siCPU != NULL);
    assert(nTimesCalled >= 0);
    assert(buf != NULL);
    assert(buflen > 0);

    times[0].ngl_tagName = NGL_XML_REAL_TIME;
    times[0].ngl_si      = siReal;
    times[1].ngl_tagName = NGL_XML_CPU_TIME;
    times[1].ngl_si      = siCPU;
    times[2].ngl_tagName = NULL;
    times[2].ngl_si      = NULL;

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't get the version number of partner's protocol.\n"); 
	return 0;
    }

    wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes,
        "<?xml version=\"1.0\" encoding=\"us-ascii\"?>\n"
        "<%s:%s xmlns:%s=\"%s\">\n", prefix, NGL_XML_SESSION_INFORMATION,
        prefix, NGL_XML_NS_SESSION_INFORMATION);


    for (i = 0;times[i].ngl_tagName != NULL;++i) {
        /* Real Time */
        wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes,
            " <%s:%s\n", prefix, times[i].ngl_tagName);

#define NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(tv, attr) \
    do { \
        ret &= ngiTimevalToString(tvBuf, sizeof(tvBuf), (tv), log, error); \
        wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes, \
            "  %s=\"%s\"\n", (attr), tvBuf); \
    } while(0)

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_transferArgument),
            NGL_XML_TRANSFER_ARGUMENT);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_transferFileClientToRemote),
            NGL_XML_TRANSFER_FILE_CLIENT_TO_REMOTE);
    
        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_calculation),
            NGL_XML_CALCULATION);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_transferResult),
            NGL_XML_TRANSFER_RESULT);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_transferFileRemoteToClient),
            NGL_XML_TRANSFER_FILE_REMOTE_TO_CLIENT);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_callbackTransferArgument),
            NGL_XML_CALLBACK_TRANSFER_ARGUMENT);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_callbackCalculation),
            NGL_XML_CALLBACK_CALCULATION);

        NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE(
            &(times[i].ngl_si->ngsie_callbackTransferResult),
            NGL_XML_CALLBACK_TRANSFER_RESULT);

        wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes,
            " />\n");
    }
#undef NGL_PRINT_TIMEVAL_AS_XML_ATTRIBUTE

    wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes,
	" <%s:%s %s=\"%d\"/>\n",
        prefix, NGL_XML_CALLBACK_INFORMATION, NGL_XML_CALLBACK_NTIMES_CALLED,
        nTimesCalled);

    for (i = 0; i < nCompInfos;i++) {
	result = nglProtocolGetXMLstringFromCompressionInformation(
	    protocol, &compInfo[i],
	    i + 1, buf, buflen, &wNbytes, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
		"Can't make the Compression Information.\n"); 
		return 0;
	}
    }

    wNbytes += snprintf(&buf[wNbytes], buflen - wNbytes,
        "</%s:%s>\n", prefix, NGL_XML_SESSION_INFORMATION);

    if (wNbytes >= buflen) {
        NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Session Information: Buffer overflow.\n"); 
        return 0;
    }

    if (ret == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Session Information: Convert the time to string failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Pull Back Session: Convert Session Information.
 */
static int
nglProtocolGetXMLstringFromCompressionInformation(
    ngiProtocol_t *protocol,
    ngCompressionInformation_t *comp,
    int argNo,
    char *buf,
    int nBytes,
    int *wNbytes,
    ngLog_t *log,
    int *error)
{
    int ret;
    char tvBuf[NGI_TIMEVAL_STRING_MAX];
    char *prefix = NGL_XML_PREFIX_SESSION_INFORMATION;
    static const char fName[] = "nglProtocolGetXMLstringFromCompressionInformation";

    /* Check the arguments */
    assert(protocol != NULL);
    assert(comp != NULL);
    assert(argNo > 0);
    assert(buf != NULL);
    assert(nBytes > 0);
    assert(wNbytes != NULL);
    assert(*wNbytes >= 0);

    ret = 1;

    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	" <%s:%scompressionInformation \n",
        prefix, NGL_XML_COMPRESSION_INFORMATION);

    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	"  %s=\"%d\"\n", NGL_XML_COMPRESSION_ARGUMENT_NO, argNo);

    /* Compression */
    if (comp->ngci_out.ngcic_compression.ngcie_measured == 0) {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"\"\n"
	    "  %s=\"\"\n"
	    "  %s=\"\"\n"
	    "  %s=\"\"\n",
            NGL_XML_BEFORE_COMPRESSION_LENGTH,
            NGL_XML_AFTER_COMPRESSION_LENGTH,
            NGL_XML_COMPRESSION_REAL_TIME,
            NGL_XML_COMPRESSION_CPU_TIME);
    } else {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"%lu\"\n", NGL_XML_BEFORE_COMPRESSION_LENGTH,
	    (unsigned long)comp->ngci_out.ngcic_compression.ngcie_lengthRaw);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"%lu\"\n", NGL_XML_AFTER_COMPRESSION_LENGTH,
	    (unsigned long)
            comp->ngci_out.ngcic_compression.ngcie_lengthCompressed);

        ret &= ngiTimevalToString(tvBuf, sizeof(tvBuf),
	    &(comp->ngci_out.ngcic_compression.ngcie_executionRealTime),
            log, error);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"%s\"\n", NGL_XML_COMPRESSION_REAL_TIME, tvBuf);

        ret &= ngiTimevalToString(tvBuf, sizeof(tvBuf),
	    &(comp->ngci_out.ngcic_compression.ngcie_executionCPUtime),
            log, error);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"%s\"\n", NGL_XML_COMPRESSION_CPU_TIME, tvBuf);
    }

    /* Decompression */
    if (comp->ngci_in.ngcic_decompression.ngcie_measured == 0) {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  %s=\"\"\n"
	    "  %s=\"\"\n"
	    "  %s=\"\"\n"
	    "  %s=\"\"\n",
            NGL_XML_BEFORE_DECOMPRESSION_LENGTH,
            NGL_XML_AFTER_DECOMPRESSION_LENGTH,
            NGL_XML_DECOMPRESSION_REAL_TIME,
            NGL_XML_DECOMPRESSION_CPU_TIME);
    } else {
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  beforeDecompressionLength=\"%lu\"\n",
	    (unsigned long)comp->ngci_in.ngcic_decompression.ngcie_lengthRaw);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  afterDecompressionLength=\"%lu\"\n",
	    (unsigned long)
	    comp->ngci_in.ngcic_decompression.ngcie_lengthCompressed);

        ret &= ngiTimevalToString(tvBuf, sizeof(tvBuf),
	    &(comp->ngci_in.ngcic_decompression.ngcie_executionRealTime),
            log, error);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionRealTime=\"%s\"\n", tvBuf);

        ret &= ngiTimevalToString(tvBuf, sizeof(tvBuf),
	    &(comp->ngci_in.ngcic_decompression.ngcie_executionCPUtime),
            log, error);
	*wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	    "  decompressionCPUtime=\"%s\"\n", tvBuf);
    }

    *wNbytes += snprintf(&buf[*wNbytes], nBytes - *wNbytes,
	" />\n");

    if (ret == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Session Information: Convert the time to string failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

