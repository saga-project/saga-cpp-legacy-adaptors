#!/bin/sh

# $RCSfile: gtvariable.sh,v $ $Revision: 1.1 $ $Date: 2007/01/29 07:53:23 $
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
# Get variable from globus-makefile-header result.
#

usage="Usage: ${0} file variable-name"

result_file=$1
variable=$2

if [ "x${result_file}" = "x" ]; then
    echo $usage >&2
    exit 1
fi

if [ "x${variable}" = "x" ]; then
    echo $usage >&2
    exit 1
fi

if [ ! -f $result_file ]; then
    echo "${result_file} not exist." >&2
    exit 1
fi

line=`grep "^${variable} " ${result_file}`

if [ "x${line}" = "x" ]; then
    exit 2
fi

output=`echo ${line} | sed "s/^${variable} *= *//"`

echo $output

exit 0

