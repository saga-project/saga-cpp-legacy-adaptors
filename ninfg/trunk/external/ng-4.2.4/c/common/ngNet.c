#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngNet.c,v $ $Revision: 1.34 $ $Date: 2006/02/02 07:04:35 $";
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
 * Module for managing the communication layer for Ninf-G Client/Executable.
 */

#include <assert.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static int nglNetCommunicatorInitializeDataSize(
    NET_Communicator *, ngLog_t *, int *);
static void nglNetCommunicatorInitializeMember(NET_Communicator *);
static void nglNetCommunicatorInitializePointer(NET_Communicator *);
static int nglNetCommunicatorSetEncodeBuffer(
    NET_Communicator *, char *, size_t, ngLog_t *, int *);
static int nglNetCommunicatorSetDecodeBuffer(
    NET_Communicator *, char *, size_t, ngLog_t *, int *);
static int nglNetCommunicatorReleaseBuffer(
    NET_Communicator *, ngLog_t *, int *);
#if 0 /* Is this necessary? */
static int nglNetCommunicatorInitializeXDR(
    NET_Communicator *, int, int, ngLog_t *, int *);
static int nglNetCommunicatorFinalizeXDR(NET_Communicator *, ngLog_t *, int *);
#endif

static size_t nglNetCommunicatorGetStrlenXDR(
    NET_Communicator *, size_t, ngLog_t *, int *);
static int nglNetCommunicatorReleaseXDRstring(
    NET_Communicator *, char **, ngLog_t *, int *);

static int nglNetCommunicatorConvertDataTypeToNet(
    ngArgumentDataType_t, ngLog_t *, int *);

#if defined(NG_OS_IRIX) || (__INTEL_COMPILER)
#define nglAssertSizet(expr) assert(1)
#define nglCheckSizetInvalid(expr) (0)
#else /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */
#define nglAssertSizet(expr) assert(expr)
#define nglCheckSizetInvalid(expr) (expr)
#endif /* defined(NG_OS_IRIX) || (__INTEL_COMPILER) */

/**
 * Construct
 */
NET_Communicator *
ngiNetCommunicatorConstruct(
    ngLog_t *log,
    int *error)
{
    int result;
    NET_Communicator *netComm;
    static const char fName[] = "ngiNetCommunicatorConstruct";

    /* Allocate */
    netComm = ngiNetCommunicatorAllocate(log, error);
    if (netComm == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Communicator.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiNetCommunicatorInitialize(netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the Communicator.\n", fName);
	goto error;
    }

    /* Success */
    return netComm;

    /* Error occurred */
error:
    result = ngiNetCommunicatorFree(netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't deallocate the storage for Communicator.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngiNetCommunicatorDestruct(NET_Communicator *netComm, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiNetCommunicatorDestruct";

    /* Check the arguments */
    assert(netComm != NULL);

    /* Finalize */
    result = ngiNetCommunicatorFinalize(netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Communicator.\n", fName);
	return 0;
    }

    result = ngiNetCommunicatorFree(netComm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't deallocate the storage for Communicator.\n", fName);
	return 0;
    }

    /* Success */
    return 0;
}

/**
 * Allocate
 */
NET_Communicator *
ngiNetCommunicatorAllocate(ngLog_t *log, int *error)
{
    NET_Communicator *netComm;
    static const char fName[] = "ngiNetCommunicatorAllocate";

    netComm = globus_libc_calloc(1, sizeof (NET_Communicator));
    if (netComm == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Communicator.", fName);
	return NULL;
    }

    /* Success */
    return netComm;
}

/**
 * Deallocate
 */
int
ngiNetCommunicatorFree(NET_Communicator *netComm, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);

    /* Deallocate */
    globus_libc_free(netComm);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngiNetCommunicatorInitialize(
    NET_Communicator *netComm,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiNetCommunicatorInitialize";

    /* Check the arguments */
    assert(netComm != NULL);

    /* Initialize the pointers */
    nglNetCommunicatorInitializeMember(netComm);

    /* Get the data size */
    result = nglNetCommunicatorInitializeDataSize(netComm, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the Data Size.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the data size.
 */
static int
nglNetCommunicatorInitializeDataSize(
    NET_Communicator *netComm,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglNetCommunicatorInitializeDataSize";

    /* Check the arguments */
    assert(netComm != NULL);

    /* Get the size of char */
    result = NET_xdrsizeof(NET_CHAR);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of char.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_char = result;

    /* Get the size of short */
    result = NET_xdrsizeof(NET_S_INT);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of short.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_short = result;

    /* Get the size of int */
    result = NET_xdrsizeof(NET_I);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of int.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_int = result;

    /* Get the size of long */
    result = NET_xdrsizeof(NET_L_INT);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of long.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_long = result;

    /* Get the size of float */
    result = NET_xdrsizeof(NET_S);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of float.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_float = result;

    /* Get the size of double */
    result = NET_xdrsizeof(NET_D);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of double.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_double = result;

    /* Get the size of scomplex */
    result = NET_xdrsizeof(NET_C);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of scomplex.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_scomplex = result;

    /* Get the size of dcomplex */
    result = NET_xdrsizeof(NET_Z);
    if (result <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the size of dcomplex.\n", fName);
	return 0;
    }
    netComm->nc_dataSize.ngds_dcomplex = result;

    /* Success */
    return 1;
}

/**
 * Finalize
 */
int
ngiNetCommunicatorFinalize(
    NET_Communicator *netComm,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);

    /* Initialize the pointers */
    nglNetCommunicatorInitializeMember(netComm);

    /* Success */
    return 1;
}

/**
 * Initialize the member.
 */
static void
nglNetCommunicatorInitializeMember(NET_Communicator *netComm)
{
    /* Check the arguments */
    assert(netComm != NULL);

    /* Initialize the pointers */
    nglNetCommunicatorInitializePointer(netComm);

    /* Initialize the members */
    netComm->nc_nBytes = 0;
}

/**
 * Initialize the pointer.
 */
static void
nglNetCommunicatorInitializePointer(NET_Communicator *netComm)
{
    /* Check the arguments */
    assert(netComm != NULL);

    /* Initialize the pointers */
    netComm->nc_buffer = NULL;
}

/**
 * Set the buffer for encode.
 */
static int
nglNetCommunicatorSetEncodeBuffer(
    NET_Communicator *netComm,
    char *buffer,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(netComm->nc_buffer == NULL);
    assert(netComm->nc_nBytes == 0);
    assert(buffer != NULL);
    assert(nBytes > 0);

    /* Set the buffer */
    netComm->nc_buffer = buffer;
    netComm->nc_nBytes = nBytes;

    /* Initialize the XDR stream */
    xdrmem_create(&netComm->nc_xdrStream, buffer, nBytes, XDR_ENCODE);

    /* Success */
    return 1;
}

/**
 * Set the buffer for decode.
 */
static int
nglNetCommunicatorSetDecodeBuffer(
    NET_Communicator *netComm,
    char *buffer,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(netComm->nc_buffer == NULL);
    assert(netComm->nc_nBytes == 0);
    assert(buffer != NULL);
    assert(nBytes > 0);

    /* Set the buffer */
    netComm->nc_buffer = buffer;
    netComm->nc_nBytes = nBytes;

    /* Initialize the XDR stream */
    xdrmem_create(&netComm->nc_xdrStream, buffer, nBytes, XDR_DECODE);

    /* Success */
    return 1;
}

/**
 * Release the buffer.
 */
static int
nglNetCommunicatorReleaseBuffer(
    NET_Communicator *netComm,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(netComm->nc_buffer != NULL);
    assert(netComm->nc_nBytes > 0);

    /* Set the buffer */
    netComm->nc_buffer = NULL;
    netComm->nc_nBytes = 0;

    /* Destroy the XDR stream */
    xdr_destroy(&netComm->nc_xdrStream);

    /* Success */
    return 1;
}

#if 0 /* 2003.8.29 Is this necessary? */
/**
 * Initialize XDR
 */
static int
nglNetCommunicatorInitializeXDR(
    NET_Communicator *netComm,
    int dataType,
    int nElements,
    ngLog_t *log,
    int *error)
{
    size_t xdrNbytes;
    int result;
    static const char fName[] = "nglNetCommunicatorInitializeXDR";

    /* Check tha arguments */
    assert(netComm != NULL);
    assert(netComm->nc_xdrBuffer == NULL);
    assert(nElements > 0);

    /* Get the number of bytes of data */
    result = ngiNetCommunicatorGetDataSize(
    	netComm, dataType, &xdrNbytes, log, error);
    if ((result == 0) || (nBytes < 0)) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the number of bytes of data.\n", fName);
	return 0;
    }
    netComm->nc_xdrNbytes = xdrNbytes * nElements;

    /* Allocate the data buffer */
    netComm->nc_xdrBuffer = globus_libc_calloc(1, netComm->nc_xdrNbytes);
    if (netComm->nc_xdrBuffer == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
   	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for Data Buffer.\n", fName);
	return 0;
    }

    /* Initialize the XDR stream */
    xdrmem_create(
    	&netComm->nc_xdrStream, netComm->nc_xdrBuffer, nBytes, XDR_FREE);
    xdrmem_create(
    	&netComm->nc_xdrStream, netComm->nc_xdrBuffer, nBytes, XDR_ENCODE);

    /* Success */
    return 1;
}

/**
 * Finalize XDR
 */
static int
nglNetCommunicatorFinalizeXDR(
    NET_Communicator *netComm, ngLog_t *log, int error)
{
    static const char fName[] = "nglNetCommunicatorFinalizeXDR";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(netComm->nc_xdrBuffer != NULL);
    assert(netComm->nc_xdrNbytes > 0);

    /* Destroy the XDR stream */
    xdr_destroy(&netComm->xdrStream);

    /* Deallocate the data buffer */
    globus_libc_free(netComm->nc_xdrBuffer);
    netComm->nc_xdrBuffer = NULL;
    netComm->nc_xdrNbytes = 0;

    /* Success */
    return 1;
}
#endif /* 0 */

/**
 * Copy the data size.
 */
void
ngiNetCommunicatorCopyDataSize(
    NET_Communicator *netComm,
    ngiDataSize_t *dest)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(dest != NULL);

    *dest = netComm->nc_dataSize;
}

/**
 * Get the data size.
 */
int
ngiNetCommunicatorGetDataSize(
    NET_Communicator *netComm,
    ngArgumentDataType_t dataType,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
#if 1
{
    static const char fName[] = "ngiNetCommunicatorGetDataSize";

    /* Check the arguments */
    if (nBytes == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    switch (dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
    	*nBytes = netComm->nc_dataSize.ngds_char;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_SHORT:
    	*nBytes = netComm->nc_dataSize.ngds_short;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_INT:
    	*nBytes = netComm->nc_dataSize.ngds_int;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_LONG:
    	*nBytes = netComm->nc_dataSize.ngds_long;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_FLOAT:
    	*nBytes = netComm->nc_dataSize.ngds_float;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
    	*nBytes = netComm->nc_dataSize.ngds_double;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
    	*nBytes = netComm->nc_dataSize.ngds_scomplex;
    	return 1;

    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
    	*nBytes = netComm->nc_dataSize.ngds_dcomplex;
    	return 1;

    default:
    	/* Do nothing */
	break;
    }

    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	NULL, "%s: Unknown data type %d.\n", fName, dataType);

    /* Failed */
    return 0;
}
#else /* 1 */
{
    int result;
    int netNbytes;
    int netDataType;
    static const char fName[] = "ngiNetCommunicatorGetDataSize";

    /* Check the arguments */
    if ((netComm == NULL) || (nBytes == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    /* Convert the Data Type */
    netDataType = nglNetCommunicatorConvertDataTypeToNet(
	dataType, log, error);
    if (netDataType < 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Unknown data type %d.\n", fName, dataType);
	return 0;
    }

    netNbytes = NET_xdrsizeof(netDataType);
    if (netNbytes < 0) {
	NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: NET_xdrsizeof failed.\n", fName);
	return 0;
    }
    *nBytes = netNbytes;

    /* Success */
    return 1;
}
#endif /* 1 */

/**
 * Write the INT.
 */
int
ngiNetCommunicatorWriteInt(
    NET_Communicator *netComm,
    int data,
    void *buffer,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiNetCommunicatorWriteInt";

    /* Check the arguments */
    assert(netComm != NULL);

    /* Write the int */
    result = ngiNetCommunicatorWriteArray(
	netComm, (ngArgumentDataType_t) NET_I, &data, 1, buffer,
	netComm->nc_dataSize.ngds_int, log, error);
    if (result < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the data to Stream Buffer.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Write the array.
 */
int
ngiNetCommunicatorWriteArray(
    NET_Communicator *netComm,
    ngArgumentDataType_t dataType,
    void *data,		/* Read from this data */
    int nElements,	/* The number of elements of data above */
    void *buffer,	/* Write to this buffer */
    size_t nBytes,	/* The number of bytes of buffer above */
    ngLog_t *log,
    int *error)
{
    int result;
    int netDataType;
    static const char fName[] = "ngiNetCommunicatorWriteArray";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(data != NULL);
    assert(nElements > 0);

    /* Convert the Data Type */
    netDataType = nglNetCommunicatorConvertDataTypeToNet(
	dataType, log, error);
    if (netDataType < 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Unknown data type %d.\n", fName, dataType);
	return 0;
    }

    /* Set the buffer to write */
    result = nglNetCommunicatorSetEncodeBuffer(
	netComm, buffer, nBytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the buffer to Net Communicator.\n", fName);
	return 0;
    }

    /* Write the data */
    result = NET_sendArray(netComm, netDataType, data, nElements);
    if (result == 0) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't write the data to Net Communicator.\n", fName);
	goto error;
    }

    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
    }

    /* Faild */
    return 0;
}

/**
 * Read the array.
 */
int
ngiNetCommunicatorReadArray(
    NET_Communicator *netComm,
    ngArgumentDataType_t dataType,
    void *buffer,	/* Read from this buffer */
    size_t nBytes,	/* The number of bytes of buffer above */
    void *data,		/* Write to this data */
    int nElements,	/* The number of elements of data above */
    ngLog_t *log,
    int *error)
{
    int result;
    int netDataType;
    static const char fName[] = "ngiNetCommunicatorReadArray";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(data != NULL);
    assert(nElements > 0);

    /* Convert the Data Type */
    netDataType = nglNetCommunicatorConvertDataTypeToNet(
	dataType, log, error);
    if (netDataType < 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Unknown data type %d.\n", fName, dataType);
	return 0;
    }

    /* Set the buffer to read */
    result = nglNetCommunicatorSetDecodeBuffer(
	netComm, buffer, nBytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the buffer to Net Communicator.\n", fName);
	return 0;
    }

    /* Read the data */
    result = NET_recvArray(netComm, netDataType, data, nElements);
    if (result < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the data from Net Communicator.\n", fName);
	return 0;
    }

    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

#if 0 /* Is this necessary? */
    /* Error occurred */
error:
    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
	return 0;
    }

    /* Failed */
    return 0;
#endif
}

/**
 * Write the string.
 */
int
ngiNetCommunicatorWriteString(
    NET_Communicator *netComm,
    char *string,	/* Read from this buffer */
    size_t strNbytes,	/* The number of bytes of xdrBuffer above */
    void **xdrBuff,	/* Write to this string */
    size_t *xdrNbytes,	/* The number of bytes of string above */
    ngLog_t *log,
    int *error)
{
    int result;
    int buffInitialized = 0;
    static const char fName[] = "ngiNetCommunicatorWriteString";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(string != NULL);
    nglAssertSizet(strNbytes >= 0);
    assert(xdrBuff != NULL);
    assert(xdrNbytes != NULL);

    /* Initialize the argument */
    *xdrBuff = NULL;

    /* Get sizeof string of XDR encoded */
    *xdrNbytes = nglNetCommunicatorGetStrlenXDR(netComm, strNbytes, log, error);
    if (nglCheckSizetInvalid(*xdrNbytes < 0)) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the sizeof data of XDR encoded.\n", fName);
	goto error;
    }

    /* Allocate the storage for XDR */
    *xdrBuff = globus_libc_calloc(1, *xdrNbytes);
    if (*xdrBuff == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the storage for XDR.\n", fName);
	goto error;
    }

    /* Set the buffer to read */
    result = nglNetCommunicatorSetEncodeBuffer(
	netComm, *xdrBuff, *xdrNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the buffer to Net Communicator.\n", fName);
	goto error;
    }
    buffInitialized = 1;

    /* Read the data */
    result = NET_sendString(netComm, string, strNbytes);
    if (result < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the data from Net Communicator.\n", fName);
	goto error;
    }

    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, error);
    buffInitialized = 0;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate the storage for XDR */
    if (*xdrBuff != NULL)
	globus_libc_free(xdrBuff);
    xdrBuff = NULL;

    /* Release the buffer to write */
    if (buffInitialized != 0) {
	buffInitialized = 0;
	result = nglNetCommunicatorReleaseBuffer(netComm, log, NULL);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't release the buffer from Net Communicator.\n",
		fName);
	}
    }

    /* Failed */
    return 0;
}

/**
 * Get the number of bytes of string after XDR encoded.
 */
static size_t
nglNetCommunicatorGetStrlenXDR(
    NET_Communicator *netComm,
    size_t strNbytes,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    nglAssertSizet(strNbytes >= 0); 

    /*
     * Calculate the number of bytes.
     * length (4bytes) + strNbytes + padding
     * See also RFC 1832 about string.
     */
    return 4 + ((strNbytes + 4) / 4 * 4);
}

/**
 * Deallocate the storage for XDR.
 */
int
ngiNetCommunicatorReleaseXDRbuffer(
    NET_Communicator *netComm,
    void *xdrBuff,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(xdrBuff != NULL);

    /* Deallocate */
    globus_libc_free(xdrBuff);

    /* Success */
    return 1;
}

/**
 * Read the string.
 */
int
ngiNetCommunicatorReadString(
    NET_Communicator *netComm,
    void *xdrBuffer,	/* Read from this buffer */
    size_t xdrNbytes,	/* The number of bytes of xdrBuffer above */
    char **string,	/* Write to this string */
    size_t *strNbytes,	/* The number of bytes of string above */
    ngLog_t *log,
    int *error)
{
    int result;
    int buffInitialized = 0;
    char *tmp = NULL;
    static const char fName[] = "ngiNetCommunicatorReadString";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(xdrBuffer != NULL);
    assert(xdrNbytes > 0);
    assert(string != NULL);
    assert(strNbytes != NULL);

    /* Initialize the argument */
    *string = NULL;

    /* Set the buffer to read */
    result = nglNetCommunicatorSetDecodeBuffer(
	netComm, xdrBuffer, xdrNbytes, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't set the buffer to Net Communicator.\n", fName);
	goto error;
    }
    buffInitialized = 1;

    /* Read the data */
    result = NET_recvString(netComm, &tmp);
    if (result < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't read the data from Net Communicator.\n", fName);
	goto error;
    }

    /* Copy the string */
    *strNbytes = strlen(tmp);
    *string = strdup(tmp);
    if (*string == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't duplicate the string.\n", fName);
	goto error;
    }

    /* Release the buffer to write */
    result = nglNetCommunicatorReleaseBuffer(netComm, log, error);
    buffInitialized = 0;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer from Net Communicator.\n",
	    fName);
	goto error;
    }

    /* Release the buffer of temporary */
    result = nglNetCommunicatorReleaseXDRstring(netComm, &tmp, log, error);
    tmp = NULL;
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't release the buffer for temporary.\n",
	    fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate the storage for string */
    if (*string != NULL)
	free(string);
    string = NULL;

    /* Release the buffer to write */
    if (buffInitialized != 0) {
	buffInitialized = 0;
	result = nglNetCommunicatorReleaseBuffer(netComm, log, NULL);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't release the buffer from Net Communicator.\n",
		fName);
	}
    }

    /* Release the buffer of temporary */
    if (tmp != NULL) {
	result = nglNetCommunicatorReleaseXDRstring(netComm, &tmp, log, error);
	tmp = NULL;
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't release the buffer for temporary.\n", fName);
	}
    }

    /* Failed */
    return 0;
}

/**
 * Release the XDR string.
 */
static int
nglNetCommunicatorReleaseXDRstring(
    NET_Communicator *netComm,
    char **buffer,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglNetCommunicatorReleaseXDRstring";

    /* Check the arguments */
    assert(netComm != NULL);
    assert(buffer != NULL);

    /* Initialize the XDR stream */
    xdrmem_create(&netComm->nc_xdrStream, NULL, 0, XDR_FREE);
    result = xdr_string(&netComm->nc_xdrStream, buffer, strlen(*buffer) + 1);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
           "%s: xdr_string failed.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}
/**
 * Release the string.
 */
int
ngiNetCommunicatorReleaseString(
    NET_Communicator *netComm,
    char *string,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(netComm != NULL);
    assert(string != NULL);

    /* Deallocate the storage */
    free(string);
    string = NULL;

    /* Success */
    return 1;
}
/**
 * Convert data type from Ninf-G internal to NetSolve.
 */
static int
nglNetCommunicatorConvertDataTypeToNet(
    ngArgumentDataType_t dataType,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglNetCommunicatorConvertDataTypeToNet";

    switch (dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
    	return NET_CHAR;

    case NG_ARGUMENT_DATA_TYPE_SHORT:
    	return NET_S_INT;

    case NG_ARGUMENT_DATA_TYPE_INT:
    	return NET_I;

    case NG_ARGUMENT_DATA_TYPE_LONG:
    	return NET_L_INT;

    case NG_ARGUMENT_DATA_TYPE_FLOAT:
    	return NET_S;

    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
    	return NET_D;

    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
    	return NET_C;

    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
    	return NET_Z;

    default:
	/* Do nothing */
	break;
    }

    /* Unknown data type */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
    	"%s: Unknown data type %d.\n", fName, dataType);
    return -1;
}
