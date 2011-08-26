/*
 * Portions of this file Copyright 1999-2005 University of Chicago
 * Portions of this file Copyright 1999-2005 The University of Southern California.
 *
 * This file or a portion of this file is licensed under the
 * terms of the Globus Toolkit Public License, found at
 * http://www.globus.org/toolkit/download/license.html.
 * If you redistribute this file, with or without
 * modifications, you must include this notice in the file.
 */

/******************************************************************************
globus_gass_server_ez.h
 
Description:
    Simple wrappers around globus_gass_server API for server functionality.
    Implements the following:
        Write access to local files, with optional line buffering
	Write access to stdout and stderr
	Shutdown callback, so client can stop the server
 
CVS Information:
 
    $Source: /home/globdev/CVS/globus-packages/gass/server_ez/source/globus_gass_server_ez.h,v $
    $Date: 2005/04/18 23:01:10 $
    $Revision: 1.9 $
    $Author: mlink $
******************************************************************************/
#ifndef _GLOBUS_GASS_INCLUDE_GLOBUS_GASS_SIMPLE_SERVER_H_
#define _GLOBUS_GASS_INCLUDE_GLOBUS_GASS_SIMPLE_SERVER_H_


#ifndef EXTERN_C_BEGIN
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
#endif

#include "globus_gass_transfer.h"

EXTERN_C_BEGIN

#define GLOBUS_GASS_SERVER_EZ_LINE_BUFFER              1UL 
#define GLOBUS_GASS_SERVER_EZ_TILDE_EXPAND             2UL
#define GLOBUS_GASS_SERVER_EZ_TILDE_USER_EXPAND        4UL
#define GLOBUS_GASS_SERVER_EZ_READ_ENABLE              8UL
#define GLOBUS_GASS_SERVER_EZ_WRITE_ENABLE             16UL
#define GLOBUS_GASS_SERVER_EZ_STDOUT_ENABLE            32UL
#define GLOBUS_GASS_SERVER_EZ_STDERR_ENABLE            64UL
#define GLOBUS_GASS_SERVER_EZ_CLIENT_SHUTDOWN_ENABLE   128UL

#if (GLOBUS_GASS_SERVER_EZ_TILDE_EXPAND != GLOBUS_TILDE_EXPAND)
#error "Inconsistant definition of GLOBUS_GASS_SERVER_EZ_TILDE_EXPAND and GLOBUS_TILDE_EXPAND"
#endif
#if (GLOBUS_GASS_SERVER_EZ_TILDE_USER_EXPAND != GLOBUS_TILDE_USER_EXPAND)
#error "Inconsistant definition of GLOBUS_GASS_SERVER_EZ_TILDE_USER_EXPAND and GLOBUS_TILDE_USER_EXPAND"
#endif

typedef void (*globus_gass_server_ez_client_shutdown_t) (void);
/*
typedef globus_object_t globus_gass_transfer_listener_t;
typedef globus_object_t globus_gass_transfer_listenerattr_t;
typedef globus_object_t globus_gass_transfer_requestattr_t;
*/


int
globus_gass_server_ez_init(globus_gass_transfer_listener_t * listener,
                           globus_gass_transfer_listenerattr_t * attr,
                           char * scheme,
                           globus_gass_transfer_requestattr_t * reqattr,
                           unsigned long options,
                           globus_gass_server_ez_client_shutdown_t callback,
                           int fd);

int
globus_gass_server_ez_shutdown(globus_gass_transfer_listener_t listener);

#define globus_gass_server_ez_poll() globus_poll()
/******************************************************************************
 *                    Module Definition
 *****************************************************************************/

extern globus_module_descriptor_t globus_i_gass_server_ez_module;

#define SAGA_GASS_SERVER_EZ_MODULE (&globus_i_gass_server_ez_module)

EXTERN_C_END

#endif
