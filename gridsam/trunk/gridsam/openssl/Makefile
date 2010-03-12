#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

SAGA_CSRC         = $(wildcard *.c)
SAGA_OBJC         = $(SAGA_CSRC:%.c=%.o)

-include ../../config/make.cfg

include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CPPDEFS += -DPTHREADS -DWITH_NONAMESPACES 

