/*
 * $RCSfile: pi_client.c,v $ $Revision: 1.3 $ $Date: 2008/02/27 10:03:12 $
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

char *func_name = "pi_mpi/pi_trial";
char *config_file = "client.conf";

int
main(int argc, char *argv[])
{
    grpc_error_t result;
    char *host = NULL;
    grpc_function_handle_t handle;
    long times, answer;

    if (argc != 3){
        fprintf(stderr, "USAGE: %s TIMES HOSTNAME\n", argv[0]);
        exit(2);
    }
    times = atol(argv[1]);
    host = argv[2];

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handles */
    result = grpc_function_handle_init(&handle, host, func_name);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Execute the RPC */
    result = grpc_call(&handle, times, &answer);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n", grpc_error_string(result));
    }

    /* Destruct Function handles */
    result = grpc_function_handle_destruct(&handle);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "handle_destruct error. (%s)\n",
	    grpc_error_string(result));
        exit(2);
    }

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
	    grpc_error_string(result));
        exit(2);
    }

    /* Print the result */
    printf("PI = %f\n", 4.0 * ((double)answer / times));

    /* Success */
    return 0;
}
