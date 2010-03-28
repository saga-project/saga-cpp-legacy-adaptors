#!/bin/sh

# $RCSfile: ngCompile.sh,v $ $Revision: 1.2 $ $Date: 2005/09/22 09:40:46 $
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

# ngCompile.sh
#
# This tool is similar to ng_cc.
# But, do not refer $NG_DIR.
#
# including template.mk in Makefile.in is not work well.
# "make distclean" removes template.mk at first,
# Then make distclean on this directory fails.
# Thus, this tool is used.
#
# Usage:
# % ngCompile.sh include-file compiler arguments ...
#

topDir=$1
shift

compiler=$1
shift

objs=$1
shift

. ${topDir}/lib/template.sh

echo ${compiler} ${NG_CPPFLAGS} ${NG_CFLAGS} ${objs} ${NG_GT_LDFLAGS} $*
${compiler} ${NG_CPPFLAGS} ${NG_CFLAGS} ${objs} ${NG_GT_LDFLAGS} $*

exitCode=$?

exit $exitCode

