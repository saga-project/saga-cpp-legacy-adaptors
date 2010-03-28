/*
 * $RCSfile: ngInvokeServerGT2c.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:04 $
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
 * Invoke Server for GT2 GRAM. C version.
 * The type name is "GT2c".
 *
 * This program is for to debug,
 * and reference to implement new type of Invoke Server.
 *
 * NOTE : This program supports only pthread flavor.
 *
 * stdin  : request
 * stdout : reply
 * stderr : notify
 *
 */

#include "ngEnvironment.h"
#ifdef NG_PTHREAD
#include "ngInvokeServer.h"
#endif /* NG_PTHREAD */

NGI_RCSID_EMBED("$RCSfile: ngInvokeServerGT2c.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:04 $")

static char *help =
"Ninf-G Invoke Server for GT2 GRAM C version.\n"
"Invoke Server type is \"GT2c\".\n"
"options:\n"
"  -l [log file name] : output the log file.\n"
"  -h : help.\n"
"\n";

/**
 * Prototype
 */
#ifdef NG_PTHREAD
static int ngislInvokeServerProcess(
    ngisiContext_t *, char *, char *, int, char **, int *);
#endif /* NG_PTHREAD */

/**
 * Data
 */
#ifdef NG_PTHREAD
static ngisiContext_t ngislContextEntity;
#endif /* NG_PTHREAD */

/**
 * Main
 */
int
main(
    int argc,
    char **argv)
{
    int *error, errorEntity;
    char *logFile;
    int i, result;

    logFile = NULL;
    error = &errorEntity;

    /* Process arguments */
    for (i = 1; i < argc; i++) {
        if ((argv[i])[0] != '-') {
            fprintf(stderr, "Invalid argument: \"%s\".\n", argv[i]);
            exit(1);
        }

        switch((argv[i])[1]) {
        case 'l':
            if (argc < (i + 1 + 1)) {
                fprintf(stderr, "No file specified for log file.\n");
                exit(1);
            }
            logFile = argv[i + 1];
            i++;
            break;
        case 'h':
            fprintf(stderr, help);
            exit(0);
            break;
        default:
            fprintf(stderr, "Invalid option: \"%s\".\n", argv[i]);
            exit(1);
            break;
        }
    }

#ifndef NG_PTHREAD
    {
        result = 0;
        error = NULL;
        errorEntity = 0;
        logFile = NULL;
        assert(result == 0);
        assert(error == NULL);
        assert(errorEntity == 0);
        assert(logFile == NULL);
        fprintf(stderr, "This binary was compiled with Non thread version.\n");
        fprintf(stderr, "Non thread version not supported.\n");
        exit(1);
    }

#else /* NG_PTHREAD */

    result = ngislInvokeServerProcess(
        &ngislContextEntity, NGISI_SERVER_TYPE, logFile, argc, argv, error);
    if (result == 0) {
        /* No error output for stderr */
        exit(1);
    }
     
#endif /* NG_PTHREAD */

    /* Success */
    exit(0);
}

#ifdef NG_PTHREAD
/**
 * Invoke Server Process
 */
static int
ngislInvokeServerProcess(
    ngisiContext_t *context,
    char *serverType, 
    char *logFile,
    int argc,
    char **argv,
    int *error)
{
    static const char fName[] = "ngislInvokeServerProcess";
    int result;

    /* Initialize the Context */
    result = ngisiContextInitialize(
        context, serverType, logFile, argc, argv, error);
    if (result == 0) {
        /* No error output for stderr */
        return 0;
    }

    result = ngisiRequestReaderWaitDone(context, error);
    if (result == 0) {
        ngisiLogPrintf(context, NGISI_LOG_LEVEL_ERROR, fName, 
            "Request Reader was done by error\n");
        return 0;
    }

    /* Finalize the Context */
    result = ngisiContextFinalize(
        context, error);
    if (result == 0) {
        /* No error output for stderr */
        return 0;
    }

    /* Success */
    return 1;
}
#endif /* NG_PTHREAD */

