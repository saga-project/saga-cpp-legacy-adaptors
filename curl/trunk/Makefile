#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include config/make.cfg

SAGA_SUBDIRS    = config test

ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
  SAGA_SUBDIRS += curl_context
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_FILE)" "xyes"
  SAGA_SUBDIRS += curl_file
endif

all:: config/make.cfg

config/make.cfg: 
	@echo ""
	@echo " ================================= "
	@echo "  you need to run configure first  "
	@echo " ================================= "
	@echo ""
	@false


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

