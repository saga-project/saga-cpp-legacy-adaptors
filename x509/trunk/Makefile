#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_SUBDIRS = context 

ifdef SAGA_ROOT
  SAGA_MAKE_INCLUDE_ROOT = $(SAGA_ROOT)/make
endif

ifdef SAGA_LOCATION
  SAGA_MAKE_INCLUDE_ROOT = $(SAGA_LOCATION)/share/saga/make
endif

ifndef SAGA_MAKE_INCLUDE_ROOT
 $(error "Set SAGA_LOCATION or SAGA_ROOT to use the SAGA build system")
endif

include $(SAGA_MAKE_INCLUDE_ROOT)/saga.config.mk


ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
  SAGA_SUBDIRS += context
endif

ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS += context
endif

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

