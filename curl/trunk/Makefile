#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_SUBDIRS    = config test

-include config/make.cfg

# ifeq "x$(SAGA_BUILD_ADAPTOR_CONTEXT)" "xyes"
#   SAGA_SUBDIRS += curl_context
# endif

ifeq "x$(SAGA_BUILD_ADAPTOR_FILE)" "xyes"
  SAGA_SUBDIRS += curl_file
endif

ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS +=curl_file # curl_context
endif

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

