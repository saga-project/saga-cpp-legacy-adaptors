/*
 * $RCSfile: grpcError.h,v $ $Revision: 1.17 $ $Date: 2005/10/05 11:50:27 $
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
#ifndef _GRPCERROR_H_
#define _GRPCERROR_H_

/**
 * This file define Error code and Error Message for Grid RPC API internal.
 *
 */

/* Error code */
#define GRPC_NO_ERROR                   0 /* Success */
#define GRPC_NOT_INITIALIZED            1 /* GRPC client not initialized yet */
#define GRPC_ALREADY_INITIALIZED        2 /* The function is called more than once */
#define GRPC_CONFIGFILE_NOT_FOUND       3 /* Specified configuration file not found */
/* An error occurred parsing or processing the configuration file */
#define GRPC_CONFIGFILE_ERROR           4
#define GRPC_SERVER_NOT_FOUND           5 /* GRPC client cannot find any server */
/* GRPC client cannot find the function on the default server */
#define GRPC_FUNCTION_NOT_FOUND         6
#define GRPC_INVALID_FUNCTION_HANDLE    7 /* Function handle is not valid */
#define GRPC_INVALID_SESSION_ID         8 /* Session ID is not valid */
/* RPC invocation refused by the server, possibly because of a security issue */
#define GRPC_RPC_REFUSED                9
#define GRPC_COMMUNICATION_FAILED      10 /* Communication with the server failed somehow */
#define GRPC_SESSION_FAILED            11 /* The specified session failed */
#define GRPC_NOT_COMPLETED             12 /* Call has not completed */
#define GRPC_NONE_COMPLETED            13 /* No calls have completed */
#define GRPC_INVALID_OBJECT_HANDLE_NP  14 /* Object handle is not valid */
/* GRPC client cannot find the class on the default server */
#define GRPC_CLASS_NOT_FOUND_NP        15
#define GRPC_TIMEOUT_NP                16 /* Timeout */
#define GRPC_CANCELED_NP               17 /* Session Cancel */

#define GRPC_OTHER_ERROR_CODE          18 /* Internal error detected */
/* Error description string requested for an unknown error code*/
#define GRPC_UNKNOWN_ERROR_CODE        19
#define GRPC_LAST_ERROR_CODE           20 /* Highest numerical error code */

#endif /* _GRPCERROR_H_ */
