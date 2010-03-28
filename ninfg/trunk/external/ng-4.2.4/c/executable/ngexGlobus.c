#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngexGlobus.c,v $ $Revision: 1.9 $ $Date: 2004/12/08 07:33:12 $";
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
 * @file ngexGlobus.c
 * Globus module for Ninf-G Executable internal.
 */

#include "ngEx.h"

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
ngexiGlobusInitialize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngexiGlobusInitialize";

    result = globus_module_activate(GLOBUS_IO_MODULE);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_module_activate failed %d.\n", fName, result);
    	return 0;
    }

    result = globus_module_activate(GLOBUS_GASS_COPY_MODULE);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_module_activate failed %d.\n", fName, result);
    	return 0;
    }

    return 1;
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
ngexiGlobusFinalize(ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngexiGlobusFinalize";

    result = globus_module_deactivate(GLOBUS_GASS_COPY_MODULE);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_module_activate failed %d.\n", fName, result);
    	return 0;
    }

    result = globus_module_deactivate(GLOBUS_IO_MODULE);
    if (result != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_GLOBUS);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_module_deactivate failed %d.\n", fName, result);
    	return 0;
    }

    return 1;
}

/**
 * Register I/O callback function for select.
 */
int
ngexiGlobusIoRegisterSelect(
    globus_io_handle_t *ioHandle,
    globus_io_callback_t cbFunc,
    void *cbArg,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngexiGlobusIoRegisterSelect";

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
ngexiGlobusIoUnregister(
    globus_io_handle_t *ioHandle,
    ngLog_t *log,
    int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngexiGlobusIoUnregister";

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

