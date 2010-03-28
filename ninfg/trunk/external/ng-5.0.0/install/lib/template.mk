#  $RCSfile: template.mk.in,v $ $Revision: 1.4 $ $Date: 2008/01/11 03:22:14 $
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

# for common

NG_INCLUDES	=	-I$(NG_DIR)/include

NG_COMMON_LIBS	=	-lngcommon -lngnet -lngutility \
			-lexpat -lz  

# for server

NG_CFLAGS	=	-g -O2 -Wall -D_THREAD_SAFE  $(NG_INCLUDES)

NG_CPPFLAGS	=	 -DHAVE_CONFIG_H

NG_LIBS		=	-lngexecutable $(NG_COMMON_LIBS)

NG_STUB_LDFLAGS	=	-L$(NG_DIR)/lib $(NG_LIBS)

# for client

NG_CLIENT_COMPILER	= gcc
NG_CLIENT_CFLAGS	= -g -O2 -Wall -D_THREAD_SAFE 
NG_CLIENT_CPPFLAGS	= $(NG_CPPFLAGS) $(NG_INCLUDES)
NG_CLIENT_LIBS		= -lnggrpc -lngclient $(NG_COMMON_LIBS)
NG_CLIENT_LDFLAGS	= -L$(NG_DIR)/lib

