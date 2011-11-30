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

#  Copyright (c) 2006-2008 Ole Weidner (oweidner@cct.lsu.edu)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_SRC       = $(wildcard *.cpp)
SAGA_OBJ       = $(SAGA_SRC:%.cpp=%.o)

ifndef irods_file_Dir
irods_file_Dir = ../irods_file
endif
SAGA_CPPINCS     += -I$(irods_file_Dir)
SAGA_SUBDIRS      = $(irods_file_Dir)
SAGA_OBJ         += $(irods_file_Dir)/irods_file_adaptor_file_nsentry_impl.o 
SAGA_OBJ         += $(irods_file_Dir)/irods_api_dir.o $(irods_file_Dir)/irods_api_file.o
SAGA_OBJ         += $(irods_file_Dir)/irods_file_adaptor_helper.o 

ifndef icommandDir
icommandDir = $(irods_file_Dir)/icommands
endif
SAGA_SUBDIRS      = $(icommandDir)
SAGA_OBJ         += $(icommandDir)/icp.o   $(icommandDir)/iget.o
SAGA_OBJ         += $(icommandDir)/ils.o   $(icommandDir)/imkdir.o 
SAGA_OBJ         += $(icommandDir)/imv.o   $(icommandDir)/iput.o 
SAGA_OBJ         += $(icommandDir)/ipwd.o  $(icommandDir)/irm.o
SAGA_OBJ         += $(icommandDir)/icd.o   $(icommandDir)/iquest.o
SAGA_OBJ         += $(icommandDir)/iread.o $(icommandDir)/iwrite.o
SAGA_OBJ         += $(icommandDir)/imeta.o 

SAGA_ADAPTOR_TYPE  = replica
SAGA_ADAPTOR_NAME  = irods_replica
SAGA_ADAPTOR_INI   = irods_replica_adaptor.ini
#SAGA_ADAPTOR_PACKAGES     = replica
#SAGA_ADAPTOR_REQUIREMENTS = irods
SAGA_ADAPTOR_OBJ   = $(SAGA_OBJ)

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

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
