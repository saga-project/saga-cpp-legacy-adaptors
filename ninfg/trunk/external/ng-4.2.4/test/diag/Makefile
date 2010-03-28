# $RCSfile: Makefile,v $ $Revision: 1.16 $ $Date: 2005/11/07 08:02:22 $
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

TEST_PROGRAMS = data_test async_test file_sub file_sub_array \
                callback_test nullArgument_test skip_test zeroElement_test \
		cancel_test

CFLAGS = -g
RM = /bin/rm -f
NG_GEN = $(NG_DIR)/bin/ng_gen
NG_CC =  $(NG_DIR)/bin/ng_cc

IDL_FILES = async.idl callback.idl cancel.idl data.idl file.idl \
	    nullArgument.idl skip_test.idl zero_element.idl
MAKE_FILES = $(IDL_FILES:.idl=.mak)

all: $(MAKE_FILES) $(TEST_PROGRAMS) 

install: $(MAKE_FILES)
	for i in $(MAKE_FILES); \
	do \
	    make -f $$i install; \
	done

clean: 
	$(RM) $(TEST_PROGRAMS) *.o tmp*.config tmp*.log
	$(RM) *.ldif

veryclean: clean
	for i in $(MAKE_FILES); \
	do \
	    if [ -f $$i ]; \
	    then \
		make -f $$i veryclean; \
	    fi \
	done
	$(RM) _stub_*.c _inf_*
	$(RM) $(MAKE_FILES)

############## Makefiles ##############

async.mak: async.idl
	$(NG_GEN) async.idl
	make -f $@

callback.mak: callback.idl
	$(NG_GEN) callback.idl
	make -f $@

cancel.mak: cancel.idl
	$(NG_GEN) cancel.idl
	make -f $@

data.mak: data.idl
	$(NG_GEN) data.idl
	make -f $@

file.mak: file.idl
	$(NG_GEN) file.idl
	make -f $@

nullArgument.mak: nullArgument.idl
	$(NG_GEN) nullArgument.idl
	make -f $@

skip_test.mak: skip_test.idl
	$(NG_GEN) skip_test.idl
	make -f $@

zero_element.mak: zero_element.idl
	$(NG_GEN) zero_element.idl
	make -f $@

############## test programs ##############
async_test: async_test.c
	$(NG_CC) $(CFLAGS) -o $@ async_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

data_test: data_test.c
	$(NG_CC) $(CFLAGS) -o $@ data_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

callback_test: callback_test.c
	$(NG_CC) $(CFLAGS) -o $@ callback_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

cancel_test: cancel_test.c
	$(NG_CC) $(CFLAGS) -o $@ cancel_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

file_sub : file_sub.c
	$(NG_CC) $(CFLAGS) -o $@ file_sub.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

file_sub_array : file_sub_array.c
	$(NG_CC) $(CFLAGS) -o $@ file_sub_array.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

nullArgument_test: nullArgument_test.c
	$(NG_CC) $(CFLAGS) -o $@ nullArgument_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

skip_test: skip_test.c
	$(NG_CC) $(CFLAGS) -o $@ skip_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

zeroElement_test: zeroElement_test.c
	$(NG_CC) $(CFLAGS) -o $@ zeroElement_test.c \
	         $(LDFLAGS) $(NINF_LDFLAGS) -lm

