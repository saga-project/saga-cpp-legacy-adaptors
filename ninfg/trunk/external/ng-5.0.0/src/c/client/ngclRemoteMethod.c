/*
 * $RCSfile: ngclRemoteMethod.c,v $ $Revision: 1.7 $ $Date: 2007/11/27 02:27:41 $
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
 * Remote Method Information modules for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclRemoteMethod.c,v $ $Revision: 1.7 $ $Date: 2007/11/27 02:27:41 $")

/**
 * Prototype declaration of static functions.
 */
static void ngcllRemoteMethodInformationInitializeMember(
    ngRemoteMethodInformation_t *);
static void ngcllRemoteMethodInformationInitializePointer(
    ngRemoteMethodInformation_t *);
static int ngcllArgumentInformationFree(
    ngclContext_t *, ngArgumentInformation_t *, int *);
static int ngcllArgumentInformationCopy(ngclContext_t *,
    ngArgumentInformation_t *, ngArgumentInformation_t *, int *);
static int ngcllArgumentInformationRelease(
    ngclContext_t *, ngArgumentInformation_t *, int *);
static void ngcllArgumentInformationInitializeMember(
    ngArgumentInformation_t *);
static void ngcllArgumentInformationInitializePointer(
    ngArgumentInformation_t *);
static int ngcllSubscriptInformationFree(
    ngclContext_t *, ngSubscriptInformation_t *, int *);
static int ngcllSubscriptInformationCopy(ngclContext_t *,
    ngSubscriptInformation_t *, ngSubscriptInformation_t *, int *);
static int ngcllSubscriptInformationRelease(
    ngclContext_t *, ngSubscriptInformation_t *, int *);
static void ngcllSubscriptInformationInitializeMember(
    ngSubscriptInformation_t *);
static void ngcllSubscriptInformationInitializePointer(
    ngSubscriptInformation_t *);
static int ngcllExpressionElementFree(
    ngclContext_t *, ngExpressionElement_t *, int *);
static int ngcllExpressionElementCopy(ngclContext_t *,
    ngExpressionElement_t *, ngExpressionElement_t *, int *);
static int ngcllExpressionElementRelease(
    ngclContext_t *, ngExpressionElement_t *, int *);
static void ngcllExpressionElementInitializeMember(
    ngExpressionElement_t *);
static void ngcllExpressionElementInitializePointer(
    ngExpressionElement_t *);
static int ngcllExpressionElementGetSize(
    ngclContext_t *, ngExpressionElement_t *, int *, int *);


/**
 * Get the information by method name.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngRemoteMethodInformation_t *
ngcliRemoteMethodInformationCacheGet(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    char *methodName,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationCacheGet";
    int result;
    int i;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	    ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	        "Ninf-G Context is not valid.\n"); 
    }

    /* Check the arguments */
    if (methodName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "methodName is NULL.\n"); 
	return NULL;
    }

    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
    	assert(rcInfo->ngrci_method[i].ngrmi_methodName != NULL);
	if (strcmp(rcInfo->ngrci_method[i].ngrmi_methodName, methodName) == 0) {
	    /* Found */
	    return &rcInfo->ngrci_method[i];
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Remote Method Information is not found by method name\"%s\".\n",
        methodName); 

    return NULL;
}

/**
 * Get the next information.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngRemoteMethodInformation_t *
ngcliRemoteMethodInformationCacheGetNext(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    ngRemoteMethodInformation_t *current,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationCacheGetNext";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
	    ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	        "Ninf-G Context is not valid.\n"); 
	return 0;
    }

    /* Check the arguments */
    assert(rcInfo->ngrci_nMethods > 0);

    if (current == NULL) {
	/* Return the first information */
        return &rcInfo->ngrci_method[0];
    } else {
	/* Return the next information */
        assert(current >= rcInfo->ngrci_method);
        if (current < &rcInfo->ngrci_method[rcInfo->ngrci_nMethods - 1]) {
	    return current + 1;
	}
    }

    /* Not found */
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "The last Remote Method Information was reached.\n"); 

    return NULL;
}

/**
 * GetCopy.
 */
int
ngcliRemoteMethodInformationGetCopy(
    ngclContext_t *context,
    ngRemoteClassInformation_t *rcInfo,
    char *methodName,
    ngRemoteMethodInformation_t *method,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationGetCopy";
    int result, i;

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if ((rcInfo == NULL) || (methodName == NULL) || (method == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    assert(rcInfo->ngrci_method != NULL);
    for (i = 0; i < rcInfo->ngrci_nMethods; i++) {
        assert(rcInfo->ngrci_method[i].ngrmi_methodName != NULL);
        if (strcmp(methodName, rcInfo->ngrci_method[i].ngrmi_methodName)
            == 0) {
            /* Found */
            result = ngcliRemoteMethodInformationCopy(
                context, &rcInfo->ngrci_method[i], method, error);
            if (result == 0) {
                ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy the Remote Method Information\n"); 
                return 0;
            }

            /* Success */
            return 1;
            
        }
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogInfoContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "Can't get Remote Method Information.\n"); 
    return 0;
}

/**
 * Allocate.
 */
ngRemoteMethodInformation_t *
ngcliRemoteMethodInformationAllocate(
    ngclContext_t *context,
    int size,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationAllocate";
    ngRemoteMethodInformation_t *rMethods;
    ngLog_t *log;
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (size <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    /* Allocate */
    rMethods = (ngRemoteMethodInformation_t *)
        ngiCalloc(size, sizeof(ngRemoteMethodInformation_t), log, error);
    if (rMethods == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for the Remote Methods.\n"); 
        return NULL;
    }

    /* Success */
    return rMethods;
}

/**
 * Deallocate.
 */
int
ngcliRemoteMethodInformationFree(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationFree";
    ngLog_t *log;
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;
    ngiFree(rMethod, log, error);

    /* Success */
    return 1;
}

/**
 * Copy.
 */
int
ngcliRemoteMethodInformationCopy(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *src,
    ngRemoteMethodInformation_t *dest,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationCopy";
    int elementSize;
    ngLog_t *log;
    int result;
    int i;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* check the arguments */
    if ((src == NULL) || (dest == NULL)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "RemoteMethod is NULL.\n"); 
        return 0;
    }
    assert(src->ngrmi_methodName != NULL);
    assert(src->ngrmi_methodID >= 0);
    assert(src->ngrmi_calculationOrder != NULL);
    assert(src->ngrmi_nArguments >= 0);
    assert((src->ngrmi_nArguments > 0) ?
        (src->ngrmi_arguments != NULL) : (src->ngrmi_arguments == NULL));

    /* Initialize the members */
    ngcllRemoteMethodInformationInitializeMember(dest);

    /* Copy values */
    dest->ngrmi_methodID = src->ngrmi_methodID;
    dest->ngrmi_nArguments = src->ngrmi_nArguments;
    dest->ngrmi_shrink = src->ngrmi_shrink;

    /* Copy the strings */
#define NGL_COPY_STRING(src, dest, member) \
    do { \
        assert((src)->member != NULL); \
        (dest)->member = ngiStrdup((src)->member, log, error); \
        if ((dest)->member == NULL) { \
            ngclLogErrorContext(context, \
                NG_LOGCAT_NINFG_PURE, fName,  \
                "Can't allocate the storage " \
                "for Remote Method Information.\n"); \
            goto error; \
        } \
    } while(0)
#define  NGL_COPY_STRING_IF_VALID(str, dest, member) \
    do {\
        if ((src)->member != NULL) { \
            NGL_COPY_STRING(str, dest, member); \
        } \
    } while (0)
    
    NGL_COPY_STRING(src, dest, ngrmi_methodName); 
    NGL_COPY_STRING_IF_VALID(src, dest, ngrmi_description);

#undef NGL_COPY_STRING

    /* Copy calculation order */
    result = ngcllExpressionElementGetSize(
        context, src->ngrmi_calculationOrder, &elementSize, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get ExpressionElement size.\n"); 
        goto error;
    }

    dest->ngrmi_calculationOrder =
        ngcliExpressionElementAllocate(context, elementSize, error);
    if (dest->ngrmi_calculationOrder == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate ExpressionElement.\n"); 
        goto error;
    }

    for (i = 0; i < elementSize; i++) {
        result = ngcllExpressionElementCopy(context,
            &src->ngrmi_calculationOrder[i],
            &dest->ngrmi_calculationOrder[i], error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy ExpressionElement.\n"); 
            goto error;
        }
    }
    
    /* Copy arguments */
    if (src->ngrmi_nArguments > 0) {
        dest->ngrmi_arguments = ngcliArgumentInformationAllocate(
            context, src->ngrmi_nArguments, error);
        if (dest->ngrmi_arguments == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate ArgumentInformation.\n"); 
            goto error;
        }

        for (i = 0; i < src->ngrmi_nArguments; i++) {
            result = ngcllArgumentInformationCopy(context,
                &src->ngrmi_arguments[i],
                &dest->ngrmi_arguments[i], error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy ArgumentInformation.\n"); 
                goto error;
            }
        }
    } else {
        dest->ngrmi_arguments = NULL;
    }

    assert(dest->ngrmi_methodName != NULL);
    assert(dest->ngrmi_methodID >= 0);
    assert(dest->ngrmi_calculationOrder != NULL);
    assert(dest->ngrmi_nArguments >= 0);
    assert((dest->ngrmi_nArguments > 0) ?
        (dest->ngrmi_arguments != NULL) : (dest->ngrmi_arguments == NULL));

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Release */
    if (dest->ngrmi_methodName != NULL) {
        ngiFree(dest->ngrmi_methodName, log, NULL);
    }

    if (dest->ngrmi_description != NULL) {
        ngiFree(dest->ngrmi_description, log, NULL);
    }

    if (dest->ngrmi_calculationOrder != NULL) {
        ngiFree(dest->ngrmi_calculationOrder, log, NULL);
    }

    if (dest->ngrmi_arguments != NULL) {
        ngiFree(dest->ngrmi_arguments, log, NULL);
    }

    /* Failed */
    return 0;
}

/**
 * Release.
 */
int
ngcliRemoteMethodInformationRelease(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationRelease";
    int elementSize;
    ngLog_t *log;
    int result;
    int i;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (rMethod == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "RemoteMethod is NULL.\n"); 
	return 0;
    }

    /* Release ArgumentInformation */
    for (i = 0; i < rMethod->ngrmi_nArguments; i++) {
	result = ngcllArgumentInformationRelease(
	    context, &rMethod->ngrmi_arguments[i], error);
    	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release ArgumentInformation.\n"); 
	    return 0;
	}
    }

    /* Release CalculationOrder */
    assert(rMethod->ngrmi_calculationOrder != NULL);
    result = ngcllExpressionElementGetSize(
        context, rMethod->ngrmi_calculationOrder, &elementSize, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get ExpressionElement size.\n"); 
        return 0;
    }
    for (i = 0; i < elementSize; i++) {
	result = ngcllExpressionElementRelease(
	    context, &rMethod->ngrmi_calculationOrder[i], error);
    	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release ExpressionElement.\n"); 
	    return 0;
	}
    }

    /* Deallocate the members */
    ngiFree(rMethod->ngrmi_methodName, log, error);
    if (rMethod->ngrmi_description != NULL) {
        ngiFree(rMethod->ngrmi_description, log, error);
    }

    if (rMethod->ngrmi_nArguments > 0) {
        result = ngcllArgumentInformationFree(
            context, rMethod->ngrmi_arguments, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't free ArgumentInformation.\n"); 
            return 0;
        }
    }

    result = ngcllExpressionElementFree(
        context, rMethod->ngrmi_calculationOrder, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't free ExpressionElement.\n"); 
        return 0;
    }

    /* Initialize members */
    ngcllRemoteMethodInformationInitializeMember(rMethod);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliRemoteMethodInformationInitialize(
    ngclContext_t *context,
    ngRemoteMethodInformation_t *rMethod,
    int *error)
{
    static const char fName[] = "ngcliRemoteMethodInformationInitialize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (rMethod == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllRemoteMethodInformationInitializeMember(rMethod);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllRemoteMethodInformationInitializeMember(
    ngRemoteMethodInformation_t *rMethod)
{
    /* Initialize the members */
    ngcllRemoteMethodInformationInitializePointer(rMethod);
    rMethod->ngrmi_methodID = 0;
    rMethod->ngrmi_nArguments = 0;
    rMethod->ngrmi_shrink = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllRemoteMethodInformationInitializePointer(
    ngRemoteMethodInformation_t *rMethod)
{
    /* Initialize the pointers */
    rMethod->ngrmi_methodName = NULL;
    rMethod->ngrmi_calculationOrder = NULL;
    rMethod->ngrmi_arguments = NULL;
    rMethod->ngrmi_description = NULL;
}

/**
 * ArgumentInformation
 */

/**
 * Allocate.
 */
ngArgumentInformation_t *
ngcliArgumentInformationAllocate(
    ngclContext_t *context,
    int size,
    int *error)
{   
    static const char fName[] = "ngcliArgumentInformationAllocate";
    ngArgumentInformation_t *arguments;
    ngLog_t *log;
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (size <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    arguments = (ngArgumentInformation_t *)
        ngiCalloc(size, sizeof(ngArgumentInformation_t), log, error);
    if (arguments == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for the Argument Informations.\n"); 
        return NULL;
    }

    /* Success */
    return arguments;
}

/**
 * Deallocate.
 */
static int
ngcllArgumentInformationFree(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    int *error)
{   
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(argument != NULL);

    log = context->ngc_log;

    ngiFree(argument, log, error);

    /* Success */
    return 1;
}

/**
 * Copy.
 */
static int
ngcllArgumentInformationCopy(
    ngclContext_t *context,
    ngArgumentInformation_t *src,
    ngArgumentInformation_t *dest,
    int *error)
{
    static const char fName[] = "ngcllArgumentInformationCopy";
    int result;
    int i;

    /* check the arguments */
    assert(context != NULL);
    assert(src != NULL);
    assert(dest != NULL);

    assert(src->ngai_nDimensions >= 0);
    assert((src->ngai_nDimensions > 0) ?
        (src->ngai_subscript != NULL) : (src->ngai_subscript == NULL));
    assert((src->ngai_callback != NULL) ? (src->ngai_nDimensions == 0) : 1);

    /* Initialize the members */
    ngcllArgumentInformationInitializeMember(dest);

    /* Copy values */
    dest->ngai_ioMode = src->ngai_ioMode;
    dest->ngai_dataType = src->ngai_dataType;
    dest->ngai_nDimensions = src->ngai_nDimensions;

    /* Copy SubscriptInformation */
    if (src->ngai_nDimensions > 0) {
        dest->ngai_subscript = ngcliSubscriptInformationAllocate(
            context, src->ngai_nDimensions, error);
        if (dest->ngai_subscript == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate SubscriptInformation.\n"); 
            return 0;
        }

        for (i = 0; i < src->ngai_nDimensions; i++) {
            result = ngcllSubscriptInformationCopy(context,
                &src->ngai_subscript[i],
                &dest->ngai_subscript[i], error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy SubscriptInformation.\n"); 
                return 0; 
            }
        }
    } else {
        dest->ngai_subscript = NULL;
    }

    /* Copy Callback Information */
    if (src->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
        assert(src->ngai_callback != NULL);

        dest->ngai_callback = ngcliRemoteMethodInformationAllocate(
            context, 1, error);
        if (dest->ngai_callback == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate Callback Information.\n"); 
            return 0;
        }

        result = ngcliRemoteMethodInformationCopy(context,
            src->ngai_callback, dest->ngai_callback, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't copy Callback Information.\n"); 
            return 0; 
        }
    } else {
        dest->ngai_callback = NULL;
    }

    assert(dest->ngai_nDimensions >= 0);
    assert((dest->ngai_nDimensions > 0) ?
        (dest->ngai_subscript != NULL) : (dest->ngai_subscript == NULL));
    assert((dest->ngai_callback != NULL) ? (dest->ngai_nDimensions == 0) : 1);

    /* Success */
    return 1;
}

/**
 * Release.
 */
static int
ngcllArgumentInformationRelease(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    int *error)
{
    static const char fName[] = "ngcllArgumentInformationRelease";
    int result;
    int i;

    /* Check the arguments */
    assert(context != NULL);
    assert(argument != NULL);

    assert(argument->ngai_nDimensions >= 0);
    assert((argument->ngai_nDimensions > 0) ?
        (argument->ngai_subscript != NULL) :
        (argument->ngai_subscript == NULL));
    assert((argument->ngai_callback != NULL) ?
        (argument->ngai_nDimensions == 0) : 1);

    /* Release SubscriptInformation */
    if (argument->ngai_nDimensions > 0) {
        assert(argument->ngai_subscript != NULL);

        for (i = 0; i < argument->ngai_nDimensions; i++) {

            result = ngcllSubscriptInformationRelease(context,
                &argument->ngai_subscript[i], error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't release SubscriptInformation.\n"); 
                return 0;
            }
        }
    }

    /* Release Callback Information */
    if (argument->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
        assert(argument->ngai_callback != NULL);

	result = ngcliRemoteMethodInformationRelease(
	    context, argument->ngai_callback, error);
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release Callback Information.\n"); 
	    return 0;
	}
    }

    /* Deallocate the members */
    if (argument->ngai_nDimensions > 0) {
        result = ngcllSubscriptInformationFree(
            context, argument->ngai_subscript, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't free SubscriptInformation.\n"); 
            return 0;
        }
    }

    if (argument->ngai_dataType == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
        result = ngcliRemoteMethodInformationFree(
            context, argument->ngai_callback, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't free Callback Information.\n"); 
            return 0;
        }
    }

    /* Initialize members */
    ngcllArgumentInformationInitializeMember(argument);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliArgumentInformationInitialize(
    ngclContext_t *context,
    ngArgumentInformation_t *argument,
    int *error)
{
    static const char fName[] = "ngcliArgumentInformationInitialize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (argument == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    ngcllArgumentInformationInitializeMember(argument);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllArgumentInformationInitializeMember(
    ngArgumentInformation_t *argument)
{
    /* Initialize the members */
    ngcllArgumentInformationInitializePointer(argument);
    argument->ngai_ioMode = NG_ARGUMENT_IO_MODE_NONE;
    argument->ngai_dataType = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
    argument->ngai_nDimensions = 0; 
}

/**
 * Initialize the pointers.
 */
static void
ngcllArgumentInformationInitializePointer(
    ngArgumentInformation_t *argument)
{
    /* Initialize the pointers */
    argument->ngai_subscript = NULL;
    argument->ngai_callback = NULL;
}

/**
 * SubscriptInformation
 */

/**
 * Allocate.
 */
ngSubscriptInformation_t *
ngcliSubscriptInformationAllocate(
    ngclContext_t *context,
    int size,
    int *error)
{   
    static const char fName[] = "ngcliSubscriptInformationAllocate";
    ngSubscriptInformation_t *subscripts;
    ngLog_t *log;
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (size <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    subscripts = (ngSubscriptInformation_t *)
        ngiCalloc(size, sizeof(ngSubscriptInformation_t), log, error);
    if (subscripts == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for the Subscript Informations.\n"); 
        return NULL;
    }

    /* Success */
    return subscripts;
}

/**
 * Deallocate.
 */
static int
ngcllSubscriptInformationFree(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    int *error)
{   
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(subscript != NULL);

    log = context->ngc_log;

    ngiFree(subscript, log, error);

    /* Success */
    return 1;
}

/**
 * Copy.
 */
static int
ngcllSubscriptInformationCopy(
    ngclContext_t *context,
    ngSubscriptInformation_t *src,
    ngSubscriptInformation_t *dest,
    int *error)
{
    static const char fName[] = "ngcllSubscriptInformationCopy";
    ngExpressionElement_t *srcElems[4], **destElems[4];
    ngExpressionElement_t *srcElem, **destElem;
    int elementSize;
    int result;
    int i, j;

    /* Check the arguments */
    assert(context != NULL);
    assert(src != NULL);
    assert(dest != NULL);

    assert(src->ngsi_size != NULL);
    assert(src->ngsi_start != NULL);
    assert(src->ngsi_end != NULL);
    assert(src->ngsi_skip != NULL);

    /* Initialize the members */
    ngcllSubscriptInformationInitializeMember(dest);

    /* Copy ExpressionElement arrays */
    srcElems[0] = src->ngsi_size;
    srcElems[1] = src->ngsi_start;
    srcElems[2] = src->ngsi_end;
    srcElems[3] = src->ngsi_skip;
    destElems[0] = &dest->ngsi_size;
    destElems[1] = &dest->ngsi_start;
    destElems[2] = &dest->ngsi_end;
    destElems[3] = &dest->ngsi_skip;

    for (i = 0; i < 4; i++) {
        srcElem = srcElems[i];
        destElem = destElems[i];

        result = ngcllExpressionElementGetSize(
            context, srcElem, &elementSize, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get ExpressionElement size.\n"); 
            return 0;
        }

        *destElem = ngcliExpressionElementAllocate(context, elementSize, error);
        if (*destElem == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate ExpressionElement.\n"); 
            return 0;
        }

        for (j = 0; j < elementSize; j++) {
            result = ngcllExpressionElementCopy(context,
                &srcElem[j], &(*destElem)[j], error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't copy ExpressionElement.\n"); 
                return 0; 
            }
        }
    }

    assert(dest->ngsi_size != NULL);
    assert(dest->ngsi_start != NULL);
    assert(dest->ngsi_end != NULL);
    assert(dest->ngsi_skip != NULL);

    /* Success */
    return 1;
}

/**
 * Release.
 */
static int
ngcllSubscriptInformationRelease(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    int *error)
{
    static const char fName[] = "ngcllSubscriptInformationRelease";
    ngExpressionElement_t *elems[4];
    ngExpressionElement_t *elem;
    int elementSize;
    int result;
    int i, j;


    /* Check the arguments */
    assert(context != NULL);
    assert(subscript != NULL);

    assert(subscript->ngsi_size != NULL);
    assert(subscript->ngsi_start != NULL);
    assert(subscript->ngsi_end != NULL);
    assert(subscript->ngsi_skip != NULL);

    elems[0] = subscript->ngsi_size;
    elems[1] = subscript->ngsi_start;
    elems[2] = subscript->ngsi_end;
    elems[3] = subscript->ngsi_skip;

    /* Release ExpressionElement arrays */
    for (i = 0; i < 4; i++) {
        elem = elems[i];

        result = ngcllExpressionElementGetSize(
            context, elem, &elementSize, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get ExpressionElement size.\n"); 
            return 0;
        }

        for (j = 0; j < elementSize; j++) {
            result = ngcllExpressionElementRelease(context, &elem[j], error);
            if (result == 0) {
                ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't release ExpressionElement.\n"); 
                return 0; 
            }
        }
    }

    /* Deallocate the members */
    for (i = 0; i < 4; i++) {
        elem = elems[i];
        result = ngcllExpressionElementFree(
            context, elem, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't free ExpressionElement.\n"); 
            return 0;
        }
    }

    /* Initialize members */
    ngcllSubscriptInformationInitializeMember(subscript);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliSubscriptInformationInitialize(
    ngclContext_t *context,
    ngSubscriptInformation_t *subscript,
    int *error)
{
    static const char fName[] = "ngcliSubscriptInformationInitialize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (subscript == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid SubscriptInformation.\n"); 
        return 0;
    }

    ngcllSubscriptInformationInitializeMember(subscript);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllSubscriptInformationInitializeMember(
    ngSubscriptInformation_t *subscript)
{
    /* Initialize the members */
    ngcllSubscriptInformationInitializePointer(subscript);
}

/**
 * Initialize the pointers.
 */
static void
ngcllSubscriptInformationInitializePointer(
    ngSubscriptInformation_t *subscript)
{
    /* Initialize the pointers */
    subscript->ngsi_size = NULL;
    subscript->ngsi_start = NULL;
    subscript->ngsi_end = NULL;
    subscript->ngsi_skip = NULL;
}

/**
 * ExpressionElement
 */

/**
 * Allocate.
 */
ngExpressionElement_t *
ngcliExpressionElementAllocate(
    ngclContext_t *context,
    int size,
    int *error)
{   
    static const char fName[] = "ngcliExpressionElementAllocate";
    ngExpressionElement_t *expElems;
    ngLog_t *log;
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    log = context->ngc_log;

    /* Check the arguments */
    if (size <= 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid argument.\n"); 
        return 0;
    }

    expElems = (ngExpressionElement_t *)
        ngiCalloc(size, sizeof(ngExpressionElement_t), log, error);
    if (expElems == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for the Expression Elements.\n"); 
        return NULL;
    }

    /* Success */
    return expElems;
}

/**
 * Deallocate.
 */
static int
ngcllExpressionElementFree(
    ngclContext_t *context,
    ngExpressionElement_t *expElem,
    int *error)
{   
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(expElem != NULL);

    log = context->ngc_log;

    ngiFree(expElem, log, error);

    /* Success */
    return 1;
}

/**
 * Copy.
 */
static int
ngcllExpressionElementCopy(
    ngclContext_t *context,
    ngExpressionElement_t *src,
    ngExpressionElement_t *dest,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(src != NULL);
    assert(dest != NULL);

    /* Initialize the members */
    ngcllExpressionElementInitializeMember(dest);

    /* Copy values */
    dest->ngee_valueType = src->ngee_valueType;
    dest->ngee_value = src->ngee_value;

    /* Success */
    return 1;
}

/**
 * Release.
 */
static int
ngcllExpressionElementRelease(
    ngclContext_t *context,
    ngExpressionElement_t *expElem,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(expElem != NULL);

    /* Initialize members */
    ngcllExpressionElementInitializeMember(expElem);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
int
ngcliExpressionElementInitialize(
    ngclContext_t *context,
    ngExpressionElement_t *expElem,
    int *error)
{
    static const char fName[] = "ngcliExpressionElementInitialize";
    int result;

    /* Is context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
        ngLogFatal(NULL, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
        return 0;
    }

    /* Check the arguments */
    if (expElem == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid expElem.\n"); 
        return 0;
    }

    ngcllExpressionElementInitializeMember(expElem);

    /* Success */
    return 1;
}

/**
 * Initialize the variable of members.
 */
static void
ngcllExpressionElementInitializeMember(
    ngExpressionElement_t *expElem)
{
    /* Initialize the members */
    ngcllExpressionElementInitializePointer(expElem);
    expElem->ngee_valueType = NG_EXPRESSION_VALUE_TYPE_NONE;
    expElem->ngee_value = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllExpressionElementInitializePointer(
    ngExpressionElement_t *expElem)
{
    /* Initialize the pointers */
}

/**
 * Get size of Expression Element array
 */
static int
ngcllExpressionElementGetSize(
    ngclContext_t *context,
    ngExpressionElement_t *expElem,
    int *elementCount,
    int *error)
{
    static const char fName[] = "ngcllExpressionElementGetSize";
    int type, i;

    /* Check the arguments */
    assert(context != NULL);
    assert(expElem != NULL);
    assert(elementCount != NULL);

    *elementCount = -1;

    i = 0;
    while (expElem[i].ngee_valueType != NG_EXPRESSION_VALUE_TYPE_END) {
        type = expElem[i].ngee_valueType;

        /* Validity check */
        if (!((type == NG_EXPRESSION_VALUE_TYPE_NONE) ||
              (type == NG_EXPRESSION_VALUE_TYPE_CONSTANT) ||
              (type == NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT) ||
              (type == NG_EXPRESSION_VALUE_TYPE_OPCODE) ||
              (type == NG_EXPRESSION_VALUE_TYPE_END))) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Invalid Expression Element.\n"); 
            return 0;
        }

        i++;
    }
    *elementCount = i + 1; /* return size of expElem */
    
    /* Success */
    return 1;
}

