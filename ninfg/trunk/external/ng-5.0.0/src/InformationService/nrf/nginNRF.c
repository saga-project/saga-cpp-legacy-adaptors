/*
 * $RCSfile: nginNRF.c,v $ $Revision: 1.11 $ $Date: 2008/03/07 04:07:06 $
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

#include "nginProtocol.h"
#include "nginReadNRF.h"

NGI_RCSID_EMBED("$RCSfile: nginNRF.c,v $ $Revision: 1.11 $ $Date: 2008/03/07 04:07:06 $")

static int nginlNRFqueryBegin(nginProtocol_t *, ngemOptionAnalyzer_t *);
static ngemResult_t nginlNRFqueryEnd(nginProtocol_t *, int, nginQueryREIoptions_t *, ngemResult_t);
static ngemResult_t nginlNRFcancelQuery(nginProtocol_t *, int);

static nginProtocolActions_t nginlActions = {
    nginlNRFqueryBegin,
    nginlNRFqueryEnd,
    nginlNRFcancelQuery,
};

NGEM_DECLARE_LIST_OF(nginREIcontainer_t);

int
main(
    int argc,
    char *argv[])
{
    int result;
    int debug;
    char *logFileName = NULL;
    int opt;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    bool callbackInitialized = false;
    bool logInitialized = false;
    nginProtocol_t *protocol = NULL;
    int ret = 1;
    NGEM_FNAME(main);

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

    /* Log */
    nResult = ngemLogInitialize("Information Service NRF", logFileName);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(NULL, NGIN_LOGCAT_NRF, fName,
            "Can't initialize the log module.\n");
        goto finalize;
    }
    logInitialized = true;
    log = ngemLogGetDefault();

    /* Callback Manager */
    nResult = ngemCallbackManagerInitialize();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't initialize the Callback manager.\n");
        goto finalize;
    }
    callbackInitialized = true;

    /* Protocol */
    protocol = nginProtocolCreate(&nginlActions, NULL);
    if (protocol == NULL) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't create the Protocol.\n");
        goto finalize;
    }

    /* Run */
    result = ngemCallbackManagerRun();
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Error is occurred in running this program.\n");
        goto finalize;
    }

    ret = 0;
finalize:

    if (protocol != NULL) {
        result = nginProtocolDestroy(protocol);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't destroy the protocol.\n");
            ret = 1;
        }
        protocol = NULL;
    }

    if (callbackInitialized) {
        callbackInitialized = false;
        result = ngemCallbackManagerFinalize();
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't finalize the callback manager.\n");
            ret = 1;
        }
    }

    if (logInitialized) {
        logInitialized = false;
        nResult = ngemLogFinalize();
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName,
                "Can't finalize the log module.\n");
            ret = 1;
        }
    }

    return 0;
}

static int
nginlNRFqueryBegin(
    nginProtocol_t *protocol,
    ngemOptionAnalyzer_t *analyzer)
{
    static int identifyBase = 0;

    if (identifyBase == INT_MAX) {
        /* TODO: overflow */
        identifyBase = 0;
    }

    return identifyBase++;
}

static ngemResult_t
nginlNRFqueryEnd(
    nginProtocol_t *protocol,
    int identify,
    nginQueryREIoptions_t *opts, 
    ngemResult_t cResult)
{
    NGEM_LIST_ITERATOR_OF(char) it1;
    char *source;
    nginREIcontainer_t *container = NULL;
    nginRemoteExecutableInformation_t *reInfo = NULL;
    ngemResult_t nResult;
    ngLog_t *log = NULL;
    char *errorMessage = NULL;
    char *message = NULL;
    ngemStringBuffer_t sBuf;
    bool sendNotify = false;
    bool fatal = false;
    bool sBufInitialized = false;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(nginlNRFqueryEnd);

    log = ngemLogGetDefault();

    if (cResult == NGEM_FAILED) {
        return NGEM_FAILED;
    }

    nResult = ngemStringBufferInitialize(&sBuf);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGIN_LOGCAT_NRF, fName,
            "Can't initialize the string buffer.\n");
        fatal = true;
        goto finalize;
    }
    sBufInitialized = true;

    /* Find */
    container = NULL;
    NGEM_LIST_FOREACH(char, &opts->ngqo_sources, it1, source) {
	ngLogInfo(log, NGIN_LOGCAT_NRF, fName, "Reads Ninf-G Remote Information File.\n");
	container = nginReadNRF(source);
	if (container == NULL) {
	    nResult = ngemStringBufferFormat(&sBuf,
		"%s: Can't read Ninf-G Remote information File.\n", source);
	    if (nResult != NGEM_SUCCESS) {
		ngLogError(log, NGIN_LOGCAT_NRF, fName,
		    "Can't append error message to the string buffer.\n");
		fatal = true;
		goto finalize;
	    }
	    continue;
	}

        reInfo = nginREIcontainerFind(
            container, opts->ngqo_hostname, opts->ngqo_classname);
        if (reInfo == NULL) {
	    nResult = nginREIcontainerDestroy(container);
	    if (nResult != NGEM_SUCCESS) {
		ngLogError(log, NGIN_LOGCAT_NRF, fName,
		    "Can't destroy REI container.\n");
		fatal = true;
		ret = NGEM_FAILED;
		goto finalize;
	    }
	    container = NULL;

            nResult = ngemStringBufferFormat(&sBuf,
                "%s: Doesn't exist  Remote Executable Information "
                "whose hostname is \"%s\" and classname \"%s\".\n",
                source, opts->ngqo_hostname, opts->ngqo_classname);
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName,
                    "Can't append error message to the string buffer.\n");
                fatal = true;
                goto finalize;
            }
            continue;
        }

        sendNotify = true;
        nResult = nginProtocolSendInformationNotify(
            protocol, identify, reInfo->ngrei_xmlString);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't send notify.\n");
            fatal = true;
            ret = NGEM_FAILED;
            goto finalize;
        }
	break;
    }

finalize:
    if (container != NULL) {
	nResult = nginREIcontainerDestroy(container);
	if (nResult != NGEM_SUCCESS) {
	    ngLogError(log, NGIN_LOGCAT_NRF, fName,
		"Can't destroy REI container.\n");
	    fatal = true;
	    ret = NGEM_FAILED;
	    goto finalize;
	}
    }
    if (!sendNotify) {
        if (fatal) {
            message = "Fatal Error.";
        } else {
            errorMessage = ngemStringBufferRelease(&sBuf);
            if (errorMessage == NULL) {
                ngLogError(log, NGIN_LOGCAT_NRF, fName,
                    "Can't release the string.\n");
                message = "Fatal Error.";
            } else {
                message = errorMessage;
            }
        }
        ngLogError(log, NGIN_LOGCAT_NRF, fName, "%s\n", message);

        sendNotify = true;
        nResult = nginProtocolSendInformationNotifyFailed(
            protocol, identify, message);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGIN_LOGCAT_NRF, fName, "Can't send notify.\n");
            ret = NGEM_FAILED;
        }
    }

    ngemStringBufferFinalize(&sBuf);

    ngiFree(errorMessage, log, NULL);

    return ret;
}

static ngemResult_t
nginlNRFcancelQuery(
    nginProtocol_t *protocol,
    int cResult)
{
    /* Do Nothing */
    return NGEM_SUCCESS;
}

