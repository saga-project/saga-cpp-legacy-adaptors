/* This is sample program for Ninf-G, calculate PI with handle_array */
#include "grpc.h"

char *func_name = "pi/pi_trial";
char *config_file = "client.conf";

int
main(int argc, char *argv[])
{
    grpc_function_handle_t *handles;
    grpc_sessionid_t *ids = NULL;
    grpc_error_t result = GRPC_NO_ERROR;
    char *host = NULL;
    double pi;
    long *count = NULL;
    long times, sum;
    int n;
    int i;

    if (argc != 4){
        fprintf(stderr, "USAGE: %s TIMES HOSTNAME n\n", argv[0]);
        exit(2);
    }

    times = atol(argv[1]);
    host = argv[2];
    n = atoi(argv[3]);

    times = times / n;

    handles = (grpc_function_handle_t *)malloc(
        sizeof(grpc_function_handle_t) * n);
    if (handles == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage for handles.\n");
        exit(2);
    }

    count = (long *)malloc(sizeof(long) * n);
    if (count == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage.\n");
        exit(2);
    }

    ids = (grpc_sessionid_t *)malloc(sizeof(grpc_sessionid_t) * n);
    if (ids == NULL) {
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
    result = grpc_function_handle_array_init_np(handles, n, host,
        func_name);
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_array_init_np() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Asynchronous call */
    for (i = 0; i < n; i++) {
        result = grpc_call_async(&handles[i], &ids[i], i, times, &count[i]);
        if (result != GRPC_NO_ERROR){
            fprintf(stderr, "grpc_call_async() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }
    }

    /* wait for session */
    result = grpc_wait_all();
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_wait_all() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
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

    /* Compute and display pi. */
    for (i = 0, sum = 0; i < n; i++) {
        sum += count[i];
    }
    pi = 4.0 * (sum / ((double) times * n));
    printf("PI = %f\n", pi);

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    free(handles);
    free(count);
    free(ids);

    return 0;
}
