#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: zeroElement_test.c,v $ $Revision: 1.5 $ $Date: 2007/05/16 09:59:42 $";
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
#include <math.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "grpc.h"
#define N    5
#define STRING_LENGTH 128

#define MODULE_NAME "zero_element"

static grpc_function_handle_t handles;

static int verbose = 0;

int zero_element_array_test();
int zero_length_string_test();
int zero_element_string_array_test();

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

    printf("zero_element_array_testing: ");
    ret = zero_element_array_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("zero_length_string_testing: ");
    ret = zero_length_string_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("zero_element_string_array_testing: ");
    ret = zero_element_string_array_test();
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
zero_element_array_test()
{
    grpc_error_t ret;
    char a_char[N], b_char[N];
    short a_short[N], b_short[N];
    int a_int[N], b_int[N];
    long a_long[N], b_long[N];
    float a_float[N], b_float[N];
    double a_double[N], b_double[N];
    scomplex a_scomplex[N], b_scomplex[N];
    dcomplex a_dcomplex[N], b_dcomplex[N];
    int success = 1, rpcResult = 0;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/zero_element_array");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a_char[i] = i;
        b_char[i] = N - i;
        a_short[i] = i;
        b_short[i] = N - i;
        a_int[i] = i;
        b_int[i] = N - i;
        a_long[i] = i;
        b_long[i] = N - i;
        a_float[i] = i;
        b_float[i] = N - i;
        a_double[i] = i;
        b_double[i] = N - i;
        a_scomplex[i].r = i;
        a_scomplex[i].i = i;
        b_scomplex[i].r = N - i;
        b_scomplex[i].i = N - i;
        a_dcomplex[i].r = i;
        a_dcomplex[i].i = i;
        b_dcomplex[i].r = N - i;
        b_dcomplex[i].i = N - i;
    }
    ret = grpc_call(&handles, 0, &rpcResult,
	a_char, b_char, a_short, b_short, a_int, b_int,  a_long, b_long,
	a_float, b_float, a_double, b_double, a_scomplex, b_scomplex,
	a_dcomplex, b_dcomplex);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (rpcResult == 0) {
	success = 0;
	if (verbose) {
	    printf("The error was occurred at Remote Executable.\n");
	}
        goto finalize;
    }

    for (i = 0; i < N; i++) {
        if ((a_char[i] != i) || (b_char[i] != N - i) ||
            (a_short[i] != i) || (b_short[i] != N - i) ||
            (a_int[i] != i) || (b_int[i] != N - i) ||
            (a_long[i] != i) || (b_long[i] != N - i) ||
            (a_float[i] != i) || (b_float[i] != N - i) ||
            (a_double[i] != i) || (b_double[i] != N - i) ||
            (a_scomplex[i].r != i) || (a_scomplex[i].i != i) ||
            (b_scomplex[i].r != N - i) || (b_scomplex[i].i != N - i) ||
            (a_dcomplex[i].r != i) || (a_dcomplex[i].i != i) ||
            (b_dcomplex[i].r != N - i) || (b_dcomplex[i].i != N - i)) {
	    success = 0;
            if (verbose) {
                printf("zero_element_array_test: [%d] failed\n", i);
	    }
            goto finalize;
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
zero_length_string_test()
{
    grpc_error_t ret;
    char *b_string = NULL;
    int success = 1, rpcResult = 0;
    int handleInitialized = 0;

    /* String length zero */
    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/zero_length_string");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, &rpcResult, "", &b_string);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (rpcResult == 0) {
	success = 0;
	if (verbose) {
	    printf("The error was occurred at Remote Executable.\n");
	}
    }

    if ((b_string == NULL) || (strcmp(b_string, "") != 0)) {
	success = 0;
	if (verbose) {
	    printf("OUT string is not equal \"\"\n");
	}
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
zero_element_string_array_test()
{
    grpc_error_t ret;
    char *a_string[N], *b_string[N];
    char a_buf[STRING_LENGTH], b_buf[STRING_LENGTH];
    int success = 1, rpcResult = 0;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/zero_element_string_array");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
	a_string[i] = malloc(STRING_LENGTH);
	assert(a_string != NULL);
	b_string[i] = malloc(STRING_LENGTH);
	assert(b_string != NULL);
	ret = sprintf(a_string[i], "%d", i);
	assert(ret > 0);
	ret = sprintf(b_string[i], "%d", N - i);
	assert(ret > 0);
    }
    ret = grpc_call(&handles, 0, &rpcResult, a_string, b_string);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (rpcResult == 0) {
	success = 0;
	if (verbose) {
	    printf("The error was occurred at Remote Executable.\n");
	}
        goto finalize;
    }

    for (i = 0; i < N; i++) {
	ret = sprintf(a_buf, "%d", i);
	assert(ret > 0);
	ret = sprintf(b_buf, "%d", N - i);
	assert(ret > 0);
	if ((strcmp(a_string[i], a_buf) != 0) ||
	    (strcmp(b_string[i], b_buf) != 0)) {
	    success = 0;
            if (verbose) {
                printf("zero_element_array_test: [%d] failed\n", i);
	    }
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}
