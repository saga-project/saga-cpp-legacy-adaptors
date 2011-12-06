#
# SYNOPSIS
#
#   AX_SAGA_CHECK_ZMQ([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the ZMQ libraries of a particular version (or newer)
#
#   If no path to the installed zmq library is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/zmq-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_ZMQ)
#     AC_SUBST(ZMQ_LOCATION)
#     AC_SUBST(ZMQ_CPPFLAGS) 
#     AC_SUBST(ZMQ_LDFLAGS)
#     AC_SUBST(ZMQ_S_LIBS)
#
# LAST MODIFICATION
#
#   2007-03-04
#
# COPYLEFT
#
#   Copyright (c) 2007 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_SAGA_CHECK_ZMQ],
[
  AC_ARG_VAR([ZMQ_LOCATION],[ZMQ installation directory])

  HAVE_ZMQ=no

  tmp_location=""
  AC_ARG_WITH([zmq],
              AS_HELP_STRING([--with-zmq=DIR],
              [use zmq (default is YES) at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                want_zmq="no"
              elif test "$withval" = "yes"; then
                want_zmq="yes"
                tmp_location=""
              else
                want_zmq="yes"
                tmp_location="$withval"
              fi
              ],
              [want_zmq="yes"])

  # use ZMQ_LOCATION if avaialble, and if not 
  # overwritten by --with-zmq=<dir>

  if test "x$want_zmq" = "xyes"; then
    
    packages=`ls /usr/local/package/{zmq,zeromq}-* 2>>/dev/null`
  
    for tmp_path in $tmp_location $ZMQ_LOCATION /usr /usr/local /opt $packages; do
      
      AC_MSG_CHECKING(for zmq in $tmp_path)

      saved_cppflags=$CPPFLAGS
      saved_ldflags=$LDFLAGS

      ZMQ_PATH=$tmp_path
      ZMQ_LDFLAGS="-L$tmp_path/lib/ -L$tmp_path/lib64/ -lzmq"
      ZMQ_CPPFLAGS="-I$tmp_path/include/"

      CPPFLAGS="$CPPFLAGS $ZMQ_CPPFLAGS"
      export CPPFLAGS

      LDFLAGS="$LDFLAGS $ZMQ_LDFLAGS"
      export LDFLAGS

      AC_LINK_IFELSE([AC_LANG_PROGRAM([[@%:@include <zmq.h>]],
                                      [[void * context = zmq_init (1);
                                        if ( NULL == context )
                                          return -1;
                                        return 0;
                                      ]])],
                                      link_zmq="yes",
                                      link_zmq="no")
            
      if test "x$link_zmq" = "xno"; then
      
        AC_MSG_RESULT(no)

        LDFLAGS=$saved_ldflags
        CPPFLAGS=$saved_cppflags
      
      else

        AC_MSG_RESULT(yes)

        SAGA_ZMQ_DEP_FILES="$tmp_path/include/zmq.h"

        ZMQ_VERSION_MAJ=`grep ZMQ_VERSION_MAJOR $tmp_path/include/zmq.h | cut -f 3 -d ' '`
        ZMQ_VERSION_MIN=`grep ZMQ_VERSION_MINOR $tmp_path/include/zmq.h | cut -f 3 -d ' '`
        ZMQ_VERSION_SUB=`grep ZMQ_VERSION_PATCH $tmp_path/include/zmq.h | cut -f 3 -d ' '`

        ZMQ_VERSION="$ZMQ_VERSION_MAJ.$ZMQ_VERSION_MIN.$ZMQ_VERSION_SUB"
        export ZMQ_VERSION


        AC_MSG_CHECKING(for static lib zmq)
        if test -e "$zmq_libpath/libzmq.a"; then
          ZMQ_S_LIBS="$zmq_libpath/libzmq.a"
          AC_MSG_RESULT([$ZMQ_S_LIBS])
        else
          AC_MSG_RESULT([no])
        fi

        ZMQ_LOCATION=$tmp_path
        HAVE_ZMQ=yes

        export HAVE_ZMQ

        AC_SUBST(HAVE_ZMQ)
        AC_SUBST(ZMQ_VERSION)
        AC_SUBST(ZMQ_LOCATION)
        AC_SUBST(ZMQ_CPPFLAGS)
        AC_SUBST(ZMQ_LDFLAGS)
        AC_SUBST(ZMQ_S_LIBS)
        
        if test "$tmp_location" != "" && \
           test "$tmp_location" != "$tmp_path" ; then
          AC_MSG_WARN([ZMQ found:
                       not in $tmp_location
                       but in $tmp_path])
        fi

        break;

      fi # link ok

    done # foreach path


    if test "$HAVE_ZMQ" == "no" ; then
      ZMQ_LOCATION=""
      AC_MSG_WARN(ZMQ not found)
    fi

  fi # want_zmq

])

