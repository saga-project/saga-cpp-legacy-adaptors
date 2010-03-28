/*
 * $RCSfile: ngEx.h,v $ $Revision: 1.11 $ $Date: 2007/11/28 10:50:11 $
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
#ifndef _NGEX_H_
#define _NGEX_H_

/**
 * This file define the Data Structures and Constant Values for user of
 * Pure Ninf-G for server side 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <globus_common.h>
#include <globus_module.h>
#include <globus_gass_copy.h>
#include <globus_io.h>
#include <globus_libc.h>
#include "ngFunctionInformation.h"
#include "ngCommon.h"
#include "grpc_executable.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Prototype declaration of APIs for stub.
 */
int ngexStubAnalyzeArgumentWithExit(int, char **,
    ngRemoteClassInformation_t *, int *);
int ngexStubInitialize(int, char **, ngRemoteClassInformation_t *, int *);
int ngexStubInitializeMPI(int, char **, ngRemoteClassInformation_t *, int, int *);
int ngexStubGetRequest(int *, int *);
int ngexStubGetArgument(int, void *, int *);
int ngexStubCalculationStart(int *);
int ngexStubCalculationEnd(int *);
int ngexStubFinalize(int *);
int ngexIsCanceled(int *);
int ngexStubCallback(int, int *, ...);
int ngexGetError();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "ngInternal.h"
#include "ngExecutableInternal.h"
#endif /* _NGEX_H_ */
