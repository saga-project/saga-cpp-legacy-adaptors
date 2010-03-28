# $RCSfile: Makefile.in,v $ $Revision: 1.6 $ $Date: 2007/11/27 02:27:42 $
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

DISTCLEAN_TARGET	=	./ngConfig.h

all:
	@true

server: all
client: all

install_server:: install

install_client:: install


HDRS =	grpc.h \
	grpcError.h \
	grpcLocal.h \
	grpc_executable.h \
	grpc_error.h \
	net.h \
	ng.h \
	ngClientInternal.h \
	ngCommon.h \
	ngConfig.h \
	ngConfigFile.h \
	ngEx.h \
	ngExecutableInternal.h \
	ngFunctionInformation.h \
	ngInternal.h \
	ngEnvironment.h \
	ngConfig.h

INC_TARGET = $(HDRS)

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

