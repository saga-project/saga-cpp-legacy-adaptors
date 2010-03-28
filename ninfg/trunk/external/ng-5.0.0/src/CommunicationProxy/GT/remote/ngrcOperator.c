/*
 * $RCSfile: ngrcOperator.c,v $ $Revision: 1.4 $ $Date: 2008/02/07 06:22:02 $
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

#include "ngrcGT.h"

NGI_RCSID_EMBED("$RCSfile: ngrcOperator.c,v $ $Revision: 1.4 $ $Date: 2008/02/07 06:22:02 $")

/**
 * Argument for callback
 */
typedef struct ngrclOperatorCallbackArg_s {
    ngrcOperator_t     *ngoca_operator;
    bool                 ngoca_done;
    globus_size_t        ngoca_bytes;
    globus_result_t      ngoca_result;
} ngrclOperatorCallbackArg_t;

static void ngrclGlobusCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t, globus_xio_data_descriptor_t, void *);

static void ngrclOperatorCallbackArgInitialize(ngrclOperatorCallbackArg_t *, ngrcOperator_t *);
static void ngrclOperatorCallbackArgFinalize(ngrclOperatorCallbackArg_t *);

/**
 * Operator: Initialize
 */
ngemResult_t
ngrcOperatorInitialize(
    ngrcOperator_t *op,
    globus_xio_handle_t handle)
{
    globus_result_t gResult;
    ngLog_t *log;
    bool mutexInitialized = false;
    NGEM_FNAME(ngrcOperatorInitialize);

    log = ngemLogGetDefault();

    NGEM_ASSERT(op != NULL);
    NGEM_ASSERT(handle != NULL);

    op->ngo_handle    = handle;
    op->ngo_canceled  = false;

    gResult = globus_mutex_init(&op->ngo_mutex, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_init", gResult);
        goto error;
    }
    mutexInitialized = true;

    globus_cond_init(&op->ngo_cond, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_cond_init", gResult);
        goto error;
    }
    return NGEM_SUCCESS;

error:
    if (mutexInitialized) {
        gResult = globus_mutex_destroy(&op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_mutex_destroy", gResult);
        }
    }
    return NGEM_FAILED;
}

/**
 * Operator: Finalize
 */
ngemResult_t
ngrcOperatorFinalize(
    ngrcOperator_t*op)
{
    globus_result_t gResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngrcOperatorFinalize);

    log = ngemLogGetDefault();

    NGEM_ASSERT(op != NULL);

    op->ngo_handle   = NULL;
    op->ngo_canceled = false;

    gResult = globus_mutex_destroy(&op->ngo_mutex);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_destroy", gResult);
        ret = NGEM_FAILED;
    }
    gResult = globus_cond_destroy(&op->ngo_cond);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_cond_destroy", gResult);
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Operator: read
 */
ngemResult_t
ngrcOperatorRead(
    ngrcOperator_t *op,
    void *buf,
    size_t size,
    size_t *nRead,
    bool *canceled)
{
    ngemResult_t ret = NGEM_FAILED;
    globus_result_t gResult;
    ngLog_t *log = NULL;
    bool locked = false;
    ngrclOperatorCallbackArg_t arg;
    NGEM_FNAME(ngrcOperatorRead);

    log = ngemLogGetDefault();

    NGEM_ASSERT(op != NULL);
    NGEM_ASSERT(size >= 0);
    NGEM_ASSERT(nRead != NULL);
    NGEM_ASSERT(canceled != NULL);

    *canceled = false;

    gResult = globus_mutex_lock(&op->ngo_mutex);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_lock", gResult);
        goto finalize;
    } 
    locked = true;

    ngrclOperatorCallbackArgInitialize(&arg, op);

    if (op->ngo_canceled) {
        *canceled = true;
        ret = NGEM_SUCCESS;
        goto finalize;
    }

    gResult = globus_xio_register_read(
        op->ngo_handle, buf, size, 1, NULL,
        ngrclGlobusCallback, &arg);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        goto finalize;
    }

    while (arg.ngoca_done == false) {
        gResult = globus_cond_wait(&op->ngo_cond, &op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_cond_wait", gResult);
        }
    }

    if (arg.ngoca_result != GLOBUS_SUCCESS) {
        if (globus_xio_error_is_canceled(arg.ngoca_result) == GLOBUS_TRUE) {
            *canceled = true;
        } else if (globus_xio_error_is_eof(arg.ngoca_result) == GLOBUS_TRUE) {
            *nRead = 0;
        } else {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "Callback function for reading", arg.ngoca_result);
            goto finalize;
        }
    } else {
        *nRead = arg.ngoca_bytes;
        NGEM_ASSERT(*nRead > 0);
    }

    ret = NGEM_SUCCESS;
finalize:
    ngrclOperatorCallbackArgFinalize(&arg);

    if (locked) {
        gResult = globus_mutex_unlock(&op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_mutex_unlock", gResult);
            ret = NGEM_FAILED;
        }
        locked = false;
    }

    return ret;
}

/**
 * Operator: write
 */
ngemResult_t
ngrcOperatorWrite(
    ngrcOperator_t *op,
    void *buf,
    size_t size,
    size_t *nWrite,
    bool *canceled)
{
    globus_result_t gResult;
    ngemResult_t ret = NGEM_FAILED;
    bool locked = false;
    ngLog_t *log = NULL;
    ngrclOperatorCallbackArg_t arg;
    NGEM_FNAME(ngrcOperatorWrite);

    log = ngemLogGetDefault();

    NGEM_ASSERT(op != NULL);
    NGEM_ASSERT(size >= 0);
    NGEM_ASSERT(nWrite != NULL);
    NGEM_ASSERT(canceled != NULL);

    *canceled = false;

    gResult = globus_mutex_lock(&op->ngo_mutex);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_lock", gResult);
        goto finalize;
    } 
    locked = true;

    ngrclOperatorCallbackArgInitialize(&arg, op);

    if (op->ngo_canceled) {
        *canceled = true;
    } else {

        ngLogDebug(log, NGRC_LOGCAT_GT, fName,
            "user_data = %p.\n", &arg);

        gResult = globus_xio_register_write(
            op->ngo_handle, buf, size, size, NULL,
            ngrclGlobusCallback, &arg);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_xio_register_write", gResult);
            goto finalize;
        }

        while (arg.ngoca_done == false) {
            gResult = globus_cond_wait(&op->ngo_cond, &op->ngo_mutex);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                    "globus_cond_wait", gResult);
            }
        }

        if (arg.ngoca_result != GLOBUS_SUCCESS) {
            if (globus_xio_error_is_canceled(arg.ngoca_result) == GLOBUS_FALSE) {
                ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                    "Callback function for writing", arg.ngoca_result);
                goto finalize;
            }
            *canceled = true;
        } else {
            ngLogDebug(log, NGRC_LOGCAT_GT, fName,
                "Writes %lu bytes\n", (unsigned long)arg.ngoca_bytes);
            *nWrite = arg.ngoca_bytes;
        }
    }

    ret = NGEM_SUCCESS;

finalize:
    ngrclOperatorCallbackArgFinalize(&arg);

    if (locked) {
        gResult = globus_mutex_unlock(&op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_mutex_unlock", gResult);
            ret = NGEM_FAILED;
        }
        locked = false;
    }

    return ret;
}

/**
 * Operator: cancel.
 */
ngemResult_t
ngrcOperatorCancel(
    ngrcOperator_t *op)
{
    ngLog_t *log;
    globus_result_t gResult;
    ngemResult_t ret = NGEM_SUCCESS;
    bool locked = false;
    NGEM_FNAME(ngrcOperatorCancel);

    log = ngemLogGetDefault();

    gResult = globus_mutex_lock(&op->ngo_mutex);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_lock", gResult);
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }

    op->ngo_canceled = true;
    gResult = globus_cond_broadcast(&op->ngo_cond);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_cond_broadcast", gResult);
        ret = NGEM_FAILED;
    }
    if (locked) {
        gResult = globus_mutex_unlock(&op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_mutex_unlock", gResult);
            ret = NGEM_FAILED;
        }
        locked = false;
    }

    gResult = globus_xio_handle_cancel_operations(
        op->ngo_handle,
        GLOBUS_XIO_CANCEL_OPEN | GLOBUS_XIO_CANCEL_READ | GLOBUS_XIO_CANCEL_WRITE);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_xio_handle_cancel_operations", gResult);
        ret = NGEM_FAILED;
    }

    return ret;
}

/*
 * Callback function for reading and writing
 */
static void
ngrclGlobusCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                len,
    globus_size_t                nbytes,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userArg)
{
    ngrclOperatorCallbackArg_t *arg = userArg;
    ngrcOperator_t *op = arg->ngoca_operator;
    globus_result_t gResult;
    bool locked = false;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngrclGlobusCallback);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Called\n");

    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "user_data = %p.\n", arg);

    gResult = globus_mutex_lock(&op->ngo_mutex);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_mutex_lock", gResult);
    } else {
        locked = true;
    }

    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "nBytes = %lu.\n", (unsigned long)nbytes);

    arg->ngoca_done   = true;
    arg->ngoca_result = cResult;
    arg->ngoca_bytes  = nbytes;

    gResult = globus_cond_broadcast(&op->ngo_cond);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_cond_broadcast", gResult);
    }
    if (locked) {
        gResult = globus_mutex_unlock(&op->ngo_mutex);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_mutex_unlock", gResult);
        }
        locked = false;
    }

    return;
}

/**
 * 
 */
static void
ngrclOperatorCallbackArgInitialize(
    ngrclOperatorCallbackArg_t *arg,
    ngrcOperator_t *op)
{
    NGEM_FNAME_TAG(ngrclOperatorCallbackArgInitialize);

    arg->ngoca_operator = op;
    arg->ngoca_done     = false;
    arg->ngoca_bytes    = 0;
    arg->ngoca_result   = GLOBUS_SUCCESS;;

    return;
}

static void
ngrclOperatorCallbackArgFinalize(
    ngrclOperatorCallbackArg_t *arg)
{
    NGEM_FNAME_TAG(ngrclOperatorCallbackArgFinalize);

    arg->ngoca_operator = NULL;
    arg->ngoca_done     = false;
    arg->ngoca_bytes    = 0;
    arg->ngoca_result   = GLOBUS_SUCCESS;;

    return;
}
