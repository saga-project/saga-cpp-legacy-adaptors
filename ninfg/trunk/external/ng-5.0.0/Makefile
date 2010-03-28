# $RCSfile: Makefile.in,v $ $Revision: 1.14 $ $Date: 2008/01/18 07:26:14 $
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

all:	makeAll

DIRS =	./src/c/include \
	./src/c/client \
	./src/c/executable \
	./src/c/net \
	./src/c/common \
	./src/c/grpc \
	./src/lib/utility \
	./src/lib/ExternalModule \
	./src/lib/expat \
	./src/lib/queue \
	./src/InvokeServer \
	./src/CommunicationProxy \
	./src/InformationService \
	./utility/template \
	./utility/generator \
	./utility/hostname \
	./utility/version \
	./utility/script \
	./doc

DISTCLEAN_TARGET = \
	./utility/compile/defvar.mk \
	./utility/template/globus_makefile_header-result.txt

makeAll:
	for i in $(DIRS); do \
		(cd $$i; $(MAKE)) || exit 1; \
	done;

server:
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) server) || exit 1; \
	done;

client:
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) client) || exit 1; \
	done;

forCores:
	$(RM) .libs
	find . -name '*.a' -a -type f -print > .libs
	(cd lib; \
		$(RM) *.a ; \
		for i in `cat ../.libs` ; do \
			ln -s ../$$i . ; \
		done; )
	$(RM) .libs

install::
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) install) || exit 1; \
	done;

install_server::
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) install_server) || exit 1; \
	done;

install_client::
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) install_client) || exit 1; \
	done;

clean::
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) clean); \
	done;
	$(RM) core *.core *~

distclean::
	for i in $(DIRS); do \
		(cd $$i; $(MAKE) distclean); \
	done;
	$(RM) Makefile config.* .depend* $(DISTCLEAN_TARGET)

Makefile::
	sh ./config.status

