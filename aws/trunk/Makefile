#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

SAGA_SUBDIRS    = config internal aws_suite aws_context

-include config/make.cfg

ifeq "x$(SAGA_BUILD_ADAPTOR_JOB)" "xyes"
  SAGA_SUBDIRS += aws_job
else
 ifdef SAGA_IS_CLEANING
   SAGA_SUBDIRS += aws_job
 endif
endif

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

# directory dependencies...
aws_suite:     internal
aws_context:   aws_suite
aws_job:       aws_context

