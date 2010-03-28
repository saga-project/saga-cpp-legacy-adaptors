#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_SUBDIRS    = config

-include config/make.cfg

ifeq "x$(SAGA_BUILD_ADAPTOR_FILESYSTEM)" "xyes"
  SAGA_SUBDIRS    += ssh_file
  BUILD_SOMETHING  = yes
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += ssh_job
  BUILD_SOMETHING  = yes
endif

ifdef BUILD_SOMETHING
  SAGA_SUBDIRS += ssh_context
# SAGA_SUBDIRS += ssh_suite
endif

ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS += ssh_file ssh_job
endif


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk


ifeq "x$(SAGA_BUILD_ADAPTOR_FILE)" "xyes"
ssh_file:    ssh_context
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
ssh_job:     ssh_context
endif

ifdef BUILD_SOMETHING
# ssh_context: ssh_suite 
endif

