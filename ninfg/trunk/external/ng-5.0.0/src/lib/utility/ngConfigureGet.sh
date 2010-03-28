#!/bin/sh

# $RCSfile: ngConfigureGet.sh,v $ $Revision: 1.4 $ $Date: 2007/03/16 06:09:22 $
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
# command ngConfigureGet.sh
#
# This script gets configure options specified by user and
# configure results.
# configure option is brought from config.log,
# which was created by configure.
#
# Assumably, This program depends on strict autoconf-2.59 implementation.
#

log_file=$1
result_file=$2

if [ "x${log_file}" = "x" -o "x${result_file}" = "x" ]; then
    command_name=`basename $0`;
    echo "Usage: ${command_name} config.log ngConfigureValue.mk" >&2
    exit 1
fi

if [ ! -r $log_file ]; then
    echo "no configure log file \"${log_file}\"." >&2
    exit 1
fi

if [ ! -r $result_file ]; then
    echo "no configure result file \"${result_file}\"." >&2
    exit 1
fi

# Get configure options and build hostname.

configure_options=`grep '^ *\$ ' ${log_file} | grep configure`

configure_host=`grep '^hostname *=' ${log_file}`

# output
echo_for_c_string () {
    string=$1
    echo $string | sed -e 's/^/\"/' -e 's/$/\\n\"/'
}

echo
echo "#ifndef _NG_CONFIGURE_RESULT_H_"
echo "#define _NG_CONFIGURE_RESULT_H_"
echo
echo "/* configure result. */"
echo "static char *nglConfigureResult ="
echo_for_c_string "configure options : ${configure_options}"
echo_for_c_string "configure host    : ${configure_host}"
echo_for_c_string ""

sed -e '/^$/d' -e '/^#$/d' -e '/^#[^#]/d' \
    -e 's/^##//' -e 's/^/\"/' -e 's/$/\\n\"/' $result_file

echo_for_c_string ""
echo ";"
echo
echo "#endif /* _NG_CONFIGURE_RESULT_H_ */"

exit

