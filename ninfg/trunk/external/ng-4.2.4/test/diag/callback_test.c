#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: callback_test.c,v $ $Revision: 1.14 $ $Date: 2007/05/16 09:59:42 $";
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
#include <string.h>
#include <unistd.h>
#include "grpc.h"
#define N    5

#define MODULE_NAME "callback"

static grpc_function_handle_t handles;

static int verbose = 0;

int callback_test();
int callback_return_test();
int callback_multi_test();
int callback2D_test();
int callback_string_test();
int callback_max_test();

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

    printf("callback_testing: ");
    ret = callback_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("callback2D_testing: ");
    ret = callback2D_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("callback_return_testing: ");
    ret = callback_return_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("callback_multi_testing: ");
    ret = callback_multi_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("callback_string_testing: ");
    ret = callback_string_test();
    printf(ret ? "\tOK\n" : "\tfailed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("callback_max_testing: ");
    ret = callback_max_test();
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

double sum;

void
callback_func(double *dp)
{
    if (verbose) {
        printf("callbacked %f\n", *dp);
    }

    sum += *dp;
}

int
callback_test()
{
    grpc_error_t ret;
    double initial = 100.0, lsum = 0.0;
    int times = 1;
    int rVal = 1;

    sum = 0.0;

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/callback_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto end;
    }

    ret = grpc_call(&handles, initial, times, callback_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    if (rVal != 0) {
        int i;
        double d = 0.0;

        for (i = 0; i < times; i++) {
            d += initial;
            lsum += d;
        }
        if (sum != lsum) {
            if (verbose) {
                printf("sum = %f(should be %f)\n", sum, lsum);
            }
            rVal = 0;
        }
    }

end:
    return rVal;
}

double global_dp[N][N];

void
callback2D_func(double dp[][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            global_dp[i][j] = dp[i][j];
        }
    }
}

int
callback2D_test()
{
    grpc_error_t ret;
    double a[N][N];
    int i, j;
    int rVal = 1;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            a[i][j] = i * N + j;
        }
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/callback2D_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto end;
    }

    ret = grpc_call(&handles, N, a, callback2D_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (a[i][j] != global_dp[i][j]) {
                    rVal = 0;
                    goto end;
                }
            }
        }
    }

end:
    return rVal;
}

void
callback_return_func(double c[][N], double d[][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            d[i][j] = c[i][j];
        }
    }
}

int
callback_return_test()
{
    grpc_error_t ret;
    double a[N][N], b[N][N];
    int i, j;
    int rVal = 1;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            a[i][j] = i * N + j;
            b[i][j] = 0.0;
        }
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/callback_return_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto end;
    }

    ret = grpc_call(&handles, N, a, b, callback_return_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (a[i][j] != b[i][j]) {
                    rVal = 0;
                    goto end;
                }
            }
        }
    }

end:
    return rVal;
}

int
callback_multi_test()
{
    grpc_error_t ret;
    double a[N][N], b[N][N];
    int i, j;
    int rVal = 1;

    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            a[i][j] = i * N + j;
            b[i][j] = 0.0;
        }
    }

    ret = grpc_function_handle_default(&handles,
        MODULE_NAME "/callback_multi_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto end;
    }

    ret = grpc_call(&handles, N, a, b,
        callback_return_func, callback_return_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    if (rVal != 0) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                if (a[i][j] != b[i][j]) {
                    rVal = 0;
                    goto end;
                }
            }
        }
    }

end:
    return rVal;
}

char *callbacked;

void
callbackstring_func(char **str)
{
    callbacked = strdup(*str);
}

int
callback_string_test()
{
    grpc_error_t ret;
    char *strings[] = {"caller0","caller1","caller2","caller3","caller4"};
    char string[64]= "";
    int i;
    int rVal = 1;

    for (i = 0; i < 5; i++) {
        strcat(string, strings[i]);
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/callbackstr");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto end;
    }

    ret = grpc_call(&handles, strings, callbackstring_func);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
    }

    if (verbose) {
        printf("\ncallbacked = '%s'", callbacked);
    }

    if (verbose) {
        printf("\nfrontend   = '%s'\n", string);
    }

    if (rVal != 0) {
        if (strcmp(callbacked, string) != 0) {
            rVal = 0;
        }
    }

end:
    return rVal;
}

int callback_max_working;
int callback_max_comp_value;

void
callback_max_start()
{
    callback_max_working = 1;
}

void
callback_max_end()
{
    callback_max_working = 0;
}

void
callback_max_get_func(
    int *g01, int *g02, int *g03, int *g04,
    int *g05, int *g06, int *g07, int *g08,
    int *g09, int *g10, int *g11, int *g12,
    int *g13, int *g14, int *g15, int *g16,
    int *g17, int *g18, int *g19, int *g20,
    int *g21, int *g22, int *g23, int *g24,
    int *g25, int *g26, int *g27, int *g28,
    int *g29, int *g30, int *g31, int *g32)
{
    assert((g01 != NULL) && (g02 != NULL) && (g03 != NULL) && (g04 != NULL));
    assert((g05 != NULL) && (g06 != NULL) && (g07 != NULL) && (g08 != NULL));
    assert((g09 != NULL) && (g10 != NULL) && (g11 != NULL) && (g12 != NULL));
    assert((g13 != NULL) && (g14 != NULL) && (g15 != NULL) && (g16 != NULL));
    assert((g17 != NULL) && (g18 != NULL) && (g19 != NULL) && (g20 != NULL));
    assert((g21 != NULL) && (g22 != NULL) && (g23 != NULL) && (g24 != NULL));
    assert((g25 != NULL) && (g26 != NULL) && (g27 != NULL) && (g28 != NULL));
    assert((g29 != NULL) && (g30 != NULL) && (g31 != NULL) && (g32 != NULL));

    if (callback_max_working == 0) {
        return;
    }

    *g01 =  1; *g02 =  2; *g03 =  3; *g04 =  4;
    *g05 =  5; *g06 =  6; *g07 =  7; *g08 =  8;
    *g09 =  9; *g10 = 10; *g11 = 11; *g12 = 12;
    *g13 = 13; *g14 = 14; *g15 = 15; *g16 = 16;
    *g17 = 17; *g18 = 18; *g19 = 19; *g20 = 20;
    *g21 = 21; *g22 = 22; *g23 = 23; *g24 = 24;
    *g25 = 25; *g26 = 26; *g27 = 27; *g28 = 28;
    *g29 = 29; *g30 = 30; *g31 = 31; *g32 = 32;
}

void
callback_max_put_func(
    int *p01, int *p02, int *p03, int *p04,
    int *p05, int *p06, int *p07, int *p08,
    int *p09, int *p10, int *p11, int *p12,
    int *p13, int *p14, int *p15, int *p16,
    int *p17, int *p18, int *p19, int *p20,
    int *p21, int *p22, int *p23, int *p24,
    int *p25, int *p26, int *p27, int *p28,
    int *p29, int *p30, int *p31, int *p32)
{
    int *comp;
    
    assert((p01 != NULL) && (p02 != NULL) && (p03 != NULL) && (p04 != NULL));
    assert((p05 != NULL) && (p06 != NULL) && (p07 != NULL) && (p08 != NULL));
    assert((p09 != NULL) && (p10 != NULL) && (p11 != NULL) && (p12 != NULL));
    assert((p13 != NULL) && (p14 != NULL) && (p15 != NULL) && (p16 != NULL));
    assert((p17 != NULL) && (p18 != NULL) && (p19 != NULL) && (p20 != NULL));
    assert((p21 != NULL) && (p22 != NULL) && (p23 != NULL) && (p24 != NULL));
    assert((p25 != NULL) && (p26 != NULL) && (p27 != NULL) && (p28 != NULL));
    assert((p29 != NULL) && (p30 != NULL) && (p31 != NULL) && (p32 != NULL));

    if (callback_max_working == 0) {
        return;
    }

    comp = &callback_max_comp_value;

    *comp += *p01 + *p02 + *p03 + *p04 + *p05 + *p06 + *p07 + *p08;
    *comp += *p09 + *p10 + *p11 + *p12 + *p13 + *p14 + *p15 + *p16;
    *comp += *p17 + *p18 + *p19 + *p20 + *p21 + *p22 + *p23 + *p24;
    *comp += *p25 + *p26 + *p27 + *p28 + *p29 + *p30 + *p31 + *p32;
}

int
callback_max_test()
{
    int in, out, *comp, finalComp, retResult, answer, i;
    grpc_object_handle_t_np objectHandle;
    grpc_error_t ret;
    int rVal = 1;
    int handleInitialized = 0;

    comp = &callback_max_comp_value;
    finalComp = 0;
    retResult = 1;

    ret = grpc_object_handle_default_np(
        &objectHandle, MODULE_NAME "/callback_max_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_object_handle_default_np() error. (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }
    handleInitialized = 1;

    ret = grpc_invoke_np(&objectHandle, "initialize");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (1). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    /* Compute  (1 + 2 + ... + 32) * 2 */
    answer = 0;
    for (i = 1; i <= 32; i++) {
        answer += i;
    }
    answer *= 2;

    *comp = 0;
    in = 1;
    out = 0;

    ret = grpc_invoke_np(&objectHandle, "max_method2x",
        in, &out, callback_max_start, callback_max_get_func,
        callback_max_put_func, callback_max_end);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (2). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    finalComp = *comp;

    if (out != 1) {
        retResult = 0;
    }

    if (finalComp != answer) {
        retResult = 0;
    }

    if (verbose) {
        printf("\nout = %d, answer = %d, comp = %d ",
            out, answer, finalComp);
    }

    out = 0;

    ret = grpc_invoke_np(&objectHandle, "get_result", &out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (3). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    if (out != 1) {
        retResult = 0;
    }

    if (verbose) {
        printf("\nresult = %d, ret = %d ", out, retResult);
    }

    /* Compute  (1 + 2 + ... + 32) * 4 */
    answer = 0;
    for (i = 1; i <= 32; i++) {
        answer += i;
    }
    answer *= 4;

    *comp = 0;
    in = 1;
    out = 0;

    ret = grpc_invoke_np(&objectHandle, "max_method4x",
        in, &out, callback_max_start, callback_max_get_func,
        callback_max_put_func, callback_max_end);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (4). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    finalComp = *comp;

    if (out != 1) {
        retResult = 0;
    }

    if (finalComp != answer) {
        retResult = 0;
    }

    if (verbose) {
        printf("\nout = %d, answer = %d, comp = %d ",
            out, answer, finalComp);
    }

    out = 0;

    ret = grpc_invoke_np(&objectHandle, "get_result", &out);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (5). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    if (out != 1) {
        retResult = 0;
    }

    if (verbose) {
        printf("\nresult = %d, ret = %d ", out, retResult);
    }

    ret = grpc_invoke_np(&objectHandle, "finalize");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_invoke_np() error (6). (%s)\n",
            grpc_error_string(ret));
        rVal = 0;
        goto finalize;
    }

    if (out != 1) {
        retResult = 0;
    }

    if (verbose) {
        printf("\nfinal ret = %d ", retResult);
    }

finalize:
    if (handleInitialized != 0) {
        ret = grpc_object_handle_destruct_np(&objectHandle);
        if (ret != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_object_handle_destruct_np() error. (%s)\n",
                grpc_error_string(ret));
            rVal = 0;
        }
    }

    if (retResult == 0) {
        rVal = 0;
    }

    return rVal;
}

