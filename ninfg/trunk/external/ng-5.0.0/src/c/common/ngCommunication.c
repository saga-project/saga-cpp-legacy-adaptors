/*
 * $RCSfile: ngCommunication.c,v $ $Revision: 1.25 $ $Date: 2008/02/19 06:36:34 $
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
 * Module for managing Communication for Ninf-G Client/Executable.
 */

#include "ngInternal.h"

NGI_RCSID_EMBED("$RCSfile: ngCommunication.c,v $ $Revision: 1.25 $ $Date: 2008/02/19 06:36:34 $")

#define NGL_COMMUNICATION_TCP   "ng_tcp"
#define NGL_COMMUNICATION_LOCAL "ng_local"

/**
 * Prototype declaration of static functions.
 */
static ngiCommunication_t *nglCommunicationConstruct(
    ngiEvent_t *, ngLog_t *, int *);
static int nglCommunicationInitialize(
    ngiCommunication_t *, ngiEvent_t *, ngLog_t *, int *);
static int nglCommunicationFinalize(ngiCommunication_t *, ngLog_t *, int *);
static void nglCommunicationInitializeMember(ngiCommunication_t *);
static void nglCommunicationInitializePointer(ngiCommunication_t *);
static void nglAddressInitializeMember(ngiAddress_t *);

/**
 * Construct server.
 */
ngiCommunication_t *
ngiCommunicationConstructServer(
    ngiEvent_t *event,
    int portNo,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiIOhandle_t *handle;
    ngiCommunication_t *comm = NULL;
    int portNoAllocated;
    static const char fName[] = "ngiCommunicationConstructServer";

    /* Check the arguments */
    assert(event != NULL);

    /* Construct */
    comm = nglCommunicationConstruct(event, log, error);
    if (comm == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the Communication Manager.\n"); 
	return NULL;
    }

    handle = comm->ngc_ioHandle;

    /* Create the TCP port listener */
    portNoAllocated = 0;
    result = ngiIOhandleTCPlistenerCreate(
        handle, portNo, &portNoAllocated,
        NGI_IO_HANDLE_LISTENER_CREATE_BACKLOG_DEFAULT, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "TCP listener create failed.\n"); 
	goto error;
    }

    comm->ngc_portNo = portNoAllocated;

    /* log */
    ngLogDebug(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
        "listener created. port is %d.\n", comm->ngc_portNo); 

    /* Success */
    return comm;

    /* Error occurred */
error:
    result = ngiCommunicationDestruct(comm, log, NULL);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't destruct the Communication Manager.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Construct accept.
 */
ngiCommunication_t *
ngiCommunicationConstructAccept(
    ngiEvent_t *event,
    ngiCommunication_t *comm,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiIOhandle_t *handle;
    ngiCommunication_t *newComm = NULL;
    static const char fName[] = "ngiCommunicationConstructAccept";

    /* Check the arguments */
    assert(comm != NULL);

    /* Construct */
    newComm = nglCommunicationConstruct(
        event, log, error);
    if (newComm == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the Communication Manager.\n"); 
	return NULL;
    }

    handle = newComm->ngc_ioHandle;

    /* Accept */
    result = ngiIOhandleTCPaccept(
        comm->ngc_ioHandle, handle, &newComm->ngc_acceptResult,
        log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "TCP accept failed.\n"); 
	goto error;
    }

    assert(newComm->ngc_acceptResult != NULL);

    result = ngiIOhandleAcceptResultLogOutput(
        newComm->ngc_acceptResult, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Output the Accept Result log failed.\n"); 
	goto error;
    }

    newComm->ngc_ioHandle = handle;

    /* Success */
    return newComm;

    /* Error occurred */
error:
    result = ngiCommunicationDestruct(newComm, log, NULL);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't destruct the Communication Manager.\n"); 
	return NULL;
    }

    return NULL;
}

/**
 * Construct client.
 */
ngiCommunication_t *
ngiCommunicationConstructClient(
    ngiEvent_t *event,
    int tcpNodelay,
    char *address,
    ngiConnectRetryInformation_t retryInfo,
    ngiRandomNumber_t *randomSeed,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 0;
    ngiCommunication_t *comm = NULL;
    struct timeval retryTimeval;
    ngiConnectRetryStatus_t retryStatus;
    int connectSuccess, onceFailed, retryRequired, doRetry;
    ngiAddress_t ad;
    int adInitialized = 0;
    static const char fName[] = "ngiCommunicationConstructClient";

    /* Check the arguments */
    assert(event != NULL);
    assert(address != NULL);
    assert(randomSeed != NULL);

    connectSuccess = 0;
    onceFailed = 0;
    retryRequired = 0;
    doRetry = 0;

    result = ngiAddressInitialize(&ad, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't initialize the address.\n"); 
        goto finalize;
    }
    adInitialized = 1;

    /* Parses address */
    result = ngiCommunicationParseAddress(address, &ad, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't parse the address \"%s\".\n", address); 
        goto finalize;
    }

    /* Construct */
    comm = nglCommunicationConstruct(event, log, error);
    if (comm == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Can't construct the communication manager.\n"); 
        goto finalize;
    }

    /* Initialize the Retry Status */
    result = ngiConnectRetryInitialize(
        &retryStatus, &retryInfo, randomSeed, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Retry status initialize failed.\n"); 
        goto finalize;
    }

    do {
        connectSuccess = 0;
        retryRequired = 0;
        doRetry = 0;

        /* Connect */
        switch (ad.nga_type) {
        case NGI_IOHANDLE_SOCKET_TYPE_UNIX:
            ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                "UNIX domain socket connect for path \"%s\".\n", ad.nga_path);
            result = ngiIOhandleUNIXconnect(
                comm->ngc_ioHandle, ad.nga_path, log, error);
            break;
        case NGI_IOHANDLE_SOCKET_TYPE_TCP:
            ngLogDebug(log, NG_LOGCAT_NINFG_PURE, fName,  
                "TCP connect for host \"%s\" port %hu.\n",
                ad.nga_hostname, ad.nga_port); 
            result = ngiIOhandleTCPconnect(
                comm->ngc_ioHandle, ad.nga_hostname,
                ad.nga_port, log, error);
            break;
        default:
            assert(0);
        }
        if (result == 0) {
    	    ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
    	        "Creating connection failed.\n"); 
            /* Not return */
        }

        if (result == 0) {
            connectSuccess = 0;
            onceFailed = 1;
            retryRequired = 1;
        } else {
            connectSuccess = 1;
            retryRequired = 0;
        }

        if (retryRequired != 0) {
            /* Get Next Retry */
            result = ngiConnectRetryGetNextRetrySleepTime(
                &retryStatus, &doRetry, &retryTimeval, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "Getting next retry time failed.\n"); 
                goto finalize;
            }
        }

        if (doRetry != 0) {
            /* Sleep before retry */
            result = ngiSleepTimeval(&retryTimeval, 0, log, error);
            if (result == 0) {
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                    "sleep failed.\n"); 
                goto finalize;
            }

            /* to tell loglevel == Warning user */
            ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Retrying to connect.\n"); 
        }
    } while (doRetry != 0);

    if (connectSuccess == 0) {
        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "connect was finally failed.\n"); 
        goto finalize;
    }

    if (onceFailed != 0) {
        /* to tell loglevel == Warning user */
        ngLogWarn(log, NG_LOGCAT_NINFG_PURE, fName,  
            "connect was finally successful.\n"); 
    }

    /* Finalize the Retry Status */
    result = ngiConnectRetryFinalize(&retryStatus, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Retry status finalize failed.\n"); 
        goto finalize;
    }

    /* Set TCP_NODELAY */
    if (ad.nga_type == NGI_IOHANDLE_SOCKET_TYPE_TCP) {
        result = ngiCommunicationSetTcpNodelay(
            comm, tcpNodelay, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Set TCP_NODELAY failed.\n"); 
            goto finalize;
        }
    }

    ret = 1;
finalize:
    if (adInitialized != 0) {
        result = ngiAddressFinalize(&ad, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't finalize the address.\n"); 
            error = NULL;
            ret = 0;
        }
        adInitialized = 0;
    }
    if (ret == 0) {
        error = NULL;
        result = ngiCommunicationDestruct(comm, log, NULL);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Can't destruct the Communication Manager.\n"); 
        }
        comm = NULL;
    }

    return comm;
}

/**
 * Construct
 */
static ngiCommunication_t *
nglCommunicationConstruct(
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    int result;
    ngiCommunication_t *comm = NULL;
    static const char fName[] = "nglCommunicationConstruct";

    /* Check the arguments */

    /* Allocate */
    comm = NGI_ALLOCATE(ngiCommunication_t, log, error);
    if (comm == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't allocate the storage for Communication Manager.\n"); 
	return NULL;
    }

    /* Initialize */
    result = nglCommunicationInitialize(comm, event, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't initialize the Communication Manager.\n"); 
	goto error;
    }

    /* Success */
    return comm;

    /* Error occurred */
error:
    result = NGI_DEALLOCATE(ngiCommunication_t, comm, log, NULL);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't deallocate the storage for Communication Manager.\n"); 
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

    if (comm == NULL) {
        /* Success */
        return 0;
    }

    /* Finalize */
    result = nglCommunicationFinalize(comm, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't finalize the Communication Manager.\n"); 
	return 0;
    }

    result = NGI_DEALLOCATE(ngiCommunication_t, comm, log, error);
    if (result == 0) {
        ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "Can't deallocate the storage for Communication Manager.\n"); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Initialize
 */
static int
nglCommunicationInitialize(
    ngiCommunication_t *comm,
    ngiEvent_t *event,
    ngLog_t *log,
    int *error)
{
    ngiIOhandle_t *handle;
    int result;

    static const char fName[] = "nglCommunicationInitialize";

    handle = NULL;

    /* Check the arguments */
    assert(comm != NULL);

    /* Initialize the pointer */
    nglCommunicationInitializeMember(comm);

    /* Create the handle */
    handle = ngiIOhandleConstruct(event, log, error);
    if (handle == NULL) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "I/O handle construct failed.\n"); 
	goto error;
    }
    comm->ngc_ioHandle = handle;

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Finalize */
    result = nglCommunicationFinalize(comm, log, NULL);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
    	    "Can't finalize the Communication.\n"); 
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
    static const char fName[] = "nglCommunicationFinalize";

    /* Check the arguments */
    assert(comm != NULL);

    /* Destruct the Communication Log */
    if (comm->ngc_commLog != NULL) {
        result = ngiCommunicationLogUnregister(comm, log, error);
        comm->ngc_commLog = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't unregister the Communication Log.\n"); 
            return 0;
        }
    }

    /* Destruct the I/O handle */
    if (comm->ngc_ioHandle != NULL) {
        result = ngiIOhandleDestruct(
            comm->ngc_ioHandle, log, error);
        comm->ngc_ioHandle = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
                "Can't destruct the I/O handle on Communication.\n"); 
            return 0;
        }
    }

    if (comm->ngc_acceptResult != NULL) {
        result = ngiIOhandleAcceptResultDestruct(
            comm->ngc_acceptResult, log, error);
        comm->ngc_acceptResult = NULL;
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Destruct the Accept Result failed.\n"); 
            return 0;
        }
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
}

/**
 * Initialize the pointers.
 */
static void
nglCommunicationInitializePointer(ngiCommunication_t *comm)
{
    /* Initialize the pointers */
    comm->ngc_commLog = NULL;
    comm->ngc_ioHandle = NULL;
    comm->ngc_acceptResult = NULL;
}

/**
 * Close the I/O Handle
 */
int
ngiCommunicationClose(ngiCommunication_t *comm, ngLog_t *log, int *error)
{
    static const char fName[] = "ngiCommunicationClose";
    int result;

    /* Check the arguments */
    if (comm == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "communication is NULL\n"); 
        return 0;
    }

    /* Close the I/O Handle */
    result = ngiIOhandleClose(comm->ngc_ioHandle, log, error);
    if (result == 0) {
        NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
            "The handle close failed.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get string for connect to this communication.
 */
char *
ngiCommunicationGetContactString(
    ngiCommunication_t *comm,
    const char *hostname,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCommunicationGetContactString";
    char buf[NGI_HOST_NAME_MAX] = "";
    char *contact = NULL;
    int result;
    
    if (hostname == NULL) {
        result = ngiHostnameGet(buf, NGI_HOST_NAME_MAX, log, error);
        if (result == 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,
                "Can't get the hostname.\n");
            return NULL;
        }
        hostname = &buf[0];
    }
    contact = ngiStrdupPrintf(log, error, "%s://%s:%u/",
        NGL_COMMUNICATION_TCP, hostname, comm->ngc_portNo);
    if (contact == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,
            "Can't generate the contact string.\n");
        return NULL;
    }

    return contact;
}

/**
 * Register the Communication Log
 */
int
ngiCommunicationLogRegister(
    ngiCommunication_t *comm,
    ngCommLog_t *commLog,
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Communication log was already registered.\n"); 
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
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Communication log was not registered.\n"); 
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
    int result;
    size_t writeNbytes;
    static const char fName[] = "ngiCommunicationSend";

    /* Check the arguments */
    assert(comm != NULL);
    assert(buf != NULL);

    /* Is length less equal zero? */
    if (nBytes <= 0) {
	ngLogInfo(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Data length (%lu) is less equal zero.\n",
	    (unsigned long)nBytes); 
	return 1;
    }

    /* Print the communication log */
    ngCommLogSend(comm->ngc_commLog, buf, nBytes, log, error);

    /* Send */
    result = ngiIOhandleWrite(
        comm->ngc_ioHandle, buf, nBytes, nBytes,
        &writeNbytes, log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "I/O handle write failed.\n"); 
	return 0;
    }

    /* Is all data sent? */
    if (writeNbytes != nBytes) {
    	NGI_SET_ERROR(error, NG_ERROR_COMMUNICATION);
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't send the all data: Request %lubytes, Sent %lubytes.\n",
            (unsigned long)nBytes, (unsigned long)writeNbytes); 
	return 0;
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
    static const char fName[] = "ngiCommunicationReceive";

    /* Check the arguments */
    assert(comm != NULL);
    assert(buf != NULL);
    assert(receiveNbytes != NULL);

    /* Initialize the variables */
    *receiveNbytes = 0;

    /* Receive */
    result = ngiIOhandleRead(
        comm->ngc_ioHandle, buf, maxNbytes, waitNbytes, receiveNbytes,
        log, error);
    if (result == 0) {
    	ngLogFatal(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "I/O handle read failed.\n"); 
	return 0;
    }

    /* Print the communication log */
    if (*receiveNbytes > 0)
        ngCommLogReceive(comm->ngc_commLog, buf, *receiveNbytes, log, error);

    /* Is all data received? */
    if (*receiveNbytes < waitNbytes) {
        if (*receiveNbytes == 0) {
            NGI_SET_ERROR(error, NG_ERROR_DISCONNECT);
            ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
                "Connection was closed.\n");
            return 0;
        }

	NGI_SET_ERROR(error, NG_ERROR_PROTOCOL);
	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
	    "Can't receive all data: Request %lubytes, Received %lubytes.\n",
            (unsigned long)waitNbytes, (unsigned long)*receiveNbytes); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Set TCP_NODELAY attribute
 */
int
ngiCommunicationSetTcpNodelay(
    ngiCommunication_t *comm,
    int enable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCommunicationSetTcpNodelay";
    ngiIOhandle_t *handle;
    int result;

    assert(comm != NULL);

    handle = comm->ngc_ioHandle;

    result = ngiIOhandleTCPnodelaySet(handle, enable, log, error);
    if (result == 0) {
	ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
	    "Set the TCP nodelay (%d) failed.\n", enable); 
	return 0;
    }

    /* Success */
    return 1;
}

/**
 * Get whether peer is localhost or not
 */
int
ngiCommunicationPeerIsLocalhost(
    ngiCommunication_t *comm,
    int *pIsLocal,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiCommunicationPeerIsLocalhost";
    int isLocal = 0;
    int result;

    assert(comm != NULL);
    assert(pIsLocal != NULL);

    *pIsLocal = 0;

    if (comm->ngc_acceptResult == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Communication doesn't have accept result.\n");
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        return 0;
    }

    result = ngiIOhandleAcceptResultIsLocalhost(
        comm->ngc_acceptResult, &isLocal, log, error);
    if (result == 0) {
    	ngLogError(log, NG_LOGCAT_NINFG_PROTOCOL, fName,  
    	    "Check the Accept Result failed.\n"); 
        return 0;
    }
    *pIsLocal = isLocal;

    return 1;
}


/**
 * Parse address.
 * Warning: Not supported IPv6.
 */
int
ngiCommunicationParseAddress(
    const char *string,
    ngiAddress_t *address,
    ngLog_t *log,
    int *error)
{
    ngiIOhandleSocketType_t type = NGI_IOHANDLE_SOCKET_TYPE_NONE;
    char *copy = NULL;
    char *typeString = NULL;
    char *portString = NULL;
    char *path = NULL;
    char *h = NULL;
    char *p = NULL;
    char *hostname = NULL;
    char *endp;
    int ret = 0;
    long lport = 0L;
    static const char fName[] = "ngiCommunicationParseAddress";

    assert(address != NULL);
    assert(address->nga_hostname == NULL);
    assert(address->nga_path == NULL);

    copy = ngiStrdup(string, log, error);
    if (copy == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't copy string.\n");
        goto finalize;
    }

    /* Protocol */
    p = strstr(copy, "://");
    if (p == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" doesn't include \"://\".\n", copy);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        goto finalize;
    }
    typeString = copy;
    *p = '\0';
    p += strlen("://");

    if (strcmp(typeString, NGL_COMMUNICATION_TCP) == 0) {
        type = NGI_IOHANDLE_SOCKET_TYPE_TCP;
        h = p;
        p = strstr(h, ":");
        if (p == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't find port number:%s\n",  string);
            goto finalize;
        }
        portString = p + 1;
        *p = '\0';

        hostname = ngiStrdup(h, log, error);
        if (hostname == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't copy string.\n");
            goto finalize;
        }

        endp = NULL;
        errno = 0;
        lport = strtol(portString, &endp, 10);
        if (errno != 0) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "strtol(%s): %s.\n", portString, strerror(errno));
            NGI_SET_ERROR(error, NG_ERROR_SYSCALL);
            goto finalize;
        }
        if ((endp == NULL) || (endp == portString)) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "\"%s\" is invalid port number.\n", portString);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }

        if (lport < NGI_PORT_MIN) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Port number \"%s\" is too small.\n", portString);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }

        if (lport > NGI_PORT_MAX) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Port number \"%s\" is too large.\n", portString);
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            goto finalize;
        }

        if (strcmp(endp, "/") != 0) {
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "path is invalid if socket type is TCP: %s.\n", endp);
            goto finalize;
        }

    } else if (strcmp(typeString, NGL_COMMUNICATION_LOCAL) == 0) {
        type = NGI_IOHANDLE_SOCKET_TYPE_UNIX;
        if (*p != '/') {
            NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "If hostname must be empty if socket type is UNIX domain socket: %s\n",
                string);
            goto finalize;
        }

        path = ngiStrdup(p, log, error);
        if (path == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                "Can't copy string.\n");
            goto finalize;
        }
    } else {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "\"%s\" is unknown socket type.\n", typeString);
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        goto finalize;
    }

    ret = 1;
finalize:

    ngiFree(copy, log ,error);

    if (ret != 0) {
        address->nga_type     = type;
        address->nga_hostname = hostname;
        address->nga_port     = lport;
        address->nga_path     = path;
    } else {
        ngiFree(path,     log, NULL);
        ngiFree(hostname, log, NULL);
    }

    return ret;
}

int
ngiAddressInitialize(
    ngiAddress_t *address,
    ngLog_t *log,
    int *error)
{
    assert(address != NULL);

    nglAddressInitializeMember(address);

    return 1;
}

int
ngiAddressFinalize(
    ngiAddress_t *address,
    ngLog_t *log,
    int *error)
{
    int result;
    int ret = 1;
    static const char fName[] = "ngiAddressFinalize";
    assert(address != NULL);

    result = ngiFree(address->nga_hostname, log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }
    result = ngiFree(address->nga_path , log, error);
    if (result == 0) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName, 
            "Can't free the string.\n");
        error = NULL;
        ret = 0;
    }

    nglAddressInitializeMember(address);

    return ret;
}

static void
nglAddressInitializeMember(
    ngiAddress_t *address)
{
    assert(address != NULL);

    address->nga_type     = NGI_IOHANDLE_SOCKET_TYPE_NONE;
    address->nga_hostname = NULL;
    address->nga_port     = 0;
    address->nga_path     = NULL;

    return;
}

