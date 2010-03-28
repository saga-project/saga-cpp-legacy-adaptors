# $RCSfile: Makefile.in,v $ $Revision: 1.15 $ $Date: 2008/02/18 09:51:50 $
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

HDRS =	$(TOPDIR)/src/c/include/ng.h \
	$(TOPDIR)/src/c/include/ngClientInternal.h \
	$(TOPDIR)/src/c/include/ngConfigFile.h \
	$(TOPDIR)/src/c/include/ngFunctionInformation.h \
	$(TOPDIR)/src/c/include/ngInternal.h \
	$(TOPDIR)/src/lib/utility/ngUtility.h \
	$(TOPDIR)/src/c/include/ngCommon.h \
	$(TOPDIR)/src/c/include/ngEnvironment.h \
	$(TOPDIR)/src/c/include/ngConfig.h

SRCS =	ngclArgumentStack.c \
	ngclConfigFileRead.c \
	ngclContext.c \
	ngclDefaultRemoteMachine.c \
	ngclExecutable.c \
	ngclExecutablePath.c \
	ngclJob.c \
	ngclInvokeServer.c \
	ngclCommunicationProxy.c \
	ngclInformationService.c \
	ngclLocalMachine.c \
	ngclLog.c \
	ngclNinfgManager.c \
	ngclProtocol.c \
	ngclRemoteClass.c \
	ngclRemoteClassGenerate.c \
	ngclRemoteMachine.c \
	ngclRemoteMethod.c \
	ngclHeartBeat.c \
	ngclSessionTimeout.c \
	ngclTransferTimeout.c \
	ngclJobStartTimeout.c \
	ngclSession.c \
	ngclUtility.c

OBJS = $(SRCS:.c=.o)

TARGET		=	libngclient.a
LIB_CLIENT_TARGET	=	$(TARGET)
CLEAN_TARGET = ngclSessionInvokeCallback.c

all: $(TARGET)

server:

client: all

$(TARGET): $(OBJS)
	$(RM) $@
	$(AR) $@ $(OBJS)

ngclSessionInvokeCallback.c: ngclSessionInvokeCallback.sh
	./ngclSessionInvokeCallback.sh > $@  || $(RM) $@

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngclArgumentStack.o: ngclArgumentStack.c $(HDRS)
ngclConfigFileLdif.o: ngclConfigFileLdif.c $(HDRS)
ngclConfigFileRead.o: ngclConfigFileRead.c $(HDRS)
ngclContext.o: ngclContext.c $(HDRS)
ngclDefaultRemoteMachine.o: ngclDefaultRemoteMachine.c $(HDRS)
ngclExecutable.o: ngclExecutable.c $(HDRS)
ngclExecutablePath.o: ngclExecutablePath.c $(HDRS)
ngclJob.o: ngclJob.c $(HDRS)
ngclInvokeServer.o: ngclInvokeServer.c $(HDRS)
ngclCommunicationProxy.o: ngclCommunicationProxy.c $(HDRS)
ngclInformationService.o: ngclInformationService.c $(HDRS)
ngclLocalMachine.o: ngclLocalMachine.c $(HDRS)
ngclLog.o: ngclLog.c $(HDRS)
ngclNinfgManager.o: ngclNinfgManager.c $(HDRS)
ngclProtocol.o: ngclProtocol.c $(HDRS)
ngclRemoteClass.o: ngclRemoteClass.c $(HDRS)
ngclRemoteClassGenerate.o: ngclRemoteClassGenerate.c $(HDRS)
ngclRemoteMachine.o: ngclRemoteMachine.c $(HDRS)
ngclRemoteMethod.o: ngclRemoteMethod.c $(HDRS)
ngclHeartBeat.o: ngclHeartBeat.c $(HDRS)
ngclSessionTimeout.o: ngclSessionTimeout.c $(HDRS)
ngclTransferTimeout.o: ngclTransferTimeout.c $(HDRS)
ngclJobStartTimeout.o: ngclJobStartTimeout.c $(HDRS)
ngclSession.o: ngclSession.c ngclSessionInvokeCallback.c $(HDRS)
ngclUtility.o: ngclUtility.c $(HDRS)

