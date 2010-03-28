/*
 * $RCSfile: ngrGT.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $
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
#include "ngemUtility.h"
#include "ngrGT.h"
#include "ngrMain.h"

NGI_RCSID_EMBED("$RCSfile: ngrGT.c,v $ $Revision: 1.4 $ $Date: 2008/03/28 03:52:30 $")

#define NGRL_LOGCAT_GT        "Relay GT"
#define NGRL_BUFFER_SIZE      (4 * 1024)

typedef struct ngcrRelay_s ngcrRelay_t;
typedef struct ngcrUpCallbackArgs_s ngcrUpCallbackArgs_t;

/**
 * Ninf-G Client
 *  | fork&exec
 *  V
 * Client Communication Proxy
 *  | connect          A      | Down
 *  V                  | Up   V
 * Client Relay(It's me!)     
 *  | fork&exec        A      | Down
 *  V                  | UP   V
 * Client Communication Proxy
 *
 * A flow to Ninf-G Client side will be called "Up". 
 * An opposite flow will be called a "Down". 
 */

/**
 * Buffer for the "Up" flow.
 */
typedef union ngcrUpBuffer_s {
    uint32_t      ngub_packHeader[NGR_PACK_HEADER_NELEMENTS];
    unsigned char ngub_buffer[NGR_PACK_HEADER_SIZE + NGRL_BUFFER_SIZE];
} ngcrUpBuffer_t;

/**
 * Callback function's argument for "Up" flow.
 */
struct ngcrUpCallbackArgs_s {
    ngcrRelay_t          *nguca_relay;
    ngcrUpBuffer_t       *nguca_buffer;
    globus_xio_handle_t   nguca_reader;
    size_t                nguca_dataSize;
    ngcrUpCallbackArgs_t *nguca_pair;
};

/**
 * Data Relay
 */
struct ngcrRelay_s {
    ngiRlock_t           ngr_rLock;
    bool                 ngr_destroying;
    pid_t                ngr_pid;
    globus_xio_handle_t  ngr_stdin;
    globus_xio_handle_t  ngr_client;
    unsigned char        ngr_requestBuffer[NGRL_BUFFER_SIZE];

    /* Head 4 bytes of buffer is used by packing header. */
    ngcrUpBuffer_t       ngr_replyBuffer;
    ngcrUpBuffer_t       ngr_notifyBuffer;
    bool                 ngr_writingToClient;
    ngcrUpCallbackArgs_t ngr_upCallbackArgs[2];
};

static ngcrRelay_t *ngrlRelayCreate(globus_xio_handle_t);
static ngemResult_t ngrlRelayDestroy(ngcrRelay_t *);
static ngemResult_t ngrlRelayInvokeCommunicationProxy(ngcrRelay_t *, uid_t, gid_t, char *);
static bool ngrlRelayIsDestroying(ngcrRelay_t *);

/* Callback for communication */
static void ngrlAcceptCallback(
    globus_xio_server_t, globus_xio_handle_t, globus_result_t, void *);

static void ngrlDownReadCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t,
    globus_xio_data_descriptor_t, void *);
static void ngcclDownWriteCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t,
    globus_xio_data_descriptor_t, void *);

static void ngrlUpReadCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t,
    globus_xio_data_descriptor_t, void *);
static void ngrlUpWriteCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t,
    globus_xio_data_descriptor_t, void *);

static void ngrlStdinReadCallback(globus_xio_handle_t, globus_result_t,
    globus_byte_t *, globus_size_t, globus_size_t,
    globus_xio_data_descriptor_t, void *);

static bool ngrlIsOption(char *, char *, char **);
static void ngrlUsage(void);

/* File local variables */
char       *ngrlCommunicationProxy = NULL;
char       *ngrlCommunicationProxyLogPath = NULL;
char       *ngrlLogFileName = NULL;
bool        ngrlCommunicationProxyLog = false;
bool        ngrlAllowNotPrivate = false;
ngiRlock_t  ngrlLock;
bool        ngrlContinue = true;
bool        ngrlRelayCount = 0;
ngcpCommunicationSecurity_t ngrlSecurity = NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY;

int
ngrMain(
    int argc,
    char *argv[],
    char *appname,
    char *comm_proxy)
{
    char *ngdir = NULL;
    ngLog_t *log = NULL;
    globus_xio_server_t server = NULL;
    globus_xio_handle_t hStdin = NULL;
    ngcpPortRange_t portRange;
    ngemResult_t nResult;
    globus_result_t gResult;
    int result;
    char *contact_string = NULL;
    int ret = 1;
    bool locked = false;
    globus_byte_t buffer[NGRL_BUFFER_SIZE];
    char *val;
    int i;
    bool logInitialized = true;
    NGEM_FNAME(ngrMain);

    /* Initialize file local variables */
    ngrlCommunicationProxy = NULL;
    ngrlLock               = NGI_RLOCK_NULL;
    ngrlContinue           = true;
    NGCP_PORT_RANGE_SET(&portRange, 0, 0);

    for (i = 1;i < argc;++i) {
        if (strcmp(argv[i], "--") == 0) {
            ++i;
            break;
        }
        if (argv[i][0] != '-') {
            break;
        }
        if (argv[i][1] == '-') {
            if (ngrlIsOption(argv[i], "allow-not-private", &val)) {
                if (val != NULL) {
                    fprintf(stderr, "--allow-not-private option does not a argument.\n");
                    ngrlUsage();
                    exit(1);
                }
                ngrlAllowNotPrivate = true;
            } else 
            if (ngrlIsOption(argv[i], "crypt", &val)) {
                if (strcmp(val, "true") == 0) {
                    ngrlSecurity = NGCP_COMMUNICATION_SECURITY_CONFIDENTIALITY;
                } else if (strcmp(val, "false") == 0) {
                    ngrlSecurity = NGCP_COMMUNICATION_SECURITY_NONE;
                } else {
                    fprintf(stderr, "%s: An invalid as argument of --crypt option.\n", argv[i]);
                    ngrlUsage();
                    exit(1);
                }
            } else 
            if (ngrlIsOption(argv[i], "communication-proxy-log", &val)) {
                if (val != NULL) {
                    fprintf(stderr, "--communication-proxy-log option does not a argument.\n");
                    ngrlUsage();
                    exit(1);
                }
                ngrlCommunicationProxyLog = true;
            } else {
                fprintf(stderr, "%s: Unkown option.\n", argv[i]);
                ngrlUsage();
                exit(1);
            }
        } else {
            /* short */
            switch (argv[i][1]) {
            case 'l':
                if (strlen(&argv[i][2]) > 0) {
                    val = &argv[i][2];
                } else {
                    i++;
                    if (i == argc) {
                        exit(1);
                    }
                    val = argv[i];
                }
                ngrlLogFileName = val;
                break;
            default:
                fprintf(stderr, "%s: Unkown option.\n", argv[i]);
                ngrlUsage();
                exit(1);
            }
        }
    }

    if (i < argc) {
        fprintf(stderr, "%s is not argument which is not option.\n", argv[0]);
        ngrlUsage();
        exit(1);
    }

    nResult = ngemLogInitialize(appname, ngrlLogFileName);
    if (nResult != NGEM_SUCCESS) {
        fprintf(stderr, "Can't initialize the log.\n");
        goto finalize;
    }
    logInitialized = true;
    log = ngemLogGetDefault();

    if (ngrlSecurity != NGCP_COMMUNICATION_SECURITY_NONE) {
        if (!ngcpCredentialAvailable()) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "The proxy credential is not available.\n");
            goto finalize;
        }
    }

    if (!ngrlAllowNotPrivate) {
#define NGRL_ENV_ONLY_PRIVATE (NGCP_ENV_ONLY_PRIVATE"=1")
        result = putenv(NGRL_ENV_ONLY_PRIVATE);
        if (result < 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "setenv: %s\n", strerror(errno));
            goto finalize;
        }
    }

    ngdir = getenv(NGI_NG_DIR_ENV_NAME);
    if (ngdir == NULL) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Does not define %s environment variable.\n",
            NGI_NG_DIR_ENV_NAME);
        goto finalize;
    }

    /* LogFileName */
    if (ngrlCommunicationProxyLog) {
        if (ngrlLogFileName != NULL) {
            ngrlCommunicationProxyLogPath =
                ngemStrdupPrintf("%s-%%p", ngrlLogFileName);
        } else {
            ngrlCommunicationProxyLogPath =
                ngemStrdupPrintf("%ld-%%p.log", (long)getpid());
        }
        if (ngrlCommunicationProxyLogPath == NULL) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't duplicate the string.\n");
            goto finalize;
        }
    }

    /* Path */
    ngrlCommunicationProxy = ngemStrdupPrintf(
        "%s/bin/%s", ngdir, comm_proxy);
    if (ngrlCommunicationProxy == NULL) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't duplicate the string.\n");
        goto finalize;
    }
    result = ngiRlockInitialize(&ngrlLock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't duplicate the string.\n");
        goto finalize;
    }

    nResult = ngcpGlobusXIOinitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't initialize the .\n");
        goto finalize;
    }

    nResult = ngcpGlobusXIOfileOpen(&hStdin, "stdin://");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't open stdin.\n");
        goto finalize;
    }

    nResult = ngcpGlobusXIOcreateListener(&server, portRange, ngrlSecurity, true);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't create a listener.\n");
        goto finalize;
    }

    gResult = globus_xio_server_get_contact_string(
        server, &contact_string);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_server_get_contact_string", gResult);
        goto finalize;
    }

    fprintf(stdout, "%s\n", contact_string);
    fflush(stdout);

    globus_free(contact_string);
    contact_string = NULL;

    gResult = globus_xio_server_register_accept(
        server, ngrlAcceptCallback, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_server_register_accept", gResult);
        goto finalize;
    }

    gResult = globus_xio_register_read(hStdin, buffer,
        NGRL_BUFFER_SIZE, 1, NULL, ngrlStdinReadCallback, server);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        goto finalize;
    }
    server = NULL;

    /* Waiting */
    result = ngiRlockLock(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the application.\n");
        goto finalize;
    }
    locked = true;
    while ((ngrlContinue) || (ngrlRelayCount > 0)) {
        result = ngiRlockWait(&ngrlLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't wait the application.\n");
            goto finalize;
        }
    }

    ret = 0;

finalize:
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&ngrlLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the application.\n");
            ret = 1;
        }
    }

    if (hStdin != NULL) {
        gResult = globus_xio_close(hStdin, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
    }

    if (server != NULL) {
        gResult = globus_xio_server_close(server);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_server_close", gResult);
        }
    }

    result = ngiRlockFinalize(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't finalize the lock.\n");
        ret = 1;
    }

    ngiFree(ngrlCommunicationProxy, log, NULL);
    ngiFree(ngrlCommunicationProxyLogPath, log, NULL);

    if (logInitialized) {
        logInitialized = false;
        nResult = ngemLogFinalize();
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't finalize the log.\n");
            ret = 1;
        }
    }

    return ret;
}

/**
 * Callback function for accept()
 */
static void
ngrlAcceptCallback(
    globus_xio_server_t server,
    globus_xio_handle_t handle,
    globus_result_t     cResult,
    void               *user_arg)
{
    globus_result_t gResult;
    int result;
    ngLog_t *log = NULL;
    ngcrRelay_t *relay = NULL;
    ngemResult_t nResult;
    uid_t uid = 0;
    gid_t gid = 0;
    bool locked = false;
    gss_cred_id_t cred = GSS_C_NO_CREDENTIAL;
    char *proxyName = NULL;
    bool ok = false;
    NGEM_FNAME(ngrlAcceptCallback);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called\n");

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
            goto callback_end;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
            goto callback_end;
         }
         ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
             "Callback function for accepting", cResult);
         /* Error through */
    }

    gResult = globus_xio_server_register_accept(server, ngrlAcceptCallback, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_server_register_accept", gResult);
        goto callback_end;
    }
    if (cResult != GLOBUS_SUCCESS) {
        return;
    }

    relay = ngrlRelayCreate(handle);
    if (relay == NULL) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't create the Relay.\n");
        goto error;
    }
    if (ngrlSecurity != NGCP_COMMUNICATION_SECURITY_NONE) {
        if (getuid() == 0) {
            nResult = ngcpGlobusXIOcheckPeerNameByGridmap(handle, &ok, &uid, &gid);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRL_LOGCAT_GT, fName,
                    "Failed to check peer subject.\n");
                goto error;
            }

            if (!ok) {
                ngLogError(log, NGRL_LOGCAT_GT, fName,
                    "Peer name is not authorized.\n");
                goto error;
            }
            /* Delegate */
            gResult = ngcpGlobusXIOacceptDelegation(handle, &cred);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                    "Accepts delegation", gResult);
                goto error;
            }
            nResult = ngcpGSSexportCred(cred, &proxyName, uid, gid);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRL_LOGCAT_GT, fName,
                    "Can't export credential.\n");
                goto error;
            }
        } else {
            nResult = ngcpGlobusXIOcheckPeerName(
                handle, NGCP_COMMUNICATION_AUTH_SELF, &ok);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGRL_LOGCAT_GT, fName,
                    "Failed to check peer subject.\n");
                goto error;
            }
            if (!ok) {
                ngLogError(log, NGRL_LOGCAT_GT, fName,
                    "Peer name is not authorized.\n");
                goto error;
            }
        }
    }

    nResult = ngrlRelayInvokeCommunicationProxy(relay, uid, gid, proxyName);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't invoke a new client communication proxy.\n");
        goto error;
    }

    return;
error:
    nResult = ngrlRelayDestroy(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't destroy the Relay.\n");
    }
    if (handle != NULL) {
        gResult = globus_xio_close(handle, NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_close", gResult);
        }
        handle = NULL;
    }

    return;

callback_end:

    result = ngiRlockLock(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't get the lock.\n");
    } else {
        locked = true;
    }
    ngrlContinue = false;
    result = ngiRlockBroadcast(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't broadcast the signal.\n");
    }
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&ngrlLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't release the lock.\n");
        }
    }

    return;
}

/**
 * Relay: Create
 */
static ngcrRelay_t *
ngrlRelayCreate(
    globus_xio_handle_t handle)
{
    ngLog_t *log = NULL;
    ngcrRelay_t *relay = NULL;
    int result;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngcrUpCallbackArgs_t *upcArg;
    NGEM_FNAME(ngrlRelayCreate);

    log = ngemLogGetDefault();

    relay = NGI_ALLOCATE(ngcrRelay_t, log, NULL);
    if (relay == NULL) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't allocate storage for the relay.\n");
        goto error;
    }
    memset(relay, '\0', sizeof(*relay));

    relay->ngr_rLock           = NGI_RLOCK_NULL;
    relay->ngr_pid             = -1;
    relay->ngr_client          = handle;
    relay->ngr_stdin           = NULL;
    relay->ngr_writingToClient = false;

    upcArg = &relay->ngr_upCallbackArgs[0];
    upcArg->nguca_relay    = relay;
    upcArg->nguca_buffer   = &relay->ngr_replyBuffer;
    upcArg->nguca_reader   = NULL;
    upcArg->nguca_dataSize = 0;
    upcArg->nguca_pair     = &relay->ngr_upCallbackArgs[1];
    upcArg->nguca_buffer->ngub_packHeader[0] = htonl(NGR_PACK_TYPE_REPLY);
    upcArg->nguca_buffer->ngub_packHeader[1] = htonl(0U);/* Size */

    upcArg = &relay->ngr_upCallbackArgs[1];
    upcArg->nguca_relay    = relay;
    upcArg->nguca_buffer   = &relay->ngr_notifyBuffer;
    upcArg->nguca_reader   = NULL;
    upcArg->nguca_dataSize = 0;
    upcArg->nguca_pair     = &relay->ngr_upCallbackArgs[0];
    upcArg->nguca_buffer->ngub_packHeader[0] = htonl(NGR_PACK_TYPE_NOTIFY);
    upcArg->nguca_buffer->ngub_packHeader[1] = htonl(0U);/* Size */

    result = ngiRlockInitialize(&relay->ngr_rLock, NULL, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't initialize the lock.\n");
        goto error;
    }

    /* Open Remote Connection */
    gResult = globus_xio_open(relay->ngr_client, NULL, NULL);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_open", gResult);
        goto error;
    }

    result = ngiRlockLock(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the relay.\n");
        goto error;
    }
    ngrlRelayCount++;
    ngiRlockUnlock(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't unlock the relay.\n");
        goto error;
    }

    return relay;
error:
    nResult = ngrlRelayDestroy(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't destroy the relay.\n");
    }

    return NULL;
}

/**
 * Relay: Destroy
 */
static ngemResult_t
ngrlRelayDestroy(
    ngcrRelay_t *relay)
{
    bool locked = false;
    bool glocked = false;
    ngLog_t *log = NULL;
    globus_xio_handle_t handles[4];
    int i;
    int result;
    globus_result_t gResult;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngrlRelayDestroy);

    log = ngemLogGetDefault();

    if (relay == NULL) {
        return NGEM_SUCCESS;
    }

    result = ngiRlockLock(&ngrlLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        glocked = true;
    }
    ngrlRelayCount--;
    if (ngrlRelayCount == 0) {
        result = ngiRlockBroadcast(&ngrlLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't broadcast the signal.\n");
            ret = NGEM_FAILED;
        }
    }
    if (glocked) {
        glocked = false;
        result = ngiRlockUnlock(&ngrlLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
    }

    result = ngiRlockLock(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the Relay.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }
    relay->ngr_destroying = true;
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&relay->ngr_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the Relay.\n");
            ret = NGEM_FAILED;
        }
    }
    i = 0;
    handles[i++] = relay->ngr_client;
    handles[i++] = relay->ngr_stdin;
    handles[i++] = relay->ngr_upCallbackArgs[0].nguca_reader;
    handles[i++] = relay->ngr_upCallbackArgs[1].nguca_reader;
    NGEM_ASSERT(i == 4);

    for (i = 0;i < 4;++i) {
        if (handles[i] == NULL) {
            continue;
        }
        gResult = globus_xio_close(handles[i], NULL);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_handle_close", gResult);
            ret = NGEM_FAILED;
        }
    }
    if (relay->ngr_pid >= 0)  {
        ngcpKillAndWait(relay->ngr_pid);
        relay->ngr_pid = -1;
    }

    result = ngiRlockFinalize(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't finalize the rlock.\n");
        ret = NGEM_FAILED;
    }

    relay->ngr_rLock           = NGI_RLOCK_NULL;
    relay->ngr_pid             = -1;
    relay->ngr_client          = NULL;
    relay->ngr_stdin           = NULL;
    relay->ngr_writingToClient = false;

    relay->ngr_upCallbackArgs[0].nguca_relay    = NULL;
    relay->ngr_upCallbackArgs[0].nguca_buffer   = NULL;
    relay->ngr_upCallbackArgs[0].nguca_reader   = NULL;
    relay->ngr_upCallbackArgs[0].nguca_dataSize = 0;
    relay->ngr_upCallbackArgs[0].nguca_pair     = NULL;
    relay->ngr_upCallbackArgs[1].nguca_relay    = NULL;
    relay->ngr_upCallbackArgs[1].nguca_buffer   = NULL;
    relay->ngr_upCallbackArgs[1].nguca_reader   = NULL;
    relay->ngr_upCallbackArgs[1].nguca_dataSize = 0;
    relay->ngr_upCallbackArgs[1].nguca_pair     = NULL;

    result = NGI_DEALLOCATE(ngcrRelay_t, relay, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't deallocate the relay.\n");
        ret = NGEM_FAILED;
    }

    return ret;
}

/**
 * Relay: Invoke Communication Proxy
 */
static ngemResult_t
ngrlRelayInvokeCommunicationProxy(
    ngcrRelay_t *relay, 
    uid_t uid,
    gid_t gid,
    char *proxy)
{
    pid_t pid = -1;
    ngcpXIOstandardIO_t stdio = {NULL, NULL, NULL};
    char *argv[4];
    ngLog_t *log = NULL;
    bool locked = false;
    int result;
    globus_result_t gResult;
    int i;
    ngemEnvironment_t env[2];
    ngemEnvironment_t *penv = NULL;
    ngemResult_t ret = NGEM_FAILED;
    ngcrUpCallbackArgs_t *args;
    NGEM_FNAME(ngrlRelayInvokeCommunicationProxy);

    log = ngemLogGetDefault();

    i = 0;
    argv[i++] = ngrlCommunicationProxy;
    if (ngrlCommunicationProxyLogPath != NULL) {
        argv[i++] = "-l";
        argv[i++] = ngrlCommunicationProxyLogPath;
    }
    argv[i++] = NULL;

    if (proxy != NULL) {
        penv = env;
        env[0].nge_name  = "X509_USER_PROXY";
        env[0].nge_value = proxy;
        env[1].nge_name  = NULL;
        env[1].nge_value = NULL;
    }

    pid = ngcpXIOpopenArgv(&stdio, argv, uid, gid, penv);
    if (pid < 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't invoke a new communication proxy.\n");
        goto finalize;
    }

    relay->ngr_pid             = pid;
    relay->ngr_stdin           = stdio.ngsio_in;
    relay->ngr_upCallbackArgs[0].nguca_reader = stdio.ngsio_out;
    relay->ngr_upCallbackArgs[1].nguca_reader = stdio.ngsio_err;

    result = ngiRlockLock(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the Relay.\n");
        goto finalize;
    }
    locked = true;

    ngLogDebug(log, NGRL_LOGCAT_GT, fName,
        "globus_xio_register_read() is called!\n");
    gResult = globus_xio_register_read(
        relay->ngr_client, relay->ngr_requestBuffer,
        NGRL_BUFFER_SIZE, 1, NULL, ngrlDownReadCallback, relay);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_read(fromClient)", gResult);
        goto finalize;
    }

    for (i = 0;i < 2;++i) {
        args = &relay->ngr_upCallbackArgs[i];
        ngLogDebug(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_read() is called!\n");
        gResult = globus_xio_register_read(
            args->nguca_reader, 
            args->nguca_buffer->ngub_buffer + NGR_PACK_HEADER_SIZE, 
            NGRL_BUFFER_SIZE, 1, NULL, ngrlUpReadCallback, args);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_register_read", gResult);
            goto finalize;
        }
    }
    ret = NGEM_SUCCESS;
finalize:
    if (ret != NGEM_SUCCESS) {
        if (relay->ngr_pid < 0) {
            ngcpKillAndWait(relay->ngr_pid);
        }
    }

    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&relay->ngr_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the Relay.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Relay: Is destroying?
 */
static bool 
ngrlRelayIsDestroying(
    ngcrRelay_t *relay)
{
    ngLog_t *log = NULL;
    int result;
    bool ret = true;
    bool locked = false;
    NGEM_FNAME(ngrlRelayIsDestroying);

    NGEM_ASSERT(relay != NULL);

    result = ngiRlockLock(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the Relay.\n");
    } else {
        locked = true;
    }
    ret = relay->ngr_destroying;
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&relay->ngr_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the Relay.\n");
        }
    }

    return ret;
}

static void
ngrlDownReadCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                bufferSize,
    globus_size_t                nRead,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngLog_t *log = NULL;
    ngcrRelay_t *relay = userData;
    globus_result_t gResult;
    ngemResult_t nResult;
    NGEM_FNAME(ngrlDownReadCallback);

    NGEM_ASSERT(relay != NULL);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called with ngcrRelay_t *[%p]\n", relay);

    if (ngrlRelayIsDestroying(relay)) {
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_FALSE) {
             ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                 "Callback function for reading", cResult);
         }
         goto communication_close;
    }

    ngLogDebug(log, NGRL_LOGCAT_GT, fName,
        "Reads %ld bytes.\n", (unsigned long)nRead);

    gResult = globus_xio_register_write(
        relay->ngr_stdin, buffer, nRead, nRead, NULL,
        ngcclDownWriteCallback, relay);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_write", gResult);
         goto communication_close;
    }

    return;
communication_close:

    nResult = ngrlRelayDestroy(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogDebug(log, NGRL_LOGCAT_GT, fName,
            "Can't destroy the Relay.\n");
    }

    return;
}

static void
ngcclDownWriteCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                len,
    globus_size_t                nWrite,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngLog_t *log = NULL;
    ngcrRelay_t *relay = userData;
    globus_result_t gResult;
    ngemResult_t nResult;
    NGEM_FNAME(ngcclDownWriteCallback);

    NGEM_ASSERT(relay != NULL);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called with ngcrRelay_t[%p]\n", relay);

    if (ngrlRelayIsDestroying(relay)) {
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
             "Callback function for reading", cResult);
         goto communication_close;
    }

    ngLogDebug(log, NGRL_LOGCAT_GT, fName,
        "globus_xio_register_read() is called!\n");
    gResult = globus_xio_register_read(
        relay->ngr_client, relay->ngr_requestBuffer,
        NGRL_BUFFER_SIZE, 1, NULL, ngrlDownReadCallback, relay);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_read(fromClient)", gResult);
        goto communication_close;
    }

    return;

communication_close:

    nResult = ngrlRelayDestroy(relay);
    if (nResult != NGEM_SUCCESS) {
        ngLogDebug(log, NGRL_LOGCAT_GT, fName,
            "Can't destroy the Relay.\n");
    }
    return;
}

static void
ngrlUpReadCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                bufferSize,
    globus_size_t                nRead,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngLog_t *log = NULL;
    ngcrUpCallbackArgs_t *args = userData;
    ngcrRelay_t *relay = NULL;
    globus_result_t gResult;
    ngemResult_t nResult;
    int result;
    bool locked = false;
    bool communication_end = false;
    NGEM_FNAME(ngrlUpReadCallback);

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called with ngcrUpCallbackArgs_t *[%p]\n", args);

    NGEM_ASSERT(args != NULL);
    NGEM_ASSERT(args->nguca_relay != NULL);
    relay = args->nguca_relay;

    if (ngrlRelayIsDestroying(relay)) {
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
            ngLogInfo(log, NGRL_LOGCAT_GT, fName, "EOF.\n");
         } else {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "Callback function for reading", cResult);
         }
         communication_end = true;
         goto finalize;
    }

    ngLogDebug(log, NGRL_LOGCAT_GT, fName,
        "Reads %ld bytes.\n", (unsigned long)nRead);

    /* Check writing */
    result = ngiRlockLock(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the Relay.\n");
        communication_end = true;
        goto finalize;
    }
    locked = true;

    NGEM_ASSERT(args->nguca_dataSize == 0);
    if (relay->ngr_writingToClient) {
        args->nguca_dataSize = nRead;
    } else {
        args->nguca_buffer->ngub_packHeader[1] = htonl(nRead);
        gResult = globus_xio_register_write(
            relay->ngr_client, args->nguca_buffer->ngub_buffer,
            NGR_PACK_HEADER_SIZE+nRead,
            NGR_PACK_HEADER_SIZE+nRead, NULL,
            ngrlUpWriteCallback, args);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_write", gResult);
            communication_end = true;
            goto finalize;
        }
        relay->ngr_writingToClient = true;
    }

finalize:
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&relay->ngr_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the Relay.\n");
            communication_end = true;
        }
    }
    if (communication_end) {
        nResult = ngrlRelayDestroy(relay);
        if (nResult != NGEM_SUCCESS) {
            ngLogDebug(log, NGRL_LOGCAT_GT, fName,
                "Can't destroy the Relay.\n");
        }
    }
    return;
}

static void
ngrlUpWriteCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                len,
    globus_size_t                nWrite,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngLog_t *log = NULL;
    ngcrUpCallbackArgs_t *args = userData;
    ngcrUpCallbackArgs_t *pair = NULL;
    ngcrRelay_t *relay = NULL;
    globus_result_t gResult;
    ngemResult_t nResult;
    int result;
    bool locked = false;
    bool communication_end = false;
    NGEM_FNAME(ngrlUpWriteCallback);

    NGEM_ASSERT(args != NULL);
    relay = args->nguca_relay;
    pair = args->nguca_pair;

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called with ngcrUpCallbackArgs_t *[%p]\n", args);

    assert(relay != NULL);

    if (ngrlRelayIsDestroying(relay)) {
        return;
    }

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             return;
         }
         ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
             "Callback function for reading", cResult);
         communication_end = true;
         goto finalize;
    }

    ngLogDebug(log, NGRL_LOGCAT_GT, fName,
        "Writes %ld bytes %d.\n", (unsigned long)nWrite, args->nguca_buffer->ngub_packHeader[0]);

    /* Check writing */
    result = ngiRlockLock(&relay->ngr_rLock, log, NULL);
    if (result == 0) {
        ngLogError(log, NGRL_LOGCAT_GT, fName,
            "Can't lock the Relay.\n");
        communication_end = true;
        goto finalize;
    }
    locked = true;

    NGEM_ASSERT(args->nguca_dataSize == 0);

    gResult = globus_xio_register_read(
        args->nguca_reader, 
        args->nguca_buffer->ngub_buffer + NGR_PACK_HEADER_SIZE, 
        NGRL_BUFFER_SIZE, 1, NULL, ngrlUpReadCallback, args);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        communication_end = true;
        goto finalize;
    }

    NGEM_ASSERT(relay->ngr_writingToClient == true);
    if (pair->nguca_dataSize > 0) {
        pair->nguca_buffer->ngub_packHeader[1] = htonl(pair->nguca_dataSize);
        gResult = globus_xio_register_write(
            relay->ngr_client, pair->nguca_buffer->ngub_buffer,
            NGR_PACK_HEADER_SIZE+pair->nguca_dataSize, 
            NGR_PACK_HEADER_SIZE+pair->nguca_dataSize, NULL,
            ngrlUpWriteCallback, pair);
        if (gResult != GLOBUS_SUCCESS) {
            ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
                "globus_xio_write", gResult);
            communication_end = true;
            goto finalize;
        }
        relay->ngr_writingToClient = true;
        pair->nguca_dataSize = 0;
    } else {
        relay->ngr_writingToClient = false;
    }

finalize:
    if (locked) {
        locked = false;
        result = ngiRlockUnlock(&relay->ngr_rLock, log, NULL);
        if (result == 0) {
            ngLogError(log, NGRL_LOGCAT_GT, fName,
                "Can't unlock the Relay.\n");
            communication_end = true;
        }
    }

    if (communication_end) {
        nResult = ngrlRelayDestroy(relay);
        if (nResult != NGEM_SUCCESS) {
            ngLogDebug(log, NGRL_LOGCAT_GT, fName,
                "Can't destroy the Relay.\n");
        }
    }
    return;
}

static void
ngrlStdinReadCallback(
    globus_xio_handle_t          handle,
    globus_result_t              cResult,
    globus_byte_t               *buffer,
    globus_size_t                len,
    globus_size_t                nRead,
    globus_xio_data_descriptor_t dataDesc,
    void                        *userData)
{
    ngLog_t *log = NULL;
    ngcrUpCallbackArgs_t *args = userData;
    ngcrUpCallbackArgs_t *pair = NULL;
    ngcrRelay_t *relay = NULL;
    globus_result_t gResult;
    bool communication_end = false;
    NGEM_FNAME(ngrlStdinReadCallback);

    NGEM_ASSERT(args != NULL);
    relay = args->nguca_relay;
    pair = args->nguca_pair;

    log = ngemLogGetDefault();
    ngLogDebug(log, NGRL_LOGCAT_GT, fName, "Called with ngcrUpCallbackArgs_t *[%p]\n", args);

    if (cResult != GLOBUS_SUCCESS) {
         if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
             goto callback_end;
         }
         if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
             goto callback_end;
         }
         ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
             "Callback function for reading", cResult);
         communication_end = true;
         goto callback_end;
    }

    gResult = globus_xio_register_read(
        handle, buffer, len, 1, NULL, ngrlStdinReadCallback, userData);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_register_read", gResult);
        goto callback_end;
    }
    return;

callback_end:
    gResult = globus_xio_server_close((globus_xio_server_t)userData);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGRL_LOGCAT_GT, fName,
            "globus_xio_server_close", gResult);
    }

    return;
}

static bool
ngrlIsOption(
    char *arg,
    char *opt,
    char **val)
{

    NGEM_ASSERT(arg != NULL);
    NGEM_ASSERT(strlen(arg) > 2);
    NGEM_ASSERT(arg[0] == '-');
    NGEM_ASSERT(arg[1] == '-');
    NGEM_ASSERT(opt != NULL);
    NGEM_ASSERT(strlen(opt) > 0);
    NGEM_ASSERT(val != NULL);

    *val = NULL;

    if (strncmp(&arg[2], opt, strlen(opt)) != 0) {
        return false;
    }
    switch(arg[2+strlen(opt)]) {
        case '\0':
            return true;
        case '=':
            *val = &arg[2+strlen(opt) + 1];
            return true;
            break;
        default:
            ;
    }
    return false;
}

static void
ngrlUsage(void)
{
    printf("\n"
        "Relay using Globus Toolkit,\n"
        "options:\n"
        "\t--allow-not-private       : Allows the communication which is not private.\n"
        "\t--crypt=true/false        : Whether control communication is encrypted or not.\n"
        "\t--communication-proxy-log : output the log of communication proxy\n"
        "\t-l [log file name]        : outputs the log to file\n");
}
