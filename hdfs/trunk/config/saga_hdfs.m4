#
# SYNOPSIS
#
#   AX_SAGA_CHECK_LIBHDFS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the HDFS library of a particular version (or newer)
#
#   If no path to the installed libhdfs library is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/libhdfs-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_LIBHDFS)
#     AC_SUBST(LIBHDFS_LOCATION)
#     AC_SUBST(LIBHDFS_CPPFLAGS) 
#     AC_SUBST(LIBHDFS_LDFLAGS)
#     AC_SUBST(LIBHDFS_S_LIBS)
#
# LAST MODIFICATION
#
#   2010-11-22
#
# COPYLEFT
#
#   Copyright (c) 2007 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty
#   provided the copyright notice and this notice are preserved.

AC_DEFUN([AX_SAGA_CHECK_LIBHDFS],
[
  AC_ARG_VAR([LIBHDFS_LOCATION],[LIBHDFS installation directory])

  HAVE_LIBHDFS="no"
  tmp_location=""

  AC_ARG_WITH([libhdfs-location],
              AS_HELP_STRING([--with-libhdfs-location=DIR],
              [use libhdfs (default is 'autodetect') at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                tmp_location=""
              elif test "$withval" = "yes"; then
                tmp_location=""
              else
                tmp_location="$withval"
              fi
              ],
              [tmp_location=""])

  # use LIBHDFS_LOCATION if avaialble, and if not 
  # overwritten by --with-libhdfs=<dir>

  packages=`ls /usr/local/package/hdfs* 2>>/dev/null`
  
  for tmp_path in $tmp_location $LIBHDFS_LOCATION /usr /usr/local /opt /opt/local $packages; do
    
    AC_MSG_CHECKING(for libhdfs in $tmp_path)

    have_something=`ls $tmp_path/lib/libhdfs.*   2>/dev/null`

    saved_cppflags=$CPPFLAGS
    saved_ldflags=$LDFLAGS

    LIBHDFS_PATH=$tmp_path
    LIBHDFS_LDFLAGS="-L$tmp_path/lib/ -lhdfs"
    LIBHDFS_CPPFLAGS="-I$tmp_path/include/"

    CPPFLAGS="$CPPFLAGS $LIBHDFS_CPPFLAGS"
    export CPPFLAGS

    LDFLAGS="$LDFLAGS $LIBHDFS_LDFLAGS"
    export LDFLAGS

    AC_LINK_IFELSE([AC_LANG_PROGRAM([[@%:@include <hdfs.h>]],
                                    [[
                                      hdfsFS test;
                                      return (0);
                                    ]])],
                                    link_libhdfs="yes",
                                    link_libhdfs="no")
          
    if test "x$link_libhdfs" = "xyes"; then

      AC_MSG_RESULT(yes)

      AC_MSG_CHECKING(for static lib libhdfs)
      if test -e "$tmp_path/lib/libhdfs.a"; then
        LIBHDFS_S_LIBS="$tmp_path/lib/libhdfs.a"
        AC_MSG_RESULT([$LIBHDFS_S_LIBS])
      else
        AC_MSG_RESULT([no])
      fi

      LIBHDFS_LOCATION=$tmp_path
      LIBHDFS_SOURCE="system"
      HAVE_LIBHDFS=yes

      export HAVE_LIBHDFS

      break;
      
    else # link ok

      AC_MSG_RESULT(no)
      LIBHDFS_LDFLAGS=""
      LIBHDFS_CPPFLAGS=""

    fi # link ok

  done # foreach path


  if test "x$HAVE_LIBHDFS" = "xno"; then
    AC_MSG_ERROR([Could not find libhdfs])
  fi

  AC_SUBST(HAVE_LIBHDFS)
  AC_SUBST(LIBHDFS_LOCATION)
  AC_SUBST(LIBHDFS_SOURCE)
  AC_SUBST(LIBHDFS_CPPFLAGS)
  AC_SUBST(LIBHDFS_LDFLAGS)
  AC_SUBST(LIBHDFS_S_LIBS)
  AC_SUBST(LIBHDFS_NEEDS_BOOL)

])

