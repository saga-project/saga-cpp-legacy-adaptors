#! /bin/sh
# $RCSfile: makeeach.sh,v $ $Revision: 1.1 $ $Date: 2007/11/27 03:45:02 $
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

MAKE=$1
if test -z "$MAKE"; then
    echo '$0: make command is not specified.' >&1
    exit 1
fi
shift
target=$1
if test -z "$target"; then
    echo '$0: target is not specified.' >&1
    exit 1
fi
shift

for t in / "$@"; do
    case $t in
        /)  true;;
        *)
            if test -f "./Makefile.$t"; then
                (${MAKE} -f "./Makefile.$t" ${target}) || exit 1
            else
                (cd $t && ${MAKE} ${target}) || exit 1
            fi
        ;;
    esac
done

exit 0
