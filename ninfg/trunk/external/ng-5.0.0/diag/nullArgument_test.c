/*
 * $RCSfile: nullArgument_test.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:02 $
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
#include <string.h>

#define MODULE_NAME "nullArgument/nullArgument"

static int verbose;

/* Prototype declaration */
static int nullArgument_test(grpc_argument_transfer_t_np);

/*
 * MAIN
 */
int
main(int argc, char *argv[])
{
    grpc_error_t result;
    char *configFile, *programName;
    int ch;
    int ret, exit_code;

    programName = argv[0];

    /* Parse the arguments */
    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    verbose = 1;
	    break;

	default:
	usage:
	    fprintf(stderr, "Usage: %s [-v] config-file\n", programName);
	    exit(2);
	}
    }
    argc -= optind;
    argv += optind;

    /* Save the configuration file name. */
    if (argc < 1) {
	goto usage;
    }
    configFile = argv[0];
    exit_code = 0;

    setbuf(stdout, NULL);

    /* Initialize the Grid RPC */
    result = grpc_initialize(configFile);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    /* Do the tests */
    printf("null argument wait testing:\t");
    ret = nullArgument_test(GRPC_ARGUMENT_TRANSFER_WAIT);
    printf(ret ? "OK\n" : "failed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("null argument nowait testing:\t");
    ret = nullArgument_test(GRPC_ARGUMENT_TRANSFER_NOWAIT);
    printf(ret ? "OK\n" : "failed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    printf("null argument copy testing:\t");
    ret = nullArgument_test(GRPC_ARGUMENT_TRANSFER_COPY);
    printf(ret ? "OK\n" : "failed\n");
    exit_code = ((ret && (exit_code == 0)) ? 0 : 1);

    /* Finalize the Grid RPC */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_finalize() error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    /* Success */
    return exit_code;
}

/*
 * Test the NULL argument.
 */
static int
nullArgument_test(grpc_argument_transfer_t_np argTransfer)
{
    int rpcResult[3];
    grpc_error_t result;
    grpc_object_handle_t_np handle;
    grpc_handle_attr_t_np attribute;
    static char *stringArray2[3] = {NULL, NULL, NULL};

    /* Initialize the attribute */
    result = grpc_handle_attr_initialize_np(&attribute);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_handle_attr_initialize_np error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Set function name to the attribute */
    result = grpc_handle_attr_set_np(&attribute,
        GRPC_HANDLE_ATTR_FUNCNAME, (void *)MODULE_NAME);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_handle_attr_set_np error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Set argument transfer to the attribute */
    result = grpc_handle_attr_set_np(&attribute,
        GRPC_HANDLE_ATTR_WAIT_ARG_TRANSFER, (void *)&argTransfer);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_handle_attr_set_np error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Create the object handle */
    result = grpc_object_handle_init_with_attr_np(&handle, &attribute);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_object_handle_default error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 0;
    result = grpc_invoke_np(&handle, "nullStringInScalar", NULL, &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 1) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 1;
    result = grpc_invoke_np(&handle, "nullStringInScalar", "", &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 0) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 1;
    rpcResult[1] = 0;
    rpcResult[2] = 0;
    stringArray2[0] = NULL;
    stringArray2[1] = "";
    stringArray2[2] = "test";
    result = grpc_invoke_np(&handle, "nullStringInArray",
	3, &stringArray2[0], &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if ((rpcResult[0] != 1) || (rpcResult[1] != 0) || (rpcResult[1] != 0)) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 0;
    result = grpc_invoke_np(&handle, "nullFilenameIN", NULL, &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 1) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 1;
    result = grpc_invoke_np(&handle, "nullFilenameIN", "", &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 0) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 0;
    result = grpc_invoke_np(&handle, "nullFilenameOUT", NULL, &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 1) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    rpcResult[0] = 1;
    result = grpc_invoke_np(&handle, "nullFilenameOUT", "", &rpcResult[0]);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_invoke error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }
    if (rpcResult[0] != 0) {
	fprintf(stderr, "Remote Executable returned the NG.\n");
	exit(2);
    }

    /* Invoke the remote method */
    result = grpc_invoke_np(&handle, "nullArgument", 0,
	/* IN char,   short,  int,    long,   float,  double, */
	      NULL,   NULL,   NULL,   NULL,   NULL,   NULL,

	/* IN scomplex, dcomplex */
	      NULL,     NULL);
    if (result != GRPC_NO_ERROR) {
	fprintf(stderr, "grpc_invoke error. (%s)\n", grpc_error_string(result));
        exit(2);
    }

    /* Destruct the object handle */
    result = grpc_object_handle_destruct_np(&handle);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_object_handle_destruct error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Success */
    return 1;
}
