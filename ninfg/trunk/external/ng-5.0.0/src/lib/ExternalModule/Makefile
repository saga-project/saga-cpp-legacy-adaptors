# $RCSfile: Makefile.in,v $ $Revision: 1.1 $ $Date: 2007/05/11 08:27:59 $
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

HDRS =	ngemCallbackManager.h ngemEnvironment.h ngemList.h ngemLog.h \
	ngemProtocol.h ngemType.h ngemUtility.h
SRCS =	ngemCallbackManager.c ngemLineBuffer.c ngemList.c ngemLog.c \
	ngemOption.c ngemProtocol.c ngemUtility.c
OBJS =	$(SRCS:.c=.o)

TARGET		=	libngemcommon.a
LIB_TARGET	=	$(TARGET)

all: $(TARGET)

server: all

client: all

$(TARGET): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngemCallbackManager.o: ngemCallbackManager.c ${HDRS}
ngemLineBuffer.o: ngemLineBuffer.c ${HDRS}
ngemList.o: ngemList.c ${HDRS}
ngemLog.o: ngemLog.c ${HDRS}
ngemOption.o: ngemOption.c ${HDRS}
ngemProtocol.o: ngemProtocol.c ${HDRS}
ngemUtility.o: ngemUtility.c ${HDRS}
