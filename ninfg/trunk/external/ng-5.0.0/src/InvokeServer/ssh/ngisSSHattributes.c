/*
 * $RCSfile: ngisSSHattributes.c,v $ $Revision: 1.6 $ $Date: 2008/03/03 09:11:21 $
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

#include "ngInvokeServerSSH.h"

NGI_RCSID_EMBED("$RCSfile: ngisSSHattributes.c,v $ $Revision: 1.6 $ $Date: 2008/03/03 09:11:21 $")

static char *ngislMPIoptionStringToFormat(char *option, char special);

/* Job Attribute */
ngisSSHjobAttributes_t *
ngisSSHjobAttributesCreate(
    ngisOptionContainer_t *opts)
{
    ngisSSHjobAttributes_t *new = NULL;
    int r;
    char *mpiNPoption          = NULL;
    char *mpiMachineFileOption = NULL;
    static const char fName[] = "ngisSSHjobAttributesCreate";

    NGIS_ASSERT(opts != NULL);

    new = NGIS_ALLOC(ngisSSHjobAttributes_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't allocate storage for SSH job attributes.\n");
        goto error;
    }

    /* Initialize Members */
    new->ngsja_attributes       = NULL;
    new->ngsja_sshCommand       = NULL;
    new->ngsja_sshRemoteSh      = NULL;
    new->ngsja_sshUser          = NULL;
    new->ngsja_sshMPIcommand    = NULL;
    new->ngsja_sshRemoteTempdir = NULL;
    new->ngsja_sshSubmitCommand = NULL;
    new->ngsja_sshStatusCommand = NULL;
    new->ngsja_sshDeleteCommand = NULL;
    new->ngsja_sshMPInProcessorsOption  = NULL;
    new->ngsja_sshMPImachinefileOption  = NULL;
    new->ngsja_sshSGEparallelEnvironment = NULL;
    new->ngsja_sshPBSprocessorsPerNode  = 1; /* Default Value */
    new->ngsja_sshPBSrsh                = NULL;

    NGIS_LIST_SET_INVALID_VALUE(&new->ngsja_sshOptions);
    NGIS_LIST_SET_INVALID_VALUE(&new->ngsja_sshMPIoptions);

    /* List Initialize */
    NGIS_LIST_INITIALIZE(char, &new->ngsja_sshOptions);
    NGIS_LIST_INITIALIZE(char, &new->ngsja_sshMPIoptions);

    /* Create Job Attribute */
    new->ngsja_attributes = ngisJobAttributesCreate(opts);
    if (new->ngsja_attributes == NULL) {
        ngisErrorPrint(NULL, fName, "Can't create job attributes.\n");
        goto error;
    }

    /* Get Attributes of SSH job*/
    r = 1;
    r = r && ngisAttrSetString(&new->ngsja_sshCommand,       opts, "ssh_command",          0);
    r = r && ngisAttrSetString(&new->ngsja_sshRemoteSh,      opts, "ssh_remoteSh",         0);
    r = r && ngisAttrSetString(&new->ngsja_sshUser,          opts, "ssh_user",             0);
    r = r && ngisAttrSetString(&new->ngsja_sshMPIcommand,    opts, "ssh_MPIcommand",       0);
    r = r && ngisAttrSetString(&new->ngsja_sshRemoteTempdir, opts, "ssh_remoteTempdir",    0);
    r = r && ngisAttrSetStringList(&new->ngsja_sshOptions,   opts, "ssh_option"             );
    r = r && ngisAttrSetString(&new->ngsja_sshSubmitCommand, opts, "ssh_submitCommand",   0);
    r = r && ngisAttrSetString(&new->ngsja_sshStatusCommand, opts, "ssh_statusCommand",   0);
    r = r && ngisAttrSetString(&new->ngsja_sshDeleteCommand, opts, "ssh_deleteCommand",   0);

    r = r && ngisAttrSetStringList(&new->ngsja_sshMPIoptions,        opts, "ssh_MPIoption"                     );
    r = r && ngisAttrSetString(&new->ngsja_sshMPInProcessorsOption,  opts, "ssh_MPInumberOfProcessorsOption", 0);
    r = r && ngisAttrSetString(&new->ngsja_sshMPImachinefileOption,  opts, "ssh_MPImachinefileOption",        0);
    r = r && ngisAttrSetString(&new->ngsja_sshSGEparallelEnvironment, opts, "ssh_SGEparallelEnvironment",       0);
    r = r && ngisAttrSetInt(&new->ngsja_sshPBSprocessorsPerNode,     opts, "ssh_PBSprocessorsPerNode",        0);
    r = r && ngisAttrSetString(&new->ngsja_sshPBSrsh,                opts, "ssh_PBSrsh",                      0);

    if (r == 0) {
        goto error;
    }

    if (new->ngsja_commProxyStaging) {
        ngisErrorPrint(NULL, fName,
            "Invoke Server SSH does not support staging of Remote Communication Proxy.\n");
        goto error;
    }

    /* Set Default */
    r = 1;
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshRemoteTempdir, NGIS_DEFAULT_REMOTE_TEMPDIR);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshCommand,       NGIS_DEFAULT_SSH_COMMAND);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshRemoteSh,      NGIS_SHELL);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshMPIcommand,    NGIS_DEFAULT_MPI_COMMAND);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshSubmitCommand, NGIS_DEFAULT_SUBMIT_COMMAND);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshStatusCommand, NGIS_DEFAULT_STATUS_COMMAND);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshDeleteCommand, NGIS_DEFAULT_DELETE_COMMAND);

    r = r && ngisAttrDefaultSetString(&new->ngsja_sshSGEparallelEnvironment , NGIS_DEFAULT_SGE_PE);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshMPInProcessorsOption, NGIS_MPI_DEFAULT_N_PROCESSORS_OPTION);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshMPImachinefileOption, NGIS_MPI_DEFAULT_MACHINEFILE_OPTION);
    r = r && ngisAttrDefaultSetString(&new->ngsja_sshPBSrsh, NGIS_DEFAULT_SSH_COMMAND);

    if (r == 0) {
        goto error;
    }

    mpiNPoption = ngislMPIoptionStringToFormat(
        new->ngsja_sshMPInProcessorsOption, 'd');
    if (mpiNPoption == NULL) {
        ngisErrorPrint(NULL, fName,
            "Error in processing \"%s\".\n",
            new->ngsja_sshMPInProcessorsOption);
        goto error;
    }
    free(new->ngsja_sshMPInProcessorsOption);
    new->ngsja_sshMPInProcessorsOption = mpiNPoption;

    mpiMachineFileOption = ngislMPIoptionStringToFormat(
        new->ngsja_sshMPImachinefileOption, 's');
    if (mpiMachineFileOption == NULL) {
        ngisErrorPrint(NULL, fName,
            "Error in processing \"%s\".\n",
            new->ngsja_sshMPImachinefileOption);
        goto error;
    }
    free(new->ngsja_sshMPImachinefileOption);
    new->ngsja_sshMPImachinefileOption = mpiMachineFileOption;

    return new;

    /* Error occurred */
error:

    if (new != NULL) {
        ngisSSHjobAttributesDestroy(new);
        new = NULL;
    }
    return NULL;
}

void
ngisSSHjobAttributesDestroy(
    ngisSSHjobAttributes_t *sshAttr)
{
    NGIS_LIST_ITERATOR_OF(char) it;
    NGIS_LIST_ITERATOR_OF(char) last;
#if 0
    static const char fName[] = "ngisSSHjobAttributesDestroy";
#endif
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshDeleteCommand);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshStatusCommand);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshSubmitCommand);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshCommand);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshRemoteSh);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshUser);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshMPIcommand);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshRemoteTempdir);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshMPInProcessorsOption);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshMPImachinefileOption);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshSGEparallelEnvironment);
    NGIS_NULL_CHECK_AND_FREE(sshAttr->ngsja_sshPBSrsh);

    if (!NGIS_LIST_IS_INVALID_VALUE(&sshAttr->ngsja_sshMPIoptions)) {
        it = NGIS_LIST_BEGIN(char, &sshAttr->ngsja_sshMPIoptions);
        last = NGIS_LIST_END(char, &sshAttr->ngsja_sshMPIoptions);
        while (it != last) {
            free(NGIS_LIST_GET(char, it));
            it = NGIS_LIST_ERASE(char, it);
        }
        NGIS_LIST_FINALIZE(char, &sshAttr->ngsja_sshMPIoptions);
    }

    if (!NGIS_LIST_IS_INVALID_VALUE(&sshAttr->ngsja_sshOptions)) {
        it = NGIS_LIST_BEGIN(char, &sshAttr->ngsja_sshOptions);
        last = NGIS_LIST_END(char, &sshAttr->ngsja_sshOptions);
        while (it != last) {
            free(NGIS_LIST_GET(char, it));
            it = NGIS_LIST_ERASE(char, it);
        }
        NGIS_LIST_FINALIZE(char, &sshAttr->ngsja_sshOptions);
    }

    if (sshAttr->ngsja_attributes != NULL) {
        ngisJobAttributesDestroy(sshAttr->ngsja_attributes);
        sshAttr->ngsja_attributes = NULL;
    }

    NGIS_FREE(sshAttr);

    return;
}

static char *
ngislMPIoptionStringToFormat(char *option, char special)
{
    char *buffer = NULL;
    char *src;
    char *dest;
    int  specialAppeared = 0;
    static const char fName[] = "ngislMPIoptionStringToFormat";

    NGIS_ASSERT(option != NULL);

    /* Result string's length is no more * than (strlen(option) * 2) */
    buffer = malloc(strlen(option) * 2 + 1);
    if (buffer == NULL) {
        ngisErrorPrint(NULL, fName, "malloc: %s.\n", strerror(errno));
        goto error;
    }

    src  = option;
    dest = buffer;

    while (*src != '\0') {
        switch (*src) {
        case '\n':
        case '\r':
            ngisErrorPrint(NULL, fName, "Invalid character.\n");
            goto error;
            break;
        default:
            break;
        }
        *dest++ = *src;
        if (*src++ == '%') {
            if (*src == '\0') {
                /* %\0 -> %%\0 */
                *dest++ = '%';
            }else if (*src == special) {
                /* %d */
                if (specialAppeared != 0) {
                    ngisErrorPrint(NULL, fName, 
                        "There are more than one \"%%%c\" in option.\n",
                        special);
                    goto error;
                }
                specialAppeared = 1;
                *dest++ = *src++;
            } else if (*src == '%') {
                /* %% -> %% */
                *dest++ = *src++;
            } else {
                /* %X -> %%X */
                *dest++ = '%';
                *dest++  = *src++;
            }
        }
    }
    *dest =  '\0';
    
    if (specialAppeared == 0) {
        ngisErrorPrint(NULL, fName, 
            "There are not \"%%%c\" in option.\n",
            special);
        goto error;
    }

    ngisDebugPrint(NULL, fName, "MPI option = \"%s\".\n", buffer);

    return buffer;
error:

    NGIS_NULL_CHECK_AND_FREE(buffer);

    return NULL;
}
