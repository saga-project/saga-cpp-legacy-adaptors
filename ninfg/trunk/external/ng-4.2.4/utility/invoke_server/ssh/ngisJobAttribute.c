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
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <limits.h>

#include "ngisUtility.h"
#include "ngInvokeServer.h"

const char *ngisJobStatusStrings[] = {
    "PENDING",
    "ACTIVE",
    "DONE",
    "FAILED"
};

static char *ngislAttrBoolStrings[] = {
    "false",
    "true",
};

static char *ngislAttrBackendStrings[] = {
    "NORMAL",
    "MPI",
    "BLACS",
};

static void ngislJobAttributesInitializeMember(ngisJobAttributes_t *);
static void ngislAttrEraseName(
    ngisOptionContainer_t *, ngisOption_t, char *);

/**
 *  Job Attributes: Create
 */
ngisJobAttributes_t *
ngisJobAttributesCreate(
    ngisOptionContainer_t *opts)
{
    ngisJobAttributes_t *new = NULL;
    int r;
    static const char fName[] = "ngisJobAttributesCreate";

    new = NGIS_ALLOC(ngisJobAttributes_t);
    if (new == NULL) {
        ngisErrorPrint(NULL, fName,
            "Can't allocate storage for job attributes.\n");
        goto error;
    }
    ngislJobAttributesInitializeMember(new);
    
    /* List */
    NGIS_LIST_INITIALIZE(char, &new->ngja_arguments);
    NGIS_LIST_INITIALIZE(char, &new->ngja_environments);
    NGIS_LIST_INITIALIZE(char, &new->ngja_rslExtensions);
    
    /* Set value */
    r = 1;
    r = r && ngisAttrSetString(&new->ngja_hostname,         opts, "hostname",           1);
    r = r && ngisAttrSetUshort(&new->ngja_port,             opts, "port",               1);
    r = r && ngisAttrSetString(&new->ngja_jobManager,       opts, "jobmanager",         0);
    r = r && ngisAttrSetString(&new->ngja_clientHostname,   opts, "client_name",        1);
    r = r && ngisAttrSetString(&new->ngja_executablePath,   opts, "executable_path",    1);
    r = r && ngisAttrSetBackend(&new->ngja_jobBackend,      opts, "backend",            1);
    r = r && ngisAttrSetStringList(&new->ngja_arguments,    opts, "argument"             );
    r = r && ngisAttrSetString(&new->ngja_workDirectory,    opts, "work_directory",     0);
    r = r && ngisAttrSetStringList(&new->ngja_environments, opts, "environment"          );
    r = r && ngisAttrSetUint(&new->ngja_statusPolling,      opts, "status_polling",     1);
    r = r && ngisAttrSetUint(&new->ngja_refreshCredential,  opts, "refresh_credential", 1);
    r = r && ngisAttrSetUint(&new->ngja_count,              opts, "count",              1);
    r = r && ngisAttrSetBool(&new->ngja_staging,            opts, "staging",            1);
    r = r && ngisAttrSetBool(&new->ngja_redirectEnable,     opts, "redirect_enable",    1);
    r = r && ngisAttrSetString(&new->ngja_stdoutFile,       opts, "stdout_file",        0);
    r = r && ngisAttrSetString(&new->ngja_stderrFile,       opts, "stderr_file",        0);
    r = r && ngisAttrSetInt(&new->ngja_maxTime,             opts, "max_time",           0);
    r = r && ngisAttrSetInt(&new->ngja_maxWallTime,         opts, "max_wall_time",      0);
    r = r && ngisAttrSetInt(&new->ngja_maxCpuTime,          opts, "max_cpu_time",       0);
    r = r && ngisAttrSetString(&new->ngja_queueName,        opts, "queue_name",         0);
    r = r && ngisAttrSetString(&new->ngja_project,          opts, "project",            0);
    r = r && ngisAttrSetInt(&new->ngja_hostCount,           opts, "host_count",         0);
    r = r && ngisAttrSetInt(&new->ngja_minMemory,           opts, "min_memory",         0);
    r = r && ngisAttrSetInt(&new->ngja_maxMemory,           opts, "max_memory",         0);
    r = r && ngisAttrSetString(&new->ngja_tmpDir,           opts, "tmp_dir",            0);
    r = r && ngisAttrSetString(&new->ngja_gassURL,          opts, "gass_url",           0);
    r = r && ngisAttrSetStringList(&new->ngja_rslExtensions,opts, "rsl_extensions"       );
    
    if (r == 0) {
        goto error;
    }

    if (new->ngja_redirectEnable != 0) {
        if (new->ngja_stdoutFile == NULL) {
            ngisErrorPrint(NULL, fName, "\"redirect_enable\" is true, "
                                       "but \"stdout_file\" doesn't exist.\n");
            goto error;
        }
        if (new->ngja_stderrFile == NULL) {
            ngisErrorPrint(NULL, fName, "\"redirect_enable\" is true, "
                                       "but \"stderr_file\" doesn't exist.\n");
            goto error;
        }
    }
    
    return new;
error:    
    if (new != NULL) {
        ngisJobAttributesDestroy(new);
        new = NULL;
    }

    return NULL;
}

static void
ngislJobAttributesInitializeMember(
    ngisJobAttributes_t *attr)
{
#if 0
    static const char fName[] = "ngislJobAttributesInitializeMember";
#endif

    NGIS_ASSERT(attr != NULL);
    
    attr->ngja_hostname               = NULL;
    attr->ngja_port                   = 0;
    attr->ngja_jobManager             = NULL;
    attr->ngja_clientHostname         = NULL;
    attr->ngja_executablePath         = NULL;
    attr->ngja_jobBackend             = NGIS_BACKEND_NORMAL;
    attr->ngja_workDirectory          = NULL;
    attr->ngja_statusPolling          = 0;
    attr->ngja_refreshCredential      = 0;
    attr->ngja_count                  = 0;
    attr->ngja_staging                = 0; /* False */
    attr->ngja_redirectEnable         = 0; /* False */
    attr->ngja_stdoutFile             = NULL;
    attr->ngja_stderrFile             = NULL;
    attr->ngja_maxTime                = 0;
    attr->ngja_maxWallTime            = 0;
    attr->ngja_maxCpuTime             = 0;
    attr->ngja_queueName              = NULL;
    attr->ngja_project                = NULL;
    attr->ngja_hostCount              = 0;
    attr->ngja_minMemory              = 0;
    attr->ngja_maxMemory              = 0;
    attr->ngja_tmpDir                 = NULL;
    attr->ngja_gassURL                = NULL;

    NGIS_LIST_SET_INVALID_VALUE(&attr->ngja_arguments);
    NGIS_LIST_SET_INVALID_VALUE(&attr->ngja_environments);
    NGIS_LIST_SET_INVALID_VALUE(&attr->ngja_rslExtensions);
    
    return;
}
       
/**
 *  Job Attributes: Destroy
 */
void
ngisJobAttributesDestroy(
    ngisJobAttributes_t *attr)
{
    char *arg;
    char *env;    
    char *line;
    NGIS_LIST_ITERATOR_OF(char) it;
    NGIS_LIST_ITERATOR_OF(char) last;    
#if 0
    static const char fName[] = "ngisJobAttributesDestroy";
#endif
    
    NGIS_ASSERT(attr != NULL);
    
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_hostname);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_jobManager);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_clientHostname);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_executablePath);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_workDirectory);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_stdoutFile);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_stderrFile);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_queueName);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_project);
    NGIS_NULL_CHECK_AND_FREE(attr->ngja_gassURL);

    /* List */
    if (!NGIS_LIST_IS_INVALID_VALUE(&attr->ngja_rslExtensions)) {
        it = NGIS_LIST_BEGIN(char, &attr->ngja_rslExtensions);
        last = NGIS_LIST_END(char, &attr->ngja_rslExtensions);
        while (it != last) {
            line = NGIS_LIST_GET(char, it);
            NGIS_ASSERT(line != NULL);
            NGIS_NULL_CHECK_AND_FREE(line);
            
            it = NGIS_LIST_ERASE(char, it);
        }
        
        NGIS_LIST_FINALIZE(char, &attr->ngja_rslExtensions);
    }
    
    /* List */
    if (!NGIS_LIST_IS_INVALID_VALUE(&attr->ngja_arguments)) {
        it = NGIS_LIST_BEGIN(char, &attr->ngja_arguments);
        last = NGIS_LIST_END(char, &attr->ngja_arguments);
        while (it != last) {
            arg = NGIS_LIST_GET(char, it);
            NGIS_ASSERT(arg != NULL);
            NGIS_NULL_CHECK_AND_FREE(arg);
            
            it = NGIS_LIST_ERASE(char, it);
        }
        
        NGIS_LIST_FINALIZE(char, &attr->ngja_arguments);
    }

    if (!NGIS_LIST_IS_INVALID_VALUE(&attr->ngja_environments)) {
        it = NGIS_LIST_BEGIN(char, &attr->ngja_environments);
        last = NGIS_LIST_END(char, &attr->ngja_environments);
        while (it != last) {
            env = NGIS_LIST_GET(char, it);
            NGIS_ASSERT(env != NULL);
            NGIS_NULL_CHECK_AND_FREE(env);
            
            it = NGIS_LIST_ERASE(char, it);
        }    

        NGIS_LIST_FINALIZE(char, &attr->ngja_environments);
    }

    ngislJobAttributesInitializeMember(attr);

    NGIS_FREE(attr);
    
    return;
}

int
ngisAttrSetInt(
    int *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required)
{
    long lret;
    int result;
    static const char fName[] = "ngisAttrSetInt";
    
    NGIS_ASSERT(ret != NULL);
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    lret = *ret;
    result = ngisAttrSetLong(&lret, opts, name, required);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't set \"%s\" attribute as long.\n", name);
        return 0;
    }
    if ((lret < INT_MIN) || (lret > INT_MAX)) {
        ngisErrorPrint(NULL, fName,
            "Value is too large or too small.\n");
        return 0;
    }
    *ret = lret;
    
    return 1;
}

int
ngisAttrSetLong(
    long *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required) 
{
    ngisOption_t opt;
    char *value = NULL;
    long tmp;
    char *p;
    static const char fName[] ="ngisAttrSetLong";

    NGIS_ASSERT(ret  != NULL);
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    opt = ngisOptionContainerFindFirst(opts, name);
    if (opt != ngisOptionContainerEnd(opts)) {
        value = ngisOptionValue(opt);
        NGIS_ASSERT(value != NULL);
        errno = 0;
        tmp = strtol(value, &p, 0);
        if (errno != 0) { 
            ngisErrorPrint(NULL, fName, "strtol: %s.\n", strerror(errno));
            return 0;
        }
        if (*p != '\0') {
            ngisErrorPrint(NULL, fName,
                "%s includes invalid charactor as number.\n", value);
            return 0;                
        }
        *ret = tmp;
        ngislAttrEraseName(opts, opt, name);
    } else if (required != 0) {
        ngisErrorPrint(NULL, fName,
            "Invoke Server option \"%s\" doesn't exist.\n", name);
        return 0;
    }
    
    return 1;
}

int
ngisAttrSetUshort(
    unsigned short *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required)
{
    unsigned long lret;
    int result;
    static const char fName[] ="ngisAttrSetUshort";
    
    NGIS_ASSERT(ret != NULL);
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    lret = *ret;
    result = ngisAttrSetUlong(&lret, opts, name, required);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't set \"%s\" attribute as unsigned long.\n", name);
        return 0;
    }
    if (lret > USHRT_MAX) {
        ngisErrorPrint(NULL, fName, "Value is too large.\n");
        return 0;
    }
    *ret = lret;
    
    return 1;
}

int
ngisAttrSetUint(
    unsigned int *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required)
{
    unsigned long lret;
    int result;
    static const char fName[] ="ngisAttrSetUint";
    
    NGIS_ASSERT(ret != NULL);
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    lret = *ret;
    result = ngisAttrSetUlong(&lret, opts, name, required);
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't set \"%s\" attribute as unsigned long.\n", name);
        return 0;
    }
    if (lret > UINT_MAX) {
        ngisErrorPrint(NULL, fName, "Value is too large.\n");
        return 0;
    }
    *ret = lret;
    
    return 1;
}

int
ngisAttrSetUlong(
    unsigned long *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required) 
{
    char *value = NULL;
    unsigned long tmp;
    char *p;
    ngisOption_t opt;
    static const char fName[] ="ngisAttrSetUlong";
    
    NGIS_ASSERT(ret  != NULL);
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    opt = ngisOptionContainerFindFirst(opts, name);
    if (opt != ngisOptionContainerEnd(opts)) {
        value = ngisOptionValue(opt);
        NGIS_ASSERT(value != NULL);
        errno = 0;
        tmp = strtoul(value, &p, 0);
        if (errno != 0) { 
            ngisErrorPrint(NULL, fName, "strtoul: %s.\n", strerror(errno));
            return 0;
        }
        if (*p != '\0') {
            ngisErrorPrint(NULL, fName,
                "%s includes invalid charactor as number.\n", value);
            return 0;                
        }
        *ret = tmp;
        ngislAttrEraseName(opts, opt, name);
    } else if (required != 0) {
        ngisErrorPrint(NULL, fName,
            "Invoke Server option \"%s\" doesn't exist.\n", name);
        return 0;
    }
    
    return 1;
}

int
ngisAttrSetEnum(
    int *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required,
    char **items,
    int nItems)
{
    char *value = NULL;
    int i;
    ngisOption_t opt;
    static const char fName[] ="ngisAttrSetEnum";
    
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);
    NGIS_ASSERT(items != NULL);
    NGIS_ASSERT(nItems > 0);

    opt = ngisOptionContainerFindFirst(opts, name);
    if (opt != ngisOptionContainerEnd(opts)) {
        value = ngisOptionValue(opt);
        NGIS_ASSERT(value != NULL);
        for (i = 0;i < nItems;++i) {
            NGIS_ASSERT((items[i] != NULL) && strlen(items[i]) > 0);
            if (strcmp(value, items[i]) == 0) {
                *ret = i;
                break;
            }
        }
        if (i == nItems) {
            ngisErrorPrint(NULL, fName, "\"%s\" is invalid string.\n", value);
            return 0;
        }
        ngislAttrEraseName(opts, opt, name);
    } else if (required != 0) {
        ngisErrorPrint(NULL, fName,
            "Invoke Server option \"%s\" doesn't exist.\n", name);
        return 0;
    }
    return 1;
}

int
ngisAttrSetBool(
    int *ret,
    ngisOptionContainer_t *opts,
     char *name,
     int required)
{
    int eret = 0;
    int result;
    static const char fName[] ="ngisAttrSetBool";

    NGIS_ASSERT(ret  != NULL);    
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);
    
    eret = (int)*ret;
    result = ngisAttrSetEnum(&eret, opts, name, required,
        ngislAttrBoolStrings, NGIS_NELEMENTS(ngislAttrBoolStrings));
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't set \"%s\" attribute as bool.\n", name);
        return 0;
    }
    *ret = (int)eret;

    return 1;
}

int
ngisAttrSetBackend(
    ngisJobBackend_t *ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required)
{
    int eret = 0;
    int result;
    static const char fName[] ="ngisAttrSetBackend";
    
    NGIS_ASSERT(ret  != NULL);        
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(name != NULL);

    eret = (int)*ret;    
    result = ngisAttrSetEnum(
        &eret, opts, name, required,
        ngislAttrBackendStrings,
        NGIS_NELEMENTS(ngislAttrBackendStrings));
    if (result == 0) {
        ngisErrorPrint(NULL, fName,
            "Can't set \"%s\" attribute as Job Backend.\n", name);
        return 0;
    }
    *ret = (ngisJobBackend_t)eret;

    return 1;
}

int
ngisAttrSetString(
    char **ret,
    ngisOptionContainer_t *opts,
    char *name,
    int required)
{
    ngisOption_t opt;
    ngisOption_t last;
    char *value = NULL;
    char *str;
    static const char fName[] ="ngisAttrSetString"; 

    NGIS_ASSERT(ret  != NULL);    
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT_STRING(name);

    last = ngisOptionContainerEnd(opts);
    opt = ngisOptionContainerFindFirst(opts, name);
    if (opt != last) {
        value = ngisOptionValue(opt);
        NGIS_ASSERT(value != NULL);
        str = strdup(value);
        if (str == NULL) {
            ngisErrorPrint(NULL, fName,
                "Can't allocate storage for a string.\n");
            return 0;
        }
        *ret = str;
        ngislAttrEraseName(opts, opt, name);
    } else if (required != 0) {
        ngisErrorPrint(NULL, fName,
            "Invoke Server option \"%s\" doesn't exist.\n", name);
        return 0;
    }
    
    return 1;
}

int
ngisAttrDefaultSetString(
    char **string,
    char *defaltValue)
{
    static const char fName[] = "ngisAttrDefaultSetString";

    NGIS_ASSERT(string != NULL);

    if (*string == NULL) {
        *string = strdup(defaltValue);
        if (*string == NULL) {
            ngisErrorPrint(NULL, fName,
                "Can't set default value.\n");
            return 0;
        }
    }
    return 1;
}

int
ngisAttrSetStringList(
    NGIS_LIST_OF(char) *ret,
    ngisOptionContainer_t *opts,
    char *name)
{
    ngisOption_t it;
    ngisOption_t last;    
    char *value = NULL;
    char *copy = NULL;
    int result;
    static const char fName[] ="ngisAttrSetStringList";

    NGIS_ASSERT(ret != NULL);    
    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT((name != NULL) && (strlen(name) > 0));

    it = ngisOptionContainerBegin(opts);
    last = ngisOptionContainerEnd(opts);
    while ((it = ngisOptionFind(it, last, name)) != last) {
        value = ngisOptionValue(it);
        NGIS_ASSERT(value != NULL);
        
        copy = strdup(value);
        if (copy == NULL) {
            ngisErrorPrint(NULL, fName,
                "Can't allocate storage for a string.\n");
            goto error;
        }
        ngisDebugPrint(NULL, fName, "Insert \"%s\".\n", copy);
        result = NGIS_LIST_INSERT_TAIL(char, ret, copy);
        if (result == 0) {
            ngisErrorPrint(NULL, fName, "Can't insert string to list.\n");
            goto error;
        }
        copy = NULL;
        it = ngisOptionErase(it);
    }
    
    return 1;
error:
    NGIS_NULL_CHECK_AND_FREE(copy);

    return 0;
}

/**
 * Check duplication.
 * And this function erase option whose name is name of 'opt' from container.
 */
static void
ngislAttrEraseName(
    ngisOptionContainer_t *opts,
    ngisOption_t opt,
    char *name)
{
    ngisOption_t last;
    static const char fName[] = "ngislAttrEraseName";

    NGIS_ASSERT(opts != NULL);
    NGIS_ASSERT(opt != NULL);

    last = ngisOptionContainerEnd(opts);
    for (;;) {
        opt = ngisOptionErase(opt);
        opt = ngisOptionFind(opt, last, name);
        if (opt == last) {
            break;
        }
        ngisWarningPrint(NULL, fName,
            "\"%s %s\" is ignored "
            "because this invoke server option is already set.\n",
            ngisOptionName(opt), ngisOptionValue(opt));
    }
    return;
}
