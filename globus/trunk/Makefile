#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2006-2008 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include config/make.cfg 

SAGA_SUBDIRS   = config

ifeq "x$(BUILD_ADAPTOR_LOADER)" "xyes"
  SAGA_SUBDIRS += loader
endif

ifeq "x$(BUILD_ADAPTOR_REPLICA)" "xyes"
  SAGA_SUBDIRS += replica
endif

ifeq "x$(BUILD_ADAPTOR_FILE)" "xyes"
  SAGA_SUBDIRS += file
endif

ifeq "x$(BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += job
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
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.dist.mk


# dir depenencies for parallel builds
job:     loader
file:    loader
replica: loader

