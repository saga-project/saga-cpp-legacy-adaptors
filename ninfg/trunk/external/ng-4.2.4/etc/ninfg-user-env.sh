#

# $RCSfile: ninfg-user-env.sh,v $ $Revision: 1.6 $ $Date: 2005/02/15 07:03:16 $
# $AIST_Release: 4.2.4 $
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

# set up your environment for Ninf-G applications.
# this requires that NG_DIR be set.
#

if [ -z "${NG_DIR}" ]; then
    echo "ERROR: environment variable NG_DIR not defined" 1>&2
    return 1
fi

# delete previous set of $PATH
if [ -n "${NG_PATH}" ]; then
    PATH=`echo "${PATH}" | sed -e "s%:${NG_PATH}/bin%%g" -e "s%^${NG_PATH}/bin:\{0,1\}%%"`
fi

PATH=`echo "${PATH}" | sed -e "s%:${NG_DIR}/bin%%g" -e "s%^${NG_DIR}/bin:\{0,1\}%%"`

# add new set of $PATH
NG_PATH=${NG_DIR}
PATH="${NG_DIR}/bin:${PATH}";

# default loglevel = Error
NG_LOG_LEVEL=2

export NG_PATH PATH NG_LOG_LEVEL
