#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. 
#  (See accompanying file LICENSE or copy at 
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = context
SAGA_ADAPTOR_NAME = aws_context
SAGA_ADAPTOR_INI  = aws_context_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk

SAGA_CPPINCS     += -I../aws_suite
SAGA_LDFLAGS     += -L../aws_suite -lsaga_adaptor_aws_suite

distclean::
	@$(RM) aws_context_adaptor.ini

