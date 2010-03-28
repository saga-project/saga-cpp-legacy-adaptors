# $RCSfile: ngConfigureValue.mk.in,v $ $Revision: 1.4 $ $Date: 2008/02/06 07:16:36 $
# $AIST_Release: 5.0.0 $
# $AIST_Copyright:
#  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#  National Institute of Advanced Industrial Science and Technology
#  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  $

#
# ngConfigureValue.mk: This file reflects configure result values.
#
# The line started by double sharp indicates empty line of output.
# To keep output equivalent with source, do not include tab character.
#

build       = i686-apple-darwin9.4.0
host        = i686-apple-darwin9.4.0

##
# TOPDIR is possibly no more exist.
TOPDIR      = /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0

##
PREFIX      = /Users/merzky/links/saga/svn/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/install
prefix      = /Users/merzky/links/saga/svn/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/install
exec_prefix = ${prefix}

##
BINDIR      = ${exec_prefix}/bin
LIBDIR      = ${exec_prefix}/lib
INCDIR      = ${prefix}/include
DOCDIR      = $(PREFIX)/doc
ETCDIR      = $(PREFIX)/etc

##
CC        = gcc
CPP       = gcc -E
IDL_CPP   = gcc -E -xc

RANLIB    = ranlib
AR        = /usr/bin/ar cr

YACC      = bison -y

# INSTALL refers $(TOPDIR).
#INSTALL   = /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/configure/aux/install-sh -c
MKDIR     = /bin/mkdir -p
TAR       = /usr/bin/tar
RM        = /bin/rm -f

PYTHON    = /usr/bin/python
JAVA_HOME = 
JAVA      = 
JAVAC     = 
JAR       = 
ANT       = true

##
NAREGI_SS_CLIENT_LIB_JAR = @NAREGI_SS_CLIENT_LIB_JAR@

##
CONF_CFLAGS    = -g -O2 -Wall -D_THREAD_SAFE 
CONF_CPPFLAGS  = 
CONF_DEFS      = -DHAVE_CONFIG_H
#CONF_INCLUDES  = -I$(TOPDIR)/src/c/include -I$(TOPDIR)/src/lib/utility -I$(TOPDIR)/src/lib/queue
CONF_LDFLAGS   = 
CONF_LIBS      = -lz  

##
INVOKE_SERVER_MODULES       = templ ssh gt4py gt2c
COMMUNICATION_PROXY_MODULES = ccommon  GT
INFORMATION_SERVICE_MODULES = ccommon  nrf

#INVOKE_SERVER_MODULES_ALL       = gt2c gt4py naregiss templ ssh
#COMMUNICATION_PROXY_MODULES_ALL = ccommon GT
#INFORMATION_SERVICE_MODULES_ALL = ccommon nrf

# ##
#MK_VARTMPL    = /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defvar.mk
#MK_DEFRULE    = /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

#
# Globus variables.
#

##
GLOBUS_LOCATION  = /usr/local/globus/
GLOBUS_VERSION   = 4.0.7

# GLOBUS_EMBED_LD_RUN_PATH = -Wl,-L/usr/local/globus//lib

# Globus variables are not printed because they are many output.
# and this information is not so important after the installation.

# for client.

# ##
# GLOBUS_CLIENT_CC       = /usr/bin/gcc
# GLOBUS_CLIENT_CPP      = /usr/bin/gcc -E
# GLOBUS_CLIENT_CFLAGS   = -g -fno-common -D_REENTRANT -Wall
# GLOBUS_CLIENT_CPPFLAGS = -I/usr/local/globus//include -I/usr/local/globus//include/gcc32dbgpthr -no-cpp-precomp -I/sw/include -DBIND_8_COMPAT
# GLOBUS_CLIENT_INCLUDES = -I/usr/local/globus//include/gcc32dbgpthr
# GLOBUS_CLIENT_LIBS     = -lpthread
# GLOBUS_CLIENT_LDFLAGS  = -L/usr/local/globus//lib -L/usr/local/globus//lib -L/sw/lib
# GLOBUS_CLIENT_PKG_LIBS = -lglobus_gram_client_gcc32dbgpthr -lglobus_gass_server_ez_gcc32dbgpthr -lglobus_gram_protocol_gcc32dbgpthr -lglobus_gass_transfer_gcc32dbgpthr -lglobus_io_gcc32dbgpthr -lglobus_xio_gcc32dbgpthr -lgssapi_error_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gsi_proxy_core_gcc32dbgpthr -lglobus_gsi_credential_gcc32dbgpthr -lglobus_gsi_callback_gcc32dbgpthr -lglobus_oldgaa_gcc32dbgpthr -lglobus_gsi_sysconfig_gcc32dbgpthr -lglobus_gsi_cert_utils_gcc32dbgpthr -lglobus_openssl_gcc32dbgpthr -lglobus_rsl_gcc32dbgpthr -lglobus_openssl_error_gcc32dbgpthr -lglobus_callout_gcc32dbgpthr -lglobus_proxy_ssl_gcc32dbgpthr -lglobus_common_gcc32dbgpthr -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr -lltdl_gcc32dbgpthr -lm

# for server.

# ##
# GLOBUS_SERVER_CC       = /usr/bin/gcc
# GLOBUS_SERVER_CPP      = /usr/bin/gcc -E
# GLOBUS_SERVER_CFLAGS   = -g -fno-common -D_REENTRANT -Wall
# GLOBUS_SERVER_CPPFLAGS = -I/usr/local/globus//include -I/usr/local/globus//include/gcc32dbgpthr -no-cpp-precomp -I/sw/include -DBIND_8_COMPAT
# GLOBUS_SERVER_INCLUDES = -I/usr/local/globus//include/gcc32dbgpthr
# GLOBUS_SERVER_LIBS     = -lpthread
# GLOBUS_SERVER_LDFLAGS  = -L/usr/local/globus//lib -L/usr/local/globus//lib -L/sw/lib
# GLOBUS_SERVER_PKG_LIBS = -lglobus_gram_client_gcc32dbgpthr -lglobus_gass_server_ez_gcc32dbgpthr -lglobus_gram_protocol_gcc32dbgpthr -lglobus_gass_transfer_gcc32dbgpthr -lglobus_io_gcc32dbgpthr -lglobus_xio_gcc32dbgpthr -lgssapi_error_gcc32dbgpthr -lglobus_gss_assist_gcc32dbgpthr -lglobus_gssapi_gsi_gcc32dbgpthr -lglobus_gsi_proxy_core_gcc32dbgpthr -lglobus_gsi_credential_gcc32dbgpthr -lglobus_gsi_callback_gcc32dbgpthr -lglobus_oldgaa_gcc32dbgpthr -lglobus_gsi_sysconfig_gcc32dbgpthr -lglobus_gsi_cert_utils_gcc32dbgpthr -lglobus_openssl_gcc32dbgpthr -lglobus_rsl_gcc32dbgpthr -lglobus_openssl_error_gcc32dbgpthr -lglobus_callout_gcc32dbgpthr -lglobus_proxy_ssl_gcc32dbgpthr -lglobus_common_gcc32dbgpthr -lssl_gcc32dbgpthr -lcrypto_gcc32dbgpthr -lltdl_gcc32dbgpthr -lm

# End.

