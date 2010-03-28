# $RCSfile: Makefile.in,v $ $Revision: 1.2 $ $Date: 2008/02/27 09:56:32 $
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

TARGET=ng_invoke_server.SSH

HDRS=	ngInvokeServer.h  ngInvokeServerSSH.h  ngisList.h  \
	ngisLog.h ngisUtility.h

SRCS=ngInvokeServer.c ngInvokeServerSSH.c ngisCallbackManager.c \
	ngisJobAttribute.c ngisLineBuffer.c ngisList.c ngisLog.c \
	ngisOptions.c ngisProtocol.c ngisSSHPBS.c ngisSSHSGE.c \
	ngisSSHattributes.c ngisSSHcreateCommand.c ngisSSHfileTransfer.c \
	ngisSSHprepare.c ngisSSHscript.c ngisUtility.c

OBJS=${SRCS:.c=.o}

EXEC_CLIENT_TARGET = ${TARGET}

all: ${TARGET}

all: ${TARGET}

server:

client: all

${TARGET}: ${OBJS}
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} ${LIBS} ${OBJS}

clean::
	${RM} ${OBJS} ${TARGET}

distclean::

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/defrules.mk

ngInvokeServer.o: ngInvokeServer.c $(HDRS)
ngInvokeServerSSH.o: ngInvokeServerSSH.c $(HDRS)
ngisCallbackManager.o: ngisCallbackManager.c $(HDRS)
ngisJobAttribute.o: ngisJobAttribute.c $(HDRS)
ngisLineBuffer.o: ngisLineBuffer.c $(HDRS)
ngisList.o: ngisList.c $(HDRS)
ngisLog.o: ngisLog.c $(HDRS)
ngisProtocol.o: ngisProtocol.c $(HDRS)
ngisSSHPBS.o: ngisSSHPBS.c $(HDRS)
ngisSSHSGE.o: ngisSSHSGE.c  $(HDRS)
ngisSSHattributes.o: ngisSSHattributes.c $(HDRS)
ngisSSHcreateCommand.o: ngisSSHcreateCommand.c  $(HDRS)
ngisSSHfileTransfer.o: ngisSSHfileTransfer.c $(HDRS)
ngisSSHprepare.o: ngisSSHprepare.c $(HDRS)
ngisSSHscript.o: ngisSSHscript.c $(HDRS)
ngisUtility.o: ngisUtility.c $(HDRS)

