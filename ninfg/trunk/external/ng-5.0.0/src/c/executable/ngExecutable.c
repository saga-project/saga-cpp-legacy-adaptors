/*
 * $RCSfile: ngExecutable.c,v $ $Revision: 1.22 $ $Date: 2008/03/27 10:46:15 $
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
#include "grpc_executable.h"

NGI_RCSID_EMBED("$RCSfile: ngExecutable.c,v $ $Revision: 1.22 $ $Date: 2008/03/27 10:46:15 $")

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

extern char **environ;

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
    int result, i, j, requireClose;
    char *versionName;
    char *switchName, *subSwitchName, hostName[NGI_HOST_NAME_MAX];
    char workingDirectory[NGI_DIR_NAME_MAX], *resultPtr;

    /* Check the arguments */
    assert(requireImmediateExit != NULL);

    *requireImmediateExit = 0;
    requireClose = 0;
    versionName = NULL;

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

            result = ngVersionGet(&versionName, NULL, NULL);
            if (result != 0) {
                printf("version : %s\n", versionName);
            } else {
                printf("version : unknown by error.\n");
            }

            result = ngiHostnameGet(hostName, sizeof(hostName), NULL, NULL);
            if (result != 0) {
                printf("hostname : %s\n", hostName);
            } else {
                printf("hostname : unknown by error.\n");
            }

            printf("pid  : %ld\n", (long)getpid());

            resultPtr = getcwd(workingDirectory, sizeof(workingDirectory));
            if (resultPtr != NULL) {
                printf("cwd  : %s\n", workingDirectory);
            } else {
                printf("cwd  : unknown by error.\n");
            }

            printf("path : %s\n", argv[0]);

            subSwitchName = "--environment";
            if ((i + 1 < argc) && 
                (strcmp(argv[i + 1], subSwitchName) == 0)) {

                printf("environment variables:\n");
                for (j = 0; environ[j] != NULL; j++) {
                    printf("%s\n", environ[j]);
                }

            }

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
int
ngexStubInitializeMPI(
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
    ngCommLogPairInfo_t pairInfo;
    char host[NGI_HOST_NAME_MAX];
    ngLogConstructArgument_t logCarg;
    int logCargInitialized = 0;
    ngLogInformation_t *logInfo = NULL;
    ngLog_t *log = NULL;
    static const char fName[] = "ngexlStubInitialize";

    /* Is argc valid? */
    if (argc <= 1) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "The argc %d is smaller equal 1.\n", argc); 
	return 0;
    }

    /* Is argv valid? */
    if (argv == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "The argv is NULL.\n"); 
	return 0;
    }

    /* Is argv[1] valid? */
    if (argv[1] == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
	    "The argv[1] is NULL.\n"); 
	return 0;
    }

    if (rank < 0) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,
            "rank %d is a negative number.\n", rank);
        return 0;
    }

    if (rank == 0) {
        /* Initialize the Signal Manager */
        result = ngexiSignalManagerInitialize(NULL, error);
        if (result == 0) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Signal Manager.\n"); 
            return 0;
        }

        /* Initialize the Context */
        result = ngexiContextInitialize(
            &ngexiContext, argc, argv, rcInfo, rank, error);
        if (result == 0) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Context.\n"); 
            return 0;
        }
        ngexiContextInitialized = 1;
        log = ngexiContext.ngc_log;

        /* log */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Ninf-G Executable Context was created.\n"); 

        /* Negotiation */
        result = ngexiProcessNegotiation(&ngexiContext, log, error);
        if (result == 0) {
            ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Negotiation failed.\n"); 
            return 0;
        }

        /* log */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Executable ID = %d.\n",
            ngexiContext.ngc_commInfo.ngci_executableID); 

        /* Change log file name to executableID numbered file */
        result = ngiLogExecutableIDchanged(log,
            ngexiContext.ngc_commInfo.ngci_executableID, error);
        if (result == 0) {
            ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't change the log file name.\n"); 
            return 0;
        }

        /* Construct the Communication Log */
        if (ngexiContext.ngc_lmInfo.nglmi_commLogEnable != 0) {
     
            /* Create the message */
            logInfo = &ngexiContext.ngc_lmInfo.nglmi_commLogInfo;
     
            result = ngLogConstructArgumentInitialize(&logCarg, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't initialize log construct argument.\n"); 
                goto error;
            }
            logCargInitialized = 1;
     
            if (logInfo->ngli_filePath == NULL) {
                logCarg.nglca_output       = NG_LOG_STDERR;
            } else {
                logCarg.nglca_output       = NG_LOG_FILE;
                logCarg.nglca_nFiles       = logInfo->ngli_nFiles;
                logCarg.nglca_maxFileSize  = logInfo->ngli_maxFileSize;
                logCarg.nglca_overWriteDir = logInfo->ngli_overWriteDir;
                logCarg.nglca_appending    = 0;/* false */
     
                logCarg.nglca_filePath = ngiStrdup(
                    logInfo->ngli_filePath, log, error);
                if (logCarg.nglca_filePath == NULL) {
                    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                        "Can't copy string.\n");
                    goto error;
                }
     
                if (logInfo->ngli_suffix != NULL) {
                    logCarg.nglca_suffix = ngiStrdup(
                        logInfo->ngli_suffix, log, error);
                    if (logCarg.nglca_suffix == NULL) {
                        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                            "Can't copy string.\n");
                        goto error;
                    }
                }
            }
     
            result = ngiHostnameGet(&host[0], sizeof (host), NULL, error);
            if (result == 0) {
                ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get the host name.\n"); 
                goto error;
            }
     
            pairInfo.ngcp_localAppName   = "Executable";
            pairInfo.ngcp_localHostname  = host;
            pairInfo.ngcp_remoteAppName  = "Client";
            pairInfo.ngcp_remoteHostname = ngexiContext.ngc_commInfo.ngci_hostname;
     
            ngexiContext.ngc_commLog = ngCommLogConstructForExecutable(
                &pairInfo, ngexiContext.ngc_commInfo.ngci_executableID,
                &logCarg, log, error);
            if (ngexiContext.ngc_commLog == NULL) {
                ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't create the Communication Log.\n"); 
                goto error;
            }
     
            /* Register the Communication Log */
            result = ngiCommunicationLogRegister(
                ngexiContext.ngc_communication, ngexiContext.ngc_commLog,
                ngexiContext.ngc_log, error);
            if (result == 0) {
                ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't register the Communication Log.\n"); 
                goto error;
            }
     
            result = ngLogConstructArgumentFinalize(&logCarg, log, error);
            logCargInitialized = 0;
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't get the host name.\n"); 
                goto error;
            }
        }
     
        /* Switch stdout/stderr output if necessary */
        result = ngexiContextSwitchStdoutStderr(
            &ngexiContext, ngexiContext.ngc_commInfo.ngci_executableID, error);
        if (result == 0) {
            ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't switch stdout/stderr output.\n"); 
            return 0;
        }
     
        /* log */
        ngLogInfo(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
            "Initialize was successful.\n"); 
     
        /* Register the I/O Callback */
        result = ngexiProtocolRegisterCallback(&ngexiContext, error);
        if (result == 0) {
            ngLogError(ngexiContext.ngc_log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't register I/O callback.\n"); 
            return 0;
        }
   } else {
        /* rank != 0 */

        /* Initialize the Context */
        result = ngexiContextInitialize(
            &ngexiContext, argc, argv, rcInfo, rank, error);
        if (result == 0) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't initialize the Context.\n"); 
            return 0;
        }
        ngexiContextInitialized = 1;
    }

    /* Success */
    return 1;
error:

    if (logCargInitialized != 0) {
        result = ngLogConstructArgumentFinalize(&logCarg, log, NULL);
        logCargInitialized = 0;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't get the host name.\n"); 
            goto error;
        }
    }

    return 0;
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "GetRequest started.\n"); 

    /* Check the arguments */
    if (methodID == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "methodID is NULL.\n"); 
        return NGEXI_FAIL;
    }

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    if ((status < NGEXI_EXECUTABLE_STATUS_IDLE) ||
        (status > NGEXI_EXECUTABLE_STATUS_END)) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unexpected state: %d.\n", status); 
        return NGEXI_FAIL;
    }

    finished = 0;
    while (finished == 0) {
        
        /* Wait the status */
        result = ngexiContextExecutableStatusWait(
            context, &waitStatus[0], NGI_NELEMENTS(waitStatus),
            &status, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the status.\n"); 
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
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Can't send the reply.\n"); 
                goto error;
            }

            if (status == NGEXI_EXECUTABLE_STATUS_EXIT_REQUESTED) {
                returnCode = NGEXI_EXIT;
                finished = 1;

                /* log */
                ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "GetRequest is returned by EXIT.\n"); 
            }
            break;

        case NGEXI_EXECUTABLE_STATUS_CALCULATING:

            *methodID = ngexiContextGetMethodID(context);

            returnCode = NGEXI_INVOKE_METHOD;
            finished = 1;

            /* log */
            ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
                "GetRequest is returned by Invoke method. MethodID = %d.\n",
                *methodID); 
            break;

        case NGEXI_EXECUTABLE_STATUS_END:
        default:
            /* log */
            string = ngexiContextExecutableStatusStringGet(status, log, NULL);
            if (string == NULL) {
                string = "Not valid status";
            }
            ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unexpected status \"%s\":%d was found.\n", string, status); 
            goto error;
        }
    }

    /* Success */
    return returnCode;

    /* Error occurred */
error:
    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
        "return by abnormal error.\n"); 

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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Getting argument %3d for method %d from Context.\n",
        argumentNo, methodID); 

    /* Check the status */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        return 0;
    }

    /* Get the Method Information */
    rmInfo = ngexiRemoteMethodInformationGet(
	context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Remote Method Information is not found.\n"); 
	return 0;
    }

    /* Get the Argument Data */
    ngexiContextGetMethodArgument(context, &arg);
    if (arg == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Argument Data is NULL.\n"); 
	return 0;
    }
    if (arg->nga_nArguments < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "The number of Argument Data is smaller than zero.\n"); 
	return 0;
    }
    if (arg->nga_nArguments > rmInfo->ngrmi_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "The number of Argument Data %d is greater than maximum %d.\n",
            arg->nga_nArguments, rmInfo->ngrmi_nArguments); 
	return 0;
    }
    if (argumentNo < 0) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "The requested Argument No. %d is smaller equal zero.\n", argumentNo); 
	return 0;
    }
    if (argumentNo >= rmInfo->ngrmi_nArguments) {
	NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "The number of Argument Data %d is greater than maximum %d.\n",
            argumentNo, rmInfo->ngrmi_nArguments); 
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
	    ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't get the Argument Data.\n"); 
	    return 0;
	}
    }

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Getting argument %3d for method %d finished.\n", argumentNo, methodID); 

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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Calculation started.\n"); 

    /* Check the status */
    result = ngexiContextExecutableStatusCheck(
        context, &reqStatus[0], NGI_NELEMENTS(reqStatus), log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Invalid state.\n"); 
        return 0;
    }

    /* Set start time */
    result = ngexiContextSetMethodStartTime(&ngexiContext, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the Start Time.\n"); 
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
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't set the End Time.\n"); 
	return 0;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Calculation finished.\n"); 

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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unexpected state: %d.\n", status); 
	return 0;
    }

    /* Send notify */
    result = ngexiProtocolNotifyCalculationEnd(
        context, protocol, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't send the notify Calculation End.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize.
 */
int
ngexStubFinalize(
    int *error)
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
    int rank;
    ngLog_t *log;
    static const char fName[] = "ngexlStubFinalize";

    /* Initialize the local variables */
    log = ngexiContext.ngc_log;
    rank = -1;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Finalizing Ninf-G Executable.\n"); 

    /* Finalize the Context */
    if (ngexiContextInitialized != 0) {
        rank = ngexiContext.ngc_rank;

	result = ngexiContextFinalize(&ngexiContext, error);
	ngexiContextInitialized = 0;
        log = NULL;
	if (result == 0) {
            log = NULL;
	    ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	        "Can't finalize the Context.\n"); 
	    return 0;
	}
    }

    log = NULL;

    if (rank == 0) {
        /* Finalize the Signal Manager */
        result = ngexiSignalManagerFinalize(log, error);
        if (result == 0) {
            ngLogError(NULL, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't finalize the Signal Manager.\n"); 
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
    int result, signalBlocked;
    ngexiExecutableStatus_t status;
    static const char fName[] = "ngexlIsCanceled";

    /* Initialize the local variables */
    context = &ngexiContext;
    log = context->ngc_log;
    signalBlocked = 0;

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Checking if the session was canceled.\n"); 

    /* Block the signal for NonThread */
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_BLOCK_READ, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't block signal.\n"); 
        goto error;
    }
    signalBlocked = 1;

    /* Pass the control for I/O callback. although NonThread */
    result = ngiThreadYield(context->ngc_event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "thread yield failed.\n"); 
        goto error;
    }

    /* Unblock the signal */
    signalBlocked = 0;
    result = ngexiHeartBeatSendBlock(
        context, NGEXI_HEARTBEAT_SEND_UNBLOCK_READ, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Can't unblock signal.\n"); 
        goto error;
    }

    /* Get status */
    status = ngexiContextExecutableStatusGet(context, error);

    /* Check Status */
    switch(status) {
    case NGEXI_EXECUTABLE_STATUS_CALCULATING:
        /* log */
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The session was not canceled.\n"); 

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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unexpected state: %d\n", status); 
        goto error;
    }

    /* log */
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "The session was canceled.\n"); 

    /* Canceled */
    return 1;

    /* Error occurred */
error:
    ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
        "return by abnormal error\n"); 

    /* Unblock the signal */
    if (signalBlocked != 0) {
        signalBlocked = 0;
        result = ngexiHeartBeatSendBlock(
            context, NGEXI_HEARTBEAT_SEND_UNBLOCK_READ, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unblock signal.\n"); 
        }
    }

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
ngexGetError(void)
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "Performing callback (id=%d).\n", callbackID); 

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
        ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
            "The session is already canceled. (status=%d)\n", status); 
        return 0;
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_STATE);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Unexpected state: %d.\n", status); 
        return 0;
    }

    /* Set the callback ID to Protocol */
    result = ngiProtocolSetCallbackID(protocol, callbackID, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the callback ID.\n"); 
        return 0;
    }

    /* Get the argument pointer */
    va_start(ap, error);

    /* Get the Method Information */
    methodID = ngexiContextGetMethodID(context);
    rmInfo = ngexiRemoteMethodInformationGet(
	context, methodID, log, error);
    if (rmInfo == NULL) {
	ngLogFatal(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Remote Method Information is not found.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Callback ID %d is not valid.\n", callbackID); 
        return 0;
    }
    rmInfoCallback = rmInfo->ngrmi_arguments[i].ngai_callback;

    /* Get the argument */
    ngexiContextGetMethodArgument(&ngexiContext, &arg);
    if (arg == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't get the argument.\n"); 
	return 0;
    }

    /* Construct the Argument */
    callbackArg = ngiArgumentConstruct(
	rmInfoCallback->ngrmi_arguments, rmInfoCallback->ngrmi_nArguments,
        log, error);
    if (callbackArg == NULL) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't construct the argument.\n"); 
	return 0;
    }

    result = ngiArgumentElementInitializeVarg(
	callbackArg, rmInfoCallback->ngrmi_nArguments, ap,
	NGI_ARGUMENT_DELIVERY_C, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Argument Element.\n"); 
	return 0;
    }

    /* Initialize the Subscript Value of Argument */
    result = ngiArgumentInitializeSubscriptValue(
	callbackArg, arg, rmInfoCallback->ngrmi_arguments, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Can't initialize the Argument Element.\n"); 
	return 0;
    }

    /* Check the Subscript Value of Argument */
    result = ngiArgumentCheckSubscriptValue(callbackArg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Subscript Value of Argument is not valid.\n"); 
        goto error;
    }

    /* Set the Argument for Callback*/
    ngexiContextSetCallbackArgument(context, callbackArg);

    /* Set the argument to protocol */
    result = ngiProtocolSetCallbackArgument(protocol, callbackArg, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set the argument to protocol.\n"); 
        return 0;
    }

    /* Send Notify Invoke Callback */
    result = ngexiProtocolNotifyInvokeCallback(
        context, protocol, callbackID, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't set executable ID to protocol.\n"); 
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
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't wait the status.\n"); 
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
            ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Unexpected status \"%s\":%d was found.\n", string, status); 
            goto error;
        }
    }

    /* Release the argument pointer */
    va_end(ap);

    /* Release the argument to protocol */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the argument to protocol.\n"); 
        return 0;
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the callback ID to protocol.\n"); 
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
    ngLogInfo(log, NG_LOGCAT_NINFG_PURE, fName,  
        "The callback was successfully done (id=%d).\n", callbackID); 

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Release the argument pointer */
    va_end(ap);

    /* Release the argument to protocol */
    result = ngiProtocolReleaseCallbackArgument(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the argument to protocol.\n"); 
    }

    /* Release the callback ID to protocol */
    result = ngiProtocolReleaseCallbackID(protocol, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't release the callback ID to protocol.\n"); 
    }

    /* Initialize the IDs */
    ngiProtocolInitializeID(protocol);

    return 0;
}

