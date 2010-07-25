#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2010 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Distributed under the Boost Software License, Version 1.0. (See accompanying 
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg

SAGA_SUBDIRS   = test

SAGA_SRC         = $(wildcard *.cpp)
SAGA_OBJ         = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_NAME = fast_advert
SAGA_ADAPTOR_INI  = fast_advert_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES     = advert
SAGA_ADAPTOR_REQUIREMENTS = soci

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk
-include ../config/saga.config.soci.c.mk

distclean::
	@$(RM) fast_advert_adaptor.ini

