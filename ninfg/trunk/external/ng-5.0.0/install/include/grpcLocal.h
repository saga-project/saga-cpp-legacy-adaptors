/*
 * $RCSfile: grpcLocal.h,v $ $Revision: 1.7 $ $Date: 2008/01/25 03:57:38 $
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
#ifndef _GRPCINTERNAL_H_
#define _GRPCINTERNAL_H_

/**
 * This file define the Data Structures for Grid RPC API internal.
 *
 */

#include "grpc.h"


/**
 * Identification number
 */
/* Handle ID */
#define GRPC_L_HANDLE_ID_MIN        0              /* Minimum value */
#define GRPC_L_HANDLE_ID_MAX        0x7fffffff     /* Maximum value */
#define GRPC_L_HANDLE_ID_UNDEFINED  (-1)           /* Undefined value */

/**
 * Grid RPC API manager.
 */
typedef struct grpc_l_manager_s {
    /* The lock for operating ngclSession_t.
     * Get a lock, when you operate ngclSession_t.
     */
     ngExclusiveLock_t gm_exclusiveLock;
} grpc_l_manager_t;

/**
 * Session management data.
 */
typedef enum grpc_l_session_use_e {
    /* A ngclSession_s structure is under use by nobody */
    GRPC_L_SESSION_NOT_USE,

    /* A ngclSession_s structure is under use by grpc_call*() or
     * grpc_wait*().
     */
    GRPC_L_SESSION_USE_CALL
} grpc_l_session_use_t;

typedef struct grpc_l_session_data_s {
    int gsd_executableID;
    grpc_l_session_use_t gsd_sessionUse;
} grpc_l_session_data_t;

/**
 * Completed Session management queue
 */
typedef enum grpc_l_session_queue_type_e {
    GRPC_L_SESSION_QUEUE_TYPE_UNDEFINED, /* Undefined */
    GRPC_L_SESSION_QUEUE_TYPE_THRESHOLD, /* The size is limited */
    GRPC_L_SESSION_QUEUE_TYPE_NOT_SAVE, /* Do not save */
    GRPC_L_SESSION_QUEUE_TYPE_SAVE_ALL  /* Save all */
} grpc_l_session_queue_type_t;

typedef struct grpc_l_session_queue_s {
    ngclSession_t *gsq_list_head;   /* Completed Session list head */
    ngclSession_t *gsq_list_tail;   /* Completed Session list tail */
    int            gsq_nSessions;   /* The number of Sessions */
    grpc_l_session_queue_type_t gsq_type; /* store type */ 
    int            gsq_threshold;   /* threshold */
    ngiRWlock_t    gsq_lock;        /* for lock of this instance */
} grpc_l_session_queue_t;

/**
 * Error code and error Message of last generated error
 */
typedef struct grpc_l_error_s {
    grpc_error_t ge_code;    /* Error code of last generated error */
    char        *ge_message; /* Error Message of last generated error */
    ngiRWlock_t  ge_lock;    /* for lock of this instance */
} grpc_l_error_t;

/**
 * Internal function arguments
 */
typedef enum grpc_l_handle_type_e {
    GRPC_L_HANDLE_TYPE_FUNCTION, /* The handle should be function handle */
    GRPC_L_HANDLE_TYPE_OBJECT    /* The handle should be object handle */
} grpc_l_handle_type_t;

typedef enum grpc_l_handle_use_default_e {
    GRPC_L_HANDLE_USE_NO_DEFAULT, /* Use handle_init API */
    GRPC_L_HANDLE_USE_DEFAULT     /* Use handle_default API */
} grpc_l_handle_use_default_t;

typedef enum grpc_l_handle_attr_type_e {
    GRPC_L_HANDLE_ATTR_NOT_GIVEN, /* Handle Attribute was given */
    GRPC_L_HANDLE_ATTR_GIVEN      /* Handle Attribute was not given */
} grpc_l_handle_attr_type_t;

typedef enum grpc_l_argument_type_e {
    GRPC_L_ARGUMENT_TYPE_VA_LIST,   /* The argument is va_list */
    GRPC_L_ARGUMENT_TYPE_ARG_STACK  /* The argument is grpc_arg_stack_t */
} grpc_l_argument_type_t;

typedef enum grpc_l_call_wait_type_e {
    GRPC_L_CALL_WAIT_TYPE_SYNC,  /* blocking call, wait */
    GRPC_L_CALL_WAIT_TYPE_ASYNC  /* non blocking call, no wait */
} grpc_l_call_wait_type_t;

typedef enum grpc_l_wait_type_e {
    GRPC_L_WAIT_TYPE_ALL, /* Wait all session done */
    GRPC_L_WAIT_TYPE_ANY  /* Wait one session done */
} grpc_l_wait_type_t;

typedef enum grpc_l_wait_set_e {
    GRPC_L_WAIT_SET_ALL,   /* candidates all sessions */
    GRPC_L_WAIT_SET_GIVEN  /* candidates given sessions only */
} grpc_l_wait_set_t;

typedef enum grpc_l_session_attr_type_e {
    GRPC_L_SESSION_ATTR_NOT_GIVEN, /* Session Attribute was given */
    GRPC_L_SESSION_ATTR_GIVEN      /* Session Attribute was not given */
} grpc_l_session_attr_type_t;

/**
 * This structure can treat function_handle and object_handle
 * by one action.
 */
typedef struct grpc_l_executable_handle_s {
    grpc_l_handle_type_t     geh_handle_type;
    void                    *geh_is_handle_null;
    grpc_function_handle_t  *geh_function_handle;
    grpc_object_handle_t_np *geh_object_handle;
    int                     *geh_id;
    grpc_handle_attr_t_np  **geh_attr;
    ngclExecutable_t       **geh_exec;
    grpc_error_t            *geh_errorCode;
    char                   **geh_errorMessage;
    grpc_handle_discernment_t_np *geh_discernment;
} grpc_l_executable_handle_t;

typedef enum grpc_l_session_queue_unregister_mode_e {
    GRPC_L_SESSION_QUEUE_REMOVE_UNDEFINED,     /* Undefined */
    GRPC_L_SESSION_QUEUE_REMOVE_BY_SESSIONID,  /* Remove by SESSION_ID */
    GRPC_L_SESSION_QUEUE_REMOVE_TO_COUNT,      /* Reduce sessions to count */
    GRPC_L_SESSION_QUEUE_REMOVE_ALL,           /* Remove all sessions */
    GRPC_L_SESSION_QUEUE_REMOVE_NONE           /* Do not remove sessions */
} grpc_l_session_queue_unregister_mode_t;

typedef enum grpc_l_session_queue_get_mode_e {
    GRPC_L_SESSION_QUEUE_GET_UNDEFINED,     /* Undefined */
    GRPC_L_SESSION_QUEUE_GET_BY_ID,         /* Get by SessionID */
    GRPC_L_SESSION_QUEUE_GET_LAST           /* Get Last Session */
} grpc_l_session_queue_get_mode_t;

typedef enum grpc_l_session_info_get_mode_e {
    GRPC_L_SESSION_INFO_GET_UNDEFINED,      /* Undefined */
    GRPC_L_SESSION_INFO_GET_BY_ID,          /* Get by SessionID */
    GRPC_L_SESSION_INFO_GET_LAST            /* Get Last Session */
} grpc_l_session_info_get_mode_t;

typedef enum grpc_l_session_info_require_e {
    /* Require all information */
    GRPC_L_SESSION_INFO_REQUIRE_ALL,
    /* Require only execution information */
    GRPC_L_SESSION_INFO_REQUIRE_ONLY_EXECUTION
} grpc_l_session_info_require_t;

/**
 * Prototype declaration of GRPC internal APIs
 */

/**
 * Initialize internal API
 */
grpc_error_t grpc_l_set_initialize(void);
grpc_error_t grpc_l_unset_initialize(void);
grpc_error_t grpc_l_initialize_performed_check(void);

/* Handle management internal API */
grpc_error_t grpc_l_handle_init(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, grpc_l_handle_use_default_t,
    grpc_l_handle_attr_type_t, size_t, char *, char *,
    grpc_handle_attr_t_np *);
grpc_error_t grpc_l_handle_destruct(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, size_t);
grpc_error_t grpc_l_handle_initialize_member(grpc_l_executable_handle_t *);
grpc_error_t grpc_l_handle_setup(grpc_l_executable_handle_t *,
    ngclExecutable_t *, grpc_handle_attr_t_np *);
grpc_error_t grpc_l_handle_destruct_one(grpc_l_executable_handle_t *);
grpc_error_t grpc_l_handle_unregister_unused_session(
    grpc_l_executable_handle_t *, int *);
grpc_error_t grpc_l_handle_get_attr(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, grpc_handle_attr_t_np *);
grpc_error_t grpc_l_handle_perror(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, char *);
grpc_error_t grpc_l_handle_get_error(
    const char *, grpc_function_handle_t *,
    grpc_object_handle_t_np *, grpc_l_handle_type_t);
grpc_error_t grpc_l_handle_is_ready(
    const char *, grpc_function_handle_t *,
    grpc_object_handle_t_np *, grpc_l_handle_type_t);
grpc_error_t grpc_l_handle_get(
    const char *, grpc_sessionid_t, grpc_l_handle_type_t,
    grpc_function_handle_t **, grpc_object_handle_t_np **);
grpc_error_t grpc_l_handle_set_error(grpc_l_executable_handle_t *);
int grpc_l_get_handleId(void);

void grpc_l_handle_attr_initialize_member(grpc_handle_attr_t_np *);
void grpc_l_handle_attr_finalize_member(grpc_handle_attr_t_np *);
grpc_error_t grpc_l_handle_attr_copy(
    grpc_handle_attr_t_np *, grpc_handle_attr_t_np *);

void grpc_l_executable_handle_convert(
    grpc_l_executable_handle_t *, grpc_l_handle_type_t,
    grpc_function_handle_t *, grpc_object_handle_t_np *, int i);
grpc_error_t grpc_l_executable_handle_convert_with_check(
    grpc_l_executable_handle_t *, grpc_l_handle_type_t,
    grpc_function_handle_t *, grpc_object_handle_t_np *);

/* Session invoke and wait internal API */
grpc_error_t grpc_l_session_start(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, char *, grpc_l_argument_type_t, va_list,
    grpc_arg_stack_t *, grpc_l_call_wait_type_t, grpc_sessionid_t *,
    grpc_l_session_attr_type_t, grpc_session_attr_t_np *);
static grpc_error_t grpc_l_session_set_not_use(
    ngclSession_t *, grpc_l_session_data_t *);
grpc_error_t grpc_l_wait(
    const char *, grpc_l_wait_type_t, grpc_l_wait_set_t,
    grpc_sessionid_t *, size_t,
    ngclExecutable_t *, ngclSession_t *, grpc_sessionid_t *);
static grpc_error_t grpc_l_wait_all(ngclExecutable_t *, ngclSession_t *);

/* Session cancel internal API */
grpc_error_t grpc_l_cancel(const char *, grpc_sessionid_t *, size_t);

/* Ninf-G Executable management internal API */
ngclExecutable_t *grpc_l_executable_get(ngclSession_t *, grpc_error_t *);

/* Ninf-G Session management internal API */
ngclSession_t *grpc_l_session_get_api_list(
    grpc_sessionid_t *, size_t, grpc_error_t *);
grpc_error_t grpc_l_session_release_api_list(ngclSession_t *);
ngclSession_t *grpc_l_session_get_cancel_list(
    grpc_sessionid_t *, size_t, grpc_error_t *);
grpc_error_t grpc_l_session_release_cancel_list(ngclSession_t *);
grpc_error_t grpc_l_session_info_get(
    grpc_l_session_info_get_mode_t,
    grpc_l_session_info_require_t,
    grpc_sessionid_t,
    grpc_session_info_t_np *,
    int *);
grpc_error_t grpc_l_session_info_get_from_session(
    ngclSession_t *,
    grpc_session_info_t_np *);
static void grpc_l_session_info_copy(
    ngclSession_t *,
    grpc_session_info_t_np *);
static void grpc_l_session_info_copy_element(
    ngCompressionInformationElement_t *,
    ngCompressionInformationElement_t *,
    grpc_compression_info_t_np *);

/* Session Attribute internal API */
grpc_error_t grpc_l_session_attr_initialize(
    const char *, grpc_function_handle_t *, grpc_object_handle_t_np *,
    grpc_l_handle_type_t, grpc_session_attr_t_np *);

/* Session queue management internal API */
grpc_error_t grpc_l_session_queue_initialize(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_finalize(grpc_l_session_queue_t *);
void grpc_l_session_queue_initialize_member(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_read_lock(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_read_unlock(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_write_lock(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_write_unlock(grpc_l_session_queue_t *);
grpc_error_t grpc_l_session_queue_set_threshold(
    grpc_l_session_queue_t *, int);
grpc_error_t grpc_l_session_queue_maintain_threshold(
    grpc_l_session_queue_t *, int);
grpc_error_t grpc_l_session_queue_register_session_list(
    grpc_l_session_queue_t *, ngclSession_t *);
static grpc_error_t
grpc_l_session_queue_register_session_list_without_unregister(
    grpc_l_session_queue_t *, ngclSession_t *);
grpc_error_t grpc_l_session_queue_unregister_session(
    grpc_l_session_queue_t *, int, grpc_l_session_queue_unregister_mode_t,
    int, grpc_sessionid_t);
ngclSession_t *grpc_l_session_queue_get_session(
    grpc_l_session_queue_t *, grpc_l_session_queue_get_mode_t,
    grpc_sessionid_t);
ngclSession_t *grpc_l_session_queue_get_next_session(
    grpc_l_session_queue_t *, ngclSession_t *);

/* Error management internal API */
grpc_error_t grpc_l_error_set(grpc_error_t);
void grpc_l_print_error(char *, char *, grpc_error_t);

/* Utility */
void *grpc_l_calloc(size_t, size_t);
void grpc_l_free(void *);

#endif /* _GRPCINTERNAL_H_ */
