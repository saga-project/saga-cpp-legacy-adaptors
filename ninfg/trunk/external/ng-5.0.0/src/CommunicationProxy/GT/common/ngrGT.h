/* 
 * $RCSfile: ngrGT.h,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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

#ifndef NGR_GT_H_
#define NGR_GT_H_

#include "ngEnvironment.h"

#define NGR_PACK_HEADER_NELEMENTS (2)
#define NGR_PACK_HEADER_SIZE      (NGR_PACK_HEADER_NELEMENTS * sizeof(uint32_t))

#define NGR_PACK_TYPE_REPLY  0x1U
#define NGR_PACK_TYPE_NOTIFY 0x2U

#define NGCP_ENV_ONLY_PRIVATE "NG_COMMUNICATION_PROXY_ONLY_PRIVATE"

#endif /*NGR_GT_H_*/
