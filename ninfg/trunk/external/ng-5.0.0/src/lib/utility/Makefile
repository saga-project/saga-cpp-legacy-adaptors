# $RCSfile: Makefile.in,v $ $Revision: 1.13 $ $Date: 2008/01/11 03:22:14 $
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

HDRS =	$(TOPDIR)/src/lib/utility/ngUtility.h

SRCS =	ngLog.c \
	ngUtility.c \
	ngThread.c \
	ngSelector.c \
	ngEvent.c \
	ngExternalModule.c \
	ngSignalManager.c \
	ngVersion.c \
	ngXMLparser.c

OBJS =	$(SRCS:.c=.o)

TARGET		=	libngutility.a
LIB_TARGET	=	$(TARGET)
INC_TARGET	=	ngUtility.h

CONFIGURE_RESULT_HDR	=	ngConfigureResult.h
CONFIGURE_RESULT_MK	=	ngConfigureValue.mk
CONFIGURE_LOG		=	$(TOPDIR)/config.log

CLEAN_TARGET		=	$(CONFIGURE_RESULT_HDR)
DISTCLEAN_TARGET	=	$(CONFIGURE_RESULT_MK)

all: $(TARGET)

server: all

client: all

$(TARGET): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

$(CONFIGURE_RESULT_HDR): $(CONFIGURE_LOG) $(CONFIGURE_RESULT_MK)
	$(RM) $(CONFIGURE_RESULT_HDR)
	./ngConfigureGet.sh $(CONFIGURE_LOG) $(CONFIGURE_RESULT_MK) \
		> $(CONFIGURE_RESULT_HDR)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngLog.o: ngLog.c $(HDRS)
ngUtility.o: ngUtility.c $(HDRS)
ngThread.o: ngThread.c $(HDRS)
ngEvent.o: ngEvent.c $(HDRS)
ngSelector.o: ngSelector.c $(HDRS)
ngExternalModule.o: ngExternalModule.c $(HDRS)
ngSignalManager.o: ngSignalManager.c $(HDRS)
ngVersion.o: ngVersion.c $(HDRS) $(CONFIGURE_RESULT_HDR)
ngXMLelement.o: ngXMLelement.c $(HDRS) $(TOPDIR)/src/lib/utility/ngXML.h
ngXMLparser.o: ngXMLparser.c $(HDRS) $(TOPDIR)/src/lib/utility/ngXML.h

