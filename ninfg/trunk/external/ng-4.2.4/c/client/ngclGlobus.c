#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngclGlobus.c,v $ $Revision: 1.30 $ $Date: 2006/08/17 07:21:19 $";
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
 * @file ngcliGlobus.c
 * Globus module for Ninf-G Client internal.
 */

#include <string.h>
#include "ng.h"

/**
 * Prototype declaration of internal functions.
 */
static ngclGASSserverManager_t *ngcllGASSserverAllocate(ngLog_t *, int *);
static int ngcllGASSserverFree(ngclGASSserverManager_t *, ngLog_t *, int *);
static int ngcllGASSserverInitialize(
    ngclGASSserverManager_t *, char *, unsigned long, ngLog_t *, int *);
static int ngcllGASSserverFinalize(
    ngclGASSserverManager_t *, ngLog_t *, int *);
static void ngcllGASSserverInitializeMember(ngclGASSserverManager_t *);
static void ngcllGASSserverInitializePointer(ngclGASSserverManager_t *);

/* Define the GLobus Modules for activate/deactivate */
static globus_module_descriptor_t *ngcllGlobusModules[] = {
    GLOBUS_IO_MODULE,
    GLOBUS_GASS_SERVER_EZ_MODULE,
#ifndef NGI_NO_MDS4_MODULE
    INDEXSERVICE_MODULE,
#endif /* NGI_NO_MDS4_MODULE */
};

/**
 * Initialize the Module of Globus Toolkit.
 *
 * @param error
 * Pointer to set the error code, if an error occurred.
 *
 * @return
 * This function returns 1 if success.
 * It returns 0 if an error occurred, and sets error to indicate the error.
 */ 
int
ngcliGlobusInitialize(ngLog_t *log, int *error)
{
    int i;
    int result;
    static const char fName[] = "ngcliGlobusInitialize";

    /* Activate the modules */
    for (i = 0; i < NGI_NELEMENTS(ngcllGlobusModules); i++) {
    	result = globus_module_activate(ngcllGlobusModules[i]);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: globus_module_activate failed %d.\n", fName, result);
            if (ngcllGlobusModules[i] == GLOBUS_GRAM_CLIENT_MODULE) {
                ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                    NG_LOG_LEVEL_FATAL, NULL,
                    "%s: failed %d because :%s .\n", fName, result,
                    globus_gram_protocol_error_string(result));
            }
    	    goto error;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deactivate the modules */
    for (i--; i >= 0; i--) {
    	result = globus_module_deactivate(ngcllGlobusModules[i]);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: globus_module_deactivate failed %d.\n", fName, result);
	}
    }

    return 0;
}

/**
 * Finalize the Module of Globus Toolkit.
 *
 * @param error
 * Pointer to set the error code, if an error occurred.
 *
 * @return
 * This function returns 1 if success.
 * It returns 0 if an error occurred, and sets error to indicate the error.
 */ 
int
ngcliGlobusFinalize(ngLog_t *log, int *error)
{
    int i;
    int result;
    static const char fName[] = "ngcliGlobusFinalize";

    /* Activate the modules */
    for (i = NGI_NELEMENTS(ngcllGlobusModules) - 1; i >= 0; i--) {
    	result = globus_module_deactivate(ngcllGlobusModules[i]);
	if (result != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: globus_module_deactivate failed %d.\n", fName, result);
    	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Initialize the Common Module of Globus Toolkit.
 */ 
int
ngcliGlobusCommonInitialize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcliGlobusCommonInitialize";

    /* Activate the common module */
    result = globus_module_activate(GLOBUS_COMMON_MODULE);
    if (result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_module_activate failed %d.\n", fName, result);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Finalize the Common Module of Globus Toolkit.
 */ 
int
ngcliGlobusCommonFinalize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcliGlobusCommonFinalize";

    /* Activate the common module */
    result = globus_module_deactivate(GLOBUS_COMMON_MODULE);
    if (result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_module_deactivate failed %d.\n", fName, result);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the GRAM Module of Globus Toolkit.
 */ 
int
ngcliGlobusGRAMclientInitialize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngcliGlobusGRAMclientInitialize";

    result = globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE);
    if (result != GLOBUS_SUCCESS) {
        NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_module_activate(GRAM_CLIENT) failed %d.\n",
            fName, result);
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL, NULL,
            "%s: failed %d because :%s .\n", fName, result,
            globus_gram_protocol_error_string(result));
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * GASS Server: Construct
 */
ngclGASSserverManager_t *
ngcliGASSserverConstruct(
    char *gassScheme,
    unsigned long options,
    ngLog_t *log,
    int *error)
{
    int result;
    ngclGASSserverManager_t *gassMng;
    static const char fName[] = "ngcliGASSserverConstruct";

    /* Check the arguments */
    assert(gassScheme != NULL);
    assert(gassScheme[0] != '\0');

    /* Allocate */
    gassMng = ngcllGASSserverAllocate(log, error);
    if (gassMng == NULL) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for GASS Server Manager.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = ngcllGASSserverInitialize(
	gassMng, gassScheme, options, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't initialize the GASS Server Manager.\n", fName);
	goto error;
    }

    /* Success */
    return gassMng;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngcllGASSserverFree(gassMng, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for GASS Server Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * GASS Server: Destruct
 */
int
ngcliGASSserverDestruct(
    ngclGASSserverManager_t *gassMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcliGASSserverDestruct";

    /* Check the arguments */
    assert(gassMng != NULL);

    /* Finalize */
    result = ngcllGASSserverFinalize(gassMng, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL, NULL,
	    "%s: Can't finalize the GASS Server Manager.\n", fName);
	goto error;
    }

    /* Deallocate */
    result = ngcllGASSserverFree(gassMng, log, error);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: Can't deallocate the storage for GASS Server Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Deallocate */
    result = ngcllGASSserverFree(gassMng, log, NULL);
    if (result == 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for GASS Server Manager.\n",
	    fName);
	return 0;
    }

    return 0;
}

/**
 * GASS Server: Allocate
 */
static ngclGASSserverManager_t *
ngcllGASSserverAllocate(ngLog_t *log, int *error)
{
    ngclGASSserverManager_t *gassMng;
    static const char fName[] = "ngcllGASSserverAllocate";

    /* Allocate */
    gassMng = globus_libc_calloc(1, sizeof (ngclGASSserverManager_t));
    if (gassMng == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for GASS Server Manager.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return gassMng;
}

/**
 * GASS Server: Free
 */
static int
ngcllGASSserverFree(
    ngclGASSserverManager_t *gassMng,
    ngLog_t *log,
    int *error)
{
    globus_libc_free(gassMng);

    /* Success */
    return 1;
}

/**
 * GASS Server: Initialize
 */
static int
ngcllGASSserverInitialize(
    ngclGASSserverManager_t *gassMng,
    char *scheme,
    unsigned long options,
    ngLog_t *log,
    int *error)
{
    int result;
    char *urlString;
    static const char fName[] = "ngcllGASSserverInitialize";

    /* Check the arguments */
    assert(gassMng != NULL);
    assert(scheme != NULL);
    assert(scheme[0] != '\0');

    /* Initialize the members */
    ngcllGASSserverInitializeMember(gassMng);
    gassMng->nggsm_options = options;

    /* Duplicate the Scheme */
    gassMng->nggsm_scheme = strdup(scheme);
    if (gassMng->nggsm_scheme == NULL) {
    	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't allocate the storage for Scheme.\n", fName);
    }

    /* Start the GASS Server */
    result = globus_gass_server_ez_init(
	&gassMng->nggsm_listener, GLOBUS_NULL, scheme, GLOBUS_NULL, options,
	GLOBUS_NULL);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: globus_gass_server_ez_init failed by %d.\n",
	    fName, result);
	return 0;
    }

    /* Get the URL */
    urlString = globus_gass_transfer_listener_get_base_url(
	gassMng->nggsm_listener);
    if (urlString == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	NULL, "%s: Can't get the URL for GASS server.\n", fName);
	goto error;
    }
    gassMng->nggsm_url = strdup(urlString);
    if (gassMng->nggsm_url == NULL) {
	NGI_SET_ERROR(error, NG_ERROR_MEMORY);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
	NULL, "%s: Can't copy the URL for GASS server.\n", fName);
	goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Shutdown the GASS server */
    result = globus_gass_server_ez_shutdown(gassMng->nggsm_listener);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: globus_gass_server_ez_shutdown failed by %d.\n",
	    fName, result);
	return 0;
    }

    return 0;
}

/**
 * GASS Server: Finalize
 */
static int
ngcllGASSserverFinalize(
    ngclGASSserverManager_t *gassMng,
    ngLog_t *log,
    int *error)
{
    int result;
    static const char fName[] = "ngcllGASSserverFinalize";

    /* Check the arguments */
    assert(gassMng != NULL);

    /* Deallocate the URL */
    globus_libc_free(gassMng->nggsm_url);
    globus_libc_free(gassMng->nggsm_scheme);

    /* Shutdown the GASS server */
    result = globus_gass_server_ez_shutdown(gassMng->nggsm_listener);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: globus_gass_server_ez_shutdown failed by %d.\n",
	    fName, result);
	return 0;
    }

    /* Initialize the members */
    ngcllGASSserverInitializeMember(gassMng);

    /* Success */
    return 1;
}

/**
 * GASS server: Initialize the members.
 */
static void
ngcllGASSserverInitializeMember(ngclGASSserverManager_t *gassMng)
{
    /* Check the arguments */
    assert(gassMng != NULL);

    /* Initialize the pointers */
    ngcllGASSserverInitializePointer(gassMng);

    /* Initialize the members */
    gassMng->nggsm_options = 0;
}

/**
 * GASS server: Initialize the pointers.
 */
static void
ngcllGASSserverInitializePointer(ngclGASSserverManager_t *gassMng)
{
    /* Check the arguments */
    assert(gassMng != NULL);

    /* Initialize the pointers */
    gassMng->nggsm_next = NULL;
    gassMng->nggsm_scheme = NULL;
    gassMng->nggsm_url = NULL;
}

/**
 * Register I/O callback function for listen.
 */
int
ngcliGlobusIoTcpRegisterListen(
    globus_io_handle_t *ioHandle,
    globus_io_callback_t cbFunc,
    void *cbArg,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngcliGlobusIoTcpRegisterListen";

    /* Check the arguments */
    assert(ioHandle != NULL);
    assert(cbFunc != NULL);
    assert(cbArg != NULL);

    /* Register */
    gResult = globus_io_tcp_register_listen(ioHandle, cbFunc, cbArg);
    if (gResult != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcp_register_listen() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Register I/O callback function for select.
 */
int
ngcliGlobusIoRegisterSelect(
    globus_io_handle_t *ioHandle,
    globus_io_callback_t cbFunc,
    void *cbArg,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngcliGlobusIoRegisterSelect";

    /* Check the arguments */
    assert(ioHandle != NULL);
    assert(cbFunc != NULL);
    assert(cbArg != NULL);

    /* Register */
    gResult = globus_io_register_select(
    	ioHandle, cbFunc, cbArg, NULL, NULL, NULL, NULL);
    if (gResult != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_register_select() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Unregister I/O callback function.
 */
int
ngcliGlobusIoUnregister(
    globus_io_handle_t *ioHandle,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngcliGlobusIoUnregister";

    /* Check the arguments */
    assert(ioHandle != NULL);

    /* Register */
    gResult = globus_io_cancel(ioHandle, GLOBUS_FALSE);
    if (gResult != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_cancel() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, NULL);
    	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Acquire credential.
 */
int
ngcliGlobusAcquireCredential(
    gss_cred_id_t *gss_cred,
    ngLog_t *log,
    int *error)
{
    OM_uint32 major_status;
    OM_uint32 minor_status;
    char *message;
    static const char fName[] = "ngcliGlobusAcquireCredential";

    /* acquire credential */
    major_status = globus_gss_assist_acquire_cred(&minor_status,
        GSS_C_BOTH, gss_cred);
    if (major_status != GSS_S_COMPLETE) {
        globus_gss_assist_display_status_str(
            &message,
            "",
            major_status,
            minor_status,
            0
            );

	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_WARNING,
            NULL, "%s: Failed to refresh cred(%s).\n", fName, message);
    	return 0;
    }

    /* Success */
    return 1;
}
