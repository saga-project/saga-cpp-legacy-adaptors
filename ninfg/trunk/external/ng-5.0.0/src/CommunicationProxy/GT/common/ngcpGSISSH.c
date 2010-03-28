/*
 * $RCSfile: ngcpGSISSH.c,v $ $Revision: 1.3 $ $Date: 2008/03/28 03:52:30 $
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

#include "ngcpXIO.h"
#include "ngcpRelayHandler.h"

NGI_RCSID_EMBED("$RCSfile: ngcpGSISSH.c,v $ $Revision: 1.3 $ $Date: 2008/03/28 03:52:30 $")

#define NGCPL_GSISSH "gsissh"
#define NGCPL_BUFFERSIZE (32 * 1024)
#define NGCPL_LOGCAT_GSISSH "gsissh"

#define NGL_UINT16_MAX_DIGITS 5 /* ceil(16 * log10(2)) */

/**
 * Buffer for STDOUT and STDERR.
 */
typedef struct ngcpLineBuffer_s {
    char             *nglb_name;
    char              nglb_buffer[NGCPL_BUFFERSIZE+1];
    ngcpGSISSHinfo_t *nglb_info;
    bool              nglb_addressline;
} ngcpLineBuffer_t;

/**
 * Result of write operation
 */
typedef struct ngcplWriteResult_s {
    ngcpGSISSHinfo_t *ngwr_info;
    globus_size_t     ngwr_nWrite;
    globus_result_t   ngwr_result;
} ngcplWriteResult_t;

static ngemResult_t ngcplLineBufferRead(globus_xio_handle_t, ngcpGSISSHinfo_t *, char *, bool);
static ngcpLineBuffer_t *ngcplLineBufferCreate(void);
static void ngcplLineBufferDestroy(ngcpLineBuffer_t *);
static void ngcplLineBufferReadCallback(
        globus_xio_handle_t, globus_result_t, globus_byte_t *, globus_size_t,
    globus_size_t, globus_xio_data_descriptor_t, void *);
static void ngcplLineBufferWriteCallback(
    globus_xio_handle_t, globus_result_t, globus_byte_t *, globus_size_t,
    globus_size_t, globus_xio_data_descriptor_t, void *);
static char * ngcplCreateGSISSHcommand(char *, char *, NGEM_LIST_OF(char) *);
static bool ngcplStringIsNum(char *);
static ngemResult_t ngcplGSISSHinvokeRelay(ngcpGSISSHinfo_t *, char *, NGEM_LIST_OF(char) *, ngcpCommunicationSecurity_t);
static ngemResult_t ngcplGSISSHinfoCallbackCountDec(ngcpGSISSHinfo_t *);
static char *ngcplCreateCommandString(char *, NGEM_LIST_OF(char) *, ngcpCommunicationSecurity_t);

static char *ngcplLineBufferGetNext(ngcpLineBuffer_t *, char *, char *);
static void ngcplLineBufferCloseCallback(globus_xio_handle_t , globus_result_t, void *);

/**
 * Invoke Client/Remote Relay using GSISSH
 */
ngcpGSISSHinfo_t *
ngcpGSISSHinvoke(
    char *contactString,
    char *gsisshCommand,
    NGEM_LIST_OF(char) *gsisshOptions,
    char *command,
    NGEM_LIST_OF(char) *options,
    ngcpCommunicationSecurity_t crypt,
    ngcpCommonLock_t *lock)
{
    char *args[4];
    ngemResult_t nResult;
    int i;
    ngLog_t *log;
    bool locked = false;
    char *commandString = NULL;
    ngcpGSISSHinfo_t *info = NULL;
    NGEM_FNAME(ngcpGSISSHinvoke);

    log = ngemLogGetDefault();

    NGEM_ASSERT(contactString != NULL);
    NGEM_ASSERT(command != NULL);
    NGEM_ASSERT(lock != NULL);

    info = NGI_ALLOCATE(ngcpGSISSHinfo_t, log, NULL);
    if (info == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't allocate storage for the gsissh information.\n");
        goto finalize;
    }
    info->nggs_sio.ngsio_in  = NULL;
    info->nggs_sio.ngsio_out = NULL;
    info->nggs_sio.ngsio_err = NULL;
    info->nggs_pid           = -1;
    info->nggs_address       = NULL;
    info->nggs_lock          = lock;
    info->nggs_callbackCount = 0;

    commandString = ngcplCreateGSISSHcommand(gsisshCommand, contactString, gsisshOptions);
    if (commandString == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't create the command string for invoking gsissh command.\n");
        goto finalize;
    }

    /* Create command */
    i = 0;
    args[i++] = NGEM_SHELL;
    args[i++] = "-c";
    args[i++] = commandString;
    args[i++] = NULL;

    info->nggs_pid = ngcpXIOpopenArgv(&info->nggs_sio, args, 0, 0, NULL);
    if (info->nggs_pid < 0) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't execute gsissh command.\n");
        goto finalize;
    }

    nResult = ngcplGSISSHinvokeRelay(info, command, options, crypt);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't invoke the relay.\n");
        goto finalize;
    }

    nResult = ngcpCommonLockLock(info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
        goto finalize;
    }
    locked = true;

    nResult = ngcplLineBufferRead(info->nggs_sio.ngsio_out, info, "stdout", true);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't read the line from stdout of child process.\n");
        goto finalize;
    }
    nResult = ngcplLineBufferRead(info->nggs_sio.ngsio_err, info, "stderror", false);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't read the line from stderr of child process.\n");
        goto finalize;
    }

    while ((info->nggs_address == NULL) &&
           (info->nggs_callbackCount > 0) &&
           (!NGCP_COMMON_LOCK_FINALIZING(lock))) {
        nResult = ngcpCommonLockWait(info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't wait the lock.\n");
            goto finalize;
        }
    }
    
    if (info->nggs_address == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get contact string.\n");
        goto finalize;
    }

finalize:

    if ((info != NULL) && (info->nggs_address == NULL)) {
        /* Failed */
        ngcpGSISSHinfoDestroy(info);
        info = NULL;
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(lock);
        if (nResult == 0) {
            ngLogFatal(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't release the lock.\n");
        }
    }

    ngiFree(commandString, log, NULL);

    return info;
}

/**
 * GSISSH information: Destroy
 * Kills GSISSH process.
 */
void
ngcpGSISSHinfoDestroy(
    ngcpGSISSHinfo_t *info)
{
    bool locked = false;
    ngemResult_t nResult;
    globus_result_t gResult;
    globus_xio_handle_t handles[3];
    ngLog_t *log;
    int i;
    NGEM_FNAME(ngcpGSISSHinfoDestroy);

    log = ngemLogGetDefault();

    if (info == NULL) {
        return;
    }

    nResult = ngcpCommonLockLock(info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
    } else {
        locked = true;
    }

    i = 0;
    handles[i++] = info->nggs_sio.ngsio_in;
    handles[i++] = info->nggs_sio.ngsio_out;
    handles[i++] = info->nggs_sio.ngsio_err;
    NGEM_ASSERT(i == 3);
    for (i = 0;i < 3;++i) {
        if (handles[i] != NULL) {
            gResult = globus_xio_register_close(handles[i], NULL,
                ngcplLineBufferCloseCallback, info);
            if (gResult != GLOBUS_SUCCESS) {
                ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
                    "globus_xio_register_close", gResult);
            }
            info->nggs_callbackCount++;
        }
    }

    while(info->nggs_callbackCount > 0) {
        nResult = ngcpCommonLockWait(info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogFatal(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't wait the lock.\n");
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(info->nggs_lock);
        if (nResult == 0) {
            ngLogFatal(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't release the lock.\n");
        }
    }
    if (info->nggs_pid >= 0) {
        ngcpKillAndWait(info->nggs_pid);
        info->nggs_pid = -1;
    }
    ngiFree(info->nggs_address, log, NULL);

    info->nggs_sio.ngsio_in  = NULL;
    info->nggs_sio.ngsio_out = NULL;
    info->nggs_sio.ngsio_err = NULL;
    info->nggs_pid           = -1;
    info->nggs_address       = NULL;
    info->nggs_lock          = NULL;
    info->nggs_callbackCount = 0;

    NGI_DEALLOCATE(ngcpGSISSHinfo_t,info, log, NULL);
    info = NULL;

    return;
}

/**
 * Create Command Line of GSISSH
 */
static char *
ngcplCreateGSISSHcommand(
    char *command,
    char *host,
    NGEM_LIST_OF(char) *options)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    char *ret = NULL;
    ngemStringBuffer_t sBuf = NGEM_STRING_BUFFER_NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    char *p;
    NGEM_FNAME(ngcplCreateGSISSHcommand);

    log = ngemLogGetDefault();

    if (command == NULL) {
        command = NGCPL_GSISSH;
    }

    nResult = ngemStringBufferInitialize(&sBuf);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't initialize the string buffer.\n");
        goto finalize;
    }

    nResult = ngemStringBufferAppend(&sBuf, command);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't add string to the string buffer.\n");
        goto finalize;
    }

    NGEM_LIST_FOREACH(char, options, it, val) {
        nResult = ngemStringBufferFormat(&sBuf, " %s", val);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't add string to the string buffer.\n");
            goto finalize;
        }
    }
    p = strchr(host, ':');
    if (p != NULL) {
        if (!ngcplStringIsNum(p+1)) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't invalid port number.\n");
            goto finalize;
        }

        nResult =
            ngemStringBufferFormat(&sBuf, " -p %s ", p+1) &&
            ngemStringBufferNappend(&sBuf, host, p - host);

    } else {
        ngemStringBufferFormat(&sBuf, " %s", host);
    }
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't add string to the string buffer.\n");
        goto finalize;
    }

    ret = ngemStringBufferRelease(&sBuf);
    if (ret == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get string from the string buffer.\n");
        goto finalize;
    }

    ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
        "GSISSH Command is \"%s\".\n", ret);
finalize:
    ngemStringBufferFinalize(&sBuf);

    return ret;
}

/**
 * String is number?
 */
static bool
ngcplStringIsNum(char *s)
{
    char *p;
    if (strlen(s) ==  0) {
        return false;
    }

    for (p = s;*p != '\0';++p) {
        if (!isdigit((int)*p)) {
            return false;
        }
    }
    return true;
}

/**
 * Create Command Line of Client/Remote Relay
 */
static char *
ngcplCreateCommandString(
    char *command,
    NGEM_LIST_OF(char) *options,
    ngcpCommunicationSecurity_t crypt)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    char *ret = NULL;
    ngemStringBuffer_t sBuf = NGEM_STRING_BUFFER_NULL;
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngcplCreateCommandString);

    log = ngemLogGetDefault();

    nResult = ngemStringBufferInitialize(&sBuf);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't initialize the string buffer.\n");
        goto finalize;
    }

    nResult = ngemStringBufferAppend(&sBuf, "exec ");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't add string to the string buffer.\n");
        goto finalize;
    }

    nResult = ngemStringBufferAppend(&sBuf, command);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't add string to the string buffer.\n");
        goto finalize;
    }

    if (crypt == NGCP_COMMUNICATION_SECURITY_NONE) {
        nResult = ngemStringBufferAppend(&sBuf, " --crypt=false");
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't add string to the string buffer.\n");
            goto finalize;
        }
    }

    NGEM_LIST_FOREACH(char, options, it, val) {
        nResult = ngemStringBufferFormat(&sBuf, " %s", val);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't add string to the string buffer.\n");
            goto finalize;
        }
    }

    /* Kills for shell when command is finished */
    nResult = ngemStringBufferAppend(&sBuf, " ; exit\n");
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't add string to the string buffer.\n");
        goto finalize;
    }

    ret = ngemStringBufferRelease(&sBuf);
    if (ret == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get string from the string buffer.\n");
        goto finalize;
    }

    ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
        "Command is \"%s\".\n", ret);
finalize:
    ngemStringBufferFinalize(&sBuf);

    return ret;
}

/**
 * Invokes Client/Remote Relay
 * (Writes command to stdin of remote shell)
 */
static ngemResult_t
ngcplGSISSHinvokeRelay(
    ngcpGSISSHinfo_t *info,
    char *command,
    NGEM_LIST_OF(char) *options,
    ngcpCommunicationSecurity_t crypt)
{
    ngcplWriteResult_t writeResult;
    char *commandString = NULL;
    bool locked = false;
    ngemResult_t ret = NGEM_FAILED;
    globus_result_t gResult;
    ngLog_t *log = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngcplGSISSHinvokeRelay);

    log = ngemLogGetDefault();

    commandString = ngcplCreateCommandString(command, options, crypt);
    if (commandString == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't create the command string for \"%s\".\n", command);
        goto finalize;
    }

    nResult = ngcpCommonLockLock(info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
        goto finalize;
    }
    locked = true;

    /* Invoke relay in remote */
    writeResult.ngwr_info   = info;
    writeResult.ngwr_nWrite = 0;
    writeResult.ngwr_result = GLOBUS_SUCCESS;
    gResult = globus_xio_register_write(info->nggs_sio.ngsio_in,
        (void *)commandString, strlen(commandString), strlen(commandString), NULL, 
        ngcplLineBufferWriteCallback, &writeResult);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCPL_LOGCAT_GSISSH, fName,
            "globus_xio_register_write", gResult);
        goto finalize;
    }
    info->nggs_callbackCount++;

    while ((info->nggs_callbackCount > 0) &&
           (!NGCP_COMMON_LOCK_FINALIZING(info->nggs_lock))) {
        nResult = ngcpCommonLockWait(info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't wait the lock.\n");
            goto finalize;
        }
    }
    if (NGCP_COMMON_LOCK_FINALIZING(info->nggs_lock)) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Process is canceled.\n");
        goto finalize;
    }
    NGEM_ASSERT(info->nggs_callbackCount == 0);

    if (writeResult.ngwr_result != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Error in callback of globus_xio_register_write()",
            writeResult.ngwr_result);
        goto finalize;
    }

    if (writeResult.ngwr_nWrite != strlen(commandString)) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't write all date.\n");
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(info->nggs_lock);
        if (nResult == 0) {
            ngLogFatal(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
    }

    ngiFree(commandString, log, NULL);

    return ret;
}

/**
 * GSI SSH information: decrement callback count
 */
static ngemResult_t
ngcplGSISSHinfoCallbackCountDec(
    ngcpGSISSHinfo_t *info)
{
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_SUCCESS;
    ngLog_t *log;
    bool locked = false;
    NGEM_FNAME(ngcplGSISSHinfoCallbackCountDec);

    log = ngemLogGetDefault();

    NGEM_ASSERT(info != NULL);

    nResult = ngcpCommonLockLock(info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
        ret = NGEM_FAILED;
    } else {
        locked = true;
    }

    NGEM_ASSERT(info->nggs_callbackCount > 0);
    info->nggs_callbackCount--;
    if (info->nggs_callbackCount == 0) {
        nResult = ngcpCommonLockBroadcast(info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't broadcast the signal.\n");
            ret = NGEM_FAILED;
        }
    }

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
    }

    return ret;
}

/**
 * Line Buffer: Starts to read
 */
static ngemResult_t
ngcplLineBufferRead(
    globus_xio_handle_t handle,
    ngcpGSISSHinfo_t *info,
    char *name,
    bool addressline)
{
    ngcpLineBuffer_t *buf = NULL;
    ngemResult_t nResult;
    ngemResult_t ret = NGEM_FAILED;
    ngLog_t *log;
    bool locked = false;
    globus_result_t gResult;
    NGEM_FNAME(ngcplLineBufferRead);

    log = ngemLogGetDefault();

    buf = ngcplLineBufferCreate();
    if (buf == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't create the line buffer.\n");
        goto finalize;
    }

    nResult = ngcpCommonLockLock(info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
        goto finalize;
    }
    locked = true;

    buf->nglb_name        = name;
    buf->nglb_info        = info;
    buf->nglb_addressline = addressline;
    gResult = globus_xio_register_read(handle,
        (globus_byte_t *)buf->nglb_buffer, NGCPL_BUFFERSIZE, 1,
        NULL, ngcplLineBufferReadCallback, buf);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCPL_LOGCAT_GSISSH, fName,
            "globus_xio_register_read", gResult);
        goto finalize;
    }
    info->nggs_callbackCount++;
    buf = NULL;

    ret = NGEM_SUCCESS;
finalize:
    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(info->nggs_lock);
        if (nResult == 0) {
            ngLogFatal(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't release the lock.\n");
            ret = NGEM_FAILED;
        }
    }
    if (buf != NULL) {
        ngcplLineBufferDestroy(buf);
        buf = NULL;
    }
    return ret;
}

/**
 * Line Buffer: Create
 */
static ngcpLineBuffer_t *
ngcplLineBufferCreate(void)
{
    ngcpLineBuffer_t *buffer = NULL;
    ngLog_t *log;
    NGEM_FNAME(ngcplLineBufferCreate);

    log = ngemLogGetDefault();

    buffer = NGI_ALLOCATE(ngcpLineBuffer_t, log, NULL);
    if (buffer == NULL) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't allocate storage for line buffer.\n");
        return NULL;
    }
    
    memset(buffer->nglb_buffer, '\0', sizeof(buffer->nglb_buffer));
    buffer->nglb_name        = NULL;
    buffer->nglb_info        = NULL;
    buffer->nglb_addressline = false;

    return buffer;
}

/**
 * Line Buffer: Destroy
 */
static void
ngcplLineBufferDestroy(
    ngcpLineBuffer_t *lb)
{
    ngLog_t *log;

    if (lb == NULL) {
        return;
    }

    log = ngemLogGetDefault();

    memset(lb->nglb_buffer, '\0', sizeof(lb->nglb_buffer));
    lb->nglb_name        = NULL;
    lb->nglb_info        = NULL;
    lb->nglb_addressline = false;

    NGI_DEALLOCATE(ngcpLineBuffer_t, lb, log, NULL);

    return;
}

/**
 * Line Buffer: Callback for reading
 */
static void
ngcplLineBufferReadCallback(
    globus_xio_handle_t handle,
    globus_result_t     cResult,
    globus_byte_t      *buffer,
    globus_size_t       len,
    globus_size_t       nbytes,
    globus_xio_data_descriptor_t data_desc,
    void *userData) 
{
    ngcpLineBuffer_t *lb = userData;
    ngLog_t *log;
    int i;
    char *copy = NULL;
    size_t rest;
    size_t l;
    char *end;
    char *head;
    char *next;
    bool locked = false;
    globus_result_t gResult;
    ngemResult_t nResult;
    ngcpCommonLock_t *lock = NULL;;
    NGEM_FNAME(ngcplLineBufferReadCallback);

    log = ngemLogGetDefault();
    lock = lb->nglb_info->nggs_lock;

    ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
        "Called with %p,\n", userData);

    if (cResult != GLOBUS_SUCCESS) {
        if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
            goto callback_end;
        }
        if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
            ngLogInfo(log, NGCPL_LOGCAT_GSISSH, fName,
                "%s: EOF.\n", lb->nglb_name);
            goto callback_end;
        }
        ngcpLogGlobusError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Callback function for reading", cResult);
        goto callback_end;
    }

    /* Info */
    ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
        "Reads %lu bytes(%p).\n", (unsigned long)nbytes, buffer);

    /* Check NULL character*/
    for (i = 0;i < nbytes;++i) {
        if (buffer[i] == '\0') {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Read data includes NULL character.\n");
            goto callback_end;
        }
    }

    end = (char *)&buffer[nbytes];
    *end = '\0';
    head = lb->nglb_buffer;
    while ((next = ngcplLineBufferGetNext(lb, head, end)) != NULL) {
        if (lb->nglb_addressline) {
            lb->nglb_addressline = false;

            /* Get Address */
            copy = ngiStrdup(head, log, NULL);
            if (copy == NULL) {
                ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                    "Can't duplicate the string.\n");
                goto callback_end;
            }
        } else {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "(pid %lu:%s)\n \"%s\"\n",
                (unsigned long)lb->nglb_info->nggs_pid, lb->nglb_name, head);
            /* PID is readonly. therefore "lock" need be not gotten. */
        }
        head = next;
    }

    if ((head  == lb->nglb_buffer) && (strlen(head) == NGCPL_BUFFERSIZE)) {
        /* Buffer is not enough for this line */
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "(pid %lu:%s)\n \"%s\"\n",
            (unsigned long)lb->nglb_info->nggs_pid, lb->nglb_name, head);
        /* PID is readonly. therefore "lock" need be not gotten. */
        head = lb->nglb_buffer;
        rest = NGCPL_BUFFERSIZE;
    } else if (*head != '\0') {
        ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
            "Slide the data\n");
        /* Slide */
        l = strlen(head);
        if (head != lb->nglb_buffer) {
            for (i = 0;i < l;++i) {
                lb->nglb_buffer[i] = head[i];
            }
        }
        head = &lb->nglb_buffer[l];
        rest = NGCPL_BUFFERSIZE - l;
    } else {
        ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
            "All data is printed.\n");
        head = lb->nglb_buffer;
        rest = NGCPL_BUFFERSIZE;
    }

    if (copy != NULL) {
        nResult = ngcpCommonLockLock(lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't get the lock.\n");
            goto callback_end;
        }
        locked = true;
        lb->nglb_info->nggs_address = copy;
        copy = NULL;
        nResult = ngcpCommonLockBroadcast(lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't broadcast the signal.\n");
            goto callback_end;
        }
    }

    gResult = globus_xio_register_read(
        handle, (globus_byte_t *)head, rest, 1, NULL,
        ngcplLineBufferReadCallback, lb);
    if (gResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCPL_LOGCAT_GSISSH, fName,
            "globus_xio_register_read", gResult);
        goto callback_end;
    }
    goto finalize;
callback_end:
    if (!locked) {
        nResult = ngcpCommonLockLock(lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't get the lock.\n");
        }
        locked = true;
    }
    NGEM_ASSERT(lb->nglb_info->nggs_callbackCount > 0);
    nResult = ngcplGSISSHinfoCallbackCountDec(lb->nglb_info);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't decrement the callback count.\n");
    }
    ngcplLineBufferDestroy(lb);
    lb = NULL;

finalize:
    if (locked ) {
        nResult = ngcpCommonLockUnlock(lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't get the lock.\n");
        }
    }
    ngiFree(copy, log, NULL);

    return;
}

/**
 * Line Buffer: Callback for writing
 */
static void
ngcplLineBufferWriteCallback(
    globus_xio_handle_t handle,
    globus_result_t cResult,
    globus_byte_t *buffer,
    globus_size_t bufferSize,
    globus_size_t nWrite,
    globus_xio_data_descriptor_t desc,
    void *userData)
{
    ngcplWriteResult_t *wr = userData;
    ngLog_t *log;
    bool locked = false;
    ngemResult_t nResult;
    NGEM_FNAME(ngcplLineBufferWriteCallback);

    log = ngemLogGetDefault();

    ngLogDebug(log, NGCPL_LOGCAT_GSISSH, fName,
        "Called with %p,\n", userData);

    if (cResult != GLOBUS_SUCCESS) {
        if (globus_xio_error_is_canceled(cResult) == GLOBUS_TRUE) {
            /* Canceled */
            return;
        }
        if (globus_xio_error_is_eof(cResult) == GLOBUS_TRUE) {
            NGEM_ASSERT_NOTREACHED();
        }
    }

    nResult = ngcpCommonLockLock(wr->ngwr_info->nggs_lock);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't get the lock.\n");
    } else {
        locked = true;
    }

    wr->ngwr_nWrite = nWrite;
    wr->ngwr_result = cResult;
    nResult = ngcplGSISSHinfoCallbackCountDec(wr->ngwr_info);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't decrement callback count.\n");
    }
    NGEM_ASSERT(wr->ngwr_info->nggs_callbackCount == 0);

    if (locked) {
        locked = false;
        nResult = ngcpCommonLockUnlock(wr->ngwr_info->nggs_lock);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
                "Can't get the lock.\n");
        }
    }

    return;
}

/**
 * Line Buffer: Callback for close
 */
static void
ngcplLineBufferCloseCallback(   
    globus_xio_handle_t handle,
    globus_result_t     cResult,
    void               *userData)
{
    ngcpGSISSHinfo_t *info = userData;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngcplLineBufferCloseCallback);

    log = ngemLogGetDefault();

    NGEM_ASSERT(userData);

    if (cResult != GLOBUS_SUCCESS) {
        ngcpLogGlobusError(log, NGCP_LOGCAT_GT, fName,
            "Callback of globus_xio_register_close", cResult);
    }
    nResult = ngcplGSISSHinfoCallbackCountDec(info);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGCPL_LOGCAT_GSISSH, fName,
            "Can't decrement callback count.\n");
    }

    return;
}   

/**
 * Line Buffer: Get next line
 */
static char *
ngcplLineBufferGetNext(
    ngcpLineBuffer_t *lb,
    char *head, 
    char *end)
{

    NGEM_ASSERT(lb != NULL);
    NGEM_ASSERT(head != NULL);
    NGEM_ASSERT(end != NULL);
    NGEM_ASSERT(head <= end);

    for (;head != end;++head) {
        if (*head == '\n') {
            *head = '\0';
            return head+1;
        }
    }
    return NULL;
}
