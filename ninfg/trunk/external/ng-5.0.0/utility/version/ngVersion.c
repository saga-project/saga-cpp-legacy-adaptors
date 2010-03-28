/*
 * $RCSfile: ngVersion.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $
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

#include "ng.h"

NGI_RCSID_EMBED("$RCSfile: ngVersion.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $")

static char *usage =
"Usage ng_version [-v]\n"
"\n"
"print version of Ninf-G.\n"
"\n"
" -v : print configure options and results\n"
"\n";

/**
 * main : output version number
 */
int
main(int argc, char *argv[])
{
    int displayConfigureOptions;
    char *version, *configure;
    int result, ch;

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

    version = NULL;
    result = ngVersionGet(&version, NULL, NULL);
    if ((result == 0) || (version == NULL)) {
        fprintf(stderr, "failed to get the Ninf-G version.\n");
        exit(1);
    }

    printf("Ninf-G version %s\n", version);

    if (displayConfigureOptions != 0) {
        configure = NULL;
        result = ngConfigureGet(&configure, NULL, NULL);
        if ((result == 0) || (configure == NULL)) {
            fprintf(stderr, "failed to get the Ninf-G configure result.\n");
            exit(1);
        }

        printf("\n");
        printf("%s", configure);
    }

    exit(0);
}

