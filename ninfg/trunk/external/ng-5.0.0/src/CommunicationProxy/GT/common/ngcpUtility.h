/* 
 * $RCSfile: ngcpUtility.h,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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

#ifndef NGCP_UTILITY_H_
#define NGCP_UTILITY_H_

#include <ngUtility.h>
#include <ngemType.h>

typedef struct ngcpCommonLock_s {
    ngiRlock_t ngcl_lock;
    bool       ngcl_finalizing;
} ngcpCommonLock_t;

#define NGCP_COMMON_LOCK_NULL ngcpCommonLockNull;
#define NGCP_COMMON_LOCK_IS_NULL(lock) \
        ((NGI_RLOCK_IS_NULL(&lock->ngcl_lock))
#define NGCP_COMMON_LOCK_IS_VALID(lock) (!NGCP_COMMON_LOCK_IS_NULL(lock))

extern const ngcpCommonLock_t ngcpCommonLockNull;

ngemResult_t ngcpCommonLockInitialize(ngcpCommonLock_t *);
ngemResult_t ngcpCommonLockFinalize(ngcpCommonLock_t *);

ngemResult_t ngcpCommonLockLock(ngcpCommonLock_t *);
ngemResult_t ngcpCommonLockUnlock(ngcpCommonLock_t *);
ngemResult_t ngcpCommonLockWait(ngcpCommonLock_t *);
ngemResult_t ngcpCommonLockBroadcast(ngcpCommonLock_t *);
ngemResult_t ngcpCommonLockSetFinalize(ngcpCommonLock_t *);

#define NGCP_COMMON_LOCK_FINALIZING(lock) ((lock)->ngcl_finalizing)

void ngcpKillAndWait(pid_t pid);

#endif/*NGCP_UTILITY_H_*/
