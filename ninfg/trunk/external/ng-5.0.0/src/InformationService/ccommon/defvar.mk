# $RCSfile: defvar.mk,v $ $Revision: 1.1 $ $Date: 2007/05/11 08:27:58 $
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

include ${TOPDIR}/src/lib/ExternalModule/defvar.mk

INFORMATION_SERVICE_COMMON_LIB=${TOPDIR}/src/InformationService/ccommon/libngincommon.a

INCLUDES	+= -I${TOPDIR}/src/InformationService/ccommon/
LDFLAGS		+= -L${TOPDIR}/src/InformationService/ccommon/
LIBS		:= -lngincommon ${LIBS} 
