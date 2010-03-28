# $RCSfile: ninfg-user-env.csh,v $ $Revision: 1.1 $ $Date: 2007/01/29 07:53:24 $
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
# set up your environment for Ninf-G applications.
# this requires that NG_DIR be set.
#

if (! $?NG_DIR ) then
    echo "ERROR: environment variable NG_DIR not defined"
    exit 1
endif

# delete previous set of $PATH
if ( $?NG_PATH ) then
    setenv PATH `echo "${PATH}" | sed -e "s%:${NG_PATH}/bin%%g" -e "s%^${NG_PATH}/bin:\{0,1\}%%"`
endif

setenv PATH `echo "${PATH}" | sed -e "s%:${NG_DIR}/bin%%g" -e "s%^${NG_DIR}/bin:\{0,1\}%%"`

# add new set of $PATH
setenv NG_PATH "${NG_DIR}"
setenv PATH "${NG_DIR}/bin:${PATH}"

# default loglevel = Error
setenv NG_LOG_LEVEL 2

