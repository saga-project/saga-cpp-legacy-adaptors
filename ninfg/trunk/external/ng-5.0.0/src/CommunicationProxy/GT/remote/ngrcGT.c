/*
 * $RCSfile: ngrcGT.c,v $ $Revision: 1.11 $ $Date: 2008/03/27 13:42:10 $
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

#include "ngemEnvironment.h"
#include "ngcpXIO.h"
#include "ngcpOptions.h"
#include "ngrcProtocol.h"

#include "ngrcGT.h"

NGI_RCSID_EMBED("$RCSfile: ngrcGT.c,v $ $Revision: 1.11 $ $Date: 2008/03/27 13:42:10 $")

#define NGRC_LOGCAT_GT "Remote Communication Proxy GT"

/**
 * Relay
 */
typedef struct ngrclRelay_s {
    ngiRlock_t                 ngr_lock;
    globus_thread_t            ngr_listenThread;
    bool                       ngr_listenThreadAlive;
    ngrcSocket_t              *ngr_listener;
    globus_xio_server_t        ngr_GTlistener;
    bool                       ngr_tcpNodelay;
    bool                       ngr_requestStop;
    ngcpOptions_t             *ngr_options;
    int                        ngr_nWorkThread;
    bool                       ngr_remoteRelay;
    ngrcRelayHandler_t        *ngr_relayHandler;
    char                      *ngr_clientRelayHost;
    bool                       ngr_clientRelayCrypt;
} ngrclRelay_t;

/**
 * Argument for worker
 */
typedef struct ngrclThreadArgument_s {
    ngrclRelay_t        *ngta_relay;
    ngrcSocket_t        *ngta_executable;
    ngrcOperator_t      *ngta_rcp;
    ngrcOperator_t      *ngta_client;
} ngrclThreadArgument_t;

/* Functions */
static ngemResult_t ngrclProtocolProcess(void);

static ngemResult_t ngrclInitializeBegin(
    ngrcProtocol_t *, ngemOptionAnalyzer_t *);
static ngemResult_t ngrclInitializeEnd(
    ngrcProtocol_t *, ngrcInitializeOptions_t *, char **, ngemResult_t);

/* Relay */
static ngrclRelay_t *ngrclRelayCreate(void);
static ngemResult_t ngrclRelayDestroy(ngrclRelay_t *);
static ngemResult_t ngrclRelayStartListen(ngrclRelay_t *, char **);
static ngemResult_t ngrclRelayStop(ngrclRelay_t *);

static void *ngrclRelayListenThread(void *);
static void *ngrclRelayUpThread(void *);
static void *ngrclRelayDownThread(void *);

static ngemResult_t ngrclRelayThreadCountUp(ngrclRelay_t *);
static ngemResult_t ngrclRelayThreadCountDown(ngrclRelay_t *);
static ngemResult_t ngrclRelayThreadCountWait(ngrclRelay_t *, int);

static ngemResult_t ngrclRelayOneCommunication(ngrclRelay_t *, bool *);

#define NGRCL_RELAY_INFO(relay) (&((relay)->ngr_info))

/* Worker thread argument */
void ngrclThreadArgumentZeroClear(ngrclThreadArgument_t *);

/* Protocol Action */
static ngrcProtocolActions_t ngrclActions = {
    ngrclInitializeBegin,
    ngrclInitializeEnd
};

/**
 * main
 */
int
main(int argc, char *argv[])
{
    bool debug = false;
    char *logFileName = NULL;
    int opt;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    int ret = 0;
    char *string;
    NGEM_FNAME(main);

    string = NGCP_OPTION_COMMUNICATION_SECURITY;

    /* Options analyze */
    while ((opt = getopt(argc, argv, "l:d")) >= 0) {
        switch (opt) {
        case 'l':
            /* LOG */
            logFileName = optarg;
            break;
        case 'd':
            debug = true;
            break;
        case '?':
        default:
            /* Ignore arguments */
            ;
        }
    }

    /* Log */
    nResult = ngemLogInitialize("Remote Communication Proxy GT", logFileName);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't initialize log.\n");
        ret = 1;
        goto finalize;
    }
    log = ngemLogGetDefault();

    nResult = ngrclProtocolProcess();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't communication to Client Communication Proxy.\n");
        ret = 1;
        goto finalize;
    }
    
finalize:
    nResult = ngemLogFinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't finalize log.\n");
        ret = 1;
    }

    return ret;
}

/**
 * Processing protocol with Ninf-G Executable
 */
static ngemResult_t
ngrclProtocolProcess(void)
{
    ngrclRelay_t *relay = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    ngrcProtocol_t *protocol = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngrclProtocolProcess);

    log = ngemLogGetDefault();

    /* Initialize Globus XIO */
    nResult = ngcpGlobusXIOinitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't initialize Globus XIO.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* Callback Manager */
    nResult = ngemCallbackManagerInitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't initialize the callback manager.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    relay = ngrclRelayCreate();
    if (relay == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create the relay for communication.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* Protocol */
    protocol = ngrcProtocolCreate(&ngrclActions, relay);
    if (protocol == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create the remote communication protocol.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }

    /* Run */
    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "Enters the event loop.\n");
    nResult = ngemCallbackManagerRun();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Error is occurred in communicating to Ninf-G Executable.\n");
        ret = NGEM_FAILED;
        goto finalize;
    }
    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "Leaves the event loop.\n");

    /* Wait Listener Thread */
    nResult = ngrclRelayStop(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't stop the listener thread.\n");
        ret = 1;
        goto finalize;
    }

finalize:
    nResult = ngrcProtocolDestroy(protocol);
    if (protocol == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the remote communication protocol.\n");
        ret = NGEM_FAILED;
    }

    if (relay != NULL) {
        nResult = ngrclRelayDestroy(relay);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't destroy the relay for communication.\n");
            ret = 1;
        }
    }


    nResult = ngemCallbackManagerFinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't finalize the callback manager.\n");
        ret = NGEM_FAILED;
    }

    /* Initialize Globus XIO */
    nResult = ngcpGlobusXIOfinalize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't finalize Globus XIO.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Callback on beginning INITIALIZE request.
 */
static ngemResult_t 
ngrclInitializeBegin(
    ngrcProtocol_t *protocol,
    ngemOptionAnalyzer_t *analyzer)
{
    ngcpOptions_t *opts = NULL;
    ngrclRelay_t *relay = NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngrclInitializeBegin);

    log = ngemLogGetDefault();

    relay = protocol->ngp_userData;
    opts = relay->ngr_options;

    nResult = 
        NGEM_OPTION_ANALYZER_SET_ACTION(
            bool, analyzer,
            NGRC_OPTION_REMOTE_RELAY,
            ngemOptionAnalyzerSetBool,
            &relay->ngr_remoteRelay, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            char *, analyzer,
            NGCP_OPTION_CONTACT_STRING,
            ngemOptionAnalyzerSetString,
            &opts->ngo_contactString, 1, 1)
        &&
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
            &relay->ngr_clientRelayHost, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_CLIENT_RELAY_INVOKE_METHOD,
            ngemOptionAnalyzerSetIgnore, NULL, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_CLIENT_RELAY_OPTION,
            ngemOptionAnalyzerSetIgnore, NULL, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            bool, analyzer,
            NGCP_OPTION_CLIENT_RELAY_CRYPT,
            ngemOptionAnalyzerSetBool,
            &relay->ngr_clientRelayCrypt, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_CLIENT_RELAY_GSISSH_COMMAND,
            ngemOptionAnalyzerSetIgnore,
            NULL, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            void, analyzer,
            NGCP_OPTION_CLIENT_RELAY_GSISSH_OPTION,
            ngemOptionAnalyzerSetIgnore, NULL, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            char *, analyzer,
            NGCP_OPTION_REMOTE_RELAY_HOST,
            ngemOptionAnalyzerSetString,
            &opts->ngo_relayHost, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            ngcpRelayInvokeMethod_t, analyzer,
            NGCP_OPTION_REMOTE_RELAY_INVOKE_METHOD,
            ngcpOptionAnalyzerSetRelayInvokeMethod,
            &opts->ngo_relayInvokeMethod,  0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            NGEM_LIST_OF(char),  analyzer,
            NGCP_OPTION_REMOTE_RELAY_OPTION,
            ngemOptionAnalyzerSetStringList,
            &opts->ngo_relayOptions, 0, -1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            bool, analyzer,
            NGCP_OPTION_REMOTE_RELAY_CRYPT,
            ngemOptionAnalyzerSetBool,
            &opts->ngo_relayCrypt, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            char *, analyzer,
            NGCP_OPTION_REMOTE_RELAY_GSISSH_COMMAND,
            ngemOptionAnalyzerSetString,
            &opts->ngo_relayGSISSHcommand, 0, 1)
        &&
        NGEM_OPTION_ANALYZER_SET_ACTION(
            NGEM_LIST_OF(char), analyzer,
            NGCP_OPTION_REMOTE_RELAY_GSISSH_OPTION,
            ngemOptionAnalyzerSetStringList,
            &opts->ngo_relayGSISSHoptions, 0, -1);

    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't set action to the option analyzer.\n");
        return NGEM_FAILED;
    }

    return NGEM_SUCCESS;
}

/**
 * Callback on end of INITIALIZE request.
 */
static ngemResult_t 
ngrclInitializeEnd(
    ngrcProtocol_t *protocol,
    ngrcInitializeOptions_t *options,
    char **address,
    ngemResult_t result)
{
    ngLog_t *log = NULL;
    ngcpOptions_t *opts = NULL;
    ngrclRelay_t *relay = NULL;
    ngemResult_t nResult;
    char *env = NULL;
    char *listenAddress = NULL;
    NGEM_FNAME(ngrclInitializeEnd);

    log = ngemLogGetDefault();

    relay = protocol->ngp_userData;
    opts = relay->ngr_options;
    relay->ngr_tcpNodelay = options->ngio_tcpNodelay;

    if (result == NGEM_SUCCESS) {
        /* Prints options */
        if (opts->ngo_communicationSecurity != NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY) {
            env = getenv(NGCP_ENV_ONLY_PRIVATE);
            if ((env != NULL) && (strcmp(env, "0") != 0)) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Allows \"confidentiality\" communication only.\n");
                goto error;
            }
        }

        if ((opts->ngo_communicationSecurity != NGCP_COMMUNICATION_SECURITY_NONE) ||
            ((opts->ngo_relayHost != NULL) && (opts->ngo_relayCrypt))) {
            if (!ngcpCredentialAvailable()) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "The proxy credential is not available\n");
                goto error;
            }
        }

        if (opts->ngo_relayHost != NULL) {
            relay->ngr_relayHandler = ngrcRelayHandlerCreate(opts,
                options, relay->ngr_clientRelayHost,
                relay->ngr_clientRelayCrypt);
            if (relay->ngr_relayHandler == NULL) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Can't create the relay.\n");
                goto error;
            }
        }

        nResult = ngrclRelayStartListen(relay, &listenAddress);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't start to listen.\n");
            goto error;
        }
    }

    *address = listenAddress;

    return NGEM_SUCCESS;

error:
    ngiFree(listenAddress, log, NULL);

    return NGEM_FAILED;
}

/**
 * Relay: Create
 */
static ngrclRelay_t *
ngrclRelayCreate(void)
{
    ngrclRelay_t *relay = NULL;
    ngLog_t *log = NULL;
    int result;
    ngemResult_t nResult;
    NGEM_FNAME(ngrclRelayCreate);

    log = ngemLogGetDefault();

    relay = NGI_ALLOCATE(ngrclRelay_t, log, NULL);
    if (relay == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't allocate storage for the relayer.\n");
        goto error;
    }

    relay->ngr_lock              = NGI_RLOCK_NULL;
    relay->ngr_listenThreadAlive = false;
    relay->ngr_listener          = NULL;
    relay->ngr_GTlistener        = NULL;
    relay->ngr_requestStop       = false;
    relay->ngr_options           = NULL;
    relay->ngr_nWorkThread       = 0;
    relay->ngr_remoteRelay       = false;
    relay->ngr_relayHandler      = NULL;
    relay->ngr_clientRelayHost   = NULL;
    relay->ngr_clientRelayCrypt  = false;

    result = ngiRlockInitialize(&relay->ngr_lock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't initialize the lock.\n");
        goto error;
    }

    relay->ngr_options = ngcpOptionsCreate();
    if (relay->ngr_options == NULL) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't create the options.\n");
        goto error;
    }

    return relay;
    
error:
    nResult = ngrclRelayDestroy(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the relayer.\n");
    }
    relay = NULL;

    return NULL;
}

/**
 * Relay: Destroy
 */
static ngemResult_t
ngrclRelayDestroy(
    ngrclRelay_t *relay)
{
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngrclRelayDestroy);

    log = ngemLogGetDefault();

    if (relay == NULL) {
        return NGEM_SUCCESS;
    }

    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Wait the thread.\n");

    nResult = ngrclRelayStop(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't request to stop thread.\n");
        ret = NGEM_FAILED;
    }

    result = ngiRlockFinalize(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't finalize the lock.\n");
        ret = NGEM_FAILED;
    }

    nResult = ngcpOptionsDestroy(relay->ngr_options);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the options.\n");
        ret = NGEM_FAILED;
    }
    relay->ngr_options = NULL;

    nResult = ngrcSocketDestroy(relay->ngr_listener);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the listener.\n");
        ret = NGEM_FAILED;
    }
    relay->ngr_listener = NULL;

    ngiFree(relay->ngr_clientRelayHost, log, NULL);
    relay->ngr_clientRelayHost = NULL;

    relay->ngr_lock              = NGI_RLOCK_NULL;
    relay->ngr_listenThreadAlive = false;
    relay->ngr_listener          = NULL;
    relay->ngr_GTlistener        = NULL;
    relay->ngr_requestStop       = false;
    relay->ngr_options           = NULL;
    relay->ngr_nWorkThread       = 0;
    relay->ngr_remoteRelay       = false;
    relay->ngr_relayHandler      = NULL;
    relay->ngr_clientRelayHost   = NULL;
    relay->ngr_clientRelayCrypt  = false;

    result = NGI_DEALLOCATE(ngrclRelay_t, relay, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't deallocate for storage for the relay.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Relay: Start thread for listen
 * This thread wait that Ninf-G Executable connects to.
 * This thread handle 1 connection only.
 */
static ngemResult_t 
ngrclRelayStartListen(
    ngrclRelay_t *relay,
    char **address)
{
    globus_result_t gResult;
    ngemResult_t ret = NGEM_FAILED;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *contactString = NULL;
    bool locked = false;
    int result;
    ngcpPortRange_t port = {0,0};
    ngcpCommunicationAuth_t auth;
    NGEM_FNAME(ngrclRelayStartListen);

    NGEM_ASSERT(relay != NULL);
    NGEM_ASSERT(relay->ngr_options->ngo_contactString != NULL);
    NGEM_ASSERT(address != NULL);

    log = ngemLogGetDefault();

    if (relay->ngr_remoteRelay) {
        auth = NGCP_COMMUNICATION_AUTH_SELF;
        if ((getuid() == 0) && (relay->ngr_remoteRelay)) {
            auth = NGCP_COMMUNICATION_AUTH_NONE;
        }

        nResult = ngcpGlobusXIOcreateListener(&relay->ngr_GTlistener,
            port, relay->ngr_options->ngo_communicationSecurity,
            relay->ngr_tcpNodelay);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't create the listener.\n");
            goto finalize;
        }
        gResult = globus_xio_server_get_contact_string(
            relay->ngr_GTlistener, &contactString);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_xio_server_get_contact_string", gResult);
            goto finalize;
        }
        *address = contactString;
    } else {
        relay->ngr_listener = ngrcSocketCreateListener(relay->ngr_tcpNodelay);
        if (relay->ngr_listener == NULL) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't create the listener.\n");
            goto finalize;
        }

        contactString = ngrcSocketGetContactString(relay->ngr_listener);
        if (contactString == NULL) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't get contact string from socket.\n");
            goto finalize;
        }
        *address = contactString;
    }

    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        goto finalize;
    }
    locked = true;

    gResult = globus_thread_create(
        &relay->ngr_listenThread, NULL, ngrclRelayListenThread, relay);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_thread_create", gResult);
        goto finalize;
    }
    relay->ngr_listenThreadAlive = true;
    ret = NGEM_SUCCESS;

finalize:
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }

    return ret;
}

/**
 * Relay: Stop the thread for waiting to be connected.
 */
static ngemResult_t
ngrclRelayStop(
    ngrclRelay_t *relay)
{
    ngLog_t *log;
    ngemResult_t nResult;
    bool locked = false;
    ngemResult_t ret = NGEM_SUCCESS;
    int result;
    globus_result_t gResult;
    NGEM_FNAME(ngrclRelayStop);

    log = ngemLogGetDefault();

    /* Cancels Listener */
    if (relay->ngr_listener != NULL) {
        nResult = ngrcSocketCancel(relay->ngr_listener);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't cancel the listener");
            goto finalize;
        }
    }
    if (relay->ngr_GTlistener != NULL) {
        gResult = globus_xio_server_close(relay->ngr_GTlistener);
        relay->ngr_GTlistener = NULL;
        if (gResult != GLOBUS_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't close the listener");
            goto finalize;
        }
    }

    /* Waits the thread */
    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        goto finalize;
    }
    locked = true;
    while (relay->ngr_listenThreadAlive) {
        result = ngiRlockWait(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't wait the lock.\n");
            goto finalize;
        }
    }

    ret = NGEM_SUCCESS;
finalize:
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }

    return ret;
}

/**
 * Relay: Thread entry point for waiting to be connected.
 */
static void *
ngrclRelayListenThread(
    void *arg)
{
    ngrclRelay_t *relay = arg;
    ngLog_t *log;
    ngemResult_t nResult;
    bool cont = true;
    bool locked = false;
    int result;
    NGEM_FNAME(ngrclRelayListenThread);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "Enter.\n");

    while (cont) {
        nResult = ngrclRelayOneCommunication(
            relay, &cont);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't process the communication.\n");
            /* Through */
        }
    }

    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
    } else {
        locked = true;
    }
    relay->ngr_listenThreadAlive = false;
    result = ngiRlockBroadcast(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
    }
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
        }
        locked = false;
    }
    ngLogDebug(log, NGRC_LOGCAT_GT, fName,
        "Leave.\n");

    return NULL;
}

/**
 * Relay: Handling one connection pair.
 */
static ngemResult_t
ngrclRelayOneCommunication(
    ngrclRelay_t *relay,
    bool *cont)
{
    ngrcSocket_t *sock = NULL;
    ngrcSocket_t *sockCopy = NULL;
    ngrclThreadArgument_t up;
    ngrclThreadArgument_t down;
    globus_result_t gResult;
    ngLog_t *log;
    ngemResult_t nResult;
    globus_xio_handle_t handle = NULL;
    globus_xio_handle_t rcpHandle = NULL;
    globus_thread_t upThread;
    globus_thread_t downThread;
    ngemResult_t ret = NGEM_FAILED;
    ngrcOperator_t op;
    bool opInitialized = false;
    ngrcOperator_t rcpOp;
    bool rcpOpInitialized = false;
    bool canceled;
    char *address = NULL;
    bool ok;
    unsigned int auth;
    NGEM_FNAME(ngrclRelayOneCommunication);

    log = ngemLogGetDefault();

    NGEM_ASSERT(relay != NULL);
    NGEM_ASSERT(cont  != NULL);
    NGEM_ASSERT(relay->ngr_nWorkThread == 0);

    *cont = true;

    ngrclThreadArgumentZeroClear(&up);
    ngrclThreadArgumentZeroClear(&down);

    if (relay->ngr_remoteRelay) {
        /* Connection from Remote Communication Proxy */

        NGEM_ASSERT(relay->ngr_GTlistener != NULL);

        gResult = globus_xio_server_accept(&rcpHandle, relay->ngr_GTlistener);
        if (gResult != GLOBUS_SUCCESS) {
            if (globus_xio_error_is_canceled(gResult) == GLOBUS_TRUE) {
                ngLogInfo(log, NGRC_LOGCAT_GT, fName,
                    "Accept operation is canceled.\n");
                ret = NGEM_SUCCESS;
                *cont = false;
                goto finalize;
            } else {
                ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                    "globus_xio_server_accept", gResult);
                goto finalize;
            }
        }

        gResult = globus_xio_open(rcpHandle, NULL, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            if (globus_xio_error_is_canceled(gResult) == GLOBUS_TRUE) {
                ngLogInfo(log, NGRC_LOGCAT_GT, fName,
                    "Open operation is canceled.\n");
                ret = NGEM_SUCCESS;
                *cont = false;
                goto finalize;
            } else {
                ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                    "globus_xio_open", gResult);
                goto finalize;
            }
        }

        if (relay->ngr_options->ngo_communicationSecurity != NGCP_COMMUNICATION_SECURITY_NONE) {
            nResult = ngcpGlobusXIOcheckPeerName(rcpHandle, NGCP_COMMUNICATION_AUTH_SELF, &ok);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Can't check peer name.\n");
                goto finalize;
            }
            if (!ok) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Peer name is not authorized.\n");
                goto finalize;
            }
        }

        nResult = ngrcOperatorInitialize(&rcpOp, rcpHandle);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't initialize operator.\n");
            goto finalize;
        }
        rcpOpInitialized = true;
    } else {
        NGEM_ASSERT(relay->ngr_listener != NULL);
        sock = ngrcSocketAccept(relay->ngr_listener, &canceled);
        if (sock == NULL) {
            *cont = false;
            if (canceled) {
                ngLogInfo(log, NGRC_LOGCAT_GT, fName,
                    "Accept operation is canceled.\n");
                ret = NGEM_SUCCESS;
                goto finalize;
            } else {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Failed to accept a connection.\n");
                goto finalize;
            }
        }

        sockCopy = ngrcSocketDup(sock);
        if (sockCopy == NULL) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't duplicate the socket.\n");
            goto finalize;
        }
    }

    auth = NGCP_COMMUNICATION_AUTH_SELF;
    address = relay->ngr_options->ngo_contactString;
    if (relay->ngr_relayHandler != NULL) {
        /* Connect to Remote Relay */
        NGEM_ASSERT(relay->ngr_relayHandler->ngrh_address != NULL);
        address = relay->ngr_relayHandler->ngrh_address;
        if (!relay->ngr_relayHandler->ngrh_relayHandler->ngrh_crypt) {
            auth |= NGCP_COMMUNICATION_AUTH_HOST;
        }
    }
    if ((relay->ngr_clientRelayHost != NULL) && (!relay->ngr_clientRelayCrypt)) {
        /* Connect to Client Relay(Non-crypt) */
        auth |= NGCP_COMMUNICATION_AUTH_HOST;
    }

    nResult = ngcpGlobusXIOconnect(
        &handle, address,
        relay->ngr_options->ngo_communicationSecurity,
        NGCP_COMMUNICATION_AUTH_SELF | NGCP_COMMUNICATION_AUTH_HOST,
        relay->ngr_tcpNodelay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't connect to the Client Communication Proxy.\n");
        goto finalize;
    }

    nResult = ngrcOperatorInitialize(&op, handle);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't initialize operator.\n");
        goto finalize;
    }
    opInitialized = true;

    up.ngta_relay      = relay;
    up.ngta_client     = &op;
    if (rcpOpInitialized) {
        up.ngta_rcp = &rcpOp;
    }
    up.ngta_executable = sock;

    gResult = globus_thread_create(
        &upThread, NULL, ngrclRelayUpThread, &up);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_thread_create", gResult);
        goto finalize;
    }

    nResult = ngrclRelayThreadCountUp(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't count up number of worker threads.\n");
    }

    down.ngta_relay      = relay;
    down.ngta_client     = &op;
    if (rcpOpInitialized) {
        down.ngta_rcp = &rcpOp;
    }
    down.ngta_executable = sockCopy;

    gResult = globus_thread_create(
        &downThread, NULL, ngrclRelayDownThread, &down);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
            "globus_thread_create", gResult);
        goto finalize;
    }

    nResult = ngrclRelayThreadCountUp(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't count up number of worker threads.\n");
    }

    ret = NGEM_SUCCESS;
finalize:

    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Waiting one thread.\n");
    /* Wait that one*/
    nResult = ngrclRelayThreadCountWait(relay, 1);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't wait worker threads.\n");
    }

    if (opInitialized) {
        nResult = ngrcOperatorCancel(&op);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't cancel the I/O operation.\n");
            ret = NGEM_FAILED;
        }
    }
    if (sock != NULL) {
        nResult = ngrcSocketCancel(sock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't cancel the socket.\n");
            ret = NGEM_FAILED;
        }
    }
    if (rcpOpInitialized) {
        nResult = ngrcOperatorCancel(&rcpOp);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't cancel the I/O operation.\n");
            ret = NGEM_FAILED;
        }
    }
    if (sockCopy != NULL) {
        nResult = ngrcSocketCancel(sockCopy);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't cancel the socket.\n");
            ret = NGEM_FAILED;
        }
    }
    /* wait thread end */
    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Waiting all thread.\n");
    nResult = ngrclRelayThreadCountWait(relay, 0);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't wait worker threads.\n");
    }

    /* Finalize */
    if (opInitialized) {
        nResult =  ngrcOperatorFinalize(&op);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't finalize the operator.\n");
            ret = NGEM_FAILED;
        }
        opInitialized = false;
    }

    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_xio_handle_close", gResult);
            ret = NGEM_FAILED;
        }
        handle = NULL;
    }

    /* Finalize */
    if (rcpOpInitialized) {
        nResult =  ngrcOperatorFinalize(&rcpOp);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't finalize the operator.\n");
            ret = NGEM_FAILED;
        }
        rcpOpInitialized = false;
    }

    if (rcpHandle != NULL) {
        gResult = globus_xio_close(rcpHandle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRC_LOGCAT_GT, fName,
                "globus_xio_handle_close", gResult);
            ret = NGEM_FAILED;
        }
        rcpHandle = NULL;
    }

    nResult = ngrcSocketDestroy(sock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the socket.\n");
        ret = NGEM_FAILED;
    }
    sock = NULL;

    nResult = ngrcSocketDestroy(sockCopy);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't destroy the socket.\n");
        ret = NGEM_FAILED;
    }
    sockCopy = NULL;

    return ret;
}


/**
 * Relay data from Ninf-G Executable to Ninf-G Client(Up).
 */
static void *
ngrclRelayUpThread(
    void *arg)
{
    ngrclThreadArgument_t *ta = arg;
    size_t nread;
    size_t nwrite;
    unsigned char buf[NGRC_BUFFER_SIZE];
    ngLog_t *log;
    ngemResult_t nResult;
    bool canceled;
    NGEM_FNAME(ngrclRelayUpThread);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Enter.\n");

    for (;;) {
        /* Read from Ninf-G Executable. */
        if (ta->ngta_rcp != NULL) {
            nResult = ngrcOperatorRead(ta->ngta_rcp, buf, NGRC_BUFFER_SIZE, &nread, &canceled);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't send data to client.\n");
                goto thread_end;
            }
            if (canceled) {
                ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Reading is canceled.\n");
                goto thread_end;
            }
        } else {
            nResult = ngrcSocketRead(ta->ngta_executable, buf, NGRC_BUFFER_SIZE, &nread);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Can't read from Ninf-G Executable.\n");
                goto thread_end;
            }
        }
        if (nread == 0) {
            /* EOF or Canceled */
            ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Reading is canceled or EOF.\n");
            goto thread_end;
        }
        ngLogDebug(log, NGRC_LOGCAT_GT, fName,
            "Reads %lu bytes from Ninf-G Executable.\n", (unsigned long)nread);
        NGEM_ASSERT(nread <= sizeof(buf));

        nResult = ngrcOperatorWrite(ta->ngta_client, buf, nread, &nwrite, &canceled);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't send data to client.\n");
            goto thread_end;
        }
        if (canceled) {
            ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Writing is canceled.\n");
            goto thread_end;
        }

        ngLogDebug(log, NGRC_LOGCAT_GT, fName,
            "Writes %lu bytes to Client Communication Proxy.\n", (unsigned long)nwrite);
    }

thread_end:

    nResult = ngrclRelayThreadCountDown(ta->ngta_relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't count down number of worker threads.\n");
    }

    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Leave.\n");

    return NULL;
}

/**
 * Relay data from Ninf-G Executable to Ninf-G Client(Down).
 */
static void *
ngrclRelayDownThread(
    void *arg)
{
    ngrclThreadArgument_t *ta = arg;
    size_t nread;
    size_t nwrite;
    unsigned char buf[NGRC_BUFFER_SIZE];
    ngLog_t *log;
    ngemResult_t nResult;
    bool canceled;
    NGEM_FNAME(ngrclRelayDownThread);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Enter.\n");

    for (;;) {
        canceled = false;
        nread = 0;
        nResult = ngrcOperatorRead(ta->ngta_client, buf, NGRC_BUFFER_SIZE, &nread, &canceled);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't send data to client.\n");
            goto thread_end;
        }
        if (canceled) {
            ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Reading is canceled.\n");
            goto thread_end;
        }
        if (nread == 0) {
            ngLogInfo(log, NGRC_LOGCAT_GT, fName, "EOF.\n");
            /* EOF */
            goto thread_end;
        }
        ngLogDebug(log, NGRC_LOGCAT_GT, fName,
            "Reads %lu bytes from Client Communication Proxy.\n", (unsigned long)nread);
        NGEM_ASSERT(nread <= sizeof(buf));

        /* Read from Ninf-G Executable. */
        if (ta->ngta_rcp != NULL) {
            nResult = ngrcOperatorWrite(ta->ngta_rcp, buf, nread, &nwrite, &canceled);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRC_LOGCAT_GT, fName, "Can't send data to Remote Communication Proxy.\n");
                goto thread_end;
            }
            if (canceled) {
                ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Writing is canceled.\n");
                goto thread_end;
            }
        } else {
            nResult = ngrcSocketWrite(ta->ngta_executable, buf, nread, &nwrite);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Can't write to the Client Communication Proxy.\n");
                goto thread_end;
            }
        }
        ngLogDebug(log, NGRC_LOGCAT_GT, fName,
            "Writes %lu bytes to Ninf-G Executable.\n", (unsigned long)nwrite);

        if (nread != nwrite) {
            /* Canceled */
            ngLogInfo(log, NGRC_LOGCAT_GT, fName, "Writing is canceled.\n");
            goto thread_end;
        }
    }

thread_end:

    nResult = ngrclRelayThreadCountDown(ta->ngta_relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't count down number of worker threads.\n");
    }
    ngLogDebug(log, NGRC_LOGCAT_GT, fName, "Leave.\n");

    return NULL;
}

static ngemResult_t
ngrclRelayThreadCountUp(
    ngrclRelay_t *relay)
{
    bool locked = false;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngrclRelayThreadCountUp);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }
    relay->ngr_nWorkThread++;
    result = ngiRlockBroadcast(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
        ret = NGEM_FAILED;
    }
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }
    return ret;
}

static ngemResult_t
ngrclRelayThreadCountDown(
    ngrclRelay_t *relay)
{
    bool locked = false;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    NGEM_FNAME(ngrclRelayThreadCountDown);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }
    NGEM_ASSERT(relay->ngr_nWorkThread > 0);
    relay->ngr_nWorkThread--;
    result = ngiRlockBroadcast(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
        ret = NGEM_FAILED;
    }
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }
    return ret;
}

static ngemResult_t
ngrclRelayThreadCountWait(
    ngrclRelay_t *relay,
    int count)
{
    bool locked = false;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngrclRelayThreadCountWait);

    log = ngemLogGetDefault();

    result = ngiRlockLock(&relay->ngr_lock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRC_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }
    while (relay->ngr_nWorkThread > count) {
        if (locked) {
            result = ngiRlockWait(&relay->ngr_lock, log, NULL);
            if (result == 0) {
                ngLogError(log, NGRC_LOGCAT_GT, fName,
                    "Can't wait the lock.\n");
                ret = NGEM_FAILED;
            }
        }
    }
    if (locked) {
        result = ngiRlockUnlock(&relay->ngr_lock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRC_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
        locked = false;
    }
    return ret;
}

void
ngrclThreadArgumentZeroClear(
    ngrclThreadArgument_t *arg)
{
    NGEM_ASSERT(arg != NULL);

    arg->ngta_relay      = NULL;
    arg->ngta_client     = NULL;
    arg->ngta_executable = NULL;
    arg->ngta_rcp        = NULL;

    return;
}

