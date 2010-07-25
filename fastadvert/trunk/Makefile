#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2010 Ole Weidner (oweidner@cct.lsu.edu)
#
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include $(SAGA_LOCATION)/share/saga/make/saga.util.mk
-include config/make.cfg

SAGA_SUBDIRS    = config 

ifeq "x$(SAGA_BUILD_ADAPTOR_ADVERT)" "xyes"
  SAGA_SUBDIRS += external
  SAGA_SUBDIRS += advert
endif


ifdef SAGA_IS_CLEANING
  SAGA_SUBDIRS += advert
endif

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.dist.mk


# directory dependencies...
advert:    external
