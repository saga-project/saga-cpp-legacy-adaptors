/*
 * $RCSfile: ngclProtocol.c,v $ $Revision: 1.31 $ $Date: 2008/03/28 09:26:27 $
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
 * Module of Protocol manager for Ninf-G Client.
 */

#define NGI_REQUIRE_ARCHITECTURE_IDS
#include "ng.h"
#undef NGI_REQUIRE_ARCHITECTURE_IDS

NGI_RCSID_EMBED("$RCSfile: ngclProtocol.c,v $ $Revision: 1.31 $ $Date: 2008/03/28 09:26:27 $")

/**
 * Prototype declaration of static functions.
 */
static int ngcllCallbackNegotiationFromExecutable(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllCallbackNegotiationFromExecutableFirstConnect(
    ngclContext_t *, ngiProtocol_t *, ngcliJobManager_t *,
    ngiProtocolNegotiationFromExecutable_t *, ngclExecutable_t **,
    int *, int *, int *, int *, int *);
static int ngcllCallbackNegotiationFromExecutableSecondConnect(
    ngclContext_t *, ngiProtocol_t *, ngcliJobManager_t *,
    ngiProtocolNegotiationFromExecutable_t *, ngclExecutable_t **,
    int *, int *, int *, int *, int *);

static int ngcllCallbackNegotiationResult(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllCallbackNegotiationResultFirstConnect(
    ngclContext_t *, ngclExecutable_t *, int *);
static int ngcllCallbackNegotiationResultSecondConnect(
    ngclContext_t *, ngclExecutable_t *, int *);

static int ngcllCallbackReadChunk(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);
static int ngcllCallbackProtocol(
    void *, ngiIOhandle_t *, ngiIOhandleState_t, ngLog_t *, int *);

static int ngcllProtocolProcess(
    ngclContext_t *, ngclExecutable_t *, ngiProtocol_t *,
    ngiProtocolHeader_t *, int *, int *, ngLog_t *, int *);
static int ngcllProtocolProcessReply(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, int *, ngLog_t *, int *);
static int ngcllProtocolProcessNotify(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyQueryFunctionInformation(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#if 0 /* Temporary comment out */
static int ngcllProtocolReplyQueryExecutableInformation(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
#endif
static int ngcllProtocolReplyResetExecutable(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolReplyExitExecutable(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, ngLog_t *, int *);

static int ngcllProtocolReplyInvokeSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, int *, ngLog_t *, int *);

static int ngcllProtocolRequestTransferArgumentData(
    ngclSession_t *, ngiProtocol_t *, int *, int *, ngLog_t *, int *);
static int ngcllProtocolReplyTransferArgumentData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, ngLog_t *, int *);

static int ngcllProtocolReplyTransferResultData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyCancelSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolRequestPullBackSession(
    ngclSession_t *, ngiProtocol_t *, ngLog_t *, int *);
static int ngcllProtocolReplyPullBackSession(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolReplyTransferCallbackArgumentData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, int *, ngLog_t *, int *);
static int ngcllProtocolReplyTransferCallbackResultData(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, ngLog_t *, int *);

static int ngcllProtocolNotifyCalculationEnd(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolNotifyInvokeCallback(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);

static int ngcllProtocolNotifyIamAlive(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, ngLog_t *, int *);
static int ngcllProtocolReplyConnectionClose(
    ngclContext_t *, ngclExecutable_t *, ngclSession_t *,
    ngiProtocol_t *, ngiProtocolHeader_t *, int *, ngLog_t *, int *);

/**
 * Callback function for accept.
 */
int
ngcliCallbackAccept(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    int result;
    ngLog_t *log;
    int callbackEnd;
    int *error, errorEntity;
    ngiProtocol_t *protocol, *newProto;
    ngiCommunication_t *comm, *newComm;
    ngclContext_t *context;
    static const char fName[] = "ngcliCallbackAccept";

    /* Check and get the arguments */
    assert(cbArg != NULL);
    protocol = (ngiProtocol_t *)cbArg;
    comm = protocol->ngp_communication;
    assert(ioHandle != NULL);
    assert(ioHandle == comm->ngc_ioHandle);
    callbackEnd = 1;
    newProto = NULL;
    newComm = NULL;

    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    log = NULL;

    /* Get the Ninf-G Context */
    context = ngiProtocolGetUserData(protocol, log, error);
    if (context == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Ninf-G Context is NULL.\n"); 
	return 0;
    }
    log = context->ngc_log;
    
    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback handle was closed.\n"); 

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback was canceled.\n"); 
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Invalid handle state %d.\n", argState); 
            goto error;
        }

        goto finish2;
    }

    /* log */
    ngclLogDebugContext(context, NG_LOGCAT_NINFG_PURE, fName,  
        "New TCP connection request.\n"); 

    /* Construct the new Communication */
    newComm = ngiCommunicationConstructAccept(
        context->ngc_event, comm, log, error);
    if (newComm == NULL) {
	if (*error == NG_ERROR_DISCONNECT) {
	    goto finish;
        }

    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't construct the Communication.\n"); 
	goto error;
    }

    /* Construct the new Protocol */
    newProto = ngiProtocolConstruct(
    	&protocol->ngp_attr, newComm, context->ngc_event,
        ngcliProtocolGetRemoteMethodInformation, log, error);
    if (newProto == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't construct the Protocol.\n"); 
	goto error;
    }

    /* Register the User Data */
    result = ngiProtocolRegisterUserData(
    	newProto, context, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't register the User Data to Protocol.\n"); 
	goto error;
    }

    /* Register the callback function for Negotiation Information */
    result = ngiIOhandleReadCallbackRegister(
	newComm->ngc_ioHandle,
        ngcllCallbackNegotiationFromExecutable,
	newProto, log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't register the callback function for Select.\n"); 
	goto error;
    }

finish:
    /* Register the callback function for accept */
    result = ngiIOhandleTCPlistenerCallbackRegister(
    	comm->ngc_ioHandle, ngcliCallbackAccept, protocol, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't register the callback function for Accept.\n"); 
	return 0;
    }
    callbackEnd = 0;

finish2:
    /* Listen callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &context->ngc_protoCallbackWaiter, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Listen callback.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

error:

    /* Destruct Protocol (and Communication) */
    if (newProto != NULL) {
	assert(newComm != NULL);

	result = ngiProtocolDestruct(newProto, log, NULL);
	newProto = NULL;
	newComm = NULL;
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Protocol.\n"); 
	}
    } 

    /* Destruct Communication */
    if (newComm != NULL) {
	result = ngiCommunicationDestruct(newComm, log, NULL);
	newComm = NULL;
	if (result == 0) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Communication.\n"); 
	}
    }

    /* Set the error code */
    result = ngcliContextSetError(context, *error, NULL);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the error code to the Ninf-G Context.\n"); 
    }

    /* Listen callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &context->ngc_protoCallbackWaiter, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Listen callback.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Callback function for Negotiation Information.
 */
static int
ngcllCallbackNegotiationFromExecutable(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    int result;
    int *error, errorEntity;
    ngLog_t *log;
    int lockJobList;
    int lockExecutableList;
    ngclContext_t *context;
    ngiProtocol_t *protocol;
    ngcliJobManager_t *jobMng;
    ngclExecutable_t *executable;
    ngiProtocolNegotiationFromExecutable_t *nego;
    int protocolRegistered, callbackEnd;
    int isLocal = 0;
    static const char fName[] = "ngcllCallbackNegotiationFromExecutable";

    /* Check and get the arguments */
    assert(cbArg != NULL);
    protocol = (ngiProtocol_t *)cbArg;
    assert(ioHandle != NULL);
    assert(ioHandle == protocol->ngp_communication->ngc_ioHandle);

    /* Initialize variable */
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    lockJobList = 0;
    lockExecutableList = 0;
    protocolRegistered = 0;
    callbackEnd = 0; /* Executable is unknown at first. */
    jobMng = NULL;
    executable = NULL;
    nego = NULL;
    log = NULL;

    /* Get the Ninf-G Context */
    context = ngiProtocolGetUserData(protocol, NULL, error);
    if (context == NULL) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Ninf-G Context is NULL.\n"); 
	goto error2;
    }
    log = context->ngc_log;

    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback handle was closed.\n"); 

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback was canceled.\n"); 
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Invalid handle state %d.\n", argState); 
            goto error;
        }

        goto finish;
    }

    /* Get the Negotiation Information */
    nego = ngiProtocolReceiveNegotiationFromExecutable(
    	protocol, log, error);
    if (nego == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't receive the Negotiation Information.\n"); 
	goto error2;
    }

    /**
     * Check the Negotiation Information.
     */

    /* Set the Version Number of a partner's Protocol */
    result = ngiProtocolSetProtocolVersionOfPartner(
	protocol, nego->ngpnfe_protocolVersion, log, error);
    if (result == 0) {
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the version number of a partner's protocol.\n"); 
	goto error;
    }

    /* Check the Ninf-G Context */
    if (nego->ngpnfe_contextID != context->ngc_ID) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Context ID of received %ld and Ninf-G Context %d are difference.\n",
            nego->ngpnfe_contextID, context->ngc_ID); 
	goto error;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListWriteLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Job Manager.\n"); 
        goto error;
    }
    lockJobList = 1;

    /* Lock the list of Executable */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        goto error;
    }
    lockExecutableList = 1;

    /* Get the Job Manager */
    jobMng = ngcliContextGetJobManager(context, nego->ngpnfe_jobID, error);
    if (jobMng == NULL) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Job Manager is not found by ID %ld.\n", nego->ngpnfe_jobID); 
	goto error;
    }

    if (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_commProxyType != NULL) {
        result = ngiCommunicationPeerIsLocalhost(protocol->ngp_communication,
            &isLocal, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get whether peer is localhost or not.\n"); 
            goto error;
        }
        if (isLocal == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "The executable uses the Communication Proxy,"
                " however it is connected from remote host.\n");
            goto error;
        }
    }

    /* Check the Simple Auth Number */
    if ((nego->ngpnfe_simpleAuthNumber < NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN) ||
        (nego->ngpnfe_simpleAuthNumber > NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX) ||
    	(nego->ngpnfe_simpleAuthNumber != jobMng->ngjm_simpleAuthNumber)) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Simple Auth Number mismatch.\n"); 

	/* log can be output for attack detection. */
	/**
         * Note : No context error must be set, because,
         * attack must be just ignored.
         */
	goto error;
    }

    /* First connect? or continue? */
    if (nego->ngpnfe_connectCount == 0) {
        result = ngcllCallbackNegotiationFromExecutableFirstConnect(
            context, protocol, jobMng, nego, &executable,
            &protocolRegistered, &callbackEnd,
            &lockJobList, &lockExecutableList,  error);
        if (result == 0) {
    	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	        "Failed to process first connect negotiation.\n"); 
            goto error;
        }
    } else if (nego->ngpnfe_connectCount > 0) {
        result = ngcllCallbackNegotiationFromExecutableSecondConnect(
            context, protocol, jobMng, nego, &executable,
            &protocolRegistered, &callbackEnd,
            &lockJobList, &lockExecutableList,  error);
        if (result == 0) {
    	    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	        "Failed to process second connect negotiation.\n"); 
            goto error;
        }
    } else {
    	/* Failed */
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Invalid connect count %ld.\n", nego->ngpnfe_connectCount); 
	goto error;
    }

    if (executable == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Failed to get executable.\n"); 
        goto error;
    }

    /* Unlock the list of Executable */
    assert(lockExecutableList != 0);
    result = ngclExecutableListWriteUnlock(context, error);
    lockExecutableList = 0;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        goto error;
    }

    /* Unlock the list of Job Manager */
    assert(lockJobList != 0);
    result = ngcliContextJobManagerListWriteUnlock(context, log, error);
    lockJobList = 0;
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Job Manager.\n"); 
        goto error;
    }

    /* Register the read chunk callback. */
    result = ngiIOhandleReadChunkCallbackRegister(
        ioHandle,
        ngcllCallbackReadChunk,
        executable,
        log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't register the read chunk callback function.\n"); 
	goto error;
    }

    /* Register the callback for Negotiation Result */
    result = ngiIOhandleReadCallbackRegister(
    	ioHandle,
    	ngcllCallbackNegotiationResult,
    	executable,
	log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't register the callback function for Negotiation Result.\n"); 
	goto error;
    }
    callbackEnd = 0;

finish:
    /* Release the Negotiation Information */
    if (nego != NULL) {
        result = ngiProtocolReleaseData(nego, log, error);
        if (result == 0) {
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Negotiation Information.\n"); 
        }
        nego = NULL;
    }

    /* Destruct Protocol */
    if (protocolRegistered == 0) {
	result = ngiProtocolDestruct(protocol, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Protocol.\n"); 
	}
    } 

    /* I/O callback end */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
        }
    }

    /* Success */
    return 1;


    /* Error occurred */
error:

    /* Unlock the list of Executable */
    if (lockExecutableList != 0) {
        lockExecutableList = 0;
        result = ngclExecutableListWriteUnlock(context, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Executable.\n"); 
        }
    }

    /* set Unusable to Executable */
    if (executable != NULL) {
	result = ngcliExecutableUnusable(executable, NG_ERROR_PROTOCOL, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Something wrong was happened in negotiation.\n"); 
        }
    }

    /* Unlock the list of Job Manager */
    if (lockJobList != 0) {
        lockJobList = 0;
        result = ngcliContextJobManagerListWriteUnlock(context, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the list of Job Manager.\n"); 
        }
    }

    /* Set the error code */
    result = ngcliContextSetError(context, *error, NULL);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the error code to the Ninf-G Context.\n"); 
    }

error2:
    /* Release the Negotiation Information */
    if (nego != NULL) {
        result = ngiProtocolReleaseData(nego, log, NULL);
        if (result == 0) {
            ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't release the Negotiation Information.\n"); 
        }
        nego = NULL;
    }

    /* Destruct Protocol */
    if (protocolRegistered == 0) {
	result = ngiProtocolDestruct(protocol, log, NULL);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the Protocol.\n"); 
	}
    } 

    /* I/O callback end */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Callback function for Negotiation Information for first connection.
 * Note: The Lock for JobManager and Executable List must be performed
 *     before call this function.
 */
static int
ngcllCallbackNegotiationFromExecutableFirstConnect(
    ngclContext_t *context,
    ngiProtocol_t *protocol,
    ngcliJobManager_t *jobMng,
    ngiProtocolNegotiationFromExecutable_t *nego,
    ngclExecutable_t **foundExecutable,
    int *protocolRegistered,
    int *callbackEnd,
    int *lockJobList,
    int *lockExecutableList,
    int *error)
{
    int result;
    ngLog_t *log;
    int nNegoOpts;
    long negoOpt[NGI_PROTOCOL_NEGOTIATION_OPTION_SIZE_MAX];
    ngiCommunication_t *comm;
    ngclExecutable_t *executable;
    ngiByteStreamConversion_t conv;
    static const char fName[] =
        "ngcllCallbackNegotiationFromExecutableFirstConnect";

    /* Check the arguments */
    assert(context != NULL);
    assert(protocol != NULL);
    assert(jobMng != NULL);
    assert(nego != NULL);
    assert(foundExecutable != NULL);
    assert(protocolRegistered != NULL);
    assert(callbackEnd != NULL);
    assert(lockJobList != NULL);
    assert(lockExecutableList != NULL);

    assert(protocol->ngp_communication);
    comm = protocol->ngp_communication;

    log = context->ngc_log;
    *foundExecutable = NULL;
    *protocolRegistered = 0;
    *callbackEnd = 0;

    /* Set the end time */
    result = ngcliJobSetEndTimeOfInvoke(jobMng, log, error);
    if (result == 0) {
        ngcliLogErrorJob(jobMng, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End time.\n"); 
    }

    /* Get the Executable */
    executable = ngcliJobGetNextExecutable(jobMng, NULL, error);
    if (executable == NULL) {
	ngclLogWarnContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "No executable is registered.\n"); 
	goto error;
    }
    while (executable->nge_status != NG_EXECUTABLE_STATUS_INITIALIZED) {
	executable = ngcliJobGetNextExecutable(jobMng, executable, error);
	if (executable == NULL) {
	    ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "No executable is found.\n"); 
	    goto error;
	}
    }
    *foundExecutable = executable;

    /* I/O Callback Start */
    result = ngiIOhandleCallbackWaiterStart(
        &executable->nge_ioCallbackWaiter, log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start Executable I/O callback. \n"); 
        goto error;
    }
    *callbackEnd = 1;

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
	nego->ngpnfe_architectureID, NULL, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "The architecture code %lu is not valid.\n",
            nego->ngpnfe_architectureID); 
        goto error;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfe_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol Version is not equal. My version is %#x and partner's version is %#lx.\n",
            NGI_PROTOCOL_VERSION, nego->ngpnfe_protocolVersion); 
	/* This is not a error. Continue the process. */
    }

    /* Is Connect Count valid? */
    if ((nego->ngpnfe_connectCount != 0) ||
        (protocol->ngp_connectCount != 0)) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "The connect count is not zero. Client=%d, Executable=%ld.\n",
            protocol->ngp_connectCount, nego->ngpnfe_connectCount); 
        goto error;
    }

    assert(executable->nge_connecting == 0);
    executable->nge_connecting = 1;

    /* Register the Communication Log */
    if (executable->nge_commLog != NULL) {
        result = ngiCommunicationLogRegister(
            comm, executable->nge_commLog, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't register the Communication Log.\n"); 
            goto error;
        }
    }

    /* Check forceXDR. if it's set, set architecture ID to 0 */
    if (jobMng->ngjm_attr.ngja_forceXDR) {
        result = ngiProtocolSetArchitectureID(
            protocol, NGI_ARCHITECTURE_UNDEFINED, log, error);
        if (result == 0) {
            ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't set the architecture ID.\n"); 
            goto error;
        }
    }

    /* Is architecture same? */
    if ((nego->ngpnfe_architectureID == NGI_ARCHITECTURE_ID) &&
       (nego->ngpnfe_architectureID != NGI_ARCHITECTURE_UNDEFINED) &&
       (protocol->ngp_attr.ngpa_architecture != NGI_ARCHITECTURE_UNDEFINED)) {
	result = ngiProtocolSetXDR(protocol, NG_XDR_NOT_USE, log, error);
	if (result == 0) {
	    ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't set the XDR operation.\n"); 
	    goto error;
	}
    }

    /* Set the Simple Auth Number */
    result = ngiProtocolSetSimpleAuthNumber(
        protocol, jobMng->ngjm_simpleAuthNumber, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the Simple Auth Number.\n"); 
	goto error;
    }

    /* Set the Job ID */
    result = ngiProtocolSetJobID(
        protocol, jobMng->ngjm_ID, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the Job ID.\n"); 
	goto error;
    }

    /* Set the Executable ID */
    result = ngiProtocolSetExecutableID(
    	protocol, executable->nge_ID, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the Executable ID.\n"); 
	goto error;
    }

    /* Set the Keep Connect */
    result = ngiProtocolSetKeepConnect(
        protocol, executable->nge_keepConnect, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the Keep Connect.\n"); 
	goto error;
    }

    /* Connecting */
    result = ngcliExecutableConnecting(executable, protocol, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Executable can't process the Connecting.\n"); 
	goto error;
    }
    *protocolRegistered = 1;

    /* Set TCP_NODELAY socket option */
    result = ngiCommunicationSetTcpNodelay(protocol->ngp_communication,
	jobMng->ngjm_attr.ngja_rmInfo.ngrmi_tcpNodelay,
	log, error);
    if (result == 0) {
	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set TCP_NODELAY option.\n"); 
	goto error;
    }

    /* Set the Conversion Method */
    if (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionType ==
            NG_COMPRESSION_TYPE_ZLIB) {
        conv.ngbsc_zlib = 1;
        conv.ngbsc_zlibThreshold =
            jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionThresholdNbytes;
    } else {
        conv.ngbsc_zlib = 0;
        conv.ngbsc_zlibThreshold = 0;
    }
    conv.ngbsc_argumentBlockSize = 
        jobMng->ngjm_attr.ngja_rmInfo.ngrmi_argumentBlockSize;

    result = ngiProtocolSetConversionMethod(
        protocol, &conv,
        &nego->ngpnfe_conversionMethod[0], nego->ngpnfe_nConversions,
        log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Conversion Method.\n"); 
        goto error;
    }

    /* Reply the result to Ninf-G Executable */
    result = ngiProtocolSendNegotiationResult(
    	protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't reply the result of Negotiation.\n"); 
	goto error;
    }

    nNegoOpts = 0;
    if (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionType ==
	NG_COMPRESSION_TYPE_RAW) {
        negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_RAW;
        negoOpt[nNegoOpts++] = 0;
    } else if (jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionType ==
	NG_COMPRESSION_TYPE_ZLIB) {
	/* Initialize the Negotiation Options */
	negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_ZLIB;
	negoOpt[nNegoOpts++] = 1;
	negoOpt[nNegoOpts++] =
	    jobMng->ngjm_attr.ngja_rmInfo.ngrmi_compressionThresholdNbytes;
    }
    negoOpt[nNegoOpts++] = NGI_BYTE_STREAM_CONVERSION_DIVIDE;
    negoOpt[nNegoOpts++] = 1;
    negoOpt[nNegoOpts++] =
        jobMng->ngjm_attr.ngja_rmInfo.ngrmi_argumentBlockSize;

    assert(nNegoOpts <= NGI_PROTOCOL_NEGOTIATION_OPTION_SIZE_MAX);

    /* Send the Negotiation Information to Ninf-G Executable */
    result = ngiProtocolSendNegotiationFromClient(
	protocol, &negoOpt[0], nNegoOpts, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the Negotiation Information.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unregister the Communication Log */
    if ((executable != NULL) && (executable->nge_commLog != NULL)) {
        result = ngiCommunicationLogUnregister(comm, log, NULL);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't register the Communication Log.\n"); 
        }
    }

    /* Failed */
    return 0;
}
 
/**
 * Callback function for Negotiation Information for second connection.
 * Note: The Lock for JobManager and Executable List must be performed
 *    before  call this function.
 */
static int
ngcllCallbackNegotiationFromExecutableSecondConnect(
    ngclContext_t *context,
    ngiProtocol_t *tmpProtocol,
    ngcliJobManager_t *jobMng,
    ngiProtocolNegotiationFromExecutable_t *nego,
    ngclExecutable_t **foundExecutable,
    int *protocolRegistered,
    int *callbackEnd,
    int *lockJobList,
    int *lockExecutableList,
    int *error)
{
    ngLog_t *log;
    int result, exeID;
    ngiProtocol_t *protocol;
    ngiCommunication_t *comm;
    ngclExecutable_t *executable;
    static const char fName[] =
        "ngcllCallbackNegotiationFromExecutableSecondConnect";

    /* Check the arguments */
    assert(context != NULL);
    assert(tmpProtocol != NULL);
    assert(jobMng != NULL);
    assert(nego != NULL);
    assert(foundExecutable != NULL);
    assert(protocolRegistered != NULL);
    assert(callbackEnd != NULL);
    assert(lockJobList != NULL);
    assert(lockExecutableList != NULL);

    log = context->ngc_log;
    protocol = NULL;
    executable = NULL;
    exeID = NGI_EXECUTABLE_ID_UNDEFINED;

    *foundExecutable = NULL;
    *protocolRegistered = 0;
    *callbackEnd = 0;

    /* Get the Executable */
    executable = ngcliJobGetNextExecutable(jobMng, NULL, error);
    if (executable == NULL) {
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "No executable is registered.\n"); 
        goto error;
    }
    while (executable->nge_ID != nego->ngpnfe_executableID) {
        executable = ngcliJobGetNextExecutable(jobMng, executable, error);
        if (executable == NULL) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "No executable is found.\n"); 
            goto error;
        }
    }
    exeID = executable->nge_ID;

    /**
     * Note: Unlock the Executable and Job list.
     *   Because, waiting IO Callback End must be done.
     *   It will returned after the Transfer Argument Data reply arrived.
     *   For the reply callback, ngclSessionGet() must be called and
     *   it requires Executable list lock.
     */

    /* Unlock the list of Executable */
    *lockExecutableList = 0;
    result = ngclExecutableListWriteUnlock(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Executable.\n");
        goto error;
    }

    /* Unlock the list of Job Manager */
    *lockJobList = 0;
    result = ngcliContextJobManagerListWriteUnlock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't unlock the list of Job Manager.\n");
        goto error;
    }

    /* Wait I/O Callback End */
    result = ngiIOhandleCallbackWaiterWait(
        &executable->nge_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't wait Executable I/O callback. \n"); 
        goto error;
    }

    /* Lock the list of Job Manager */
    result = ngcliContextJobManagerListWriteLock(context, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Job Manager.\n");
        goto error;
    }
    *lockJobList = 1;

    /* Lock the list of Executable */
    result = ngclExecutableListWriteLock(context, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't lock the list of Executable.\n");
        goto error;
    }
    *lockExecutableList = 1;

    executable = ngcliJobGetExecutable(jobMng, exeID, error);
    if (executable == NULL) {
        ngclLogWarnContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "No executable is registered.\n"); 
        goto error;
    }
    *foundExecutable = executable;

    assert(executable->nge_connectionCloseRequested == 0);

    /* I/O Callback Start */
    result = ngiIOhandleCallbackWaiterStart(
        &executable->nge_ioCallbackWaiter, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start Executable I/O callback. \n"); 
        goto error;
    }
    *callbackEnd = 1;

    /* Use old protocol. tmpProtocol is removed. */
    protocol = executable->nge_protocol;
    *protocolRegistered = 0;

    /* Is connect count valid? */
    if (nego->ngpnfe_connectCount != protocol->ngp_connectCount) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Connection count mismatch. Client %d, Executable %ld.\n",
            protocol->ngp_connectCount, nego->ngpnfe_connectCount); 
        goto error;
    }

    /* Is architecture ID valid? */
    result = ngiProtocolIsArchitectureValid(
        nego->ngpnfe_architectureID, NULL, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "The architecture code %lu is not valid.\n",
            nego->ngpnfe_architectureID); 
        goto error;
    }

    /* Is protocol version valid? */
    if (NGI_PROTOCOL_VERSION_IS_NOT_EQUAL( 
        nego->ngpnfe_protocolVersion, NGI_PROTOCOL_VERSION)) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol Version is not equal. My version is %#x and partner's version is %#lx.\n",
            NGI_PROTOCOL_VERSION, nego->ngpnfe_protocolVersion); 
        /* This is not a error. Continue the process. */
    }

    comm = tmpProtocol->ngp_communication;

    /* Clear the communication on temporary protocol. */
    result = ngiProtocolSetCommunication(
        tmpProtocol, NULL, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Communication to protocol. \n"); 
        goto error;
    }

    /* Set the communication to protocol. */
    result = ngiProtocolSetCommunication(
        protocol, comm, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Communication to protocol. \n"); 
        goto error;
    }

    /* Register the Communication Log */
    if (executable->nge_commLog != NULL) {
        result = ngiCommunicationLogRegister(
            comm, executable->nge_commLog, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't register the Communication Log.\n"); 
            goto error;
        }
    }

    assert(executable->nge_connecting == 0);
    executable->nge_connecting = 1;

    /* Initialize the IDs  */
    ngiProtocolInitializeID(protocol);

    /* Set TCP_NODELAY socket option */
    result = ngiCommunicationSetTcpNodelay(protocol->ngp_communication,
        jobMng->ngjm_attr.ngja_rmInfo.ngrmi_tcpNodelay,
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set TCP_NODELAY option.\n"); 
        goto error;
    }

    /* Reply the result to Ninf-G Executable */
    result = ngiProtocolSendNegotiationResult(
        protocol, NGI_PROTOCOL_RESULT_OK, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't reply the result of Negotiation.\n"); 
        goto error;
    }

    /* Send the Negotiation Information to Ninf-G Executable */
    result = ngiProtocolSendNegotiationFromClient(
        protocol, NULL, 0, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't send the Negotiation Information.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}



#if 0 /* Is this necessary? Use ngiProtocolIsArchitectureValid instead */
/**
 * Is architecture ID valid?
 */
static int
ngcllProtocolIsArchitectureValid(long archID, ngLog_t *log, int *error)
{
    int i;
    static const char fName[] = "ngcllProtocolIsArchitectureValid";

    for (i = 0; i < NGI_NARCHITECTUREIDS; i++) {
    	if (ngiArchitectureIDs[i] == archID) {
	    /* It is defined */
	    return 1;
	}
    }

    /* The architecture ID is not defined */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "The architecture ID %d is not defined.\n"); 

    return 0;
}
#endif

/**
 * Callback function for Negotiation Result.
 */
static int
ngcllCallbackNegotiationResult(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    int result;
    int *error, errorEntity;
    ngclContext_t *context;
    ngLog_t *log = NULL;
    ngclExecutable_t *executable;
    ngiProtocol_t *protocol;
    int callbackEnd;
    ngiProtocolNegotiationResult_t negoResult;
    static const char fName[] = "ngcllCallbackNegotiationResult";

    /* Initialize variable */
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    callbackEnd = 1;

    /* Check and get the arguments */
    assert(cbArg != NULL);
    executable = (ngclExecutable_t *)cbArg;
    assert(executable->nge_context != NULL);
    context = executable->nge_context;
    assert(executable->nge_protocol != NULL);
    protocol = executable->nge_protocol;
    assert(protocol->ngp_communication != NULL);
    assert(ioHandle != NULL);
    assert(ioHandle == protocol->ngp_communication->ngc_ioHandle);


    /* Is Executable valid */
    result = ngcliExecutableIsValid(context, executable, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable is not valid.\n"); 
	return 0;
    }

    /* Get the log */
    log = context->ngc_log;

    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback handle was closed.\n"); 

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Callback was canceled.\n"); 
        } else {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Invalid handle state %d.\n", argState); 
            goto error2;
        }

        goto finish;
    }

    /* Get the Negotiation Information */
    result = ngiProtocolReceiveNegotiationResult(
    	protocol, &negoResult, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't receive the Negotiation Result.\n"); 

        /* Failed */
        goto error2;
    }

    /* Is negotiation success? */
    if (negoResult.ngpnr_result != NGI_PROTOCOL_RESULT_OK) {
    	/* Failed */
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Negotiation failed.\n"); 
	goto error;
    }


    /* First connect? or continue? */
    if (protocol->ngp_connectCount == 0) {
        result = ngcllCallbackNegotiationResultFirstConnect(
            context, executable, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Failed to process first connect negotiation.\n"); 
            goto error;
        }
    } else if (protocol->ngp_connectCount > 0) {
        result = ngcllCallbackNegotiationResultSecondConnect(
            context, executable, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Failed to process second connect negotiation.\n"); 
            goto error;
        }
    } else {
    	/* Failed */
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Invalid connect count %d.\n", protocol->ngp_connectCount); 
	goto error;
    }

    /* Increment the connect count. */
    result = ngiProtocolConnected(protocol, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't increment the connection count on protocol. \n"); 
        goto error;
    }

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
    	ioHandle, ngcllCallbackProtocol, executable, log, error);
    if (result == 0) {
    	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't register the callback function for Protocol.\n"); 
	goto error;
    }
    callbackEnd = 0;

finish:
    if (callbackEnd != 0) {
        /* I/O callback end */
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, error);
        if (result == 0) {
            /* Error occurred */
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Set the error code */
    result = ngcliContextSetError(context, *error, NULL);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the error code to the Ninf-G Context.\n"); 
    }

error2:
    /* I/O callback end */
    if (callbackEnd != 0) { 
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, NULL);
        if (result == 0) {
            /* Error occurred */
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Callback function for First connect Negotiation Result.
 */
static int
ngcllCallbackNegotiationResultFirstConnect(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    int result;
    ngLog_t *log;
    int doQueryFunctionInformation;
    static const char fName[] = "ngcllCallbackNegotiationResultFirstConnect";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    log = context->ngc_log;

    /* Initialize HeartBeat last arrived time */
    result = ngcliExecutableHeartBeatTimeInitialize(executable, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Executable can't set heartbeat arrived time.\n"); 
        goto error;
    }

    /* Connected */
    result = ngcliExecutableConnected(executable, log, error);
    if (result == 0) {
	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Executable can't process the Connected.\n"); 
	goto error;
    }

    /* Treat Remote Class Information Exist or not */
    result = ngcliExecutableRemoteClassInformationCheck(
        executable, &doQueryFunctionInformation, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't check the RemoteClassInformation.\n"); 
        goto error;
    }

    /* Invoke the waiting session */
    result = ngcliExecutableExecuteSession(executable, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't invoke the waiting Session.\n"); 
        goto error;
    }

    /* Idle */
    if (doQueryFunctionInformation == 0) {
        result = ngcliExecutableIdle(executable, log, error);
        if (result == 0) {
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Executable can't process the Idle.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Callback function for Second Connect Negotiation Result.
 */
static int
ngcllCallbackNegotiationResultSecondConnect(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    int *error)
{
    ngLog_t *log;

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);

    log = context->ngc_log;

    /**
     * Do nothing especially. Just return normal state.
     */

    /* Success */
    return 1;
}


/**
 * Callback function for Read Chunk.
 */
static int
ngcllCallbackReadChunk(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    int result;
    ngLog_t *log;
    int *error, errorEntity;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    static const char fName[] = "ngcllCallbackReadChunk";

    /* Initialize variable */
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    log = NULL;

    /* Check and get the arguments */
    assert(cbArg != NULL);
    executable = (ngclExecutable_t *)cbArg;

    assert(executable != NULL);
    assert(executable->nge_context != NULL);
    context = executable->nge_context;
    log = context->ngc_log;

    /* No log output, because chunk callback is called many times. */
    
    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Read chunk callback handle was closed.\n"); 

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Read chunk callback was canceled.\n"); 
        } else {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Invalid handle state %d.\n", argState); 
            goto error;
        }

        goto finish;
    }

    /* Set the heartbeat */
    result = ngcliExecutableHeartBeatArrive(executable, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
            "Can't Process the heartbeat arrive.\n");
        goto error;
    }

    /* Read chunk callback is not unregistered. */

finish:

    /* Success */
    return 1;
    
    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Callback function for Protocol.
 */
static int
ngcllCallbackProtocol(
    void *cbArg,
    ngiIOhandle_t *ioHandle,
    ngiIOhandleState_t argState,
    ngLog_t *argLog,
    int *argError)
{
    int result;
    int received;
    ngLog_t *log;
    int finalCallback;
    int *error, errorEntity;
    ngiProtocolReceiveMode_t mode;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngiProtocol_t *proto;
    ngiProtocolHeader_t head;
    int callbackEnd;
    static const char fName[] = "ngcllCallbackProtocol";

    /* Initialize variable */
    error = &errorEntity;
    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
    callbackEnd = 1;   /* Is callback end and destructable? */
    finalCallback = 0; /* Is callback register not required? */
    log = NULL;

    /* Check and get the arguments */
    assert(cbArg != NULL);
    executable = (ngclExecutable_t *)cbArg;

    /* Is Executable valid */
    result = ngcliExecutableIsValid(NULL, executable, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Executable is not valid.\n"); 
	return 0;
    }
    assert(executable->nge_context != NULL);
    assert(executable->nge_protocol != NULL);
    context = executable->nge_context;
    proto = executable->nge_protocol;
    assert(proto->ngp_communication != NULL);
    log = context->ngc_log;

    /* Output the log */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Start.\n"); 
    
    if (argState != NGI_IOHANDLE_STATE_NORMAL) {
        if (argState == NGI_IOHANDLE_STATE_CLOSED) {
            ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Callback handle was closed.\n"); 

        } else if (argState == NGI_IOHANDLE_STATE_CANCELED) {
            ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Callback was canceled.\n"); 
        } else {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Invalid handle state %d.\n", argState); 
            goto error;
        }

        goto finish;
    }

    mode = NGI_PROTOCOL_RECEIVE_MODE_WAIT;
    received = -1;

    /* Set the context ID to protocol */
    result = ngiProtocolSetIDofContext(proto, context->ngc_ID,
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set context ID to protocol.\n"); 
        goto error;
    }

    /* Set the executable ID to protocol */
    result = ngiProtocolSetIDofExecutable(proto, executable->nge_ID,
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set executable ID to protocol.\n"); 
        goto error;
    }

    while (1) {
        /* Receive the Protocol */
        result = ngiProtocolReceive(proto, &head, mode, &received,
            log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Receiving protocol failed.\n"); 
            goto error;
        }

	if (received == 0) {
	    break;
        }

        if (mode == NGI_PROTOCOL_RECEIVE_MODE_WAIT) {
            mode = NGI_PROTOCOL_RECEIVE_MODE_NOWAIT;
        }

        /* Process the Protocol */
        result = ngcllProtocolProcess(
            context, executable, proto, &head,
	    &finalCallback, &callbackEnd, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable,
		NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Protocol.\n"); 
            goto error;
        }
 
        /* Release the Session ID of Protocol */
        result = ngiProtocolReleaseIDofSession(proto, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Can't Release the Session ID of Protocol.\n"); 
            goto error;
        }

	/* Is callback register? */
        if (finalCallback != 0) {

	    /* Success */
            goto finish;
	}
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(proto);

    /* Output the log */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "Register the callback.\n"); 

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
	ioHandle, ngcllCallbackProtocol, executable, log, error);
    if (result == 0) {
	ngclLogFatalContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't register the callback function for Protocol.\n"); 
	goto error;
    }
    callbackEnd = 0;

finish:

    /* I/O callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, error);
        if (result == 0) {
            /* Error occurred */
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;
    
    /* Error occurred */
error:
    /* Make the Executable unusable */
    result = ngcliExecutableUnusable(executable, *error, NULL);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Failed to set the executable unusable.\n"); 
    }

    /* Set the error code */
    result = ngcliContextSetError(context, *error, NULL);
    if (result == 0) {
    	ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the error code to the Ninf-G Context.\n"); 
    }

    /* I/O callback End */
    if (callbackEnd != 0) {
        callbackEnd = 0;
        result = ngiIOhandleCallbackWaiterEnd(
            &executable->nge_ioCallbackWaiter, log, NULL);
        if (result == 0) {
            /* Error occurred */
            ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't end the Executable I/O callback.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Protocol
 */
static int
ngcllProtocolProcess(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    int *callbackEnd,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclSession_t *session = NULL;
    static const char fName[] = "ngcllProtocolProcess";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);
    assert(callbackEnd != NULL);

    *finalCallback = 0;

    /* Get the Session */
    if (header->ngph_sessionID != NGI_SESSION_ID_UNDEFINED) {
        session = ngclSessionGet(
            context, executable, header->ngph_sessionID, error);
        if (session == NULL) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Session is not found by ID %ld.\n", header->ngph_sessionID); 
            goto error;
        }
    }

    /* Is Request Type valid? */
    switch (header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT) {
    case NGI_PROTOCOL_REQUEST_TYPE_REPLY:
        result = ngcllProtocolProcessReply(
            context, executable, session, protocol, header,
            finalCallback, callbackEnd, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't process the Reply.\n"); 
            goto error;
        }
        break;

    case NGI_PROTOCOL_REQUEST_TYPE_NOTIFY:
        result = ngcllProtocolProcessNotify(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't process the Notify.\n"); 
            goto error;
        }
        break;

    default:
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Request Type %ld is not valid.\n",
            header->ngph_requestCode >> NGI_PROTOCOL_REQUEST_CODE_NUMSHIFT); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    NGI_SET_CB_ERROR_CONTEXT(context, NG_ERROR_PROTOCOL, NULL);
#if 0 /* Temporary comment out */
    NGI_SET_CB_ERROR_EXECUTABLE(executable, NG_ERROR_PROTOCOL, NULL);
    NGI_SET_CB_ERROR_SESSION(session, NG_ERROR_PROTOOCL, NULL);
#endif /* Temporary comment out */
    return 0;
}

/**
 * Process the Reply.
 */
static int
ngcllProtocolProcessReply(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    int *callbackEnd,
    ngLog_t *log,
    int *error)
{
    int result;
    int reply;

    static const char fName[] = "ngcllProtocolProcessReply";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);
    assert(callbackEnd != NULL);

    /* Get the Reply Code */
    reply = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (reply) {
    case NGI_PROTOCOL_REQUEST_CODE_QUERY_FUNCTION_INFORMATION:
        result = ngcllProtocolReplyQueryFunctionInformation(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Query Function Information.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_QUERY_EXECUTABLE_INFORMATION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESET_EXECUTABLE:
        result = ngcllProtocolReplyResetExecutable(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Reset Executable.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_EXIT_EXECUTABLE:
        result = ngcllProtocolReplyExitExecutable(
            context, executable, session, protocol, header,
            finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Reset Executable.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_INVOKE_SESSION:
        result = ngcllProtocolReplyInvokeSession(
            context, executable, session, protocol, header,
            finalCallback, callbackEnd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Invoke Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_SUSPEND_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_RESUME_SESSION:
        break;

    case NGI_PROTOCOL_REQUEST_CODE_CANCEL_SESSION:
        result = ngcllProtocolReplyCancelSession(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Cancel Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_PULL_BACK_SESSION:
        result = ngcllProtocolReplyPullBackSession(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Pull Back Session.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_ARGUMENT_DATA:
        result = ngcllProtocolReplyTransferArgumentData(
            context, executable, session, protocol, header,
            finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Transfer Argument Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_RESULT_DATA:
        result = ngcllProtocolReplyTransferResultData(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Transfer Result Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_ARGUMENT_DATA:
        result = ngcllProtocolReplyTransferCallbackArgumentData(
            context, executable, session, protocol, header,
            finalCallback, callbackEnd, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Transfer Callback Argument Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_TRANSFER_CALLBACK_RESULT_DATA:
        result = ngcllProtocolReplyTransferCallbackResultData(
            context, executable, session, protocol, header,
            finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Transfer Callback Result Data.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_REQUEST_CODE_CONNECTION_CLOSE:
        result = ngcllProtocolReplyConnectionClose(
            context, executable, session, protocol, header,
            finalCallback, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Reply Connection Close.\n"); 
            return 0;
        }
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Request Code %d is not valid.\n", reply); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Process the Notify.
 */
static int
ngcllProtocolProcessNotify(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int notify;
    static const char fName[] = "ngcllProtocolProcessNotify";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Get the Reply Code */
    notify = header->ngph_requestCode & NGI_PROTOCOL_REQUEST_CODE_MASK;

    switch (notify) {
    case NGI_PROTOCOL_NOTIFY_CODE_I_AM_ALIVE:
        result = ngcllProtocolNotifyIamAlive(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Notify Heartbeat.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_NOTIFY_CODE_CALCULATION_END:
        assert(session != NULL);
        result = ngcllProtocolNotifyCalculationEnd(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Notify Calculation End.\n"); 
            return 0;
        }
        break;

    case NGI_PROTOCOL_NOTIFY_CODE_INVOKE_CALLBACK:
        result = ngcllProtocolNotifyInvokeCallback(
            context, executable, session, protocol, header, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't Process the Notify Invoke Callback.\n"); 
            return 0;
        }
        break;

    default:
        assert(session != NULL);
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
#if 0 /* Temporary comment out */
        NGI_SET_CB_ERROR_EXECUTABLE(executable, NG_ERROR_PROTOCOL);
        NGI_SET_CB_ERROR_SESSION(session, NG_ERROR_PROTOCOL);
#endif /* Temporary comment out */
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Unknown Notify %#x.\n", notify); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Send request: Query Function Information.
 */
int
ngcliProtocolRequestQueryFunctionInformation(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestQueryFunctionInformation";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: QUERY FUNCTION INFORMATION.\n"); 

    /* Set the request */
    result = ngcliExecutableStatusSet(executable,
        NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED,
        log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestQueryFunctionInformation(protocol, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send the Request.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Query Function Information.
 */
static int
ngcllProtocolReplyQueryFunctionInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    char *rcInfoString;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyQueryFunctionInformation";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(head != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does QUERY FUNCTION INFORMATION requested? */
    if (executable->nge_status !=
        NG_EXECUTABLE_STATUS_QUERY_FUNCTION_INFORMATION_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not requested the QUERY FUNCTION INFORMATION.\n"); 
        goto error;
    }

    /* Get the received Remote Class Information (XML string) */
    if (protocol->ngp_rcInfo == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Remote Class Information was not available.\n"); 
        goto error;
    }
    rcInfoString = protocol->ngp_rcInfo;
    protocol->ngp_rcInfo = NULL;

    /* Register Remote Class Information */
    result = ngcliExecutableRemoteClassInformationArrived(
        executable, rcInfoString, error);
    if (result  == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Register Remote Class Information failed.\n"); 
        goto error;
    }

    /* Release Remote Class Information (XML string) */
    ngiFree(rcInfoString, log, error);

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Invoke the waiting session */
    result = ngcliExecutableExecuteSession(executable, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't invoke the waiting Session.\n"); 
        goto error;
    }

    /* Idle */
    result = ngcliExecutableIdle(executable, log, error);
    if (result == 0) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Executable can't process the Idle.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

#if 0 /* Temporary comment out */
/**
 * Send request: Query Executable Information.
 */
int
ngcliProtocolRequestQueryExecutableInformation(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    /* Success */
    return 1;
}

/**
 * Receive reply: Query Executable Information.
 */
static int
ngcllProtocolReplyQueryExecutableInformation(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *head,
    ngLog_t *log,
    int *error)
{
    int result;
    char *funcInfo;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyQueryExecutableInformation";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(head != NULL);
    assert(head->ngp_nBytes > 0);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does QUERY FUNCTION INFORMATION requested? */
    if (executable->nge_status !=
        NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not requested the " "QUERY EXECUTABLE INFORMATION.\n"); 
        goto error;
    }

    /* Receive the Function Information */
    result = ngiReceiveReplyQueryExecutableInformation(
    	protocol, head, funcInfo, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't receive the Function Information.\n"); 
	goto error;
    }

    /* Parse XML and register to cache */
    result = XXX();
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "\n"); 
	goto error;
    }

    /* Release Function Information */
    result = ngiProtocolRelease(funcInfo, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't release the Function Information.\n"); 
	goto error;
    }

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_QUERY_EXECUTABLE_INFORMATION_DONE,
        log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
	goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Release Function Information */
    result = ngiProtocolRelease(funcInfo, log, NULL);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't release the Function Information.\n"); 
	return 0;
    }

    /* Failed */
    return 0;
}
#endif /* Temporary comment out */

/**
 * Send request: Reset Executable.
 */
int
ngcliProtocolRequestResetExecutable(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestResetExecutable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: RESET EXECUTABLE.\n"); 

    /* Set the request */
    result = ngcliExecutableStatusSet(
    	executable, NG_EXECUTABLE_STATUS_RESET_REQUESTED, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestResetExecutable(protocol, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send the Request.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Reset Executable.
 */
static int
ngcllProtocolReplyResetExecutable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyResetExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Check the Protocol Header */
    if (header->ngph_nBytes != 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol: Parameter length %ld is not zero.\n",
            header->ngph_nBytes); 
	return 0;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does EXECUTABLE RESET requested? */
    if (executable->nge_status != NG_EXECUTABLE_STATUS_RESET_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not requested the EXECUTABLE RESET.\n"); 
        goto error;
    }

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_RESET_DONE, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Exit Executable.
 */
int
ngcliProtocolRequestExitExecutable(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestExitExecutable";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: EXIT EXECUTABLE.\n"); 

    /* Set the request */
    result = ngcliExecutableStatusSet(
    	executable, NG_EXECUTABLE_STATUS_EXIT_REQUESTED, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestExitExecutable(protocol, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send the Request.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Exit Executable.
 */
static int
ngcllProtocolReplyExitExecutable(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyExitExecutable";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    *finalCallback = 1;

    /* Check the Protocol Header */
    if (header->ngph_nBytes != 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol: Parameter length %ld is not zero.\n",
            header->ngph_nBytes); 
        return 0;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does EXECUTABLE EXIT requested? */
    if (executable->nge_status != NG_EXECUTABLE_STATUS_EXIT_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not requested the EXECUTABLE EXIT.\n"); 
        goto error;
    }

    /* Save the result */
    executable->nge_requestResult = header->ngph_result;

    /* Request is done */
    result = ngcliExecutableStatusSet(
        executable, NG_EXECUTABLE_STATUS_EXIT_DONE, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Invoke Session.
 */
int
ngcliProtocolRequestInvokeSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestInvokeSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: INVOKE SESSION.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	return 0;
    }

    /* Invoke Session */
    result = ngiSendRequestInvokeSession(
	protocol, session->ngs_ID, session->ngs_rmInfo.ngrmi_methodID,
        log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Invoke Session.
 */
static int
ngcllProtocolReplyInvokeSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    int *callbackEnd,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyInvokeSession";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);
    assert(callbackEnd != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does INVOKE SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_INVOKE_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the INVOKE SESSION.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: INVOKE SESSION.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
    } else {
        /* Transfer Argument Data */
        result = ngcllProtocolRequestTransferArgumentData(
            session, protocol, finalCallback, callbackEnd, log, error);
    }
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Argument Data.
 */
static int
ngcllProtocolRequestTransferArgumentData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    int *finalCallback,
    int *callbackEnd,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllProtocolRequestTransferArgumentData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);
    assert(session->ngs_arg != NULL);
    assert(finalCallback != NULL);
    assert(callbackEnd != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: TRANSFER ARGUMENT DATA.\n"); 

    /* Start the Transfer Timeout */
    result = ngcliTransferTimeoutTransferStart(
        session, NGCLI_TRANSFER_TIMEOUT_ARGUMENT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Transfer Timeout.\n"); 
	goto error;
    }

    /* Start the measurement */
    result = ngiProtocolSessionInformationStartMeasurement(
	protocol, session->ngs_arg->nga_nArguments, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the measurement.\n"); 
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_transferArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start time.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSARG_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Set the argument to protocol*/
    result = ngiProtocolSetArgument(protocol, session->ngs_arg, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't set the argument.\n"); 
	goto error;
    }

    /**
     * Note: TransferArgumentData may send big data,
     * which will cause long time to send.
     * To receive heartbeat while sending argument data,
     * callback is registered before the TransferArgumentData.
     */

    /* Set the IDs for Transfer */
    result = ngiProtocolTransferAttributeSet(protocol, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Attribute.\n"); 
        goto error;
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(protocol);

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
        protocol->ngp_communication->ngc_ioHandle,
        ngcllCallbackProtocol, session->ngs_executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't register the callback function for Protocol.\n"); 
        goto error;
    }
    *finalCallback = 1;
    *callbackEnd = 0;

    /* Send request */
    result = ngiSendRequestTransferArgumentData(
	protocol, session->ngs_ID, session->ngs_arg, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
	goto error;
    }

    /* Clear the IDs for Transfer */
    result = ngiProtocolTransferAttributeInitialize(protocol, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Attribute.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Argument Data.
 */
static int
ngcllProtocolReplyTransferArgumentData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    int executableLocked;
    int result, commClosed;
    ngiCommunication_t *comm;
    static const char fName[] = "ngcllProtocolReplyTransferArgumentData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    executableLocked = 0;
    commClosed = 0;
    comm = NULL;

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does TRANSFER ARGUMENT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_TRANSARG_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the TRANSFER ARGUMENT DATA.\n"); 
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_transferArgument, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End time.\n"); 
        goto error;
    }

    /* Transfer Timeout Done */
    result = ngcliTransferTimeoutTransferDone(
        session, NGCLI_TRANSFER_TIMEOUT_ARGUMENT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set done the Transfer Timeout.\n"); 
	goto error;
    }

    /* check result */
    if (header->ngph_result == NGI_PROTOCOL_RESULT_BAD_ARGUMENT) {
        /* Unlock the Executable with send */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }

        /* disable the session */
        result = ngcliSessionUnusable(executable, session,
            NG_ERROR_INVALID_ARGUMENT, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Invalid result data.\n"); 
        }

        return 1;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSARG_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Close the Communication */
    if (executable->nge_keepConnect == 0) {
        comm = protocol->ngp_communication;
        assert(comm != NULL);

        /* Clear the communication on protocol. */
        result = ngiProtocolSetCommunication(
            protocol, NULL, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to clear the communication on protocol.\n");
            goto error;
        }

        /* Destruct the Communication */
        result = ngiCommunicationDestruct(comm, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct the communication.\n");
            goto error;
        }

        assert(executable->nge_connecting != 0);
        executable->nge_connecting = 0;
        commClosed = 1;
        *finalCallback = 1;
    }

    /* Is cancel required? */
    if ((commClosed == 0) && (session->ngs_cancelRequest != 0)) {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }

        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't send the request.\n"); 
            goto error;
        }
    } else {
        /* Set the start time */
        result = ngiSetStartTime(
            &session->ngs_executionTime.ngs_calculation, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't set the start time.\n"); 
            goto error;
        }
        
        /* Set the status */
        result = ngcliSessionStatusSet(
       	    session, NG_SESSION_STATUS_CALCULATE_EXECUTING, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't set the status.\n"); 
            goto error;
        }

        /* Unlock the Executable with send */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }
        
    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify Calculation End.
 */
static int
ngcllProtocolNotifyCalculationEnd(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked, sendEnable;
    static const char fName[] = "ngcllProtocolNotifyCalculationEnd";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    executableLocked = 0;
    sendEnable = 0;

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_calculation, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End time.\n"); 
	goto error;
    }

    /* Output the log */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive the Reply Calculation End.\n"); 

    /* Is Connection Close requested? */
    if (executable->nge_connectionCloseRequested != 0) {
        ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "The connection is closing. Ignore CALCULATION END Notify.\n"); 

        goto success;
    }

    /* Is not Session calculating? */
    if (session->ngs_status != NG_SESSION_STATUS_CALCULATE_EXECUTING) {
        ngclLogInfoSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "The status is not CALCULATE EXECUTING.\n"); 

        goto success;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CALCULATE_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Try lock for send */
    result = ngcliExecutableLockTrySend(executable, &sendEnable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock for send.\n"); 
        goto error;
    }

success:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    if (sendEnable != 0) {
        /* Send request */
        result = ngcliProtocolRequestTransferResultData(
            session, protocol, log, error);
       if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the request.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Result Data.
 */
int
ngcliProtocolRequestTransferResultData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestTransferResultData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: TRANSFER RESULT DATA.\n"); 

    /* Start Transfer Timeout */
    result = ngcliTransferTimeoutTransferStart(
        session, NGCLI_TRANSFER_TIMEOUT_RESULT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Transfer Timeout.\n"); 
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_transferResult, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the start time.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSRES_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Send the request */
    result = ngiSendRequestTransferResultData(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Result Data.
 */
static int
ngcllProtocolReplyTransferResultData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyTransferResultData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does TRANSFER RESULT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_TRANSRES_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the TRANSRES.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: TRANSFER RESULT DATA.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_TRANSRES_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
        goto error;
    }

    /* Release the argument from protocol */
    result = ngiProtocolReleaseArgument(executable->nge_protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't release the argument.\n"); 
        goto error;
    }

    /* Destruct the argument */
    if (session->ngs_arg) {
	result = ngiArgumentDestruct(session->ngs_arg, log, error);
	session->ngs_arg = NULL;
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't destruct the argument.\n"); 
	    return 0;
	}
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_transferResult, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the End time.\n"); 
        goto error;
    }

    /* Transfer Timeout Done */
    result = ngcliTransferTimeoutTransferDone(
        session, NGCLI_TRANSFER_TIMEOUT_RESULT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set done the Transfer Timeout.\n"); 
	goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Pull back session */
    result = ngcllProtocolRequestPullBackSession(
        session, protocol, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Cancel Session.
 */
int
ngcliProtocolRequestCancelSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestCancelSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: CANCEL SESSION.\n"); 

    /* Set the request */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CANCEL_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	return 0;
    }

    /* Send request */
    result = ngiSendRequestCancelSession(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Cancel Session.
 */
static int
ngcllProtocolReplyCancelSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyCancelSession";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does CANCEL SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_CANCEL_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the CANCEL.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: TRANSFER ARGUMENT DATA.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CANCEL_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Pull Back Session */
    result = ngcllProtocolRequestPullBackSession(
        session, protocol, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Pull Back Session.
 */
static int
ngcllProtocolRequestPullBackSession(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllProtocolRequestPullBackSession";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: PULL BACK SESSION.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_PULLBACK_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	return 0;
    }

    /* Send the request */
    result = ngiSendRequestPullBackSession(
	protocol, session->ngs_ID, log, error);
    if (result == 0) {
    	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't send the request.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Pull Back Session.
 */
static int
ngcllProtocolReplyPullBackSession(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked = 0;
    static const char fName[] = "ngcllProtocolReplyPullBackSession";

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does PULL BACK SESSION requested? */
    if (session->ngs_status != NG_SESSION_STATUS_PULLBACK_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the PULLBACK SESSION.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: PULLBACK SESSION.\n"); 

    /* Get the Session Information from protocol */
    result = ngiProtocolGetSessionInfo(
        protocol, &session->ngs_info, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get the Session Information from protocol.\n"); 
        goto error;
    }

    /* Finish the measurement */
    result = ngiProtocolSessionInformationFinishMeasurement(
	protocol, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finish the measurement.\n"); 
	goto error;
    }

    /* Set the execution time */
    result = ngcliSessionSetExecutionTime(
        context, executable->nge_jobMng, executable, session, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the Execution time.\n"); 
        goto error;
    }

    /* Session is done */
    result = ngcliSessionStatusSetDone(session, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the status.\n"); 
        goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Process the Notify Invoke Callback.
 */
static int
ngcllProtocolNotifyInvokeCallback(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    int result;
    int executableLocked, sendEnable;
    static const char fName[] = "ngcllProtocolNotifyInvokeCallback";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    executableLocked = 0;
    sendEnable = 0;

    /* Is cancel required? */
    if (session->ngs_cancelRequest != 0) {
        /* Set the callback ID to Protocol */
        result = ngiProtocolSetCallbackID(protocol, NGI_CALLBACK_ID_UNDEFINED,
            log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't set the callback ID.\n"); 
            return 0;
        }

        /* It was canceled. */
        return 1;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive notify: INVOKE CALLBACK.\n"); 

    /* Is Connection Close requested? */
    if (executable->nge_connectionCloseRequested != 0) {
        ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "The connection is closing. Ignore INVOKE CALLBACK Notify.\n"); 

        goto success;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_INVOKE_CALLBACK_RECEIVED, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Increment the number of times to which the callback was called */
    session->ngs_executionTime.ngs_callbackNtimesCalled++;

    /* Try lock for send */
    result = ngcliExecutableLockTrySend(executable, &sendEnable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock for send.\n"); 
        goto error;
    }

success:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    if (sendEnable != 0) {

        /* Send request */
        result = ngcliProtocolRequestTransferCallbackArgumentData(
            session, protocol, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't send the request.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Send request: Transfer Callback Argument Data.
 */
int
ngcliProtocolRequestTransferCallbackArgumentData(
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] =
        "ngcliProtocolRequestTransferCallbackArgumentData";

    /* Check the arguments */
    assert(session != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: TRANSFER CALLBACK ARGUMENT DATA.\n"); 

    /* Start Transfer Timeout */
    result = ngcliTransferTimeoutTransferStart(
        session, NGCLI_TRANSFER_TIMEOUT_CB_ARGUMENT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Transfer Timeout.\n"); 
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackTransferArgument, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the start time.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSARG_REQUESTED, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Send request */
    result = ngiSendRequestTransferCallbackArgumentData(
        protocol, session->ngs_ID, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the request.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Callback Argument Data.
 */
static int
ngcllProtocolReplyTransferCallbackArgumentData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    int *callbackEnd,
    ngLog_t *log,
    int *error)
{
    ngRemoteMethodInformation_t *rmInfo;
    ngiArgument_t *arg, *callbackArg;
    int callbackID;
    int executableLocked = 0;
    int i, count = 0;
    int result;
    static const char fName[] =
        "ngcllProtocolReplyTransferCallbackArgumentData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);
    assert(callbackEnd != NULL);

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does INVOKE CALLBACK received? */
    if (session->ngs_status != NG_SESSION_STATUS_CB_TRANSARG_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the TRANSFER CALLBACK ARGUMENT DATA.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: TRANSFER CALLBACK ARGUMENT DATA.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSARG_DONE, log, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the session status.\n"); 
	goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackTransferArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End time.\n"); 
	goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackTransferArgumentReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferArgumentReal,
	    session->ngs_executionTime.ngs_callbackTransferArgument.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackTransferArgumentCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferArgumentCPU,
	    session->ngs_executionTime.ngs_callbackTransferArgument.nget_cpu.
		nget_execution);

    /* Transfer Timeout Done */
    result = ngcliTransferTimeoutTransferDone(
        session, NGCLI_TRANSFER_TIMEOUT_CB_ARGUMENT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set done the Transfer Timeout.\n"); 
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackCalculation, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the start time.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_EXECUTING, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
	goto error;
    }

    /* Get the callback ID */
    callbackID = ngiProtocolGetCallbackID(protocol, log, error);
    if (callbackID == NGI_CALLBACK_ID_UNDEFINED) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get the callback ID.\n"); 
	goto error;
    }

    /* Get the argument of callback */
    callbackArg = ngiProtocolGetCallbackArgument(protocol, log, error);
    if (callbackArg == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get the argument of callback.\n"); 
	goto error;
    }

    /* Get the argument */
    arg = ngiProtocolGetArgument(protocol, log, error);
    if (arg == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get the argument.\n"); 
        goto error;
    }

    for (i = 0; i < arg->nga_nArguments; i++) {
        if (arg->nga_argument[i].ngae_dataType ==
            NG_ARGUMENT_DATA_TYPE_CALLBACK) {
            if (count == callbackID) {
                break;
            }
            count += 1;
        }
    }
    if ((i == arg->nga_nArguments) && (count != callbackID)) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Callback ID %d is not valid.\n", callbackID); 
        goto error;
    }

    /* Get the Remote Method Information */
    rmInfo = protocol->ngp_getRemoteMethodInfo(protocol, log, error);
    if (rmInfo == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't get the Remote Method Information.\n"); 
        goto error;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
        callbackArg, arg,
        rmInfo->ngrmi_arguments[i].ngai_callback->ngrmi_arguments, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the Subscript Value of Argument.\n"); 
        goto error;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(callbackArg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Subscript Value of Argument is not valid.\n"); 
        goto error;
    }

    /* Allocate the storage for Argument Data of OUT mode */
    result = ngiArgumentAllocateDataStorage(callbackArg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't allocate the storage for Argument Data.\n"); 
        goto error;
    }

    /* Invoke the Callback function */
    result = ngcliSessionInvokeCallback(
        arg->nga_argument[i].ngae_pointer.ngap_function,
        callbackArg, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't invoke the callback.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_EXECUTE_DONE, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackCalculation, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End time.\n"); 
        goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackCalculationReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackCalculationReal,
	    session->ngs_executionTime.ngs_callbackCalculation.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackCalculationCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackCalculationCPU,
	    session->ngs_executionTime.ngs_callbackCalculation.nget_cpu.
		nget_execution);

    /* Start Transfer Timeout */
    result = ngcliTransferTimeoutTransferStart(
        session, NGCLI_TRANSFER_TIMEOUT_CB_RESULT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't start the Transfer Timeout.\n"); 
	goto error;
    }

    /* Set the start time */
    result = ngiSetStartTime(
    	&session->ngs_executionTime.ngs_callbackTransferResult, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the start time.\n"); 
        goto error;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSRES_REQUESTED, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Unlock the Executable */
    executableLocked = 0;
    result = ngcliExecutableUnlock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }
 
    /**
     * Note: TransferCallbackResult may send big data,
     * which will cause long time to send.
     * To receive heartbeat while sending callback result data,
     * callback is registered before the TransferCallbackResultData.
     */

    /* Set the IDs for Transfer */
    result = ngiProtocolTransferAttributeSet(protocol, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Attribute.\n"); 
        goto error;
    }

    /* Initialize the IDs  */
    ngiProtocolInitializeID(protocol);

    /* Register the callback for Protocol */
    result = ngiIOhandleReadCallbackRegister(
        protocol->ngp_communication->ngc_ioHandle,
        ngcllCallbackProtocol, executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't register the callback function for Protocol.\n"); 
        goto error;
    }
    *finalCallback = 1;
    *callbackEnd = 0;

    /* Send request */
    result = ngiSendRequestTransferCallbackResultData(
        protocol, session->ngs_ID, callbackArg, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the request.\n"); 
        goto error;
    }

    /* Clear the IDs for Transfer */
    result = ngiProtocolTransferAttributeInitialize(protocol, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the Transfer Attribute.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Receive reply: Transfer Callback Result Data.
 */
static int
ngcllProtocolReplyTransferCallbackResultData(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    int result, commClosed;
    int executableLocked;
    ngiCommunication_t *comm;
    ngiArgument_t *callbackArg;
    static const char fName[] = "ngcllProtocolReplyTransferCallbackResultData";

    /* Check the argument */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session != NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    executableLocked = 0;
    commClosed = 0;
    comm = NULL;

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Does TRANSFER CALLBACK RESULT DATA requested? */
    if (session->ngs_status != NG_SESSION_STATUS_CB_TRANSRES_REQUESTED) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not requested the TRANSFER CALLBACK RESULT DATA.\n"); 
        goto error;
    }

    /* Print the debug message */
    ngclLogDebugSession(session, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: TRANSFER CALLBACK RESULT DATA.\n"); 

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CB_TRANSRES_DONE, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Set the end time */
    result = ngiSetEndTime(
        &session->ngs_executionTime.ngs_callbackTransferResult, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End time.\n"); 
        goto error;
    }
    session->ngs_executionTime.ngs_sumCallbackTransferResultReal =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferResultReal,
	    session->ngs_executionTime.ngs_callbackTransferResult.nget_real.
		nget_execution);
    session->ngs_executionTime.ngs_sumCallbackTransferResultCPU =
	ngiTimevalAdd(
	    session->ngs_executionTime.ngs_sumCallbackTransferResultCPU,
	    session->ngs_executionTime.ngs_callbackTransferResult.nget_cpu.
		nget_execution);

    /* Transfer Timeout Done */
    result = ngcliTransferTimeoutTransferDone(
        session, NGCLI_TRANSFER_TIMEOUT_CB_RESULT, error);
    if (result == 0) {
	ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set done the Transfer Timeout.\n"); 
	goto error;
    }

    /* Get the argument of callback */
    callbackArg = ngiProtocolGetCallbackArgument(protocol, log, error);
    if (callbackArg == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get the argument of callback.\n"); 
        goto error;
    }

    /* Release the argument of callback */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't Release the argument of callback.\n"); 
        goto error;
    }

    /* Destruct the Argument */
    if (callbackArg != NULL) {
        result = ngiArgumentDestruct(callbackArg, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the argument.\n"); 
            goto error;
        }
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the callback ID to protocol.\n"); 
        goto error;
    }

    /* Close the Communication */
    if (executable->nge_keepConnect == 0) {
        comm = protocol->ngp_communication;
        assert(comm != NULL);

        /* Clear the communication on protocol. */
        result = ngiProtocolSetCommunication(
            protocol, NULL, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to clear the communication on protocol.\n");
            goto error;
        }

        /* Destruct the Communication */
        result = ngiCommunicationDestruct(comm, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct the communication.\n");
            goto error;
        }

        executable->nge_connecting = 0;
        commClosed = 1;
        *finalCallback = 1;
    }

    /* Set the status */
    result = ngcliSessionStatusSet(
	session, NG_SESSION_STATUS_CALCULATE_EXECUTING, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't set the status.\n"); 
        goto error;
    }

    /* Is cancel required? */
    if ((commClosed == 0) && (session->ngs_cancelRequest != 0)) {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }

        /* Session Cancel */
        result = ngcliProtocolRequestCancelSession(
            session, protocol, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't send the request.\n"); 
            goto error;
        }
    } else {
        /* Unlock the Executable */
        executableLocked = 0;
        result = ngcliExecutableUnlockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}


/**
 * Process the Notify I am Alive.
 */
static int
ngcllProtocolNotifyIamAlive(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    ngLog_t *log,
    int *error)
{
    ngclSession_t *cur;
    int executableLocked;
    int result, requireClose, requireCancel;
    static const char fName[] = "ngcllProtocolNotifyIamAlive";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(protocol != NULL);
    assert(header != NULL);

    executableLocked = 0;
    requireClose = 0;
    requireCancel = 0;
    cur = NULL;

    /**
     * No heartbeat arrival process,
     * which is treated by read chunk callback.
     */

    ngclLogInfoExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,
        "Got heartbeat.\n");

    /* Request the Session Cancel or Connection Close if needed. */

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    if (executable->nge_keepConnect != 0) {
        requireClose = 0;
    } else {
        cur = NULL; /* retrieve head item */
        while ((cur = ngclSessionGetNext(
            executable, cur, error)) != NULL) {

            if (cur->ngs_status == NG_SESSION_STATUS_CALCULATE_EXECUTING) {
                if (cur->ngs_cancelRequest != 0) {
                    requireCancel = 1;
                } else {
                    requireClose = 1;
                }
                break;
            }
        }
    }

    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
            goto error;
        }
    }

    if ((requireCancel != 0) || (requireClose != 0)) {
        /* Lock the Executable with send */
        result = ngcliExecutableLockWithSend(executable, log, error);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't lock the Executable.\n"); 
            return 0;
        }
        executableLocked = 1;

        /* Unlock the Executable */
        if (executableLocked != 0) {
            executableLocked = 0;
            result = ngcliExecutableUnlock(executable, log, error);
            if (result == 0) {
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't unlock the Executable.\n"); 
                goto error;
            }
        }

        if (requireCancel != 0) {
            /* Session Cancel */
            result = ngcliProtocolRequestCancelSession(
                cur, protocol, log, error);
            if (result == 0) {
                ngclLogErrorSession(cur, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't send the cancel request.\n");
                goto error;
            }
        } else if (requireClose != 0) {
            /* Connection Close */
            result = ngcliProtocolRequestConnectionClose(
                executable, protocol, log, error);
            if (result == 0) {
                ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't send the connection close request.\n");
                goto error;
            }
        } else {
            abort();
        }
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlock(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorSession(session, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}


/**
 * Send request: Connection Close.
 */
int
ngcliProtocolRequestConnectionClose(
    ngclExecutable_t *executable,
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliProtocolRequestConnectionClose";

    /* Check the arguments */
    assert(executable != NULL);
    assert(protocol != NULL);

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Send request: CONNECTION CLOSE.\n"); 

    executable->nge_connectionCloseRequested = 1;

    /* Send the request */
    result = ngiSendRequestConnectionClose(protocol, log, error);
    if (result == 0) {
    	ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't send the Request.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Receive reply: Connection Close.
 */
static int
ngcllProtocolReplyConnectionClose(
    ngclContext_t *context,
    ngclExecutable_t *executable,
    ngclSession_t *session,
    ngiProtocol_t *protocol,
    ngiProtocolHeader_t *header,
    int *finalCallback,
    ngLog_t *log,
    int *error)
{
    int executableLocked;
    int result, commClosed;
    ngiCommunication_t *comm;
    static const char fName[] = "ngcllProtocolReplyConnectionClose";

    /* Check the arguments */
    assert(context != NULL);
    assert(executable != NULL);
    assert(session == NULL);
    assert(protocol != NULL);
    assert(header != NULL);
    assert(finalCallback != NULL);

    executableLocked = 0;;
    commClosed = 0;
    comm = NULL;

    /* Print the debug message */
    ngclLogDebugExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
        "Receive reply: CONNECTION CLOSE.\n"); 

    /* Check the Protocol Header */
    if (header->ngph_nBytes != 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Protocol: Parameter length %ld is not zero.\n",
            header->ngph_nBytes); 
        return 0;
    }

    /* Lock the Executable */
    result = ngcliExecutableLock(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Executable.\n"); 
        goto error;
    }
    executableLocked = 1;

    /* Executable is sending? */
    if (executable->nge_sending == 0) {
        NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not sending.\n"); 
        goto error;
    }

    /* Close the Communication */
    comm = protocol->ngp_communication;
    assert(comm != NULL);

    /* Clear the communication on protocol. */
    result = ngiProtocolSetCommunication(
        protocol, NULL, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to clear the communication on protocol.\n");
        goto error;
    }

    /* Destruct the Communication */
    result = ngiCommunicationDestruct(comm, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to destruct the communication.\n");
        goto error;
    }

    assert(executable->nge_connecting != 0);
    executable->nge_connecting = 0;
    executable->nge_connectionCloseRequested = 0;
    commClosed = 1;
    *finalCallback = 1;

    /* Unlock the Executable with send */
    executableLocked = 0;
    result = ngcliExecutableUnlockWithSend(executable, log, error);
    if (result == 0) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Executable.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the Executable */
    if (executableLocked != 0) {
        executableLocked = 0;
        result = ngcliExecutableUnlockWithAll(executable, log, NULL);
        if (result == 0) {
            ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Executable.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the Remote Method Information
 */
ngRemoteMethodInformation_t *
ngcliProtocolGetRemoteMethodInformation(
    ngiProtocol_t *protocol,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclContext_t *context;
    ngclExecutable_t *executable;
    ngclSession_t *session; 
    ngRemoteMethodInformation_t *rmInfo;
    int contextID, executableID, sessionID;
    ngiProtocolTransferAttribute_t *transferAttr;
    static const char fName[] = "ngcliProtocolGetRemoteMethodInformation";

    /* Check the arguments */
    assert(protocol != NULL);

    transferAttr = &protocol->ngp_transferAttr;
    contextID = NGI_CONTEXT_ID_UNDEFINED;
    executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    sessionID = NGI_SESSION_ID_UNDEFINED;

    if (transferAttr->ngpta_valid != 0) {
        contextID = (int)transferAttr->ngpta_contextID;
        executableID = (int)transferAttr->ngpta_executableID;
        sessionID = (int)transferAttr->ngpta_sessionID;
    } else {
        contextID = protocol->ngp_contextID;
        executableID = protocol->ngp_executableID;
        sessionID = protocol->ngp_sessionID;
    }

    /* Get the Ninf-G Context */
    context = ngcliNinfgManagerGetContext(contextID, log, error);
    if (context == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Context is not found by ID %d.\n", contextID);
        return NULL;
    }

    /* Lock the list */
    result = ngclExecutableListReadLock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Executable.\n"); 
        return NULL;
    }

    /* Get */
    executable = ngclExecutableGet(context, executableID, error);
    if (executable == NULL) {
        ngclLogErrorContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable is not found by ID %d.\n", executableID); 
        goto error1;
    }

    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
        return NULL;
    }

    /* Lock the list */
    result = ngclSessionListReadLock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the list of Session.\n"); 
        return NULL;
    }

    /* Get the Session */
    session = ngclSessionGet(
        context, executable, sessionID, error);
    if (session == NULL) {
        ngclLogErrorExecutable(executable, NG_LOGCAT_NINFG_PURE, fName,  
            "Session is not found by ID %d.\n", sessionID); 
        goto error2;
    }

    rmInfo = &session->ngs_rmInfo;

    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
        return NULL;
    }

    /* Success */
    return rmInfo;

/* Error occurred */
error1:
    /* Unlock the list */
    result = ngclExecutableListReadUnlock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Executable.\n"); 
    }
    return NULL;

error2:
    /* Unlock the list */
    result = ngclSessionListReadUnlock(context, error);
    if (result == 0) {
        ngclLogFatalContext(context, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the list of Session.\n"); 
    }
    return NULL;
}

