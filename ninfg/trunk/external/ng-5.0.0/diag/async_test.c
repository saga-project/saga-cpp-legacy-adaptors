/*
 * $RCSfile: async_test.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:02 $
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

#define MODULE_NAME "async"

static grpc_function_handle_t handles[M];
static int verbose = 0;

int wait_all_test();
int wait_any_test();
int wait_test();
int wait_and_test();
int wait_or_test();

void init_data(double a[][N], double b[][N]);
void init_data_for_or(int arg[]);
int check_data(double a[][N], double b[][N]);

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

    config_file = argv[0];
    exit_code = 0;

    setbuf(stdout, NULL);

    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    printf("wait     testing: ");
    ret = wait_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("wait_all testing: ");
    ret = wait_all_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("wait_and testing: ");
    ret = wait_and_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("wait_any testing: ");
    ret = wait_any_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("wait_or  testing: ");
    ret = wait_or_test();
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
wait_all_test()
{
    grpc_sessionid_t id[M];
    grpc_error_t ret;
    double scalarIn[M], scalarOut[M];
    double a[M][N], b[M][N];
    int i;
    int nInit;
    int nCall;
    int rVal = 1;

    init_data(a, b);

    for (nInit = 0; nInit < M; nInit++) {
        ret = grpc_function_handle_default(&handles[nInit],
            MODULE_NAME "/double_test");
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

    nCall = 0;
    for (i = 0; i < nInit; i++) {
        ret = grpc_call_async(&handles[i], &id[i],
            N, scalarIn[i], a[i], &scalarOut[i], b[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_call_async() error. (%s)\n", grpc_error_string(ret));
            rVal = 0;
        } else {
            nCall++;
        }
    }

    if (nCall > 0) {
        ret = grpc_wait_all();
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_wait_all() error. (%s)\n", grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

finalize:
    for (i = 0; i < nInit; i++) {
        ret = grpc_function_handle_destruct(&handles[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
    }

    return rVal && check_data(a, b);
}

int
wait_any_test()
{
    grpc_sessionid_t id[M];
    grpc_error_t ret;
    double scalarIn[M], scalarOut[M];
    double a[M][N], b[M][N];
    int i;
    int nInit;
    int rVal = 1;
    grpc_sessionid_t wait_id = GRPC_SESSIONID_VOID;

    init_data(a, b);

    for (nInit = 0; nInit < M; nInit++) {
        ret = grpc_function_handle_default(&handles[nInit],
            MODULE_NAME "/double_test");
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

    for (i = 0; i < nInit; i++) {
        ret = grpc_call_async(&handles[i], &id[i],
            N, scalarIn[i], a[i], &scalarOut[i], b[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_call_async() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            break;
        }
    }

    do {
        wait_id = GRPC_SESSIONID_VOID;

        ret = grpc_wait_any(&wait_id);
        if (ret == GRPC_NO_ERROR) {
            if (verbose) {
                printf("session %d done\n", wait_id);
            }
        } else {
            fprintf(stderr,
                "grpc_wait_any() error. (%s)\n", grpc_error_string(ret));
            rVal = 0;

            if ((ret != GRPC_SESSION_FAILED) &&
                (ret != GRPC_TIMEOUT_NP    ) &&
                (ret != GRPC_CANCELED_NP   )) {
                break;
            }
        }
    } while (wait_id != GRPC_SESSIONID_VOID);

finalize:
    for (i = 0; i < nInit; i++) {
        ret = grpc_function_handle_destruct(&handles[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
    }

    return rVal && check_data(a, b);
}

int
wait_test()
{
    grpc_sessionid_t id[M];
    grpc_error_t ret;
    double scalarIn[M], scalarOut[M];
    double a[M][N], b[M][N];
    int i;
    int nInit;
    int nCall;
    int rVal = 1;

    init_data(a, b);

    nInit = 0;
    nCall = 0;
    for (i = 0; i < M; i++) {
        ret = grpc_function_handle_default(&handles[i],
            MODULE_NAME "/double_test");
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            break;
        }
        nInit++;

        ret = grpc_call_async(&handles[i], &id[i],
            N, scalarIn[i], a[i], &scalarOut[i], b[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr,
                "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            printf("%dth invocation failed", i + 1);
            rVal = 0;
            break;
        }
        nCall++;
    }

    for (i = 0; i < nCall; i++) {
        ret = grpc_wait(id[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_wait() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        if (verbose) {
            printf("res[%d] = %d,", i, ret);
        }
    }

    for (i = 0; i < nInit; i++) {
        ret = grpc_function_handle_destruct(&handles[i]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
    }

    return rVal && check_data(a, b);
}

int
wait_and_test()
{
    grpc_sessionid_t id[M];
    grpc_error_t ret;
    double scalarIn[M], scalarOut[M];
    double a[M][N], b[M][N];
    int i;
    int nInit;
    int nCall;
    int rVal = 1;

    init_data(a, b);

    for (nInit = 0; nInit < M; nInit++) {
        ret = grpc_function_handle_default(&handles[nInit],
            MODULE_NAME "/double_test"); 
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

    for (nCall = 0; nCall < M; nCall++){
        ret = grpc_call_async(&handles[nCall], &id[nCall],
            N, scalarIn[nCall], a[nCall], &scalarOut[nCall], b[nCall]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_call_async() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            break;
        }
    }

    if (nCall > 0) {
        ret = grpc_wait_and(id, nCall);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_wait_and() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
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

    return rVal && check_data(a, b);
}

int
wait_or_test()
{  
    grpc_sessionid_t id[M], id2;
    grpc_error_t ret;
    int arg[M];
    int i, j, found;
    int nInit;
    int nCall;
    int rVal = 1;

    for (nInit = 0; nInit < M; nInit++) {
        ret = grpc_function_handle_default(&handles[nInit], MODULE_NAME "/sleep");
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            goto finalize;
        }
    }

    init_data_for_or(arg);

    for (nCall = 0; nCall < M; nCall++) {
        ret = grpc_call_async(&handles[nCall], &id[nCall], arg[nCall]);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_call_async() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            break;
        }
    }

    for (j = nCall; j > 0; j--) {
        id2 = GRPC_SESSIONID_VOID;
        ret = grpc_wait_or(id, j, &id2);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_wait_or() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
            if ((ret != GRPC_SESSION_FAILED) &&
                (ret != GRPC_TIMEOUT_NP    ) &&
                (ret != GRPC_CANCELED_NP   )) {
                break;
            }
        }

        /* Find the returned id index and replace it with last id */
        found = 0;
        for (i = 0; i < j; i++) {
            if (id[i] == id2) {
                id[i] = id[j - 1];
                id[j - 1] = id2;
                found = 1;
                break;
            }
        }

        if (found == 0) {
            if (verbose) {
                printf("Returned session %d was not found.\n", id2);
            }
            rVal = 0;
            break;
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

void
init_data(double a[][N], double b[][N])
{
    int i, j;

    for (j = 0; j < M; j++) {
        for (i = 0; i < N; i++) {
            a[j][i] = i + 100 * j;
            b[j][i] = 0;
        }
    }
}

void
init_data_for_or(int arg[])
{
    int i;
    arg[0] = 1;
    for (i = 1; i < M; i++) {
        arg[i] = 30;
    }
}

int
check_data(double a[][N], double b[][N])
{
    int i, j;

    for (j = 0; j < M; j++) {
        for (i = 0; i < N; i++) {
            if (a[j][i] != b[j][i]) {
                if (verbose) {
                    printf("a[%d][%d](%f) != b[%d][%d](%f)\n",
                        j, i, a[j][i], j, i, b[j][i]);
                }
                return 0;
            }
        }
    }
    return 1;
}
