#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexContext.c,v $ $Revision: 1.83 $ $Date: 2008/07/02 09:29:38 $";
#endif /* NG_OS_IRIX */
/* 
 * $AIST_Release: 4.2.4 $
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

#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#ifndef NG_PTHREAD
# ifdef linux
#  include <sys/poll.h>
# else /* linux */
#  include <poll.h>
#endif /* linux */
#endif /* NG_PTHREAD */
#include "ngEx.h"

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
static int ngexlAnalyzeArgument(ngexiContext_t *, int, char *[], int, int *);
static int ngexlAnalyzeArgumentSub(ngexiContext_t *, char *, int, int *);
static int ngexlAnalyzeArgumentClient(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentGassServer(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentCrypt(ngexiContext_t *, char *, int *);
static int ngexlAnalyzeArgumentProtocol(ngexiContext_t *, char *, int *);
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
static int ngexlAnalyzeArgumentCheckValue(ngexiContext_t *, int *);
static char *ngexlMakeSystemConfigFileName(int *);
static char *ngexlMakeUserConfigFileName(int *);
static int ngexlReleaseConfigFileName(char *fileName, ngLog_t *, int *);
static int ngexlContextLogOutput(ngexiContext_t *, int, char *[], int *);
static int ngexlContextSignalRegister(ngexiContext_t *, int *);
static int ngexlContextExecutableStatusMatch(
    ngexiExecutableStatus_t, ngexiExecutableStatus_t *, int);
static int ngexlContextExitOnError(ngexiContext_t *, int *);
static void ngexlContextProcessImmediateExit(ngexiContext_t *, int *);

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
    int result;
    char *sysConfig = NULL;
    char *userConfig = NULL;
    ngiProtocolAttribute_t protoAttr;
    ngLogInformation_t logInfoTemp;
    static const char fName[] = "ngexiContextInitialize";

    /* Check the atguments */
    assert(context != NULL);

    /* Initialize the members */
    ngexlContextInitializeMember(context);

    /* Set initial status for context */
    context->ngc_executableStatus = NGEXI_EXECUTABLE_STATUS_INITIALIZING;

    /* Save the Remote Class Information */
    context->ngc_rcInfo = rcInfo;

    /* Initialize Temporary Log Information */
    result = ngiLogInformationSetDefault(
        &logInfoTemp, NG_LOG_TYPE_GENERIC, error);
    if (result == 0) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't initialize Log Information.\n", fName);
        return 0;
    }

    /* Construct the temporary Log */
    context->ngc_log = ngiLogConstruct(
        NG_LOG_TYPE_GENERIC, "Executable", &logInfoTemp,
        NGI_LOG_EXECUTABLE_ID_NOT_APPEND, error);
    if (context->ngc_log == NULL) {
        ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't construct the Log Manager.\n", fName);
        return 0;
    }

    /* Finalize Temporary Log Information */
    result = ngLogInformationFinalize(&logInfoTemp);
    if (result == 0) {
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't finalize Log Information.\n", fName);
        return 0;
    }

    context->ngc_rank = rank;
    if (rank == 0) {

        /* Initialize the Communication Information */
        result = ngexlCommunicationInformationInitialize(
            &context->ngc_commInfo, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't initialize the Communication Information.\n", fName);
            return 0;
        }

        /* Initialize the Random Number */
        result = ngiRandomNumberInitialize(
            &context->ngc_random, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't initialize the Random Number Seed.\n",
                fName);
            return 0;
        }

        /* Set Default Value */
        context->ngc_retryInfo.ngcri_count = 0;
        context->ngc_heartBeatSend.nghbs_interval = 0;
    }

    /* Analyze the arguments */
    result = ngexlAnalyzeArgument(context, argc, argv, rank, error);
    if (result == 0) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't analyze the arguments.\n", fName);
        return 0;
    }

    if (rank == 0) {
        /* Initialize the Globus Toolkit */
        result = ngexiGlobusInitialize(context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't initialize the Globus Toolkit.\n", fName);
            return 0;
        }

        /* Initialize the Mutex and Condition Variable for status */
        result = ngexlContextInitializeMutexAndCond(context, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't initialize the Mutex and Condition Variable.\n", fName);
            return 0;
        }

        /* Re set the status. for to ensure */
        result = ngexiContextExecutableStatusSet(
            context, NGEXI_EXECUTABLE_STATUS_INITIALIZING, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't set the status.\n", fName);
            return 0;
        }

        /* Make the file name of configuration of system */
        sysConfig = ngexlMakeSystemConfigFileName(error);
        if (sysConfig == NULL) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't make the file name of configuration file of system.\n",
                fName);
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            /* Continue to below */
        }

        /* Make the file name of configuration of user */
        userConfig = ngexlMakeUserConfigFileName(error);
        if (userConfig == NULL) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: Can't make the file name of configuration file of user.\n",
                fName);
            NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);
            /* Continue to below */
        }

        /* Read configuration file */
        result = ngexiConfigFileRead(context, sysConfig, userConfig, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read the Configuration file.\n", fName);
            goto error;
        }

        /* Destruct Temporary Log Manager */
        result = ngiLogDestruct(context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Log Manager.\n", fName);
            goto error;
        }

        /* Construct the Log Manager */
        context->ngc_log = ngiLogConstruct(
            NG_LOG_TYPE_GENERIC, "Executable",
            &context->ngc_lmInfo.nglmi_log,
            NGI_LOG_EXECUTABLE_ID_UNDEFINED, error);
        if (context->ngc_log == NULL) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't construct the Log Manager.\n", fName);
            goto error;
        }
     
        /* Release the file name of configuration of system */
        if (sysConfig != NULL) {
            result = ngexlReleaseConfigFileName(sysConfig, context->ngc_log, error);
            sysConfig = NULL;
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't release the file name of configuration file of system.\n",
                    fName);
                goto error;
            }
        }

        /* Release the file name of configuration of user */
        if (userConfig != NULL) {
            result = ngexlReleaseConfigFileName(
                userConfig, context->ngc_log, error);
            userConfig = NULL;
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't release the file name of configuration file of user.\n",
                    fName);
                goto error;
            }
        }

        /* log */
        result = ngexlContextLogOutput(context, argc, argv, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't output the log.\n", fName);
            goto error;
        }

        /* Register the signal */
        result = ngexlContextSignalRegister(context, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the signal.\n", fName);
            goto error;
        }

        /* Construct the GASS Copy Manager */
        context->ngc_gassCopy = ngexiGASScopyConstruct(context->ngc_log, error);
        if (context->ngc_gassCopy == NULL) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the GASS Copy Manager.\n", fName);
            goto error;
        }

        /* Construct the Communication Manager */
        context->ngc_communication = ngiCommunicationConstructClient(
            context->ngc_commInfo.ngci_crypt,
            context->ngc_commInfo.ngci_tcpNodelay,
            context->ngc_commInfo.ngci_hostName,
            context->ngc_commInfo.ngci_portNo,
            context->ngc_retryInfo,
            &context->ngc_random,
            context->ngc_log, error);
        if (context->ngc_communication == NULL) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't construct the Communication Manager.\n", fName);
            goto error;
        }

        /* Initialize the attribute of Protocol Manager */
        protoAttr.ngpa_protocolType = context->ngc_commInfo.ngci_protoType;
        protoAttr.ngpa_architecture = NGI_ARCHITECTURE_ID;
        protoAttr.ngpa_xdr = NG_XDR_USE;
        protoAttr.ngpa_protocolVersion= NGI_PROTOCOL_VERSION;
        protoAttr.ngpa_sequenceNo = NGI_PROTOCOL_SEQUENCE_NO_DEFAULT;
        protoAttr.ngpa_contextID = context->ngc_commInfo.ngci_contextID;
        protoAttr.ngpa_jobID = context->ngc_commInfo.ngci_jobID;
        protoAttr.ngpa_executableID = NGI_EXECUTABLE_ID_UNDEFINED;
        protoAttr.ngpa_tmpDir = context->ngc_lmInfo.nglmi_tmpDir;

        /* Construct the Protocol Manager */
        context->ngc_protocol = ngiProtocolConstruct(
            &protoAttr, context->ngc_communication,
            ngexiProtocolGetRemoteMethodInformation, context->ngc_log, error);
        if (context->ngc_protocol == NULL) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                NG_LOG_LEVEL_ERROR, NULL,
                "Can't construct the Protocol Manager.\n", fName);
            goto error;
        }

        /* Initialize Heart Beat */
        result = ngexiHeartBeatInitialize(context, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize heartbeat.\n", fName);
            goto error;
        }

        /* Set the status */
        result = ngexiContextExecutableStatusSet(
            context, NGEXI_EXECUTABLE_STATUS_IDLE, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't set the status.\n", fName);
            goto error;
        }
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
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register the Communication Log.\n", fName);
        }

        /* Destruct the Communication Log */
        result = ngiLogDestruct(context->ngc_commLog, error);
        context->ngc_commLog = NULL;
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't destruct the Communication Log.\n", fName);
        }
    }

    /* Destruct the GASS Copy Manager */
    if (context->ngc_gassCopy != NULL) {
	result = ngexiGASScopyDestruct(
	    context->ngc_gassCopy, context->ngc_log, error);
	context->ngc_gassCopy = NULL;
	if (result == 0) {
	    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't destruct the GASS Copy Manager.\n", fName);
	}
    }

    /* Release the file name of configuration of system */
    if (sysConfig != NULL) {
	result = ngexlReleaseConfigFileName(sysConfig, context->ngc_log, NULL);
	sysConfig = NULL;
	if (result == 0) {
	    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the file name of configuration file of system.\n",
		fName);
	}
    }

    /* Release the file name of configuration of user */
    if (userConfig != NULL) {
	result = ngexlReleaseConfigFileName(userConfig, context->ngc_log, NULL);
	userConfig = NULL;
	if (result == 0) {
	    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't release the file name of configuration file of user.\n",
		fName);
	}
    }

    /* Finalize the Context */
    result = ngexiContextFinalize(context, NULL);
    if (result == 0) {
    	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the Context.\n", fName);
	return 0;
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

    if (context->ngc_rank == 0) {
        /* Finalize Heart Beat */
        result = ngexiHeartBeatFinalize(context, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize heartbeat.\n",
                fName);
            return 0;
        }

        if (context->ngc_commLog != NULL) {
            /* Unregister the Communication Log */
            result = ngiCommunicationLogUnregister(
                context->ngc_communication, context->ngc_log, error);
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't unregister the Communication Log.\n", fName);
            }

            /* Destruct the Communication Log */
            result = ngiLogDestruct(context->ngc_commLog, error);
            context->ngc_commLog = NULL;
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destruct the Communication Log.\n", fName);
            }
        }

        /* Destruct the Protocol Manager */
        if (context->ngc_protocol != NULL) {
            result = ngiProtocolDestruct(
                context->ngc_protocol, context->ngc_log, error);
            context->ngc_protocol = NULL;
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't destruct the Protocol Manager.\n", fName);
                return 0;
            }
        }

        /* Destruct the GASS Copy Manager */
        if (context->ngc_gassCopy != NULL) {
            result = ngexiGASScopyDestruct(
                context->ngc_gassCopy, context->ngc_log, error);
            context->ngc_gassCopy = NULL;
            if (result == 0) {
                ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't construct the GASS Copy Manager.\n", fName);
                return 0;
            }
        }

        /* Finalize the Local Machine Information */
        result = ngexiLocalMachineInformationFinalize(
            &context->ngc_lmInfo, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Local Machine Information.\n", fName);
            return 0;
        }

        /* Finalize the Debugger Information */
        result = ngiDebuggerInformationFinalize(
            &context->ngc_dbgInfo, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Debugger Information.\n", fName);
            return 0;
        }

        /* Finalize the Communication Manager */
        result = ngexlCommunicationInformationFinalize(
            &context->ngc_commInfo, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Communication Manager.\n", fName);
            return 0;
        }

        /* Finalize the Random Number */
        result = ngiRandomNumberFinalize(
            &context->ngc_random, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Random Number Seed.\n",
                fName);
            return 0;
        }

        /* Unset the log from Signal Manager */
        result = ngexiSignalManagerLogSet(
            NULL, context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't unregister the log for Signal Manager.\n",
                fName);
            return 0;
        }
    }

    /* Destruct the Log */
    if (context->ngc_log != NULL) {
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Executable log destruct.\n", fName);

        result = ngiLogDestruct(context->ngc_log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't destruct the Log.\n", fName);
            return 0;
        }
    }
    context->ngc_log = NULL;

    if (context->ngc_rank == 0) {
        /* Finalize the Mutex and Condition Variable for status */
        result = ngexlContextFinalizeMutexAndCond(context, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
                NULL, "%s: Can't initialize the Mutex and Condition Variable.\n",
                fName);
            return 0;
        }

        /* Finalize the Globus Toolkit */
        result = ngexiGlobusFinalize(NULL, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_ERROR,
                NULL, "%s: Can't finalize the Globus Toolkit.\n", fName);
            return 0;
        }
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
#if 0 /* Is this necessary? */
    ngexlCommunicationInformationInitializeMember(&context->ngc_commInfo);
#endif

    context->ngc_retryInfo.ngcri_count = 0;
    context->ngc_retryInfo.ngcri_interval = 0;
    context->ngc_retryInfo.ngcri_increase = 0.0;
    context->ngc_retryInfo.ngcri_useRandom = 0;

    context->ngc_mutexInitialized = 0;
    context->ngc_condInitialized = 0;

    context->ngc_heartBeatSend.nghbs_interval = 0;
    context->ngc_heartBeatSend.nghbs_continue = 0;
    context->ngc_heartBeatSend.nghbs_stopped = 0;
    context->ngc_heartBeatSend.nghbs_mutexInitialized = 0;
    context->ngc_heartBeatSend.nghbs_condInitialized = 0;

    context->ngc_random = 0;

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
    context->ngc_protocol = NULL;
    context->ngc_log = NULL;
    context->ngc_commLog = NULL;
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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't initialize the mutex.\n", fName);
        goto error;
    }
    context->ngc_mutexInitialized = 1;

    /* Initialize the cond */
    result = ngiCondInitialize(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't initialize the cond.\n", fName);
        goto error;
    }
    context->ngc_condInitialized = 1;

    /* Success */
    return 1;

    /* Error occurred */
error:
    result = ngexlContextFinalizeMutexAndCond(context, NULL);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Can't finalize the Mutex and Cond.\n", fName);
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
    if (context->ngc_mutexInitialized != 0) {
        result = ngiMutexDestroy(&context->ngc_mutex, log, error);
        context->ngc_mutexInitialized = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't initialize the mutex.\n", fName);
            return 0;
        }
    }

    /* Finalize the cond */
    if (context->ngc_condInitialized != 0) {
        result = ngiCondDestroy(&context->ngc_cond, log, error);
        context->ngc_condInitialized = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Can't initialize the cond.\n", fName);
            return 0;
        }
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
    /* Check the arguments */
    assert(lmInfo != NULL);

    ngexlLocalMachineInformationInitializeMember(lmInfo);

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
    static const char fName[] = "ngexiLocalMachineInformationFinalize";

    /* Check the arguments */
    assert(lmInfo != NULL);

    /* Finalize the Log Information */
    result = ngLogInformationFinalize(&lmInfo->nglmi_log);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Log Information.\n", fName);
	return 0;
    }

    /* Finalize the Communication Log Information */
    result = ngLogInformationFinalize(&lmInfo->nglmi_commLog);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Communication Log Information.\n", fName);
	return 0;
    }

    /* Deallocate the members */
    globus_libc_free(lmInfo->nglmi_tmpDir);
    lmInfo->nglmi_tmpDir = NULL;

    globus_libc_free(lmInfo->nglmi_path);
    lmInfo->nglmi_path = NULL;

    if (lmInfo->nglmi_saveStdout != NULL) {
        globus_libc_free(lmInfo->nglmi_saveStdout);
        lmInfo->nglmi_saveStdout = NULL;
    }

    if (lmInfo->nglmi_saveStderr != NULL) {
        globus_libc_free(lmInfo->nglmi_saveStderr);
        lmInfo->nglmi_saveStderr = NULL;
    }

    if (lmInfo->nglmi_signals != NULL) {
        globus_libc_free(lmInfo->nglmi_signals);
        lmInfo->nglmi_signals = NULL;
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
    ngLogInformationInitialize(&lmInfo->nglmi_log);
    ngLogInformationInitialize(&lmInfo->nglmi_commLog);

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

    /* Default Variable */
    commInfo->ngci_crypt = NG_PROTOCOL_CRYPT_NONE;
    commInfo->ngci_protoType = NG_PROTOCOL_TYPE_BINARY; /* temporary setting */
    commInfo->ngci_tcpNodelay = 0; /* false */

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
    globus_libc_free(commInfo->ngci_hostName);
    globus_libc_free(commInfo->ngci_gassServer);

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
    commInfo->ngci_portNo = NGI_PORT_ANY;
    commInfo->ngci_crypt = NG_PROTOCOL_CRYPT_NONE;
    commInfo->ngci_protoType = NG_PROTOCOL_TYPE_XML;
    commInfo->ngci_tcpNodelay = 0; /*false*/
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
    commInfo->ngci_hostName = NULL;
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
        ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for path.\n", fName);
        return 0;
    }

    /* Analyze each option arguments */
    for (i = 1; i < argc; i++) {
	/* Is argument valid? */
	if ((argv[i][0] != '-') || (argv[i][1] != '-')) {
	    ngLogPrintf(context->ngc_log,
		NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
		"%s: Argument No.%d \"%s\" is not valid. ignored.\n",
		fName, i, argv[i]);
	    continue;
	}

	/* Analyze the argument */
	result = ngexlAnalyzeArgumentSub(context, argv[i], rank, error);
	if (result == 0) {
	    ngLogPrintf(context->ngc_log,
		NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
		"%s: Argument No.%d \"%s\" is not valid. ignored.\n",
		fName, i, argv[i]);
	    continue;
	}
    }

    if (rank == 0) {
        result = ngexlAnalyzeArgumentCheckValue(context, error);
        if (result == 0) {
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Check the argument information failed.\n",
                fName);
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
    	{"client",		ngexlAnalyzeArgumentClient,        0},
	{"gassServer",		ngexlAnalyzeArgumentGassServer,    0},
	{"crypt",		ngexlAnalyzeArgumentCrypt,         0},
	{"protocol",		ngexlAnalyzeArgumentProtocol,      0},
	{"contextID",		ngexlAnalyzeArgumentContextID,     0},
	{"jobID",		ngexlAnalyzeArgumentJobID,         0},
	{"heartbeat",		ngexlAnalyzeArgumentHeartBeat,     0},
	{"connectRetry",	ngexlAnalyzeArgumentConnectRetry,  0},
	{"workDirectory",	ngexlAnalyzeArgumentWorkDirectory, 1},
	{"coreDumpSize",	ngexlAnalyzeArgumentCoreDumpSize,  1},
	{"debugTerminal",	ngexlAnalyzeArgumentDebugTerminal, 0},
	{"debugDisplay",	ngexlAnalyzeArgumentDebugDisplay,  0},
	{"debugger",		ngexlAnalyzeArgumentDebugger,      0},
	{"debugEnable",		ngexlAnalyzeArgumentDebugEnable,   0},
	{"debugBusyLoop",	ngexlAnalyzeArgumentDebugBusyLoop, 0},
	{"tcpNodelay",          ngexlAnalyzeArgumentTcpNodelay,    0},
	{NULL, 			NULL},
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
    ngLogPrintf(context->ngc_log,
	NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	"%s: %s: Argument is not defined.\n", fName, argv);
    return 0;

    /* Argument is found */
found:
    if ((rank > 0) && (aa[i].handlingSubRank == 0)) {
        /* Just ignore the argument */
        return 1;
    }

    if ((argv[param - 1] != '=') || (argv[param] == '\0')) {
	/* Argument is not valid */
	ngLogPrintf(context->ngc_log,
	    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Syntax error.\n", fName, argv);
	return 0;
    }

    result = aa[i].func(context, &argv[param], error);
    if (result == 0) {
	/* Argument is not valid */
	ngLogPrintf(context->ngc_log,
	    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument is not valid.\n", fName, argv);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --client
 */
static int
ngexlAnalyzeArgumentClient(ngexiContext_t *context, char *arg, int *error)
{
    char *hostName, *pNo;
    int portNo;
    char *end, *endptr;
    static const char fName[] = "ngexlAnalyzeArgumentClient";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Duplicate the host name */
    hostName = strdup(arg);
    if (hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Client host name.\n", fName);
	return 0;
    }

    /* Get the host name */
    end = strchr(hostName, ':');
    if (end == NULL) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Invalid host name: ':' is not found.\n", fName, arg);
	return 0;
    }

    /* Delete the ':' */
    *end = '\0';

    /**
     * Get the port number.
     */
    /* Get the start position of port number */
    pNo = end + 1;
    if (pNo == '\0') {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Port number is not defined.\n", fName, arg);
	return 0;
    }

    /* Get the port number */
    end = &pNo[strlen(pNo)];
    portNo = strtol(pNo, &endptr, 0);
    if (endptr != end) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument contain the invalid character.\n", fName, arg);
	return 0;
    }

    /* Is Port number smaller than minimum? */
    if (portNo < NGI_PORT_MIN) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Port number %d is smaller than minimum %d.\n",
	    fName, arg, portNo, NGI_PORT_MIN);
	return 0;
    }

    /* Is Context ID greater than maximum? */
    if (portNo > NGI_PORT_MAX) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Port number %d is greater than maximum %d.\n",
	    fName, arg, portNo, NGI_PORT_MAX);
	return 0;
    }

    /* Copy the host name and port number */
    context->ngc_commInfo.ngci_hostName = hostName;
    context->ngc_commInfo.ngci_portNo = portNo;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --gassServer
 */
static int
ngexlAnalyzeArgumentGassServer(ngexiContext_t *context, char *arg, int *error)
{
    char *gassServer;
    static const char fName[] = "ngexlAnalyzeArgumentGassServer";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Duplicate the GASS Server */
    gassServer = strdup(arg);
    if (gassServer == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for URL of GASS server.\n", fName);
	return 0;
    }

    /* Copy the GASS server */
    context->ngc_commInfo.ngci_gassServer = gassServer;

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --crypt
 */
static int
ngexlAnalyzeArgumentCrypt(ngexiContext_t *context, char *arg, int *error)
{
    static const char fName[] = "ngexlAnalyzeArgumentCrypt";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Analyze the arguments */
    if (strcmp(arg, "none") == 0) {
    	context->ngc_commInfo.ngci_crypt = NG_PROTOCOL_CRYPT_NONE;
    } else if (strcmp(arg, "authonly") == 0) {
    	context->ngc_commInfo.ngci_crypt = NG_PROTOCOL_CRYPT_AUTHONLY;
    } else if (strcmp(arg, "GSI") == 0) {
    	context->ngc_commInfo.ngci_crypt = NG_PROTOCOL_CRYPT_GSI;
    } else if (strcmp(arg, "SSL") == 0) {
    	context->ngc_commInfo.ngci_crypt = NG_PROTOCOL_CRYPT_SSL;
    } else {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Invalid crypt type.\n", fName, arg);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Analyze the argument: --protocol
 */
static int
ngexlAnalyzeArgumentProtocol(ngexiContext_t *context, char *arg, int *error)
{
    static const char fName[] = "ngexlAnalyzeArgumentProtocol";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    /* Analyze the argument */
    if (strcmp(arg, "XML") == 0) {
    	context->ngc_commInfo.ngci_protoType = NG_PROTOCOL_TYPE_XML;
    } else if (strcmp(arg, "binary") == 0) {
    	context->ngc_commInfo.ngci_protoType = NG_PROTOCOL_TYPE_BINARY;
    } else {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Invalid protocol type.\n", fName, arg);
	return 0;
    }

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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument contain the invalid character.\n", fName, arg);
	return 0;
    }

    /* Is Context ID smaller than minimum? */
    if (contextID < NGI_CONTEXT_ID_MIN) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Context ID %d is smaller than minimum %d.\n",
	    fName, arg, contextID, NGI_CONTEXT_ID_MIN);
	return 0;
    }

    /* Is Context ID greater than maximum? */
    if (contextID > NGI_CONTEXT_ID_MAX) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Context ID %d is greater than maximum %d.\n",
	    fName, arg, contextID, NGI_CONTEXT_ID_MAX);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument contain the invalid character.\n", fName, arg);
	return 0;
    }

    /* Is Job ID smaller than minimum? */
    if (jobID < NGI_JOB_ID_MIN) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Job ID %d is smaller than minimum %d.\n",
	    fName, arg, jobID, NGI_JOB_ID_MIN);
	return 0;
    }

    /* Is Job ID greater than maximum? */
    if (jobID > NGI_JOB_ID_MAX) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Context ID %d is greater than maximum %d.\n",
	    fName, arg, jobID, NGI_JOB_ID_MAX);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument contain the invalid character.\n", fName, arg);
	return 0;
    }

    if (heartBeatInterval < 0) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: heartbeat %d is smaller than 0.\n",
	    fName, arg, heartBeatInterval);
	return 0;
    }

    context->ngc_heartBeatSend.nghbs_interval = heartBeatInterval;

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
    ngLogPrintf(context->ngc_log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: %s: Argument contain the invalid character.\n", fName, arg);

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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Change directory to \"%s\" failed.\n", fName, workDirectory);
	return 0;
    }

    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_DEBUG, NULL,
	"%s: Change working directory to \"%s\".\n", fName, workDirectory);

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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Argument contain the invalid character.\n", fName, arg);
	return 0;
    }

    /* Is coreDumpSize valid? */
    if (coreSize < -1) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_WARNING, NULL,
	    "%s: %s: Invalid core size %d.\n",
	    fName, arg, coreSize);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: setrlimit system call failed.\n", fName);
	return 0;
    }

    if (coreSize == -1) {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_DEBUG, NULL,
	    "%s: Set limit for core file size to infinite.\n", fName);
    } else {
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_DEBUG, NULL,
	    "%s: Set limit for core file size to %dkB.\n", fName, coreSize);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Debug Terminal.\n", fName);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Debug Display.\n", fName);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Debugger.\n", fName);
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
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Debugger command too long (wrote %d).\n", fName, wroteLength);
	return 0;
    }

    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Debug enabled. executing command \"%s\"\n", fName, command);

    /* Execute the debugger command */
    result = system(command);
    if (result != 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Debugger command failed.\n", fName, result);
	return 0;
    }
    /**
     * Note: system("command &") always returns 0,
     *     regardless of existence of command.
     */

    /* Wait for debugger to attach this program */
    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: Waiting %d seconds for debugger to attach.\n",
        fName, NGEXI_DEBUG_WAIT_FOR_ATTACH);
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

    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Entering busy-loop to wait attach for debugger.\n", fName);

    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
        NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Please attach process %ld by debugger.\n",
        fName, (long)getpid());

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
    static const char fName[] = "ngexlAnalyzeArgumentTcpNodelay";

    /* Check the arguments */
    assert(context != NULL);
    assert(arg != NULL);
    assert(arg[0] != '\0');

    ngLogPrintf(context->ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
	NG_LOG_LEVEL_INFORMATION, NULL,
	"%s: set TCP_NODELAY to true.\n", fName);

    context->ngc_commInfo.ngci_tcpNodelay = 1;/* true */

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

    if (commInfo->ngci_hostName == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid hostname.\n",
	    fName);
        return 0;
    }

    if (commInfo->ngci_portNo == 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid port number.\n",
	    fName);
        return 0;
    }

    if ((commInfo->ngci_contextID < NGI_CONTEXT_ID_MIN) ||
        (commInfo->ngci_contextID > NGI_CONTEXT_ID_MAX)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid Context ID %d.\n",
	    fName, commInfo->ngci_contextID);
        return 0;
    }

    if ((commInfo->ngci_jobID < NGI_JOB_ID_MIN) ||
        (commInfo->ngci_jobID > NGI_JOB_ID_MAX)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Invalid Job ID %d.\n",
	    fName, commInfo->ngci_jobID);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Make the file name of Configuration of system.
 */
static char *
ngexlMakeSystemConfigFileName(int *error)
{
    char *gtLocation;
    char *fileName;
    static const char fName[] = "ngexlMakeSystemConfigFileName";

    /* Get the globus location */
    gtLocation = getenv("GLOBUS_LOCATION");
    if (gtLocation == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
	ngLogPrintf(NULL,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	    "%s: Environment variable of GLOBUS_LOCATION is not defined.\n",
	    fName);
	return NULL;
    }

    /* Allocate the storage for file name */
    fileName = globus_libc_calloc(
	1, strlen(gtLocation) + strlen(NGEXI_CONFIG_FILENAME_SYSTEM) + 1);
    if (fileName == 0) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for System Configuration File.\n",
	    fName);
	return NULL;
    }

    /* Make the file name */
    sprintf(fileName, "%s%s", gtLocation, NGEXI_CONFIG_FILENAME_SYSTEM);

    /* Success */
    return fileName;
}

/**
 * Make the file name of Configuration of user.
 */
static char *
ngexlMakeUserConfigFileName(int *error)
{
    struct passwd *passwd;
    char *fileName, *tmp;
    static const char fName[] = "ngexlMakeUserConfigFileName";

    /* Functionality for developers */
    tmp = getenv("NG_STUBRC");
    if (tmp != NULL) {
        if (strcmp(tmp, "") != 0) {
            fileName = strdup(tmp);
            if (fileName == NULL) {
         	       ngLogPrintf(NULL,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
         	           "%s: strdup failed.\n", fName);
                return NULL;
            }

            return fileName;
        } else {
	    ngLogPrintf(NULL,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
	        "%s: empty NG_STUBRC environment variable was set.\n", fName);
        }
    }

    /* Get the passwd database */
    passwd = getpwuid(geteuid());
    if (passwd == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: getpwuid() failed.\n", fName);
	return NULL;
    }

    /* Is home directory valid? */
    if (passwd->pw_dir == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Home directory is NULL.\n", fName);
	return NULL;
    }

    /* Allocate the storage for file name */
    fileName = globus_libc_calloc(
	1,
	strlen(passwd->pw_dir) +
	strlen(NGEXI_CONFIG_FILENAME_USER) + 1);
    if (fileName == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for User Configuration File.\n",
	    fName);
	return NULL;
    }

    /* Make the file name */
    sprintf(fileName, "%s%s",passwd->pw_dir, NGEXI_CONFIG_FILENAME_USER);

    /* Success */
    return fileName;
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
    globus_libc_free(fileName);

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
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Executable log was created.\n", fName);

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Ninf-G Executable was invoked.\n", fName);

    /* hostname */
    result = globus_libc_gethostname(hostName, NGI_HOST_NAME_MAX);
    if (result != 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get hostname.\n", fName);
        /* not return */
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: host name is \"%s\".\n", fName, hostName);
    }

    /* pid */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: process id = %ld.\n", fName, (long)getpid());

    /* argv */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: The number of invocation arguments = %d.\n", fName, argc);

    for (i = 0; i < argc; i++) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Argument No.%d : \"%s\".\n", fName, i, argv[i]);
    }

    /* current working directory */
    resultPtr = getcwd(workingDirectory, NGI_DIR_NAME_MAX);
    if (resultPtr == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Can't get current working directory.\n", fName);
        /* not return */
    } else {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: cwd : \"%s\".\n", fName, workingDirectory);
    }

    /* pthread */
    str = "NonThread";
#ifdef NG_PTHREAD
    str = "Pthread";
#endif /* NG_PTHREAD */

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: This Executable binary is %s version.\n", fName, str);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't register the Signal.\n", fName);
        return 0;
    }

    result = ngexiSignalManagerStart(log, error);
    if (result == 0) { 
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't start the Signal Manager.\n", fName);
        return 0;
    }

    result = ngexiSignalManagerLogSet(log, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the log for Signal Manager.\n", fName);
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
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to generate %s filename.\n",
                fName, targetName);
            return 0;
        }

        targetFile = buf;

        /* log */
        ngLogPrintf(context->ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: Switching %s output to file \"%s\".\n",
            fName, targetName, targetFile);

        /* Open file */
        fp = fopen(targetFile, "a");
        if (fp == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to open %s file \"%s\" (%s).\n",
                fName, targetName, targetFile, strerror(errno));
            return 0;
        }

        fd = fileno(fp);

        /* Connect to stdout/stderr */
        result = dup2(fd, targetFd);
        if (result < 0) {
            NGI_SET_ERROR(error, NG_ERROR_FILE);
            ngLogPrintf(context->ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to switch %s to file \"%s\" (%s).\n",
                fName, targetName, targetFile, strerror(errno));
            return 0;
        }

        if (sameFile != 0) {
            result = dup2(fd, targetFd2);
            if (result < 0) {
                NGI_SET_ERROR(error, NG_ERROR_FILE);
                ngLogPrintf(context->ngc_log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to switch %s to same file \"%s\" (%s).\n",
                    fName, targetName, targetFile, strerror(errno));
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Method ID %s is smaller than zero.\n", fName, methodID);
	return NULL;
    }
    if (methodID >= context->ngc_rcInfo->ngrci_nMethods) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Method ID %s is greater than maximum.\n", fName, methodID);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't get the Session Information.\n", fName);
	error = NULL;
	retResult = 0;
	goto error;
    }
    getInfo = 1;

    /* Allocate the storage for Compression Information */
    if (argument->nga_nArguments > 0) {
        context->ngc_sessionInfo.ngsi_compressionInformation =
            globus_libc_calloc(
            argument->nga_nArguments,
            sizeof (*context->ngc_sessionInfo.ngsi_compressionInformation));
        if (context->ngc_sessionInfo.ngsi_compressionInformation
            == NULL) {
            NGI_SET_ERROR(error, NG_ERROR_MEMORY);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Compression Information.\n",
                fName);
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
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
		NULL, "%s: Can't get the Session Information.\n", fName);
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
	globus_libc_free(context->ngc_sessionInfo.ngsi_compressionInformation);
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
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Invalid state %d.\n", fName,
            context->ngc_executableStatus);
        return 0;
    }

    /* Set the error code */
    NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Invalid state: Now %s.\n", fName, string);

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
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for status.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    /* Print the debug message */
    oldStatusString = ngexiContextExecutableStatusStringGet(
        context->ngc_executableStatus, log, error);
    if (oldStatusString == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get old status string.\n", fName);
        goto error;
    }

    newStatusString = ngexiContextExecutableStatusStringGet(
        status, log, error);
    if (newStatusString == NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get new status string.\n", fName);
        goto error;
    }

    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Executable status \"%s\" -> \"%s\".\n",
            fName, oldStatusString, newStatusString);

    /* Set the status */
    context->ngc_executableStatus = status;

    /* Notify signal */
    result = ngiCondBroadcast(&context->ngc_cond, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't signal the Condition Variable for status.\n", fName);
        goto error;
    }
    
    /* Unlock the mutex */
    assert(mutexLocked != 0);
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    mutexLocked = 0;
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for status.\n", fName);
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
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex for status.\n", fName);
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
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Waiting for the Executable status grow.\n",
            fName);

    /* Block the signal for NonThread */
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_BLOCK, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't block signal.\n", fName);
        goto error;
    }
    signalBlocked = 1;

    /* Lock the mutex */
    result = ngiMutexLock(&context->ngc_mutex, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Mutex for status.\n", fName);
        goto error;
    }
    mutexLocked = 1;

    /* Wait the status */
    while ((ngexlContextExecutableStatusMatch(
        context->ngc_executableStatus, waitStatus, nStatus) == 0) &&
        (context->ngc_executableStatus != NGEXI_EXECUTABLE_STATUS_END)) {

#ifdef NG_PTHREAD
        result = ngiCondWait(
            &context->ngc_cond, &context->ngc_mutex, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable for status.\n",
                fName);
            goto error;
        }
#else /* NG_PTHREAD */
        /* Wait the status and Check the callback event */
        result = ngiCondTimedWait(
            &context->ngc_cond, &context->ngc_mutex,
            0, &timeout, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the Condition Variable for status.\n",
                fName);
            goto error;
        }

        /* Unlock the mutex */
        assert(mutexLocked != 0);
        result = ngiMutexUnlock(&context->ngc_mutex, log, error);
        mutexLocked = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex for status.\n", fName);
            goto error;
        }
     
        /* Unblock the signal */
        assert(signalBlocked != 0);
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK, error);
        signalBlocked = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't block signal.\n", fName);
            goto error;
        }

        /**
         * Unblock the signal to enable periodic heartbeat send.
         * Mutex is released because globus_io_write() on
         * heartbeat send causes an other I/O callback invocation.
         */

        result = poll(NULL, 0, 1); /* sleep 1ms to aware high load */
        if ((result < 0) && (errno != EINTR)) {
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: poll failed: %s.\n", fName, strerror(errno));
            goto error;
        }

        /* Block the signal */
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_BLOCK, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't block signal.\n", fName);
            goto error;
        }
        signalBlocked = 1;
     
        /* Lock the mutex */
        result = ngiMutexLock(&context->ngc_mutex, log, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the Mutex for status.\n", fName);
            goto error;
        }
        mutexLocked = 1;

#endif /* NG_PTHREAD */
    }

    /* Set result */
    *resultStatus = context->ngc_executableStatus;

    /* log */
    string = ngexiContextExecutableStatusStringGet(*resultStatus, log, NULL);
    if (string == NULL) {
        string = "Not valid status";
    }
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: status was grown into \"%s\" (%d). return.\n",
        fName, string, *resultStatus);

    /* Is error occurred? */
    if (context->ngc_error != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, context->ngc_error);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Is error occurred in callback? */
    if (context->ngc_cbError != NG_ERROR_NO_ERROR) {
        NGI_SET_ERROR(error, context->ngc_cbError);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Error was occurred while waiting status.\n", fName);
        goto error;
    }

    /* Unlock the mutex */
    assert(mutexLocked != 0);
    result = ngiMutexUnlock(&context->ngc_mutex, log, error);
    mutexLocked = 0;
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Mutex for status.\n", fName);
        goto error;
    }

    /* Unblock the signal */
    assert(signalBlocked != 0);
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK, error);
    signalBlocked = 0;
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't block signal.\n", fName);
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
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the Mutex for status.\n", fName);
        }
    }

    /* Unblock the signal */
    if (signalBlocked != 0) {
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK, NULL);
        signalBlocked = 0;
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't block signal.\n", fName);
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
    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	"%s: Unknown Executable status %d.\n", fName, status);

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
 * Set the error code occurred by Globus I/O callback.
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
 * Get the error code occurred by Globus I/O callback.
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
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Setting this executable unusable.\n", fName);

    /* Get the Communication */
    comm = NULL;
    if ((context->ngc_protocol != NULL) &&
        (context->ngc_protocol->ngp_communication != NULL)) {
        comm = context->ngc_protocol->ngp_communication;
    }

    if (comm != NULL) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Closing Communication.\n", fName);

        /* Close the connection and finish I/O callback */
        result = ngiCommunicationClose(comm, log, error);
        if (result == 0) {
            retResult = 0;
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't close communication.\n", fName);
        }
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Setting error code %d.\n", fName, errorCause);
    
    /* Check error */
    if (errorCause == NG_ERROR_NO_ERROR) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
            "%s: Setting error to NO_ERROR.\n", fName);
    }
    
    /* Set the error */
    result = ngexiContextSetError(context, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the error.\n", fName);
    }

    /* Set the I/O callback error */
    result = ngexiContextSetCbError(context, errorCause, error);
    if (result == 0) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the I/O callback error.\n", fName);
    }

    /* Set the status */
    result = ngexiContextExecutableStatusSet(
        context, NGEXI_EXECUTABLE_STATUS_END, error);
    if (result == 0) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the status.\n", fName);
    }

    /* Exit */
    result = ngexlContextExitOnError(context, error);
    if (result == 0) {
        retResult = 0;
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't process the exit.\n", fName);
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
	ngLogPrintf(log,
	    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: No exit performed. wait the user defined function finish.\n",
	    fName);

	return 1;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Ninf-G Executable is exiting.\n", fName);

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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Argument Data.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't release the Argument Data.\n", fName);
	return 0;
    }

    /* Success */
    context->ngc_callbackArgument = NULL;
    return  1;
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
	return 0;
    }

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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
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
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
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
