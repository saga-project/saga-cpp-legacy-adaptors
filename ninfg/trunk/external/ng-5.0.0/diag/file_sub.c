/*
 * $RCSfile: file_sub.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:02 $
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
#include <stdlib.h>

#define MODULE_NAME "file"

static grpc_function_handle_t handles;

/*
 * Copy from inout-file to out-file and from in-file to inout-file.
 */
int
main(int argc, char *argv[])
{
    grpc_error_t ret;

    if (argc < 5) {
        fprintf(stderr, "Usage: %s config in-file inout-file out-file\n",
            argv[0]);
        exit(2);
    }

    ret = grpc_initialize(argv[1]);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(ret));
        exit(2);
    }

    ret = grpc_function_handle_default(&handles, MODULE_NAME "/filename_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        exit(2);
    }

    ret = grpc_call(&handles, argv[2], argv[3], argv[4]);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(ret));
        exit(2);
    }

    ret = grpc_function_handle_destruct(&handles);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(ret));
        exit(2);
    }

    ret = grpc_finalize();
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(ret));
        exit(2);
    }
 
    return 0;
}
