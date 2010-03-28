/*
 * $RCSfile: ngEnvironment.h,v $ $Revision: 1.13 $ $Date: 2008/02/06 03:16:05 $
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
#ifndef _NG_ENVIRONMENT_H_
#define _NG_ENVIRONMENT_H_

/**
 * This file include the system header files,
 * which was checked by configure script.
 * The configure result depend on each operating environment.
 */

#if HAVE_CONFIG_H
#include "ngConfig.h"
#else /* HAVE_CONFIG_H */
    NO_HAVE_CONFIG_H_DEFINED_ERROR
#endif /* HAVE_CONFIG_H */


#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_STDDEF_H
#include <stddef.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_ASSERT_H
#include <assert.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_PWD_H
#include <pwd.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_ENDIAN_H
#include <endian.h>
#endif

#if HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif

#if HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# endif
# if HAVE_TIME_H
#  include <time.h>
# endif
#endif

#if HAVE_SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#if HAVE_RPC_RPC_H
#include <rpc/rpc.h>
#endif

#if HAVE_RPC_TYPES_H
#include <rpc/types.h>
#endif

#if HAVE_RPC_XDR_H
#include <rpc/xdr.h>
#endif

#if HAVE_SCHED_H
#include <sched.h>
#endif

#if HAVE_POLL_H
#include <poll.h>
#endif

#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_ZLIB_H
#include <zlib.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#if HAVE_PWD_H
#include <pwd.h>
#endif

#if NG_ATTRIBUTE_PRINTF_ENABLE
#define NG_ATTRIBUTE_PRINTF(p1, p2) \
    __attribute__ ((__format__ (__printf__, p1, p2)))
#else
#define NG_ATTRIBUTE_PRINTF(p1, p2)
#endif

#if HAVE_SOCKLEN_T
typedef socklen_t ngiSockLen_t;
#else
typedef int       ngiSockLen_t;
#endif

#if NGI_RCSID_EMBEDABLE
#define NGI_RCSID_EMBED(cvs_keywords) \
    static const char rcsid[] = cvs_keywords;
#else
#define NGI_RCSID_EMBED(cvs_keywords) /* just ignore. */
#endif

#if NGI_SIZET_CHECK_NEGATIVE
#define ngiSizetAssert(expr) assert(expr)
#define ngiSizetInvalidCheck(expr) (expr)
#else
#define ngiSizetAssert(expr) assert(1)
#define ngiSizetInvalidCheck(expr) (0)
#endif

#endif /* _NG_ENVIRONMENT_H_ */

