# Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
# Copyright (C) 2008-2012 National Institute of Informatics in Japan.
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 

#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

ifndef commandsDir
commandsDir = ./commands
endif
SAGA_SUBDIRS      = $(commandsDir)

SAGA_ADAPTOR_TYPE = replica
SAGA_ADAPTOR_NAME = rns_replica
SAGA_ADAPTOR_INI  = rns_replica_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

#SAGA_ADAPTOR_PACKAGES = file namespace
SAGA_ADAPTOR_PACKAGES = replica
SAGA_ADAPTOR_UTIL_OPTIONS = java

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

SAGA_CPPINCS += -I$(JAVA_HOME)/include
SAGA_CPPINCS += -I$(JAVA_HOME)/include/linux

SAGA_LDFLAGS += -L$(JAVA_HOME)/jre/lib/i386/client -ljvm
