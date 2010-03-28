#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngArgumentConvert.c,v $ $Revision: 1.100 $ $Date: 2007/05/14 05:10:45 $";
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
 * Module of Convert Argument for Ninf-G Client.
 */

#include <stdlib.h>
#include <string.h>
#if defined(NG_OS_IRIX) || defined(NG_OS_FREEBSD) || defined(NG_OS_MACOSX)
#include <rpc/rpc.h>
#elif defined(NG_OS_SOLARIS)
#include <rpc/types.h>
#include <rpc/xdr.h>
#else
#include <rpc/xdr.h>
#endif
#include "ng.h"
#ifndef NGI_NO_ZLIB
#include <zlib.h>
#endif /* !NGI_NO_ZLIB */


/**
 * Prototype declaration of internal functions.
 */
static ngiArgument_t *nglArgumentAllocate(int, ngLog_t *log, int *);
static int nglArgumentFree(ngiArgument_t *arg, ngLog_t *log, int *error);
static int nglArgumentInitialize(
    ngiArgument_t *, ngArgumentInformation_t *, int,
    ngLog_t *, int *);
static int nglArgumentFinalize(ngiArgument_t *, ngLog_t *, int *);
static void nglArgumentInitializeMember(ngiArgument_t *);
static void nglArgumentInitializePointer(ngiArgument_t *);

static int nglArgumentDataRelease(ngiArgument_t *, ngLog_t *, int *);

static int nglArgumentGetValueInt(
    ngiArgument_t *, ngiArgument_t *, int, int *, ngLog_t *, int *);

#if 0
/* Note:
 * Nobody is using this function. However, it leaves the definition for
 * compatibility with other functions.
 */
static ngiArgumentElement_t *nglArgumentElementConstruct(
    ngArgumentInformation_t *, int, ngLog_t *, int *);
static int nglArgumentElementDestruct(
    ngiArgumentElement_t *, int, ngLog_t *, int *);
static ngiArgumentElement_t *nglArgumentElementAllocate(int, ngLog_t *, int *);
static int nglArgumentElementFree(ngiArgumentElement_t *, ngLog_t *, int *);
#endif
static int nglArgumentElementInitialize(
    ngiArgumentElement_t *, ngArgumentInformation_t *, ngLog_t *, int *);
static int nglArgumentElementFinalize(
    ngiArgumentElement_t *, ngLog_t *, int *);
static void nglArgumentElementInitializeMember(ngiArgumentElement_t *);
static void nglArgumentElementInitializePointer(ngiArgumentElement_t *);

static int nglArgumentElementArgumentDataCopy(
    ngiArgumentElement_t *, ngLog_t *, int *);
static int nglArgumentElementArgumentDataRelease(
    ngiArgumentElement_t *, ngLog_t *, int *);

static int nglArgumentEncodeAppendRawConversionMethod(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t **,
    ngLog_t *, int *);
static int nglArgumentEncodeDivision(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t **,
    ngLog_t *, int *);
static int nglArgumentEncodeCompression(
    ngiArgumentElement_t *, ngCompressionInformationElement_t *,
    ngiProtocol_t *, ngiStreamManager_t **, ngLog_t *, int *);
static int nglArgumentEncodeData(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglArgumentEncodeString(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglArgumentEncodeStringArray(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    ngLog_t *, int *);
static int nglArgumentEncodeFile(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    int, ngLog_t *, int *);
static int nglArgumentAppendPadding(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    size_t, ngLog_t *, int *);
static int nglArgumentForceAppendPadding(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    size_t, ngLog_t *, int *);
static int nglArgumentForceAppendPaddingStreamManager(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    size_t, ngLog_t *, int *);

static int
nglArgumentDecodeConversion(ngiArgumentElement_t *, 
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *, void *, ngiProtocolArgumentData_t *,
    int, ngLog_t *, int *);
static int
nglArgumentDecodeConversionSub(ngiArgumentElement_t *,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *, ngiStreamManager_t *,
    ngiStreamManager_t *, ngiProtocolArgumentData_t *, ngLog_t *, int *);
static int nglArgumentDecodeData(ngiArgumentElement_t *,
    ngCompressionInformationComplex_t *compInfo,
    int, ngiProtocol_t *, ngiProtocolArgumentData_t *,
    ngLog_t *, int *);
static int nglArgumentDecodeStringWithoutAllocate(
    ngiArgumentElement_t *, 
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *, ngiProtocolArgumentData_t *, ngLog_t *, int *);
static int nglArgumentDecodeStringWithAllocate(
    ngiArgumentElement_t *, 
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *, ngiProtocolArgumentData_t *, ngLog_t *, int *);
static int
nglArgumentDecodeXDRString(ngiArgumentElement_t *,
    ngiProtocol_t *, ngiStreamManager_t *,
    ngiProtocolArgumentData_t *, ngLog_t *, int *);
static int nglArgumentDecodeFile(
    ngiArgumentElement_t *, 
    ngCompressionInformationComplex_t *compInfo,
    int, ngiProtocol_t *, ngiProtocolArgumentData_t *,
    ngLog_t *, int *);
 
#if 0 /* Is this necessary? */
static int nglArgumentDeletePadding(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    size_t, ngLog_t *, int *);
#endif
static int nglArgumentDeletePaddingWithReceive(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiCommunication_t *,
    size_t, ngLog_t *, int *);
#if 0 /* Is this necessary? */
static int nglArgumentDeletePaddingFromStream(
    ngiArgumentElement_t *, ngiProtocol_t *, ngiStreamManager_t *,
    size_t, ngLog_t *, int *);
#endif

static int nglArgumentEncodeSkip(
    ngiArgumentElement_t *, void *, int, void **, int *, ngLog_t *, int *);
static void nglArgumentEncodeSkipRecursiveDimension(
    ngiArgumentElement_t *, void *, void *, int *, int, int);
static int nglArgumentDecodeSkip(
    ngiArgumentElement_t *, void *, int, void **, int *, ngLog_t *, int *);
static void nglArgumentDecodeSkipRecursiveDimension(
    ngiArgumentElement_t *, void *, int *, void *, int, int);
static void nglSkipCopyChar(char *, int *, char *, int, int, int);
static void nglSkipCopyShort(short *, int *, short *, int, int, int);
static void nglSkipCopyInt(int *, int *, int *, int, int, int);
static void nglSkipCopyLong(long *, int *, long *, int, int, int);
static void nglSkipCopyFloat(float *, int *, float *, int, int, int);
static void nglSkipCopyDouble(double *, int *, double *, int, int, int);
static void nglSkipCopyScomplex(scomplex *, int *, scomplex *, int, int, int);
static void nglSkipCopyDcomplex(dcomplex *, int *, dcomplex *, int, int, int);
static void nglSkipCopyCharOfDecode(char *, char *, int *, int, int, int);
static void nglSkipCopyShortOfDecode(short *, short *, int *, int, int, int);
static void nglSkipCopyIntOfDecode(int *, int *, int *, int, int, int);
static void nglSkipCopyLongOfDecode(long *, long *, int *, int, int, int);
static void nglSkipCopyFloatOfDecode(float *, float *, int *, int, int, int);
static void nglSkipCopyDoubleOfDecode(
    double *, double *, int *, int, int, int);
static void nglSkipCopyScomplexOfDecode(
    scomplex *, scomplex *, int *, int, int, int);
static void nglSkipCopyDcomplexOfDecode(
    dcomplex *, dcomplex *, int *, int, int, int);
#ifndef NGI_NO_ZLIB
static int nglArgumentEncodeZlib(
    ngiStreamManager_t *, ngiStreamManager_t *, size_t *,
    ngCompressionInformationElement_t *, ngLog_t *, int *);
static int nglArgumentDecodeZlib(
    ngiStreamManager_t *, long, ngiStreamManager_t *, size_t *,
    ngCompressionInformationElement_t *, ngLog_t *, int *);
#endif /* !NGI_NO_ZLIB */

static int nglEvaluateExpressionPlus(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionMinus(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionMultiply(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionDivide(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionModulo(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionUnaryMinus(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionPower(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionEqual(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionNotEqual(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionGreaterThan(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionLessThan(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionGreaterEqual(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionLessEqual(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
static int nglEvaluateExpressionTri(
    ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);

/**
 * Argument Data Manager: Construct
 */
ngiArgument_t *
ngiArgumentConstruct(
    ngArgumentInformation_t *argInfo,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiArgument_t *arg;
    static const char fName[] = "ngiArgumentConstruct";

    /* Check the arguments */
    assert(((nArguments > 0) && (argInfo != NULL)) ||
	   ((nArguments == 0) && (argInfo == NULL)));

    /* Allocate */
    arg = nglArgumentAllocate(nArguments, log, error);
    if (arg == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = nglArgumentInitialize(arg, argInfo, nArguments, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Argument.\n", fName);
	goto error;
    }

    /* Success */
    return arg;

    /* Error occurred */
error:
    result = nglArgumentFree(arg, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Argument.\n", fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Argument Data Manager: Destruct
 */
int
ngiArgumentDestruct(
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiArgumentDestruct";

    /* Check the arguments */
    assert(arg != NULL);

    /* Release the copied argument data */
    result = nglArgumentDataRelease(arg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the argument data.\n", fName);
	return 0;
    }

    /* Finalize */
    result = nglArgumentFinalize(arg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Argument.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = nglArgumentFree(arg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Argument.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Argument Data Manager: Allocate
 */
static ngiArgument_t *
nglArgumentAllocate(int nArguments, ngLog_t *log, int *error)
{
    ngiArgument_t *arg;
    static const char fName[] = "nglArgumentAllocate";

    arg = globus_libc_calloc(
    	1,
	sizeof (ngiArgument_t) - sizeof (ngiArgumentElement_t) +
	(sizeof (ngiArgumentElement_t) * nArguments));
    if (arg == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument.\n", fName);
	return NULL;
    }

    /* Success */
    return arg;
}

/**
 * Argument Data Manager: Deallocate
 */
static int
nglArgumentFree(ngiArgument_t *arg, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(arg != NULL);

    /* Deallocate */
    globus_libc_free(arg);

    /* Success */
    return 1;
}

/**
 * Argument Data Manager: Initialize
 */
static int
nglArgumentInitialize(
    ngiArgument_t *arg,
    ngArgumentInformation_t *argInfo,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    static const char fName[] = "nglArgumentInitialize";

    /* Check the arguments */
    assert(arg != NULL);
    assert(((nArguments > 0) && (argInfo != NULL)) ||
	   ((nArguments == 0) && (argInfo == NULL)));

    /* Initialize the members */
    nglArgumentInitializeMember(arg);
    arg->nga_nArguments = nArguments;

    /* Initialize the Argument Element */
    for (i = 0; i < nArguments; i++) {
	result = nglArgumentElementInitialize(
	    &arg->nga_argument[i], &argInfo[i], log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Argument.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Argument Data Manager: Finalize
 */
static int
nglArgumentFinalize(ngiArgument_t *arg, ngLog_t *log, int *error)
{
    int result;
    int i;
    static const char fName[] = "nglArgumentFinalize";

    /* Check the arguments */
    assert(arg != NULL);


    /* Finalize the Argument Element */
    for (i = 0; i < arg->nga_nArguments; i++) {
	result = nglArgumentElementFinalize(&arg->nga_argument[i], log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't finalize the Argument.\n", fName);
	    return 0;
	}
    }

    /* Finalize */
    nglArgumentInitializeMember(arg);

    /* Success */
    return 1;
}

/**
 * Argument Data Manager: Initialize members
 */
static void
nglArgumentInitializeMember(ngiArgument_t *arg)
{
    /* Initialize the pointers */
    nglArgumentInitializePointer(arg);

    /* Initialize the members */
    arg->nga_nArguments = 0;
}

/**
 * Argument Data Manager: Initialize pointers
 */
static void
nglArgumentInitializePointer(ngiArgument_t *arg)
{
    /* Initialize the pointers */
    /* Do nothing */
}

/**
 * Argument: Copy the argument data.
 */
int
ngiArgumentDataCopy(ngiArgument_t *arg, ngLog_t *log, int *error)
{
    int result;
    int i;
    static const char fName[] = "ngiArgumentDataCopy";

    /* Check the arguments */
    assert(arg != NULL);

    for (i = 0; i < arg->nga_nArguments; i++) {
        result = nglArgumentElementArgumentDataCopy(
            &arg->nga_argument[i], log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't copy the argument data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Argument: Release the argument data.
 */
static int
nglArgumentDataRelease(ngiArgument_t *arg, ngLog_t *log, int *error)
{
    int result;
    int i;
    static const char fName[] = "nglArgumentDataRelease";

    /* Check the arguments */
    assert(arg != NULL);

    for (i = 0; i < arg->nga_nArguments; i++) {
        result = nglArgumentElementArgumentDataRelease(
            &arg->nga_argument[i], log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't release the argument data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Scalar Variable: Get the scalar variable of integer.
 */
static int
nglArgumentGetValueInt(
    ngiArgument_t *arg,
    ngiArgument_t *referArg,
    int position,
    int *value,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "nglArgumentGetValueInt";

    /* Check the arguments */
    assert(arg != NULL);
    assert(referArg != NULL);
    assert(position >= 0);
    assert(position < referArg->nga_nArguments);
    assert(value != NULL);

    /* Get the value */
    switch (referArg->nga_argument[position].ngae_dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
	*value = referArg->nga_argument[position].ngae_data.ngad_char;
	break;

    case NG_ARGUMENT_DATA_TYPE_SHORT:
	*value = referArg->nga_argument[position].ngae_data.ngad_short;
	break;

    case NG_ARGUMENT_DATA_TYPE_INT:
	*value = referArg->nga_argument[position].ngae_data.ngad_int;
	break;

    case NG_ARGUMENT_DATA_TYPE_LONG:
	*value = referArg->nga_argument[position].ngae_data.ngad_long;
	break;

    default:
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Data type %d is not supported.\n",
            fName, referArg->nga_argument[position].ngae_dataType);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Argument: Allocate data storage.
 */
int
ngiArgumentAllocateDataStorage(
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    static const char fName[] = "ngiArgumentAllocateDataStorage";

    for (i = 0; i < arg->nga_nArguments; i++) {
	if (arg->nga_argument[i].ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME)
	    continue;

	if ((arg->nga_argument[i].ngae_ioMode != NG_ARGUMENT_IO_MODE_OUT) &&
	    (arg->nga_argument[i].ngae_ioMode != NG_ARGUMENT_IO_MODE_WORK))
	    continue;

	/* No elements? */
	if (arg->nga_argument[i].ngae_nElements <= 0)
	    continue;

	/* Allocate */
	result = ngiArgumentElementAllocateDataStorage(
	    &arg->nga_argument[i],
	    arg->nga_argument[i].ngae_nElements,
            arg->nga_argument[i].ngae_nativeDataNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Argument Data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Argument: Initialize Subscript Value
 */
int
ngiArgumentInitializeSubscriptValue(
    ngiArgument_t *arg,
    ngiArgument_t *referArg,
    ngArgumentInformation_t *argInfo,
    ngLog_t *log,
    int *error)
{
    int i;
    ngiSubscriptValue_t *subscript;
    static const char fName[] = "ngiArgumentInitializeSubscriptValue";

    for (i = 0; i < arg->nga_nArguments; i++) {
	if (argInfo[i].ngai_nDimensions == 0) {
	    arg->nga_argument[i].ngae_nElements = 1;
	    continue;
	}

	/* Construct the Subscript Value */
	subscript = ngiSubscriptValueConstruct(
	    argInfo[i].ngai_subscript, argInfo[i].ngai_nDimensions,
	    arg, referArg, log, error);
	if (subscript == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't construct the Subscript Value.\n", fName);
	    return 0;
	}
	arg->nga_argument[i].ngae_subscript = subscript;

	/* Get the number of elements */
	arg->nga_argument[i].ngae_nElements =
	    subscript[argInfo[i].ngai_nDimensions - 1].ngsv_totalSize;
    }

    /* Success */
    return 1;
}

/**
 * Argument: Finalize Subscript Value
 */
int
ngiArgumentFinalizeSubscriptValue(
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    static const char fName[] = "ngiArgumentFinalizeSubscriptValue";

    for (i = 0; i < arg->nga_nArguments; i++) {
	/* Construct the Subscript Value */
	result = ngiSubscriptValueDestruct(
	    arg->nga_argument[i].ngae_subscript, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destruct the Subscript Value.\n", fName);
	    return 0;
	}
	arg->nga_argument[i].ngae_subscript = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Argument: Check Subscript Value
 */
int
ngiArgumentCheckSubscriptValue(
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    int i, j;
    static const char fName[] = "ngiArgumentCheckSubscriptValue";

    for (i = 0; i < arg->nga_nArguments; i++) {
	if (arg->nga_argument[i].ngae_nDimensions == 0) {
	    continue;
	}

        for (j = 0; j < arg->nga_argument[i].ngae_nDimensions; j++) {
            /* Check the value of subscript */
            if (arg->nga_argument[i].ngae_subscript[j].ngsv_end == 0) {
                arg->nga_argument[i].ngae_subscript[j].ngsv_end = 
                    arg->nga_argument[i].ngae_subscript[j].ngsv_size;
            }

            if (arg->nga_argument[i].ngae_subscript[j].ngsv_skip == 0) {
                arg->nga_argument[i].ngae_subscript[j].ngsv_skip = 1;
            }

            /* Check the value of subscript */
            if ((arg->nga_argument[i].ngae_subscript[j].ngsv_start >
                arg->nga_argument[i].ngae_subscript[j].ngsv_size)) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: The value of start of subscript is larger than size.(start = %d, size = %d)\n",
                    fName, arg->nga_argument[i].ngae_subscript[j].ngsv_start,
                    arg->nga_argument[i].ngae_subscript[j].ngsv_size);
                return 0;
            } else if (arg->nga_argument[i].ngae_subscript[j].ngsv_start >
                arg->nga_argument[i].ngae_subscript[j].ngsv_end) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: The value of start of subscript is larger than end.(start = %d, end = %d)\n",
                    fName, arg->nga_argument[i].ngae_subscript[j].ngsv_start,
                    arg->nga_argument[i].ngae_subscript[j].ngsv_end);
                return 0;
            } else if (arg->nga_argument[i].ngae_subscript[j].ngsv_end >
                arg->nga_argument[i].ngae_subscript[j].ngsv_size) {
                NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: The value of end of subscript is larger than size.(end = %d, size = %d)\n",
                    fName, arg->nga_argument[i].ngae_subscript[j].ngsv_end,
                    arg->nga_argument[i].ngae_subscript[j].ngsv_size);
                return 0;
            }
        }
    }

    /* Success */
    return 1;
}

#if 0
/* Note:
 * Nobody is using this function. However, it leaves the definition for
 * compatibility with other functions.
 */
/**
 * Argument Element: Construct
 */
static ngiArgumentElement_t *
nglArgumentElementConstruct(
    ngArgumentInformation_t *argInfo,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiArgumentElement_t *argElement;
    static const char fName[] = "nglArgumentElementConstruct";

    /* Check the arguments */
    assert(argInfo != NULL);
    assert(nArguments >= 0);

    /* Allocate */
    argElement = nglArgumentElementAllocate(nArguments, log, error);
    if (argElement == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument Element.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = nglArgumentElementInitialize(argElement, argInfo, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Argument Element.\n", fName);
	goto error;
    }

    /* Success */
    return argElement;

    /* Error occurred */
error:
    result = nglArgumentElementFree(argElement, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument Element.\n", fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}
#endif

#if 0
/* Note:
 * Nobody is using this function. However, it leaves the definition for
 * compatibility with other functions.
 */
/**
 * Argument Element: Destruct
 */
static int
nglArgumentElementDestruct(
    ngiArgumentElement_t *argElement,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglArgumentElementDestruct";

    /* Check the arguments */
    assert(argElement != NULL);

    /* Finalize */
    result = nglArgumentElementFinalize(argElement, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Argument Element.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = nglArgumentElementFree(argElement, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument Element.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}
#endif

#if 0
/* Note:
 * Nobody is using this function. However, it leaves the definition for
 * compatibility with other functions.
 */
/**
 * Argument Element: Allocate
 */
static ngiArgumentElement_t *
nglArgumentElementAllocate(int nArguments, ngLog_t *log, int *error)
{
    ngiArgumentElement_t *argElement;
    static const char fName[] = "nglArgumentElementAllocate";

    /* Check the arguments */
    assert(nArguments > 0);

    /* Allocate */
    argElement = globus_libc_calloc(nArguments, sizeof (ngiArgumentElement_t));
    if (argElement == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument Element.\n", fName);
	return NULL;
    }

    /* Success */
    return argElement;
}
#endif

#if 0
/* Note:
 * Nobody is using this function. However, it leaves the definition for
 * compatibility with other functions.
 */
/**
 * Argument Element: Free
 */
static int
nglArgumentElementFree(
    ngiArgumentElement_t *argElement,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(argElement != NULL);

    /* Deallocate */
    globus_libc_free(argElement);

    /* Success */
    return 1;
}
#endif

/**
 * Argument Element: Initialize
 */
static int
nglArgumentElementInitialize(
    ngiArgumentElement_t *argElement,
    ngArgumentInformation_t *argInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t nBytes;
    static const char fName[] = "nglArgumentElementInitialize";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(argInfo != NULL);

    /* Initialize the members */
    nglArgumentElementInitializeMember(argElement);

    argElement->ngae_dataType = argInfo->ngai_dataType;
    argElement->ngae_ioMode = argInfo->ngai_ioMode;
    argElement->ngae_nDimensions = argInfo->ngai_nDimensions;
    argElement->ngae_typeOfDivision = NGI_TYPE_OF_DIVISION_NONE;

    /* Get the size of one element */
    result = ngiGetSizeofData(argInfo->ngai_dataType, &nBytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the number of bytes of data.\n", fName);
        return 0;
    }
    argElement->ngae_nativeDataNbytes = nBytes;

    /* Success */
    return 1;
}

/**
 * Argument Element: Initialize Varg
 */
int
ngiArgumentElementInitializeVarg(
    ngiArgument_t *arg,
    int nArgs,
    va_list ap,
    ngiArgumentDelivery_t argDelivery,
    ngLog_t *log,
    int *error)
{
    int i, result;
    static const char fName[] = "ngiArgumentElementInitializeVarg";

    for (i = 0; i < nArgs; i++) {
	if (arg->nga_argument[i].ngae_ioMode == NG_ARGUMENT_IO_MODE_WORK) {
	    (void)va_arg(ap, void *);
	    continue;
	}

	/* Initialize */
	arg->nga_argument[i].ngae_data.ngad_long = 0;
	arg->nga_argument[i].ngae_pointer.ngap_void = NULL;

	if (arg->nga_argument[i].ngae_dataType == NG_ARGUMENT_DATA_TYPE_STRING) {
	    if (arg->nga_argument[i].ngae_nDimensions <= 0) {
		arg->nga_argument[i].ngae_pointer.ngap_string =
		    va_arg(ap, char *);
	    } else {
		arg->nga_argument[i].ngae_pointer.ngap_stringArray =
		    va_arg(ap, char **);
	    }

	} else if (arg->nga_argument[i].ngae_dataType
            == NG_ARGUMENT_DATA_TYPE_FILENAME) {

	    if (arg->nga_argument[i].ngae_nDimensions <= 0) {
		arg->nga_argument[i].ngae_pointer.ngap_fileName =
		    va_arg(ap, char *);
	    } else {
		arg->nga_argument[i].ngae_pointer.ngap_fileNameArray =
		    va_arg(ap, char **);
	    }
	} else if ((argDelivery == NGI_ARGUMENT_DELIVERY_C) &&
		   (arg->nga_argument[i].ngae_nDimensions <= 0)) {
	    switch (arg->nga_argument[i].ngae_dataType) {
	    case NG_ARGUMENT_DATA_TYPE_CHAR:
		arg->nga_argument[i].ngae_data.ngad_char = va_arg(ap, int);
		arg->nga_argument[i].ngae_pointer.ngap_char =
		    &arg->nga_argument[i].ngae_data.ngad_char;
		break;

	    case NG_ARGUMENT_DATA_TYPE_SHORT:
		arg->nga_argument[i].ngae_data.ngad_short = va_arg(ap, int);
		arg->nga_argument[i].ngae_pointer.ngap_short =
		    &arg->nga_argument[i].ngae_data.ngad_short;
		break;

	    case NG_ARGUMENT_DATA_TYPE_INT:
		arg->nga_argument[i].ngae_data.ngad_int = va_arg(ap, int);
		arg->nga_argument[i].ngae_pointer.ngap_int =
		    &arg->nga_argument[i].ngae_data.ngad_int;
		break;

	    case NG_ARGUMENT_DATA_TYPE_LONG:
		arg->nga_argument[i].ngae_data.ngad_long = va_arg(ap, long);
		arg->nga_argument[i].ngae_pointer.ngap_long =
		    &arg->nga_argument[i].ngae_data.ngad_long;
		break;

	    case NG_ARGUMENT_DATA_TYPE_FLOAT:
		arg->nga_argument[i].ngae_data.ngad_float = va_arg(ap, double);
		arg->nga_argument[i].ngae_pointer.ngap_float =
		    &arg->nga_argument[i].ngae_data.ngad_float;
		break;

	    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
		arg->nga_argument[i].ngae_data.ngad_double = va_arg(ap, double);
		arg->nga_argument[i].ngae_pointer.ngap_double =
		    &arg->nga_argument[i].ngae_data.ngad_double;
		break;

	    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
		arg->nga_argument[i].ngae_data.ngad_scomplex =
		    va_arg(ap, scomplex);
		arg->nga_argument[i].ngae_pointer.ngap_scomplex =
		    &arg->nga_argument[i].ngae_data.ngad_scomplex;
		break;

	    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
		arg->nga_argument[i].ngae_data.ngad_dcomplex =
		    va_arg(ap, dcomplex);
		arg->nga_argument[i].ngae_pointer.ngap_dcomplex =
		    &arg->nga_argument[i].ngae_data.ngad_dcomplex;
		break;

	    case NG_ARGUMENT_DATA_TYPE_CALLBACK:
		arg->nga_argument[i].ngae_pointer.ngap_function =
#ifndef NGI_NO_VA_ARG_FUNCTION_PTR
		    va_arg(ap, void(*)());
#else /* NGI_NO_VA_ARG_FUNCTION_PTR */
		    (void (*)())va_arg(ap, void *);
#endif /* NGI_NO_VA_ARG_FUNCTION_PTR */
		break;

	    default:
		NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Unknown data type %d.\n",
		    fName, arg->nga_argument[i].ngae_dataType);
		return 0;
	    }

	    /* Is argument NULL? */
	    switch (arg->nga_argument[i].ngae_dataType) {
	    case NG_ARGUMENT_DATA_TYPE_CALLBACK:
		break;
	    default:
		assert(arg->nga_argument[i].ngae_pointer.ngap_void != NULL);
		break;
	    }
	 } else {
	    /* Argument is array */
	    switch (arg->nga_argument[i].ngae_dataType) {
	    case NG_ARGUMENT_DATA_TYPE_CHAR:
		arg->nga_argument[i].ngae_pointer.ngap_char =
		    va_arg(ap, char *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_SHORT:
		arg->nga_argument[i].ngae_pointer.ngap_short =
		    va_arg(ap, short *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_INT:
		arg->nga_argument[i].ngae_pointer.ngap_int = va_arg(ap, int *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_LONG:
		arg->nga_argument[i].ngae_pointer.ngap_long =
		    va_arg(ap, long *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_FLOAT:
		arg->nga_argument[i].ngae_pointer.ngap_float =
		    va_arg(ap, float *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
		arg->nga_argument[i].ngae_pointer.ngap_double =
		    va_arg(ap, double *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
		arg->nga_argument[i].ngae_pointer.ngap_scomplex =
		    va_arg(ap, scomplex *);
		break;

	    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
		arg->nga_argument[i].ngae_pointer.ngap_dcomplex =
		    va_arg(ap, dcomplex *);
		break;

	    default:
		NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Unknown data type %d.\n",
		    fName, arg->nga_argument[i].ngae_dataType);
		return 0;
	    }
	}

	/* Initialize the Argument */
	result = ngiArgumentElementInitializeData(
	    &arg->nga_argument[i], arg->nga_argument[i].ngae_pointer,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Argument Element.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Argument Element: Initialize argument stack
 */
int
ngiArgumentElementInitializeStack(
    ngiArgument_t *arg,
    int nArgs,
    ngiArgumentStack_t *argStack,
    ngLog_t *log,
    int *error)
{
    int i, result;
    static const char fName[] = "ngiArgumentElementInitializeStack";

    /* Check the argument stack */
    if (argStack->ngas_nargs < nArgs) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Too few arguments (%d < %d).\n",
	    fName, argStack->ngas_nargs, nArgs);
	return 0;
    }

    for (i = 0; i < nArgs; i++) {
	if (arg->nga_argument[i].ngae_ioMode == NG_ARGUMENT_IO_MODE_WORK) {
	    continue;
	}

	/* Initialize */
	arg->nga_argument[i].ngae_data.ngad_long = 0;
	arg->nga_argument[i].ngae_pointer.ngap_void = argStack->ngas_argp[i];

	/* Initialize the Argument */
	result = ngiArgumentElementInitializeData(
	    &arg->nga_argument[i], arg->nga_argument[i].ngae_pointer,
	    log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't initialize the Argument Element.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Argument Element: Initialize Data
 */
int
ngiArgumentElementInitializeData(
    ngiArgumentElement_t *argElement,
    ngiArgumentPointer_t argPointer,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiArgumentPointer_t argP;
    static const char fName[] = "ngiArgumentElementInitializeData";

    if (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_WORK) {
	/* Success */
	return 1;
    }

    /* Save the Argument Pointer */
    argElement->ngae_pointer = argPointer;
    if ((argPointer.ngap_void == NULL) &&
	((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_STRING) ||
         (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME))) {
	if (argElement->ngae_nDimensions == 0) {
	    argElement->ngae_nElements = 1;
	} else {
	    argElement->ngae_nElements = 0;
	}
	return 1;
    }

    /* Get the argument data */
    argP.ngap_void = &argElement->ngae_data;
    result = ngiGetArgumentData(argElement->ngae_dataType,
	argPointer, argP, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the argument data and pointer.\n", fName);
	return 0;
    }

    /* Is argument type scalar? */
    if (argElement->ngae_nDimensions == 0) {
	argElement->ngae_nElements = 1;
    } else {
	/* Is argument valid? */
	switch (argElement->ngae_dataType) {
	case NG_ARGUMENT_DATA_TYPE_CALLBACK:
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: This data type %d could not use array.\n",
		fName, argElement->ngae_dataType);
	    return 0;

	default:
	    /* Do nothing */
	    break;
	}
    }

    /* Success */
    return 1;
}

/**
 * Argument Element: Finalize
 */
static int
nglArgumentElementFinalize(
    ngiArgumentElement_t *argElement,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglArgumentElementFinalize";

    /* Check the arguments */
    assert(argElement != NULL);

    /* Deallocate the storage for data */
    result = ngiArgumentElementFreeDataStorage(argElement, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for data.\n", fName);
	return 0;
    }

    /* Destruct the Subscript Value */
    if (argElement->ngae_subscript != NULL) {
	result = ngiSubscriptValueDestruct(
	    argElement->ngae_subscript, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't construct the Subscript Value.\n", fName);
	    return 0;
	}
	argElement->ngae_subscript = NULL;
    }

    /* Destruct the tmpFileNameTable */
    if (argElement->ngae_tmpFileNameTable != NULL) {
        globus_libc_free(argElement->ngae_tmpFileNameTable);
        argElement->ngae_tmpFileNameTable = NULL;
    }

    /* Initialize */
    nglArgumentElementInitializeMember(argElement);

    /* Success */
    return 1;
}

/**
 * Argument Element: Initialize the members.
 */
static void
nglArgumentElementInitializeMember(ngiArgumentElement_t *argElement)
{
    /* Check the arguments */
    assert(argElement != NULL);

    /* Initialize the pointers */
    nglArgumentElementInitializePointer(argElement);

    /* Initialize the members */
    argElement->ngae_ioMode = NG_ARGUMENT_IO_MODE_NONE;
    argElement->ngae_dataType = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
    argElement->ngae_nativeDataNbytes = 0;
    argElement->ngae_networkDataNbytes = 0;
    argElement->ngae_nElements = 0;
    argElement->ngae_nDimensions = 0;
    argElement->ngae_networkTotalNbytes = 0;
    argElement->ngae_networkTotalNbytesIsVeryLarge = 0;
    argElement->ngae_data.ngad_long = 0;
    argElement->ngae_malloced = 0;
    argElement->ngae_dataCopied = 0;
    argElement->ngae_typeOfDivision = 0;

    argElement->ngae_fileNumber = 0;
    argElement->ngae_readPaddingNbytes = 0;
}

/**
 * Argument Element: Initialize the pointers.
 */
static void
nglArgumentElementInitializePointer(ngiArgumentElement_t *argElement)
{
    /* Check the arguments */
    assert(argElement != NULL);

    /* Initialize the pointers */
    argElement->ngae_pointer.ngap_void = NULL;
    argElement->ngae_subscript = NULL;
    argElement->ngae_sMngRemain = NULL;
    argElement->ngae_tmpFileNameTable = NULL;
}

/**
 * Argument Element: Copy the argument data.
 */
static int
nglArgumentElementArgumentDataCopy(
    ngiArgumentElement_t *argElement,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    size_t nBytes;
    ngiArgumentPointer_t argp;
    static const char fName[] = "nglArgumentElementArgumentDataCopy";

    /* Check the argument */
    assert(argElement != NULL);

    /* Initialize the local variable */
    argp.ngap_void = NULL;

    if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME)
        goto copy;

    if (argElement->ngae_ioMode != NG_ARGUMENT_IO_MODE_IN)
        return 1;

    if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK)
        return 1;

    /* Scalar variable is not copy */
    if (argElement->ngae_nDimensions == 0) {
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_STRING:
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            break;

        default:
            return 1;
        }
    }

    /* Copy the argument data */
copy:
    switch (argElement->ngae_dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
    case NG_ARGUMENT_DATA_TYPE_SHORT:
    case NG_ARGUMENT_DATA_TYPE_INT:
    case NG_ARGUMENT_DATA_TYPE_LONG:
    case NG_ARGUMENT_DATA_TYPE_FLOAT:
    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
        /* Check the arguments */
        if (argElement->ngae_nElements <= 0)
	    goto success;

        /* Get the sizeof data */
        result = ngiGetSizeofData(
            argElement->ngae_dataType, &nBytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get the size of data.\n", fName);
            return 0;
        }

        /* Allocate the memory */
	argp.ngap_void = globus_libc_calloc(
            argElement->ngae_nElements, nBytes);
        if (argp.ngap_void == NULL)
            goto cantAllocate;

        /* Copy the argument data */
        memcpy(argp.ngap_void, argElement->ngae_pointer.ngap_void,
               nBytes * argElement->ngae_nElements);
	break;

    case NG_ARGUMENT_DATA_TYPE_STRING:
        if (argElement->ngae_nDimensions <= 0) {
            /* Check the argument */
            if (argElement->ngae_pointer.ngap_string == NULL)
		goto success;

            /* Allocate the memory */
            argp.ngap_string = globus_libc_calloc(
                1, strlen(argElement->ngae_pointer.ngap_string) + 1);
            if (argp.ngap_string == NULL)
                goto cantAllocate;

            /* Copy the argument data */
            strcpy(argp.ngap_string, argElement->ngae_pointer.ngap_string);
        } else {      
            /* Check the arguments */
            if ((argElement->ngae_pointer.ngap_stringArray == NULL) ||
                (argElement->ngae_nElements <= 0))
		goto success;

            /* Allocate the memory */
            argp.ngap_stringArray = globus_libc_calloc(
                argElement->ngae_nElements, sizeof (char *));
            if (argp.ngap_stringArray == NULL)
                goto cantAllocate;
 
            for (i = 0; i < argElement->ngae_nElements; i++) {
                argp.ngap_stringArray[i] = NULL;

                /* Check the arguments */
                if (argElement->ngae_pointer.ngap_stringArray[i] == NULL)
		    continue;

                /* Allocate the memory */
                argp.ngap_stringArray[i] = globus_libc_calloc(1,
                    strlen(argElement->ngae_pointer.ngap_stringArray[i]) + 1);
                if (argp.ngap_stringArray == NULL)
                    goto cantAllocate;

                /* Copy the argument data */
                strcpy(argp.ngap_stringArray[i],
                       argElement->ngae_pointer.ngap_stringArray[i]);
            }
        }
        break;

    case NG_ARGUMENT_DATA_TYPE_FILENAME:
        if (argElement->ngae_nDimensions <= 0) {
            /* Check the argument */
            if (argElement->ngae_pointer.ngap_fileName == NULL)
		goto success;

            /* Allocate the memory */
            argp.ngap_string = globus_libc_calloc(
                1, strlen(argElement->ngae_pointer.ngap_fileName) + 1);
            if (argp.ngap_fileName == NULL)
                goto cantAllocate;

            /* Copy the argument data */
            strcpy(argp.ngap_fileName, argElement->ngae_pointer.ngap_fileName);
        } else {      
            /* Check the arguments */
            if ((argElement->ngae_pointer.ngap_fileNameArray == NULL) ||
                (argElement->ngae_nElements <= 0))
		goto success;

            /* Allocate the memory */
            argp.ngap_fileNameArray = globus_libc_calloc(
                argElement->ngae_nElements, sizeof (char *));
            if (argp.ngap_fileNameArray == NULL)
                goto cantAllocate;
 
            for (i = 0; i < argElement->ngae_nElements; i++) {
                argp.ngap_fileNameArray[i] = NULL;

                /* Check the arguments */
                if (argElement->ngae_pointer.ngap_fileNameArray[i] == NULL)
		    continue;

                /* Allocate the memory */
                argp.ngap_fileNameArray[i] = globus_libc_calloc(1,
                    strlen(argElement->ngae_pointer.ngap_fileNameArray[i]) + 1);
                if (argp.ngap_fileNameArray == NULL)
                    goto cantAllocate;

                /* Copy the argument data */
                strcpy(argp.ngap_fileNameArray[i],
                       argElement->ngae_pointer.ngap_fileNameArray[i]);
            }
        }
        break;

    default:
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Unknown data type %d.\n",
                fName, argElement->ngae_dataType);
	return 0;
    }

    /* Replace the argument pointer */
    argElement->ngae_dataCopied = 1;
success:
    argElement->ngae_pointer = argp;

    /* Success */
    return 1;

    /* Can't allocate the storage */
cantAllocate:
    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't allocate the storage for argument data.\n", fName);

    /* Deallocate the array of string */
    if ((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_STRING) &&
        (argp.ngap_stringArray != NULL)) {
        for (i = 0; i < argElement->ngae_nElements; i++) {
	    if (argp.ngap_stringArray[i] != NULL) {
		globus_libc_free(argp.ngap_stringArray[i]);
		argp.ngap_stringArray[i] = NULL;
	    }
        }
    }

    /* Deallocate the array of fileName */
    if ((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) &&
        (argp.ngap_fileNameArray != NULL)) {
        for (i = 0; i < argElement->ngae_nElements; i++) {
	    if (argp.ngap_fileNameArray[i] != NULL) {
		globus_libc_free(argp.ngap_fileNameArray[i]);
		argp.ngap_fileNameArray[i] = NULL;
	    }
        }
    }

    /* Deallocate */
    if (argp.ngap_void != NULL) {
        globus_libc_free(argp.ngap_void);
        argp.ngap_void = NULL;
    }

    /* Failed */
    return 0;
}

/**
 * Argument Element: Release the argument data.
 */
static int
nglArgumentElementArgumentDataRelease(
    ngiArgumentElement_t *argElement,
    ngLog_t *log,
    int *error)
{
    int i;

    /* Check the argument */
    assert(argElement != NULL);

    /* Not allocated? */
    if (argElement->ngae_dataCopied == 0)
        return 1;
    argElement->ngae_dataCopied = 0;

    /* Deallocate the array of string */
    if ((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_STRING) &&
        (argElement->ngae_nDimensions > 0) &&
        (argElement->ngae_pointer.ngap_stringArray != NULL)) {
        for (i = 0; i < argElement->ngae_nElements; i++) {
            if (argElement->ngae_pointer.ngap_stringArray[i] == NULL)
                continue;
            globus_libc_free(argElement->ngae_pointer.ngap_stringArray[i]);
            argElement->ngae_pointer.ngap_stringArray[i] = NULL;
        }
    }

    /* Deallocate the array of fileName */
    if ((argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) &&
        (argElement->ngae_nDimensions > 0) &&
        (argElement->ngae_pointer.ngap_fileNameArray != NULL)) {
        for (i = 0; i < argElement->ngae_nElements; i++) {
            if (argElement->ngae_pointer.ngap_fileNameArray[i] == NULL)
                continue;
            globus_libc_free(argElement->ngae_pointer.ngap_fileNameArray[i]);
            argElement->ngae_pointer.ngap_fileNameArray[i] = NULL;
        }
    }

    /* Deallocate */
    if (argElement->ngae_pointer.ngap_void != NULL) {
        globus_libc_free(argElement->ngae_pointer.ngap_void);
        argElement->ngae_pointer.ngap_void = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Argument Element: Allocate data storage.
 */
int
ngiArgumentElementAllocateDataStorage(
    ngiArgumentElement_t *argElement,
    int nElements,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    void *data;
    static const char fName[] = "ngiArgumentElementAllocateDataStorage";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(argElement->ngae_malloced == 0);
    assert(argElement->ngae_pointer.ngap_void == NULL);

    /* Allocate */
    if (nElements > 0) {
        data = globus_libc_malloc(nBytes * nElements);
        if (data == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Argument Data.\n", fName);
            return 0;
        }
        argElement->ngae_malloced = 1;
        argElement->ngae_pointer.ngap_void = data;
    }

    /* Success */
    return 1;
}

/**
 * Argument Element: Deallocate data storage.
 */
int
ngiArgumentElementFreeDataStorage(
    ngiArgumentElement_t *argElement,
    ngLog_t *log,
    int *error)
{
    int i;
    char **stringArray, **fileNameArray;

    /* Check the arguments */
    assert(argElement != NULL);

    /* Deallocate */
    if (argElement->ngae_malloced != 0) {
	assert(argElement->ngae_pointer.ngap_void != NULL);

        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_STRING:
            if (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_IN) {
                stringArray = argElement->ngae_pointer.ngap_stringArray;
                for (i = 0; i < argElement->ngae_nElements; i++) {
                    if (stringArray[i] != NULL) {
                        globus_libc_free(stringArray[i]);
                        stringArray[i] = NULL;
                    }
                }
            }
            break;

        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            if (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_IN) {
                fileNameArray = argElement->ngae_pointer.ngap_fileNameArray;
                for (i = 0; i < argElement->ngae_nElements; i++) {
                    if (fileNameArray[i] != NULL) {
                        globus_libc_free(fileNameArray[i]);
                        fileNameArray[i] = NULL;
                    }
                }
            }
            break;

        default:
	    /* Do nothing */
            break;
        }

	globus_libc_free(argElement->ngae_pointer.ngap_void);
	argElement->ngae_malloced = 0;
	argElement->ngae_pointer.ngap_void = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Encode the argument
 */
int
ngiArgumentEncode(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *protocol,
    ngiStreamManager_t **sMng,
    int when,
    ngLog_t *log,
    int *error)
{
    ngiStreamManager_t *sMngArg = NULL;
    long protocolVersion;
    int isTooLarge;
    size_t totalNetworkNbytes;
    int result;
    static const char fName[] = "ngiArgumentEncode";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* First Call? */
    if (argElement->ngae_typeOfDivision == NGI_TYPE_OF_DIVISION_NONE) {
        /* Get the Version Number of partner's Protocol */
        result = ngiProtocolGetProtocolVersionOfPartner(
            protocol, &protocolVersion, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get the version number of partner's protocol.\n",
                fName);
            goto error;
        }

        /* Construct the Stream Manager */
        sMngArg = ngiMemoryStreamManagerConstruct(
            NGI_PROTOCOL_STREAM_NBYTES, log, error);
        if (sMngArg == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't construct Stream Manager.\n", fName);
            goto error;
        }

        /* Is data type string or filename */
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            if (NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(
                    protocolVersion)) {
                result = nglArgumentEncodeFile(
                    argElement, protocol, sMngArg, when, log, error);
                break;
            }
            /* BREAKTHROUGH */
        case NG_ARGUMENT_DATA_TYPE_STRING:
            if (argElement->ngae_nDimensions == 0) {
                result = nglArgumentEncodeString(
                    argElement, protocol, sMngArg, log, error);
            } else {
                result = nglArgumentEncodeStringArray(
                    argElement, protocol, sMngArg, log, error);
            }
            break;

        default:
            result = nglArgumentEncodeData(
                argElement, protocol, sMngArg, log, error);
            break;
        }

        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't encode the argument data.\n", fName);
            goto error;
        }

        /* Set argument size */
        result = ngiStreamManagerGetTotalBytesOfReadableData(
                sMngArg, &isTooLarge, &totalNetworkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get number of bytes of readable data.\n", fName);
            return 0;
        }
        argElement->ngae_networkTotalNbytes = totalNetworkNbytes;
        argElement->ngae_networkTotalNbytesIsVeryLarge = isTooLarge;

        /* Append Conversion Header */
        result = nglArgumentEncodeAppendRawConversionMethod(
            argElement, protocol, &sMngArg, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append header to the argument data.\n", fName);
            goto error;
        }
    }

    /* Division */
    result = nglArgumentEncodeDivision(
        argElement, protocol, &sMngArg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't divide the argument data.\n", fName);
        goto error;
    }

    /* Compress */
    result = nglArgumentEncodeCompression(
        argElement,
	(compInfo == NULL) ? NULL : &compInfo->ngcic_compression,
	protocol, &sMngArg, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't compress the argument data.\n", fName);
        goto error;
    }

    *sMng = sMngArg;
    sMngArg = NULL;

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (sMngArg != NULL) {
        result = ngiStreamManagerDestruct(
            sMngArg, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngArg = NULL;
    }
    return 0;
}

static int
nglArgumentEncodeAppendRawConversionMethod(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t **sMng,
    ngLog_t *log,
    int *error)
{
    ngiStreamManager_t *sMngArg = NULL;
    ngiStreamManager_t *sMngHeader = NULL;
    size_t networkSizeofData, networkNbytes;
    size_t networkNbytesOfConversionType;
    int result;    
    long protocolVersion;
    long compressType;
    static const char fName[] = "nglArgumentEncodeAppendRawConversionMethod";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(*sMng != NULL);

    sMngArg = *sMng;

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    /* Is support the data conversion? */
    if (NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(protocolVersion)) {
        /* Create Argument Header */

	/* Construct the Stream Manager */
	sMngHeader = ngiMemoryStreamManagerConstruct(
	    NGI_PROTOCOL_STREAM_NBYTES, log, error);
	if (sMngHeader == NULL) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't construct Stream Manager.\n", fName);
	    return 0;
	}

	/* Write the Compress type to Stream Manager */
	compressType = NGI_BYTE_STREAM_CONVERSION_RAW;
	result = ngiProtocolBinary_WriteXDRdata(
	    protocol, sMngHeader, NG_ARGUMENT_DATA_TYPE_LONG, &compressType, 1,
	    &networkSizeofData, &networkNbytesOfConversionType, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		 NULL, "%s: Can't encode the argument data.\n", fName);
	    return 0;
	}

	/* Write the Data size to Stream Manager */
	result = ngiProtocolBinary_WriteXDRdata(
	    protocol, sMngHeader, NG_ARGUMENT_DATA_TYPE_LONG,
	    &argElement->ngae_networkTotalNbytes, 1,
	    &networkSizeofData, &networkNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
		 NULL, "%s: Can't encode the argument data.\n", fName);
	    return 0;
	}

	/* Append the Stream Manager */
	result = ngiStreamManagerAppend(
	    sMngHeader, sMngArg, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't append the Stream Manager.\n", fName);
	    return 0;
	}
	*sMng = sMngHeader;
    }

    return 1;
}

/**
 * Encode the argument: division
 */
static int
nglArgumentEncodeDivision(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t **sMng,
    ngLog_t *log,
    int *error)
{
    ngiStreamManager_t *smPartial;
    ngiStreamManager_t *smHeader;
    ngiStreamManager_t *smArg;
    size_t networkSizeofData;
    size_t networkNbytesOfConversionType;
    long conversionType;
    size_t blockSize;
    size_t partialSize;
    long networkSize;
    int result;
    int isTooLarge;/* Dummy */
    static const char fName[] = "nglArgumentEncodeDivision";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    blockSize = protocol->ngp_byteStreamConversion.ngbsc_argumentBlockSize;

    /* Divide argument? */
    if (blockSize <= 0) {
        /* Do nothing */
        return 1;
    }

    switch (argElement->ngae_typeOfDivision) {
    case NGI_TYPE_OF_DIVISION_NONE:
        /* First Call */
        assert(argElement->ngae_sMngRemain == NULL);
        assert(*sMng != NULL);
        
        argElement->ngae_sMngRemain = *sMng;
        argElement->ngae_typeOfDivision =
            NGI_TYPE_OF_DIVISION_CONTINUATION;
        break;

    case NGI_TYPE_OF_DIVISION_END:
        if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) {
            argElement->ngae_fileNumber++;
            if (argElement->ngae_fileNumber < argElement->ngae_nElements) {
                argElement->ngae_typeOfDivision =
                    NGI_TYPE_OF_DIVISION_CONTINUATION;
                break;
            }
        }
        /* Last Call */
        result = ngiStreamManagerDestruct(
            argElement->ngae_sMngRemain, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
            goto error;
        }
        argElement->ngae_sMngRemain = NULL;
        argElement->ngae_typeOfDivision = NGI_TYPE_OF_DIVISION_NONE;
        argElement->ngae_fileNumber = 0;

        /* Success */
        return 1;
        break;
    case NGI_TYPE_OF_DIVISION_CONTINUATION:
        /* Do Nothing */
        break;
    default:
        /* NOTREACHED */
        assert(0);
        break;
    }
    smArg =  argElement->ngae_sMngRemain;

    /* Destroy the Stream Manager read already. */
    result = ngiStreamManagerDestructReadAlready(&smArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't destruct the Stream Manager read already.\n",
            fName);
        goto error;
    }
    argElement->ngae_sMngRemain = smArg;

    /* Create the Partial Stream Manager */ 
    smPartial = ngiPartialStreamManagerConstruct(smArg, blockSize, log, error);
    if (smPartial == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Partial Stream Manager.\n", fName);
        goto error;
    }

    /* GetDataSize */
    result = ngiStreamManagerGetBytesOfReadableData(smPartial, &isTooLarge, &partialSize, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get number of bytes of readable data.\n", fName);
        goto error;
    }

    /* Create the Memory Stream Manager */
    smHeader = ngiMemoryStreamManagerConstruct(NGI_PROTOCOL_STREAM_NBYTES, log, error);
    if (smHeader == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Partial Stream Manager.\n", fName);
        goto error;
    }

    /* Write the Compress type to Stream Manager */
    conversionType = NGI_BYTE_STREAM_CONVERSION_DIVIDE;
    result = ngiProtocolBinary_WriteXDRdata(
        protocol, smHeader, NG_ARGUMENT_DATA_TYPE_LONG, &conversionType, 1,
        &networkSizeofData, &networkNbytesOfConversionType, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't encode the argument data.\n", fName);
        goto error;
    }

    /* Write the Type of Division */
    if (partialSize == 0) {
        argElement->ngae_typeOfDivision = NGI_TYPE_OF_DIVISION_END;
    }

    result = ngiProtocolBinary_WriteXDRdata(
        protocol, smHeader, NG_ARGUMENT_DATA_TYPE_LONG, &argElement->ngae_typeOfDivision, 1,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't encode the argument data.\n", fName);
        goto error;
    }

    /* Write the Data size to Stream Manager */
    networkSize = partialSize;
    result = ngiProtocolBinary_WriteXDRdata(
        protocol, smHeader, NG_ARGUMENT_DATA_TYPE_LONG,
        &networkSize, 1,
        NULL, NULL, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
             NULL, "%s: Can't encode the argument data.\n", fName);
        goto error;
    }

    /* Append the partial argument to the Header */
    result = ngiStreamManagerAppend(
        smHeader, smPartial, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't append the Stream Manager.\n", fName);
        goto error;
    }

    /* Append padding */
    result = nglArgumentForceAppendPaddingStreamManager(
        argElement, protocol, smHeader, partialSize, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the padding data.\n", fName);
        goto error;
    }
    
    *sMng = smHeader;

    return 1;

error:
    return 0;
}

/**
 * Encode the argument: Compression
 */
static int
nglArgumentEncodeCompression(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationElement_t *compInfo,
    ngiProtocol_t *protocol,
    ngiStreamManager_t **sMng,
    ngLog_t *log,
    int *error)
{
#ifndef NGI_NO_ZLIB
    ngiStreamManager_t *sMngOfZlib = NULL;
    ngiStreamManager_t *sMngOfZlibHeader = NULL;
    size_t networkNbytes, compressDataBytes;
    size_t totalNbytes = 0;
    long compressType;
    int result;    
    static const char fName[] = "nglArgumentEncodeCompression";
#endif /* !NGI_NO_ZLIB */

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

#ifndef NGI_NO_ZLIB
    if (*sMng == NULL) {
        /* Do nothing */
        return 1;
    }

    /* Check the Zlib compression*/
    if ((protocol->ngp_byteStreamConversion.ngbsc_zlib != 0) &&
	((argElement->ngae_networkTotalNbytesIsVeryLarge != 0) ||
         (argElement->ngae_networkTotalNbytes >=
	    protocol->ngp_byteStreamConversion.ngbsc_zlibThreshold))) {

        /* Construct the Stream Manager */
        sMngOfZlibHeader = ngiMemoryStreamManagerConstruct(
            NGI_PROTOCOL_STREAM_NBYTES, log, error);
        if (sMngOfZlibHeader == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct Stream Manager.\n", fName);
            goto error;
        }

        /* Write the Compress type to Stream Manager */
        compressType = NGI_BYTE_STREAM_CONVERSION_ZLIB;
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMngOfZlibHeader, NG_ARGUMENT_DATA_TYPE_LONG,
            &compressType, 1, NULL, &networkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode the argument data.\n", fName);
            goto error;
        }
        totalNbytes += networkNbytes;

        /* Construct the Stream Manager */
        sMngOfZlib = ngiMemoryStreamManagerConstruct(
            NGI_PROTOCOL_STREAM_NBYTES, log, error);
        if (sMngOfZlib == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct Stream Manager.\n", fName);
            goto error;
        }

        /* Data is compressed by zlib */
        result = nglArgumentEncodeZlib(*sMng, sMngOfZlib,
            &compressDataBytes, compInfo, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode of Zlib the argument data.\n", fName);
            goto error;
        }

        /* Destruct the Source Stream Manager */
        result = ngiStreamManagerDestruct(*sMng, log, error);
        *sMng = NULL;
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
            goto error;
        }

        /* Write the Data size to Stream Manager */
        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMngOfZlibHeader, NG_ARGUMENT_DATA_TYPE_LONG,
            &compressDataBytes, 1, NULL, &networkNbytes,
            log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode the argument data.\n", fName);
            goto error;
        }
        totalNbytes += networkNbytes;

        /* Append the Stream Manager */
        result = ngiStreamManagerAppend(
            sMngOfZlibHeader, sMngOfZlib, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append the Stream Manager.\n", fName);
            goto error;
        }
        sMngOfZlib = NULL;
        totalNbytes += compressDataBytes;

        /* Append the padding */
        result = nglArgumentForceAppendPadding(argElement, protocol, sMngOfZlibHeader,
            totalNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the padding data.\n", fName);
            goto error;
        }

        *sMng = sMngOfZlibHeader;
        sMngOfZlibHeader = NULL;
    }
#endif /* !NGI_NO_ZLIB */

    return 1;
#ifndef NGI_NO_ZLIB
    /* Error occurred */
error:
    if (sMngOfZlib != NULL) {
        result = ngiStreamManagerDestruct(sMngOfZlib, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngOfZlib = NULL;
    }

    if (sMngOfZlibHeader != NULL) {
        result = ngiStreamManagerDestruct(sMngOfZlibHeader, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngOfZlibHeader = NULL;
    }
    return 0;
#endif /* !NGI_NO_ZLIB */
}

/**
 * Encode the argument: Data
 */
static int
nglArgumentEncodeData(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;    
    void *data;
    int dataNelements;
    size_t networkSizeofData, networkNbytes;
    ngRemoteMethodInformation_t *rmInfo;
    void *skipData = NULL;
    int skipDataNelements;
    static const char fName[] = "nglArgumentEncodeData";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Initialize the local variables */
    data = argElement->ngae_pointer.ngap_void;
    dataNelements = argElement->ngae_nElements;
    argElement->ngae_networkDataNbytes = 0;
    argElement->ngae_networkTotalNbytes = 0;

    /* Is argument NULL or no element */
    if ((argElement->ngae_pointer.ngap_void == NULL) ||
	(argElement->ngae_nElements <= 0)) {
	/* Success */
	return 1;
    }

    /* Get the Remote Method Infomation */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't get the Remote Method Information.\n", fName);
        return 0;
    }

    /* Check the Shrink */
    if ((rmInfo->ngrmi_shrink == 1) && (argElement->ngae_subscript != NULL)) {
        /* Encode of skip */
        result = nglArgumentEncodeSkip(argElement, data, dataNelements,
            &skipData, &skipDataNelements, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode of skip the argument data.\n", fName);

            goto error;
        }

        assert(skipDataNelements >= 0);

        /* no element */
        if (skipDataNelements == 0) {
            assert(skipData == NULL);

            /* Success */
            return 1;
        }

        result = ngiProtocolBinary_WriteData(
            protocol, sMng, argElement->ngae_dataType, skipData,
            skipDataNelements, NGI_DATA_THROUGH_BUFFER,
	    &networkSizeofData, &networkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode the argument data of skip.\n", fName);
            goto error;
        }

        assert(skipData != NULL);
        globus_libc_free(skipData);
        skipData = NULL;
    } else {
        result = ngiProtocolBinary_WriteData(
            protocol, sMng, argElement->ngae_dataType, data, dataNelements,
	    NGI_DATA_DIRECTLY, &networkSizeofData, &networkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode the argument data.\n", fName);
            goto error;
        }
    }

    argElement->ngae_networkDataNbytes = networkSizeofData;
    argElement->ngae_networkTotalNbytes = networkNbytes;

    /* Append Padding */
    result = nglArgumentAppendPadding(argElement, protocol, sMng,
        networkNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the padding data.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (skipData != NULL) {
        globus_libc_free(skipData);
        skipData = NULL;
    }

    return 0;
}

/**
 * Encode the argument: String
 */
static int
nglArgumentEncodeString(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;    
    char *string;
    long strNbytes;
    size_t networkNbytes;
    static const char fName[] = "nglArgumentEncodeString";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Initialize the local variables */
    string = argElement->ngae_pointer.ngap_char;
    if (string == NULL) {
	strNbytes = 0;
    } else {
	strNbytes = strlen(string);
	if (strNbytes < 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: String length (%d) is less than zero.\n",
		fName, strNbytes);
	    return 0;
	}
    }

    /* Write the string */
    result = ngiProtocolBinary_WriteString(
        protocol, sMng, string, strNbytes, NGI_PROTOCOL_CONTAIN_LENGTH,
        &networkNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't encode the string.\n", fName);
        return 0;
    }
    argElement->ngae_networkTotalNbytes = networkNbytes;

    /* Append the padding */
    result = nglArgumentAppendPadding(argElement, protocol, sMng,
        networkNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the padding data.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Encode the argument: String array
 */
static int
nglArgumentEncodeStringArray(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngLog_t *log,
    int *error)
{
    int result;    
    int i;
    long strNbytes;
    char **stringArray;
    size_t networkNbytes;
    static const char fName[] = "nglArgumentEncodeStringArray";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Is array NULL? */
    if (argElement->ngae_pointer.ngap_void == NULL) {
	/* Success */
	return 1;
    }

    /* Is data string array? */
    if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_STRING) {
        stringArray = argElement->ngae_pointer.ngap_stringArray;
    } else if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) {
        stringArray = argElement->ngae_pointer.ngap_fileNameArray;
    } else {
        abort();
    }

    for (i = 0; i < argElement->ngae_nElements; i++) {
	if (stringArray[i] == NULL) {
	    strNbytes = 0;
	} else {
	    strNbytes = strlen(stringArray[i]);
	    if (strNbytes < 0) {
		NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
		ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: String length is 0.\n", fName);
		return 0;
	    }
	}

        /* Write the string */
        result = ngiProtocolBinary_WriteString(
            protocol, sMng, stringArray[i], strNbytes,
            NGI_PROTOCOL_CONTAIN_LENGTH, &networkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't encode the string array.\n", fName);
            return 0;
        }
        argElement->ngae_networkTotalNbytes += networkNbytes;

        /* Append the padding */
        result = nglArgumentAppendPadding(argElement, protocol, sMng,
            networkNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the padding data.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Encode the argument: String array
 */
static int
nglArgumentEncodeFile(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    int when,
    ngLog_t *log,
    int *error)
{
    int result;    
    int i;
    int isTooLarge;
    char **fileNameArray;
    ngiStreamManager_t *sMngFile      = NULL;
    ngiStreamManager_t *sMngDelimiter = NULL;
    ngiStreamManager_t *sMngHeader    = NULL;
    size_t networkNbytes;
    size_t fileNbytes;
    long fileHeader[NGI_PROTOCOL_FILE_TRANSFER_HEADER_SIZE];
    int index;
    int dataIsSent = 0;
    size_t headerOfLength;
    ngiStreamManager_t *sMngTail = NULL;
    static const char fName[] = "nglArgumentEncodeFile";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);
    assert(argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME);

    if (argElement->ngae_nDimensions == 0) {
        assert(argElement->ngae_nElements == 1);
        fileNameArray = &argElement->ngae_pointer.ngap_fileName;
    } else {
        fileNameArray = argElement->ngae_pointer.ngap_fileNameArray;
    }

    /* Is array NULL? */
    if (fileNameArray == NULL) {
        /* Success */
        return 1;
    }

    /* send data? */
    switch (when) {
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        /* Is argument IN or INOUT? */
        if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_IN) ||
            (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_INOUT)) {
            dataIsSent = 1;
        }
        break;
    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        /* Is argument OUT or INOUT? */
        if ((argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_OUT) ||
            (argElement->ngae_ioMode == NG_ARGUMENT_IO_MODE_INOUT)) {
            dataIsSent = 1;
        }
        break;
    default:
        assert(0);
    }

    sMngTail = sMng; 
    for (i = 0; i < argElement->ngae_nElements; i++) {
        networkNbytes = 0;
        index = 0;
        if (fileNameArray[i] == NULL) {
            fileHeader[index++] = NGI_PROTOCOL_FILE_TRANSFER_NULL;
            fileHeader[index++] = 0;/* File size is 0 */
        } else if (strcmp(fileNameArray[i], "") == 0) {
            fileHeader[index++] = NGI_PROTOCOL_FILE_TRANSFER_EMPTY_FILENAME;
            fileHeader[index++] = 0;/* File size is 0 */
        } else if (dataIsSent == 0) {
            fileHeader[index++] = NGI_PROTOCOL_FILE_TRANSFER_FILE_DATA;
            fileHeader[index++] = 0;/* File size is 0 */
        } else {
            /* The File Stream Manager Construct */
            sMngFile = ngiFileStreamManagerConstruct(fileNameArray[i], log, error);
            if (sMngFile == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't construct the File Stream Manager.\n", fName);
                goto error;
            }

            /* Get Size */
            result = ngiStreamManagerGetBytesOfReadableData(sMngFile, &isTooLarge, &fileNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't get number of readable bytes from the File Stream Manager.\n", fName);
                goto error;
            }

            fileHeader[index++] = NGI_PROTOCOL_FILE_TRANSFER_FILE_DATA;
            /* File Size */
            if (isTooLarge != 0) {
                fileHeader[index++] = -1;
            } else {
                fileHeader[index++] = fileNbytes;
            }
        } 

        /* Append File Header */
        sMngHeader = ngiMemoryStreamManagerConstruct(BYTES_PER_XDR_UNIT, log, error);
        if (sMngHeader == NULL) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Stream Manager.\n", fName);
            goto error;
        }

        result = ngiProtocolBinary_WriteXDRdata(
            protocol, sMngHeader, NG_ARGUMENT_DATA_TYPE_LONG, &fileHeader, index,
            NULL, &headerOfLength, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the length.\n", fName);
            goto error;
        }
        networkNbytes += headerOfLength;

        result = ngiStreamManagerAppend(sMngTail, sMngHeader, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't append the Stream Manager.\n", fName);
            goto error;
        }
        sMngTail = sMngHeader;
        sMngHeader = NULL;

        if (sMngFile != NULL) {
            /* Append the File Stream Manager */
            result = ngiStreamManagerAppend(sMngTail, sMngFile, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't append the Stream Manager.\n", fName);
                goto error;
            }
            sMngTail = sMngFile;
            sMngFile = NULL;

            networkNbytes += fileNbytes;
        }

        /* Divide? */
        if (protocol->ngp_byteStreamConversion.ngbsc_argumentBlockSize > 0) {

            /* Append Delimiter */
            sMngDelimiter = ngiDelimiterStreamManagerConstruct(log, error);
            if (sMngDelimiter == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't construct the Stream Manager.\n", fName);
                goto error;
            }

            result = ngiStreamManagerAppend(sMngTail, sMngDelimiter, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't append the Stream Manager.\n", fName);
                goto error;
            }
            sMngTail = sMngDelimiter;
            sMngDelimiter = NULL;
        } else {
            /* Append padding */
            result = nglArgumentForceAppendPaddingStreamManager(
                argElement, protocol, sMngTail, networkNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't write the padding data.\n", fName);
                goto error;
            }
        }
    }

    /* Success */
    return 1;
error:
    /* Destruct the Stream Managers */
    if (sMngFile != NULL) {
        result = ngiStreamManagerDestruct(sMngFile, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngFile = NULL;
    }

    if (sMngHeader != NULL) {
        result = ngiStreamManagerDestruct(sMngHeader, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngHeader = NULL;
    }

    if (sMngDelimiter != NULL) {
        result = ngiStreamManagerDestruct(sMngDelimiter, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngDelimiter = NULL;
    }

    return 0;
}

/**
 * Append the padding
 * Append the padding when Conversion is Native.
 */
static int
nglArgumentAppendPadding(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t writeNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t padNbytes;
    ngiStreamManager_t *smLast = NULL, *sMngTmp = NULL;
    static char padding[BYTES_PER_XDR_UNIT] = {0};
    static const char fName[] = "nglArgumentAppendPadding";

    /* Check arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Get the last of Stream Manager */
    sMngTmp = sMng;
    do {
        smLast = sMngTmp;
        sMngTmp = ngiStreamManagerGetNext(sMngTmp, log, error);
    } while (sMngTmp != NULL);

    /* Padding */
    padNbytes = writeNbytes % BYTES_PER_XDR_UNIT;
    if (ngiProtocolIsNative(protocol) && (padNbytes > 0)) {
	padNbytes = BYTES_PER_XDR_UNIT - padNbytes;
        argElement->ngae_networkTotalNbytes += padNbytes;
	result = ngiStreamManagerWrite(
	    smLast, &padding[0], padNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the padding data.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}


/**
 * Force append the padding
 * Append the padding when Conversion is XDR and Native.
 */
static int
nglArgumentForceAppendPadding(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t writeNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t padNbytes;
    ngiStreamManager_t *smLast = NULL, *sMngTmp = NULL;
    static char padding[BYTES_PER_XDR_UNIT] = {0};
    static const char fName[] = "nglArgumentForceAppendPadding";

    /* Check arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Get the last of Stream Manager */
    sMngTmp = sMng;
    do {
        smLast = sMngTmp;
        sMngTmp = ngiStreamManagerGetNext(sMngTmp, log, error);
    } while (sMngTmp != NULL);

    /* Padding */
    padNbytes = writeNbytes % BYTES_PER_XDR_UNIT;
    if (padNbytes > 0) {
	padNbytes = BYTES_PER_XDR_UNIT - padNbytes;
        argElement->ngae_networkTotalNbytes += padNbytes;
	result = ngiStreamManagerWrite(
	    smLast, &padding[0], padNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write the padding data.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Force append the padding
 * Append the padding when Conversion is XDR and Native.
 */
static int
nglArgumentForceAppendPaddingStreamManager(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t writeNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiStreamManager_t *smPadding = NULL;
    static const char fName[] = "nglArgumentForceAppendPaddingStreamManager";

    /* Check arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Append padding */
    smPadding = ngiMemoryStreamManagerConstruct(BYTES_PER_XDR_UNIT, log, error);
    if (smPadding == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't construct the Stream Manager.\n", fName);
        goto error;
    }

    result = nglArgumentForceAppendPadding(argElement, protocol, smPadding,
        writeNbytes, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write the padding data.\n", fName);
        goto error;
    }

    result = ngiStreamManagerAppend(sMng, smPadding, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't append the Stream Manager.\n", fName);
        goto error;
    }

    return 1;
error:
    /* Destroy Padding data? */
    if (smPadding != NULL) {
        result = ngiStreamManagerDestruct(smPadding, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        smPadding = NULL;
    }
    return 0;
}

/**
 * Decode the argument
 */
int
ngiArgumentDecode(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    int allocate,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int result;
    long protocolVersion;
    static const char fName[] = "ngiArgumentDecode";

    /* Check the argument */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    switch (argElement->ngae_dataType) {
    case NG_ARGUMENT_DATA_TYPE_FILENAME:
        if (NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(
                protocolVersion)) {
            result = nglArgumentDecodeFile(
                argElement, compInfo, allocate, protocol, argHead, log, error);
            break;
        }
        /* BREAKTHROUGH */
    case NG_ARGUMENT_DATA_TYPE_STRING:
        if (allocate == 0) {
            result = nglArgumentDecodeStringWithoutAllocate(
                argElement, compInfo, protocol, argHead, log, error);
        } else {
            result = nglArgumentDecodeStringWithAllocate(
                argElement, compInfo, protocol, argHead, log, error);
        }
	break;

    default:
	result = nglArgumentDecodeData(
	    argElement, compInfo, allocate, protocol, argHead, log, error);
	break;
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't decode the argument data.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;
}

static int
nglArgumentDecodeConversion(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *protocol,
    void *pointer,
    ngiProtocolArgumentData_t *argHeader,
    int allocate,
    ngLog_t *log,
    int *error)
{
    int result;
    int isXdr = 0;
    size_t readNbytes;
    ngiStreamManager_t *sMngDest = NULL;
    long protocolVersion;
    static const char fName[] = "nglArgumentDecodeConversion";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol   != NULL);
    assert(argHeader  != NULL);

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    if (NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(protocolVersion) &&
        argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) {
        /* File */
        sMngDest = ngiFileReceivingStreamManagerConstruct(argElement, protocol, argHeader, allocate, log, error);
    } else if (ngiProtocolIsXDR(protocol) != 0) {
        /* XDR */
        isXdr = 1;
        sMngDest = ngiMemoryStreamManagerConstruct(NGI_PROTOCOL_STREAM_NBYTES, log, error);
    } else {
        /* Native */
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
        case NG_ARGUMENT_DATA_TYPE_STRING:
            sMngDest = ngiStringReceivingStreamManagerConstruct(argElement, protocol, argHeader, log, error);
            break;
        default:
            assert(argHeader->ngpad_nElements == 0 || pointer != NULL);
            sMngDest = ngiReceivingStreamManagerConstruct(argElement, protocol, argHeader, pointer, log, error);
            break;
        }
    }
    if (sMngDest == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't construct the Stream Manager for receiving.\n", fName);
        goto error;
    }

    /* Decode Conversion Method */
    result = nglArgumentDecodeConversionSub(
        argElement, compInfo, protocol, NULL, sMngDest, argHeader, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't decode conversion.\n", fName);
        goto error;
    }
    argElement->ngae_fileNumber = 0;

    /* Decode XDR */
    result = 1;
    if (isXdr != 0) {
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            assert(!NGI_PROTOCOL_IS_SUPPORT_FILE_TRANSFER_ON_PROTOCOL(protocolVersion));
        case NG_ARGUMENT_DATA_TYPE_STRING:
            /* Read the data */
            result = nglArgumentDecodeXDRString(
                argElement, protocol, sMngDest,
                argHeader, log, error);
            break;
        default:
            /* Read the data */
            if (argHeader->ngpad_nElements > 0) {
                assert(pointer != NULL);
                result = ngiProtocolBinary_ReadXDRdata(
                    protocol, sMngDest, argElement->ngae_dataType,
                    pointer,
                    argHeader->ngpad_nElements,
                    &readNbytes, log, error);
            }
            break;
        }
    }
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
        NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't decode the argument data.\n", fName);
        goto error;
    }

    /* Destruct */
    result = ngiStreamManagerDestruct(
        sMngDest, log, error);
    sMngDest = NULL;
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't destruct the Stream Manager.\n", fName);
        goto error;
    }

    /* Success */
    return 1;
error:
    if (sMngDest != NULL) {
        /* Destruct */
        result = ngiStreamManagerDestruct(
            sMngDest, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngDest = NULL;
    }
    return 0;
}

static int
nglArgumentDecodeConversionSub(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMngSrc,
    ngiStreamManager_t *sMngDest,
    ngiProtocolArgumentData_t *argHeader,
    ngLog_t *log,
    int *error)
{
    long conversionMethod;
    long typeOfDivision;
    long nBytes;
    int result;
    size_t readNbytes = 0;
    size_t readableNbytes;
    size_t readTotalBytes = 0;
    int isTooLarge;/* Dummy */
    ngiStreamManager_t *sMng = NULL;
#ifndef NGI_NO_ZLIB
    ngiStreamManager_t *sMngZlib = NULL;
#endif /* NGI_NO_ZLIB */
    ngiStreamManager_t *sMngForMerge = NULL;
    long protocolVersion;
    long prevConversionMethod = 0;
    int loop = 1;
    static const char fName[] = "nglArgumentDecodeConversionSub";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol   != NULL);
    assert(sMngDest   != NULL);
    assert(argHeader  != NULL);
    assert(((protocol->ngp_sessionInfo.ngsi_nCompressionInformations == 0) &&
            (protocol->ngp_sessionInfo.ngsi_compressionInformation == NULL)) ||
           ((protocol->ngp_sessionInfo.ngsi_nCompressionInformations > 0) &&
            (protocol->ngp_sessionInfo.ngsi_compressionInformation != NULL)));

    /* Get the Version Number of partner's Protocol */
    result = ngiProtocolGetProtocolVersionOfPartner(
	protocol, &protocolVersion, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't get the version number of partner's protocol.\n",
	    fName);
	return 0;
    }

    /* Loop is for mergin argument data */
    while (loop != 0) {
        readNbytes = 0;
        readTotalBytes = 0;
        sMng = NULL;
#ifndef NGI_NO_ZLIB
        sMngZlib = NULL;
#endif /* NGI_NO_ZLIB */

        /* Is protocol support the data conversion? */
        if (NGI_PROTOCOL_IS_SUPPORT_CONVERSION_METHOD(protocolVersion)) {
            /* Read the Conversion Method */
            result = ngiProtocolBinary_ReadXDRdata(
                protocol, sMngSrc, NG_ARGUMENT_DATA_TYPE_LONG,
                &conversionMethod, 1, &readNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't read the Compress type.\n", fName);
                goto error;
            }
            readTotalBytes += readNbytes;

            /* Read the type of division from Stream Manager */
            if (conversionMethod == NGI_BYTE_STREAM_CONVERSION_DIVIDE) {
                result = ngiProtocolBinary_ReadXDRdata(
                    protocol, sMngSrc, NG_ARGUMENT_DATA_TYPE_LONG,
                    &typeOfDivision, 1, &readNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                        NULL, "%s: Can't read the conversion method.\n", fName);
                    goto error;
                }
                readTotalBytes += readNbytes;
            }

            /* Read the nBytes */
            result = ngiProtocolBinary_ReadXDRdata(
                protocol, sMngSrc, NG_ARGUMENT_DATA_TYPE_LONG,
                &nBytes, 1, &readNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't read the Compress type.\n", fName);
                goto error;
            }
            readTotalBytes += readNbytes;
        } else {
            conversionMethod = NGI_BYTE_STREAM_CONVERSION_RAW;
            nBytes = argHeader->ngpad_nBytes;
        }

        /* Prepare receiving */
        switch(conversionMethod) {
        case NGI_BYTE_STREAM_CONVERSION_DIVIDE:
            if (sMngForMerge == NULL) {
                /* the part of head */
                sMngForMerge = ngiConversionMethodStreamManagerConstruct(protocol, sMngDest, log, error);
                if (sMngForMerge == NULL) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                        NULL, "%s: Can't construct the Stream Manager.\n", fName);
                    goto error;
                }
            }
            sMng = sMngForMerge;
            break;
#ifndef NGI_NO_ZLIB
        case NGI_BYTE_STREAM_CONVERSION_ZLIB:
            sMng = ngiMemoryStreamManagerConstruct(NGI_PROTOCOL_STREAM_NBYTES, log, error);
            if (sMng == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't construct the Stream Manager.\n", fName);
                goto error;
            }
            break;
#endif /* NGI_NO_ZLIB */
        case NGI_BYTE_STREAM_CONVERSION_RAW:
            sMng = sMngDest;
            break;
        default:
            NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Receive invalid value of conversion method.\n", fName);
            goto error;
        }

        /* Read data From Source to Dest */
        if (sMngSrc == NULL) {
            /* Receive argument data from communication */
            result = ngiStreamManagerGetTotalBytesOfReadableData(sMng, &isTooLarge, &readableNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't get number of bytes of readable data.\n", fName);
                goto error;
            }

            result = ngiStreamManagerReceive(sMng, protocol->ngp_communication, nBytes + readableNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't receive from the Socket.\n", fName);
                goto error;
            }
            readTotalBytes += nBytes;

            result = nglArgumentDeletePaddingWithReceive(
                argElement, protocol, protocol->ngp_communication,
                readTotalBytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't read the padding data.\n", fName);
                goto error;
            }
        } else {
            /* Get argument data from Buffer */
            switch (prevConversionMethod) {
            case NGI_BYTE_STREAM_CONVERSION_ZLIB:
                result = ngiStreamManagerCopy(sMngSrc, sMng, nBytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't read from the Stream Manager.\n", fName);
                    goto error;
                }
                break;
            case NGI_BYTE_STREAM_CONVERSION_DIVIDE:
                /* Do Nothing */
                break;
            case NGI_BYTE_STREAM_CONVERSION_RAW:
            default:
                /* Not reached */
                assert(0);
            }
            /* Destroy the Stream Manager */
            result = ngiStreamManagerDestruct(
                sMngSrc, log, error);
            sMngSrc = NULL;
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destruct the Stream Manager.\n", fName);
                goto error;
            }
        }

        /* For Next Conversion method */
        switch (conversionMethod) {
        case NGI_BYTE_STREAM_CONVERSION_DIVIDE:
            /* For File Array */
            if (typeOfDivision == NGI_TYPE_OF_DIVISION_END) {
                if (argElement->ngae_dataType == NG_ARGUMENT_DATA_TYPE_FILENAME) {
                    argElement->ngae_fileNumber++;
                    if (argElement->ngae_fileNumber < argHeader->ngpad_nElements) {
                        typeOfDivision = NGI_TYPE_OF_DIVISION_CONTINUATION;
                    }
                }
            }
            result = 1;
            switch (typeOfDivision) {
            case NGI_TYPE_OF_DIVISION_CONTINUATION:
                sMngSrc = NULL;
                prevConversionMethod = 0;
                break;
            case NGI_TYPE_OF_DIVISION_END:
                /* Decode the Conversion Method */
                sMngSrc = sMngForMerge;
                sMngForMerge = NULL;
                prevConversionMethod = conversionMethod;
                break;
            default:
                NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Receive invalid value of type of division.\n", fName);
                goto error;
            }
            break;

#ifndef NGI_NO_ZLIB
        case NGI_BYTE_STREAM_CONVERSION_ZLIB:
            /* Create the Stream Buffer */
            sMngZlib = ngiMemoryStreamManagerConstruct(NGI_PROTOCOL_STREAM_NBYTES, log, error);
            if (sMng == NULL) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't construct the Stream Manager.\n", fName);
                goto error;
            }

            /* Inflate */
            result = nglArgumentDecodeZlib(
                sMng, nBytes, sMngZlib, NULL,
                (compInfo == NULL) ? NULL : &compInfo->ngcic_decompression,
                log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't decode of Zlib the argument data.\n", fName);
                goto error;
            }
            /* Destroy the Stream Manager */
            result = ngiStreamManagerDestruct(
                sMng, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destruct the Stream Manager.\n", fName);
                goto error;
            }
            prevConversionMethod = conversionMethod;
            sMngSrc = sMngZlib;
            sMngZlib = NULL;
            break;
#endif /* NGI_NO_ZLIB */
        case NGI_BYTE_STREAM_CONVERSION_RAW:
            /* Do Nothing */
            loop = 0;
            break;
        default:
            /* NOTREACHED */
            assert(0);
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Destroy the Source Stream Manager */
    if (sMngSrc != NULL) {
        result = ngiStreamManagerDestruct(
            sMngSrc, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngSrc = NULL;
    }

    /* Destroy the Stream Manager for merge */
    if (sMngForMerge != NULL) {
        result = ngiStreamManagerDestruct(
            sMngForMerge, log, NULL);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Stream Manager.\n", fName);
        }
        sMngForMerge = NULL;
    }
    return 0;
}

/**
 * Argument: Data decode to native data.
 */
static int
nglArgumentDecodeData(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    int allocate,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHeader,
    ngLog_t *log,
    int *error)
{
    int result;
    void *data = NULL;
    int nElements;
    int decodeElements;
    size_t sizeofNativeData;
    static const char fName[] = "nglArgumentDecodeData";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHeader != NULL);

    /* Initialize the local variables */
    sizeofNativeData = argElement->ngae_nativeDataNbytes;
    /* Check the Shrink */
    if ((argHeader->ngpad_encode == NGI_CONVERSION_SKIP) ||
        (argHeader->ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
        (argHeader->ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
        nElements = argElement->ngae_subscript[argElement->ngae_nDimensions - 1].ngsv_totalSize;
    } else {
        nElements = argHeader->ngpad_nElements;
    }

    /* Is buffer allocate? */
    if (allocate != 0) {
        /* Allocate the storage for Argument Data */
        result = ngiArgumentElementAllocateDataStorage(
            argElement, nElements, sizeofNativeData, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR,	NULL,
                "%s: Can't allocate the storage for Argument Data.\n", fName);
            goto error;
        }
    }

    /* Check the Shrink */
    if ((argHeader->ngpad_encode == NGI_CONVERSION_SKIP) ||
        (argHeader->ngpad_encode == NGI_CONVERSION_NINF_SKIP) ||
        (argHeader->ngpad_encode == NGI_CONVERSION_XDR_SKIP)) {
        if (argHeader->ngpad_nElements > 0) {
            data = globus_libc_malloc(sizeofNativeData * argHeader->ngpad_nElements);
            if (data == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL,
                    "%s: Can't allocate the storage for Decode of skip.\n", fName);
                goto error;
            }
        }

        result = nglArgumentDecodeConversion(
            argElement, compInfo, protocol, data, argHeader, allocate, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't decode the argument data.\n", fName);
            goto error;
        }

        if (argHeader->ngpad_nElements > 0) {
            /* Decode of skip */
            nglArgumentDecodeSkip(
                argElement, data, argHeader->ngpad_nElements,
                argElement->ngae_pointer.ngap_void, &decodeElements, log, error);

            globus_libc_free(data);
            data = NULL;
        }
    } else {
        result = nglArgumentDecodeConversion(
            argElement, compInfo, protocol, argElement->ngae_pointer.ngap_void,
            argHeader, allocate, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't decode the argument data.\n", fName);
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (data != NULL) {
        globus_libc_free(data);
        data = NULL;
    }
    if (allocate != 0) {
	result = ngiArgumentElementFreeDataStorage(argElement, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't deallocate the string.\n", fName);
	}
    }

    /* Failed */
    return 0;
}

/**
 * Decode the argument: String without allocate the array of string.
 */
static int
nglArgumentDecodeStringWithoutAllocate(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    char **stringArray;
    static const char fName[] = "nglArgumentDecodeStringWithoutAllocate";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);
    assert(argHead->ngpad_nElements >= 0);

    stringArray = argElement->ngae_pointer.ngap_stringArray;

    /* Initialize the array of string */
    for (i = 0; i < argHead->ngpad_nElements; i++) {
	stringArray[i] = NULL;
    }

    /* Read the string */
    result = nglArgumentDecodeConversion(
        argElement, compInfo, protocol, NULL, argHead, 0, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't decode the argument data.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    for (i = 0; i < argHead->ngpad_nElements; i++) {
	if (stringArray[i] != NULL) {
            free(stringArray[i]);
	    stringArray[i] = NULL;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Decode the argument: String with allocate the array of string.
 */
static int
nglArgumentDecodeStringWithAllocate(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    char **stringArray;
    static const char fName[] = "nglArgumentDecodeStringWithAllocate";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);
    assert(argHead->ngpad_nElements >= 0);

    /* Allocate the array of string */
    result = ngiArgumentElementAllocateDataStorage(
        argElement, argHead->ngpad_nElements, sizeof (char *), log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for array of string.\n", fName);
        return 0;
    }

    stringArray = argElement->ngae_pointer.ngap_stringArray;

    /* Initialize the array of string */
    for (i = 0; i < argHead->ngpad_nElements; i++) {
	stringArray[i] = NULL;
    }

    /* Read the string */
    result = nglArgumentDecodeConversion(
        argElement, compInfo, protocol, NULL, argHead, 1, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't decode the argument data.\n", fName);
        goto error;
    }

    /* Is string NULL? */
    if ((argElement->ngae_nDimensions == 0) &&
    	(stringArray[0] == NULL)) {
	result = ngiArgumentElementFreeDataStorage(argElement, log, NULL);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't deallocate the string.\n", fName);
	    /* Failed */
	    return 0;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate the storage */
    result = ngiArgumentElementFreeDataStorage(argElement, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't deallocate the string.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Decode the argument: XDR strings
 */
static int
nglArgumentDecodeXDRString(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int i;
    size_t strNbytes;
    size_t readNbytesFromStream;
    char **stringArray;
    char *tmpString;
    char *string;
    int result;
    static const char fName[] = "nglArgumentDecodeXDRString";

    /* Check the arguments */
    assert(sMng != NULL);

    stringArray = argElement->ngae_pointer.ngap_stringArray;

    for (i = 0; i < argHead->ngpad_nElements; i++) {
        stringArray[i] = NULL;
    }

    for (i = 0; i < argHead->ngpad_nElements; i++) {
        /* Read the XDR string */
        result = ngiProtocolBinary_ReadXDRstring(
            protocol, sMng, 0, NGI_PROTOCOL_CONTAIN_LENGTH,
            &tmpString, &strNbytes, &readNbytesFromStream, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't read the string.\n", fName);
            goto error;
        }

        /* Is string NULL? */
        if (tmpString == NULL) {
            continue;
        }
        /* Use malloc() instead globus_libc_malloc().
         * Because this string use in application.
         * Application freed this string by free() instead globus_libc_free().
         * Therefore, string made by malloc() instead globus_libc_malloc().
         */
        string = malloc(strNbytes + 1);
        if (string == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't allocate the storage for string.\n",
                fName);
            goto error;
        }
        stringArray[i] = string;

        if (strNbytes > 0) {
            /* Copy the string */
            strncpy(string, tmpString, strNbytes);

            /* Release the string */
            result = ngiProtocolBinary_ReleaseString(protocol, tmpString, log, error);
            tmpString = NULL;
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't release the string.\n", fName);
                goto error;
            }
        }
        string[strNbytes] = '\0';
    }
    return 1;
error:
    for (i = 0; i < argHead->ngpad_nElements; i++) {
        if (stringArray[i] != NULL) {
            free(stringArray[i]);
            stringArray[i] = NULL;
        }
    }
    /* Failed */
    return 0;
}


/**
 * Decode the argument: File
 */
static int
nglArgumentDecodeFile(
    ngiArgumentElement_t *argElement,
    ngCompressionInformationComplex_t *compInfo,
    int allocate,
    ngiProtocol_t *protocol,
    ngiProtocolArgumentData_t *argHead,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    char **fileNameArray = NULL;
    /*
    char *fileName = NULL;
    size_t readTotalBytes;
    */
    static const char fName[] = "nglArgumentDecodeFile";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(argHead != NULL);
    assert(argHead->ngpad_nElements >= 0);

    if (allocate != 0) {
        /* Allocate temporary file name table */
        if (argHead->ngpad_nElements > 0) {
            fileNameArray = globus_libc_malloc(argHead->ngpad_nElements * sizeof(char *));
            if (fileNameArray == NULL) {
                NGI_SET_ERROR(error, NG_ERROR_MEMORY);
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
                    NULL,
                    "%s: Can't allocate the storage for Argument Data.\n", fName);
                goto error;
            }
        }
        /* Initialized fileNameArray */
        for (i = 0;i < argHead->ngpad_nElements;i++) {
            fileNameArray[i] = NULL;
        }

        argElement->ngae_tmpFileNameTable = fileNameArray;
        fileNameArray = NULL;

        /* Set argument pointer */
        if (argElement->ngae_nDimensions > 0) {
            argElement->ngae_pointer.ngap_fileNameArray = 
                argElement->ngae_tmpFileNameTable;
        }
    }

    result = nglArgumentDecodeConversion(
        argElement, compInfo, protocol, NULL, argHead, allocate, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't decode the argument data.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Free fileNameArray? */
    if (fileNameArray != NULL) {
        globus_libc_free(fileNameArray);
        fileNameArray = NULL;
    }

    /* Failed */
    return 0;
}


#if 0 /* Is this necessary? */
/**
 * Delete the padding.
 */
static int
nglArgumentDeletePadding(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytesRead,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "nglArgumentDeletePadding";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);

    /* Delete from Stream? */
    if (sMng != NULL) {
	result = nglArgumentDeletePaddingFromStream(
	    argElement, protocol, sMng, nBytesRead, log, error);
    } else {
	result = nglArgumentDeletePaddingWithReceive(
	    argElement, protocol, protocol->ngp_communication, nBytesRead,
	    log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't delete the padding.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}
#endif

/**
 * Delete the padding with receive.
 */
static int
nglArgumentDeletePaddingWithReceive(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiCommunication_t *comm,
    size_t nBytesRead,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t padNbytes;
    size_t receiveNbytes;
    char buf[BYTES_PER_XDR_UNIT];
    static const char fName[] = "nglArgumentDeletePaddingWithReceive";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(comm != NULL);

    /* Padding */
    padNbytes = nBytesRead % BYTES_PER_XDR_UNIT;
    if (padNbytes > 0) {
	/* Receive padding from socket */
        padNbytes = BYTES_PER_XDR_UNIT - padNbytes;
	result = ngiCommunicationReceive(comm, buf, padNbytes, padNbytes,
	    &receiveNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't receive the padding.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

#if 0 /* Is this necessary? */
/**
 * Delete the padding from Stream.
 */
static int
nglArgumentDeletePaddingFromStream(
    ngiArgumentElement_t *argElement,
    ngiProtocol_t *protocol,
    ngiStreamManager_t *sMng,
    size_t nBytesRead,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t padNbytes;
    static const char fName[] = "nglArgumentDeletePaddingFromStream";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(protocol != NULL);
    assert(sMng != NULL);

    /* Padding */
    padNbytes = nBytesRead % BYTES_PER_XDR_UNIT;
    if (padNbytes > 0) {
        padNbytes = BYTES_PER_XDR_UNIT - padNbytes;

	/* Receive padding to Stream Manager */
	result = ngiStreamManagerReceive(
	    sMng, protocol->ngp_communication, padNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't receive the padding.\n", fName);
	    return 0;
	}

	/* Truncate padding from Stream */
	result = ngiStreamManagerReadBuffer(sMng, padNbytes, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't truncate the padding.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}
#endif 

/**
 * Encode: Skip element of array.
 */
static int
nglArgumentEncodeSkip(
    ngiArgumentElement_t *argElement,
    void *src,
    int srcNbytes,
    void **dest,
    int *destNelements,
    ngLog_t *log,
    int *error)
{
    void *buf = NULL;
    int bufNelem;
    size_t nBytes = 0;
    int nElements;
    int result;
    int i;
    static const char fName[] = "nglArgumentEncodeSkip";

    /* Check the arguments */
    assert(argElement != NULL);
    assert(argElement->ngae_nDimensions > 0);
    assert(src != NULL);
    assert(srcNbytes > 0);
    assert(destNelements != NULL);

    /* Get the number of bytes of native data */
    result = ngiGetSizeofData(argElement->ngae_dataType, &nBytes, log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
            NULL, "%s: Invalid data type %d.\n", argElement->ngae_dataType);
        return 0;
    }

    /* Calculate number of elements of array */
    nElements = 1;
    for (i = 0; i < argElement->ngae_nDimensions; i++) {
        assert(argElement->ngae_subscript[i].ngsv_skip > 0);
        nElements *= 
            (argElement->ngae_subscript[i].ngsv_end -
             argElement->ngae_subscript[i].ngsv_start + 
             argElement->ngae_subscript[i].ngsv_skip - 1) /
            argElement->ngae_subscript[i].ngsv_skip;
    }

    assert(nElements >= 0);
    if (nElements != 0) {
        /* Allocate the data buffer */
        buf = globus_libc_malloc(nBytes * nElements);
        if (buf == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't allocate the storage for Argument Data.\n", fName);
            return 0;
        }

        bufNelem = 0;
        nglArgumentEncodeSkipRecursiveDimension(
            argElement, src, buf, &bufNelem, argElement->ngae_nDimensions - 1, 0);
    }

    /* Success */
    *dest = buf;
    *destNelements = nElements;

    return 1;
}

static void
nglArgumentEncodeSkipRecursiveDimension(
    ngiArgumentElement_t *argElement,
    void *data,
    void *buf,
    int *bufNelem,
    int dimension,
    int index)
{
    int i;
    int space;
    ngiSubscriptValue_t *sv;

    assert(data != NULL);
    assert(buf != NULL);
    assert(bufNelem != NULL);
    assert(*bufNelem >= 0);
    assert(dimension >= 0);
    assert(index >= 0);

    sv = &argElement->ngae_subscript[dimension];

    if (dimension != 0) {
        for (i = 0, space = 1; i < dimension; i++) {
            space *= argElement->ngae_subscript[i].ngsv_size;
        }

        for (i = sv->ngsv_start; i < sv->ngsv_end; i += sv->ngsv_skip) {
            nglArgumentEncodeSkipRecursiveDimension(
                argElement, data, buf, bufNelem, dimension - 1,
                index + i * space);
        }
    } else {
        /* Copy the data with skip */
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_CHAR:
            nglSkipCopyChar(
                buf, bufNelem, ((char *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_SHORT:
            nglSkipCopyShort(
                buf, bufNelem, ((short *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_INT:
            nglSkipCopyInt(
                buf, bufNelem, ((int *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_LONG:
            nglSkipCopyLong(
                buf, bufNelem, ((long *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_FLOAT:
            nglSkipCopyFloat(
                buf, bufNelem, ((float *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_DOUBLE:
            nglSkipCopyDouble(
                buf, bufNelem, ((double *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
            nglSkipCopyScomplex(
                buf, bufNelem, ((scomplex *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
            nglSkipCopyDcomplex(
                buf, bufNelem, ((dcomplex *)data + index),
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;

        default:
            /* Do nothing */
            break;
        }
    }
}

/**
 * Decode: Skip element of array.
 */
static int
nglArgumentDecodeSkip(
    ngiArgumentElement_t *argElement,
    void *src,
    int srcNbytes,
    void **dest,
    int *destElements,
    ngLog_t *log,
    int *error)
{
    int bufNelem;
#if 0
    static const char fName[] = "nglArgumentDecodeSkip";
#endif

    /* Check the arguments */
    assert(argElement != NULL);
    assert(argElement->ngae_nDimensions > 0);
    assert(src != NULL);
    assert(srcNbytes > 0);
    assert(destElements != NULL);

    bufNelem = 0;
    nglArgumentDecodeSkipRecursiveDimension(
        argElement, src, &bufNelem, dest, argElement->ngae_nDimensions - 1, 0);

    return 1;
}

static void
nglArgumentDecodeSkipRecursiveDimension(
    ngiArgumentElement_t *argElement,
    void *data,
    int *bufNelem,
    void *buf,
    int dimension,
    int index)
{
    int i;
    int space;
    ngiSubscriptValue_t *sv;

    assert(data != NULL);
    assert(buf != NULL);
    assert(bufNelem != NULL);
    assert(*bufNelem >= 0);
    assert(dimension >= 0);
    assert(index >= 0);

    sv = &argElement->ngae_subscript[dimension];

    if (dimension != 0) {
        for (i = 0, space = 1; i < dimension; i++) {
            space *= argElement->ngae_subscript[i].ngsv_size;
        }

        for (i = sv->ngsv_start; i < sv->ngsv_end; i += sv->ngsv_skip) {
            nglArgumentDecodeSkipRecursiveDimension(
                argElement, data, bufNelem, buf,
                    dimension - 1, index + i * space);
        }
    } else {
        /* Copy the data with skip */
        switch (argElement->ngae_dataType) {
        case NG_ARGUMENT_DATA_TYPE_CHAR:
            nglSkipCopyCharOfDecode(
                ((char *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_SHORT:
            nglSkipCopyShortOfDecode(
                ((short *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_INT:
            nglSkipCopyIntOfDecode(
                ((int *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_LONG:
            nglSkipCopyLongOfDecode(
                ((long *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_FLOAT:
            nglSkipCopyFloatOfDecode(
                ((float *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_DOUBLE:
            nglSkipCopyDoubleOfDecode(
                ((double *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
            nglSkipCopyScomplexOfDecode(
                ((scomplex *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;
        case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
            nglSkipCopyDcomplexOfDecode(
                ((dcomplex *)buf + index), data, bufNelem,
                sv->ngsv_start, sv->ngsv_end, sv->ngsv_skip);
            break;

        default:
            /* Do nothing */
            break;
        }
    }
}

#ifndef NGI_NO_ZLIB
/**
 * Encode: Zlib
 */
static int
nglArgumentEncodeZlib(
    ngiStreamManager_t *sMngSrc,
    ngiStreamManager_t *sMngDest,
    size_t *totalSize,
    ngCompressionInformationElement_t *compInfo,
    ngLog_t *log,
    int *error)
{
    void *readBuf, *writeBuf;/* Buffer of Stream Manager */
    size_t canReadNbytes = 0;
    size_t canWriteNbytes = 0;
    size_t totalReadBytes = 0;
    z_stream z;
    int zInitialized = 0;
    int flush, state;
    int result;
    static const char fName[] = "nglArgumentEncodeZlib";

    /* Check arguments */
    assert(sMngSrc != NULL);
    assert(sMngDest != NULL);
    assert(totalSize != NULL);

    /* Initialize */
    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;

    /* Measurement the compression information*/
    if (compInfo != NULL) {
	result = ngiSetStartTime(&compInfo->ngcie_timeMeasurement, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't set the Start time.\n", fName);
	    return 0;
	}
    }

    /* Initialize */
    result = deflateInit(&z, Z_DEFAULT_COMPRESSION);
    if (result != Z_OK) {
        NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: deflateInit failed.: %s\n",
            fName, (z.msg) ? z.msg : "Compression error.");
        return 0;
    }
    zInitialized = 1;

    z.next_out  = 0;
    z.avail_out = 0;
    z.next_in   = 0;
    z.avail_in  = 0;

    flush = Z_NO_FLUSH;
    do {
        if ((z.avail_in == 0) &&
            (flush != Z_FINISH)) {
            assert(sMngSrc != NULL);
            /* Read from Stream Buffer */
            if (canReadNbytes > 0) {
                result = ngiStreamManagerReadBuffer(
                    sMngSrc, canReadNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR,	NULL,
                        "%s: Can't read the Stream Buffer.\n", fName);
                    goto error;
                }
                totalReadBytes += canReadNbytes;
            }

            canReadNbytes = 0;
            while (canReadNbytes == 0) {
                /* Get the buffer of Stream Manager */
                result = ngiStreamManagerGetReadableBuffer(
                    sMngSrc, &readBuf, 0, &canReadNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't get the Readable Buffer from Stream Manager.\n",
                        fName);
                    goto error;
                }

                if (canReadNbytes == 0) {
                    /* Check the remaining buffer of Stream Manager */
                    sMngSrc = ngiStreamManagerGetNext(
                        sMngSrc, log, error);
                    if (sMngSrc == NULL) {
                        canReadNbytes = 0;
                        readBuf = NULL;
                        break;
                    }
                }
            }

            z.avail_in = canReadNbytes;
            z.next_in  = readBuf;

            if (z.avail_in == 0) {
                flush = Z_FINISH;
            }
        }

        if (z.avail_out == 0) {
            if (canWriteNbytes > 0) {
                /* Write to Stream Buffer */
                result = ngiStreamManagerWriteBuffer(
                    sMngDest, canWriteNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR,	NULL,
                        "%s: Can't write the Stream Buffer.\n", fName);
                    goto error;
                }
            }

            /* Get the buffer of Stream Manager */
            result = ngiStreamManagerGetWritableBuffer(
                sMngDest, &writeBuf, 1, &canWriteNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't get the Writable Buffer from Stream Manager.\n",
                    fName);
                goto error;
	    }
            z.next_out  = writeBuf;
            z.avail_out = canWriteNbytes;
        }

        /* Compress */
        state = deflate(&z, flush);
        if ((state != Z_STREAM_END) &&
            (state != Z_OK)) {
            NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: deflate failed.: %s\n",
                fName, (z.msg) ? z.msg : "Compression error.");
            goto error;
        }
    } while (state != Z_STREAM_END);

    assert(canReadNbytes == 0);

    if (canWriteNbytes - z.avail_out > 0) {
        /* Write to Stream Buffer */
        result = ngiStreamManagerWriteBuffer(
            sMngDest, canWriteNbytes - z.avail_out, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR,	NULL,
                "%s: Can't write the Stream Buffer.\n", fName);
            goto error;
        }
    }

    if (totalSize != NULL) {
        *totalSize = z.total_out;
    }

    /* Measurement the compression information*/
    if (compInfo != NULL) {
	compInfo->ngcie_lengthRaw += z.total_in;
	compInfo->ngcie_lengthCompressed += z.total_out;
	compInfo->ngcie_measured = 1;
	result = ngiSetEndTime(&compInfo->ngcie_timeMeasurement, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't set the End time.\n", fName);
            goto error;
	}
	compInfo->ngcie_executionRealTime = ngiTimevalAdd(
	    compInfo->ngcie_executionRealTime,
	    compInfo->ngcie_timeMeasurement.nget_real.nget_execution);
	compInfo->ngcie_executionCPUtime = ngiTimevalAdd(
	    compInfo->ngcie_executionCPUtime,
	    compInfo->ngcie_timeMeasurement.nget_cpu.nget_execution);
    }

    /* Output the log (Size of the data after compression) */
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION,
        NULL, "%s: Before compress Data = %d Bytes, After compress Data = %d Bytes, The rate of compression = %f%%.\n",
        fName, z.total_in, z.total_out,
        ((double)z.total_out / (double)z.total_in) * 100.0);

    /* Finalize */
    zInitialized = 0;
    result = deflateEnd(&z);
    if (result != Z_OK) {
        NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: deflateEnd failed.: %s\n",
            fName, (z.msg) ? z.msg : "Compression error.");
        goto error;
    }

    return 1;

    /* Error occurred */
error:
    if (zInitialized != 0) {
        zInitialized = 0;
        result = deflateEnd(&z);
        if (result != Z_OK) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: deflateEnd failed.: %s\n",
                fName, (z.msg) ? z.msg : "Compression error.");
        }
    }

    return 0;
}

/**
 * Decode: Zlib
 */
static int
nglArgumentDecodeZlib(
    ngiStreamManager_t *sMngSrc,
    long srcSize,
    ngiStreamManager_t *sMngDest,
    size_t *totalSize,
    ngCompressionInformationElement_t *compInfo,
    ngLog_t *log,
    int *error)
{
    void *readBuf;
    size_t canReadNbytes = 0;
    void *writeBuf;
    size_t canWriteNbytes = 0;
    size_t srcNbytes;
    int state;
    int result;
    z_stream z;
    int zInitialized = 0;
    int isTooLarge;/* dummy */
    static const char fName[] = "nglArgumentDecodeZlib";

    /* Check Argument */
    assert(sMngSrc  != NULL);
    assert(sMngDest != NULL);

    if (srcSize < 0) {
        /* GetSize */
        result = ngiStreamManagerGetTotalBytesOfReadableData(sMngSrc, &isTooLarge, &srcNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't get number of bytes of total data from the Stream Manager\n",
                fName);
            goto error;
        }
    } else {
	srcNbytes = srcSize;
    }

    if (srcNbytes == 0) {
        /* Data is not exist */
        /* Success */
        return 1;
    }

    /* Measurement the compression information*/
    if (compInfo != NULL) {
	result = ngiSetStartTime(&compInfo->ngcie_timeMeasurement, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't set the Start time.\n", fName);
	    return 0;
	}
    }

    /* Initialize */
    z.zalloc   = Z_NULL;
    z.zfree    = Z_NULL;
    z.opaque   = Z_NULL;
    result = inflateInit(&z);
    if (result != Z_OK) {
        NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: inflateInit failed.: %s\n",
            fName, (z.msg) ? z.msg : "Compression error.");
        goto error;
    }
    zInitialized = 1;

    z.next_in   = NULL;
    z.avail_in  = 0;
    z.next_out  = NULL;
    z.avail_out = 0;

    do {
        if (z.avail_in == 0) {
            assert(sMngSrc != NULL);

            /* Read from Stream Buffer */
            if (canReadNbytes > 0) {
                result = ngiStreamManagerReadBuffer(
                    sMngSrc, canReadNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't read Buffer from Stream Manager.\n",
                        fName);
                    goto error;
                }
                srcNbytes -= canReadNbytes;
                if (srcNbytes == 0) {
                    NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: The Input data is not enough\n",
                        fName);
                    goto error;
                }
            }
            canReadNbytes = 0;
            for (;;) {
                /* Get the buffer of Stream Manager */
                result = ngiStreamManagerGetReadableBuffer(
                    sMngSrc, &readBuf, 0, &canReadNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR, NULL,
                        "%s: Can't get the Readable Buffer from Stream Manager.\n",
                        fName);
                    goto error;
                }

                if (canReadNbytes == 0) {
                    /* Check the remaining buffer of Stream Manager */
                    sMngSrc = ngiStreamManagerGetNext(
                        sMngSrc, log, error);
                    if (sMngSrc == NULL) {
                        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                            NG_LOG_LEVEL_ERROR, NULL,
                            "%s: Can't read the Stream Buffer.\n", fName);
                        goto error;
                    }
                } else {
                    break;
                }
            }

            if (srcNbytes < canReadNbytes) {
                canReadNbytes = srcNbytes;
            }
            z.next_in  = readBuf;
            z.avail_in = canReadNbytes;
        }

        if (z.avail_out == 0) {
            if (canWriteNbytes > 0) {
                /* Write to Stream Buffer */
                result = ngiStreamManagerWriteBuffer(
                    sMngDest, canWriteNbytes, log, error);
                if (result == 0) {
                    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                        NG_LOG_LEVEL_ERROR,	NULL,
                        "%s: Can't write the Stream Buffer.\n", fName);
                    goto error;
                }
            }

            /* Get the buffer of Stream Manager */
            result = ngiStreamManagerGetWritableBuffer(
                sMngDest, &writeBuf, 1, &canWriteNbytes, log, error);
            if (result == 0) {
                ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't get the Writable Buffer from Stream Manager.\n",
                    fName);
                goto error;
	    }
            assert(canWriteNbytes > 0);

            z.next_out  = writeBuf;
            z.avail_out = canWriteNbytes;
        }

        /* Decompress */
        state = inflate(&z, Z_NO_FLUSH);
        if ((state != Z_STREAM_END) &&
            (state != Z_OK)) {
            NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: inflate failed.: %s\n",
                fName, (z.msg) ? z.msg : "Compression error.");
            goto error;
        }
    } while (state != Z_STREAM_END);

    /* Read from Stream Buffer */
    if (canReadNbytes > 0) {
        result = ngiStreamManagerReadBuffer(
            sMngSrc, canReadNbytes, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Stream Buffer.\n", fName);
            goto error;
        }
    }

    /* Write to Stream Buffer */
    if (canWriteNbytes - z.avail_out > 0) {
        result = ngiStreamManagerWriteBuffer(
            sMngDest, canWriteNbytes - z.avail_out, log, error);
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR,	NULL,
                "%s: Can't write the Stream Buffer.\n", fName);
            goto error;
        }
    }

    if (totalSize != NULL) {
        *totalSize = z.total_out;
    }

    /* Measurement the compression information*/
    if (compInfo != NULL) {
	compInfo->ngcie_lengthCompressed += z.total_in;
	compInfo->ngcie_lengthRaw += z.total_out;
	compInfo->ngcie_measured = 1;
	result = ngiSetEndTime(&compInfo->ngcie_timeMeasurement, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't set the End time.\n", fName);
            goto error;
	}
	compInfo->ngcie_executionRealTime = ngiTimevalAdd(
	    compInfo->ngcie_executionRealTime,
	    compInfo->ngcie_timeMeasurement.nget_real.nget_execution);
	compInfo->ngcie_executionCPUtime = ngiTimevalAdd(
	    compInfo->ngcie_executionCPUtime,
	    compInfo->ngcie_timeMeasurement.nget_cpu.nget_execution);
    }

    /* Finalize */
    zInitialized = 0;
    result = inflateEnd(&z);
    if (result != Z_OK) {
        NGI_SET_ERROR(error, NG_ERROR_COMPRESSION);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: inflateEnd failed.: %s\n",
            fName, (z.msg) ? z.msg : "Compression error.");
        goto error;
    }

    /* Success */
    return 1;
error:
    /* Finalize */
    if (zInitialized != 0) {
        zInitialized = 0;
        result = inflateEnd(&z);
        if (result != Z_OK) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: inflateEnd failed.: %s\n",
                fName, (z.msg) ? z.msg : "Compression error.");
        }
    }

    return 0;
}
#endif /* !NGI_NO_ZLIB */

#if 0 /* Is this necessary? */
/**
 * Enode: XDR
 */
static int
nglArgumentEncodeXDR(
    ngiArgumentConverter_t *conv,
    ngiArgumentConvertParameter_t *param,
    void *src,
    size_t srcNbytes,
    void **dest,
    size_t *destNbytes,
    ngLog_t *log,
    int *error)
{
#if 0 /* Temporary commented out */
    int result;
    size_t nBytes;
    int nElements;
    void *buf;
    static const char fName[] = "nglArgumentEncodeXDR";

    /* Check the arguments */
    assert(conv != NULL);
    assert(param != NULL);
    assert(src != NULL);
    assert(srcNbytes > 0);
    assert(dest != NULL);
    assert(destNbytes != NULL);

    /* Convert the data */
    result = ngiGetCommunicatorEncodeArray(
    	conv->ngac_netComm, src, srcNbytes, dest, destNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't convert the argument data.\n", fName);
	return 0;
    }
#endif

    /* Success */
    return 1;
}

/**
 * Encode: Zlib
 */
static int
nglArgumentEncodeZlib(
    ngiArgumentConverter_t *conv,
    ngiArgumentConvertParameter_t *param,
    void *src,
    size_t srcNbytes,
    void **dest,
    size_t *destNbytes,
    ngLog_t *log,
    int *error)
{
#if 0 /* Temporary commented out */
    int result;
    size_t nBytes
    int nElements;
    void *buf;
    z_stream zStream;
    static const char fName[] = "nglArgumentEncodeZlib";

    /* Check the arguments */
    assert(conv != NULL);
    assert(param != NULL);
    assert(src != NULL);
    assert(srcNbytes > 0);
    assert(dest != NULL);
    assert(destNbytes != NULL);

    /* Calculate the number of bytes of Encoded Data */
    nBytes = srcNbytes + 100;

    /* Convert the data */
    result = ngiGetCommunicatorEncodeArray(
    	conv->ngac_netComm, src, srcNbytes, dest, destNbytes, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't convert the argument data.\n", fName);
	return 0;
    }

    /* Allocate the data buffer */
    buf = globus_libc_malloc(nBytes);
    if (buf == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Argument Data.\n", fName);
	return 0;
    }

    /* Initialize the Zlib Stream */
    zStream.zalloc = Z_NULL;
    zStream.zfree = Z_NULL;
    zStream.opaque = Z_NULL;
    zStream.avail_in = srcNbytes;
    zStream.next_in = src;
    zStream.avail_out = nBytes;
    zStream.next_out = buf;

    /* Encode */
    result = deflate(&zStream, Z_NO_FLUSH);
    if (result != Z_STREAM_END) {
    	NGI_SET_ERROR(error, NG_ERROR_COMPRESS);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't compress the Argument Data.\n", fName);
	goto error;
    }

    /* Success */
    *dest = buf;
    *destNbytes = nBytes;
    return 1;

    /* Error occurred */
error:
    /* Deallocate */
    globus_libc_free(buf);
#endif /* Temporary commented out */

    return 0;
}
#endif

/**
 * Copy the char with skip.
 */
static void
nglSkipCopyChar(
    char *dest,
    int *destNelem,
    char *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the short with skip.
 */
static void
nglSkipCopyShort(
    short *dest,
    int *destNelem,
    short *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the int with skip.
 */
static void
nglSkipCopyInt(
    int *dest,
    int *destNelem,
    int *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the long with skip.
 */
static void
nglSkipCopyLong(
    long *dest,
    int *destNelem,
    long *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the float with skip.
 */
static void
nglSkipCopyFloat(
    float *dest,
    int *destNelem,
    float *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the double with skip.
 */
static void
nglSkipCopyDouble(
    double *dest,
    int *destNelem,
    double *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem] = src[i];
    }
}

/**
 * Copy the scomplex with skip.
 */
static void
nglSkipCopyScomplex(
    scomplex *dest,
    int *destNelem,
    scomplex *src,
    int start,
    int end,
    int skip)

{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
    	dest[*destNelem].r = src[i].r;
    	dest[*destNelem].i = src[i].i;
    }
}

/**
 * Copy the dcomplex with skip.
 */
static void
nglSkipCopyDcomplex(
    dcomplex *dest,
    int *destNelem,
    dcomplex *src,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(destNelem != NULL);
    assert(src != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*destNelem)++) {
        dest[*destNelem].r = src[i].r;
        dest[*destNelem].i = src[i].i;
    }
}

/**
 * Copy the char with skip Of Decode.
 */
static void
nglSkipCopyCharOfDecode(
    char *dest,
    char *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the short with skip Of Decode.
 */
static void
nglSkipCopyShortOfDecode(
    short *dest,
    short *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the int with skip Of Decode.
 */
static void
nglSkipCopyIntOfDecode(
    int *dest,
    int *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the long with skip Of Decode.
 */
static void
nglSkipCopyLongOfDecode(
    long *dest,
    long *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the float with skip Of Decode.
 */
static void
nglSkipCopyFloatOfDecode(
    float *dest,
    float *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the double with skip Of Decode.
 */
static void
nglSkipCopyDoubleOfDecode(
    double *dest,
    double *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
    	dest[i] = src[*srcNelem];
    }
}

/**
 * Copy the scomplex with skip Of Decode.
 */
static void
nglSkipCopyScomplexOfDecode(
    scomplex *dest,
    scomplex *src,
    int *srcNelem,
    int start,
    int end,
    int skip)

{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
        dest[i].r = src[*srcNelem].r;
        dest[i].i = src[*srcNelem].i;
    }
}

/**
 * Copy the dcomplex with skip Of Decode.
 */
static void
nglSkipCopyDcomplexOfDecode(
    dcomplex *dest,
    dcomplex *src,
    int *srcNelem,
    int start,
    int end,
    int skip)
{
    int i;

    /* Check the argument */
    assert(dest != NULL);
    assert(src != NULL);
    assert(srcNelem != NULL);
    assert(start <= end);
    assert(skip > 0);

    /* Copy the data */
    for (i = start; i < end; i += skip, (*srcNelem)++) {
        dest[i].r = src[*srcNelem].r;
        dest[i].i = src[*srcNelem].i;
    }
}

/**
 * Subscript Value: Construct
 */
ngiSubscriptValue_t *
ngiSubscriptValueConstruct(
    ngSubscriptInformation_t *subInfo,
    int nSubInfos,
    ngiArgument_t *arg,
    ngiArgument_t *referArg,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiSubscriptValue_t *subValue;
    static const char fName[] = "ngiSubscriptValueConstruct";

    /* Check the arguments */
    assert(subInfo != NULL);
    assert(nSubInfos > 0);
    assert(arg != NULL);
    assert(referArg != NULL);

    /* Allocate */
    subValue = ngiSubscriptValueAllocate(nSubInfos, log, error);
    if (subValue == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Subscript Value.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiSubscriptValueInitialize(
    	subValue, subInfo, nSubInfos, arg, referArg, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Subscript Value.\n", fName);
	goto error;
    }

    /* Success */
    return subValue;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngiSubscriptValueFree(subValue, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Subscript Value.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Subscript Value: Destruct
 */
int
ngiSubscriptValueDestruct(
    ngiSubscriptValue_t *subValue,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiSubscriptValueDestruct";

    /* Check the arguments */
    assert(subValue != NULL);

    /* Finalize */
    result = ngiSubscriptValueFinalize(subValue, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Subscript Value.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiSubscriptValueFree(subValue, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Subscript Value.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Subscript Value: Allocate
 */
ngiSubscriptValue_t *
ngiSubscriptValueAllocate(int nSubInfos, ngLog_t *log, int *error)
{
    ngiSubscriptValue_t *subValue;
    static const char fName[] = "ngiSubscriptValueAllocate";

    /* Check the arguments */
    assert(nSubInfos > 0);

    /* Allocate */
    subValue = globus_libc_calloc(nSubInfos, sizeof (ngiSubscriptValue_t));
    if (subValue == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Subscript Value.\n", fName);
	return NULL;
    }

    /* Success */
    return subValue;
}

/**
 * Subscript Value: Dellocate
 */
int
ngiSubscriptValueFree(ngiSubscriptValue_t *subValue, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(subValue != NULL);

    /* Deallocate */
    globus_libc_free(subValue);

    /* Success */
    return 1;
}

/**
 * Subscript Value: Initialize
 */
int
ngiSubscriptValueInitialize(
    ngiSubscriptValue_t *subValue,
    ngSubscriptInformation_t *subInfo,
    int nSubInfos,
    ngiArgument_t *arg,
    ngiArgument_t *referArg,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    static const char fName[] = "ngiSubscriptValueInitialize";

    /* Check the arguments */
    assert(subValue != NULL);
    assert(subInfo != NULL);
    assert(nSubInfos > 0);
    assert(arg != NULL);
    assert(referArg != NULL);

    /* Initialize */
    for (i = 0; i < nSubInfos; i++) {
	/* Calculate the size */
	result = ngiEvaluateExpression(
	    subInfo[i].ngsi_size, arg, referArg, &subValue[i].ngsv_size,
            log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't evaluate the expression.\n", fName);
	    abort();
	    return 0;
	}

	/* Calculate the start */
	result = ngiEvaluateExpression(
	    subInfo[i].ngsi_start, arg, referArg, &subValue[i].ngsv_start,
            log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't evaluate the expression.\n", fName);
	    abort();
	    return 0;
	}

	/* Calculate the end */
	result = ngiEvaluateExpression(
	    subInfo[i].ngsi_end, arg, referArg, &subValue[i].ngsv_end,
            log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't evaluate the expression.\n", fName);
	    abort();
	    return 0;
	}

	/* Calculate the skip */
	result = ngiEvaluateExpression(
	    subInfo[i].ngsi_skip, arg, referArg, &subValue[i].ngsv_skip,
            log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%s: Can't evaluate the expression.\n", fName);
	    abort();
	    return 0;
	}
    }

    /* Calculate the total size */
    subValue[0].ngsv_totalSize = subValue[0].ngsv_size;
    for (i = 1; i < nSubInfos; i++) {
    	subValue[i].ngsv_totalSize =
	    subValue[i].ngsv_size * subValue[i - 1].ngsv_totalSize;
    }

    /* Success */
    return 1;
}

/**
 * Subscript Value: Finalize
 */
int
ngiSubscriptValueFinalize(
    ngiSubscriptValue_t *subValue,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(subValue != NULL);

    /* Do nothing */

    /* Success */
    return 1;
}

/**
 * Function table for Evaluate expression.
 *
 * The ngee_function is NULL as terminater.
 */
static struct ngiEvaluateExpression_s {
    ngExpressionOperationCode_t	ngee_opcode; /* Operation Code */
    int (*ngee_function)(
    	ngExpressionElement_t *, ngiStackInt_t *, int *, ngLog_t *, int *);
} ngiEEFunctions[] = {
    {NG_EXPRESSION_OPCODE_PLUS,          nglEvaluateExpressionPlus},
    {NG_EXPRESSION_OPCODE_MINUS,         nglEvaluateExpressionMinus},
    {NG_EXPRESSION_OPCODE_MULTIPLY,      nglEvaluateExpressionMultiply},
    {NG_EXPRESSION_OPCODE_DIVIDE,        nglEvaluateExpressionDivide},
    {NG_EXPRESSION_OPCODE_MODULO,        nglEvaluateExpressionModulo},
    {NG_EXPRESSION_OPCODE_UNARY_MINUS,   nglEvaluateExpressionUnaryMinus},
    {NG_EXPRESSION_OPCODE_POWER,         nglEvaluateExpressionPower},
    {NG_EXPRESSION_OPCODE_EQUAL,         nglEvaluateExpressionEqual},
    {NG_EXPRESSION_OPCODE_NOT_EQUAL,     nglEvaluateExpressionNotEqual},
    {NG_EXPRESSION_OPCODE_GREATER_THAN,  nglEvaluateExpressionGreaterThan},
    {NG_EXPRESSION_OPCODE_LESS_THAN,     nglEvaluateExpressionLessThan},
    {NG_EXPRESSION_OPCODE_GREATER_EQUAL, nglEvaluateExpressionGreaterEqual},
    {NG_EXPRESSION_OPCODE_LESS_EQUAL,    nglEvaluateExpressionLessEqual},
    {NG_EXPRESSION_OPCODE_TRI,           nglEvaluateExpressionTri},
    {(ngExpressionOperationCode_t) 0, NULL},
};

/**
 * Evaluate expression.
 */
int
ngiEvaluateExpression(
    ngExpressionElement_t *expression,
    ngiArgument_t *arg,
    ngiArgument_t *referArg,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int i, iOp;
    int value;
    ngiStackInt_t *stack;
    static const char fName[] = "ngiEvaluateExpression";
#define NGI_STACK_SIZE 10

    /* Check the arguments */
    assert(expression != NULL);
    assert(arg != NULL);
    assert(referArg != NULL);
    assert(answer != NULL);

#if 1 /* 2003/9/30 asou: Is this necessary? */
    /* Is there no subscripts? */
    switch (expression[0].ngee_valueType) {
    case NG_EXPRESSION_VALUE_TYPE_NONE:
    case NG_EXPRESSION_VALUE_TYPE_END:
	/* Do nothing */
	return 1;

    default:
	/* Do nothing */
	break;
    }
#endif

    /* Construct the stack */
    stack = ngiStackIntConstruct(NGI_STACK_SIZE, log, error);
    if (stack == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the Stack.\n", fName);
	return 0;
    }

    /* Evaluate expression */
    *answer = 0;
    for (i = 0; expression[i].ngee_valueType != NG_EXPRESSION_VALUE_TYPE_END; i++) {
    	switch (expression[i].ngee_valueType) {
	case NG_EXPRESSION_VALUE_TYPE_NONE:
	    /* Do nothing */
	    break;

	case NG_EXPRESSION_VALUE_TYPE_CONSTANT:
	    /* Push the value */
	    result = ngiStackIntPush(
	    	stack, expression[i].ngee_value, log, error);
	    if (result == 0) {
	    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Can't push the data.\n", fName);
		abort();
	    	return 0;
	    }
	    break;

	case NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT:
	    result = nglArgumentGetValueInt(
	    	arg, referArg, expression[i].ngee_value, &value, log, error);
	    if (result == 0) {
	    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Can't get the value of Argument.\n", fName);
		abort();
	    	return 0;
	    }

	    /* Push the value */
	    result = ngiStackIntPush(stack, value, log, error);
	    if (result == 0) {
	    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Can't push the data.\n", fName);
		abort();
	    	return 0;
	    }

	    break;

	case NG_EXPRESSION_VALUE_TYPE_OPCODE:
	    /* Find function */
	    for (iOp = 0; ngiEEFunctions[iOp].ngee_function != NULL; iOp++) {
	    	if (expression[i].ngee_value == ngiEEFunctions[iOp].ngee_opcode)
		    goto found;
	    }

	default:
	    /* Function is not found */
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    	NULL, "%: Operation Code %d is not defined.\n",
		fName, expression[i].ngee_value);
	    abort();
	    return 0;
	    
	    /* Found */
    	found:
	    /* Evaluate expression */
	    result = ngiEEFunctions[iOp].ngee_function(
	    	expression, stack, &value, log, error);
	    if (result == 0) {
	    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		    NULL, "%s: Can't evaluate the expression.\n", fName);
		abort();
		return 0;
	    }

	    /* Push the value */
	    result = ngiStackIntPush(stack, value, log, error);
	    if (result == 0) {
	    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		    NULL, "%s: Can't push the data.\n", fName);
		abort();
	    	return 0;
	    }

	    break;
	}
    }

    /* Pop the answer */
    result = ngiStackIntPop(stack, &value, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't push the data.\n", fName);
	abort();
    	return 0;
    }
    *answer = value;

    /* Destruct the stack */
    result = ngiStackIntDestruct(stack, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't destruct the Stack.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

#undef NGI_STACK_SIZE
}

/**
 * Evaluate expression: Plus
 */
static int
nglEvaluateExpressionPlus(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionPlus";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 + value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Minus
 */
static int
nglEvaluateExpressionMinus(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionMinus";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 - value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Multiply
 */
static int
nglEvaluateExpressionMultiply(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionMultiply";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 * value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Divide
 */
static int
nglEvaluateExpressionDivide(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionDivide";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 / value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Modulo
 */
static int
nglEvaluateExpressionModulo(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionModulo";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 % value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Unary Minus
 */
static int
nglEvaluateExpressionUnaryMinus(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1;
    static const char fName[] = "nglEvaluateExpressionUnaryMinus";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = -value1;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Power
 */
static int
nglEvaluateExpressionPower(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionPower";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = 1;
    for (i = 0; i < value2; i++)
    	*answer *= value1;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Equal
 */
static int
nglEvaluateExpressionEqual(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionEqual";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = (value1 == value2);

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Not Equal
 */
static int
nglEvaluateExpressionNotEqual(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionNotEqual";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = (value1 != value2);

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Greater Than
 */
static int
nglEvaluateExpressionGreaterThan(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionGreaterThan";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = value1 > value2;

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Less Than
 */
static int
nglEvaluateExpressionLessThan(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionLessThan";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = (value1 < value2);

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Greater Equal
 */
static int
nglEvaluateExpressionGreaterEqual(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionGreaterEqual";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = (value1 >= value2);

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Less Equal
 */
static int
nglEvaluateExpressionLessEqual(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int value1, value2;
    static const char fName[] = "nglEvaluateExpressionLessEqual";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = (value1 <= value2);

    /* Success */
    return 1;
}

/**
 * Evaluate expression: Tri
 */
static int
nglEvaluateExpressionTri(
    ngExpressionElement_t *expression,
    ngiStackInt_t *stack,
    int *answer,
    ngLog_t *log,
    int *error)
{
    int result;
    int cond, value1, value2;
    static const char fName[] = "nglEvaluateExpressionTri";

    /* Check the arguments */
    assert(expression != NULL);
    assert(stack != NULL);
    assert(answer != NULL);

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value2, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the operand */
    result = ngiStackIntPop(stack, &value1, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Pop the condition */
    result = ngiStackIntPop(stack, &cond, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't pop the value.\n", fName);
	abort();
	return 0;
    }

    /* Calculate */
    *answer = cond ? value1 : value2;

    /* Success */
    return 1;
}

/**
 * Stack: Int: Construct
 */
ngiStackInt_t *
ngiStackIntConstruct(int sizeOfStack, ngLog_t *log, int *error)
{
    int result;
    ngiStackInt_t *stack;
    static const char fName[] = "ngiStackIntConstruct";

    /* Check the arguments */
    assert(sizeOfStack > 0);

    /* Allocate */
    stack = ngiStackIntAllocate(sizeOfStack, log, error);
    if (stack == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stack.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiStackIntInitialize(stack, sizeOfStack, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Stack.\n", fName);
	goto error;
    }

    /* Success */
    return stack;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngiStackIntFree(stack, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stack.\n", fName);
	goto error;
    }

    return NULL;
}

/**
 * Stack: Int: Destruct
 */
int
ngiStackIntDestruct(ngiStackInt_t *stack, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiStackIntDestruct";

    /* Check the arguments */
    assert(stack != NULL);

    /* Finalize */
    result = ngiStackIntFinalize(stack, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Stack.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngiStackIntFree(stack, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Stack.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Stack: Int: Allocate
 */
ngiStackInt_t *
ngiStackIntAllocate(int sizeOfStack, ngLog_t *log, int *error)
{
    ngiStackInt_t *stack;
    static const char fName[] = "ngiStackIntAllocate";

    /* Check the arguments */
    assert(sizeOfStack > 0);

    /* Allocate */
    stack = globus_libc_calloc(1,
    	sizeof (ngiStackInt_t) - sizeof (stack->ngsi_data)
	+ (sizeof (stack->ngsi_data[0]) * sizeOfStack));
    if (stack == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Stack.\n", fName);
	return NULL;
    }

    /* Success */
    return stack;
}

/**
 * StackInt: Deallocate
 */
int
ngiStackIntFree(ngiStackInt_t *stack, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(stack != NULL);

    /* Deallocate */
    globus_libc_free(stack);

    /* Success */
    return 1;
}

/**
 * Stack: Int: Initialize
 */
int
ngiStackIntInitialize(
    ngiStackInt_t *stack,
    int sizeOfStack,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(stack != NULL);
    assert(sizeOfStack > 0);

    /* Initialize */
    stack->ngsi_nData = sizeOfStack;
    stack->ngsi_current = 0;

    /* Success */
    return 1;
}

/**
 * Stack: Int: Finalize
 */
int
ngiStackIntFinalize(ngiStackInt_t *stack, ngLog_t *log, int *error)
{
    /* Do nothing */

    /* Success */
    return 1;
}

/**
 * Stack: Int: Push
 */
int
ngiStackIntPush(ngiStackInt_t *stack, int data, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiStackIntPush";

    /* Check the argument */
    assert(stack != NULL);
    assert(stack->ngsi_current >= 0);
    assert(stack->ngsi_current < stack->ngsi_nData);

    /* Is stack overflow? */
    if (stack->ngsi_current >= stack->ngsi_nData) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Stack is overflow.\n", fName);
	return 0;
    }

    /* Push the data */
    stack->ngsi_data[stack->ngsi_current] = data;
    stack->ngsi_current++;

    /* Success */
    return 1;
}

/**
 * Stack: Int: Pop
 */
int
ngiStackIntPop(ngiStackInt_t *stack, int *data, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiStackIntPop";

    /* Check the argument */
    assert(stack != NULL);
    assert(stack->ngsi_current > 0);
    assert(stack->ngsi_current <= stack->ngsi_nData);
    assert(data != NULL);

    /* Is stack underflow? */
    if (stack->ngsi_current <= 0) {
    	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Stack is underflow.\n", fName);
	return 0;
    }

    /* Pop the data */
    stack->ngsi_current--;
    *data = stack->ngsi_data[stack->ngsi_current];

    /* Success */
    return 1;
}

/**
 * Get the Argument Data.
 */
int
ngiGetArgumentData(
    ngArgumentDataType_t dataType,
    ngiArgumentPointer_t src,
    ngiArgumentPointer_t dest,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiGetArgumentData";

    /* Is argument NULL? */
    if (src.ngap_void == NULL) {
	/* Success */
	return 1;
    }

    switch (dataType) {
    case NG_ARGUMENT_DATA_TYPE_CHAR:
	*dest.ngap_char = *src.ngap_char;
	break;

    case NG_ARGUMENT_DATA_TYPE_SHORT:
	*dest.ngap_short = *src.ngap_short;
	break;

    case NG_ARGUMENT_DATA_TYPE_INT:
	*dest.ngap_int = *src.ngap_int;
	break;

    case NG_ARGUMENT_DATA_TYPE_LONG:
	*dest.ngap_long = *src.ngap_long;
	break;

    case NG_ARGUMENT_DATA_TYPE_FLOAT:
	*dest.ngap_float = *src.ngap_float;
	break;

    case NG_ARGUMENT_DATA_TYPE_DOUBLE:
	*dest.ngap_double = *src.ngap_double;
	break;

    case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
	*dest.ngap_scomplex = *src.ngap_scomplex;
	break;

    case NG_ARGUMENT_DATA_TYPE_DCOMPLEX:
	*dest.ngap_dcomplex = *src.ngap_dcomplex;
	break;

    case NG_ARGUMENT_DATA_TYPE_STRING:
    case NG_ARGUMENT_DATA_TYPE_FILENAME:
    case NG_ARGUMENT_DATA_TYPE_CALLBACK:
	/* Do nothing */
	break;

    default:
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Unknown data type %d.\n", fName, dataType);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the number of bytes of native data.
 */
int
ngiGetSizeofData(
    ngArgumentDataType_t dataType,
    size_t *nBytes,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiGetSizeofData";

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
	*nBytes = sizeof (char *);
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
    	"%s: Unknown data type %d.\n", fName, dataType);
    abort();
    return 0;
}

