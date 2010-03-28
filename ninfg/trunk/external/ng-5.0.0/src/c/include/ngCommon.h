/* 
 * $RCSfile: ngCommon.h,v $ $Revision: 1.10 $ $Date: 2008/03/28 08:50:58 $
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
#ifndef _NGCOMMON_H_
#define _NGCOMMON_H_

/**
 * This file define the Data Structures and Constant Values for stub
 */

#include "ngEnvironment.h"
#include "queue.h"

/**
 * Define the error code.
 */
#define NG_ERROR_NO_ERROR		0	/* No error */
#define NG_ERROR_GLOBUS			1	/* Error occurred in API of Globus Toolkit */
#define NG_ERROR_MEMORY			2	/* Memory error */
#define NG_ERROR_FILE			3	/* File access error */
#define NG_ERROR_PROTOCOL		4	/* Protocol error */
#define NG_ERROR_COMMUNICATION		5	/* Communication error */
#define NG_ERROR_INVALID_ARGUMENT	6	/* Invalid argument */
#define NG_ERROR_INVALID_STATE		7	/* Invalid state */
#define NG_ERROR_SOCKET			8	/* Socket error */
#define NG_ERROR_ALREADY		9	/* Data is already registered */
#define NG_ERROR_NOT_EXIST		10	/* Data is not exist */
#define NG_ERROR_EXIST			11	/* Data is exist */
#define NG_ERROR_JOB_DEAD		12	/* Job is dead */
#define NG_ERROR_EXCEED_LIMIT		13	/* Exceed limit */
#define NG_ERROR_JOB_INVOKE		14	/* Invoke job */
#define NG_ERROR_JOB_CANCEL		15	/* Cancel job */
#define NG_ERROR_THREAD			16	/* Thread error */
#define NG_ERROR_SYSCALL		17	/* System call and standard library error */
#define NG_ERROR_TIMEOUT		18	/* Timeout */
#define NG_ERROR_NOT_LOCKED		19	/* Not locked */
#define NG_ERROR_UNLOCK			20	/* Unlock */
#define	NG_ERROR_INITIALIZE		21	/* Initialize */
#define NG_ERROR_FINALIZE		22	/* Finalize */
#define NG_ERROR_OVERFLOW		23	/* Overflow */
#define NG_ERROR_UNDERFLOW		24	/* Underflow */
#define NG_ERROR_SYNTAX			25	/* Syntax error */
#define NG_ERROR_DISCONNECT		26	/* Connection was closed */
#define NG_ERROR_CANCELED		27	/* Session was canceled */
#define NG_ERROR_CONFIGFILE_NOT_FOUND   28      /* Configuration file not found */
#define NG_ERROR_CONFIGFILE_SYNTAX      29      /* Syntax error of Configuration file */
#define NG_ERROR_COMPRESSION            30      /* Compression error */
#define NG_ERROR_PIPE                   31      /* Pipe error */

/**
 * Set the error code.
 */
#define NGI_SET_ERROR(error, code) \
    do { \
	if ((error) != NULL) \
	    *(error) = (code); \
    } while (0)

#define NGI_NG_DIR_ENV_NAME "NG_DIR"

/* Maximum decimal digits of int */
#define NGI_INT_MAX_DECIMAL_DIGITS 14

/* Temporary file */
#define NGI_ENVIRONMENT_TMPDIR "TMPDIR"           /* Environment variable */
#define NGI_TMP_DIR            "/tmp"             /* Directory */
#define NGI_TMP_FILE           "ngtmpfile.XXXXXX" /* File */


/* Maximum number of bytes of File Name */
#define NGI_FILE_NAME_MAX		(1024 * 4)

#define NGI_HOSTNAME_ENVIRONMENT_NAME	"NG_HOSTNAME"
#define NGI_HOST_NAME_MAX 1024
#define NGI_DIR_NAME_MAX (4 * 1024)

#endif /* _NGCOMMON_H_ */

