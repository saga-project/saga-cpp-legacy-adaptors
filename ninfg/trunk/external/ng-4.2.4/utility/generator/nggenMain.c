#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: nggenMain.c,v $ $Revision: 1.13 $ $Date: 2006/09/22 06:54:37 $";
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
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include "ngGenerator.h"

#define FILE_NAME_LEN 125
char title_file_name[FILE_NAME_LEN];
#ifndef CPP_FILE
#define CPP_FILE "gcc -xc -E"
#endif

static void nggen_check_nerrors(void);
static void nggen_print_usage(void);
static FILE *nggen_cpp_open(char *, char *, char *, int, int *);
static int nggen_cpp_tmpfile_register(char *);
static int nggen_cpp_tmpfile_unregister(void);
static void nggen_cpp_tmpfile_atexit(void);

static char cpp_command[NG_BUFSIZ];
static char cpp_options[NG_BUFSIZ];
static char cpp_tmpfile[NG_BUFSIZ];
int cpp_ignore = 0;
int gmake_g = 0;
int debug_flag_g = FALSE;
FILE *debug_fp_g;
int nerrors_g;
int warning_flag_g = FALSE; 
char *source_file_name = NULL;
FILE *source_file;
int source_file_popened = 0;

char *program = NULL;

/**/
/*---------------------------------------------------------------------------*/
/* ID     = main                                                             */
/*---------------------------------------------------------------------------*/
int 
main(
    int argc, 
    char *argv[]
)
{
    struct stub_gen_entry *curr_stub = NULL, *prev_stub = NULL;
    int nread, size_of_file, result;
    char read_buffer[NG_BUFSIZ];

    nread = 0;
    size_of_file = 0;
    cpp_command[0] = '\0';
    cpp_options[0] = '\0';
    cpp_tmpfile[0] = '\0';

    program = argv[0];
    --argc;
    ++argv;
    snprintf(cpp_command, NG_BUFSIZ, "%s", CPP_FILE);
    /*====================================
      input arg check
      ====================================*/
    while ((argc > 0) && (argv[0][0] == '-')) { /* parse command line      */

        if (strcmp(argv[0], "-d") == 0) { /* debug */
            ++debug_flag_g;

        } else if (strcmp(argv[0], "-g") == 0) { /* GNU make */
            gmake_g = 1;

        } else if (strcmp(argv[0], "--no-cpp") == 0) { /* No cpp */
            cpp_ignore = 1;

        } else if (strncmp(argv[0],
            "--with-cpp=", strlen("--with-cpp=")) == 0) { /* With cpp */
            snprintf(cpp_command, NG_BUFSIZ, "%s",
                argv[0] + strlen("--with-cpp="));

        } else if (
            (strcmp(argv[0], "-h") == 0) ||
            (strcmp(argv[0], "-help") == 0) ||
            (strcmp(argv[0], "--help") == 0)) { /* help */
            nggen_print_usage();
            exit(0);

        } else {
	    /* unrecognized option, pass to cpp */
            strncat(cpp_options, argv[0],
                NG_BUFSIZ - strlen(cpp_options) - 1);
            strncat(cpp_options, " ",
                NG_BUFSIZ - strlen(cpp_options) - 1);
        }

        /* move to next option */
        --argc;
        ++argv;
    }

    if (argc > 1) {
        fprintf(stderr, "Error: Too many arguments.\n");
        nggen_print_usage();
        exit(1);
    }

    if (argc <= 0) {
        fprintf(stderr, "Error: The input IDL file was not specified.\n");
        nggen_print_usage();
        exit(1);
    }

    /*====================================
      initialize stub entries
      ====================================*/
    stubs_head = NULL;
    stubs_tail = NULL;

    /*====================================
      input file setup
      ====================================*/
    source_file_name = argv[0];

    result = atexit(nggen_cpp_tmpfile_atexit);
    if (result != 0) {
        fprintf(stderr, "Error: atexit() failed.\n");
        exit(1);
    }

    /* Get the size of input */
    source_file = nggen_cpp_open(
        cpp_command, cpp_options, source_file_name,
        cpp_ignore, &source_file_popened);
    if (source_file == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", source_file_name);
        exit(1);
    }
    while ((nread =
        fread(read_buffer, 1, sizeof(read_buffer), source_file)) != 0) {
        /* append size of the file */
        size_of_file += nread;
    }
    if (source_file_popened != 0) {
        pclose(source_file);
    } else {
        fclose(source_file);
    }
    result = nggen_cpp_tmpfile_unregister();
    if (result == 0) {
        fprintf(stderr, "Error: cannot remove temporary file.\n");
        exit(1);
    }
    
    /* set size of buffer */
    nggen_set_lexbuffer_size(size_of_file);

    source_file = nggen_cpp_open(
        cpp_command, cpp_options, source_file_name,
        cpp_ignore, &source_file_popened);
    if (source_file == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", source_file_name);
        exit(1);
    }
    strcpy(title_file_name,source_file_name); /* save the filename    */

    /* DEBUG */ 
    debug_fp_g = stdout;

    nggen_init_expr_code_info();
    nggen_initialize_lex();
    /*================================
            parse
      ================================*/
    yyparse();

    if ( debug_flag_g ) { /* debug ON                      */
        nggen_dump_stubs();             
    }

    if ( !nerrors_g ) { /*  no error */
        /*=========================
	      generate stub
          =========================*/
        for ( curr_stub = stubs_head; curr_stub != NULL;
          curr_stub = curr_stub->next_stub ) { /* generate stub programs     */
	    nggen_stub_program(curr_stub);
        }
        /*=========================
	     generate make file
          =========================*/
        nggen_stubs_makefile();         /* generate make file                */
    }

    /* release information about stub */
    for ( curr_stub = stubs_head; curr_stub != NULL; ) {
      int i;

      for ( i = 0; i < curr_stub->n_methods; i++ ) {
        int j;
        struct method_gen_desc *curr_method = &curr_stub->methods[i];

        for ( j = 0; j < curr_method->nparam; j++ ) {
            int k;

            /* release information about dim */
            for ( k = 0; k < curr_method->params[j].ndim; k++ ) {
                free(curr_method->params[j].dim[k].size_exp.type);
                free(curr_method->params[j].dim[k].size_exp.val);
                free(curr_method->params[j].dim[k].start_exp.type);
                free(curr_method->params[j].dim[k].start_exp.val);
                free(curr_method->params[j].dim[k].end_exp.type);
                free(curr_method->params[j].dim[k].end_exp.val);
                free(curr_method->params[j].dim[k].step_exp.type);
                free(curr_method->params[j].dim[k].step_exp.val);
            }
	    free(curr_method->params[j].dim);
        }

        /* release information about param */
        free(curr_method->params);
      }

      /* release information about method */
      free(curr_stub->methods);
      free(curr_stub->status);

      prev_stub = curr_stub;
      curr_stub = curr_stub->next_stub;
      free(prev_stub);
    }

    result = nggen_cpp_tmpfile_unregister();
    if (result == 0) {
        fprintf(stderr, "Error: cannot remove temporary file.\n");
        exit(1);
    }

    return(nerrors_g ? 1 : 0);
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpp_open                                                   */
/*---------------------------------------------------------------------------*/
static FILE *
nggen_cpp_open(
    char *cpp_command_str,
    char *cpp_options_str,
    char *cpp_target,
    int cpp_not_use,
    int *by_popen
)
{
    char cpp_execute[NG_BUFSIZ];
    char cpp_filename_argument[NG_BUFSIZ];
    FILE *return_fp;
    int cpp_idl_rename;
    int result;

    /* Check the arguments */
    assert(cpp_command_str != NULL);
    assert(cpp_options_str != NULL);
    assert(cpp_target != NULL);
    assert(by_popen != NULL);

    return_fp = NULL;
    cpp_idl_rename = 0;
    cpp_execute[0] = '\0';
    cpp_filename_argument[0] = '\0';
    *by_popen = 0;

    if (cpp_not_use != 0) {
        return_fp = fopen(cpp_target, "r");
        *by_popen = 0;
        return return_fp;
    }

#ifdef NGI_NO_CPP_ACCEPT_IDL_FILE
    cpp_idl_rename = 1;
#endif /* NGI_NO_CPP_ACCEPT_IDL_FILE */

    if (cpp_idl_rename == 0) {
        snprintf(cpp_filename_argument, NG_BUFSIZ,
            "%s", cpp_target);
    } else {
        /* Convert target IDL file suffix from .idl to .c */
        snprintf(cpp_filename_argument, NG_BUFSIZ,
            "%s-ng_gen-tmp-%05ld.c", cpp_target, (long)getpid());

        result = symlink(cpp_target, cpp_filename_argument);
        if (result != 0) {
            fprintf(stderr,
                "Error: symlink \"%s\" to \"%s\" failed: %s.\n",
                cpp_target, cpp_filename_argument, strerror(errno));
            return NULL;
        }

        result = nggen_cpp_tmpfile_register(cpp_filename_argument);
        if (result == 0) {
            fprintf(stderr,
                "Error: cannot register temporary file \"%s\".\n",
                cpp_filename_argument);
            return NULL;
        }
    }

    snprintf(cpp_execute, NG_BUFSIZ,
        "%s %s %s", cpp_command_str, cpp_options_str, cpp_filename_argument);

    return_fp = popen(cpp_execute, "r");
    if (return_fp == NULL) {
        fprintf(stderr,
            "Error: popen \"%s\" failed: %s.\n",
            cpp_execute, strerror(errno));
        /* Not return */
    }
    *by_popen = 1;

    return return_fp;
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpp_tmpfile_register                                       */
/*---------------------------------------------------------------------------*/
static int
nggen_cpp_tmpfile_register(
    char *newfile
)
{
    if (cpp_tmpfile[0] != '\0') {
        fprintf(stderr,
            "Error: temporary file \"%s\" already registered.\n",
            cpp_tmpfile);
        return 0;
    }

    strncpy(cpp_tmpfile, newfile, NG_BUFSIZ); 

    return 1;
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpp_tmpfile_unregister                                     */
/*---------------------------------------------------------------------------*/
static int
nggen_cpp_tmpfile_unregister(
    void
)
{
    int result;

    if (cpp_tmpfile[0] != '\0') {
        result = unlink(cpp_tmpfile);
        if (result != 0) {
            fprintf(stderr,
                "Error: unlink \"%s\" failed: %s.\n",
                cpp_tmpfile, strerror(errno));
            return 0;
        }
        cpp_tmpfile[0] = '\0';
    }

    return 1;
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpp_tmpfile_atexit                                         */
/*---------------------------------------------------------------------------*/
static void
nggen_cpp_tmpfile_atexit(
    void
)
{
    nggen_cpp_tmpfile_unregister();
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_where                                                      */
/*---------------------------------------------------------------------------*/
void 
nggen_where(
    int line_no
)
{ 
    /* print location of error  */
    line_no--;                          /*                 1999.10.22 INSERT */
    fprintf(stderr, "\"%s\", line %d: ", title_file_name, line_no);
}


/*---------------------------------------------------------------------------*/
/* ID     = nggen_error                                                      */
/*---------------------------------------------------------------------------*/
void 
nggen_error(
    char *fmt, ...
) 
{ 
    va_list args;                       

    ++nerrors_g;                        
    nggen_where(lineno_g);              
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);        
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);                     
    nggen_check_nerrors();              
}


/*---------------------------------------------------------------------------*/
/* ID     = nggen_error_at_node                                              */
/*---------------------------------------------------------------------------*/
void 
nggen_error_at_node(
    expr x, 
    char *fmt, ...
) 
{ 
    va_list args;                       

    ++nerrors_g;                        /* update(+1) to nerrors_g           */
    va_start(args, fmt);
    if (x != NULL) {
        nggen_where((int)EXPR_LINENO(x));   /* , "ErrorAtNode");             */
    }
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n" );
    fflush(stderr);
    if ( nerrors_g > 30 ) { 
                               /* give the compiler the benefit of the doubt */
        fprintf(stderr, 
            "too many error, cannot recover from earlier errors: goodbye!\n" );
        exit(1);
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_warning_at_node                                            */
/*---------------------------------------------------------------------------*/
void 
nggen_warning_at_node(
    expr x,
    char *fmt, ...
) 
{ 
    va_list args;                       
  
    nggen_where(EXPR_LINENO(x));        
    fprintf(stderr, "warning:");
    va_start(args, fmt);                
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_check_nerrors                                              */
/*---------------------------------------------------------------------------*/
void
nggen_check_nerrors(
    void
)
{
    if ( nerrors_g > 30 ) { /* give the compiler the benefit of the doubt */
        fprintf(stderr, 
            "too many error, cannot recover from earlier errors: goodbye!\n" );
        exit(1);
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_fatal                                                      */
/*---------------------------------------------------------------------------*/
void
nggen_fatal(
    char *fmt, ...
)
{ 
    va_list args;                       
    
    nggen_where(lineno_g);              
    fprintf(stderr, "compiler error: " );
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
    abort();
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_warning                                                    */
/*---------------------------------------------------------------------------*/
void
nggen_warning(
    char *fmt, ...
) 
{ 
    va_list args;                      
    
    if ( warning_flag_g ) {
        return;
    }
    nggen_where(lineno_g);             
    va_start(args, fmt);
    fprintf(stderr, "warning: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

/*---------------------------------------------------------------------------*/
/* ID     = nggen_print_usage                                                */
/*---------------------------------------------------------------------------*/
void
nggen_print_usage(
    void
)
{
    puts("Usage: ng_gen [-d] [-g] [-h] input_file ");
    puts("\t-d Debug option");
    puts("\t-g Produce makefile in gnu make style");
    puts("\t--no-cpp Do not use cpp");
    puts("\t--with-cpp=cpp_command Specify the cpp");
    puts("\t-h Show this message");
    puts("");
}
