#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngGlobus.c,v $ $Revision: 1.9 $ $Date: 2005/03/09 03:13:25 $";
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
 * Module for managing Communication for Ninf-G Client/Executable.
 */

#include <assert.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static int nglGlobusErrorTypeMatch(globus_result_t,
        const globus_object_type_t *, const char *, ngLog_t *, int *);

#define NGL_GLOBUS_ERROR_TYPE_MATCH(gResult, errorType, log, error) \
    (nglGlobusErrorTypeMatch((gResult), (errorType), (# errorType), (log), (error)))

/**
 * NOTE: These three functions uses globus_error_get().
 *   ngiGlobusIsIoEOF(), ngiGlobusIsCallbackCancel(), ngiGlobusError()
 * Thus, subsequent call causes an error.
 *  (2 times globus_error_get() for one globus_result_t.)
 * Please use each of one for one globus_result_t error code.
 */

/**
 * Is I/O EOF?
 */
int
ngiGlobusIsIoEOF(globus_result_t gResult, ngLog_t *log, int *error)
{
    int retResult;
    int local_error;
    static const char fName[] = "ngiGlobusIsIoEOF";

    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    retResult = NGL_GLOBUS_ERROR_TYPE_MATCH(gResult, 
        GLOBUS_IO_ERROR_TYPE_EOF,
        log, &local_error);

    if ((retResult == 0) &&
        (local_error != NG_ERROR_NO_ERROR)) {

        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL, "%s: "
            "Error type isn't \"EOF\"\n", fName);

        NGI_SET_ERROR(error, local_error);
    }

    return retResult;
}

/**
 * Is callback canceled?
 */
int
ngiGlobusIsCallbackCancel(globus_result_t gResult, ngLog_t *log, int *error)
{
    int retResult;
    int local_error;
    static const char fName[] = "ngiGlobusIsCallbackCancel";

    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    retResult = NGL_GLOBUS_ERROR_TYPE_MATCH(gResult, 
        GLOBUS_IO_ERROR_TYPE_IO_CANCELLED,
        log, &local_error);

    if ((retResult == 0) &&
        (local_error != NG_ERROR_NO_ERROR)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Error type isn't "
            "\"Is Canceled\"\n", fName);

        NGI_SET_ERROR(error, local_error);
    }

    return retResult;
}

#undef NGL_GLOBUS_ERROR_TYPE_MATCH

/**
 * Error Type Match?
 */
static int
nglGlobusErrorTypeMatch(
    globus_result_t gResult,
    const globus_object_type_t *errorType,
    const char * errorTypeString,
    ngLog_t *log,
    int *error)
{
    globus_bool_t bool;
    globus_object_t *object;
    const globus_object_type_t *objectType;
    static const char fName[] = "nglGlobusErrorTypeMatch";

    assert(errorTypeString != NULL);

    /* Success? */
    if (gResult == GLOBUS_SUCCESS) {
    	/* Do nothing. */
    	return 0;
    }

    /* Get the Error Object */
    object = globus_error_get(gResult);
    if (object == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't get Error Object.\n", fName);
	return 0;
    }

    /* Get the Object Type */
    objectType = globus_object_get_type(object);
    if (objectType == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't get Object Type.\n", fName);
	goto error;
    }

    /* Error Type Match ? */
    bool = globus_object_type_match(
	errorType, objectType);
    if (bool != GLOBUS_TRUE) {
        /* Error type was not matched */
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Error type doesn't match \"%s\".\n",
            fName, errorTypeString);
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngiGlobusErrorByObject(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
            fName, object, NULL);

        goto error;
    }

    /* Deallocate object */
    globus_object_free(object);

    /* Error type was matched */
    return 1;

    /* Error occurred */
error:

    /* Deallocate object */
    globus_object_free(object);

    return 0;
}

/**
 * Print the error message.
 */
int
ngiGlobusError(
    ngLog_t *log,
    ngLogCategory_t category,
    ngLogLevel_t level,
    const char *message,
    globus_result_t gResult,
    int *error)
{
    int result;
    globus_object_t *object;
    static const char fName[] = "ngiGlobusError";

    /* Get the error object */
    object = globus_error_get(gResult);
    if (object == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't get the Error Object.\n", fName);
	return 0;
    }

    result = ngiGlobusErrorByObject(
        log, category, level, message, object, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't output Globus Toolkit error message.\n", fName);
        goto error;
    }

    /* Deallocate objects */
    globus_object_free(object);

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Deallocate objects */
    globus_object_free(object);

    return 0;
}

/**
 * Print the error message from error object.
 */
int
ngiGlobusErrorByObject(
    ngLog_t *log,
    ngLogCategory_t category,
    ngLogLevel_t level,
    const char *message,
    globus_object_t *obj,
    int *error)
{
    char *msg;
    globus_object_t *nextObj, *curObj;
    static const char fName[] = "ngiGlobusErrorByObject";

    /* Check the arguments */
    if (obj == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Invalid argument.\n", fName);
	return 0;
    }

    for (curObj = obj; curObj != NULL; curObj = nextObj) {
    	/* Get the error message */
    	msg = globus_object_printable_to_string(curObj);
	if (msg != NULL) {
	    /* Print out the message */
	    ngLogPrintf(log, category, level, error, "%s: %s\n", message, msg);
	}

	/* Get next object */
	nextObj = curObj->parent_object;

	/* Deallocate error message */
	globus_libc_free(msg);
    }

    /* Success */
    return 1;
}
