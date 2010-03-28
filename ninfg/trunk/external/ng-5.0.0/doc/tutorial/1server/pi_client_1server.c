/* This is sample program for Ninf-G, calculate PI on one server */
#include "grpc.h"

char *func_name = "pi/pi_trial";
char *config_file = "client.conf";

int
main(int argc, char *argv[])
{
    grpc_function_handle_t handle;
    grpc_error_t result = GRPC_NO_ERROR;
    char *host = NULL;
    long times, answer;

    if (argc != 3){
        fprintf(stderr, "USAGE: %s TIMES HOSTNAME\n", argv[0]);
        exit(2);
    }
    times = atol(argv[1]);
    host = argv[2];

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handle */
    result = grpc_function_handle_init(&handle, host, func_name);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Synchronous call */
    result = grpc_call(&handle, 0, times, &answer);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_call() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Destruct Function handles */
    result = grpc_function_handle_destruct(&handle);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Compute and display pi. */
    printf("PI = %f\n", 4.0 * ((double)answer / times));

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    return 0;
}
