#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclLog.c,v $ $Revision: 1.14 $ $Date: 2004/03/11 07:24:29 $";
#endif /* NG_OS_IRIX */
/**
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
 * Module of Log for Ninf-G.
 */

#include <stdlib.h>
#if 0 /* Is this necessary? */
#include <varargs.h>
#endif
#include "ng.h"

/**
 * Printf for Ninf-G Context.
 */
int
ngclLogPrintfContext(
    ngclContext_t *context,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    ...)
{
    va_list ap;
    char buf[256];

    /* Get the argument pointer */
    va_start(ap, format);

    /* Is Ninf-G Context not valid? */
    if (context == NULL) {
	ngiLogVprintf(NULL, category, level, error, NULL, format, ap);
    } else {
	snprintf(buf, sizeof (buf), "Context %d: ", context->ngc_ID);
	ngiLogVprintf(context->ngc_log, category, level, error, buf,
	    format, ap);
    }

    /* Release the argument pointer */
    va_end(ap);

    return 1;
}

/**
 * Printf for Job Manager.
 */
int
ngcliLogPrintfJob(
    ngcliJobManager_t *jobMng,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    ...)
{
    va_list ap;
    ngclContext_t *context;
    char buf[256];

    /* Get the argument pointer */
    va_start(ap, format);

    /* Is Job Manager not valid? */
    if ((jobMng == NULL) || (jobMng->ngjm_context == NULL)) {
	ngiLogVprintf(NULL, category, level, error, NULL, format, ap);
    } else {
	context = jobMng->ngjm_context;
	snprintf(buf, sizeof (buf), "Context %d: Job %d: ",
	    context->ngc_ID, jobMng->ngjm_ID);
	ngiLogVprintf(context->ngc_log, category, level, error, buf,
	    format, ap);
    }

    /* Release the argument pointer */
    va_end(ap);

    return 1;
}

/**
 * Printf for Executable Handle.
 */
int
ngclLogPrintfExecutable(
    ngclExecutable_t *executable,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    ...)
{
    va_list ap;
    ngclContext_t *context;
    char buf[256];

    /* Get the argument pointer */
    va_start(ap, format);

    /* Is Executable not valid? */
    if ((executable == NULL) || (executable->nge_context == NULL)) {
	ngiLogVprintf(NULL, category, level, error, NULL, format, ap);
    } else {
	context = executable->nge_context;
	snprintf(buf, sizeof (buf), "Context %d: Executable %d: ",
	    context->ngc_ID, executable->nge_ID);
	ngiLogVprintf(context->ngc_log, category, level, error, buf,
	    format, ap);
    }

    /* Release the argument pointer */
    va_end(ap);

    return 1;
}

/**
 * Printf for Session Manager.
 */
int
ngclLogPrintfSession(
    ngclSession_t *session,
    ngLogCategory_t category,
    ngLogLevel_t level,
    int *error,
    char *format,
    ...)
{
    va_list ap;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    char buf[256];

    /* Get the argument pointer */
    va_start(ap, format);

    /* Is Session not valid? */
    if ((session == NULL) || (session->ngs_executable == NULL) ||
        (session->ngs_executable->nge_context == NULL)) {
	ngiLogVprintf(NULL, category, level, error, NULL, format, ap);
    } else {
	context = session->ngs_context;
	executable = session->ngs_executable;
	snprintf(buf, sizeof (buf), "Context %d: Executable %d: Session %d: ",
	    context->ngc_ID, executable->nge_ID, session->ngs_ID);
	ngiLogVprintf(context->ngc_log, category, level, error, buf,
	    format, ap);
    }

    /* Release the argument pointer */
    va_end(ap);

    return 1;
}
