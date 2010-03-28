#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: add_client.c,v $ $Revision: 1.8 $ $Date: 2004/07/23 08:10:25 $";
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
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#include "grpc.h"

#define DEF_SCALAR 3

char *func_name = "add/dadd";
char *config_file = "client.conf";
int port = 0;

int
main(int argc, char *argv[])
{
    grpc_function_handle_t *handles;
    grpc_sessionid_t *ids = NULL, id;
    grpc_error_t result;
    char **hosts = NULL;
    double param1[DEF_SCALAR][DEF_SCALAR];
    double param2[DEF_SCALAR][DEF_SCALAR];
    double *param3;
    double wk1, wk2;
    int n;
    int index, base;
    int i, j, k;

    if (argc < 2) {
        fprintf(stderr, "USAGE: %s HOSTNAME.\n", argv[0]);
        fprintf(stderr, "USAGE: %s HOSTNAME1 HOSTNAME2...\n", argv[0]);
        exit(2);
    }

    n = argc - 1;
    hosts = (char **)malloc(sizeof(char *) * n);
    if (hosts == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage for hostname.\n");
        exit(2);
    }
    for (i = 1, j = 0; j < n; i++, j++) {
        hosts[j] = NULL;
        hosts[j] = strdup(argv[i]);
        if (*hosts == NULL) {
            fprintf(stderr,
                "strdup: Can't allocate the storage for hostname.\n");
            exit(2);
        }
    }

    handles = (grpc_function_handle_t *)malloc(
        sizeof(grpc_function_handle_t) * n);
    if (handles == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage for handles.\n");
        exit(2);
    }

    ids = (grpc_sessionid_t *)malloc(sizeof(grpc_sessionid_t) * n);
    if (ids == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage.\n");
        exit(2);
    }

    param3 = (double *)malloc(sizeof(double) * n * DEF_SCALAR * DEF_SCALAR);
    if (param3 == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage.\n");
        exit(2);
    }

    /* Initialize the parameter */
    for (j = 0, wk1 = 1.0, wk2 = 500.0; j < DEF_SCALAR; j++) {
        for (k = 0; k < DEF_SCALAR; k++) {
            param1[j][k] = wk1++;
            param2[j][k] = wk2++;
        }
    }
    for (i = 0; i < (n * DEF_SCALAR * DEF_SCALAR); i++) {
    	param3[i] = 0.0;
    }

    for (i = 0; i < DEF_SCALAR; i++) {
        for (j = 0; j < DEF_SCALAR; j++) {
            fprintf(stdout, "%f ", param1[i][j]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");

    for (i = 0; i < DEF_SCALAR; i++) {
        for (j = 0; j < DEF_SCALAR; j++) {
            fprintf(stdout, "%f ", param2[i][j]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
	    grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handles */
    for (i = 0; i < n; i++) {
        result = grpc_function_handle_init(&handles[i], hosts[i],
            func_name);
        if (result != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }      
    }
    /* Asynchronous call */
    for (i = 0; i < n; i++) {
        result = grpc_call_async(&handles[i], &ids[i], DEF_SCALAR, param1,
            param2, param3 + (i * DEF_SCALAR * DEF_SCALAR));
        if (result != GRPC_NO_ERROR){
            fprintf(stderr, "grpc_call_async() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }
    }

    for (k = 0; k < n; k++) {
        /* Wait any session */
        result = grpc_wait_any(&id);
        if (result != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_wait_any() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }

        fprintf(stdout, "got Session ID = %d\n", id);
	for (index = 0; index < n; index++) {
            if (ids[index] == id) {
                break;
            }
        }
        fprintf(stdout, "index = %d, ID = %d\n", index, id);

        base = index * DEF_SCALAR * DEF_SCALAR;
	for (i = 0; i < DEF_SCALAR; i++) {
            for (j = 0; j < DEF_SCALAR; j++) {
                fprintf(stdout, "%f ",
                    param3[base + (i * DEF_SCALAR) + j]);
            }
            fprintf(stdout, "\n");
        }
        fprintf(stdout, "\n");
    }

    /* Destruct Function handles */
    for (i = 0; i < n; i++) {
        result = grpc_function_handle_destruct(&handles[i]);
        if (result != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }
    }

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    for (i = 0; i < n; i++) {
        free(hosts[i]);
    }
    free(hosts);
    free(handles);
    free(ids);
    free(param3);

    return 0;
}
