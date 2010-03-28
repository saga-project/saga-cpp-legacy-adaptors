/*
 * $RCSfile: ngCommunicationProxy.c,v $ $Revision: 1.7 $ $Date: 2008/02/26 02:32:11 $
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
 * Module for Communication Proxy(Ninf-G Client/Executable).
 */

#include "ngInternal.h"

NGI_RCSID_EMBED("$RCSfile: ngCommunicationProxy.c,v $ $Revision: 1.7 $ $Date: 2008/02/26 02:32:11 $")

static int nglIsCharAsIs(int, ngLog_t *, int *);

static const char nglCharSetAsIs[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_";
static const char nglCharSetXdigit[] = "0123456789ABCDEFabcdef";

/**
 * Encodes the string by the following way(like URL encoding).
 *  - Alphabetic characters, digits(0 through 9), '.', '-' and '_' is not changed. 
 *  - Space is changed to '+'.
 *  - Another character is changed to "%XX".
 *    (XX is an expression of the character-code by the hexadecimal
 *     number of two digits) 
 */
char *
ngiCommunicationProxyEncode(
    const char *src,
    ngLog_t *log,
    int *error)
{
    const char *p;
    char *q;
    int len;
    char *ret = NULL;
    static const char fName[] = "ngiCommunicationProxyEncode";

    assert(src != NULL);

    len = 0;
    for (p = src;*p != '\0';++p) {
        if (nglIsCharAsIs(*p, log, error) != 0) {
            len++;
        } else if (*p == ' ') {
            len++;
        }else {
            len += 3;/* length of %XX */
        }
    }
    len++; /* for '\0' */

    ret = ngiMalloc(len, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for string.\n");
        return NULL;

    }

    q = ret;
    for (p = src;*p != '\0';++p) {
        if (nglIsCharAsIs(*p, log, error) != 0) {
            *q = *p;
            q++;
        } else if (*p == ' ') {
            *q = '+';
            q++;
        } else {
            snprintf(q, 3+1, "%%%02X", (int)(unsigned char)*p); 
            q += 3;/* length of %XX */
        }
    }
    *q = '\0';

    return ret;
}

/**
 * Decode by the string encoded by ngiCommunicationProxyEncode().
 */
char *
ngiCommunicationProxyDecode(
    const char *src,
    ngLog_t *log,
    int *error)
{
    const char *p;
    char *q;
    int len;
    int req = 0;
    char hex[3];/* XX\0 */
    int i;
    int found;
    char *ret = NULL;
    static const char fName[] = "ngiCommunicationProxyDecode";

    assert(src != NULL);

    len = 0;
    req = 0;
    for (p = src;*p != '\0';++p) {
        if (req == 0) {
            if (*p == '%') {
                req = 2;
                /* %XX */
                /*  ^^ */
            } 
            len++;
        } else {
            found = 0;
            for (i = 0;nglCharSetXdigit[i] != '\0';++i) {
                if (nglCharSetXdigit[i] == *p) {
                    found = 1;
                    break;
                }
            }
            if (found == 0) {
                /* Error */
                ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
                    "%s: Invalid String.\n", src);
                NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
                return NULL;
            }
            req--;
        }
    }
    if (req > 0) {
        NGI_SET_ERROR(error, NG_ERROR_SYNTAX);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "%s: Invalid String.\n", src);
        return NULL;
    }
    len++; /* for '\0' */

    ret = ngiMalloc(len, log, error);
    if (ret == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,
            "Can't allocate storage for string.\n");
        return NULL;
    }

    q = ret;
    for (p = src;*p != '\0';++p) {
        if (*p == '%') {
            hex[0] = *++p;
            hex[1] = *++p;
            hex[2] = '\0';
            assert(hex[0] != '\0');
            assert(hex[1] != '\0');
            *q = (unsigned char)strtol(hex, NULL, 16);
        } else if (*p == '+') {
            *q = ' ';
        } else {
            *q = *p;
        }
        q++;
    }
    *q = '\0';

    return ret;
}

/**
 *  Is c Alphabetic character, digit(0 through 9), '.', '-' or '_'?
 */
static int
nglIsCharAsIs(
    int c,
    ngLog_t *log,
    int *error)
{
    const char *p;
#if 0
    static const char fName[] = "nglIsCharAsIs";
#endif

    for (p = nglCharSetAsIs;*p != '\0';++p) {
        if (c == (int)*p) {
            return 1;
        }
    }
    return 0;
}
