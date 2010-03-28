/*
 * $RCSfile: ngemOption.c,v $ $Revision: 1.13 $ $Date: 2008/03/28 03:52:31 $
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

#include "ngemUtility.h"
#include "ngemProtocol.h"

NGI_RCSID_EMBED("$RCSfile: ngemOption.c,v $ $Revision: 1.13 $ $Date: 2008/03/28 03:52:31 $")

static ngemOptionAction_t *ngemlOptionAnalyzerFind(ngemOptionAnalyzer_t *, char *);

/**
 * Option Analyzer: Create
 */
ngemOptionAnalyzer_t *
ngemOptionAnalyzerCreate()
{
    ngemOptionAnalyzer_t *analyzer = NULL;
    ngLog_t *log;
    ngemResult_t nResult;
    NGEM_FNAME(ngemOptionAnalyzerCreate);

    log = ngemLogGetDefault();

    analyzer = NGI_ALLOCATE(ngemOptionAnalyzer_t, log, NULL);
    if (analyzer == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't allocate storage for Option Analyzer,\n");
        goto error;
    }
    NGEM_LIST_INITIALIZE(ngemOptionAction_t, &analyzer->ngoa_actions);
    analyzer->ngoa_nLines  = 0;
    analyzer->ngoa_curLine = NULL;
    analyzer->ngoa_printed = false;
    analyzer->ngoa_first   = true;
    nResult = ngemStringBufferInitialize(&analyzer->ngoa_messages);
    if (nResult != NGEM_SUCCESS) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't initialize the string buffer,\n");
        goto error;
    }
    analyzer->ngoa_messagesValid = true;
    analyzer->ngoa_error = NGEM_OPTION_ERROR_NO_ERROR;

    return analyzer;
error:

    NGEM_LIST_FINALIZE(ngemOptionAction_t, &analyzer->ngoa_actions);
    NGI_DEALLOCATE(ngemOptionAnalyzer_t, analyzer, log, NULL);

    return NULL;
}

/**
 * Option Analyzer: Destroy
 */
ngemResult_t
ngemOptionAnalyzerDestroy(
    ngemOptionAnalyzer_t *oa)
{
    int i = 0;
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(ngemOptionAction_t) it;
    ngemOptionAction_t *act;
    NGEM_FNAME(ngemOptionAnalyzerDestroy);

    log = ngemLogGetDefault();

    if (oa == NULL) {
        return NGEM_SUCCESS;
    }

    NGEM_LIST_ERASE_EACH (ngemOptionAction_t, &oa->ngoa_actions, it ,act) {
        NGEM_ASSERT(act != NULL);
        NGEM_ASSERT_STRING(act->ngoa_name);
        ngiFree(act->ngoa_name, log, NULL);
        ngiFree(act, log, NULL);
        i++;
    }
    ngLogDebug(log, NGEM_LOGCAT_OPTION, fName, "%d actions is destroyed.\n", i);

    ngemStringBufferFinalize(&oa->ngoa_messages);
    NGEM_LIST_FINALIZE(ngemOptionAction_t, &oa->ngoa_actions);
    NGI_DEALLOCATE(ngemOptionAnalyzer_t, oa, log, NULL);

    return NGEM_SUCCESS;
}

ngemResult_t
ngemOptionAnalyzerReset(
    ngemOptionAnalyzer_t *oa)
{
    int i = 0;
    ngLog_t *log;
    NGEM_LIST_ITERATOR_OF(ngemOptionAction_t) it;
    ngemOptionAction_t *act;
    NGEM_FNAME(ngemOptionAnalyzerReset);

    log = ngemLogGetDefault();

    if (oa == NULL) {
        return NGEM_SUCCESS;
    }

    NGEM_LIST_ERASE_EACH (ngemOptionAction_t, &oa->ngoa_actions, it ,act) {
        NGEM_ASSERT(act != NULL);
        NGEM_ASSERT_STRING(act->ngoa_name);
        ngiFree(act->ngoa_name, log, NULL);
        ngiFree(act, log, NULL);
        i++;
    }
    ngLogDebug(log, NGEM_LOGCAT_OPTION, fName, "%d actions is destroyed.\n", i);

    ngemStringBufferReset(&oa->ngoa_messages);

    return NGEM_SUCCESS;
}

/**
 * Option Analyzer: Set callback function for reading a option.
 */
ngemResult_t
ngemOptionAnalyzerSetAction(
    ngemOptionAnalyzer_t *oa,
    char *name,
    ngemOptionHandler_t handler,
    void *userData,
    int min,
    int max/* negative value is used as unbound.*/)
{
    ngemOptionAction_t *act = NULL;
    ngemOptionAction_t *new = NULL;
    char *nameCopy = NULL;
    ngLog_t *log;
    int result;
    NGEM_FNAME(ngemOptionAnalyzerSetAction);

    NGEM_ASSERT(max != 0);
    NGEM_ASSERT(min >= 0);
    NGEM_ASSERT((max < 0) || (min <= max));

    log = ngemLogGetDefault();
    
    act = ngemlOptionAnalyzerFind(oa, name);
    if (act == NULL) {
        /* New */
        new = NGI_ALLOCATE(ngemOptionAction_t, log, NULL);
        if (new == NULL) {
            ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                "Can't allocate storage for option analyzer.\n");
            goto error;
        }

        nameCopy = ngiStrdup(name, log, NULL);
        if (nameCopy == NULL) {
            ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                "Can't copy string.\n");
            goto error;
        }
        new->ngoa_name = nameCopy;

        result = NGEM_LIST_INSERT_TAIL(
            ngemOptionAction_t, &oa->ngoa_actions, new);
        if (result == 0) {
            ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                "Can't insert new action to list.\n");
            goto error;
        }
        act = new;
    }

    act->ngoa_handler   = handler;
    act->ngoa_user_data = userData;
    act->ngoa_min       = min;
    act->ngoa_max       = max;
    act->ngoa_count     = 0;

    return NGEM_SUCCESS;
error:
    NGI_DEALLOCATE(ngemOptionAction_t, new, log, NULL);

    return NGEM_FAILED;
}

/**
 * Option Analyzer: analyze line
 */
ngemResult_t
ngemOptionAnalyzerAnalyzeLine(
    ngemOptionAnalyzer_t *oa,
    char *line)
{
    size_t name_len;
    ngemResult_t result;
    char *value = NULL;
    char *name = NULL;
    ngemOptionAction_t *act = NULL;
    ngLog_t *log;
    ngemResult_t ret = NGEM_FAILED;
    NGEM_FNAME(ngemOptionAnalyzerAnalyzeLine);

    log = ngemLogGetDefault();

    NGEM_ASSERT(oa != NULL);
    NGEM_ASSERT(line != NULL);

    if (oa->ngoa_error == NGEM_OPTION_ERROR_INTERNAL) {
        return NGEM_SUCCESS;
    }

    oa->ngoa_nLines++;
    oa->ngoa_curLine = line;
    oa->ngoa_printed = false;

    /* Get the name of option */    
    name_len = 0;   
    while ((line[name_len] != '\0') && (!isspace((int)line[name_len]))) {
        name_len++;
    }
    if (name_len == 0) {
        ngemOptionSyntaxError(oa, fName,
            "Option's name is empty.\n");
        goto finalize;
    }
    
    name = ngiStrndup(line, name_len, log, NULL);
    if (name == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't copy string of option's name.\n");
        oa->ngoa_error = NGEM_OPTION_ERROR_INTERNAL;
        goto finalize;
    }
    
    /* Get the value of option */
    if (strlen(&line[name_len]) > 1) {
        value = ngiStrdup(line + name_len + 1, log, NULL);
    } else {
        value = ngiStrdup("", log, NULL);
    }
    if (value == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't copy string of option's value.\n");
        oa->ngoa_error = NGEM_OPTION_ERROR_INTERNAL;
        goto finalize;
    }
    ngemStringStrip(value);

    /* Find Action */
    act = ngemlOptionAnalyzerFind(oa, name);
    if (act == NULL) {
        ngemOptionSyntaxError(oa, fName,
            "\"%s\" is unknown option.\n", name);
        goto finalize;
    }

    /* Check time of appearance */
    act->ngoa_count++;
    if ((act->ngoa_max > 0) && (act->ngoa_count > act->ngoa_max)) {
        /* Prints error message once. */
        if (act->ngoa_count == act->ngoa_max+1) {
            ngemOptionSyntaxError(oa, fName,
                "Too many \"%s\" option.\n", name);
        }
        oa->ngoa_error = NGEM_OPTION_ERROR_SYNTAX;

        goto finalize;
    }

    /* Callback */
    result = act->ngoa_handler(oa, act->ngoa_user_data, name, value);
    if (result == NGEM_FAILED) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName, "Callback function failed.\n");
        goto finalize;
    }

    ret = NGEM_SUCCESS;
finalize:
    if (oa->ngoa_error == NGEM_OPTION_ERROR_SYNTAX) {
        /* Not error if syntax error occurs */
        ret = NGEM_SUCCESS;
    }

    oa->ngoa_curLine = NULL;
    oa->ngoa_printed = false;

    ngiFree(name, log, NULL);
    ngiFree(value, log, NULL);
    
    return ret;
}

/**
 * Option Analyzer: Informs "Option Analyzer" that there are not more options.
 */
ngemResult_t
ngemOptionAnalyzerAnalyzeEnd(
    ngemOptionAnalyzer_t *oa, 
    char **errorMessage)
{
    NGEM_LIST_ITERATOR_OF(ngemOptionAction_t) it;
    ngemOptionAction_t *act;
    ngLog_t *log = NULL;
    ngemResult_t ret = NGEM_SUCCESS;
    NGEM_FNAME(ngemOptionAnalyzerAnalyzeEnd);
    
    log = ngemLogGetDefault();

    oa->ngoa_printed = false;

    if (oa->ngoa_error == NGEM_OPTION_ERROR_INTERNAL) {
        return NGEM_SUCCESS;
    }

    /* Check count of options */
    NGEM_LIST_FOREACH (ngemOptionAction_t, &oa->ngoa_actions, it, act) {
        if (act->ngoa_count < act->ngoa_min) {
            NGEM_ASSERT((act->ngoa_max < 0) || (act->ngoa_count <= act->ngoa_max));
            ngemOptionSyntaxError(oa, fName,
                "Requires %d %s options, but there are %d %s options.\n",
                act->ngoa_min, act->ngoa_name, act->ngoa_count, act->ngoa_name);
        }
    }

    if (oa->ngoa_error == NGEM_OPTION_ERROR_SYNTAX) {
        if (errorMessage != NULL) {
            *errorMessage = NULL;
            if (oa->ngoa_messagesValid) {
                *errorMessage = ngemStringBufferRelease(&oa->ngoa_messages);
                oa->ngoa_messagesValid = false;
            } else {
                *errorMessage = ngiStrdup("Unkown error", log, NULL);
                if (*errorMessage == NULL) {
                    ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                        "Can't copy string.\n");
                    ret = NGEM_FAILED;
                }
            }
        }
    }

    return ret;
}

/**
 * Option Analyzer: Callback function for long type option.
 */
ngemResult_t
ngemOptionAnalyzerSetLong(
    ngemOptionAnalyzer_t *oa,
    long *userData,
    char *name,
    char *value)
{
    long tmp;
    char *p;
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetLong);

    NGEM_ASSERT(userData != NULL);
    NGEM_ASSERT(name     != NULL);
    NGEM_ASSERT(value    != NULL);

    log= ngemLogGetDefault();

    errno = 0;
    tmp = strtol(value, &p, 0);
    if (errno != 0) { 
        ngemOptionSyntaxError(oa, fName,
            "strtol: %s.\n", strerror(errno));
        return NGEM_FAILED;
    }
    if (*p != '\0') {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "\"%s %s\" includes invalid character as number.\n", name, value);
        return NGEM_FAILED;
    }
    *userData = tmp;
    
    return NGEM_SUCCESS;
}

#define NGEML_DEFINE_OPTION_ANALYZER_SET_INT(name, type, min, max) \
ngemResult_t                                                       \
ngemOptionAnalyzerSet##name (                                      \
    ngemOptionAnalyzer_t *oa,                                      \
    type *userData,                                                \
    char *name,                                                    \
    char *value)                                                   \
{                                                                  \
    long tmp;                                                      \
    ngemResult_t result;                                           \
    ngLog_t *log;                                                  \
    NGEM_FNAME(name);                                              \
                                                                   \
    log= ngemLogGetDefault();                                      \
                                                                   \
    result = ngemOptionAnalyzerSetLong(oa, &tmp, name, value);     \
    if (result != NGEM_SUCCESS) {                                  \
        return NGEM_FAILED;                                        \
    }                                                              \
                                                                   \
    if (tmp < (min)) {                                             \
        ngemOptionSyntaxError(oa, fName,                           \
            "%ld is too small.\n", tmp);                           \
        return NGEM_FAILED;                                        \
    }                                                              \
    if ((max) < tmp) {                                             \
        ngemOptionSyntaxError(oa, fName,                           \
            "%ld is too large.\n", tmp);                           \
        return NGEM_FAILED;                                        \
    }                                                              \
    *userData = tmp;                                               \
                                                                   \
    return NGEM_SUCCESS;                                           \
}

/**
 * Option Analyzer: Callback function for integer types option.
 */
NGEML_DEFINE_OPTION_ANALYZER_SET_INT(Int,    int,      INT_MIN, INT_MAX)
NGEML_DEFINE_OPTION_ANALYZER_SET_INT(Short,  short,   SHRT_MIN, SHRT_MAX)
NGEML_DEFINE_OPTION_ANALYZER_SET_INT(Ulong,  unsigned long,  0, ULONG_MAX)
NGEML_DEFINE_OPTION_ANALYZER_SET_INT(Uint,   unsigned int,   0, UINT_MAX)
NGEML_DEFINE_OPTION_ANALYZER_SET_INT(Ushort, unsigned short, 0, USHRT_MAX)

/**
 * Option Analyzer: Callback function for string type option.
 */
ngemResult_t
ngemOptionAnalyzerSetString(
    ngemOptionAnalyzer_t *oa,
    char **userData,
    char *name,
    char *value)
{
    char *tmp = NULL;
    ngLog_t *log = NULL;
    NGEM_FNAME(ngemOptionAnalyzerSetString);

    NGEM_ASSERT(userData != NULL);
    NGEM_ASSERT(name     != NULL);
    NGEM_ASSERT(value    != NULL);

    log = ngemLogGetDefault();

    if (strlen(value) == 0) {
        ngemOptionSyntaxError(oa, fName,
            "Option value is empty.\n");
        return NGEM_FAILED;
    }

    tmp = ngiStrdup(value, log, NULL);
    if (tmp == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't copy string.\n");
        return NGEM_FAILED;
    }
    *userData = tmp;
    
    return NGEM_SUCCESS;
}

/**
 * Option Analyzer: Callback function for string list type option.
 */
ngemResult_t
ngemOptionAnalyzerSetStringList(
    ngemOptionAnalyzer_t *oa,
    NGEM_LIST_OF(char) *userData,
    char *name,
    char *value)
{
    char *copy = NULL;
    int result;
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetStringList);

    NGEM_ASSERT((name != NULL) && (strlen(name) > 0));
    NGEM_ASSERT(value != NULL);
    NGEM_ASSERT(userData != NULL);

    log = ngemLogGetDefault();

    if (strlen(value) == 0) {
        ngemOptionSyntaxError(oa, fName,
            "Option value is empty.\n");
        goto error;
    }

    copy = strdup(value);
    if (copy == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "Can't allocate storage for a string.\n");
        return NGEM_FAILED;
    }
        
    ngLogDebug(log, NGEM_LOGCAT_OPTION, fName, "Insert \"%s\".\n", copy);

    result = NGEM_LIST_INSERT_TAIL(char, userData, copy);
    if (result == 0) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName, "Can't insert string to list.\n");
        goto error;
    }
    copy = NULL;
    
    return NGEM_SUCCESS;
error:
    ngiFree(copy, log, NULL);

    return NGEM_FAILED;
}

/**
 * Option Analyzer: Callback function for bool type option.
 */
ngemResult_t
ngemOptionAnalyzerSetBool(
    ngemOptionAnalyzer_t *analyzer,
    bool *userData,
    char *name,
    char *value)
{
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetBool);

    NGEM_ASSERT((name != NULL) && (strlen(name) > 0));
    NGEM_ASSERT(value != NULL);
    NGEM_ASSERT(userData != NULL);

    log = ngemLogGetDefault();

    if (strcmp(value, "true") == 0) {
        *userData = true;
    } else if (strcmp(value, "false") == 0) {
        *userData = false;
    } else {
        ngemOptionSyntaxError(analyzer, fName,
            "%s: \"%s\" is invalid as bool .\n", name, value);
        return NGEM_FAILED;
    }
    
    return NGEM_SUCCESS;
}

ngemResult_t
ngemOptionAnalyzerSetIbool(
    ngemOptionAnalyzer_t *analyzer,
    int *userData,
    char *name,
    char *value)
{
    ngemResult_t ret;
    bool bval;

    NGEM_ASSERT((name != NULL) && (strlen(name) > 0));
    NGEM_ASSERT(value != NULL);
    NGEM_ASSERT(userData != NULL);

    ret = ngemOptionAnalyzerSetBool(analyzer, &bval, name, value);
    if (ret == NGEM_SUCCESS) {
        *userData = bval?1:0;
    }

    return ret;
}

ngemResult_t
ngemOptionAnalyzerSetDouble(
    ngemOptionAnalyzer_t *analyzer,
    double *userData,
    char *name,
    char *value)
{
    double tmp;
    char *p;
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetDouble);

    NGEM_ASSERT(userData != NULL);
    NGEM_ASSERT(name     != NULL);
    NGEM_ASSERT(value    != NULL);

    log= ngemLogGetDefault();

    errno = 0;
    tmp = strtod(value, &p);
    if (errno != 0) { 
        ngemOptionSyntaxError(analyzer, fName,
            "strtod: %s.\n", strerror(errno));
        return NGEM_FAILED;
    }
    if (*p != '\0') {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "\"%s %s\" includes invalid character as number.\n", name, value);
        return NGEM_FAILED;
    }
    *userData = tmp;
    
    return NGEM_SUCCESS;
}

ngemResult_t
ngemOptionAnalyzerSetResult(
    ngemOptionAnalyzer_t *analyzer,
    ngemResult_t *userData,
    char *name,
    char *value)
{
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetResult);

    NGEM_ASSERT((name != NULL) && (strlen(name) > 0));
    NGEM_ASSERT(value != NULL);
    NGEM_ASSERT(userData != NULL);

    log = ngemLogGetDefault();

    if (strcmp(value, "S") == 0) {
        *userData = NGEM_SUCCESS;
    } else if (strcmp(value, "F") == 0) {
        *userData = NGEM_FAILED;
    } else {
        ngemOptionSyntaxError(analyzer, fName,
            "%s: \"%s\" is invalid as result.\n", name, value);
        return NGEM_FAILED;
    }
    
    return NGEM_SUCCESS;
}

ngemResult_t
ngemOptionAnalyzerSetIgnore(
    ngemOptionAnalyzer_t *analyzer,
    void *userData,
    char *name,
    char *value)
{
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerSetIgnore);

    NGEM_ASSERT((name != NULL) && (strlen(name) > 0));
    NGEM_ASSERT(value != NULL);

    log = ngemLogGetDefault();

    ngLogInfo(log, NGEM_LOGCAT_OPTION, fName,
        "%s: This option is ignored\n", name);
    
    return NGEM_SUCCESS;
}


/**
 * Option Analyzer: Find action(callback functions) information using option's name as key.
 */
static ngemOptionAction_t *
ngemlOptionAnalyzerFind(
    ngemOptionAnalyzer_t *oa,
    char *name)
{
    NGEM_LIST_ITERATOR_OF(ngemOptionAction_t) it;
    ngemOptionAction_t *act;
    NGEM_FNAME_TAG(ngemlOptionAnalyzerFind);

    NGEM_ASSERT(oa != NULL);
    NGEM_ASSERT(name != NULL);

    /* Find Action */
    NGEM_LIST_FOREACH (ngemOptionAction_t, &oa->ngoa_actions, it, act) {
        if (strcmp(act->ngoa_name, name) == 0) {
            return act;
        }
    }
    return NULL;
}

/**
 * Option Analyzer: Syntax Error.
 */
void
ngemOptionSyntaxError(
    ngemOptionAnalyzer_t *oa,
    const char *func_name,
    const char *format,
    ...)
{
    ngLog_t *log;
    va_list ap;
    char *message = NULL;
    ngemResult_t nResult;
    NGEM_FNAME(ngemOptionSyntaxError);

    log = ngemLogGetDefault();

    NGEM_ASSERT(oa != NULL);

    if (oa->ngoa_error == NGEM_OPTION_ERROR_INTERNAL) {
        goto error;
    }

    oa->ngoa_error = NGEM_OPTION_ERROR_SYNTAX;

    if (!oa->ngoa_messagesValid) {
        goto error;
    }

    if (oa->ngoa_messagesValid) {
        if (oa->ngoa_first) {
            NGEM_ASSERT(oa->ngoa_printed == false);
            oa->ngoa_first = false;
            nResult = ngemStringBufferFormat(&oa->ngoa_messages,
                "Syntax Error:\n");
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                    "Can't append to string buffer..\n");
                oa->ngoa_error = NGEM_OPTION_ERROR_INTERNAL;
                goto error;
            }
        }

        if (oa->ngoa_printed == false) {
            if (oa->ngoa_curLine == NULL) {
                nResult = ngemStringBufferFormat(&oa->ngoa_messages,
                    "\nEnd of options: \n");
            } else {
                nResult = ngemStringBufferFormat(&oa->ngoa_messages,
                    "\nline %d: %s:\n", oa->ngoa_nLines, oa->ngoa_curLine);
            }
            oa->ngoa_printed = true;
            if (nResult != NGEM_SUCCESS) {
                ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                    "Can't append to string buffer..\n");
                oa->ngoa_error = NGEM_OPTION_ERROR_INTERNAL;
                goto error;
            }
        }
        va_start(ap, format);
        nResult = 
            ngemStringBufferAppend(&oa->ngoa_messages, "\t");
            ngemStringBufferVformat(&oa->ngoa_messages, format, ap);
        va_end(ap);
        if (nResult != NGEM_SUCCESS) {
            ngLogError(log, NGEM_LOGCAT_OPTION, fName,
                "Can't append to string buffer..\n");
            oa->ngoa_error = NGEM_OPTION_ERROR_INTERNAL;
            goto error;
        }
    } else {
        goto error;
    }

    return;
error:
    if (oa->ngoa_curLine == NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, func_name,
            "End : Error occurs\n");
    } else {
        ngLogError(log, NGEM_LOGCAT_OPTION, func_name,
            "%4d: %s: Error occurs\n", oa->ngoa_nLines, oa->ngoa_curLine);
    }
    va_start(ap, format);
    ngLogVprintf(log, NGEM_LOGCAT_OPTION, NG_LOG_LEVEL_ERROR, NULL, func_name,
        format, ap);
    va_end(ap);

    ngiFree(message, log, NULL);

    return;
}

ngemResult_t
ngemOptionAnalyzerAnalyzeList(
    ngemOptionAnalyzer_t *oa,
    NGEM_LIST_OF(char) *list)
{
    NGEM_LIST_ITERATOR_OF(char) it;
    char *val;
    char *message = NULL;
    char *tmpMessage = NULL;
    ngemResult_t ret = NGEM_SUCCESS; 
    ngemResult_t nResult;
    ngLog_t *log;
    NGEM_FNAME(ngemOptionAnalyzerAnalyzeList);

    log = ngemLogGetDefault();

    NGEM_LIST_FOREACH(char, list, it, val) {
        nResult = ngemOptionAnalyzerAnalyzeLine(oa, val);
        if (nResult != NGEM_SUCCESS) {
            message = "Can't analyze the list.";
            /* Throw */
        }
    }
    nResult = ngemOptionAnalyzerAnalyzeEnd(oa, &tmpMessage);
    if (nResult != NGEM_SUCCESS) {
        message = "Can't analyze the list.";
        /* Throw */
    } else {
        if (tmpMessage != NULL) {
            message = tmpMessage;
        }
    }

    if (message != NULL) {
        ngLogError(log, NGEM_LOGCAT_OPTION, fName,
            "%s\n", message);
        ret = NGEM_FAILED;
    }

    ngiFree(tmpMessage, log, NULL);

    return ret;
}
