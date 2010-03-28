/*
 * $RCSfile: nggenMisc.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $
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

#include "ngGenerator.h"

NGI_RCSID_EMBED("$RCSfile: nggenMisc.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:14:09 $")

FILE *print_fp_g;

void nggen_expr_print_rec(expr x, int l);

/**/
/**/
struct {
    enum expr_code code;
    char *name;
} expr_code_info_table[] = {
    {ERROR_NODE, "ERROR_NODE"},
    {LIST, "LIST"},
    {IDENT, "IDENT"},
    {STRING_CONSTANT, "STRING_CONSTANT"},
    {INT_CONSTANT, "INT_CONSTANT"},
    {LONG_CONSTANT, "LONG_CONSTANT"},
    {FLOAT_CONSTANT, "FLOAT_CONSTANT"},
    {BASIC_TYPE_NODE, "BASIC_TYPE_NODE"},
    {MODE_SPEC_NODE, "MODE_SPEC_NODE"},
    {DISTMODE_SPEC_NODE, "DISTMODE_SPEC_NODE"},

    {PLUS_EXPR, "+"},
    {MINUS_EXPR, "-"},
    {UNARY_MINUS_EXPR, "unary-"},
    {MUL_EXPR, "*"},
    {DIV_EXPR, "/"},
    {MOD_EXPR, "%"},
    {POW_EXPR, "^"},

    {POINTER_REF, "POINTER_REF"},
    {ARRAY_REF, "ARRAY_REF"},
    {(enum expr_code)0, NULL},
};
struct expr_code_info expr_code_info[MAX_CODE];

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_expr_print_rec                                             */
/*---------------------------------------------------------------------------*/
void
nggen_expr_print_rec(
    expr x,
    int l
)
{
    int i;
    struct list_node *lp;
    

    for ( i = 0; i < l; i++ ) {    /* indent */
        fprintf(print_fp_g, "    ");
    }
    if ( x == NULL ) { /* special case */
        fprintf(print_fp_g, "<NULL>");
        return;
    }
    fprintf(print_fp_g, "(%s", EXPR_CODE_NAME(EXPR_CODE(x)));
    fprintf(print_fp_g, ":%d", EXPR_LINENO(x));
    switch ( EXPR_CODE(x) ) {
        case IDENT:
            fprintf(print_fp_g, " \"%s\")", SYM_NAME(EXPR_SYM(x)));
            break;

        case STRING_CONSTANT:
            fprintf(print_fp_g, " \"%s\")", EXPR_STR(x));
            break;

        case INT_CONSTANT:
        case LONG_CONSTANT:
            fprintf(print_fp_g, " %ld)", EXPR_INT(x));
            break;

        case FLOAT_CONSTANT:
            fprintf(print_fp_g, " %g)", EXPR_FLOAT(x));
            break;

        case BASIC_TYPE_NODE:
            fprintf(print_fp_g, " <%s>)",
                nggen_basic_type_name(EXPR_TYPE(x)));
            break;

        case MODE_SPEC_NODE:
            fprintf(print_fp_g, " <%s>)",
                nggen_mode_spec_name(EXPR_MODE_SPEC(x)));
            break;

        case DISTMODE_SPEC_NODE:
            fprintf(print_fp_g, " <%s>)",
                nggen_distmode_spec_name(EXPR_DISTMODE_SPEC(x)));
            break;

        default:
            /* list */
            if ( (lp = EXPR_LIST(x)) == NULL ) {
                fprintf(print_fp_g, ")");
            } else {
                for ( /* */; lp != NULL; lp = NGG_LIST_NEXT(lp) ) {
                    fprintf(print_fp_g, "\n");
                    nggen_expr_print_rec(NGG_LIST_ITEM(lp), l + 1);
                }
                fprintf(print_fp_g, ")");
            }
            break;
    }
    return;
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_expr_print                                                 */
/*---------------------------------------------------------------------------*/
void
nggen_expr_print(
    expr x,
    FILE *fp
)
{
    print_fp_g = fp;   
    nggen_expr_print_rec(x, 0);
    fprintf(print_fp_g, "\n");
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_basic_type_name                                            */
/*---------------------------------------------------------------------------*/
char *
nggen_basic_type_name(
    DATA_TYPE type
)
{
    char *rc;

    switch ( type ) {
        case NG_ARGUMENT_DATA_TYPE_CHAR:
            rc = "CHAR";
            break;

        case NG_ARGUMENT_DATA_TYPE_SHORT:
            rc = "SHORT";
            break;

        case NG_ARGUMENT_DATA_TYPE_INT:
            rc = "INT";
            break;

        case NG_ARGUMENT_DATA_TYPE_LONG:
            rc = "LONG";
            break;

        case NG_ARGUMENT_DATA_TYPE_FLOAT:
            rc = "FLOAT";
            break;

        case NG_ARGUMENT_DATA_TYPE_DOUBLE:
            rc = "DOUBLE";
            break;

        case NG_ARGUMENT_DATA_TYPE_FILENAME:
            rc = "FILENAME";
            break;

        case NG_ARGUMENT_DATA_TYPE_CALLBACK:
            rc = "FUNC";
            break;

        default:
            rc = "?unknowntype?";
            break;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_mode_spec_name                                             */
/*---------------------------------------------------------------------------*/
char *
nggen_mode_spec_name(
    MODE_SPEC mode
)
{
    char *rc;

    switch ( mode ) {
        case NG_ARGUMENT_IO_MODE_NONE:
            rc = "mode_none";
            break;

        case NG_ARGUMENT_IO_MODE_IN:
            rc = "mode_in";
            break;

        case NG_ARGUMENT_IO_MODE_OUT:
            rc = "mode_out";
            break;

        case NG_ARGUMENT_IO_MODE_INOUT:
            rc = "mode_inout";
            break;

        default:
            rc = "?unknownmode?";
            break;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_distmode_spec_name                                         */
/*---------------------------------------------------------------------------*/
char *
nggen_distmode_spec_name(
    DISTMODE_SPEC distmode
)
{
    char *rc;

    switch ( distmode ) {
        case NGGEN_ARGUMENT_DIST_MODE_NORMAL:
            rc = "";
            break;

        case NGGEN_ARGUMENT_DIST_MODE_ALLOCATE:
            rc = "allocate";
            break;

        case NGGEN_ARGUMENT_DIST_MODE_BROADCAST:
            rc = "broadcast";
            break;

        default:
            rc = "?unknownmode?";
            break;
    }
    return(rc);
}

/**/
/*---------------------------------------------------------------------------*/
/* ID     = nggen_init_expr_code_info                                        */
/*---------------------------------------------------------------------------*/
void
nggen_init_expr_code_info(
    void
)
{
    int i;

    for ( i = 0; expr_code_info_table[i].name != NULL; i++ ) {
        expr_code_info[(int)expr_code_info_table[i].code].code_name =
            expr_code_info_table[i].name;
        expr_code_info[(int)expr_code_info_table[i].code].operator_name =
            expr_code_info_table[i].name; /* same */
    }
}
