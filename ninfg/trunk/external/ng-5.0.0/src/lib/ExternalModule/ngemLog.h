/*
 * $RCSfile: ngemLog.h,v $ $Revision: 1.3 $ $Date: 2008/02/25 05:21:46 $
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
#ifndef _NGEM_LOG_H_
#define _NGEM_LOG_H_

#include "ngemEnvironment.h"
#include "ngemType.h"

#include "ngUtility.h"

ngemResult_t ngemLogInitialize(char *, char*);
ngLog_t *ngemLogGetDefault(void);
ngemResult_t ngemLogFinalize(void);

#endif /* _NGEM_LOG_H_ */
