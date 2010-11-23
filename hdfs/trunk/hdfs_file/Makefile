#  Copyright (c) 2005-2007 Chris Miceli (cmicel1@cct.lsu.edu)
# 
#  Distributed under the Boost Software License, 
#  Version 1.0. (See accompanying LICENSE file 
#  or copy at http://www.boost.org/LICENSE_1_0.txt)

include ../config/make.cfg

ifndef HADOOP_HOME
 $(error "HADOOP_HOME must be set for this Makefile to work")
endif



SAGA_SRC          = $(wildcard *.cpp)
SAGA_OBJ          = $(SAGA_SRC:%.cpp=%.o)

SAGA_ADAPTOR_TYPE = filesystem
SAGA_ADAPTOR_NAME = hdfs_file
SAGA_ADAPTOR_INI  = hdfs_file_adaptor.ini
SAGA_ADAPTOR_OBJ  = $(SAGA_OBJ)

SAGA_ADAPTOR_PACKAGES = filesystem namespace

include $(SAGA_MAKE_INCLUDE_ROOT)/saga.adaptor.mk

