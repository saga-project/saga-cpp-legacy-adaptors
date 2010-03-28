#  Copyright (c) 2005-2008 Michael Miceli (mmicel2@cct.lsu.edu)
# 
#  Distributed under the Boost Software License, Version 1.0. 
#  (See accompanying file LICENSE or copy at 
#  http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

THRIFT_DIR = /home/mmicel2/hadoop/thrift/include/thrift
LIB_DIR = /home/mmicel2/hadoop/thrift/lib
GEN_SRC = Hbase.cpp \
	       Hbase_types.cpp \
	       Hbase_constants.cpp

SAGA_SRC          = $(wildcard *.cpp) $(GEN_SRC)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = advert
SAGA_ADAPTOR_NAME = hbase_advert
SAGA_ADAPTOR_INI  = hbase_advert_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = namespace advert

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

SAGA_CXXFLAGS += -I$(SAGA_LOCATION)/include/saga/
SAGA_CXXFLAGS += -I${THRIFT_DIR}
SAGA_LDFLAGS  += -L$(LIB_DIR) -lthrift

