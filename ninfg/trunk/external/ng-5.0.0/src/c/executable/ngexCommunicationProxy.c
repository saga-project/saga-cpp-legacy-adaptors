/*
 * $RCSfile: ngexCommunicationProxy.c,v $ $Revision: 1.11 $ $Date: 2008/03/07 06:26:07 $
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

/**
 * Module of handling Remote Communication Proxy
 */

#include "ngEx.h"
#include "grpc_executable.h"

NGI_RCSID_EMBED("$RCSfile: ngexCommunicationProxy.c,v $ $Revision: 1.11 $ $Date: 2008/03/07 06:26:07 $")

#define NGEXL_COMMUNICATION_PROXY_INITIALIZE_REQUEST "INITIALIZE"
#define NGEXL_COMMUNICATION_PROXY_OPTION_ADDRESS    "address"
#define NGEXL_COMMUNICATION_PROXY_EXIT_REQUEST      NGI_EXTERNAL_MODULE_REQUEST_EXIT
#define NGEXL_COMMUNICATION_PROXY_PROTOCOL_VERSION  "1.0"

/**
 * Prototype declaration of internal functions.
 */
static void ngexlCommunicationProxyInitializeMember(ngexiCommunicationProxy_t *);
static int ngexlCommunicationProxyQueryFeatures(
    ngexiCommunicationProxy_t *, int *);
static int ngexlCommunicationProxySendInitialize(
    ngexiCommunicationProxy_t *, ngiLineList_t *, char **, int *);
static int ngexlCommunicationProxySendExit(
    ngexiCommunicationProxy_t *commProxy, int *error);

static const char *ngexlCommunicationProxyNecessaryRequests[] = {
    "QUERY_FEATURES",
    "INITIALIZE",
    "EXIT",
    NULL
};

ngexiCommunicationProxy_t *
ngexiCommunicationProxyConstruct(
    ngexiContext_t *context, 
    char           *type,
    char           *path,
    ngiLineList_t  *options,
    char          **address,
    ngLog_t        *log,
    int            *error)
{
    int result;
    ngexiCommunicationProxy_t *commProxy = NULL;
    static const char fName[] = "ngexiCommunicationProxyConstruct";

    assert(context != NULL);
    assert((type != NULL) || (path != NULL));

    commProxy = NGI_ALLOCATE(ngexiCommunicationProxy_t, log, error);
    if (commProxy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate the storage for the Communication Proxy.\n");
        goto error;
    }
    ngexlCommunicationProxyInitializeMember(commProxy);

    commProxy->ngcp_context = context;

    commProxy->ngcp_externalModuleManager =
        ngiExternalModuleManagerConstruct(
        context->ngc_event, log, error);
    if (commProxy->ngcp_externalModuleManager == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the external module manager.\n");
        goto error;
    }

    result = ngiExternalModuleManagerListWriteLock(
            commProxy->ngcp_externalModuleManager, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock external module manager.\n");
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,
        "Communication Proxy log is %s.\n",
        ((context->ngc_lmInfo.nglmi_commProxyLogFilePath != NULL) ?
        context->ngc_lmInfo.nglmi_commProxyLogFilePath : "not printed"));

    commProxy->ngcp_externalModule = 
        ngiExternalModuleConstruct(
            commProxy->ngcp_externalModuleManager,
            NGI_EXTERNAL_MODULE_TYPE_COMMUNICATION_PROXY,
            NGI_EXTERNAL_MODULE_SUB_TYPE_REMOTE_COMMUNICATION_PROXY,
            type, path, NULL,
            context->ngc_lmInfo.nglmi_commProxyLogFilePath,
            0, NULL, log, error);
    if (commProxy->ngcp_externalModuleManager == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the external module.\n");
        error = NULL;
    }

    result = ngiExternalModuleManagerListWriteUnlock(
            commProxy->ngcp_externalModuleManager, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock external module manager.\n");
        goto error;
    }
    if (commProxy->ngcp_externalModule == NULL) {
        goto error;
    }

    result = ngexlCommunicationProxyQueryFeatures(commProxy, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't send QUERY_FEATURES.\n");
        goto error;
    }

    result = ngexlCommunicationProxySendInitialize(
        commProxy, options, address, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't send INITIALIZE.\n");
        goto error;
    }

    return commProxy;
error:
    result = ngexiCommunicationProxyDestruct(
        commProxy, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the Communication Proxy.\n");
    }

    return NULL;
}

int
ngexiCommunicationProxyDestruct(
    ngexiCommunicationProxy_t *commProxy,
    int                       *error)
{
    int ret = 1;
    int result;
    ngLog_t *log;
    int locked = 0;
    static const char fName[] = "ngexiCommunicationProxyDestruct";

    if (commProxy == NULL) {
        /* Success */
        return 1;
    }

    assert(commProxy != NULL);
    assert(commProxy->ngcp_context != NULL);

    log = commProxy->ngcp_context->ngc_log;

    result = ngexlCommunicationProxySendExit(commProxy, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't send EXIT request.\n");
        error = NULL;
        ret = 0;
    }

    result = ngiExternalModuleManagerListWriteLock(
            commProxy->ngcp_externalModuleManager, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock external module manager.\n");
        error = NULL;
        ret = 0;
    } else {
        locked = 1;
    }

    result = ngiExternalModuleDestruct(
        commProxy->ngcp_externalModule, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the external module.\n");
        error = NULL;
        ret = 0;
    }

    if (locked != 0) {
        result = ngiExternalModuleManagerListWriteUnlock(
                commProxy->ngcp_externalModuleManager, log ,error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't unlock external module manager.\n");
            error = NULL;
            ret = 0;
        }
    }

    result = ngiExternalModuleManagerDestruct(
        commProxy->ngcp_externalModuleManager, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the external module manager.\n");
        error = NULL;
        ret = 0;
    }

    ngexlCommunicationProxyInitializeMember(commProxy);

    result = NGI_DEALLOCATE(ngexiCommunicationProxy_t,
        commProxy, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free the storage for Communication Proxy.\n");
        error = NULL;
        ret = 0;
    }


    return ret;
}

static void
ngexlCommunicationProxyInitializeMember(
    ngexiCommunicationProxy_t *commProxy)
{
    assert(commProxy != NULL);

     commProxy->ngcp_context               = NULL;
     commProxy->ngcp_externalModuleManager = NULL;
     commProxy->ngcp_externalModule        = NULL;

    return;
}

/**
 * Communication Proxy: Send QUERY_FEATURES.
 */
static int
ngexlCommunicationProxyQueryFeatures(
    ngexiCommunicationProxy_t *commProxy,
    int *error)
{
    int requestSuccess = 0;
    ngiLineList_t *features = NULL;
    ngiLineList_t *requests = NULL;
    char *version = NULL;
    char *errorMessage = NULL;
    int result;
    ngLog_t *log = NULL;
    ngexiContext_t *context = NULL;
    const char **p;
    char *it;
    int found;
    int ret = 0;
    static const char fName[] = "ngexlCommunicationProxyQueryFeatures";

    assert(commProxy != NULL);
    assert(commProxy->ngcp_context != NULL);

    context = commProxy->ngcp_context;
    log = context->ngc_log;

    /* Query Features. */
    result = ngiExternalModuleQueryFeatures(
        commProxy->ngcp_externalModule,
        &requestSuccess, &version, &features, &requests,
        &errorMessage, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed.\n");
        goto finalize;
    }
    if (requestSuccess == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Query the features failed: \"%s\".\n",
            ((errorMessage != NULL) ? errorMessage : ""));

        result = ngiFree(errorMessage, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free error message string.\n");
            ret = 0;
            error = NULL;
        }
        errorMessage = NULL;

        goto finalize;
    }

    assert(version  != NULL);
    assert(features != NULL);
    assert(requests != NULL);

    /* Check Version */
    if (strcmp(version, NGEXL_COMMUNICATION_PROXY_PROTOCOL_VERSION) != 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Version \"%s\" is unknown.\n", version);
        goto finalize;
    }

    /* Check Feature */
    /* There is no feature in Ninf-G Executable */

    /* Check REQUESTS */
    for (p = ngexlCommunicationProxyNecessaryRequests; *p != NULL;++p) {
        found = 0;
        it = NULL;
        while ((it = ngiLineListLineGetNext(requests, it, log, error)) != NULL) {
            if (strcmp(*p, it) == 0) {
                found = 1;
                break;
            }
        }
        if (found == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Communication Proxy does not \"%s\" request.\n", *p);
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    if (features != NULL) {
        result = ngiLineListDestruct(features, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy a line list.\n");
            ret = 0;
            error = NULL;
        }
    }
    
    if (requests != NULL) {
        result = ngiLineListDestruct(requests, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy a line list.\n");
            ret = 0;
            error = NULL;
        }
    }

    result = ngiFree(version, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free version string.\n");
        ret = 0;
        error = NULL;
    }

    return ret;
}

/**
 * Communication Proxy: Send INITIALIZE
 */
static int
ngexlCommunicationProxySendInitialize(
    ngexiCommunicationProxy_t *commProxy,
    ngiLineList_t *options,
    char **address,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    ngexiContext_t *context = NULL;
    ngiLineList_t *arguments = NULL;
    ngiLineList_t *replyArgs = NULL;
    ngiExternalModuleArgument_t *arg = NULL;
    char *p;
    int replySuccess = 0;
    int ret = 0;
    char *replyMessage = NULL;
    char *str = NULL;
    char *retAddress = NULL;
    static const char fName[] = "ngexlCommunicationProxySendInitialize";

    assert(commProxy != NULL);
    assert(commProxy->ngcp_context != NULL);

    context = commProxy->ngcp_context;
    log     = context->ngc_log;

    /* Encode options */
    arguments = ngiLineListConstruct(log, error);
    if (arguments == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize line list.\n");
        goto finalize;
    }

    assert(context->ngc_commInfo.ngci_hostname != NULL);
    result = ngiLineListPrintf(arguments, log, error, "hostname %s",
        context->ngc_commInfo.ngci_hostname);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    p = NULL;
    while ((p = ngiLineListLineGetNext(options, p, log, error)) != NULL) {
        str = NULL;
        str = ngiCommunicationProxyDecode(p, log, error);
        if (str == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't decode string \"%s\".\n", p);
            goto finalize;
        }
        result = ngiLineListAppend(arguments, str, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't append string to line list.\n");
            goto finalize;
        }
        result = ngiFree(str, log ,error);
        str = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the string.\n");
            goto finalize;
        }
    }

    /* TCP_NODELAY */
    result = ngiLineListPrintf(arguments, log, error,
        "tcp_nodelay %s", (context->ngc_commInfo.ngci_tcpNodelay != 0)?("true"):("false"));
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    /* TCP connect retry */
    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryCount %d", context->ngc_retryInfo.ngcri_count);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryBaseInterval %d", context->ngc_retryInfo.ngcri_interval);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryIncreaseRatio %lf", context->ngc_retryInfo.ngcri_increase);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    result = ngiLineListPrintf(arguments, log, error,
        "tcp_connect_retryRandom %s", 
        (context->ngc_retryInfo.ngcri_useRandom!= 0)?("true"):("false"));
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append string to line list.\n");
        goto finalize;
    }

    /* Send Request */
    result = ngiExternalModuleRequest(
        commProxy->ngcp_externalModule,
        NGEXL_COMMUNICATION_PROXY_INITIALIZE_REQUEST,
        NULL, arguments, 0, 
        &replySuccess, &replyMessage, &replyArgs, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        goto finalize;
    }

    if (replySuccess == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));
        goto finalize;
    }

    assert(replyArgs != NULL);

    p = NULL;
    while ((p = ngiLineListLineGetNext(replyArgs, p, log, error)) != NULL) {
        arg = ngiExternalModuleArgumentConstruct(p, log, error);
        if (arg == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't parse the reply argument.\n");
            goto finalize;
        }
        if (strcmp(NGEXL_COMMUNICATION_PROXY_OPTION_ADDRESS,
                arg->ngea_name) != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" is unknown option name.\n", arg->ngea_name);
            goto finalize;
        } else if (retAddress != NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" appears one more time.\n",
                NGEXL_COMMUNICATION_PROXY_OPTION_ADDRESS);
            goto finalize;
        } else {
            retAddress = ngiStrdup(arg->ngea_value, log, error);
            if (retAddress == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't copy the string.\n");
                goto finalize;
            }
        }

        result = ngiExternalModuleArgumentDestruct(arg, log, error);
        arg = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't parse the reply argument.\n");
            goto finalize;
        }
    }
    /* Check */
    if (retAddress == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "%s does not appear.",
            NGEXL_COMMUNICATION_PROXY_OPTION_ADDRESS);
        goto finalize;
    }

    ret = 1;
finalize:
    if (ret == 0) {
        error = NULL;
    }

    result = ngiExternalModuleArgumentDestruct(arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't destruct the reply argument.\n");
        error = NULL;
        ret = 0;
    }
    if (arguments != NULL) {
        result = ngiLineListDestruct(arguments, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't destroy line list.\n");
            error = NULL;
            ret = 0;
        }
        arguments = NULL;
    }
    if (str != NULL) {
        result = ngiFree(str, log ,error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
        str = NULL;
    }
    if (replyMessage != NULL) {
        result = ngiFree(replyMessage, log ,error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
        replyMessage = NULL;
    }
    
    if (ret != 0) {
        *address = retAddress;
    } else {
        result = ngiFree(retAddress, log ,error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the string.\n");
            error = NULL;
            ret = 0;
        }
        retAddress = NULL;
    }

    return ret;
}

/**
 * Communication Proxy: Send Exit
 */
static int
ngexlCommunicationProxySendExit(
    ngexiCommunicationProxy_t *commProxy,
    int *error)
{
    int result;
    ngLog_t *log = NULL;
    int replySuccess = 0;
    char *replyMessage = NULL;
    ngexiContext_t *context = NULL;
    static const char fName[] = "ngexlCommunicationProxySendExit";

    assert(commProxy != NULL);
    assert(commProxy->ngcp_context != NULL);

    context = commProxy->ngcp_context;
    log     = context->ngc_log;

    /* Send Request */
    result = ngiExternalModuleRequest(
        commProxy->ngcp_externalModule,
        NGEXL_COMMUNICATION_PROXY_EXIT_REQUEST, NULL,
        NULL, 0, 
        &replySuccess, &replyMessage, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Request to the Communication Proxy failed.\n");
        return 0;
    }

    if (replySuccess == 0) {
        /* Failed */
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Request's result is \"failed\": %s.\n",
            NGI_EXTERNAL_MODULE_REPLY_MESSAGE(replyMessage));

        result = ngiFree(replyMessage, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't free the string.\n");
        }
        return 0;
    }
    assert(replyMessage == NULL);

    return 1;
}
