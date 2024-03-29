/*
 * $RCSfile: ngVersion.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:09 $
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
 * Utility module for Ninf-G.
 */

#include "ngUtility.h"
#include "ngConfigureResult.h" /* Generated by configure. */

NGI_RCSID_EMBED("$RCSfile: ngVersion.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:09 $")

/**
 * Prototype declaration of internal functions.
 */

/**
 * Functions.
 */

/**
 * Get the version of Ninf-G.
 * This is effective only on released version of Ninf-G.
 * Otherwise, the version string is "CVS".
 * This function allocates the string to the version.
 * The caller must be free this result.
 */
int
ngVersionGet(
    char **version,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngVersionGet";
    char *versionFull, *versionStart, *versionReturn, *versionEnd;

    /* Check the arguments */
    if (version == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, "Invalid argument.\n");
        return 0;
    }

    *version = NULL;

    versionFull = "$AIST_Release: 5.0.0 $";

    /**
     * If version string replaced to the keyword by the release script,
     * a character : (colon) will appear.
     * If not, then the source was CVS checked out one.
     * That's not released Ninf-G.
     */
    versionStart = strchr(versionFull, ':');
    if (versionStart == NULL) {

        /* This is CVS version of Ninf-G. */

        versionReturn = ngiStrdup("CVS", log, error);
        if (versionReturn == NULL) {
            ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
                "duplicate the string failed.\n");
            return 0;
        }

        *version = versionReturn;

        /* Success */
        return 1;
    }

    /* This is released version of Ninf-G. */

    /* skip first ' ' */
    while (*versionStart == ' ') {
        versionStart++;
    }

    versionReturn = ngiStrdup(versionStart, log, error);
    if (versionReturn == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "duplicate the string failed.\n");
        return 0;
    }

    /* cut last '$' */
    versionEnd = strrchr(versionReturn, '$');
    if (versionEnd != NULL) {
        *versionEnd = '\0';
    }

    *version = versionReturn;

    /* Success */
    return 1;
}

/**
 * Get the configure options and defines.
 * This function allocates the string to the configure result.
 * The caller must be free this result.
 */
int
ngConfigureGet(
    char **configureResult,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngConfigureGet";
    char *str;

    /* Check the arguments */
    if (configureResult == NULL) {
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName,
            "Invalid argument.\n");
        return 0;
    }

    *configureResult = NULL;

    str = ngiStrdup(nglConfigureResult, log, error);
    if (str == NULL) {
        ngLogError(log, NG_LOGCAT_NINFG_LIB, fName, 
            "duplicate the string failed.\n");
        return 0;
    }

    *configureResult = str;

    /* Success */
    return 1;
}

