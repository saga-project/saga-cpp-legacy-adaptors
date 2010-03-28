/*
 * $RCSfile: grpc.h,v $ $Revision: 1.53 $ $Date: 2006/02/02 07:04:36 $
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
#ifndef _GRPC_H_
#define _GRPC_H_

/**
 * This file define the Data Structures for Grid RPC API.
 *
 */

#include <assert.h>
#include "ng.h"
#include "grpcError.h"

/**
 * The value returned when effective Session ID is not found.
 */
#define GRPC_SESSIONID_VOID               (-1)

/**
 * Status of Session
 */
/* Session is done */
#define GRPC_SESSION_DONE                  1
/* Argument transmission is not completed. */
#define GRPC_SESSION_ARG_IS_NOT_TRANSMITED 2
/* Session is running */
#define GRPC_SESSION_EXECUTING             3
/* Session is not running */
#define GRPC_SESSION_DOWN                  4
/* API was failed */
#define GRPC_SESSION_UNKNOWN_STATUS        5

/**
 * Represent a specific non-blocking GridRPC call
 */
typedef int grpc_sessionid_t;

/**
 * Infomation for all error and status codes
 */
typedef int grpc_error_t;

/*
 * Wait for completion of transfer argument
 */
typedef enum ngArgumentTransfer_e grpc_argument_transfer_t_np;

/* Wait for completion */
#define GRPC_ARGUMENT_TRANSFER_WAIT   NG_ARGUMENT_TRANSFER_WAIT 
/* Nowait for completion */
#define GRPC_ARGUMENT_TRANSFER_NOWAIT NG_ARGUMENT_TRANSFER_NOWAIT
/* Copy the argument */
#define GRPC_ARGUMENT_TRANSFER_COPY   NG_ARGUMENT_TRANSFER_COPY

/**
 * Handle attribute
 */
typedef struct grpc_handle_attr_s_np {
    char *gha_remoteHostName;  /* Host name of Remote machine */
    int   gha_remotePortNo;    /* Port number of Remote machine */
    char *gha_jobManager;      /* Jobmanager of Remote machine */
    char *gha_subject;         /* Subject of Remote machine */
    char *gha_functionName;    /* Function name of Remote machine */
    int   gha_jobStartTimeout; /* Timeout when Executable invoke */
    int   gha_jobStopTimeout;  /* Timeout when Executable end */
    /* Wait for the end of argument transfer or not */
    grpc_argument_transfer_t_np  gha_waitArgTransfer;
    char *gha_queueName;       /* Queue name of Remote machine */
    int   gha_mpiNcpus;        /* Number of CPUs */
} grpc_handle_attr_t_np;

/**
 * Handle attribute name
 */
typedef enum grpc_handle_attr_name_e_np {
    GRPC_HANDLE_ATTR_HOSTNAME,          /* Host name of Remote machine */
    GRPC_HANDLE_ATTR_PORT,              /* Port number of Remote machine */
    GRPC_HANDLE_ATTR_JOBMANAGER,        /* Jobmanager of Remote machine */
    GRPC_HANDLE_ATTR_SUBJECT,           /* Subject of Remote machine */
    GRPC_HANDLE_ATTR_FUNCNAME,          /* Function name of Remote machine */
    GRPC_HANDLE_ATTR_JOBSTARTTIMEOUT,   /* Timeout when Executable invoke */
    GRPC_HANDLE_ATTR_JOBSTOPTIMEOUT,    /* Timeout when Executable end */
    GRPC_HANDLE_ATTR_WAIT_ARG_TRANSFER, /* Wait for the end of argument transfer or not */
    GRPC_HANDLE_ATTR_QUEUENAME,         /* Queue name of Remote machine */
    GRPC_HANDLE_ATTR_MPI_NCPUS          /* Number of CPUs */
} grpc_handle_attr_name_t_np;

/**
 * Session attribute
 */
typedef struct grpc_session_attr_s_np {
    /* Wait for the end of argument transfer or not */
    grpc_argument_transfer_t_np  gsa_waitArgTransfer;
    int                          gsa_timeout;
} grpc_session_attr_t_np;

/**
 * Session attribute name
 */
typedef enum grpc_session_attr_name_e_np {
    GRPC_SESSION_ATTR_WAIT_ARG_TRANSFER, /* Wait for the end of argument transfer or not */
    GRPC_SESSION_ATTR_SESSION_TIMEOUT   /* Session timeout */
} grpc_session_attr_name_t_np;

/**
 * This type for distinguishing function handle and object handle
 */
typedef enum grpc_handle_discernment_e_np {
    /* For to detect uninitialized handle */
    GRPC_HANDLE_TYPE_INVALID  = 0,
    GRPC_HANDLE_TYPE_FUNCTION = 12345, /* Magic number to detect error */
    GRPC_HANDLE_TYPE_OBJECT   = 54321  /* Magic number to detect error */
} grpc_handle_discernment_t_np;

/**
 * Function handle
 */
typedef struct grpc_function_handle_s {
    int                    gfh_id;
    grpc_handle_attr_t_np *gfh_attr;
    ngclExecutable_t      *gfh_exec;
    grpc_error_t           gfh_errorCode;
    char                  *gfh_errorMessage;
    grpc_handle_discernment_t_np gfh_discernment;
} grpc_function_handle_t;

/**
 * Object handle
 */
typedef struct grpc_object_handle_s_np {
    int                    goh_id;
    grpc_handle_attr_t_np *goh_attr;
    ngclExecutable_t      *goh_exec;
    grpc_error_t           goh_errorCode;
    char                  *goh_errorMessage;
    grpc_handle_discernment_t_np goh_discernment;
} grpc_object_handle_t_np;

/**
 * Infomation for Argument stacks
 */
#define grpc_arg_stack_t ngclArgumentStack_t

/**
 * Session Infomation
 */
/* Measured by the remote method */
typedef struct grpc_exec_info_executable_s_np {
    int callbackNtimesCalled;

    /* The time concerning argument transmission */
    struct timeval transferArgumentToRemoteRealTime;
    struct timeval transferArgumentToRemoteCpuTime;

    /* The time concerning transfer file from client to remote */
    struct timeval transferFileToRemoteRealTime;
    struct timeval transferFileToRemoteCpuTime;

    /* The time of Calculation time of executable */
    struct timeval calculationRealTime;
    struct timeval calculationCpuTime;

    /* The time concerning transmitting a result */
    struct timeval transferResultToClientRealTime;
    struct timeval transferResultToClientCpuTime;

    /* The time concerning transfer file from client to remote */
    struct timeval transferFileToClientRealTime;
    struct timeval transferFileToClientCpuTime;

    /* The time concerning argument transmission of callback */
    struct timeval callbackTransferArgumentToClientRealTime;
    struct timeval callbackTransferArgumentToClientCpuTime;

    /* The time concerning callback */
    struct timeval callbackCalculationRealTime;
    struct timeval callbackCalculationCpuTime;

    /* The time concerning transmitting a result of callback */
    struct timeval callbackTransferResultToRemoteRealTime;
    struct timeval callbackTransferResultToRemoteCpuTime;
} grpc_exec_info_executable_t_np;

/* Measured by the client */
typedef struct grpc_exec_info_client_s_np {
    int callbackNtimesCalled;

    /* The time concerning request remote machine information */
    struct timeval remoteMachineInfoRequestRealTime;
    struct timeval remoteMachineInfoRequestCpuTime;

    /* The time concerning request remote class information */
    struct timeval remoteClassInfoRequestRealTime;
    struct timeval remoteClassInfoRequestCpuTime;

    /* The time concerning invoke GRAM */
    struct timeval gramInvokeRealTime;
    struct timeval gramInvokeCpuTime;

    /* The time concerning argument transmission */
    struct timeval transferArgumentToRemoteRealTime;
    struct timeval transferArgumentToRemoteCpuTime;

    /* The Calculation time of client */
    struct timeval calculationRealTime;
    struct timeval calculationCpuTime;

    /* The time concerning transmitting a result */
    struct timeval transferResultToClientRealTime;
    struct timeval transferResultToClientCpuTime;

    /* The time concerning argument transmission of callback */
    struct timeval callbackTransferArgumentToClientRealTime;
    struct timeval callbackTransferArgumentToClientCpuTime;

    /* The time concerning calculation of callback */
    struct timeval callbackCalculationRealTime;
    struct timeval callbackCalculationCpuTime;

    /* The time concerning transmitting a result of callback */
    struct timeval callbackTransferResultToRemoteRealTime;
    struct timeval callbackTransferResultToRemoteCpuTime;

} grpc_exec_info_client_t_np;

/* These defines as follows are for compatibility with an old version */
#define transferArgumentRealTime	transferArgumentToRemoteRealTime
#define transferArgumentCpuTime		transferArgumentToRemoteCpuTime
#define transferFileClientToRemoteRealTime transferFileToRemoteRealTime
#define transferFileClientToRemoteCpuTime  transferFileToRemoteCpuTime
#define transferResultRealTime		transferResultToClientRealTime
#define transferResultCpuTime		transferResultToClientCpuTime
#define transferFileRemoteToClientRealTime transferFileToClientRealTime
#define transferFileRemoteToClientCpuTime  transferFileToClientCpuTime
#define callbackTransferArgumentRealTime callbackTransferArgumentToClientRealTime
#define callbackTransferArgumentCpuTime callbackTransferArgumentToClientCpuTime
#define callbackTransferResultRealTime callbackTransferResultToRemoteRealTime
#define callbackTransferResultCpuTime  callbackTransferResultToRemoteCpuTime

/* Compression Information */
typedef struct grpc_compression_info_s_np {
    int		valid;	/* data below valid? 0:invalid, 1:valid */

    /* Number of bytes of data before compression */
    size_t	originalNbytes;

    /* Number of bytes of data after compression */
    size_t	compressionNbytes;

    /* Lapsed time at the time of compression */
    struct timeval compressionRealTime;
    struct timeval compressionCpuTime;

    /* Lapsed time at the time of decompression */
    struct timeval decompressionRealTime;
    struct timeval decompressionCpuTime;
} grpc_compression_info_t_np;

/* Session Information */
typedef struct grpc_session_info_s_np {
    grpc_exec_info_executable_t_np gei_measureExecutable;
    grpc_exec_info_client_t_np     gei_measureClient;

    struct {
	/* Number of elements as toRemote and toClient */
	int nElements;
	grpc_compression_info_t_np *toRemote;
	grpc_compression_info_t_np *toClient;
    } gei_compressionInformation;
} grpc_session_info_t_np;

#define grpc_exec_info_s_np grpc_session_info_s_np
typedef struct grpc_exec_info_s_np grpc_exec_info_t_np;

/*
 * The following definitions are the definition for compatibility with the old
 * version. Use of calculation*Time are recommended.
 */
#define executableRealTime calculationRealTime
#define executableCpuTime  calculationCpuTime
#define clientRealTime     calculationRealTime
#define clientCpuTime      calculationCpuTime

/**
 * For Backward compatibility.
 */
#define grpc_get_last_error_np grpc_last_error_get_np


/**
 * Prototype declaration of GRPC APIs
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initializing/Finalizing API */
grpc_error_t grpc_initialize(char *);
grpc_error_t grpc_finalize(void);

/* Read the configuration file */
grpc_error_t grpc_config_file_read_np(char *);

/* Function handle management API */
grpc_error_t grpc_function_handle_init(
    grpc_function_handle_t *, char *, char *);
grpc_error_t grpc_function_handle_init_with_attr_np(
    grpc_function_handle_t *, grpc_handle_attr_t_np *);
grpc_error_t grpc_function_handle_default(grpc_function_handle_t *, char *);
grpc_error_t grpc_function_handle_destruct(grpc_function_handle_t *);
grpc_error_t grpc_function_handle_array_init_np(
    grpc_function_handle_t *, size_t, char *, char *);
grpc_error_t grpc_function_handle_array_init_with_attr_np(
    grpc_function_handle_t *, size_t, grpc_handle_attr_t_np *);
grpc_error_t grpc_function_handle_array_default_np(
    grpc_function_handle_t *, size_t, char *);
grpc_error_t grpc_function_handle_array_destruct_np(
    grpc_function_handle_t *, size_t);
grpc_error_t grpc_function_handle_get_attr_np(
    grpc_function_handle_t *, grpc_handle_attr_t_np *);
grpc_error_t grpc_function_handle_get_from_session_np(
    grpc_function_handle_t **, grpc_sessionid_t);
grpc_error_t grpc_get_handle(grpc_function_handle_t **, grpc_sessionid_t);
grpc_error_t grpc_function_handle_perror_np(grpc_function_handle_t *, char *);
grpc_error_t grpc_function_handle_get_error_np(grpc_function_handle_t *);
grpc_error_t grpc_function_handle_is_ready_np(grpc_function_handle_t *);

/* Object handle management API */
grpc_error_t grpc_object_handle_init_np(
    grpc_object_handle_t_np *, char *, char *);
grpc_error_t grpc_object_handle_init_with_attr_np(
    grpc_object_handle_t_np *, grpc_handle_attr_t_np *);
grpc_error_t grpc_object_handle_default_np(grpc_object_handle_t_np *, char *);
grpc_error_t grpc_object_handle_destruct_np(grpc_object_handle_t_np *);
grpc_error_t grpc_object_handle_array_init_np(
    grpc_object_handle_t_np *, size_t, char *, char *);
grpc_error_t grpc_object_handle_array_init_with_attr_np(
    grpc_object_handle_t_np *, size_t, grpc_handle_attr_t_np *);
grpc_error_t grpc_object_handle_array_default_np(
    grpc_object_handle_t_np *, size_t, char *);
grpc_error_t grpc_object_handle_array_destruct_np(
    grpc_object_handle_t_np *, size_t);
grpc_error_t grpc_object_handle_get_attr_np(
    grpc_object_handle_t_np *, grpc_handle_attr_t_np *);
grpc_error_t grpc_object_handle_get_from_session_np(
    grpc_object_handle_t_np **, grpc_sessionid_t);
grpc_error_t grpc_object_handle_perror_np(grpc_object_handle_t_np *, char *);
grpc_error_t grpc_object_handle_get_error_np(grpc_object_handle_t_np *);
grpc_error_t grpc_object_handle_is_ready_np(grpc_object_handle_t_np *);

/* Handle attribute management API */
grpc_error_t grpc_handle_attr_initialize_np(grpc_handle_attr_t_np *);
grpc_error_t grpc_handle_attr_destruct_np(grpc_handle_attr_t_np *);
grpc_error_t grpc_handle_attr_get_np(
    grpc_handle_attr_t_np *, grpc_handle_attr_name_t_np, void **);
grpc_error_t grpc_handle_attr_set_np(
    grpc_handle_attr_t_np *, grpc_handle_attr_name_t_np, void *);
grpc_error_t grpc_handle_attr_release_np(void *);

/* GridRPC call API */
grpc_error_t grpc_call(grpc_function_handle_t *, ...);
grpc_error_t grpc_call_async(grpc_function_handle_t *, grpc_sessionid_t *, ...);
int  grpc_call_arg_stack(grpc_function_handle_t *, grpc_arg_stack_t *);
int  grpc_call_arg_stack_async(grpc_function_handle_t *, grpc_arg_stack_t *);

grpc_error_t grpc_call_with_attr_np(grpc_function_handle_t *,
    grpc_session_attr_t_np *, ...);
grpc_error_t grpc_call_async_with_attr_np(grpc_function_handle_t *,
    grpc_sessionid_t *, grpc_session_attr_t_np *, ...);
int  grpc_call_arg_stack_with_attr_np(grpc_function_handle_t *,
    grpc_session_attr_t_np *, grpc_arg_stack_t *);
int  grpc_call_arg_stack_async_with_attr_np(grpc_function_handle_t *,
    grpc_session_attr_t_np *, grpc_arg_stack_t *);

/* GridRPC invoke API */
grpc_error_t grpc_invoke_np(grpc_object_handle_t_np *, char *, ...);
grpc_error_t grpc_invoke_async_np(
    grpc_object_handle_t_np *, char *, grpc_sessionid_t *, ...);
int grpc_invoke_arg_stack_np(
    grpc_object_handle_t_np *, char *, grpc_arg_stack_t *);
int grpc_invoke_arg_stack_async_np(
    grpc_object_handle_t_np *, char *, grpc_arg_stack_t *);

grpc_error_t grpc_invoke_with_attr_np(
    grpc_object_handle_t_np *, char *, grpc_session_attr_t_np *, ...);
grpc_error_t grpc_invoke_async_with_attr_np(
    grpc_object_handle_t_np *, char *, grpc_sessionid_t *,
    grpc_session_attr_t_np *, ...);
int grpc_invoke_arg_stack_with_attr_np(
    grpc_object_handle_t_np *, char *, grpc_session_attr_t_np *, 
    grpc_arg_stack_t *);
int grpc_invoke_arg_stack_async_with_attr_np(
    grpc_object_handle_t_np *, char *, grpc_session_attr_t_np *,
    grpc_arg_stack_t *);

/* Session attribute management API */
grpc_error_t grpc_session_attr_initialize_np(
    grpc_function_handle_t *, grpc_session_attr_t_np *);
grpc_error_t grpc_session_attr_initialize_with_object_handle_np(
    grpc_object_handle_t_np *, grpc_session_attr_t_np *);
grpc_error_t grpc_session_attr_destruct_np(grpc_session_attr_t_np *);
grpc_error_t grpc_session_attr_get_np(
    grpc_session_attr_t_np *, grpc_session_attr_name_t_np, void **);
grpc_error_t grpc_session_attr_set_np(
    grpc_session_attr_t_np *, grpc_session_attr_name_t_np, void *);
grpc_error_t grpc_session_attr_release_np(void *);

/* Argument Stack management API */
grpc_arg_stack_t *grpc_arg_stack_new(int);
int grpc_arg_stack_destruct(grpc_arg_stack_t *);
int grpc_arg_stack_push_arg(grpc_arg_stack_t *, void *);
void *grpc_arg_stack_pop_arg(grpc_arg_stack_t *);

/* Asynchronous GridRPC wait API */
grpc_error_t grpc_wait(grpc_sessionid_t);
grpc_error_t grpc_wait_all(void);
grpc_error_t grpc_wait_any(grpc_sessionid_t *);
grpc_error_t grpc_wait_and(grpc_sessionid_t *, size_t);
grpc_error_t grpc_wait_or(grpc_sessionid_t *, size_t, grpc_sessionid_t *);

/* Asynchronous GridRPC control API */
grpc_error_t grpc_cancel(grpc_sessionid_t);
grpc_error_t grpc_cancel_all(void);
grpc_error_t grpc_probe(grpc_sessionid_t);
grpc_error_t grpc_probe_or(grpc_sessionid_t *, size_t, grpc_sessionid_t *);

/* Session Infomation management API */
grpc_error_t grpc_session_info_get_np(
    grpc_sessionid_t, grpc_session_info_t_np **, int *);
grpc_error_t grpc_session_info_release_np(
    grpc_session_info_t_np *);

grpc_error_t grpc_get_info_np(grpc_sessionid_t, grpc_exec_info_t_np *, int *);
grpc_error_t grpc_get_last_info_np(grpc_exec_info_t_np *, int *);
grpc_error_t grpc_get_error(grpc_sessionid_t);
grpc_error_t grpc_session_info_remove_np(grpc_sessionid_t);
grpc_error_t grpc_session_info_set_threshold_np(int);

/* Signal API */
grpc_error_t grpc_signal_handler_set_np(int, void (*)(int));

/* Error Reporting API */
char *grpc_error_string(grpc_error_t);
grpc_error_t grpc_get_failed_sessionid(grpc_sessionid_t *);
grpc_error_t grpc_perror_np(char *);
grpc_error_t grpc_last_error_get_np(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _GRPC_H_ */
