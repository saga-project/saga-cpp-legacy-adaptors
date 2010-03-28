#  Copyright (c) 2005-2006 Andre Merzky (andre@merzky.net)
# 
#  Use, modification and distribution is subject to the Boost Software
#  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt)

-include ../../config/make.cfg
-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.util.mk

SAGA_TOOL_DIR   = aws-tools/bin/
SAGA_TOOLS      = $(wildcard saga-image-prep-*)

-include $(SAGA_MAKE_INCLUDE_ROOT)/saga.mk

install::
	 @$(ECHO) "      installing   aws tools" 
	 @$(INSTALL)     $(SAGA_TOOLS)   $(SAGA_BIN_ROOT)/$(SAGA_TOOL_DIR)

