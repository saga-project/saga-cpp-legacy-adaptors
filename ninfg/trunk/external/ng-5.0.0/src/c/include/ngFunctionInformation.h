/* 
 * $RCSfile: ngFunctionInformation.h,v $ $Revision: 1.1 $ $Date: 2006/12/18 08:29:44 $
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
#ifndef _NGFUNCTIONINFORMATION_H_
#define _NGFUNCTIONINFORMATION_H_

/**
 * This file define the Data Structures and Constant Values of Remote Class
 * and Remote Method.
 */

#define NGI_METHOD_ID_UNDEFINED	(-1)

#define NGI_METHOD_NAME_DEFAULT "__default__"

/**
 * Type of operation code for expression.
 */
typedef enum ngOperationCode_e {
    NG_EXPRESSION_OPCODE_PLUS          = 1,	/* '+' */
    NG_EXPRESSION_OPCODE_MINUS         = 2,	/* '-' */
    NG_EXPRESSION_OPCODE_MULTIPLY      = 3,	/* '*' */
    NG_EXPRESSION_OPCODE_DIVIDE        = 4,	/* '/' */
    NG_EXPRESSION_OPCODE_MODULO        = 5,	/* '%' */
    NG_EXPRESSION_OPCODE_UNARY_MINUS   = 6,	/* '-' sign. ex: "-100" */
    NG_EXPRESSION_OPCODE_POWER         = 7,	/* '^' */
    NG_EXPRESSION_OPCODE_EQUAL         = 8,	/* "==" */
    NG_EXPRESSION_OPCODE_NOT_EQUAL     = 9,	/* "!=" */
    NG_EXPRESSION_OPCODE_GREATER_THAN  = 10,	/* '>' */
    NG_EXPRESSION_OPCODE_LESS_THAN     = 11,	/* '<' */
    NG_EXPRESSION_OPCODE_GREATER_EQUAL = 12,	/* ">=" */
    NG_EXPRESSION_OPCODE_LESS_EQUAL    = 13,	/* "<=" */
    NG_EXPRESSION_OPCODE_TRI           = 14	/* "? : " */
} ngExpressionOperationCode_t;

/**
 * Type of value for expression.
 */
typedef enum ngExpressionValueType_e {
    NG_EXPRESSION_VALUE_TYPE_ERROR       = -1,	/* Error */
    NG_EXPRESSION_VALUE_TYPE_NONE        = 0,	/* None */
    NG_EXPRESSION_VALUE_TYPE_CONSTANT    = 1,	/* Constant value */
    NG_EXPRESSION_VALUE_TYPE_IN_ARGUMENT = 2,	/* Specified by scaler argument of IN */
    NG_EXPRESSION_VALUE_TYPE_OPCODE      = 3,	/* Operation code */
    NG_EXPRESSION_VALUE_TYPE_END         = 4	/* The end of array of argument */
#if 0 /* Are these necessary? */
    NG_EXPRESSION_VALUE_NTYPES			/* Number of types of this enum */
#endif
} ngExpressionValueType_t;

/**
 * IN/OUT mode of argument.
 */
typedef enum ngArgumentIOmode_e {
    NG_ARGUMENT_IO_MODE_NONE  = 0,	/* None */
    NG_ARGUMENT_IO_MODE_IN    = 1,	/* IN: Transfer from client to remote */
    NG_ARGUMENT_IO_MODE_OUT   = 2,	/* OUT: Transfer from remote to client */
    NG_ARGUMENT_IO_MODE_INOUT = 3,	/* INOUT: Transfer both directions */
    NG_ARGUMENT_IO_MODE_WORK  = 4	/* WORK: Allocated at remote side */
} ngArgumentIOmode_t;

/**
 * Type of data of argument.
 */
typedef enum ngArgumentDataType_e {
    NG_ARGUMENT_DATA_TYPE_UNDEFINED = 0,	/* Undefined */
    NG_ARGUMENT_DATA_TYPE_CHAR      = 1,	/* char */
    NG_ARGUMENT_DATA_TYPE_SHORT     = 2,	/* short */
    NG_ARGUMENT_DATA_TYPE_INT       = 3,	/* int */
    NG_ARGUMENT_DATA_TYPE_LONG      = 4,	/* long */
    NG_ARGUMENT_DATA_TYPE_FLOAT     = 0x11,	/* float */
    NG_ARGUMENT_DATA_TYPE_DOUBLE    = 0x12,	/* double */
    NG_ARGUMENT_DATA_TYPE_SCOMPLEX  = 0x13,	/* scomplex */
    NG_ARGUMENT_DATA_TYPE_DCOMPLEX  = 0x14,	/* dcomplex */
    NG_ARGUMENT_DATA_TYPE_STRING    = 0x21,	/* string */
    NG_ARGUMENT_DATA_TYPE_FILENAME  = 0x22,	/* filename */
    NG_ARGUMENT_DATA_TYPE_CALLBACK  = 0x23	/* callback function */
} ngArgumentDataType_t;

/**
 * Backend of remote Environment.
 */
typedef enum ngBackend_e {
    NG_BACKEND_NORMAL,		/* Normal */
    NG_BACKEND_MPI,		/* MPI */
    NG_BACKEND_BLACS		/* Blacs */
} ngBackend_t;

/**
 * Type of language.
 */
typedef enum ngClassLanguage_e {
    NG_CLASS_LANGUAGE_C,	/* C */
    NG_CLASS_LANGUAGE_FORTRAN	/* FORTRAN */
} ngClassLanguage_t;

/**
 * Element of Expression.
 */
typedef struct ngExpressionElement_s {
    ngExpressionValueType_t	ngee_valueType;	/* Type of value */
    int	ngee_value;	/* Value of expression */
} ngExpressionElement_t;

/**
 * Information about Subscript.
 */
typedef struct ngSubscriptInformation_s {
    ngExpressionElement_t	*ngsi_size;	/* Number of elements of array */
    ngExpressionElement_t	*ngsi_start;	/* Start position of transmission */
    ngExpressionElement_t	*ngsi_end;	/* End position of transmission */
    ngExpressionElement_t	*ngsi_skip;	/* Skip count of transmission */
} ngSubscriptInformation_t;

/**
 * Information about Argument.
 */
typedef struct ngArgumentInformation_s {
    ngArgumentIOmode_t		ngai_ioMode;	/* IN/OUT mode of argument */
    ngArgumentDataType_t	ngai_dataType;	/* Data type of argument */
    int	ngai_nDimensions;	/* Number of dimensions of array */
    ngSubscriptInformation_t	*ngai_subscript;	/* Subscript */
    struct ngRemoteMethodInformation_s	*ngai_callback;	/* Information about callback function */
} ngArgumentInformation_t;

/**
 * Information about Remote Method.
 */
typedef struct ngRemoteMethodInformation_s {
    char	*ngrmi_methodName;	/* Method name */
    int		ngrmi_methodID;		/* Method ID */
    ngExpressionElement_t	*ngrmi_calculationOrder; /* Calculation order */

    /* Argument Information */
    int		ngrmi_nArguments;	/* Number of arguments */
    ngArgumentInformation_t	*ngrmi_arguments;

    /* Thin out the element of array, which saved into Executable */
    int		ngrmi_shrink;

    char       *ngrmi_description;	/* Description of this method */
} ngRemoteMethodInformation_t;

/**
 * Information about Remote Class.
 */
typedef struct ngRemoteClassInformation_s {
    char       *ngrci_className;	/* Class name */
    char       *ngrci_version;		/* Version number of this class */
    char       *ngrci_description;	/* Description of this class */
    ngBackend_t	ngrci_backend;		/* Backend */
    ngClassLanguage_t ngrci_language;	/* Type of language */
    int		ngrci_nMethods;		/* Number of methods */
    ngRemoteMethodInformation_t   *ngrci_method; /* Array of Remote Method Information */
} ngRemoteClassInformation_t;

#endif /* _NGFUNCTIONINFORMATION_H */
