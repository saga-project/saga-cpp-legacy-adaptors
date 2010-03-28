# $RCSfile: Makefile.in,v $ $Revision: 1.5 $ $Date: 2008/02/25 05:21:46 $
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

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defvar.mk
include ${TOPDIR}/src/lib/ExternalModule/defvar.mk

SRCS = ngcpCallback.c ngcpGSISSH.c ngcpOptions.c ngcpRelayHandler.c ngcpUtility.c ngcpXIO.c
OBJS = $(SRCS:.c=.o)

CC       = ${GLOBUS_CLIENT_CC}
CFLAGS   += ${GLOBUS_CLIENT_CFLAGS}
CPPFLAGS += ${GLOBUS_CLIENT_CPPFLAGS} ${GLOBUS_CLIENT_INCLUDES}
INCLUDES += 
LIBS     += ${GLOBUS_CLIENT_LIBS}
LDFLAGS  += ${GLOBUS_CLIENT_LDFLAGS} ${GLOBUS_CLIENT_PKG_LIBS}

TARGET	= ngcpGTcommon.a

all: $(TARGET)

server: all

client: all

$(TARGET): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

include ${TOPDIR}/src/lib/ExternalModule/defrules.mk
include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngcpCallback.o: ngcpCallback.c ngcpCallback.h
ngcpGSISSH.o: ngcpGSISSH.c ngcpOptions.h ngrGT.h ngcpXIO.h ngcpRelayHandler.h ngcpUtility.h
ngcpOptions.o: ngcpOptions.c ngcpXIO.h ngcpOptions.h ngcpUtility.h
ngcpRelayHandler.o: ngcpRelayHandler.c ngcpXIO.h ngcpOptions.h ngrGT.h ngcpRelayHandler.h ngcpUtility.h
ngcpUtility.o: ngcpUtility.c ngcpXIO.h ngcpUtility.h
ngcpXIO.o: ngcpXIO.c ngcpXIO.h ngcpUtility.h
