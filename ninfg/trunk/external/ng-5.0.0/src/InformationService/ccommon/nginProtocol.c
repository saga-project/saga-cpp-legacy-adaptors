/*
 * $RCSfile: nginProtocol.c,v $ $Revision: 1.12 $ $Date: 2008/03/17 09:27:44 $
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

#include "ngemUtility.h"
#include "nginProtocol.h"

NGI_RCSID_EMBED("$RCSfile: nginProtocol.c,v $ $Revision: 1.12 $ $Date: 2008/03/17 09:27:44 $")

static ngemResult_t QUERY_REMOTE_EXECUTABLE_INFORMATION_begin(ngemRequestFunctionArgument_t *);
static ngemResult_t QUERY_REMOTE_EXECUTABLE_INFORMATION_end(ngemRequestFunctionArgument_t *);
static ngemResult_t QUERY_REMOTE_EXECUTABLE_INFORMATION_reply(ngemRequestFunctionArgument_t *);
static ngemResult_t CANCEL_QUERY_begin(ngemRequestFunctionArgument_t *);

static nginQueryREIoptions_t *nginlQueryREIoptionsCreate(void);
static void nginlQueryREIoptionsDestroy(nginQueryREIoptions_t *);
static void nginlQueryREIoptionsReset(nginQueryREIoptions_t *);

static ngemResult_t nginlProtocolSendInformationNotify(
    nginProtocol_t *, int, ngemResult_t, const char *, const char *);

/* Requests */
static ngemRequestInformation_t nginlProtocolQueryRemoteExecutableInformationRequest = {
    "QUERY_REMOTE_EXECUTABLE_INFORMATION",
    0,
    true,
    QUERY_REMOTE_EXECUTABLE_INFORMATION_begin,
    QUERY_REMOTE_EXECUTABLE_INFORMATION_end,
    QUERY_REMOTE_EXECUTABLE_INFORMATION_reply
};
static ngemRequestInformation_t nginlProtocolCancelQueryRequest = {
    "CANCEL_QUERY",
    1,
    false,
    CANCEL_QUERY_begin,
    NULL,
    NULL
};

/**
 * Information Service Protocol: Create
 */
nginProtocol_t *
nginProtocolCreate(
    nginProtocolActions_t *actions,
    void *userData)
{
    nginProtocol_t *protocol = NULL;
    ngemProtocol_t *baseProtocol = NULL;
    nginQueryREIoptions_t *opts = NULL;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(nginProtocolCreate);

    log = ngemLogGetDefault();

    protocol = NGI_ALLOCATE(nginProtocol_t, log, NULL);
    if (protocol == NULL) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't allocate storage for the protocol.\n");
        goto error;
    }

    baseProtocol = ngemProtocolCreate((void *)protocol, "\r\n",
        NGIN_PROTOCOL_VERSION_MAJOR, NGIN_PROTOCOL_VERSION_MINOR);
    if (baseProtocol == NULL) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't create the Base Protocol Manager.\n");
        goto error;
    }

    nResult = (ngemResult_t)(
        ngemProtocolAppendRequestInfo(baseProtocol, &nginlProtocolQueryRemoteExecutableInformationRequest) &&
        ngemProtocolAppendRequestInfo(baseProtocol, &nginlProtocolCancelQueryRequest)                      &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolQueryFeaturesRequest)                     &&
        ngemProtocolAppendRequestInfo(baseProtocol, &ngemProtocolExitRequest));
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't append request information.\n");
        goto error;
    }

    opts = nginlQueryREIoptionsCreate();
    if (opts == NULL) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't create the options container.\n");
        goto error;
    }

    protocol->ngp_protocol             = baseProtocol;
    protocol->ngp_actions              = actions;
    protocol->ngp_currentQueryIdentify = -1;
    protocol->ngp_options              = opts;
    protocol->ngp_userData             = userData;

    return protocol;
error:
    nResult = nginProtocolDestroy(protocol);
    if (nResult == NGEM_FAILED) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't destroy the protocol.\n");
    }
    return NULL;
}

/**
 * Information Service Protocol: Destroy
 */
ngemResult_t
nginProtocolDestroy(
    nginProtocol_t *protocol)
{
    ngLog_t *log = NULL;
    int result;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(nginProtocolDestroy);

    log = ngemLogGetDefault();

    if (protocol == NULL) {
        return ret;
    }

    result = ngemProtocolDestroy(protocol->ngp_protocol);
    if (result == 0) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't destroy the Base Protocol Manager.\n");
        ret = NGEM_FAILED;
    }
    protocol->ngp_protocol = NULL;

    nginlQueryREIoptionsDestroy(protocol->ngp_options);
    protocol->ngp_options = NULL;

    protocol->ngp_protocol             = NULL;
    protocol->ngp_actions              = NULL;
    protocol->ngp_currentQueryIdentify = -1;
    protocol->ngp_options              = NULL;

    NGI_DEALLOCATE(nginProtocol_t, protocol, log, NULL);

    return ret;
}

static ngemResult_t
QUERY_REMOTE_EXECUTABLE_INFORMATION_begin(ngemRequestFunctionArgument_t *arg)
{
    ngemOptionAnalyzer_t *analyzer = arg->ngra_analyzer;
    nginProtocol_t *protocol = (nginProtocol_t *)arg->ngra_userData;
    int identify = -1;
    ngLog_t *log = NULL;
    nginQueryREIoptions_t *opts = NULL;
    ngemResult_t nResult;
    char *message = NULL;
    NGEM_FNAME(QUERY_REMOTE_EXECUTABLE_INFORMATION_begin);

    log = ngemLogGetDefault();

    opts = protocol->ngp_options;
    nginlQueryREIoptionsReset(opts);
    nResult = (ngemResult_t) (
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, analyzer, "hostname",  ngemOptionAnalyzerSetString, &opts->ngqo_hostname,   1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(char *, analyzer, "classname", ngemOptionAnalyzerSetString, &opts->ngqo_classname,  1, 1) &&
        NGEM_OPTION_ANALYZER_SET_ACTION(NGEM_LIST_OF(char), analyzer, "source", ngemOptionAnalyzerSetStringList, &opts->ngqo_sources, 1, -1));
    if (nResult != NGEM_SUCCESS) {
        message = "Can't set options to analyzer.";
        goto error;
    }

    identify = protocol->ngp_actions->ngpa_queryBegin(protocol, analyzer);
    if (identify < 0) {
        message = "Can't start to process QUERY_REMOTE_EXECUTABLE_INFORMATION.";
        goto error;
    }

    NGEM_ASSERT(protocol->ngp_currentQueryIdentify < 0);
    protocol->ngp_currentQueryIdentify = identify;

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);
    ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "%s\n", message);
    ngemReplySetError(arg->ngra_reply, message);

    return NGEM_SUCCESS;
}

static ngemResult_t
QUERY_REMOTE_EXECUTABLE_INFORMATION_end(
    ngemRequestFunctionArgument_t *arg)
{
    int identify = -1;
    ngemReply_t *reply = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *message;
    nginProtocol_t *protocol = (nginProtocol_t *)arg->ngra_userData;
    NGEM_FNAME(QUERY_REMOTE_EXECUTABLE_INFORMATION_end);

    log = ngemLogGetDefault();
    reply = arg->ngra_reply;
    identify = protocol->ngp_currentQueryIdentify;
    if (identify < 0) {
        message = "Invalid status, current query ID is invalid.";
        goto error;
    }
    nResult = ngemReplyAddParam(reply, "%d", identify);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't add query ID to parameters.";
        goto error;
    }

    return NGEM_SUCCESS;
error:
    NGEM_ASSERT(message != NULL);

    ngemReplySetError(reply, message);
    ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
        "%s\n", message);

    return NGEM_SUCCESS;
}

static ngemResult_t
QUERY_REMOTE_EXECUTABLE_INFORMATION_reply(ngemRequestFunctionArgument_t *arg)
{
    ngemResult_t nResult;
    nginProtocol_t *protocol = (nginProtocol_t *)arg->ngra_userData;
    int identify = -1;
    ngLog_t *log = NULL;
    NGEM_FNAME(QUERY_REMOTE_EXECUTABLE_INFORMATION_reply);

    log = ngemLogGetDefault();
    identify = protocol->ngp_currentQueryIdentify;
    protocol->ngp_currentQueryIdentify = -1;

    if (identify >= 0) {
        nResult = protocol->ngp_actions->ngpa_queryEnd(
            protocol, identify, protocol->ngp_options,
            ngemReplyGetError(arg->ngra_reply));
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
                "Can't process QUERY_REMOTE_EXECUTABLE_INFORMATION.");
            goto error;
        }
    }
    nginlQueryREIoptionsReset(protocol->ngp_options);
    return NGEM_SUCCESS;
error:
    abort();
    return NGEM_SUCCESS;
}

static ngemResult_t
CANCEL_QUERY_begin(ngemRequestFunctionArgument_t *arg)
{
    ngemResult_t nResult;
    nginProtocol_t *protocol = (nginProtocol_t *)arg->ngra_userData;
    int identify = -1;
    ngLog_t *log = NULL;
    char *endp;
    NGEM_FNAME(CANCEL_QUERY_begin);

    log = ngemLogGetDefault();

    errno = 0;
    identify = (int)strtol(arg->ngra_params[0], &endp, 0);
    if ((errno != 0) && (*endp != '\0')) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't get identify.\n");
        goto error;
    }

    nResult = protocol->ngp_actions->ngpa_cancelQuery(protocol, identify);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't cancel query(id=%d).\n", identify);
        goto error;
    }
    
    return NGEM_SUCCESS;
error:
    abort();
    return NGEM_SUCCESS;
}

/**
 * nginProtocol: Send REMOTE_EXECUTABLE_INFORMATION_NOTIFY
 */
static ngemResult_t
nginlProtocolSendInformationNotify(
    nginProtocol_t *protocol,
    int queryId,
    ngemResult_t queryResult,
    const char *xml, 
    const char *errorMessage)
{
    ngemNotify_t *notify = NULL;
    ngemResult_t ret = NGEM_FAILED;
    ngemResult_t nResult;
    char *xmlCopy = NULL;
    char *p = NULL;
    char *q = NULL;
    ngLog_t *log = NULL;
    char *resultString = NULL;
    NGEM_FNAME(nginlProtocolSendInformationNotify);

    log = ngemLogGetDefault();

    notify = ngemNotifyCreate("REMOTE_EXECUTABLE_INFORMATION_NOTIFY", true);
    if (notify == NULL) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't create a notify.\n");
        goto finalize;
    }

    nResult = ngemNotifyAddOption(notify, "query_id", "%d", queryId);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't add \"query_id\" option to the notify.\n");
        goto finalize;
    }

    if (queryResult == NGEM_SUCCESS) {
        resultString = "S";
    } else {
        resultString = "F";
    }
    nResult = ngemNotifyAddOption(notify, "result", resultString);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't add \"result\" option to the notify.\n");
        goto finalize;
    }

    if (queryResult == NGEM_SUCCESS) {
        NGEM_ASSERT_STRING(xml);
        xmlCopy = strdup(xml);
        if (xmlCopy == NULL) {
            ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't copy xml string.\n");
            goto finalize;
        }

        p = q = xmlCopy;
        while (*p != '\0') {
            NGEM_ASSERT_STRING(p);

            q = strchr(p, '\n');
            if (q != NULL) {
                *q = '\0';
            }
            nResult = ngemNotifyAddOption(
                notify, "remote_executable_information", "%s", p);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
                    "Can't add \"remote_executable_information\" "
                    "option to the notify.\n");
                goto finalize;
            }
            if (q != NULL) {
                p = q + 1;
            } else {
                p = &p[strlen(p)];
            }
        }
    } else {
        NGEM_ASSERT_STRING(errorMessage);
        nResult = ngemNotifyAddOption(notify, "error_message", errorMessage);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
                "Can't add \"error_message\" option to the notify.\n");
            goto finalize;
        }
    }

    ngLogDebug(log, NGIN_LOGCAT_PROTOCOL, fName, "Send Notify.\n");
    nResult = ngemProtocolSendNotify(protocol->ngp_protocol, &notify);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName, "Can't send the notify.\n");
        goto finalize;
    }
    
    ret = NGEM_SUCCESS;
finalize:

    ngiFree(xmlCopy, log, NULL);

    if (notify != NULL) {
        ngemNotifyDestroy(notify);
        notify = NULL;
    }
    return ret;
}

/**
 * nginProtocol: Send REMOTE_EXECUTABLE_INFORMATION_NOTIFY
 */
ngemResult_t
nginProtocolSendInformationNotify(
    nginProtocol_t *protocol,
    int queryId,
    const char *xml)
{
    NGEM_FNAME_TAG(nginProtocolSendInformationNotify);

    return nginlProtocolSendInformationNotify(
        protocol, queryId, NGEM_SUCCESS, xml, NULL);
}

/**
 * nginProtocol: Send REMOTE_EXECUTABLE_INFORMATION_NOTIFY(if failed)
 */
ngemResult_t
nginProtocolSendInformationNotifyFailed(
    nginProtocol_t *protocol,
    int queryId,
    const char *errorMessage)
{
    NGEM_FNAME_TAG(nginProtocolSendInformationNotifyFailed);

    return nginlProtocolSendInformationNotify(
        protocol, queryId, NGEM_FAILED, NULL, errorMessage);
}

static nginQueryREIoptions_t *
nginlQueryREIoptionsCreate(void)
{
    nginQueryREIoptions_t *opts = NULL;
    ngLog_t *log;
    NGEM_FNAME(nginlQueryREIoptionsCreate);

    log = ngemLogGetDefault();

    opts = NGI_ALLOCATE(nginQueryREIoptions_t, log, NULL);
    if (opts == NULL) {
        ngLogError(log, NGIN_LOGCAT_PROTOCOL, fName,
            "Can't allocate storage for QueryREIoptions.\n");
        return NULL;
    }

    opts->ngqo_hostname  = NULL;
    opts->ngqo_classname = NULL;
    NGEM_LIST_INITIALIZE(char, &opts->ngqo_sources);

    return opts;
}

static void
nginlQueryREIoptionsDestroy(
    nginQueryREIoptions_t *opts)
{
    ngLog_t *log;
    NGEM_FNAME_TAG(nginlQueryREIoptionsDestroy);

    if (opts == NULL) {
        return;
    }

    log = ngemLogGetDefault();
    
    nginlQueryREIoptionsReset(opts);
    NGEM_LIST_FINALIZE(char, &opts->ngqo_sources);
    NGI_DEALLOCATE(nginQueryREIoptions_t, opts, log, NULL);

    return;
}

static void
nginlQueryREIoptionsReset(
    nginQueryREIoptions_t *opts)
{
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(char) it;
    char *p;
    NGEM_FNAME_TAG(nginlQueryREIoptionsReset);

    NGEM_ASSERT(opts != NULL);

    log = ngemLogGetDefault();
    
    ngiFree(opts->ngqo_hostname, log, NULL);
    ngiFree(opts->ngqo_classname, log, NULL);

    NGEM_LIST_ERASE_EACH(char, &opts->ngqo_sources, it, p) {
        ngiFree(p, log, NULL);
    }

    opts->ngqo_hostname   = NULL;
    opts->ngqo_classname  = NULL;

    return;
}
