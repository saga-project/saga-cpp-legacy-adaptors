# $RCSfile: Makefile.in,v $ $Revision: 1.7 $ $Date: 2008/02/25 05:21:46 $
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
include ${TOPDIR}/src/CommunicationProxy/ccommon/remote/defvar.mk

SRCS =	ngrcGT.c ngrcOperator.c ngrcRelayHandler.c ngrcSocket.c
OBJS =	$(SRCS:.c=.o)

COMM_LIB	=	../common/ngcpGTcommon.a

CC       = ${GLOBUS_SERVER_CC}
CFLAGS   += ${GLOBUS_SERVER_CFLAGS}
CPPFLAGS += ${GLOBUS_SERVER_CPPFLAGS} ${GLOBUS_SERVER_INCLUDES} -I../common
LIBS     += ${GLOBUS_SERVER_LIBS} ${COMM_LIB}
LDFLAGS  += ${GLOBUS_SERVER_LDFLAGS} ${GLOBUS_EMBED_LD_RUN_PATH} ${GLOBUS_SERVER_PKG_LIBS}

TARGET		=	ng_remote_communication_proxy.GT
EXEC_SERVER_TARGET	=	${TARGET}

all: $(TARGET)

server: all

client:

$(TARGET): $(OBJS) $(COMM_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

$(COMM_LIB):
	cd ../common && ${MAKE}

include ${TOPDIR}/src/CommunicationProxy/ccommon/remote/defrules.mk
include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngrcGT.o: ngrcGT.c ngrcGT.h
ngrcOperator.o: ngrcOperator.c ngrcGT.h
ngrcRelayHandler.o: ngrcRelayHandler.c ngrcGT.h
ngrcSocket.o: ngrcSocket.c ngrcGT.h
