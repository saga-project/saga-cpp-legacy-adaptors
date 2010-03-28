#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: grpc.c,v $ $Revision: 1.147 $ $Date: 2007/11/21 06:35:53 $";
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
 * This file define Grid RPC API.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "grpc.h"
#include "grpcLocal.h"
#include "grpc_error.h"

#ifdef NG_OS_OSF5_1
#define GRPC_I_DECLARE_EMPTY_VALIST va_list ap;
#define AP ap
#else /* NG_OS_OSF5_1 */
#define GRPC_I_DECLARE_EMPTY_VALIST
#define AP NULL
#endif

/**
 * Grid RPC API manager.
 */
static grpc_l_manager_t grpc_l_manager;

/**
 * Error message
 */
static char *grpc_l_error_message[] = {
    "No error",
    "GRPC client not initialized yet",
    "The function is called more than once",
    "Specified configuration file not found",
    "Specified configuration file Error",
    "GRPC client cannot find any server",
    "GRPC client cannot find the function",
    "Function handle is not valid",
    "Session ID is not valid",
    "RPC invocation refused by the server",
    "Communication with the server failed",
    "The specified session failed",
    "Call has not completed",
    "No calls have completed",
    "Object handle is not valid",
    "GRPC client cannot find the class",
    "Timeout",
    "Session canceled",
    "Internal error detected",
    "Unknown Error code",
    "Highest numerical error code"
};

static ngclContext_t *grpc_l_context = NULL; /* Ninf-G Context */
static grpc_l_session_queue_t grpc_l_session_queue;
static grpc_l_error_t grpc_l_last_error;
static int grpc_l_initialized = 0;

/**
 * Initialize
 */
grpc_error_t
grpc_initialize(
    char *config_file_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t ret = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_initialize";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_set_initialize();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to initialize.\n", fName);
        return error_code;
    }

    /* Ninf-G Context Construct */
    grpc_l_context = ngclContextConstruct(config_file_name, &error);
    if (grpc_l_context == NULL) {
        error_code = grpc_i_get_error_from_ng_error(error);

        /* Non blocking set error for failed to Ninf-G Context construct */
        grpc_l_last_error.ge_code = error_code;
        grpc_l_last_error.ge_message
            = grpc_l_error_message[grpc_l_last_error.ge_code];

        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to construct the Ninf-G Context.\n", fName);
        goto error;
    }

    /* Initialize the Exclusive lock */
    result = ngclExclusiveLockInitialize(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Exclusive lock.\n", fName);
        goto error;
    }

    /* Initialize the RWlock */
    result = ngclRWlockInitialize(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Read/Write Lock.\n", fName);
        goto error;
    }

    /* Initialize the Session Queue */
    error_code = grpc_l_session_queue_initialize(&grpc_l_session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to initialize the Session Queue.\n",
            fName);
        goto error;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, fName);

    return error_code;

error:
    /* Unset flag of initialize. */
    ret = grpc_l_unset_initialize();
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unset initialize.\n",
            fName);
        return ret;
    }

    return error_code;
}

/**
 * Finalize
 */
grpc_error_t
grpc_finalize(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_finalize";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, fName);

    /* Finalize the Session Queue */
    error_code = grpc_l_session_queue_finalize(&grpc_l_session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to destruct the Session Queue.\n", fName);
        return error_code;
    }

    /* Finalize the RWlock */
    result = ngclRWlockFinalize(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Read/Write Lock.\n", fName);
        return error_code;
    }

    /* Finalize the RWlock */
    result = ngclExclusiveLockFinalize(
        grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Exclusive lock.\n", fName);
        return error_code;
    }

    /* Ninf-G Context destruct */
    result = ngclContextDestruct(grpc_l_context, &error);
    grpc_l_context = NULL;
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);

        /* Non blocking set error for failed to Ninf-G Context construct */
        grpc_l_last_error.ge_code = error_code;
        grpc_l_last_error.ge_message
            = grpc_l_error_message[grpc_l_last_error.ge_code];

        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to destruct the Ninf-G context.\n", fName);
        return error_code;
    }

    /* Unset flag of initialize. */
    error_code = grpc_l_unset_initialize();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unset initialize.\n",
            fName);
        return error_code;
    }

    return error_code;
}

/**
 * Configuration file read.
 */
grpc_error_t
grpc_config_file_read_np(
    char *config_file_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_config_file_read_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, fName);

    /* Read the configuration file */
    result = ngclContextConfigurationFileRead(
        grpc_l_context, config_file_name, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to read the Ninf-G Configuration file.\n", fName);
        return error_code;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, fName);

    return error_code;
}

/**
 * Function handle initialize.
 */
grpc_error_t
grpc_function_handle_init(
    grpc_function_handle_t *handle,
    char *server_name,
    char *func_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_init";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        1, server_name, func_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Function handle initialize with handle attribute.
 */
grpc_error_t
grpc_function_handle_init_with_attr_np(
    grpc_function_handle_t *handle,
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_init_with_attr_np";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_GIVEN,
        1, NULL, NULL, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Function handle initialize, use default.
 */
grpc_error_t
grpc_function_handle_default(
    grpc_function_handle_t *handle,
    char *func_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_default";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        1, NULL, func_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Function handle destruct.
 */
grpc_error_t
grpc_function_handle_destruct(
    grpc_function_handle_t *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_destruct";

    error_code = grpc_l_handle_destruct(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, 1);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle destruct.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Function handle initialize.
 */
grpc_error_t
grpc_function_handle_array_init_np(
    grpc_function_handle_t *handle,
    size_t length,
    char *server_name,
    char *func_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_array_init_np";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        length, server_name, func_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Function handle initialize with attribute.
 */
grpc_error_t
grpc_function_handle_array_init_with_attr_np(
    grpc_function_handle_t *handle,
    size_t length,
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_array_init_with_attr_np";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_GIVEN,
        length, NULL, NULL, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Function handle initialize, use default.
 */
grpc_error_t
grpc_function_handle_array_default_np(
    grpc_function_handle_t *handle,
    size_t length,
    char *func_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_array_default_np";

    error_code = grpc_l_handle_init(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        GRPC_L_HANDLE_USE_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        length, NULL, func_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Function handle destruct.
 */
grpc_error_t
grpc_function_handle_array_destruct_np(
    grpc_function_handle_t *handle,
    size_t length)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_array_destruct_np";

    error_code = grpc_l_handle_destruct(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, length);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle destruct.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get attribute of function handle.
 */
grpc_error_t
grpc_function_handle_get_attr_np(
    grpc_function_handle_t *handle,
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_get_attr_np";

    /* Get the attribute */
    error_code = grpc_l_handle_get_attr(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get handle attribute.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get Function handle from Session ID (Non portal)
 */
grpc_error_t
grpc_function_handle_get_from_session_np(
    grpc_function_handle_t **handle,
    grpc_sessionid_t sessionId)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_get_from_session_np";

    /* Get the function handle */
    error_code = grpc_l_handle_get(
        fName, sessionId, GRPC_L_HANDLE_TYPE_FUNCTION, handle, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get function handle.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get Function handle from Session ID.
 */
grpc_error_t
grpc_get_handle(
    grpc_function_handle_t **handle,
    grpc_sessionid_t sessionId)
{
    return grpc_function_handle_get_from_session_np(handle, sessionId);
}

/**
 * Print error message associated with error generated by Function handle
 */
grpc_error_t
grpc_function_handle_perror_np(
    grpc_function_handle_t *handle,
    char *str)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_perror_np";

    /* perror */
    error_code = grpc_l_handle_perror(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, str);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to print the error.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get error code associated with error generated by Function handle
 */
grpc_error_t
grpc_function_handle_get_error_np(
    grpc_function_handle_t *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_get_error_np";

    /* Get error */
    error_code = grpc_l_handle_get_error(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION);

    return error_code;
}

/**
 * Is the function handle ready?
 */
grpc_error_t
grpc_function_handle_is_ready_np(
    grpc_function_handle_t *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_function_handle_is_ready_np";

    /* Is ready */
    error_code = grpc_l_handle_is_ready(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION);

    return error_code;
}

/**
 * Object handle initialize.
 */
grpc_error_t
grpc_object_handle_init_np(
    grpc_object_handle_t_np *handle,
    char *server_name,
    char *class_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_init_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        1, server_name, class_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Object handle initialize with attribute.
 */
grpc_error_t
grpc_object_handle_init_with_attr_np(
    grpc_object_handle_t_np *handle,
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_init_with_attr_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_GIVEN,
        1, NULL, NULL, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Object handle initialize, use default.
 */
grpc_error_t
grpc_object_handle_default_np(
    grpc_object_handle_t_np *handle,
    char *class_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_default_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        1, NULL, class_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Object handle destruct.
 */
grpc_error_t
grpc_object_handle_destruct_np(
    grpc_object_handle_t_np *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_destruct_np";

    error_code = grpc_l_handle_destruct(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, 1);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle destruct.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Function handle initialize.
 */
grpc_error_t
grpc_object_handle_array_init_np(
    grpc_object_handle_t_np *handle,
    size_t length,
    char *server_name,
    char *class_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_array_init_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        length, server_name, class_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Object handle initialize with attribute.
 */
grpc_error_t
grpc_object_handle_array_init_with_attr_np(
    grpc_object_handle_t_np *handle,
    size_t length,
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_array_init_with_attr_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_NO_DEFAULT, GRPC_L_HANDLE_ATTR_GIVEN,
        length, NULL, NULL, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Object handle initialize, use default.
 */
grpc_error_t
grpc_object_handle_array_default_np(
    grpc_object_handle_t_np *handle,
    size_t length,
    char *class_name)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_array_default_np";

    error_code = grpc_l_handle_init(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        GRPC_L_HANDLE_USE_DEFAULT, GRPC_L_HANDLE_ATTR_NOT_GIVEN,
        length, NULL, class_name, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle init.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Array of Object handle destruct.
 */
grpc_error_t
grpc_object_handle_array_destruct_np(
    grpc_object_handle_t_np *handle,
    size_t length)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_array_destruct_np";

    error_code = grpc_l_handle_destruct(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, length);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to handle destruct.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get attribute of Object handle.
 */
grpc_error_t
grpc_object_handle_get_attr_np(
    grpc_object_handle_t_np *handle,
    grpc_handle_attr_t_np  *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_get_attr_np";

    /* Get the attribute */
    error_code = grpc_l_handle_get_attr(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get handle attribute.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get Object handle from Session ID.
 */
grpc_error_t
grpc_object_handle_get_from_session_np(
    grpc_object_handle_t_np **handle,
    grpc_sessionid_t sessionId)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_get_from_session_np";

    /* Get the object handle */
    error_code = grpc_l_handle_get(
        fName, sessionId, GRPC_L_HANDLE_TYPE_OBJECT, NULL, handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get function handle.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Print error message associated with error generated by Object handle
 */
grpc_error_t
grpc_object_handle_perror_np(
    grpc_object_handle_t_np *handle,
    char *str)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_perror_np";

    /* perror */
    error_code = grpc_l_handle_perror(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, str);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to print the error.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Get error code associated with error generated by Object handle
 */
grpc_error_t
grpc_object_handle_get_error_np(
    grpc_object_handle_t_np *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_get_error_np";

    /* Get error */
    error_code = grpc_l_handle_get_error(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT);

    return error_code;
}

/**
 * Is the object handle ready?
 */
int
grpc_object_handle_is_ready_np(
    grpc_object_handle_t_np *handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_object_handle_is_ready_np";

    /* Is ready */
    error_code = grpc_l_handle_is_ready(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT);

    return error_code;
}

/**
 * Handle attribute initialize.
 */
grpc_error_t
grpc_handle_attr_initialize_np(
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    ngclExecutableAttribute_t execAttr;
    int error, result;
    static const char fName[] = "grpc_handle_attr_initialize_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    /* NULL clear */
    grpc_l_handle_attr_initialize_member(attr);

    /* Get the Attribute of Ninf-G Executable */
    result = ngclExecutableAttributeInitialize(grpc_l_context,
        &execAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Get Attribute of Ninf-G Executable.\n", fName);
        return error_code;
    }

    attr->gha_remotePortNo    = execAttr.ngea_portNo;
    attr->gha_jobManager      = NULL; /* execAttr.ngea_jobManager == NULL */
    attr->gha_subject         = NULL; /* execAttr.ngea_subject == NULL */
    attr->gha_jobStartTimeout = execAttr.ngea_jobStartTimeout;
    attr->gha_jobStopTimeout  = execAttr.ngea_jobStopTimeout;
    attr->gha_waitArgTransfer = execAttr.ngea_waitArgumentTransfer;
    attr->gha_queueName       = NULL; /* execAttr.ngea_queueName == NULL */
    attr->gha_mpiNcpus        = execAttr.ngea_mpiNcpus;

    /* Release Attribute of Ninf-G Executable */
    result = ngclExecutableAttributeFinalize(grpc_l_context, &execAttr,
        &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Release Attribute of Ninf-G Executable.\n", fName);
        return error_code;
    }

    return error_code;
}

/**
 * Handle attribute destruct.
 */
grpc_error_t
grpc_handle_attr_destruct_np(
    grpc_handle_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_handle_attr_destruct_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    grpc_l_handle_attr_finalize_member(attr);

    return error_code;
}

/**
 *  Get value of handle attribute.
 */
grpc_error_t
grpc_handle_attr_get_np(
    grpc_handle_attr_t_np *attr,
    grpc_handle_attr_name_t_np attr_name,
    void **attr_value)
{
    void *value = NULL;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_handle_attr_get_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute value is not valid.\n", fName);
        return error_code;
    }

    *attr_value = NULL;

    /* check attribute name */
    switch (attr_name) {
    case GRPC_HANDLE_ATTR_HOSTNAME:
        if (attr->gha_remoteHostName != NULL) {
            value = (void *)strdup(attr->gha_remoteHostName);
            if (value == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for host name.\n",
                    fName);
                return error_code;
            }
        } else {
            value = NULL;
        }
        break;

    case GRPC_HANDLE_ATTR_PORT:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for handle attribute.\n",
                fName);
            return error_code;
        }
        /* Set remote host port number */
        *((int *)value) = attr->gha_remotePortNo;
        break;

    case GRPC_HANDLE_ATTR_JOBMANAGER:
        if (attr->gha_jobManager != NULL) {
            value = (void *)strdup(attr->gha_jobManager);
            if (value == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for jobmanager.\n",
                    fName);
                return error_code;
            }
        } else {
            value = NULL;
        }
        break;

    case GRPC_HANDLE_ATTR_SUBJECT:
        if (attr->gha_subject != NULL) {
            value = (void *)strdup(attr->gha_subject);
            if (value == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for Subject.\n",
                    fName);
                return error_code;
            }
        } else {
            value = NULL;
        }
        break;

    case GRPC_HANDLE_ATTR_FUNCNAME:
        if (attr->gha_functionName != NULL) {
            value = (void *)strdup(attr->gha_functionName);
            if (value == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for func name.\n",
                    fName);
                return error_code;
            }
        } else {
            value = NULL;
        }
        break;

    case GRPC_HANDLE_ATTR_JOBSTARTTIMEOUT:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for handle attribute.\n",
                fName);
            return error_code;
        }
        /* Set timeout of Ninf-G Executable start */
        *((int *)value) = attr->gha_jobStartTimeout;
        break;

    case GRPC_HANDLE_ATTR_JOBSTOPTIMEOUT:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for handle attribute.\n",
                fName);
            return error_code;
        }
        /* Set timeout of Ninf-G Executable stop */
        if (attr->gha_jobStopTimeout == 
            /* See grpc_handle_attr_set_np(). */
            NGCL_EXECUTABLE_ATTRIBUTE_JOB_STOP_TIMEOUT_WAIT_FOREVER) {
            *((int *)value) = -1;
        } else {
            *((int *)value) = attr->gha_jobStopTimeout;
        }
        break;

    case GRPC_HANDLE_ATTR_WAIT_ARG_TRANSFER:
        value = globus_libc_calloc(1, sizeof(grpc_argument_transfer_t_np));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for handle attribute.\n",
                fName);
            return error_code;
        }
        /* Copy flag of wait for the end of argument transfer or not */
        *((grpc_argument_transfer_t_np *)value) = attr->gha_waitArgTransfer;
        break;

    case GRPC_HANDLE_ATTR_QUEUENAME:
        if (attr->gha_queueName != NULL) {
            value = (void *)strdup(attr->gha_queueName);
            if (value == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't allocate the storage for queue name.\n",
                    fName);
                return error_code;
            }
        } else {
            value = NULL;
        }
        break;

    case GRPC_HANDLE_ATTR_MPI_NCPUS:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for handle attribute.\n",
                fName);
            return error_code;
        }
        /* Set timeout of MPI Number of CPUs */
        *((int *)value) = attr->gha_mpiNcpus;
        break;

    default:
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value of Handle attribute name is not valid.\n", fName);
        return error_code;
    }

    *attr_value = value;

    return error_code;
}

/**
 * Handle attribute set value.
 */
grpc_error_t
grpc_handle_attr_set_np(
    grpc_handle_attr_t_np *attr,
    grpc_handle_attr_name_t_np attr_name,
    void *attr_value)
{
    int given_value_int;
    grpc_argument_transfer_t_np given_value_transfer;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_handle_attr_set_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value for Handle attribute is not valid.\n", fName);
        return error_code;
    }

    switch (attr_name) {
    case GRPC_HANDLE_ATTR_HOSTNAME:
        /* Release the previous value */
        if (attr->gha_remoteHostName != NULL) {
            globus_libc_free(attr->gha_remoteHostName);
            attr->gha_remoteHostName = NULL;
        }

        /* Copy remote host name */
        attr->gha_remoteHostName = strdup((char *)attr_value);
        if (attr->gha_remoteHostName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for remote host name.\n",
                fName);
            return error_code;
        }
        break;

    case GRPC_HANDLE_ATTR_PORT:
        given_value_int = *((int *)attr_value);

        /* Check the value */
        if (given_value_int < 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The value for handle attribute port"
                " is too small (%d).\n",
                fName, given_value_int);
            return error_code;
        }

        /* Copy remote host port number */
        attr->gha_remotePortNo = given_value_int;
        break;

    case GRPC_HANDLE_ATTR_JOBMANAGER:
        /* Release the previous value */
        if (attr->gha_jobManager != NULL) {
            globus_libc_free(attr->gha_jobManager);
            attr->gha_jobManager = NULL;
        }

        /* Copy jobmanager */
        attr->gha_jobManager = strdup((char *)attr_value);
        if (attr->gha_jobManager == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for jobmanager.\n", fName);
            return error_code;
        }
        break;

    case GRPC_HANDLE_ATTR_SUBJECT:
        /* Release the previous value */
        if (attr->gha_subject != NULL) {
            globus_libc_free(attr->gha_subject);
            attr->gha_subject = NULL;
        }

        /* Copy subject */
        attr->gha_subject = strdup((char *)attr_value);
        if (attr->gha_subject == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Subject.\n", fName);
            return error_code;
        }
        break;

    case GRPC_HANDLE_ATTR_FUNCNAME:
        /* Release the previous value */
        if (attr->gha_functionName != NULL) {
            globus_libc_free(attr->gha_functionName);
            attr->gha_functionName = NULL;
        }

        /* Copy function name */
        attr->gha_functionName = strdup((char *)attr_value);
        if (attr->gha_functionName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for function name.\n", fName);
            return error_code;
        }
        break;

    case GRPC_HANDLE_ATTR_JOBSTARTTIMEOUT:
        given_value_int = *((int *)attr_value);

        /* Check the value */
        if (given_value_int < 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The value for handle attribute job start timeout"
                " is too small (%d).\n",
                fName, given_value_int);
            return error_code;
        }

        /* Copy timeout of Ninf-G Executable start */
        attr->gha_jobStartTimeout = given_value_int;
        break;

    case GRPC_HANDLE_ATTR_JOBSTOPTIMEOUT:
        given_value_int = *((int *)attr_value);

        /* Check the value */
        if (given_value_int < -1) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The value for handle attribute job stop timeout"
                " is too small (%d).\n",
                fName, given_value_int);
            return error_code;
        }

        if (given_value_int < 0) {
            /* -1 is used as NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED.
             * Thus NGCL_EXECUTABLE_ATTRIBUTE_JOB_STOP_TIMEOUT_WAIT_FOREVER
             * is set to job stop timeout attribute.
             */
            attr->gha_jobStopTimeout =
                NGCL_EXECUTABLE_ATTRIBUTE_JOB_STOP_TIMEOUT_WAIT_FOREVER;
        } else {
            /* Copy timeout of Ninf-G Executable stop */
            attr->gha_jobStopTimeout = given_value_int;
        }
        break;

    case GRPC_HANDLE_ATTR_WAIT_ARG_TRANSFER:
        given_value_transfer =
            *((grpc_argument_transfer_t_np *)attr_value);

        /* Check the value */
        if (!((given_value_transfer == GRPC_ARGUMENT_TRANSFER_WAIT) ||
            (given_value_transfer == GRPC_ARGUMENT_TRANSFER_NOWAIT) ||
            (given_value_transfer == GRPC_ARGUMENT_TRANSFER_COPY))) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Invalid value for handle attribute argument transfer"
                " (%d).\n",
                fName, (int)given_value_transfer);
            return error_code;
        }

        /* Copy flag of wait for the end of argument transfer or not */
        attr->gha_waitArgTransfer = given_value_transfer;
        break;

    case GRPC_HANDLE_ATTR_QUEUENAME:
        /* Release the previous value */
        if (attr->gha_queueName != NULL) {
            globus_libc_free(attr->gha_queueName);
            attr->gha_queueName = NULL;
        }

        /* Copy queue name */
        attr->gha_queueName = strdup((char *)attr_value);
        if (attr->gha_queueName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for queue name.\n",
                fName);
            return error_code;
        }
        break;

    case GRPC_HANDLE_ATTR_MPI_NCPUS:
        given_value_int = *((int *)attr_value);

        /* Check the value */
        if (given_value_int < 1) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The value for handle attribute MPI number of cpus"
                " is too small (%d).\n",
                fName, given_value_int);
            return error_code;
        }

        /* Copy MPI Number of CPUs */
        attr->gha_mpiNcpus = given_value_int;
        break;

    default:
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value of Handle attribute name is not valid.\n", fName);
        return error_code;
    }

    return error_code;
}

/**
 * Handle attribute release.
 */
grpc_error_t
grpc_handle_attr_release_np(
    void *attr_value)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_handle_attr_release_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value for Handle attribute is not valid.\n", fName);
        return error_code;
    }

    globus_libc_free(attr_value);

    return error_code;
}

/**
 * GridRPC call
 */
grpc_error_t
grpc_call(
    grpc_function_handle_t *handle,
    ...)
{
    va_list ap;
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call";

    /* Start the session */
    va_start(ap, handle);
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC asynchronous call
 */
grpc_error_t
grpc_call_async(
    grpc_function_handle_t *handle,
    grpc_sessionid_t *sessionId,
    ...)
{
    va_list ap;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_async";

    /* Start the session */
    va_start(ap, sessionId);
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, sessionId,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC call use argument stack
 */
int
grpc_call_arg_stack(
    grpc_function_handle_t *handle,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_arg_stack";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * GridRPC asynchronous call use argument stack
 */
int
grpc_call_arg_stack_async(
    grpc_function_handle_t *handle,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_arg_stack_async";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return session_id;
}

/**
 * GridRPC call with Session Attribute
 */
grpc_error_t
grpc_call_with_attr_np(
    grpc_function_handle_t *handle,
    grpc_session_attr_t_np *session_attr,
    ...)
{
    va_list ap;
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_with_attr_np";

    /* Start the session */
    va_start(ap, session_attr);
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC asynchronous call with Session Attribute
 */
grpc_error_t
grpc_call_async_with_attr_np(
    grpc_function_handle_t *handle,
    grpc_sessionid_t *sessionId,
    grpc_session_attr_t_np *session_attr,
    ...)
{
    va_list ap;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_async_with_attr_np";

    /* Start the session */
    va_start(ap, session_attr);
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, sessionId,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC call use argument stack with Session Attribute
 */
int
grpc_call_arg_stack_with_attr_np(
    grpc_function_handle_t *handle,
    grpc_session_attr_t_np *session_attr,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_arg_stack_with_attr_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * GridRPC asynchronous call use argument stack with Session Attribute
 */
int
grpc_call_arg_stack_async_with_attr_np(
    grpc_function_handle_t *handle,
    grpc_session_attr_t_np *session_attr,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_call_arg_stack_async_with_attr_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION, NULL,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return session_id;
}

/**
 * GridRPC invoke
 */
grpc_error_t
grpc_invoke_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    ...)
{
    va_list ap;
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_np";

    /* Start the session */
    va_start(ap, method_name);
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC asynchronous invoke
 */
grpc_error_t
grpc_invoke_async_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_sessionid_t *sessionId,
    ...)
{
    va_list ap;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_async_np";

    /* Start the session */
    va_start(ap, sessionId);
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, sessionId,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC invoke use argument stack
 */
int
grpc_invoke_arg_stack_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_arg_stack_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * GridRPC asynchronous invoke use argument stack
 */
int
grpc_invoke_arg_stack_async_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_arg_stack_async_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, &session_id,
        GRPC_L_SESSION_ATTR_NOT_GIVEN, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return session_id;
}

/**
 * GridRPC invoke with Session Attribute
 */
grpc_error_t
grpc_invoke_with_attr_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_session_attr_t_np *session_attr,
    ...)
{
    va_list ap;
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_with_attr_np";

    /* Start the session */
    va_start(ap, session_attr);
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC asynchronous invoke with Session Attribute
 */
grpc_error_t
grpc_invoke_async_with_attr_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_sessionid_t *sessionId,
    grpc_session_attr_t_np *session_attr,
    ...)
{
    va_list ap;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_async_with_attr_np";

    /* Start the session */
    va_start(ap, session_attr);
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_VA_LIST, ap, NULL,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, sessionId,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    va_end(ap);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * GridRPC invoke use argument stack with Session Attribute
 */
int
grpc_invoke_arg_stack_with_attr_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_session_attr_t_np *session_attr,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_arg_stack_with_attr_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_SYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * GridRPC asynchronous invoke use argument stack with Session Attribute
 */
int
grpc_invoke_arg_stack_async_with_attr_np(
    grpc_object_handle_t_np *handle,
    char *method_name,
    grpc_session_attr_t_np *session_attr,
    grpc_arg_stack_t *stack)
{
    GRPC_I_DECLARE_EMPTY_VALIST
    grpc_sessionid_t session_id;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_invoke_arg_stack_async_with_attr_np";

    /* Start the session */
    error_code = grpc_l_session_start(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT, method_name,
        GRPC_L_ARGUMENT_TYPE_ARG_STACK, AP, stack,
        GRPC_L_CALL_WAIT_TYPE_ASYNC, &session_id,
        GRPC_L_SESSION_ATTR_GIVEN, session_attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);
        return -1;
    }

    /* Success */
    return session_id;
}

/**
 * Session attribute initialize.
 */
grpc_error_t
grpc_session_attr_initialize_np(
    grpc_function_handle_t *handle,
    grpc_session_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_initialize_np";

    /* Initialize the Session Attribute */
    error_code = grpc_l_session_attr_initialize(
        fName, handle, NULL, GRPC_L_HANDLE_TYPE_FUNCTION,
        attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to initialize the Session Attribute.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session attribute initialize.
 */
grpc_error_t
grpc_session_attr_initialize_with_object_handle_np(
    grpc_object_handle_t_np *handle,
    grpc_session_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_initialize_with_object_handle_np";

    /* Initialize the Session Attribute */
    error_code = grpc_l_session_attr_initialize(
        fName, NULL, handle, GRPC_L_HANDLE_TYPE_OBJECT,
        attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to initialize the Session Attribute.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session attribute destruct.
 */
grpc_error_t
grpc_session_attr_destruct_np(
    grpc_session_attr_t_np *attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_destruct_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session attribute is not valid.\n", fName);
        return error_code;
    }

    /* release alloced area here */

    return error_code;
}

/**
 *  Get value of session attribute.
 */
grpc_error_t
grpc_session_attr_get_np(
    grpc_session_attr_t_np *attr,
    grpc_session_attr_name_t_np attr_name,
    void **attr_value)
{
    void *value = NULL;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_get_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session attribute is not valid.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session attribute value is not valid.\n", fName);
        return error_code;
    }

    /* check attribute name */
    switch (attr_name) {
    case GRPC_SESSION_ATTR_WAIT_ARG_TRANSFER:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for session attribute.\n",
                fName);
            return error_code;
        }
        /* Copy flag of wait for the end of argument transfer or not */
        *((int *)value) = attr->gsa_waitArgTransfer;
        break;

    case GRPC_SESSION_ATTR_SESSION_TIMEOUT:
        value = globus_libc_calloc(1, sizeof(int));
        if (value == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for session attribute.\n",
                fName);
            return error_code;
        }
        /* Copy flag of session timeout */
        *((int *)value) = attr->gsa_timeout;
        break;

    default:
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value of Session attribute name is not valid.\n", fName);
        return error_code;
    }

    *attr_value = value;

    return error_code;
}

/**
 * Session attribute set value.
 */
grpc_error_t
grpc_session_attr_set_np(
    grpc_session_attr_t_np *attr,
    grpc_session_attr_name_t_np attr_name,
    void *attr_value)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_set_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session attribute is not valid.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value for Session attribute is not valid.\n", fName);
        return error_code;
    }

    /* check attribute name */
    switch (attr_name) {
    case GRPC_SESSION_ATTR_WAIT_ARG_TRANSFER:
        /* Copy flag of wait for the end of argument transfer or not */
        attr->gsa_waitArgTransfer =
            *((grpc_argument_transfer_t_np *)attr_value);
        break;

    case GRPC_SESSION_ATTR_SESSION_TIMEOUT:
        /* Copy flag of session timeout */
        attr->gsa_timeout = *((int *)attr_value);
        break;

    default:
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Value of Session attribute name is not valid.\n", fName);
        return error_code;
    }

    return error_code;
}

/**
 *  Session attribute release.
 */
grpc_error_t
grpc_session_attr_release_np(
    void *attr_value)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_attr_release_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (attr_value == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session attribute is not valid.\n", fName);
        return error_code;
    }

    globus_libc_free(attr_value);

    return error_code;
}

/**
 * Argument Stack create.
 */
grpc_arg_stack_t *
grpc_arg_stack_new(
    int maxsize)
{
    ngclArgumentStack_t *stack = NULL;
    int error, result;
    static const char fName[] = "grpc_arg_stack_new";

    /* Check whether initialize was already performed. */
    result = grpc_l_initialize_performed_check();
    if (result != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return NULL;
    }

    /* Argument stack construct by Pure API */
    stack = ngclArgumentStackConstruct(grpc_l_context, maxsize, &error);
    if (stack == NULL) {
        result = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(result);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to construct the Argument Stack.\n", fName);
        return NULL;
    }

    /* Success */
    return stack;
}

/**
 * Argument Stack destruct.
 */
int
grpc_arg_stack_destruct(
    grpc_arg_stack_t *stack)
{
    int error, result;
    static const char fName[] = "grpc_arg_stack_destruct";

    /* Check whether initialize was already performed. */
    result = grpc_l_initialize_performed_check();
    if (result != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return -1;
    }

    /* Check the arguments */
    if (stack == NULL) {
        grpc_l_error_set(GRPC_OTHER_ERROR_CODE);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument stack is not valid.\n", fName);
        return -1;
    }

    /* Argument stack destruct by Pure API */
    result = ngclArgumentStackDestruct(stack, &error);
    if (result == 0) {
        result = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(result);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to destruct the argument Stack.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * Push argument onto stack.
 */
int
grpc_arg_stack_push_arg(
    grpc_arg_stack_t *stack,
    void *arg)
{
    int error, result;
    static const char fName[] = "grpc_arg_stack_push_arg";

    /* Check whether initialize was already performed. */
    result = grpc_l_initialize_performed_check();
    if (result != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return -1;
    }

    /* Check the arguments */
    if (stack == NULL) {
        grpc_l_error_set(GRPC_OTHER_ERROR_CODE);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument stack is not valid.\n", fName);
        return -1;
    }

#if 0 /* 2004/9/8 asou
       * Is this necessary? Does anybody specify a NULL argument?
       */
    /* Check the arguments */
    if (arg == NULL) {
        grpc_l_error_set(GRPCERR_INVALID_ARGUMENT);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument is not valid.\n", fName);
        return -1;
    }
#endif

    /* Push argument onto stack by Pure API */
    result = ngclArgumentStackPush(stack, arg, &error);
    if (result == 0) {
        result = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(result);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to push onto Argument Stack.\n", fName);
        return -1;
    }

    /* Success */
    return 0;
}

/**
 * Remove top argument from stack.
 */
void *
grpc_arg_stack_pop_arg(
    grpc_arg_stack_t *stack)
{
    void *arg = NULL;
    int error, result;
    static const char fName[] = "grpc_arg_stack_pop_arg";

    /* Check whether initialize was already performed. */
    result = grpc_l_initialize_performed_check();
    if (result != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return NULL;
    }

    /* Check the arguments */
    if (stack == NULL) {
        grpc_l_error_set(GRPC_OTHER_ERROR_CODE);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument stack is not valid.\n", fName);
        return NULL;
    }

    /* Remove top argument from stack by Pure API */
    arg = ngclArgumentStackPop(((ngclArgumentStack_t *)stack), &error);
#if 0 /* 2004/9/8 asou
       * Is this necessary? Does anybody specify a NULL argument?
       */
    if (arg == NULL) {
        result = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(result);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to pop from Argument Stack.\n", fName);
        return NULL;
    }
#endif

    /* Success */
    return arg;
}

/**
 * Wait Session complete
 */
grpc_error_t
grpc_wait(
    grpc_sessionid_t sessionId)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_wait";

    error_code = grpc_l_wait(
        fName, GRPC_L_WAIT_TYPE_ALL, GRPC_L_WAIT_SET_GIVEN,
        &sessionId, 1, NULL, NULL, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Wait all Session complete
 */
grpc_error_t
grpc_wait_all(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_wait_all";

    error_code = grpc_l_wait(
        fName, GRPC_L_WAIT_TYPE_ALL, GRPC_L_WAIT_SET_ALL,
        NULL, 0, NULL, NULL, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Wait any Session complete
 */
grpc_error_t
grpc_wait_any(
    grpc_sessionid_t *idPtr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_wait_any";

    error_code = grpc_l_wait(
        fName, GRPC_L_WAIT_TYPE_ANY, GRPC_L_WAIT_SET_ALL,
        NULL, 0, NULL, NULL, idPtr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Wait all given Session complete
 */
grpc_error_t
grpc_wait_and(
    grpc_sessionid_t *idArray,
    size_t length)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_wait_and";

    error_code = grpc_l_wait(
        fName, GRPC_L_WAIT_TYPE_ALL, GRPC_L_WAIT_SET_GIVEN,
        idArray, length, NULL, NULL, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Wait any given Session complete
 */
grpc_error_t
grpc_wait_or(
    grpc_sessionid_t *idArray,
    size_t length,
    grpc_sessionid_t *idPtr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_wait_or";

    error_code = grpc_l_wait(
        fName, GRPC_L_WAIT_TYPE_ANY, GRPC_L_WAIT_SET_GIVEN,
        idArray, length, NULL, NULL, idPtr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Cancel Session
 */
grpc_error_t
grpc_cancel(
    grpc_sessionid_t sessionId)
{
    static const char fName[] = "grpc_cancel";

    return grpc_l_cancel(fName, &sessionId, 1);
}

/**
 * Cancel Session
 */
grpc_error_t
grpc_cancel_all(
    void)
{
    static const char fName[] = "grpc_cancel_all";

    return grpc_l_cancel(fName, NULL, 0);
}

/**
 * Probe whether Session has completed
 */
grpc_error_t
grpc_probe(
    grpc_sessionid_t sessionId)
{
    int result;
    ngclSession_t *session;
    grpc_l_session_queue_t *session_queue;
    grpc_error_t ret = GRPC_NO_ERROR;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_probe";

    session_queue = &grpc_l_session_queue;
    session = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Invoke I/O callback (for NonThread) */
    result = ngclNinfgManagerYieldForCallback();
    if (result == 0) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't yield for I/O callback.\n", fName);
        return error_code;
    }

    /* Lock the Session Queue */
    ret = grpc_l_session_queue_read_lock(session_queue);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the Session Queue.\n", fName);
        return ret;
    }

    /* Get the Session from Session ID (Session Queue) */
    session = grpc_l_session_queue_get_session(
        session_queue, GRPC_L_SESSION_QUEUE_GET_BY_ID, sessionId);
    if (session != NULL) {
        if (session->ngs_status == NG_SESSION_STATUS_DONE) {
            error_code = GRPC_NO_ERROR;
        } else {
            error_code = GRPC_NOT_COMPLETED;
        }
    }

    /* Unlock the Session Queue */
    ret = grpc_l_session_queue_read_unlock(session_queue);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the Session Queue.\n", fName);
        return ret;
    }

    if (session != NULL) {
        /* Success */
        return error_code;
    }

    /* Get Session from Session ID by Pure API */
    session = grpc_l_session_get_api_list(&sessionId, 1, &error_code);
    if (session == NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No such Session of Session ID.\n", fName);
        return error_code;
    }

    if (session->ngs_status == NG_SESSION_STATUS_DONE) {
        error_code = GRPC_NO_ERROR;
    } else {
        error_code = GRPC_NOT_COMPLETED;
    }

    /* Release Ninf-G Session */
    ret = grpc_l_session_release_api_list(session);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Ninf-G Session.\n", fName);
        return ret;
    }

    return error_code;
}

/**
 * Probe whether Session has completed
 */
grpc_error_t
grpc_probe_or(
    grpc_sessionid_t *sessionids,
    size_t length,
    grpc_sessionid_t *id)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t result;
    int i;
    static const char fName[] = "grpc_probe_or";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (sessionids == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session IDs is not valid.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (id == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session ID is not valid.\n", fName);
        return error_code;
    }

    /* Invoke I/O callback (for NonThread) */
    result = ngclNinfgManagerYieldForCallback();
    if (result == 0) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't yield for I/O callback.\n", fName);
        return error_code;
    }

    *id = GRPC_SESSIONID_VOID;

    for (i = 0; i < length; i++) {
        result = grpc_probe(sessionids[i]);
        if (result != GRPC_NO_ERROR) {
            if (result != GRPC_NOT_COMPLETED) {
                error_code = result;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to probe session.\n", fName);

                return error_code;
            }
        } else {
            /* Found */
            *id = sessionids[i];
            break;
        }
    }

    if (*id == GRPC_SESSIONID_VOID) {
        /* Not found */
        assert(i == length);
        error_code = GRPC_NONE_COMPLETED;
        grpc_l_error_set(error_code);
    }

    return error_code;
}

/**
 * Get the Session information
 */
grpc_error_t
grpc_session_info_get_np(
    grpc_sessionid_t sessionId,
    grpc_session_info_t_np **info,
    int *status)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_info_get_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Allocate the storage for Session Information */
    if (info != NULL) {
	*info = globus_libc_calloc(1, sizeof(grpc_session_info_t_np));
	if (*info == NULL) {
	    error_code = GRPC_OTHER_ERROR_CODE;
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't allocate the storage for Session Information.\n",
		fName);
	    return error_code;
	}
    }

    /* Get the Session Information */
    error_code = grpc_l_session_info_get(
        GRPC_L_SESSION_INFO_GET_BY_ID,
	GRPC_L_SESSION_INFO_REQUIRE_ALL,
	sessionId, (info == NULL) ? NULL : *info, status);
    if (error_code != GRPC_NO_ERROR) {
	if (info != NULL) {
	    grpc_session_info_release_np(*info);
	    *info = NULL;
	}
    }

    return error_code;
}

/**
 * Release the Session Information.
 */
grpc_error_t
grpc_session_info_release_np(grpc_session_info_t_np *info)
{
    if (info != NULL) {
	if (info->gei_compressionInformation.toClient != NULL)
	    globus_libc_free(info->gei_compressionInformation.toClient);
	info->gei_compressionInformation.toClient = NULL;

	if (info->gei_compressionInformation.toRemote != NULL)
	    globus_libc_free(info->gei_compressionInformation.toRemote);
	info->gei_compressionInformation.toRemote = NULL;

	globus_libc_free(info);
	info = NULL;
    }

    /* Success */
    return GRPC_NO_ERROR;
}

/**
 * Get Session information
 */
grpc_error_t
grpc_get_info_np(
    grpc_sessionid_t sessionId,
    grpc_exec_info_t_np *info,
    int *status)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_get_info_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    error_code = grpc_l_session_info_get(
        GRPC_L_SESSION_INFO_GET_BY_ID,
	GRPC_L_SESSION_INFO_REQUIRE_ONLY_EXECUTION,
	sessionId, info, status);

    return error_code;
}

/**
 *  Get error code associated with given Session ID
 */
grpc_error_t
grpc_get_error(
    grpc_sessionid_t sessionId)
{
    int error, result;
    ngclSession_t *session;
    grpc_l_session_queue_t *session_queue;
    grpc_error_t ret = GRPC_NO_ERROR;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_get_error";

    session_queue = &grpc_l_session_queue;
    session = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Lock the Session Queue */
    ret = grpc_l_session_queue_read_lock(session_queue);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the Session Queue.\n", fName);
        return ret;
    }

    /* Get the Session from Session ID (Session Queue) */
    session = grpc_l_session_queue_get_session(
        session_queue, GRPC_L_SESSION_QUEUE_GET_BY_ID, sessionId);
    if (session != NULL) {
        /* Get Error code from Pure API Error code */
        error_code = grpc_i_get_error_from_ng_error(session->ngs_error);
    }

    /* Unlock the Session Queue */
    ret = grpc_l_session_queue_read_unlock(session_queue);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the Session Queue.\n", fName);
        return ret;
    }

    if (session != NULL) {
        /* Success */
        return error_code;
    }

    /* Get Session from Session ID by Pure API */
    session = grpc_l_session_get_api_list(&sessionId, 1, &error_code);
    if (session == NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: No such Session of Session ID.\n", fName);
        return error_code;
    }

    /* Get Error code of Session */
    result = ngclSessionGetError(session, &error);
    if (result == -1) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get session error code.\n", fName);
        return error_code;
    }

    /* Get Error code from Error code of Pure API */
    error_code = grpc_i_get_error_from_ng_error(result);

    /* Release Ninf-G Session */
    ret = grpc_l_session_release_api_list(session);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Ninf-G Session.\n", fName);
        return ret;
    }

    return error_code;
}

/**
 * Get Session information of last completed Session
 */
grpc_error_t
grpc_get_last_info_np(
    grpc_exec_info_t_np *info,
    int *status)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_get_last_info_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    error_code = grpc_l_session_info_get(
        GRPC_L_SESSION_INFO_GET_LAST,
	GRPC_L_SESSION_INFO_REQUIRE_ONLY_EXECUTION,
	0, info, status);

    return error_code;
}

/**
 * Remove Session information
 *   All remove are remove when SessionId is -1.
 */
grpc_error_t
grpc_session_info_remove_np(
    grpc_sessionid_t sessionId)
{
    grpc_sessionid_t remove_sessionId;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_l_session_queue_unregister_mode_t remove_mode;
    static const char fName[] = "grpc_session_info_remove_np";

    remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_UNDEFINED;
    remove_sessionId = -1;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, fName);

    /* Set the flag */
    if (sessionId == -1) {
        remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_ALL;
    } else {
        remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_BY_SESSIONID;
        remove_sessionId = sessionId;
    }

    /* Unregister */
    error_code = grpc_l_session_queue_unregister_session(
        &grpc_l_session_queue, 1, remove_mode, 0, remove_sessionId);
   if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister Ninf-G Session.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, fName);

    /* Success */
    return error_code;
}

/**
 * Get error message associated with given error code
 */
char *
grpc_error_string(
    grpc_error_t error_code)
{
#define GRPC_L_NMESSAGES \
    (sizeof (grpc_l_error_message) / sizeof (grpc_l_error_message[0]))

    if ((error_code < 0) || (error_code >= GRPC_L_NMESSAGES)) {
         return "GRPC_UNKNOWN_ERROR_CODE";
    }

    return grpc_l_error_message[error_code];
#undef GRPC_L_NMESSAGES
}

/**
 * Get Session ID of failed Session
 */
grpc_error_t
grpc_get_failed_sessionid(
    grpc_sessionid_t *id)
{
    int locked;
    ngclSession_t *current_session, *candidate_session;
    grpc_l_session_queue_t *session_queue;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_get_failed_sessionid";

    locked = 0;
    session_queue = &grpc_l_session_queue;
    current_session = NULL;
    candidate_session = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    if (id == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session ID pointer is not valid.\n", fName);
        return error_code;
    }

    *id = GRPC_SESSIONID_VOID;
    candidate_session = NULL;

    /* Lock the Session Queue */
    error_code = grpc_l_session_queue_read_lock(session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the Session Queue.\n", fName);
        goto error;
    }
    locked = 1;

    /* Find the most recent error session */
    current_session = NULL; /* Retrieve head item */
    while ((current_session = grpc_l_session_queue_get_next_session(
        session_queue, current_session)) != NULL) {

        if ((current_session->ngs_error != GRPC_NO_ERROR) &&
            (current_session->ngs_wasFailureNotified == 0)) {
            *id = current_session->ngs_ID;
            candidate_session = current_session;
            /* Do not break, loop through to tail. */
        }
    }

    /* Set the flag. This session is skipped on next time. */
    if (candidate_session != NULL) {
        candidate_session->ngs_wasFailureNotified = 1;
    }

    /* Unlock the Session Queue */
    error_code = grpc_l_session_queue_read_unlock(session_queue);
    locked = 0;
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the Session Queue.\n", fName);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_read_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read unlock the Session Queue.\n", fName);
        }
    }
    return return_code;
}

/**
 * Set number of saves Session information
 */
grpc_error_t
grpc_session_info_set_threshold_np(
    int threshold)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_session_info_set_threshold_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, fName);

    /* Set threshold */
    error_code = grpc_l_session_queue_set_threshold(
        &grpc_l_session_queue, threshold);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Set the session_info threshold failed.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, fName);

    /* Success */
    return error_code;
}

/**
 * The signal handler: set
 */
grpc_error_t
grpc_signal_handler_set_np(
    int sigNo,
    void (*handler)(int))
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error;
    int result;
    static const char fName[] = "grpc_signal_handler_set_np";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, fName);

    result = ngclNinfgManagerSetSignalHandler(sigNo,
        handler, grpc_l_context->ngc_log, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(result);
        grpc_l_error_set(error_code);

        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't set the signal handler\n", fName, fName);

        goto finalize;
    }

finalize:
    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, fName);

    return error_code;
}

/**
 * Print error message associated with last generated error
 */
grpc_error_t
grpc_perror_np(
    char *str)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
#if 0
    static const char fName[] = "grpc_perror_np";
#endif

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        grpc_l_print_error(
            str, grpc_l_last_error.ge_message, grpc_l_last_error.ge_code);

        return error_code;
    }

    /* Check the Ninf-G Context is valid */
    if (grpc_l_context == NULL) {
        /* Non blocking read of error */

        grpc_l_print_error(
            str, grpc_l_last_error.ge_message, grpc_l_last_error.ge_code);
    } else {
        /* Lock the Last Error */
        result = ngclRWlockReadLock(
            grpc_l_context, &grpc_l_last_error.ge_lock, &error);
        if (result == 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
            grpc_l_error_set(error_code);

            grpc_l_print_error(
                str, grpc_l_last_error.ge_message, grpc_l_last_error.ge_code);

            return error_code;
        }

        grpc_l_print_error(
            str, grpc_l_last_error.ge_message, grpc_l_last_error.ge_code);

        /* Unlock the Last Error */
        result = ngclRWlockReadUnlock(
            grpc_l_context, &grpc_l_last_error.ge_lock, &error);
        if (result == 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
            grpc_l_error_set(error_code);

            grpc_l_print_error(
                str, grpc_l_last_error.ge_message, grpc_l_last_error.ge_code);

            return error_code;
        }
    }

    return error_code;
}

/**
 * Get error code associated with last generated error
 */
grpc_error_t
grpc_last_error_get_np(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_last_error_get_np";

    /* Set Unknown error for default */
    error_code = GRPC_OTHER_ERROR_CODE;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        /* Non blocking read  error code */
        error_code = grpc_l_last_error.ge_code;
    } else {
        /* Lock the Last Error */
        result = ngclRWlockReadLock(
            grpc_l_context, &grpc_l_last_error.ge_lock, &error);
        if (result == 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't lock the last error lock.\n", fName);
            return error_code;
        }

        error_code = grpc_l_last_error.ge_code;

        /* Unlock the Last Error */
        result = ngclRWlockReadUnlock(
            grpc_l_context, &grpc_l_last_error.ge_lock, &error);
        if (result == 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't unlock the last error lock.\n", fName);
            return error_code;
        }
    }

    return error_code;
}

/**
 * GRPC Internal API
 */

/**
 * Set Flags whether initialize was already performed.
 */
grpc_error_t
grpc_l_set_initialize(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
#ifdef NG_PTHREAD
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int result;
#endif /* NG_PTHREAD */
    static const char fName[] = "grpc_l_set_initialize";

#ifdef NG_PTHREAD
    /* Lock mutex */
    result = pthread_mutex_lock(&mutex);
    if (result != 0) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_last_error.ge_code = error_code;
        grpc_l_last_error.ge_message =
            grpc_l_error_message[grpc_l_last_error.ge_code];
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Failed to pthread_mutex_lock.\n", fName);
        return error_code;
    }
#endif /* NG_PTHREAD */

    /* Check whether it is already initialized */
    if (grpc_l_initialized != 0) {
        /* Already initialized */
        error_code = GRPC_ALREADY_INITIALIZED;
        grpc_l_last_error.ge_code = error_code;
        grpc_l_last_error.ge_message =
            grpc_l_error_message[grpc_l_last_error.ge_code];

        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: GRPC client has already been initialized.\n", fName);

#ifdef NG_PTHREAD
        /* Unlock mutex */
        result = pthread_mutex_unlock(&mutex);
        if (result != 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_last_error.ge_code = error_code;
            grpc_l_last_error.ge_message =
                grpc_l_error_message[grpc_l_last_error.ge_code];
            ngclLogPrintfContext(NULL,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Failed to pthread_mutex_lock.\n", fName);
            return error_code;
        }
#endif /* NG_PTHREAD */

        return error_code;
    }

    grpc_l_initialized = 1;

#ifdef NG_PTHREAD
        /* Unlock mutex */
        result = pthread_mutex_unlock(&mutex);
        if (result != 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_last_error.ge_code = error_code;
            grpc_l_last_error.ge_message =
                grpc_l_error_message[grpc_l_last_error.ge_code];
            ngclLogPrintfContext(NULL,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Failed to pthread_mutex_lock.\n", fName);
            return error_code;
        }
#endif /* NG_PTHREAD */

    return error_code;
}

/**
 * Unset Flags whether initialize was already performed.
 */
grpc_error_t
grpc_l_unset_initialize(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
#ifdef NG_PTHREAD
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int result;
    static const char fName[] = "grpc_l_unset_initialize";
#endif /* NG_PTHREAD */

#ifdef NG_PTHREAD
    /* Lock mutex */
    result = pthread_mutex_lock(&mutex);
    if (result != 0) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_last_error.ge_code = error_code;
        grpc_l_last_error.ge_message =
            grpc_l_error_message[grpc_l_last_error.ge_code];
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
            "%s: Failed to pthread_mutex_lock.\n", fName);
        return error_code;
    }
#endif /* NG_PTHREAD */

    /* Check whether it is already unset initialized */
    if (grpc_l_initialized == 0) {
        /* Already unset initialized */
        error_code = GRPC_OTHER_ERROR_CODE;
#ifdef NG_PTHREAD
        /* Unlock mutex */
        result = pthread_mutex_unlock(&mutex);
        if (result != 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_last_error.ge_code = error_code;
            grpc_l_last_error.ge_message =
                grpc_l_error_message[grpc_l_last_error.ge_code];
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Failed to pthread_mutex_lock.\n", fName);
            return error_code;
        }
#endif /* NG_PTHREAD */

        return error_code;
    }

    grpc_l_initialized = 0;

#ifdef NG_PTHREAD
        /* Unlock mutex */
        result = pthread_mutex_unlock(&mutex);
        if (result != 0) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_last_error.ge_code = error_code;
            grpc_l_last_error.ge_message =
                grpc_l_error_message[grpc_l_last_error.ge_code];
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_FATAL, NULL,
                "%s: Failed to pthread_mutex_lock.\n", fName);
            return error_code;
        }
#endif /* NG_PTHREAD */

    return error_code;
}

/**
 * Check whether initialize was already performed.
 */
grpc_error_t
grpc_l_initialize_performed_check(
    void)
{
    grpc_error_t error_code = GRPC_NO_ERROR;

    /* Check whether it is already initialized */
    if (grpc_l_initialized != 0) {
        /* Already initialized */
        return error_code;
    }

    error_code = GRPC_NOT_INITIALIZED;

    /* Non blocking set error for not initialize */
    grpc_l_last_error.ge_code = error_code;
    grpc_l_last_error.ge_message =
        grpc_l_error_message[grpc_l_last_error.ge_code];

    return error_code;
}

/**
 * Initialize the handle.
 */
grpc_error_t
grpc_l_handle_init(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    grpc_l_handle_use_default_t use_default,
    grpc_l_handle_attr_type_t attr_type,
    size_t length,
    char *server_name,
    char *func_name,
    grpc_handle_attr_t_np *attr)
{
    int i, result, error;
    int exeAttr_initialized;
    grpc_handle_attr_t_np saveAttr;
    ngclExecutableAttribute_t exeAttr;
    ngclRemoteMachineInformation_t rmInfo;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    ngclExecutable_t *executable, *currentExecutable;
    char *given_func_name, *handle_type_name;
    static const char fName[] = "grpc_l_handle_init";

    handle_type_name = NULL;
    exeAttr_initialized = 0;
    executable = NULL;
    currentExecutable = NULL;
    given_func_name = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    grpc_l_handle_attr_initialize_member(&saveAttr);

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    assert((use_default == GRPC_L_HANDLE_USE_NO_DEFAULT) ||
           (use_default == GRPC_L_HANDLE_USE_DEFAULT));

    assert((attr_type == GRPC_L_HANDLE_ATTR_NOT_GIVEN) ||
           (attr_type == GRPC_L_HANDLE_ATTR_GIVEN));

    assert(!((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) &&
           (object_handle != NULL)));

    assert(!((handle_type == GRPC_L_HANDLE_TYPE_OBJECT) &&
           (function_handle != NULL)));

    assert(!((use_default == GRPC_L_HANDLE_USE_DEFAULT) &&
           (server_name != NULL)));

    assert(!((attr_type == GRPC_L_HANDLE_ATTR_GIVEN) &&
           ((server_name != NULL) || (func_name != NULL))));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Set the string variable */
    handle_type_name = NULL;
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        handle_type_name = "Function handle";

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        handle_type_name = "Object handle";

    } else {
        abort();
    }

    /**
     * Check the arguments given by user.
     */

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        if (function_handle == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s is not valid.\n", fName, handle_type_name);
            goto error;
        }
    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        if (object_handle == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s is not valid.\n", fName, handle_type_name);
            goto error;
        }
    } else {
        abort();
    }

    /* length check suppressed. */

    /* Check the attribute */
    if ((attr_type == GRPC_L_HANDLE_ATTR_GIVEN) &&
        (attr == NULL)) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        goto error;
    }

    /* Check the server_name */
    if ((use_default == GRPC_L_HANDLE_USE_NO_DEFAULT) &&
        (attr_type == GRPC_L_HANDLE_ATTR_NOT_GIVEN) &&
        (server_name == NULL)) {
        error_code = GRPC_SERVER_NOT_FOUND;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Server host is not valid.\n", fName);
        goto error;
    }

    /* Check the func_name */
    if (attr_type == GRPC_L_HANDLE_ATTR_NOT_GIVEN) {
        given_func_name = func_name;
    } else if (attr_type == GRPC_L_HANDLE_ATTR_GIVEN) {
        assert(attr != NULL);
        given_func_name = attr->gha_functionName;
    } else {
        abort();
    }
    if (given_func_name == NULL) {
        if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
            error_code = GRPC_FUNCTION_NOT_FOUND;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Function name is not valid.\n", fName);

        } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
            error_code = GRPC_CLASS_NOT_FOUND_NP;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Class name is not valid.\n", fName);
        } else {
            abort();
        }

        goto error;
    }

    /* Decoding the server name is performed later. */

    /* Initialize the handle */
    for (i = 0; i < length; i++) {
        grpc_l_executable_handle_convert(
            &exec_handle, handle_type,
            function_handle, object_handle, i);

        error_code = grpc_l_handle_initialize_member(&exec_handle);
        if (error_code != GRPC_NO_ERROR) {
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to initialize the %s.\n",
                fName, handle_type_name);
            goto error;
        }
    }

    /* Initialize the Attribute of Ninf-G Executable */
    result = ngclExecutableAttributeInitialize(
        grpc_l_context, &exeAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Initialize the Attribute of Ninf-G Executable.\n",
            fName);
        goto error;
    }
    exeAttr_initialized = 1;

    /**
     * Set the Value of Ninf-G Executable Attribute
     */

    exeAttr.ngea_invokeNjobs = length;

    assert(given_func_name != NULL);
    exeAttr.ngea_className = strdup(given_func_name);
    if (exeAttr.ngea_className == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string for class name.\n",
            fName);
        goto error;
    }

    if (attr != NULL) {
        if (attr->gha_remoteHostName != NULL) {
            /* Check the host name */
            /* When host name is null, remote host is default server host. */
            exeAttr.ngea_hostName = strdup(attr->gha_remoteHostName);
            if (exeAttr.ngea_hostName == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string for host name.\n",
                    fName);
                goto error;
            }
        }

        exeAttr.ngea_portNo = attr->gha_remotePortNo;

        if (attr->gha_jobManager != NULL) {
            exeAttr.ngea_jobManager = strdup(attr->gha_jobManager);
            if (exeAttr.ngea_jobManager == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string for jobmanager.\n",
                    fName);
                goto error;
            }
        }

        if (attr->gha_subject != NULL) {
            exeAttr.ngea_subject = strdup(attr->gha_subject);
            if (exeAttr.ngea_subject == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string for Subject.\n",
                    fName);
                goto error;
            }
        }

        /* exeAttr.ngea_className */
        /* exeAttr.ngea_invokeNjobs */

        exeAttr.ngea_jobStartTimeout = attr->gha_jobStartTimeout;
        exeAttr.ngea_jobStopTimeout = attr->gha_jobStopTimeout;
        exeAttr.ngea_waitArgumentTransfer = attr->gha_waitArgTransfer;
        if (attr->gha_queueName != NULL) {
            exeAttr.ngea_queueName = strdup(attr->gha_queueName);
            if (exeAttr.ngea_queueName == NULL) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't duplicate the string for queue name.\n",
                    fName);
                goto error;
            }
        }
        exeAttr.ngea_mpiNcpus             = attr->gha_mpiNcpus;

    } else {
        /* attr == NULL */

        /* Decode the server name */
        if (server_name != NULL) {
            error_code = grpc_l_server_name_decode(&exeAttr, server_name);
            if (error_code != GRPC_NO_ERROR) {
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Can't decode the server name.\n", fName);
                goto error;
            }
        }
    }

    /* Ninf-G Executable construct */
    executable = ngclExecutableConstruct(
        grpc_l_context, &exeAttr, &error);
    if (executable == NULL) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to construct the Ninf-G Executable.\n",
            fName);
        goto error;
    }

    grpc_l_handle_attr_initialize_member(&saveAttr);

    /* Get the Remote Machine Information */
    result = ngclRemoteMachineInformationGetCopy(
        grpc_l_context, executable->nge_hostName,
        given_func_name, &rmInfo, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Get Remote Machine Information.\n", fName);
        goto error;
    }

    /* Copy the host name of remote machine */
    saveAttr.gha_remoteHostName = strdup(executable->nge_hostName);
    if (saveAttr.gha_remoteHostName == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string for server name.\n",
            fName);
        goto error;
    }

    /* Set the attribute of handle from remote machine */
    saveAttr.gha_remotePortNo = rmInfo.ngrmi_portNo;

    if (rmInfo.ngrmi_jobManager != NULL) {
        saveAttr.gha_jobManager = strdup(rmInfo.ngrmi_jobManager);
        if (saveAttr.gha_jobManager == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for jobmanager.\n",
                fName);
            goto error;
        }
    }

    if (rmInfo.ngrmi_subject != NULL) {
        saveAttr.gha_subject = strdup(rmInfo.ngrmi_subject);
        if (saveAttr.gha_subject == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for Subject.\n",
                fName);
            goto error;
        }
    }

    saveAttr.gha_functionName = strdup(given_func_name);
    if (saveAttr.gha_functionName == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string for class name.\n",
            fName);
        goto error;
    }

    saveAttr.gha_jobStartTimeout = rmInfo.ngrmi_jobStartTimeout;
    saveAttr.gha_jobStopTimeout  = rmInfo.ngrmi_jobEndTimeout;
    saveAttr.gha_waitArgTransfer = rmInfo.ngrmi_argumentTransfer;

    if (rmInfo.ngrmi_jobQueue != NULL) {
        saveAttr.gha_queueName = strdup(rmInfo.ngrmi_jobQueue);
        if (saveAttr.gha_queueName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for queue name.\n",
                fName);
            goto error;
        }
    }
    saveAttr.gha_mpiNcpus = rmInfo.ngrmi_mpiNcpus;

    /* Release the information */
    result = ngclRemoteMachineInformationRelease(
        grpc_l_context, &rmInfo, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Release Remote Machine Information.\n",
            fName);
        goto error;
    }

    /* Copy the defined Executable Attribute */

    if (exeAttr.ngea_portNo != NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        saveAttr.gha_remotePortNo = exeAttr.ngea_portNo;
    }

    if (exeAttr.ngea_jobManager != NULL) {
        if (saveAttr.gha_jobManager != NULL) {
            globus_libc_free(saveAttr.gha_jobManager);
            saveAttr.gha_jobManager = NULL;
        }
        saveAttr.gha_jobManager = strdup(exeAttr.ngea_jobManager);
        if (saveAttr.gha_jobManager == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for jobmanager.\n",
                fName);
            goto error;
        }
    }

    if (exeAttr.ngea_subject != NULL) {
        if (saveAttr.gha_subject != NULL) {
            globus_libc_free(saveAttr.gha_subject);
            saveAttr.gha_subject = NULL;
        }
        saveAttr.gha_subject = strdup(exeAttr.ngea_subject);
        if (saveAttr.gha_subject == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for Subject.\n",
                fName);
            goto error;
        }
    }

    if (exeAttr.ngea_jobStartTimeout !=
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        saveAttr.gha_jobStartTimeout = exeAttr.ngea_jobStartTimeout;
    }

    if (exeAttr.ngea_jobStopTimeout !=
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        saveAttr.gha_jobStopTimeout = exeAttr.ngea_jobStopTimeout;
    }

    if (exeAttr.ngea_waitArgumentTransfer !=
        NG_ARGUMENT_TRANSFER_UNDEFINED) {
        saveAttr.gha_waitArgTransfer = exeAttr.ngea_waitArgumentTransfer;
    }

    if (exeAttr.ngea_queueName != NULL) {
        if (saveAttr.gha_queueName != NULL) {
            globus_libc_free(saveAttr.gha_queueName);
            saveAttr.gha_queueName = NULL;
        }
        saveAttr.gha_queueName = strdup(exeAttr.ngea_queueName);
        if (saveAttr.gha_queueName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for queue name.\n",
                fName);
            goto error;
        }
    }

    if (exeAttr.ngea_mpiNcpus !=
        NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED) {
        saveAttr.gha_mpiNcpus = exeAttr.ngea_mpiNcpus;
    }

    /* handle setup */
    currentExecutable = executable;
    for (i = 0; i < length; i++) {
        assert(currentExecutable != NULL);
        grpc_l_executable_handle_convert(
            &exec_handle, handle_type,
            function_handle, object_handle, i);

        error_code = grpc_l_handle_setup(
            &exec_handle, currentExecutable, &saveAttr);
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to setup the %s.\n",
                fName, handle_type_name);
            goto error;
        }

        if ((i + 1) < length) {
            currentExecutable = ngclExecutableMultiHandleListGetNext(
                grpc_l_context, currentExecutable, &error);
            if ((currentExecutable == NULL) && (error != NG_ERROR_NO_ERROR)) {
                error_code = grpc_i_get_error_from_ng_error(error);
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to setup the %s.\n",
                    fName, handle_type_name);
                goto error;
            }
        } else {
            currentExecutable = NULL;
        }
    }

    /* Release Attribute of Ninf-G Executable */
    result = ngclExecutableAttributeFinalize(
        grpc_l_context, &exeAttr, &error);
    exeAttr_initialized = 0;
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Release Attribute of Ninf-G Executable.\n",
            fName);
        goto error;
    }

    grpc_l_handle_attr_finalize_member(&saveAttr);

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;

    /* Error occurred */
error:
    if (exeAttr_initialized != 0) {
        result = ngclExecutableAttributeFinalize(
            grpc_l_context, &exeAttr, NULL);
        exeAttr_initialized = 0;
        if (result == 0) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to Release Attribute of Ninf-G Executable.\n",
                fName);
        }
    }

    grpc_l_handle_attr_finalize_member(&saveAttr);

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return by error.\n", fName, api_name);

    return error_code;
}

/**
 * Destruct the handle.
 */
grpc_error_t
grpc_l_handle_destruct(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    size_t length)
{
    int i;
    char *handle_type_name;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_destruct";

    handle_type_name = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    assert(!((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) &&
           (object_handle != NULL)));

    assert(!((handle_type == GRPC_L_HANDLE_TYPE_OBJECT) &&
           (function_handle != NULL)));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Set the string variable */
    handle_type_name = NULL;
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        handle_type_name = "Function handle";

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        handle_type_name = "Object handle";

    } else {
        abort();
    }

    /**
     * Check the arguments given by user.
     */

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        if (function_handle == NULL) {
            error_code = GRPC_INVALID_FUNCTION_HANDLE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s is not valid.\n", fName, handle_type_name);
            goto error;
        }
    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        if (object_handle == NULL) {
            error_code = GRPC_INVALID_OBJECT_HANDLE_NP;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s is not valid.\n", fName, handle_type_name);
            goto error;
        }
    } else {
        abort();
    }

    /* length check suppressed. */

    /* Destruct All handles */
    for (i = 0; i < length; i++) {
        grpc_l_executable_handle_convert(
            &exec_handle, handle_type,
            function_handle, object_handle, i);

        /* Check the handle type */
        if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
            if (*(exec_handle.geh_discernment) !=
                GRPC_HANDLE_TYPE_FUNCTION) {
                error_code = GRPC_INVALID_FUNCTION_HANDLE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: This handle is not %s.\n",
                    fName, handle_type_name);
                goto error;
            }
        } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
            if (*(exec_handle.geh_discernment) !=
                GRPC_HANDLE_TYPE_OBJECT) {
                error_code = GRPC_INVALID_OBJECT_HANDLE_NP;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: This handle is not %s.\n",
                    fName, handle_type_name);
                goto error;
            }
        } else {
            abort();
        }

        /* handle destruct */
        error_code = grpc_l_handle_destruct_one(&exec_handle);
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to destruct the %s.\n",
                fName, handle_type_name);
            goto error;
        }

        /* Initialize the handle */
        error_code = grpc_l_handle_initialize_member(&exec_handle);
        if (error_code != GRPC_NO_ERROR) {
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to initialize the %s.\n",
                fName, handle_type_name);
                return error_code;
        }
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;

    /* Error occurred */
error:
    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return by error.\n", fName, api_name);

    return error_code;
}

/**
 * Function/Object handle initialize
 */
grpc_error_t
grpc_l_handle_initialize_member(
    grpc_l_executable_handle_t *exec_handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
#if 0
    static const char fName[] = "grpc_l_handle_initialize_member";
#endif

    /* Check the arguments */
    assert(exec_handle != NULL);

    *exec_handle->geh_id           = GRPC_L_HANDLE_ID_UNDEFINED;
    *exec_handle->geh_exec         = NULL;
    *exec_handle->geh_errorCode    = GRPC_NO_ERROR;
    *exec_handle->geh_errorMessage = NULL;
    *exec_handle->geh_discernment  = GRPC_HANDLE_TYPE_INVALID;
    *exec_handle->geh_attr         = NULL;

    return error_code;
}

/**
 * Decode the server name.
 */
grpc_error_t
grpc_l_server_name_decode(
    ngclExecutableAttribute_t *exeAttr,
    char *server_name)
{
    int i;
    char *tmp, *cur, *end;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_server_name_decode";

    /* Check the arguments */
    assert(server_name != NULL);
    assert(exeAttr != NULL);
    assert(exeAttr->ngea_hostName == NULL);
    assert(exeAttr->ngea_jobManager == NULL);
    assert(exeAttr->ngea_subject == NULL);
    
    tmp = NULL;

    /**
     * grpc_function_handle_init() argument server_name allows
     * following types.
     *
     *    host
     *    host:port
     *    host:port/service
     *    host/service
     *    host:/service
     *    host::subject
     *    host:port:subject
     *    host/service:subject
     *    host:/service:subject
     *    host:port/service:subject
     */

    cur = server_name;

    /* NULL string check */
    if (*cur == '\0') {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: server name is null string.\n",
            fName);
        goto error;
    }

    /* Copy the hostname */
    tmp = strdup(cur);
    if (tmp == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string for server name \"%s\".\n",
            fName, server_name);
        goto error;
    }

    i = 0;
    while ((*cur != ':') && (*cur != '/') && (*cur != '\0')) {
        i++;
        cur++;
    }

    if ((*cur == ':') || (*cur == '/')) {
        tmp[i] = '\0';
    }

    exeAttr->ngea_hostName = tmp;
    tmp = NULL;

    /* Copy the port */
    if (*cur == ':') {
        cur++;
        if ((*cur != ':') && (*cur != '/') && (*cur != '\0')) {
            exeAttr->ngea_portNo = strtol(cur, &end, 10);
            if ((end == NULL) || ((end != NULL) &&
                (*end != ':') && (*end != '/') && (*end != '\0'))) {
                error_code = GRPC_OTHER_ERROR_CODE;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Invalid port number on server name \"%s\".\n",
                    fName, server_name);
                goto error;
            }
            cur = end;
        }
    }

    /* Copy the jobmanager */
    if (*cur == '/') {
        cur++;

        tmp = strdup(cur);
        if (tmp == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the jobmanager string on "
                "server name \"%s\".\n",
                fName, server_name);
            goto error;
        }
     
        i = 0;
        while ((*cur != ':') && (*cur != '\0')) {
            i++;
            cur++;
        }
     
        if (*cur == ':') {
            tmp[i] = '\0';
        }

        exeAttr->ngea_jobManager = tmp;
        tmp = NULL;
    }

    /* Copy the Subject */
    if (*cur == ':') {
        cur++;

        tmp = strdup(cur);
        if (tmp == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the Subject string on "
                "server name \"%s\".\n",
                fName, server_name);
            goto error;
        }
     
        exeAttr->ngea_subject = tmp;
        tmp = NULL;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
        "%s: Can't parse the server name \"%s\".\n",
        fName, server_name);

    return error_code;
}

/**
 * handle setup
 */
grpc_error_t
grpc_l_handle_setup(
    grpc_l_executable_handle_t *exec_handle,
    ngclExecutable_t *executable,
    grpc_handle_attr_t_np *attr)
{
    grpc_handle_attr_t_np *newAttr;
    grpc_l_handle_type_t handle_type;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t ret = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_l_handle_setup";

    /* Check the arguments */
    assert(exec_handle != NULL);

    newAttr = NULL;
    handle_type = exec_handle->geh_handle_type;
    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    *(exec_handle->geh_id) = grpc_l_get_handleId();
    if (*(exec_handle->geh_id) == GRPC_L_HANDLE_ID_UNDEFINED) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get handle ID.\n", fName);
        return error_code;
    }

    *(exec_handle->geh_exec) = executable;
    *(exec_handle->geh_errorCode) = GRPC_NO_ERROR;

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        *(exec_handle->geh_discernment) = GRPC_HANDLE_TYPE_FUNCTION;
    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        *(exec_handle->geh_discernment) = GRPC_HANDLE_TYPE_OBJECT;
    } else {
        abort();
    }

    *(exec_handle->geh_errorMessage) = NULL;
    *(exec_handle->geh_errorMessage) = strdup("No error");
    if (*(exec_handle->geh_errorMessage) == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for handle error message.\n",
            fName);
        return error_code;
    }

    newAttr = (grpc_handle_attr_t_np *)
        globus_libc_calloc(1, sizeof(grpc_handle_attr_t_np));
    if (newAttr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for handle attribute.\n",
            fName);
        return error_code;
    }

    error_code = grpc_l_handle_attr_copy(newAttr, attr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the handle attribute.\n",
            fName);
        return error_code;
    }

    *(exec_handle->geh_attr) = newAttr;
    newAttr = NULL;

    /* Register handle to Ninf-G Executable */
    result = ngclExecutableRegisterUserData(
        executable,
        ((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ?
            (void *)exec_handle->geh_function_handle :
            ((handle_type == GRPC_L_HANDLE_TYPE_OBJECT) ?
            (void *)exec_handle->geh_object_handle : NULL)),
        NULL, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to register user data to Ninf-G Executable.\n",
            fName);

        ret = grpc_l_handle_set_error(exec_handle);
        if (ret != GRPC_NO_ERROR) {
            error_code = ret;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to unregister user data from Ninf-G Executable.\n",
                fName);
            goto error;
        }
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    /* Deallocate the storage */
    if (newAttr != NULL) {
        grpc_l_handle_attr_finalize_member(newAttr);
        globus_libc_free(newAttr);
        newAttr = NULL;
        *(exec_handle->geh_attr) = NULL;
    }

    return error_code;
}

/**
 * one handle destruct
 */
grpc_error_t
grpc_l_handle_destruct_one(
    grpc_l_executable_handle_t *exec_handle)
{
    int error, result;
    char *error_message;
    grpc_error_t ret = GRPC_NO_ERROR;
    grpc_handle_attr_t_np *handle_attr;
    grpc_error_t error_code = GRPC_NO_ERROR;
    int unusedSessions = 0;
    static const char fName[] = "grpc_l_handle_destruct_one";

    /* Check the arguments */
    assert(exec_handle != NULL);

    handle_attr = NULL;
    error_message = NULL;

    /* Unregister userData from Ninf-G Executable */
    result = ngclExecutableUnregisterUserData(
        *(exec_handle->geh_exec), &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister user data from Ninf-G Executable.\n",
            fName);
        goto error;
    }

    /* handle destruct */
    error_code = grpc_l_handle_unregister_unused_session(
	exec_handle, &unusedSessions);
    if (error_code != GRPC_NO_ERROR) {
	ngclLogPrintfContext(grpc_l_context,
	    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Failed to unregister the Session.\n", fName);
	goto error;
    }

    /* Ninf-G Executable destruct */
    result = ngclExecutableDestruct(
        *(exec_handle->geh_exec), unusedSessions != 0, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to destruct the Ninf-G Executable.\n", fName);
        goto error;
    }

    handle_attr = *(exec_handle->geh_attr);
    if (handle_attr != NULL) {
        grpc_l_handle_attr_finalize_member(handle_attr);
        globus_libc_free(handle_attr);
        handle_attr = NULL;
    }
    *(exec_handle->geh_attr) = NULL;

    error_message = *(exec_handle->geh_errorMessage);
    if (error_message != NULL) {
        globus_libc_free(error_message);
        error_message = NULL;
    }
    *(exec_handle->geh_errorMessage) = NULL;

    /* Success */
    return error_code;

    /* Error occurred */
error:
    ret = grpc_l_handle_set_error(exec_handle);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to set error to handle.\n", fName);
        error_code = ret;
        grpc_l_error_set(error_code);
    }

    return error_code;
}

/**
 * Unused Session will be unregister from Executable.
 */
grpc_error_t
grpc_l_handle_unregister_unused_session(
    grpc_l_executable_handle_t *exec_handle, int *unusedSessions)
{
    int error, result;
    grpc_error_t ret = GRPC_NO_ERROR;
    grpc_error_t error_code = GRPC_NO_ERROR;
    int exclusiveLocked = 0;
    int sessionListLocked = 0;
    ngclSession_t *session;
    grpc_l_session_data_t *session_data;
    static const char fName[] = "grpc_l_handle_unregister_unused_session";

    /* Check the arguments */
    assert(exec_handle != NULL);
    assert(unusedSessions != NULL);

    /* Initialize the arguments */
    *unusedSessions = 0;

    /* Lock the GRPC manager */
    result = ngclExclusiveLock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the GRPC manager.\n", fName);
        goto error;
    }
    exclusiveLocked = 1;

    /* Lock the Session list */
    result = ngclSessionListWriteLock(grpc_l_context, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the GRPC manager.\n", fName);
        goto error;
    }
    sessionListLocked = 1;

    session = NULL;
    while (1) {
	/* Get the next Session */
	error = NG_ERROR_NO_ERROR;
	session = ngclSessionGetNext(
	    *(exec_handle->geh_exec), session, &error);
	if (session == NULL) {
	    if (error != NG_ERROR_NO_ERROR) {
		error_code = grpc_i_get_error_from_ng_error(error);
		grpc_l_error_set(error_code);
		ngclLogPrintfContext(grpc_l_context,
		    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		    "%s: Failed to get Session.\n", fName);
		goto error;
	    }
	    break;
	}

	/* Get user data */
	result = ngclSessionGetUserData(
	    session, (void *)&session_data, &error);
	if (result == 0) {
	    error_code = grpc_i_get_error_from_ng_error(error);
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to get user data from Ninf-G Session.\n", fName);
	    goto error;
	}

	/* Is Session under use? */
	if (session_data->gsd_sessionUse != GRPC_L_SESSION_NOT_USE)
	    continue;

	/* Unused session is exist */
	*unusedSessions = 1;

	/* Unregister the Session from Executable */
	result = ngclSessionUnregisterWithoutLock(session, &error);
	if (result == 0) {
	    error_code = grpc_i_get_error_from_ng_error(error);
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to Unregist Session from Ninf-G Executable.\n",
		fName);
	    goto error;
	}

	/* Register session list to Session Queue */
	ret = grpc_l_session_queue_register_session_list_without_unregister(
	    &grpc_l_session_queue, session);
	if (ret != GRPC_NO_ERROR) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to register Session to Session Queue.\n",
		fName);
	    return ret;
	}

	ngclLogPrintfSession(session, NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_WARNING, NULL,
	    "%s: Session was unregistered from Executable.\n", fName);

	session = NULL;
    }

    /* Unlock the Session list */
    sessionListLocked = 0;
    result = ngclSessionListWriteUnlock(grpc_l_context, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the GRPC manager.\n", fName);
        goto error;
    }

    /* Unlock the GRPC manager */
    exclusiveLocked = 0;
    result = ngclExclusiveUnlock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the GRPC manager.\n", fName);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    ret = grpc_l_handle_set_error(exec_handle);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to set error to handle.\n", fName);
    }

    /* Unlock the Session list */
    if (sessionListLocked != 0) {
	sessionListLocked = 0;
	result = ngclSessionListWriteUnlock(grpc_l_context, &error);
	if (result == 0) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
	}
    }

    /* Unlock the GRPC manager */
    if (exclusiveLocked != 0) {
	exclusiveLocked = 0;
	result = ngclExclusiveUnlock(
	    grpc_l_context, &grpc_l_manager.gm_exclusiveLock, NULL);
	if (result == 0) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
	}
    }

    return error_code;
}

/**
 * Get attribute of handle.
 */
grpc_error_t
grpc_l_handle_get_attr(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    grpc_handle_attr_t_np *attr)
{
    grpc_handle_attr_t_np *handleAttr;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_get_attr";

    handleAttr = NULL;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    /* Check the attr */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    handleAttr = *(exec_handle.geh_attr);
    if (handleAttr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute in the handle is not valid.\n",
            fName);
        return error_code;
    }

    /* Copy the attribute */
    error_code = grpc_l_handle_attr_copy(attr, handleAttr);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't copy the handle attribute.\n",
            fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;
}

/**
 * Print error message associated with error generated by handle.
 */
grpc_error_t
grpc_l_handle_perror(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    char *str)
{
    char *h_error_message;
    grpc_error_t h_error_code;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_perror";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    h_error_message = *(exec_handle.geh_errorMessage);
    h_error_code = *(exec_handle.geh_errorCode);

    grpc_l_print_error(str, h_error_message, h_error_code);

    /* Success */
    return error_code;
}

/**
 * Get error code associated with error generated by handle.
 */
grpc_error_t
grpc_l_handle_get_error(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type)
{
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_get_error";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    return *(exec_handle.geh_errorCode);
}

/**
 * Is the handle ready?
 */
grpc_error_t
grpc_l_handle_is_ready(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type)
{
    int error, result;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_is_ready";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    /* Check if it's IDLE */
    result = ngclExecutableIsIdle(
        grpc_l_context, *(exec_handle.geh_exec), &error);
    if (result == 1) {
        /* the handle is ready(returns NO_ERROR) */
        error_code = GRPC_NO_ERROR;
    } else {
        if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
            error_code = GRPC_INVALID_FUNCTION_HANDLE;
        } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
            error_code = GRPC_INVALID_OBJECT_HANDLE_NP;
        } else {
            abort();
        }

        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_INFORMATION, NULL,
            "%s: This handle is not ready.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;
}

/**
 * Get handle from Session ID.
 */
grpc_error_t
grpc_l_handle_get(
    const char *api_name,
    grpc_sessionid_t sessionId,
    grpc_l_handle_type_t handle_type,
    grpc_function_handle_t **function_handle,
    grpc_object_handle_t_np **object_handle)
{
    int locked;
    void *handle;
    int error, result;
    grpc_l_session_data_t *session_data = NULL;
    char *handle_type_name;
    ngclSession_t *session = NULL;
    ngclExecutable_t *executable = NULL;
    grpc_l_session_queue_t *session_queue;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_get";

    locked = 0;
    session_queue = &grpc_l_session_queue;

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Set the string variable */
    handle_type_name = NULL;
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        handle_type_name = "Function handle";

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        handle_type_name = "Object handle";

    } else {
        abort();
    }

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        assert(object_handle == NULL);
        if (function_handle == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s pointer is not valid.\n",
                fName, handle_type_name);
            goto error;
        }
        *function_handle = NULL;

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        assert(function_handle == NULL);
        if (object_handle == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: %s pointer is not valid.\n",
                fName, handle_type_name);
            goto error;
        }
        *object_handle = NULL;

    } else {
        abort();
    }

    /* Lock the Session Queue */
    error_code = grpc_l_session_queue_read_lock(session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the Session Queue.\n", fName);
        goto error;
    }
    locked = 1;

    /* Get the Session from Session ID (Session Queue) */
    session = grpc_l_session_queue_get_session(
        session_queue, GRPC_L_SESSION_QUEUE_GET_BY_ID, sessionId);
    if (session == NULL) {
        /* Get Session from Session ID by Pure API */
        session = grpc_l_session_get_api_list(&sessionId, 1, &error_code);
        if (session == NULL) {
            error_code = GRPC_INVALID_SESSION_ID;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: No such Session of Session ID.\n", fName);
            goto error;
        }

        /* Release Ninf-G Session */
        result = grpc_l_session_release_api_list(session);
        if (result != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to release Ninf-G Session.\n", fName);
            goto error;
        }
    }

    /* get executable ID from Ninf-G Session */
    result = ngclSessionGetUserData(session, (void *)&session_data, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get user data from Ninf-G Session.\n", fName);
        goto error;
    }

    /* Get Ninf-G Executable by ID */
    executable = ngclExecutableGet(
	grpc_l_context, session_data->gsd_executableID, &error);
    if (executable == NULL) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Ninf-G Executable.\n", fName);
        goto error;
    }

    /* Get handle by Pure API */
    result = ngclExecutableGetUserData(executable, (void **)&handle, &error);
    if ((result == 0) || (handle == NULL)) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get user data from Ninf-G Executable.\n", fName);
        goto error;
    }

    /* Set the handle */
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        *function_handle = (grpc_function_handle_t *)handle;

        if ((*function_handle)->gfh_discernment != GRPC_HANDLE_TYPE_FUNCTION) {
            error_code = GRPC_INVALID_SESSION_ID;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The session ID is not %s.\n",
                fName, handle_type_name);
            goto error;
        }

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        *object_handle = (grpc_object_handle_t_np *)handle;

        if ((*object_handle)->goh_discernment !=  GRPC_HANDLE_TYPE_OBJECT) {
            error_code = GRPC_INVALID_SESSION_ID;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The session ID is not %s.\n",
                fName, handle_type_name);
            goto error;
        }
    } else {
        abort();
    }

    /* Unlock the Session Queue */
    error_code = grpc_l_session_queue_read_unlock(session_queue);
    locked = 0;
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the Session Queue.\n", fName);
        goto error;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_read_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read unlock the Session Queue.\n", fName);
        }
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return by error.\n", fName, api_name);

    return return_code;
}

/**
 * Set error to handle.
 */
grpc_error_t
grpc_l_handle_set_error(
    grpc_l_executable_handle_t *exec_handle)
{
    int error, result;
    char *error_message;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_set_error";

    /* Check the arguments */
    assert(exec_handle != NULL);

    error_message = NULL;

    /* Lock Session management queue */
    result = ngclRWlockReadLock(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(result);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to lock Session management queue.\n", fName);
        return error_code;
    }

    *(exec_handle->geh_errorCode) = grpc_l_last_error.ge_code;

    error_message = *(exec_handle->geh_errorMessage);
    if (error_message != NULL) {
        globus_libc_free(error_message);
        error_message = NULL;
    }
    *(exec_handle->geh_errorMessage) = NULL;

    error_message = strdup(grpc_l_last_error.ge_message);
    if (error_message == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't duplicate the string for error message.\n",
            fName);
    }
    *(exec_handle->geh_errorMessage) = error_message;

    /* Unlock Session management queue */
    result = ngclRWlockReadUnlock(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unlock Session management queue.\n", fName);
        return error_code;
    }

    return error_code;
}

/**
 * Get handle ID
 */
int
grpc_l_get_handleId(
    void)
{
    static int handleIdConter = GRPC_L_HANDLE_ID_MIN;
    static const char fName[] = "grpc_l_get_handleId";

    if ((handleIdConter + 1) >= GRPC_L_HANDLE_ID_MAX) {
        grpc_l_error_set(GRPC_OTHER_ERROR_CODE);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: There is no IDs for Handle to use.\n", fName);

        return GRPC_L_HANDLE_ID_UNDEFINED;
    }

    return handleIdConter++;
}

/**
 * Initialize the handle attribute.
 */
void
grpc_l_handle_attr_initialize_member(
    grpc_handle_attr_t_np *attr)
{
#if 0
    static const char fName[] = "grpc_l_handle_attr_initialize_member";
#endif

    /* Check the arguments */
    assert(attr != NULL);

    /* for safety */
    attr->gha_remoteHostName = NULL;
    attr->gha_remotePortNo = NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED;
    attr->gha_jobManager = NULL;
    attr->gha_subject = NULL;
    attr->gha_functionName = NULL;
    attr->gha_jobStartTimeout = NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED;
    attr->gha_jobStopTimeout = NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED;
    attr->gha_waitArgTransfer = NG_ARGUMENT_TRANSFER_UNDEFINED;
    attr->gha_queueName = NULL;
    attr->gha_mpiNcpus = NGCL_EXECUTABLE_ATTRIBUTE_MEMBER_UNDEFINED;

    return;
}

/**
 * Finalize the handle attribute.
 */
void
grpc_l_handle_attr_finalize_member(
    grpc_handle_attr_t_np *attr)
{
#if 0
    static const char fName[] = "grpc_l_handle_attr_finalize_member";
#endif

    /* Check the arguments */
    assert(attr != NULL);

    if (attr->gha_remoteHostName != NULL) {
        globus_libc_free(attr->gha_remoteHostName);
        attr->gha_remoteHostName = NULL;
    }

    if (attr->gha_jobManager != NULL) {
        globus_libc_free(attr->gha_jobManager);
        attr->gha_jobManager = NULL;
    }

    if (attr->gha_subject != NULL) {
        globus_libc_free(attr->gha_subject);
        attr->gha_subject = NULL;
    }

    if (attr->gha_functionName != NULL) {
        globus_libc_free(attr->gha_functionName);
        attr->gha_functionName = NULL;
    }

    if (attr->gha_queueName != NULL) {
        globus_libc_free(attr->gha_queueName);
        attr->gha_queueName = NULL;
    }

    grpc_l_handle_attr_initialize_member(attr);

    return;
}

/**
 * Copy the handle attribute.
 */
grpc_error_t
grpc_l_handle_attr_copy(
    grpc_handle_attr_t_np *dstAttr, grpc_handle_attr_t_np *srcAttr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_handle_attr_copy";

    /* Check the arguments */
    assert(dstAttr != NULL);
    assert(srcAttr != NULL);

    grpc_l_handle_attr_initialize_member(dstAttr);

    *dstAttr = *srcAttr;

    if (srcAttr->gha_remoteHostName != NULL) {
        dstAttr->gha_remoteHostName = strdup(srcAttr->gha_remoteHostName);
        if (dstAttr->gha_remoteHostName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for host name.\n",
                fName);
            return error_code;
        }
    }

    if (srcAttr->gha_jobManager != NULL) {
        dstAttr->gha_jobManager = strdup(srcAttr->gha_jobManager);
        if (dstAttr->gha_jobManager == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for jobmanager.\n",
                fName);
            return error_code;
        }
    }

    if (srcAttr->gha_subject != NULL) {
        dstAttr->gha_subject = strdup(srcAttr->gha_subject);
        if (dstAttr->gha_subject == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for Subject.\n",
                fName);
            return error_code;
        }
    }

    if (srcAttr->gha_functionName != NULL) {
        dstAttr->gha_functionName = strdup(srcAttr->gha_functionName);
        if (dstAttr->gha_functionName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for func name.\n",
                fName);
            return error_code;
        }
    }

    if (srcAttr->gha_queueName != NULL) {
        dstAttr->gha_queueName = strdup(srcAttr->gha_queueName);
        if (dstAttr->gha_queueName == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't duplicate the string for queue name.\n",
                fName);
            return error_code;
        }
    }

    /* Success */
    return error_code;
}

/**
 * Convert to grpc_l_executable_handle_t.
 */
void
grpc_l_executable_handle_convert(
    grpc_l_executable_handle_t *exec_handle,
    grpc_l_handle_type_t handle_type,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    int i)
{
    grpc_function_handle_t *f_handle_i;
    grpc_object_handle_t_np *o_handle_i;

    /* Check the arguments */
    assert(exec_handle != NULL);
    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    f_handle_i = NULL;
    o_handle_i = NULL;

    exec_handle->geh_handle_type = handle_type;

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        assert(function_handle != NULL);
        assert(object_handle == NULL);

        f_handle_i = &function_handle[i];

        exec_handle->geh_is_handle_null = (void *)function_handle;
        exec_handle->geh_function_handle = f_handle_i;
        exec_handle->geh_object_handle = NULL;

        exec_handle->geh_id           = &f_handle_i->gfh_id;
        exec_handle->geh_attr         = &f_handle_i->gfh_attr;
        exec_handle->geh_exec         = &f_handle_i->gfh_exec;
        exec_handle->geh_errorCode    = &f_handle_i->gfh_errorCode;
        exec_handle->geh_errorMessage = &f_handle_i->gfh_errorMessage;
        exec_handle->geh_discernment  = &f_handle_i->gfh_discernment;

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        assert(object_handle != NULL);
        assert(function_handle == NULL);

        o_handle_i = &object_handle[i];

        exec_handle->geh_is_handle_null = (void *)object_handle;
        exec_handle->geh_function_handle = NULL;
        exec_handle->geh_object_handle = o_handle_i;

        exec_handle->geh_id           = &o_handle_i->goh_id;
        exec_handle->geh_attr         = &o_handle_i->goh_attr;
        exec_handle->geh_exec         = &o_handle_i->goh_exec;
        exec_handle->geh_errorCode    = &o_handle_i->goh_errorCode;
        exec_handle->geh_errorMessage = &o_handle_i->goh_errorMessage;
        exec_handle->geh_discernment  = &o_handle_i->goh_discernment;

    } else {
        abort();
    }

    /* Check the executable handle */
    assert(exec_handle->geh_id != NULL);
    assert(exec_handle->geh_attr != NULL);
    assert(exec_handle->geh_exec != NULL);
    assert(exec_handle->geh_errorCode != NULL);
    assert(exec_handle->geh_errorMessage != NULL);
    assert(exec_handle->geh_discernment != NULL);

    return;
}

/**
 * Convert to grpc_l_executable_handle_t with check.
 */
grpc_error_t
grpc_l_executable_handle_convert_with_check(
    grpc_l_executable_handle_t *exec_handle,
    grpc_l_handle_type_t handle_type,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_executable_handle_convert_with_check";

    /* Check the arguments */
    assert(exec_handle != NULL);
    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        if (function_handle == NULL) {
            error_code = GRPC_INVALID_FUNCTION_HANDLE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Function handle is not valid.\n", fName);
            return error_code;
        }

        if (function_handle->gfh_discernment != GRPC_HANDLE_TYPE_FUNCTION) {
            error_code = GRPC_INVALID_FUNCTION_HANDLE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: This handle is not the Function handle.\n", fName);
            return error_code;
        }

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        if (object_handle == NULL) {
            error_code = GRPC_INVALID_OBJECT_HANDLE_NP;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Object handle is not valid.\n", fName);
            return error_code;
        }

        if (object_handle->goh_discernment != GRPC_HANDLE_TYPE_OBJECT) {
            error_code = GRPC_INVALID_OBJECT_HANDLE_NP;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Object handle is not valid.\n", fName);
            return error_code;
        }

    } else {
        abort();
    }

    grpc_l_executable_handle_convert(
        exec_handle, handle_type, function_handle, object_handle, 0);

    return error_code;
}

/**
 * GridRPC session start
 */
grpc_error_t
grpc_l_session_start(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    char *method_name,
    grpc_l_argument_type_t argument_type,
    va_list va_argument,
    grpc_arg_stack_t *arg_stack,
    grpc_l_call_wait_type_t wait_type,
    grpc_sessionid_t *sessionID,
    grpc_l_session_attr_type_t attr_type,
    grpc_session_attr_t_np *session_attr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t ret = GRPC_NO_ERROR;
    grpc_l_executable_handle_t exec_handle;
    char *handle_type_name;
    int error, result;
    ngclSession_t *session;
    ngclExecutable_t *executable;
    grpc_l_session_data_t *session_data = NULL;
    ngclSessionAttribute_t ngclSessionAttr;
    int exclusiveLocked = 0;
    static const char fName[] = "grpc_l_session_start";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    assert((argument_type == GRPC_L_ARGUMENT_TYPE_VA_LIST) ||
           (argument_type == GRPC_L_ARGUMENT_TYPE_ARG_STACK));

    assert((wait_type == GRPC_L_CALL_WAIT_TYPE_SYNC) ||
           (wait_type == GRPC_L_CALL_WAIT_TYPE_ASYNC));

    assert((attr_type == GRPC_L_SESSION_ATTR_NOT_GIVEN) ||
           (attr_type == GRPC_L_SESSION_ATTR_GIVEN));

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    /* Set the string variable */
    handle_type_name = NULL;
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        handle_type_name = "Function handle";

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        handle_type_name = "Object handle";

    } else {
        abort();
    }

    /* Handle check was already performed */

    /* Check the method_name is valid */
    if (handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) {
        assert(method_name == NULL);

    } else if (handle_type == GRPC_L_HANDLE_TYPE_OBJECT) {
        if (method_name == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Method_name is not valid.\n", fName);
            goto error;
        }
    } else {
        abort();
    }

    /* Check the the grpc_call argument */
    if (argument_type == GRPC_L_ARGUMENT_TYPE_VA_LIST) {
        assert(arg_stack == NULL);

    } else if (argument_type == GRPC_L_ARGUMENT_TYPE_ARG_STACK) {
        if (arg_stack == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Argument stack is not valid.\n", fName);
            goto error;
        }

    } else {
        abort();
    }

    /* Check the argument for returned sessionID */
    if (wait_type == GRPC_L_CALL_WAIT_TYPE_SYNC) {
        assert((sessionID != NULL) || (sessionID == NULL)); /* both ok */

    } else if (wait_type == GRPC_L_CALL_WAIT_TYPE_ASYNC) {
        if (sessionID == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: The Session ID is not valid.\n", fName);
            goto error;
        }

    } else {
        abort();
    }
    if (sessionID != NULL) {
        *sessionID = -1;
    }

    /* Check the Session Attribute */
    if (attr_type == GRPC_L_SESSION_ATTR_GIVEN) {
        if (session_attr == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Session attribute is not valid.\n", fName);
            goto error;
        }
    } else if (attr_type == GRPC_L_SESSION_ATTR_NOT_GIVEN) {
        assert(session_attr == NULL);
    } else {
        abort();
    }

    executable = *(exec_handle.geh_exec);

    /* Get the Attribute of Session */
    result = ngclSessionAttributeInitialize(
        grpc_l_context, executable, &ngclSessionAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Get Attribute of Session.\n", fName);
        goto error;
    }

    /* set members */
    if (session_attr != NULL) {
        ngclSessionAttr.ngsa_waitArgumentTransfer =
            session_attr->gsa_waitArgTransfer;
        ngclSessionAttr.ngsa_timeout = session_attr->gsa_timeout;
    }

    /* Lock the GRPC manager */
    result = ngclExclusiveLock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the GRPC manager.\n", fName);
        goto error;
    }
    exclusiveLocked = 1;

    /* Ninf-G Session construct */
    session = ngclSessionConstruct(
        grpc_l_context, executable, &ngclSessionAttr, &error);
    if (session == NULL) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to construct the Ninf-G Session.\n", fName);
        goto error;
    }

    /* Release Attribute of Session */
    result = ngclSessionAttributeFinalize(
        grpc_l_context, executable, &ngclSessionAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Release Attribute of Session.\n", fName);
        goto error;
    }

    /* Allocate the storage for Session management data */
    session_data = globus_libc_calloc(1, sizeof (*session_data));
    if (session_data == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't allocate the storage for Session management data.\n",
            fName);
        goto error;
    }

    /* register executable ID into session */
    session_data->gsd_executableID = ngclExecutableGetID(executable, &error);
    session_data->gsd_sessionUse = GRPC_L_SESSION_USE_CALL;
    result = ngclSessionRegisterUserData(session, session_data, NULL, &error);
    if (result == 0) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to register user data to Ninf-G Session.\n", fName);
	goto error;
    }

    /* Unlock the GRPC manager */
    exclusiveLocked = 0;
    result = ngclExclusiveUnlock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the GRPC manager.\n", fName);
        goto error;
    }

    /* Session start by Pure API */
    if (argument_type == GRPC_L_ARGUMENT_TYPE_VA_LIST) {
        result = ngclSessionStartVarg(
            session, method_name, va_argument, &error);

    } else if (argument_type == GRPC_L_ARGUMENT_TYPE_ARG_STACK) {
        result = ngclSessionStartWithArgumentStack(
            session, method_name, ((ngclArgumentStack_t *)arg_stack), &error);

    } else {
        abort();
    }
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session start.\n", fName);

        /* Unregister session from Pure */
        ret = grpc_l_session_queue_register_session_list(
            &grpc_l_session_queue, session);
        if (ret != GRPC_NO_ERROR) {
            error_code = ret;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to register Session to Session Queue.\n",
                fName);
        }
        goto error;
    }

    /* Return if this call was non blocking call */
    if (wait_type == GRPC_L_CALL_WAIT_TYPE_ASYNC) {
        *sessionID = session->ngs_ID;

	/* Wait the session */
	error_code = grpc_l_session_set_not_use(session, session_data);
	if (error_code != GRPC_NO_ERROR) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to Session wait.\n", fName);
	    goto error;
	}

        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: API %s() return.\n", fName, api_name);

        /* Success */
        return error_code;
    }

    /* Wait the session */
    error_code = grpc_l_wait(
        NULL, GRPC_L_WAIT_TYPE_ALL, GRPC_L_WAIT_SET_GIVEN,
        NULL, 1, executable, session, NULL);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Session wait.\n", fName);
        goto error;
    }

    if (sessionID != NULL) {
        *sessionID = session->ngs_ID;
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;

    /* Error occurred */
error:
    ret = grpc_l_handle_set_error(&exec_handle);
    if (ret != GRPC_NO_ERROR) {
        error_code = ret;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to set error to %s.\n", fName, handle_type_name);
    }

    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return by error.\n", fName, api_name);

    /* Unlock the GRPC manager */
    if (exclusiveLocked != 0) {
	exclusiveLocked = 0;
	result = ngclExclusiveUnlock(
	    grpc_l_context, &grpc_l_manager.gm_exclusiveLock, NULL);
	if (result == 0) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
	}
    }

    return error_code;
}

/**
 * A flag is cleared during use.
 */
static grpc_error_t
grpc_l_session_set_not_use(
    ngclSession_t *session,
    grpc_l_session_data_t *session_data)
{
    grpc_error_t error_code;
    int result;
    int error;
    int exclusiveLocked = 0;
    static const char fName[] = "grpc_l_session_set_not_use";

    /* Lock the GRPC manager */
    result = ngclExclusiveLock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
	error_code = grpc_i_get_error_from_ng_error(error);
	grpc_l_error_set(error_code);
	ngclLogPrintfContext(grpc_l_context,
	    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the GRPC manager.\n", fName);
	return error_code;
    }
    exclusiveLocked = 1;

    /* A flag is cleared during use */
    session_data->gsd_sessionUse = GRPC_L_SESSION_NOT_USE;

    /* Unlock the GRPC manager */
    if (exclusiveLocked != 0) {
	exclusiveLocked = 0;
	result = ngclExclusiveUnlock(
	    grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
	if (result == 0) {
	    error_code = grpc_i_get_error_from_ng_error(error);
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
	    return error_code;
	}
    }

    return GRPC_NO_ERROR;
}

/**
 * GridRPC wait API substance
 */
grpc_error_t
grpc_l_wait(
    const char *api_name,
    grpc_l_wait_type_t wait_type,
    grpc_l_wait_set_t wait_set,
    grpc_sessionid_t *id_array,
    size_t id_length,
    ngclExecutable_t *given_executable,
    ngclSession_t *given_session,
    grpc_sessionid_t *id_ptr)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t ret = GRPC_NO_ERROR;
    int result, error;
    int ret_id;
    ngclExecutable_t *executable;
    ngclSession_t *session_list = NULL;
    ngclSession_t *session_pure_arg, *current_session;
    grpc_l_session_data_t *session_data;
    int exclusiveLocked = 0;
    static const char fName[] = "grpc_l_wait";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    if (api_name != NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: API %s() enter.\n", fName, api_name);
    }

    /* Check the arguments */
    assert((api_name == NULL) || (api_name != NULL)); /* both OK */
    assert((wait_type == GRPC_L_WAIT_TYPE_ALL) ||
           (wait_type == GRPC_L_WAIT_TYPE_ANY));
    assert((wait_set == GRPC_L_WAIT_SET_ALL) ||
           (wait_set == GRPC_L_WAIT_SET_GIVEN));

    if (wait_type == GRPC_L_WAIT_TYPE_ALL) {
        assert(id_ptr == NULL);

    } else if (wait_type == GRPC_L_WAIT_TYPE_ANY) {
        assert(given_executable == NULL);
        assert(given_session == NULL);

        if (id_ptr == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: argument is invalid."
                " unable to store the returned session. (id == NULL)\n", fName);
            return error_code;
        }
    } else {
        abort();
    }

    /* Lock the GRPC manager */
    result = ngclExclusiveLock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
	error_code = grpc_i_get_error_from_ng_error(error);
	grpc_l_error_set(error_code);
	ngclLogPrintfContext(grpc_l_context,
	    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't lock the GRPC manager.\n", fName);
	goto error;
    }
    exclusiveLocked = 1;

    session_list = NULL;
    session_pure_arg = NULL;
    executable = NULL;

    /* All sessions */
    if (wait_set == GRPC_L_WAIT_SET_ALL) {

	/* Check the arguments */
	assert(id_array == NULL);
	assert(id_length == 0);
	assert(given_executable == NULL);
	assert(given_session == NULL);

        /* Create the session list */
        session_list = grpc_l_session_get_api_list(NULL, 0, &error_code);
        if (error_code != GRPC_NO_ERROR) {
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to get Ninf-G Session.\n", fName);
                goto error;
        }
        if (session_list == NULL) {
            /* If there are no sessions, return GRPC_NO_ERROR */
            if (id_ptr != NULL) {
                *id_ptr = GRPC_SESSIONID_VOID;
            }
            goto error;
        }
        session_pure_arg = NULL;
        executable       = NULL;

    } else if (wait_set == GRPC_L_WAIT_SET_GIVEN) {

        /* Given executable and session */
        if ((given_executable != NULL) || (given_session != NULL)) {
            
            /* Check the arguments */
            assert(given_executable != NULL);
            assert(given_session != NULL);
            assert(given_session->ngs_apiNext == NULL);

            assert(id_array == NULL);
            assert(id_length == 1);

            /* Create the session list */
            session_list = given_session;
            executable = given_executable;
        } 
        
        /* Given Session ID*/
        else {

            /* Check the arguments */
            if (id_length <= 0) {
                error_code = GRPC_INVALID_SESSION_ID;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: argument is invalid."
                    " session array should be larger than 0. (length == %d)\n",
                    fName, id_length);
                goto error;
            }

             if (id_array == NULL) {
                error_code = GRPC_INVALID_SESSION_ID;
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: argument is invalid. session array is NULL\n", fName);
                goto error;
            }

            /* Create the session list */
            session_list = grpc_l_session_get_api_list(id_array, id_length,
                &error_code);
            if (session_list == NULL) {
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to get Ninf-G Session.\n", fName);
                goto error;
            }
            executable = NULL;
        }
    } else {
        abort();
    }

    /* A flag is set during use */
    for (current_session = session_list; current_session != NULL;
	current_session = current_session->ngs_apiNext) {

	/* get executable ID from Ninf-G Session */
	result = ngclSessionGetUserData(
	    current_session, (void *)&session_data, &error);
	if (result == 0) {
	    error_code = grpc_i_get_error_from_ng_error(error);
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to get user data from Ninf-G Session.\n", fName);
	    goto error;
	}
	session_data->gsd_sessionUse = GRPC_L_SESSION_USE_CALL;
    }

    /* Unlock the GRPC manager */
    exclusiveLocked = 0;
    result = ngclExclusiveUnlock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
	error_code = grpc_i_get_error_from_ng_error(error);
	grpc_l_error_set(error_code);
	ngclLogPrintfContext(grpc_l_context,
	    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't unlock the GRPC manager.\n", fName);
	goto error;
    }

    /*  Wait Sessions by Pure API */
    ret_id = -1;
    session_pure_arg = session_list;

    if (wait_type == GRPC_L_WAIT_TYPE_ALL) {
        error_code = grpc_l_wait_all(executable, session_pure_arg);

        if (api_name != NULL) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
                "%s: API %s() return.\n", fName, api_name);
        }

        return error_code;

    } else if (wait_type == GRPC_L_WAIT_TYPE_ANY) {
        result = ngclSessionWaitAny(
            grpc_l_context, executable, session_pure_arg, &error);
        ret_id = result;
        if (result < 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
        }
    } else {
        abort();
    }

    /* Wait failed */
    if (error_code != GRPC_NO_ERROR) {
        error_code = GRPC_SESSION_FAILED;
        grpc_l_error_set(error_code);

        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Session wait returned by error.\n", fName);

        /* Find the error session and inform */
        for (current_session = session_list; current_session != NULL;
            current_session = current_session->ngs_apiNext) {

            if ((current_session->ngs_status == NG_SESSION_STATUS_DONE) &&
                (current_session->ngs_error != NG_ERROR_NO_ERROR)) {

                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: The error occurred for session %d. (reason=%d)\n",
                    fName, current_session->ngs_ID, current_session->ngs_error);

                ret_id = current_session->ngs_ID;
            }
        }

        /* The error session was not found */
        if ((wait_type == GRPC_L_WAIT_TYPE_ANY) && (ret_id < 0)) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to find the error session.\n", fName);
            goto error;
        }
    }

    /* Release session list and ready to next grpc_wait_any() */
    if (wait_type == GRPC_L_WAIT_TYPE_ANY) {
        assert(ret_id >= 0);
        *id_ptr = ret_id;

        ret = grpc_l_session_release_api_list(session_list);
        if (ret != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to release Ninf-G Session.\n", fName);
            return ret;
        }

        /* Get the Ninf-G Session */
        session_list = grpc_l_session_get_api_list(&ret_id, 1, &ret);
        if (session_list == NULL) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to get Ninf-G Session %d.\n", fName, ret_id);
            return ret;
        }
    }

    /* Register session list to Session Queue */
    ret = grpc_l_session_queue_register_session_list(
        &grpc_l_session_queue, session_list);
    if (ret != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to register Session to Session Queue.\n",
            fName);
        return ret;
    }

    if (api_name != NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: API %s() return.\n", fName, api_name);
    }

    return error_code;

    /* Error occurred */
error:
    if (api_name != NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: API %s() return by error.\n", fName, api_name);
    }

    /* Unlock the GRPC manager */
    if (exclusiveLocked != 0) {
	exclusiveLocked = 0;
	result = ngclExclusiveUnlock(
	    grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
	if (result == 0) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
        }
    }

    if (session_list != NULL) {
	ret = grpc_l_session_release_api_list(session_list);
	if (ret != GRPC_NO_ERROR) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to release Ninf-G Session.\n", fName);
	}
    }

    return error_code;
}

/**
 * Wait all session.
 */
static grpc_error_t
grpc_l_wait_all(ngclExecutable_t *executable, ngclSession_t *session_list)
{
    int result;
    int error;
    grpc_error_t error_wait = GRPC_NO_ERROR;
    grpc_error_t error_other = GRPC_NO_ERROR;
    ngclSession_t *curr;
    ngclSession_t *next;
    ngclSession_t list;
    ngclSession_t *listPrev;
    ngclSession_t done;
    ngclSession_t *donePrev;
    static const char fName[] = "grpc_l_wait_all";

    for (; session_list != NULL; session_list = list.ngs_apiNext) {
        result = ngclSessionWaitAll(
            grpc_l_context, executable, session_list, &error);
        if (result == 0) {
	    /* Wait failed */
	    error_wait = GRPC_SESSION_FAILED;
	    grpc_l_error_set(error_wait);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Session wait returned by error.\n", fName);
	}

	/* The completed session is removed from list and append to done list.
	 */
	list.ngs_apiNext = NULL;
	listPrev = &list;
	done.ngs_apiNext = NULL;
	donePrev = &done;
	for (curr = session_list; curr != NULL; curr = next) {
	    next = curr->ngs_apiNext;
	    curr->ngs_apiNext = NULL;
	    if (curr->ngs_status != NG_SESSION_STATUS_DONE) {
		listPrev->ngs_apiNext = curr;
		listPrev = curr;
	    } else {
		donePrev->ngs_apiNext = curr;
		donePrev = curr;

		/* Find the error session and inform */
		if (curr->ngs_error != NG_ERROR_NO_ERROR) {
		    ngclLogPrintfContext(grpc_l_context,
			NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
			"%s: The error occurred for session %d. (reason=%d)\n",
			fName, curr->ngs_ID, curr->ngs_error);
		}
	    }
	}

	/* Is no session done? */
	if (done.ngs_apiNext == NULL) {
	    ngclLogPrintfContext(grpc_l_context, NG_LOG_CATEGORY_NINFG_GRPC,
		NG_LOG_LEVEL_ERROR, NULL, "%s: No session is done.\n", fName);
	    return GRPC_OTHER_ERROR_CODE;
	}

	/* Register session list to Session Queue */
	error_other = grpc_l_session_queue_register_session_list(
	    &grpc_l_session_queue, done.ngs_apiNext);
	if (error_other != GRPC_NO_ERROR) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to register Session to Session Queue.\n",
		fName);
	}
    }

    return (error_wait != GRPC_NO_ERROR) ? error_wait : error_other;
}

/**
 * GridRPC cancel API substance
 */
grpc_error_t
grpc_l_cancel(
    const char *api_name,
    grpc_sessionid_t *sessionIds,
    size_t length)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t ret = GRPC_NO_ERROR;
    ngclSession_t *session = NULL;
    int error, result;
    static const char fName[] = "grpc_l_cancel";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Get Ninf-G Session from Session ID */
    session = grpc_l_session_get_cancel_list(sessionIds, length, &error_code);
    if (session == NULL) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Ninf-G Session %d.\n", fName);
        goto error;
    }

    /* Cancel session by Pure API */
    result = ngclSessionCancel(grpc_l_context, NULL, session, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to cancel session.\n", fName);

        /* Release Ninf-G Session */
        ret = grpc_l_session_release_cancel_list(session);
        if (ret != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to release Ninf-G Session.\n", fName);
            error_code = ret;
            grpc_l_error_set(error_code);
        }

        goto error;
    }

    /* Release Ninf-G Session */
    error_code = grpc_l_session_release_cancel_list(session);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Ninf-G Session.\n", fName);
        goto error;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    /* Success */
    return error_code;

    /* Error occurred */
error:
    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return by error.\n", fName, api_name);

    return error_code;
}

/**
 * Get Ninf-G Executable from Session
 */
ngclExecutable_t *
grpc_l_executable_get(
    ngclSession_t *session,
    grpc_error_t *error_code)
{
    ngclExecutable_t *executable = NULL;
    int error;
    static const char fName[] = "grpc_l_executable_get";

    /* Check the arguments */
    if (session == NULL) {
        *error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(*error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument is not valid.\n", fName);
        return NULL;
    }

    /* Get Ninf-G Executable from Session */
    executable = ngclSessionGetExecutable(session, &error);
    if (executable == NULL) {
        *error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(*error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Ninf-G Executable from Ninf-G Session.\n",
            fName);
        return NULL;
    }

    return executable;
}

/**
 * Session attribute initialize.
 */
grpc_error_t
grpc_l_session_attr_initialize(
    const char *api_name,
    grpc_function_handle_t *function_handle,
    grpc_object_handle_t_np *object_handle,
    grpc_l_handle_type_t handle_type,
    grpc_session_attr_t_np *attr)
{
    int result, error;
    ngclSessionAttribute_t sessionAttr;
    grpc_l_executable_handle_t exec_handle;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_attr_initialize";

    /* Check whether initialize was already performed. */
    error_code = grpc_l_initialize_performed_check();
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(NULL,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Not initialized.\n", fName);
        return error_code;
    }

    /* Check the arguments */
    assert(api_name != NULL);

    assert((handle_type == GRPC_L_HANDLE_TYPE_FUNCTION) ||
           (handle_type == GRPC_L_HANDLE_TYPE_OBJECT));

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() enter.\n", fName, api_name);

    /* Get executable handle */
    error_code = grpc_l_executable_handle_convert_with_check(
        &exec_handle, handle_type, function_handle, object_handle);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get the handle.\n", fName);
        return error_code;
    }

    /* Check the attr */
    if (attr == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Handle attribute is not valid.\n", fName);
        return error_code;
    }

    /* Get the Attribute of Session */
    result = ngclSessionAttributeInitialize(
        grpc_l_context, *(exec_handle.geh_exec), &sessionAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Get Attribute of Session.\n", fName);
        return error_code;
    }

    /* Initialize member */
    attr->gsa_waitArgTransfer = sessionAttr.ngsa_waitArgumentTransfer;
    attr->gsa_timeout = sessionAttr.ngsa_timeout;

    /* Release Attribute of Session */
    result = ngclSessionAttributeFinalize(
        grpc_l_context, *(exec_handle.geh_exec), &sessionAttr, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to Release Attribute of Session.\n", fName);
        return error_code;
    }

    /* log */
    ngclLogPrintfContext(grpc_l_context,
        NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: API %s() return.\n", fName, api_name);

    return error_code;
}

/**
 * Get Session from Session ID
 * Note: if sessionIds is NULL, this function returns list of all sessions.
 * When there are no sessions, this function returns NULL and sets error_code
 * to GRPC_NO_ERROR.
 */
ngclSession_t *
grpc_l_session_get_api_list(
    grpc_sessionid_t *sessionIds,
    size_t length,
    grpc_error_t *error_code)
{
    ngclSession_t *session = NULL;
    int error;
    static const char fName[] = "grpc_l_session_get_api_list";

    assert(error_code != NULL);

    *error_code = GRPC_NO_ERROR;

    /* Get list of Ninf-G Session from Session ID by Pure API */
    session = ngclSessionGetList(grpc_l_context, NULL, sessionIds, length,
        &error);
    if ((session == NULL) &&
        (error != NG_ERROR_NO_ERROR)) {
        if (error == NG_ERROR_NOT_EXIST) {
            *error_code = GRPC_INVALID_SESSION_ID;
        } else {
            *error_code = grpc_i_get_error_from_ng_error(error);
        }
        grpc_l_error_set(*error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Ninf-G Session.\n", fName);
        return NULL;
    }

    return session;
}

/**
 * Release Session
 */
grpc_error_t
grpc_l_session_release_api_list(
    ngclSession_t *session)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_l_session_release_api_list";

    /* Release list of Ninf-G Session by Pure API */
    result = ngclSessionReleaseList(session, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Ninf-G Session.\n", fName);

        return error_code;
    }

    return error_code;
}

/**
 * Get Canceling session from Session ID
 */
ngclSession_t *
grpc_l_session_get_cancel_list(
    grpc_sessionid_t *sessionIds,
    size_t length,
    grpc_error_t *error_code)
{
    ngclSession_t *session = NULL;
    int error;
    static const char fName[] = "grpc_l_session_get_cancel_list";

    /* Get Canceling list of Ninf-G Session from Session ID by Pure API */
    session = ngclSessionGetCancelList(
        grpc_l_context, NULL, sessionIds, length, &error);
    if (session == NULL) {
        *error_code = GRPC_INVALID_SESSION_ID;
        grpc_l_error_set(*error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Ninf-G Session.\n", fName);
        return NULL;
    }

    return session;
}

/**
 * Release Canceling Session
 */
grpc_error_t
grpc_l_session_release_cancel_list(
    ngclSession_t *session)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_l_session_release_cancel_list";

    /* Release list of Ninf-G Session by Pure API */
    result = ngclSessionReleaseCancelList(session, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Ninf-G Session.\n", fName);

        return error_code;
    }

    return error_code;
}

/**
 * Get Session Infomation from Session
 */
grpc_error_t
grpc_l_session_info_get(
    grpc_l_session_info_get_mode_t info_get_mode,
    grpc_l_session_info_require_t require,
    grpc_sessionid_t sessionId,
    grpc_session_info_t_np *info,
    int *status)
{
    int locked;
    struct timeval tv;
    ngclSession_t *session;
    grpc_exec_info_executable_t_np *info_grpc_exe;
    grpc_exec_info_client_t_np *info_grpc_cli;
    ngSessionInformationClient_t *info_pure_cli_real, *info_pure_cli_cput;
    ngSessionInformationExecutable_t *info_pure_exe_real, *info_pure_exe_cput;
    grpc_l_session_queue_t *session_queue;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    grpc_l_session_queue_get_mode_t get_mode;
    static const char fName[] = "grpc_l_session_info_get";

    locked = 0;
    session_queue = &grpc_l_session_queue;
    session = NULL;
    get_mode = GRPC_L_SESSION_QUEUE_GET_UNDEFINED;
    info_grpc_exe = NULL;
    info_grpc_cli = NULL;
    info_pure_exe_real = NULL;
    info_pure_exe_cput = NULL;
    info_pure_cli_real = NULL;
    info_pure_cli_cput = NULL;

    /* Check the arguments */
    if (status == NULL) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Argument is not valid.\n", fName);
        goto error;
    }

    /* Lock the Session Queue */
    error_code = grpc_l_session_queue_read_lock(session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read lock the Session Queue.\n", fName);
        goto error;
    }
    locked = 1;

    if (info_get_mode == GRPC_L_SESSION_INFO_GET_BY_ID) {
        get_mode = GRPC_L_SESSION_QUEUE_GET_BY_ID;
    } else if (info_get_mode == GRPC_L_SESSION_INFO_GET_LAST) {
        get_mode = GRPC_L_SESSION_QUEUE_GET_LAST;
    } else {
        abort();
    }

    /* Get the Session from Session ID (Session Queue) */
    session = grpc_l_session_queue_get_session(
        session_queue, get_mode, sessionId);
    if (session == NULL) {
        /* Get Session from Session ID by Pure API */
        session = grpc_l_session_get_api_list(&sessionId, 1, &error_code);
        if (session == NULL) {
            *status = GRPC_SESSION_UNKNOWN_STATUS;
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: No such Session of Session ID.\n", fName);
            goto error;
        }
    }

    /* Check Session status */
    if (session->ngs_status == NG_SESSION_STATUS_INITIALIZED) {
        *status = GRPC_SESSION_ARG_IS_NOT_TRANSMITED;
    } else if (session->ngs_status == NG_SESSION_STATUS_CALCULATE_EXECUTING) {
        *status = GRPC_SESSION_EXECUTING;
    } else if ((session->ngs_error != NG_ERROR_NO_ERROR) ||
        (session->ngs_heartBeatStatus == NG_HEART_BEAT_STATUS_ERROR)) {
        *status = GRPC_SESSION_DOWN;
    } else if (session->ngs_status == NG_SESSION_STATUS_DONE) {
        *status = GRPC_SESSION_DONE;
    } else {
        *status = GRPC_SESSION_UNKNOWN_STATUS;
    }

    if (info == NULL) {
        goto finish;
    }

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    /* Set value 0 */
    /* Measured by Executable */
    info_grpc_exe = &info->gei_measureExecutable;
    info_grpc_cli = &info->gei_measureClient;

    info_grpc_exe->callbackNtimesCalled = 0;

    info_grpc_exe->transferArgumentToRemoteRealTime = tv;
    info_grpc_exe->transferArgumentToRemoteCpuTime  = tv;
    info_grpc_exe->transferFileToRemoteRealTime     = tv;
    info_grpc_exe->transferFileToRemoteCpuTime      = tv;
    info_grpc_exe->calculationRealTime              = tv;
    info_grpc_exe->calculationCpuTime               = tv;
    info_grpc_exe->transferResultToClientRealTime   = tv;
    info_grpc_exe->transferResultToClientCpuTime    = tv;
    info_grpc_exe->transferFileToClientRealTime     = tv;
    info_grpc_exe->transferFileToClientCpuTime      = tv;

    info_grpc_exe->callbackTransferArgumentToClientRealTime = tv;
    info_grpc_exe->callbackTransferArgumentToClientCpuTime  = tv;
    info_grpc_exe->callbackCalculationRealTime              = tv;
    info_grpc_exe->callbackCalculationCpuTime               = tv;
    info_grpc_exe->callbackTransferResultToRemoteRealTime   = tv;
    info_grpc_exe->callbackTransferResultToRemoteCpuTime    = tv;

    /* Measured by Client */
    info_grpc_cli->callbackNtimesCalled = 0;

    info_grpc_cli->remoteMachineInfoRequestRealTime = tv;
    info_grpc_cli->remoteMachineInfoRequestCpuTime  = tv;
    info_grpc_cli->remoteClassInfoRequestRealTime   = tv;
    info_grpc_cli->remoteClassInfoRequestCpuTime    = tv;
    info_grpc_cli->gramInvokeRealTime               = tv;
    info_grpc_cli->gramInvokeCpuTime                = tv;

    info_grpc_cli->transferArgumentToRemoteRealTime = tv;
    info_grpc_cli->transferArgumentToRemoteCpuTime  = tv;
    info_grpc_cli->calculationRealTime              = tv;
    info_grpc_cli->calculationCpuTime               = tv;
    info_grpc_cli->transferResultToClientRealTime   = tv;
    info_grpc_cli->transferResultToClientCpuTime    = tv;

    info_grpc_cli->callbackTransferArgumentToClientRealTime = tv;
    info_grpc_cli->callbackTransferArgumentToClientCpuTime  = tv;
    info_grpc_cli->callbackCalculationRealTime              = tv;
    info_grpc_cli->callbackCalculationCpuTime               = tv;
    info_grpc_cli->callbackTransferResultToRemoteRealTime   = tv;
    info_grpc_cli->callbackTransferResultToRemoteCpuTime    = tv;

    info_grpc_exe = &info->gei_measureExecutable;
    info_grpc_cli = &info->gei_measureClient;
    info_pure_exe_real = &session->ngs_info.ngsi_executableRealTime;
    info_pure_exe_cput = &session->ngs_info.ngsi_executableCPUtime;
    info_pure_cli_real = &session->ngs_info.ngsi_clientRealTime;
    info_pure_cli_cput = &session->ngs_info.ngsi_clientCPUtime;

    /* Compression Information */
    info->gei_compressionInformation.nElements = 0;
    info->gei_compressionInformation.toRemote = NULL;
    info->gei_compressionInformation.toClient = NULL;

    /* Set value Session Information*/
    /* Measured by Executable */
    info_grpc_exe->callbackNtimesCalled
	= session->ngs_info.ngsi_executableCallbackNtimesCalled;
    info_grpc_exe->transferArgumentToRemoteRealTime
        = info_pure_exe_real->ngsie_transferArgument;
    info_grpc_exe->transferArgumentToRemoteCpuTime
        = info_pure_exe_cput->ngsie_transferArgument;
    info_grpc_exe->transferFileToRemoteRealTime
        = info_pure_exe_real->ngsie_transferFileClientToRemote;
    info_grpc_exe->transferFileToRemoteCpuTime
        = info_pure_exe_cput->ngsie_transferFileClientToRemote;
    info_grpc_exe->calculationRealTime
        = info_pure_exe_real->ngsie_calculation;
    info_grpc_exe->calculationCpuTime
        = info_pure_exe_cput->ngsie_calculation;
    info_grpc_exe->transferResultToClientRealTime
        = info_pure_exe_real->ngsie_transferResult;
    info_grpc_exe->transferResultToClientCpuTime
        = info_pure_exe_cput->ngsie_transferResult;
    info_grpc_exe->transferFileToClientRealTime
        = info_pure_exe_real->ngsie_transferFileRemoteToClient;
    info_grpc_exe->transferFileToClientCpuTime
        = info_pure_exe_cput->ngsie_transferFileRemoteToClient;

    info_grpc_exe->callbackTransferArgumentToClientRealTime
        = info_pure_exe_real->ngsie_callbackTransferArgument;
    info_grpc_exe->callbackTransferArgumentToClientCpuTime
        = info_pure_exe_cput->ngsie_callbackTransferArgument;
    info_grpc_exe->callbackCalculationRealTime
        = info_pure_exe_real->ngsie_callbackCalculation;
    info_grpc_exe->callbackCalculationCpuTime
        = info_pure_exe_cput->ngsie_callbackCalculation;
    info_grpc_exe->callbackTransferResultToRemoteRealTime
        = info_pure_exe_real->ngsie_callbackTransferResult;
    info_grpc_exe->callbackTransferResultToRemoteCpuTime
        = info_pure_exe_cput->ngsie_callbackTransferResult;

    /* Measured by Client */
    info_grpc_cli->callbackNtimesCalled
	= session->ngs_info.ngsi_clientCallbackNtimesCalled;

    info_grpc_cli->remoteMachineInfoRequestRealTime
        = info_pure_cli_real->ngsic_queryRemoteMachineInformation;
    info_grpc_cli->remoteMachineInfoRequestCpuTime
        = info_pure_cli_cput->ngsic_queryRemoteMachineInformation;
    info_grpc_cli->remoteClassInfoRequestRealTime
        = info_pure_cli_real->ngsic_queryRemoteClassInformation;
    info_grpc_cli->remoteClassInfoRequestCpuTime
        = info_pure_cli_cput->ngsic_queryRemoteClassInformation;
    info_grpc_cli->gramInvokeRealTime
        = info_pure_cli_real->ngsic_invokeExecutable;
    info_grpc_cli->gramInvokeCpuTime
        = info_pure_cli_cput->ngsic_invokeExecutable;

    info_grpc_cli->transferArgumentToRemoteRealTime
        = info_pure_cli_real->ngsic_transferArgument;
    info_grpc_cli->transferArgumentToRemoteCpuTime
        = info_pure_cli_cput->ngsic_transferArgument;
    info_grpc_cli->calculationRealTime
        = info_pure_cli_real->ngsic_calculation;
    info_grpc_cli->calculationCpuTime
        = info_pure_cli_cput->ngsic_calculation;
    info_grpc_cli->transferResultToClientRealTime
        = info_pure_cli_real->ngsic_transferResult;
    info_grpc_cli->transferResultToClientCpuTime
        = info_pure_cli_cput->ngsic_transferResult;

    info_grpc_cli->callbackTransferArgumentToClientRealTime
        = info_pure_cli_real->ngsic_callbackTransferArgument;
    info_grpc_cli->callbackTransferArgumentToClientCpuTime
        = info_pure_cli_cput->ngsic_callbackTransferArgument;
    info_grpc_cli->callbackCalculationRealTime
        = info_pure_cli_real->ngsic_callbackCalculation;
    info_grpc_cli->callbackCalculationCpuTime
        = info_pure_cli_cput->ngsic_callbackCalculation;
    info_grpc_cli->callbackTransferResultToRemoteRealTime
        = info_pure_cli_real->ngsic_callbackTransferResult;
    info_grpc_cli->callbackTransferResultToRemoteCpuTime
        = info_pure_cli_cput->ngsic_callbackTransferResult;

    /* Session Information */
    if (require == GRPC_L_SESSION_INFO_REQUIRE_ALL) {
	error_code = grpc_l_session_info_get_from_session(session, info);
	if (error_code != GRPC_NO_ERROR) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't get the Session Information.\n", fName);
	    goto error;
	}
    }

finish:
    /* Unlock the Session Queue */
    error_code = grpc_l_session_queue_read_unlock(session_queue);
    locked = 0;
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't read unlock the Session Queue.\n", fName);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_read_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't read unlock the Session Queue.\n", fName);
        }
    }

    return return_code;
}

/**
 * Get Session Infomation from Session
 */
grpc_error_t
grpc_l_session_info_get_from_session(
    ngclSession_t *session,
    grpc_session_info_t_np *info)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_info_get_from_session";

    /* Check the arguments */
    assert(session != NULL);
    assert(info != NULL);

    /* Initialize the variables */
    info->gei_compressionInformation.nElements = 0;
    info->gei_compressionInformation.toRemote = NULL;
    info->gei_compressionInformation.toClient = NULL;

    /* Allocate the storage for Compression Information */
    if (session->ngs_info.ngsi_nCompressionInformations > 0) {
        info->gei_compressionInformation.toRemote = globus_libc_calloc(
            session->ngs_info.ngsi_nCompressionInformations,
            sizeof (grpc_compression_info_t_np));
        if (info->gei_compressionInformation.toRemote == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Compression Information.\n",
                fName);
            goto error;
        }
    }

    if (session->ngs_info.ngsi_nCompressionInformations > 0) {
        info->gei_compressionInformation.toClient = globus_libc_calloc(
            session->ngs_info.ngsi_nCompressionInformations,
            sizeof (grpc_compression_info_t_np));
        if (info->gei_compressionInformation.toClient == NULL) {
            error_code = GRPC_OTHER_ERROR_CODE;
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't allocate the storage for Compression Information.\n",
                fName);
            goto error;
        }
    }

    info->gei_compressionInformation.nElements =
	session->ngs_info.ngsi_nCompressionInformations;

    /* Copy the Session Information */
    grpc_l_session_info_copy(session, info);

    /* Success */
    return error_code;

error:
    if (info->gei_compressionInformation.toClient != NULL)
	globus_libc_free(info->gei_compressionInformation.toClient);
    info->gei_compressionInformation.toClient = NULL;

    if (info->gei_compressionInformation.toRemote != NULL)
	globus_libc_free(info->gei_compressionInformation.toRemote);
    info->gei_compressionInformation.toRemote = NULL;

    return error_code;
}

/**
 * Copy Session Infomation from Session.
 */
static void
grpc_l_session_info_copy(
    ngclSession_t *session,
    grpc_session_info_t_np *info)
{
    int i;
    ngCompressionInformation_t *compInfo;

    compInfo = session->ngs_info.ngsi_compressionInformation;
    for (i = 0; i < session->ngs_info.ngsi_nCompressionInformations; i++) {
	/* to remote */
	grpc_l_session_info_copy_element(
	    &compInfo[i].ngci_in.ngcic_compression,
	    &compInfo[i].ngci_in.ngcic_decompression,
	    &info->gei_compressionInformation.toRemote[i]);

	/* to client */
	grpc_l_session_info_copy_element(
	    &compInfo[i].ngci_out.ngcic_compression,
	    &compInfo[i].ngci_out.ngcic_decompression,
	    &info->gei_compressionInformation.toClient[i]);
    }
}

/**
 * Copy Session Infomation Element.
 */
static void
grpc_l_session_info_copy_element(
    ngCompressionInformationElement_t *comp,
    ngCompressionInformationElement_t *decomp,
    grpc_compression_info_t_np *info)
{
    info->valid = 0;

    /* Compression */
    if (comp->ngcie_measured != 0) {
	info->valid = 1;
	info->originalNbytes = comp->ngcie_lengthRaw;
	info->compressionNbytes = comp->ngcie_lengthCompressed;
	info->compressionRealTime = comp->ngcie_executionRealTime;
	info->compressionCpuTime = comp->ngcie_executionCPUtime;
    }

    /* Decompression */
    if (decomp->ngcie_measured != 0) {
	info->valid = 1;
	info->originalNbytes = comp->ngcie_lengthRaw;
	info->compressionNbytes = comp->ngcie_lengthCompressed;
	info->decompressionRealTime = decomp->ngcie_executionRealTime;
	info->decompressionCpuTime = decomp->ngcie_executionCPUtime;
    }
}

/**
 * Session Queue Initialize.
 */
grpc_error_t
grpc_l_session_queue_initialize(
    grpc_l_session_queue_t *session_queue)
{
    int threshold;
    int error, result;
    ngclLocalMachineInformation_t lmInfo;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_initialize";

    /* Check the arguments */
    assert(session_queue != NULL);

    threshold = 0;

    grpc_l_session_queue_initialize_member(session_queue);

    /* Initialize completed session list */
    session_queue->gsq_list_head = NULL;
    session_queue->gsq_list_tail = session_queue->gsq_list_head;

    /* Initialize count of completed session list */
    session_queue->gsq_nSessions = 0;

    /* Initialize the RWlock */
    result = ngclRWlockInitialize(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't initialize the Read/Write Lock.\n", fName);
        return error_code;
    }

    /* Get Local Machine Information */
    result = ngclLocalMachineInformationGetCopy(
        grpc_l_context, &lmInfo, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to get Local Machine Information.\n", fName);
        return error_code;
    }

    /* Get the threshold */
    threshold = lmInfo.nglmi_saveNsessions;

    /* Release Local Machine Information */
    result = ngclLocalMachineInformationRelease(
        grpc_l_context, &lmInfo, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to release Local Machine Information.\n", fName);
        return error_code;
    }

    error_code = grpc_l_session_queue_set_threshold(
        session_queue, threshold);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to set the threshold.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Finalize.
 */
grpc_error_t
grpc_l_session_queue_finalize(
    grpc_l_session_queue_t *session_queue)
{
    int error, result;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_finalize";

    /* Check the arguments */
    assert(session_queue != NULL);

    /* Unregister all sessions */
    error_code = grpc_l_session_queue_unregister_session(
        session_queue, 1, GRPC_L_SESSION_QUEUE_REMOVE_ALL, 0, 0);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister Ninf-G Session.\n", fName);
        return error_code;
    }

    /* Finalize the RWlock */
    result = ngclRWlockFinalize(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't finalize the Read/Write Lock.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Initialize Member.
 */
void
grpc_l_session_queue_initialize_member(
    grpc_l_session_queue_t *session_queue)
{
    /* Check the arguments */
    assert(session_queue != NULL);

    /* Initialize the variable */
    session_queue->gsq_list_head = NULL;
    session_queue->gsq_list_tail = NULL;
    session_queue->gsq_nSessions = 0;
    session_queue->gsq_type = GRPC_L_SESSION_QUEUE_TYPE_UNDEFINED;
    session_queue->gsq_threshold = 0;
}

/**
 * Session Queue Read Lock.
 */
grpc_error_t
grpc_l_session_queue_read_lock(
    grpc_l_session_queue_t *session_queue)
{
    int error, result;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_read_lock";

    /* Check the arguments */
    assert(session_queue != NULL);

    /* Lock the Session Queue */
    result = ngclRWlockReadLock(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the Session Queue.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Read Unlock.
 */
grpc_error_t
grpc_l_session_queue_read_unlock(
    grpc_l_session_queue_t *session_queue)
{
    int error, result;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_read_unlock";

    /* Check the arguments */
    assert(session_queue != NULL);

    /* Unlock the Session Queue */
    result = ngclRWlockReadUnlock(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the Session Queue.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Write Lock.
 */
grpc_error_t
grpc_l_session_queue_write_lock(
    grpc_l_session_queue_t *session_queue)
{
    int error, result;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_write_lock";

    /* Check the arguments */
    assert(session_queue != NULL);

    /* Write Lock the Session Queue */
    result = ngclRWlockWriteLock(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Session Queue.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Write Unlock.
 */
grpc_error_t
grpc_l_session_queue_write_unlock(
    grpc_l_session_queue_t *session_queue)
{
    int error, result;
    grpc_error_t error_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_write_unlock";

    /* Check the arguments */
    assert(session_queue != NULL);

    /* Unlock the Session Queue */
    result = ngclRWlockWriteUnlock(
        grpc_l_context, &session_queue->gsq_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Session Queue.\n", fName);
        return error_code;
    }

    /* Success */
    return error_code;
}

/**
 * Session Queue Set Threshold.
 */
grpc_error_t
grpc_l_session_queue_set_threshold(
    grpc_l_session_queue_t *session_queue,
    int threshold)
{
    int locked;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    static const char fName[] = "grpc_l_session_queue_set_threshold";

    /* Check the arguments */
    assert(session_queue != NULL);

    locked = 0;

    /* Write Lock the Session Queue */
    error_code = grpc_l_session_queue_write_lock(session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Session Queue.\n", fName);
        return error_code;
    }
    locked = 1;

    if (threshold > 0) {
        session_queue->gsq_type = GRPC_L_SESSION_QUEUE_TYPE_THRESHOLD;
        session_queue->gsq_threshold = threshold;

    } else if (threshold == 0) {
        session_queue->gsq_type = GRPC_L_SESSION_QUEUE_TYPE_NOT_SAVE;
        session_queue->gsq_threshold = 0;

    } else if (threshold < 0) {
        session_queue->gsq_type = GRPC_L_SESSION_QUEUE_TYPE_SAVE_ALL;
        session_queue->gsq_threshold = -1;
    }

    /* Unregister unnecessary sessions */
    error_code = grpc_l_session_queue_maintain_threshold(session_queue, 0);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister Ninf-G Session.\n", fName);
        goto error;
    }

    /* Write Unlock the Session Queue */
    error_code = grpc_l_session_queue_write_unlock(session_queue);
    locked = 0;
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Session Queue.\n", fName);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
        }
    }

    return return_code;
}

/**
 * Session Queue Maintain Threshold.
 */
grpc_error_t
grpc_l_session_queue_maintain_threshold(
    grpc_l_session_queue_t *session_queue,
    int require_lock)
{
    int locked;
    int remove_remain_count;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    grpc_l_session_queue_unregister_mode_t remove_mode;
    static const char fName[] = "grpc_l_session_queue_maintain_threshold";

    /* Check the arguments */
    assert(session_queue != NULL);

    remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_UNDEFINED;
    remove_remain_count = 0;
    locked = 0;

    /* Write Lock the Session Queue */
    if (require_lock != 0) {
        error_code = grpc_l_session_queue_write_lock(session_queue);
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write lock the Session Queue.\n", fName);
            return error_code;
        }
        locked = 1;
    }

    /* Set the parameters */
    switch (session_queue->gsq_type) {
    case GRPC_L_SESSION_QUEUE_TYPE_THRESHOLD:
        remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_TO_COUNT;
        remove_remain_count = session_queue->gsq_threshold;
        break;
        
    case GRPC_L_SESSION_QUEUE_TYPE_NOT_SAVE:
        remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_ALL;
        remove_remain_count = 0;
        break;

    case GRPC_L_SESSION_QUEUE_TYPE_SAVE_ALL:
        remove_mode = GRPC_L_SESSION_QUEUE_REMOVE_NONE;
        remove_remain_count = -1;
        break;

    default:
        abort();
    }

    /* Unregister unnecessary sessions */
    error_code = grpc_l_session_queue_unregister_session(
        session_queue, 0, remove_mode, remove_remain_count, 0);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister Ninf-G Session.\n", fName);
        goto error;
    }

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
            goto error;
        }
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
        }
    }

    return return_code;
}

/**
 * Session Queue Register Session List.
 */
grpc_error_t
grpc_l_session_queue_register_session_list(
    grpc_l_session_queue_t *session_queue,
    ngclSession_t *session_list)
{
    int result, error;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    ngclSession_t *current_session;
    grpc_l_session_data_t *session_data;
    int exclusiveLocked = 0;
    static const char fName[] = "grpc_l_session_queue_register_session_list";

    /* Check the arguments */
    assert(session_queue != NULL);

    current_session = NULL;

    /* Lock the GRPC manager */
    result = ngclExclusiveLock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the GRPC manager.\n", fName);
        goto error;
    }
    exclusiveLocked = 1;

    /* Unregist Ninf-G Session from Ninf-G Executable */
    for (current_session = session_list; current_session != NULL;
	current_session = current_session->ngs_apiNext) {

        result = ngclSessionUnregister(current_session, &error);
        if (result == 0) {
            error_code = grpc_i_get_error_from_ng_error(error);
            grpc_l_error_set(error_code);
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Failed to Unregist Session from Ninf-G Executable.\n",
                fName);
            goto error;
        }

	/* get executable ID from Ninf-G Session */
	result = ngclSessionGetUserData(
	    current_session, (void *)&session_data, &error);
	if (result == 0) {
	    error_code = grpc_i_get_error_from_ng_error(error);
	    grpc_l_error_set(error_code);
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Failed to get user data from Ninf-G Session.\n", fName);
	    goto error;
	}
	/* A flag is cleared during use */
	session_data->gsd_sessionUse = GRPC_L_SESSION_NOT_USE;
    }

    /* Unlock the GRPC manager */
    exclusiveLocked = 0;
    result = ngclExclusiveUnlock(
	grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the GRPC manager.\n", fName);
        goto error;
    }

    /* Register session list to Session Queue */
    error_code = grpc_l_session_queue_register_session_list_without_unregister(
        &grpc_l_session_queue, session_list);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to register Session to Session Queue.\n",
            fName);
        return error_code;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Unlock the GRPC manager */
    if (exclusiveLocked != 0) {
	exclusiveLocked = 0;
	result = ngclExclusiveUnlock(
	    grpc_l_context, &grpc_l_manager.gm_exclusiveLock, &error);
	if (result == 0) {
	    ngclLogPrintfContext(grpc_l_context,
		NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't unlock the GRPC manager.\n", fName);
	}
    }

    return return_code;
}

/**
 * Session Queue Register Session List.
 */
static grpc_error_t
grpc_l_session_queue_register_session_list_without_unregister(
    grpc_l_session_queue_t *session_queue,
    ngclSession_t *session_list)
{
    int locked, session_count;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    ngclSession_t *current_session, *queue_tail;
    static const char fName[] =
	"grpc_l_session_queue_register_session_list_without_unregister";

    /* Check the arguments */
    assert(session_queue != NULL);

    locked = 0;
    current_session = NULL;
    queue_tail = NULL;

    /* Write Lock the Session Queue */
    error_code = grpc_l_session_queue_write_lock(session_queue);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write lock the Session Queue.\n", fName);
        goto error;
    }
    locked = 1;

    /* Register the Session List to Session Queue */
    if (session_queue->gsq_list_head == NULL) {
        session_queue->gsq_list_head = session_list;
    } else {
        session_queue->gsq_list_tail->ngs_apiNext = session_list;
    }

    /* Fix the list_tail and nSessions */
    session_count = 0;
    queue_tail = NULL;
    current_session = session_queue->gsq_list_head;
    while (current_session != NULL) {
        session_count++;
        queue_tail = current_session;
        current_session = current_session->ngs_apiNext;
    }
    session_queue->gsq_nSessions = session_count;
    session_queue->gsq_list_tail = queue_tail;

    /* Unregister unnecessary sessions */
    error_code = grpc_l_session_queue_maintain_threshold(session_queue, 0);
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Failed to unregister Ninf-G Session.\n", fName);
        goto error;
    }

    /* Write Unlock the Session Queue */
    error_code = grpc_l_session_queue_write_unlock(session_queue);
    locked = 0;
    if (error_code != GRPC_NO_ERROR) {
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't write unlock the Session Queue.\n", fName);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        locked = 0;
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
        }
    }

    return return_code;
}

/**
 * Session Queue Unregister Sessions.
 */
grpc_error_t
grpc_l_session_queue_unregister_session(
    grpc_l_session_queue_t *session_queue,
    int require_lock,
    grpc_l_session_queue_unregister_mode_t remove_mode,
    int remain_count,
    grpc_sessionid_t sessionID)
{
    int removed;
    int result, error, locked;
    ngclSession_t *queue_tail;
    grpc_error_t error_code = GRPC_NO_ERROR;
    grpc_error_t return_code = GRPC_NO_ERROR;
    int session_count, loop_continue, remove_this;
    ngclSession_t **current_session_ptr, *current_session, *next_session;
    int no_remove, remove_by_sessionid, remove_by_count, remove_remain_count;
    static const char fName[] = "grpc_l_session_queue_unregister_session";

    /* Check the arguments */
    assert(session_queue != NULL);

    locked = 0;
    removed = 0;
    no_remove = 0;
    remove_by_sessionid = 0;
    remove_by_count = 0;
    remove_remain_count = 0;

    switch(remove_mode) {
    case GRPC_L_SESSION_QUEUE_REMOVE_BY_SESSIONID:
        remove_by_sessionid = 1;
        break;

    case GRPC_L_SESSION_QUEUE_REMOVE_TO_COUNT:
        remove_by_count = 1;
        remove_remain_count = remain_count;
        break;

    case GRPC_L_SESSION_QUEUE_REMOVE_ALL:
        remove_by_count = 1;
        remove_remain_count = 0;
        break;

    case GRPC_L_SESSION_QUEUE_REMOVE_NONE:
        no_remove = 1;
        break;

    default:
        abort();
    }

    if (no_remove != 0) {
        /* Success */
        return error_code;
    }

    /* Write Lock the Session Queue */
    if (require_lock != 0) {
        error_code = grpc_l_session_queue_write_lock(session_queue);
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write lock the Session Queue.\n", fName);
            goto error;
        }
        locked = 1;
    }

    /* Count the session */
    session_count = 0;
    current_session = session_queue->gsq_list_head;
    while (current_session != NULL) {
        session_count++;
        current_session = current_session->ngs_apiNext;
    }
    
    /* The loop for Session Remove */
    loop_continue = 1;
    current_session_ptr = &session_queue->gsq_list_head;
    current_session = *current_session_ptr;
    while (loop_continue != 0) {
        remove_this = 0;
        next_session = NULL;

        /* Check finished */
        if (current_session == NULL) {
            loop_continue = 0;
            break;
        }
        if (remove_by_count != 0) {
            if (session_count <= remove_remain_count) {
                loop_continue = 0;
                break;
            } else {
                remove_this = 1;
            }
        } else if (remove_by_sessionid != 0) {
            if (current_session->ngs_ID == sessionID) {
                remove_this = 1;
            }
        } else {
            abort();
        }
        next_session = current_session->ngs_apiNext;

        /* Unregister from the Session Queue */
        if (remove_this != 0) {

            /* Finalize Ninf-G Session */
            result = ngclSessionFinalize(current_session, &error);
            if (result == 0) {
                error_code = grpc_i_get_error_from_ng_error(error);
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to finalize Ninf-G session.\n", fName);
                /* Not Return */
            }
         
            /* Free Ninf-G Session */
            result = ngclSessionFree(current_session, &error);
            if (result == 0) {
                error_code = grpc_i_get_error_from_ng_error(error);
                grpc_l_error_set(error_code);
                ngclLogPrintfContext(grpc_l_context,
                    NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Failed to free Ninf-G session.\n", fName);
                /* Not Return */
            }

            removed = 1;
            current_session = NULL;

            /* Treat the list */
            *current_session_ptr = next_session;

            session_queue->gsq_nSessions--;
            session_count--;
        }

        /* Get Next */
        if (remove_by_count != 0) {
            current_session_ptr = &session_queue->gsq_list_head;
            current_session = *current_session_ptr;
        } else if (remove_by_sessionid != 0) {
            if (remove_this != 0) {
                /* Keep unchange the current_session_ptr */
                current_session = *current_session_ptr;
            } else {
                assert(current_session != NULL);
                current_session_ptr = &current_session->ngs_apiNext;
                current_session = *current_session_ptr;
            }
        } else {
            abort();
        }
    }

    /* Fix the list_tail and nSessions */
    session_count = 0;
    queue_tail = NULL;
    current_session = session_queue->gsq_list_head;
    while (current_session != NULL) {
        session_count++;
        queue_tail = current_session;
        current_session = current_session->ngs_apiNext;
    }
    session_queue->gsq_nSessions = session_count;
    session_queue->gsq_list_tail = queue_tail;

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
            goto error;
        }
    }

    /* Check removed */
    if ((remove_by_sessionid != 0) && (removed == 0)) {
        error_code = GRPC_OTHER_ERROR_CODE;
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't find the session by ID %d.\n",
            fName, sessionID);
        goto error;
    }

    /* Success */
    return error_code;

    /* Error occurred */
error:
    return_code = error_code;

    /* Write Unlock the Session Queue */
    if (locked != 0) {
        error_code = grpc_l_session_queue_write_unlock(session_queue);
        locked = 0;
        if (error_code != GRPC_NO_ERROR) {
            ngclLogPrintfContext(grpc_l_context,
                NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
                "%s: Can't write unlock the Session Queue.\n", fName);
        }
    }

    return return_code;
}

/**
 * Session Queue Get Session.
 * Note :
 * lock the list before using this function, and unlock the list
 * after use.
 */
ngclSession_t *
grpc_l_session_queue_get_session(
    grpc_l_session_queue_t *session_queue,
    grpc_l_session_queue_get_mode_t get_mode,
    grpc_sessionid_t sessionId)
{
    ngclSession_t *current_session;
#if 0
    static const char fName[] = "grpc_l_session_queue_get_session";
#endif

    /* Check the arguments */
    assert(session_queue != NULL);

    if (get_mode == GRPC_L_SESSION_QUEUE_GET_LAST) {
        return session_queue->gsq_list_tail;
    }

    assert(get_mode == GRPC_L_SESSION_QUEUE_GET_BY_ID);

    current_session = session_queue->gsq_list_head;
    while (current_session != NULL) {
        if (current_session->ngs_ID == sessionId) {
            /* Found */
            return current_session;
        }
        current_session = current_session->ngs_apiNext;
    }
    
    /* Not found */
    return NULL;
}

/**
 * Session Queue Get Next Session.
 *
 * Return the Session from the top of list, if current is NULL.
 * Return the next Session of current, if current is not NULL.
 * Note :
 * lock the list before using this function, and unlock the list
 * after use.
 */
ngclSession_t *
grpc_l_session_queue_get_next_session(
    grpc_l_session_queue_t *session_queue,
    ngclSession_t *current)
{
#if 0
    static const char fName[] = "grpc_l_session_queue_get_next_session";
#endif

    /* Check the arguments */
    assert(session_queue != NULL);

    if (current == NULL) {
        return session_queue->gsq_list_head;
    }
    
    return current->ngs_apiNext;
}

/**
 * Set last error code and error message.
 */
grpc_error_t
grpc_l_error_set(
    grpc_error_t errorCode)
{
    grpc_error_t error_code = GRPC_NO_ERROR;
    int error, result;
    static const char fName[] = "grpc_l_error_set";

    /* Lock the Last Error */
    result = ngclRWlockWriteLock(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't lock the last error lock.\n", fName);
        return error_code;
    }

    grpc_l_last_error.ge_code = errorCode;
    grpc_l_last_error.ge_message = grpc_l_error_message[errorCode];

    /* Unlock the Last Error */
    result = ngclRWlockWriteUnlock(
        grpc_l_context, &grpc_l_last_error.ge_lock, &error);
    if (result == 0) {
        error_code = grpc_i_get_error_from_ng_error(error);
        grpc_l_error_set(error_code);
        ngclLogPrintfContext(grpc_l_context,
            NG_LOG_CATEGORY_NINFG_GRPC, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Can't unlock the last error lock.\n", fName);
        return error_code;
    }

    return error_code;
}

/**
 * Print the error.
 */
void
grpc_l_print_error(
    char *str,
    char *message,
    grpc_error_t code)
{
    fprintf(stderr, "%s%s%s\n",
        ((str != NULL) ? str : ""),
        ((str != NULL) ? ": " : ""),
        ((message != NULL) ? message : grpc_error_string(code)));
}

