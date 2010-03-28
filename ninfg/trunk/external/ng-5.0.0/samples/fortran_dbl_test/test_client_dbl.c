/*
 * $RCSfile: test_client_dbl.c,v $ $Revision: 1.4 $ $Date: 2008/02/27 10:03:11 $
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

#include <time.h>

char *func_name = "mpi_test/mpi_test_dbl";
char *config_file = "client.conf";

int
main(int argc, char *argv[])
{
    grpc_function_handle_t handle;
    grpc_error_t result = GRPC_NO_ERROR;
    grpc_handle_attr_t_np attr;
    char *host = NULL;
    int n;
    double scalarIn;
    double scalarOut;
    double *arrayIn;
    double *arrayOut;
    int i;
    int mismatchCount = 0;

    if (argc < 3){
        fprintf(stderr, "USAGE: %s HOSTNAME No_of_CPUs\n", argv[0]);
        exit(2);
    }

    host = argv[1];
    n = atoi(argv[2]);

    /* prepare arg data */
    arrayIn = (double *)malloc(sizeof(double)*n);
    if (arrayIn == NULL) {
        perror("malloc");
        exit(2);
    }
    arrayOut = (double *)malloc(sizeof(double)*n);
    if (arrayOut == NULL) {
        perror("malloc");
        exit(2);
    }

    scalarIn = 1;
    for (i = 0; i < n; i++){
        arrayIn[i] = (double)i;
    }

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
        grpc_error_string(result));
        exit(2);
    }
    result = grpc_handle_attr_initialize_np(&attr);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_handle_attr_initialize_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }      
    result = grpc_handle_attr_set_np(&attr, GRPC_HANDLE_ATTR_MPI_NCPUS, &n);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_handle_attr_set_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }      
    result = grpc_handle_attr_set_np(&attr, GRPC_HANDLE_ATTR_HOSTNAME, host);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_handle_attr_set_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }      
    result = grpc_handle_attr_set_np(&attr, GRPC_HANDLE_ATTR_FUNCNAME, func_name);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_handle_attr_set_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handles */
    result = grpc_function_handle_init_with_attr_np(&handle, &attr);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }      
    result = grpc_handle_attr_destruct_np(&attr);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_handle_attr_destruct_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    printf("grpc_call start: %s, number of CPUs is %d\n", host, n); 

    /* Call */
    result = grpc_call(&handle, n, scalarIn, &scalarOut, arrayIn, arrayOut);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    printf("grpc_call end\n");

    /* Destruct Function handles */
    result = grpc_function_handle_destruct(&handle);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    printf("destruct handle end\n");

    /* Check Result */
    if(scalarIn != scalarOut){
        printf("Result is NOT correct: scalarIn(%f) != scalarOut(%f)\n", scalarIn, scalarOut);
    }
    else{
        printf("Result is correct: scalarIn(%f) == scalarOut(%f)\n", scalarIn, scalarOut);
    }

    for (i = 0; i < n; i++) {
        if(arrayIn[i] != arrayOut[i]){
            printf("Result is NOT correct: arrayIn(%f) != arrayOut(%f) at %d\n", 
            arrayIn[i], arrayOut[i], i);
            mismatchCount++;
        }
    }
    if(mismatchCount == 0){
        printf("Result is correct for all array args\n");
    }

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
        grpc_error_string(result));
        exit(2);
    }

    free(arrayIn);
    free(arrayOut);

    return 0;
}
