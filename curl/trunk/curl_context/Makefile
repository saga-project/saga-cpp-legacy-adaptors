#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)


SAGA_ADAPTOR_NAME = curl_context
SAGA_ADAPTOR_INI  = curl_context_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = 

-include ../config/make.cfg
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

