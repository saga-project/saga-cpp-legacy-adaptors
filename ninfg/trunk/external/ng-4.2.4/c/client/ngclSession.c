#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclSession.c,v $ $Revision: 1.119 $ $Date: 2006/04/07 10:08:39 $";
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
 * Module of Session manager for Ninf-G Client.
 */

#include <stdlib.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static void ngcllSessionInitializeMember(ngclSession_t *);
static void ngcllSessionInitializeFlag(ngclSession_t *);
static void ngcllSessionInitializePointer(ngclSession_t *);
static int ngcllSessionCreateID(ngclContext_t *, int *);
static int ngcllSessionCancelAllSession(
    ngclContext_t *, ngclSession_t *, ngLog_t *, int *);
static int ngcllSessionCancelSingle(ngclSession_t *, ngLog_t *, int *);
static ngclSession_t *ngcllSessionConstruct(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);
static ngclSession_t *ngcllSessionAllocate(
    ngclContext_t *, ngclExecutable_t *, int *);
static int ngcllSessionFree(ngclSession_t *, int *);
static int ngcllSessionInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngclSessionAttribute_t *, int *);
static int ngcllSessionFinalize(ngclSession_t *, int *);
static int ngcllSessionRegister(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
static int ngcllSessionRegisterUserData(
    ngclSession_t *, void *, void (*)(ngclSession_t *), int *);
static int ngcllSessionUnregisterUserData(ngclSession_t *, int *);
static int ngcllSessionGetUserData(ngclSession_t *, void **, int *);
static ngclExecutable_t *ngcllSessionGetExecutable(ngclSession_t *, int *);
static ngclSession_t *ngcllSessionGetNext(
    ngclExecutable_t *, ngclSession_t *, int *);
static int ngcllSessionStart(
    ngclSession_t *, char *, ngclArgumentStack_t *, va_list, int *);
static int ngcllSessionSuspend(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
static int ngcllSessionResume(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *, int *);
static int ngcllSessionAttributeInitialize(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);
static void ngcllSessionAttributeInitializeMember(ngclSessionAttribute_t *);
static void ngcllSessionAttributeInitializePointer(ngclSessionAttribute_t *);
static int ngcllSessionAttributeFinalize(
    ngclContext_t *, ngclExecutable_t *, ngclSessionAttribute_t *, int *);

/**
 * Construct
 */
ngclSession_t *
ngclSessionConstruct(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    int local_error, result;
    ngclSession_t *session;
    static const char fName[] = "ngclSessionConstruct";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    session = ngcllSessionConstruct(
	context, executable, sessionAttr, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return session;
}

static ngclSession_t *
ngcllSessionConstruct(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    int result;
    ngclSession_t *session;
    static const char fName[] = "ngcllSessionConstruct";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Executable available */
    result = ngcliExecutableIsAvailable(context, executable, error);
    if (result == 0) {
        ngclLogPrintfExecutable(executable,
             NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Executable is not available to create session.\n", fName);
        return NULL;
    }

    /* Allocate */
    session = ngcllSessionAllocate(context, executable, error);
    if (session == NULL) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Session.\n", fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllSessionInitialize(
        context, executable, session, sessionAttr, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Session.\n", fName);
	goto error;
    }

    /* Register */
    result = ngclSessionRegister(context, executable, session, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't register the Session.\n", fName);
	goto error;
    }

    /* Success */
    return session;

    /* Error occurred */
error:
    result = ngcllSessionFree(session, NULL);
    if (session == NULL) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't deallocate the storage for Session.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngclSessionDestruct(
    ngclSession_t *session,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngclSessionDestruct";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session is not valid.\n", fName);
        return 0;
    }

    /* Initialize the local variables */
    context = session->ngs_context;
    log = context->ngc_log;

    /* Unregister */
    result = ngclSessionUnregister(session, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't unregister the Session.\n", fName);
	return 0;
    }

    /* Finalize */
    result = ngcllSessionFinalize(session, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't finalize the Session.\n", fName);
	return 0;
    }

    /* Deallocate */
    result = ngcllSessionFree(session, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for Session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
ngclSession_t *
ngclSessionAllocate(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    /* just wrap for user */
    int local_error, result;
    ngclSession_t *session;
    static const char fName[] = "ngclSessionAllocate";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, &local_error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not valid.\n", fName);
        return NULL;
    }

    session = ngcllSessionAllocate(context, executable, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return session;
}

static ngclSession_t *
ngcllSessionAllocate(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngclSession_t *session;
    static const char fName[] = "ngcllSessionAllocate";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not valid.\n", fName);
        return NULL;
    }

    /* Allocate */
    session = globus_libc_calloc(1, sizeof (ngclSession_t));
    if (session == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the Session.\n", fName);
        return NULL;
    }

   /* Error occurred */
    return session;
}

/**
 * Deallocate
 */
int
ngclSessionFree(ngclSession_t *session, int *error)
{
    /* just wrap for user */
    return ngcllSessionFree(session, error);
}

static int
ngcllSessionFree(ngclSession_t *session, int *error)
{
    static const char fName[] = "ngcllSessionFree";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    if (session == NULL) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Session is not valid.\n", fName);
        return 0;
    }

    /* Deallocate */
    globus_libc_free(session);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngclSessionInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllSessionInitialize(
        context, executable, session, sessionAttr, error);
}

static int
ngcllSessionInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    int result;
    static const char fName[] = "ngcllSessionInitialize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (session == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Session is NULL.\n", fName);
	goto error;
    }

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not valid.\n", fName);
        goto error;
    }

    /* Initialize the members */
    ngcllSessionInitializeMember(session);
    session->ngs_context = context;
    session->ngs_executable = executable;

    /* Initialize the Session Information */
    result = ngiSessionInformationInitialize(
	&session->ngs_info, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Session Information.\n", fName);
	return 0;
    }

    /* Initialize the mutex */
    result = ngiMutexInitialize(&session->ngs_mutex, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't initialize the mutex.\n", fName);
	goto error;
    }
    session->ngs_mutexInitialized = 1;

    /* Initialize the cond */
    result = ngiCondInitialize(&session->ngs_cond, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't initialize the cond.\n", fName);
	goto error;
    }
    session->ngs_condInitialized = 1;

    /* Initialize the Read/Write Lock for own instance */
    result = ngiRWlockInitialize(
	&session->ngs_rwlOwn, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't Initialize the Read/Write Lock for own instance.\n", fName);
	return 0;
    }
    session->ngs_rwlOwnInitialized = 1;

    /* Create the ID of Session */
    result = ngcllSessionCreateID(context, error);
    if (result < 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "Can't assign the ID to Session.\n", fName);
	goto error;
    }
    session->ngs_ID = result;

    /* Inherit heartBeatStatus */
    session->ngs_heartBeatStatus = executable->nge_heartBeatStatus;

    /* Set wait transferArgument */
    session->ngs_waitArgumentTransfer = executable->nge_waitArgumentTransfer;
    if (sessionAttr->ngsa_waitArgumentTransfer !=
        NG_ARGUMENT_TRANSFER_UNDEFINED) {
        session->ngs_waitArgumentTransfer =
            sessionAttr->ngsa_waitArgumentTransfer;
    }

    /* Set session timout */
    session->ngs_timeout = executable->nge_sessionTimeout;
    if (sessionAttr->ngsa_timeout !=
        NGCL_SESSION_ATTRIBUTE_MEMBER_UNDEFINED) {
        session->ngs_timeout = sessionAttr->ngsa_timeout;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize the Read/Write Lock for own instance */
    result = ngcllSessionFinalize(session, NULL);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Session.\n", fName);
	return 0;
    }

    return 0 ;
}

/**
 * Finalize
 */
int
ngclSessionFinalize(
    ngclSession_t *session,
    int *error)
{
    return ngcllSessionFinalize(session, error);
}

static int
ngcllSessionFinalize(
    ngclSession_t *session,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngLog_t *log;
    static const char fName[] = "ngcllSessionFinalize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (session == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Session is NULL.\n", fName);
	return 0;
    }
    if (session->ngs_context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Context is NULL.\n", fName);
	return 0;
    }

    /* Initialize the local variables */
    context = session->ngs_context;
    log = context->ngc_log;

    /* Execute the user defined destructor */
    if (session->ngs_userDestructer != NULL) {
        session->ngs_userDestructer(session);
    } else {
        if (session->ngs_userData != NULL) {
            globus_libc_free(session->ngs_userData);
        }
    }
    session->ngs_userData = NULL;
    session->ngs_userDestructer = NULL;

    /* Destruct the Argument */
    if (session->ngs_arg != NULL) {
	result = ngiArgumentDestruct(session->ngs_arg, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't destruct the argument.\n", fName);
	    return 0;
	}
    }
    session->ngs_arg = NULL;

    if (session->ngs_rmInfoExist != 0) {
    	result = ngcliRemoteMethodInformationRelease(
	    context, &session->ngs_rmInfo, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Can't release the Remote Method Information.\n",
		fName);
	    return 0;
	}
    }
    session->ngs_rmInfoExist = 0;

    /* Finalize the Read/Write Lock for own instance */
    if (session->ngs_rwlOwnInitialized != 0) {
	result = ngiRWlockFinalize(&session->ngs_rwlOwn, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't finalize the Read/Write Lock for own instance.\n",
		fName);
	    return 0;
	}
    }
    session->ngs_rwlOwnInitialized = 0;

    /* Finalize the cond */
    if (session->ngs_condInitialized != 0) {
	result = ngiCondDestroy(&session->ngs_cond, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't finalize the cond.\n", fName);
	    return 0;
	}
    }
    session->ngs_condInitialized = 0;

    /* Finalize the mutex */
    if (session->ngs_mutexInitialized != 0) {
	result = ngiMutexDestroy(&session->ngs_mutex, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't finalize the mutex.\n", fName);
	    return 0;
	}
    }
    session->ngs_mutexInitialized = 0;

    /* Finalize the Session Information */
    result = ngiSessionInformationFinalize(
	&session->ngs_info, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Session Information.\n", fName);
	return 0;
    }

    /* Initialize the members */
    ngcllSessionInitializeMember(session);

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
ngcllSessionInitializeMember(ngclSession_t *session)
{
    /* Initialize the flags and pointers */
    ngcllSessionInitializeFlag(session);
    ngcllSessionInitializePointer(session);

    /* Initialize the members */
    session->ngs_ID = NGI_SESSION_ID_UNDEFINED;
    session->ngs_status = NG_SESSION_STATUS_INITIALIZED;
    session->ngs_cancelRequest = 0;
    session->ngs_error = NG_ERROR_NO_ERROR;
    session->ngs_wasFailureNotified = 0;
    session->ngs_heartBeatStatus = NG_HEART_BEAT_STATUS_OK;
    session->ngs_timeout = 0;
    session->ngs_timeoutTime = 0;
    session->ngs_waitArgumentTransfer = NG_ARGUMENT_TRANSFER_UNDEFINED;
}

/**
 * Initialize the flags.
 */
static void
ngcllSessionInitializeFlag(ngclSession_t *session)
{
    session->ngs_mutexInitialized = 0;
    session->ngs_condInitialized = 0;
    session->ngs_rwlOwnInitialized = 0;
    session->ngs_rmInfoExist = 0;
}

/**
 * Initialize the pointers.
 */
static void
ngcllSessionInitializePointer(ngclSession_t *session)
{
    session->ngs_next = NULL;
    session->ngs_apiNext = NULL;
    session->ngs_waitNext = NULL;
    session->ngs_cancelNext = NULL;
    session->ngs_doneNext = NULL;
    session->ngs_context = NULL;
    session->ngs_executable = NULL;
    session->ngs_userData = NULL;
    session->ngs_userDestructer = NULL;
    session->ngs_arg = NULL;
}

/**
 * Register
 */
int
ngclSessionRegister(
    ngclContext_t *context,
    ngclExecutable_t *executable, 
    ngclSession_t *session,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionRegister";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, &local_error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionRegister(context, executable, session, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionRegister(
    ngclContext_t *context,
    ngclExecutable_t *executable, 
    ngclSession_t *session,
    int *error)
{
    int result;
    int incremented = 0;
    int locked = 0;
    static const char fName[] = "ngcllSessionRegister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Context is NULL.\n", fName);
	return 0;
    }
    if (executable == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Executable is NULL.\n", fName);
	return 0;
    }
    if (session == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The Session is NULL.\n", fName);
	return 0;
    }

    /* Increment the number of Sessions */
    result = ngcliExecutableNsessionsIncrement(
	executable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't increment the number of Sessions.\n", fName);
	error = NULL;
	goto error;
    }
    incremented = 1;

    /* Lock the list */
    result = ngclSessionListWriteLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write-lock the list for Session contained in Executable.\n",
	    fName);
	error = NULL;
	goto error;
    }
    locked = 1;

    /* Append at last of the list */
    session->ngs_next = NULL;
    if (executable->nge_session_head == NULL) {
	/* No Session is registered */
	assert(executable->nge_session_tail == NULL);
	executable->nge_session_head = session;
	executable->nge_session_tail = session;
    } else {
	/* Any Session is already registered */
	assert(executable->nge_session_tail != NULL);
	assert(executable->nge_session_tail->ngs_next == NULL);
	executable->nge_session_tail->ngs_next = session;
	executable->nge_session_tail = session;
    }

    /* Unlock the list */
    locked = 0;
    result = ngclSessionListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't write-unlock the list for Session contained in Executable.\n",
	    fName);
	error = NULL;
	goto error;
    }

    /* Success */
    return 1;

error:
    /* Unlock the list */
    if (locked != 0) {
	locked = 0;
	result = ngclSessionListWriteUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't write-unlock the list for Session contained in Executable.\n",
		fName);
	    error = NULL;
	}
    }

    /* Decrement the number of Sessions */
    if (incremented != 0) {
	incremented = 0;
	result = ngcliExecutableNsessionsDecrement(
	    executable, context->ngc_log, error);
	if (result == 0) {
	    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't decrement the number of Sessions.\n", fName);
	    error = NULL;
	}
    }

    /* Faild */
    return 0;
}

/**
 * Unregister
 */
int
ngclSessionUnregister(
    ngclSession_t *session,
    int *error)
{
    int result;
    int locked = 0;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    static const char fName[] = "ngclSessionUnregister";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session is not valid.\n", fName);
        return 0;
    }
    context = session->ngs_context;
    executable = session->ngs_executable;

    /* Lock the list */
    result = ngclSessionListWriteLock(context, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	return 0;
    }
    locked = 1;

    /* Unregister */
    result = ngclSessionUnregisterWithoutLock(session, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't Unregister Session.\n", fName);
	goto error;
    }

    /* Unlock the list */
    locked = 0;
    result = ngclSessionListWriteUnlock(context, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the list of Session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;

error:
    /* Unlock the list */
    if (locked != 0) {
	locked = 0;
	result = ngclSessionListWriteUnlock(context, NULL);
	if (result == 0) {
	    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the list of Session.\n", fName);
	    return 0;
	}
    }

    /* Failed */
    return 0;
}

/**
 * Unregister without lock
 *
 * Note:
 * Write lock the list before using this function, and unlock the list after
 * use.
 */
int
ngclSessionUnregisterWithoutLock(
    ngclSession_t *session,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngclSession_t *prev, *curr;
    static const char fName[] = "ngclSessionUnregisterWithoutLock";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session is not valid.\n", fName);
        return 0;
    }
    context = session->ngs_context;
    executable = session->ngs_executable;

    /* Find the Session */
    prev = NULL;
    curr = executable->nge_session_head;
    for (; curr != session; curr = curr->ngs_next) {
    	if (curr == NULL)
	    goto notFound;
	prev = curr;
    }

    /* Unregister the Session */
    if (session == executable->nge_session_head)
    	executable->nge_session_head = session->ngs_next;
    if (session == executable->nge_session_tail)
    	executable->nge_session_tail = prev;
    if (prev != NULL)
    	prev->ngs_next = session->ngs_next;
    session->ngs_next = NULL;

    /* Decrement the number of Sessions */
    result = ngcliExecutableNsessionsDecrement(
	executable, context->ngc_log, error);
    if (result == 0) {
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't increment the number of Sessions.\n", fName);
	    error = NULL;
	return 0;
    }

    /* Success */
    return 1;

    /* Not found */
notFound:
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
    	NG_LOG_LEVEL_ERROR, NULL,
	"%s: Session is not found.\n", fName);

    /* Failed */
    return 0;
}

/**
 * Register the user defined data.
 */
int
ngclSessionRegisterUserData(
    ngclSession_t *session,
    void *userData,
    void (*userDestructer)(ngclSession_t *),
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionRegisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
        return 0;
    }

    result = ngcllSessionRegisterUserData(
	session, userData, userDestructer, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionRegisterUserData(
    ngclSession_t *session,
    void *userData,
    void (*userDestructer)(ngclSession_t *),
    int *error)
{
    int result;
    static const char fName[] = "ngcllSessionRegisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock this instance */
    result = ngclSessionWriteLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return 0;
    }

    /* Register the user defined data */
    session->ngs_userData = userData;
    session->ngs_userDestructer = userDestructer;

    /* Unlock this instance */
    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister the user defined data.
 */
int
ngclSessionUnregisterUserData(
    ngclSession_t *session,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionUnregisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionUnregisterUserData(session, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionUnregisterUserData(
    ngclSession_t *session,
    int *error)
{
    int result;
    static const char fName[] = "ngcllSessionUnregisterUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock this instance */
    result = ngclSessionWriteLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return 0;
    }

    /* Unregister the user defined data */
    session->ngs_userData = NULL;
    session->ngs_userDestructer = NULL;

    /* Unlock this instance */
    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the user defined data.
 */
int
ngclSessionGetUserData(ngclSession_t *session, void **userData, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionGetUserData";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    if (session == NULL) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionGetUserData(session, userData, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionGetUserData(ngclSession_t *session, void **userData, int *error)
{
    int result;
    static const char fName[] = "ngcllSessionGetUserData";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Check the arguments */
    if (userData == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: The userData is NULL.\n", fName);
        return 0;
    }

    /* Lock this instance */
    result = ngclSessionReadLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return 0;
    }

    /* Get the user defined data */
    *userData = session->ngs_userData;

    /* Unlock this instance */
    result = ngclSessionReadUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the Executable.
 */
ngclExecutable_t *
ngclSessionGetExecutable(ngclSession_t *session, int *error)
{
    int local_error, result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngclSessionGetExecutable";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    executable = ngcllSessionGetExecutable(session, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return executable;
}

static ngclExecutable_t *
ngcllSessionGetExecutable(ngclSession_t *session, int *error)
{
    int result;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllSessionGetExecutable";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock this instance */
    result = ngclSessionReadLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the Session.\n", fName);
	return NULL;
    }

    /* Get the error code */
    executable = session->ngs_executable;

    /* Unlock this instance */
    result = ngclSessionReadUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the Session.\n", fName);
	return NULL;
    }

    /* Success */
    return executable;
}

/**
 * Get the list of Sessions.
 */
ngclSession_t *
ngclSessionGetList(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *sessionIDs,
    int nSessions,
    int *error)
{
    int result;
    int localError = NG_ERROR_NO_ERROR;
    ngclSession_t *list;
    static const char fName[] = "ngclSessionGetList";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session ID specified? */
    if (sessionIDs != NULL) {
	/* Is number of Sessions valid? */
	if (nSessions <= 0) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Number of sessions %d is not valid.\n",
		fName, nSessions);
	    return NULL;
	}

	/* Is Executable specified? */
	if (executable != NULL) {
	    /* Get the Session from Executable which specified */
	    list = ngcliExecutableGetSessionList(
	    	executable, sessionIDs, nSessions, error);
	    if (list == NULL) {
	    	ngclLogPrintfExecutable(executable,
		    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't get the Session.\n", fName);
		return NULL;
	    }
	}

	/* Is Ninf-G Context specified? */
	else if (context != NULL) {
	    /* Get the Session from Ninf-G Context which specified */
	    list = ngcliContextGetSessionList(
	    	context, sessionIDs, nSessions, error);
	    if (list == NULL) {
	    	ngclLogPrintfContext(context,
		    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't get the Session.\n", fName);
		return NULL;
	    }
	}

	else {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Ninf-G Context and Executable are NULL.\n",
		fName);
	    return NULL;
	}
    }

    /* Is Executable specified? */
    else if (executable != NULL) {
	/* Is Executable valid? */
	result = ngcliExecutableIsValid(context, executable, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Executable is not valid.\n", fName);
	    return NULL;
	}

    	/* Get the all Sessions which conteined in Executable */
	list = ngcliExecutableGetAllSessionList(executable, &localError);
        if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session.\n", fName);
	    return NULL;
	}
    }

    /* Is Context specified? */
    else if (context != NULL) {
	/* Is Context valid? */
	result = ngcliContextIsValid(context, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Context is not valid.\n", fName);
	    return NULL;
	}

    	/* Wait all Sessions which conteined in Ninf-G Context */
	list = ngcliContextGetAllSessionList(context, &localError);
        if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session.\n", fName);
	    return NULL;
	}
    }

    else {
	/* No Sessions were specified */
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: No Sessions were specified.\n", fName);
	return NULL;
    }

    /* Success */
    return list;
}

/**
 * Release the list of Sessions.
 */
int
ngclSessionReleaseList(
    ngclSession_t *session,
    int *error)
{
    int result;
    ngclSession_t *curr, *next;
    static const char fName[] = "ngclSessionReleaseList";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session is not valid.\n", fName);
        return 0;
    }

    /* Release the list */
    for (curr = session; curr != NULL; curr = next) {
        next = curr->ngs_apiNext;
        curr->ngs_apiNext = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Get the waiting list of Session.
 */
ngclSession_t *
ngcliSessionGetWaitList(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclSession_t **prev, *curr;
    int localError;
    static const char fName[] = "ngcliSessionGetWaitList";

    localError = NG_ERROR_NO_ERROR;

    /* Is Session specified? */
    if (session != NULL) {
	/* Is Session valid? */
	result = ngcliSessionIsValid(NULL, NULL, session, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Session is not valid.\n", fName);
	    return 0;
	}

	/* Make the waiting list */
	prev = &session->ngs_waitNext;
	curr = session->ngs_apiNext;
	for (; curr != NULL; curr = curr->ngs_apiNext) {
	    *prev = curr;
	    prev = &curr->ngs_waitNext;
	}
	*prev = NULL;
    }

    /* Is Executable specified? */
    else if (executable != NULL) {
	/* Is Executable valid? */
	result = ngcliExecutableIsValid(context, executable, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Executable is not valid.\n", fName);
	    return NULL;
	}

        /* Get the List of Session by waitNext */
        session = ngcliExecutableGetAllSessionWaitList(executable, &localError);
        if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the list of Session.\n", fName);
	    return NULL;
        }
    }

    /* Is Context specified? */
    else if (context != NULL) {
	/* Is Context valid? */
	result = ngcliContextIsValid(context, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Context is not valid.\n", fName);
	    return NULL;
	}

        /* Get the List of Session by waitNext */
        session = ngcliContextGetAllSessionWaitList(context, &localError);
        if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the list of Session.\n", fName);
	    return NULL;
        }
    }

    /* No Sessions were specified */
    else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: No Sessions were specified.\n", fName);
	return NULL;
    }

    /* Success */
    return session;
}

/**
 * Release the waiting list of Session.
 */
int
ngcliSessionReleaseWaitList(
    ngclSession_t *session,
    ngLog_t *log,
    int *error)
{
    ngclSession_t *curr, *prev;

    /* Check the arguments */
    assert(session != NULL);

    /* Release the list */
    for (curr = session; curr != NULL; curr = prev) {
        prev = curr->ngs_waitNext;
        curr->ngs_waitNext = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Get the canceling list of Sessions.
 */
ngclSession_t *
ngclSessionGetCancelList(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *sessionIDs,
    int nSessions,
    int *error)
{
    int result;
    ngclSession_t *list;
    int localError;
    static const char fName[] = "ngclSessionGetCancelList";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    localError = NG_ERROR_NO_ERROR;

    /* Is Session ID specified? */
    if (sessionIDs != NULL) {
	/* Is number of Sessions valid? */
	if (nSessions <= 0) {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Number of sessions %d is not valid.\n",
		fName, nSessions);
	    return NULL;
	}

	/* Is Executable specified? */
	if (executable != NULL) {
	    /* Get the Session from Executable which specified */
	    list = ngcliExecutableGetSessionCancelList(
	    	executable, sessionIDs, nSessions, error);
	    if (list == NULL) {
	    	ngclLogPrintfExecutable(executable,
		    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't get the Session.\n", fName);
		return NULL;
	    }
	}

	/* Is Ninf-G Context specified? */
	else if (context != NULL) {
	    /* Get the Session from Ninf-G Context which specified */
	    list = ngcliContextGetSessionCancelList(
	    	context, sessionIDs, nSessions, error);
	    if (list == NULL) {
	    	ngclLogPrintfContext(context,
		    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't get the Session.\n", fName);
		return NULL;
	    }
	}

	else {
	    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Ninf-G Context and Executable are NULL.\n",
		fName);
	    return NULL;
	}
    }

    /* Is Executable specified? */
    else if (executable != NULL) {
	/* Is Executable valid? */
	result = ngcliExecutableIsValid(context, executable, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Executable is not valid.\n", fName);
	    return NULL;
	}

    	/* Get the all Sessions which conteined in Executable */
	list = ngcliExecutableGetAllSessionCancelList(executable, &localError);
	if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session.\n", fName);
	    return NULL;
	}
    }

    /* Is Context specified? */
    else if (context != NULL) {
	/* Is Context valid? */
	result = ngcliContextIsValid(context, error);
	if (result == 0) {
    	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    	NULL, "%s: Context is not valid.\n", fName);
	    return NULL;
	}

    	/* Wait all Sessions which conteined in Ninf-G Context */
	list = ngcliContextGetAllSessionCancelList(context, &localError);
	if (localError != NG_ERROR_NO_ERROR) {
            NGI_SET_ERROR(error, localError);
    	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session.\n", fName);
	    return NULL;
	}
    }

    else {
	/* No Sessions were specified */
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: No Sessions were specified.\n", fName);
	return NULL;
    }

    /* Success */
    return list;
}

/**
 * Release the canceling list of Session.
 */
int
ngclSessionReleaseCancelList(
    ngclSession_t *session,
    int *error)
{
    ngclSession_t *curr, *prev;

    /* Check the arguments */
    assert(session != NULL);

    /* Release the list */
    for (curr = session; curr != NULL; curr = prev) {
        prev = curr->ngs_cancelNext;
        curr->ngs_cancelNext = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Get the Session by ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclSession_t *
ngclSessionGet(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int sessionID,
    int *error)
{
    int result;
    ngclSession_t *session;
    static const char fName[] = "ngclSessionGet";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Executable specify */
    if (executable != NULL) {
	/* Executable is specified */

	/* Is Executable valid? */
	result = ngcliExecutableIsValid(context, executable, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		NULL, "%s: Executable is not valid.\n", fName);
	    return NULL;
	}
	/* Get the Ninf-G Context */
	context = executable->nge_context;

	return ngclExecutableGetSession(executable, sessionID, error);
    } else if (context != NULL) {
	/* Is Ninf-G Context valid? */
	result = ngcliContextIsValid(context, error);
	if (result == 0) {
	    ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		NULL, "%s: Ninf-G Context is not valid.\n", fName);
	    return NULL;
	}

	executable = ngclExecutableGetNext(context, NULL, error);
	while (executable != NULL) {
	    /* Get the Session */
	    session = ngclExecutableGetSession(executable, sessionID, error);
	    if (session != NULL)
		return session;

	    /* Get the next Executable */
	    executable = ngclExecutableGetNext(context, executable, error);
	}
    } else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Ninf-G Context and Executable are NULL.\n", fName);
	return NULL;
    }

    /* Failed */
    return NULL;
}

/**
 * Get the next Session.
 *
 * Return the Session from the top of list, if current is NULL.
 * Return the next Session of current, if current is not NULL.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclSession_t *
ngclSessionGetNext(
    ngclExecutable_t *executable,
    ngclSession_t *current,
    int *error)
{
    int local_error, result;
    ngclSession_t *session;
    static const char fName[] = "ngclSessionGetNext";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(NULL, executable, &local_error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Executable is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return NULL;
    }

    session = ngcllSessionGetNext(executable, current, &local_error);
    NGI_SET_ERROR_EXECUTABLE(executable, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return session;
}

ngclSession_t *
ngcliSessionGetNext(
    ngclExecutable_t *executable,
    ngclSession_t *current,
    int *error)
{
    /* Check the argument */
    assert(executable != NULL);

    /* Get the next Session */
    return ngcllSessionGetNext(executable, current, error);
}

static ngclSession_t *
ngcllSessionGetNext(
    ngclExecutable_t *executable,
    ngclSession_t *current,
    int *error)
{
    static const char fName[] = "ngcllSessionGetNext";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    if (current == NULL) {
	/* Return the first Session */
	if (executable->nge_session_head != NULL) {
	    assert(executable->nge_session_tail != NULL);
	    return executable->nge_session_head;
	}
    } else {
	/* Return the next Session */
	if (current->ngs_next != NULL) {
	    return current->ngs_next;
	}
    }

    /* The last Session was reached */
    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Session was reached.\n", fName);

    return NULL;
}

/**
 * Get the error code.
 */
int
ngclSessionGetError(ngclSession_t *session, int *error)
{
    int result;
    int retError;
    static const char fName[] = "ngclSessionGetError";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
        return -1;
    }

    /* Lock this instance */
    result = ngclSessionReadLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return -1;
    }

    /* Get the error code */
    retError = session->ngs_error;

    /* Unlock this instance */
    result = ngclSessionReadUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return -1;
    }

    /* Success */
    return retError;
}

/**
 * Set the execution time.
 */
int
ngcliSessionSetExecutionTime(
    ngclContext_t *context,
    ngcliJobManager_t *jobMng,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(jobMng != NULL);
    assert(executable != NULL);
    assert(session != NULL);

    /* Real time */
    session->ngs_info.ngsi_clientRealTime.ngsic_queryRemoteMachineInformation
    	= executable->nge_executionTime.nge_queryRemoteMachineInformation.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_queryRemoteClassInformation
    	= executable->nge_executionTime.nge_queryRemoteClassInformation.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_invokeExecutable
    	= jobMng->ngjm_invoke.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_transferArgument
        = session->ngs_executionTime.ngs_transferArgument.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_calculation
        = session->ngs_executionTime.ngs_calculation.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_transferResult
        = session->ngs_executionTime.ngs_transferResult.
	  nget_real.nget_execution;
    session->ngs_info.ngsi_clientRealTime.ngsic_callbackTransferArgument
        = session->ngs_executionTime.ngs_sumCallbackTransferArgumentReal;
    session->ngs_info.ngsi_clientRealTime.ngsic_callbackCalculation
        = session->ngs_executionTime.ngs_sumCallbackCalculationReal;
    session->ngs_info.ngsi_clientRealTime.ngsic_callbackTransferResult
        = session->ngs_executionTime.ngs_sumCallbackTransferResultReal;

    /* CPU time */
    session->ngs_info.ngsi_clientCPUtime.ngsic_queryRemoteMachineInformation
    	= executable->nge_executionTime.nge_queryRemoteMachineInformation.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_queryRemoteClassInformation
    	= executable->nge_executionTime.nge_queryRemoteClassInformation.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_invokeExecutable
    	= jobMng->ngjm_invoke.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_transferArgument
        = session->ngs_executionTime.ngs_transferArgument.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_calculation
        = session->ngs_executionTime.ngs_calculation.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_transferResult
        = session->ngs_executionTime.ngs_transferResult.
	  nget_cpu.nget_execution;
    session->ngs_info.ngsi_clientCPUtime.ngsic_callbackTransferArgument
        = session->ngs_executionTime.ngs_sumCallbackTransferArgumentCPU;
    session->ngs_info.ngsi_clientCPUtime.ngsic_callbackCalculation
        = session->ngs_executionTime.ngs_sumCallbackCalculationCPU;
    session->ngs_info.ngsi_clientCPUtime.ngsic_callbackTransferResult
        = session->ngs_executionTime.ngs_sumCallbackTransferResultCPU;

    /* The number of times to which the function was called */
    session->ngs_info.ngsi_clientCallbackNtimesCalled
	= session->ngs_executionTime.ngs_callbackNtimesCalled;

    /* Success */
    return 1;
}

/**
 * Set the status of Session is set to done.
 */
int
ngcliSessionStatusSetDone(
    ngclSession_t *session,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *context;
    static const char fName[] = "ngcliSessionStatusSetDone";

    /* Check the arguments */
    assert(session != NULL);
    assert(session->ngs_context != NULL);
    context = session->ngs_context;

    /* Stop Session timeout measurement */
    result = ngcliSessionTimeoutSessionDone(context, session, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the session timeout.\n",
            fName);
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
        session, NG_SESSION_STATUS_DONE, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't set the status.\n", fName);
        return 0;
    }

    /* Notify to Context */
    result = ngcliContextNotifySession(context, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't notify to Session.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the status.
 */
int
ngcliSessionStatusSet(
    ngclSession_t *session,
    ngclSessionStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliSessionStatusSet";

    /* Check the arguments */
    assert(session != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The state of Session is set to %s.\n",
        fName, ngcliSessionStatusToString(status));

    /* Set the status */
    session->ngs_status = status;

    /* Notify signal */
    result = ngiCondBroadcast(&session->ngs_cond, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't signal the Condition Variable.\n", fName);
	goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&session->ngs_mutex, log, NULL);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Wait the status (equal).
 */
int
ngcliSessionStatusWait(
    ngclSession_t *session,
    ngclSessionStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliSessionStatusWait";

    /* Check the arguments */
    assert(session != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: It waits for the state of Session to grow into %s.\n",
        fName, ngcliSessionStatusToString(status));

    /* Wait the status */
    while ((session->ngs_status != status) &&
            (session->ngs_status != NG_SESSION_STATUS_DONE)) {
	result = ngiCondWait(
	    &session->ngs_cond, &session->ngs_mutex, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The state of Session grew into %s.\n",
        fName, ngcliSessionStatusToString(status));

    /* Is error occurred? */
    if (session->ngs_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, session->ngs_error);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            (session->ngs_error != NG_ERROR_CANCELED ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_INFORMATION), NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Is error occurred in callback? */
    if (session->ngs_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, session->ngs_cbError);
        session->ngs_error = session->ngs_cbError;
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Unexpected DONE is error */
    if (session->ngs_status != status) {
        assert(session->ngs_status == NG_SESSION_STATUS_DONE);

        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session become unexpected state: DONE.\n", fName);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&session->ngs_mutex, log, NULL);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Wait the status (greater equal).
 */
int
ngcliSessionStatusWaitGreaterEqual(
    ngclSession_t *session,
    ngclSessionStatus_t status,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliSessionStatusWaitGreaterEqual";

    /* Check the arguments */
    assert(session != NULL);

    /* Lock the mutex */
    result = ngiMutexLock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the Mutex.\n", fName);
    	return 0;
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: It waits for the state of Session to grow into %s.\n",
        fName, ngcliSessionStatusToString(status));

    /* Wait the status */
    while (session->ngs_status < status) {
	result = ngiCondWait(
	    &session->ngs_cond, &session->ngs_mutex, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't wait the Condition Variable.\n", fName);
	    goto error;
	}
    }

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL,
        "%s: The state of Session grew into %s.\n",
        fName, ngcliSessionStatusToString(session->ngs_status));

    /* Is error occurred? */
    if (session->ngs_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, session->ngs_error);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Is error occurred in callback? */
    if (session->ngs_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, session->ngs_cbError);
        session->ngs_error = session->ngs_cbError;
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Unlock the mutex */
    result = ngiMutexUnlock(&session->ngs_mutex, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock */
    result = ngiMutexUnlock(&session->ngs_mutex, log, NULL);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the Mutex.\n", fName);
    }

    /* Failed */
    return 0;
}

/**
 * Set the Session unusable
 */
int
ngcliSessionUnusable(
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int errorCause,
    int *error)
{
    ngLog_t *log;
    int result, retResult;
    static const char fName[] = "ngcliSessionUnusable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(executable->nge_context != NULL);
    assert(session != NULL);

    /* Initialize the local variables */
    log = executable->nge_context->ngc_log;
    retResult = 1;

    /* log */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Setting this session unusable.\n", fName);

    /* Set the error */
    result = ngclSessionSetError(session, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the error.\n", fName);
    }

    /* Set the callback error */
    result = ngclSessionSetCbError(session, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the callback error.\n", fName);
    }

    /* Session is done */
    result = ngcliSessionStatusSetDone(session, log, error);
    if (result == 0) {
        retResult = 0;
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status DONE.\n", fName);
        return 0;
    }

    return retResult;
}

/**
 * Set the error code.
 */
int
ngclSessionSetError(ngclSession_t *session, int setError, int *error)
{
    int result;
    static const char fName[] = "ngclSessionSetError";

    /* Check the arguments */
    assert(session != NULL);

    /* Lock this instance */
    result = ngclSessionWriteLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return 0;
    }

    /* Set the error code */
    session->ngs_error = setError;

    /* Unlock this instance */
    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * $et the error code, it occurred in callback function.
 */
int
ngclSessionSetCbError(ngclSession_t *session, int setError, int *error)
{
    int result;
    static const char fName[] = "ngclSessionSetCbError";

    /* Check the arguments */
    assert(session != NULL);

    /* Lock this instance */
    result = ngclSessionWriteLock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't lock the session.\n", fName);
	return 0;
    }

    /* Get the error code */
    session->ngs_cbError = setError;

    /* Unlock this instance */
    result = ngclSessionWriteUnlock(session, error);
    if (result == 0) {
    	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL, "%s: Can't unlock the session.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

#if 0 /* This function defined at below */
/**
 * Get the copy of Session by ID.
 */
int
ngclSessionGetCopy(
    ngclContext_t *context,
    ngclSession_t *session,
    int sessionID,
    int *error)
{
    int result;
    ngclSession_t *sess;
    static const char fName[] = "ngclSessionGetCopy";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n", fName);
	return 0;
    }

    /* Get */
    sess = ngclSessionGet(context, sessionID, error);
    if (sess == NULL) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Session is not found.\n", fName);
	goto error;
    }

    /* Copy */
    *session = *sess;

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Session.\n", fName);
	return 0;
    }

    /* Clear the pointers */
    ngcllSessionInitializePointer(session);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, NULL);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_LEVEL_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't unlock the list of Session.\n", fName);
	return 0;
    }

    return 0;
}
#endif

#if 0 /* This function defined at below */
/**
 * Copy the Session.
 */
int
ngclSessionCopy(
    ngclContext_t *context,
    ngclSession_t *src,
    ngclSession_t *dest,
    int *error)
{
    int result;
    static const char fName[] = "ngclSessionCopy";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Context valid? */
    result = ngcliContextIsValid(context, error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Ninf-G Context is not valid.\n", fName);
	return 0;
    }

    /* Is Session valid? */
    result = ngcliSessionIsValid(context, NULL, src, error);
    if (result == 0) {
    	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session is not valid.\n", fName);
	return 0;
    }

    /* Copy */
    *dest = *src;

    /* Clear the pointers */
    ngcllSessionInitializePointer(dest);

    /* Success */
    return 1;
}
#endif

/**
 * Is Session valid?
 */
int
ngcliSessionIsValid(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int result;
    ngclSession_t *curr;
    static const char fName[] = "ngcliSessionIsValid";

    /* Is Session NULL */
    if (session == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Session is NULL.\n", fName);
	return 0;
    }

    /* Use Executable which contained in Session, if executable is NULL */
    if (executable == NULL) {
	executable = session->ngs_executable;
    }

    /* Use Context which conteined in Session, if context is NULL */
    if (context == NULL) {
	context = session->ngs_context;
    }

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Executable is not valid.\n", fName);
	return 0;
    }

    /* Is ID smaller than minimum? */
    if (session->ngs_ID < NGI_SESSION_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session ID %d is smaller than minimum %d.\n",
	    fName, session->ngs_ID, NGI_SESSION_ID_MIN);
	return 0;
    }

    /* Is ID smaller than maximum? */
    if (session->ngs_ID > NGI_SESSION_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Session ID %d is smaller than maximum %d.\n",
	    fName, session->ngs_ID, NGI_SESSION_ID_MAX);
	return 0;
    }

    /* Is Session exist? */
    curr = executable->nge_session_head;
    for (; curr != session; curr = curr->ngs_next) {
	if (curr == NULL) {
	    /* Not found */
	    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session is not registered into Executable.\n", fName);
	    return 0;
	}
    }

    /* Session is valid */
    return 1;
}

/**
 * Create the identifier of Session.
 */
static int
ngcllSessionCreateID(ngclContext_t *context, int *error)
{
    int result;
    int newID;
    static const char fName[] = "ngcllSessionCreateID";

    /* Lock this instance */
    result = ngclContextWriteLock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't lock the Ninf-G Context.\n", fName);
        return -1;
    }

    /* Get the new ID, which is not used */
    newID = context->ngc_sessionID;

    newID++;
    if (newID >= NGI_SESSION_ID_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_EXCEED_LIMIT);
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: There is no IDs for Ninf-G Session to use.\n", fName);

        /* Unlock this instance */
        result = ngclContextWriteUnlock(context, error);
        if (result == 0) {
            ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't unlock the Ninf-G Context.\n", fName);
        }
        return -1;
    }

    context->ngc_sessionID = newID;

    /* Unlock this instance */
    result = ngclContextWriteUnlock(context, error);
    if (result == 0) {
        ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't unlock the Ninf-G Context.\n", fName);
        return -1;
    }

    /* Success */
    return newID;
}

#if 0 /* This function is not necessary */
/**
 * Get the Session Manager specified by Session ID.
 *
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclSession_t *
ngclSessionGet(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *sessionIds,
    int nSessionIds,
    int *error)

{
#if 0
CAUTION:
This function is not completed.
Return the list of Session, use ngs_apiNext.
Find the Sessions from Context, if executable is NULL.
Find the Sessions form Executable, if executable is not NULL.
Return the all Sessions, if sessionIds is NULL.
#endif
    ngclSession_t *session;
    static const char fName[] = "ngclSessionGet";

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not valid.\n", fName);
        return NULL;
    }

    /* Is ID less than minimum? */
    if (id < NGI_SESSION_ID_MIN) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintf(context, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: ID number %d is less than %d.\n",
	    fName, id, NGI_SESSION_ID_MIN);
	goto error;
    }

    /* Is ID greater than maximum? */
    if (id > NGI_SESSION_ID_MAX) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngclLogPrintf(context, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: ID number %d is greater than %d.\n",
	    fName, id, NGI_SESSION_ID_MAX);
	goto error;
    }

    session = executable->ngc_session_head;
    for (; session != NULL; session = session->ngs_next) {
	assert(session->ngs_ID >= NGI_SESSION_ID_MIN);
	assert(session->ngs_ID <= NGI_SESSION_ID_MAX);
	if (session->ngs_ID == id) {
	    /* Found */
	    return session;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Session is not found by ID %d.\n", fName, id);
    return NULL;
}
#endif /* 0 */

#if 0 /* This function defined at below */
/**
 * Get the next Session Manager.
 *
 * Return the Session Handle from the top of the list, if current is NULL.
 * Return the next Session of current, if current is not NULL.
 * 
 * Note:
 * Lock the list before using this function, and unlock the list after use.
 */
ngclSession_t *
ngclSessionGetNext(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *current,
    int *error)
{
    int result;
    static const char fName[] = "ngclSessionGetNext";

    /* Is Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Executable is not valid.\n", fName);
        return NULL;
    }

    if (current == NULL) {
	/* Return the first Session */
	if (executable->nge_session_head != NULL) {
	    assert(executable->nge_session_tail != NULL);
	    return executable->nge_session_head;
	}
    } else {
	/* Return the next Session */
	if (current->ngs_next != NULL) {
	    return current->ngs_next;
	}
    }

    /* The last Session was reached */
    ngclLogPrintfExecutable(executable, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: The last Session was reached.\n", fName);

    return NULL;
}
#endif

/**
 * Invoke session.
 */
int
ngclSessionStart(
    ngclSession_t *session,
    char *methodName,
    int *error,
    ...)
{
    int result;
    va_list ap;
#if 0
    static const char fName[] = "ngclSessionStart";
#endif

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Get the argument pointer */
    va_start(ap, error);

    /* Execute */
    result = ngclSessionStartVarg(session, methodName, ap, error);

    /* Release the argument pointer */
    va_end(ap);

    return result;
}

/**
 * Session start.
 */
int
ngclSessionStartVarg(
    ngclSession_t *session,
    char *methodName,
    va_list ap,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionStartVarg";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionStart(session, methodName, NULL, ap, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionStart(
    ngclSession_t *session,
    char *methodName,
    ngclArgumentStack_t *argStack,
    va_list ap,
    int *error)
{
    int result;
    int executableLocked = 0;
    int executableLockedWithSend = 0;
    int sessionListLocked = 0;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngclSession_t *tmpSession;
    ngclSession_t *saveApiNext;
    ngiArgument_t *arg;
    int fortranCompatible;
    ngiArgumentDelivery_t argDelivery;
    ngRemoteMethodInformation_t *rmInfo;
    ngLog_t *log;
    ngArgumentTransfer_t waitArgumentTransfer;
    static const char fName[] = "ngcllSessionStart";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    context = session->ngs_context;
    executable = session->ngs_executable;
    log = context->ngc_log;

    /* Is Method name valid? */
    if (methodName == NULL) {
    	/* Use default */
	methodName = NGI_METHOD_NAME_DEFAULT;
    }

    /* Is length of Method name zero? */
    if (methodName[0] == '\0') {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The length of Method name is zero.\n", fName);
	return 0;
    }
    
    /* Output the log */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,	"%s: Start the Session.\n", fName);

    /* Start Session timeout measurement */
    result = ngcliSessionTimeoutSessionStart(context, session, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the session timeout.\n",
            fName);
        return 0;
    }

    /* Get the Fortran Compatible */
    result = ngcliExecutableGetLocalMachineInformationFortranCompatible(
	executable, &fortranCompatible, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Fortran Compatible.\n", fName);
	goto error;
    }
    argDelivery = (fortranCompatible == 0) ?
	NGI_ARGUMENT_DELIVERY_C : NGI_ARGUMENT_DELIVERY_FORTRAN;

    /* Wait until the Remote Class Information available */
    result = ngcliExecutableRemoteClassInformationWait(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't wait the Executable until Remote Class prepared.\n",
            fName);
        goto error;
    }
    assert(executable->nge_rcInfoExist != 0);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        return 0;
    }
    executableLocked = 1;

    /* Is error occurred? */
    if (session->ngs_error != NG_ERROR_NO_ERROR) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error (%d) has already occurred.\n",
            fName, session->ngs_error);
        goto error;
    }

    /* Is status initialized? */
    if (session->ngs_status != NG_SESSION_STATUS_INITIALIZED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session is not initialized.\n", fName);
        goto error;
    }

    /* Get the Remote Method Information */
    result = ngcliRemoteMethodInformationGetCopy(
    	context, &executable->nge_rcInfo, methodName,
	&session->ngs_rmInfo, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Remote Method \"%s\" is not found on Remote Class \"%s\".\n",
            fName, methodName, executable->nge_rcInfo.ngrci_className);
	goto error;
    }
    session->ngs_rmInfoExist = 1;
    rmInfo = &session->ngs_rmInfo;

    /* Construct the Argument */
    arg = ngiArgumentConstruct(
	rmInfo->ngrmi_arguments, rmInfo->ngrmi_nArguments, log, error);
    if (arg == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the argument.\n", fName);
	goto error;
    }
    session->ngs_arg = arg;

    if (argStack == NULL) {
	result = ngiArgumentElementInitializeVarg(
	    arg, session->ngs_rmInfo.ngrmi_nArguments, ap,
	    argDelivery, log, error);
    } else {
	result = ngiArgumentElementInitializeStack(
	    arg, session->ngs_rmInfo.ngrmi_nArguments, argStack, log, error);
    }
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the Argument Element.\n", fName);
	goto error;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
	arg, arg, rmInfo->ngrmi_arguments, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't initialize the Argument Element.\n", fName);
	goto error;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(arg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Subscript Value of Argument is not valid.\n", fName);
        goto error;
    }

    /* get information about wait Argument */
    waitArgumentTransfer = session->ngs_waitArgumentTransfer;

    /* Is argument data copied? */
    if (waitArgumentTransfer == NG_ARGUMENT_TRANSFER_COPY) {
        result = ngiArgumentDataCopy(arg, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't copy the argument data.\n", fName);
            goto error;
        }
    }

    /*
     * Find the Session that status is Calculating.
     */
    /* Unlock the Executable */
    if (executableLocked != 0) {
	executableLocked = 0;
	result = ngcliExecutableUnlock(executable, log, error);
	if (result == 0) {
	    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the Executable.\n", fName);
	    goto error;
	}
    }

    /* Lock the list of Session */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't lock the list of Session.\n",
	    fName);
	goto error;
    }
    sessionListLocked = 1;

    /* Find the Session that status is calculating */
    for (tmpSession = NULL;
	(tmpSession = ngclSessionGetNext(executable, tmpSession, error))
	    != NULL;) {
	if (tmpSession == session)
	    continue;

	/* Is session not calculating? */
	if ((tmpSession->ngs_status <= NG_SESSION_STATUS_INITIALIZED) ||
	    (tmpSession->ngs_status >= NG_SESSION_STATUS_DONE))
	    continue;

	if (waitArgumentTransfer != NG_ARGUMENT_TRANSFER_WAIT) {
	    /* Set the status */
	    result = ngcliSessionStatusSet(
		session, NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE,
		log, error);
	    if (result == 0) {
		ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't set the Session Status.\n", fName);
		goto error;
	    }
	    goto success;
	} else {
	    /* Unock the list of Session */
	    if (sessionListLocked != 0) {
		sessionListLocked = 0;
		result = ngclSessionListReadUnlock(context, error);
		if (result == 0) {
		    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
			NG_LOG_LEVEL_FATAL, NULL,
			"%s: Can't unlock the list of Session.\n",
			fName);
		    goto error;
		}
	    }

	    /* Wait the session */
	    saveApiNext = tmpSession->ngs_apiNext;
	    tmpSession->ngs_apiNext = NULL;
	    result = ngclSessionWaitAll(context, NULL, tmpSession, error);
	    tmpSession->ngs_apiNext = saveApiNext;
	    if (result == 0) {
		ngclLogPrintfSession(tmpSession, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Can't wait the list of Session.\n", fName);
		goto error;
	    }

	    /* Lock the list of Session */
	    result = ngclSessionListReadLock(context, error);
	    if (result == 0) {
		ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		    NG_LOG_LEVEL_FATAL, NULL,
		    "%s: Can't lock the list.\n",
		    fName);
		goto error;
	    }
	    sessionListLocked = 1;
	}
    }

    /* Unock the list of Session */
    if (sessionListLocked != 0) {
	sessionListLocked = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n",
		fName);
	    goto error;
	}
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        return 0;
    }
    executableLocked = 1;

    /* Is executable not connected? And does not it wait for the completion
     * of transmission of arguments?
     */
    if ((executable->nge_status < NG_EXECUTABLE_STATUS_CONNECTED) &&
        (waitArgumentTransfer != NG_ARGUMENT_TRANSFER_WAIT)) {
        /* Set the status */
        result = ngcliSessionStatusSet(
            session, NG_SESSION_STATUS_WAIT_CONNECT, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the Session Status.\n", fName);
            goto error;
        }
	goto success;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* Wait Executable */
    result = ngcliExecutableStatusWaitIdle(executable, log, error);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Executable is not connected.\n", fName);
	goto error;
    }

    /* Lock the Executable */
    result = ngcliExecutableLockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        return 0;
    }
    executableLockedWithSend = 1;

    /* Is error occurred? */
    if (executable->nge_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR(error, executable->nge_error);
	goto error;
    }
    if (executable->nge_cbError != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR(error, executable->nge_cbError);
	goto error;
    }

    /* Unlock the Executable.
     *
     * Note:
     *   Unlock the Executable with sending maintained.
     *   Because, SESSION INVOKE is sending now.
     */
    executableLockedWithSend = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }

    /* Invoke the Session */
    result = ngcliProtocolRequestInvokeSession(
        session, executable->nge_protocol, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't invoke the Session.\n", fName);
        goto error;
    }

    /* Does not it wait for the completion of transmission of arguments? */
    if (waitArgumentTransfer != NG_ARGUMENT_TRANSFER_WAIT) {
        /* Success */
        return 1;
    }

    /* Wait the status of Session to grow into TRANSARG_DONE */
    result = ngcliSessionStatusWaitGreaterEqual(
        session, NG_SESSION_STATUS_TRANSARG_DONE, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Can't wait the status.\n", fName);
        goto error;
    }

    /* Success */
    return 1;

success:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
	    goto error;
        }
    }

    /* Unock the list of Session */
    if (sessionListLocked != 0) {
	sessionListLocked = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n",
		fName);
	    goto error;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Lock the Executable with send */
    if (executableLockedWithSend != 0) {
        executableLockedWithSend = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Unock the list of Session */
    if (sessionListLocked != 0) {
	sessionListLocked = 0;
	result = ngclSessionListReadUnlock(context, error);
	if (result == 0) {
	    ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: Can't unlock the list of Session.\n",
		fName);
	}
    }

    /* Failed */
    return 0;
}

/**
 * Invoke session with argument stack.
 */
int
ngclSessionStartWithArgumentStack(
    ngclSession_t *session,
    char *methodName,
    ngclArgumentStack_t *argStack,
    int *error)
{
#ifdef NG_OS_OSF5_1
    va_list ap;
#define AP ap
#else /* NG_OS_OSF5_1 */
#define AP NULL
#endif /* NG_OS_OSF5_1 */
    int local_error, result;
    static const char fName[] = "ngclSessionStartWithArgumentStack";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(NULL, NULL, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    /* Check the arguments */
    if (argStack == NULL) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid argument: Argument stack is NULL.\n", fName);
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	return 0;
    }

    /* Start the session */
    result = ngcllSessionStart(
	session, methodName, argStack, AP, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

/**
 * Suspend session.
 */
int
ngclSessionSuspend(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionSuspend";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(context, executable, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionSuspend(context, executable, session, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionSuspend(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
#if 0 /* Temporary commented out */
    int result;
    static const char fName[] = "ngcllSessionSuspend";

    /* Suspend session */
    result = ngcliProtocolSuspendSession(
	session, executable->nge_protocol, methodID, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't process the Suspend Session.\n", fName);
	return 0;
    }

#endif /* Temporary commented out */
    /* Success */
    return 1;
}

/**
 * Resume session.
 */
int
ngclSessionResume(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int local_error, result;
    static const char fName[] = "ngclSessionResume";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Session valid? */
    result = ngcliSessionIsValid(context, executable, session, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Session is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllSessionResume(context, executable, session, &local_error);
    if (local_error != NG_ERROR_NO_ERROR) {
	NGI_SET_ERROR_SESSION(session, local_error, NULL);
    }
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllSessionResume(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
#if 0 /* Temporary commented out */
    int result;
    static const char fName[] = "ngcllSessionResume";

    /* Resume session */
    result = ngclProtocolResumeSession(
	session, executable->nge_protocol, methodID, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't process the Resume Session.\n", fName);
	return 0;
    }
#endif /* Temporary commented out */

    /* Success */
    return 1;
}

/**
 * Cancel session.
 */
int
ngclSessionCancel(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    static const char fName[] = "ngclSessionCancel";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Initialize the local variables */
    if (session != NULL) {
	if (session->ngs_context != NULL)
	    log = session->ngs_context->ngc_log;
    } else if (executable != NULL) {
	if (executable->nge_context != NULL)
	    log = executable->nge_context->ngc_log;
    } else if (context != NULL) {
	log = context->ngc_log;
    }

    /* Cancel all sessions */
    result = ngcllSessionCancelAllSession(context, session, log, error);
    if (result == 0) {
	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't cancel sessions.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Cancel all sessions.
 */
static int
ngcllSessionCancelAllSession(
    ngclContext_t *context,
    ngclSession_t *list,
    ngLog_t *log,
    int *error)
{
    int result, retResult = 1;
    ngclSession_t *curr;
    static const char fName[] = "ngcllSessionCancelAllSession";

    /* Check the arguments */
    assert(context != NULL);
    assert(list != NULL);

    for (curr = list; curr != NULL; curr = curr->ngs_cancelNext) {
        result = ngcllSessionCancelSingle(curr, log, error);
        if (result == 0) {
            retResult = 0;
            ngclLogPrintfSession(curr, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't cancel the Session.\n", fName);
        }
    }

    /* Finish */
    return retResult;
}

/**
 * Cancel the Session.
 */
static int
ngcllSessionCancelSingle(ngclSession_t *session, ngLog_t *log, int *error)
{
    int result;
    int executableLocked = 0;
    int executableLockedWithSend = 0;
    int sendPermission;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllSessionCancelSingle";

    /* Check the arguments */
    assert(session != NULL);
    assert(session->ngs_executable != NULL);

    /* Initialize the local variables */
    executable = session->ngs_executable;

    /* Print the debug message */
    ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_DEBUG, NULL, "%s: Cancel the Session.\n", fName);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Executable.\n", fName);
        goto error;
    }
    executableLocked = 1;

    /* Is error occurred? */
    if (session->ngs_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, session->ngs_error);
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error (%d) has already occurred.\n",
            fName, session->ngs_error);
        goto error;
    }

    /* Set the error code */
    session->ngs_error = NG_ERROR_CANCELED;

    switch (session->ngs_status) {
    case NG_SESSION_STATUS_INITIALIZED:
    case NG_SESSION_STATUS_WAIT_CONNECT:
    case NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE:
        result = ngcliSessionStatusSetDone(session, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the status.\n", fName);
            goto error;
        }
        break;

    case NG_SESSION_STATUS_INVOKE_REQUESTED:
    case NG_SESSION_STATUS_TRANSARG_REQUESTED:
    case NG_SESSION_STATUS_SUSPEND_REQUESTED:
    case NG_SESSION_STATUS_RESUME_REQUESTED:
        /* Print the debug message */
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Set the cancel request.\n", fName);

        session->ngs_cancelRequest = 1;
        break;

    case NG_SESSION_STATUS_INVOKE_DONE:
    case NG_SESSION_STATUS_TRANSARG_DONE:
    case NG_SESSION_STATUS_CALCULATE_EXECUTING:
    case NG_SESSION_STATUS_CALCULATE_DONE:
    case NG_SESSION_STATUS_SUSPEND_DONE:
    case NG_SESSION_STATUS_RESUME_DONE:
    case NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED:
        /* Try lock for send */
        result = ngcliExecutableLockTrySend(
            executable, &sendPermission, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL, "Can't lock for send.\n", fName);
            goto error;
        }
        if (sendPermission == 0) {
            executableLocked = 0;
            result = ngcliExecutableUnlock(executable, log, error);
            if (result == 0) {
                ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't unlock the Executable.\n", fName);
                goto error;
            }

            /* Lock the Executable */
            result = ngcliExecutableLockWithSend(executable, log, error);
            if (result == 0) {
                ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't lock the Executable.\n", fName);
                return 0;
            }
        }

        /* Unlock the Executable.
         *
         * Note:
         *   Unlock the Executable with sending maintained.
         *   Because, SESSION INVOKE is sending now.
         */
        executableLocked = 0;
        executableLockedWithSend = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
            goto error;
        }

        result = ngcliProtocolRequestCancelSession(
            session, executable->nge_protocol, log, error);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't send the request.\n", fName);
            goto error;
        }
        goto wait;

    case NG_SESSION_STATUS_CANCEL_REQUESTED:
    case NG_SESSION_STATUS_CANCEL_DONE:
        /* Print the debug message */
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Do nothing.\n", fName);

        /* Do nothing */
        break;

    case NG_SESSION_STATUS_CB_TRANSARG_REQUESTED:
    case NG_SESSION_STATUS_CB_TRANSARG_DONE:
    case NG_SESSION_STATUS_CB_EXECUTING:
    case NG_SESSION_STATUS_CB_EXECUTE_DONE:
    case NG_SESSION_STATUS_CB_TRANSRES_REQUESTED:
    case NG_SESSION_STATUS_CB_TRANSRES_DONE:
        /* Wait cancel in callback.
         * 
         * Note:
         *   After Ninf-G Ver.2.0.0 release is corrected 
         *   although cancel is waited in Ninf-G Ver.2.0.0.
         */
        /* Print the debug message */
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Set the cancel request.\n", fName);

        session->ngs_cancelRequest = 1;
        break;

    case NG_SESSION_STATUS_TRANSRES_REQUESTED:
    case NG_SESSION_STATUS_TRANSRES_DONE:
    case NG_SESSION_STATUS_PULLBACK_REQUESTED:
    case NG_SESSION_STATUS_PULLBACK_DONE:
    case NG_SESSION_STATUS_DONE:
        /* Print the debug message */
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Do nothing.\n", fName);

        /* Do nothing */
        break;
    }

    /* Unlock the Executable.
     *
     * Note:
     *   Unlock the Executable with sending maintained.
     *   Because, SESSION INVOKE is sending now.
     */
    executableLocked = 0;
    executableLockedWithSend = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Executable.\n", fName);
        goto error;
    }

    /* After canceled */
wait:
    /* Finish */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if ((executableLocked != 0) || executableLockedWithSend != 0) {
        executableLocked = 0;
        executableLockedWithSend = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Executable.\n", fName);
        }
    }

    /* Failed */
    return 0;
}
/**
 * Wait all sessions.
 */
int
ngclSessionWaitAll(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int result;
    int retResult = 1;
    ngclSession_t *list;
    ngLog_t *log = NULL;
    int localError;
    static const char fName[] = "ngclSessionWaitAll";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    localError = NG_ERROR_NO_ERROR;

    /* Initialize the local variables */
    if (session != NULL) {
	if (session->ngs_context != NULL)
	    log = session->ngs_context->ngc_log;
    } else if (executable != NULL) {
	if (executable->nge_context != NULL)
	    log = executable->nge_context->ngc_log;
    } else if (context != NULL)
	log = context->ngc_log;
    else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
	return 0;
    }

    /* Get the waiting list */
    list = ngcliSessionGetWaitList(context, executable, session, log,
        &localError);
    if (localError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, localError);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the list of Session.\n", fName);
	return 0;
    }
    assert((session == NULL) || (list != NULL));

    /* Wait the all Session which specified */
    if ((context == NULL) && (executable != NULL))
	context = executable->nge_context;
    if ((context == NULL) && (session != NULL))
	context = session->ngs_context;
    if (context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't find the Ninf-G Context.\n", fName);
	retResult = 0;
	error = NULL;
	goto error;
    }
    result = ngcliContextWaitAllSession(context, list, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't wait Session.\n", fName);
	retResult = 0;
	error = NULL;
	goto error;
    }

    /* Release the waiting list */
error:
    result = ngcliSessionReleaseWaitList(list, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the list.\n", fName);
	return 0;
    }

    /* Success */
    return retResult;
}

/**
 * Wait any sessions.
 */
int
ngclSessionWaitAny(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    int *error)
{
    int result;
    int sessionID = -1;
    ngclSession_t *list;
    ngLog_t *log = NULL;
    int localError;
    static const char fName[] = "ngclSessionWaitAny";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    localError = NG_ERROR_NO_ERROR;

    /* Initialize the local variables */
    if (session != NULL) {
	if (session->ngs_context != NULL)
	    log = session->ngs_context->ngc_log;
    } else if (executable != NULL) {
	if (executable->nge_context != NULL)
	    log = executable->nge_context->ngc_log;
    } else if (context != NULL)
	log = context->ngc_log;
    else {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
	return -1;
    }

    /* Get the waiting list */
    list = ngcliSessionGetWaitList(context, executable, session, log, &localError);
    if (localError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, localError);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the list of Session.\n", fName);
	return -1;
    }
    assert((session != NULL) || (list != NULL));

    /* Wait the any Session which specified */
    if ((context == NULL) && (executable != NULL))
	context = executable->nge_context;
    if ((context == NULL) && (session != NULL))
	context = session->ngs_context;
    if (context == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't find the Ninf-G Context.\n", fName);
	sessionID = -1;
	error = NULL;
	goto error;
    }
    sessionID = ngcliContextWaitAnySession(context, list, log, &localError);
    if (localError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, localError);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL, "%s: Can't wait Session.\n", fName);
	sessionID = -1;
	error = NULL;
	goto error;
    }
    assert((session != NULL) || (list != NULL));

    /* Release the waiting list */
error:
    result = ngcliSessionReleaseWaitList(list, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the list.\n", fName);
	return -1;
    }

    return sessionID;
}

/*
 * Invoke Callback
 */
int
ngcliSessionInvokeCallback(
    void (*callbackFunc)(),
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcliSessionInvokeCallback";

    switch (arg->nga_nArguments){
    case  0:
        (void)(*callbackFunc)();
        break;
    case  1:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void);
        break;
    case  2:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void);
        break;
    case  3:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void);
        break;
    case  4:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void);
        break;
    case  5:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void);
        break;
    case  6:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void);
        break;
    case  7:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void);
        break;
    case  8:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void);
        break;
    case  9:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void);
        break;
    case 10:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void);
        break;
    case 11:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void);
        break;
    case 12:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void);
        break;
    case 13:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void);
        break;
    case 14:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void);
        break;
    case 15:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void);
        break;
    case 16:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void);
        break;
    case 17:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void);
        break;
    case 18:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void);
        break;
    case 19:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void);
        break;
    case 20:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void);
        break;
    case 21:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void);
        break;
    case 22:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void);
    case 23:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void);
        break;
    case 24:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void);
        break;
    case 25:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void);
        break;
    case 26:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void);
        break;
    case 27:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void);
        break;
    case 28:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void,
             arg->nga_argument[27].ngae_pointer.ngap_void);
        break;
    case 29:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void,
             arg->nga_argument[27].ngae_pointer.ngap_void,
             arg->nga_argument[28].ngae_pointer.ngap_void);
        break;
    case 30:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void,
             arg->nga_argument[27].ngae_pointer.ngap_void,
             arg->nga_argument[28].ngae_pointer.ngap_void,
             arg->nga_argument[29].ngae_pointer.ngap_void);
        break;
    case 31:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void,
             arg->nga_argument[27].ngae_pointer.ngap_void,
             arg->nga_argument[28].ngae_pointer.ngap_void,
             arg->nga_argument[29].ngae_pointer.ngap_void,
             arg->nga_argument[30].ngae_pointer.ngap_void);
        break;
    case 32:
        (void)(*callbackFunc)(
             arg->nga_argument[0].ngae_pointer.ngap_void,
             arg->nga_argument[1].ngae_pointer.ngap_void,
             arg->nga_argument[2].ngae_pointer.ngap_void,
             arg->nga_argument[3].ngae_pointer.ngap_void,
             arg->nga_argument[4].ngae_pointer.ngap_void,
             arg->nga_argument[5].ngae_pointer.ngap_void,
             arg->nga_argument[6].ngae_pointer.ngap_void,
             arg->nga_argument[7].ngae_pointer.ngap_void,
             arg->nga_argument[8].ngae_pointer.ngap_void,
             arg->nga_argument[9].ngae_pointer.ngap_void,
             arg->nga_argument[10].ngae_pointer.ngap_void,
             arg->nga_argument[11].ngae_pointer.ngap_void,
             arg->nga_argument[12].ngae_pointer.ngap_void,
             arg->nga_argument[13].ngae_pointer.ngap_void,
             arg->nga_argument[14].ngae_pointer.ngap_void,
             arg->nga_argument[15].ngae_pointer.ngap_void,
             arg->nga_argument[16].ngae_pointer.ngap_void,
             arg->nga_argument[17].ngae_pointer.ngap_void,
             arg->nga_argument[18].ngae_pointer.ngap_void,
             arg->nga_argument[19].ngae_pointer.ngap_void,
             arg->nga_argument[20].ngae_pointer.ngap_void,
             arg->nga_argument[21].ngae_pointer.ngap_void,
             arg->nga_argument[22].ngae_pointer.ngap_void,
             arg->nga_argument[23].ngae_pointer.ngap_void,
             arg->nga_argument[24].ngae_pointer.ngap_void,
             arg->nga_argument[25].ngae_pointer.ngap_void,
             arg->nga_argument[26].ngae_pointer.ngap_void,
             arg->nga_argument[27].ngae_pointer.ngap_void,
             arg->nga_argument[28].ngae_pointer.ngap_void,
             arg->nga_argument[29].ngae_pointer.ngap_void,
             arg->nga_argument[30].ngae_pointer.ngap_void,
             arg->nga_argument[31].ngae_pointer.ngap_void);
        break;
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Too many argument for callback.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}


/**
 * Session status to string.
 */
char *
ngcliSessionStatusToString(ngclSessionStatus_t status)
{
    switch (status) {
    case NG_SESSION_STATUS_INITIALIZED:
        return "INITIALIZED";

    case NG_SESSION_STATUS_WAIT_CONNECT:
        return "WAIT CONNECT";

    case NG_SESSION_STATUS_WAIT_PREVIOUS_SESSION_WAS_DONE:
	return "WAIT PREVIOUS SESSION WAS DONE";

    case NG_SESSION_STATUS_INVOKE_REQUESTED:
        return "INVOKE REQUESTED";

    case NG_SESSION_STATUS_INVOKE_DONE:
        return "INVOKE DONE";

    case NG_SESSION_STATUS_TRANSARG_REQUESTED:
        return "TRANSARG REQUESTED";

    case NG_SESSION_STATUS_TRANSARG_DONE:
        return "TRANSARG DONE";

    case NG_SESSION_STATUS_CALCULATE_EXECUTING:
        return "CALCULATE EXECUTING";

    case NG_SESSION_STATUS_CALCULATE_DONE:
        return "CALCULATE DONE";

    case NG_SESSION_STATUS_SUSPEND_REQUESTED:
        return "SUSPEND REQUESTED";

    case NG_SESSION_STATUS_SUSPEND_DONE:
        return "SUSPEND DONE";

    case NG_SESSION_STATUS_RESUME_REQUESTED:
        return "RESUME REQUESTED";

    case NG_SESSION_STATUS_RESUME_DONE:
        return "RESUME DONE";

    case NG_SESSION_STATUS_CANCEL_REQUESTED:
        return "CANCEL REQUESTED";

    case NG_SESSION_STATUS_CANCEL_DONE:
        return "CANCEL DONE";

    case NG_SESSION_STATUS_TRANSRES_REQUESTED:
        return "TRANSRES REQUESTED";

    case NG_SESSION_STATUS_TRANSRES_DONE:
        return "TRANSRES DONE";

    case NG_SESSION_STATUS_PULLBACK_REQUESTED:
        return "PULLBACK REQUESTED";

    case NG_SESSION_STATUS_PULLBACK_DONE:
        return "PULLBACK DONE";

    case NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED:
        return "INVOKE CALLBACK RECEIVED";

    case NG_SESSION_STATUS_CB_TRANSARG_REQUESTED:
        return "CB TRANSARG REQUESTED";

    case NG_SESSION_STATUS_CB_TRANSARG_DONE:
        return "CB TRANSARG DONE";

    case NG_SESSION_STATUS_CB_EXECUTING:
        return "CB EXECUTING";

    case NG_SESSION_STATUS_CB_EXECUTE_DONE:
        return "CB EXECUTE DONE";

    case NG_SESSION_STATUS_CB_TRANSRES_REQUESTED:
        return "CB TRANSRES REQUESTED";

    case NG_SESSION_STATUS_CB_TRANSRES_DONE:
        return "CB TRANSRES DONE";

    case NG_SESSION_STATUS_DONE:
        return "DONE";
    }

    return "Unknown status";
}

/**
 * SessionAttribute Initialize
 */
int
ngclSessionAttributeInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllSessionAttributeInitialize(context,
        executable, sessionAttr, error);
}

static int
ngcllSessionAttributeInitialize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllSessionAttributeInitialize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = executable->nge_context->ngc_log;

    /* Check the arguments */
    if (sessionAttr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: The Session Attribute is NULL.\n", fName);
        return 0;
    }

    ngcllSessionAttributeInitializeMember(sessionAttr);

    /* Success */
    return 1;
}

static void
ngcllSessionAttributeInitializeMember(
    ngclSessionAttribute_t *sessionAttr)
{
    assert(sessionAttr != NULL);

    ngcllSessionAttributeInitializePointer(sessionAttr);

    sessionAttr->ngsa_waitArgumentTransfer = NG_ARGUMENT_TRANSFER_UNDEFINED;
    sessionAttr->ngsa_timeout = NGCL_SESSION_ATTRIBUTE_MEMBER_UNDEFINED;
}

static void
ngcllSessionAttributeInitializePointer(
    ngclSessionAttribute_t *sessionAttr)
{
    assert(sessionAttr != NULL);

    /* nothing will be done here */
}

/**
 * SessionAttribute Finalize
 */
int
ngclSessionAttributeFinalize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    /* just wrap for user */
    return ngcllSessionAttributeFinalize(context,
        executable, sessionAttr, error);
}

static int
ngcllSessionAttributeFinalize(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSessionAttribute_t *sessionAttr,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngcllSessionAttributeFinalize";

    /* Clear the error */
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Executable valid? */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
        return 0;
    }

    log = executable->nge_context->ngc_log;

    /* Check the arguments */
    if (sessionAttr == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: The Session Attribute is NULL.\n", fName);
        return 0;
    }

    ngcllSessionAttributeInitializeMember(sessionAttr);

    /* Success */
    return 1;
}

