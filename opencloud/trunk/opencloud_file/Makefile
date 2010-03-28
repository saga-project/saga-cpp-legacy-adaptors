#  Copyright (c) 2005-2009 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. 
#  (See accompanying file LICENSE or copy at 
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg

SAGA_SRC              = $(wildcard *.cpp)
SAGA_OBJ              = $(SAGA_SRC:%.cpp=%.o)
                    
SAGA_ADAPTOR_TYPE     = file
SAGA_ADAPTOR_NAME     = opencloud_file
SAGA_ADAPTOR_INI      = opencloud_file_adaptor.ini
SAGA_ADAPTOR_OBJ      = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = namespace

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk

SAGA_CXXFLAGS += $(SAGA_SECSPH_CXXFLAGS)
SAGA_LDFLAGS  += $(SAGA_SECSPH_LDFLAGS)

