#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclUtility.c,v $ $Revision: 1.11 $ $Date: 2004/03/12 08:06:20 $";
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
 * @file ngiUtility.c
 * Utility module for Ninf-G internal.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static int ngcllRWlockInitialize(ngclContext_t *, ngRWlock_t *, int *);
static int ngcllRWlockFinalize(ngclContext_t *, ngRWlock_t *, int *);
static int ngcllRWlockReadLock(ngclContext_t *, ngRWlock_t *, int *);
static int ngcllRWlockReadUnlock(ngclContext_t *, ngRWlock_t *, int *);
static int ngcllRWlockWriteLock(ngclContext_t *, ngRWlock_t *, int *);
static int ngcllRWlockWriteUnlock(ngclContext_t *, ngRWlock_t *, int *);

/**
 * Read/Write Lock: Initialize
 */
int
ngclRWlockInitialize(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockInitialize";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockInitialize(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockInitialize(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockInitialize";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockInitialize(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Finalize
 */
int
ngclRWlockFinalize(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockFinalize";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockFinalize(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockFinalize(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockFinalize";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockFinalize(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Read lock
 */
int
ngclRWlockReadLock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockReadLock";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockReadLock(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockReadLock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockReadLock";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockReadLock(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Read unlock
 */
int
ngclRWlockReadUnlock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockReadUnlock";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockReadUnlock(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockReadUnlock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockReadUnlock";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockReadUnlock(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Write lock
 */
int
ngclRWlockWriteLock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockWriteLock";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockWriteLock(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockWriteLock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockWriteLock";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockWriteLock(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Read/Write Lock: Write unlock
 */
int
ngclRWlockWriteUnlock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int local_error, result;
    static const char fName[] = "ngclRWlockWriteUnlock";

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    /* Is Ninf-G Context valid? */
    result = ngcliContextIsValid(context, &local_error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Ninf-G Context is not valid.\n", fName);
	NGI_SET_ERROR(error, local_error);
        return 0;
    }

    result = ngcllRWlockWriteUnlock(context, rwLock, &local_error);
    NGI_SET_ERROR_CONTEXT(context, local_error, NULL);
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngcllRWlockWriteUnlock(ngclContext_t *context, ngRWlock_t *rwLock, int *error)
{
    int result;
    static const char fName[] = "ngcllRWlockWriteUnlock";

    /* Initialize the Read/Write Lock */
    result = ngiRWlockWriteUnlock(rwLock, context->ngc_log, error);
    if (result == 0) {
    	ngclLogPrintfContext(context, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't Finalize the Read/Write Lock.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get the version number of Ninf-G.
 */
char *
ngclGetVersion(int *error)
{
    char *versionFull, *versionStart, *versionReturn, *versionEnd;

    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    versionFull = "$AIST_Release: 4.2.4 $";

    /**
     * If version string replaced to the keyword by the release script,
     * a character : (colon) will appear.
     * If not, then the source was CVS checked out one.
     * That's not released Ninf-G.
     */
    versionStart = strchr(versionFull, ':');
    if (versionStart != NULL) {
        /* skip first ' ' */
        while (*versionStart == ' ') {
            versionStart++;
        }

        versionReturn = strdup(versionStart);

        /* cut last '$' */
        versionEnd = strrchr(versionReturn, '$');
        if (versionEnd != NULL) {
            *versionEnd = '\0';
        }
    } else {
        versionReturn = strdup("CVS");
    }

    /* Success */
    return versionReturn;
}

