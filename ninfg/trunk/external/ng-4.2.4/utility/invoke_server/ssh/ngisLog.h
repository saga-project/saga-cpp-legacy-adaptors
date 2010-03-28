/*
 * $RCSfile: ngisLog.h,v $ $Revision: 1.2 $ $Date: 2006/08/19 12:56:40 $
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
#ifndef _NGIS_LOG_H_
#define _NGIS_LOG_H_

#include <stdarg.h>
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include "ngisUtility.h"

typedef enum ngisLogLevel_e {
    NGIS_LOG_LEVEL_OFF = 0,
    NGIS_LOG_LEVEL_ABORT,
    NGIS_LOG_LEVEL_ERROR,
    NGIS_LOG_LEVEL_WARNING,
    NGIS_LOG_LEVEL_DEBUG
} ngisLogLevel_t;

/**
 * Log
 */
typedef struct ngisLog_s {
    char *ngl_moduleName;
} ngisLog_t;

int ngisLogInitializeModule(const char *, ngisLogLevel_t);
int ngisLogFinalizeModule();

ngisLog_t *ngisLogCreatef(const char *, ...);
ngisLog_t *ngisLogCreate(const char *);
int ngisLogDestroy(ngisLog_t *);

int ngisLogPrintf(
    ngisLog_t *, ngisLogLevel_t, const char *, const char *, ...)
    NGIS_ATTRIBUTE_PRINTF(4, 5);

int ngisLogDumpFile(
    ngisLog_t *, ngisLogLevel_t, const char *, const char *);
int ngisLogVprintf(
    ngisLog_t *, ngisLogLevel_t, const char *, const char *, va_list);

int ngisAbortPrint(ngisLog_t *, const char *, const char *, ...)
    NGIS_ATTRIBUTE_PRINTF(3, 4);
int ngisErrorPrint(ngisLog_t *, const char *, const char *, ...)
    NGIS_ATTRIBUTE_PRINTF(3, 4);
int ngisDebugPrint(ngisLog_t *, const char *, const char *, ...)
    NGIS_ATTRIBUTE_PRINTF(3, 4);
int ngisWarningPrint(ngisLog_t *, const char *, const char *, ...)
    NGIS_ATTRIBUTE_PRINTF(3, 4);

#endif /* _NGIS_LOG_H_ */
