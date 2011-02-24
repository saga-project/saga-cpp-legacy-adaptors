#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)


ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

-include ../../../../config/make.cfg
include $(SAGA_LOCATION)/share/saga/make/saga.util.mk


ifeq "$(SAGA_HAVE_MYSQL)" "yes"

  SAGA_ARC        = soci_mysql
  SAGA_SRC        = $(wildcard *.cpp)
  SAGA_LIB_OBJ    = $(SAGA_SRC:%.cpp=%.o)

  SAGA_DONT_INSTALL = 1

endif

include $(SAGA_LOCATION)/share/saga/make/saga.mk
-include ../../../../config/saga.config.mysql.c.mk

SAGA_CPPINCS    += -I../../core/
SAGA_LDFLAGS    += -L../../core/ -lsoci_core

distclean::
	@$(RM) mysql-config.hpp

