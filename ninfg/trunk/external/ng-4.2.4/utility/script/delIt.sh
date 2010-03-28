# $RCSfile: delIt.sh,v $ $Revision: 1.3 $ $Date: 2003/10/28 05:33:37 $
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
getSuffix () {
    _sfx_=`echo $1 | awk -F. '{ print $NF }'`
    if test "x${_sfx_}" = "x$1"; then
	_sfx_=""
    elif test "x${_sfx_}" = "x"; then
	_sfx_="."
    else
	_sfx_=.${_sfx_}
	if test "x$1" = "x${_sfx_}"; then
	    _sfx_=""
	fi
    fi
    
    echo "%${_sfx_}%" | sed 's:%::g'
    unset _sfx_
}


delIt () {
    for _i_ in $*
    do
	if test -d ${_i_}; then
	    rm -rf ${_i_}
	    st=$?
	    continue
	fi

	rm -f ${_i_}
	st=$?

	_sfx_=`getSuffix ${_i_}`
	if test "x${_sfx_}" = "x"; then
	    # no suffix.
	    rm -f ${_i_}${exe}
	    st=$?
	elif test "x${_sfx_}" = "x.o"; then
	    # foo.o
	    bNm=`echo ${_i_} | sed "s:${_sfx_}$::"`
	    rm -f ${bNm}${obj}
	    st=$?
	fi
    done

    unset _i_
    unset _sfx_
    return $st
}
