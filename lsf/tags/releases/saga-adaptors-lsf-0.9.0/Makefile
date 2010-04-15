#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2006-2008 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include config/make.cfg 

SAGA_SUBDIRS   = config

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += lsf_job
endif


all:: config.summary

config.summary:
	@echo ""
	@echo " ================================= "
	@echo "  you need to run configure first  "
	@echo " ================================= "
	@echo ""
	@false


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk
-include $(SAGA_LOCATION)/share/saga/make/saga.dist.mk


