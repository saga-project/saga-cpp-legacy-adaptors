# $RCSfile: Makefile,v $ $Revision: 1.3 $ $Date: 2003/11/05 09:42:31 $
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

SHELL  = /bin/sh

CLIENT = pi_client
MODULE = pi

NG_CC  = $(NG_DIR)/bin/ng_cc
NG_GEN = $(NG_DIR)/bin/ng_gen

all: $(CLIENT) stub

install:
	make -f $(MODULE).mak install

$(CLIENT): $(CLIENT).c
	$(NG_CC) -g -o $@ $(CLIENT).c

stub: $(MODULE).idl
	$(NG_GEN) $(MODULE).idl
	make -f $(MODULE).mak

clean:
	if test -f ./$(MODULE).mak; then \
		make -f $(MODULE).mak veryclean; \
	fi
	rm -f  $(MODULE).mak $(CLIENT) $(MODULE):*
	rm -f *~ *.o core core.*
