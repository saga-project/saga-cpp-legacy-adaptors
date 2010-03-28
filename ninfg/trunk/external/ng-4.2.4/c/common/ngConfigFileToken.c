#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngConfigFileToken.c,v $ $Revision: 1.37 $ $Date: 2006/02/15 02:52:52 $";
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
 * Module of Configuration File Token spliting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "ngConfigFile.h"

#define NGL_CHAR_COMMENT         '#'  /* comment until end of line */
#define NGL_CHAR_TAGSTRING_START '<'
#define NGL_CHAR_TAGSTRING_END   '>'
#define NGL_CHAR_SECTION_END     '/'

/**
 * Conversion table for time, size
 */
ngiUnitConvTable_t ngiTimeUnitTable[] = {
    {"second",            1},
    {"minute",           60},
    {"hour",        60 * 60},
    {"day",    24 * 60 * 60},
    {NULL, 0}
};

ngiUnitConvTable_t ngiSizeUnitTable[] = {
    {"Kilo",  1024},
    {"Mega",  1024 * 1024},
    {"Giga",  1024 * 1024 * 1024},
    {NULL, 0}
};

/**
 * internal function prototypes
 */

static char *nglFindTokenBeginInStr(char *line);
static int nglCopyQuotedString(
    char *dst, char *src, size_t len,
    int allowContinue, int *valueContinueToNextLine);
static int nglConvertUnit(char *unitStr,
    ngiUnitConvTable_t *table, int caseSensitive);
static int nglStringListCountChars(ngiStringList_t *strings);


/**
 * functions
 */

/**
 * Open config file and ready to GetToken (initialize TokenReadInfo)
 */
int
ngiConfigFileOpen(
    char *filename,
    ngiTokenReadInfo_t **tokenReadInfo,
    int requireAvailable,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConfigFileOpen";
    ngiTokenReadInfo_t *TokenRead;
    FILE *fp;

    if (filename == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL, "%s: Invalid argument.\n", fName);
        return 0;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        /* This will always happen */
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_NOT_FOUND);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            ((requireAvailable != 0) ?
            NG_LOG_LEVEL_ERROR : NG_LOG_LEVEL_INFORMATION), NULL,
            "%s: Opening file \"%s\" failed: %s.\n",
            fName, filename, strerror(errno));
        return 0;
    }

    TokenRead = (ngiTokenReadInfo_t *)
        globus_libc_malloc(sizeof(ngiTokenReadInfo_t));
    if (TokenRead == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage parse configuration file.\n",
             fName);
        return 0;
    }

    TokenRead->ntri_line = (char *)
        globus_libc_malloc(sizeof(char) * NGI_CONFIG_LINE_MAX);
    if (TokenRead->ntri_line == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage parse configuration file.\n",
             fName);
        return 0;
    }
    TokenRead->ntri_token = (char *)
        globus_libc_malloc(sizeof(char) * NGI_CONFIG_LINE_MAX);
    if (TokenRead->ntri_token == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't allocate the storage parse configuration file.\n",
             fName);
        return 0;
    }
    TokenRead->ntri_currentRead = NULL; /* not read yet in *fp */
    TokenRead->ntri_readTokensInline = 0;

    TokenRead->ntri_fp = fp;
    if (TokenRead->ntri_fp == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_NOT_FOUND);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Opened file pointer was invalid.\n",
             fName);
        return 0;
    }

    TokenRead->ntri_lineno = 0;
    TokenRead->ntri_filename = strdup(filename);
    if (TokenRead->ntri_filename == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_MEMORY);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL, 
            "%s: Can't duplicate the string.\n",
             fName);
        return 0;
    }

    *tokenReadInfo = TokenRead;
    return 1;
}

/**
 * Close config file and free TokenReadInfo
 */
int
ngiConfigFileClose(
    ngiTokenReadInfo_t *tokenReadInfo,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiConfigFileClose";

    assert(tokenReadInfo != NULL);

    if(fclose(tokenReadInfo->ntri_fp) != 0) {
        NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_NOT_FOUND);
        ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
            NG_LOG_LEVEL_ERROR, NULL,
            "%s: Closing configuration file failed.\n", fName);
        return 0;
    }

    assert(tokenReadInfo->ntri_line != NULL);
    globus_libc_free(tokenReadInfo->ntri_line);

    assert(tokenReadInfo->ntri_token != NULL);
    globus_libc_free(tokenReadInfo->ntri_token);

    assert(tokenReadInfo->ntri_filename != NULL);
    globus_libc_free(tokenReadInfo->ntri_filename);

    globus_libc_free(tokenReadInfo);

    return 1;
}


/**
 * Getting Token from TokenReadInfo.
 */
int
ngiGetToken(
    ngiTokenReadInfo_t *tokenReadInfo,
    ngiTokenInfo_t *token,
    ngiGetTokenType_t tokenType,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngiGetToken";
    int tokenExist;
    char *returnCode, *p, *to;

    assert(tokenReadInfo != NULL);
    assert(token != NULL);

    token->nti_readInfo = tokenReadInfo;

    if (tokenReadInfo->ntri_currentRead != NULL) {
        tokenReadInfo->ntri_currentRead =
            nglFindTokenBeginInStr(tokenReadInfo->ntri_currentRead);
        /* This will not happen on NGI_TOKEN_QUOTED */
    }

    /* skip until token appear */
    if (tokenReadInfo->ntri_currentRead == NULL) {
        if (tokenType == NGI_GETTOKEN_ARGS) {
            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Attribute argument must not appear in top of the line.\n",
                fName);
            return NGI_GET_TOKEN_INVALID;
        }

        tokenExist = 0;
        while (!tokenExist) {
            returnCode = fgets(tokenReadInfo->ntri_line,
                                  NGI_CONFIG_LINE_MAX,
                                    tokenReadInfo->ntri_fp);
            if (returnCode == NULL) {
                tokenReadInfo->ntri_currentRead = NULL;
                token->nti_type = NGI_TOKEN_NO_TOKEN_ANYMORE;
                token->nti_tokennoInline = 0;
                token->nti_tokenStr = NULL;
                return NGI_GET_TOKEN_EOF;
            }
            tokenReadInfo->ntri_lineno++;
            tokenReadInfo->ntri_readTokensInline = 0;

            if (tokenType == NGI_GETTOKEN_QUOTED) {
                tokenReadInfo->ntri_currentRead = tokenReadInfo->ntri_line;
                tokenExist = 1;
                break;
            }

            returnCode = nglFindTokenBeginInStr(
                tokenReadInfo->ntri_line);
            if (returnCode != NULL) {
                tokenReadInfo->ntri_currentRead = returnCode;
                tokenExist = 1;
            }
        }
    }

    tokenReadInfo->ntri_readTokensInline++;

    if ((*tokenReadInfo->ntri_currentRead == NGL_CHAR_TAGSTRING_START)
        && !((tokenType == NGI_GETTOKEN_ARGS)
        || (tokenType == NGI_GETTOKEN_QUOTED))) {

        token->nti_type = NGI_TOKEN_TAG;
        token->nti_tokennoInline = tokenReadInfo->ntri_readTokensInline;
        token->nti_tokenStr = tokenReadInfo->ntri_token;

        p = tokenReadInfo->ntri_currentRead;
        to = tokenReadInfo->ntri_token;

        /* string copy until section tag finish (and search section tag end)*/
        while ((*p != NGL_CHAR_TAGSTRING_END) && (*p != ' ') && (*p != '\t') &&
                     (*p != NGL_CHAR_COMMENT) &&
                     (*p != '\n') && (*p != '\r') && (*p != '\0')) {
            *to = *p;
            p++;
            to++;
        }

        if (*p == NGL_CHAR_TAGSTRING_END) {
            *to = *p;
            *(to+1) = '\0';
        } else {
            *to = '\0';

            if ((*p == NGL_CHAR_COMMENT) ||
                (*p == '\n') || (*p == '\r') || (*p == '\0')) {
                tokenReadInfo->ntri_currentRead = NULL;
            } else {
                tokenReadInfo->ntri_currentRead = p + 1;
            }

            NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);
            ngLogPrintf(log, NG_LOG_CATEGORY_NINFG_PURE,
                NG_LOG_LEVEL_ERROR, NULL,
                "%s: Tag name invalid.\n", fName);
            return NGI_GET_TOKEN_INVALID;
        }
        tokenReadInfo->ntri_currentRead = p + 1;

    } else {
        token->nti_type = NGI_TOKEN_ATTR;
        token->nti_tokennoInline = tokenReadInfo->ntri_readTokensInline;
        token->nti_tokenStr = tokenReadInfo->ntri_token;

        p = tokenReadInfo->ntri_currentRead;
        to = tokenReadInfo->ntri_token;

        if ((tokenType != NGI_GETTOKEN_ARGS) &&
            (tokenType != NGI_GETTOKEN_QUOTED)) {
            /* string copy until value ends (space comes) */
            while ((*p != ' ') && (*p != '\t') &&
                       (*p != NGL_CHAR_COMMENT) &&
                       (*p != '\n') && (*p != '\r') && (*p != '\0')) {
                *to = *p;
                p++;
                to++;
            }
        } else {  /* tokenType == GETTOKEN_ARGS */
            /* string copy until value ends */
            while ((*p != '\n') && (*p != '\r') && (*p != '\0')) {
                /* Note: comment is included in the token */
                *to = *p;
                p++;
                to++;
            }
        }
        *to = '\0';

        if ((*p == NGL_CHAR_COMMENT) ||
            (*p == '\n') || (*p == '\r') || (*p == '\0')) {
            tokenReadInfo->ntri_currentRead = NULL;
        } else {
            tokenReadInfo->ntri_currentRead = p + 1;
        }
    }

    return NGI_GET_TOKEN_SUCCESS;
}

/**
 * Search beginning of token, or return NULL if not found
 */
static char *
nglFindTokenBeginInStr(char *line)
{
    char *p;

    assert(line != NULL);

    p = line;
    while((*p == ' ') || (*p == '\t')) {
        p++;
    }

    if ((*p == NGL_CHAR_COMMENT) ||
        (*p == '\n') || (*p == '\r') || (*p == '\0')) {
       return NULL; /* no token anymore in this line */
    }

    return p;
}

/**
 * pop tagname SOME from tag <SOME>,
 *   if tag is beginning, <SOME> return NG_GET_TAGNAME_BEGIN
 *    and tag is outgoing </SOME> return NG_GET_TAGNAME_END
 */
int
ngiGetTagName(char *tag, char *tagname)
{
    int isBeginning;
    char *from, *to;

    assert(tag != NULL);
    assert(tagname != NULL);

    from = tag;
    to = tagname;

    if (*from != NGL_CHAR_TAGSTRING_START) {
        *to = '\0';
        return NGI_GET_TAGNAME_FAIL;
    }
    from++;

    if (*from == NGL_CHAR_SECTION_END) {
        from++;
        isBeginning = 0;
    } else {
        isBeginning = 1;
    }

    while ((*from != NGL_CHAR_TAGSTRING_END) && (*from != '\0')) {
        *to = *from;
        from++;
        to++;
    }
    *to = '\0';
    if (*from != NGL_CHAR_TAGSTRING_END) {
        return NGI_GET_TAGNAME_FAIL;
    }

    return (isBeginning ? NGI_GET_TAGNAME_BEGIN : NGI_GET_TAGNAME_END);
}


/* utility functions */

/**
 * This function converts arg to integer.
 * converted result is stored to num.
 */
int 
ngiReadIntFromArg(char *arg, int *num)
{
    char *end, *result;

    assert(arg != NULL);
    assert(num != NULL);

    *num = (int)strtol(arg, &end, 10);
    
    /* Is there any token remain? */
    result = nglFindTokenBeginInStr(end);
    if (result != NULL) {
        return 0;
    }

    return 1;
}

/**
 * This function converts arg to double.
 * converted result is stored to num.
 */
int 
ngiReadDoubleFromArg(char *arg, double *num)
{
    char *end, *result;

    assert(arg != NULL);
    assert(num != NULL);

    *num = (double)strtod(arg, &end);
    
    /* Is there any token remain? */
    result = nglFindTokenBeginInStr(end);
    if (result != NULL) {
        return 0;
    }

    return 1;
}

/**
 * This function mallocs new string buffer, and stores non-space part
 * of the string
 * Note: This function read from not space until space.
 *  Thus, It unable to treat string including space.
 *    like filename which include spaces.
 */
char *
ngiReadStringFromArg(char *arg)
{
    char *newbuf, *p, *end, *result;
    assert(arg != NULL);

    newbuf = strdup(arg);
    if (newbuf == NULL) {
        return NULL;
    }

    p = newbuf;
    while ((*p != ' ') && (*p != '\t') &&
           (*p != NGL_CHAR_COMMENT) &&
           (*p != '\n') && (*p != '\r') && (*p != '\0')) {
        p++;
    }

    /* Is there any token remain? */
    end = p;
    result = nglFindTokenBeginInStr(end);
    if (result != NULL) {
        globus_libc_free(newbuf);
        return NULL;
    }

    *p = '\0';

    return newbuf;
}

/**
 * This function reads the quoted string.
 * like :
 *  foo
 *  "foo bar"
 *  including space in its string.
 */
char *
ngiReadQuotedStringFromArg(
    char *arg,
    int allowContinue,
    int *valueContinueToNextLine,
    int continuedValue)
{
    char *newbuf, *src;
    int result;

    assert(arg != NULL);
    assert((allowContinue == 0) ||
           ((allowContinue != 0) && (valueContinueToNextLine != NULL)));

    if (allowContinue != 0) {
        *valueContinueToNextLine = 0;
    }

    /* Not Quoted? */
    if ((continuedValue == 0) && (arg[0] != '"')) {
        return ngiReadStringFromArg(arg);
    }

    /* Quoted */
    src = arg;

    /* Skip first double quote */
    if ((continuedValue == 0) && (*src == '"')) {
        src++;
    }

    newbuf = strdup(src);
    if (newbuf == NULL) {
        return NULL;
    }

    result = nglCopyQuotedString(
        newbuf, src, strlen(newbuf) + 1,
        allowContinue, valueContinueToNextLine);
    if (result == 0) {
        globus_libc_free(newbuf);
        return NULL;
    }

    return newbuf;
}

/**
 * Copy the Quoted String.
 * \" and \\ escapes are parsed.
 */
static int
nglCopyQuotedString(
    char *dst,
    char *src,
    size_t len,
    int allowContinue,
    int *valueContinueToNextLine)
{
    char *resultStr;
    size_t currSize;
    int quoting;

    assert(dst != NULL);
    assert(src != NULL);
    assert(len >= 1);
    assert((allowContinue == 0) ||
           ((allowContinue != 0) && (valueContinueToNextLine != NULL)));

    *dst = '\0';
    currSize = 1;

    if (allowContinue != 0) {
        *valueContinueToNextLine = 0;
    }

    quoting = 0;

    while (1) {
        if (currSize > len) {
            return 0;
        }

        if (quoting != 0) {
            quoting = 0;

            switch (*src) {
            case '"':
                *dst = '"';
                break;

            case '\\':
                *dst = '\\';
                break;

            case '\0':
            case '\n':
            case '\r':
                if (allowContinue != 0) {
                    /* Continue to next line, return */
                    *dst = '\0';
                    *valueContinueToNextLine = 1;
                    return 1;
                }
                return 0;

            default :
                return 0;
            }
            dst++;
            currSize++;

        } else {
            if ((*src == '\0') || (*src == '\n') || (*src == '\r')) {
                break;
            }

            if (*src == '"') {
                break;
            }

            if (*src == '\\') {
                quoting = 1;
            } else {
                quoting = 0;
            }

            if (quoting == 0) {
                *dst = *src;
                dst++;
                currSize++;
            }
        }

        src++;
    }

    if (*src != '"') {
        return 0;
    }

    *dst = '\0';
    src++;
    dst++;

    /* Is there any token remain? */
    resultStr = nglFindTokenBeginInStr(src);
    if (resultStr != NULL) {
        return 0;
    }

    return 1;
}

/**
 * This function is used to determine the string.
 * if 1st argument str is equal to any following argument,
 * then return number of that string number(in argument).
 *
 * ngiReadEnumFromArg("false", 2, "true", "false")
 *  returns 2, (because 4th argument was matched.)
 *
 * The number of arguments are specified by num.
 * return 0 if failed to read.
 * NOTE: arg should result of ngiReadStringFromArg
 */
int 
ngiReadEnumFromArg(char *arg, int caseSensitive, int num, ...)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    va_list args;
    char *candidate;
    int i, len;

    assert(arg != NULL);

    cmp = (caseSensitive ? strncmp : strncasecmp);
    assert(cmp != NULL);

    len = strlen(arg);

    va_start(args, num);
    for (i = 1; i <= num; i++) {
        candidate = (char *)va_arg(args, char *);
        if ((*cmp)(arg, candidate, len + 1) == 0) {
            va_end(args);
            return i;
        }
    }
    va_end(args);

    return 0;
}

/**
 * This function is used for to read number with unit.
 *  like 1sec, 1min, 1hour, 1kilo, 1mega
 * NOTE: arg should result of ngiReadStringFromArg
 */
int 
ngiReadUnitNumFromArg(
    char *arg,
    int *num,
    ngiUnitConvTable_t *table,
    int caseSensitive)
{
    char *end;
    int amount, unit;

    assert(arg != NULL);
    assert(num != NULL);
    assert(table != NULL);

    amount = (int)strtol(arg, &end, 10);

    if (*end != '\0') {
        unit = nglConvertUnit(end, table, caseSensitive);
        if (unit <= 0) {
            return 0;
        }
    } else {
        unit = 1;
    }

    *num = amount * unit;

    return 1;
}

/**
 * This function converts unit string to unit second.
 * if conversion fail, return 0.
 */
static int
nglConvertUnit(
    char *unitStr,
    ngiUnitConvTable_t *table,
    int caseSensitive)
{
    ngiUnitConvTable_t *cur;
    int (*cmp)(const char *s1, const char *s2, size_t n);

    cmp = (caseSensitive ? strncmp : strncasecmp);
    assert(cmp != NULL);

    cur = table;

    while (cur->nuct_unitStr != NULL) {
        if ((*cmp)(unitStr, cur->nuct_unitStr, strlen(unitStr)) == 0) {

            /* found */
            return cur->nuct_unitAmount;
        }
        cur++;
    }

    /* Not found */
    return 0;
}

/**
 * ngiReadStringListFromArg reads strings from arg,
 * mallocs for StringList.
 */
ngiStringList_t *
ngiReadStringListFromArg(char *arg)
{
    char *newbuf; /* for temporary use */
    char *p, *stringStart;
    int lastString, rc;
    ngiStringList_t *newStringList, *stringHead;

    assert(arg != NULL);

    newbuf = strdup(arg);
    if (newbuf == NULL) {
        return NULL;
    }

    p = newbuf;
    stringHead = NULL;

    while(1) {
        p = nglFindTokenBeginInStr(p);
        if (p == NULL) {
            break;
        }
        stringStart = p;

        /* host name string is fixing */
        lastString = 0; /* FALSE */
        while ((*p != ' ') && (*p != '\t') &&
               (*p != NGL_CHAR_COMMENT) &&
               (*p != '\n') && (*p != '\r') && (*p != '\0')) {
            p++;
        }
        if ((*p == NGL_CHAR_COMMENT) ||
            (*p == '\n') || (*p == '\r') || (*p == '\0')) {
            lastString = 1;
            *p = '\0';
        } else {
            *p = '\0';
            p++;
        }

        /* create string */
        newStringList = ngiStringListConstruct(stringStart);
        if (newStringList == NULL) {
            return NULL;
        }

        rc = ngiStringListRegisterList(&stringHead, newStringList);
        if (rc != 1) {
            return NULL;
        }

        if (lastString) {
            break;
        }
    }
    globus_libc_free(newbuf);

    return stringHead;
}

/**
 * this function is for to read environment.
 *  like "ENV=env", "ENV".
 *  but, "ENV = env" will possively appear for user configuration,
 *  so, We should detect them.
 */
char *
ngiReadEnvStringFromArg(char *arg)
{
    char *newbuf, *p, *dst, *end, *resultStr, *equalPoint;
    size_t argLen;
    int result;

    assert(arg != NULL);

    argLen = strlen(arg);

    newbuf = (char *)globus_libc_malloc(sizeof(char) * (argLen + 1));
    if (newbuf == NULL) {
        return NULL;
    }

    p = arg;
    dst = newbuf;
    equalPoint = NULL;

    /* copy first 'not space' string (environment variable name)*/
    while ((*p != '=') && (*p != ' ') && (*p != '\t') &&
           (*p != NGL_CHAR_COMMENT) &&
           (*p != '\n') && (*p != '\r') && (*p != '\0')) {
        *dst = *p;    
        p++;
        dst++;
    }

    /* If no environment variable name was set, then error */
    if (dst == newbuf) {
        globus_libc_free(newbuf);
        return NULL;
    }

    /* skip spaces */
    while ((*p == ' ') || (*p == '\t')) {
        p++;
    }

    /* if no equal appeared, then only variable name was set */
    if  ((*p == NGL_CHAR_COMMENT) ||
        (*p == '\n') || (*p == '\r') || (*p == '\0')) {
        *dst = '\0';
        return newbuf;
    }

    if (*p != '=') {
        globus_libc_free(newbuf);
        return NULL;
    }

    *dst = *p; /* copy '=' */
    p++;
    dst++;
    equalPoint = dst;

    /* skip spaces */
    while ((*p == ' ') || (*p == '\t')) {
        p++;
    }

    if (*p == '"') {
        p++;
        *dst = '\0';
        result = nglCopyQuotedString(
            dst, p, (argLen + 1) - strlen(newbuf), 0, NULL);
        if (result == 0) {
            globus_libc_free(newbuf);
            return NULL;
        }
    } else {
        while ((*p != ' ') && (*p != '\t') &&
               (*p != NGL_CHAR_COMMENT) &&
               (*p != '\n') && (*p != '\r') && (*p != '\0')) {
            *dst = *p;    
            p++;
            dst++;
        }
        /* If no environment variable's value was set, then error */
        if (dst == equalPoint) {
            globus_libc_free(newbuf);
            return NULL;
        }

        /* Is there any token remain? */
        end = p;
        resultStr = nglFindTokenBeginInStr(end);
        if (resultStr != NULL) {
            globus_libc_free(newbuf);
            return NULL;
        }

        *dst = '\0';
    }

    return newbuf;
}

/**
 * This function is for to read string formatted
 *     "string = num". (eg. "pi/pi_trial=3")
 *  result is stored to str and num.
 * It's also ok for format " = num", only stored to num instead.
 */
int
ngiReadStrEqualNumberFromArg(char *arg, char **str, int *num)
{
    char *newbuf, *p, *dst, *tokenAvail;
    int result, argLen;

    assert(arg != NULL);
    assert(str != NULL);
    assert(num != NULL);

    *str = NULL;
    *num = 0;

    p = arg;
    if (*p == '=') {
        p++;
        return ngiReadIntFromArg(p, num);
    }

    /* Check if arg includes only number */
    while (isdigit((int)*p)) {
        p++;
    }
    tokenAvail = nglFindTokenBeginInStr(p);
    if (tokenAvail == NULL) {
        return ngiReadIntFromArg(arg, num);
    }

    /* string = num format */
    argLen = strlen(arg);
    newbuf = (char *)globus_libc_malloc(sizeof(char) * (argLen + 1));
    if (newbuf == NULL) {
        return 0;
    }

    p = arg;
    dst = newbuf;

    /* copy first 'not space' string */
    while ((*p != '=') && (*p != ' ') && (*p != '\t') &&
           (*p != NGL_CHAR_COMMENT) &&
           (*p != '\n') && (*p != '\r') && (*p != '\0')) {
        *dst = *p;    
        p++;
        dst++;
    }
    *dst = '\0';

    /* skip spaces */
    while ((*p == ' ') || (*p == '\t')) {
        p++;
    }

    if  (*p != '=') {
        globus_libc_free(newbuf);
        return 0;
    } else {
        p++; 
    }

    /* skip spaces */
    while ((*p == ' ') || (*p == '\t')) {
        p++;
    }
   
    /* Get number */
    result = ngiReadIntFromArg(p, num);
    if  (result != 1) {
        globus_libc_free(newbuf);
        return 0;
    }

    *str = newbuf;

    /* Success */
    return 1;
}

/**
 * Output Syntax Error
 */
int
ngiConfigFileSyntaxError(
    ngLog_t *log,
    ngiTokenInfo_t *token,
    char *errMessage,
    const char *fName,
    char *attrName,
    char *keyword,
    int *error)
{
    /* Set the Error */
    NGI_SET_ERROR(error, NG_ERROR_CONFIGFILE_SYNTAX);

    /* Output the Log */
    ngLogPrintf(log,
        NG_LOG_CATEGORY_NINFG_PURE, NG_LOG_LEVEL_ERROR, NULL,
        "%s:syntax error:%s:line %ld:%s%s%s %s%s%s%s.\n",
        fName,
        token->nti_readInfo->ntri_filename,
        token->nti_readInfo->ntri_lineno,
        ((attrName != NULL) ? "attribute " : ""),
        ((attrName != NULL) ? attrName : ""),
        ((attrName != NULL) ? ":" : ""),
        errMessage,
        ((keyword != NULL) ? " \"" : ""),
        ((keyword != NULL) ? keyword : ""),
        ((keyword != NULL) ? "\"" : ""));

    return 1;
}

/**
 * StringList_t
 */

/**
 * Construct
 */
ngiStringList_t *
ngiStringListConstruct(char *string)
{
    ngiStringList_t *newStringList;

    if (string == NULL) {
        return NULL;
    }
    
    newStringList = (ngiStringList_t *)
        globus_libc_malloc(sizeof(ngiStringList_t));
    if (newStringList == NULL) {
        return NULL;
    }
    newStringList->nsl_next = NULL;
    newStringList->nsl_string = strdup(string);
    if (newStringList->nsl_string == NULL) {
        globus_libc_free(newStringList);
        return NULL;
    }

    return newStringList;
}

/**
 * Destruct
 */
int
ngiStringListDestruct(ngiStringList_t *stringList)
{
    ngiStringList_t *cur, *next;

    if (stringList == NULL) {
        return 0;
    }

    cur = stringList;
    while (cur != NULL) {
        next = cur->nsl_next;

        assert(cur->nsl_string != NULL);
        globus_libc_free(cur->nsl_string);
        globus_libc_free(cur);
        cur = next;
    }

    /* Success */
    return 1;
}

/**
 * Register the StringList
 */
int
ngiStringListRegisterList(ngiStringList_t **dst, ngiStringList_t *src)
{
    ngiStringList_t *cur;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    if (*dst == NULL) {
        *dst = src;
        return 1;
    }

    cur = *dst;
    while (cur->nsl_next != NULL) {
        cur = cur->nsl_next;
    }

    cur->nsl_next = src;

    return 1;
}

/**
 * Count the number of strings in StringList
 */
int
ngiStringListCount(ngiStringList_t *stringList, int *count)
{
    ngiStringList_t *cur;
    int i;

    *count = 0;

    /* Is empty? */
    if (stringList == NULL) {
        *count = 0;
        return 1;
    }

    i = 0;
    cur = stringList;
    while (cur != NULL) {
        i++;
        cur = cur->nsl_next;
    }

    *count = i;
    return 1;
}

/**
 * This function checks string list a, and b including same string.
 * if included, return pointer of first found element,
 * if no strings are commonly included, return NULL.
 *   returned object is not malloc()ed one, so don't free.
 */
ngiStringList_t *
ngiStringListCheckListIncludeSameString(
    ngiStringList_t *stringsA,
    ngiStringList_t *stringsB,
    int caseSensitive)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    ngiStringList_t *cur_a, *cur_b;

    cmp = (caseSensitive ? strncmp : strncasecmp);
    assert(cmp != NULL);

    cur_a = stringsA;
    while (cur_a != NULL) {
        cur_b = stringsB;
        while (cur_b != NULL) {
            assert(cur_a->nsl_string != NULL);
            assert(cur_b->nsl_string != NULL);

            if((*cmp)(cur_a->nsl_string,
                      cur_b->nsl_string, NGI_CONFIG_LINE_MAX) == 0) {
                return cur_a;
            }
            cur_b = cur_b->nsl_next;
        }
        cur_a = cur_a->nsl_next;
    }

    return NULL;
}

/**
 * Register the string
 */
int 
ngiStringListRegister(
    ngiStringList_t **dst,
    char *src)
{
    ngiStringList_t *newStrings;
    int result;

    if ((dst == NULL) || (src == NULL)) {
        return 0;
    }

    newStrings = ngiStringListConstruct(src);
    if (newStrings == NULL) {
        return 0;
    }

    result = ngiStringListRegisterList(dst, newStrings);
    if (result != 1) {
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * Check the StringList
 */
int 
ngiStringListCheckIncludeSameString(
    ngiStringList_t *strings,
    char *str,
    int caseSensitive,
    int *isInclude)
{
    ngiStringList_t *stringsB, *found;
    int result;

    if ((str == NULL) || (isInclude == NULL)) {
        return 0;
    }

    stringsB = ngiStringListConstruct(str);
    if (stringsB == NULL) {
        return 0;
    }

    found = ngiStringListCheckListIncludeSameString(
        strings, stringsB, caseSensitive);

    if (found != NULL) {
        *isInclude = 1;
    } else {
        *isInclude = 0;
    }

    result = ngiStringListDestruct(stringsB);
    if (result != 1) {
        return 0;
    }

    /* Success */
    return 1;
}

/**
 * StringList set subtract
 */
int
ngiStringListSubtract(
    ngiStringList_t *stringsA,
    ngiStringList_t *stringsB,
    int caseSensitive,
    ngiStringList_t **resultStrings)
{
    int (*cmp)(const char *s1, const char *s2, size_t n);
    ngiStringList_t *returnStrings, *curA, *curB;
    int result, found;

    if (resultStrings == NULL) {
        return 0;
    }
    *resultStrings = NULL;

    cmp = (caseSensitive ? strncmp : strncasecmp);
    assert(cmp != NULL);

    returnStrings = NULL;
    curA = stringsA;
    while (curA != NULL) {
        assert(curA->nsl_string != NULL);
        
        /* Find curA available in stringB */
        found = 0; /* FALSE */
        curB = stringsB;
        while (curB != NULL) {
            assert(curB->nsl_string != NULL);
            if((*cmp)(curA->nsl_string,
                      curB->nsl_string, NGI_CONFIG_LINE_MAX) == 0) {
                found = 1; /* TRUE */
                break;
            }
            curB = curB->nsl_next;
        }

        if (!found) {
            result = ngiStringListRegister(&returnStrings, curA->nsl_string);
            if (result != 1) {
                return 0;
            }
        }

        curA = curA->nsl_next;
    }

    *resultStrings = returnStrings;

    /* Success */
    return 1;
}

/**
 * StringList duplicate
 */
ngiStringList_t *
ngiStringListDuplicate(
    ngiStringList_t *stringList)
{
    ngiStringList_t *newList, *cur;
    int result;

    newList = NULL;

    if (stringList == NULL) {
        return NULL;
    }

    cur = stringList;
    while (cur != NULL) {
        assert(cur->nsl_string != NULL);

        result = ngiStringListRegister(&newList, cur->nsl_string);
        if (result != 1) {
            return NULL;
        }
        cur = cur->nsl_next;
    }

    /* Success */
    return newList;
}

/**
 * Merge to one string
 */
char *
ngiStringListMergeToString(
    ngiStringList_t *strings)
{
    ngiStringList_t *cur;
    char *stringBuffer, *p;
    int strSize;

    strSize = nglStringListCountChars(strings);
    if (strSize < 1) {
        return NULL;
    }

    stringBuffer = globus_libc_malloc(sizeof(char) * (strSize + 1));
    if (stringBuffer == NULL) {
        return NULL;
    }

    cur = strings;
    p = stringBuffer;
    while (cur != NULL) {
        assert(cur->nsl_string != NULL);
        strcpy(p, cur->nsl_string);

        p += strlen(cur->nsl_string);
        cur = cur->nsl_next;
    }
    
    return stringBuffer;
}

/**
 * Count characters in StringList
 */
static int 
nglStringListCountChars(ngiStringList_t *strings)
{
    ngiStringList_t *cur;
    int sum;

    cur = strings;
    sum = 0;

    while (cur != NULL) {
        assert(cur->nsl_string != NULL);
        sum += strlen(cur->nsl_string);

        cur = cur->nsl_next;
    }

    return sum;
}

/**
 * Convert StringList to string array
 */
int
ngiStringListToStringArray(
    char ***dstArray,
    int *dstSize,
    ngiStringList_t *strings)
{
    ngiStringList_t *cur;
    int size, i;

    assert(dstArray != NULL);
    assert(dstSize != NULL);

    /* Count elements */
    cur = strings;
    size = 0;
    while (cur != NULL) {
        assert(cur->nsl_string != NULL);
        size++;
       cur = cur->nsl_next;
    }

    if (size <= 0) {
        return 0;
    }
    *dstSize = size;
    
    *dstArray = (char **)globus_libc_malloc(sizeof(char *) * size);
    cur = strings;
    for (i = 0; i < size; i++) {
        assert(cur != NULL);
        (*dstArray)[i] = strdup(cur->nsl_string);
        if ((*dstArray)[i] == NULL) {
            return 0;
        }
        cur = cur->nsl_next;
    }
    
    return 1;
}

