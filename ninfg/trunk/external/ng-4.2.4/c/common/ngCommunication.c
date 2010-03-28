#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngCommunication.c,v $ $Revision: 1.57 $ $Date: 2007/12/26 12:27:17 $";
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
 * Module for managing Communication for Ninf-G Client/Executable.
 */

#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include "ng.h"

/**
 * Prototype declaration of static functions.
 */
static ngiCommunication_t *nglCommunicationConstruct(
    ngProtocolCrypt_t, ngLog_t *, int *);
static ngiCommunication_t *nglCommunicationAllocate(ngLog_t *, int *);
static int nglCommunicationFree(ngiCommunication_t *, ngLog_t *, int *);
static int nglCommunicationInitialize(
    ngiCommunication_t *, ngProtocolCrypt_t, ngLog_t *, int *);
static int nglCommunicationFinalize(ngiCommunication_t *, ngLog_t *, int *);
static void nglCommunicationInitializeMember(ngiCommunication_t *);
static void nglCommunicationInitializePointer(ngiCommunication_t *);
static int nglCommunicationSendIovec(
    ngiCommunication_t *, struct iovec *, size_t, ngLog_t *, int *);

/**
 * Construct server.
 */
ngiCommunication_t *
ngiCommunicationConstructServer(
    ngProtocolCrypt_t crypt,
    int tcpNodelay,
    unsigned short portNo,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    ngiCommunication_t *comm;
    static const char fName[] = "ngiCommunicationConstructServer";

    /* Check the arguments */
    assert((crypt == NG_PROTOCOL_CRYPT_NONE) ||
	   (crypt == NG_PROTOCOL_CRYPT_AUTHONLY) ||
	   (crypt == NG_PROTOCOL_CRYPT_GSI) ||
	   (crypt == NG_PROTOCOL_CRYPT_SSL));

    /* Construct */
    comm = nglCommunicationConstruct(crypt, log, error);
    if (comm == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Communication Manager.\n", fName);
	return NULL;
    }

    if (tcpNodelay != 0) {
        gResult = globus_io_attr_set_tcp_nodelay(&comm->ngc_ioAttr, GLOBUS_TRUE);
        if (gResult != GLOBUS_SUCCESS) {
            NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
            ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_attr_set_tcp_nodelay() failed.\n", fName);
            ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, fName, gResult, error);
            goto error;
        }
    }

    /* Create the socket */
    comm->ngc_portNo = portNo;
    gResult = globus_io_tcp_create_listener(
    	&comm->ngc_portNo, -1, &comm->ngc_ioAttr, &comm->ngc_ioHandle);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcp_create_listener() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }
    comm->ngc_flag_ioHandle = 1;

    /* log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_DEBUG, NULL,
        "%s: listener created. port is %hu.\n", fName, comm->ngc_portNo);

    /* Success */
    return comm;

    /* Error occurred */
error:
    result = ngiCommunicationDestruct(comm, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destruct the Communication Manager.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Construct accept.
 */
ngiCommunication_t *
ngiCommunicationConstructAccept(
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    ngiCommunication_t *newComm;
    static const char fName[] = "ngiCommunicationConstructAccept";

    /* Check the arguments */
    assert(comm != NULL);

    /* Allocate */
    newComm = nglCommunicationAllocate(log, error);
    if (newComm == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't allocate the Communication Manager.\n", fName);
	return NULL;
    }

    /* Initialize the members */
    nglCommunicationInitializeMember(newComm);

    /* Get the attribute */
    gResult = globus_io_tcp_get_attr(
    	&comm->ngc_ioHandle, &newComm->ngc_ioAttr);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcp_get_attr() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }
    newComm->ngc_flag_ioAttr = 1;

    /* Accept */
    gResult = globus_io_tcp_accept(
    	&comm->ngc_ioHandle, &newComm->ngc_ioAttr, &newComm->ngc_ioHandle);
    if (gResult != GLOBUS_SUCCESS) {
	result = ngiGlobusIsIoEOF(gResult, log, error);
	if (result != 0) {
	    NGI_SET_ERROR(error, NG_ERROR_DISCONNECT);
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    	NG_LOG_LEVEL_ERROR, NULL,
		"%s: Connection was closed.\n", fName);
	} 
        goto error;
    }
    newComm->ngc_flag_ioHandle = 1;


    /* Reflect attributes in IO handle to newComm->ngc_ioAttr */
    newComm->ngc_flag_ioAttr = 0;
    gResult = globus_io_tcpattr_destroy(&newComm->ngc_ioAttr);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcpattr_destroy() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }

    gResult = globus_io_tcp_get_attr(
    	&newComm->ngc_ioHandle, &newComm->ngc_ioAttr);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcp_get_attr() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }
    newComm->ngc_flag_ioAttr = 1;

    /* Success */
    return newComm;

    /* Error occurred */
error:
    result = ngiCommunicationDestruct(newComm, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destruct the Communication Manager.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Construct client.
 */
ngiCommunication_t *
ngiCommunicationConstructClient(
    ngProtocolCrypt_t crypt,
    int tcpNodelay,
    char *hostName,
    unsigned short portNo,
    ngiConnectRetryInformation_t retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    ngiCommunication_t *comm;
    struct timeval retryTimeval;
    ngiConnectRetryStatus_t retryStatus;
    int connectSuccess, onceFailed, retryRequired, doRetry;
    static const char fName[] = "ngiCommunicationConstructClient";

    /* Check the arguments */
    assert((crypt == NG_PROTOCOL_CRYPT_NONE) ||
	   (crypt == NG_PROTOCOL_CRYPT_AUTHONLY) ||
	   (crypt == NG_PROTOCOL_CRYPT_GSI) ||
	   (crypt == NG_PROTOCOL_CRYPT_SSL));
    assert(hostName != NULL);
    assert(portNo != 0);
    assert(randomSeed != NULL);

    connectSuccess = 0;
    onceFailed = 0;
    retryRequired = 0;
    doRetry = 0;

    /* Construct */
    comm = nglCommunicationConstruct(crypt, log, error);
    if (comm == NULL) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't construct the Communication Manager.\n", fName);
	return NULL;
    }

    /* Set TCP_NODELAY option */
    if (tcpNodelay != 0) {
	gResult = globus_io_attr_set_tcp_nodelay(&comm->ngc_ioAttr,
		GLOBUS_TRUE);
	if (gResult != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: globus_io_attr_set_tcp_nodelay() failed.\n", fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    goto error;
	}
    }

    /* Initialize the Retry Status */
    result = ngiConnectRetryInitialize(
        &retryStatus, &retryInfo, randomSeed, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Retry status initialize failed.\n", fName);
	goto error;
    }

    do {
        connectSuccess = 0;
        retryRequired = 0;
        doRetry = 0;

        /* log */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_DEBUG, NULL,
            "%s: TCP connect for host \"%s\" port %hu.\n",
            fName, hostName, portNo);

        /* Connect */
        gResult = globus_io_tcp_connect(
            hostName, portNo, &comm->ngc_ioAttr, &comm->ngc_ioHandle);
        if (gResult != GLOBUS_SUCCESS) {
            ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_WARNING, NULL,
                "%s: globus_io_tcp_connect() failed.\n", fName);
            ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_WARNING, fName, gResult, error);

            connectSuccess = 0;
            onceFailed = 1;
            retryRequired = 1;
        } else {
            connectSuccess = 1;
            retryRequired = 0;
            comm->ngc_flag_ioHandle = 1;
        }

        if (retryRequired != 0) {
            /* Get Next Retry */
            result = ngiConnectRetryGetNextRetrySleepTime(
                &retryStatus, &doRetry, &retryTimeval, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Getting next retry time failed.\n", fName);
                goto error;
            }
        }

        if (doRetry != 0) {
            /* Sleep before retry */
            result = ngiSleepTimeval(&retryTimeval, 0, log, error);
            if (result == 0) {
                ngLogPrintf(log,
                    NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
                    "%s: sleep failed.\n", fName);
                goto error;
            }

            /* to tell loglevel == Warning user */
            ngLogPrintf(log,
                NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_WARNING, NULL,
                "%s: Retrying globus_io_tcp_connect().\n", fName);
        }
    } while (doRetry != 0);

    if (connectSuccess == 0) {
        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
        ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: globus_io_tcp_connect() was finally failed.\n", fName);
        goto error;
    }

    if (onceFailed != 0) {
        /* to tell loglevel == Warning user */
        ngLogPrintf(log,
            NG_LOG_CATEGORY_GLOBUS_TOOLKIT, NG_LOG_LEVEL_WARNING, NULL,
            "%s: globus_io_tcp_connect() was finally successful.\n", fName);
    }

    /* Finalize the Retry Status */
    result = ngiConnectRetryFinalize(&retryStatus, log, error);
    if (result == 0) {
        ngLogPrintf(log,
            NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
            "%s: Retry status finalize failed.\n", fName);
        goto error;
    }

    /* Success */
    return comm;

    /* Error occurred */
error:
    result = ngiCommunicationDestruct(comm, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL, "%s: Can't destruct the Communication Manager.\n", fName);
	return NULL;
    }

    return NULL;
}

/**
 * Construct
 */
static ngiCommunication_t *
nglCommunicationConstruct(
    ngProtocolCrypt_t crypt,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiCommunication_t *comm;
    static const char fName[] = "nglCommunicationConstruct";

    /* Check the arguments */

    /* Allocate */
    comm = nglCommunicationAllocate(log, error);
    if (comm == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Communication Manager.\n",
	    fName);
	return NULL;
    }

    /* Initialize */
    result = nglCommunicationInitialize(comm, crypt, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't initialize the Communication Manager.\n", fName);
	goto error;
    }

    /* Success */
    return comm;

    /* Error occurred */
error:
    result = nglCommunicationFree(comm, log, NULL);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't deallocate the storage for Communication Manager.\n",
	    fName);
	return NULL;
    }

    return NULL;
}

/**
 * Destruct
 */
int
ngiCommunicationDestruct(ngiCommunication_t *comm, ngLog_t *log, int *error)
{
    int result;
    static const char fName[] = "ngiCommunicationDestruct";

    /* Check the arguments */
    assert(comm != NULL);

    /* Finalize */
    result = nglCommunicationFinalize(comm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL, "%s: Can't finalize the Communication Manager.\n", fName);
	return 0;
    }

    result = nglCommunicationFree(comm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_FATAL,
	    NULL,
	    "%s: Can't deallocate the storage for Communication Manager.\n",
	    fName);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Allocate
 */
static ngiCommunication_t *
nglCommunicationAllocate(ngLog_t *log, int *error)
{
    ngiCommunication_t *comm;
    static const char fName[] = "nglCommunicationAllocate";

    comm = globus_libc_calloc(1, sizeof (ngiCommunication_t));
    if (comm == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't allocate the storage for Communication Manager.\n",
	    fName);
	return NULL;
    }

    /* Success */
    return comm;
}

/**
 * Deallocate
 */
static int
nglCommunicationFree(ngiCommunication_t *comm, ngLog_t *log, int *error)
{
    /* Check the arguments */
    assert(comm != NULL);

    /* Deallocate */
    globus_libc_free(comm);

    /* Success */
    return 1;
}

/**
 * Initialize
 */
static int
nglCommunicationInitialize(
    ngiCommunication_t *comm,
    ngProtocolCrypt_t crypt,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    globus_io_secure_authentication_mode_t authenticationMode;
    static const char fName[] = "nglCommunicationInitialize";

    /* Check the arguments */
    assert(comm != NULL);
    assert((crypt == NG_PROTOCOL_CRYPT_NONE) ||
	   (crypt == NG_PROTOCOL_CRYPT_AUTHONLY) ||
	   (crypt == NG_PROTOCOL_CRYPT_GSI) ||
	   (crypt == NG_PROTOCOL_CRYPT_SSL));

    /* Initialize the pointer */
    nglCommunicationInitializeMember(comm);

    /* Initialize the TCP attribute */
    gResult = globus_io_tcpattr_init(&comm->ngc_ioAttr);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_tcpattr_init() failed.\n", fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }
    comm->ngc_flag_ioAttr = 1;

    /* Initialize the authorization data */
    gResult = globus_io_secure_authorization_data_initialize(&comm->ngc_authData);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_secure_authorization_data_initialize() failed.\n",
            fName);
    	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }
    comm->ngc_flag_authData = 1;

    /* Initialize the authentication mode */
    authenticationMode = (crypt == NG_PROTOCOL_CRYPT_NONE)
	? GLOBUS_IO_SECURE_AUTHENTICATION_MODE_NONE
	: GLOBUS_IO_SECURE_AUTHENTICATION_MODE_GSSAPI;
    gResult = globus_io_attr_set_secure_authentication_mode(
	&comm->ngc_ioAttr, authenticationMode, GSS_C_NO_CREDENTIAL);
    if (gResult != GLOBUS_SUCCESS) {
	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, NULL,
	    "%s: globus_io_attr_set_secure_authentication_mode() failed.\n",
	    fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	goto error;
    }

    if ((crypt == NG_PROTOCOL_CRYPT_GSI) || (crypt == NG_PROTOCOL_CRYPT_SSL)) {
	/* Initialize the authorization mode */
	comm->ngc_authMode = GLOBUS_IO_SECURE_AUTHORIZATION_MODE_SELF;
	gResult = globus_io_attr_set_secure_authorization_mode(
	    &comm->ngc_ioAttr, comm->ngc_authMode, &comm->ngc_authData);
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_attr_set_secure_authorization_mode() failed.\n",
                fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    goto error;
	}

	/* Initialize the secure channel mode */
	if (crypt == NG_PROTOCOL_CRYPT_GSI) {
	    gResult = globus_io_attr_set_secure_channel_mode(
	    	&comm->ngc_ioAttr, GLOBUS_IO_SECURE_CHANNEL_MODE_GSI_WRAP);
	} else {
	    gResult = globus_io_attr_set_secure_channel_mode(
	    	&comm->ngc_ioAttr, GLOBUS_IO_SECURE_CHANNEL_MODE_SSL_WRAP);
	}
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_attr_set_secure_channel_mode() failed.\n",
                fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    goto error;
	}

	/* Initialize the secure protection mode */
	gResult = globus_io_attr_set_secure_protection_mode(
	    &comm->ngc_ioAttr, GLOBUS_IO_SECURE_PROTECTION_MODE_PRIVATE);
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_attr_set_secure_protection_mode() failed.\n",
                fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    goto error;
	}
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize */
    result = nglCommunicationFinalize(comm, log, NULL);
    if (result == 0) {
    	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
	    "%s: Can't finalize the Communication.\n", fName);
	return 0;
    }

    /* Failed */
    return 0;
}

/**
 * Finalize
 */
static int
nglCommunicationFinalize(ngiCommunication_t *comm, ngLog_t *log, int *error)
{
    int result;
    globus_result_t gResult;
    static const char fName[] = "nglCommunicationFinalize";

    /* Check the arguments */
    assert(comm != NULL);

    /* Destruct the Communication Log */
    if (comm->ngc_commLog != NULL) {
        result = ngiCommunicationLogUnregister(comm, log, error);
        comm->ngc_commLog = NULL;
        if (result == 0) {
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
                NULL,
                "%s: Can't unregister the Communication Log.\n", fName);
            return 0;
        }
    }

    /* Destroy the TCP attribute */
    if (comm->ngc_flag_ioAttr) {
	gResult = globus_io_tcpattr_destroy(&comm->ngc_ioAttr);
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_tcpattr_destroy() failed.\n", fName);
    	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
    	    return 0;
	}
    }
    comm->ngc_flag_ioAttr = 0;

    /* Destroy the authorization data */
    if (comm->ngc_flag_authData) {
	gResult = globus_io_secure_authorization_data_destroy(&comm->ngc_authData);
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_secure_authorization_data_destroy() failed.\n",
                fName);
    	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    return 0;
	}
    }
    comm->ngc_flag_authData = 0;

    /* Close the I/O Handle */
    result = ngiCommunicationClose(comm, log, error);
    if (result == 0) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL,
            "%s: Can't close the Communication.\n", fName);
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize the members.
 */
static void
nglCommunicationInitializeMember(ngiCommunication_t *comm)
{
    /* Initialize the pointers */
    nglCommunicationInitializePointer(comm);

    /* Initialize the members */
    comm->ngc_portNo = NGI_PORT_INVALID;
    comm->ngc_flag_ioHandle = 0;
    comm->ngc_flag_ioAttr = 0;
    comm->ngc_flag_authData = 0;
}

/**
 * Initialize the pointers.
 */
static void
nglCommunicationInitializePointer(ngiCommunication_t *comm)
{
    /* Initialize the pointers */
    comm->ngc_commLog = NULL;
}

/**
 * Close the I/O Handle
 */
int
ngiCommunicationClose(ngiCommunication_t *comm, ngLog_t *log, int *error)
{
    globus_result_t gResult;
    static const char fName[] = "ngiCommunicationClose";

    /* Check the arguments */
    if (comm == NULL) {
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: communication is NULL\n", fName);
        return 0;
    }

    /* Close the I/O Handle */
    if (comm->ngc_flag_ioHandle) {
    	gResult = globus_io_close(&comm->ngc_ioHandle);
	if (gResult != GLOBUS_SUCCESS) {
    	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
                NG_LOG_LEVEL_FATAL, NULL,
                "%s: globus_io_close() failed.\n", fName);
    	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    return 0;
	}
    }
    comm->ngc_flag_ioHandle = 0;

    /* Success */
    return 1;
}

/**
 * Register the Communication Log
 */
int
ngiCommunicationLogRegister(
    ngiCommunication_t *comm,
    ngLog_t *commLog,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCommunicationLogRegister";

    /* Check the arguments */
    assert(comm != NULL);
    assert(commLog != NULL);

    /* Is Communication Log already registered? */
    if (comm->ngc_commLog != NULL) {
        NGI_SET_ERROR(error, NG_ERROR_ALREADY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Communication log was already registered.\n", fName);
        return 0;
    }

    /* Register */
    comm->ngc_commLog = commLog;

    /* Success */
    return 1;
}

/**
 * Unregister the Communication Log
 */
int
ngiCommunicationLogUnregister(
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCommunicationLogUnregister";

    /* Check the arguments */
    assert(comm != NULL);

    /* Is Communication Log not registered? */
    if (comm->ngc_commLog == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_NOT_EXIST);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR,
            NULL, "%s: Communication log was not registered.\n", fName);
        return 0;
    }

    /* Register */
    comm->ngc_commLog = NULL;

    /* Success */
    return 1;
}

/**
 * Send
 */
int
ngiCommunicationSend(
    ngiCommunication_t *comm,
    void *buf,
    size_t nBytes,
    ngLog_t *log,
    int *error)
{
    size_t writeNbytes;
    struct iovec iov;
    globus_result_t gResult;
    static const char fName[] = "ngiCommunicationSend";

    /* Check the arguments */
    assert(comm != NULL);
    assert(buf != NULL);

    /* Is length less equal zero? */
    if (nBytes <= 0) {
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	    NG_LOG_LEVEL_INFORMATION, NULL,
	    "%s: Data length (%d) is less equal zero.\n", fName, nBytes);
	return 1;
    }

    /* Print the communication log */
    iov.iov_base = buf;
    iov.iov_len = nBytes;
    ngiCommLogSendIovec(comm->ngc_commLog, &iov, 1, error);

    /* Send */
    gResult = globus_io_write(&comm->ngc_ioHandle, buf, nBytes, &writeNbytes);
    if (gResult != GLOBUS_SUCCESS) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
    	ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
            NG_LOG_LEVEL_FATAL, NULL,
            "%s: globus_io_write() failed.\n", fName);
	ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
	    NG_LOG_LEVEL_FATAL, fName, gResult, error);
	return 0;
    }

    /* Is all data sent? */
    if (writeNbytes != nBytes) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't send the all data: Request %dbytes, Sent %dbytes.\n",
	    fName, nBytes, writeNbytes);
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Send the iovec
 */
int
ngiCommunicationSendIovec(
    ngiCommunication_t *comm,
    struct iovec *iov,
    size_t nIovs,
    ngLog_t *log,
    int *error)
{
    int result;
    size_t requireNiovs;
    static const char fName[] = "ngiCommunicationSendIovec";

    /* Check the arguments */
    assert(comm != NULL);
    assert(iov != NULL);

    for (; nIovs > 0; iov += requireNiovs, nIovs -= requireNiovs) {
	requireNiovs = (nIovs <= NGI_IOV_MAX) ? nIovs : NGI_IOV_MAX;
	result = nglCommunicationSendIovec(comm, iov, requireNiovs, log, error);
	if (result == 0) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		NG_LOG_LEVEL_ERROR, NULL,
		"%s: Can't sent the data.\n", fName);
	    return 0;
	}
    }

    /* Success */
    return 1;
}

/**
 * Send the iovec
 */
static int
nglCommunicationSendIovec(
    ngiCommunication_t *comm,
    struct iovec *iov,
    size_t nIovs,
    ngLog_t *log,
    int *error)
{
    size_t writeNbytes;
    globus_result_t gResult;
    static const char fName[] = "nglCommunicationSendIovec";

    /* Check the arguments */
    assert(comm != NULL);
    assert(iov != NULL);

    /* Print the communication log */
    ngiCommLogSendIovec(comm->ngc_commLog, iov, nIovs, error);

    while (nIovs > 0) {
	/* Send */
	gResult = globus_io_writev(
	    &comm->ngc_ioHandle, iov, nIovs, &writeNbytes);
	if (gResult != GLOBUS_SUCCESS) {
	    NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	    ngLogPrintf(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, NULL,
		"%s: globus_io_writev() failed.\n", fName);
	    ngiGlobusError(log, NG_LOG_CATEGORY_GLOBUS_TOOLKIT,
		NG_LOG_LEVEL_FATAL, fName, gResult, error);
	    return 0;
	}

	/* Update the some parameters */
	while (writeNbytes > 0) {
	    if (iov->iov_len < writeNbytes) {
		writeNbytes -= iov->iov_len;
		iov++;
		nIovs--;
	    } else {
		iov->iov_base = (char *)iov->iov_base + writeNbytes;
		iov->iov_len -= writeNbytes;
		if (iov->iov_len <= 0) {
		    iov++;
		    nIovs--;
		}
		break;
	    }
	}
    }

    /* Success */
    return 1;
}

/**
 * Receive
 */
int
ngiCommunicationReceive(
    ngiCommunication_t *comm,
    void *buf,
    size_t maxNbytes,
    size_t waitNbytes,
    size_t *receiveNbytes,
    ngLog_t *log,
    int *error)
{
    int result;
    globus_result_t gResult;
    static const char fName[] = "ngiCommunicationReceive";

    /* Check the arguments */
    assert(comm != NULL);
    assert(buf != NULL);
    assert(receiveNbytes != NULL);

    /* Initialize the variables */
    *receiveNbytes = 0;

    /* Receive */
    gResult = globus_io_read(
    	&comm->ngc_ioHandle, buf, maxNbytes, waitNbytes, receiveNbytes);

    /* Print the communication log */
    if (*receiveNbytes > 0)
        ngiCommLogReceive(comm->ngc_commLog, buf, *receiveNbytes, error);

    /* Is all data received? */
    if (*receiveNbytes < waitNbytes) {
    	if (gResult != GLOBUS_SUCCESS) {
	    ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
	        NG_LOG_LEVEL_ERROR, NULL,
                "%s: globus_io_read() failed.\n", fName);
	    result = ngiGlobusIsIoEOF(gResult, log, error);
	    if (result != 0) {
	        NGI_SET_ERROR(error, NG_ERROR_DISCONNECT);
	        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL,
		    NG_LOG_LEVEL_ERROR, NULL,
                    "%s: Connection was closed.\n", fName);
	    } else {
	        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
            }
	    return 0;
        }

	NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PROTOCOL, NG_LOG_LEVEL_ERROR,
	    NULL,
	    "%s: Can't receive the all data: Request %dbytes, Received %dbytes.\n",
	    fName, waitNbytes, *receiveNbytes);
	return 0;
    }

    /* Success */
    return 1;
}
