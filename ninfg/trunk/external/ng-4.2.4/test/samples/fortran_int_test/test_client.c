#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: test_client.c,v $ $Revision: 1.1 $ $Date: 2007/05/29 06:47:20 $";
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

#include "grpc.h"
#include <time.h>

char *func_name = "mpi_test/mpi_test_int";
char *config_file = "client.conf";
int port = 0;

int
main(int argc, char *argv[])
{
    grpc_function_handle_t *handle;
    grpc_sessionid_t *id = NULL;
    grpc_error_t result = GRPC_NO_ERROR;
    char *host = NULL;
    int n;
    int scalarIn;
    int scalarOut;
    int *arrayIn;
    int *arrayOut;

	int i;
	int mismatchCount = 0;

    if (argc < 3){
        fprintf(stderr, "USAGE: %s HOSTNAME No_of_CPUs\n", argv[0]);
        exit(2);
    }

    host = strdup(argv[1]);
    n = atoi(argv[2]);

    /* prepare arg data */
	arrayIn = (int*)malloc(sizeof(int)*n);
	arrayOut = (int*)malloc(sizeof(int)*n);

	scalarIn = 1;
	for (i = 0; i < n; i++){
		arrayIn[i] = i;
	}
    
   handle = (grpc_function_handle_t *)malloc(sizeof(grpc_function_handle_t));
   if (handle == NULL) {
       fprintf(stderr, "malloc: Can't allocate the storage for handles.\n");
       exit(2);
   }

   id = (grpc_sessionid_t *)malloc(sizeof(grpc_sessionid_t));
   if (id == NULL) {
       fprintf(stderr, "malloc: Can't allocate the storage.\n");
       exit(2);
   }

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handles */
     result = grpc_function_handle_init(handle, host, func_name);
     if (result != GRPC_NO_ERROR) {
         fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
                grpc_error_string(result));
         exit(2);
     }      

printf("async call start: %s\n", host); 

    /* Asynchronous call */
	result = grpc_call_async(handle, id, n, scalarIn, &scalarOut, arrayIn, arrayOut);
	if (result != GRPC_NO_ERROR){
		fprintf(stderr, "grpc_call_async() error. (%s)\n",
				grpc_error_string(result));
		exit(2);
	}

    /* Asynchronous call wait*/
    result = grpc_wait_all();
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_wait_all() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

printf("wait_all end\n");

    /* Destruct Function handles */
	result = grpc_function_handle_destruct(handle);
	if (result != GRPC_NO_ERROR) {
		fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(result));
		exit(2);
	}

printf("destruct handle end\n");

    /* Check Result */
	if(scalarIn != scalarOut){
		printf("Result is NOT correct: scalarIn(%d) != scalarOut(%d)\n", scalarIn, scalarOut);
	}
	else{
		printf("Result is correct: scalarIn(%d) == scalarOut(%d)\n", scalarIn, scalarOut);
	}
		
	for (i = 0; i < n; i++){
		if(arrayIn[i] != arrayOut[i]){
			printf("Result is NOT correct: arrayIn(%d) != arrayOut(%d) at %d\n", 
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

    free(host);
    free(handle);
    free(id);

    return 0;
}
