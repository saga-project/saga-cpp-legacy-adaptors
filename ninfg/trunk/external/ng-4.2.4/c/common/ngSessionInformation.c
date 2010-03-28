#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngSessionInformation.c,v $ $Revision: 1.3 $ $Date: 2005/07/11 08:13:34 $";
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
 * Module for managing for Session Information.
 */

#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static void nglSessionInformationInitializeMember(ngSessionInformation_t *);
static void nglSessionInformationInitializePointer(ngSessionInformation_t *);

/**
 * Construct
 */
ngSessionInformation_t *
ngiSessionInformationConstruct(
    ngLog_t *log,
    int *error)
{
    int result;
    ngSessionInformation_t *info;
    static const char fName[] = "ngiSessionInformationConstruct";

    /* Allocate */
    info = ngiSessionInformationAllocate(log, error);
    if (info == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Session Information.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngiSessionInformationInitialize(info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Session Information.\n", fName);
	goto error;
    }

    /* Success */
    return info;

    /* Error occurred */
error:
    result = ngiSessionInformationFree(info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Session Information.\n",
	    fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Destruct
 */
int
ngiSessionInformationDestruct(
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiSessionInformationDestruct";

    /* Check the arguments */
    assert(info != NULL);

    /* Finalize */
    result = ngiSessionInformationFinalize(info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Session Information.\n", fName);
	return 0;
    }

    result = ngiSessionInformationFree(info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't deallocate the storage for Session Information.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngSessionInformation_t *
ngiSessionInformationAllocate(ngLog_t *log, int *error)
{
    ngSessionInformation_t *info;
    static const char fName[] = "ngiSessionInformationAllocate";

    info = globus_libc_calloc(1, sizeof (ngSessionInformation_t));
    if (info == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Session Information.\n", fName);
	return NULL;
    }

    /* Success */
    return info;
}

/**
 * Deallocate
 */
int
ngiSessionInformationFree(
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(info != NULL);

    /* Deallocate */
    globus_libc_free(info);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngiSessionInformationInitialize(
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(info != NULL);

    /* Initialize the Session Information */
    memset(info, 0, sizeof (*info));

    /* Initialize the members */
    nglSessionInformationInitializeMember(info);

    /* Success */
    return 1;
}

/**
 * Finalize
 */
int
ngiSessionInformationFinalize(
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiSessionInformationFinalize";

    /* Check the arguments */
    assert(info != NULL);

    /* Finish the measurement */
    result = ngiSessionInformationFinishMeasurement(info, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finish the measurement.\n", fName);
	return 0;
    }

    /* Initialize the members */
    nglSessionInformationInitializeMember(info);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglSessionInformationInitializeMember(ngSessionInformation_t *info)
{
    /* Check the arguments */
    assert(info != NULL);

    /* Initialize the pointers */
    nglSessionInformationInitializePointer(info);

    /* Initialize the members */
    info->ngsi_executableCallbackNtimesCalled = 0;
    info->ngsi_clientCallbackNtimesCalled = 0;
    info->ngsi_nCompressionInformations = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglSessionInformationInitializePointer(ngSessionInformation_t *info)
{
    /* Check the arguments */
    assert(info != NULL);

    /* Initialize the pointers */
    info->ngsi_compressionInformation = NULL;
}

/**
 * Start the measurement.
 */
int
ngiSessionInformationStartMeasurement(
    ngSessionInformation_t *info,
    int nArguments,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngiSessionInformationStartMeasurement";

    /* Check tha arguments */
    assert(info != NULL);
    assert(nArguments >= 0);
    assert(info->ngsi_compressionInformation == NULL);

    /* Initialize */
    result = ngiSessionInformationInitialize(info, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Session Information.\n", fName);
	return 0;
    }

    /* Allocate the storage for Compression Information */
    info->ngsi_nCompressionInformations = nArguments;
    if (nArguments > 0) {
	info->ngsi_compressionInformation = globus_libc_calloc(
	    nArguments, sizeof (*info->ngsi_compressionInformation));
	if (info->ngsi_compressionInformation == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL,
		"%s: Can't allocate the storage for Session Information.\n",
		fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Finish the measurement.
 */
int
ngiSessionInformationFinishMeasurement(
    ngSessionInformation_t *info,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(((info->ngsi_nCompressionInformations == 0) &&
	    (info->ngsi_compressionInformation == NULL)) ||
	   ((info->ngsi_nCompressionInformations > 0) &&
	    (info->ngsi_compressionInformation != NULL)));

    /* Deallocate */
    if (info->ngsi_compressionInformation != NULL)
	globus_libc_free(info->ngsi_compressionInformation);

    /* Initialize the variables */
    info->ngsi_nCompressionInformations = 0;
    info->ngsi_compressionInformation = NULL;

    /* Success */
    return 1;
}
