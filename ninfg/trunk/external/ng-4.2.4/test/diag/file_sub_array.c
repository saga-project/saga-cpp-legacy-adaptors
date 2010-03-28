#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: file_sub_array.c,v $ $Revision: 1.3 $ $Date: 2007/05/16 09:59:42 $";
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "grpc.h"

#define MODULE_NAME "file"
int destroy_file_names(int, char ***, char ***, char ***);
int create_file_names(int, char *, char *, char *,
    char ***, char ***, char ***);
int check_files(int, char **, char **);


static grpc_function_handle_t handles;

/*
 * Copy from inout-file to out-file and from in-file to inout-file.
 */
int
main(int argc, char *argv[])
{
    grpc_error_t ret;
    int result;
    char *conf, *array_size_str, *end;
    int array_size;
    char **in_file_array, **inout_file_array, **out_file_array;
    char *in_file, *inout_file, *out_file;

    if (argc < 6) {
        fprintf(stderr, "Usage: %s config count in-file inout-file out-file\n",
            argv[0]);
        exit(2);
    }

    conf = argv[1];
    array_size_str = argv[2];
    in_file = argv[3];
    inout_file = argv[4];
    out_file = argv[5];

    ret = grpc_initialize(conf);
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr,
            "grpc_initialize() error. (%s)\n", grpc_error_string(ret));
        exit(2);
    }

    array_size = strtol(array_size_str, &end, 0);
    if (*end != '\0') {
        fprintf(stderr, "Argument \"%s\" is not digit.\n", array_size_str);
        exit(1);
    } else if (array_size < 1){
        fprintf(stderr, "array_size %d < 1 (min)\n", array_size);
        exit(1);
    }

    result = create_file_names(array_size, in_file, inout_file, out_file,
        &in_file_array, &inout_file_array, &out_file_array);
    if (result == 0) {
        fprintf(stderr, "creating filenames failed\n");
        exit(1);
    }

    result = check_files(array_size, in_file_array, inout_file_array);
    if (result == 0) {
        fprintf(stderr, "files are not prepared. no test performed.\n");
        exit(1);
    }

    ret = grpc_function_handle_default(
        &handles, MODULE_NAME "/filename_array_test");
    if (ret != GRPC_NO_ERROR) {
        fprintf(stderr, "grpc_function_handle_default() error. (%s)\n",
            grpc_error_string(ret));
        exit(2);
    }

    ret = grpc_call(&handles,
        array_size, in_file_array, inout_file_array, out_file_array);
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

    result = destroy_file_names(array_size, &in_file_array, &inout_file_array, &out_file_array);
    if (result == 0) {
        fprintf(stderr, "destroying filenames failed\n");
        exit(1);
    }
 
    return 0;
}

int
create_file_names(
    int array_size,
    char *in_file,
    char *inout_file,
    char *out_file,
    char ***in_file_array,
    char ***inout_file_array,
    char ***out_file_array)
{
    int file_type_max, file_type_i, i, name_size;
    char *name_str;
    char *file_type[3];
    char **file_name_table[3];

    assert(in_file != NULL);
    assert(inout_file != NULL);
    assert(out_file != NULL);
    assert(in_file_array != NULL);
    assert(inout_file_array != NULL);
    assert(out_file_array != NULL);

    file_type_max = 3;
    file_type[0] = in_file;
    file_type[1] = inout_file;
    file_type[2] = out_file;

    for (file_type_i = 0; file_type_i < file_type_max; file_type_i++) {

        file_name_table[file_type_i] =
            (char **)malloc(sizeof(char *) * array_size);
        if (file_name_table[file_type_i] == NULL) {
            fprintf(stderr, "malloc() failed.\n");
            exit(3);
        }

        name_size = strlen(file_type[file_type_i]) + 8; /* for suffix numbers */

        for (i = 0; i < array_size; i++) {
            name_str = (char *)malloc(sizeof(char) * name_size);
            if (name_str == NULL) {
                fprintf(stderr, "malloc() failed.\n");
                exit(3);
            }
     
            snprintf(name_str, name_size, "%s-%03d",
                file_type[file_type_i], i);
            (file_name_table[file_type_i])[i] = name_str;
        }
    }

    *in_file_array    = file_name_table[0];
    *inout_file_array = file_name_table[1];
    *out_file_array   = file_name_table[2];

    return 1;
}

int
check_files(
    int array_size,
    char **in_file_array,
    char **inout_file_array)
{
    int result, i;

    for (i = 0; i < array_size; i++) {
        result = access(in_file_array[i], R_OK);
        if (result != 0) {
            fprintf(stderr, "file %s read failed.\n", in_file_array[i]);
            exit(3);
        }

        result = access(inout_file_array[i], R_OK);
        if (result != 0) {
            fprintf(stderr, "file %s read failed.\n", inout_file_array[i]);
            exit(3);
        }
    }

    return 1;
}


int
destroy_file_names(
    int array_size,
    char ***in_file_array,
    char ***inout_file_array,
    char ***out_file_array)
{
    char **file_name_table[3];
    int i;
    int j;

    i = 0;
    file_name_table[i++] = *in_file_array;
    file_name_table[i++] = *inout_file_array;
    file_name_table[i++] = *out_file_array;

    for (i = 0;i < 3;++i) {
        for (j = 0; j < array_size; j++) {
            free(file_name_table[i][j]);
            file_name_table[i][j] = NULL;
        }
        free(file_name_table[i]);
    }

    *in_file_array    = NULL;
    *inout_file_array = NULL;
    *out_file_array   = NULL;

    return 1;
}
