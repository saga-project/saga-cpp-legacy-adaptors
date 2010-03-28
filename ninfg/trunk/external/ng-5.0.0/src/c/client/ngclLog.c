/*
 * $RCSfile: ngclLog.c,v $ $Revision: 1.5 $ $Date: 2007/11/27 02:27:41 $
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
 * Module of Log for Ninf-G.
 */

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngclLog.c,v $ $Revision: 1.5 $ $Date: 2007/11/27 02:27:41 $")

#define NGCLL_LOG_OBJECT_STRING_LENGTH_MAX 256

static void ngcllLogVprintfContext(ngclContext_t *,
    const char *, ngLogLevel_t, const char *, char *, va_list);
static void ngcllLogVprintfJob(ngcliJobManager_t *,
    const char *, ngLogLevel_t, const char *, char *, va_list);
static void ngcllLogVprintfExecutable(ngclExecutable_t *,
    const char *, ngLogLevel_t, const char *, char *, va_list);
static void ngcllLogVprintfSession(ngclSession_t *,
    const char *, ngLogLevel_t, const char *, char *, va_list);

/**
 * Printf for Ninf-G Context.
 */

void
ngclLogPrintfContext(
    ngclContext_t *context,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    ...)
{
    va_list ap;

    va_start(ap, format);
    ngcllLogVprintfContext(context, category, level, func_name, format, ap);
    va_end(ap);
}

/**
 * Printf for Job Manager.
 */
void
ngcliLogPrintfJob(
    ngcliJobManager_t *jobMng,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    ...)
{
    va_list ap;

    va_start(ap, format);
    ngcllLogVprintfJob(jobMng, category, level, func_name, format, ap);
    va_end(ap);
}

/**
 * Printf for Executable Handle.
 */
void
ngclLogPrintfExecutable(
    ngclExecutable_t *executable,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    ...)
{
    va_list ap;

    va_start(ap, format);
    ngcllLogVprintfExecutable(executable,
        category, level, func_name, format, ap);
    va_end(ap);
}

/**
 * Printf for Session Manager.
 */
void
ngclLogPrintfSession(
    ngclSession_t *session,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    ...)
{
    va_list ap;

    va_start(ap, format);
    ngcllLogVprintfSession(session,
        category, level, func_name, format, ap);
    va_end(ap);
}

/**
 * Vprintf for Ninf-G Context.
 */
static void
ngcllLogVprintfContext(
    ngclContext_t *context,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    va_list ap)
{
    char buf[NGCLL_LOG_OBJECT_STRING_LENGTH_MAX];

    /* Is Ninf-G Context not valid? */
    if (context == NULL) {
	ngLogVprintf(NULL, category, level, NULL, func_name, format, ap);
    } else {
	snprintf(buf, sizeof (buf), "Context %d", context->ngc_ID);
	ngLogVprintf(context->ngc_log,
            category, level, buf, func_name, format, ap);
    }
}

/**
 * Vprintf for Job Manager.
 */
static void
ngcllLogVprintfJob(
    ngcliJobManager_t *jobMng,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    va_list ap)
{
    ngclContext_t *context;
    char buf[NGCLL_LOG_OBJECT_STRING_LENGTH_MAX];

    /* Is Job Manager not valid? */
    if ((jobMng == NULL) || (jobMng->ngjm_context == NULL)) {
	ngLogVprintf(NULL, category, level, NULL, func_name, format, ap);
    } else {
	context = jobMng->ngjm_context;
	snprintf(buf, sizeof (buf), "Context %d: Job %d",
	    context->ngc_ID, jobMng->ngjm_ID);
	ngLogVprintf(context->ngc_log,
            category, level, buf, func_name, format, ap);
    }
}

/**
 * Vprintf for Executable Handle.
 */
static void
ngcllLogVprintfExecutable(
    ngclExecutable_t *executable,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    va_list ap)
{
    ngclContext_t *context;
    char buf[NGCLL_LOG_OBJECT_STRING_LENGTH_MAX];

    /* Is Executable not valid? */
    if ((executable == NULL) || (executable->nge_context == NULL)) {
	ngLogVprintf(NULL, category, level, NULL, func_name, format, ap);
    } else {
	context = executable->nge_context;
	snprintf(buf, sizeof (buf), "Context %d: Executable %d",
	    context->ngc_ID, executable->nge_ID);
	ngLogVprintf(context->ngc_log,
            category, level, buf, func_name, format, ap);
    }
}

/**
 * Vprintf for Session Manager.
 */
static void
ngcllLogVprintfSession(
    ngclSession_t *session,
    const char *category,
    ngLogLevel_t level,
    const char *func_name,
    char *format,
    va_list ap)
{
    ngclContext_t *context;
    ngclExecutable_t *executable;
    char buf[NGCLL_LOG_OBJECT_STRING_LENGTH_MAX];

    /* Is Session not valid? */
    if ((session == NULL) || (session->ngs_executable == NULL) ||
        (session->ngs_executable->nge_context == NULL)) {
	ngLogVprintf(NULL, category, level, NULL, func_name, format, ap);
    } else {
	context = session->ngs_context;
	executable = session->ngs_executable;
	snprintf(buf, sizeof (buf), "Context %d: Executable %d: Session %d",
	    context->ngc_ID, executable->nge_ID, session->ngs_ID);
	ngLogVprintf(context->ngc_log,
            category, level, buf, func_name, format, ap);
    }
}

#define NGCLL_LOG_FUNC_DEFINE(func, vfprintf, object_t, level) \
void                                                            \
func (                                                          \
    object_t *obj,                                              \
    const char *category,                                       \
    const char *funcName,                                       \
    char *fmt,                                                  \
    ...)                                                        \
{                                                               \
    va_list ap;                                                 \
                                                                \
    va_start(ap, fmt);                                          \
    vfprintf(obj, category, level, funcName, fmt, ap);          \
    va_end(ap);                                                 \
                                                                \
    return;                                                     \
}

NGCLL_LOG_FUNC_DEFINE(ngclLogDebugContext, ngcllLogVprintfContext, ngclContext_t, NG_LOG_LEVEL_DEBUG)
NGCLL_LOG_FUNC_DEFINE(ngclLogInfoContext,  ngcllLogVprintfContext, ngclContext_t, NG_LOG_LEVEL_INFORMATION)
NGCLL_LOG_FUNC_DEFINE(ngclLogWarnContext,  ngcllLogVprintfContext, ngclContext_t, NG_LOG_LEVEL_WARNING)
NGCLL_LOG_FUNC_DEFINE(ngclLogErrorContext, ngcllLogVprintfContext, ngclContext_t, NG_LOG_LEVEL_ERROR)
NGCLL_LOG_FUNC_DEFINE(ngclLogFatalContext, ngcllLogVprintfContext, ngclContext_t, NG_LOG_LEVEL_FATAL)

NGCLL_LOG_FUNC_DEFINE(ngcliLogDebugJob, ngcllLogVprintfJob, ngcliJobManager_t, NG_LOG_LEVEL_DEBUG)
NGCLL_LOG_FUNC_DEFINE(ngcliLogInfoJob,  ngcllLogVprintfJob, ngcliJobManager_t, NG_LOG_LEVEL_INFORMATION)
NGCLL_LOG_FUNC_DEFINE(ngcliLogWarnJob,  ngcllLogVprintfJob, ngcliJobManager_t, NG_LOG_LEVEL_WARNING)
NGCLL_LOG_FUNC_DEFINE(ngcliLogErrorJob, ngcllLogVprintfJob, ngcliJobManager_t, NG_LOG_LEVEL_ERROR)
NGCLL_LOG_FUNC_DEFINE(ngcliLogFatalJob, ngcllLogVprintfJob, ngcliJobManager_t, NG_LOG_LEVEL_FATAL)

NGCLL_LOG_FUNC_DEFINE(ngclLogDebugExecutable, ngcllLogVprintfExecutable, ngclExecutable_t, NG_LOG_LEVEL_DEBUG)
NGCLL_LOG_FUNC_DEFINE(ngclLogInfoExecutable,  ngcllLogVprintfExecutable, ngclExecutable_t, NG_LOG_LEVEL_INFORMATION)
NGCLL_LOG_FUNC_DEFINE(ngclLogWarnExecutable,  ngcllLogVprintfExecutable, ngclExecutable_t, NG_LOG_LEVEL_WARNING)
NGCLL_LOG_FUNC_DEFINE(ngclLogErrorExecutable, ngcllLogVprintfExecutable, ngclExecutable_t, NG_LOG_LEVEL_ERROR)
NGCLL_LOG_FUNC_DEFINE(ngclLogFatalExecutable, ngcllLogVprintfExecutable, ngclExecutable_t, NG_LOG_LEVEL_FATAL)

NGCLL_LOG_FUNC_DEFINE(ngclLogDebugSession, ngcllLogVprintfSession, ngclSession_t, NG_LOG_LEVEL_DEBUG)
NGCLL_LOG_FUNC_DEFINE(ngclLogInfoSession,  ngcllLogVprintfSession, ngclSession_t, NG_LOG_LEVEL_INFORMATION)
NGCLL_LOG_FUNC_DEFINE(ngclLogWarnSession,  ngcllLogVprintfSession, ngclSession_t, NG_LOG_LEVEL_WARNING)
NGCLL_LOG_FUNC_DEFINE(ngclLogErrorSession, ngcllLogVprintfSession, ngclSession_t, NG_LOG_LEVEL_ERROR)
NGCLL_LOG_FUNC_DEFINE(ngclLogFatalSession, ngcllLogVprintfSession, ngclSession_t, NG_LOG_LEVEL_FATAL)
