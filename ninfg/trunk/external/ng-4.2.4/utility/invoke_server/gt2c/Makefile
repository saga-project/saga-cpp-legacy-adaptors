# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.3 $ $Date: 2006/08/21 02:19:05 $
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

HDRS =  ngInvokeServer.h

SRCS =	ngInvokeServerGT2c.c \
	ngisContext.c \
	ngisRequest.c \
	ngisJob.c \
	ngisRefreshCredential.c \
	ngisUtility.c

OBJS =	ngInvokeServerGT2c.o \
	ngisContext.o \
	ngisRequest.o \
	ngisJob.o \
	ngisRefreshCredential.o \
	ngisUtility.o

CPPFLAGS	=	$(CONF_CPPFLAGS) $(GLOBUS_INCDIR)
TARGET		=	ng_invoke_server.GT2c

COMPILE_TOOL	=	./ngCompile.sh

EXEC_CLIENT_TARGET = $(TARGET)

all: $(TARGET)

server:

client: all

$(TARGET): $(OBJS)
	$(COMPILE_TOOL) $(TOPDIR) "$(CC)" "$(OBJS)" -o $@

clean::

distclean::

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk

ngInvokeServerGT2c.o: ngInvokeServerGT2c.c $(HDRS)
ngisContext.o: ngisContext.c $(HDRS)
ngisJob.o: ngisJob.c $(HDRS)
ngisRequest.o: ngisRequest.c $(HDRS)
ngisUtility.o: ngisUtility.c $(HDRS)

