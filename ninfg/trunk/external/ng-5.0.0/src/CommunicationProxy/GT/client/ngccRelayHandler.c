/*
 * $RCSfile: ngccRelayHandler.c,v $ $Revision: 1.4 $ $Date: 2008/03/27 10:02:52 $
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

#include "ngcpRelayHandler.h"
#include "ngcpOptions.h"
#include "ngccGT.h"

NGI_RCSID_EMBED("$RCSfile: ngccRelayHandler.c,v $ $Revision: 1.4 $ $Date: 2008/03/27 10:02:52 $")

/* Types */
typedef struct ngcclCommunicationReply_s ngcclCommunicationReply_t;

/* Lists */
NGEM_DECLARE_LIST_OF(ngccRelayHandler_t);
NGEM_DECLARE_LIST_OF(ngcclCommunicationReply_t);

/**
 * Client Relay Handler manager
 */
struct ngccRelayHandlerManager_s {
    NGEM_LIST_OF(ngccRelayHandler_t)         ngrhm_handlers;
    ngccRelayHandlerConnectRequestCallback_t ngrhm_callback;
    void                                    *ngrhm_userData;
};

/**
 * Client Relay Handler
 */
struct ngccRelayHandler_s {
    ngcpRelayHandler_t                     *ngrh_relayHandler;
    ngccRelayHandlerManager_t              *ngrh_manager;
    int                                     ngrh_ref;
    int                                     ngrh_requestIDbase;
    ngcpCommonLock_t                        ngrh_lock;
    NGEM_LIST_OF(ngcclCommunicationReply_t) ngrh_commReplys;
    ngcpOptions_t    *ngrh_options;/* Used as ID */
};

/**
 * Information of COMMUNICATION_REPLY notify
 */
struct ngcclCommunicationReply_s {
    int           ngcp_id;
    bool          ngcp_finalize;
    bool          ngcp_received;
    ngemResult_t  ngcp_result;
    char         *ngcp_contactString;
    char         *ngcp_message;
};

/* Prototypes */
static ngccRelayHandler_t *ngcclRelayHandlerCreate(ngccRelayHandlerManager_t *, ngcpOptions_t *);
static void ngcclRelayHandlerDestroy(ngccRelayHandler_t *);

static ngemResult_t ngcclRelayHandlerSendQueryFeatures(ngccRelayHandler_t *);
static ngemResult_t ngcclRelayHandlerSendInitialize(ngccRelayHandler_t *, int);
static ngemResult_t ngcclRelayHandlerSendPrepareCommunication(ngccRelayHandler_t *, int, ngccPrepareCommunicationOptions_t *);
static ngemResult_t ngcclRelayHandlerConnect(ngccRelayHandler_t *, int);
static ngemResult_t ngcclRelayHandlerRef(ngccRelayHandler_t *relayHandler);
static void ngcclRelayHandlerOnCommunicationReply(ngcpRelayHandler_t *, ngcpNotify_t *, void *);
static void ngcclRelayHandlerOnConnectRequestNp(ngcpRelayHandler_t *, ngcpNotify_t *, void *);

/**
 * Client Relay Handler Manager: Create
 */
ngccRelayHandlerManager_t *
ngccRelayHandlerManagerCreate(
    ngccRelayHandlerConnectRequestCallback_t callback,
    void *userData)
{
    ngccRelayHandlerManager_t *manager = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngccRelayHandlerManagerCreate);

    log = ngemLogGetDefault();

    NGEM_ASSERT(callback != NULL);

    manager = NGI_ALLOCATE(ngccRelayHandlerManager_t, log, NULL);
    if (manager == NULL) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't allocate storage for the Relay Handler manager.\n");
        goto error;
    }

    NGEM_LIST_SET_INVALID_VALUE(&manager->ngrhm_handlers);

    NGEM_LIST_INITIALIZE(
        ngccRelayHandler_t, &manager->ngrhm_handlers);
    manager->ngrhm_callback = callback;
    manager->ngrhm_userData = userData;

    return manager;
error:
    ngccRelayHandlerManagerDestroy(manager);

    return NULL;
}

/**
 * Client Relay Handler Manager: Destroy
 */
void
ngccRelayHandlerManagerDestroy(
    ngccRelayHandlerManager_t *manager)
{
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_LIST_ITERATOR_OF(ngccRelayHandler_t) it;
    ngccRelayHandler_t *val;
    NGEM_FNAME(ngccRelayHandlerManagerDestroy);

    log = ngemLogGetDefault();

    if (manager == NULL) {
        return;
    }

    NGEM_LIST_FOREACH(ngccRelayHandler_t, &manager->ngrhm_handlers, it, val) {
        ngcclRelayHandlerDestroy(val);
    }
    NGEM_LIST_FINALIZE(ngccRelayHandler_t, &manager->ngrhm_handlers);

    nResult = NGI_DEALLOCATE(ngccRelayHandlerManager_t, manager, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCP_LOGCAT_GT, fName,
            "Can't deallocate storage for the Relay Handler manager.\n");
    }
    return;
}

/**
 * Client Relay Handler Manager: Get Client Relay Handler having "contactString".
 */
ngccRelayHandler_t *
ngccRelayHandlerManagerGet(
    ngccRelayHandlerManager_t *manager,
    ngcpOptions_t *opts)
{
    NGEM_LIST_ITERATOR_OF(ngccRelayHandler_t) it;
    ngccRelayHandler_t *relayHandler = NULL;
    ngccRelayHandler_t *val= NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    bool created = false;
    NGEM_FNAME(ngccRelayHandlerManagerGet);

    log = ngemLogGetDefault();

    NGEM_ASSERT(manager != NULL);
    NGEM_ASSERT(opts != NULL);

    NGEM_LIST_FOREACH(ngccRelayHandler_t, &manager->ngrhm_handlers, it, val) {
        if (ngcpOptionsEqualForRelayInvoking(val->ngrh_options, opts)) {
            relayHandler = val;
            break;
        }
    }

    /* If not found, Create */
    if (relayHandler == NULL) {
        relayHandler = ngcclRelayHandlerCreate(manager, opts);
        if (relayHandler == NULL) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't create a new Client Relay handler.\n");
            goto error;
        }
        created = true;
        /* Register to manager */
        nResult = NGEM_LIST_INSERT_TAIL(ngccRelayHandler_t,
            &manager->ngrhm_handlers, relayHandler);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't register new Client Register Handler to manager.\n");
            goto error;
        }
    }

    nResult = ngcclRelayHandlerRef(relayHandler);
    if (nResult == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't register new Client Register Handler to manager.\n");
        goto error;
    }

    return relayHandler;
error:
    if ((relayHandler != NULL) && (created)) {

        ngcclRelayHandlerDestroy(relayHandler);
    }
    return NULL;
}

/**
 * Client Relay Handler: Prepares communication.
 */
ngemResult_t
ngccRelayHandlerPrepareCommunication(
    ngccRelayHandler_t *relayHandler,
    ngccPrepareCommunicationOptions_t *pCommOpts,
    ngccPrepareCommunicationResult_t *res)
{
    ngLog_t *log;
    ngemResult_t nResult;
    ngcclCommunicationReply_t commReply;
    ngemResult_t ret = NGEM_FAILED;
    bool locked = false;
    bool commReplyInserted = false;
    int requestID;
    char *message = NULL;
    NGEM_FNAME(ngccRelayHandlerPrepareCommunication);

    NGEM_ASSERT(relayHandler != NULL);
    NGEM_ASSERT(res != NULL);

    log = ngemLogGetDefault();

    /* Lock */
    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the the Client Relay Handler.\n");
        goto finalize;
    }
    locked = true;

    if (NGCP_COMMON_LOCK_FINALIZING(&relayHandler->ngrh_lock)) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Relay handler is finalized/\n");
        goto finalize;
    }

    if (relayHandler->ngrh_relayHandler == NULL) {
        nResult = ngcclRelayHandlerConnect(relayHandler, 0);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't connect to the relay.\n");
            goto finalize;
        }

        if (NGCP_COMMON_LOCK_FINALIZING(&relayHandler->ngrh_lock)) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Relay handler is finalized/\n");
            goto finalize;
        }
    }

    res->ngpcr_result        = NGEM_FAILED;
    res->ngpcr_contactString = NULL;
    res->ngpcr_message       = NULL;
    res->ngpcr_canceled      = false;

    requestID = relayHandler->ngrh_requestIDbase++;

    commReply.ngcp_id            = requestID;
    commReply.ngcp_received      = false;
    commReply.ngcp_finalize      = false;
    commReply.ngcp_result        = NGEM_FAILED;
    commReply.ngcp_contactString = NULL;
    commReply.ngcp_message       = NULL;

    nResult = NGEM_LIST_INSERT_TAIL(ngcclCommunicationReply_t,
        &relayHandler->ngrh_commReplys, &commReply);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't insert information for waiting"
            " COMMUNICATION_REPLY notify to list.\n");
        goto finalize;
    }
    commReplyInserted = true;

    nResult = ngcclRelayHandlerSendPrepareCommunication(relayHandler, requestID, pCommOpts);
    if (nResult == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send PREPARE_COMMUNICATION request.\n");
        goto finalize;
    }

    while ((!commReply.ngcp_received) &&
           (!NGCP_COMMON_LOCK_FINALIZING(&relayHandler->ngrh_lock))) {
        nResult = ngcpCommonLockWait(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't wait the the Client Relay Handler.\n");
            goto finalize;
        }
    }

    if (NGCP_COMMON_LOCK_FINALIZING(&relayHandler->ngrh_lock)) {
        res->ngpcr_canceled = true;
        ngLogInfo(log, NGCC_LOGCAT_GT, fName,
            "Waiting the notify is canceled.\n");
    } else {
        if ((commReply.ngcp_result == NGEM_SUCCESS) && (commReply.ngcp_contactString == NULL)) {
            message = "COMMUNICATION_REPLY's result is a success, but it does not have contact string";
            goto syntax_error;
        }

        if ((commReply.ngcp_result == NGEM_SUCCESS) && (commReply.ngcp_message != NULL)) {
            message = "COMMUNICATION_REPLY's result is a success, but it has error message";
            goto syntax_error;
        }

        if ((commReply.ngcp_result != NGEM_SUCCESS) && (commReply.ngcp_contactString != NULL)) {
            message = "COMMUNICATION_REPLY's result is failure, but it has contact string";
            goto syntax_error;
        }

        if ((commReply.ngcp_result != NGEM_SUCCESS) && (commReply.ngcp_message == NULL)) {
            message = "COMMUNICATION_REPLY's result is failure, but it does not have error message";
            goto syntax_error;
        }
        if (message == NULL) {
            res->ngpcr_result = commReply.ngcp_result;
            if (commReply.ngcp_result != NGEM_SUCCESS) {
                res->ngpcr_message = commReply.ngcp_message;
                commReply.ngcp_message = NULL;
            } else {
                res->ngpcr_contactString = commReply.ngcp_contactString;
                commReply.ngcp_contactString = NULL;
            }
        } else {
syntax_error:
            res->ngpcr_result  = NGEM_FAILED;
            res->ngpcr_message = ngiStrdup(message, log, NULL);
            /* Does not check return value */
        }
    }

    ret = NGEM_SUCCESS;
finalize:
    /* Finalize: commReply */

    if (commReplyInserted) {
        nResult = NGEM_LIST_ERASE_BY_ADDRESS(ngcclCommunicationReply_t,
            &relayHandler->ngrh_commReplys, &commReply);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't erase information for waiting"
                " COMMUNICATION_REPLY notify from list.\n");
            ret = NGEM_FAILED;
        }
    }
    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 *  Relay Handler: ref
 */
static
ngemResult_t
ngcclRelayHandlerRef(
    ngccRelayHandler_t *relayHandler)
{
    bool locked = false;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcclRelayHandlerRef);

    log = ngemLogGetDefault();

    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the the Client Relay Handler.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }

    relayHandler->ngrh_ref++;

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 *  Relay Handler: unref
 */
ngemResult_t
ngccRelayHandlerUnref(
    ngccRelayHandler_t *relayHandler)
{
    bool locked = false;
    ngemResult_t ret = NGEM_SUCCESS;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngccRelayHandlerUnref);

    log = ngemLogGetDefault();

    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the the Client Relay Handler.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }

    NGEM_ASSERT(relayHandler->ngrh_ref > 0);
    relayHandler->ngrh_ref--;
    if (relayHandler->ngrh_ref == 0) {
        nResult = ngcpCommonLockBroadcast(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't broadcast the signal.\n");
            ret = NGEM_FAILED;
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 * Relay Handler: Create
 */
static ngccRelayHandler_t *
ngcclRelayHandlerCreate(
    ngccRelayHandlerManager_t *manager,
    ngcpOptions_t *opts)
{
    ngccRelayHandler_t *relayHandler = NULL;
    globus_xio_handle_t handle = NULL;
    ngLog_t *log;
    globus_result_t gResult;
    ngemResult_t nResult;
    NGEM_FNAME(ngcclRelayHandlerCreate);

    log = ngemLogGetDefault();

    /* Allocate */
    relayHandler = NGI_ALLOCATE(ngccRelayHandler_t, log, NULL);
    if (relayHandler == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't allocate storage for the relay handler.\n");
        goto error;
    }

    relayHandler->ngrh_relayHandler  = NULL;
    relayHandler->ngrh_manager       = manager;
    relayHandler->ngrh_ref           = 0;
    relayHandler->ngrh_requestIDbase = 1;
    relayHandler->ngrh_lock          = NGCP_COMMON_LOCK_NULL;
    relayHandler->ngrh_options       = NULL;
    NGEM_LIST_INITIALIZE(ngcclCommunicationReply_t, &relayHandler->ngrh_commReplys);

    relayHandler->ngrh_options = ngcpOptionsCreateCopy(opts);
    if (opts == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't copy options.\n");
        goto error;
    }

    /* Lock */
    nResult = ngcpCommonLockInitialize(&relayHandler->ngrh_lock);
    if (nResult == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't initialize the relay handler.\n");
        goto error;
    }

    return relayHandler;
error:
    ngcclRelayHandlerDestroy(relayHandler);
    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
    }

    return NULL;
}

/**
 * Relay Handler: Destroy
 */
static void
ngcclRelayHandlerDestroy(
    ngccRelayHandler_t *relayHandler)
{
    ngLog_t *log;
    ngemResult_t nResult;
    bool locked = false;
    NGEM_FNAME(ngcclRelayHandlerDestroy);

    log = ngemLogGetDefault();

    if (relayHandler == NULL) {
        /* Do nothing */
        return;
    }

    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the the Client Relay Handler.\n");
    } else {
        locked = true;
    }

    if (relayHandler->ngrh_relayHandler != NULL) {
        nResult = ngcpRelayHandlerSendExit(relayHandler->ngrh_relayHandler);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't send EXIT request.\n");
        }
    }

    nResult = ngcpCommonLockSetFinalize(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set finalize flag.\n");
    }

    while (relayHandler->ngrh_ref > 0) {
        nResult = ngcpCommonLockWait(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't wait the the Client Relay Handler.\n");
        }
    }

    /* unlock */
    if (locked) {
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't lock the the Client Relay Handler.\n");
        }
        locked = false;
    }

    NGEM_ASSERT(NGEM_LIST_IS_EMPTY(ngcclCommunicationReply_t, &relayHandler->ngrh_commReplys));

    ngcpOptionsDestroy(relayHandler->ngrh_options);
    relayHandler->ngrh_options = NULL;

    ngcpRelayHandlerDestroy(relayHandler->ngrh_relayHandler);

    nResult = ngcpCommonLockFinalize(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize the relay handler.\n");
    }

    relayHandler->ngrh_relayHandler  = NULL;
    relayHandler->ngrh_manager       = NULL;
    relayHandler->ngrh_requestIDbase = 0;
    relayHandler->ngrh_lock          = NGCP_COMMON_LOCK_NULL;
    relayHandler->ngrh_options       = NULL;
    NGEM_LIST_FINALIZE(ngcclCommunicationReply_t, &relayHandler->ngrh_commReplys);

    nResult = NGI_DEALLOCATE(ngccRelayHandler_t, relayHandler, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't deallocate storage for the relay handler.\n");
    }

    return;
}

/**
 * Connect
 */
static ngemResult_t
ngcclRelayHandlerConnect(
    ngccRelayHandler_t *relayHandler,
    int bufferSize)
{
    ngcpOptions_t *opts = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    globus_xio_handle_t handle;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME(ngcclRelayHandlerConnect);

    log = ngemLogGetDefault();

    opts = relayHandler->ngrh_options;

    relayHandler->ngrh_relayHandler =
        ngcpRelayHandlerCreate(opts, "ng_client_relay.GT", &relayHandler->ngrh_lock);
    if (relayHandler->ngrh_relayHandler == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the relay handler commom module.\n");
        goto finalize;
    }
    handle = NULL;

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "handler = %p\n", relayHandler->ngrh_relayHandler);

    /* Register Notify Handler */
    nResult = ngcpRelayHandlerRegisterNotify(
        relayHandler->ngrh_relayHandler, "COMMUNICATION_REPLY", true, 
        ngcclRelayHandlerOnCommunicationReply, relayHandler);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't register notify callback.\n");
        goto finalize;
    }

    nResult = ngcpRelayHandlerRegisterNotify(
        relayHandler->ngrh_relayHandler, "CONNECT_REQUEST_NP", true, 
        ngcclRelayHandlerOnConnectRequestNp, relayHandler->ngrh_manager);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't register notify callback.\n");
        goto finalize;
    }

    nResult = ngcclRelayHandlerSendQueryFeatures(relayHandler);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the relay handler commom module.\n");
        goto finalize;
    }

    nResult = ngcclRelayHandlerSendInitialize(relayHandler, bufferSize);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the relay handler commom module.\n");
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:

    return ret;
}


/**
 * Relay Handler: Send QUERY_FEATURES requests
 */
static ngemResult_t
ngcclRelayHandlerSendQueryFeatures(
    ngccRelayHandler_t *relayHandler)
{
    static char *features[] = {NULL};
    static char *requests[] = {
        "QUERY_FEATURES", "INITIALIZE", "PREPARE_COMMUNICATION", "EXIT", NULL};

    return ngcpRelayHandlerSendQueryFeatures(relayHandler->ngrh_relayHandler, features, requests);
}

/**
 * Relay Handler: Send INITIALIZE requests
 */
static ngemResult_t
ngcclRelayHandlerSendInitialize(
    ngccRelayHandler_t *relayHandler,
    int bufferSize)
{
    ngemResult_t nResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    bool locked = false;
    ngcpRequest_t *req = NULL;
    NGEM_FNAME(ngcclRelayHandlerSendInitialize);

    log = ngemLogGetDefault();

    /* Lock */
    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    req = ngcpRequestCreate("INITIALIZE", true, false);
    if (req == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the request.\n");
        goto finalize;
    }
    nResult =
        ngcpRequestAppendArgument(req, "listen_port 0") &&
        ngcpRequestAppendArgument(req, "%s true", NGCC_OPTION_CLIENT_RELAY) &&
        ngcpRequestAppendArgument(req, "buffer_size %d", bufferSize);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set options to the request.\n");
        goto finalize;
    }

    nResult = ngcpRelayHandlerSendRequest(relayHandler->ngrh_relayHandler, req);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send the request.\n");
        goto finalize;
    }

    if (req->ngr_result != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Reply result is \"failed: %s\".\n",
            req->ngr_replyMessage);
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    ngcpRequestDestroy(req);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Relay Handler: Send PREPARE_COMMUNICATION requests
 */
static ngemResult_t
ngcclRelayHandlerSendPrepareCommunication(
    ngccRelayHandler_t *relayHandler,
    int requestID,
    ngccPrepareCommunicationOptions_t *pCommOpts)
{
    ngemResult_t nResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    bool locked = false;
    ngcpRequest_t *req = NULL;
    ngcpOptions_t *opts = pCommOpts->ngpo_userData;
    NGEM_FNAME(ngcclRelayHandlerSendPrepareCommunication);

    log = ngemLogGetDefault();

    /* Lock */
    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    req = ngcpRequestCreate("PREPARE_COMMUNICATION", true, false);
    if (req == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the request.\n");
        goto finalize;
    }

    nResult =
        ngcpRequestAppendArgument(req, "request_id %d", requestID) &&
        ngcpRequestAppendArgument(req, "%s %s", NGCP_OPTION_COMMUNICATION_SECURITY, ngcpCommunicationSecurityString[opts->ngo_communicationSecurity]) &&
        ngcpRequestAppendArgument(req, "tcp_nodelay %s", pCommOpts->ngpo_tcpNodelay?"true":"false") &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryCount %d", pCommOpts->ngpo_retryInfo.ngcri_count) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryBaseInterval %d", pCommOpts->ngpo_retryInfo.ngcri_interval) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryIncreaseRatio %lf", pCommOpts->ngpo_retryInfo.ngcri_increase) &&
        ngcpRequestAppendArgument(req, "tcp_connect_retryRandom %s", pCommOpts->ngpo_retryInfo.ngcri_count?"true":"false");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set options to request.\n");
        goto finalize;
    }

    nResult = ngcpRelayHandlerSendRequest(relayHandler->ngrh_relayHandler, req);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send the request.\n");
        goto finalize;
    }

    if (req->ngr_result != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Reply result is \"failed: %s\".\n",
            req->ngr_replyMessage);
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    ngcpRequestDestroy(req);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Relay Handler: Callback function called on receiving COMMUNICATION_REPLY notify
 */
static void
ngcclRelayHandlerOnCommunicationReply(
    ngcpRelayHandler_t *rh,
    ngcpNotify_t *notify,
    void *userData)
{
    NGEM_LIST_ITERATOR_OF(ngcclCommunicationReply_t) it;
    ngccRelayHandler_t *relayHandler = userData;
    ngcclCommunicationReply_t *val;
    ngcclCommunicationReply_t *comReply = NULL;
    ngemOptionAnalyzer_t *oa = NULL;
    ngLog_t *log;
    ngemResult_t notifyResult = NGEM_FAILED;
    char *message = NULL;
    char *cs = NULL;
    int request_id;
    ngemResult_t nResult;
    bool locked = false;
    ngemResult_t ret;
    NGEM_FNAME(ngcclRelayHandlerOnCommunicationReply);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called with (%p).\n", userData);

    /* Lock */
    nResult = ngcpCommonLockLock(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the relay handler.\n");
        goto finalize;
    }
    locked = true;

    oa = ngemOptionAnalyzerCreate();
    if (oa == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create option analyzer.\n");
        goto finalize;
    }

    /* Setting */
    nResult =
        NGEM_OPTION_ANALYZER_SET_ACTION(int, oa, 
            "request_id", ngemOptionAnalyzerSetInt, &request_id, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(ngemResult_t, oa,
            "result", ngemOptionAnalyzerSetResult, &notifyResult, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, oa,
            NGCP_OPTION_CONTACT_STRING, ngemOptionAnalyzerSetString, &cs, 0, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, oa,
            "message", ngemOptionAnalyzerSetString, &message, 0, 1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set action to the options analyzer.\n");
        goto finalize;
    }
    nResult = ngemOptionAnalyzerAnalyzeList(oa, &notify->ngn_arguments);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't analyzer the line list.\n");
        goto finalize;
    }

    /* Check reply */
    NGEM_LIST_FOREACH(ngcclCommunicationReply_t,
        &relayHandler->ngrh_commReplys, it, val) {
        if ((!val->ngcp_received) &&
            (val->ngcp_id == request_id)) {
            comReply = val;
            break;
        }
    }

    ngLogInfo(log, NGCC_LOGCAT_GT, fName,
        "request_id of notify is %d.\n",
        request_id);

    if (comReply == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Nobody wait COMMUNICATION_REPLY notify whose request_id is %d.\n",
            request_id);
        goto finalize;
    }
    comReply->ngcp_result        = notifyResult;
    comReply->ngcp_contactString = cs;
    comReply->ngcp_message       = message;
    comReply->ngcp_received      = true;
    cs      = NULL;
    message = NULL;

    nResult = ngcpCommonLockBroadcast(&relayHandler->ngrh_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:
    if (oa != NULL) {
        ngemOptionAnalyzerDestroy(oa);
    }

    ngiFree(cs, log, NULL);
    ngiFree(message, log, NULL);

    /* Unlock */
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(&relayHandler->ngrh_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the relay handler.\n");
            ret = NGEM_FAILED;
        }
    }
    return;
}

/**
 * Relay Handler: Callback function called on receiving CONNECT_REQUEST_NP notify
 */
static void
ngcclRelayHandlerOnConnectRequestNp(
    ngcpRelayHandler_t *rh,
    ngcpNotify_t *notify,
    void *userData)
{
    ngccRelayHandlerManager_t *manager = userData;
    ngemOptionAnalyzer_t *oa = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    char *contactString = NULL;
    ngcpCommunicationSecurity_t sec;
    ngccRelayHandlerConnectRequestCallback_t callback = NULL;
    void *callbackUserData = NULL;
    bool self;
    bool tcpNodelay = false;
    NGEM_FNAME(ngcclRelayHandlerOnConnectRequestNp);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called with (%p).\n", userData);

    callback         = manager->ngrhm_callback;
    callbackUserData = manager->ngrhm_userData;

    oa = ngemOptionAnalyzerCreate();
    if (oa == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create option analyzer.\n");
        goto finalize;
    }

    /* Setting */
    nResult =
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, oa, 
            NGCP_OPTION_CONTACT_STRING,
            ngemOptionAnalyzerSetString, &contactString, 1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(ngcpCommunicationSecurity_t, oa,
            NGCP_OPTION_COMMUNICATION_SECURITY,
            ngcpOptionAnalyzerSetCommunicationSecurity, &sec, 1, 1);
        NGEM_OPTION_ANALYZER_SET_ACTION(bool, oa,
            "tcp_nodelay",
            ngemOptionAnalyzerSetBool, &tcpNodelay, 1, 1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set action to the options analyzer.\n");
        goto finalize;
    }
    nResult = ngemOptionAnalyzerAnalyzeList(oa, &notify->ngn_arguments);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't analyzer the line list.\n");
        goto finalize;
    }

    self = false;
    if (rh->ngrh_crypt) {
        self = true;
    }

    callback(callbackUserData, contactString, sec, self, tcpNodelay);
    
finalize:
    ngiFree(contactString, log, NULL);
    contactString = NULL;

    return;
}
