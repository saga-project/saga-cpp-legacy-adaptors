#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: nggenStub.c,v $ $Revision: 1.52 $ $Date: 2007/11/28 10:50:10 $";
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
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "ngGenerator.h"

#ifndef COMPILER 
#define COMPILER "gcc"
#endif
#define FORTRAN "fortran"
#define C_LANG "c"
#define CXX_LANG "c++"
#define GET_VAL_STRING(i) getCodeString(value_type_string, i)
#define DUMP_INFO_TAG "NGEX_NOT_REMOTE_EXECUTABLE"
#define STUB_INFO_PROGRAM "_stub_%s-info"

static char *nggen_c_type_name(DATA_TYPE);
static char *nggen_mpi_type_name(DATA_TYPE);
static int nggen_need_strlen(char * name, struct method_gen_desc *ep);
static int nggen_need_and(char *name, struct method_gen_desc *ep);
static int nggen_need_and_sub(expr body_expr, DATA_TYPE type,
                          int ndim, ngArgumentIOmode_t param_inout);
static char *nggen_make_function_name(expr body_expr, char *fformat);
static void nggen_print_expression(FILE *, struct method_gen_desc *, expr);

static char function_name_buffer[NG_BUFSIZ];

/* save data type String for expression value, mode, ninf data type */
typedef struct code_type {
    int type;
    char *typeString;
} code_type_t;
#define END_OF_CODE_TYPE -999

/* return string for code for target */
static char *getCodeString(code_type_t codelist[], int type) {
    int code_index = 0;
    while (codelist[code_index].type != END_OF_CODE_TYPE) {
	if (codelist[code_index].type == type) {
	    /* found! */
	    return codelist[code_index].typeString;
	}
	code_index++;
    }
    /* not found */
    return NULL;
}

/* codes for expression value type */
static code_type_t value_type_string[] = {
    {NG_EXPRESSION_VALUE_TYPE_NONE, "NG_EXPRESSION_VALUE_TYPE_NONE"},
    {NG_EXPRESSION_VALUE_TYPE_CONSTANT, "NG_EXPRESSION_VALUE_TYPE_CONSTANT"},
    {NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT, "NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT"},
    {NG_EXPRESSION_VALUE_TYPE_OPCODE, "NG_EXPRESSION_VALUE_TYPE_OPCODE"},
    {NG_EXPRESSION_VALUE_TYPE_END, "NG_EXPRESSION_VALUE_TYPE_END"},
    {END_OF_CODE_TYPE, "BASIC_TYPE_END"},
};

/* codes for value mode */
#define GET_MODE_STRING(i) getCodeString(mode_type_string, i)
static code_type_t mode_type_string[] = {
    {NG_ARGUMENT_IO_MODE_NONE, "NG_ARGUMENT_IO_MODE_NONE"},
    {NG_ARGUMENT_IO_MODE_IN, "NG_ARGUMENT_IO_MODE_IN"},
    {NG_ARGUMENT_IO_MODE_OUT, "NG_ARGUMENT_IO_MODE_OUT"},
    {NG_ARGUMENT_IO_MODE_INOUT, "NG_ARGUMENT_IO_MODE_INOUT"},
    {NG_ARGUMENT_IO_MODE_WORK, "NG_ARGUMENT_IO_MODE_WORK"},
    {END_OF_CODE_TYPE, "BASIC_TYPE_END"},
};

/* codes for data type */
#define GET_DATA_STRING(i) getCodeString(data_type_string, i)
static code_type_t data_type_string[] = {
    {NG_ARGUMENT_DATA_TYPE_CHAR, "NG_ARGUMENT_DATA_TYPE_CHAR"},
    {NG_ARGUMENT_DATA_TYPE_SHORT, "NG_ARGUMENT_DATA_TYPE_SHORT"},
    {NG_ARGUMENT_DATA_TYPE_INT, "NG_ARGUMENT_DATA_TYPE_INT"},
    {NG_ARGUMENT_DATA_TYPE_LONG, "NG_ARGUMENT_DATA_TYPE_LONG"},
    {NG_ARGUMENT_DATA_TYPE_FLOAT, "NG_ARGUMENT_DATA_TYPE_FLOAT"},
    {NG_ARGUMENT_DATA_TYPE_DOUBLE, "NG_ARGUMENT_DATA_TYPE_DOUBLE"},
    {NG_ARGUMENT_DATA_TYPE_SCOMPLEX, "NG_ARGUMENT_DATA_TYPE_SCOMPLEX"},
    {NG_ARGUMENT_DATA_TYPE_DCOMPLEX, "NG_ARGUMENT_DATA_TYPE_DCOMPLEX"},
    {NG_ARGUMENT_DATA_TYPE_STRING, "NG_ARGUMENT_DATA_TYPE_STRING"},
    {NG_ARGUMENT_DATA_TYPE_FILENAME, "NG_ARGUMENT_DATA_TYPE_FILENAME"},
    {NG_ARGUMENT_DATA_TYPE_CALLBACK, "NG_ARGUMENT_DATA_TYPE_CALLBACK"},
    {END_OF_CODE_TYPE, "BASIC_TYPE_END"},
};

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_stubs_makefile                                             */
/*---------------------------------------------------------------------------*/
void
nggen_stubs_makefile(
    void
)
{
    FILE *fp;                            
    char fname[NG_BUFSIZ];      
    int i, j;                            
    char *ent_name;                      
    struct stub_gen_entry *curr;

    if ( current_module ) {
        sprintf(fname, "%s.mak", current_module);
    } else {
        nggen_error("module is not specified, can't create makefile.\n");
        return;
    }
    if ( (fp = fopen(fname, "w")) == NULL ) {
        nggen_error("%s: cannot create\n", fname);
        return;
    }
    /*======================================================
                    start generation of makefile
    ======================================================*/
    fprintf(fp, "# This file '%s' was created by %s. Don't edit\n\n",
        fname, program);
    fprintf(fp, "CC = %s\n", COMPILER);
    fprintf(fp, "include $(NG_DIR)/lib/template.mk\n");

    fprintf(fp, "\n# CompileOptions:\n");

    fprintf(fp, "NG_USER_CFLAGS = ");
    for ( i = 0; i < n_options_g; i++ ) {
        fprintf(fp, "%s ",compile_options[i]);
    }
    fprintf(fp, "\n");

    fprintf(fp, "\n#  Define NG_COMPILER & NG_LINKER as "
        "$(CC) if it is not defined.\n\n");


    if (compiler_name == NULL && linker_name == NULL) {
      if ( gmake_g ) { 
        /***  use gnu make style  ***/ /* YES                               */
#if !defined(__OLD_GMAKE__) /**  Before Version 3.76.90  **/
	fprintf(fp, "NG_COMPILER ?= $(CC)\n");
	fprintf(fp, "NG_LINKER ?= $(CC)\n");
#else /** __OLD_GMAKE__ **/
        fprintf(fp, "NG_COMPILER := "
		"$(NG_COMPILER?$(NG_COMPILER):$(CC))\n");
        fprintf(fp, "NG_LINKER := "
		"$(NG_LINKER?$(NG_LINKER):$(CC))\n");
#endif
      } else {  /**  NOT GMAKE  **/
#if defined(__alpha__)
        fprintf(fp, "NG_COMPILER = "
		"$(NG_COMPILER?$(NG_COMPILER):$(CC))\n");
        fprintf(fp, "NG_LINKER = "
		"$(NG_LINKER?$(NG_LINKER):$(CC))\n");
#else /** default **/
	fprintf(fp, "NG_COMPILER = $(CC)\n");
	fprintf(fp, "NG_LINKER = $(CC)\n");
#endif
      }
    } else {
      if (compiler_name == NULL)
	fprintf(fp, "NG_COMPILER = $(CC)\n");
      else
	fprintf(fp, "NG_COMPILER = %s\n", compiler_name);
      if (linker_name == NULL)
	fprintf(fp, "NG_LINKER = $(CC)\n");
      else 
	fprintf(fp, "NG_LINKER = %s\n", linker_name);
    }

    /*======================================================
                     stub source
    ======================================================*/
    fprintf(fp, "\n# stub sources\n\nNG_STUB_SRC =");
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        fprintf(fp, " _stub_%s.c ", SYM_NAME(curr->ent_id));
    }

    /*======================================================
                    output stub_programs
    ======================================================*/
    fprintf(fp, "\n\n# stub programs\n\nNG_STUB_PROGRAM =");
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        fprintf(fp, " _stub_%s", SYM_NAME(curr->ent_id));
    }

    fprintf(fp,"\n\n# stub inf files\nNG_INF_FILES =");
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        fprintf(fp," _stub_%s.inf", SYM_NAME(curr->ent_id));
    }
    fprintf(fp,"\n\n# LDAP dif file\nLDAP_DIF = root.ldif");


    fprintf(fp,"\n\nall: $(NG_STUB_PROGRAM) ");
    fprintf(fp,"$(NG_INF_FILES) $(LDAP_DIF)");
    fprintf(fp,"\n\n");

    /*======================================================
                   compiler and linker
    ======================================================*/
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        ent_name = SYM_NAME(curr->ent_id);
        fprintf(fp, "_stub_%s.o: _stub_%s.c\n\t$(NG_COMPILER) "
            "$(NG_CPPFLAGS) $(CFLAGS) $(NG_CFLAGS) $(NG_USER_CFLAGS) -c _stub_%s.c\n\n",
            ent_name, ent_name, ent_name);

        fprintf(fp, "_stub_%s: _stub_%s.o %s\n\t$(NG_LINKER) "
            "$(NG_CFLAGS) $(CFLAGS) -o _stub_%s _stub_%s.o $(LDFLAGS) "
            "$(NG_STUB_LDFLAGS) %s $(LIBS)",
            ent_name, ent_name,
            (curr->required == NULL) ? "": curr->required,
            ent_name, ent_name,
            (curr->required == NULL) ? "": curr->required);
        for ( j = 0; j < n_libs_g; j++ ) {
            fprintf(fp, " %s", libs[j]);
        }
        fprintf(fp, "\n\n");

        if (curr->backend == NG_BACKEND_MPI ||
            curr->backend == NG_BACKEND_BLACS) {
            fprintf(fp, "_stub_%s-info.o: _stub_%s.c\n"
				"\t$(CC) $(NG_CPPFLAGS) $(NG_CFLAGS) $(CFLAGS) $(NG_USER_CFLAGS) "
			"-D%s "
			"-o _stub_%s-info.o -c _stub_%s.c \n\n",
			ent_name, ent_name, DUMP_INFO_TAG, ent_name, ent_name);

			fprintf(fp, STUB_INFO_PROGRAM ": _stub_%s-info.o\n"
				"\t$(CC) $(NG_CPPFLAGS) $(NG_CFLAGS) $(CFLAGS) $(NG_USER_CFLAGS) "
			"-o " STUB_INFO_PROGRAM " _stub_%s-info.o "
			"$(LDFLAGS) $(NG_STUB_LDFLAGS) $(LIBS)\n\n",
			ent_name, ent_name, ent_name, ent_name);

            fprintf(fp,"_stub_%s.inf: " STUB_INFO_PROGRAM "\n"
				"\t ./" STUB_INFO_PROGRAM " -i _stub_%s.inf\n\n",
				ent_name, ent_name, ent_name, ent_name);
        }else {
            fprintf(fp,"_stub_%s.inf: _stub_%s\n"
				    "\t ./_stub_%s -i _stub_%s.inf\n\n",
				    ent_name, ent_name, ent_name, ent_name);

        }

        fprintf(fp, "\n\n");
    }
    fprintf(fp, "\n$(LDAP_DIF): $(NG_INF_FILES)\n");
	fprintf(fp, "\t$(NG_DIR)/bin/ng_gen_dif $(NG_STUB_PROGRAM)\n");

    fprintf(fp, "\ninstall: $(LDAP_DIF)\n");
        fprintf(fp, "\t$(INSTALL) *.ldif $(LDIF_INSTALL_DIR)\n");

    fprintf(fp, "\nclean:\n");
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        if (curr->backend == NG_BACKEND_MPI ||
            curr->backend == NG_BACKEND_BLACS) {
            fprintf(fp, "\trm -f _stub_%s.o _stub_%s-info.o _stub_%s.c \n",
                SYM_NAME(curr->ent_id),
                SYM_NAME(curr->ent_id),
                SYM_NAME(curr->ent_id));
        } else {
            fprintf(fp, "\trm -f _stub_%s.o _stub_%s.c\n",
                SYM_NAME(curr->ent_id),
                SYM_NAME(curr->ent_id));
        }
    }

    fprintf(fp, "\nveryclean: clean\n");
    fprintf(fp, "\trm -f $(NG_STUB_PROGRAM)");
	fprintf(fp, " $(NG_INF_FILES) $(LDAP_DIF) *.ngdef");
	fprintf(fp, "\n");
    for ( curr = stubs_head; curr != NULL; curr = curr->next_stub ) {
        if ( curr->required != NULL ) {
            fprintf(fp, "\trm -f ");
            fprintf(fp, "%s\n", curr->required);
        }
    }
    fprintf(fp, "\trm -f _stub_*-info\n");

    fprintf(fp, "\n");
    fprintf(fp, "\n# END OF Makefile\n");
    fclose(fp);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_expression_print                                           */
/*---------------------------------------------------------------------------*/
void
nggen_expression_print(
    FILE *fp, 
    NGGEN_EXPRESSION *exp
)
{
    int i;   

    /*======================================================
                       type,val
    ======================================================*/
    for ( i = 0; ; i++ ) {
	if ((i == 0) && (exp->type[i] == NG_EXPRESSION_VALUE_TYPE_NONE)) {
	    /* if only have VALUE_NONE, then put it once */
	    fprintf(fp, "\t{%s, %d},\n", 
		GET_VAL_STRING(exp->type[i]), exp->val[i]);
	    fprintf(fp, "\t{%s, 0}\n", 
		GET_VAL_STRING(NG_EXPRESSION_VALUE_TYPE_END));
	    break;
	} else if (exp->type[i] == NG_EXPRESSION_VALUE_TYPE_END) {
	    /* if it's end, then finish list */
	    fprintf(fp, "\t{%s, %d}\n", 
		GET_VAL_STRING(exp->type[i]), exp->val[i]);
	    break;
	} else {
	    /* normal */
	    fprintf(fp, "\t{%s, %d},\n", 
		GET_VAL_STRING(exp->type[i]), exp->val[i]);
	}
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_stub_param_desc                                            */
/*---------------------------------------------------------------------------*/
void
nggen_stub_param_desc(
    FILE *fp, 
    struct method_gen_desc *ep,
    int mi
)
{
    struct param_gen_desc *dp;
    int i, j;                 
    int callbackHasCome = 0;

    /* check if number of param were 0 */
    if ( ep->nparam < 1 ) {
	/* nothing will be done */
	return;
    }

    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];
	if ( dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK ) {
	    break;
	}
        if ( dp->ndim > 0 ) { 
            fprintf(fp, "\n");
            for ( j = 0; j < dp->ndim; j++ ) {
                /*=================================================
                                size
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t size%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].size_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                start
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t start%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].start_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                end
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t end%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].end_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                step
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t step%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].step_exp));
                fprintf(fp, "};\n\n");
            }
        }
    }
    fprintf(fp, "\n");

    /* print subscriptlist */
    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];
	if ( dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK ) {
	    break;
	}
        if ( dp->ndim > 0 ) { 
            fprintf(fp, "ngSubscriptInformation_t dims%d_%d[] = {\n", mi, i);
            for ( j = 0; j < dp->ndim; j++ ) {  
                fprintf(fp, "\t{ size%d_%d_%d, start%d_%d_%d, end%d_%d_%d, step%d_%d_%d },\n",
			    mi, i, j, mi, i, j, mi, i, j, mi, i, j);
            }
            fprintf(fp, "};\n");
	    fprintf(fp, "\n");
        }
    }

    fprintf(fp, "ngArgumentInformation_t param_desc_%d[] = {\n", mi);

    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];

	if ((callbackHasCome != 0) &&
		(dp->param_type != NG_ARGUMENT_DATA_TYPE_CALLBACK) ) {
	    continue;
	}

        fprintf(fp, "\t{ %s, %s, %d,",
            GET_MODE_STRING(dp->param_inout),
            GET_DATA_STRING(dp->param_type), dp->ndim);

	/* check dim and point ArgSubScript */
        if ( dp->ndim > 0) {
            fprintf(fp, " dims%d_%d, ", mi, i);
        } else {
            fprintf(fp, " NULL, ");
        }

	/* callback, point RemoteMethodInformation for callback */
        if ( dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK ) {
	    callbackHasCome = 1;
            fprintf(fp, " callback%d_%d ", mi, i);
        } else {
            fprintf(fp, " NULL ");
        }
        fprintf(fp, "},\n");
   }
   fprintf(fp, "};\n");
   fprintf(fp, "\n");


}

void
nggen_stub_callback_param_desc(
    FILE *fp, 
    struct method_gen_desc *ep,
    int mi
)
{
    struct param_gen_desc *dp = NULL;
    int i, j;                 
    int in_func = FALSE;
    int hasCallback = FALSE;

    /* print subscripts of callback parameters */
    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];

	if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
	    hasCallback = TRUE;
	    continue;
	}
	if (hasCallback != TRUE) {
	    continue;
	}

        if ( dp->ndim > 0 ) { 
            fprintf(fp, "\n");
            for ( j = 0; j < dp->ndim; j++ ) {
                /*=================================================
                                size
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t size%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].size_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                start
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t start%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].start_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                end
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t end%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].end_exp));
                fprintf(fp, "};\n\n");
                /*=================================================
                                step
                 ================================================*/
                fprintf(fp, "ngExpressionElement_t step%d_%d_%d[] = {\n",
			    mi, i, j);
                nggen_expression_print(fp, &(dp->dim[j].step_exp));
                fprintf(fp, "};\n\n");
            }
        }
	fprintf(fp, "\n");
    }

    /* print subscriptlist for callback */
    hasCallback = FALSE;
    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];

	if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
	    hasCallback = TRUE;
	    continue;
	}
	if (hasCallback != TRUE) {
	    continue;
	}

        if ( dp->ndim > 0 ) { 
            fprintf(fp, "ngSubscriptInformation_t dims%d_%d[] = {\n", mi, i);
            for ( j = 0; j < dp->ndim; j++ ) {  
                fprintf(fp, "\t{ size%d_%d_%d, start%d_%d_%d, end%d_%d_%d, step%d_%d_%d },\n",
			    mi, i, j, mi, i, j, mi, i, j, mi, i, j);
            }
            fprintf(fp, "};\n");
	    fprintf(fp, "\n");
        }
    }

    /* check all of callback argument */
    in_func = FALSE;
    for ( i = 0; i < ep->nparam; i++ ) {
	dp = &ep->params[i];
	/* found callback type parameter */
	if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
	    if (in_func == TRUE) {
		/* close previous entry */
		fprintf(fp, "};\n");
		fprintf(fp, "\n");
	        in_func = FALSE;
	    }

	    /* Is callback argument not available? */
	    if (((i + 1) >= ep->nparam) ||
	        (((i + 1) < ep->nparam) &&
	        (ep->params[i + 1].param_type ==
	        NG_ARGUMENT_DATA_TYPE_CALLBACK))) {
		/* suppress definition */
	        continue;
	    }

	    /* open new entry */
	    in_func = TRUE;
	    fprintf(fp, 
		"ngArgumentInformation_t callbackparams%d_%d[] = {\n", mi, i);
	    continue;
	} else if (in_func == TRUE) {
	    /* print argument of callback */
	    fprintf(fp, "\t{ %s, %s, %d, ",
		GET_MODE_STRING(dp->param_inout),
		GET_DATA_STRING(dp->param_type), dp->ndim);
	    /* check dim and point ArgSubScript */
	    if ( dp->ndim > 0) {
		fprintf(fp, " dims%d_%d, ", mi, i);
	    } else {
		fprintf(fp, " NULL, ");
	    }
	    /* print information of callback */
	    fprintf(fp, "NULL},\n");
	}
    }

    if (in_func == TRUE) {
	/* close entry */
	fprintf(fp, "};\n");
	fprintf(fp, "\n");
	in_func = FALSE;
    }

    /* print callback information */
    if (hasCallback == TRUE) {
	int callback_id = 0;
	int callback_arg_count = 0;
	char callback_param_name[NG_BUFSIZ];
	for ( i = 0; i < ep->nparam; i++ ) {
	    dp = &ep->params[i];
	    /* found callback type parameter */
	    if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK) {
		if (callback_id > 0) {
		    fprintf(fp, "\t\t%d,\n", callback_arg_count);
		    fprintf(fp, "\t\t%s,\n",
		        ((callback_arg_count > 0) ?
		        callback_param_name : "NULL"));
		    fprintf(fp, "\t\t0,\n");  /* shrink */
		    fprintf(fp, "\t\t\"\"\n");  /* description */
		    fprintf(fp, "\t}\n");
		    fprintf(fp, "};\n");
		    fprintf(fp, "\n");
		    callback_arg_count = 0;
		}
		/* calcOrder for callback */
		fprintf(fp,
		    "ngExpressionElement_t callback_calc_order%d_%d[] = {\n",
		    mi, i);
		fprintf(fp, "\t{NG_EXPRESSION_VALUE_TYPE_NONE, 0},\n");
		fprintf(fp, "\t{NG_EXPRESSION_VALUE_TYPE_END, 0},\n");
		fprintf(fp, "};\n");
		fprintf(fp, "\n");

		/* callback */
		fprintf(fp, 
		    "ngRemoteMethodInformation_t callback%d_%d[] = {\n",
		    mi, i);
		fprintf(fp, "\t{\n");
		fprintf(fp, "\t\t\"%s\",\n", SYM_NAME(dp->param_id));
		fprintf(fp, "\t\t%d,\n", callback_id++);
		fprintf(fp, "\t\tcallback_calc_order%d_%d,\n", mi, i);
		sprintf(callback_param_name, "callbackparams%d_%d", mi, i);
		continue;
	    }
	    if (callback_id > 0) {
		/* increment counter */ 
		callback_arg_count++;
	    }
	}

	if (callback_id > 0) {
	    fprintf(fp, "\t\t%d,\n", callback_arg_count);
	    fprintf(fp, "\t\t%s,\n",
		((callback_arg_count > 0) ?  callback_param_name : "NULL"));
	    fprintf(fp, "\t\t0,\n");  /* shrink */
	    fprintf(fp, "\t\t\"\"\n");  /* description */
	    fprintf(fp, "\t}\n");
	    fprintf(fp, "};\n");
	    fprintf(fp, "\n");
	}
    }
}

void
nggen_stub_method_desc_description(FILE * fp, 
				      struct method_gen_desc * ep, int mi) {
    /* output description */
    if (ep->description) {
      fprintf(fp,"/* DESCRIPTION:\n%s\n*/\n\n",ep->description);
      fprintf(fp,"static char method_description_%d[] = \"%s\";\n", 
	      mi,
	      ep->description);
      fprintf(fp, "\n");
    } else {
      fprintf(fp,"static char method_description_%d[] = \"\";\n", mi);
      fprintf(fp, "\n");
    }
}

void
nggen_stub_method_desc(FILE * fp, struct stub_gen_entry * ep)
{
    int mi;

    /* print callback parameter */
    for (mi = 0; mi < ep->n_methods; mi++) {
	nggen_stub_callback_param_desc(fp, &(ep->methods[mi]), mi);
    }

    /* print parameter */
    for (mi = 0; mi < ep->n_methods; mi++) {
       	nggen_stub_param_desc(fp, &(ep->methods[mi]), mi);
    }

    /* print method description */
    for (mi = 0; mi < ep->n_methods; mi++) {
	nggen_stub_method_desc_description(fp, &(ep->methods[mi]), mi);
    }

    /* print calc order */
    for (mi = 0; mi < ep->n_methods; mi++) {
	fprintf(fp,"ngExpressionElement_t calc_order%d[] = {\n", mi);
    	nggen_expression_print(fp, &(ep->methods[mi].order));
	fprintf(fp,"};\n\n");
    }

    /* print method information */
    fprintf(fp,"ngRemoteMethodInformation_t methods_desc[] = {\n");
    for (mi = 0; mi < ep->n_methods; mi++){
	int nparam = 0, hasCallBack = 0, i;
	for (i = 0; i < ep->methods[mi].nparam; i++) {
	    if ( (hasCallBack != 0) &&
		    (ep->methods[mi].params[i].param_type !=
		    NG_ARGUMENT_DATA_TYPE_CALLBACK) ) {
		continue;
	    } else if (ep->methods[mi].params[i].param_type ==
		    NG_ARGUMENT_DATA_TYPE_CALLBACK) {
		hasCallBack = 1;
	    }
	    nparam++;
	}
	/* check no arguments */
	if (ep->methods[mi].nparam < 1) {
	    fprintf(fp,"\t{ \"%s\", %d, calc_order%d, %d, NULL,\n",
		SYM_NAME(ep->methods[mi].method_id),
		mi,
		mi,
		nparam);
	} else {
	    fprintf(fp,"\t{ \"%s\", %d, calc_order%d, %d, param_desc_%d,\n",
		SYM_NAME(ep->methods[mi].method_id),
		mi,
		mi,
		nparam,
		mi);
	}
    	fprintf(fp, "\t\t%d, /* server side shrink */\n", ep->shrink);
    	fprintf(fp, "\t\tmethod_description_%d, \n", mi);
    	fprintf(fp,"\t}, \n");
    }
    fprintf(fp, "};\n");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_stub_info_structure                                        */
/*---------------------------------------------------------------------------*/
void
nggen_stub_info_structure(
    FILE *fp, 
    struct stub_gen_entry *ep,
    char *ent_name
)
{
    int info_type;


    info_type = NG_CLASS_LANGUAGE_C;

    if ( ep->language != NULL ) {
        if ( strcasecmp(ep->language, C_LANG) == 0 ) {
            info_type = NG_CLASS_LANGUAGE_C;
        } else if ( strcasecmp(ep->language, CXX_LANG) == 0 ) {
            info_type = NG_CLASS_LANGUAGE_C;
        } else if (strcasecmp(ep->language, FORTRAN) == 0 ) {
            info_type = NG_CLASS_LANGUAGE_FORTRAN;
        } else {
            nggen_warning("warning: Unknown language was specified(use C).");
        }
    }

    /* print method information */
    nggen_stub_method_desc(fp, ep);

    /* print class information */
    fprintf(fp, "\n/** stub info declaration */\n");
    fprintf(fp, "\n");
    fprintf(fp, "ngRemoteClassInformation_t remoteClassInfo = {\n");
    fprintf(fp, "\t\"%s/%s\",\n",
	current_module == NULL ? "" : current_module, ent_name);
    /* TODO: set version for a class by  user */
    fprintf(fp, "\t\"%d.%d\",\n", MAJOR_VERSION, MINOR_VERSION);
    fprintf(fp, "\tstub_description, \n");
    fprintf(fp, "\t(ngBackend_t)%d, /* backend */\n", ep->backend);
    fprintf(fp, "\t(ngClassLanguage_t)%d, /* lang */\n", info_type);
    fprintf(fp, "\t%d, /* number of methods */\n", ep->n_methods);
    fprintf(fp, "\tmethods_desc, \n");
    fprintf(fp, "};\n\n");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_callback_proxy_sub                                         */
/*---------------------------------------------------------------------------*/
void
nggen_callback_proxy_sub(
    FILE *fp, 
    struct method_gen_desc *ep, 
    int pos, 
    int count,
    int callback_id
)
{
    int i;
    struct param_gen_desc *dp;

    fprintf(fp, "\n");
    fprintf(fp, "int\n");
    fprintf(fp, "%s(", SYM_NAME(ep->params[pos].param_id));

    for (i = 1; i <= count; i++) {
        dp = &ep->params[pos + i];
        fprintf(fp, "%s %s%s", 
            nggen_c_type_name(dp->param_type), (dp->ndim > 0) ? "* ":"",
            SYM_NAME(dp->param_id));
        fprintf(fp, "%s", ((i < count) ? ", " : ""));
    }
    fprintf(fp, ")\n{\n");

    fprintf(fp, "\tint error;\n");
    fprintf(fp, "\treturn ngexStubCallback(%d, &error", callback_id);

    for (i = 1; i <= count; i++) {
        dp = &ep->params[pos + i];
        fprintf(fp, "%s", ", ");
        fprintf(fp, "%s",  SYM_NAME(dp->param_id));
    }
    fprintf(fp, ");\n");  
    fprintf(fp, "}\n");  
    fprintf(fp, "\n");  
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_callback_proxy                                             */
/*---------------------------------------------------------------------------*/
void
nggen_callback_proxy(
    FILE *fp, 
    struct stub_gen_entry *ep
)
{
    struct param_gen_desc *dp;
    int callback_id, cb_args;
    int mi, i, j;


    /* generate callback proxy */
    fprintf(fp, "\n/* Callback proxy */\n");
   
    for ( mi = 0; mi < ep->n_methods; mi++ ) {
        callback_id = 0;

        for (i = 0; i < ep->methods[mi].nparam; i++) {
            dp = &ep->methods[mi].params[i];
            if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK) {

                cb_args = 0;
                for (j = i + 1; j < ep->methods[mi].nparam; j++) {
                    if (ep->methods[mi].params[j].param_type ==
                        NG_ARGUMENT_DATA_TYPE_CALLBACK) {
                        break;
                    }
                    cb_args++;
                }

                nggen_callback_proxy_sub(
                    fp, &ep->methods[mi], i, cb_args, callback_id++);
            }
        }
    }

    fprintf(fp, "\n\n");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_method_body                                                */
/*---------------------------------------------------------------------------*/
void
nggen_method_body(
    struct method_gen_desc *ep, 
    char *language,
    FILE *fp
)
{
    list lp;               
    char *function_name;   
    char *tmp;             


    if ( ep->body_expr ) { /* generate body                      */
        function_name = nggen_make_function_name
            (ep->body_expr, current_fortran_format);
        fprintf(fp, "\t\t%s(", function_name);

        FOR_ITEMS_IN_LIST(lp, EXPR_ARG3(ep->body_expr)) {
            tmp = SYM_NAME(EXPR_SYM(LIST_ITEM(lp)));
            fprintf(fp,"%s%s", nggen_need_and(tmp, ep)?"&":"", tmp);

            if (fortran_string_convention == FORTRAN_STRING_CONVENTION_JUST_AFTER &&
                language != NULL &&
                strcasecmp(language, FORTRAN) == 0 &&
                nggen_need_strlen(tmp, ep)){
              fprintf(fp,",strlen(%s)", tmp);
            }

            if ( LIST_NEXT(lp) != NULL ) { 
                fprintf(fp, ",");
            }
        }
        if (fortran_string_convention == FORTRAN_STRING_CONVENTION_LAST &&
	    language != NULL &&
            strcasecmp(language, FORTRAN) == 0){
          FOR_ITEMS_IN_LIST(lp,EXPR_ARG3(ep->body_expr)){
            char * tmp = SYM_NAME(EXPR_SYM(LIST_ITEM(lp)));
            if (nggen_need_strlen(tmp, ep)){
              fprintf(fp,",strlen(%s)", tmp);
            }
          }
        }
        fprintf(fp, ");\n");
    } else {
        fprintf(fp, "%s\n", ep->body);
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_stub_program_head                                          */
/*---------------------------------------------------------------------------*/
void
nggen_stub_program_head(
    FILE *fp,
    char *fname
)
{
    time_t clock;  
    char * pc;     

    clock = time(NULL);
    pc = ctime(&clock);
    *(pc + 24) = '\0';
    fprintf(fp, "/*====================================================="
        "========================\n");
    fprintf(fp, "Module          :%s\n", current_module == NULL ? "" : current_module);
    fprintf(fp, "Filename        :%s\n", fname);
    fprintf(fp, "RCS             :\n");
    fprintf(fp, "        $Source:\n");
    fprintf(fp, "        $Revision:\n");
    fprintf(fp, "        $Author:\n");
    fprintf(fp, "        $Data:%s\n", pc);
    fprintf(fp, "        $Locker:\n");
    fprintf(fp, "        $state:\n");
    fprintf(fp, "========================================================"
        "=====================*/\n");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = print_proto_type                                                 */
/*---------------------------------------------------------------------------*/
static void 
print_proto_type(
    FILE *fp, 
    struct stub_gen_entry *entry
) 
{
    int i, mi;
    char *function_name;
    unsigned short need_definition = 0;

    if ( entry->language && strcasecmp(entry->language, "fortran") == 0){ 
	return ;
    }

    for ( mi = 0; mi < entry->n_methods; mi++ ) {
	if (entry->methods[mi].body_expr == NULL) {
	    continue;
	} else if (!( entry->language && 
			strcasecmp(entry->language, CXX_LANG) == 0) &&
			need_definition == 0) {
	    fprintf(fp, "#ifdef __cplusplus\n");
	    fprintf(fp, "extern \"C\"   {\n");
	    fprintf(fp, "#endif\n\n");
	    need_definition = 1;
	}

	function_name = nggen_make_function_name
	    (entry->methods[mi].body_expr, current_fortran_format);

	fprintf(fp, "extern void %s(", function_name);

	for ( i = 0; i < entry->methods[mi].nparam; i++ ) {
	    struct param_gen_desc *desc = &entry->methods[mi].params[i];
	    if ( desc->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK ) {
		break;
	    }
	    fprintf(fp, "%s%s %s%s", i > 0 ? "," : "",
		nggen_c_type_name(desc->param_type), desc->ndim > 0 ? "*":"",
		SYM_NAME(desc->param_id));
	}
	fprintf(fp, ");\n");
    }

    if (need_definition == 1 &&
	!( entry->language && strcasecmp(entry->language, CXX_LANG) == 0)) {
	fprintf(fp, "\n#ifdef __cplusplus\n");
	fprintf(fp, "}\n");
	fprintf(fp, "#endif\n\n");
    }
}

#if 0 /* Is this necessary? */
/*---------------------------------------------------------------------------*/
/* ID     = in_param_list                                                    */
/*---------------------------------------------------------------------------*/
static int 
in_param_list(
    struct stub_gen_entry *ep, 
    int type
)
{
    int i, mi;
    for ( mi = 0; mi < ep->n_methods; mi++) {
	for (i = 0; i < ep->methods[mi].nparam; i++) {
	    if (ep->methods[mi].params[i].param_type == type) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}
#endif

void 
nggen_special_handle(struct stub_gen_entry * ep, FILE * fp,
			char * handle_name){
  int mi;
  for (mi = 0; mi < ep->n_methods; mi++){
    struct method_gen_desc *mp = &(ep->methods[mi]);
    if (mp->nparam == 0 &&
	strncmp(handle_name, SYM_NAME(mp->method_id), strlen(handle_name))
	== 0){
      fprintf(fp, "/* %s method */\n", handle_name);
      nggen_method_body(mp, ep->language, fp);
      return;
    }      
  }
}


void
nggen_method_handle(struct method_gen_desc *ep, 
		       char * language,
		       int backend,
		       FILE * fp, int gen_set_arg)
{
  struct param_gen_desc *dp;
  int i, j;

  fprintf(fp, "\t\t{         /* %s */\n", SYM_NAME(ep->method_id));
  /* declare parameters */
  for (i = 0; i < ep->nparam; i++){
    dp = &ep->params[i];
    if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK)
      break;
    fprintf(fp, "\t\t\t%s %s%s;\n", nggen_c_type_name(dp->param_type),
	    (dp->ndim > 0) ? "*":"",
	    SYM_NAME(dp->param_id));
  }

  if (gen_set_arg) {
    /* input and setup working area */
    for (i = 0; i < ep->nparam; i++){
      dp = &ep->params[i];
      if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK)
	break;
      
      fprintf(fp,"\t\tngexStubGetArgument(%d,&%s,&error);\n",
	      i,SYM_NAME(dp->param_id));
    }
  }

  /* broadcast/allocate parameters for MPI */
  if ((backend == NG_BACKEND_MPI) || (backend == NG_BACKEND_BLACS)) {
    int first = 1;

    /* 1st, broadcast scalar variables */
    for (i = 0; i < ep->nparam; i++) {
      /* get param_gen_desc */
      dp = &ep->params[i];

      /* callback indicates the end of params */
      if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK)
        break;

      /* broadcast scalar parameters */
      if ((dp->ndim == 0) &&
          (dp->param_distmode == NGGEN_ARGUMENT_DIST_MODE_BROADCAST)) {
          /* put comment before the first variable */
          if (first == 1) {
            fprintf(fp, "\n\t\t\t/* broadcast scalar variables */\n");
            first = 0;
          }
            
          /* string/filename variable */
          if ((dp->param_type == NG_ARGUMENT_DATA_TYPE_STRING) ||
              (dp->param_type == NG_ARGUMENT_DATA_TYPE_FILENAME)) {
            fprintf(fp, "\t\t\t{\n");
            fprintf(fp, "\t\t\tint ng_str_len;\n");

            /* get length of string (rank == 0) */
            if (gen_set_arg) {
              fprintf(fp, "\t\t\tng_str_len = strlen(%s);\n",
                SYM_NAME(dp->param_id));
            }

            /* broadcast length of string */
            fprintf(fp,
              "\t\t\tMPI_Bcast(&ng_str_len, 1, MPI_INT, 0, MPI_COMM_WORLD);\n");

            /* allocate area for string (rank != 0) */
            if (!gen_set_arg) {
              fprintf(fp,
                "\t\t\t%s = (char *)malloc(sizeof(char) * ng_str_len);\n",
                SYM_NAME(dp->param_id));
            }

            /* broadcast string */
            fprintf(fp,
              "\t\t\tMPI_Bcast(%s, ng_str_len, MPI_CHAR, 0, MPI_COMM_WORLD);\n",
                SYM_NAME(dp->param_id));
            fprintf(fp, "\t\t\t}\n");
          } else if ((dp->param_type == NG_ARGUMENT_DATA_TYPE_SCOMPLEX) ||
              (dp->param_type == NG_ARGUMENT_DATA_TYPE_DCOMPLEX)) {
            /* scomplex/dcomplex type */
            fprintf(fp, "\t\t\tMPI_Bcast(&%s, 2, %s, 0, MPI_COMM_WORLD);\n",
              SYM_NAME(dp->param_id),
              nggen_mpi_type_name(dp->param_type));
          } else {
            /* other type */
            fprintf(fp, "\t\t\tMPI_Bcast(&%s, 1, %s, 0, MPI_COMM_WORLD);\n",
              SYM_NAME(dp->param_id),
              nggen_mpi_type_name(dp->param_type));
          }
        }
    }

    /* reset flag for comment */
    first = 1;

    /* then, broadcast and allocate array variables */
    for (i = 0; i < ep->nparam; i++) {
      /* get param_gen_desc */
      dp = &ep->params[i];

      /* callback indicates the end of params */
      if (dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK)
        break;

      /* broadcast/allocate array parameters */
      if (dp->ndim > 0) {
        if ((dp->param_distmode == NGGEN_ARGUMENT_DIST_MODE_BROADCAST) ||
            (dp->param_distmode == NGGEN_ARGUMENT_DIST_MODE_ALLOCATE)) {

          /* put comment before the first variable */
          if (first == 1) {
            fprintf(fp, "\n\t\t\t/* broadcast or allocate variables */\n");
            first = 0;
          }
            
          /* allocate before broadcast */
          if (! gen_set_arg) {
            fprintf(fp, "\t\t\t%s = (%s *)malloc(sizeof(%s) * (",
              SYM_NAME(dp->param_id),
              nggen_c_type_name(dp->param_type),
              nggen_c_type_name(dp->param_type));

            if (dp->dim[0].size_expr == NULL) {
              fprintf(fp, "1");
            } else {
              for (j = 0; j < dp->ndim; j++) {
                nggen_print_expression(fp, ep, dp->dim[j].size_expr);
                if ((j + 1) >= dp->ndim) {
                  break;
                }
                fprintf(fp, " * ");
              }
            }
            fprintf(fp, "));\n");
          }

          /* allocation process is over, continue to loop */
          if (dp->param_distmode == NGGEN_ARGUMENT_DIST_MODE_ALLOCATE) {
            continue;
          }

          /* broadcast */
          if ((dp->param_type == NG_ARGUMENT_DATA_TYPE_STRING) ||
              (dp->param_type == NG_ARGUMENT_DATA_TYPE_FILENAME)) {
            fprintf(fp, "\t\t\t{\n");
            fprintf(fp, "\t\t\tint i;\n");
            fprintf(fp, "\t\t\tint ng_str_len;\n");
            fprintf(fp, "\t\t\t\n");
            fprintf(fp, "\t\t\tfor(i = 0; i < ");
            nggen_print_expression(fp, ep, dp->dim[0].size_expr);
            fprintf(fp, "\t\t\t; i++)\n");
            fprintf(fp, "\t\t\t{\n");

            /* get length of string (rank == 0) */
            if (gen_set_arg) {
              fprintf(fp, "\t\t\t\tng_str_len = strlen(%s[i]);\n",
                SYM_NAME(dp->param_id));
            }

            /* broadcast length of string */
            fprintf(fp,
              "\t\t\t\tMPI_Bcast(&ng_str_len, 1, MPI_INT, 0, MPI_COMM_WORLD);\n");

            /* allocate area for string (rank != 0) */
            if (!gen_set_arg) {
              fprintf(fp,
                "\t\t\t\t%s[i] = (char *)malloc(sizeof(char) * ng_str_len);\n",
                SYM_NAME(dp->param_id));
            }

            /* broadcast string */
            fprintf(fp,
              "\t\t\t\tMPI_Bcast(%s[i], ng_str_len, MPI_CHAR, 0, MPI_COMM_WORLD);\n",
                SYM_NAME(dp->param_id));
            fprintf(fp, "\t\t\t}\n");

            fprintf(fp, "\t\t\t}\n");
          } else {
            fprintf(fp, "\t\t\tMPI_Bcast(%s, (", SYM_NAME(dp->param_id));

            if (dp->dim[0].size_expr == NULL) {
              fprintf(fp, "1");
            } else {
              for (j = 0; j < dp->ndim; j++) {
                nggen_print_expression(fp, ep, dp->dim[j].size_expr);
                if ((j + 1) >= dp->ndim) {
                  break;
                }
                fprintf(fp, " * ");
              }
            }

            if ((dp->param_type == NG_ARGUMENT_DATA_TYPE_SCOMPLEX) ||
                (dp->param_type == NG_ARGUMENT_DATA_TYPE_DCOMPLEX)) {
              fprintf(fp, " * 2");
            }

            fprintf(fp, "), %s, 0, MPI_COMM_WORLD);\n",
              nggen_mpi_type_name(dp->param_type));
          }
        }
      }
    }
  }

  nggen_method_body(ep, language, fp);
  fprintf(fp, "\t\t}\n");
}

static void
nggen_print_not_remote_executable_main(FILE *fp)
{

    fprintf(fp, "#ifdef %s\n", DUMP_INFO_TAG);
    fprintf(fp, "\n/* To print the Remote Class Information */\n");
    fprintf(fp, "int main(int argc, char ** argv){\n");
    fprintf(fp, "\tint error;\n");
    fprintf(fp, "\tint immediateExit;\n\n");

    fprintf(fp, "\timmediateExit = ngexStubAnalyzeArgumentWithExit(\n");
    fprintf(fp, "\t\targc, argv, &remoteClassInfo, &error);\n");
    fprintf(fp, "\tif (immediateExit != 0) {\n");
    fprintf(fp, "\t\texit(0);\n");
    fprintf(fp, "\t}\n\n");

    fprintf(fp, "\treturn 0;\n\n");
    fprintf(fp, "}\n");
    fprintf(fp, "#else /* %s */\n", DUMP_INFO_TAG);

    return ;
}
/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_stub_program                                               */
/*---------------------------------------------------------------------------*/
void
nggen_stub_program(
    struct stub_gen_entry *ep
)
{
    FILE *fp;
    char fname[NG_BUFSIZ];
    char *ent_name;
    int i;

    ent_name = SYM_NAME(ep->ent_id);
    sprintf(fname, "_stub_%s.c", ent_name);
    if ( (fp = fopen(fname, "w")) == NULL ) {
        nggen_error("%s: cannot create\n");
        return;
    }
    nggen_stub_program_head(fp, fname);
    if ( ep->backend == NG_BACKEND_MPI ||
	 ep->backend == NG_BACKEND_BLACS ) { /* MPI */
        fprintf(fp, "#ifndef %s\n", DUMP_INFO_TAG);
        fprintf(fp, "#include <mpi.h>\n");
        fprintf(fp, "#include <string.h>\n");
        fprintf(fp, "#endif /* %s */\n", DUMP_INFO_TAG);
    }
    fprintf(fp, "#include \"ngEx.h\"\n\n");

    /* proto type declaration	*/
    print_proto_type(fp, ep);

    /* output description */
    if ( ep->description ) {
        fprintf(fp, "/* DESCRIPTION:\n%s\n*/\n\n", ep->description);
        fprintf(fp, "static char stub_description[] = \"%s\";\n\n",
            ep->description);
    } else {
        fprintf(fp, "static char stub_description[] = \"\";\n\n");
    }

    /* generate stub structure */
    nggen_stub_info_structure(fp, ep, ent_name);

    /* generate globals */
    fprintf(fp, "/* Globals */\n");
    for ( i = 0; i < n_globals_g; i++ ) {
        fprintf(fp, "%s\n", globals[i]);
    }

    /* generate status */
    fprintf(fp, "/* Status for obj */\n");
    for ( i = 0; i < ep->n_status; i++ ) {
	fprintf(fp, "%s\n", ep->status[i]);
    }

    nggen_callback_proxy(fp, ep);

    /*================================================
                        stub main 
      ================================================*/

    /* main() when defined NGEX_NOT_REMOTE_EXECUTABLE */
    if (ep->backend == NG_BACKEND_MPI ||
        ep->backend == NG_BACKEND_BLACS ) {
        nggen_print_not_remote_executable_main(fp);
    }

    fprintf(fp, "\n/* Stub Main program */\n");
    fprintf(fp, "int main(int argc, char ** argv){\n");
    fprintf(fp, "\tint error;\n");
    fprintf(fp, "\tint result;\n");
    fprintf(fp, "\tint methodID;\n");
    fprintf(fp, "\tint immediateExit;\n");

    /* values for backend */
    switch(ep->backend) {
    case NG_BACKEND_MPI:	/* using MPI */
	fprintf(fp, "\tint ng_stub_rank;\n");
	fprintf(fp, "\tint result_tmp;\n");
	fprintf(fp, "\tint nprocs;\n");
	fprintf(fp, "\tint i;\n");
	break;
    case NG_BACKEND_BLACS:
	fprintf(fp, "\tint ictxt, nprocs, ng_stub_rank;\n");
	fprintf(fp, "\tint result_tmp;\n");
	fprintf(fp, "\tint i;\n");
	break;
    default:
	break;
    }

    fprintf(fp, "\n");

    fprintf(fp, "\timmediateExit = ngexStubAnalyzeArgumentWithExit(\n");
    fprintf(fp, "\t\targc, argv, &remoteClassInfo, &error);\n");
    fprintf(fp, "\tif (immediateExit != 0) {\n");
    fprintf(fp, "\t\texit(0);\n");
    fprintf(fp, "\t}\n\n");

    /* init backend */
    switch(ep->backend) {
    case NG_BACKEND_MPI:	/* using MPI */
	fprintf(fp, "\tMPI_Init(&argc, &argv);\n");
	fprintf(fp, "\tMPI_Comm_rank(MPI_COMM_WORLD, &ng_stub_rank);\n");
        fprintf(fp, "\tMPI_Comm_size(MPI_COMM_WORLD, &nprocs);\n\n");
	break;
    case NG_BACKEND_BLACS:
	fprintf(fp, "\tMPI_Init(&argc, &argv);\n");
	fprintf(fp, "\tCblacs_pinfo( &ng_stub_rank, &nprocs);\n");
	fprintf(fp, "\tCblacs_get( -1, 0, &ictxt);\n\n");
	break;
    default:
	break;
    }

    if ( ep->backend == NG_BACKEND_MPI ||
	 ep->backend == NG_BACKEND_BLACS) { /* using MPI */
        fprintf(fp, "\n\tresult = ngexStubInitializeMPI("
            "argc, argv, &remoteClassInfo, ng_stub_rank, &error);\n");
        fprintf(fp, "\tfor (i = 0;i < nprocs; ++i) {\n");
        fprintf(fp, "\t\tif (ng_stub_rank == i) {\n");
        fprintf(fp, "\t\t\tresult_tmp = result;\n");
        fprintf(fp, "\t\t}\n\n");
        fprintf(fp, "\t\tMPI_Bcast(&result_tmp, 1, MPI_INT , i, MPI_COMM_WORLD);\n");    
        fprintf(fp, "\t\tif (result_tmp == 0) {\n");
        fprintf(fp, "\t\t\tgoto ng_stub_end;\n");
        fprintf(fp, "\t\t}\n");
        fprintf(fp, "\t}\n\n");
    } else {
        fprintf(fp, "\n\tresult = ngexStubInitialize(argc, argv, &remoteClassInfo, &error);\n");
    }

    fprintf(fp, "\tif (result == 0) {\n");
    fprintf(fp, "\t\tgoto ng_stub_end;\n");
    fprintf(fp, "\t}\n\n");

    if ( ep->backend == NG_BACKEND_MPI ||
	 ep->backend == NG_BACKEND_BLACS) { /* using MPI */
        fprintf(fp, "\tif (ng_stub_rank == 0){\n");
    } 

    /** generate codes for initialization */
    nggen_special_handle(ep, fp, "_initialize");

    fprintf(fp, "\twhile(1) {\n");
    fprintf(fp, "\t\tresult = ngexStubGetRequest(&methodID, &error);\n");

    if ( ep->backend == NG_BACKEND_MPI ||
	 ep->backend == NG_BACKEND_BLACS) { /* using MPI */
        fprintf(fp, "\t\tMPI_Bcast(&result, 1, MPI_INT");
        fprintf(fp, ", 0, MPI_COMM_WORLD);\n");    
    }

    fprintf(fp, "\t\tif (result != NGEXI_INVOKE_METHOD) break;\n");

    if ( ep->backend == NG_BACKEND_MPI ||
	 ep->backend == NG_BACKEND_BLACS) { /* using MPI */
        fprintf(fp, "\t\tMPI_Bcast(&methodID, 1, MPI_INT");
        fprintf(fp, ", 0, MPI_COMM_WORLD);\n");    
    }

    fprintf(fp, "\t\tngexStubCalculationStart(&error);\n");

    /****** the switch *****/

    fprintf(fp, "\t\tswitch (methodID) {\n");
    for ( i = 0; i < ep->n_methods; i++) {
	fprintf(fp, "\t\tcase %d:\n", i);
	/* body */
	nggen_method_handle(&(ep->methods[i]),
          ep->language, ep->backend, fp, TRUE);
	fprintf(fp, "\t\tbreak;\n");
    }
    fprintf(fp, "\t\tdefault:\n");
    /* FIX ME: Is there any kind of ninf_error() solution */
    fprintf(fp, "\t\t\tfprintf(stderr, \"unknown method number\");\n");
    fprintf(fp, "\t\t}\n");

    fprintf(fp,"\t\tresult = ngexStubCalculationEnd(&error);\n");
    fprintf(fp, "\t\tif (result == 1){\n");
    fprintf(fp, "\t\t    continue;\n");    
    fprintf(fp, "\t\t} else {\n");    
    fprintf(fp, "\t\t    break;\n");    
    fprintf(fp, "\t\t}\n\t}\n");    

    /** generate codes for finalization */
    nggen_special_handle(ep, fp, "_finalize");

    /* using MPI */
    if (ep->backend == NG_BACKEND_MPI
	|| ep->backend == NG_BACKEND_BLACS){
      fprintf(fp,"} else {\n");

      /** generate codes for initialization */
      nggen_special_handle(ep, fp, "_initialize");

      fprintf(fp,"\twhile (1){\n");
      fprintf(fp,"\t\tMPI_Bcast(&result, 1, MPI_INT, 0, MPI_COMM_WORLD);\n");
      fprintf(fp,"\t\tif (result != NGEXI_INVOKE_METHOD) break;\n");
      fprintf(fp, "\t\tMPI_Bcast(&methodID, 1, MPI_INT");
      fprintf(fp, ", 0, MPI_COMM_WORLD);\n");    

      /****** the switch *****/

      fprintf(fp, "\t\tswitch (methodID) {\n");
      for ( i = 0; i < ep->n_methods; i++) {
          fprintf(fp, "\t\tcase %d:\n", i);
          /* body */
          nggen_method_handle(&(ep->methods[i]),
            ep->language, ep->backend, fp, FALSE);
          fprintf(fp, "\t\tbreak;\n");
      }
      fprintf(fp, "\t\tdefault:\n");
      /* FIX ME: Is there any kind of ninf_error() solution */
      fprintf(fp, "\t\t\tfprintf(stderr, \"unknown method number\");\n");
      fprintf(fp, "\t\t}\n");

      fprintf(fp,"\t}\n");

      /** generate codes for finalization */
      nggen_special_handle(ep, fp, "_finalize");

      fprintf(fp,"}\n\n");
    }

    fprintf(fp,"ng_stub_end:\n");
    fprintf(fp,"\tngexStubFinalize(&error);\n");

    /**  Backend-dependent finalization part  **/

    switch(ep->backend) {
    case NG_BACKEND_MPI:
	fprintf(fp,"\tMPI_Finalize();\n");
	break;
    case NG_BACKEND_BLACS:
	fprintf(fp, "\tCblacs_gridexit(ictxt);\n");
	fprintf(fp, "\tCblacs_exit();\n");
	break;
    default:
	break;
    }

    fprintf(fp,"\n");
    fprintf(fp,"\texit(0);\n");

    fprintf(fp, "}\n");

    if (ep->backend == NG_BACKEND_MPI ||
        ep->backend == NG_BACKEND_BLACS) {
        fprintf(fp, "#endif /* %s */\n", DUMP_INFO_TAG);
    }
    fprintf(fp, "/*                     */\n");
    fclose(fp);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_fortran_format                                             */
/*---------------------------------------------------------------------------*/
char * 
nggen_fortran_format(
    char *buf, 
    char *fformat, 
    char *fname
)
{
    char *answer = buf;

    if ( fformat == NULL ) {
        return fname;
    }

    while ( *fformat != '\0' ) {
        if ( *fformat != '%' ) {
            *buf++ = *fformat++;
            continue;
        } else {
            char * tmp;
            switch ( *(++fformat) ) {
                case 's':
                case 'S': 
                    tmp = fname;
                    while ( *tmp != '\0' ) {
                        *buf++ = *tmp++;
                    }
                    break;
                case 'l':
                case 'L': 
                    tmp = fname;
                    while ( *tmp != '\0' ) {
                        char tc = *tmp++;
                        *buf++ = toupper(tc);
                    }
                    break;
                default:
                    printf("unknown fortran format %%%c: ignore\n", *fformat);
            }
            fformat++;
        }
    }
    *buf = '\0';
    return answer;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_make_function_name                                         */
/*---------------------------------------------------------------------------*/
char * 
nggen_make_function_name(
    expr body_expr, 
    char *fformat
)
{
    if ( EXPR_ARG1(body_expr) != NULL ) {
        if ( strcasecmp(EXPR_STR(EXPR_ARG1(body_expr)), FORTRAN) == 0 ) {
            return nggen_fortran_format(function_name_buffer, fformat,
                SYM_NAME(EXPR_SYM(EXPR_ARG2(body_expr))));
        } 
    }
    return SYM_NAME(EXPR_SYM(EXPR_ARG2(body_expr)));
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_need_strlen                                                */
/*---------------------------------------------------------------------------*/
static int
nggen_need_strlen(char * name, struct method_gen_desc *ep){
  int i;
  struct param_gen_desc *dp;
  for (i = 0; i < ep->nparam; i++){
    dp = &ep->params[i];
    if (strcmp(SYM_NAME(dp->param_id), name) != 0)
      continue;
    if ((dp->ndim == 0 && dp->param_type == NG_ARGUMENT_DATA_TYPE_STRING) ||
        dp->param_type == NG_ARGUMENT_DATA_TYPE_FILENAME)
      return 1;
    else
      return 0;
  }
  return 0;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_need_and                                                   */
/*---------------------------------------------------------------------------*/
static int
nggen_need_and(
    char *name, 
    struct method_gen_desc *ep
)
{
    int i;
    struct param_gen_desc *dp;

    for ( i = 0; i < ep->nparam; i++ ) {
        dp = &ep->params[i];
        if ( strcmp(SYM_NAME(dp->param_id), name) != 0 ) { 
            continue;
        }
        if ( dp->param_type == NG_ARGUMENT_DATA_TYPE_CALLBACK ) {
            break;
        }
        return nggen_need_and_sub(ep->body_expr, dp->param_type, 
            dp->ndim, dp->param_inout);
    }
    return 0;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_need_and_sub                                               */
/*---------------------------------------------------------------------------*/
static int
nggen_need_and_sub(
    expr body_expr, 
    DATA_TYPE type, 
    int ndim, 
    ngArgumentIOmode_t param_inout
)
{
    int rc;

    
    if ( EXPR_ARG1(body_expr) == NULL ) {
        rc = 0;
    } else if ( strcasecmp(EXPR_STR(EXPR_ARG1(body_expr)), FORTRAN) != 0 ) {
        rc = 0;
    } else if ( ndim != 0 ) {
        rc = 0;
    } else if ( (param_inout & NG_ARGUMENT_IO_MODE_IN ) == 0 ) {
        rc = 0;
    } else  if ( type == NG_ARGUMENT_DATA_TYPE_STRING ) {
        rc = 0;
    } else  if ( type == NG_ARGUMENT_DATA_TYPE_FILENAME ) {
        rc = 0;
    } else {
        rc = 1;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_c_type_name                                                */
/*---------------------------------------------------------------------------*/
static char *
nggen_c_type_name(
    DATA_TYPE type
)
{
    char *rc;

    switch ( type ) {
        case NG_ARGUMENT_DATA_TYPE_CHAR:
            rc = "char";
            break;
        case NG_ARGUMENT_DATA_TYPE_SHORT:
             rc = "short";
            break;
        case NG_ARGUMENT_DATA_TYPE_INT:
            rc = "int";
            break;
        case NG_ARGUMENT_DATA_TYPE_LONG:
            rc = "long";
            break;
        case NG_ARGUMENT_DATA_TYPE_FLOAT:
            rc = "float";
            break;
        case NG_ARGUMENT_DATA_TYPE_DOUBLE:
            rc = "double";
            break;
        case NG_ARGUMENT_DATA_TYPE_STRING:
            rc = "char *";
            break;
        case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
            rc = "scomplex";
            /* rc = "float"; */
            break;
        case NG_ARGUMENT_DATA_TYPE_DCOMPLEX: 
            rc = "dcomplex";
            /* rc = "double";  */
            break;
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            rc = "char *";
            break;
        case NG_ARGUMENT_DATA_TYPE_CALLBACK:
            /*        rc = "char *";                 */
            rc = "char ";
            break;
        default:
            nggen_fatal("unknown type name %d\n", type);
            rc = "unknown";
            break;
    }
    return(rc);    
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_mpi_type_name                                              */
/*---------------------------------------------------------------------------*/
static char *
nggen_mpi_type_name(
    DATA_TYPE type
)
{
    char *rc;

    switch ( type ) {
        case NG_ARGUMENT_DATA_TYPE_CHAR:
            rc = "MPI_CHAR";
            break;
        case NG_ARGUMENT_DATA_TYPE_SHORT:
             rc = "MPI_SHORT";
            break;
        case NG_ARGUMENT_DATA_TYPE_INT:
            rc = "MPI_INT";
            break;
        case NG_ARGUMENT_DATA_TYPE_LONG:
            rc = "MPI_LONG";
            break;
        case NG_ARGUMENT_DATA_TYPE_FLOAT:
            rc = "MPI_FLOAT";
            break;
        case NG_ARGUMENT_DATA_TYPE_DOUBLE:
            rc = "MPI_DOUBLE";
            break;
        case NG_ARGUMENT_DATA_TYPE_STRING:
	    /* TODO: How do this? */
            rc = "MPI_CHAR";
            break;
        case NG_ARGUMENT_DATA_TYPE_SCOMPLEX:
	    /* TODO: How do this? */
            rc = "MPI_FLOAT";
            break;
        case NG_ARGUMENT_DATA_TYPE_DCOMPLEX: 
	    /* TODO: How do this? */
            rc = "MPI_DOUBLE";
            break;
        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            rc = "MPI_CHAR";
            break;
        case NG_ARGUMENT_DATA_TYPE_CALLBACK:
            rc = "MPI_CHAR ";
            break;
        default:
            nggen_fatal("unknown type name %d\n", type);
            rc = "unknown";
            break;
    }
    return(rc);    
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_print_expression                                           */
/*---------------------------------------------------------------------------*/
static void 
nggen_print_expression(
    FILE *fp,
    struct method_gen_desc *ep,
    expr x
)
{
    int j;


    if ( x == NULL ) {
        return; /* none */
    }

    switch ( EXPR_CODE(x) ) {
        case INT_CONSTANT:
            fprintf(fp, "%ld", EXPR_INT(x));
            break;

        case IDENT:
            /* search IN scalar parameter */
            for ( j = 0; j < ep->nparam; j++ ) {
                if ( ep->params[j].param_id == EXPR_SYM(x) ) { 
                    break;
                }
            }
            if ( j == ep->nparam ) {
                nggen_error_at_node(x, "input parameter is not found, %s",
                    SYM_NAME(EXPR_SYM(x)));
            } else if ( ep->params[j].ndim == 0 &&
                IS_IN_MODE(ep->params[j].param_inout)) {
                if (! IS_DISTMODE_BROADCAST(ep->params[j].param_distmode)) {
                  nggen_warning(
                    "Warning: parameter %s should be broadcast. It's used in expression.\n",
                    SYM_NAME(ep->params[j].param_id));
                }
                fprintf(fp, "%s", SYM_NAME(ep->params[j].param_id));
            } else {
                nggen_error_at_node(x,
                    "parameter in expression must be IN scalar, %s",
                    SYM_NAME(EXPR_SYM(x)));
            }
            break;
        /*===================
                "+"
        ===================*/
        case PLUS_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " + ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                "-"
        ===================*/
        case MINUS_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " - ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "*"
        ===================*/
        case MUL_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " * ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "^"
        ===================*/
        case POW_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " ^ ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "/"
        ===================*/
        case DIV_EXPR:      
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " / ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "%"
        ===================*/
        case MOD_EXPR:      
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " %% ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "=="
        ===================*/
        case EQ_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " == ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "!="
        ===================*/
        case NEQ_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " != ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 ">"
        ===================*/
        case GT_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " > ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "<"
        ===================*/
        case LT_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " < ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 ">="
        ===================*/
        case GE_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " >= ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "<="
        ===================*/
        case LE_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " <= ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, ")");
            break;
        /*===================
                 "?:"
        ===================*/
        case TRY_EXPR:
            fprintf(fp, "(");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, " ? ");
            nggen_print_expression(fp, ep, EXPR_ARG2(x));
            fprintf(fp, " : ");
            nggen_print_expression(fp, ep, EXPR_ARG3(x));
            fprintf(fp, ")");
            break;
        /*===================
               "unary-",
        ===================*/
        case UNARY_MINUS_EXPR:
            fprintf(fp, "(");
            fprintf(fp, "-");
            nggen_print_expression(fp, ep, EXPR_ARG1(x));
            fprintf(fp, ")");
            break;

        default:
            nggen_error_at_node(x, "expression is not supported yet, sorry!");
            break;
    }
}

