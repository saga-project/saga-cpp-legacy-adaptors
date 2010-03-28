# $RCSfile: defvar.mk.in,v $ $Revision: 1.6 $ $Date: 2003/12/15 05:10:00 $
# $AIST_Release: 4.2.4 $
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
# ---------------------------------------------------------
# System macro: DON'T CHANGE!!! JUST USE IT.

TOPDIR		=	/Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4

prefix		=	$(DESTDIR)/Users/merzky/links/saga/svn/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/install

RM		=	$(TOPDIR)/utility/script/del.del.del
AR		=	ar cr
RANLIB		=	ranlib

INSTALL		=	$(TOPDIR)/utility/script/install-sh -c
MKDIR		=	$(TOPDIR)/utility/script/mkdirhier

YACC		=	/usr/bin/yacc

GCCINC		=	-I`case "$(CC)" in *gcc) echo \`$(CC) -v 2>&1 | grep specs | sed 's/specs$$/include/' | awk '{ print $$NF }'\`;; esac`

CC		=	cc
CPP		=	cc -E

# Top include
NG_INCDIR	=	-I$(TOPDIR)/c/include

# NS DEFS
NG_DEFS		=	-DCOMPILER='"$(CC)"' -DCPP_FILE='"cc -xc -E"'

# XML include
XML_INCDIR	=	-I$(TOPDIR)/c/expat/xmlparse

# Globus include
GLOBUS_INCDIR	=	 -I/usr/local/globus//include -I/usr/local/globus//include/gcc32dbgpthr -I/usr/local/globus//include/gcc32dbgpthr/wsrf/services

# All package include
PKG_INCDIR	=	$(XML_INCDIR) $(GLOBUS_INCDIR)

# Overall cpp flags
CONF_CPPFLAGS	=	 -DNGI_SIGWAIT_HAS_BUG=1 -DHAVE_ALLOCA_H=1 -DHAVE_ALLOCA=1 -DHAVE_STDARG_H=1 -DHAVE_LOCALE_H=1 -DHAVE_SETLOCALE=1 -DNO_IEEEFP_H=1 -DHAVE_STRDUP=1 -DHAVE_SYS_TIME_H=1 -DTIME_WITH_SYS_TIME=1 -DSTDC_HEADERS=1 -DRETSIGTYPE=void -DHAS_LONGLONG=1 -DHAS_LONGDOUBLE=1 -DSIZEOF_UNSIGNED_SHORT=2 -DSIZEOF_UNSIGNED_INT=4 -DSIZEOF_UNSIGNED_LONG=4 -DSIZEOF_UNSIGNED_LONG_LONG=8 -DSIZEOF_FLOAT=4 -DSIZEOF_DOUBLE=8 -DSIZEOF_LONG_DOUBLE=16 -DSIZEOF_VOID_P=4 -DCHAR_ALIGN=1 -DSHORT_ALIGN=2 -DINT_ALIGN=4 -DLONG_ALIGN=4 -DLONGLONG_ALIGN=4 -DFLOAT_ALIGN=4 -DDOUBLE_ALIGN=4 -DLONGDOUBLE_ALIGN=16 -DHAS_INT16=1 -DHAS_INT32=1 -DHAS_INT64=1  $(NG_DEFS)  -DNG_CPU_ -DNG_OS_ -DPLAT_SCO -DNGI_ARCHITECTURE_ID=12   -D_REENTRANT   -DNG_PTHREAD -DNGI_NO_MDS2_MODULE -DNGI_NO_MDS4_MODULE  -D__USE_FIXED_PROTOTYPES__ $(NG_INCDIR) $(PKG_INCDIR)

CONF_CFLAGS	=	 -Wall -O2 
CONF_LDFLAGS	=	


# Math library
MATH_LIBS	=	-lm

# Socket library
SOCK_LIBS	=	

# XDR library
XDR_LIBS	=	

# Libraries for network transport with XDR
TRANS_LIBS	=	$(SOCK_LIBS) $(XDR_LIBS)

# XML library
XML_LIBS	=	-L$(TOPDIR)/c/expat -lexpat

# Globus libraries dir
GLOBUS_LIBDIR	=	 -L/usr/local/globus//lib

# All package libraries
PKG_LIBS	=	$(XML_LIBS)



# ---------------------------------------------------------
# Default value of user editable macro

# You should/can specify EXEC_TARGET and INSTALL_BINDIR for executable
# install.

# Executable target
EXEC_TARGET	=
# Where EXEC_TARGET will be stored.
INSTALL_BINDIR	=	$(prefix)/bin


# You should/can specify LIB_TARGET and INSTALL_LIBDIR for library
# install.

# Library target
LIB_TARGET	=
# Where LIB_TARGET will be stored.
INSTALL_LIBDIR	=	$(prefix)/lib


# You should/can specify INC_TARGET and INSTALL_INCDIR for include
# file install.

# Include file target
INC_TARGET	=
# Where INC_TERGET will be stored.
INSTALL_INCDIR	=	$(prefix)/include


# You should/can specify DOC_TARGET and INSTALL_DOCDIR for
# Document/template file install.

# Document/template file target
DOC_TARGET	=
# Where INC_TERGET will be stored.
INSTALL_DOCDIR	=	$(prefix)/doc


# You should/can specify ETC_TARGET and INSTALL_ETCDIR for
# Configuration file install.

# Configuration file target
ETC_TARGET	=
# Where INC_TERGET will be stored.
INSTALL_ETCDIR	=	$(prefix)/etc


# C compiler flags
CFLAGS		=	$(CONF_CFLAGS)

# C preprosessor flags
CPPFLAGS	=	$(CONF_CPPFLAGS)

# Linker flags
LDFLAGS		=	$(CONF_LDFLAGS)

# makedepend flags
DEPENDFLAGS	=	$(CPPFLAGS)

