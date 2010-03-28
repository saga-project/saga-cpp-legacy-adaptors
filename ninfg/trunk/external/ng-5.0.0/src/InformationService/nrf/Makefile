# $RCSfile: Makefile.in,v $ $Revision: 1.3 $ $Date: 2008/01/18 07:26:15 $
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
include ${TOPDIR}/src/InformationService/ccommon/defvar.mk

HDRS =	nginReadNRF.h

SRCS =	nginNRF.c nginReadNRF.c
OBJS =	$(SRCS:.c=.o)
LIBS := ${XML_LIBS} ${LIBS}

TARGET		=	ng_information_service.NRF
EXEC_CLIENT_TARGET	=	${TARGET}

all: $(TARGET)

server:

client: all

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

include ${TOPDIR}/src/InformationService/ccommon/defrules.mk
include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

nginNRF.o:     nginNRF.c     ${HDRS}
nginReadNRF.o: nginReadNRF.c ${HDRS}
