# $RCSfile: defvar.mk,v $ $Revision: 1.6 $ $Date: 2008/02/14 09:06:45 $
# $AIST_Release: 5.0.0 $
# $AIST_Copyright:
#  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#  National Institute of Advanced Industrial Science and Technology
#  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  $

#
# Default variables.
#
COMMON_UTILITY_LIB=${TOPDIR}/src/lib/utility/libngutility.a
EXTERNAL_MODULE_COMMON_LIB=${TOPDIR}/src/lib/ExternalModule/libngemcommon.a

EM_LIBS		=	${EXTERNAL_MODULE_COMMON_LIB} \
			${COMMON_UTILITY_LIB}

CFLAGS		=	$(NG_CFLAGS)
CPPFLAGS	=	$(NG_CPPFLAGS)
LDFLAGS 	=	$(NG_LDFLAGS) -L${TOPDIR}/src/lib/ExternalModule \
			-L${TOPDIR}/src/lib/utility
INCLUDES	=	$(NG_INCLUDES) -I${TOPDIR}/src/lib/ExternalModule
LIBS 		:= 	-lngemcommon -lngutility ${NG_LIBS}
