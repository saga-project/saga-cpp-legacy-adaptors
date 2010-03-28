#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngExecutable.c,v $ $Revision: 1.76 $ $Date: 2008/03/28 06:23:29 $";
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
#include <stdio.h>
#include <string.h>
#include "ngEx.h"
#include "grpc_executable.h"
#include "grpc_error.h"

/**
 * Prototype declaration of internal functions.
 */
static int ngexlStubAnalyzeArgumentWithExit(int, char **,
    ngRemoteClassInformation_t *, int *, int *);
static int ngexlStubInitialize(int, char **,
    ngRemoteClassInformation_t *, int, int *);
static int ngexlStubGetRequest(int *, int *);
static int ngexlStubGetArgument(int, void *, int *);
static int ngexlStubCalculationStart(int *);
static int ngexlStubCalculationEnd(int *);
static int ngexlStubFinalize(int *);
static int ngexlIsCanceled(int *);

/* Context for Executable */
static int ngexiContextInitialized = 0;
static ngexiContext_t ngexiContext;

/**
 * Analyze argument with exit.
 * This function is called before ngexStubInitialize().
 * When the argument which requires immediate exit comes,
 * This function exits in the function. And not return to caller.
 */
int
ngexStubAnalyzeArgumentWithExit(
    int argc,
    char *argv[],
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int local_error, result, requireImmediateExit;

    requireImmediateExit = 0;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubAnalyzeArgumentWithExit(
        argc, argv, rcInfo, &requireImmediateExit, &local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    if (result == 0) {
        exit(1);
    }

    /* Success */
    return requireImmediateExit;
}

static int
ngexlStubAnalyzeArgumentWithExit(
    int argc,
    char *argv[],
    ngRemoteClassInformation_t *rcInfo,
    int *requireImmediateExit,
    int *error)
{
    FILE *fp;
    int result, exitCode, i, requireClose;
    char *switchName, *subSwitchName, hostName[NGI_HOST_NAME_MAX];
    char workingDirectory[NGI_DIR_NAME_MAX], *resultPtr;

    /* Check the arguments */
    assert(requireImmediateExit != NULL);

    *requireImmediateExit = 0;
    requireClose = 0;

    for (i = 1; i < argc; i++) {

        switchName = "-i";
        if (strncmp(argv[i], switchName, strlen(switchName)) == 0) {
            *requireImmediateExit = 1;

            /* Output Remote Class Information */
            if ((i + 1 < argc) && (argv[i + 1] != NULL)) {
                requireClose = 1;
                fp = fopen(argv[i + 1], "w");
                if (fp == NULL) {
                    fprintf(stderr, "couldn't open file \"%s\".\n",
                        argv[i + 1]);
                    exit(1);
                }
            } else {
                fp = stdout;
            }

            result = ngexiPrintRemoteClassInformation(
                rcInfo, fp, NULL, error);
            if (result == 0) {
                exit(1);
            }

            if (requireClose != 0) {
                fclose(fp);
                fp = NULL;
                requireClose = 0;
            }

            /* Success */
            exit(0);
        }

        switchName = "--gassCopy";
        if (strncmp(argv[i], switchName, strlen(switchName)) == 0) {
            /* Allow "--gassCopy", "--gassCopy=1" */

            *requireImmediateExit = 1;

            /* Perform GASS COPY and exit */
            exitCode = ngexiGASScopyProcess(argc, argv, error);

            /* Success */
            exit(exitCode);
        }

        switchName = "--invokeCheck";
        if (strncmp(argv[i], switchName, strlen(switchName)) == 0) {
            /* Allow "--invokeCheck", "--invokeCheck=1" */

            *requireImmediateExit = 1;

            /* Check Remote Executable invocation by GRAM */
            subSwitchName = "--quiet";
            if ((i + 1 < argc) && 
                (strcmp(argv[i + 1], subSwitchName) == 0)) {

                /* Success */
                exit(0);
            }
            printf("Ninf-G Executable process invoked.\n");

            result = globus_libc_gethostname(hostName, sizeof(hostName));
            if (result != 0) {
                printf("hostname : unknown by error.\n");
            } else {
                printf("hostname : %s\n", hostName);
            }
            printf("pid  : %ld\n", (long)getpid());

            resultPtr = getcwd(workingDirectory, sizeof(workingDirectory));
            if (resultPtr == NULL) {
                printf("cwd  : unknown by error.\n");
            } else {
                printf("cwd  : %s\n", workingDirectory);
            }

            printf("path : %s\n", argv[0]);
            printf("exit.\n");

            /* Success */
            exit(0);
        }
    }

    /* Success */
    return 1;
}

/**
 * Initialize
 */
int
ngexStubInitialize(
    int argc,
    char *argv[],
    ngRemoteClassInformation_t *rcInfo,
    int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubInitialize(argc, argv, rcInfo, 0, &local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

/**
 * Initialize for MPI program
 */
int ngexStubInitializeMPI(
    int argc,
    char **argv,
    ngRemoteClassInformation_t *rcInfo,
    int rank,
    int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubInitialize(argc, argv, rcInfo, rank, &local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubInitialize(
    int argc,
    char *argv[],
    ngRemoteClassInformation_t *rcInfo,
    int rank,
    int *error)
{
    int result;
    int nBytes;
    char tmp[1024];
    char host[NGI_HOST_NAME_MAX];
    static const char fName[] = "ngexlStubInitialize";

    /* Is argc valid? */
    if (argc <= 1) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The argc %d is smaller equal 1.\n", fName, argc);
	return 0;
    }

    /* Is argv valid? */
    if (argv == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The argv is NULL.\n", fName);
	return 0;
    }

    /* Is argv[1] valid? */
    if (argv[1] == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: The argv[1] is NULL.\n", fName);
	return 0;
    }

    if (rank < 0) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: rank is a negative number.\n", fName);
	return 0;
    }

    if (rank == 0) {
        /* Initialize the Signal Manager */
        result = ngexiSignalManagerInitialize(NULL, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Signal Manager.\n", fName);
            return 0;
        }

        /* Initialize the Context */
        result = ngexiContextInitialize(&ngexiContext, argc, argv, rcInfo, rank, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Context.\n", fName);
            return 0;
        }
        ngexiContextInitialized = 1;

        /* log */
        ngLogPrintf(ngexiContext.ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Ninf-G Executable Context was created.\n", fName);

        /* Negotiation */
        result = ngexiProcessNegotiation(
            &ngexiContext, ngexiContext.ngc_log, error);
        if (result == 0) {
            ngLogPrintf(ngexiContext.ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL, "%s: Negotiation failed.\n", fName);
            return 0;
        }

        /* log */
        ngLogPrintf(ngexiContext.ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Executable ID = %d.\n",
            fName, ngexiContext.ngc_commInfo.ngci_executableID);

        /* Change log file name to executableID numbered file */
        result = ngiLogExecutableIDchanged(ngexiContext.ngc_log,
            ngexiContext.ngc_commInfo.ngci_executableID, error);
        if (result == 0) {
            ngLogPrintf(ngexiContext.ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't change the log file name.\n", fName);
            return 0;
        }

        /* Construct the Communication Log */
        if (ngexiContext.ngc_lmInfo.nglmi_commLog.ngli_enable != 0) {

            /* Create the message */
            result = gethostname(&host[0], sizeof (host));
            if (result < 0) {
                NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
                ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                    NULL, "%s: Can't get the host name.\n", fName);
                return 0;
            }
            nBytes = sprintf(tmp, "Executable: %s, Client: %s",
                host, ngexiContext.ngc_commInfo.ngci_hostName);
            assert((nBytes + 1) <= sizeof (tmp));


            ngexiContext.ngc_commLog = ngiLogConstruct(
                NG_LOG_TYPE_COMMUNICATION, tmp,
                &ngexiContext.ngc_lmInfo.nglmi_commLog,
                ngexiContext.ngc_commInfo.ngci_executableID, error);
            if (ngexiContext.ngc_commLog == NULL) {
                ngLogPrintf(ngexiContext.ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't create the Communication Log.\n", fName);
                return 0;
            }

            /* Register the Communication Log */
            result = ngiCommunicationLogRegister(
                ngexiContext.ngc_communication, ngexiContext.ngc_commLog,
                ngexiContext.ngc_log, error);
            if (result == 0) {
                ngLogPrintf(ngexiContext.ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't register the Communication Log.\n", fName);
                return 0;
            }
        }

        /* Switch stdout/stderr output if necessary */
        result = ngexiContextSwitchStdoutStderr(
            &ngexiContext, ngexiContext.ngc_commInfo.ngci_executableID, error);
        if (result == 0) {
            ngLogPrintf(ngexiContext.ngc_log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't switch stdout/stderr output.\n", fName);
            return 0;
        }

        /* log */
        ngLogPrintf(ngexiContext.ngc_log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: Initialize was successful.\n", fName);

        /* Register the I/O Callback */
        result = ngexiProtocolRegisterCallback(&ngexiContext, error);
        if (result == 0) {
            ngLogPrintf(ngexiContext.ngc_log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't register I/O callback.\n", fName);
            return 0;
        }

    } else {
        /* rank != 0 */

        /* Initialize the Context */
        result = ngexiContextInitialize(&ngexiContext, argc, argv, rcInfo, rank, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't initialize the Context.\n", fName);
            return 0;
        }
        ngexiContextInitialized = 1;
    }


    /* Success */
    return 1;
}

/**
 * Get request
 *
 * This function returned when receive request, which ARGUMENT or EXIT.
 * Otherwise, process the requests and not returned.
 *
 * @return
 * This function returns method ID. The value of method ID is greater equal 0.
 * Otherwise, returns NGEXI_EXIT when received exit request and returns 
 * NGEXI_FAIL when error occurred.
 */
int
ngexStubGetRequest(int *methodID, int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubGetRequest(methodID, &local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubGetRequest(int *methodID, int *error)
{
    int result;
    int finished;
    char *string;
    ngLog_t *log;
    int returnCode;
    ngexiContext_t *context;
    ngiProtocol_t *protocol;
    ngexiExecutableStatus_t status;
    static const char fName[] = "ngexlStubGetRequest";

    static ngexiExecutableStatus_t waitStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CALCULATING,
        NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_END,
    };

    /* Initialize the local variables */
    context = &ngexiContext;
    protocol = context->ngc_protocol;
    log = context->ngc_log;
    returnCode = NGEXI_FAIL;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: GetRequest started.\n", fName);

    /* Check the arguments */
    if (methodID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: methodID is NULL.\n", fName);
        return NGEXI_FAIL;
    }

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    if ((status < NGEXI_EXECUTABLE_STATUS_IDLE) ||
        (status > NGEXI_EXECUTABLE_STATUS_END)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unexpected state: %d.\n", fName, status);
        return NGEXI_FAIL;
    }

    finished = 0;
    while (finished == 0) {
        
        /* Wait the status */
        result = ngexiContextExecutableStatusWait(
            context, &waitStatus[0], NGI_NELEMENTS(waitStatus),
            &status, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the status.\n", fName);
            goto error;
        }

        switch (status) {
        case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
        case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
        case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
            /* Process request */
            result = ngexiProtocolReplyByStatus(
                context, protocol, status, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't send the reply.\n", fName);
                goto error;
            }

            if (status == NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED) {
                returnCode = NGEXI_EXIT;
                finished = 1;

                /* log */
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
                    "%s: GetRequest is returned by EXIT.\n",
                    fName);
            }
            break;

        case NGEXI_EXECUTABLE_STATUS_CALCULATING:

            *methodID = ngexiContextGetMethodID(context);

            returnCode = NGEXI_INVOKE_METHOD;
            finished = 1;

            /* log */
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
                "%s: GetRequest is returned by Invoke method. MethodID = %d.\n",
                fName, *methodID);
            break;

        case NGEXI_EXECUTABLE_STATUS_END:
        default:
            /* log */
            string = ngexiContextExecutableStatusStringGet(status, log, NULL);
            if (string == NULL) {
                string = "Not valid status";
            }
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Unexpected status \"%s\":%d was found.\n",
                fName, string, status);
            goto error;
        }
    }

    /* Success */
    return returnCode;

    /* Error occurred */
error:
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s: return by abnormal error\n", fName);

    /* Failed */
    return NGEXI_FAIL;
}

/**
 * Get the argument.
 */
int
ngexStubGetArgument(int argumentNo, void *data, int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubGetArgument(argumentNo, data, &local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubGetArgument(int argumentNo, void *data, int *error)
{
    int result;
    int methodID;
    ngLog_t *log;
    ngiArgument_t *arg;
    ngiArgumentPointer_t src, dest;
    ngRemoteMethodInformation_t *rmInfo;
    ngexiContext_t *context;
    static const char fName[] = "ngexlStubGetArgument";

    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CALCULATING,
    };

    /* Initialize the local variables */
    context = &ngexiContext;
    log = context->ngc_log;

    /* Get methodID */
    methodID = ngexiContextGetMethodID(context);

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Getting argument %3d for method %d from Context.\n",
        fName, argumentNo, methodID);

    /* Check the status */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        return 0;
    }

    /* Get the Method Information */
    rmInfo = ngexiRemoteMethodInformationGet(
	context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Remote Method Information is not found.\n", fName);
	return 0;
    }

    /* Get the Argument Data */
    ngexiContextGetMethodArgument(context, &arg);
    if (arg == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Argument Data is NULL.\n", fName);
	return 0;
    }
    if (arg->nga_nArguments < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: The number of Argument Data is smaller than zero.\n", fName);
	return 0;
    }
    if (arg->nga_nArguments > rmInfo->ngrmi_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: The number of Argument Data %d is greater than maximum %d.\n",
	    fName, arg->nga_nArguments, rmInfo->ngrmi_nArguments);
	return 0;
    }
    if (argumentNo < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: The requested Argument No. %d is smaller equal zero.\n",
	    fName, argumentNo);
	return 0;
    }
    if (argumentNo >= rmInfo->ngrmi_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: The number of Argument Data %d is greater than maximum %d.\n",
	    fName, argumentNo, rmInfo->ngrmi_nArguments);
	return 0;
    }

    /* Is argument scalar variable? */
    src = arg->nga_argument[argumentNo].ngae_pointer;
    if (arg->nga_argument[argumentNo].ngae_dataType ==
	NG_ARGUMENT_DATA_TYPE_FILENAME) {
        if (arg->nga_argument[argumentNo].ngae_tmpFileNameTable == NULL) {
	    *(char **)data = NULL;
        } else if (arg->nga_argument[argumentNo].ngae_nDimensions <= 0) {
            *(char **)data =
                arg->nga_argument[argumentNo].ngae_tmpFileNameTable[0];
        } else {
            *(char ***)data =
                arg->nga_argument[argumentNo].ngae_tmpFileNameTable;
        } 
    } else if (arg->nga_argument[argumentNo].ngae_dataType ==
	       NG_ARGUMENT_DATA_TYPE_STRING) {
	if (src.ngap_string == NULL) {
	    *(char **)data = NULL;
        } else if (arg->nga_argument[argumentNo].ngae_nDimensions <= 0) {
            *(char **)data = src.ngap_stringArray[0];
        } else {
            *(char ***)data = src.ngap_stringArray;
        }
    } else if (arg->nga_argument[argumentNo].ngae_nDimensions != 0) {
	*(void **)data = src.ngap_void;
    } else {
	dest.ngap_void = data;
	result = ngiGetArgumentData(
	    arg->nga_argument[argumentNo].ngae_dataType,
	    src, dest, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
		NULL, "%s: Can't get the Argument Data.\n", fName);
	    return 0;
	}
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: Getting argument %3d for method %d finished.\n",
        fName, argumentNo, methodID);

    /* Success */
    return 1;
}

/**
 * Calculation start.
 */
int
ngexStubCalculationStart(int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubCalculationStart(&local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubCalculationStart(int *error)
{
    int result;
    ngLog_t *log;
    ngexiContext_t *context;
    static const char fName[] = "ngexlStubCalculationStart";

    static const ngexiExecutableStatus_t reqStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CALCULATING,
    };

    /* Initialize the local variables */
    context = &ngexiContext;
    log = context->ngc_log;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Calculation started.\n", fName);

    /* Check the status */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Invalid state.\n", fName);
        return 0;
    }

    /* Set start time */
    result = ngexiContextSetMethodStartTime(&ngexiContext, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the Start Time.\n", fName);
	return 0;
    }

    return 1;
}

/**
 * Calculation end.
 */
int
ngexStubCalculationEnd(int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubCalculationEnd(&local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubCalculationEnd(int *error)
{
    int result;
    ngLog_t *log;
    ngexiContext_t *context;
    ngiProtocol_t *protocol;
    ngexiExecutableStatus_t status;
    static const char fName[] = "ngexlStubCalculationEnd";

    /* Initialize the local variables */
    context = &ngexiContext;
    protocol = context->ngc_protocol;
    log = context->ngc_log;

    /* Set end time */
    result = ngexiContextSetMethodEndTime(context, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't set the End Time.\n", fName);
	return 0;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Calculation finished.\n", fName);

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    switch(status) {
    case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_PULL_WAIT:
    case NGEXI_EXECUTABLE_STATUS_END:
	return 1;
    case NGEXI_EXECUTABLE_STATUS_CALCULATING:
        /* Do nothing, go next */
        break;
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unexpected state: %d.\n", fName, status);
	return 0;
    }

    /* Send notify */
    result = ngexiProtocolNotifyCalculationEnd(
        context, protocol, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't send the notify Calculation End.\n", fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
int
ngexStubFinalize(int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlStubFinalize(&local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlStubFinalize(
    int *error)
{
    int result;
    int rank = -1;
    ngLog_t *log = NULL;
    static const char fName[] = "ngexlStubFinalize";

    log = ngexiContext.ngc_log;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Finalizing Ninf-G Executable.\n", fName);

    /* Finalize the Context */
    if (ngexiContextInitialized != 0) {
        rank = ngexiContext.ngc_rank;

	result = ngexiContextFinalize(&ngexiContext, error);
	ngexiContextInitialized = 0;
        log = NULL;
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
		NG_LOG_LEVEL_ERROR, NULL, "%s: Can't finalize the Context.\n",
		fName);
	    return 0;
	}
    }
    log = NULL;

    if (rank == 0) {
        /* Finalize the Signal Manager */
        result = ngexiSignalManagerFinalize(log, error);
        if (result == 0) {
            ngLogPrintf(NULL, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't finalize the Signal Manager.\n", fName);
            return 0;
        }
    }

    /* Success */
    return 1;
}

/**
 * Is canceled session?
 */
int
grpc_is_canceled_np(grpc_error_t *error)
{
    int isCanceled;
    int err;

    isCanceled = ngexIsCanceled(&err);
    *error = grpc_i_get_error_from_ng_error(err);
    return isCanceled;
}

int
ngexIsCanceled(int *error)
{
    int local_error, result;

    /* Clear the error */
    NGI_SET_ERROR(&local_error, NG_ERROR_NO_ERROR);

    result = ngexlIsCanceled(&local_error);
    ngexiContext.ngc_error = local_error;
    NGI_SET_ERROR(error, local_error);

    return result;
}

static int
ngexlIsCanceled(int *error)
{
    ngLog_t *log;
    ngexiContext_t *context;
    ngexiExecutableStatus_t status;
    static const char fName[] = "ngexlIsCanceled";

    /* Initialize the local variables */
    context = &ngexiContext;
    log = context->ngc_log;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Checking if the session was canceled.\n", fName);

    /* Pass the control for Globus I/O callback. although NonThread */
    globus_thread_yield();

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    switch(status) {
    case NGEXI_EXECUTABLE_STATUS_CALCULATING:
        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: The session was not canceled.\n", fName);

        /* Not canceled */
        return 0;

    case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_PULL_WAIT:
    case NGEXI_EXECUTABLE_STATUS_END:
        /* Do not reply. reply is treated by ngexStubGetRequest */
        break;

    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unexpected state: %d\n", fName, status);
        goto error;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: The session was canceled.\n", fName);

    /* Canceled */
    return 1;

    /* Error occurred */
error:
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
        "%s: return by abnormal error\n", fName);

    /* Failed */
    return 0; /* Attention : same as not canceled */
}

/**
 * Get Ninf-G Context for Executable
 */
ngexiContext_t *
ngexiContextGet(ngLog_t *log, int *error)
{
    return &ngexiContext;
}

/**
 * Get error code
 */
int
ngexGetError()
{
    return ngexiContextGetError(&ngexiContext, NULL);
}

/**
 * Callback function is performed.
 */
int
ngexStubCallback(int callbackID, int *error, ...)
{
    int result;
    va_list ap;
    int finished;
    int methodID;
    char *string;
    ngLog_t *log;
    int i, count = 0;
    ngiProtocol_t *protocol;
    ngexiContext_t *context;
    ngexiExecutableStatus_t status;
    ngiArgument_t *arg, *callbackArg;
    ngRemoteMethodInformation_t *rmInfo, *rmInfoCallback;
    static const char fName[] = "ngexStubCallback";

    static ngexiExecutableStatus_t waitStatus[] = {
        NGEXI_EXECUTABLE_STATUS_CALCULATING,
        NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED,
        NGEXI_EXECUTABLE_STATUS_END,
    };

    /* Initialize the local variables */
    context = &ngexiContext;
    protocol = context->ngc_protocol;
    log = context->ngc_log;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: Performing callback (id=%d).\n", fName, callbackID);

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    switch(status) {
    case NGEXI_EXECUTABLE_STATUS_CALCULATING:
        /* Do nothing, go next */
        break;
    case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_PULL_WAIT:
    case NGEXI_EXECUTABLE_STATUS_END:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: The session is already canceled. (status=%d)\n",
            fName, status);
        return 0;
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Unexpected state: %d.\n", fName, status);
        return 0;
    }

    /* Set the callback ID to Protocol */
    result = ngiProtocolSetCallbackID(protocol, callbackID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the callback ID.\n", fName);
        return 0;
    }

    /* Get the argument pointer */
    va_start(ap, error);

    /* Get the Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(
	context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Remote Method Information is not found.\n", fName);
	return 0;
    }
    for (i = 0; i < rmInfo->ngrmi_nArguments; i++) {
        if (rmInfo->ngrmi_arguments[i].ngai_dataType ==
            NG_ARGUMENT_DATA_TYPE_CALLBACK) {
            if (count == callbackID) {
                break;
            }
            count += 1;
        }
    }
    if ((i == rmInfo->ngrmi_nArguments) && (count != callbackID)) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Callback ID %d is not valid.\n", fName, callbackID);
        return 0;
    }
    rmInfoCallback = rmInfo->ngrmi_arguments[i].ngai_callback;

    /* Get the argument */
    ngexiContextGetMethodArgument(&ngexiContext, &arg);
    if (arg == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Can't get the argument.\n", fName);
	return 0;
    }

    /* Construct the Argument */
    callbackArg = ngiArgumentConstruct(
	rmInfoCallback->ngrmi_arguments, rmInfoCallback->ngrmi_nArguments,
        log, error);
    if (callbackArg == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't construct the argument.\n", fName);
	return 0;
    }

    result = ngiArgumentElementInitializeVarg(
	callbackArg, rmInfoCallback->ngrmi_nArguments, ap,
	NGI_ARGUMENT_DELIVERY_C, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
	    NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't initialize the Argument Element.\n", fName);
	return 0;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
	callbackArg, arg, rmInfoCallback->ngrmi_arguments, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Argument Element.\n", fName);
	return 0;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(callbackArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Subscript Value of Argument is not valid.\n", fName);
        goto error;
    }

    /* Set the Argument for Callback*/
    ngexiContextSetCallbackArgument(context, callbackArg);

    /* Set the argument to protocol */
    result = ngiProtocolSetCallbackArgument(protocol, callbackArg, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the argument to protocol.\n", fName);
        return 0;
    }

    /* Send Notify Invoke Callback */
    result = ngexiProtocolNotifyInvokeCallback(
        context, protocol, callbackID, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set executable ID to protocol.\n", fName);
        return 0;
    }

    /* Increment the number of times to which the callback was called */
    context->ngc_sessionInfo.ngsi_callbackNtimesCalled++;

    finished = 0;
    while (finished == 0) {
        
        /* Wait the status */
        result = ngexiContextExecutableStatusWait(
            context, &waitStatus[0], NGI_NELEMENTS(waitStatus),
            &status, error);
        if (result == 0) {
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't wait the status.\n", fName);
            goto error;
        }

        switch (status) {
        case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
        case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
        case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:

            /* Do not reply. reply is treated by ngexStubGetRequest */
            finished = 1;
            break;

        case NGEXI_EXECUTABLE_STATUS_CALCULATING:
            finished = 1;
            break;

        case NGEXI_EXECUTABLE_STATUS_END:
        default:
            /* log */
            string = ngexiContextExecutableStatusStringGet(status, log, NULL);
            if (string == NULL) {
                string = "Not valid status";
            }
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Unexpected status \"%s\":%d was found.\n",
                fName, string, status);
            goto error;
        }
    }

    /* Release the argument pointer */
    va_end(ap);

    /* Release the argument to protocol */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the argument to protocol.\n", fName);
        return 0;
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the callback ID to protocol.\n", fName);
        return 0;
    }

    /* Release the Sequence number to protocol */
    result = ngiProtocolReleaseSequenceNoOfCallback(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Sequence number to protocol.\n", fName);
        return 0;
    }

    /* Check canceled */
    switch (status) {
    case NGEXI_EXECUTABLE_STATUS_CANCEL_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_RESET_REQUESTED:
    case NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED:
        return 0;
    default:
        break;
    }

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_INFORMATION, NULL,
        "%s: The callback was successfully done (id=%d).\n",
        fName, callbackID);

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the argument pointer */
    va_end(ap);

    /* Release the argument to protocol */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the argument to protocol.\n", fName);
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the callback ID to protocol.\n", fName);
    }

    /* Release the Sequence number to protocol */
    result = ngiProtocolReleaseSequenceNoOfCallback(protocol, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't release the Sequence number to protocol.\n", fName);
    }

    /* Initialize the IDs */
    ngiProtocolInitializeID(protocol);

    return 0;
}

