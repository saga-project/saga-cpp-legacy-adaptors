#ifndef NG_OS_IRIX
static const char rcsid[] = "$RCSfile: nggenMem.c,v $ $Revision: 1.3 $ $Date: 2004/03/11 07:30:26 $";
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
#include "ngGenerator.h"

#define XMALLOC(t, size) ((t)nggen_xmalloc(size))
#define SYMBOL_HASH_SIZE 0x400
#define SYMBOL_HASH_MASK (SYMBOL_HASH_SIZE - 1)
SYMBOL symbol_hash_table[SYMBOL_HASH_SIZE];

void nggen_fatal(char *fmt, ...);

char * 
nggen_xmalloc(
    int size
)
{
    char *p;

    if ( (p = (char *)malloc(size)) == NULL ) {
        nggen_fatal("no memory");
    }
    return(p);
}

SYMBOL 
nggen_find_symbol(
    char *name
)
{
    SYMBOL sp;
    int hcode;
    char *cp;
    
    hcode = 0;                          /* hash code, bad ??                 */
    for ( cp = name; *cp != 0; cp++ ) {
        hcode = (hcode << 1) + *cp;
    }
    hcode &= SYMBOL_HASH_MASK;

    for ( sp = symbol_hash_table[hcode]; sp != NULL; sp = sp->s_next ) {
        if ( strcmp(name, sp->s_name) == 0 ) {
            return(sp);
        }
    }
                                        /* not found, then allocate symbol   */
    sp = XMALLOC(SYMBOL, sizeof(*sp));
    memset((char *)sp, 0, sizeof(*sp));
    sp->s_name = nggen_save_str(name);
                                        /* link it                           */
    sp->s_next = symbol_hash_table[hcode];
    symbol_hash_table[hcode] = sp;
    return(sp);
}

char *
nggen_save_str(
    char *s
)
{
    char *p;
    
    p = XMALLOC(char *, strlen(s) + 1);
    strcpy(p, s);
    return(p);
}

double * 
nggen_save_float(
    float d
)
{
    double *dp;
    
    dp = XMALLOC(double *, sizeof(double));
    *dp = d;
    return(dp);
}

expr 
nggen_make_enode(
    enum expr_code code,
    long int v
)
{
    expr ep;
    
    ep = XMALLOC(expr, sizeof(*ep));
    ep->e_code = code;
    ep->e_lineno = lineno_g;
    ep->v.e_ival = v;
    return(ep);
}

expr 
nggen_make_enode_p(
    enum expr_code code,
    void *v
)
{
    expr ep;
    
    ep = XMALLOC(expr, sizeof(*ep));
    ep->e_code = code;
    ep->e_lineno = lineno_g;
    ep->v.e_str = (char *)v;
    return(ep);
}

struct list_node *
nggen_cons_list(
    expr x, 
    struct list_node *l
)
{
    struct list_node *lp;
    
    lp = XMALLOC(struct list_node *, sizeof(struct list_node));
    lp->l_next = l;
    lp->l_item = x;
    return(lp);
}

expr 
nggen_list0(
    enum expr_code code
)
{
    return(nggen_make_enode_p(code, NULL));
}

expr 
nggen_list1(
    enum expr_code code, 
    expr x1
)
{
    return(nggen_make_enode_p(code, (void *)nggen_cons_list(x1, NULL)));
}

expr 
nggen_list2(
    enum expr_code code, 
    expr x1, 
    expr x2
)
{
    return(nggen_make_enode_p(code,
        (void *)nggen_cons_list(x1, nggen_cons_list(x2, NULL))));
}

expr 
nggen_list3(
    enum expr_code code, 
    expr x1, 
    expr x2, 
    expr x3
)
{
    return(nggen_make_enode_p(code, (void *)
        nggen_cons_list(x1,
        nggen_cons_list(x2, nggen_cons_list(x3, NULL)))));
}

expr 
nggen_list4(
    enum expr_code code, 
    expr x1, 
    expr x2, 
    expr x3, 
    expr x4
)
{
  return(nggen_make_enode_p(code, (void *)
      nggen_cons_list(x1, nggen_cons_list(x2, nggen_cons_list
      (x3, nggen_cons_list(x4, NULL))))));
}

expr 
nggen_list_put_last(
    expr lx, 
    expr x
)
{
    struct list_node *lp;
    
    lp = lx->v.e_lp;    
    if ( lp == NULL ) {
        lx->v.e_lp = nggen_cons_list(x, NULL);
    } else {
        for ( ; lp->l_next != NULL; lp = lp->l_next ) {
            ;
        }
        lp->l_next = nggen_cons_list(x, NULL);
    }
    return(lx);
}
