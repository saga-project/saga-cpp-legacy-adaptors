/*
 * $RCSfile: ngSessionInformation.c,v $ $Revision: 1.7 $ $Date: 2008/02/07 10:26:15 $
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
 * Module for managing for Session Information.
 */

#include "ngInternal.h"

NGI_RCSID_EMBED("$RCSfile: ngSessionInformation.c,v $ $Revision: 1.7 $ $Date: 2008/02/07 10:26:15 $")

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
    info = NGI_ALLOCATE(ngSessionInformation_t, log, error);
    if (info == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for Session Information.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngiSessionInformationInitialize(info, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Session Information.\n"); 
	goto error;
    }

    /* Success */
    return info;

    /* Error occurred */
error:
    result = NGI_DEALLOCATE(ngSessionInformation_t, info, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the storage for Session Information.\n"); 
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
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Session Information.\n"); 
	return 0;
    }

    result = NGI_DEALLOCATE(ngSessionInformation_t, info, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't deallocate the storage for Session Information.\n"); 
	return 0;
    }

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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finish the measurement.\n"); 
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

    /* Check the arguments */
    assert(info != NULL);
    assert(nArguments >= 0);
    assert(info->ngsi_compressionInformation == NULL);

    /* Initialize */
    result = ngiSessionInformationInitialize(info, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the Session Information.\n"); 
	return 0;
    }

    /* Allocate the storage for Compression Information */
    info->ngsi_nCompressionInformations = nArguments;
    if (nArguments > 0) {
	info->ngsi_compressionInformation = ngiCalloc(
	    nArguments, sizeof (*info->ngsi_compressionInformation),
	    log, error);
	if (info->ngsi_compressionInformation == NULL) {
	    NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't allocate the storage for Session Information.\n"); 
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
	ngiFree(info->ngsi_compressionInformation, log, error);

    /* Initialize the variables */
    info->ngsi_nCompressionInformations = 0;
    info->ngsi_compressionInformation = NULL;

    /* Success */
    return 1;
}
