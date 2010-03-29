#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

-include ../config/make.cfg


SAGA_SUBDIRS       = stubs

ifeq "x$(SAGA_HAVE_OPENSSL)" "xyes"
  SAGA_SUBDIRS    += openssl
endif

SAGA_SRC           = $(wildcard *.cpp)

SAGA_TMP          += stubs/bes/besC.cpp 
SAGA_TMP          += stubs/bes/besBESSOAPBindingProxy.cpp
SAGA_TMP          += stubs/gridsam/gridsamC.cpp 
SAGA_TMP          += stubs/gridsam/gridsamJobControlSOAPBindingProxy.cpp
SAGA_TMP          += stubs/gridsam/gridsamJobMonitoringSOAPBindingProxy.cpp 
SAGA_TMP          += stubs/gridsam/gridsamJobSubmissionSOAPBindingProxy.cpp
SAGA_TMP          += stubs/gsoap-2.7.11/stdsoap2.cpp
SAGA_TMPC         += openssl/th-lock.c

SAGA_OBJ          += $(SAGA_SRC:%.cpp=%.o) 
SAGA_ADD_LIB_OBJ  += $(SAGA_TMP:%.cpp=%.o) 
SAGA_ADD_LIB_OBJC += $(SAGA_TMPC:%.c=%.o) 

SAGA_ADAPTOR_NAME  = omii_gridsam_job
SAGA_ADAPTOR_INI   = omii_gridsam_adaptor.ini
SAGA_ADAPTOR_OBJ   = $(SAGA_OBJ) $(SAGA_OBJC)

# SAGA_ADAPTOR_REQUIREMENTS = openssl
SAGA_ADAPTOR_PACKAGES     = job

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk
-include ../config/make.cfg
-include ../config/saga.config.openssl.c.mk

SAGA_CPPINCS     += -Istubs/gsoap-2.7.11
SAGA_CPPDEFS     += -DPTHREADS -DWITH_NONAMESPACES -DWITH_DOM -DWITH_IPV6

ifeq "x$(SAGA_HAVE_OPENSSL)" "xyes"
  SAGA_CPPDEFS   += -DWITH_OPENSSL -DWITH_COOKIES
endif

