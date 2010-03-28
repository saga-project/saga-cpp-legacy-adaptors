/*
 * $RCSfile: ngclArgumentStack.c,v $ $Revision: 1.6 $ $Date: 2007/09/26 04:14:05 $
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
 * Module of Argument Stack for Ninf-G Client.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclArgumentStack.c,v $ $Revision: 1.6 $ $Date: 2007/09/26 04:14:05 $")

/**
 * Prototype declaration of internal functions.
 */
static void ngcllArgumentStackInitializeMember(ngclArgumentStack_t *);
static void ngcllArgumentStackInitializePointer(ngclArgumentStack_t *);
static ngclArgumentStack_t *ngcllArgumentStackConstruct(
    ngclContext_t *, int, int *);
static ngclArgumentStack_t *ngcllArgumentStackAllocate(
    ngclContext_t *, int, int *);
static int ngcllArgumentStackFree(
    ngclContext_t *, ngclArgumentStack_t *, int *);
static int ngcllArgumentStackInitialize(
    ngclContext_t *, ngclArgumentStack_t *, int, int *);
static int ngcllArgumentStackFinalize(
    ngclContext_t *, ngclArgumentStack_t *, int *);


/**
 * Construct
 */
ngclArgumentStack_t *
ngclArgumentStackConstruct(
    ngclContext_t *context,
    int nArguments,
    int *error)
{
    int local_error, result;
    ngclArgumentStack_t *stack;
    static const char fName[] = "ngclArgumentStackConstruct";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Context is not valid.\n"); 
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    stack = ngcllArgumentStackConstruct(context, nArguments, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return stack;
}

static ngclArgumentStack_t *
ngcllArgumentStackConstruct(
    ngclContext_t *context,
    int nArguments,
    int *error)
{
    int result;
    ngclArgumentStack_t *stack;
    static const char fName[] = "ngcllArgumentStackConstruct";

    /* Is nArguments valid? */
    if (nArguments < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Invalid argument.\n"); 
	return NULL;
    }

    /* Allocate */
    stack = ngcllArgumentStackAllocate(context, nArguments, error);
    if (stack == NULL) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Argument Stack.\n"); 
	return NULL;
    }

    /* Initialize */
    result = ngcllArgumentStackInitialize(context, stack, nArguments, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Argument Stack.\n"); 
        goto error;
    }

    /* Success */
    return stack;

    /* Error occurred */
error:
    result = ngcllArgumentStackFree(context, stack, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't deallocate the Argument Stack.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngclArgumentStackDestruct(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    int *error)
{
    int result;
    static const char fName[] = "ngclArgumentStackDestruct";

    /* Finalize */
    result = ngcllArgumentStackFinalize(context, stack, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Argument Stack.\n"); 
        return 0;
    }

    /* Deallocate */
    result = ngcllArgumentStackFree(context, stack, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't deallocate the Argument Stack.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngclArgumentStack_t *
ngcllArgumentStackAllocate(
    ngclContext_t *context,
    int nArguments,
    int *error)
{
    ngclArgumentStack_t *stack;
    static const char fName[] = "ngcllArgumentStackAllocate";

    /* Allocate */
    stack = ngiCalloc(1,
    	sizeof (ngclArgumentStack_t)
	+ (sizeof (stack->ngas_argp) * (nArguments - 1)),
	context->ngc_log, error);
    if (stack == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Argument Stack.\n"); 
	return NULL;
    }

    /* Success */
    return stack;
}

/**
 * Deallocate
 */
static int
ngcllArgumentStackFree(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    int *error)
{
    /* Deallocate */
    ngiFree(stack, context->ngc_log, error);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
static int
ngcllArgumentStackInitialize(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    int nArguments,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the members */
    ngcllArgumentStackInitializeMember(stack);
    stack->ngas_nargs = nArguments;
    stack->ngas_index = 0;

    /* Success */
    return 1;
}

/**
 * Finalize
 */
static int
ngcllArgumentStackFinalize(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    int *error)
{
    /* Initialize the members */
    ngcllArgumentStackInitializeMember(stack);

    /* Success */
    return 1;
}

/**
 * Initialize the member.
 */
static void
ngcllArgumentStackInitializeMember(
    ngclArgumentStack_t *stack)
{
    /* Check the argument */
    assert(stack != NULL);

    /* Initialize the pointers */
    ngcllArgumentStackInitializePointer(stack);
    stack->ngas_nargs = 0;
    stack->ngas_index = 0;
}

/**
 * Initialize the pointer.
 */
static void
ngcllArgumentStackInitializePointer(
    ngclArgumentStack_t *stack)
{
    /* Check the argument */
    assert(stack != NULL);

    /* Initialize the pointers */
    ; /* Do nothing */
}

/**
 * Push the argument.
 */
int
ngclArgumentStackPush(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    void *arg,
    int *error)
{
    static const char fName[] = "ngclArgumentStackPush";

    /* Is stack NULL? */
    if (stack == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The stack is NULL.\n"); 
	return 0;
    }

    /* Is arg NULL? */
    if (arg == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The arg is NULL.\n"); 
	return 0;
    }

    /* Is stack overflow? */
    if (stack->ngas_index >= stack->ngas_nargs) {
    	NGI_SET_ERROR(error, NG_ERROR_OVERFLOW);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Overflow the Argument Stack.\n"); 
	return 0;
    }

    /* Push the argument */
    stack->ngas_argp[stack->ngas_index] = arg;
    stack->ngas_index++;

    /* Success */
    return 1;
}

/**
 * Pop the argument.
 */
void *
ngclArgumentStackPop(
    ngclContext_t *context,
    ngclArgumentStack_t *stack,
    int *error)
{
    static const char fName[] = "ngclArgumentStackPop";

    /* Is stack NULL? */
    if (stack == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "The stack is NULL.\n"); 
	return 0;
    }

    /* Is stack underflow? */
    if (stack->ngas_index <= 0) {
    	NGI_SET_ERROR(error, NG_ERROR_UNDERFLOW);
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	    "Underflow the Argument Stack.\n"); 
	return 0;
    }

    /* Pop the argument */
    stack->ngas_index--;
    return stack->ngas_argp[stack->ngas_index];
}
