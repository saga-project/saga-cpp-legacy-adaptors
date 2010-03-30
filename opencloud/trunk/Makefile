#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include $(SAGA_LOCATION)/share/saga/make/saga.util.mk
-include config/make.cfg

SAGA_SUBDIRS    = config external

ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
  SAGA_SUBDIRS += opencloud_context
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_FILE)" "xyes"
  SAGA_SUBDIRS += opencloud_file
endif

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += opencloud_job
endif

ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS += opencloud_file opencloud_job
endif


-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.dist.mk


external:          config
opencloud_context: external
opencloud_file:    opencloud_context
opencloud_job:     opencloud_context

