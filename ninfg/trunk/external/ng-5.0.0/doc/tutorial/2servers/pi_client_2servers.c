/* This is sample program for Ninf-G, calculate PI on multi servers */
#include "grpc.h"

char *func_name = "pi/pi_trial";
char *config_file = "client.conf";

int
main(int argc, char *argv[])
{
    grpc_function_handle_t handles[2];
    grpc_sessionid_t ids[2];
    grpc_error_t result = GRPC_NO_ERROR;
    char *hosts[2];
    long count[2];
    double pi;
    long times, sum;
    int i;

    if (argc != 4){
        fprintf(stderr, "USAGE: %s TIMES HOSTNAME1 HOSTNAME2\n", argv[0]);
        exit(2);
    }

    hosts[0] = NULL;
    hosts[0] = strdup(argv[2]);
    hosts[1] = NULL;
    hosts[1] = strdup(argv[3]);
    if ((hosts[0] == NULL) || (hosts[1] == NULL)) {
      fprintf(stderr,
	      "strdup: Can't allocate the storage for hostname.\n");
      exit(2);
    }

    times = atol(argv[1]) / 2;

    /* Initialize */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    /* Initialize Function handles */
    for (i = 0; i < 2; i++) {
        result = grpc_function_handle_init(&handles[i], hosts[i],
            func_name);
        if (result != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }      
    }

    /* Asynchronous call */
    for (i = 0; i < 2; i++) {
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
    for (i = 0; i < 2; i++) {
        result = grpc_function_handle_destruct(&handles[i]);
        if (result != GRPC_NO_ERROR) {
            fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                grpc_error_string(result));
            exit(2);
        }
    }

    /* Compute and display pi. */
    for (i = 0, sum = 0; i < 2; i++) {
        sum += count[i];
    }
    pi = 4.0 * (sum / ((double) times * 2));
    printf("PI = %f\n", pi);

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(result));
        exit(2);
    }

    for (i = 0; i < 2; i++) {
        free(hosts[i]);
    }

    return 0;
}
