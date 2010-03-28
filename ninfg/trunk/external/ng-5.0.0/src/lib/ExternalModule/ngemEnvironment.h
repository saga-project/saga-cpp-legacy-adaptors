/*
 * $RCSfile: ngemEnvironment.h,v $ $Revision: 1.3 $ $Date: 2008/02/28 11:59:24 $
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
#ifndef _NGEM_ENVIRONMENT_H_
#define _NGEM_ENVIRONMENT_H_

/**
 * This file include the system header files,
 * which was checked by configure script.
 * The configure result depend on each operating environment.
 */

#include "ngEnvironment.h"

#ifndef __cplusplus
#if HAVE_STDBOOL_H
#include <stdbool.h>
#else
#if ! HAVE__BOOL
typedef unsigned char bool;
#else
#define bool _Bool
#endif
#define false 0
#define true 1
#define __bool_true_false_are_defined 1
#endif
#endif

#endif /* _NGEM_ENVIRONMENT_H_ */

