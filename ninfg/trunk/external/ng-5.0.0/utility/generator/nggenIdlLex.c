/*
 * $RCSfile: nggenIdlLex.c,v $ $Revision: 1.3 $ $Date: 2007/09/26 04:30:16 $
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

#define MAX_LINE_LEN 1024
#define GETCH() nggen_lex_getc()
#define UNGETCH(c) (have_peekc_g++, line_peekc_g = (c))

static int nggen_read_identifier(char ch);
static int nggen_read_number(int ch);
static int nggen_read_string_constant(int mark);
static int yylex0(void);

static char *yytext = NULL;
static int lex_bufsize = 0;
static char line_buf_g[MAX_LINE_LEN];
static char *linep_g = NULL;
static int have_peekc_g = FALSE;
static int line_peekc_g;
static int yylast_g;
int lineno_g = 0;
expr error_expr;

struct keyword_entry keyword_table[]=
{
    {"mode_in",        S_CLASS, (int)NG_ARGUMENT_IO_MODE_IN},
    {"mode_out",       S_CLASS, (int)NG_ARGUMENT_IO_MODE_OUT},
    {"mode_inout",     S_CLASS, (int)NG_ARGUMENT_IO_MODE_INOUT},
    {"mode_work",      S_CLASS, (int)NG_ARGUMENT_IO_MODE_WORK},
    {"IN",             S_CLASS, (int)NG_ARGUMENT_IO_MODE_IN},
    {"OUT",            S_CLASS, (int)NG_ARGUMENT_IO_MODE_OUT},
    {"INOUT",          S_CLASS, (int)NG_ARGUMENT_IO_MODE_INOUT},
    {"WORK",           S_CLASS, (int)NG_ARGUMENT_IO_MODE_WORK},

    {"allocate",       S_DISTMODE, (int)NGGEN_ARGUMENT_DIST_MODE_ALLOCATE},
    {"broadcast",      S_DISTMODE, (int)NGGEN_ARGUMENT_DIST_MODE_BROADCAST},

    {"char",           S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_CHAR},
    {"short",          S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_SHORT},
    {"int",            S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_INT},
    {"long",           S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_LONG},
    {"float",          S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_FLOAT},
    {"double",         S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_DOUBLE},
    {"string",         S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_STRING},
    {"scomplex",       S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_SCOMPLEX},
    {"dcomplex",       S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_DCOMPLEX},

    {"filename",       S_TYPE, (int)NG_ARGUMENT_DATA_TYPE_FILENAME},

    {"Module",         S_KEYWORD, MODULE},
    {"CompileOptions", S_KEYWORD, COMPILE_OPTIONS},
    {"Compiler",       S_KEYWORD, COMPILER_DEF},
    {"Linker",         S_KEYWORD, LINKER_DEF},
    {"Globals",        S_KEYWORD, GLOBALS},
    {"Define",         S_KEYWORD, DEFINE},
    {"Calls",          S_KEYWORD, CALLS},
    {"Backend",        S_KEYWORD, BACKEND},
    {"Shrink",         S_KEYWORD, SHRINK},
    {"Required",       S_KEYWORD, REQUIRED},
    {"Language",       S_KEYWORD, LANGUAGE},
    {"CalcOrder",      S_KEYWORD, CALCORDER},
    {"Library",        S_KEYWORD, LIBRARY},
    {"FortranFormat",  S_KEYWORD, FORTRANFORMAT},
    {"FortranStringConvention", S_KEYWORD, FORTRANSTRINGCONVENTION},
    {"CALLBACK",       S_KEYWORD, CALLBACK},
    {"DefClass",       S_KEYWORD, DEFCLASS},
    {"DefState",       S_KEYWORD, DEFSTATE},
    {"DefMethod",      S_KEYWORD, DEFMETHOD},
    {NULL, (enum symbol_type)0, 0}
};

void
nggen_initialize_lex(
    void
)
{
    struct keyword_entry *kp;
    SYMBOL sp;

    /* intern all keywords */
    for (kp = keyword_table; kp->kw_name != NULL; kp++ ) {
        sp = nggen_find_symbol(kp->kw_name);
        sp->s_type = kp->kw_type;
        sp->s_value = kp->kw_value;
    }

    /* make constant */
    error_expr = nggen_make_enode(ERROR_NODE, 0);

    /* alloc buffer */
    yytext = (char *)calloc(sizeof(char), lex_bufsize);
    if ( yytext == NULL ) {
        nggen_error("can't alloc buffer for lex.\n");
    }
}

void
nggen_set_lexbuffer_size(
    int size
)
{
    /* set size of buffer */
    lex_bufsize = size;
}

int 
nggen_lex_getc(
    void
)
{
    char *cp;
    

    if ( have_peekc_g ) {
        have_peekc_g = FALSE;
        return(line_peekc_g);
    }
    if ( linep_g == NULL || *linep_g == '\0' ) {

next_line:

        if ( fgets(line_buf_g, MAX_LINE_LEN, source_file) == NULL ) {
           return(EOF);
        }
        linep_g = line_buf_g;
        lineno_g++;

        if ( debug_flag_g ) {
            printf("%3d:%s", lineno_g, line_buf_g);
        }
        /* check line nubmer, # nnn "file" */
        if ( line_buf_g[0] == '#' ) { 
            linep_g = &line_buf_g[1];
            while ( *linep_g == ' ' ) {
                linep_g++;    /* skip space */
            }
            lineno_g = 0;
            while ( isdigit((int)*linep_g) ) {
                lineno_g = lineno_g * 10 + *linep_g++ - '0';
            }
            while ( *linep_g == ' ' ) {
                linep_g++;    /* skip space */
            }
            if ( *linep_g == '"' ) { /* parse file name */
                linep_g++;
                cp = title_file_name;
                while ( *linep_g != '"' && *linep_g != '\n' ) {
                    *cp++ = *linep_g++;
                }
                *cp = '\0';
            }
            goto next_line;
        }
    }
    return(*linep_g++);
}

void
yyerror(
    char *s
) 
{ 
    if ( yylast_g < 0x7F ) {
      nggen_error("%s at or near '%c'", s, yylast_g);
    } else {
        switch ( yylast_g ) {
            default:
                nggen_error(s);
                break;
            case MODE:
                nggen_error("%s at keyword '%s'", s, yytext);
                break;
            case DISTMODE:
                nggen_error("%s at distmode keyword '%s'", s, yytext);
                break;
            case TYPE:
                nggen_error("%s at type keyword '%s'", s, yytext);
                break;
            case IDENTIFIER:
                nggen_error("%s at symbol '%s'", s, yytext);
                break;
            case CONSTANT:
                nggen_error("%s at constant '%s'", s, yytext);
                break;
        }
    }
}

int
yylex(
    void
)
{
    yylast_g = yylex0();
    return(yylast_g);
}

int
yylex0(
    void
)
{
    int ch;
    
    for ( ; ; ) {
        switch ( ch = GETCH() ) {
          /* white space */
            case ' ':   case '\t':
            case '\b':  case '\r': case '\f':
                continue;
            case '\n':
                continue;
            case EOF:
            case '(':  case ')':
            case '{':  case '}':
            case '[':  case ']':
            case '*':  case '%':
            case '?':  case ':':  case ';': 
            case '^':  case '~':  case ',':
                /* single character */
                return(ch);
            case '|':   /* | and || */
#ifdef not
                if ( (ch = GETCH()) == '|' ) {
                    return(OROR);
                }
                UNGETCH(ch);
#endif
                return('|');
      
            case '&':   /* & and && */
#ifdef not
                if ( (ch = GETCH()) == '&' ) {
                    return(ANDAND);
                }
                UNGETCH(ch);
#endif
                return('&');

            case '<':   /* < and << */
                switch ( ch = GETCH() ) {
                    case '<':
                        return(LSHIFT);
                    case '=':
                        yylval.code = LE_EXPR;
                        return(RELOP);
                 }
                UNGETCH(ch);
                yylval.code = LT_EXPR;
                return(RELOP);
      
            case '>':   /* > and >> and >= */
                switch ( ch = GETCH() ) {
                    case '>':
                        return(RSHIFT);
                    case '=':
                        yylval.code = GE_EXPR;
                        return(RELOP);
                }
                UNGETCH(ch);
                yylval.code = GT_EXPR;
                return(RELOP);
      
            case '!':   /* ! and != */
                if ( (ch = GETCH()) == '=' ) {
                    yylval.code = NEQ_EXPR;
                    return(RELOP);
                }
                UNGETCH(ch);
                return('!');
      
            case '=':   /* = and == */
                if ( (ch = GETCH()) == '=' ) {
                    yylval.code = EQ_EXPR;
                    return(RELOP);
                }
                UNGETCH(ch);
                return('=');

            case '+':  /* + and ++ */
#ifdef not
                if ( (ch = GETCH()) == '+' ) {
                     return(PLUSPLUS);
                }
                UNGETCH(ch);
#endif
                return('+');
      
            case '-':   /* - and -- and -> */
#ifdef not
                switch ( ch = GETCH() ) {
                    case '-':
                        return(MINUSMINUS);
                    case '>':
                        return(STREF);
                }
                UNGETCH(ch);
#endif
                return('-');
      
            case '/':
                if ( (ch = GETCH()) == '*' ) {
                /* scan comment */
                    for ( ; ; ) {
                        ch = GETCH();
                        if ( ch == '*' && (ch = GETCH()) == '/' ) {
                            break;
                        }
                        if ( ch == EOF ) {
                            nggen_error("unexpected EOF");
                            return(EOF);
                        }
                    }
                    continue;
                }
                UNGETCH(ch);
                return('/');
      
            case '.':
                ch = GETCH();
                UNGETCH(ch);    /* peek */
                if ( ch >= '0' && ch <= '9' ) {
                    return(nggen_read_number('.'));
                }
                return('.');
      
            case '0': case '1': case '2': case '3': case '4': 
            case '5': case '6': case '7': case '8': case '9': 
                return(nggen_read_number(ch));
      
            case '"':
                return(nggen_read_string_constant('"'));
      
            case '\'':
                return(nggen_read_string_constant('\''));
      
            default:
                /* identifier */
                if ( !isalpha(ch) ) {
                    nggen_error("illegal character: 0x% in hex", ch);
                    break;
                }
            case '_':
            case '$':
                return(nggen_read_identifier(ch));
        }
    }
}

int 
nggen_read_identifier(
    char ch
)
{
    char *cp;
    SYMBOL sp;
    int rc = 0;
    
    cp = yytext;
    do {
        *cp++ = ch;
        ch = GETCH();
        if ( cp >= &yytext[lex_bufsize-1] ) {
            nggen_fatal("too long identifier");
            break;
        }
    } while ( isalnum((int)ch) || ch == '_' || ch == '$' );
    UNGETCH(ch);    /* push back */
    *cp = '\0';
    sp = nggen_find_symbol(yytext);
    
    switch ( sp->s_type ) {
        case S_KEYWORD:
            rc = sp->s_value;
        break;
        case S_TYPE:
            yylval.val = nggen_make_enode(BASIC_TYPE_NODE, sp->s_value);
            rc = TYPE;
            break;
        case S_CLASS:
            yylval.val = nggen_make_enode(MODE_SPEC_NODE, sp->s_value);
            rc = MODE;
            break;
        case S_DISTMODE:
            yylval.val = nggen_make_enode(DISTMODE_SPEC_NODE, sp->s_value);
            rc = DISTMODE;
            break;
        case S_IDENT:
            yylval.val = nggen_make_enode_p(IDENT, sp);
            rc = IDENTIFIER;
            break;
        default:
            nggen_fatal("nggen_read_identifier");
            break;
    }
    return(rc);
}

int
nggen_read_number(
    int ch
)
{
    char *cp;
    long int value;
  
    value = 0;
    if ( ch == '0' ) {
        ch = GETCH();
        if ( ch == 'x' || ch == 'X' ) { /* HEX */
            for ( ; ; ) {
                ch = GETCH();
                if ( !isxdigit(ch) ) {
                    break;
                }
                if ( isdigit(ch) ) {
                    value = value * 16 + ch - '0';
                } else if ( isupper(ch) ) {
                    value = value * 16 + ch - 'A' + 10;
                } else {
                    value = value * 16 + ch - 'a' + 10;
                }
            }
        }
        if ( ch == '.' ) {
            goto read_floating;
        } else { /* octal */
            while ( ch >= '0' && ch <= '7' ) {
                value = value * 8 + ch - '0';
                ch = GETCH();
            }
        }
        goto ret_INT;
    }
    /* else decimal or floating */
read_floating:
    cp = yytext;
    while ( isdigit(ch) ) {
        value = value * 10 + ch - '0';
        *cp++ = ch;
        ch = GETCH();
    }
    if ( ch != '.' && ch != 'e' && ch != 'E' ) {
        goto ret_INT;
    }
    /* floating */
    if ( ch == '.' ) {
        *cp++ = ch;
        /* reading floating */
        ch = GETCH();
        while ( isdigit(ch) ) {
            *cp++ = ch;
            ch = GETCH();
        }
    }
    if ( ch == 'e' || ch == 'E' ) {
        *cp++ = 'e';
        ch = GETCH();
        if ( ch == '+' || ch == '-' ) {
            *cp++ = ch;
            ch = GETCH();
        }
        while ( isdigit(ch) ) {
            *cp++ = ch;
            ch = GETCH();
        }
    }
    UNGETCH(ch);
    *cp = '\0';
    yylval.val = nggen_make_enode_p(FLOAT_CONSTANT,
        nggen_save_float(atof(yytext)));
    return(CONSTANT);
  
ret_INT:
    if ( ch == 'L' ) {
        yylval.val = nggen_make_enode(LONG_CONSTANT, value);
    } else {
        UNGETCH(ch);    
        yylval.val = nggen_make_enode(INT_CONSTANT, value);
    }
    return(CONSTANT);
}

int
nggen_read_string_constant(
    int mark
)
{
    int ch;
    char *cp;
    int value;
    int i;
    
    cp = yytext;
    while ( (ch = GETCH()) != mark ) {
        switch ( ch ) {
            case EOF:
                nggen_error("unexpected EOF");
                break;
      
#ifdef not
            case '\n':
                nggen_error("newline in string or char constant");
                break;
#endif
            case '\\':  /* escape */
                switch ( ch = GETCH() ) {
                    case '\n':
                        continue;
                    case 'n':
                        ch = '\n';
                        break;
                    case 'r':
                        ch = '\r';
                        break;
                    case 'b':
                        ch = '\b';
                        break;
                    case 't':
                        ch = '\t';
                        break;
                    case 'f':
                        ch = '\f';
                        break;
                    case 'v':
                        ch = '\013';
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        value = ch - '0';
                        ch = GETCH();  /* try for 2 */
                        if ( ch >= '0' && ch <= '7' ) {
                            value = (value << 3) | (ch - '0');
                            ch = GETCH();
                            if ( ch >= '0' && ch <= '7' ) {
                                value = (value << 3) | (ch - '0');
                            } else {
                                UNGETCH(ch);
                            }
                        } else {
                            UNGETCH(ch);
                        }
                        ch = value;
                        break;
                }
            default:
                *cp++ = ch;
        }
        if ( cp >= &yytext[lex_bufsize - 1] ) {
            nggen_fatal("too long string");
            break;
        }
    }
    *cp = '\0';
  
    if ( mark == '"' ) { /* end of string or  char constant */
        yylval.val = nggen_make_enode_p(STRING_CONSTANT,
            nggen_save_str(yytext));
        return(STRING);
    } else { 
        if ( cp == yytext ) { /* end the character constant */
            nggen_error("empty character constant");
        }
        if ( (unsigned)(cp - yytext) > (sizeof(int) / sizeof(char)) ) {
            nggen_error("too many characters in character constant");
        }
        value = 0;
        for ( i = 0; (unsigned)i < sizeof(int); i++ ) {
            if ( yytext[i] == 0 ) {
                break;
            }
            value = (value << 8)| (0xFF & yytext[i]);
        }
        yylval.val = nggen_make_enode(INT_CONSTANT, value);
    }
    return(CONSTANT);
}

expr 
nggen_read_rest_of_body(
    int flag
)
{
    char *cp;
    int ch;
    char in_string;
    int nest_level;
    
    cp = yytext;
    if ( flag ) {
        *cp++ = '{';                    /* already read */
    }
    in_string = 0;
    nest_level = 1;
    do {
        ch = GETCH();
        if ( ch == EOF ) {
            nggen_error("unexpected EOF");
            break;
        } else if ( ch == '\\' ) {         /* escape */
            *cp++ = ch;
            *cp++ = GETCH();
            continue;
        }
        if ( in_string != 0 && in_string == ch ) {
            in_string = 0;
        } else if ( in_string == 0 ) {      /* out string */
            
            if ( ch == '"' || ch == '\'' ) {
                in_string = ch;
            } else if ( ch == '{' ) {       /* else count nest level */
                nest_level++;
            } else if ( ch == '}' ) {
                nest_level--;
            }
        }
        *cp++ = ch;
    } while ( nest_level > 0 );
    if ( !flag ) {
        cp--;
    }
    *cp = '\0';
    return(nggen_make_enode_p(STRING_CONSTANT, nggen_save_str(yytext)));
}
