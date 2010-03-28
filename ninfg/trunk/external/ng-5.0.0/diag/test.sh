#! /bin/sh

# $RCSfile: test.sh,v $ $Revision: 1.3 $ $Date: 2007/09/03 03:19:11 $
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

Usage="./test.sh client.conf"

conf=$1
failure_count=0
failure_tests=
test_count=0

# Check arguments
if [ "X${conf}" = "X" ]
then
    echo "Client configuration file isn't specified."
    echo $Usage >&2
    exit 1
fi

uname_result=`uname 2> /dev/null`
if [ "X${uname_result}" = "XOSF1" ]
then
    BIN_SH=xpg4
    export BIN_SH
fi

# Async test
command="./async_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Callback test
command="./callback_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Cancel test
command="./cancel_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Data test
command="./data_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# File test
command="./file_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Null argument test
command="./nullArgument_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Skip test
command="./skip_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Zero element test
command="./zeroElement_test $conf"
test_count=`expr $test_count + 1`
echo $command
$command
if [ $? -ne 0 ]
then
    failure_count=`expr $failure_count + 1`
    failure_tests="${failure_tests} `echo ${command} | sed 's/^\.\/\(.*_test\).*$/\1/' `"
fi

# Print Results.
if [ $failure_count -ne 0 ]
then
    echo "----------------------------" >&2
    echo " ${failure_count} tests failed in ${test_count} tests. " >&2
    echo " Failure tests:"
    for t in ${failure_tests}
    do
        echo "      $t"
    done
    echo "----------------------------" >&2

    exit 1
else

    echo "----------------------------" >&2
    echo " All tests were successful. " >&2
    echo "----------------------------" >&2

    exit 0
fi
