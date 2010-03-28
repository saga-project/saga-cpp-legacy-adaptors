#define NGI_SIGWAIT_HAS_BUG 1
#define HAVE_ALLOCA_H 1
#define HAVE_ALLOCA 1
#define HAVE_STDARG_H 1
#define HAVE_LOCALE_H 1
#define HAVE_SETLOCALE 1
#define NO_IEEEFP_H 1
#define HAVE_STRDUP 1
#define HAVE_SYS_TIME_H 1
#define TIME_WITH_SYS_TIME 1
#define STDC_HEADERS 1
#define RETSIGTYPE void
#define HAS_LONGLONG 1
#define HAS_LONGDOUBLE 1
#define SIZEOF_UNSIGNED_SHORT 2
#define SIZEOF_UNSIGNED_INT 4
#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_DOUBLE 16
#define SIZEOF_VOID_P 4
#define CHAR_ALIGN 1
#define SHORT_ALIGN 2
#define INT_ALIGN 4
#define LONG_ALIGN 4
#define LONGLONG_ALIGN 4
#define FLOAT_ALIGN 4
#define DOUBLE_ALIGN 4
#define LONGDOUBLE_ALIGN 16
#define HAS_INT16 1
#define HAS_INT32 1
#define HAS_INT64 1
#ifndef NG_CPU_
#define NG_CPU_ 1
#endif /* !NG_CPU_ */
#ifndef NG_OS_
#define NG_OS_ 1
#endif /* !NG_OS_ */
/* 
 * $RCSfile: ngPlatform.h.in,v $ $Revision: 1.4 $ $Date: 2003/10/28 05:33:34 $
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

#ifndef _NGCONFIG_H
#define _NGCONFIG_H

#undef _ANSI_ARGS_
#undef CONST

#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE) || defined(__USE_FIXED_PROTOTYPES__)
#define _USING_PROTOTYPES_ 1
#define _ANSI_ARGS_(x) x
#define CONST const
#else
#define _ANSI_ARGS_(x) ()
#define CONST /**/
#endif /* __STDC__ ||.... */

#include <stdio.h>

#ifndef NO_STRING_H
#include <string.h>
#endif /* !NO_STRING_H */

#ifndef NO_UNISTD_H
#include <unistd.h>
#endif /* !NO_UNISTD_H */

#ifndef NO_LIMITS_H
#include <limits.h>
#endif /* !NO_LIMITS_H */

#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif /* !NO_STDLIB_H */

#include <ctype.h>

#include <errno.h>

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif /* HAVE_SYS_TIME_H */
#endif /* TIME_WITH_SYS_TIME */

#ifndef NO_NETDB_H
#include <netdb.h>
#endif /* !NO_NETDB_H */

#ifndef NO_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* !NO_SYS_SOCKET_H */

#ifndef NO_NETINET_IN_H
#include <netinet/in.h>
#endif /* !NO_NETINET_IN_H */

#ifndef NO_ARPA_INET_H
#include <arpa/inet.h>
#endif /* !NO_ARPA_INET_H */

#ifndef NO_RESOURCE_H
#include <sys/resource.h>
#endif /* !NO_RESOURCE_H */

#ifdef GETTOD_NOT_DECLARED
extern int      gettimeofday _ANSI_ARGS_((struct timeval *tp, struct timezone *tzp));
#endif /* GETTOD_NOT_DECLARED */

#ifdef HAVE_BSDGETTIMEOFDAY
#define gettimeofday(X, Y) BSDgettimeofday((X), (Y))
#endif /* HAVE_BSDGETTIMEOFDAY */

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif /* HAVE_LOCALE_H */

#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif /* HAVE_NET_ERRNO_H */

#if defined(__STDC__) || defined(HAVE_STDARG_H)
#   include <stdarg.h>
#   define NG_VARARGS(type, name) (type name, ...)
#   define NG_VARARGS_DEF(type, name) (type name, ...)
#   define NG_VARARGS_START(type, name, list) (va_start(list, name), name)
#else
#   include <varargs.h>
#   ifdef __cplusplus
#       define NG_VARARGS(type, name) (type name, ...)
#       define NG_VARARGS_DEF(type, name) (type va_alist, ...)
#   else
#       define NG_VARARGS(type, name) ()
#       define NG_VARARGS_DEF(type, name) (va_alist)
#   endif
#   define NG_VARARGS_START(type, name, list) \
        type name = (va_start(list), va_arg(list, type))
#endif

#ifndef HAVE_STRDUP
extern char	*strdup _ANSI_ARGS_((char *s));
#endif /* HAVE_STRDUP */

/*
 * alloca(). If using gcc, do nothing.
 */
#ifndef __GNUC__
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef NG_OS_AIX
#pragma alloca
#  else
#   ifndef alloca /* predefined by HP cc +Olibcalls */
void *	alloca _ANSI_ARGS_((size_t));
#   endif /* !alloca */
#  endif /* NG_OS_AIX */
# endif /* HAVE_ALLOCA_H */
#endif /* !__GNUC__ */

#ifdef HAS_INT16
typedef short _ngInt16_t;
typedef unsigned short _ngUint16_t;
typedef _ngInt16_t _ngShort;
typedef _ngUint16_t _ngUshort;
#endif /* HAS_INT16 */

#ifdef HAS_INT32
typedef int _ngInt32_t;
typedef unsigned int _ngUint32_t;
typedef _ngInt32_t _ngInt;
typedef _ngUint32_t _ngUint;
#endif /* HAS_INT32 */

#ifdef HAS_INT64
typedef long long int _ngInt64_t;
typedef unsigned long long int _ngUint64_t;
typedef _ngInt64_t _ngLonglongint;
typedef _ngUint64_t _ngUlonglongint;
#endif /* HAS_INT64 */

typedef unsigned int _ngAddrInt_t;
#endif /* _NGCONFIG_H */
