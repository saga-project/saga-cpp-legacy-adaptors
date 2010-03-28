#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: nggenCompile.c,v $ $Revision: 1.17 $ $Date: 2005/05/18 07:56:13 $";
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
#include "ngGenerator.h"
#include "y.tab.h"

struct stub_gen_entry *stubs_head;
struct stub_gen_entry *stubs_tail;
int n_stubs = 0;

char *current_module = NULL;
char *current_fortran_format = NULL;

#ifndef FORTRAN_STRING_CONVENTION
#define FORTRAN_STRING_CONVENTION FORTRAN_STRING_CONVENTION_LAST
#endif

int fortran_string_convention = FORTRAN_STRING_CONVENTION;

char *compile_options[MAX_COMPILE_OPTIONS];
char *compiler_name = NULL;
char *linker_name = NULL;

int n_options_g = 0;

char *globals[MAX_GLOBALS];
int n_globals_g = 0;

char *libs[MAX_LIBS];
int n_libs_g = 0;

/* static functions */
static void nggen_cpl_method(expr def_list, struct stub_gen_entry *ep,
    int method_index);
static void nggen_cpl_status(expr x, struct stub_gen_entry *ep);
static void nggen_cpl_interface_sub(expr def, expr desc, expr option_list,
    expr body, struct stub_gen_entry * ep, int method_index);
static void setup_options(struct stub_gen_entry *ep, expr backend, expr req,
    expr order, expr language, expr shrink, int method_index);
static void nggen_push_expression(int type, int val,
    NGGEN_EXPRESSION *expression, int *exp_index);
static VALUE_TYPE nggen_cpl_param_value(struct stub_gen_entry *ep,
    int method_index, expr x, int *v, NGGEN_EXPRESSION *expression,
    int *exp_index);
static VALUE_TYPE nggen_cpl_param_value_front(struct stub_gen_entry *ep, 
    int method_index, expr x, int *v, NGGEN_EXPRESSION *expression);
static int nggen_cpl_callback(struct param_gen_desc *dp, expr param);

static int nggen_cpl_param(struct param_gen_desc *dp, expr param);
static int nggen_cpl_param_decl(struct param_gen_desc *dp, int dim, expr decl);
static DATA_TYPE nggen_expr_to_type(expr);
static DATA_TYPE nggen_combine_type(expr, DATA_TYPE);

static void nggen_cpl_count_exp(struct stub_gen_entry *ep,
    int method_index, expr x, int *exp_index);
static int nggen_cpl_count_params(expr param);
static int nggen_cpl_count_dim_decl(struct param_gen_desc *dp,
    int dim, expr param);
static int nggen_cpl_count_dim(struct param_gen_desc *dp, int dim, expr param);
static void nggen_cpl_noexpression_init(NGGEN_EXPRESSION *exp);
static void nggen_cpl_expression_init(NGGEN_EXPRESSION *exp, int num_elems);
static struct stub_gen_entry* nggen_cpl_make_stub_gen_entry();
static struct method_gen_desc* nggen_cpl_make_method_gen_desc(int num_methods);
static char** nggen_cpl_make_status(int num_status);
static struct param_gen_desc* nggen_cpl_make_param_gen_desc(int num_params);
static struct dim_gen_desc* nggen_cpl_make_dim_gen_desc(int num_dims);

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_module                                                 */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_module(
    expr x
)
{
    if ( debug_flag_g ) {
        fprintf(stderr, "Module:");     /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( current_module != NULL ) { 
        nggen_error_at_node(x, "module name is already defined");
    } else {
        if (EXPR_CODE(x) == IDENT) { 
            current_module = SYM_NAME(EXPR_SYM(x));
        } else if ( EXPR_CODE(x) == STRING_CONSTANT ) {  
            current_module = EXPR_STR(x);
        } else {
            nggen_error_at_node(x, "illegal module name");
        }
    }
    return;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_fortranformat                                          */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_fortranformat(
    expr x
)
{
    if ( debug_flag_g ) { 
        printf("FortranFormat");        /* YES                               */
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) { 
        nggen_fatal("FORTRANFORMAT");
    }
    current_fortran_format = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_fortranstringconvention                                */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_fortranstringconvention(
    expr x
)
{
  char * name;
  if(EXPR_CODE(x) != IDENT)
    nggen_fatal("\"FortranStringConvention\" should be \"last\" or \"justAfter\"");

  name = SYM_NAME(EXPR_SYM(x));
  if (strcmp("last", name) == 0)
    fortran_string_convention = FORTRAN_STRING_CONVENTION_LAST;
  else if (strcmp("justAfter", name) == 0)
    fortran_string_convention = FORTRAN_STRING_CONVENTION_JUST_AFTER;
  else
    nggen_fatal("\"FortranStringConvention\" should be \"last\" or \"justAfter\"");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_compile_options                                        */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_compile_options(
    expr x
)
{
    if ( debug_flag_g ) {
        printf("CompileOptions:");      /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) {
        nggen_fatal("COMPILE_OPTIONS");
    }
    if ( n_options_g >= MAX_COMPILE_OPTIONS ) {
        nggen_fatal("too many CompileOptions");    /* YES                    */
    }
    compile_options[n_options_g++] = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_compiler                                               */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_compiler(
    expr x
)
{
    if ( debug_flag_g ) {
        printf("Compiler:");      /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) {
        nggen_fatal("COMPILER");
    }
    compiler_name = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_linker                                                 */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_linker(
    expr x
)
{
    if ( debug_flag_g ) {
        printf("Linker:");      /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) {
        nggen_fatal("LINKER");
    }
    linker_name = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_globals                                                */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_globals(
    expr x
)
{
    if ( debug_flag_g ) { 
        printf("Globals:");             /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) {
        nggen_fatal("GLBOALS");
    }
    if ( n_globals_g >= MAX_GLOBALS ) { 
        nggen_fatal("too many Globals");  
    }
    globals[n_globals_g++] = EXPR_STR(x); 
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_required                                               */
/*---------------------------------------------------------------------------*/
expr 
nggen_cpl_required(
    expr x
)
{
    return nggen_list2(LIST, nggen_make_enode(INT_CONSTANT, REQUIRED), x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_backend                                                */
/*---------------------------------------------------------------------------*/
expr 
nggen_cpl_backend(
    expr x
)
{
    return nggen_list2(LIST, nggen_make_enode(INT_CONSTANT, BACKEND), x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_shrink                                                 */
/*---------------------------------------------------------------------------*/
expr 
nggen_cpl_shrink(
    expr x
)
{
    return nggen_list2(LIST, nggen_make_enode(INT_CONSTANT, SHRINK), x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_language                                               */
/*---------------------------------------------------------------------------*/
expr 
nggen_cpl_language(
    expr x
)
{
    return nggen_list2(LIST, nggen_make_enode(INT_CONSTANT, LANGUAGE), x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_calcorder                                              */
/*---------------------------------------------------------------------------*/
expr 
nggen_cpl_calcorder(
    expr x
)
{
    return nggen_list2(LIST, nggen_make_enode(INT_CONSTANT, CALCORDER), x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_library                                                */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_library(
    expr x
)
{
    if ( debug_flag_g ) {
        printf("Library:");             /* YES                               */
        nggen_expr_print(x, stdout);
        printf("\n");
    }
    if ( EXPR_CODE(x) != STRING_CONSTANT ) {
        nggen_fatal("LIBRARY");
    }
    if ( n_options_g >= MAX_LIBS ) {
        nggen_fatal("too many LIBRARY");      /* YES                         */
    }
    libs[n_libs_g++] = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_interface_getoption                                    */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_interface_getoption(
    expr option_list, 
    expr *backendp, 
    expr *reqp,
    expr *orderp, 
    expr *languagep, 
    expr *shrinkp
)
{
    struct list_node *lp;

    if ( option_list == NULL ) {
        return;
    }
    lp = EXPR_LIST(option_list);

    while ( lp != NULL ) {
        expr tmp = LIST_ITEM(lp);
        if ( EXPR_CODE(tmp) != LIST ) {
            nggen_error_at_node(tmp, "internal error: expects list");
        }
        if ( EXPR_CODE(EXPR_ARG1(tmp)) != INT_CONSTANT ) { 
            nggen_error_at_node(tmp, "internal error: expects int");
        }
        switch ( EXPR_INT(EXPR_ARG1(tmp)) ) {
            case REQUIRED:
                *reqp = EXPR_ARG2(tmp);
                break;

            case BACKEND:
                *backendp = EXPR_ARG2(tmp);
                break;

            case CALCORDER:
                *orderp = EXPR_ARG2(tmp);
                break;

            case LANGUAGE:
                *languagep = EXPR_ARG2(tmp);
                break;

            case SHRINK:
                *shrinkp = EXPR_ARG2(tmp);
                break;

            default:
                nggen_error_at_node(tmp,
                    "internal error: unknown code");
        }
        lp = LIST_NEXT(lp);
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_interface                                              */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_interface(
    expr def,
    expr desc,
    expr option_list,
    expr body
)
{
    struct stub_gen_entry *ep;

    ep = nggen_cpl_make_stub_gen_entry();

    ep->ent_id = EXPR_SYM(EXPR_ARG1(def));

    /* init method_gen_desc */
    ep->methods = nggen_cpl_make_method_gen_desc(1);

    ep->methods[0].method_id = nggen_find_symbol(NGI_METHOD_NAME_DEFAULT);
    ep->n_methods = 1;
    nggen_cpl_interface_sub(def, desc, option_list, body, ep, 0);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_class                                                  */
/*---------------------------------------------------------------------------*/
void
nggen_cpl_class(expr name, expr desc_string,  expr option_list, expr def_list){
    expr backend = NULL;
    expr req = NULL;
    expr order = NULL;
    expr language = NULL;
    expr shrink = NULL;
    struct stub_gen_entry *ep;
    struct list_node *lp;      
    int method_index = 0;
    int num_status = 0, num_methods = 0;

    nggen_cpl_interface_getoption(option_list, &backend, &req, &order, 
				             &language, &shrink);

    ep = nggen_cpl_make_stub_gen_entry();

    ep->ent_id = EXPR_SYM(name);
    if (desc_string != NULL)
      ep->description = EXPR_STR(desc_string);

    if (def_list == NULL)  {
      nggen_error_at_node(def_list, "no definition in class");
      return;
    }
      
    lp = EXPR_LIST(def_list);
    /* init methods */
    while (lp != NULL){
      expr tmp = LIST_ITEM(lp);
      if (EXPR_CODE(tmp) == STRING_CONSTANT) /* status */
        ++num_status;
      else                     /* method_definition */
	++num_methods;
      lp = LIST_NEXT(lp);
    }
    if (num_methods <= 0) {
      fprintf(stderr, "no methods are defined in class.\n");
      return;
    }
    ep->n_methods = num_methods;
    ep->methods = nggen_cpl_make_method_gen_desc(num_methods);
    ep->status = nggen_cpl_make_status(num_status);

    lp = EXPR_LIST(def_list);
    while (lp != NULL){
      expr tmp = LIST_ITEM(lp);
      if (EXPR_CODE(tmp) == STRING_CONSTANT) /* status */
	nggen_cpl_status(tmp, ep);
      else                     /* method_definition */
	nggen_cpl_method(tmp, ep, method_index++);
      lp = LIST_NEXT(lp);
    }

    setup_options(ep, backend, req, order, language, shrink, 0);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_method                                                 */
/*---------------------------------------------------------------------------*/
static void 
nggen_cpl_method(expr def_list,
	       struct stub_gen_entry *ep,
	       int method_index)
{
  expr def, desc, option_list, body;
  def         = EXPR_ARG1(def_list);
  desc        = EXPR_ARG2(def_list);
  option_list = EXPR_ARG3(def_list);
  body        = EXPR_ARG4(def_list);

  ep->methods[method_index].method_id = EXPR_SYM(EXPR_ARG1(def));
  nggen_cpl_interface_sub(def, desc, option_list, body, ep, method_index);
}


/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_status                                                 */
/*---------------------------------------------------------------------------*/
static void 
nggen_cpl_status(expr x, struct stub_gen_entry *ep){
  if (EXPR_CODE(x) != STRING_CONSTANT) nggen_fatal("Status");
  ep->status[ep->n_status++] = EXPR_STR(x);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_interface_sub                                          */
/*---------------------------------------------------------------------------*/
static void
nggen_cpl_interface_sub(expr def, expr desc, expr option_list, expr body,
		      struct stub_gen_entry * ep, int method_index)
{

    list lp;                       
    int param_count = 0, n_param = 0;               
    struct param_gen_desc *dp;     
    struct dim_gen_desc *dimp;     
    int i;                         
    int k;                         
    expr x;                        

    expr backend = NULL;           
    expr req = NULL;               
    expr order = NULL;             
    expr language = NULL;          
    expr shrink = NULL;            

    nggen_cpl_interface_getoption(option_list, &backend, &req, &order, 
        &language, &shrink);
    if ( debug_flag_g ) { 
        printf("Define:");              /* YES                               */
        nggen_expr_print(def, stdout);
        printf("\n\tDesc:");
        nggen_expr_print(desc, stdout);
        printf("\n\trequired:");
        nggen_expr_print(req, stdout);
        printf("\n\tbody:");
        nggen_expr_print(body, stdout);
        printf("\n");
    }

    /* def=(LIST ident(LIST param1 ...)) */
    if ( EXPR_CODE(EXPR_ARG1(def)) != IDENT ) { 
        nggen_fatal("Define: not ident");
    }

    /* count number of parameters */
    FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(def)) {
	n_param += nggen_cpl_count_params(LIST_ITEM(lp));
    }
    /* init params */
    ep->methods[method_index].params = nggen_cpl_make_param_gen_desc(n_param);

    /* init dims */
    FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(def)) {
        /* set number of dims */
        param_count +=
            nggen_cpl_count_dim(&ep->methods[method_index].params[param_count],
                            0, LIST_ITEM(lp));
    }
    for ( i = 0; i < param_count; i++ ) {
        /* init dim_gen_desc */
        ep->methods[method_index].params[i].dim = nggen_cpl_make_dim_gen_desc(
            ep->methods[method_index].params[i].ndim);
    }
    /* reset counter */
    param_count = 0;

    FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(def)) {
        param_count += nggen_cpl_param(
			    &ep->methods[method_index].params[param_count],
			    LIST_ITEM(lp)); 
    }
    ep->methods[method_index].nparam = param_count;
    if ( desc != NULL ) {
        ep->methods[method_index].description = EXPR_STR(desc);
    }
    if ( req != NULL ) {
       ep->required = EXPR_STR(req);
    }

    if ( EXPR_CODE(body) == STRING_CONSTANT ) {
        ep->methods[method_index].body = EXPR_STR(body);
    } else {
        ep->methods[method_index].body_expr = body;
    }

    if (language != NULL) 
      ep->language = EXPR_STR(language);
    else if (ep->methods[method_index].body_expr != NULL && 
		EXPR_ARG1(ep->methods[method_index].body_expr) != NULL)
      ep->language = EXPR_STR(EXPR_ARG1(ep->methods[method_index].body_expr));
    else
      ep->language = NULL;

    /*================================
               Parameters 
      ================================*/ 

    for ( i = 0; i < param_count; i++ ) {
        dp = &ep->methods[method_index].params[i];
        if ( dp->param_id == NULL ) { 
            continue;                 
        }

        if ( IS_OUT_MODE(dp->param_inout) && dp->ndim == 0 &&
           dp->param_type != NG_ARGUMENT_DATA_TYPE_FILENAME ) { 

              nggen_error_at_node(def, "scalar out parameter, %s",
              SYM_NAME(dp->param_id));
              continue;
        }

        if ( IS_WORK_MODE(dp->param_inout) && dp->ndim == 0 ) { 
              nggen_error_at_node(def, "scalar work parameter, %s",
              SYM_NAME(dp->param_id));
              continue;
        }

        /* check the distmode */
        if (! (IS_IN_MODE(dp->param_inout)) &&
           dp->param_distmode == NGGEN_ARGUMENT_DIST_MODE_BROADCAST ) { 

              nggen_error_at_node(def, "broadcast out/work parameter, %s",
              SYM_NAME(dp->param_id));
              continue;
        }

        for ( k = 0; k < dp->ndim; k++ ) {
            dimp = &dp->dim[k];

            if ( dimp->size_type != NG_EXPRESSION_VALUE_TYPE_NONE ) {
                continue;               /* already set by someone */
            }

            nggen_cpl_param_value_front(ep, method_index,
		dimp->size_expr, &dimp->size, &dimp->size_exp);

            if ( dimp->size_exp.type[0] == NG_EXPRESSION_VALUE_TYPE_NONE ) {
                if ( k != 0 ) {
                    nggen_error_at_node(def,
                        "NULL dimension must be first, %s",
                        SYM_NAME(dp->param_id));
                }
                dimp->size_exp.type[0] = NG_EXPRESSION_VALUE_TYPE_CONSTANT;
                dimp->size_exp.val[0] = 1;
            }

            x = dimp->range_exprs;      /* process the range expressions */
            if ( x != NULL ) { 
                dimp->start_type =
                    nggen_cpl_param_value_front(ep, method_index, EXPR_ARG1(x),
                    &dimp->start, &dimp->start_exp);
                dimp->end_type =
                    nggen_cpl_param_value_front(ep, method_index, EXPR_ARG2(x),
                    &dimp->end, &dimp->end_exp);
                dimp->step_type =
                    nggen_cpl_param_value_front(ep, method_index, EXPR_ARG3(x),
                    &dimp->step, &dimp->step_exp);
            } else {
                nggen_cpl_noexpression_init(&dimp->start_exp);
                nggen_cpl_noexpression_init(&dimp->end_exp);
                nggen_cpl_noexpression_init(&dimp->step_exp);
            }
        }
    }

    setup_options(ep, backend, req, order, language, shrink, method_index);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = setup_options                                                    */
/*---------------------------------------------------------------------------*/
static void setup_options(
    struct stub_gen_entry *ep,
    expr backend,
    expr req,
    expr order,
    expr language,
    expr shrink,
    int method_index)
{
     NGGEN_EXPRESSION * order_p;
     int dum;
     
     if (backend != NULL){
       if ((strncasecmp(EXPR_STR(backend), "mpi", 4))== 0)
 	ep->backend = NG_BACKEND_MPI;
       else if ((strncasecmp(EXPR_STR(backend), "blacs", 4))== 0)
 	ep->backend = NG_BACKEND_BLACS;
       else
 	nggen_error_at_node(backend, "unsupported backend");
     } else
       ep->backend = NG_BACKEND_NORMAL;
 
     if (order != NULL){
       /* ep->methods[method_index].order_type = VALUE_BY_EXPR; */
       ep->methods[method_index].order_type = 5;
       order_p = &(ep->methods[method_index].order);
       nggen_cpl_param_value_front(ep, method_index, order, &dum, order_p);
     } else {
       ep->methods[method_index].order_type = NG_EXPRESSION_VALUE_TYPE_NONE;
       /* init expression */
       nggen_cpl_noexpression_init(&(ep->methods[method_index].order));
     }
 
     if (shrink != NULL &&
 	strncasecmp(EXPR_STR(shrink), "yes", 4) == 0)
       ep->shrink = TRUE;
     else
       ep->shrink = FALSE;
 
     if (req != NULL) ep->required = EXPR_STR(req);
}
 
/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_push_expression                                            */
/*---------------------------------------------------------------------------*/
static void
nggen_push_expression(
    int type, 
    int val, 
    NGGEN_EXPRESSION *expression, 
    int *exp_index
)
{
    expression->type[*exp_index] = (ngExpressionValueType_t) type;
    expression->val [*exp_index] = val;
    (*exp_index)++;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_param_value                                            */
/*---------------------------------------------------------------------------*/
static VALUE_TYPE 
nggen_cpl_param_value(
    struct stub_gen_entry *ep,
    int method_index,
    expr x, 
    int *v, 
    NGGEN_EXPRESSION *expression,
    int *exp_index
)
{
    int j;
    int v1, v2, v3, t1, t2, t3;
    VALUE_TYPE rc = NG_EXPRESSION_VALUE_TYPE_NONE;


    if ( x == NULL ) {
        return(NG_EXPRESSION_VALUE_TYPE_NONE); /* none */
    }

    switch ( EXPR_CODE(x) ) {
        case INT_CONSTANT:
            *v = EXPR_INT(x);
            rc = NG_EXPRESSION_VALUE_TYPE_CONSTANT;
            break;

        case IDENT:
            /* search IN scalar parameter */
            for ( j = 0; j < ep->methods[method_index].nparam; j++ ) {
                if ( ep->methods[method_index].params[j].param_id ==
                     EXPR_SYM(x) ) { 
                    break;
                }
            }
            if ( j == ep->methods[method_index].nparam ) {
                nggen_error_at_node(x, "input parameter is not found, %s",
                    SYM_NAME(EXPR_SYM(x)));
            } else if ( ep->methods[method_index].params[j].ndim == 0 &&
                IS_IN_MODE(ep->methods[method_index].params[j].param_inout) ) {
                *v = j;
                return(NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT);
            } else {
                nggen_error_at_node(x,
                    "parameter in expression must be IN scalar, %s",
                    SYM_NAME(EXPR_SYM(x)));
            }
            rc = (VALUE_TYPE) NG_EXPRESSION_VALUE_TYPE_ERROR;
            break;
        /*===================
                "+"
        ===================*/
        case PLUS_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_PLUS;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                "-"
        ===================*/
        case MINUS_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_MINUS;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "*"
        ===================*/
        case MUL_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_MULTIPLY;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "^"
        ===================*/
        case POW_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_POWER;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "/"
        ===================*/
        case DIV_EXPR:      
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_DIVIDE;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "%"
        ===================*/
        case MOD_EXPR:      
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
                *v = NG_EXPRESSION_OPCODE_MODULO;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "=="
        ===================*/
        case EQ_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_EQUAL;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "!="
        ===================*/
        case NEQ_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_NOT_EQUAL;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 ">"
        ===================*/
        case GT_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_GREATER_THAN;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "<"
        ===================*/
        case LT_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_LESS_THAN;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 ">="
        ===================*/
        case GE_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_GREATER_EQUAL;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "<="
        ===================*/
        case LE_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);      
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);      
            *v = NG_EXPRESSION_OPCODE_LESS_EQUAL;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
                 "?:"
        ===================*/
        case TRY_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);
            t2 = nggen_cpl_param_value(ep, method_index, EXPR_ARG2(x), 
                &v2, expression, exp_index) ;
            nggen_push_expression(t2, v2, expression, exp_index);
            t3 = nggen_cpl_param_value(ep, method_index, EXPR_ARG3(x), 
                &v3, expression, exp_index) ;
            nggen_push_expression(t3, v3, expression, exp_index);
            *v = NG_EXPRESSION_OPCODE_TRI;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;
        /*===================
               "unary-",
        ===================*/
        case UNARY_MINUS_EXPR:
            t1 = nggen_cpl_param_value(ep, method_index, EXPR_ARG1(x), 
                &v1, expression, exp_index) ;
            nggen_push_expression(t1, v1, expression, exp_index);
            *v = NG_EXPRESSION_OPCODE_UNARY_MINUS;
            rc = NG_EXPRESSION_VALUE_TYPE_OPCODE;
            break;

        default:
            nggen_error_at_node(x, "expression is not supported yet, sorry!");
            rc = (VALUE_TYPE) NG_EXPRESSION_VALUE_TYPE_ERROR;
            break;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_param_value_front                                      */
/*---------------------------------------------------------------------------*/
static VALUE_TYPE 
nggen_cpl_param_value_front(
    struct stub_gen_entry *ep, 
    int method_index,
    expr x, 
    int *v, 
    NGGEN_EXPRESSION *expression
)
{
    VALUE_TYPE tmp_t;
    int exp_index = 0;
    int num_elems = 0;

    /* count number of elements */
    nggen_cpl_count_exp(ep, method_index, x, &num_elems);
    /* init expression */
    nggen_cpl_expression_init(expression, num_elems);

    tmp_t = nggen_cpl_param_value(ep, method_index, x, v,
        expression, &exp_index);
    nggen_push_expression(tmp_t, *v, 
        expression, &exp_index);
    nggen_push_expression(NG_EXPRESSION_VALUE_TYPE_END, 0,
        expression, &exp_index);
    return tmp_t;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_callback                                               */
/*---------------------------------------------------------------------------*/
static int 
nggen_cpl_callback(
    struct param_gen_desc *dp,
    expr param
)
{
    list lp;

    int index = 1;

    dp->param_type = NG_ARGUMENT_DATA_TYPE_CALLBACK;
    dp->param_id = EXPR_SYM(EXPR_ARG1(param));
    FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(param)) {
        index++;
        nggen_cpl_param(++dp, LIST_ITEM(lp));
    }
    return index;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_param                                                  */
/*---------------------------------------------------------------------------*/
static int 
nggen_cpl_param(
    struct param_gen_desc *dp,
    expr param
)
{ 
    expr mode;
    expr distmode;
    expr type;
    int rc;

    rc = 0;
    if ( param != NULL ) {
        if ( EXPR_CODE(param) == CALLBACK_FUNC ) { 
            rc = nggen_cpl_callback(dp, param);
        } else {
            distmode = EXPR_ARG2(EXPR_ARG1(EXPR_ARG1(param)));
            if ( distmode != NULL ) {
                dp->param_distmode = EXPR_DISTMODE_SPEC(distmode);
            }

            mode = EXPR_ARG1(EXPR_ARG1(EXPR_ARG1(param)));
            if ( mode != NULL ) {
                dp->param_inout = EXPR_MODE_SPEC(mode);
            }

            type = EXPR_ARG2(EXPR_ARG1(param));
            if ( type == NULL ) {
                nggen_error_at_node(param, "no parameter type");
            } else {
                dp->param_type = nggen_expr_to_type(type);
            }
            dp->ndim = nggen_cpl_param_decl(dp, 0, EXPR_ARG2(param));
            rc = 1;
        }
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_param_decl                                             */
/*---------------------------------------------------------------------------*/
static int
nggen_cpl_param_decl(
   struct param_gen_desc *dp,
   int dim,
   expr decl
)
{
    int rc;


    switch ( EXPR_CODE(decl) ) {
        case IDENT:
            dp->param_id = EXPR_SYM(decl);
            rc = dim;                       
            break;
        case ARRAY_REF:
            dp->dim[dim].size_expr = EXPR_ARG2(decl);
            dp->dim[dim].range_exprs = EXPR_ARG3(decl);
            rc = nggen_cpl_param_decl(dp, dim + 1, EXPR_ARG1(decl));
            break;
        case POINTER_REF: /* POINTER_REF is same as size 1 array */
            dp->dim[dim].size_type = NG_EXPRESSION_VALUE_TYPE_CONSTANT;
            dp->dim[dim].size = 1;

            {
                int ndx = 0;

                /* init expression */
                nggen_cpl_expression_init(&dp->dim[dim].size_exp, 0);
                nggen_cpl_noexpression_init(&dp->dim[dim].start_exp);
                nggen_cpl_noexpression_init(&dp->dim[dim].end_exp);
                nggen_cpl_noexpression_init(&dp->dim[dim].step_exp);

                nggen_push_expression(NG_EXPRESSION_VALUE_TYPE_CONSTANT, 1, 
                    &dp->dim[dim].size_exp, &ndx);
                nggen_push_expression(NG_EXPRESSION_VALUE_TYPE_END, 0, 
                    &dp->dim[dim].size_exp, &ndx);
            }

            if ( EXPR_CODE(EXPR_ARG1(decl)) != IDENT ) {
                nggen_error_at_node(decl, "bad pointer declaration");
            }
            rc = nggen_cpl_param_decl(dp, dim + 1, EXPR_ARG1(decl));
            break;
        default: /* !!! NOTE: other declaration is not implemented yet !!! */
            nggen_error_at_node(decl, "unkonwn declarator");
            rc = 0;
            break;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_expr_to_type                                               */
/*---------------------------------------------------------------------------*/
static DATA_TYPE 
nggen_expr_to_type(
    expr x
)
{
    DATA_TYPE rc_data_type = NG_ARGUMENT_DATA_TYPE_UNDEFINED;

    if ( EXPR_CODE(x) == BASIC_TYPE_NODE ) {
          rc_data_type = EXPR_TYPE(x);
    } else if ( EXPR_CODE(x) == LIST ) {
          rc_data_type = nggen_combine_type(EXPR_ARG1(x), 
               nggen_expr_to_type(EXPR_ARG2(x)));
    } else {
        nggen_fatal("illegal type");
    }
    return(rc_data_type);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_combine_type                                               */
/*---------------------------------------------------------------------------*/
static DATA_TYPE 
nggen_combine_type(
    expr x, 
    DATA_TYPE t2
)
{
    DATA_TYPE t1, t;
    
    if ( EXPR_CODE(x) != BASIC_TYPE_NODE ) {
        nggen_error_at_node(x, "illegal type combination");
        t = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
        return(t);
    }
    t1 = EXPR_TYPE(x);
    t = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
    if ( t1 == NG_ARGUMENT_DATA_TYPE_UNDEFINED ||
         t2 == NG_ARGUMENT_DATA_TYPE_UNDEFINED ) {
        return(t);  /* don't care */
    }
    switch ( t1 ) {
        case NG_ARGUMENT_DATA_TYPE_LONG:
            switch ( t2 ) {
                case NG_ARGUMENT_DATA_TYPE_FLOAT:
                    t = NG_ARGUMENT_DATA_TYPE_DOUBLE;
                    break;
                case NG_ARGUMENT_DATA_TYPE_INT:
                    t = NG_ARGUMENT_DATA_TYPE_LONG; 
                    break;
                default: 
                    nggen_error_at_node(x, "illegal type combination");
                    t = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
                    break;
                }
                break;
            case NG_ARGUMENT_DATA_TYPE_SHORT:
            switch ( t2 ) {
                case NG_ARGUMENT_DATA_TYPE_INT:
                    t = NG_ARGUMENT_DATA_TYPE_SHORT; 
                    break;
                default: 
                    nggen_error_at_node(x, "illegal type combination");
                    t = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
                    break;
            }
        break;

        case NG_ARGUMENT_DATA_TYPE_FILENAME:   /* FILENAME      */
            t = NG_ARGUMENT_DATA_TYPE_FILENAME;
            break;

        default: 
            nggen_error_at_node(x, "illegal type combination");
            t = NG_ARGUMENT_DATA_TYPE_UNDEFINED;
            break;
    }
    return(t);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_dump_stubs                                                 */
/*---------------------------------------------------------------------------*/
void
nggen_dump_stubs(
    void
)
{
    struct stub_gen_entry *ep;
    struct param_gen_desc *dp;
    int j, k, mi;

    printf("/******************************************/\n");
    printf("/*           function structure           */\n");
    printf("/******************************************/\n");
    printf("Module: %s , #entries : %d\n",current_module, n_stubs);

    for ( ep = stubs_head; ep != NULL; ep = ep->next_stub ) {
        printf("\n*** entry name = %s\n", SYM_NAME(ep->ent_id));
	for ( mi = 0; mi < ep->n_methods; mi++ ) {
	    for ( j = 0; j < ep->methods[mi].nparam; j++ ) {
		dp = &ep->methods[mi].params[j];
		printf("param(%d)= %s %s:%s %s", j,
		    nggen_basic_type_name(dp->param_type),
		    SYM_NAME(dp->param_id),
		    nggen_mode_spec_name(dp->param_inout),
		    nggen_distmode_spec_name(dp->param_distmode));
		printf("\n");

		for ( k = 0; k < dp->ndim; k++ ) {
		    printf("\t%d:", k);
		    nggen_expr_print(dp->dim[k].size_expr, stdout);
		    if ( dp->dim[k].range_exprs ) {
			printf("\t\t+");
			nggen_expr_print(dp->dim[k].range_exprs, stdout);
		    }
		}
	    }
	    if ( ep->methods[mi].body != NULL ) {
		printf("body = %s\n", ep->methods[mi].body);
	    }

	    if ( ep->methods[mi].description != NULL ) {
		printf("description = %s\n", ep->methods[mi].description);
	    }
	}
	if ( ep->required != NULL ) {
	    printf("required = %s\n", ep->required);
	}
	if ( ep->description != NULL ) {
	    printf("description = %s\n", ep->description);
	}
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_count_exp                                              */
/*---------------------------------------------------------------------------*/
static void 
nggen_cpl_count_exp(
    struct stub_gen_entry *ep,
    int method_index,
    expr x, 
    int *exp_index
)
{
    int j;

    if ( x == NULL ) {
        return; /* none */
    }

    switch ( EXPR_CODE(x) ) {
        case INT_CONSTANT:
            break;

        case IDENT:
            /* search IN scalar parameter */
            for ( j = 0; j < ep->methods[method_index].nparam; j++ ) {
                if ( ep->methods[method_index].params[j].param_id ==
                     EXPR_SYM(x) ) { 
                    break;
                }
            }
            if ( j == ep->methods[method_index].nparam ) {
                nggen_error_at_node(x, "input parameter is not found, %s",
                    SYM_NAME(EXPR_SYM(x)));
            } else if ( ep->methods[method_index].params[j].ndim == 0 &&
                IS_IN_MODE(ep->methods[method_index].params[j].param_inout) ) {
                return;
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
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                "-"
        ===================*/
        case MINUS_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "*"
        ===================*/
        case MUL_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "^"
        ===================*/
        case POW_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "/"
        ===================*/
        case DIV_EXPR:      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "%"
        ===================*/
        case MOD_EXPR:      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "=="
        ===================*/
        case EQ_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "!="
        ===================*/
        case NEQ_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 ">"
        ===================*/
        case GT_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "<"
        ===================*/
        case LT_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 ">="
        ===================*/
        case GE_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "<="
        ===================*/
        case LE_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
                 "?:"
        ===================*/
        case TRY_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG2(x), exp_index);
            (*exp_index)++;      
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG3(x), exp_index);
            (*exp_index)++;      
            break;
        /*===================
               "unary-",
        ===================*/
        case UNARY_MINUS_EXPR:
            nggen_cpl_count_exp(ep, method_index, EXPR_ARG1(x), exp_index);
            (*exp_index)++;      
            break;

        default:
            nggen_error_at_node(x, "expression is not supported yet, sorry!");
            break;
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_count_params                                           */
/*---------------------------------------------------------------------------*/
static int
nggen_cpl_count_params(
    expr param
)
{
    list lp;                       

    if ( param != NULL ) {
        if ( EXPR_CODE(param) == CALLBACK_FUNC ) { 
            int index = 1;
            FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(param)) {
                index++;
            }
            return index;
        } else {
            return 1;
        }
    }
    return 0;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_count_dim_decl                                         */
/*---------------------------------------------------------------------------*/
static int
nggen_cpl_count_dim_decl(
    struct param_gen_desc *dp,
    int dim,
    expr param
)
{
    int ndim = 0;

    switch ( EXPR_CODE(param) ) {
        case IDENT:
            ndim = dim;
            break;
        case ARRAY_REF:
            ndim = nggen_cpl_count_dim_decl(dp, dim + 1, EXPR_ARG1(param));
            break;
        case POINTER_REF: /* POINTER_REF is same as size 1 array */
            if ( EXPR_CODE(EXPR_ARG1(param)) != IDENT ) {
                nggen_error_at_node(param, "bad pointer declaration");
            }
            ndim = nggen_cpl_count_dim_decl(dp, dim + 1, EXPR_ARG1(param));
            break;
        default: /* !!! NOTE: other declaration is not implemented yet !!! */
            nggen_error_at_node(param, "unkonwn declarator");
            ndim = 0;
            break;
    }

    return(ndim);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_count_dim                                              */
/*---------------------------------------------------------------------------*/
static int
nggen_cpl_count_dim(
    struct param_gen_desc *dp,
    int dim,
    expr param
)
{
    list lp;

    if ( param != NULL ) {
        if ( EXPR_CODE(param) == CALLBACK_FUNC ) {
            int index = 1;
            FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(param)) {
                index++;
                nggen_cpl_count_dim(++dp, 0, LIST_ITEM(lp));
            }
            return index;
        } else {
            dp->ndim = nggen_cpl_count_dim_decl(dp, 0, EXPR_ARG2(param));
            return 1;
        }
    }
    return 0;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_noexpression_init                                      */
/*---------------------------------------------------------------------------*/
static void nggen_cpl_noexpression_init(NGGEN_EXPRESSION *exp) {
    exp->type = malloc(sizeof(VALUE_TYPE));
    exp->val = malloc(sizeof(int));
    if ((exp->type == NULL) || (exp->val == NULL)) {
        fprintf(stderr, "failed to malloc expression.\n");
        exit(1);
    }
    exp->type[0] = NG_EXPRESSION_VALUE_TYPE_NONE;
    exp->val[0] = 0;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_expression_init                                        */
/*---------------------------------------------------------------------------*/
static void nggen_cpl_expression_init(NGGEN_EXPRESSION *exp, int num_elems) {
    exp->type = malloc(sizeof(VALUE_TYPE) * (num_elems + 2));
    exp->val = malloc(sizeof(int) * (num_elems + 2));
    if ((exp->type == NULL) || (exp->val == NULL)) {
        fprintf(stderr, "failed to malloc expression.\n");
        exit(1);
    }
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_make_stub_gen_entry                                    */
/*---------------------------------------------------------------------------*/
static struct stub_gen_entry*
nggen_cpl_make_stub_gen_entry() {
    struct stub_gen_entry* ep;

    /* alloc and init new stub_gen_entry */
    ep = (struct stub_gen_entry *)malloc(sizeof(struct stub_gen_entry));
    if (ep == NULL) {
        fprintf(stderr, "can't alloc new stub_gen_entry.\n");
        exit(1);
    }
    memset(ep, 0x0, sizeof(struct stub_gen_entry));

    /* append to list */
    if (stubs_tail != NULL) {
        stubs_tail->next_stub = ep;
        stubs_tail = ep;
    } else if (stubs_head == NULL) {
        stubs_head = ep;
        stubs_tail = ep;
    } else {
        fprintf(stderr, "something wrong was happened.\n");
        exit(1);
    }
    ep->next_stub = NULL;

    /* return new stub_gen_entry */
    return ep;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_make_method_gen_desc                                   */
/*---------------------------------------------------------------------------*/
static struct method_gen_desc* nggen_cpl_make_method_gen_desc(int num_methods) {
    /* alloc and init new method_gen_desc */
    struct method_gen_desc* dp;

    /* Is num_methods smaller equal zero? */
    if (num_methods <= 0) {
      /* Success */
      return NULL;
    }

    dp = (struct method_gen_desc *)
        malloc(sizeof(struct method_gen_desc) * num_methods);
    if (dp == NULL) {
        fprintf(stderr, "can't malloc method_gen_desc.\n");
        exit(1);
    }
    memset(dp, 0x0, sizeof(struct method_gen_desc) * num_methods);

    /* return new method_gen_desc */
    return dp;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_make_status                                            */
/*---------------------------------------------------------------------------*/
static char** nggen_cpl_make_status(int num_status) {
    /* alloc and init new status */
    char **status;

    /* Is num_status smaller equal zero? */
    if (num_status <= 0) {
      /* Success */
      return NULL;
    }

    status = (char **)malloc(sizeof(char *) * num_status);
    if (status == NULL) {
        fprintf(stderr, "can't malloc status.\n");
        exit(1);
    }
    memset(status, 0x0, sizeof(char *) * num_status);

    /* return new status */
    return status;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_make_param_gen_desc                                    */
/*---------------------------------------------------------------------------*/
static struct param_gen_desc* nggen_cpl_make_param_gen_desc(int num_params) {
    /* alloc and init new param_gen_desc */
    struct param_gen_desc* dp;

    /* Is num_params smaller equal zero? */
    if (num_params <= 0) {
      /* Success */
      return NULL;
    }

    dp = (struct param_gen_desc *)
        malloc(sizeof(struct param_gen_desc) * num_params);
    if (dp == NULL) {
        fprintf(stderr, "can't malloc param_gen_desc.\n");
        exit(1);
    }
    memset(dp, 0x0, sizeof(struct param_gen_desc) * num_params);

    /* return new param_gen_desc */
    return dp;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_cpl_make_dim_gen_desc                                      */
/*---------------------------------------------------------------------------*/
static struct dim_gen_desc* nggen_cpl_make_dim_gen_desc(int num_dims) {
    /* alloc and init new dim_gen_desc */
    struct dim_gen_desc* dp;

    /* Is num_dims smaller equal zero? */
    if (num_dims <= 0) {
	/* Success */
	return NULL;
    }

    dp = (struct dim_gen_desc *)
        malloc(sizeof(struct dim_gen_desc) * num_dims);
    if (dp == NULL) {
        fprintf(stderr, "can't malloc dim_gen_desc.\n");
        exit(1);
    }
    memset(dp, 0x0, sizeof(struct dim_gen_desc) * num_dims);

    /* return new dim_gen_desc */
    return dp;
}

