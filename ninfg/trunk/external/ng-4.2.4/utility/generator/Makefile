# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.8 $ $Date: 2005/03/11 03:09:18 $
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

SRCS =	nggenIdl.c \
	nggenCompile.c \
	nggenMain.c \
	nggenMem.c \
	nggenMisc.c \
	nggenStub.c

OBJS =	nggenIdl.o \
	nggenCompile.o \
	nggenMain.o \
	nggenMem.o \
	nggenMisc.o \
	nggenStub.o

YACC_TARGET = nggenIdl.c
YACC_SOURCE = nggenIdl.y

CLEAN_TARGET	=	$(YACC_TARGET) y.tab.* y.out*
TARGET		=	ng_gen
EXEC_SERVER_TARGET	=	$(TARGET)

all: $(TARGET)

server: all

client:

$(YACC_TARGET): $(YACC_SOURCE) nggenIdlLex.c
	$(YACC) -d $(YACC_SOURCE)
	mv y.tab.c $(YACC_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk
