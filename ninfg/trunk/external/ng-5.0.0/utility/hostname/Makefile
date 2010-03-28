# $RCSfile: Makefile.in,v $ $Revision: 1.4 $ $Date: 2008/01/11 03:22:14 $
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

HDRS =  $(TOPDIR)/src/lib/utility/ngUtility.h

SRCS =  ngHostname.c

OBJS =  $(SRCS:.c=.o)

TARGET	=	ng_hostname
EXEC_TARGET	=	$(TARGET)


NG_UTILITY_LIB_DIR = $(TOPDIR)/src/lib/utility
NG_UTILITY_LIB = $(NG_UTILITY_LIB_DIR)/libngutility.a

all: $(TARGET)

server: all

client: all

$(TARGET): $(OBJS) $(NG_UTILITY_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) \
	    -L$(NG_UTILITY_LIB_DIR) \
	    -o $@ $(OBJS) \
	    -lngutility $(LIBS)

$(NG_UTILITY_LIB):
	cd $(NG_UTILITY_LIB_DIR) && make all

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngHostname.o: ngHostname.c

