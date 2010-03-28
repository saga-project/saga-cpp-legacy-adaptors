/*
 * $RCSfile: skip_test.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:02 $
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

#include <stdio.h>
#include <string.h>
#include <math.h>

#define MODULE_NAME "skip_test"
#define SIZE 30
#define SKIP 3

static grpc_function_handle_t handles;

static int verbose = 0;

int skip_char_test();
int skip_short_test();
int skip_int_test();
int skip_long_test();
int skip_float_test();
int skip_double_test();
int skip_scomplex_test();
int skip_dcomplex_test();

int skip_inout_test();
int skip_one_test();

int skip_2D_test();
int skip_3D_test();

int skip_callback_test();

int skip_omit_skip_test();
int skip_omit_end_test();
int skip_omit_start_test();

int compare_float(float a, float b);
int compare_double(double a, double b);

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

    printf("skip_char_testing: ");
    ret = skip_char_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_short_testing: ");
    ret = skip_short_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_int_testing: ");
    ret = skip_int_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);
  
    printf("skip_long_testing: ");
    ret = skip_long_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_float_testing: ");
    ret = skip_int_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);
  
    printf("skip_double_testing: ");
    ret = skip_double_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_scomplex_testing: ");
    ret = skip_scomplex_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_dcomplex_testing: ");
    ret = skip_dcomplex_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_inout_testing: ");
    ret = skip_inout_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_one_testing: ");
    ret = skip_one_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_2D_testing: ");
    ret = skip_2D_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_3D_testing: ");
    ret = skip_3D_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_callback_testing: ");
    ret = skip_callback_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_omit_skip_testing: ");
    ret = skip_omit_skip_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_omit_end_testing: ");
    ret = skip_omit_end_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("skip_omit_start_testing: ");
    ret = skip_omit_start_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    ret = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_finalize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }
 
    return exit_code;
}

int
skip_char_test()
{
    grpc_error_t ret;
    char in[SIZE];
    char out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_char_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (out[i] != (in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_short_test()
{
    grpc_error_t ret;
    short in[SIZE];
    short out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_short_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (out[i] != (in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_int_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/skip_int_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (out[i] != (in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_long_test()
{
    grpc_error_t ret;
    long in[SIZE];
    long out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_long_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (out[i] != (in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%ld ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_float_test()
{
    grpc_error_t ret;
    float in[SIZE];
    float out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_float_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (!compare_float(out[i], in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%f ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_double_test()
{
    grpc_error_t ret;
    double in[SIZE];
    double out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_double_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (!compare_double(out[i], in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%f ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_scomplex_test()
{
    grpc_error_t ret;
    scomplex in[SIZE];
    scomplex out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i].r = i;
        in[i].i = i + 0.1;
    }

    for (i = 0; i < SIZE; i++) {
        out[i].r = 0;
        out[i].i = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_scomplex_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (!compare_float(out[i].r, in[i].r * 2) ||
                    !compare_float(out[i].i, in[i].i + 0.1)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (!compare_float(out[i].r, 0) ||
                    !compare_float(out[i].i, 0)) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%f, %f\n", out[i].r, out[i].i);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_dcomplex_test()
{
    grpc_error_t ret;
    dcomplex in[SIZE];
    dcomplex out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i].r = i;
        in[i].i = i + 0.1;
    }

    for (i = 0; i < SIZE; i++) {
        out[i].r = 0;
        out[i].i = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_dcomplex_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized  = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (!compare_double(out[i].r, in[i].r * 2) ||
                    !compare_double(out[i].i, in[i].i + 0.1)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (!compare_float(out[i].r, 0) ||
                    !compare_float(out[i].i, 0)) {
                    rVal = 0;
                    break;
                }
            }

            if (verbose) {
                printf("%f, %f\n", out[i].r, out[i].i);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_inout_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int inout[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        inout[i] = i * 2;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
          MODULE_NAME "/skip_inout_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, inout, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (inout[i] != (in[i] * 4)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (inout[i] != (in[i] * 2)) {
                    rVal = 0;
                    break;
                }
            }
            if (verbose) {
                printf("%d ", inout[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if ((i % SKIP) == 0) {
                if (out[i] != (in[i] * 8)) {
                    rVal = 0;
                    break;
                }
            } else {
                if (out[i] != 0) {
                    rVal = 0;
                    break;
                }
            }
            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_one_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/skip_int_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, 1, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if (out[i] != (in[i] * 2)) {
                rVal = 0;
                break;
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_2D_test()
{
    grpc_error_t ret;
    int in[SIZE][SIZE];
    int out[SIZE][SIZE];
    int i, j;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            in[i][j] = i + j;
        }
    }

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            out[i][j] = 0;
        }
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/skip_2D_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                if (((i % SKIP) == 0) && ((j % SKIP) == 0)) {
                    if (out[i][j] != (in[i][j] * 2)) {
                        rVal = 0;
                        goto out_of_loop;
                    }
                } else {
                    if (out[i][j] != 0) {
                        rVal = 0;
                        goto out_of_loop;
                    }
                }
                if (verbose) {
                    printf("%d ", out[i][j]);
                }
            }
            if (verbose) {
                printf("\n");
            }
        }
    }
out_of_loop:

    return rVal;
}

int
skip_3D_test()
{
    grpc_error_t ret;
    int in[SIZE][SIZE][SIZE];
    int out[SIZE][SIZE][SIZE];
    int i, j, k;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            for (k = 0; k < SIZE; k++) {
                in[i][j][k] = i + j + k;
            }
        }
    }

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            for (k = 0; k < SIZE; k++) {
                out[i][j][k] = 0;
            }
        }
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/skip_3D_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                for (k = 0; k < SIZE; k++) {
                    if (((i % SKIP) == 0) && ((j % SKIP) == 0) &&
                        ((k % SKIP) == 0)) {
                        if (out[i][j][k] != (in[i][j][k] * 2)) {
                            rVal = 0;
                            goto out_of_loop;
                        }
                    } else {
                        if (out[i][j][k] != 0) {
                            rVal = 0;
                            goto out_of_loop;
                        }
                    }
                }
                if (verbose) {
                    printf("%d ", out[i][j][k]);
                }
            }
            if (verbose) {
                printf("\n");
            }
        }
    }
out_of_loop:

    return rVal;
}

void
callback_func(
    int *size,
    int *start,
    int *end,
    int *skip,
    int in[][SIZE],
    int out[][SIZE])
{
    int i, j;

    memset(out, 0, sizeof (int) * *size * *size);
    for (i = (*start); i < (*size); i+= (*skip)) {
        for (j = (*start); j < (*size); j+= (*skip)) {
            out[i][j] = in[i][j] * 2;
        }
    }
}

int
skip_callback_test()
{
    grpc_error_t ret;
    int in[SIZE][SIZE];
    int out[SIZE][SIZE];
    int i, j;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            in[i][j] = i + j;
        }
    }

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            out[i][j] = 0;
        }
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_callback_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, SKIP, in, out, callback_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                if (((i % SKIP) == 0) && ((j % SKIP) == 0)) {
                    if (out[i][j] != (in[i][j] * 2)) {
                        rVal = 0;
                        goto out_of_loop;
                    }
                } else {
                    if (out[i][j] != 0) {
                        rVal = 0;
                        goto out_of_loop;
                    }
                }
                if (verbose) {
                    printf("%d ", out[i][j]);
                }
            }
            if (verbose) {
                printf("\n");
            }
        }
    }
out_of_loop:

    return rVal;
}

int
skip_omit_skip_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_omit_skip_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, SIZE, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if (out[i] != (in[i] * 2)) {
                rVal = 0;
                break;
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_omit_end_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_omit_end_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, 0, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if (out[i] != (in[i] * 2)) {
                rVal = 0;
                break;
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
skip_omit_start_test()
{
    grpc_error_t ret;
    int in[SIZE];
    int out[SIZE];
    int i;
    int handleInitialized = 0;
    int rVal = 1;

    for (i = 0; i < SIZE; i++) {
        in[i] = i;
    }

    for (i = 0; i < SIZE; i++) {
        out[i] = 0;
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/skip_omit_start_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, SIZE, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
        handleInitialized = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < SIZE; i++) {
            if (out[i] != (in[i] * 2)) {
                rVal = 0;
                break;
            }

            if (verbose) {
                printf("%d ", out[i]);
            }
        }
        if (verbose) {
            printf("\n");
        }
    }

    return rVal;
}

int
compare_float(float a, float b)
{
    if (a == 0.0 || b == 0.0) {
        return (a == b);
    }
    return (fabs((a - b) / a)) < 0.000001;
}

int
compare_double(double a, double b)
{
    if (a == 0.0 || b == 0.0) {
        return (a == b);
    }
    return (fabs((a - b) / a)) < 0.00000001;
}
