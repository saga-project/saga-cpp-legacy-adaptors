/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SERROR = 258,
     CONSTANT = 259,
     IDENTIFIER = 260,
     RELOP = 261,
     STRING = 262,
     TYPENAME = 263,
     TYPE = 264,
     MODE = 265,
     DISTMODE = 266,
     MODULE = 267,
     COMPILE_OPTIONS = 268,
     GLOBALS = 269,
     DEFINE = 270,
     DEFCLASS = 271,
     DEFSTATE = 272,
     DEFMETHOD = 273,
     CALLS = 274,
     REQUIRED = 275,
     BACKEND = 276,
     SHRINK = 277,
     CALCORDER = 278,
     LIBRARY = 279,
     FORTRANFORMAT = 280,
     FORTRANSTRINGCONVENTION = 281,
     CALLBACK = 282,
     LANGUAGE = 283,
     LINKER_DEF = 284,
     COMPILER_DEF = 285,
     RSHIFT = 286,
     LSHIFT = 287,
     UNARY = 288,
     UNARY_HIGH_PRIORITY = 289
   };
#endif
/* Tokens.  */
#define SERROR 258
#define CONSTANT 259
#define IDENTIFIER 260
#define RELOP 261
#define STRING 262
#define TYPENAME 263
#define TYPE 264
#define MODE 265
#define DISTMODE 266
#define MODULE 267
#define COMPILE_OPTIONS 268
#define GLOBALS 269
#define DEFINE 270
#define DEFCLASS 271
#define DEFSTATE 272
#define DEFMETHOD 273
#define CALLS 274
#define REQUIRED 275
#define BACKEND 276
#define SHRINK 277
#define CALCORDER 278
#define LIBRARY 279
#define FORTRANFORMAT 280
#define FORTRANSTRINGCONVENTION 281
#define CALLBACK 282
#define LANGUAGE 283
#define LINKER_DEF 284
#define COMPILER_DEF 285
#define RSHIFT 286
#define LSHIFT 287
#define UNARY 288
#define UNARY_HIGH_PRIORITY 289




/* Copy the first part of user declarations.  */
#line 55 "nggenIdl.y"

static const char rcsid[] = "$RCSfile: nggenIdl.y,v $ $Revision: 1.3 $ $Date";
#include "ngGenerator.h"

typedef union 
{
    expr val;
    enum expr_code code;
} yystype;
#define YYSTYPE yystype

static int yylex(void);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 189 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  27
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   251

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  55
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  101
/* YYNRULES -- Number of states.  */
#define YYNSTATES  177

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   289

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,     2,     2,     2,    42,    35,     2,
      48,    52,    40,    38,    53,    39,     2,    41,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    32,    49,
       2,     2,     2,    31,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    47,     2,    54,    34,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    50,    33,    51,    44,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    36,    37,    45,    46
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    11,    15,    19,    22,
      26,    30,    34,    40,    48,    52,    56,    58,    59,    61,
      64,    67,    73,    74,    76,    79,    81,    82,    84,    87,
      89,    91,    93,    95,    96,    98,   101,   103,   105,   107,
     109,   111,   116,   121,   126,   128,   132,   136,   140,   141,
     143,   147,   152,   155,   158,   161,   165,   167,   170,   174,
     176,   178,   181,   184,   186,   190,   195,   202,   205,   207,
     211,   217,   218,   220,   223,   226,   229,   232,   235,   237,
     245,   247,   249,   253,   254,   256,   257,   259,   263,   267,
     271,   275,   279,   283,   287,   293,   295,   298,   301,   306,
     308,   310
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      56,     0,    -1,    -1,    57,    -1,    58,    -1,    57,    58,
      -1,    12,     5,    49,    -1,    13,     7,    49,    -1,    14,
      85,    -1,    24,     7,    49,    -1,    25,     7,    49,    -1,
      26,     5,    49,    -1,    15,    67,    78,    65,    84,    -1,
      16,     5,    78,    63,    50,    59,    51,    -1,    30,     7,
      49,    -1,    29,     7,    49,    -1,     1,    -1,    -1,    60,
      -1,    59,    60,    -1,    17,    50,    -1,    18,    67,    78,
      61,    84,    -1,    -1,    62,    -1,    61,    62,    -1,    83,
      -1,    -1,    64,    -1,    63,    64,    -1,    79,    -1,    80,
      -1,    81,    -1,    82,    -1,    -1,    66,    -1,    65,    66,
      -1,    79,    -1,    80,    -1,    81,    -1,    83,    -1,    82,
      -1,     5,    48,    70,    52,    -1,     5,    48,    69,    52,
      -1,     5,    48,    68,    52,    -1,    71,    -1,    68,    53,
      71,    -1,    70,    53,    71,    -1,    69,    53,    71,    -1,
      -1,    72,    -1,    70,    53,    72,    -1,     5,    48,    70,
      52,    -1,    73,    76,    -1,    75,    74,    -1,    74,    75,
      -1,    74,    75,    74,    -1,     9,    -1,     9,     9,    -1,
       9,     9,     9,    -1,     8,    -1,    10,    -1,    10,    11,
      -1,    11,    10,    -1,     5,    -1,    48,    76,    52,    -1,
      76,    47,    87,    54,    -1,    76,    47,    87,    32,    77,
      54,    -1,    40,    76,    -1,    88,    -1,    88,    53,    88,
      -1,    88,    53,    88,    53,    88,    -1,    -1,     7,    -1,
      20,     7,    -1,    21,     7,    -1,    22,     7,    -1,    28,
       7,    -1,    23,    88,    -1,    50,    -1,    19,    78,     5,
      48,    86,    52,    49,    -1,    50,    -1,     5,    -1,    86,
      53,     5,    -1,    -1,    88,    -1,    -1,    89,    -1,    88,
      41,    88,    -1,    88,    42,    88,    -1,    88,    38,    88,
      -1,    88,    39,    88,    -1,    88,    40,    88,    -1,    88,
      34,    88,    -1,    88,     6,    88,    -1,    88,    31,    88,
      32,    88,    -1,    90,    -1,    40,    88,    -1,    39,    88,
      -1,    90,    47,    88,    54,    -1,     5,    -1,     4,    -1,
      48,    88,    52,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    84,    84,    85,    89,    90,    94,    96,    98,   100,
     102,   104,   106,   108,   110,   112,   114,   118,   119,   121,
     126,   128,   133,   134,   136,   141,   146,   147,   149,   154,
     156,   158,   160,   165,   166,   168,   173,   175,   177,   179,
     181,   186,   188,   190,   195,   197,   202,   204,   210,   211,
     213,   217,   221,   226,   228,   230,   235,   236,   238,   240,
     244,   246,   248,   253,   254,   256,   258,   260,   265,   267,
     269,   274,   275,   279,   284,   289,   294,   299,   305,   306,
     311,   314,   316,   318,   324,   325,   329,   330,   332,   334,
     336,   338,   340,   342,   346,   351,   352,   354,   359,   361,
     362,   363
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SERROR", "CONSTANT", "IDENTIFIER",
  "RELOP", "STRING", "TYPENAME", "TYPE", "MODE", "DISTMODE", "MODULE",
  "COMPILE_OPTIONS", "GLOBALS", "DEFINE", "DEFCLASS", "DEFSTATE",
  "DEFMETHOD", "CALLS", "REQUIRED", "BACKEND", "SHRINK", "CALCORDER",
  "LIBRARY", "FORTRANFORMAT", "FORTRANSTRINGCONVENTION", "CALLBACK",
  "LANGUAGE", "LINKER_DEF", "COMPILER_DEF", "'?'", "':'", "'|'", "'^'",
  "'&'", "RSHIFT", "LSHIFT", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'",
  "'~'", "UNARY", "UNARY_HIGH_PRIORITY", "'['", "'('", "';'", "'{'", "'}'",
  "')'", "','", "']'", "$accept", "program", "declaration_list",
  "declaration", "define_list", "define_item", "defmethod_option_list",
  "defmethod_option", "defclass_option_list", "defclass_option",
  "option_list", "decl_option", "interface_definition", "callback_list",
  "parameter_callback_list", "parameter_list", "callback", "parameter",
  "decl_specifier", "type_specifier", "mode_specifier", "declarator",
  "range_spec", "opt_string", "required", "backend", "shrink", "language",
  "calcorder", "interface_body", "globals_body", "id_list", "expr_or_null",
  "expr", "unary_expr", "primary_expr", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,    63,    58,   124,    94,    38,   286,   287,    43,    45,
      42,    47,    37,    33,   126,   288,   289,    91,    40,    59,
     123,   125,    41,    44,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    55,    56,    56,    57,    57,    58,    58,    58,    58,
      58,    58,    58,    58,    58,    58,    58,    59,    59,    59,
      60,    60,    61,    61,    61,    62,    63,    63,    63,    64,
      64,    64,    64,    65,    65,    65,    66,    66,    66,    66,
      66,    67,    67,    67,    68,    68,    69,    69,    70,    70,
      70,    71,    72,    73,    73,    73,    74,    74,    74,    74,
      75,    75,    75,    76,    76,    76,    76,    76,    77,    77,
      77,    78,    78,    79,    80,    81,    82,    83,    84,    84,
      85,    86,    86,    86,    87,    87,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    89,    89,    89,    90,    90,
      90,    90
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     2,     3,     3,     2,     3,
       3,     3,     5,     7,     3,     3,     1,     0,     1,     2,
       2,     5,     0,     1,     2,     1,     0,     1,     2,     1,
       1,     1,     1,     0,     1,     2,     1,     1,     1,     1,
       1,     4,     4,     4,     1,     3,     3,     3,     0,     1,
       3,     4,     2,     2,     2,     3,     1,     2,     3,     1,
       1,     2,     2,     1,     3,     4,     6,     2,     1,     3,
       5,     0,     1,     2,     2,     2,     2,     2,     1,     7,
       1,     1,     3,     0,     1,     0,     1,     3,     3,     3,
       3,     3,     3,     3,     5,     1,     2,     2,     4,     1,
       1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    16,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     4,     0,     0,    80,     8,     0,
      71,    71,     0,     0,     0,     0,     0,     1,     5,     6,
       7,    48,    72,    33,    26,     9,    10,    11,    15,    14,
       0,    59,    56,    60,     0,     0,     0,     0,    44,    49,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    34,
      36,    37,    38,    40,    39,     0,    27,    29,    30,    31,
      32,    48,    57,    61,    62,    43,     0,    42,     0,    41,
       0,    63,     0,     0,    52,    54,    53,    73,    74,    75,
     100,    99,     0,     0,     0,    77,    86,    95,    76,    71,
      78,    35,    12,    17,    28,     0,    58,    45,    47,    46,
      50,    67,     0,    85,    55,    97,    96,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    18,    51,     0,    64,     0,    84,   101,    93,     0,
      92,    89,    90,    91,    87,    88,     0,     0,    20,    71,
      13,    19,     0,    65,     0,    98,    83,    22,     0,    68,
      94,    81,     0,     0,    23,    25,    66,     0,     0,     0,
      24,    21,    69,    79,    82,     0,    70
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    12,    13,    14,   130,   131,   163,   164,    65,    66,
      58,    59,    20,    45,    46,    47,    48,    49,    50,    51,
      52,    84,   158,    33,    60,    61,    62,    63,    64,   102,
      18,   162,   135,    95,    96,    97
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -145
static const yytype_int16 yypact[] =
{
     158,  -145,    15,    19,   -13,    53,    55,    57,    78,    87,
      90,   111,   108,   177,  -145,    45,    89,  -145,  -145,    86,
     129,   129,    95,   102,   117,   127,   132,  -145,  -145,  -145,
    -145,   200,  -145,   176,   147,  -145,  -145,  -145,  -145,  -145,
     112,  -145,   191,   212,   214,    62,    69,    94,  -145,  -145,
       2,    70,   141,   218,   219,   220,     1,   221,    46,  -145,
    -145,  -145,  -145,  -145,  -145,   -12,  -145,  -145,  -145,  -145,
    -145,   209,   222,  -145,  -145,  -145,   224,  -145,   224,  -145,
     200,  -145,     2,     2,   183,   141,  -145,  -145,  -145,  -145,
    -145,  -145,     1,     1,     1,   114,  -145,   185,  -145,   129,
    -145,  -145,  -145,   162,  -145,   133,  -145,  -145,  -145,  -145,
    -145,   183,   -35,     1,  -145,  -145,  -145,    85,     1,     1,
       1,     1,     1,     1,     1,     1,     1,   228,   184,    53,
      -3,  -145,  -145,   209,  -145,    21,   114,  -145,   174,   101,
      71,    31,    31,  -145,  -145,  -145,     5,   187,  -145,   129,
    -145,  -145,     1,  -145,     1,  -145,   231,   215,   186,    48,
     123,  -145,   142,    33,  -145,  -145,  -145,     1,   188,   234,
    -145,  -145,    64,  -145,  -145,     1,   114
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -145,  -145,  -145,   229,  -145,   113,  -145,    81,  -145,   180,
    -145,   189,   119,  -145,  -145,   170,   -55,   -76,  -145,   -34,
     195,   139,  -145,   -21,   -10,    28,    65,    66,  -144,    88,
    -145,  -145,  -145,   -91,  -145,  -145
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -4
static const yytype_int16 yytable[] =
{
      34,   115,   116,   117,   110,    90,    91,    81,    53,    54,
      55,   118,   113,   165,   128,   129,    57,   134,    86,   165,
      15,   107,   136,   108,    67,   109,    16,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   119,    17,   103,   120,
      92,    93,    82,   121,   122,   123,   124,   125,   150,    94,
      83,   114,    99,   152,   118,    67,    56,   110,    19,   155,
      21,   159,    68,   160,    22,    99,    53,    54,    55,    56,
     118,   123,   124,   125,    57,   153,   172,   118,   127,   119,
      43,    44,   120,   100,   176,    23,   121,   122,   123,   124,
     125,   118,    24,    68,    29,   119,   100,    25,   120,    69,
      70,   167,   121,   122,   123,   124,   125,   118,    27,   121,
     122,   123,   124,   125,    75,    76,   119,   175,    26,   120,
     118,    77,    78,   121,   122,   123,   124,   125,   157,   118,
      69,    70,   119,   154,    31,   120,    32,   137,    30,   121,
     122,   123,   124,   125,    35,   119,    79,    80,   120,    41,
      42,    36,   121,   122,   123,   124,   125,   120,    -2,     1,
      71,   121,   122,   123,   124,   125,    37,    53,    54,    55,
       2,     3,     4,     5,     6,    57,    38,    -3,     1,   128,
     129,    39,     7,     8,     9,   132,   133,    10,    11,     2,
       3,     4,     5,     6,   168,   169,    53,    54,    55,    56,
      72,     7,     8,     9,    57,    40,    10,    11,    41,    42,
      43,    44,   121,   122,   123,   124,   125,    41,    42,    43,
      44,   111,   112,    73,    74,    87,    88,    89,    98,    40,
     113,   106,   126,   147,   148,   156,   161,   173,    56,   174,
     166,   105,    28,   151,   170,   104,    85,   101,   149,     0,
       0,   171
};

static const yytype_int16 yycheck[] =
{
      21,    92,    93,    94,    80,     4,     5,     5,    20,    21,
      22,     6,    47,   157,    17,    18,    28,    52,    52,   163,
       5,    76,   113,    78,    34,    80,     7,   118,   119,   120,
     121,   122,   123,   124,   125,   126,    31,    50,    50,    34,
      39,    40,    40,    38,    39,    40,    41,    42,    51,    48,
      48,    85,    19,    32,     6,    65,    23,   133,     5,    54,
       5,   152,    34,   154,     7,    19,    20,    21,    22,    23,
       6,    40,    41,    42,    28,    54,   167,     6,    99,    31,
      10,    11,    34,    50,   175,     7,    38,    39,    40,    41,
      42,     6,     5,    65,    49,    31,    50,     7,    34,    34,
      34,    53,    38,    39,    40,    41,    42,     6,     0,    38,
      39,    40,    41,    42,    52,    53,    31,    53,     7,    34,
       6,    52,    53,    38,    39,    40,    41,    42,   149,     6,
      65,    65,    31,    32,    48,    34,     7,    52,    49,    38,
      39,    40,    41,    42,    49,    31,    52,    53,    34,     8,
       9,    49,    38,    39,    40,    41,    42,    34,     0,     1,
      48,    38,    39,    40,    41,    42,    49,    20,    21,    22,
      12,    13,    14,    15,    16,    28,    49,     0,     1,    17,
      18,    49,    24,    25,    26,    52,    53,    29,    30,    12,
      13,    14,    15,    16,    52,    53,    20,    21,    22,    23,
       9,    24,    25,    26,    28,     5,    29,    30,     8,     9,
      10,    11,    38,    39,    40,    41,    42,     8,     9,    10,
      11,    82,    83,    11,    10,     7,     7,     7,     7,     5,
      47,     9,    47,     5,    50,    48,     5,    49,    23,     5,
      54,    71,    13,   130,   163,    65,    51,    58,   129,    -1,
      -1,   163
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,    12,    13,    14,    15,    16,    24,    25,    26,
      29,    30,    56,    57,    58,     5,     7,    50,    85,     5,
      67,     5,     7,     7,     5,     7,     7,     0,    58,    49,
      49,    48,     7,    78,    78,    49,    49,    49,    49,    49,
       5,     8,     9,    10,    11,    68,    69,    70,    71,    72,
      73,    74,    75,    20,    21,    22,    23,    28,    65,    66,
      79,    80,    81,    82,    83,    63,    64,    79,    80,    81,
      82,    48,     9,    11,    10,    52,    53,    52,    53,    52,
      53,     5,    40,    48,    76,    75,    74,     7,     7,     7,
       4,     5,    39,    40,    48,    88,    89,    90,     7,    19,
      50,    66,    84,    50,    64,    70,     9,    71,    71,    71,
      72,    76,    76,    47,    74,    88,    88,    88,     6,    31,
      34,    38,    39,    40,    41,    42,    47,    78,    17,    18,
      59,    60,    52,    53,    52,    87,    88,    52,    88,    88,
      88,    88,    88,    88,    88,    88,    88,     5,    50,    67,
      51,    60,    32,    54,    32,    54,    48,    78,    77,    88,
      88,     5,    86,    61,    62,    83,    54,    53,    52,    53,
      62,    84,    88,    49,     5,    53,    88
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:
#line 95 "nggenIdl.y"
    { nggen_cpl_module((yyvsp[(2) - (3)].val)); }
    break;

  case 7:
#line 97 "nggenIdl.y"
    { nggen_cpl_compile_options((yyvsp[(2) - (3)].val)); }
    break;

  case 8:
#line 99 "nggenIdl.y"
    { nggen_cpl_globals((yyvsp[(2) - (2)].val)); }
    break;

  case 9:
#line 101 "nggenIdl.y"
    { nggen_cpl_library((yyvsp[(2) - (3)].val)); }
    break;

  case 10:
#line 103 "nggenIdl.y"
    { nggen_cpl_fortranformat((yyvsp[(2) - (3)].val));}
    break;

  case 11:
#line 105 "nggenIdl.y"
    { nggen_cpl_fortranstringconvention((yyvsp[(2) - (3)].val));}
    break;

  case 12:
#line 107 "nggenIdl.y"
    { nggen_cpl_interface((yyvsp[(2) - (5)].val),(yyvsp[(3) - (5)].val),(yyvsp[(4) - (5)].val),(yyvsp[(5) - (5)].val)); }
    break;

  case 13:
#line 109 "nggenIdl.y"
    { nggen_cpl_class((yyvsp[(2) - (7)].val),(yyvsp[(3) - (7)].val),(yyvsp[(4) - (7)].val),(yyvsp[(6) - (7)].val)); }
    break;

  case 14:
#line 111 "nggenIdl.y"
    { nggen_cpl_compiler((yyvsp[(2) - (3)].val));}
    break;

  case 15:
#line 113 "nggenIdl.y"
    { nggen_cpl_linker((yyvsp[(2) - (3)].val));}
    break;

  case 17:
#line 118 "nggenIdl.y"
    {(yyval.val) = NULL;}
    break;

  case 18:
#line 120 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST, (yyvsp[(1) - (1)].val));}
    break;

  case 19:
#line 122 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (2)].val), (yyvsp[(2) - (2)].val));}
    break;

  case 20:
#line 127 "nggenIdl.y"
    { (yyval.val) = nggen_read_rest_of_body(FALSE); }
    break;

  case 21:
#line 129 "nggenIdl.y"
    { (yyval.val) = nggen_list4(LIST, (yyvsp[(2) - (5)].val),(yyvsp[(3) - (5)].val),(yyvsp[(4) - (5)].val),(yyvsp[(5) - (5)].val)); }
    break;

  case 22:
#line 133 "nggenIdl.y"
    {(yyval.val) = NULL;}
    break;

  case 23:
#line 135 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST, (yyvsp[(1) - (1)].val));}
    break;

  case 24:
#line 137 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (2)].val), (yyvsp[(2) - (2)].val));}
    break;

  case 25:
#line 142 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 26:
#line 146 "nggenIdl.y"
    {(yyval.val) = NULL;}
    break;

  case 27:
#line 148 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST, (yyvsp[(1) - (1)].val));}
    break;

  case 28:
#line 150 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (2)].val), (yyvsp[(2) - (2)].val));}
    break;

  case 29:
#line 155 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 30:
#line 157 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 31:
#line 159 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 32:
#line 161 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 33:
#line 165 "nggenIdl.y"
    {(yyval.val) = NULL;}
    break;

  case 34:
#line 167 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST, (yyvsp[(1) - (1)].val));}
    break;

  case 35:
#line 169 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (2)].val), (yyvsp[(2) - (2)].val));}
    break;

  case 36:
#line 174 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 37:
#line 176 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 38:
#line 178 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 39:
#line 180 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 40:
#line 182 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val);}
    break;

  case 41:
#line 187 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (4)].val),(yyvsp[(3) - (4)].val)); }
    break;

  case 42:
#line 189 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (4)].val),(yyvsp[(3) - (4)].val)); }
    break;

  case 43:
#line 191 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (4)].val),(yyvsp[(3) - (4)].val)); }
    break;

  case 44:
#line 196 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST,(yyvsp[(1) - (1)].val)); }
    break;

  case 45:
#line 198 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 46:
#line 203 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 47:
#line 205 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 48:
#line 210 "nggenIdl.y"
    { (yyval.val) = NULL;}
    break;

  case 49:
#line 212 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST,(yyvsp[(1) - (1)].val)); }
    break;

  case 50:
#line 214 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 51:
#line 218 "nggenIdl.y"
    { (yyval.val) = nggen_list2(CALLBACK_FUNC, (yyvsp[(1) - (4)].val), (yyvsp[(3) - (4)].val)); }
    break;

  case 52:
#line 222 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (2)].val),(yyvsp[(2) - (2)].val)); }
    break;

  case 53:
#line 227 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (2)].val),(yyvsp[(2) - (2)].val)); }
    break;

  case 54:
#line 229 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(2) - (2)].val),(yyvsp[(1) - (2)].val)); }
    break;

  case 55:
#line 231 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(2) - (3)].val),nggen_list2(LIST,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val))); }
    break;

  case 57:
#line 237 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (2)].val),(yyvsp[(2) - (2)].val)); }
    break;

  case 58:
#line 239 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (3)].val),nggen_list2(LIST,(yyvsp[(2) - (3)].val),(yyvsp[(3) - (3)].val))); }
    break;

  case 60:
#line 245 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (1)].val),NULL); }
    break;

  case 61:
#line 247 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(1) - (2)].val),(yyvsp[(2) - (2)].val)); }
    break;

  case 62:
#line 249 "nggenIdl.y"
    { (yyval.val) = nggen_list2(LIST,(yyvsp[(2) - (2)].val),(yyvsp[(1) - (2)].val)); }
    break;

  case 64:
#line 255 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(2) - (3)].val); }
    break;

  case 65:
#line 257 "nggenIdl.y"
    { (yyval.val) = nggen_list3(ARRAY_REF,(yyvsp[(1) - (4)].val),(yyvsp[(3) - (4)].val),NULL); }
    break;

  case 66:
#line 259 "nggenIdl.y"
    { (yyval.val) = nggen_list3(ARRAY_REF,(yyvsp[(1) - (6)].val),(yyvsp[(3) - (6)].val),(yyvsp[(5) - (6)].val)); }
    break;

  case 67:
#line 261 "nggenIdl.y"
    { (yyval.val) = nggen_list1(POINTER_REF,(yyvsp[(2) - (2)].val)); }
    break;

  case 68:
#line 266 "nggenIdl.y"
    { (yyval.val) = nggen_list3(LIST,NULL,(yyvsp[(1) - (1)].val),NULL); }
    break;

  case 69:
#line 268 "nggenIdl.y"
    { (yyval.val) = nggen_list3(LIST,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val),NULL); }
    break;

  case 70:
#line 270 "nggenIdl.y"
    { (yyval.val) = nggen_list3(LIST,(yyvsp[(1) - (5)].val),(yyvsp[(3) - (5)].val),(yyvsp[(5) - (5)].val)); }
    break;

  case 71:
#line 274 "nggenIdl.y"
    { (yyval.val) = NULL; }
    break;

  case 73:
#line 280 "nggenIdl.y"
    { (yyval.val) = nggen_cpl_required((yyvsp[(2) - (2)].val)); }
    break;

  case 74:
#line 285 "nggenIdl.y"
    { (yyval.val) = nggen_cpl_backend((yyvsp[(2) - (2)].val));}
    break;

  case 75:
#line 290 "nggenIdl.y"
    { (yyval.val) = nggen_cpl_shrink((yyvsp[(2) - (2)].val));}
    break;

  case 76:
#line 295 "nggenIdl.y"
    { (yyval.val) = nggen_cpl_language((yyvsp[(2) - (2)].val)); }
    break;

  case 77:
#line 300 "nggenIdl.y"
    { (yyval.val) = nggen_cpl_calcorder((yyvsp[(2) - (2)].val));}
    break;

  case 78:
#line 305 "nggenIdl.y"
    { (yyval.val) = nggen_read_rest_of_body(1); }
    break;

  case 79:
#line 307 "nggenIdl.y"
    { (yyval.val) = nggen_list3(LIST,(yyvsp[(2) - (7)].val),(yyvsp[(3) - (7)].val),(yyvsp[(5) - (7)].val)); }
    break;

  case 80:
#line 311 "nggenIdl.y"
    { (yyval.val) = nggen_read_rest_of_body(0); }
    break;

  case 81:
#line 315 "nggenIdl.y"
    { (yyval.val) = nggen_list1(LIST,(yyvsp[(1) - (1)].val)); }
    break;

  case 82:
#line 317 "nggenIdl.y"
    { (yyval.val) = nggen_list_put_last((yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 83:
#line 318 "nggenIdl.y"
    { (yyval.val) = NULL; }
    break;

  case 85:
#line 325 "nggenIdl.y"
    { (yyval.val) = NULL; }
    break;

  case 87:
#line 331 "nggenIdl.y"
    { (yyval.val) = nggen_list2(DIV_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 88:
#line 333 "nggenIdl.y"
    { (yyval.val) = nggen_list2(MOD_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 89:
#line 335 "nggenIdl.y"
    { (yyval.val) = nggen_list2(PLUS_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 90:
#line 337 "nggenIdl.y"
    { (yyval.val) = nggen_list2(MINUS_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 91:
#line 339 "nggenIdl.y"
    { (yyval.val) = nggen_list2(MUL_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 92:
#line 341 "nggenIdl.y"
    { (yyval.val) = nggen_list2(POW_EXPR,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 93:
#line 343 "nggenIdl.y"
    { yystype * tmp = (yystype *)(&((yyvsp[(2) - (3)].val)));
      (yyval.val) = nggen_list2(tmp->code,(yyvsp[(1) - (3)].val),(yyvsp[(3) - (3)].val)); }
    break;

  case 94:
#line 347 "nggenIdl.y"
    { (yyval.val) = nggen_list3(TRY_EXPR,(yyvsp[(1) - (5)].val),(yyvsp[(3) - (5)].val),(yyvsp[(5) - (5)].val)); }
    break;

  case 96:
#line 353 "nggenIdl.y"
    { (yyval.val) = nggen_list1(POINTER_REF,(yyvsp[(2) - (2)].val)); }
    break;

  case 97:
#line 355 "nggenIdl.y"
    { (yyval.val) = nggen_list1(UNARY_MINUS_EXPR,(yyvsp[(2) - (2)].val)); }
    break;

  case 98:
#line 360 "nggenIdl.y"
    { (yyval.val) = nggen_list2(ARRAY_REF,(yyvsp[(1) - (4)].val),(yyvsp[(3) - (4)].val)); }
    break;

  case 101:
#line 364 "nggenIdl.y"
    { (yyval.val) = (yyvsp[(2) - (3)].val); }
    break;


/* Line 1267 of yacc.c.  */
#line 2000 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 367 "nggenIdl.y"


#include "nggenIdlLex.c"





