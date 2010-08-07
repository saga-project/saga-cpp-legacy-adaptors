#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include config/make.cfg

SAGA_SUBDIRS       = config

ifeq "x$(SAGA_BUILD_ADAPTOR_FILESYSTEM)" "xyes"
  SAGA_SUBDIRS    += ssh_file
  BUILD_SOMETHING  = yes
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS    += ssh_job
  BUILD_SOMETHING  = yes
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
  SAGA_SUBDIRS    += ssh_context
# SAGA_SUBDIRS    += ssh_suite
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


ssh_file:    ssh_context
ssh_job:     ssh_context
# ssh_context: ssh_suite 

