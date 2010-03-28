#! /bin/sh
# $RCSfile: ngclSessionInvokeCallback.sh,v $ $Revision: 1.3 $ $Date: 2007/12/14 03:10:54 $
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

max_arg=32

# 0
cat << EOF
/* This file is generate by ngclSessionInvokeCallback.sh */
EOF
echo "typedef void (*ngcllCallbackFunc0)(void);"

i=1
while [ "$i" -le "${max_arg}" ];
do
    line="typedef void (*ngcllCallbackFunc${i})("

    j=0
    while [ "$j" -lt "$i" ];
    do
        line="${line}void *"
        j=`expr $j + 1`
        if [ "$j" -ge "$i" ];
        then
            break;
        fi
        line="${line}, "
    done
    line="${line});"
    echo "${line}"

    i=`expr $i + 1`
done

cat << EOF
/*
 * Invoke Callback
 */
int
ngcliSessionInvokeCallback(
    void (*callbackFunc)(void),
    ngiArgument_t *arg,
    ngLog_t *log,
    int *error)
{
    static const char fName[] = "ngcliSessionInvokeCallback";

    switch (arg->nga_nArguments){
EOF

i=0
while [ "$i" -le "${max_arg}" ];
do
    echo "    case ${i}:"
    line="        ((ngcllCallbackFunc${i})(callbackFunc))("

    j=0
    while [ "$j" -lt "$i" ];
    do
        echo "${line}"
        line="           arg->nga_argument[${j}].ngae_pointer.ngap_void"
        j=`expr $j + 1`
        if [ "$j" -ge "$i" ];
        then
            break;
        fi
        line="${line},"
    done
    echo "${line});"
    echo "        break;"

    i=`expr $i + 1`
done

cat << EOF
    default:
        NGI_SET_ERROR(error, NG_ERROR_INVALID_ARGUMENT);
        ngLogError(log, NG_LOGCAT_NINFG_PURE, fName,  
            "Too many argument for callback.\n"); 
        return 0;
    }

    /* Success */
    return 1;
}
EOF
