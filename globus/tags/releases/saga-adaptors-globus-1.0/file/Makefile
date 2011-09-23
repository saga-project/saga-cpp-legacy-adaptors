#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg

#SAGA_SUBDIRS   = test

SAGA_SRC       = $(wildcard *.cpp)
SAGA_OBJ       = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_NAME  = globus_gridftp_file
SAGA_ADAPTOR_INI   = globus_gridftp_file_adaptor.ini
SAGA_ADAPTOR_OBJ   = $(SAGA_OBJ)

SAGA_ADAPTOR_REQUIREMENTS = globus globus_gridftp
SAGA_ADAPTOR_PACKAGES     = filesystem namespace

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk 
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.doc.mk

SAGA_LDFLAGS += -L$(SAGA_PWD)/../loader -lsaga_adaptor_globus_module_loader_util

