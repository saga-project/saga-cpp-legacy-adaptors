/*
 * $RCSfile: data_test.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:02 $
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
#include <math.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define N    5

#define MODULE_NAME "data"

static grpc_function_handle_t handles;

static int verbose = 0;

int char_test();
int short_test();
int int_test();
int long_test();
int float_test();
int double_test();
int string_test();
int string_array_test();
int scomplex_test();
int dcomplex_test();
int work_test();
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

    printf("char_testing: ");
    ret = char_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("short_testing: ");
    ret = short_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("int_testing: ");
    ret = int_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("long_testing: ");
    ret = long_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("float_testing: ");
    ret = float_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("double_testing: ");
    ret = double_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("string_testing: ");
    ret = string_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("string_array_testing: ");
    ret = string_array_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("scomplex_testing: ");
    ret = scomplex_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("dcomplex_testing: ");
    ret = dcomplex_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("work_testing: ");
    ret = work_test();
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
char_test()
{
    grpc_error_t ret;
    char a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/char_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, (char)N, a, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_call() error. (%s)\n", grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            success = 0;
            if (verbose) {
                printf("char_test: a[%d](%d) != b[%d](%d)\n",
                    i, a[i], i, b[i]);
            }
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, 
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
short_test()
{
    grpc_error_t ret;
    short a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/short_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, (short)N, a, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_call() error. (%s)\n", grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            success = 0;
            if (verbose) {
                printf("short_test: a[%d](%d) != b[%d](%d)\n",
                    i, a[i], i, b[i]);
            }
        }
    }
finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, 
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
int_test()
{
    grpc_error_t ret;
    int a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/int_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, N, a, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_call() error. (%s)\n", grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            success = 0;
            if (verbose) {
                printf("int_test: a[%d](%d) != b[%d](%d)\n", i, a[i], i, b[i]);
            }
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, 
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
long_test()
{
    grpc_error_t ret;
    long a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/long_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, (long)N, a, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_call() error. (%s)\n", grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            success = 0;
            if (verbose) {
                printf("long_test: a[%d](%ld) != b[%d](%ld)\n",
                    i, a[i], i, b[i]);
            }
        }
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, 
                "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return success;
}

int
float_test()
{
    grpc_error_t ret;
    float scalarIn, scalarOut;
    float a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/float_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, 
            "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    scalarIn = 1.0;
    scalarOut = 0.0;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, N, scalarIn, a, &scalarOut, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (scalarOut != scalarIn) {
	success = 0;
	if (verbose) {
	    printf("float_test: scalarIn(%f) != scalarOut(%f)\n",
		scalarIn, scalarOut);
	}
    }

    for (i = 0; i < N; i++) {
        if (!compare_float(a[i], b[i])) {
            success = 0;
            if (verbose) {
                printf("float_test: a[%d](%f) != b[%d](%f)\n",
                    i, a[i], i, b[i]);
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

int
double_test()
{
    grpc_error_t ret;
    double scalarIn, scalarOut;
    double a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/double_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    scalarIn = 1.0;
    scalarOut = 0.0;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }

    ret = grpc_call(&handles, N, scalarIn, a, &scalarOut, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (scalarOut != scalarIn) {
	success = 0;
	if (verbose) {
	    printf("double_test: scalarIn(%f) != scalarOut(%f)\n",
		scalarIn, scalarOut);
	}
    }

    for (i = 0; i < N; i++) {
        if (!compare_double(a[i],b[i])) {
            success = 0;
            if (verbose) {
                printf("double_test: a[%d](%f) != b[%d](%f)\n",
                    i, a[i], i, b[i]);
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

#include <string.h>
#include <stdlib.h>
#define TEST_STR_LENGTH 1000

int
string_test()
{
    grpc_error_t ret;
    char *buffer = NULL;
    char teststring[TEST_STR_LENGTH];
    int i;
    int success = 1;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/string_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < TEST_STR_LENGTH -1; i++) {
        *(teststring + i) = (i % 10) + '0';
    }
    *(teststring + i) = '\0';

    ret = grpc_call(&handles, teststring, &buffer);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (strcmp(teststring, buffer) != 0) {
        if (verbose) {
            printf("%s != %s\n", teststring, buffer);
        }
        success = 0;
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

    /* buffer was allocated in grpc_call() by malloc() */
    free(buffer);

    return success;
}

int
string_array_test()
{
    static const char *in[] = {
        "This is a test for array of string.\n",
        "Hello, World",
        "Sun Mon Tue Wed Thu Fri Sat",
        "ls -- list directory contents",
        "All tests were successful.",
    };

    static const char *answer[] = {
        "Were all tests successful?",
        "Sun Mon Tue Wed Thu Fri Sat",
        "Good morning",
    };

    char *out[3] = {NULL, NULL, NULL};
    grpc_error_t ret;
    int success = 1;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/string_array_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, in, out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if (strcmp(out[0], answer[0]) != 0) {
        if (verbose) {
            printf("%s != %s\n", out[0], answer[0]);
        }
        success = 0;
    }

    if (strcmp(out[1], answer[1]) != 0) {
        if (verbose) {
            printf("%s != %s\n", out[1], answer[1]);
        }
        success = 0;
    }

    if (strcmp(out[2], answer[2]) != 0) {
        if (verbose) {
            printf("%s != %s\n", out[2], answer[2]);
        }
        success = 0;
    }

finalize:
    /* out was allocated in grpc_call() by malloc() */
    free(out[0]);
    free(out[1]);
    free(out[2]);

    if (handleInitialized != 0) {
        ret = grpc_function_handle_destruct(&handles);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(ret));
            success = 0;
        }
        handleInitialized = 0;
    }

    return 1;
}

int
scomplex_test()
{
    grpc_error_t ret;
    scomplex scalarIn, scalarOut;
    scomplex a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    scalarIn.r = 1.0;
    scalarIn.i = 2.0;
    scalarOut.r = 0.0;
    scalarOut.i = 0.0;

    for (i = 0; i < N; i++) {
        a[i].r = i;
        a[i].i = i + 0.1; 
        b[i].r = 0;
        b[i].i = 0; 
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/scomplex_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, N, scalarIn, a, &scalarOut, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if ((scalarOut.r != scalarIn.r) || (scalarOut.i != scalarIn.i)) {
	success = 0;
	if (verbose) {
	    printf("scomplex_test: scalarIn(%f, %f) != scalarOut(%f, %f)\n",
		scalarIn.r, scalarIn.i, scalarOut.r, scalarOut.i);
	}
    }

    for (i = 0; i < N; i++) {
        if (!compare_float(a[i].r, b[i].r) || !compare_float(a[i].i, b[i].i)) {
            success = 0;
            if (verbose) {
                printf("scomplex_test: a[%d](%f, %f) != b[%d](%f, %f)\n", 
                    i, a[i].r, a[i].i, i, b[i].r, b[i].i);
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

int
dcomplex_test()
{
    grpc_error_t ret;
    dcomplex scalarIn, scalarOut;
    dcomplex a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    scalarIn.r = 1.0;
    scalarIn.i = 2.0;
    scalarOut.r = 0.0;
    scalarOut.i = 0.0;

    for (i = 0; i < N; i++) {
        a[i].r = i;
        a[i].i = i + 0.1; 
        b[i].r = 0;
        b[i].i = 0; 
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/dcomplex_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_call(&handles, N, scalarIn, a, &scalarOut, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }

    if ((scalarOut.r != scalarIn.r) || (scalarOut.i != scalarIn.i)) {
	success = 0;
	if (verbose) {
	    printf("dcomplex_test: scalarIn(%f, %f) != scalarOut(%f, %f)\n",
		scalarIn.r, scalarIn.i, scalarOut.r, scalarOut.i);
	}
    }

    for (i = 0; i < N; i++) {
        if (!compare_double(a[i].r, b[i].r) ||
            !compare_double(a[i].i, b[i].i)) {
            success = 0;
            if (verbose) {
                printf("dcomplex_test: a[%d](%f, %f) != b[%d](%f, %f)\n", 
	            i, a[i].r, a[i].i, i, b[i].r, b[i].i);
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

int
work_test()
{
    grpc_error_t ret;
    int a[N], b[N];
    int success = 1;
    int i;
    int handleInitialized = 0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/work_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    handleInitialized = 1;

    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = 0;
    }
    ret = grpc_call(&handles, N, a, NULL, b);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        success = 0;
        goto finalize;
    }
    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            success = 0;
            if (verbose) {
                printf("work_test: a[%d](%d) != b[%d](%d)\n",
                    i, a[i], i, b[i]);
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
