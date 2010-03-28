/*
 * $RCSfile: ngcpUtility.c,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $
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

#include "ngcpUtility.h"
#include "ngcpXIO.h"

NGI_RCSID_EMBED("$RCSfile: ngcpUtility.c,v $ $Revision: 1.1 $ $Date: 2008/02/25 05:21:46 $")

const ngcpCommonLock_t ngcpCommonLockNull = {
    {
        NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED,
        {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
        {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
        {NGI_SYNCHRONIZED_OBJECT_STATUS_NOT_INITIALIZED},
        0, /* ngrl_lockLevel    */
        0, /* ngrl_nLockWaiter  */
        0, /* ngrl_nSigWaiter   */
        0  /* ngrl_signalNumber */
    },
    false,
};

ngemResult_t
ngcpCommonLockInitialize(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    lock->ngcl_finalizing = false;

    result = ngiRlockInitialize(&lock->ngcl_lock, NULL, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockFinalize(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    lock->ngcl_finalizing = false;

    result = ngiRlockFinalize(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockLock(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockUnlock(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockUnlock(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockWait(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockWait(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockBroadcast(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockBroadcast(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

ngemResult_t
ngcpCommonLockSetFinalize(
    ngcpCommonLock_t *lock)
{
    ngLog_t *log;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    bool locked = false;
    NGEM_FNAME(ngcpCommonLockSetFinalize);

    NGEM_ASSERT(lock != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }
    lock->ngcl_finalizing = true;

    result = ngiRlockBroadcast(&lock->ngcl_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
        ret = NGEM_FAILED;
    }

    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&lock->ngcl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCP_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
    }
    
    return ret;
}

void
ngcpKillAndWait(pid_t pid)
{
    static const int signals[]  = {0, SIGTERM, SIGKILL, SIGKILL};
    int i;
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngcpKillAndWait);

    log = ngemLogGetDefault();

    NGEM_ASSERT(pid > 0);

    for (i = 0;i < 4;++i) {
        waitpid(pid, NULL,  WNOHANG);
        result = kill(pid, signals[i]);
        if (result < 0) {
            if (errno == ESRCH) {
                break;
            }
            ngLogError(log, NGCP_LOGCAT_GT, fName,
            "kill: %s\n", strerror(errno));
            break;
        }
        sleep(10);
    }

    if (i == 4) {
        while (1) {
            waitpid(pid, NULL,  WNOHANG);
            result = kill(pid, SIGKILL);
            if (result < 0) {
                if (errno == ESRCH) {
                    break;
                }
                ngLogError(log, NGCP_LOGCAT_GT, fName,
                "kill: %s\n", strerror(errno));
            }
            sleep(10);
        }
    }

    return;
}


