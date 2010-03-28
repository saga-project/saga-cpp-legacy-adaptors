/*
 * $RCSfile: ngexContext.c,v $ $Revision: 1.39 $ $Date: 2008/03/28 08:50:58 $
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
 * Module of Ninf-G Executable.
 */

#include "ngEx.h"

NGI_RCSID_EMBED("$RCSfile: ngexContext.c,v $ $Revision: 1.39 $ $Date: 2008/03/28 08:50:58 $")

/**
 * Prototype declaration of internal functions.
 */
static void ngexlContextInitializeMember(ngexiContext_t *);
static void ngexlContextInitializePointer(ngexiContext_t *);
static int ngexlContextInitializeMutexAndCond(ngexiContext_t *, int *);
static int ngexlContextFinalizeMutexAndCond(ngexiContext_t *, int *);
static void ngexlLocalMachineInformationInitializeMember(
    ngexiLocalMachineInformation_t *);
static void ngexlLocalMachineInformationInitializePointer(
    ngexiLocalMachineInformation_t *);
static int ngexlCommunicationInformationInitialize(
    ngexiCommunicationInformation_t *, ngLog_t *, int *);
static int ngexlCommunicationInformationFinalize(
    ngexiCommunicationInformation_t *, ngLog_t *, int *);
static void ngexlCommunicationInformationInitializeMember(
    ngexiCommunicationInformation_t *);
static void ngexlCommunicationInformationInitializePointer(
    ngexiCommunicationInformation_t *);
static int ngexlCommunicationProxyInformationInitialize(
    ngexiCommunicationProxyInformation_t *, ngLog_t *, int *);
static int ngexlCommunicationProxyInformationFinalize(
    ngexiCommunicationProxyInformation_t *, ngLog_t *, int *);
static void ngexlCommunicationProxyInformationInitializeMember(
    ngexiCommunicationProxyInformation_t *);
static int ngexlAnalyzeArgument(ngexiContext_t *, int, char *[], int, int *);
static int ngexlAnalyzeArgumentSub(ngexiContext_t *, char *, int, int *);
static int ngexlAnalyzeArgumentConnectbackAddress(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentSimpleAuthNumberFile(
    ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentSimpleAuthNumber(
    ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentContextID(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentJobID(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentHeartBeat(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentConnectRetry(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentWorkDirectory(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCoreDumpSize(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentDebugTerminal(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentDebugDisplay(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentDebugger(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentDebugEnable(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentDebugBusyLoop(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentTcpNodelay(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCommunicationProxyType(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCommunicationProxyPath(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCommunicationProxyOption(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCheckValue(ngexiContext_t *, int *);

static char *ngexlMakeUserConfigFileName(ngLog_t *, int *);
static int ngexlReleaseConfigFileName(char *fileName, ngLog_t *, int *);
static int ngexlContextLogOutput(ngexiContext_t *, int, char *[], int *);
static int ngexlContextSignalRegister(ngexiContext_t *, int *);
static int ngexlContextExecutableStatusMatch(
    ngexiExecutableStatus_t, ngexiExecutableStatus_t *, int);
static int ngexlContextExitOnError(ngexiContext_t *, int *);
static void ngexlContextProcessImmediateExit(ngexiContext_t *, int *);
static int ngexlContextCommunicationDestruct(ngexiContext_t *, int *);

/**
 * Initialize
 */
int
ngexiContextInitialize(
    ngexiContext_t *context,
    int argc,
    char *argv[],
    ngRemoteClassInformation_t *rcInfo,
    int rank,
    int *error)
{
    int result, isPthread;
    char *sysConfig = NULL;
    char *userConfig = NULL;
    ngiProtocolAttribute_t protoAttr;
    char *address = NULL;
    static const char fName[] = "ngexiContextInitialize";

    /* Check the atguments */
    assert(context != NULL);

#ifdef NG_PTHREAD
    isPthread = 1;
#else /* NG_PTHREAD */
    isPthread = 0;
#endif /* NG_PTHREAD */

    /* Initialize the members */
    ngexlContextInitializeMember(context);

    /* Construct the temporary Log */
    context->ngc_log = NULL;

    if (rank == 0) {
        if (isPthread == 0) {
            /**
             * On NonThread version, Ninf-G Event must be available before
             * ngiCondInitialize().
             */
     
            /* Initialize the Ninf-G Event */
            context->ngc_event = ngiEventConstruct(
                NULL, context->ngc_log, error);
            if (context->ngc_event == NULL) {
                ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,
                    "Can't initialize the Ninf-G Event module.\n");
                return 0;
            }
        }
     
        /* Initialize the Mutex and Condition Variable for status */
        result = ngexlContextInitializeMutexAndCond(context, error);
        if (result == 0) {
        	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        	    "Can't initialize the Mutex and Condition Variable.\n"); 
        return 0;
        }
     
        /* Initialize the I/O Callback Waiter */
        result = ngiIOhandleCallbackWaiterInitialize(
            &context->ngc_ioCallbackWaiter,
            context->ngc_event, context->ngc_log, error);
        if (result == 0) {
        	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        	    "Can't initialize the I/O Callback Waiter.\n"); 
        return 0;
        }
    }

    /* Set initial status for context */
    context->ngc_executableStatus = NGEXI_EXECUTABLE_STATUS_INITIALIZING;

    /* Save the Remote Class Information */
    context->ngc_rcInfo = rcInfo;

    context->ngc_rank = rank;
    if (rank == 0) {
        /* Initialize the Communication Information */
        result = ngexlCommunicationInformationInitialize(
            &context->ngc_commInfo, context->ngc_log, error);
        if (result == 0) {
        	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        	    "Can't initialize the Communication Information.\n"); 
            return 0;
        }
     
        /* Initialize the Communication Proxy Information */
        result = ngexlCommunicationProxyInformationInitialize(
            &context->ngc_commProxyInfo, context->ngc_log, error);
        if (result == 0) {
        	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        	    "Can't initialize the Communication Proxy Information.\n"); 
            return 0;
        }
    }

    /* Analyze the arguments */
    result = ngexlAnalyzeArgument(context, argc, argv, rank, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't analyze the arguments.\n"); 
        return 0;
    }

    if (rank > 0) {
        /* Success */
        return 1;
    }

    /* Re set the status. for to ensure */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_INITIALIZING, error);
    if (result == 0) {
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the status.\n"); 
	return 0;
    }

    /* Make the file name of configuration of user */
    userConfig = ngexlMakeUserConfigFileName(context->ngc_log, error);
    if (userConfig == NULL) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't make the file name of configuration file of user.\n"); 
        NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
	/* Continue to below */
    }

    /* Read configuration file */
    result = ngexiConfigFileRead(context, sysConfig, userConfig, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't read the Configuration file.\n"); 
        goto error;
    }

    /* Construct the Log Manager */
    context->ngc_log = ngiLogConstructFromConfig(
        &context->ngc_lmInfo.nglmi_logInfo,
        &context->ngc_lmInfo.nglmi_logLevels,
        "Executable", NGI_LOG_EXECUTABLE_ID_UNDEFINED,
        NULL, error);
    if (context->ngc_log == NULL) {
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,
            "Can't construct the log manager.\n");
        goto error;
    }
 
    /* Release the file name of configuration of system */
    if (sysConfig != NULL) {
	result = ngexlReleaseConfigFileName(sysConfig, context->ngc_log, error);
	sysConfig = NULL;
	if (result == 0) {
	    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the file name of configuration file of system.\n"); 
	    goto error;
	}
    }

    /* Release the file name of configuration of user */
    if (userConfig != NULL) {
	result = ngexlReleaseConfigFileName(
	    userConfig, context->ngc_log, error);
	userConfig = NULL;
	if (result == 0) {
	    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the file name of configuration file of user.\n"); 
	    goto error;
	}
    }

    /* log */
    result = ngexlContextLogOutput(context, argc, argv, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't output the log.\n"); 
	goto error;
    }

    /* Register the signal */
    result = ngexlContextSignalRegister(context, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't register the signal.\n"); 
	goto error;
    }

    if (isPthread != 0) {
        assert(context->ngc_event == NULL);

        /**
         * On Pthread version, Event must be created after
         * ngiSignalManagerStart().
         */
        /* Initialize the Ninf-G Event. */
        context->ngc_event = ngiEventConstruct(NULL, context->ngc_log, error);
        if (context->ngc_event == NULL) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Ninf-G Event module.\n"); 
            goto error;
        }
    } else {
        assert(context->ngc_event != NULL);

        /* Set log to Ninf-G Event. */
        result = ngiEventLogSet(
            context->ngc_event, context->ngc_log, error);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't set the log to Ninf-G Event.\n"); 
            goto error;
        }
    }

    /* Initialize the Random Number */
    result = ngiRandomNumberInitialize(
        &context->ngc_random, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Random Number Seed.\n"); 
	return 0;
    }

    /* Create Communication Proxy Manager. */
    if ((context->ngc_commProxyInfo.ngcpi_type != NULL) ||
        (context->ngc_commProxyInfo.ngcpi_path != NULL)) {
        context->ngc_commProxy = ngexiCommunicationProxyConstruct(
            context, context->ngc_commProxyInfo.ngcpi_type,
            context->ngc_commProxyInfo.ngcpi_path, 
            context->ngc_commProxyInfo.ngcpi_options,
            &address, context->ngc_log, error);
        if (context->ngc_commProxy == NULL) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Communication Proxy.\n"); 
            goto error;
        }
        assert(address != NULL);
        if (context->ngc_commInfo.ngci_connectbackAddress != NULL) {
            result = ngiFree(context->ngc_commInfo.ngci_connectbackAddress,
                context->ngc_log, error);
            if (result == 0) {
                ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't free the string.\n"); 
                goto error;
            }
            context->ngc_commInfo.ngci_connectbackAddress = NULL;
        }
        context->ngc_commInfo.ngci_connectbackAddress = address;
        address = NULL;
    }

    /* Construct the Communication Manager */
    context->ngc_communication = ngiCommunicationConstructClient(
    	context->ngc_event,
	context->ngc_commInfo.ngci_tcpNodelay,
	context->ngc_commInfo.ngci_connectbackAddress,
	context->ngc_retryInfo,
	&context->ngc_random,
	context->ngc_log, error);
    if (context->ngc_communication == NULL) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't construct the Communication Information.\n"); 
	goto error;
    }

    /* Initialize the attribute of Protocol Manager */
    result = ngiProtocolAttributeInitialize(
        &protoAttr, context->ngc_log, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the protocol attribute.\n"); 
	goto error;
    }

    protoAttr.ngpa_architecture = NGI_ARCHITECTURE_ID;
    protoAttr.ngpa_xdr = NG_XDR_USE;
    protoAttr.ngpa_protocolVersion= NGI_PROTOCOL_VERSION;
    protoAttr.ngpa_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_DEFAULT;
    protoAttr.ngpa_simpleAuthNumber =
        context->ngc_commInfo.ngci_simpleAuthNumber;
    protoAttr.ngpa_contextID = context->ngc_commInfo.ngci_contextID;
    protoAttr.ngpa_jobID = context->ngc_commInfo.ngci_jobID;
    protoAttr.ngpa_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
    protoAttr.ngpa_tmpDir = context->ngc_lmInfo.nglmi_tmpDir;
    protoAttr.ngpa_keepConnect = 1;

    /* Construct the Protocol Manager */
    context->ngc_protocol = ngiProtocolConstruct(
    	&protoAttr, context->ngc_communication, context->ngc_event,
        ngexiProtocolGetRemoteMethodInformation, context->ngc_log, error);
    if (context->ngc_protocol == NULL) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the Protocol Manager.\n"); 
	goto error;
    }
    context->ngc_connecting = 1;

    /* Initialize Heart Beat */
    result = ngexiHeartBeatInitialize(context, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize heartbeat.\n"); 
	goto error;
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't set the status.\n"); 
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    if (context->ngc_commLog != NULL) {
        /* Unregister the Communication Log */
        result = ngiCommunicationLogUnregister(
            context->ngc_communication, context->ngc_log, error);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register the Communication Log.\n"); 
        }

        /* Destruct the Communication Log */
        result = ngCommLogDestruct(context->ngc_commLog, NULL, error);
        context->ngc_commLog = NULL;
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Communication Log.\n"); 
        }
    }

    /* Release the file name of configuration of system */
    if (sysConfig != NULL) {
	result = ngexlReleaseConfigFileName(sysConfig, context->ngc_log, NULL);
	sysConfig = NULL;
	if (result == 0) {
	    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the file name of configuration file of system.\n"); 
	}
    }

    /* Release the file name of configuration of user */
    if (userConfig != NULL) {
	result = ngexlReleaseConfigFileName(userConfig, context->ngc_log, NULL);
	userConfig = NULL;
	if (result == 0) {
	    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't release the file name of configuration file of user.\n"); 
	}
    }

    /* Finalize the Context */
    result = ngexiContextFinalize(context, NULL);
    if (result == 0) {
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Context.\n"); 
	return 0;
    }

    result = ngiFree(address, context->ngc_log, NULL);
    if (result == 0) {
    	ngLogFatal(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't free the string.\n"); 
    }

    return 0;
}

/**
 * Finalize
 */
int
ngexiContextFinalize(ngexiContext_t *context, int *error)
{
    int result;
    static const char fName[] = "ngexiContextFinalize";

    /* Check the arguments */
    assert(context != NULL);

    if (context->ngc_rank > 0) {
        /* Initialize the members */
        ngexlContextInitializeMember(context);
 
        /* Success */
        return 1;
    }

    /* Finalize Heart Beat */
    result = ngexiHeartBeatFinalize(context, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize heartbeat.\n"); 
	return 0;
    }

    if (context->ngc_commLog != NULL) {
        /* Unregister the Communication Log */
        result = ngiCommunicationLogUnregister(
            context->ngc_communication, context->ngc_log, error);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the Communication Log.\n"); 
        }

        /* Destruct the Communication Log */
        result = ngCommLogDestruct(context->ngc_commLog, NULL, error);
        context->ngc_commLog = NULL;
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Communication Log.\n"); 
        }
    }

    /* Destruct the Protocol Manager */
    if (context->ngc_protocol != NULL) {

        if ((context->ngc_protocol != NULL) &&
            (context->ngc_protocol->ngp_communication != NULL)) {

            result = ngiCommunicationClose(
                context->ngc_communication, context->ngc_log, error);
            if (result == 0) {
                ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't close the Communication.\n"); 
            }
        }

        /* Wait I/O Callback End */
        result = ngiIOhandleCallbackWaiterWait(
            &context->ngc_ioCallbackWaiter, context->ngc_log, error);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the I/O Callback End.\n"); 
        }
    
    	result = ngiProtocolDestruct(
	    context->ngc_protocol, context->ngc_log, error);
	context->ngc_protocol = NULL;
	if (result == 0) {
	    ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	        "Can't destruct the Protocol Manager.\n"); 
	    return 0;
	}
    }

    /* Destroy Communication Proxy Manager. */
    result = ngexiCommunicationProxyDestruct(context->ngc_commProxy, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't destruct the Communication Proxy.\n"); 
	return 0;
    }

    /* Finalize the Ninf-G Event */
    result = ngiEventDestruct(context->ngc_event, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Event module.\n"); 
	return 0;
    }

    /* Finalize the Local Machine Information */
    result = ngexiLocalMachineInformationFinalize(
    	&context->ngc_lmInfo, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Local Machine Information.\n"); 
	return 0;
    }

    /* Finalize the Debugger Information */
    result = ngiDebuggerInformationFinalize(
    	&context->ngc_dbgInfo, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Debugger Information.\n"); 
	return 0;
    }

    /* Finalize the Communication Proxy Information */
    result = ngexlCommunicationProxyInformationFinalize(
    	&context->ngc_commProxyInfo, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Communication Proxy Information.\n"); 
	return 0;
    }

    /* Finalize the Communication Information */
    result = ngexlCommunicationInformationFinalize(
    	&context->ngc_commInfo, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Communication Information.\n"); 
	return 0;
    }

    /* Finalize the Random Number */
    result = ngiRandomNumberFinalize(
        &context->ngc_random, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Random Number Seed.\n"); 
	return 0;
    }

    /* Unset the log from Signal Manager */
    result = ngexiSignalManagerLogSet(
        NULL, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't unregister the log for Signal Manager.\n"); 
	return 0;
    }

    /* Destruct the Log */
    if (context->ngc_log != NULL) {
        ngLogDebug(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable log destruct.\n"); 

        result = ngLogDestruct(context->ngc_log, NULL, error);
        if (result == 0) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the Log.\n"); 
            return 0;
        }
    }
    context->ngc_log = NULL;

    /* Finalize the I/O Callback Waiter */
    result = ngiIOhandleCallbackWaiterFinalize(
        &context->ngc_ioCallbackWaiter, context->ngc_log, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the I/O Callback Waiter.\n"); 
	return 0;
    }
 
    /* Finalize the Mutex and Condition Variable for status */
    result = ngexlContextFinalizeMutexAndCond(context, error);
    if (result == 0) {
    	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't initialize the Mutex and Condition Variable.\n"); 
	return 0;
    }

    /* Initialize the members */
    ngexlContextInitializeMember(context);

    /* Success */
    return 1;
}

/**
 * Initialize the member.
 */
static void
ngexlContextInitializeMember(ngexiContext_t *context)
{
    /* Check the argument */
    assert(context != NULL);

    /* Initialize the pointers */
    ngexlContextInitializePointer(context);

    /* Initialize the members */
    ngexlLocalMachineInformationInitializeMember(&context->ngc_lmInfo);
    ngiDebuggerInformationInitializeMember(&context->ngc_dbgInfo);
    ngexlCommunicationInformationInitializeMember(&context->ngc_commInfo);
    ngexlCommunicationProxyInformationInitializeMember(&context->ngc_commProxyInfo);
    ngexiHeartBeatInitializeMember(&context->ngc_heartBeatSend);

    context->ngc_retryInfo.ngcri_count = 0;
    context->ngc_retryInfo.ngcri_interval = 0;
    context->ngc_retryInfo.ngcri_increase = 0.0;
    context->ngc_retryInfo.ngcri_useRandom = 0;

    context->ngc_mutex = NGI_MUTEX_NULL;
    context->ngc_cond = NGI_COND_NULL;

    context->ngc_heartBeatInterval = 0;

    context->ngc_random = 0;

    context->ngc_connectLocked = 0;
    context->ngc_connecting = 0;
    context->ngc_connectCloseRequested = 0;
    context->ngc_afterCloseArrived = 0;
    context->ngc_afterCloseType = NGEXI_AFTER_CLOSE_TYPE_NONE;

    context->ngc_rank = 0;

    context->ngc_error = NG_ERROR_NO_ERROR;
    context->ngc_cbError = NG_ERROR_NO_ERROR;
}

/**
 * Initialize the pointer.
 */
static void
ngexlContextInitializePointer(ngexiContext_t *context)
{
    /* Check the argument */
    assert(context != NULL);

    /* Initialize the pointers */
    ngexlLocalMachineInformationInitializePointer(&context->ngc_lmInfo);
    ngiDebuggerInformationInitializePointer(&context->ngc_dbgInfo);
#if 0 /* Is this necessary? */
    ngexlCommunicationInformationInitializePointer(&context->ngc_commInfo);
#endif
    context->ngc_event = NULL;
    context->ngc_protocol = NULL;
    context->ngc_log = NULL;
    context->ngc_commLog = NULL;
    context->ngc_commProxy = NULL;
}

/**
 * Initialize the Mutex and Condition Variable for status
 */
static int
ngexlContextInitializeMutexAndCond(
    ngexiContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngexlContextInitializeMutexAndCond";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Initialize the mutex */
    result = ngiMutexInitialize(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the mutex.\n"); 
        goto error;
    }

    /* Initialize the cond */
    result = ngiCondInitialize(
        &context->ngc_cond, context->ngc_event, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the cond.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngexlContextFinalizeMutexAndCond(context, NULL);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't finalize the Mutex and Cond.\n"); 
    }
    
    /* Failed */
    return 0;
}

/**
 * Finalize the Mutex and Condition Variable for status
 */
static int
ngexlContextFinalizeMutexAndCond(
    ngexiContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngexlContextFinalizeMutexAndCond";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* Finalize the mutex */
    result = ngiMutexDestroy(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the mutex.\n"); 
        return 0;
    }

    /* Finalize the cond */
    result = ngiCondDestroy(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't initialize the cond.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Local Machine Information: Initialize.
 */
int
ngexiLocalMachineInformationInitialize(
    ngexiLocalMachineInformation_t *lmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] ="ngexiLocalMachineInformationInitialize";

    /* Check the arguments */
    assert(lmInfo != NULL);

    ngexlLocalMachineInformationInitializeMember(lmInfo);

    result = ngiLogInformationInitialize(&lmInfo->nglmi_logInfo, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the log information.\n");
        return 0;
    }

    result = ngiLogLevelInformationInitialize(
        &lmInfo->nglmi_logLevels, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the log level information.\n");
        return 0;
    }

    result = ngiLogInformationInitialize(&lmInfo->nglmi_commLogInfo, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't initialize the communication log information.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Local Machine Information: Finalize.
 */
int
ngexiLocalMachineInformationFinalize(
    ngexiLocalMachineInformation_t *lmInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngexiLocalMachineInformationFinalize";

    /* Check the arguments */
    assert(lmInfo != NULL);

    /* Finalize the Log Information */
    result = ngiLogInformationFinalize(&lmInfo->nglmi_logInfo, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Log Information.\n"); 
        ret = 0;
        error = NULL;
    }

    result = ngiLogLevelInformationFinalize(&lmInfo->nglmi_logLevels, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Log Level Information.\n"); 
        ret = 0;
        error = NULL;
    }

    /* Finalize the Communication Log Information */
    result = ngiLogInformationFinalize(&lmInfo->nglmi_commLogInfo, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Communication Log Information.\n"); 
        ret = 0;
        error = NULL;
    }

    /* Deallocate the members */
    ngiFree(lmInfo->nglmi_tmpDir, log, error);
    lmInfo->nglmi_tmpDir = NULL;

    ngiFree(lmInfo->nglmi_path, log, error);
    lmInfo->nglmi_path = NULL;

    if (lmInfo->nglmi_saveStdout != NULL) {
        ngiFree(lmInfo->nglmi_saveStdout, log, error);
        lmInfo->nglmi_saveStdout = NULL;
    }

    if (lmInfo->nglmi_saveStderr != NULL) {
        ngiFree(lmInfo->nglmi_saveStderr, log, error);
        lmInfo->nglmi_saveStderr = NULL;
    }

    if (lmInfo->nglmi_signals != NULL) {
        ngiFree(lmInfo->nglmi_signals, log, error);
        lmInfo->nglmi_signals = NULL;
    }

    if (lmInfo->nglmi_commProxyLogFilePath != NULL) {
        ngiFree(lmInfo->nglmi_commProxyLogFilePath, log, error);
        lmInfo->nglmi_commProxyLogFilePath = NULL;
    }

    /* Success */
    return 1;
}

/**
 * Local Machine Information: Initialize the member.
 */
static void
ngexlLocalMachineInformationInitializeMember(
    ngexiLocalMachineInformation_t *lmInfo)
{
    /* Check the argument */
    assert(lmInfo != NULL);

    /* Initialize the pointers */
    ngexlLocalMachineInformationInitializePointer(lmInfo);

    /* Initialize the members */
    ngiLogInformationInitializeMember(&lmInfo->nglmi_logInfo);
    ngiLogLevelInformationInitializeMember(&lmInfo->nglmi_logLevels);

    lmInfo->nglmi_commLogEnable = 0;
    ngiLogInformationInitializeMember(&lmInfo->nglmi_commLogInfo);

    lmInfo->nglmi_continueOnError = 0;
}

/**
 * Local Machine Information: Initialize the pointer.
 */
static void
ngexlLocalMachineInformationInitializePointer(
    ngexiLocalMachineInformation_t *lmInfo)
{
    /* Check the argument */
    assert(lmInfo != NULL);

    /* Initialize the pointers */
    lmInfo->nglmi_path = NULL;
    lmInfo->nglmi_tmpDir = NULL;
    lmInfo->nglmi_saveStdout = NULL;
    lmInfo->nglmi_saveStderr = NULL;
    lmInfo->nglmi_signals = NULL;
    lmInfo->nglmi_commProxyLogFilePath = NULL;
}

/**
 * Communication Information: Initialize
 */
static int
ngexlCommunicationInformationInitialize(
    ngexiCommunicationInformation_t *commInfo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(commInfo != NULL);

    /* Initialize the members */
    ngexlCommunicationInformationInitializeMember(commInfo);

    /* Success */
    return 1;
}

/**
 * Communication Information: Finalize.
 */
static int
ngexlCommunicationInformationFinalize(
    ngexiCommunicationInformation_t *commInfo,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(commInfo != NULL);

    /* Deallocate the members */
    ngiFree(commInfo->ngci_connectbackAddress, log, error);
    ngiFree(commInfo->ngci_hostname, log, error);

    /* Initialize the members */
    ngexlCommunicationInformationInitializeMember(commInfo);

    /* Success */
    return 1;
}

/**
 * Communication Information: Initialize the members.
 */
static void
ngexlCommunicationInformationInitializeMember(
    ngexiCommunicationInformation_t *commInfo)
{
    /* Check the argument */
    assert(commInfo != NULL);

    /* Initialize the pointers */
    ngexlCommunicationInformationInitializePointer(commInfo);

    /* Initialize the members */
    commInfo->ngci_simpleAuthNumber = NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_UNDEFINED;
    commInfo->ngci_tcpNodelay = 0; /* false */
    commInfo->ngci_contextID = NGI_CONTEXT_ID_UNDEFINED;
    commInfo->ngci_jobID = NGI_JOB_ID_UNDEFINED;
}

/**
 * Communication Information: Initialize the pointer.
 */
void
ngexlCommunicationInformationInitializePointer(
    ngexiCommunicationInformation_t *commInfo)
{
    /* Check the argument */
    assert(commInfo != NULL);

    /* Initialize the pointers */
    commInfo->ngci_connectbackAddress = NULL;
    commInfo->ngci_hostname = NULL;
}

/**
 * Communication Proxy Information: Initialize
 */
static int
ngexlCommunicationProxyInformationInitialize(
    ngexiCommunicationProxyInformation_t *commProxyInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngexlCommunicationProxyInformationInitialize";

    assert(commProxyInfo != NULL);

    ngexlCommunicationProxyInformationInitializeMember(commProxyInfo);

    commProxyInfo->ngcpi_options = 
        ngiLineListConstruct(log, error);
    if (commProxyInfo->ngcpi_options == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the line list.\n");
        return 0;
    }

    return 1;
}

/**
 * Communication Proxy Information: Finalize
 */
static int
ngexlCommunicationProxyInformationFinalize(
    ngexiCommunicationProxyInformation_t *commProxyInfo,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngexlCommunicationProxyInformationFinalize";

    assert(commProxyInfo != NULL);

    result = ngiFree(commProxyInfo->ngcpi_type, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free string.\n");
        ret = 0;
        error = NULL;
    }

    result = ngiFree(commProxyInfo->ngcpi_path, log ,error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't free string.\n");
        ret = 0;
        error = NULL;
    }

    result = ngiLineListDestruct(
        commProxyInfo->ngcpi_options, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't create the line list.\n");
        ret = 0;
        error = NULL;
    }
    ngexlCommunicationProxyInformationInitializeMember(commProxyInfo);

    return ret;
}

/**
 * Communication Proxy Information: Zero clear
 */
static void
ngexlCommunicationProxyInformationInitializeMember(
    ngexiCommunicationProxyInformation_t *commProxyInfo)
{
    assert(commProxyInfo != NULL);

    commProxyInfo->ngcpi_type    = NULL;
    commProxyInfo->ngcpi_path    = NULL;
    commProxyInfo->ngcpi_options = NULL;

    return;
}

/**
 * Function table for analyze the arguments.
 */
typedef struct ngexlAnalyzeArgument_s {
    char *arg;
    int (*func)(ngexiContext_t *, char *, int *);
    int handlingSubRank;
} ngexlAnalyzeArgument_t;

/**
 * Analyze the arguments.
 */
static int
ngexlAnalyzeArgument(
    ngexiContext_t *context,
    int argc,
    char *argv[],
    int rank,
    int *error)
{
    int i;
    int result;
    static const char fName[] = "ngexlAnalyzeArgument";

    /* Check the arguments */
    assert(context != NULL);
    assert(argc > 0);
    assert(argv != NULL);
    assert(rank >= 0);

    /* set invoked path */
    assert(context->ngc_lmInfo.nglmi_path == NULL);
    context->ngc_lmInfo.nglmi_path = strdup(argv[0]);
    if (context->ngc_lmInfo.nglmi_path == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't allocate the storage for path.\n"); 
        return 0;
    }

    /* Analyze each option arguments */
    for (i = 1; i < argc; i++) {
	/* Is argument valid? */
	if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
	    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Argument No.%d \"%s\" is not valid. ignored.\n", i, argv[i]); 
	    continue;
	}

	/* Analyze the argument */
	result = ngexlAnalyzeArgumentSub(context, argv[i], rank, error);
	if (result == 0) {
	    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Argument No.%d \"%s\" is not valid. ignored.\n", i, argv[i]); 
	    continue;
	}
    }

    if (rank == 0) {
        result = ngexlAnalyzeArgumentCheckValue(context, error);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Check the argument information failed.\n");
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument.
 */
static int
ngexlAnalyzeArgumentSub(
    ngexiContext_t *context,
    char *argv,
    int rank,
    int *error)
{
    int result;
    int i;
    int length, param;
    static const char fName[] = "ngexlAnalyzeArgumentSub";

    /* Function table for analyze the arguments */
    static const ngexlAnalyzeArgument_t aa[] = {
    	{"connectbackAddress", ngexlAnalyzeArgumentConnectbackAddress, 0},
    	{"authNumberFile",     ngexlAnalyzeArgumentSimpleAuthNumberFile, 0},
    	{"authNumber",	       ngexlAnalyzeArgumentSimpleAuthNumber, 0},
	{"contextID",	       ngexlAnalyzeArgumentContextID, 0},
	{"jobID",	       ngexlAnalyzeArgumentJobID, 0},
	{"heartbeat",	       ngexlAnalyzeArgumentHeartBeat, 0},
	{"connectRetry",       ngexlAnalyzeArgumentConnectRetry, 0},
	{"workDirectory",      ngexlAnalyzeArgumentWorkDirectory, 1},
	{"coreDumpSize",       ngexlAnalyzeArgumentCoreDumpSize, 1},
	{"debugTerminal",      ngexlAnalyzeArgumentDebugTerminal, 0},
	{"debugDisplay",       ngexlAnalyzeArgumentDebugDisplay, 0},
	{"debugger",	       ngexlAnalyzeArgumentDebugger, 0},
	{"debugEnable",	       ngexlAnalyzeArgumentDebugEnable, 0},
	{"debugBusyLoop",      ngexlAnalyzeArgumentDebugBusyLoop, 0},
	{"tcpNodelay",         ngexlAnalyzeArgumentTcpNodelay, 0},
        {"communicationProxyType",
                ngexlAnalyzeArgumentCommunicationProxyType, 0},
        {"communicationProxyPath",
                ngexlAnalyzeArgumentCommunicationProxyPath, 0},
        {"communicationProxyOption",
                ngexlAnalyzeArgumentCommunicationProxyOption, 0}, 
	{NULL, 			     NULL},
    };

    /* Check the arguments */
    assert(context != NULL);
    assert(argv != NULL);

    for (i = 0; aa[i].arg != NULL; i++) {
	length = strlen(aa[i].arg);
	param = 2 + length + 1;

	/* Find the argument */
	if (strncmp(aa[i].arg, &argv[2], length) == 0) {
	    /* Argument is found */
	    goto found;
	}
    }

    /* Not found */
    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s: Argument is not defined.\n", argv); 
    return 0;

    /* Argument is found */
found:
    if ((rank > 0) && (aa[i].handlingSubRank == 0)) {
        /* Just ignore the argument */
        return 1;
    }

    if ((argv[param - 1] != '=') || (argv[param] == '\0')) {
	/* Argument is not valid */
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Syntax error.\n", argv); 
	return 0;
    }

    result = aa[i].func(context, &argv[param], error);
    if (result == 0) {
	/* Argument is not valid */
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument is not valid.\n", argv); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --connectbackAddress
 */
static int
ngexlAnalyzeArgumentConnectbackAddress(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    char *address = NULL;
    char *hostname = NULL;
    ngiAddress_t ad;
    int adInitialized = 0;
    int result;
    ngLog_t *log;
    static const char fName[] = "ngexlAnalyzeArgumentConnectbackAddress";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    log = context->ngc_log;

    if (context->ngc_commInfo.ngci_connectbackAddress != NULL) {
        assert(context->ngc_commInfo.ngci_hostname != NULL);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "\"--connectbackAddress\" option appears multi times.\n"); 
        goto error;
    }
    assert(context->ngc_commInfo.ngci_hostname == NULL);
    
    result = ngiAddressInitialize(&ad, log, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the address.\n"); 
        goto error;
    }
    adInitialized = 1;

    /* Duplicate the host name */
    address = ngiStrdup(arg, log, error);
    if (address == NULL) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for connectback address.\n"); 
        goto error;
    }

    /* Parse the Address */
    result = ngiCommunicationParseAddress(
        address, &ad, log, error);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't parse address \"%s\".\n", address); 
        goto error;
    }

    if (ad.nga_hostname == NULL) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "hostname is empty.\n"); 
        goto error;
    }
    hostname = ad.nga_hostname;
    ad.nga_hostname = NULL;

    result = ngiAddressFinalize(&ad, log, error);
    adInitialized = 0;
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't finalize the address.\n"); 
        goto error;
    }

    assert(context->ngc_commInfo.ngci_connectbackAddress == NULL);
    assert(context->ngc_commInfo.ngci_hostname           == NULL);
    context->ngc_commInfo.ngci_connectbackAddress = address;
    context->ngc_commInfo.ngci_hostname           = hostname;

    /* Success */
    return 1;
error:
    result = ngiFree(address, log, NULL);
    if (result == 0) {
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't free the string.\n"); 
    }
    address = NULL;

    if (adInitialized != 0) {
        result = ngiAddressFinalize(&ad, log, NULL);
        if (result == 0) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the address.\n"); 
        }
        adInitialized = 0;
    }

    return 0;
}

/**
 * Analyze the argument: --authNumberFile
 */
static int
ngexlAnalyzeArgumentSimpleAuthNumberFile(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    FILE *fp;
    ngLog_t *log;
    int authNumber, result;
    char buf[NGEXI_LINE_BUFFER_SIZE], *fileName, *resultStr;
    static const char fName[] = "ngexlAnalyzeArgumentSimpleAuthNumberFile";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    log = context->ngc_log;

    /* Get the Auth Number */
    fileName = arg;

    fp = fopen(fileName, "r");
    if (fp == NULL) {
	ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: file open failed.\n", fileName); 
	return 0;
    }

    /* Read one line. */
    resultStr = fgets(buf, NGEXI_LINE_BUFFER_SIZE, fp);
    if (resultStr == NULL) {
	ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: file read failed.\n", fileName); 
	return 0;
    }

    result = fclose(fp);
    if (result != 0) {
	ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: file close failed.\n", fileName); 
	return 0;
    }

    authNumber = strtol(buf, NULL, 0);

    /* Is Auth Number smaller than minimum? */
    if (authNumber < NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Simple Auth Number %d is smaller than minimum %d.\n",
             arg, authNumber, NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN); 
	return 0;
    }

    /* Is Auth Number greater than maximum? */
    if (authNumber > NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Simple Auth Number %d is greater than maximum %d.\n",
            arg, authNumber, NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX); 
	return 0;
    }

    context->ngc_commInfo.ngci_simpleAuthNumber = authNumber;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --authNumber
 */
static int
ngexlAnalyzeArgumentSimpleAuthNumber(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    int authNumber;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentSimpleAuthNumber";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Get the Auth Number */
    end = &arg[strlen(arg)];
    authNumber = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    /* Is Auth Number smaller than minimum? */
    if (authNumber < NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Simple Auth Number %d is smaller than minimum %d.\n",
             arg, authNumber, NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MIN); 
	return 0;
    }

    /* Is Auth Number greater than maximum? */
    if (authNumber > NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Simple Auth Number %d is greater than maximum %d.\n",
            arg, authNumber, NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_MAX); 
	return 0;
    }

    context->ngc_commInfo.ngci_simpleAuthNumber = authNumber;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --contextID
 */
static int
ngexlAnalyzeArgumentContextID(ngexiContext_t *context, char *arg, int *error)
{
    int contextID;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentContextID";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Get the Context ID */
    end = &arg[strlen(arg)];
    contextID = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    /* Is Context ID smaller than minimum? */
    if (contextID < NGI_CONTEXT_ID_MIN) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Context ID %d is smaller than minimum %d.\n", arg, contextID, NGI_CONTEXT_ID_MIN); 
	return 0;
    }

    /* Is Context ID greater than maximum? */
    if (contextID > NGI_CONTEXT_ID_MAX) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Context ID %d is greater than maximum %d.\n", arg, contextID, NGI_CONTEXT_ID_MAX); 
	return 0;
    }

    /* Copy the Context ID */
    context->ngc_commInfo.ngci_contextID = contextID;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --jobID
 */
static int
ngexlAnalyzeArgumentJobID(ngexiContext_t *context, char *arg, int *error)
{
    int jobID;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentJobID";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Get the Job ID */
    end = &arg[strlen(arg)];
    jobID = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    /* Is Job ID smaller than minimum? */
    if (jobID < NGI_JOB_ID_MIN) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Job ID %d is smaller than minimum %d.\n", arg, jobID, NGI_JOB_ID_MIN); 
	return 0;
    }

    /* Is Job ID greater than maximum? */
    if (jobID > NGI_JOB_ID_MAX) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Context ID %d is greater than maximum %d.\n", arg, jobID, NGI_JOB_ID_MAX); 
	return 0;
    }

    /* Copy the Job ID */
    context->ngc_commInfo.ngci_jobID = jobID;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --heartbeat
 */
static int
ngexlAnalyzeArgumentHeartBeat(ngexiContext_t *context, char *arg, int *error)
{
    int heartBeatInterval;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentHeartBeat";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Get the heartbeat interval */
    end = &arg[strlen(arg)];
    heartBeatInterval = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    if (heartBeatInterval < 0) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: heartbeat %d is smaller than 0.\n", arg, heartBeatInterval); 
	return 0;
    }

    context->ngc_heartBeatInterval = heartBeatInterval;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --connectRetry
 */
static int
ngexlAnalyzeArgumentConnectRetry(ngexiContext_t *context, char *arg, int *error)
{
    double retryIncrease;
    char *p, *end, *endptr, *randomStr, *fixedStr;
    int result, retryCount, retryInterval, retryUseRandom;
    static const char fName[] = "ngexlAnalyzeArgumentConnectRetry";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    retryCount = 0;
    retryInterval = 0;
    retryIncrease = 0.0;
    retryUseRandom = 0;

    /* argument: count,interval,increase,random */
    /* each argument suppress is not allowed. ",," is invalid. */
    
    end = &arg[strlen(arg)];
    p = arg;

    /* Read retryCount */
    retryCount = strtol(p, &endptr, 0);
    if ((endptr == NULL) || (*endptr != ',')) {
        goto error;
    }
    p = endptr;
    p++;

    /* Read retryInterval */
    retryInterval = strtol(p, &endptr, 0);
    if ((endptr == NULL) || (*endptr != ',')) {
        goto error;
    }
    p = endptr;
    p++;

    /* Read retryIncrease */
    retryIncrease = strtod(p, &endptr);
    if ((endptr == NULL) || (*endptr != ',')) {
        goto error;
    }
    p = endptr;
    p++;

    /* Read retryUseRandom */
    randomStr = "random";
    fixedStr = "fixed";

    result = strncmp(p, randomStr, strlen(randomStr));
    if (result == 0) {
        retryUseRandom = 1;
        p += strlen(randomStr);
    } else {
        result = strncmp(p, fixedStr, strlen(fixedStr));
        if (result == 0) {
            retryUseRandom = 0;
            p += strlen(fixedStr);
        } else {
            goto error;
        }
    }

    if ((*p != '\0') || (p != end)) {
        goto error;
    }

    /* Set values */
    if (retryCount > 0) {
        context->ngc_retryInfo.ngcri_count = retryCount;
        context->ngc_retryInfo.ngcri_interval = retryInterval;
        context->ngc_retryInfo.ngcri_increase = retryIncrease;
        context->ngc_retryInfo.ngcri_useRandom = retryUseRandom;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "%s: Argument contain the invalid character.\n", arg); 

    /* Failed */
    return 0;
}

/**
 * Analyze the argument: --workDirectory
 */
static int
ngexlAnalyzeArgumentWorkDirectory(ngexiContext_t *context, char *arg, int *error)
{
    int result;
    char *workDirectory;
    static const char fName[] = "ngexlAnalyzeArgumentWorkDirectory";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    workDirectory = arg;

    /* Change directory to specified workDirectory */
    result = chdir(workDirectory);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Change directory to \"%s\" failed.\n", workDirectory); 
	return 0;
    }

    ngLogDebug(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "Change working directory to \"%s\".\n", workDirectory); 

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --coreDumpSize
 */
static int
ngexlAnalyzeArgumentCoreDumpSize(ngexiContext_t *context, char *arg, int *error)
{
    int coreSize, result;
    struct rlimit coreLimit;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentCoreDumpSize";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Get the Job ID */
    end = &arg[strlen(arg)];
    coreSize = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    /* Is coreDumpSize valid? */
    if (coreSize < -1) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Invalid core size %d.\n", arg, coreSize); 
	return 0;
    }

    if (coreSize == -1) {
	coreLimit.rlim_cur = RLIM_INFINITY;
	coreLimit.rlim_max = RLIM_INFINITY;
    } else {
	coreLimit.rlim_cur = coreSize;
        coreLimit.rlim_max = coreSize;
    }

    result = setrlimit(RLIMIT_CORE, &coreLimit);
    if (result != 0) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "setrlimit system call failed.\n"); 
	return 0;
    }

    if (coreSize == -1) {
	ngLogDebug(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Set limit for core file size to infinite.\n"); 
    } else {
	ngLogDebug(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Set limit for core file size to %dkB.\n", coreSize); 
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --debugTerminal
 */
static int
ngexlAnalyzeArgumentDebugTerminal(ngexiContext_t *context, char *arg, int *error)
{
    char *debugTerminal;
    static const char fName[] = "ngexlAnalyzeArgumentDebugTerminal";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Duplicate the Debug Terminal */
    debugTerminal = strdup(arg);
    if (debugTerminal == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Debug Terminal.\n"); 
	return 0;
    }

    /* Copy the Debug Terminal */
    context->ngc_dbgInfo.ngdi_terminalPath = debugTerminal;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --debugDisplay
 */
static int
ngexlAnalyzeArgumentDebugDisplay(ngexiContext_t *context, char *arg, int *error)
{
    char *debugDisplay;
    static const char fName[] = "ngexlAnalyzeArgumentDebugDisplay";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Duplicate the Debug Display */
    debugDisplay = strdup(arg);
    if (debugDisplay == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Debug Display.\n"); 
	return 0;
    }

    /* Copy the Debug Terminal */
    context->ngc_dbgInfo.ngdi_display = debugDisplay;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --debugger
 */
static int
ngexlAnalyzeArgumentDebugger(ngexiContext_t *context, char *arg, int *error)
{
    char *debugger;
    static const char fName[] = "ngexlAnalyzeArgumentDebugger";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Duplicate the Debugger */
    debugger = strdup(arg);
    if (debugger == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for Debugger.\n"); 
	return 0;
    }

    /* Copy the Debugger */
    context->ngc_dbgInfo.ngdi_debuggerPath = debugger;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --debugEnable
 * Invoke debugger on X terminal and attach to it.
 */
static int
ngexlAnalyzeArgumentDebugEnable(ngexiContext_t *context, char *arg, int *error)
{
    pid_t pid;
    char *debugTerminal, *debugDisplay, *debugger;
    char command[NGEXI_DEBUG_COMMAND_MAX];
    int wroteLength, result;
    static const char fName[] = "ngexlAnalyzeArgumentDebugEnable";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Set members to invoke debugger */
    debugTerminal = context->ngc_dbgInfo.ngdi_terminalPath;
    debugDisplay = context->ngc_dbgInfo.ngdi_display;
    debugger = context->ngc_dbgInfo.ngdi_debuggerPath;

    debugTerminal =
        (debugTerminal != NULL ? debugTerminal : NGEXI_DEBUG_TERMINAL);
    debugger = (debugger != NULL ? debugger : NGEXI_DEBUG_DEBUGGER);

    pid = getpid();

    assert(debugTerminal != NULL);
    assert(debugger != NULL);
    assert(context->ngc_lmInfo.nglmi_path != NULL);

    /* Generate command for debugger */
    wroteLength = snprintf(command, NGEXI_DEBUG_COMMAND_MAX,
	"%s%s%s%s %s %s %s %ld &",
	(debugDisplay != NULL ? "DISPLAY=" : ""),
	(debugDisplay != NULL ? debugDisplay : ""),
	(debugDisplay != NULL ? " " : ""),
	debugTerminal, NGEXI_DEBUG_TERMINAL_OPTION,
        debugger, context->ngc_lmInfo.nglmi_path, (long)pid);

    if (wroteLength <= 0 || wroteLength >= NGEXI_DEBUG_COMMAND_MAX) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Debugger command too long (wrote %d).\n", wroteLength); 
	return 0;
    }

    ngLogInfo(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "Debug enabled. executing command \"%s\"\n", command); 

    /* Execute the debugger command */
    result = system(command);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Debugger command failed (%d).\n", result); 
	return 0;
    }
    /**
     * Note: system("command &") always returns 0,
     *     regardless of existence of command.
     */

    /* Wait for debugger to attach this program */
    ngLogInfo(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "Waiting %d seconds for debugger to attach.\n",
        NGEXI_DEBUG_WAIT_FOR_ATTACH); 
    sleep(NGEXI_DEBUG_WAIT_FOR_ATTACH);

    /* Success */
    return 1;
}

static volatile int debugBusyLoop;

/**
 * Analyze the argument: --debugBusyLoop
 */
static int
ngexlAnalyzeArgumentDebugBusyLoop(ngexiContext_t *context, char *arg, int *error)
{
    static const char fName[] = "ngexlAnalyzeArgumentDebugBusyLoop";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    ngLogInfo(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "Entering busy-loop to wait attach for debugger.\n"); 

    ngLogInfo(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
        "Please attach process %ld by debugger.\n", (long)getpid()); 

    /**
     * Busy loop.
     * waiting for user attach debugger, and set debugBusyLoop = 0.
     * then, continue.
     */
    debugBusyLoop = 1;
    while (debugBusyLoop != 0) {
	sleep(1); /* In GDB, type "set var debugBusyLoop=0", and continue */
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --tcpNodelay
 */
static int
ngexlAnalyzeArgumentTcpNodelay(ngexiContext_t *context, char *arg, int *error)
{
    int enable;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentTcpNodelay";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    end = &arg[strlen(arg)];
    enable = strtol(arg, &endptr, 0);
    if (endptr != end) {
	ngLogWarn(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
	    "%s: Argument contain the invalid character.\n", arg); 
	return 0;
    }

    if (enable != 0) {
        ngLogInfo(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "set TCP_NODELAY to true.\n"); 
    }

    context->ngc_commInfo.ngci_tcpNodelay = enable;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --communicationProxyType
 */
static int
ngexlAnalyzeArgumentCommunicationProxyType(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    char *type = NULL;
    ngLog_t *log;
    static const char fName[] = "ngexlAnalyzeArgumentCommunicationProxyType";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    log = context->ngc_log;

    type = ngiStrdup(arg, log, error);
    if (type == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy string.\n");
        return 0;
    }

    context->ngc_commProxyInfo.ngcpi_type = type;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --communicationProxyPath
 */
static int
ngexlAnalyzeArgumentCommunicationProxyPath(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    char *path= NULL;
    ngLog_t *log;
    static const char fName[] = "ngexlAnalyzeArgumentCommunicationProxyPath";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    log = context->ngc_log;

    path = ngiStrdup(arg, log, error);
    if (path == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy string.\n");
        return 0;
    }

    context->ngc_commProxyInfo.ngcpi_path = path;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --communicationProxyOption
 */
static int
ngexlAnalyzeArgumentCommunicationProxyOption(
    ngexiContext_t *context,
    char *arg,
    int *error)
{
    int result;
    ngLog_t *log;
    static const char fName[] = "ngexlAnalyzeArgumentCommunicationProxyOption";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    log = context->ngc_log;

    result = ngiLineListAppend(
        context->ngc_commProxyInfo.ngcpi_options,
        arg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't append strings.\n");
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Check the information given by arguments
 */
static int
ngexlAnalyzeArgumentCheckValue(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    ngexiCommunicationInformation_t *commInfo;
    static const char fName[] = "ngexlAnalyzeArgumentCheckValue";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;
    commInfo = &context->ngc_commInfo;

    if (commInfo->ngci_hostname == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Invalid hostname.\n");
        return 0;
    }

    if (commInfo->ngci_simpleAuthNumber ==
        NGI_PROTOCOL_SIMPLE_AUTH_NUMBER_UNDEFINED) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Invalid Simple Auth Number %d.\n",
            commInfo->ngci_simpleAuthNumber);
        return 0;
    }

    if ((commInfo->ngci_contextID < NGI_CONTEXT_ID_MIN) ||
        (commInfo->ngci_contextID > NGI_CONTEXT_ID_MAX)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Invalid Context ID %d.\n",
            commInfo->ngci_contextID);
        return 0;
    }

    if ((commInfo->ngci_jobID < NGI_JOB_ID_MIN) ||
        (commInfo->ngci_jobID > NGI_JOB_ID_MAX)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Invalid Job ID %d.\n",
            commInfo->ngci_jobID);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Make the file name of Configuration of user.
 */
static char *
ngexlMakeUserConfigFileName(
    ngLog_t *log,
    int *error)
{
    struct passwd *passwd = NULL;
    char *buf = NULL;
    char *fileName = NULL;
    char *tmp;
    int result;
    static const char fName[] = "ngexlMakeUserConfigFileName";

    /* Functionality for developers */
    tmp = getenv("NG_STUBRC");
    if (tmp != NULL) {
        if (strcmp(tmp, "") != 0) {
            fileName = strdup(tmp);
            if (fileName == NULL) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "strdup failed.\n"); 
                return NULL;
            }

            return fileName;
        } else {
	    ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "empty NG_STUBRC environment variable was set.\n"); 
        }
    }

    /* Get the passwd database */
    result = ngiGetpwuid(geteuid(), &passwd, &buf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, "ngiGetpwuid() failed.\n");
        return 0;
    }

    /* Is home directory valid? */
    if (passwd->pw_dir == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Home directory is NULL.\n"); 
	goto error;
    }

    /* Allocate the storage for file name */
    fileName = ngiCalloc(
	strlen(passwd->pw_dir) +
	strlen(NGEXI_CONFIG_FILENAME_USER) + 1, sizeof(char), log, error);
    if (fileName == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't allocate the storage for User Configuration File.\n"); 
	goto error;
    }

    /* Make the file name */
    sprintf(fileName, "%s%s", passwd->pw_dir, NGEXI_CONFIG_FILENAME_USER);

    result = ngiReleasePasswd(passwd, buf, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Release the password failed.\n");
	goto error;
    }

    /* Success */
    return fileName;

    /* Error occurred */
error:
    if ((passwd != NULL) || (buf != NULL)) {
    	result = ngiReleasePasswd(passwd, buf, log, NULL);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
		"Release the password failed.\n");
	}
    }

    /* Failed */
    return NULL;
}

/**
 * Release the configuration file name.
 */
static int
ngexlReleaseConfigFileName(char *fileName, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(fileName != NULL);

    /* Deallocate */
    ngiFree(fileName, log, error);

    /* Success */
    return 1;
}

/**
 * Output the Remote Executable information for log file.
 */
static int
ngexlContextLogOutput(
    ngexiContext_t *context,
    int argc,
    char *argv[],
    int *error)
{
    ngLog_t *log;
    int result, i;
    char hostName[NGI_HOST_NAME_MAX], *str;
    char workingDirectory[NGI_DIR_NAME_MAX], *resultPtr;
    static const char fName[] = "ngexlContextLogOutput";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    /* invoked */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable log was created.\n"); 

    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Ninf-G Executable was invoked.\n"); 

    /* hostname */
    result = ngiHostnameGet(hostName, NGI_HOST_NAME_MAX, log, NULL);
    if (result == 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get hostname.\n"); 
        /* not return */
    } else {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "host name is \"%s\".\n", hostName); 
    }

    /* pid */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "process id = %ld.\n", (long)getpid()); 

    /* argv */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "The number of invocation arguments = %d.\n", argc); 

    for (i = 0; i < argc; i++) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Argument No.%d : \"%s\".\n", i, argv[i]); 
    }

    /* current working directory */
    resultPtr = getcwd(workingDirectory, NGI_DIR_NAME_MAX);
    if (resultPtr == NULL) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't get current working directory.\n"); 
        /* not return */
    } else {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "cwd : \"%s\".\n", workingDirectory); 
    }

    /* pthread */
    str = "NonThread";
#ifdef NG_PTHREAD
    str = "Pthread";
#endif /* NG_PTHREAD */

    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "This Executable binary is %s version.\n", str); 

    /* Success */
    return 1;
}

/**
 * Register the signal.
 */
static int
ngexlContextSignalRegister(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    int *signalTable, size, result;
    static const char fName[] = "ngexlContextSignalRegister";

    /* Check the arguments */
    assert(context != NULL);

    log = context->ngc_log;

    size = 0;
    signalTable = context->ngc_lmInfo.nglmi_signals;

    if (signalTable != NULL) {
        /* Find the tail */
        for (; signalTable[size] != 0; size++);
    } else {
        signalTable = NULL; /* NULL is default, size 0 is no-signal */
    }

    result = ngexiSignalManagerRegister(
        signalTable, size, log, error);
    if (result == 0) { 
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't register the Signal.\n"); 
        return 0;
    }

    result = ngexiSignalManagerStart(log, error);
    if (result == 0) { 
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't start the Signal Manager.\n"); 
        return 0;
    }

    result = ngexiSignalManagerLogSet(log, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the log for Signal Manager.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Switch stdout/stderr to file.
 */
int
ngexiContextSwitchStdoutStderr(
    ngexiContext_t *context,
    int execID,
    int *error)
{
    FILE *fp, *targetFp1, *targetFp2;
    int result, i, fd, targetFd, targetFd2, targetSetNoBuf, sameFile;
    char buf[NGI_FILE_NAME_MAX];
    char *stdoutFile, *stderrFile, *targetName, *targetFileName, *targetFile;
    static const char fName[] = "ngexiContextSwitchStdoutStderr";

    /* Check the arguments */
    assert(context != NULL);

    targetName = NULL;
    targetFileName = NULL;
    targetFd = 0;
    targetFd2 = 0;
    targetFp1 = NULL;
    targetFp2 = NULL;
    targetSetNoBuf = 0;

    stdoutFile = context->ngc_lmInfo.nglmi_saveStdout;
    stderrFile = context->ngc_lmInfo.nglmi_saveStderr;

    if ((stdoutFile == NULL) && (stderrFile == NULL)) {

        /* Success */
        return 1;
    }

    sameFile = 0;
    if (((stdoutFile != NULL) && (stderrFile != NULL)) &&
        (strcmp(stdoutFile, stderrFile) == 0)) {
        sameFile = 1;
    }

    for (i = 0; i < 3; i++) {
        switch (i) {
        case 0:
            if (sameFile != 0) {
                continue;
            }
            if (stdoutFile == NULL) {
                continue;
            }

            /* stdout */
            targetName = "stdout";
            targetFileName = stdoutFile;
            targetFd = STDOUT_FILENO;
            targetFd2 = 0; /* not used */
            targetFp1 = stdout;
            targetFp2 = NULL;
            targetSetNoBuf = NGEXI_SAVE_STDOUT_NO_BUFFERING;

            break;
        case 1:
            if (sameFile != 0) {
                continue;
            }
            if (stderrFile == NULL) {
                continue;
            }

            /* stderr */
            targetName = "stderr";
            targetFileName = stderrFile;
            targetFd = STDERR_FILENO;
            targetFd2 = 0; /* not used */
            targetFp1 = stderr;
            targetFp2 = NULL;
            targetSetNoBuf = 1;

            break;
        case 2:
            if (sameFile == 0) {
                continue;
            }
            assert(stdoutFile != NULL);
            assert(stderrFile != NULL);

            /* stdout/stderr output to same file */
            targetName = "stdout/stderr";
            targetFileName = stdoutFile;
            targetFd = STDOUT_FILENO;
            targetFd2 = STDERR_FILENO;
            targetFp1 = stdout;
            targetFp2 = stderr;
            targetSetNoBuf = 1;

            break;
        default:
            /* NOT REACHED */
            abort();
        }

        /* Append Executable ID to filename */
        result = snprintf(buf, sizeof(buf), "%s-execID-%d",
            targetFileName, execID);
        if (result >= sizeof(buf)) {
            NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to generate %s filename.\n", targetName); 
            return 0;
        }

        targetFile = buf;

        /* log */
        ngLogDebug(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Switching %s output to file \"%s\".\n", targetName, targetFile); 

        /* Open file */
        fp = fopen(targetFile, "a");
        if (fp == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to open %s file \"%s\" (%s).\n",
                targetName, targetFile, strerror(errno)); 
            return 0;
        }

        fd = fileno(fp);

        /* Connect to stdout/stderr */
        result = dup2(fd, targetFd);
        if (result < 0) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to switch %s to file \"%s\" (%s).\n",
                targetName, targetFile, strerror(errno)); 
            return 0;
        }

        if (sameFile != 0) {
            result = dup2(fd, targetFd2);
            if (result < 0) {
                NGI_SET_ERROR(error, NG_ERROR_FILE);
                ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Failed to switch %s to same file \"%s\" (%s).\n",
                    targetName, targetFile, strerror(errno)); 
                return 0;
            }
        }

        /* Disable buffering */
        if (targetSetNoBuf != 0) {
            setvbuf(targetFp1, NULL, _IONBF, 0);

            if (targetFp2 != NULL) {
                setvbuf(targetFp2, NULL, _IONBF, 0);
            }
        }
    }
    
    /* Success */
    return 1;
}

/**
 * Get the Remote Method Information.
 */
ngRemoteMethodInformation_t *
ngexiRemoteMethodInformationGet(
    ngexiContext_t *context,
    int methodID, 
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngexiRemoteMethodInformationGet";

    /* Check the arguments */
    assert(context != NULL);

    /* Is methodID valid? */
    if (methodID < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Method ID %d is smaller than zero.\n", methodID); 
	return NULL;
    }
    if (methodID >= context->ngc_rcInfo->ngrci_nMethods) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Method ID %d is greater than maximum.\n", methodID); 
	return NULL;
    }

    /* Found */
    return &context->ngc_rcInfo->ngrci_method[methodID];
}

/**
 * Set the Session ID.
 */
void
ngexiContextSetSessionID(ngexiContext_t *context, int sessionID)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Set the Method ID */
    context->ngc_sessionID = sessionID;
}

/**
 * Get the Session ID.
 */
int
ngexiContextGetSessionID(ngexiContext_t *context)
{
    /* Check the arguments */
    assert(context != NULL);

    return context->ngc_sessionID;
}

/**
 * Set the Method ID.
 */
void
ngexiContextSetRemoteMethodID(ngexiContext_t *context, int methodID)
{
    /* Check the arguments */
    assert(context != NULL);
    assert((methodID >= 0) && (methodID < context->ngc_rcInfo->ngrci_nMethods));

    /* Set the Method ID */
    context->ngc_methodID = methodID;
}

/**
 * Get the Method ID.
 */
int
ngexiContextGetMethodID(ngexiContext_t *context)
{
    /* Check the arguments */
    assert(context != NULL);

    return context->ngc_methodID;
}

/**
 * Make the Session Information.
 */
int
ngexiContextMakeSessionInformation(
    ngexiContext_t *context,
    ngiArgument_t *argument,
    ngLog_t *log,
    int *error)
{
    int result;
    int i;
    int retResult = 1;
    int getInfo = 0;
    ngSessionInformation_t sessionInfo;
    static const char fName[] = "ngexiContextMakeSessionInformation";

    /* Check the arguments */
    assert(context != NULL);
    assert(argument != NULL);
    assert(context->ngc_sessionInfo.ngsi_compressionInformation == NULL);

    /* Get Session Information measured by Protocol */
    result = ngiProtocolGetSessionInfo(
	context->ngc_protocol, &sessionInfo, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't get the Session Information.\n"); 
	error = NULL;
	retResult = 0;
	goto error;
    }
    getInfo = 1;

    /* Allocate the storage for Compression Information */
    if (argument->nga_nArguments > 0) {
        context->ngc_sessionInfo.ngsi_compressionInformation =
            ngiCalloc(
            argument->nga_nArguments,
            sizeof(*context->ngc_sessionInfo.ngsi_compressionInformation),
            log, error);
        if (context->ngc_sessionInfo.ngsi_compressionInformation
            == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't allocate the storage for Compression Information.\n"); 
            error = NULL;
            retResult = 0;
            goto error;
        }
    }

    /* Copy the Compression Informations */
    context->ngc_sessionInfo.ngsi_nCompressionInformations =
	argument->nga_nArguments;
    for (i = 0; i < argument->nga_nArguments; i++) {
	context->ngc_sessionInfo.ngsi_compressionInformation[i] =
	    sessionInfo.ngsi_compressionInformation[i];
    }

error:
    if (getInfo != 0) {
	getInfo = 0;
	result = ngiProtocolReleaseSessionInfo(
	    context->ngc_protocol, &sessionInfo, log, error);
	if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get the Session Information.\n"); 
	    /* Not returned */
	    error = NULL;
	    retResult = 0;
	}
    }

    /* Finish */
    return retResult;
}

/**
 * Release the Session Information.
 */
int
ngexiContextReleaseSessionInformation(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Deallocate the storage for Compression Information */
    if (context->ngc_sessionInfo.ngsi_compressionInformation != NULL);
	ngiFree(
            context->ngc_sessionInfo.ngsi_compressionInformation,
            log, error);
    context->ngc_sessionInfo.ngsi_nCompressionInformations = 0;
    context->ngc_sessionInfo.ngsi_compressionInformation = NULL;

    /* Success */
    return 1;
}

/**
 * Get the Session Information.
 */
void
ngexiSessionInformationGet(
    ngexiContext_t *context,
    ngexiSessionInformation_t *sessInfo)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(sessInfo != NULL);

    *sessInfo = context->ngc_sessionInfo;
}

/**
 * Check the Executable status.
 */
int
ngexiContextExecutableStatusCheck(
    ngexiContext_t *context,
    const ngexiExecutableStatus_t * const reqStatus,
    int nStatus,
    ngLog_t *log,
    int *error)
{
    int i;
    char *string;
    static const char fName[] = "ngexiContextExecutableStatusCheck";

    /* Check the arguments */
    assert(context != NULL);
    assert(reqStatus != NULL);
    assert(nStatus > 0);

    /* Get the current status */
    for (i = 0; i < nStatus; i++) {
	if (context->ngc_executableStatus == reqStatus[i]) {
	    /* Found */
	    return 1;
	}
    }

    /**
     * Not found
     */
    /* Get the Status String */
    string = ngexiContextExecutableStatusStringGet(
	context->ngc_executableStatus, log, NULL);
    if (string == NULL) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state %d.\n", context->ngc_executableStatus); 
        return 0;
    }

    /* Set the error code */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Invalid state: Now %s.\n", string); 

    return 0;
}

/**
 * Set the status of Executable.
 */
int
ngexiContextExecutableStatusSet(
    ngexiContext_t *context,
    ngexiExecutableStatus_t status,
    int *error)
{
    int result;
    ngLog_t *log;
    int mutexLocked;
    char *oldStatusString, *newStatusString;
    static const char fName[] = "ngexiContextExecutableStatusSet";

    /* Check the arguments */
    assert(context != NULL);
    assert((status > NGEXI_EXECUTABLE_STATUS_MIN) &&
           (status <  NGEXI_EXECUTABLE_STATUS_MAX));

    /* Initialize the local variables */
    log = context->ngc_log;
    oldStatusString = NULL;
    newStatusString = NULL;
    mutexLocked = 0;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex for status.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Print the debug message */
    oldStatusString = ngexiContextExecutableStatusStringGet(
        context->ngc_executableStatus, log, error);
    if (oldStatusString == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to get old status string.\n"); 
        goto error;
    }

    newStatusString = ngexiContextExecutableStatusStringGet(
        status, log, error);
    if (newStatusString == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to get new status string.\n"); 
        goto error;
    }

    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Executable status \"%s\" -> \"%s\".\n",
        oldStatusString, newStatusString); 

    /* Set the status */
    context->ngc_executableStatus = status;

    /* Notify signal */
    result = ngiCondBroadcast(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't signal the Condition Variable for status.\n"); 
        goto error;
    }
    
    /* Unlock the mutex */
    assert(mutexLocked != 0);
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    mutexLocked = 0;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex for status.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        mutexLocked = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex for status.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Get the status of Executable.
 */
ngexiExecutableStatus_t
ngexiContextExecutableStatusGet(
    ngexiContext_t *context,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    return context->ngc_executableStatus;
}

/**
 * Wait the status of Executable.
 */
int
ngexiContextExecutableStatusWait(
    ngexiContext_t *context,
    ngexiExecutableStatus_t *waitStatus,
    int nStatus,
    ngexiExecutableStatus_t *resultStatus,
    int *error)
{
    int i;
    int result;
    int timeout;
    char *string;
    ngLog_t *log;
    int mutexLocked, signalBlocked;
    static const char fName[] = "ngexiContextExecutableStatusWait";

    /* Check the arguments */
    assert(context != NULL);
    assert(nStatus > 0);
    for (i = 0; i < nStatus; i++) {
        assert((waitStatus[i] > NGEXI_EXECUTABLE_STATUS_MIN) &&
            (waitStatus[i] <  NGEXI_EXECUTABLE_STATUS_MAX));
    }
    assert(resultStatus != NULL);

    /* Initialize the local variables */
    log = context->ngc_log;
    string = NULL;
    mutexLocked = 0;
    signalBlocked = 0;
    timeout = 0;
    assert(timeout == 0); /* To avoid warning */

    *resultStatus = NGEXI_EXECUTABLE_ERROR;

    /* Print the debug message */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Waiting for the Executable status grow.\n"); 

    /* Block the signal for NonThread */
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_BLOCK_READ, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't block signal.\n"); 
        goto error;
    }
    signalBlocked = 1;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex for status.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /* Wait the status */
    while ((ngexlContextExecutableStatusMatch(
        context->ngc_executableStatus, waitStatus, nStatus) == 0) &&
        (context->ngc_executableStatus != NGEXI_EXECUTABLE_STATUS_END)) {

        result = ngiCondWait(
            &context->ngc_cond, &context->ngc_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable for status.\n"); 
            goto error;
        }
    }

    /* Set result */
    *resultStatus = context->ngc_executableStatus;

    /* log */
    string = ngexiContextExecutableStatusStringGet(*resultStatus, log, NULL);
    if (string == NULL) {
        string = "Not valid status";
    }
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "status was grown into \"%s\" (%d). return.\n", string, *resultStatus); 

    /* Is error occurred? */
    if (context->ngc_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, context->ngc_error);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Is error occurred in callback? */
    if (context->ngc_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, context->ngc_cbError);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Error was occurred while waiting status.\n"); 
        goto error;
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    mutexLocked = 0;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex for status.\n"); 
        goto error;
    }

    /* Unblock the signal */
    assert(signalBlocked != 0);
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK_READ, error);
    signalBlocked = 0;
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unblock signal.\n"); 
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Unlock the mutex */
    if (mutexLocked != 0) {
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        mutexLocked = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex for status.\n"); 
        }
    }

    /* Unblock the signal */
    if (signalBlocked != 0) {
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK_READ, NULL);
        signalBlocked = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unblock signal.\n"); 
        }
    }

    /* Failed */
    return 0;
}

/**
 * Check if the status of Executable match one of given set.
 */
static int
ngexlContextExecutableStatusMatch(
    ngexiExecutableStatus_t checkStatus,
    ngexiExecutableStatus_t *statusSet,
    int nStatus)
{
    int i;

    assert(statusSet != NULL);

    for (i = 0; i < nStatus; i++) {
        if (statusSet[i] == checkStatus) {
            /* The match was found */
            return 1;
        }
    }

    /* Not found */
    return 0;
}

/**
 * Convert the Executable status to string.
 */
char *
ngexiContextExecutableStatusStringGet(
    ngexiExecutableStatus_t status,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngexiContextExecutableStatusStringGet";

    static const struct {
	ngexiExecutableStatus_t status;
	char *string;
    } convert[] = {
	{NGEXI_EXECUTABLE_STATUS_INITIALIZING,         "Initializing"},
	{NGEXI_EXECUTABLE_STATUS_IDLE,                 "Idle"},
	{NGEXI_EXECUTABLE_STATUS_INVOKED,              "Invoked"},
	{NGEXI_EXECUTABLE_STATUS_TRANSFER_ARGUMENT,    "Transfer Argument Data"},
	{NGEXI_EXECUTABLE_STATUS_CALCULATING,          "Calculating"},
	{NGEXI_EXECUTABLE_STATUS_SUSPENDED,            "Suspended"},
	{NGEXI_EXECUTABLE_STATUS_CALCULATION_END,      "Calculation End"},
	{NGEXI_EXECUTABLE_STATUS_TRANSFER_RESULT,      "Transfer Result Data"},
	{NGEXI_EXECUTABLE_STATUS_PULL_WAIT,            "Pull Wait"},
	{NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED,     "Cancel Requested"},
	{NGEXI_EXECUTABLE_STATUS_CB_NOTIFY,            "Callback Notify"},
	{NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_ARGUMENT, "Callback Transfer Argument Data"},
	{NGEXI_EXECUTABLE_STATUS_CB_WAIT,              "Callback Wait"},
	{NGEXI_EXECUTABLE_STATUS_CB_TRANSFER_RESULT,   "Callback Transfer Result Data"},
	{NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED,      "Reset Requested"},
	{NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED,       "Exit Requested"},
	{NGEXI_EXECUTABLE_STATUS_END,                  "End"},
    };

    int i;

    for (i = 0; i < NGI_NELEMENTS(convert); i++) {
	if (convert[i].status == status) {
	    /* Found */
	    return convert[i].string;
	}
    }

    /* Not found */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Unknown Executable status %d.\n", status); 

    return NULL;
}

/**
 * Set the error code.
 */
int
ngexiContextSetError(
    ngexiContext_t *context,
    int setError,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Set the error code */
    context->ngc_error = setError;

    /* Success */
    return 1;
}

/**
 * Get the error code.
 */
int
ngexiContextGetError(
    ngexiContext_t *context,
    int *error)
{
    int retError;

    /* Check the arguments */
    assert(context != NULL);

    /* Get the error code */
    retError = context->ngc_error;

    /* Success */
    return retError;
}

/**
 * Set the error code occurred by I/O callback.
 */
int
ngexiContextSetCbError(
    ngexiContext_t *context,
    int setError,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Set the error code */
    context->ngc_cbError = setError;

    /* Success */
    return 1;
}

/**
 * Get the error code occurred by I/O callback.
 */
int
ngexiContextGetCbError(
    ngexiContext_t *context,
    int *error)
{
    int retError;

    /* Check the arguments */
    assert(context != NULL);

    /* Get the error code */
    retError = context->ngc_cbError;

    /* Success */
    return retError;
}

/**
 * Set the Executable Unusable.
 */
int
ngexiContextUnusable(
    ngexiContext_t *context,
    int errorCause,
    int *error)
{
    int result;
    int retResult;
    ngLog_t *log;
    ngiCommunication_t *comm;
    static const char fName[] = "ngexiContextUnusable";

    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the local variables */
    log = context->ngc_log;
    retResult = 1;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Setting this executable unusable.\n"); 

    /* Get the Communication */
    comm = NULL;
    if ((context->ngc_protocol != NULL) &&
        (context->ngc_protocol->ngp_communication != NULL)) {
        comm = context->ngc_protocol->ngp_communication;
    }

    if (comm != NULL) {
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Closing Communication.\n"); 

        /* Close the connection and finish I/O callback */
        result = ngiCommunicationClose(comm, log, error);
        if (result == 0) {
            retResult = 0;
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't close communication.\n"); 
        }
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Setting error code %d.\n", errorCause); 
    
    /* Check error */
    if (errorCause == NG_ERROR_NO_ERROR) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Setting error to NO_ERROR.\n"); 
    }
    
    /* Set the error */
    result = ngexiContextSetError(context, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the error.\n"); 
    }

    /* Set the I/O callback error */
    result = ngexiContextSetCbError(context, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the I/O callback error.\n"); 
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_END, error);
    if (result == 0) {
        retResult = 0;
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't set the status.\n"); 
    }

    /* Exit */
    result = ngexlContextExitOnError(context, error);
    if (result == 0) {
        retResult = 0;
        ngLogError(log,
            NG_LOGCAT_NINFG_PURE, fName,
            "Can't process the exit.\n");
    }

    return retResult;
}

/**
 * Exit on error.
 */
static int
ngexlContextExitOnError(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    static const char fName[] = "ngexlContextExitOnError";

    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the local variables */
    log = context->ngc_log;

    /* Check the configuration. */
    if (context->ngc_lmInfo.nglmi_continueOnError != 0) {
	ngLogInfo(log,
	    NG_LOGCAT_NINFG_PURE, fName,
	    "No exit performed. wait the user defined function finish.\n");

	return 1;
    }

    /* log */
    ngLogInfo(log,
        NG_LOGCAT_NINFG_PURE, fName,
        "Ninf-G Executable is exiting.\n");

    ngexlContextProcessImmediateExit(context, error);

    return 1;
}

/**
 * Exit the process.
 */
static void
ngexlContextProcessImmediateExit(
    ngexiContext_t *context,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /**
     * Terminate all threads immediately,
     * including user defined IDL functions.
     */
    exit(1);
}


/**
 * Set the Argument for Method.
 */
void
ngexiContextSetMethodArgument(ngexiContext_t *context, ngiArgument_t *arg)
{
    /* Check the arguments */
    assert(context != NULL);

    context->ngc_methodArgument = arg;
}

/**
 * Get the Argument for Method.
 */
void
ngexiContextGetMethodArgument(
    ngexiContext_t *context,
    ngiArgument_t **arg)
{
    /* Check the arguments */
    assert(context != NULL);

    *arg = context->ngc_methodArgument;
}

/**
 * Release the Argument for Method.
 */
int
ngexiContextReleaseMethodArgument(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextReleaseMethodArgument";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_methodArgument != NULL);

    /* Release the Argument */
    result = ngiArgumentDestruct(context->ngc_methodArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't release the Argument Data.\n"); 
	return 0;
    }

    /* Success */
    context->ngc_methodArgument = NULL;
    return  1;
}

/**
 * Set the Argument for Callback.
 */
void
ngexiContextSetCallbackArgument(
    ngexiContext_t *context,
    ngiArgument_t *arg)
{
    /* Check the arguments */
    assert(context != NULL);

    context->ngc_callbackArgument = arg;
}

/**
 * Get the Argument for Callback.
 */
void
ngexiContextGetCallbackArgument(
    ngexiContext_t *context,
    ngiArgument_t **arg)
{
    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);

    *arg = context->ngc_callbackArgument;
}

/**
 * Release the Argument for Callback.
 */
int
ngexiContextReleaseCallbackArgument(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextReleaseCallbackArgument";

    /* Check the arguments */
    assert(context != NULL);
    assert(context->ngc_callbackArgument != NULL);

    /* Release the Argument */
    result = ngiArgumentDestruct(context->ngc_callbackArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't release the Argument Data.\n"); 
	return 0;
    }

    /* Success */
    context->ngc_callbackArgument = NULL;
    return  1;
}

/**
 * Connection Close.
 */
int
ngexiContextConnectionClose(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked;
    static const char fName[] = "ngexiContextConnectionClose";

    /* Check the atguments */
    assert(context != NULL);

    mutexLocked = 0;
    log = context->ngc_log;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /**
     * Do not cond_wait() because Connection Close cannot stop
     * and wait, in I/O callback.
     */
    if (context->ngc_connectLocked != 0) {

        /* Already Locked. close() will performed later. */
        context->ngc_connectCloseRequested = 1;

    } else if (context->ngc_connecting != 0) {

        /* Destruct the Communication */
        result = ngexlContextCommunicationDestruct(context, error);
        if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Failed to destruct the communication.\n"); 
            goto error;
        }
        context->ngc_connecting = 0;

    } else {
        /* Do nothing */
        assert(context->ngc_connecting == 0);
    }

#ifndef NG_PTHREAD
    /* for NonThread ConnectionCloseWait(). */

    /* Notify signal */
    result = ngiCondBroadcast(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't signal the Condition Variable for Connection.\n"); 
        goto error;
    }
#endif /* NG_PTHREAD */

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    
    /* Failed */
    return 0;
}

/**
 * Destruct the Communication on Context.
 */
static int
ngexlContextCommunicationDestruct(
    ngexiContext_t *context,
    int *error)
{
    int result;
    ngLog_t *log;
    ngiProtocol_t *protocol;
    ngiCommunication_t *comm;
    static const char fName[] = "ngexlContextCommunicationDestruct";

    /* Check the atguments */
    assert(context != NULL);

    log = context->ngc_log;
    protocol = context->ngc_protocol;
    comm = context->ngc_communication;

    /* Clear the communication on protocol. */
    result = ngiProtocolSetCommunication(
        protocol, NULL, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Failed to clear the communication on protocol.\n"); 
        goto error;
    }
    context->ngc_communication = NULL;

    /* Destruct the Communication */
    if (comm != NULL) {
        result = ngiCommunicationDestruct(comm, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to destruct the communication.\n"); 
            goto error;
        }
    }

    /* Success */
    return  1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/**
 * Establish the connection and Lock.
 */
int
ngexiContextConnectionEstablishAndLock(
    ngexiContext_t *context,
    int *connectPerformed,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked, signalBlocked;
    ngiProtocol_t *protocol;
    ngiCommunication_t *comm;
    static const char fName[] = "ngexiContextConnectionEstablishAndLock";

    /* Check the atguments */
    assert(context != NULL);
    assert(connectPerformed != NULL);

    mutexLocked = 0;
    signalBlocked = 0;
    log = context->ngc_log;

    *connectPerformed = 0;
    comm = NULL;
    protocol = context->ngc_protocol;

    /* Block the signal for NonThread */
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_BLOCK_WRITE, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't block signal.\n"); 
        goto error;
    }
    signalBlocked = 1;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    /**
     * Note: Lock by this function is called by sending Notify of
     * heartbeat, invoke_callback, calculation_end.
     * double lock will not happen on NonThread, because:
     * only I/O callback function was called while sending heartbeat.
     * (Send Notify is done by main thread.)
     * While sending Notify invoke_callback or calculation_end,
     * signal is blocked and heartbeat send will not happen.
     */
#ifndef NG_PTHREAD
    if (context->ngc_connectLocked != 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Connection had already locked.\n"); 
        goto error;
    }
#endif /* NG_PTHREAD */

    /* Wait Unlocked */
    while ((context->ngc_connectLocked != 0) &&
        (context->ngc_executableStatus != NGEXI_EXECUTABLE_STATUS_END)) {
        result = ngiCondWait(
            &context->ngc_cond, &context->ngc_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable for Connection.\n"); 
            goto error;
        }
    }
    if (context->ngc_executableStatus == NGEXI_EXECUTABLE_STATUS_END) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The invalid status END.\n"); 
        goto error;
    }

    assert(context->ngc_connectLocked == 0);
    context->ngc_connectLocked = 1;

    if (context->ngc_connecting == 0) {

        /* Construct the Communication */
        comm = ngiCommunicationConstructClient(
            context->ngc_event,
            context->ngc_commInfo.ngci_tcpNodelay,
            context->ngc_commInfo.ngci_connectbackAddress,
            context->ngc_retryInfo,
            &context->ngc_random,
            log, error);
        if (comm == NULL) {
            ngLogError(context->ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't construct the Communication Manager.\n"); 
            goto error;
        }

        /* Set the communication on protocol. */
        result = ngiProtocolSetCommunication(
            protocol, comm, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Failed to set the communication on protocol.\n"); 
            goto error;
        }
        context->ngc_communication = comm;

        /* Negotiation */
        result = ngexiProcessNegotiationSecondConnect(
            context, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Negotiation failed.\n"); 
            goto error;
        }

        context->ngc_connecting = 1;
        *connectPerformed = 1;
    }

    assert(context->ngc_connecting != 0);

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    /* Unblock the signal */
    if (signalBlocked != 0) {
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK_WRITE, NULL);
        signalBlocked = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unblock signal.\n"); 
        }
    }
    
    /* Failed */
    return 0;
}

/**
 * Connection Unlock.
 */
int
ngexiContextConnectionUnlock(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked;
    static const char fName[] = "ngexiContextConnectionUnlock";

    /* Check the atguments */
    assert(context != NULL);

    mutexLocked = 0;
    log = context->ngc_log;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    if (context->ngc_connectLocked == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Connection is not locked.\n"); 
        goto error;
    }

    if ((context->ngc_connectCloseRequested != 0) &&
        (context->ngc_connecting != 0)) {
    
        /* Destruct the Communication */
        result = ngexlContextCommunicationDestruct(context, error);
        if (result == 0) {
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Failed to destruct the communication.\n"); 
            goto error;
        }
        context->ngc_connecting = 0;
        context->ngc_connectCloseRequested = 0;
    }
    assert(context->ngc_connectCloseRequested == 0);

    context->ngc_connectLocked = 0;

    /* Notify signal */
    result = ngiCondBroadcast(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't signal the Condition Variable for Connection.\n"); 
        goto error;
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    /* Unblock the signal */
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK_WRITE, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unblock signal.\n"); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

#ifdef NG_PTHREAD

/**
 * Wait the Connection Close. (Do nothing)
 */
int
ngexiContextConnectionCloseWait(
    ngexiContext_t *context,
    int *error)
{
    /* Success */
    return  1;
}

#else /* NG_PTHREAD */

/**
 * Wait the Connection Close or Cancel Session.
 * Note: heartbeat send with connect involves connection close.
 * NonThread cannot find it if this function will not called.
 */
int
ngexiContextConnectionCloseWait(
    ngexiContext_t *context,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked, isTimeout;
    static const char fName[] = "ngexiContextConnectionCloseWait";

    /* Check the atguments */
    assert(context != NULL);

    mutexLocked = 0;
    log = context->ngc_log;
    isTimeout = 0;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    while (1) {
        /* Is status valid? */
        if (context->ngc_executableStatus == NGEXI_EXECUTABLE_STATUS_END) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unexpected END status.\n"); 
            goto error;
        }

        /* Is session canceled? */
        if (context->ngc_executableStatus ==
            NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED) {
            break;
        }

        /* Is Connection closed? */
        if ((context->ngc_connectCloseRequested != 0) ||
            (context->ngc_connecting == 0)) {
            break;
        }

        /* Wait the Connection Close with timeout. */
        result = ngiCondTimedWait(
            &context->ngc_cond, &context->ngc_mutex,
            NGEXI_HEARTBEAT_WAIT_CLOSE_AFTER_CONNECT,
            &isTimeout, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable for Connection.\n"); 
            goto error;
        }

        if (isTimeout != 0) {
            break;
        }
    }

    if (isTimeout != 0) {
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "No connection close request was found. wait %d second.\n",
            NGEXI_HEARTBEAT_WAIT_CLOSE_AFTER_CONNECT); 
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}


#endif /* NG_PTHREAD */

/**
 * Start the After Connection Close wait.
 */
int
ngexiContextAfterCloseWaitStart(
    ngexiContext_t *context,
    int *error)
{
    /* Check the atguments */
    assert(context != NULL);

    context->ngc_afterCloseArrived = 0;
    context->ngc_afterCloseType = NGEXI_AFTER_CLOSE_TYPE_NONE;

    /* Success */
    return  1;
}

/**
 * End the After Connection Close wait.
 */
int
ngexiContextAfterCloseWaitEnd(
    ngexiContext_t *context,
    int *error)
{
    /* Check the atguments */
    assert(context != NULL);

    context->ngc_afterCloseArrived = 0;
    context->ngc_afterCloseType = NGEXI_AFTER_CLOSE_TYPE_NONE;

    /* Success */
    return  1;
}

/**
 * Wait the After Close event.
 * 1. Transfer Result Data Request or Connection Close.
 * 2. Transfer Callback Argument Data or Connection Close.
 *
 * Note: sending heartbeat cannot use this function, because
 * context->ngc_afterClose* have race condition.
 */
int
ngexiContextAfterCloseWait(
    ngexiContext_t *context,
    ngexiAfterCloseType_t type,
    int *closed,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked;
    ngexiAfterCloseType_t arrivedType;
    static const char fName[] = "ngexiContextAfterCloseWait";

    /* Check the atguments */
    assert(context != NULL);
    assert(type > NGEXI_AFTER_CLOSE_TYPE_NONE);
    assert(type < NGEXI_AFTER_CLOSE_TYPE_NOMORE);
    assert(closed != NULL);

    mutexLocked = 0;
    log = context->ngc_log;
    arrivedType = NGEXI_AFTER_CLOSE_TYPE_NONE;

    *closed = 0;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    while (1) {
        /* Is status valid? */
        if (context->ngc_executableStatus == NGEXI_EXECUTABLE_STATUS_END) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unexpected END status.\n"); 
            goto error;
        }

        /* Is Connection closed? */
        if (context->ngc_connecting == 0) {
            *closed = 1;
            break;
        }
 
        /* Is Request caused by Notify arrived? */
        if (context->ngc_afterCloseArrived != 0) {
            arrivedType = context->ngc_afterCloseType; 
            *closed = 0;
            break;
        }

        /* Wait the Request or Connection Close. */
        result = ngiCondWait(
            &context->ngc_cond, &context->ngc_mutex, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the Condition Variable for Connection.\n"); 
            goto error;
        }
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    if ((*closed == 0) && (arrivedType != type)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The type mismatch. (wait type %d, arrived type %d.\n",
            type, arrivedType); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Notify the Request arrived or Connection Close.
 * arrived != 0 : Transfer Result Data or Transfer Callback Argument Data
 *                Request was arrived.
 * arrived == 0 : Connection was closed.
 */
int
ngexiContextAfterCloseArrived(
    ngexiContext_t *context,
    ngexiAfterCloseType_t type,
    int arrived,
    int *error)
{
    ngLog_t *log;
    int result, mutexLocked;
    static const char fName[] = "ngexiContextAfterCloseArrived";

    /* Check the atguments */
    assert(context != NULL);
    assert(type >= NGEXI_AFTER_CLOSE_TYPE_NONE);
    assert(type < NGEXI_AFTER_CLOSE_TYPE_NOMORE);

    mutexLocked = 0;
    log = context->ngc_log;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't lock the Mutex.\n"); 
        goto error;
    }
    mutexLocked = 1;

    if (arrived != 0) {
        context->ngc_afterCloseType = type;
        context->ngc_afterCloseArrived = 1;
    }

    /**
     * Note: If the Connection was closed, No flags will be set.
     */

    /* Notify the signal. */
    result = ngiCondBroadcast(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't signal the Condition Variable for Connection.\n"); 
        goto error;
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    mutexLocked = 0;
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unlock the Mutex.\n"); 
        goto error;
    }

    /* Success */
    return  1;

    /* Error occurred */
error:
    if (mutexLocked != 0) {
        mutexLocked = 0;
        result = ngiMutexUnlock(&context->ngc_mutex, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unlock the Mutex.\n"); 
            goto error;
        }
    }

    /* Failed */
    return 0;
}

/**
 * Initialize the Session Information.
 */
int
ngexiContextInitializeSessionInformation(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    /* Check the arguments */
    assert(context != NULL);

    /* Initialize the Session Information */
    memset(&context->ngc_sessionInfo, 0, sizeof (context->ngc_sessionInfo));

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetMethodTransferArgumentStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferArgumentStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(&context->ngc_sessionInfo.ngsi_transferArgument,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetMethodTransferArgumentEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferArgumentEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(&context->ngc_sessionInfo.ngsi_transferArgument,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetMethodTransferFileClientToRemoteStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferFileClientToRemoteStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(
	&context->ngc_sessionInfo.ngsi_transferFileClientToRemote, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetMethodTransferFileClientToRemoteEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferFileClientToRemoteEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(
	&context->ngc_sessionInfo.ngsi_transferFileClientToRemote, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetMethodStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(&context->ngc_sessionInfo.ngsi_calculation,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetMethodEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(&context->ngc_sessionInfo.ngsi_calculation,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetMethodTransferResultStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferResultStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(&context->ngc_sessionInfo.ngsi_transferResult,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetMethodTransferResultEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferResultEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(&context->ngc_sessionInfo.ngsi_transferResult,
	log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetMethodTransferFileRemoteToClientStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferFileRemoteToClientStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(
	&context->ngc_sessionInfo.ngsi_transferFileRemoteToClient, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetMethodTransferFileRemoteToClientEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetMethodTransferFileRemoteToClientEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(
	&context->ngc_sessionInfo.ngsi_transferFileRemoteToClient, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetCallbackTransferArgumentStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackTransferArgumentStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(
	&context->ngc_sessionInfo.ngsi_callbackTransferArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetCallbackTransferArgumentEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackTransferArgumentEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(
	&context->ngc_sessionInfo.ngsi_callbackTransferArgument, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }
    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentReal =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentReal,
	    context->ngc_sessionInfo.ngsi_callbackTransferArgument.nget_real.
		nget_execution);
    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentCPU =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackTransferArgumentCPU,
	    context->ngc_sessionInfo.ngsi_callbackTransferArgument.nget_cpu.
		nget_execution);

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetCallbackStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(
	&context->ngc_sessionInfo.ngsi_callbackCalculation, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetCallbackEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(
	&context->ngc_sessionInfo.ngsi_callbackCalculation, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }
    context->ngc_sessionInfo.ngsi_sumCallbackCalculationReal =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackCalculationReal,
	    context->ngc_sessionInfo.ngsi_callbackCalculation.nget_real.
		nget_execution);
    context->ngc_sessionInfo.ngsi_sumCallbackCalculationCPU =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackCalculationCPU,
	    context->ngc_sessionInfo.ngsi_callbackCalculation.nget_cpu.
		nget_execution);

    /* Success */
    return 1;
}

/**
 * Set the start time.
 */
int
ngexiContextSetCallbackTransferResultStartTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackTransferResultStartTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the start time */
    result = ngiSetStartTime(
	&context->ngc_sessionInfo.ngsi_callbackTransferResult, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set the end time.
 */
int
ngexiContextSetCallbackTransferResultEndTime(
    ngexiContext_t *context,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngexiContextSetCallbackTransferResultEndTime";

    /* Check the arguments */
    assert(context != NULL);

    /* Get the end time */
    result = ngiSetEndTime(
	&context->ngc_sessionInfo.ngsi_callbackTransferResult, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }
    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultReal =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultReal,
	    context->ngc_sessionInfo.ngsi_callbackTransferResult.nget_real.
		nget_execution);
    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultCPU =
	ngiTimevalAdd(
	    context->ngc_sessionInfo.ngsi_sumCallbackTransferResultCPU,
	    context->ngc_sessionInfo.ngsi_callbackTransferResult.nget_cpu.
		nget_execution);

    /* Success */
    return 1;
}

