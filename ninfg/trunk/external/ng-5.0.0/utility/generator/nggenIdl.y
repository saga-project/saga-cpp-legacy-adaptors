/*
 * $RCSfile: nggenIdl.y,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $
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

/* 
 * NG IDL grammer
 */

%token SERROR       /* syntax error mark */
%token CONSTANT     /* constant: integer, ... */
%token IDENTIFIER   /* identifier */
%token RELOP        /* relative operator */
%token STRING       /* string */
%token TYPENAME

/* reserved words, etc */
/* type and mode keywords */
%token TYPE MODE DISTMODE

/* keywords */
%token MODULE COMPILE_OPTIONS GLOBALS DEFINE DEFCLASS DEFSTATE DEFMETHOD CALLS REQUIRED BACKEND SHRINK CALCORDER LIBRARY FORTRANFORMAT FORTRANSTRINGCONVENTION CALLBACK LANGUAGE LINKER_DEF COMPILER_DEF

/* precedence rules */
%left '?' ':'
%left '|'       /* bitwise operation */
%left '^'       /* bitwise operation */
%left '&'       /* bitwise operation */
%left RELOP
%left LSHIFT RSHIFT /* shift operation */
%left '+' '-'       /* add and minus */ 
%left '*' '/' '%'   /* mul and div , mod */
%right '!' '~'      /* unary operation */
%right UNARY
%left UNARY_HIGH_PRIORITY
%left '[' '(' 

%{
static const char rcsid[] = "$RCSfile: nggenIdl.y,v $ $Revision: 1.3 $ $Date";
#include "ngGenerator.h"

typedef union 
{
    expr val;
    enum expr_code code;
} yystype;
#define YYSTYPE yystype

static int yylex(void);

%}

/* define types */
%type <val> interface_definition parameter_list parameter id_list parameter_callback_list callback define_list define_item callback_list
%type <val> decl_specifier MODE DISTMODE mode_specifier TYPE type_specifier declarator
%type <val> interface_body globals_body opt_string required backend shrink calcorder language option_list decl_option defclass_option defclass_option_list
%type <val> defmethod_option_list defmethod_option  
%type <val> expr expr_or_null unary_expr 
%type <val> primary_expr IDENTIFIER CONSTANT STRING TYPENAME RELOP
%type <val> range_spec
%type <val> REQUIRED BACKEND SHRINK CALCORDER LANGUAGE

%start program

%%
/* program toplevel */
program:/* empty */
    | declaration_list
    ;

declaration_list: 
      declaration
    | declaration_list declaration
    ;

declaration:
          MODULE IDENTIFIER ';'
    { nggen_cpl_module($2); }
        | COMPILE_OPTIONS STRING ';'
    { nggen_cpl_compile_options($2); }
        | GLOBALS globals_body
    { nggen_cpl_globals($2); }
        | LIBRARY STRING ';'
    { nggen_cpl_library($2); }
        | FORTRANFORMAT STRING ';'
    { nggen_cpl_fortranformat($2);}
        | FORTRANSTRINGCONVENTION IDENTIFIER ';'
    { nggen_cpl_fortranstringconvention($2);}
        | DEFINE interface_definition opt_string option_list interface_body
    { nggen_cpl_interface($2,$3,$4,$5); }
 	| DEFCLASS IDENTIFIER opt_string defclass_option_list '{' define_list '}' 
    { nggen_cpl_class($2,$3,$4,$6); }
        | COMPILER_DEF STRING ';'
    { nggen_cpl_compiler($2);}
        | LINKER_DEF STRING ';'
    { nggen_cpl_linker($2);}
        | error
    ;

define_list:
	/* empty */ {$$ = NULL;}
        |  define_item
    { $$ = nggen_list1(LIST, $1);}   
        |  define_list define_item
    { $$ = nggen_list_put_last($1, $2);}
        ;

define_item:
          DEFSTATE '{' 
    { $$ = nggen_read_rest_of_body(FALSE); } 
	| DEFMETHOD interface_definition opt_string defmethod_option_list interface_body
    { $$ = nggen_list4(LIST, $2,$3,$4,$5); }
	;

defmethod_option_list:
	/* empty */ {$$ = NULL;}
        | defmethod_option
    { $$ = nggen_list1(LIST, $1);} 
        | defmethod_option_list defmethod_option 
    { $$ = nggen_list_put_last($1, $2);}     
	;

defmethod_option:
        calcorder
    { $$ = $1;}
        ;

defclass_option_list:
	/* empty */ {$$ = NULL;}
        | defclass_option
    { $$ = nggen_list1(LIST, $1);} 
        | defclass_option_list defclass_option 
    { $$ = nggen_list_put_last($1, $2);}     
	;

defclass_option:
        required
    { $$ = $1;}
        | backend
    { $$ = $1;}
        | shrink
    { $$ = $1;}
        | language
    { $$ = $1;}
	;

option_list:
    /* empty */ {$$ = NULL;}
        | decl_option
    { $$ = nggen_list1(LIST, $1);} 
        | option_list decl_option 
    { $$ = nggen_list_put_last($1, $2);}     
	;

decl_option:
        required
    { $$ = $1;}
        | backend
    { $$ = $1;}
        | shrink
    { $$ = $1;}
        | calcorder
    { $$ = $1;}
        | language
    { $$ = $1;}
    ;

interface_definition: 
      IDENTIFIER '(' parameter_list ')'
    { $$ = nggen_list2(LIST,$1,$3); }
	|  IDENTIFIER '(' parameter_callback_list ')'
    { $$ = nggen_list2(LIST,$1,$3); }
	|  IDENTIFIER '(' callback_list ')'
    { $$ = nggen_list2(LIST,$1,$3); }
	;

callback_list:
        callback
    { $$ = nggen_list1(LIST,$1); }   
        | callback_list ',' callback
    { $$ = nggen_list_put_last($1,$3); }   
        ;

parameter_callback_list:
        parameter_list ',' callback
    { $$ = nggen_list_put_last($1,$3); }   
        | parameter_callback_list ',' callback
    { $$ = nggen_list_put_last($1,$3); }   
        ;

parameter_list: 
	/* empty */ 
	{ $$ = NULL;}
	| parameter
    { $$ = nggen_list1(LIST,$1); }
	| parameter_list ',' parameter
    { $$ = nggen_list_put_last($1,$3); }
	;

callback: IDENTIFIER '(' parameter_list ')'
    { $$ = nggen_list2(CALLBACK_FUNC, $1, $3); }
        ;

parameter: decl_specifier declarator
    { $$ = nggen_list2(LIST,$1,$2); }
	;

decl_specifier: 
     mode_specifier type_specifier
    { $$ = nggen_list2(LIST,$1,$2); }
	| type_specifier mode_specifier
    { $$ = nggen_list2(LIST,$2,$1); }
	| type_specifier mode_specifier type_specifier
    { $$ = nggen_list2(LIST,$2,nggen_list2(LIST,$1,$3)); } 
	;

type_specifier:
	TYPE
	| TYPE TYPE
    { $$ = nggen_list2(LIST,$1,$2); }
	| TYPE TYPE TYPE    /* ex. unsigned long int */
    { $$ = nggen_list2(LIST,$1,nggen_list2(LIST,$2,$3)); }
	| TYPENAME
	;

mode_specifier:
	MODE
    { $$ = nggen_list2(LIST,$1,NULL); }
	| MODE DISTMODE
    { $$ = nggen_list2(LIST,$1,$2); }
	| DISTMODE MODE
    { $$ = nggen_list2(LIST,$2,$1); }
	;

declarator:
      IDENTIFIER
	| '(' declarator ')'
    { $$ = $2; }
	| declarator '['expr_or_null ']'
    { $$ = nggen_list3(ARRAY_REF,$1,$3,NULL); }
	| declarator '['expr_or_null ':' range_spec ']'
    { $$ = nggen_list3(ARRAY_REF,$1,$3,$5); }
	| '*' declarator
    { $$ = nggen_list1(POINTER_REF,$2); }
	;

range_spec:
      expr
    { $$ = nggen_list3(LIST,NULL,$1,NULL); }
	| expr ',' expr
    { $$ = nggen_list3(LIST,$1,$3,NULL); }
	| expr ',' expr ',' expr
    { $$ = nggen_list3(LIST,$1,$3,$5); }
	;

opt_string: 
    /* empty */ { $$ = NULL; }
    | STRING 
;

required:
    REQUIRED STRING
    { $$ = nggen_cpl_required($2); }
    ;

backend:
    BACKEND STRING
        { $$ = nggen_cpl_backend($2);}
    ;

shrink:
    SHRINK STRING
        { $$ = nggen_cpl_shrink($2);}
    ;

language:
    LANGUAGE STRING
    { $$ = nggen_cpl_language($2); }
    ;

calcorder:
        CALCORDER expr
        { $$ = nggen_cpl_calcorder($2);}
        ;

interface_body:
    /* body in C declaration */
    '{' { $$ = nggen_read_rest_of_body(1); } 
    | CALLS opt_string IDENTIFIER '(' id_list ')' ';'
      { $$ = nggen_list3(LIST,$2,$3,$5); }
    ;

globals_body:
    '{' { $$ = nggen_read_rest_of_body(0); } 
    ;

id_list: IDENTIFIER
      { $$ = nggen_list1(LIST,$1); }
    | id_list ',' IDENTIFIER
      { $$ = nggen_list_put_last($1,$3); }
    | { $$ = NULL; } /* null */
    ;


/* index description */
expr_or_null: 
    expr 
    | { $$ = NULL; } /* null */
    ;

expr:        
     unary_expr
    | expr '/' expr
    { $$ = nggen_list2(DIV_EXPR,$1,$3); }
    | expr '%' expr
    { $$ = nggen_list2(MOD_EXPR,$1,$3); }
    | expr '+' expr
    { $$ = nggen_list2(PLUS_EXPR,$1,$3); }
    | expr '-' expr
    { $$ = nggen_list2(MINUS_EXPR,$1,$3); }
    | expr '*' expr
    { $$ = nggen_list2(MUL_EXPR,$1,$3); }
    | expr '^' expr
    { $$ = nggen_list2(POW_EXPR,$1,$3); }
    | expr RELOP expr
    { yystype * tmp = (yystype *)(&($2));
      $$ = nggen_list2(tmp->code,$1,$3); }

        | expr '?' expr ':' expr
    { $$ = nggen_list3(TRY_EXPR,$1,$3,$5); }
    ;

unary_expr:
      primary_expr
    | '*' expr  %prec UNARY /* pointer reference */ 
    { $$ = nggen_list1(POINTER_REF,$2); }
    | '-' expr  %prec UNARY /* unary minus */
    { $$ = nggen_list1(UNARY_MINUS_EXPR,$2); }
    ;

primary_expr:
     primary_expr '[' expr ']'
      { $$ = nggen_list2(ARRAY_REF,$1,$3); }
    | IDENTIFIER
    | CONSTANT
    | '('  expr  ')'
       { $$ = $2; }
    ;

%%

#include "nggenIdlLex.c"




