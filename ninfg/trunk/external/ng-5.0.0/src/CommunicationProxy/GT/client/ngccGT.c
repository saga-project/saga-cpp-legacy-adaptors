/*
 * $RCSfile: ngccGT.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $
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

#include "ngcpXIO.h"
#include "ngccProtocol.h"
#include "ngcpRelayHandler.h"
#include "ngcpCallback.h"
#include "ngcpOptions.h"
#include "ngccGT.h"

NGI_RCSID_EMBED("$RCSfile: ngccGT.c,v $ $Revision: 1.16 $ $Date: 2008/03/28 03:52:30 $")

#define NGCCL_BUFFER_SIZE      (32 * 1024)
#define NGCCL_MAX_NLISTENERS   16

typedef enum ngccGTserverType_e {
    NGCC_GT_FOR_CLIENT,
    NGCC_GT_FOR_REMOTE
} ngccGTserverType_t;

typedef struct ngccGTcontext_s ngccGTcontext_t;

/**
 * Communication pair for relaying between Ninf-G Client and Remote
 * Communication Proxy GT.
 */
typedef struct ngcclCommunicationPair_s {
    ngccGTcontext_t    *ngcp_context;
    globus_xio_handle_t ngcp_remoteConnection;
    globus_xio_handle_t ngcp_clientConnection;
    unsigned char       ngcp_remoteBuffer[NGCCL_BUFFER_SIZE];
    unsigned char       ngcp_clientBuffer[NGCCL_BUFFER_SIZE];
    bool                ngcp_destroying;
    ngiRlock_t          ngcp_lock;
    bool                ngcp_counted;
} ngcclCommunicationPair_t;

NGEM_DECLARE_LIST_OF(ngcclCommunicationPair_t);

typedef struct ngccGTlistener_s {
    /* If handle is null, this structure is invalid.  */
    ngccGTcontext_t                       *ngl_context;
    ngccGTserverType_t                     ngl_type;
    bool                                   ngl_tcpNodelay;
    globus_xio_server_t                    ngl_handle;
    ngcpCommunicationSecurity_t            ngl_security;
    char                                  *ngl_contactString;
    NGEM_LIST_OF(ngcclCommunicationPair_t) ngl_pairQueue;
    ngiRlock_t                             ngl_lock;
    bool                                   ngl_callbackRunning;
} ngccGTlistener_t;

/**
 * Context of Client Communication Proxy GT
 */
struct ngccGTcontext_s {
    ngccGTlistener_t    nggt_listeners[NGCCL_MAX_NLISTENERS];

    ngcpPortRange_t     nggt_portRange;
    bool                nggt_relayMode;
    ngccProtocol_t     *nggt_protocol;

    bool                ngcp_destroying;
    ngiRlock_t          nggt_rLock;
    ngccRelayHandlerManager_t *nggt_handlers;

    int                 nggt_nConnection;
    ngiRlock_t          nggt_lockForNconnection;
};

typedef struct ngcclRelayHandlingCallbackArgument_s {
    ngccGTcontext_t                   *ngra_context;
    ngccPrepareCommunicationOptions_t *ngra_pCommOpts;
    ngccRelayHandler_t                *ngra_relayHandler;
} ngcclRelayHandlingCallbackArgument_t;

typedef struct ngcclRelayHandlerSendCRcallbackArgument_s {
    ngccProtocol_t                    *ngra_protocol;
    ngccPrepareCommunicationOptions_t *ngra_pCommOpts;
} ngcclRelayHandlerSendCRcallbackArgument_t;

/* Callback for requests */
static ngemResult_t ngcclInitializeBegin(ngccProtocol_t *, ngemOptionAnalyzer_t *);
static ngemResult_t ngcclInitializeEnd(ngccProtocol_t *, ngccInitializeOptions_t *, ngemReply_t *, ngemResult_t);
static ngemResult_t ngcclPrepareCommunicationBegin(ngccProtocol_t *, ngccPrepareCommunicationOptions_t *, ngemOptionAnalyzer_t *);
static ngemResult_t ngcclPrepareCommunicationEnd(ngccProtocol_t *, ngccPrepareCommunicationOptions_t*, ngemReply_t *, ngemResult_t);

/* Context */
ngemResult_t ngccGTcontextRun(char *logfile);
static ngccGTcontext_t *ngcclGTcontextCreate(char *logfile);
static ngemResult_t ngcclGTcontextDestroy(ngccGTcontext_t *);
static ngemResult_t ngcclGTcontextConnectionInc(ngccGTcontext_t *);
static ngemResult_t ngcclGTcontextConnectionDec(ngccGTcontext_t *);
static ngemResult_t ngcclGTcontextConnectionWait(ngccGTcontext_t *);

static ngccGTlistener_t *ngcclGTcontextGetListener(ngccGTcontext_t *, ngccGTserverType_t, ngcpCommunicationSecurity_t, bool);

/* Listener */
static void ngcclGTlistenerZeroClear(ngccGTlistener_t *);
static ngemResult_t ngcclGTlistenerInitialize(ngccGTlistener_t *, ngccGTcontext_t *);
static ngemResult_t ngcclGTlistenerListen(ngccGTlistener_t *, ngccGTserverType_t, ngcpCommunicationSecurity_t, bool);
static ngemResult_t ngcclGTlistenerClose(ngccGTlistener_t *);
static ngemResult_t ngcclGTlistenerFinalize(ngccGTlistener_t *listener);
static void ngcclGTlistenerAcceptCallback(globus_xio_server_t, globus_xio_handle_t, globus_result_t, void *);

/* Callback for communication */
static void ngcclReadCallback(globus_xio_handle_t, globus_result_t, globus_byte_t *, globus_size_t, globus_size_t, globus_xio_data_descriptor_t, void *);
static void ngcclWriteCallback(globus_xio_handle_t, globus_result_t, globus_byte_t *, globus_size_t, globus_size_t, globus_xio_data_descriptor_t, void *);

/* Communication Pair */
static ngcclCommunicationPair_t *ngcclCommunicationPairCreate(globus_xio_handle_t, ngccGTcontext_t *);
static ngemResult_t ngcclCommunicationPairConnect(ngcclCommunicationPair_t *, uint16_t, bool);
static ngemResult_t ngcclCommunicationPairJoin(ngcclCommunicationPair_t *, globus_xio_handle_t);
static ngemResult_t ngcclCommunicationPairRelayStart(ngcclCommunicationPair_t *);
static ngemResult_t ngcclCommunicationPairDestroy(ngcclCommunicationPair_t *);
static bool ngcclCommunicationPairIsDestroying(ngcclCommunicationPair_t *pair);

static void ngcclRelayHandlerSendCRcallback(void *, ngemCallbackResult_t);
static void ngcclRelayHandlerSendConnectRequestCallback(void *, ngemCallbackResult_t);
static void *ngcclRelayHandlingCallback(void *);

static void ngcclGTcontextOnConnectRequest(void *, const char *, ngcpCommunicationSecurity_t, bool, bool);

static void ngcclServerCloseCallback(globus_xio_server_t, void *);

/**
 * Information of the protocol
 */
static ngccProtocolInformation_t ngtlInfo = {
    ngcclInitializeBegin,
    ngcclInitializeEnd,
    ngcclPrepareCommunicationBegin,
    ngcclPrepareCommunicationEnd,
    false
};

int
main(int argc, char *argv[])
{
    int debug;
    char *logFileName = NULL;
    int opt;
    ngemResult_t nResult;
    NGEM_FNAME_TAG(main);

    /* Options analyze */
    while ((opt = getopt(argc, argv, "l:d")) >= 0) {
        switch (opt) {
        case 'l':
            /* LOG */
            logFileName = optarg;
            break;
        case 'd':
            debug = 1;
            break;
        case '?':
        default:
            /* Ignore arguments */
            ;
        }
    }

    /* Run */
    nResult = ngccGTcontextRun(logFileName);
    if (nResult == NGEM_SUCCESS) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * Callback function on receiving begin of INITIALIZE request
 */
static ngemResult_t 
ngcclInitializeBegin(
    ngccProtocol_t *protocol,
    ngemOptionAnalyzer_t *analyzer)
{
    ngemResult_t nResult;
    ngLog_t *log;
    ngccGTcontext_t *context = NULL;
    NGEM_FNAME(ngcclInitializeBegin);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called.\n");

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT(analyzer != NULL);

    context = (ngccGTcontext_t *)protocol->ngp_userData;

    NGEM_ASSERT(context != NULL);

    nResult = 
        NGEM_OPTION_ANALYZER_SET_ACTION(ngcpPortRange_t, analyzer,
            NGCC_OPTION_PORT_RANGE, ngcpOptionsAnalyzerSetPortRange, &context->nggt_portRange, 0, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(bool, analyzer,
            NGCC_OPTION_CLIENT_RELAY, ngemOptionAnalyzerSetBool, &context->nggt_relayMode, 0, 1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set callback for options.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Callback function after receiving INITIALIZE request
 * This function output log only.
 */
static ngemResult_t 
ngcclInitializeEnd(
    ngccProtocol_t *protocol,
    ngccInitializeOptions_t *options,
    ngemReply_t *reply, 
    ngemResult_t result)
{
    ngccGTcontext_t *context = NULL;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngcclInitializeEnd);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called.\n");

    NGEM_ASSERT(protocol != NULL);
    NGEM_ASSERT(options  != NULL);
    NGEM_ASSERT(reply    != NULL);

    context = protocol->ngp_userData;

    NGEM_ASSERT(context != NULL);

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "listen_port  = \"%d\"\n", options->ngio_clientPort);
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "min_port     = \"%d\"\n", context->nggt_portRange.ngpr_min);
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "max_port     = \"%d\"\n", context->nggt_portRange.ngpr_max);
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "clientRelay? = \"%s\"\n", (context->nggt_relayMode != 0)?"true":"false");

    return NGEM_SUCCESS;
}

/**
 * Callback function on receiving begin of PREPARE_COMMUNICATION request
 */
static ngemResult_t
ngcclPrepareCommunicationBegin(
    ngccProtocol_t *protocol,
    ngccPrepareCommunicationOptions_t *pCommOpts,
    ngemOptionAnalyzer_t *analyzer)
{
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    ngcpOptions_t *opts = NULL;
    NGEM_FNAME(ngcclPrepareCommunicationBegin);

    log = ngemLogGetDefault();

    NGEM_ASSERT(protocol  != NULL);
    NGEM_ASSERT(pCommOpts != NULL);
    NGEM_ASSERT(analyzer  != NULL);

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called.\n");

    opts = ngcpOptionsCreate();
    if (opts == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create PREPARE_COMMUNICATION's option for Communication Proxy GT.\n");
        goto error;
    }

    /* Options */
    nResult =
        NGEM_OPTION_ANALYZER_SET_ACTION(
            ngcpCommunicationSecurity_t, analyzer,
            NGCP_OPTION_COMMUNICATION_SECURITY,
            ngcpOptionAnalyzerSetCommunicationSecurity,
            &opts->ngo_communicationSecurity, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            char *, analyzer,
            NGCP_OPTION_CLIENT_RELAY_HOST,
            ngemOptionAnalyzerSetString,
            &opts->ngo_relayHost, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            ngcpRelayInvokeMethod_t, analyzer,
            NGCP_OPTION_CLIENT_RELAY_INVOKE_METHOD,
            ngcpOptionAnalyzerSetRelayInvokeMethod,
            &opts->ngo_relayInvokeMethod,  0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            NGEM_LIST_OF(char),  analyzer,
            NGCP_OPTION_CLIENT_RELAY_OPTION,
            ngemOptionAnalyzerSetStringList,
            &opts->ngo_relayOptions, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            bool, analyzer,
            NGCP_OPTION_CLIENT_RELAY_CRYPT,
            ngemOptionAnalyzerSetBool,
            &opts->ngo_relayCrypt, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            char *, analyzer,
            NGCP_OPTION_CLIENT_RELAY_GSISSH_COMMAND,
            ngemOptionAnalyzerSetString,
            &opts->ngo_relayGSISSHcommand, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            NGEM_LIST_OF(char), analyzer,
            NGCP_OPTION_CLIENT_RELAY_GSISSH_OPTION,
            ngemOptionAnalyzerSetStringList,
            &opts->ngo_relayGSISSHoptions, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_REMOTE_RELAY_HOST,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_REMOTE_RELAY_INVOKE_METHOD,
            ngemOptionAnalyzerSetIgnore,
            NULL,  0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void,  analyzer,
            NGCP_OPTION_REMOTE_RELAY_OPTION,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_REMOTE_RELAY_CRYPT,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_REMOTE_RELAY_GSISSH_COMMAND,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_REMOTE_RELAY_GSISSH_OPTION,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, -1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set callback for reading options.\n");
        goto error;
    }

    NGCC_PREPARE_COMMUNICATION_OPTION_USERDATA(pCommOpts) = opts;

    return NGEM_SUCCESS;
error:
    nResult = ngcpOptionsDestroy(opts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy PREPARE_COMMUNICATION's option for Communication Proxy GT.\n");
    }

    return NGEM_FAILED;
}

/**
 * Callback function after receiving PREPARE_COMMUNICATION request
 */
static ngemResult_t
ngcclPrepareCommunicationEnd(
    ngccProtocol_t *protocol,
    ngccPrepareCommunicationOptions_t *pCommOpts,
    ngemReply_t *reply,
    ngemResult_t result)
{
    ngcpOptions_t *opts = NULL;
    ngccGTcontext_t *context = NULL;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    char *message = NULL;
    ngccGTlistener_t *listener = NULL;
    bool locked = false;
    ngcclRelayHandlingCallbackArgument_t *cArg = NULL;
    char *env = NULL;
    globus_thread_t tid;
    NGEM_FNAME(ngcclPrepareCommunicationEnd);
    
    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called.\n");

    NGEM_ASSERT(protocol  != NULL);
    NGEM_ASSERT(pCommOpts != NULL);
    NGEM_ASSERT(reply     != NULL);

    context = protocol->ngp_userData;
    opts = NGCC_PREPARE_COMMUNICATION_OPTION_USERDATA(pCommOpts);

    NGEM_ASSERT(context != NULL);
    NGEM_ASSERT(opts    != NULL);

    if (result != NGEM_SUCCESS) {
        /* It is not necessary to send COMMUNICATION_NOTIFY */
        goto finalize;
    }

    if (opts->ngo_communicationSecurity != NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY) {
        env = getenv(NGCP_ENV_ONLY_PRIVATE);
        if ((env != NULL) && (strcmp(env, "0") != 0)) {
            message = "Allows \"confidentiality\" communication only";
            ret = NGEM_FAILED;
            goto finalize;
        }
    }

    if ((opts->ngo_communicationSecurity != NGCP_COMMUNICATION_SECURITY_NONE) ||
        ((opts->ngo_relayHost != NULL) && (opts->ngo_relayCrypt))) {
        if (!ngcpCredentialAvailable()) {
            message = "The proxy credential is not available";
            ret = NGEM_FAILED;
            goto finalize;
        }
    }

    result = ngiRlockLock(&context->nggt_rLock, log, NULL);
    if (result == 0) {
        message = "Can't get the lock.";
        ret = NGEM_FAILED;
        goto finalize;
    }
    locked = true;

    listener = ngcclGTcontextGetListener(
        context, NGCC_GT_FOR_REMOTE, opts->ngo_communicationSecurity,
        pCommOpts->ngpo_tcpNodelay);
    if (listener == NULL) {
        message = "Can't get the listener.";
        ret = NGEM_FAILED;
        goto finalize;
    }

    if (opts->ngo_relayHost != NULL) {
        /* Relay Process */

        cArg = NGI_ALLOCATE(ngcclRelayHandlingCallbackArgument_t, log, NULL);
        if (cArg == NULL) {
            message = "Can't allocate storage for callback arguments.";
            ret = NGEM_FAILED;
            goto finalize;
        }

        cArg->ngra_context   = context;
        cArg->ngra_pCommOpts = pCommOpts;
        cArg->ngra_relayHandler= NULL;
        cArg->ngra_relayHandler= ngccRelayHandlerManagerGet(context->nggt_handlers,
            NGCC_PREPARE_COMMUNICATION_OPTION_USERDATA(pCommOpts));
        if (cArg->ngra_relayHandler == NULL) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't get the relay handler.\n");
            ret = NGEM_FAILED;
            goto finalize;
        }

        ngLogDebug(log, NGCC_LOGCAT_GT, fName,
            "Registers callback (ngcclRelayHandlingCallbackArgument_t *)(%p).\n", cArg);

        result = globus_thread_create(&tid, NULL, ngcclRelayHandlingCallback, cArg);
        if (result != 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "globus_thread_create: %s\n", strerror(result));
            message = "Can't create a new thread.\n";
            ret = NGEM_FAILED;
            goto finalize;
        }
        cArg = NULL;
        ret = NGEM_SUCCESS;
        goto finalize2;
    }
    NGEM_ASSERT(listener->ngl_contactString != NULL);

    nResult = ngemNotifyAddOption(
        NGCC_PREPARE_COMMUNICATION_OPTION_NOTIFY(pCommOpts),
        NGCP_OPTION_CONTACT_STRING, "%s", listener->ngl_contactString);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't add contact string.";
        ret = NGEM_FAILED;
        goto finalize;
    }

finalize:
    if (ret != NGEM_SUCCESS) {
        NGEM_ASSERT_STRING(message);
        ngLogError(log, NGCC_LOGCAT_GT, fName, "%s\n", message);
        nResult = ngccPrepareCommunicationOptionsSetError(pCommOpts, message);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't set error to COMMUNICATION_REPLY notify.\n");
        }
    }

    nResult = ngccProtocolSendCommunicationReplyNotify(protocol, pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send COMMUNICATION_REPLY notify.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngcpOptionsDestroy(opts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy prepare communication options.\n");
        ret = NGEM_FAILED;
    }

finalize2:
    if (cArg != NULL) {
        if (cArg->ngra_relayHandler != NULL) {
            nResult = ngccRelayHandlerUnref(cArg->ngra_relayHandler);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't unref the relay handler.\n");
            }
        }
        NGI_DEALLOCATE(ngcclRelayHandlingCallbackArgument_t, cArg, log, NULL);
        cArg = NULL;
    }

    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&context->nggt_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the context.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Context: run application
 */
ngemResult_t
ngccGTcontextRun(
    char *logfile)
{
    ngccGTcontext_t *context = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngccGTcontextRun);

    context = ngcclGTcontextCreate(logfile);
    log = ngemLogGetDefault();
    if (context == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't create context.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* Run */
    nResult = ngemCallbackManagerRun();
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't run Callback Manager.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

finalize:

    nResult = ngcclGTcontextDestroy(context);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't destroy the context.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Context: create
 */
static ngccGTcontext_t *
ngcclGTcontextCreate(
    char *logfile)
{
    static bool firstCall = true;
    ngccGTcontext_t *context = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    ngccProtocol_t *protocol = NULL;
    int result;
    int i;
    NGEM_FNAME(ngcclGTcontextCreate);

    if (!firstCall) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't call %s more then one time.\n", fName);
        return NULL;
    }
    firstCall = false;

    /* Initialize Log */
    nResult = ngemLogInitialize(NGCC_APPLICATION_NAME, logfile);
    if (nResult == NGEM_FAILED) {
        fprintf(stderr, "Can't initialize the log.\n");
        goto error;
    }
    log = ngemLogGetDefault();

    /* Initialize Globus XIO */
    nResult = ngcpGlobusXIOinitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't initialize Globus XIO.\n");
        goto error;
    }

    /* Callback Manager */
    nResult = ngcpCallbackInitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't initialize callback.\n");
        goto error;
    }

    /* Allocate */
    context = NGI_ALLOCATE(ngccGTcontext_t, log, NULL);
    if (context == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't allocate storage for the context.\n");
        goto error;
    }

    /* Protocol */
    protocol = ngccProtocolCreate(&ngtlInfo, context);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't create the protocol.\n");
        goto error;
    }

    /* Listeners */
    for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
        ngcclGTlistenerZeroClear(&context->nggt_listeners[i]);
    }
    for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
        nResult = ngcclGTlistenerInitialize(&context->nggt_listeners[i], context);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't initialize the listener.\n");
            goto error;
        }
    }
    context->nggt_nConnection           = 0;
    context->nggt_protocol              = protocol;
    context->nggt_rLock                 = NGI_RLOCK_NULL;
    NGCP_PORT_RANGE_SET(&context->nggt_portRange, 0, 0);

    result = ngiRlockInitialize(&context->nggt_rLock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't initialize rLock.\n");
        goto error;
    }

    result = ngiRlockInitialize(&context->nggt_lockForNconnection, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't initialize rLock.\n");
        goto error;
    }

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "Registers context(%p) as argument of \"%s()\".\n",
        context, "ngcclGTcontextOnConnectRequest");
    context->nggt_handlers = ngccRelayHandlerManagerCreate(ngcclGTcontextOnConnectRequest, context);
    if (context->nggt_handlers == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the Client Relay Manager.\n");
        goto error;
    }

    return context;
error:
    nResult = ngcclGTcontextDestroy(context);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName, "Can't destroy the context.\n");
    }
    return NULL;
}

/**
 * Context: destroy
 */
ngemResult_t
ngcclGTcontextDestroy(
    ngccGTcontext_t *context)
{
    ngemResult_t nResult;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log = NULL;
    int i;
    NGEM_FNAME(ngcclGTcontextDestroy);

    log = ngemLogGetDefault();

    if (context == NULL) {
        /* Do nothing */
        return NGEM_SUCCESS;
    }

    if (!NGI_RLOCK_IS_NULL(&context->nggt_rLock)) {
        ngccRelayHandlerManagerDestroy(context->nggt_handlers);
        context->nggt_handlers = NULL;

        nResult = ngcclGTcontextConnectionWait(context);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't wait all connections to close.\n");
            ret = NGEM_FAILED;
        }

        for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
            nResult = ngcclGTlistenerClose(&context->nggt_listeners[i]);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't finalize the listener.\n");
                ret = NGEM_FAILED;
            }
        }

        for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
            nResult = ngcclGTlistenerFinalize(&context->nggt_listeners[i]);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't finalize the listener.\n");
                ret = NGEM_FAILED;
            }
        }
    }
    NGEM_ASSERT(context->nggt_handlers == NULL);

    result = ngiRlockFinalize(&context->nggt_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize rlock.\n");
        ret = NGEM_FAILED;
    }

    result = ngiRlockFinalize(&context->nggt_lockForNconnection, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize rlock.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngccProtocolDestroy(context->nggt_protocol);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy the protocol.\n");
        ret = NGEM_FAILED;
    }

    context->nggt_nConnection = 0;
    context->nggt_protocol    = NULL;
    context->nggt_rLock       = NGI_RLOCK_NULL;

    NGCP_PORT_RANGE_SET(&context->nggt_portRange, 0, 0);

    result = NGI_DEALLOCATE(ngccGTcontext_t, context, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't deallocate storage for the context.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngcpCallbackFinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize the callback manager.\n");
        ret = NGEM_FAILED;
    }

    /* Finalize XIO */ 
    nResult = ngcpGlobusXIOfinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize Globus XIO.\n");
        ret = NGEM_FAILED;
    }
    
    nResult = ngemLogFinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(NULL, NGCC_LOGCAT_GT, fName,
            "Can't finalize the log.\n");
        ret = NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Context: Increment counter of number of connections. 
 */
static ngemResult_t
ngcclGTcontextConnectionInc(
    ngccGTcontext_t *context)
{
    ngLog_t *log = NULL;
    int result;
    NGEM_FNAME(ngcclGTcontextConnectionInc);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&context->nggt_lockForNconnection, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the context.\n");
        return NGEM_FAILED;
    }
    context->nggt_nConnection++;
    result = ngiRlockUnlock(&context->nggt_lockForNconnection, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't unlock the context.\n");
        return NGEM_FAILED;
    }
    return NGEM_SUCCESS;
}

/**
 * Context: Decrements counter of number of connections. 
 */
static ngemResult_t
ngcclGTcontextConnectionDec(
    ngccGTcontext_t *context)
{
    bool locked = false;
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    int result;
    NGEM_FNAME(ngcclGTcontextConnectionDec);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&context->nggt_lockForNconnection, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the context.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }
    locked = true;
    NGEM_ASSERT(context->nggt_nConnection > 0);
    context->nggt_nConnection--;
    if (context->nggt_nConnection == 0) {
        result = ngiRlockBroadcast(&context->nggt_lockForNconnection, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't broadcast the signal.\n");
            ret = NGEM_FAILED;
            goto finalize;
        }
    }
finalize:
    if (locked) {
        result = ngiRlockUnlock(&context->nggt_lockForNconnection, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the context.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 * Context: Wait number of connections to be 0.
 */
static ngemResult_t
ngcclGTcontextConnectionWait(
    ngccGTcontext_t *context)
{
    bool locked = false;
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    int result;
    NGEM_FNAME(ngcclGTcontextConnectionWait);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&context->nggt_lockForNconnection, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the context.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }
    locked = true;
    while (context->nggt_nConnection > 0) {
        result = ngiRlockWait(&context->nggt_lockForNconnection, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't wait the signal.\n");
            ret = NGEM_FAILED;
            goto finalize;
        }
    }
finalize:
    if (locked) {
        result = ngiRlockUnlock(&context->nggt_lockForNconnection, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the context.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }
    return ret;
}

/**
 * context: get listener
 */
static ngccGTlistener_t *
ngcclGTcontextGetListener(
    ngccGTcontext_t *context,
    ngccGTserverType_t sType,
    ngcpCommunicationSecurity_t sec,
    bool tcpNodelay)
{
    ngccGTlistener_t *p = NULL;
    int i;
    ngLog_t *log;
    ngemResult_t nResult;
    ngccGTlistener_t *ret = NULL;
    int result;
    bool locked = false;
    bool listenerLocked = false;
    NGEM_FNAME(ngcclGTcontextGetListener);

    NGEM_ASSERT(context != NULL);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&context->nggt_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the context.\n");
        goto finalize;
    }
    locked = true;

    for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
        p = &context->nggt_listeners[i];
        result = ngiRlockLock(&p->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't lock the listener.\n");
            goto finalize;
        }
        listenerLocked = true;
        if ((p->ngl_handle != NULL) &&
            (p->ngl_security == sec) && (p->ngl_type == sType) &&
            (p->ngl_tcpNodelay == tcpNodelay)) {
            ret = p;
            goto finalize;
        }
        if (listenerLocked) {
            listenerLocked = false;
            result = ngiRlockUnlock(&p->ngl_lock, log, NULL);
            if (result == 0) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't unlock the listener.\n");
                goto finalize;
            }
        }
    }

    for (i = 0;i < NGI_NELEMENTS(context->nggt_listeners);++i) {
        p = &context->nggt_listeners[i];
        result = ngiRlockLock(&p->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't lock the listener.\n");
            goto finalize;
        }
        listenerLocked = true;
        if (p->ngl_handle == NULL) {
            nResult = ngcclGTlistenerListen(p, sType, sec, tcpNodelay);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't initialize the listener.\n");
                goto finalize;
            }
            ret = p;
            goto finalize;
        }
        if (listenerLocked) {
            listenerLocked = false;
            result = ngiRlockUnlock(&p->ngl_lock, log, NULL);
            if (result == 0) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't unlock the listener.\n");
                goto finalize;
            }
        }
    }

    NGEM_ASSERT_NOTREACHED();
finalize:
    if (listenerLocked) {
        NGEM_ASSERT(p != NULL);
        listenerLocked = false;
        result = ngiRlockUnlock(&p->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the context.\n");
            goto finalize;
        }
    }
    if (locked) {
        result = ngiRlockUnlock(&context->nggt_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the context.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }
    return ret;
}

/**
 * Callback function for accept()
 */
static void
ngcclGTlistenerAcceptCallback(
    globus_xio_server_t server,
    globus_xio_handle_t handle,
    globus_result_t     cResult,
    void               *user_arg)
{
    ngccGTlistener_t *listener = (ngccGTlistener_t *)user_arg;
    ngccGTlistener_t *listenerForClient = NULL;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngcclCommunicationPair_t *pair = NULL;
    ngccGTcontext_t *context = (ngccGTcontext_t *)user_arg;
    ngLog_t *log = NULL;
    bool listener_close = false;
    bool locked = true;
    int result;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME(ngcclGTlistenerAcceptCallback);

    NGEM_ASSERT(listener != NULL);

    context = listener->ngl_context;

    NGEM_ASSERT(context != NULL);
    NGEM_ASSERT(context->nggt_protocol->ngp_initOpts != NULL);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called with (ngccGTlistener_t)%p\n", user_arg);

    /* Lock */
    result = ngiRlockLock(&listener->ngl_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't locked the listener.\n");
        listener_close = true;
        goto finalize;
    }
    locked = true;
    NGEM_ASSERT(listener->ngl_callbackRunning);

    /* Check Callback result */
    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             listener_close = true;
             goto finalize;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
             listener_close = true;
         }
         ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
             "Callback function for reading", cResult);
         goto finalize;
    }

    /* Open Remote Connection */
    gResult = globus_xio_open(handle, NULL, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName, "globus_xio_open", gResult);
        goto finalize;
    }

    if (listener->ngl_security != NGCP_COMMUNICATION_SECURITY_NONE) {
        bool ok;
        unsigned int auth = NGCP_COMMUNICATION_AUTH_SELF;

        if (listener->ngl_type == NGCC_GT_FOR_REMOTE) {
            auth |= NGCP_COMMUNICATION_AUTH_HOST;
        }
        nResult = ngcpGlobusXIOcheckPeerName(handle, auth, &ok);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't check peer name.\n");
            goto finalize;
        }
        if (!ok) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Peer name is not authorized.\n");
            goto finalize;
        }
    }

    if (listener->ngl_type == NGCC_GT_FOR_CLIENT) {
        pair = NGEM_LIST_HEAD(ngcclCommunicationPair_t, &listener->ngl_pairQueue);
        if (pair == NULL) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "There are no connection from remote site.\n");
            goto finalize;
        }
        nResult = NGEM_LIST_ERASE_BY_ADDRESS(ngcclCommunicationPair_t, &listener->ngl_pairQueue, pair);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't erase item from the list.\n");
            goto finalize;
        }

        nResult = ngcclCommunicationPairJoin(pair, handle);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't connect to Ninf-G Client.\n");
            goto finalize;
        }
        handle = NULL;

        nResult = ngcclCommunicationPairRelayStart(pair);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't start relay communication.\n");
            goto finalize;
        }
    } else {
        pair = ngcclCommunicationPairCreate(handle, context);
        if (pair == NULL) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't create communication pair.\n");
            goto finalize;
        }
        handle = NULL;

        if (context->nggt_relayMode) {
            listenerForClient = ngcclGTcontextGetListener(
                context, NGCC_GT_FOR_CLIENT, listener->ngl_security,
                listener->ngl_tcpNodelay);
            if (listenerForClient == NULL) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't get listener.\n");
                goto finalize;
            }
            /* Set handle to list */
            nResult = NGEM_LIST_INSERT_TAIL(ngcclCommunicationPair_t,
                    &listenerForClient->ngl_pairQueue, pair);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't push connection from remote.\n");
                goto finalize;
            }

            nResult = ngcpCallbackOneshot(
                ngcclRelayHandlerSendConnectRequestCallback, listenerForClient);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't set callback for sending COMMUNICATION_REPLY notify.\n");
                goto finalize;
            }
        } else {
            nResult = ngcclCommunicationPairConnect(pair, 
                context->nggt_protocol->ngp_initOpts->ngio_clientPort,
                listener->ngl_tcpNodelay);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't connect to Ninf-G Client.\n");
                goto finalize;
            }

            nResult = ngcclCommunicationPairRelayStart(pair);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't start relay communication.\n");
                goto finalize;
            }
        }
    }

    ret = NGEM_SUCCESS;
finalize:
    if (!listener_close) {
        gResult = globus_xio_server_register_accept(
            server, ngcclGTlistenerAcceptCallback, user_arg);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_server_register_accept", gResult);
            listener_close = true;
        }
    }

    if (listener_close) {
        nResult = ngcclGTlistenerClose(listener);
        if (nResult == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't finalize the listener.\n");
        }
        listener->ngl_callbackRunning = false;
        result = ngiRlockBroadcast(&listener->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't broadcast signals for the listener.\n");
        }
    }
    if (ret != NGEM_SUCCESS) {
        nResult = ngcclCommunicationPairDestroy(pair);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't destroy communication.\n");
        }
    }
    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
    }

    if (locked) {
        result = ngiRlockUnlock(&listener->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlocked the listener.\n");
        }
        locked = false;
    }
    return;
}

/**
 * Callback for read()
 */
static void
ngcclReadCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                bufferSize,
    globus_size_t                nRead,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngcclCommunicationPair_t *pair = userData;
    globus_result_t gResult;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngcclReadCallback);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called\n");

    if (ngcclCommunicationPairIsDestroying(pair)) {
        ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Communication Pair is destroying.\n");
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_FALSE) {
             ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                 "Callback function for reading", cResult);
         }
         goto communication_close;
    }

    if (handle == pair->ngcp_remoteConnection) {
        /* Reading from remote communication proxy, writes Ninf-G Client */
        ngLogDebug(log, NGCC_LOGCAT_GT, fName,
            "Reads %ld bytes from Remote Communication Proxy.\n", (unsigned long)nRead);
        gResult = globus_xio_register_write(
            pair->ngcp_clientConnection,
            buffer, nRead, nRead, NULL, ngcclWriteCallback, pair);
    } else {
        NGEM_ASSERT(handle == pair->ngcp_clientConnection);
        /* Read from Ninf-G Client, writes remote communication proxy  */
        ngLogDebug(log, NGCC_LOGCAT_GT, fName,
            "Reads %ld bytes from Ninf-G Client.\n", (unsigned long)nRead);
        gResult = globus_xio_register_write(
            pair->ngcp_remoteConnection,
            buffer, nRead, nRead, NULL, ngcclWriteCallback, pair);
    }
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_register_write", cResult);
        goto communication_close;
    }

    return;
communication_close:
    nResult = ngcclCommunicationPairDestroy(pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy communication pair.\n");
    }
    return;
}

/**
 * Callback for write()
 */
static void
ngcclWriteCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                len,
    globus_size_t                nWrite,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngcclCommunicationPair_t *pair = userData;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngcclWriteCallback);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called\n");

    if (ngcclCommunicationPairIsDestroying(pair)) {
        ngLogDebug(log, NGCC_LOGCAT_GT, fName,
            "Communication Pair is destroying.\n");
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
             "Callback function for reading", cResult);
         goto communication_close;
    }

    if (handle == pair->ngcp_remoteConnection) {
        /* Read from remote*/
        gResult = globus_xio_register_read(
            pair->ngcp_clientConnection, pair->ngcp_clientBuffer,
            NGCCL_BUFFER_SIZE, 1, NULL, ngcclReadCallback, pair);
    } else  {
        NGEM_ASSERT(handle == pair->ngcp_clientConnection);
        gResult = globus_xio_register_read(
            pair->ngcp_remoteConnection, pair->ngcp_remoteBuffer,
            NGCCL_BUFFER_SIZE, 1, NULL, ngcclReadCallback, pair);
    }
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_register_read", cResult);
        goto communication_close;
    }

    return;

communication_close:
    nResult = ngcclCommunicationPairDestroy(pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy communication pair.\n");
    }
    return;
}

/**
 * Communication Pair: Create
 */
static ngcclCommunicationPair_t *
ngcclCommunicationPairCreate(
    globus_xio_handle_t remoteConnection,
    ngccGTcontext_t *context)
{
    ngcclCommunicationPair_t *pair = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    int result;
    NGEM_FNAME(ngcclCommunicationPairCreate);

    log = ngemLogGetDefault();

    pair = NGI_ALLOCATE(ngcclCommunicationPair_t, log, NULL);
    if (pair == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't allocate storage for communication pair.\n");
        goto error;
    }
    pair->ngcp_remoteConnection = NULL;
    pair->ngcp_clientConnection = NULL;
    pair->ngcp_context          = context;
    pair->ngcp_destroying       = false;
    pair->ngcp_lock             = NGI_RLOCK_NULL;
    pair->ngcp_counted          = false;
    memset(pair->ngcp_remoteBuffer, '\0', sizeof(pair->ngcp_remoteBuffer));
    memset(pair->ngcp_clientBuffer, '\0', sizeof(pair->ngcp_clientBuffer));

    result = ngiRlockInitialize(&pair->ngcp_lock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't initialize the lock.\n");
        goto error;
    }

    nResult = ngcclGTcontextConnectionInc(context);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't update counter.\n");
        goto error;
    }
    pair->ngcp_counted = true;
    pair->ngcp_remoteConnection = remoteConnection;

    return pair;
error:

    nResult = ngcclCommunicationPairDestroy(pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy communication pair.\n");
    }
    return NULL;
}

/**
 * Communication Pair: Connect to Ninf-G Client
 */
static ngemResult_t
ngcclCommunicationPairConnect(
    ngcclCommunicationPair_t *pair,
    uint16_t port, 
    bool tcpNodelay)
{
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_FAILED;
    ngLog_t *log = NULL;
    char *contactString = NULL;
    int result;
    NGEM_FNAME(ngcclCommunicationPairConnect);

    log = ngemLogGetDefault();

    /* Connect Ninf-G Client */
    contactString = ngemStrdupPrintf("localhost:%u", (unsigned int)port);
    if (contactString == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create contactString.\n");
        goto finalize;
    }

    nResult = ngcpGlobusXIOconnect(
        &pair->ngcp_clientConnection, contactString,
        NGCP_COMMUNICATION_SECURITY_NONE, NGCP_COMMUNICATION_AUTH_NONE,
        tcpNodelay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't connect to \"%s\".\n", contactString);
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (contactString != NULL) {
        result = ngiFree(contactString, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't free string.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}

/**
 * Communication Pair: Join handle to communication pair.
 */
static ngemResult_t
ngcclCommunicationPairJoin(
    ngcclCommunicationPair_t *pair,
    globus_xio_handle_t handle)
{
    ngLog_t *log = NULL;
    NGEM_FNAME_TAG(ngcclCommunicationPairJoin);

    log = ngemLogGetDefault();

    pair->ngcp_clientConnection = handle;

    return NGEM_SUCCESS;
}

/**
 * Communication Pair: Start to relay communication.
 */
static ngemResult_t
ngcclCommunicationPairRelayStart(
    ngcclCommunicationPair_t *pair)
{
    globus_result_t gResult;
    ngLog_t *log;
    NGEM_FNAME(ngcclCommunicationPairRelayStart);

    log = ngemLogGetDefault();

    gResult = globus_xio_register_read(
        pair->ngcp_remoteConnection, pair->ngcp_remoteBuffer,
        NGCCL_BUFFER_SIZE, 1, NULL, ngcclReadCallback, pair);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        return NGEM_FAILED;
    }

    gResult = globus_xio_register_read(
        pair->ngcp_clientConnection, pair->ngcp_clientBuffer,
        NGCCL_BUFFER_SIZE, 1, NULL, ngcclReadCallback, pair);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Communication Pair: Destroy
 */
static ngemResult_t
ngcclCommunicationPairDestroy(
    ngcclCommunicationPair_t *pair)
{
    int result;
    ngemResult_t nResult;
    globus_result_t gResult;
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    bool locked = false;
    NGEM_FNAME(ngcclCommunicationPairDestroy);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called\n");

    if (pair == NULL) {
        /* Do nothing */
        return ret;
    }

    if (!NGI_RLOCK_IS_NULL(&pair->ngcp_lock)) {
        result = ngiRlockLock(&pair->ngcp_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't lock the communication pair.\n");
            ret = NGEM_FAILED;
        } else {
            locked = true;
        }
    }
    pair->ngcp_destroying = true;
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&pair->ngcp_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the communication pair.\n");
            ret = NGEM_FAILED;
        }
    }

    if (pair->ngcp_clientConnection != NULL) {
        gResult = globus_xio_close(pair->ngcp_clientConnection, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
            ret = NGEM_FAILED;
        }
        pair->ngcp_clientConnection = NULL;
    }

    if (pair->ngcp_remoteConnection != NULL) {
        gResult = globus_xio_close(pair->ngcp_remoteConnection, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
            ret = NGEM_FAILED;
        }
        pair->ngcp_remoteConnection = NULL;
    }

    if (pair->ngcp_counted) {
        nResult = ngcclGTcontextConnectionDec(pair->ngcp_context);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't update counter.\n");
            ret = NGEM_FAILED;
        }
        pair->ngcp_counted = false;
    }

    result = ngiRlockFinalize(&pair->ngcp_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize the lock.\n");
        ret = NGEM_FAILED;
    }

    pair->ngcp_remoteConnection = NULL;
    pair->ngcp_clientConnection = NULL;
    pair->ngcp_context          = NULL;
    pair->ngcp_destroying       = true;
    pair->ngcp_lock             = NGI_RLOCK_NULL;
    pair->ngcp_counted          = false;
    memset(pair->ngcp_remoteBuffer, '\0', sizeof(pair->ngcp_remoteBuffer));
    memset(pair->ngcp_clientBuffer, '\0', sizeof(pair->ngcp_clientBuffer));

    result = NGI_DEALLOCATE(ngcclCommunicationPair_t, pair, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't deallocate storage for communication pair.\n");
    }

    return ret;
}

/**
 * Communication Pair: Is destroying?
 */
static bool
ngcclCommunicationPairIsDestroying(
    ngcclCommunicationPair_t *pair)
{
    bool tmp;
    bool locked = false;
    int result;
    ngLog_t *log;
    NGEM_FNAME(ngcclCommunicationPairIsDestroying);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&pair->ngcp_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the communication pair.\n");
    } else {
        locked = true;
    }
    tmp = pair->ngcp_destroying;
    result = ngiRlockUnlock(&pair->ngcp_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't unlock the communication pair.\n");
    }

    return tmp;
}

/**
 * Callback for Client Relay Handling
 */
static void *
ngcclRelayHandlingCallback(
    void *userData)
{
    ngcclRelayHandlingCallbackArgument_t *cArg = userData;
    ngccRelayHandler_t *relayHandler = NULL;
    ngccGTcontext_t *context;
    ngccPrepareCommunicationOptions_t *pCommOpts = NULL;
    int result;
    ngcpOptions_t *opts = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    ngcclRelayHandlerSendCRcallbackArgument_t *oa = NULL;
    ngemResult_t ret = NGEM_FAILED;
    char *message = NULL;
    ngccPrepareCommunicationResult_t res = {NGEM_FAILED, NULL, NULL, false};
    NGEM_FNAME(ngcclRelayHandlingCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "Called with (ngcclRelayHandlingCallbackArgument_t *)(%p).\n", userData);

    context      = cArg->ngra_context;
    pCommOpts    = cArg->ngra_pCommOpts;
    relayHandler = cArg->ngra_relayHandler;
    opts         = NGCC_PREPARE_COMMUNICATION_OPTION_USERDATA(pCommOpts);

    nResult = ngccRelayHandlerPrepareCommunication(
        relayHandler, pCommOpts, &res);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't prepare communication on Client Relay.";
        goto finalize;
    }

    if (res.ngpcr_canceled) {
        /* Do nothing */
        goto finalize2;
    }

    /* Set Info */
    if (res.ngpcr_result == NGEM_SUCCESS) {
        nResult = ngemNotifyAddOption(
            NGCC_PREPARE_COMMUNICATION_OPTION_NOTIFY(pCommOpts),
            NGCP_OPTION_CONTACT_STRING, "%s", res.ngpcr_contactString);
        if (nResult != NGEM_SUCCESS) {
            message = "Can't add contact string.";
            goto finalize;
        }
    } else {
        message = res.ngpcr_message;
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (ret != NGEM_SUCCESS) {
        message = (message != NULL)?message:"Unknown error";
        ngLogError(log, NGCC_LOGCAT_GT, fName, "%s\n", message);
        nResult = ngccPrepareCommunicationOptionsSetError(pCommOpts, message);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't set error to COMMUNICATION_REPLY notify.\n");
        }
    }
    /* finalize res */
    ngiFree(res.ngpcr_message, log, NULL);
    ngiFree(res.ngpcr_contactString, log, NULL);
    res.ngpcr_message = NULL;
    res.ngpcr_contactString = NULL;

    oa = NGI_ALLOCATE(ngcclRelayHandlerSendCRcallbackArgument_t, log, NULL);
    if (oa == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't allocate storage for argument of sendCRcallback.\n");
        goto finalize2;
    }

    oa->ngra_protocol  = context->nggt_protocol;
    oa->ngra_pCommOpts = pCommOpts;

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "Register callback with (ngcclRelayHandlerSendCRcallbackArgument_t *)(%p).\n", oa);
    nResult = ngcpCallbackOneshot(
        ngcclRelayHandlerSendCRcallback, oa);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set callback for sending COMMUNICATION_REPLY notify.\n");
        ngiFree(oa, log, NULL);
        goto finalize2;
    }

finalize2:
    if (opts != NULL) {
        nResult = ngcpOptionsDestroy(opts);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't destroy prepare communication options.\n");
            ret = NGEM_FAILED;
        }
    }

    result = NGI_DEALLOCATE(ngcclRelayHandlingCallbackArgument_t, cArg, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't deallocate storage for argument of callback for handling the relay.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngccRelayHandlerUnref(relayHandler);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't unref the relay handler.\n");
        ret = NGEM_FAILED;
    }

    return NULL;
}

/**
 * Relay Handler: Callback function for sending COMMUNICATION_REPLY
 */
static void
ngcclRelayHandlerSendCRcallback(
    void *userData,
    ngemCallbackResult_t cResult)
{
    ngcclRelayHandlerSendCRcallbackArgument_t *oa = userData;
    ngLog_t *log;
    ngemResult_t nResult;
    int result;
    NGEM_FNAME(ngcclRelayHandlerSendCRcallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "Called with (ngcclRelayHandlerSendCRcallbackArgument_t *)(%p).\n", userData);

    if ((cResult == NGEM_CALLBACK_RESULT_CANCEL) ||
        (!oa->ngra_protocol->ngp_protocol->ngp_available)) {
        ngLogWarn(log, NGCC_LOGCAT_GT, fName,
            "Receiving COMMUNICATION_REPLY from client relay,"
            " but can't send it to Ninf-G Client.\n");
        goto finalize;
    }

    nResult = ngccProtocolSendCommunicationReplyNotify(oa->ngra_protocol, oa->ngra_pCommOpts);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send COMMUNICATION_REPLY notify.\n");
    }

finalize:
    result = NGI_DEALLOCATE(ngcclRelayHandlerSendCRcallbackArgument_t, oa, log, NULL);
    if (result == 0) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't deallocate the argument for callback.\n");
    }

    return;
}

/**
 * Relay Handler: Callback function for sending CONNECT_REQUEST_NP
 */
static void
ngcclRelayHandlerSendConnectRequestCallback(
    void *userData,
    ngemCallbackResult_t cResult)
{
    ngccGTlistener_t *listener = userData;
    ngemProtocol_t *protocol;
    ngLog_t *log;
    ngemNotify_t *notify = NULL;
    ngemResult_t nResult;
    int result;
    ngcclCommunicationPair_t *pair = NULL;
    bool locked = false;
    NGEM_FNAME(ngcclRelayHandlerSendConnectRequestCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName,
        "Called with (ngccGTlistener_t*)(%p).\n", userData);

    if (cResult == NGEM_CALLBACK_RESULT_CANCEL) {
        /* Canceled */
        return;
    }

    NGEM_ASSERT(listener != NULL);
    NGEM_ASSERT(listener->ngl_context != NULL);
    NGEM_ASSERT(listener->ngl_context->nggt_protocol != NULL);
    NGEM_ASSERT(listener->ngl_context->nggt_protocol->ngp_protocol != NULL);
    protocol = listener->ngl_context->nggt_protocol->ngp_protocol;

    notify = ngemNotifyCreate("CONNECT_REQUEST_NP", true);
    if (notify == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create the notify.\n");
        goto error;
    }

    nResult = 
        ngemNotifyAddOption(notify, NGCP_OPTION_CONTACT_STRING, "%s",
            listener->ngl_contactString) &&
        ngemNotifyAddOption(notify, NGCP_OPTION_COMMUNICATION_SECURITY, "%s",
            ngcpCommunicationSecurityString[listener->ngl_security]) &&
        ngemNotifyAddOption(notify, "tcp_nodelay", "%s",
            listener->ngl_tcpNodelay?"true":"false");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't set option to the notify.\n");
        goto error;
    }

    nResult = ngemProtocolSendNotify(protocol, &notify);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't send the notify.\n");
        goto error;
    }
    NGEM_ASSERT(notify == NULL);
    return;
error:
    ngemNotifyDestroy(notify);

    result = ngiRlockLock(&listener->ngl_lock, log, NULL);
    if (result == 0) {
        ngLogFatal(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the listener.\n");
    } else {
        locked = true;
    }

    /* Erase PAIR */
    pair = NGEM_LIST_TAIL(ngcclCommunicationPair_t, &listener->ngl_pairQueue);
    if (pair == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "There are no connection from remote site.\n");
    } else {
        nResult = NGEM_LIST_ERASE_BY_ADDRESS(ngcclCommunicationPair_t, &listener->ngl_pairQueue, pair);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't erase item from the list.\n");
        } else {
            nResult = ngcclCommunicationPairDestroy(pair);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't destroy the communication pair.\n");
            }
        }
    }

    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&listener->ngl_lock, log, NULL);
        if (result == 0) {
            ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the listener.\n");
        }
    }

    return;
}

/**
 * Context: Callback function on receiving CONNECT_REQUEST_NP
 */
static void
ngcclGTcontextOnConnectRequest(
    void *userData,
    const char *contactString,
    ngcpCommunicationSecurity_t sec,
    bool self,
    bool tcpNodelay)
{
    globus_xio_handle_t handle = NULL;
    ngccGTcontext_t *context = userData;
    ngLog_t *log;
    ngemResult_t nResult;
    ngcclCommunicationPair_t *pair = NULL;
    globus_result_t gResult;
    unsigned int auth;
    NGEM_FNAME(ngcclGTcontextOnConnectRequest);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCC_LOGCAT_GT, fName, "Called with %p\n", userData);

    auth = NGCP_COMMUNICATION_AUTH_HOST;
    if (self) {
        auth = NGCP_COMMUNICATION_AUTH_HOST|NGCP_COMMUNICATION_AUTH_SELF;
    }

    /* Connect to Client Relay */
    nResult = ngcpGlobusXIOconnect(
        &handle, contactString, sec, auth, tcpNodelay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't connect to \"%s\".\n", contactString);
        goto error;
    }

    pair = ngcclCommunicationPairCreate(NULL, context);
    if (pair == NULL) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create communication pair.\n");
        goto error;
    }
    pair->ngcp_remoteConnection = handle;
    handle = NULL;

    nResult = ngcclCommunicationPairConnect(pair,
        context->nggt_protocol->ngp_initOpts->ngio_clientPort, tcpNodelay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't connect to Ninf-G Client.\n");
        goto error;
    }

    nResult = ngcclCommunicationPairRelayStart(pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't start relay communication.\n");
        goto error;
    }
    return;

error:
    nResult = ngcclCommunicationPairDestroy(pair);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't destroy the Communication pair.\n");
    }
    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
    }

    return;
}

/**
 * GT listener: Zero clear 
 */
static void
ngcclGTlistenerZeroClear(
    ngccGTlistener_t *listener)
{
    NGEM_ASSERT(listener != NULL);

    listener->ngl_context         = NULL;
    listener->ngl_type            = NGCC_GT_FOR_REMOTE;
    listener->ngl_handle          = NULL;
    listener->ngl_security        = NGCP_COMMUNICATION_SECURITY_NONE;
    listener->ngl_contactString   = NULL;
    listener->ngl_callbackRunning = false;
    listener->ngl_lock            = NGI_RLOCK_NULL;
    NGEM_LIST_SET_INVALID_VALUE(&listener->ngl_pairQueue);

    return;
}

/**
 * GT listener: Initialize
 */
static ngemResult_t
ngcclGTlistenerInitialize(
    ngccGTlistener_t *listener, 
    ngccGTcontext_t *context)
{
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcclGTlistenerInitialize);

    NGEM_ASSERT(listener != NULL);
    NGEM_ASSERT(context != NULL);

    log = ngemLogGetDefault();

    ngcclGTlistenerZeroClear(listener);

    listener->ngl_context         = context;
    NGEM_LIST_INITIALIZE(ngcclCommunicationPair_t, &listener->ngl_pairQueue);

    nResult = ngiRlockInitialize(&listener->ngl_lock, NULL, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCC_LOGCAT_GT, fName,
            "Can't initialize the lock for the listener.\n");
        goto finalize;
    }

    return NGEM_SUCCESS;
finalize:
    ngcclGTlistenerZeroClear(listener);
    return NGEM_FAILED;
}

/**
 * GT listener: Start to listen.
 */
static ngemResult_t
ngcclGTlistenerListen(
    ngccGTlistener_t *listener,
    ngccGTserverType_t sType,
    ngcpCommunicationSecurity_t sec,
    bool tcpNodelay)
{
    ngemResult_t nResult;
    globus_result_t gResult;
    ngemResult_t ret = NGEM_FAILED;
    ngLog_t *log;
    bool locked = false;
    NGEM_FNAME(ngcclGTlistenerListen);

    NGEM_ASSERT(listener != NULL);

    log = ngemLogGetDefault();
    
    nResult = ngiRlockLock(&listener->ngl_lock, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the listener.\n");
        goto finalize;
    }
    locked = true;

    listener->ngl_type       = sType;
    listener->ngl_security   = sec;
    listener->ngl_tcpNodelay = tcpNodelay;

    nResult = ngcpGlobusXIOcreateListener(&listener->ngl_handle,
        listener->ngl_context->nggt_portRange, sec, tcpNodelay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCC_LOGCAT_GT, fName,
            "Can't create a new listener.\n");
        goto finalize;
    }

    gResult = globus_xio_server_get_contact_string(
        listener->ngl_handle, &listener->ngl_contactString);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_server_get_contact_string", gResult);
        goto finalize;
    }

    gResult = globus_xio_server_register_accept(
        listener->ngl_handle, ngcclGTlistenerAcceptCallback, listener);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
            "globus_xio_server_register_accept", gResult);
        goto finalize;
    }
    listener->ngl_callbackRunning = true;

    ret = NGEM_SUCCESS;
finalize:
    if (ret != NGEM_SUCCESS) {
        nResult = ngcclGTlistenerClose(listener);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                "Can't close the listener.\n");
            ret = NGEM_FAILED;
        }
    }

    if (locked) {
        locked = false;
        nResult = ngiRlockUnlock(&listener->ngl_lock, log, NULL);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the listener.\n");
            ret = NGEM_FAILED;
        }
    }
    return ret;
}


/**
 * GT listener: Close(stop callback)
 */
static ngemResult_t
ngcclGTlistenerClose(
    ngccGTlistener_t *listener)
{
    ngemResult_t nResult;
    globus_result_t gResult;
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    ngcclCommunicationPair_t *pair;
    bool locked = false;
    NGEM_FNAME(ngcclGTlistenerClose);

    NGEM_ASSERT(listener != NULL);

    log = ngemLogGetDefault();

    nResult = ngiRlockLock(&listener->ngl_lock, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCC_LOGCAT_GT, fName,
            "Can't lock the listener.\n");
        ret = NGEM_FAILED;
    }
    locked = true;

    if (listener->ngl_handle != NULL) {
        gResult = globus_xio_server_register_close(
            listener->ngl_handle, ngcclServerCloseCallback, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGCC_LOGCAT_GT, fName,
                "globus_xio_server_close", gResult);
            ret = NGEM_FAILED;
        }
        listener->ngl_handle = NULL;

        while (!NGEM_LIST_IS_EMPTY(ngcclCommunicationPair_t, &listener->ngl_pairQueue)) {
            pair = NGEM_LIST_HEAD(ngcclCommunicationPair_t, &listener->ngl_pairQueue);
            nResult = ngcclCommunicationPairDestroy(pair);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't destroy communication pair.\n");
                ret = NGEM_FAILED;
            }
            nResult = NGEM_LIST_ERASE_BY_ADDRESS(ngcclCommunicationPair_t, &listener->ngl_pairQueue, pair);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGCC_LOGCAT_GT, fName,
                    "Can't erase item from the list.\n");
                ret = NGEM_FAILED;
            }
        }
    }

    NGEM_ASSERT(NGEM_LIST_IS_EMPTY(ngcclCommunicationPair_t, &listener->ngl_pairQueue));

    if (locked) {
        locked = false;
        nResult = ngiRlockUnlock(&listener->ngl_lock, log, NULL);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                "Can't unlock the listener.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * GT listener: Finalize
 * WARNING: Context must be locked, when this function is called.
 */
static ngemResult_t
ngcclGTlistenerFinalize(
    ngccGTlistener_t *listener)
{
    ngemResult_t nResult;
    int result;
    ngLog_t *log;
    ngemResult_t ret = NGEM_SUCCESS;
    bool locked = false;
    NGEM_FNAME(ngcclGTlistenerFinalize);

    log = ngemLogGetDefault();

    NGEM_ASSERT(listener != NULL);

    if (listener->ngl_context != NULL) {
        nResult = ngiRlockLock(&listener->ngl_lock, log, NULL);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                "Can't lock the listener.\n");
            ret = NGEM_FAILED;
        } else {
            locked = true;
        }

        nResult = ngcclGTlistenerClose(listener);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCC_LOGCAT_GT, fName,
                "Can't close the listener.\n");
            ret = NGEM_FAILED;
        }

        while (listener->ngl_callbackRunning) {
            result = ngiRlockWait(&listener->ngl_lock, log, NULL);
            if (result == 0) {
                ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                    "Can't wait lock for the listener.\n");
                break;
            }
        }

        if (locked) {
            locked = false;
            nResult = ngiRlockUnlock(&listener->ngl_lock, log, NULL);
            if (nResult != NGEM_SUCCESS) {
                ngLogFatal(log, NGCC_LOGCAT_GT, fName,
                    "Can't unlock the listener.\n");
                ret = NGEM_FAILED;
            }
        }
    }

    if (listener->ngl_contactString != NULL) {
        globus_libc_free(listener->ngl_contactString);
    }

    nResult = ngiRlockFinalize(&listener->ngl_lock, log, NULL);
    if (nResult != NGEM_SUCCESS) {
        ngLogFatal(log, NGCC_LOGCAT_GT, fName,
            "Can't finalize the lock.\n");
        ret = NGEM_FAILED;
    }

    NGEM_LIST_FINALIZE(ngcclCommunicationPair_t, &listener->ngl_pairQueue);

    ngcclGTlistenerZeroClear(listener);

    return ret;
}

/**
 * Relay Handler: Callback for canceling
 */
static void
ngcclServerCloseCallback(
    globus_xio_server_t server,
    void *userData)
{
    /* Do nothing */
    return;
}

