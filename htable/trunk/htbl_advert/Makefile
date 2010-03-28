#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. 
#  (See accompanying file LICENSE or copy at 
#  http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = advert
SAGA_ADAPTOR_NAME = htbl_advert
SAGA_ADAPTOR_INI  = htbl_advert_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = namespace advert

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

SAGA_CXXFLAGS += -I$(SAGA_LOCATION)/include/saga/
SAGA_CXXFLAGS += -I/work/mmicel2/hypertable/0.9.0.12/include/
SAGA_CXXFLAGS += -I/work/mmicel2/log4cpp/include/
SAGA_LDFLAGS  += -L/work/mmicel2/log4cpp/lib
SAGA_LDFLAGS  += -llog4cpp
