/*
 * $RCSfile: grpc_error.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:06 $
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
 * This file define Grid RPC API.
 *
 */

#include "grpc.h"

NGI_RCSID_EMBED("$RCSfile: grpc_error.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:06 $")

/**
 * Error code to Pure error code
 */
static int grpc_l_pure_error[] = {
    /* NG_ERROR_NO_ERROR:         No error */
    GRPC_NO_ERROR,
    /* NG_ERROR_GLOBUS:           Error occurred in API of Globus Toolkit */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_MEMORY:           Memory error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_FILE:             File access error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_PROTOCOL:         Protocol error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_COMMUNICATION:    Communication error */
    GRPC_COMMUNICATION_FAILED,
    /* NG_ERROR_INVALID_ARGUMENT: Invalid argument */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_INVALID_STATE:    Invalid state */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_SOCKET:           Socket error */
    GRPC_COMMUNICATION_FAILED,
    /* NG_ERROR_ALREADY:          Data is already registered */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_NOT_EXIST:        Data is not exist */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_EXIST:            Data is exist */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_JOB_DEAD:         Job is dead */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_EXCEED_LIMIT:     Exceed limit */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_JOB_INVOKE:       Invoke job */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_JOB_CANCEL:       Cancel job */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_THREAD:           Thread error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_SYSCALL:          System call and standard library error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_TIMEOUT:          Timeout */
    GRPC_TIMEOUT_NP,
    /* NG_ERROR_NOT_LOCKED:       Not locked */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_UNLOCK:           Unlock */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_INITIALIZE:       Initialize */
    GRPC_NOT_INITIALIZED,
    /* NG_ERROR_FINALIZE:         Finalize */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_OVERFLOW:         Overflow */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_UNDERFLOW:        Underflow */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_SYNTAX:           Syntax error */
    GRPC_OTHER_ERROR_CODE,
    /* NG_ERROR_DISCONNECT:       Connection was closed */
    GRPC_COMMUNICATION_FAILED,
    /* NG_ERROR_CANCELED:         Session was canceled */
    GRPC_CANCELED_NP,
    /* NG_ERROR_CONFIGFILE_NOT_FOUND: Configuration file not found */
    GRPC_CONFIGFILE_NOT_FOUND,
    /* NG_ERROR_CONFIGFILE_SYNTAX:    Syntax error of Configuration file */
    GRPC_CONFIGFILE_ERROR,
    /* NG_ERROR_COMPRESSION:          Compression error */
    GRPC_OTHER_ERROR_CODE
#if 0 /* 2004/03/03 mitani: It does not define as pure Ninf-G. */
    GRPC_RPC_REFUSED,
#endif
};

/**
 * Get error code from ng error code.
 */
grpc_error_t
grpc_i_get_error_from_ng_error(int errorCode)
{
#define GRPC_L_NERRORS \
        (sizeof (grpc_l_pure_error) / sizeof (grpc_l_pure_error[0]))

    if ((errorCode < 0) || (errorCode >= GRPC_L_NERRORS))
         return GRPC_OTHER_ERROR_CODE;

    return grpc_l_pure_error[errorCode];
#undef GRPC_L_NERRORS
}
