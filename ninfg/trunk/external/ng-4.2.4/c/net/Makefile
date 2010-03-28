# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.4 $ $Date: 2003/11/06 06:25:25 $
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

HDRS =	$(TOPDIR)/c/include/net.h \
	$(TOPDIR)/c/include/ng.h \
	$(TOPDIR)/c/include/ngClientInternal.h \
	$(TOPDIR)/c/include/ngCommon.h \
	$(TOPDIR)/c/include/ngConfig.h \
	$(TOPDIR)/c/include/ngConfigFile.h \
	$(TOPDIR)/c/include/ngEx.h \
	$(TOPDIR)/c/include/ngExecutableInternal.h \
	$(TOPDIR)/c/include/ngFunctionInformation.h \
	$(TOPDIR)/c/include/ngInternal.h \
	$(TOPDIR)/c/include/ngPlatform.h \
	$(TOPDIR)/c/include/ngPlatform.h \
	$(TOPDIR)/c/include/ngXML.h

SRCS =	communicator.c

OBJS =	communicator.o

CPPFLAGS	=	$(CONF_CPPFLAGS) $(GLOBUS_INCDIR)
TARGET		=	libngnet.a
LIB_TARGET	=	$(TARGET)

all: $(TARGET)

server: all

client: all

$(TARGET): $(OBJS)
	$(RM) -f $@
	$(AR) $@ $(OBJS)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk

communicator.o: communicator.c $(HDRS)
