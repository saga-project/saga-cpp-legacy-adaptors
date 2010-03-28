/*
 * $RCSfile: grpc_executable.h,v $ $Revision: 1.4 $ $Date: 2007/07/10 05:19:42 $
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
#ifndef _GRPC_EXECUTABLE_H_
#define _GRPC_EXECUTABLE_H_

#include "ngEx.h"
#include "grpc.h"

/**
 * Prototype declaration of Executable side GRPC APIs
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int grpc_is_canceled_np(grpc_error_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GRPC_EXECUTABLE_H_ */
