# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.15 $ $Date: 2005/09/22 09:52:17 $
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

SRCS =	ngclArgumentStack.c \
	ngclConfigFileLdif.c \
	ngclConfigFileRead.c \
	ngclContext.c \
	ngclDefaultRemoteMachine.c \
	ngclExecutable.c \
	ngclExecutablePath.c \
	ngclGlobus.c \
	ngclJob.c \
	ngclInvokeServer.c \
	ngclLocalMachine.c \
	ngclLog.c \
	ngclMDSaccess.c \
	ngclMDSserver.c \
	ngclNinfgManager.c \
	ngclProtocol.c \
	ngclRemoteClass.c \
	ngclRemoteClassGenerate.c \
	ngclRemoteMachine.c \
	ngclRemoteMethod.c \
	ngclHeartBeat.c \
	ngclSessionTimeout.c \
	ngclRefreshCredentials.c \
	ngclJobStartTimeout.c \
	ngclObserve.c \
	ngclSession.c \
	ngclUtility.c

OBJS =	ngclArgumentStack.o \
	ngclConfigFileLdif.o \
	ngclConfigFileRead.o \
	ngclContext.o \
	ngclDefaultRemoteMachine.o \
	ngclExecutable.o \
	ngclExecutablePath.o \
	ngclGlobus.o \
	ngclJob.o \
	ngclInvokeServer.o \
	ngclLocalMachine.o \
	ngclLog.o \
	ngclMDSaccess.o \
	ngclMDSserver.o \
	ngclNinfgManager.o \
	ngclProtocol.o \
	ngclRemoteClass.o \
	ngclRemoteClassGenerate.o \
	ngclRemoteMachine.o \
	ngclRemoteMethod.o \
	ngclHeartBeat.o \
	ngclSessionTimeout.o \
	ngclRefreshCredentials.o \
	ngclJobStartTimeout.o \
	ngclObserve.o \
	ngclSession.o \
	ngclUtility.o

CPPFLAGS	=	$(CONF_CPPFLAGS) $(GLOBUS_INCDIR)
TARGET		=	libngclient.a
LIB_CLIENT_TARGET	=	$(TARGET)

all: $(TARGET)

server:

client: all

$(TARGET): $(OBJS)
	$(RM) -f $@
	$(AR) $@ $(OBJS)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk

ngclArgumentStack.o: ngclArgumentStack.c $(HDRS)
ngclConfigFileLdif.o: ngclConfigFileLdif.c $(HDRS)
ngclConfigFileRead.o: ngclConfigFileRead.c $(HDRS)
ngclContext.o: ngclContext.c $(HDRS)
ngclDefaultRemoteMachine.o: ngclDefaultRemoteMachine.c $(HDRS)
ngclExecutable.o: ngclExecutable.c $(HDRS)
ngclExecutablePath.o: ngclExecutablePath.c $(HDRS)
ngclGlobus.o: ngclGlobus.c $(HDRS)
ngclJob.o: ngclJob.c $(HDRS)
ngclInvokeServer.o: ngclInvokeServer.c $(HDRS)
ngclLocalMachine.o: ngclLocalMachine.c $(HDRS)
ngclLog.o: ngclLog.c $(HDRS)
ngclMDSaccess.o: ngclMDSaccess.c $(HDRS)
ngclMDSserver.o: ngclMDSserver.c $(HDRS)
ngclNinfgManager.o: ngclNinfgManager.c $(HDRS)
ngclProtocol.o: ngclProtocol.c $(HDRS)
ngclRemoteClass.o: ngclRemoteClass.c $(HDRS)
ngclRemoteClassGenerate.o: ngclRemoteClassGenerate.c $(HDRS)
ngclRemoteMachine.o: ngclRemoteMachine.c $(HDRS)
ngclRemoteMethod.o: ngclRemoteMethod.c $(HDRS)
ngclHeartBeat.o: ngclHeartBeat.c $(HDRS)
ngclSessionTimeout.o: ngclSessionTimeout.c $(HDRS)
ngclJobStartTimeout.o: ngclJobStartTimeout.c $(HDRS)
ngclRefreshCredentials.o: ngclRefreshCredentials.c $(HDRS)
ngclObserve.o: ngclObserve.c $(HDRS)
ngclSession.o: ngclSession.c $(HDRS)
ngclUtility.o: ngclUtility.c $(HDRS)
