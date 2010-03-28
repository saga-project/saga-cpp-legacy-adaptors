# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.9 $ $Date: 2004/05/26 02:01:32 $
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
include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defvar.mk

CLEAN_TARGET	=	lib include libexpat.a tmp

SRCS	=	xmltok/xmltok.c xmltok/xmlrole.c \
		xmlparse/xmlparse.c \
		xmlwf/xmlfile.c xmlwf/codepage.c \
		xmlwf/unixfilemap.c

OBJS	=	xmltok/xmltok.o xmltok/xmlrole.o \
		xmlparse/xmlparse.o \
		xmlwf/xmlfile.o xmlwf/codepage.o \
		xmlwf/unixfilemap.o

MY_CPPFLAGS	=	-I./xmltok -I./xmlparse -I. -DXML_NS -DXML_DTD
CPPFLAGS	=	$(MY_CPPFLAGS) $(CONF_CPPFLAGS)

LIB_TARGET	=	libexpat.a

all:	libexpat.a

server: all

client: all

libexpat.a:	$(OBJS)
	$(RM) -f $@
	$(AR) $@ $(OBJS)

install_server:: install

install_client:: install

#install::

clean::
	find . -name '*.o' -print | xargs $(RM)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk

