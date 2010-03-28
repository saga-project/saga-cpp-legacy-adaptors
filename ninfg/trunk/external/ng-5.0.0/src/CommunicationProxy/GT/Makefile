# $RCSfile: Makefile.in,v $ $Revision: 1.2 $ $Date: 2008/02/25 05:21:45 $
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

all: server client_target

server:
	cd remote && $(MAKE) server
	cd relay  && $(MAKE) server

client: client_target

client_target:
	cd client && $(MAKE) client
	cd relay  && $(MAKE) client

install: install_client install_server

install_server:
	cd remote && $(MAKE) install_server
	cd relay  && $(MAKE) install_server

install_client:
	cd client && $(MAKE) install_client
	cd relay  && $(MAKE) install_client

clean:
	cd remote && $(MAKE) clean
	cd client && $(MAKE) clean
	cd common && $(MAKE) clean
	cd relay  && $(MAKE) clean

distclean:
	cd remote && $(MAKE) distclean
	cd client && $(MAKE) distclean
	cd common && $(MAKE) distclean
	cd relay  && $(MAKE) distclean
	$(RM) Makefile

