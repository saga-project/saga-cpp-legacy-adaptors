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

INFORMATION_SERVICE_MODULES = ccommon  nrf
INFORMATION_SERVICE_MODULES_ALL = ccommon nrf
MAKEEACH_SH = /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-5.0.0/utility/compile/makeeach.sh

all: client

server:
	@true

client:
	$(MAKEEACH_SH) $(MAKE) client $(INFORMATION_SERVICE_MODULES)

install: install_client

install_server:
	@true

install_client:
	$(MAKEEACH_SH) $(MAKE) install_client $(INFORMATION_SERVICE_MODULES)

clean:
	$(MAKEEACH_SH) $(MAKE) clean $(INFORMATION_SERVICE_MODULES_ALL)

distclean:
	$(MAKEEACH_SH) $(MAKE) distclean $(INFORMATION_SERVICE_MODULES_ALL)
	$(RM) Makefile

