#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_NAME = globus_globusonline_file
SAGA_ADAPTOR_INI  = saga_adaptor_globus_globusonline_file.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = filesystem namespace

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

