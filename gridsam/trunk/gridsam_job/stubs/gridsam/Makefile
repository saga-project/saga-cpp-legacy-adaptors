#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

-include ../../../config/make.cfg
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

SAGA_CPPINCS     +=  -I../gsoap-2.7.11
SAGA_CPPDEFS     += -DPTHREADS -DWITH_NONAMESPACES -DWITH_DOM -DWITH_IPV6

ifeq "$(SAGA_HAVE_OPENSSL)" "yes"
  SAGA_CPPDEFS   += -DWITH_OPENSSL -DWITH_COOKIES
endif


# distclean::
#	@$(ECHO) "      cleaning stubs"
#	@$(RM) -f *.*

