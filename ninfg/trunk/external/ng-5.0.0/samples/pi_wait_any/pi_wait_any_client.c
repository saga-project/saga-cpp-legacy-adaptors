/*
 * $RCSfile: pi_wait_any_client.c,v $ $Revision: 1.4 $ $Date: 2007/09/26 04:14:03 $
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
#include <assert.h>
#include <errno.h>

char *func_name   = "pi/pi_trial";
char *config_file = "client.conf";

/*
 * Arguments
 */
typedef struct arguments_s {
    long times;
    long rpc_times;
    int  nhosts;
    char **hosts;
} arguments_t;

/*
 * RPCs information
 */
typedef struct RPCs_information_s {
    long times;
    long rpc_times;
    long nstarted;
    long nfinished;
} RPCs_information_t;

/*
 * Information of each hosts
 */
typedef struct host_information_s {
    char                   *hostname;
    grpc_function_handle_t handle;
    int                    handle_initialized;
    int                    available;
    long                   last_rpc;

    long                   npoints;/* Result of RPC */
} host_information_t;

typedef struct host_informations_s{
    host_information_t *informations;
    int                nhosts;
    int                navailable;
} host_informations_t;

/* Prototypes */
static int analyze_argument(int, char *[], arguments_t *);
static int grpc_process(host_informations_t *, RPCs_information_t *);
static void call_async_if_available(host_information_t *,
    host_informations_t *, RPCs_information_t *);
static host_information_t *find_host_information(
    host_informations_t *, grpc_sessionid_t);
static int string_to_positive_long(const char *, long *);

int
main(int argc, char *argv[])
{
    arguments_t arg;
    host_informations_t host_informations;
    host_information_t *info = NULL;
    RPCs_information_t rpc_info;
    int result;
    int i;

    result = analyze_argument(argc, argv, &arg);
    if (result == 0) {
        fprintf(stderr, "USAGE: %s TIMES RPC_TIMES HOSTNAME1 HOSTNAME2...\n",
            argv[0]);
        exit(2);
    }

    printf("pi calculation by %ld points x %ld RPCs by %d hosts.\n",
        arg.times, arg.rpc_times, arg.nhosts);

    /* Initialize Host informations */
    host_informations.nhosts     = arg.nhosts;
    host_informations.navailable = 0;

    host_informations.informations = 
        (host_information_t *)calloc(
            host_informations.nhosts, sizeof(host_information_t));
    if (host_informations.informations == NULL) {
        fprintf(stderr, "malloc: Can't allocate the storage for"
            " RPC informations:%s\n", strerror(errno));
        exit(2);
    }
    for (i = 0; i < host_informations.nhosts; i++) {
        info = &host_informations.informations[i];
        info->hostname           = arg.hosts[i];
        info->handle_initialized = 0;/* False */
        info->available          = 0;/* False */
        info->last_rpc           = 0;
        info->npoints            = 0;
    }

    /* Initialize RPCs informations */
    rpc_info.times     = arg.times;
    rpc_info.rpc_times = arg.rpc_times;
    rpc_info.nstarted  = 0;
    rpc_info.nfinished = 0;

    result = grpc_process(&host_informations, &rpc_info);
    if (result == 0) {
        exit(2);
    }

    /* Finalize data */
    free(host_informations.informations);
    host_informations.informations = NULL;

    return EXIT_SUCCESS;
}

static int
analyze_argument(
    int argc,
    char *argv[],
    arguments_t *out)
{
    int result;

    assert(argc >= 0);
    assert(argv != NULL);
    assert(out  != NULL);

    out->times     = 0;
    out->rpc_times = 0;
    out->nhosts    = 0;
    out->hosts     = NULL;

    if (argc < 4){
	fprintf(stderr, "few arguments.\n");
        goto error;
    }

    /* Get times */
    result = string_to_positive_long(argv[1], &out->times);
    if (result == 0) {
	fprintf(stderr, "%s: Invalid TIMES.\n", argv[1]);
        goto error;
    }

    /* Get RPC times */
    result = string_to_positive_long(argv[2], &out->rpc_times);
    if (result == 0) {
	fprintf(stderr, "%s: Invalid RPC_TIMES.\n", argv[2]);
        goto error;
    }

    /* argc - 1(program name) - 1(times) - 1(rpc_times)*/
    out->nhosts = argc - 3;
    out->hosts  = &argv[3];

    /* Success */
    return 1;

    /* Error occurred */
error:

    /* Failed */
    return 0;
}

/*
 * the process from grpc_initialize() to grpc_finalize().
 */
static int
grpc_process(
    host_informations_t *his,
    RPCs_information_t *rpc_info)
{
    grpc_sessionid_t sid;
    grpc_error_t result = GRPC_NO_ERROR;
    host_information_t *info = NULL;
    long sum = 0;
    int i;
    double pi = 0.0;

    assert(his      != NULL);
    assert(rpc_info != NULL);
    
    /* Initialize GRPC */
    result = grpc_initialize(config_file);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_initialize() error. (%s)\n",
            grpc_error_string(result));
        goto error;
    }

    /* Initialize Function handles */
    for (i = 0; i < his->nhosts; i++) {
        info = &his->informations[i];
        printf("handle %2d: %-20s : init.\n", i, info->hostname);

        result = grpc_function_handle_init(
            &info->handle, info->hostname, func_name);
        if (result == GRPC_NO_ERROR) {
            /* Success */
            info->handle_initialized = 1; /* True */
            info->available          = 1; /* True */
            his->navailable++;
        } else {
            /* Failed */
            fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
                grpc_error_string(result));
        }
    }

    if (his->navailable == 0) {
        fprintf(stderr, "All available function handles are lost.\n");
        goto error;
    }

    /* First GRPC Call */
    for (i = 0;
        (rpc_info->nstarted < rpc_info->rpc_times) && (i < his->nhosts);
        i++) {
        call_async_if_available(&his->informations[i], his, rpc_info);
        if (his->navailable == 0) {
            fprintf(stderr, "All available function handles are lost.\n");
            goto error;
        }
    }

    for (;;) {
        /* Wait */
        result = grpc_wait_any(&sid);
        if (result != GRPC_NO_ERROR){
            fprintf(stderr, "grpc_wait_any() error. (%s)\n",
                grpc_error_string(result));
            goto error;
        }
        if (sid == GRPC_SESSIONID_VOID) {
            /* No sessions */
            break;
        }
        info = find_host_information(his, sid);
        if (info == NULL) {
            /* Not Found */
            fprintf(stderr, "Invalid session id\n");
            goto error;
        }
        rpc_info->nfinished++;

        /* Add result */
        sum += info->npoints;

        pi = 4.0 * (sum / ((double) rpc_info->times * rpc_info->nfinished));
        printf("RPC %3ld %-20s : finish. (%8ld/%8ld) %f\n",
            info->last_rpc, info->hostname,
            info->npoints, rpc_info->times, pi);

        if (rpc_info->nstarted < rpc_info->rpc_times) {
            assert(info->available != 0);

            call_async_if_available(info, his, rpc_info);
            if (his->navailable == 0) {
                fprintf(stderr, "All available function handles are lost.\n");
                goto error;
            }
        }
    }

    /* Destruct */
    for (i = 0; i < his->nhosts; i++) {
        info = &his->informations[i];
        if (info->handle_initialized != 0) {
            printf("handle %2d: %-20s : destruct.\n", i, info->hostname);
            result = grpc_function_handle_destruct(&info->handle);
            if (result != GRPC_NO_ERROR) {
                fprintf(stderr, "grpc_function_handle_destruct() error. (%s)\n",
                    grpc_error_string(result));
            }
            info->handle_initialized = 0;
            if (info->available != 0) {
                info->available = 0; /* False */
                his->navailable--;
            }
        }
    }
    assert(his->navailable == 0);

     /* Compute and display pi. */
    pi = 4.0 * (sum / ((double) rpc_info->times * rpc_info->rpc_times));
    printf("PI = %f\n", pi);

    /* Finalize */
    result = grpc_finalize();
    if (result != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_finalize() error. (%s)\n",
            grpc_error_string(result));
        goto error;
    }

    /* Success */
    return 1;

    /* Error occurred */
error:
    /* Failed */
    return 0;
}

/**
 * Find RPC information from session id
 */
static host_information_t *
find_host_information(
    host_informations_t *his,
    grpc_sessionid_t sid)
{
    grpc_function_handle_t *handle;
    int result;
    int i;

    assert(his != NULL);
    assert(his->informations != NULL);
    assert(sid != GRPC_SESSIONID_VOID);

    result = grpc_function_handle_get_from_session_np(&handle, sid);
    if (result != GRPC_NO_ERROR){
        fprintf(stderr, "grpc_function_handle_get_from_session_np()"
            " error. (%s)\n", grpc_error_string(result));
        return NULL;
    }

    for (i = 0;i < his->nhosts;i++) {
        if (&his->informations[i].handle == handle) {
            /* Found */
            return &his->informations[i];
        }
    }
    
    /* Not Found */
    return NULL;
}

/*
 * Call async if handle is available.
 */
static void
call_async_if_available(
    host_information_t *info,
    host_informations_t *his,
    RPCs_information_t *rpc_info)
{
    grpc_error_t result;
    grpc_sessionid_t sid;

    if (info->available != 0) {
        result = grpc_call_async(&info->handle, &sid,
            rpc_info->nstarted/* Seed of random number */,
            rpc_info->times, &info->npoints);
        if (result == GRPC_NO_ERROR) {
            /* Success */
            rpc_info->nstarted++;
            info->last_rpc = rpc_info->nstarted;
            printf("RPC %3ld %-20s : start.\n",
                rpc_info->nstarted, info->hostname);
        } else {
            /* Failed */
            fprintf(stderr, "grpc_function_handle_init() error. (%s)\n",
                grpc_error_string(result));
            info->available = 0;
            his->navailable--;
        }
    }
    return;
}

/*
 * Conversion from string to positive long integer
 */
static int 
string_to_positive_long(
    const char *in,
    long       *out)
{
    char *endp = NULL;
    long value = 0;

    assert(in  != NULL);
    assert(out != NULL);

    /* Get times */
    errno = 0;
    value = strtol(in, &endp, 0);
    if (errno != 0) {
        goto error;
    }
    if ((endp == in) || (*endp != '\0') || (value < 0)) {
        goto error;
    }
    *out = value;

    /* Success */
    return 1;

error:
    /* Failed */
    return 0;
}
