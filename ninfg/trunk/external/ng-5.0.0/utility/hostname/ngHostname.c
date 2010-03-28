/*
 * $RCSfile: ngHostname.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $
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

NGI_RCSID_EMBED("$RCSfile: ngHostname.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $")

static const char helpString[] =
"\n"
"Usage: $NG_DIR/bin/ng_hostname [-h]\n"
"\n"
"  ng_hostname prints out the hostname.\n"
"  If the environment variable NG_HOSTNAME is set,\n"
"  the value of the environment variable is used.\n"
"  ng_hostname is not just gethostname(), try to get FQDN.\n"
"\n"
"Options:\n"
"    -h : help.\n"
"\n";

/**
 * main : output hostname.
 */
int
main(int argc, char *argv[])
{
    char name[NGI_HOST_NAME_MAX];
    int result, i;

    for (i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "-H") == 0) ||
            (strcmp(argv[i], "-help") == 0) ||
            (strcmp(argv[i], "--help") == 0)) {

            fprintf(stderr, "%s", helpString);
            exit(0);

        } else {
            fprintf(stderr, "Invalid argument. try -h for help.\n");
            exit(1);
        }
    }

    result = ngiHostnameGet(name, NGI_HOST_NAME_MAX, NULL, NULL);
    if (result == 0) {
        fprintf(stderr, "hostname was not found.\n");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", name);

    exit(EXIT_SUCCESS);
}

