/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

