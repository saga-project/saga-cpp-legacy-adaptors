#  Copyright (c) 2005-2007 Andre Merzky (andre@merzky.net)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

ifndef SAGA_LOCATION
 $(error "SAGA_LOCATION must be set for this Makefile to work")
endif

KFS_DIR           = /home/mmicel2/kfs/kfs-0.2.2/build/include/kfs

SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = filesystem
SAGA_ADAPTOR_NAME = kfs_file
SAGA_ADAPTOR_INI  = kfs_file_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = filesystem namespace

include $(SAGA_LOCATION)/share/saga/make/saga.adaptor.mk

# add kfs flags
SAGA_CXXFLAGS += -I${KFS_DIR}
SAGA_LDFLAGS  += -L/home/mmicel2/kfs/kfs-0.2.2/build/lib/
SAGA_LDFLAGS  += -lkfsClient
