/* 
 * $RCSfile: ngGenerator.h,v $ $Revision: 1.14 $ $Date: 2006/02/10 11:14:02 $
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

/* include file for header */
#ifndef _NGGENERATOR_H_
#define _NGGENERATOR_H_

#include "ng.h"
#include "ngFunctionInformation.h"

/* IDL */
enum expr_code {
    ERROR_NODE = 0,
    LIST = 1,
    IDENT = 2,
    STRING_CONSTANT = 3,
    INT_CONSTANT = 4,
    LONG_CONSTANT = 5,
    FLOAT_CONSTANT = 6,
    BASIC_TYPE_NODE = 7,
    MODE_SPEC_NODE = 8,
    DISTMODE_SPEC_NODE = 9,

    PLUS_EXPR = 35,
    MINUS_EXPR = 37,
    UNARY_MINUS_EXPR = 39,
    MUL_EXPR = 40,
    POW_EXPR = 41,
    DIV_EXPR = 42,
    MOD_EXPR = 44,

    EQ_EXPR = 50,
    NEQ_EXPR = 51,
    GT_EXPR = 52,
    LT_EXPR = 53,
    GE_EXPR = 54,
    LE_EXPR = 55,
    TRY_EXPR = 56,

    POINTER_REF = 66,
    ARRAY_REF = 67,
    CALLBACK_FUNC = 68
};

enum symbol_type 
{
    S_IDENT=0, /* default */
    S_TYPE,
    S_CLASS,
    S_DISTMODE,
    S_KEYWORD
};

/* symbol and symbol table */
typedef struct symbol
{
    struct symbol *s_next;          /* backet chain */
    char *s_name;
    enum symbol_type s_type/*:8*/;  /* symbol type, KEYWORD, NAME, etc ... */
    short int s_value;
} * SYMBOL;

/* de-syntax program is represented by this data structure. */
typedef struct expression_node
{
    enum expr_code e_code/*:8*/;
    short int e_lineno;     /* line number this node created */
    union 
    {
        struct list_node *e_lp;
        SYMBOL e_sym;     /* e_code == NAME, TYPE, ...  */
        char *e_str;      /* e_code == STRING */
        long int e_ival;  /* e_code == INT_CONSTANT */
        double *e_fval;   /* e_code == FLOAT_CONSTANT */
    } v;
} * expr;

#define MAX_CODE 128
#define SYM_NAME(sp) ((sp)->s_name)
#define EXPR_CODE(x) ((x)->e_code) 
#define EXPR_SYM(x) ((x)->v.e_sym)
#define EXPR_STR(x) ((x)->v.e_str)
#define EXPR_FLOAT(x) (*((x)->v.e_fval))
#define EXPR_LIST(x) ((x)->v.e_lp)
#define EXPR_INT(x) ((x)->v.e_ival)
#define EXPR_TYPE(x) ((DATA_TYPE)((x)->v.e_ival))
#define EXPR_MODE_SPEC(x) ((MODE_SPEC)((x)->v.e_ival))
#define EXPR_DISTMODE_SPEC(x) ((DISTMODE_SPEC)((x)->v.e_ival))
#define EXPR_LINENO(x) ((x)->e_lineno)

/* list data structure, which is ended with NULL */
typedef struct list_node
{
    struct list_node *l_next;
    expr l_item;
} *list;

#define LIST_NEXT(lp) ((lp)->l_next)
#define LIST_ITEM(lp) ((lp)->l_item)
#define FOR_ITEMS_IN_LIST(lp, x) \
  if(x != NULL) for(lp = EXPR_LIST(x); lp != NULL ; lp = LIST_NEXT(lp))

#define EXPR_ARG1(x) LIST_ITEM(EXPR_LIST(x))
#define EXPR_ARG2(x) LIST_ITEM(LIST_NEXT(EXPR_LIST(x)))
#define EXPR_ARG3(x) LIST_ITEM(LIST_NEXT(LIST_NEXT(EXPR_LIST(x))))
#define EXPR_ARG4(x) LIST_ITEM(LIST_NEXT(LIST_NEXT(LIST_NEXT(EXPR_LIST(x)))))

#define EXPR_CODE_NAME(code) expr_code_info[code].code_name
#define EXPR_CODE_SYMBOL(code) expr_code_info[code].operator_name

/* definitions from "nslib_macros.h" */
#define NG_BUFSIZ	1024

/* definition for common */
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define THIS_INFO_TYPE 0
#define MAX_COMPILE_OPTIONS 100
#define MAX_GLOBALS 100
#define MAX_LIBS 100
#define STR_EQ(s1, s2) (strcmp(s1, s2) == 0)
#define VALUE_TYPE ngExpressionValueType_t
#define DATA_TYPE ngArgumentDataType_t
#define MODE_SPEC ngArgumentIOmode_t
#define DISTMODE_SPEC nggenArgumentDistmode_t
#define NGGEN_EXPRESSION nggen_expression_t
#define IS_IN_MODE(x) ((int)(x)&1)
#define IS_OUT_MODE(x) ((int)(x)&2)
#define IS_WORK_MODE(x) ((int)(x)&4)
#define IS_DISTMODE_ALLOCATE(x) ((int)(x)&1)
#define IS_DISTMODE_BROADCAST(x) ((int)(x)&2)
#define FORTRAN_STRING_CONVENTION_LAST          0
#define FORTRAN_STRING_CONVENTION_JUST_AFTER    1

/* expression of Ninf-G */
typedef struct nggen_expression 
{
  VALUE_TYPE *type;
  int *val;
} nggen_expression_t;

/* dimension of parameters */
struct dim_gen_desc{
    expr size_expr;          /* size */
    expr range_exprs;        /* range */
    VALUE_TYPE size_type;
    int size;       
    NGGEN_EXPRESSION size_exp;
    VALUE_TYPE start_type;
    int start;
    NGGEN_EXPRESSION start_exp;
    VALUE_TYPE end_type;
    int end;
    NGGEN_EXPRESSION end_exp;
    VALUE_TYPE step_type;
    int step;
    NGGEN_EXPRESSION step_exp;
};

/* distmode */
typedef enum nggenArgumentDistmode_e {
  NGGEN_ARGUMENT_DIST_MODE_NORMAL = 0,
  NGGEN_ARGUMENT_DIST_MODE_ALLOCATE = 1,
  NGGEN_ARGUMENT_DIST_MODE_BROADCAST = 2
} nggenArgumentDistmode_t;

/* information of parameters */
struct param_gen_desc{
    enum ngArgumentDataType_e param_type;      /* argument type */
    enum ngArgumentIOmode_e param_inout;     /* IN/OUT */
    enum nggenArgumentDistmode_e param_distmode;     /* allocate/broadcast */
    SYMBOL param_id;                      /* parameter name */
    int ndim;                             /* number of dimension */
    struct dim_gen_desc *dim;
};

/* information of methods */
struct method_gen_desc {
  SYMBOL method_id;
  int nparam;
  struct param_gen_desc *params;
  char *body;
  expr body_expr;
  char *description;
  int order_type;
  NGGEN_EXPRESSION order;
};

/* information of class */
struct stub_gen_entry 
{
    struct stub_gen_entry* next_stub;

    SYMBOL ent_id;                      /* entry name */

    int n_methods;

    struct method_gen_desc *methods;
    char *language;
    char *required;
    char *description;
    int backend;
    int shrink;

    char **status;
    int n_status;
};

/* functions */
/* nggenIdlLex.c */
expr nggen_read_rest_of_body(int);

/* nggenCompile.c */
void nggen_cpl_module(expr x);
void nggen_cpl_fortranformat(expr x);
void nggen_cpl_fortranstringconvention(expr x);
void nggen_cpl_compile_options(expr x);
void nggen_cpl_compiler(expr x);
void nggen_cpl_linker(expr x);
void nggen_cpl_globals(expr x);
void nggen_cpl_library(expr x);
void nggen_cpl_interface(expr def, expr desc, expr option_list, expr body);
expr nggen_cpl_required(expr x);
expr nggen_cpl_backend(expr x);
expr nggen_cpl_shrink(expr x);
expr nggen_cpl_language(expr x);
expr nggen_cpl_calcorder(expr x);
void nggen_cpl_class(expr, expr, expr, expr);

/* nggenMisc.c */
void nggen_expr_print(expr x, FILE *fp);
void nggen_error_at_node(expr x, char *fmt, ...);
void nggen_fatal(char *fmt, ...);
char *nggen_basic_type_name(DATA_TYPE type);
char *nggen_mode_spec_name(MODE_SPEC mode);
char *nggen_distmode_spec_name(DISTMODE_SPEC distmode);
void nggen_dump_stubs(void);
void nggen_init_expr_code_info(void);

/* nggenMain.c */
int yyparse(void);
void nggen_error(char *fmt, ...);
void nggen_fatal(char *fmt, ...);
void nggen_warning(char *fmt, ...);

/* nggenIdlLex.c */
void nggen_initialize_lex(void);
void nggen_set_lexbuffer_size(int);
void yyerror(char *);
#define yyerror yyerror

/* nggenMem.c */
expr nggen_make_enode(enum expr_code code, long int i);
expr nggen_make_enode_p(enum expr_code code, void * p);
char *nggen_save_str( char *);
double * nggen_save_float(float);
expr nggen_list_put_last(expr, expr);
expr nggen_list0(enum expr_code);
expr nggen_list1(enum expr_code, expr);
expr nggen_list2(enum expr_code, expr, expr);
expr nggen_list3(enum expr_code, expr, expr, expr);
expr nggen_list4(enum expr_code, expr, expr, expr, expr);
SYMBOL nggen_find_symbol(char *);

/* nggenStub.c */
void nggen_error(char *fmt, ...);
void nggen_fatal(char *fmt, ...);
void nggen_stubs_makefile(void);
void nggen_stub_program(struct stub_gen_entry *ep);
void nggen_info_file(struct stub_gen_entry *ep);

/* library for stub */
int ngstb_BEGIN();
int ngstb_INIT(int argc, char ** argv);
int ngstb_REQ();
int ngstb_SET_ARG(void *, int);
int ngstb_END();
void ngstb_EXIT();

/* globals */
/* nggenCompile.c */
extern struct stub_gen_entry *stubs_head;
extern struct stub_gen_entry *stubs_tail;
extern int n_stubs;

extern char *compile_options[];
extern char *compiler_name;
extern char *linker_name;
extern int n_options_g;
extern char *globals[];
extern int n_globals_g;
extern char *libs[];
extern int n_libs_g;
extern char *current_module;

/* nggenMain.c */
extern char *program;                   /* this program */
extern char *source_file_name;
extern char *output_file_name;
extern FILE *source_file;
extern FILE *output_file;
extern int gmake_g;
extern int debug_flag_g;
extern FILE *debug_fp_g;

/* nggenMisc.c */
extern struct expr_code_info
{
    char *code_name;
    char *operator_name;
} expr_code_info[];

/* nggenIdlLex.c */
extern int lineno_g;

/* nggenIdlLex.c */
extern char title_file_name[];
extern struct keyword_entry 
{
    char *kw_name;
    enum symbol_type kw_type;
    int kw_value;
} keyword_table[];

/* nggenStub.c */
extern char *current_fortran_format;
extern int fortran_string_convention;

#endif /* _NGGENERATOR_H_ */
