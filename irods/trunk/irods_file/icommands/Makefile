#
# Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
# Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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

#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
#  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)


-include $(SAGA_LOCATION)/share/saga/make/saga.mk

ifndef iRODS_buildDir
iRODS_buildDir = $(iRODS_HOME)
endif

include $(iRODS_buildDir)/config/config.mk
include $(iRODS_buildDir)/config/platform.mk
include $(iRODS_buildDir)/config/directories.mk
include $(iRODS_buildDir)/config/common.mk

SAGA_CPPINCS += -I$(iRODS_buildDir)/server/core/include
SAGA_CPPINCS += -I$(iRODS_buildDir)/server/icat/include
SAGA_CPPINCS += -I$(iRODS_buildDir)/server/drivers/include
SAGA_CPPINCS += -I$(iRODS_buildDir)/server/re/include
SAGA_CPPINCS += -I$(iRODS_buildDir)/lib/core/include
SAGA_CPPINCS += -I$(iRODS_buildDir)/lib/api/include 
SAGA_CPPINCS += -I$(iRODS_buildDir)/lib/md5/include

SAGA_LDFLAGS += -L$(iRODS_buildDir)/lib/core/obj -lRodsAPIs
