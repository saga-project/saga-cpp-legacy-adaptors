#ifdef NGIS_NO_WARN_RCSID
static const char rcsid[] = "$RCSfile$ $Revision$ $Date$";
#endif /* NGIS_NO_WARN_RCSID */
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

#include <string.h>

#include "ngInvokeServerSSH.h"

char *
ngisSSHjobCreateExecutableCommand(
    ngisSSHjob_t *job)
{
    int result;
    char *command = NULL;
    char *quotedExec = NULL;
    NGIS_LIST_OF(char) quotedArgs;
    NGIS_LIST_ITERATOR_OF(char) it;
    NGIS_LIST_ITERATOR_OF(char) last;    
    char *arg;
    char *tmpString = NULL;
    ngisSSHjobAttributes_t *attr;
    ngisStringBuffer_t sBuf;
    int sBufInitialized = 0;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobCreateExecutableCommand";

    /* Check the arguments */
    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);
    NGIS_ASSERT(job->ngsj_log != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    ngisDebugPrint(log, fName, "Creating command line string,\n");

    NGIS_LIST_INITIALIZE(char, &quotedArgs);

    /* Quote Executable */
    if (attr->ngsja_staging != 0) {
        /* job->ngsj_remoteExecutablePath is already quoted */
        NGIS_ASSERT(job->ngsj_remoteExecutablePath != NULL);
        quotedExec = strdup(job->ngsj_remoteExecutablePath);
    } else {
        NGIS_ASSERT(attr->ngsja_executablePath != NULL);
        quotedExec = ngisShellQuote(attr->ngsja_executablePath);
    }
    if (quotedExec == NULL) {
        ngisErrorPrint(log, fName, "Can't copy string of remote executable,\n");
        goto finalize;
    }    
    
    /* Quote Arguments */
    it = NGIS_LIST_BEGIN(char, &attr->ngsja_arguments);
    last = NGIS_LIST_END(char, &attr->ngsja_arguments);    
    while (it != last) {
        arg = NGIS_LIST_GET(char, it);
        tmpString = ngisShellQuote(arg);
        if (tmpString == NULL) {
            ngisErrorPrint(log, fName, "Can't quote string of argument,\n");
            goto finalize;
        }
        result = NGIS_LIST_INSERT_TAIL(char, &quotedArgs,
            tmpString);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't insert string of argument,\n");
            goto finalize;
        }
        tmpString = NULL;
        
        it = NGIS_LIST_NEXT(char, it);
    }

    /* Create Command */
    result = ngisStringBufferInitialize(&sBuf);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't initialize a string buffer.\n");
        goto finalize;
    }
    sBufInitialized = 1;

    result = ngisStringBufferAppend(&sBuf, quotedExec);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        goto finalize;
    }
        
    it = NGIS_LIST_BEGIN(char, &quotedArgs);
    last = NGIS_LIST_END(char, &quotedArgs);
    while (it != last) {
        arg = NGIS_LIST_GET(char, it);
        result = ngisStringBufferFormat(&sBuf, " %s", arg);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't append string to the string buffer.\n");
            goto finalize;
        }
        it = NGIS_LIST_NEXT(char, it);
    }

    command = ngisStringBufferRelease(&sBuf);

    ngisDebugPrint(log, fName, "command:\n%s\n", command);

finalize:
    NGIS_NULL_CHECK_AND_FREE(tmpString);
    NGIS_NULL_CHECK_AND_FREE(quotedExec);

    if (sBufInitialized != 0) {
        ngisStringBufferFinalize(&sBuf);
        sBufInitialized = 0;
    }    

    /* List */
    it = NGIS_LIST_BEGIN(char, &quotedArgs);
    last = NGIS_LIST_END(char, &quotedArgs);
    while (it != last) {
        arg = NGIS_LIST_GET(char, it);
        NGIS_ASSERT(arg != NULL);
        free(arg);
        it = NGIS_LIST_ERASE(char, it);
    }
    NGIS_LIST_FINALIZE(char, &quotedArgs);

    return command;
}

char *
ngisSSHjobCreateSSHcommand(
    ngisSSHjob_t *job,
    char *command)
{
    ngisStringBuffer_t sBuf;
    int sBufInitialized = 0;
    char *ret = NULL;
    char *quotedCommand = NULL;
    char *shCommand = NULL;
    char *quotedShCommand = NULL;
    NGIS_LIST_ITERATOR_OF(char) it;
    NGIS_LIST_ITERATOR_OF(char) last;
    ngisSSHjobAttributes_t *attr;
    int result;
    char *opt;
    ngisLog_t *log;
    static const char fName[] = "ngisSSHjobCreateSSHcommand";
    
    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    result = ngisStringBufferInitialize(&sBuf);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't initialize a string buffer.\n");
        goto finalize;
    }
    sBufInitialized = 1;

    /* "ssh" */
    result = ngisStringBufferAppend(&sBuf, attr->ngsja_sshCommand);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't append to string buffer.\n");
        goto finalize;
    }

    /* -p port */
    if (attr->ngsja_port > 0) {
        result = ngisStringBufferFormat(&sBuf, " -p %u", attr->ngsja_port);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't append port number to buffer.\n");
            goto finalize;
        }
    }

    /* -l user */
    if (attr->ngsja_sshUser != NULL) {
        result = ngisStringBufferFormat(&sBuf, " -l %s", attr->ngsja_sshUser);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't append user name to buffer.\n");
            goto finalize;
        }
    }

    /* Options */
    it = NGIS_LIST_BEGIN(char, &attr->ngsja_sshOptions);
    last = NGIS_LIST_END(char, &attr->ngsja_sshOptions);
    while(it != last) {
        opt = NGIS_LIST_GET(char, it);
        result = ngisStringBufferFormat(&sBuf, " %s", opt);
        if (result == 0) {
            ngisErrorPrint(log, fName, "Can't append option to buffer.\n");
            goto finalize;
        }
        it = NGIS_LIST_NEXT(char, it);
    }

    /* hostname */
    result = ngisStringBufferFormat(&sBuf, " %s", attr->ngsja_hostname);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't append hostname to buffer.\n");
        goto finalize;
    }

    /* Remote Command */
    if (command == NULL) {
        quotedShCommand = ngisShellQuote(attr->ngsja_sshRemoteSh);
    } else {
        quotedCommand = ngisShellQuote(command); 
        if (quotedCommand == NULL) {
            ngisErrorPrint(log, fName, "Can't quote string.\n");
            goto finalize;
        }
        shCommand = ngisStrdupPrintf(
            "%s -c %s", attr->ngsja_sshRemoteSh, quotedCommand);
        if (shCommand == NULL) {
            ngisErrorPrint(log, fName,
                "Can't allocate storage for string.\n");
            goto finalize;
        }
        quotedShCommand = ngisShellQuote(shCommand);
    }
    if (quotedShCommand == NULL) {
        ngisErrorPrint(log, fName, "Can't quote string.\n");
        goto finalize;
    }

    result = ngisStringBufferFormat(&sBuf, " %s", quotedShCommand);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't append remote command to buffer.\n");
        goto finalize;
    }

    ret = ngisStringBufferRelease(&sBuf);

    ngisDebugPrint(log, fName, "command is\n%s.\n", ret);            
        
finalize:
    NGIS_NULL_CHECK_AND_FREE(quotedCommand);
    NGIS_NULL_CHECK_AND_FREE(shCommand);
    NGIS_NULL_CHECK_AND_FREE(quotedShCommand);

    if (sBufInitialized != 0) {
        ngisStringBufferFinalize(&sBuf);
        sBufInitialized = 0;
    }

    return ret;
}

char *
ngisSSHjobCreateMPIcommand(
    ngisSSHjob_t *job,
    const char *machinefile)
{
    char *command = NULL;
    char *option= NULL;
    ngisLog_t *log = NULL;
    ngisSSHjobAttributes_t *attr;
    ngisStringBuffer_t sBuf;
    int sBufInitialized = 0;
    NGIS_LIST_ITERATOR_OF(char) it;
    int result;
    static const char fName[] = "ngisSSHjobCreateMPIcommand";

    NGIS_ASSERT(job != NULL);
    NGIS_ASSERT(job->ngsj_attributes != NULL);

    attr = job->ngsj_attributes;
    log  = job->ngsj_log;

    result = ngisStringBufferInitialize(&sBuf);
    if (result == 0) {
        ngisErrorPrint(log, fName, "Can't initialize the string buffer.\n");
        goto finalize;
    }
    sBufInitialized = 1;

    /* Append MPI command */
    result = ngisStringBufferFormat(&sBuf, "%s ", attr->ngsja_sshMPIcommand);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append MPI command string to the string buffer.\n");
        goto finalize;
    }

    /* Append MPI machinefile option(-machinefile %s) */
    if (machinefile != NULL) {
        result = ngisStringBufferFormat(&sBuf,
            attr->ngsja_sshMPImachinefileOption,
            machinefile);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't append string to the string buffer.\n");
            goto finalize;
        }
    }
    result = ngisStringBufferAppend(&sBuf, " ");
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        goto finalize;
    }

    /* Append MPI other option */
    it = NGIS_LIST_BEGIN(char, &attr->ngsja_sshMPIoptions);
    while (it != NGIS_LIST_END(char, &attr->ngsja_sshMPIoptions)) {
        option = NGIS_LIST_GET(char, it);
        result = ngisStringBufferFormat(&sBuf, " %s", option);
        if (result == 0) {
            ngisErrorPrint(log, fName,
                "Can't append string to the string buffer.\n");
            goto finalize;
        }
        it = NGIS_LIST_NEXT(char ,it);
    }

    result = ngisStringBufferAppend(&sBuf, " ");
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        goto finalize;
    }

    /* Append MPI number of processors option(-np %d) */
    result = ngisStringBufferFormat(&sBuf, attr->ngsja_sshMPInProcessorsOption,
        attr->ngsja_count);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        goto finalize;
    }

    /* Append command of Ninf-G Remote Executable */
    result = ngisStringBufferFormat(&sBuf, " %s", job->ngsj_command);
    if (result == 0) {
        ngisErrorPrint(log, fName,
            "Can't append string to the string buffer.\n");
        goto finalize;
    }

    command = ngisStringBufferRelease(&sBuf);

    ngisDebugPrint(log, fName, "MPI command = \"%s\".\n", command);

finalize:
    ngisStringBufferFinalize(&sBuf);
    sBufInitialized = 0;

    return command;
}
