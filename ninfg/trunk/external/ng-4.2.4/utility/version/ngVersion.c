#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: ngVersion.c,v $ $Revision: 1.10 $ $Date: 2005/10/27 07:19:42 $";
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

#include <stdio.h>
#include <stdlib.h>
#include "ng.h"
#include "ngConfigureOptions.h"

static char *usage =
"Usage ng_version [-v]\n"
"\n"
"print version of Ninf-G.\n"
"\n"
" -v : print configure options.\n"
"\n";


/**
 * main : output version number
 */
int
main(int argc, char *argv[])
{
    char *version;
    int error;
    int ch;
    int displayConfigureOptions;

    displayConfigureOptions = 0;

    while ((ch = getopt(argc, argv, "vh")) != -1) {
        switch (ch) {
        case 'v' :
            displayConfigureOptions = 1;
            break;
        case 'h' :
        case '?' :
        default :
            fprintf(stderr, usage);
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;

    version = ngclGetVersion(&error);
    if (version == NULL) {
    	fprintf(stderr, "Can't get the version.\n");
        exit(EXIT_FAILURE);
    }

    printf("Ninf-G version %s\n", version);

    if (displayConfigureOptions != 0) {
        printf("Configure Options: %s\n", configureOptions);
        printf("%s\n", configureHost);
        printf("%s\n", globusLocation);
        printf("%s\n", gptLocation);
    }

    exit(EXIT_SUCCESS);
}

char *
ngclGetVersion(int *error)
{
    char *versionFull, *versionStart, *versionReturn, *versionEnd;

    NGI_SET_ERROR(error, NG_ERROR_NO_ERROR);

    versionFull = "$AIST_Release: 4.2.4 $";

    /**
     * If version string replaced to the keyword by the release script,
     * a character : (colon) will appear.
     * If not, then the source was CVS checked out one.
     * That's not released Ninf-G.
     */
    versionStart = strchr(versionFull, ':');
    if (versionStart != NULL) {
        /* skip first ' ' */
        while (*versionStart == ' ') {
            versionStart++;
        }

        versionReturn = strdup(versionStart);

        /* cut last '$' */
        versionEnd = strrchr(versionReturn, '$');
        if (versionEnd != NULL) {
            *versionEnd = '\0';
        }
    } else {
        versionReturn = strdup("CVS");
    }

    return versionReturn;
}

