#!/bin/sh -x

# $RCSfile: bootstrap.sh,v $ $Revision: 1.2 $ $Date: 2007/03/15 05:33:56 $
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
# The script is to generate "configure" script from configure.in.
#

if [ ! -f configure.in ]; then
    : echo "no configure.in in this directory."
    exit 1
fi

if [ -f Makefile ]; then
    make distclean
fi

aclocal -I utility/configure/m4
autoheader
autoconf

#remove cache
rm -rf autom4te.cache aclocal.m4

