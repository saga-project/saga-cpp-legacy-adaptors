/*
 * $RCSfile: cancel_test.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:02 $
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

#include "grpc.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define N    5
#define M    3

#define MODULE_NAME "cancel"

static grpc_function_handle_t handles[M];
static int verbose = 0;

int cancel_test();
int cancel_all_test();

int
main(int argc, char *argv[])
{
    grpc_error_t result;
    char *config_file, *program_name;
    int ch;
    int ret, exit_code;

    program_name = argv[0];

    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
        case 'v' :
            verbose = 1;
            break;
        default :
            fprintf(stderr, "Usage: %s [-v] config\n", program_name);
            exit(2);
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        fprintf(stderr, "Usage: %s [-v] config\n", program_name);
        exit(2);
    }

    fprintf(stdout, "----------------------------------------------------\n");
    fprintf(stdout, " Please ignore error messages, if the result is OK.\n");
    fprintf(stdout, "----------------------------------------------------\n");

    config_file = argv[0];
    exit_code = 0;

    setbuf(stdout, NULL);

    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    printf("cancel   testing: ");
    ret = cancel_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("cancel_all testing: ");
    ret = cancel_all_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_finalize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    return exit_code;
}

int
cancel_test()
{
    grpc_sessionid_t id;
    grpc_error_t ret;
    int rVal = 1;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles[0], MODULE_NAME "/cancel");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call_async(&handles[0], &id);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call_async() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    ret = grpc_cancel(id);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_cancel() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    ret = grpc_wait(id);
    if (ret != GRPC_SESSION_FAILED) {
        fprintf(stderr, "grpc_wait() error. (%s)\n",
            grpc_error_string(ret));
        if (verbose) {
            printf("session %d was finished successfully...\n", id);
        }
        rVal = 0;
        goto finalize;
    }

    if (grpc_get_error(id) == GRPC_CANCELED_NP) {
        if (verbose) {
            printf("session %d is canceled\n", id);
        }
    } else {
        rVal = 0;
        if (verbose) {
            printf("session %d is not canceled: something went wrong.\n", id);
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles[0]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }
    return rVal;
}

int
cancel_all_test()
{
    grpc_sessionid_t id[M];
    grpc_error_t ret;
    int i;
    int nInit = 0;
    int nCall = 0;
    int rVal = 1;

    for (nInit = 0; nInit < M; nInit++) {
        ret = grpc_function_handle_default(&handles[nInit], MODULE_NAME "/cancel");
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

    for (nCall = 0; nCall < M; nCall++) {
        ret = grpc_call_async(&handles[nCall], &id[nCall]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_call_async() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            break;
        }
    }

    ret = grpc_cancel_all();
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_cancel() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_wait_all();
    if (ret != GRPC_SESSION_FAILED) {
        fprintf(stderr, "grpc_wait_all() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    for (i = 0; i < nCall; i++) {
        if (grpc_get_error(id[i]) == GRPC_CANCELED_NP) {
            if (verbose) {
                printf("session %d is canceled\n", id[i]);
            }
        } else {
            rVal = 0;
            if (verbose) {
                printf("session %d is not canceled: something went wrong.\n",
                    id[i]);
            }
        }
    }

finalize:
    for (i = 0; i < nInit; i++) {
        ret = grpc_function_handle_destruct(&handles[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
    }

    return rVal;
}
