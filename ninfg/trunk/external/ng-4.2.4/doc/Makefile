# Generated automatically from Makefile.in by configure.
# $RCSfile: Makefile.in,v $ $Revision: 1.2 $ $Date: 2005/08/04 07:19:23 $
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

DOC_TARGET	=
USERS_MANUAL 	= users-manual
TUTORIAL 	= tutorial
DOC_INSTALLS	= $(USERS_MANUAL) $(TUTORIAL)

all:
	@true

server: all

client: all

install_server:: install

install_client:: install

install::
	@if test ! -d $(INSTALL_DOCDIR); then \
	    (echo "Making directory $(INSTALL_DOCDIR)" ; \
	    $(MKDIR) $(INSTALL_DOCDIR) ; \
	    chmod 755 $(INSTALL_DOCDIR)) ; \
	fi
	@for i in / $(DOC_INSTALLS) ; do \
	    case $$i in \
	    /)  true;; \
	    *)  tar cpf - $$i | ( cd $(INSTALL_DOCDIR); tar xpf -); \
	        echo "$$i -> $(INSTALL_DOCDIR)";; \
	    esac; \
	done

include /Users/merzky/links/saga/trunk/adaptors/ninfg/ninfg-adaptor-1112/external/ng-4.2.4/utility/compile/defrules.mk
