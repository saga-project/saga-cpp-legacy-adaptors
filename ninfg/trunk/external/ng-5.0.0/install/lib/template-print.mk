#  $RCSfile: template-print.mk,v $ $Revision: 1.1 $ $Date: 2007/01/29 07:53:24 $
#  $AIST_Release: 5.0.0 $
#  $AIST_Copyright:
#   Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#   National Institute of Advanced Industrial Science and Technology
#   Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#   
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#   
#       http://www.apache.org/licenses/LICENSE-2.0
#   
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#   $

#
# template-print.mk:
#   print out template.mk rules.
#
# The message is output to stderr,
# to avoid recursive make verbosity.
#

include $(NG_DIR)/lib/template.mk

# for test

print_TEST:
	@echo "print_TEST_check_output" >&2

# for client

print_NG_CLIENT_COMPILER:
	@echo $(NG_CLIENT_COMPILER) >&2

print_NG_CLIENT_CFLAGS:
	@echo $(NG_CLIENT_CFLAGS) >&2

print_NG_CLIENT_CPPFLAGS:
	@echo $(NG_CLIENT_CPPFLAGS) >&2

print_NG_CLIENT_LIBS:
	@echo $(NG_CLIENT_LIBS) >&2

print_NG_CLIENT_LDFLAGS:
	@echo $(NG_CLIENT_LDFLAGS) >&2

