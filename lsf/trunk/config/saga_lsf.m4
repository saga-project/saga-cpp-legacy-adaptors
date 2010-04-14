
#
# SYNOPSIS
#
#   AX_SAGA_CHECK_LSF([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the LSF libraries of a particular version (or newer)
#
#   If no path to the installed lsf library is given,
#   the macro searchs under $LSF_LOCATION, /usr, /usr/local, /opt, 
#   /usr/lsf, /usr/local/lsf, /opt/lsf, and /usr/local/package/lsf-*,
#   in that order.
#
#   This macro calls:
#
#     AC_SUBST(SAGA_HAVE_LSF)
#     AC_SUBST(LSF_LOCATION)
#     AC_SUBST(LSF_VERSION) 
#     AC_SUBST(LSF_PATH) 
#     AC_SUBST(LSF_BIN_VERSION) 
#     AC_SUBST(LSF_BIN_Q) 
#     AC_SUBST(LSF_BIN_SUBMIT) 
#     AC_SUBST(LSF_BIN_RM) 
#
# LAST MODIFICATION
#
#   2008-09-26
#
# COPYLEFT
#
#   Copyright (c) 2007 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_SAGA_CHECK_LSF],
[
  AC_ARG_VAR([LSF_LOCATION],[lsf installation directory])
  AC_ARG_VAR([LSF_BINDIR],[lsf tools directory])

  SAGA_HAVE_LSF=no

  tmp_location=""
  AC_ARG_WITH([lsf],
              AS_HELP_STRING([--with-lsf=DIR],
              [use lsf (default is YES) at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                want_lsf="no"
              elif test "$withval" = "yes"; then
                want_lsf="yes"
                tmp_location=""
              else
                want_lsf="yes"
                tmp_location="$withval"
              fi
              ],
              [want_lsf="yes"])

  # use LSF_LOCATION and LSF_BINDIR if avaialble, and if not 
  # overwritten by --with-lsf=<dir>

  # echo "tmp_location: '$tmp_location'"
  
  if test "x$tmp_location" != "x"; then

    lsf_location=$tmp_location

  else

    if test "x$LSF_BINDIR" != "x"; then
      lsf_location=`echo $LSF_BINDIR | rev | cut -f 2- -d '/' | rev`
      
      if test "x$lsf_location" = "x"; then
        lsf_location=$LSF_BINDIR
      fi
    fi

    if test "x$LSF_LOCATION" != "x"; then
      lsf_location=$LSF_LOCATION
    fi
  fi

  # echo "lsf_location: $lsf_location"

  if test "x$want_lsf" = "xyes"; then
    
    packages=`ls /usr/local/package/lsf-* 2>>/dev/null`

    if test "$tmp_location-$LSF_LOCATION" = "-"; then
      paths="/usr/lsf /usr/local/lsf /opt/lsf /usr /usr/local /opt /sw $packages"
    else
      paths="$tmp_location $LSF_LOCATION"
    fi

    for tmp_path in $paths; do
      
      AC_MSG_CHECKING(for lsf in $tmp_path)
    
      test -x $tmp_path/bin/lsinfo      && LSF_BIN_INFO=$tmp_path/bin/lsinfo
      test -x $tmp_path/bin/bsub        && LSF_BIN_BSUB=$tmp_path/bin/bsub   
      test -x $tmp_path/bin/bstop       && LSF_BIN_BSTOP=$tmp_path/bin/bstop  
      test -x $tmp_path/bin/bresume     && LSF_BIN_BRESUME=$tmp_path/bin/bresume
      test -x $tmp_path/bin/bkill       && LSF_BIN_BKILL=$tmp_path/bin/bkill  
      test -x $tmp_path/bin/bjobs       && LSF_BIN_BJOBS=$tmp_path/bin/bjobs  


      if test "x$LSF_BIN_INFO" != "x"; then
        LSF_VERSION=`$LSF_BIN_INFO -V 2>&1 | head -1 | cut -f 3 -d ' ' | cut -f 1-3 -d '.'`
      fi

      if test "x$LSF_BIN_INFO"      != "x"; then
        if test "x$LSF_BIN_BSUB"      != "x"; then
          if test "x$LSF_BIN_BSTOP"     != "x"; then
            if test "x$LSF_BIN_BRESUME"   != "x"; then
              if test "x$LSF_BIN_BKILL"     != "x"; then
                if test "x$LSF_BIN_BJOBS"     != "x"; then
                  AC_MSG_RESULT(yes)
                  SAGA_HAVE_LSF=yes
                  LSF_LOCATION=$tmp_path
                  LSF_PATH=$tmp_path/bin
                  break;
                fi
              fi
            fi
          fi
        fi
      fi

      AC_MSG_RESULT(no)

    done # foreach path

  fi # want_lsf


  if  test "$SAGA_HAVE_LSF" != "yes"; then

    AC_MSG_WARN([no lsf found])

  else

    export LSF_BIN_VERSION
    export LSF_BIN_Q
    export LSF_BIN_SUBMIT
    export LSF_BIN_RM

    AC_SUBST(LSF_BIN_VERSION)
    AC_SUBST(LSF_BIN_Q)
    AC_SUBST(LSF_BIN_SUBMIT)
    AC_SUBST(LSF_BIN_RM)

  fi


  AC_SUBST(SAGA_HAVE_LSF)
  AC_SUBST(LSF_LOCATION)
  AC_SUBST(LSF_VERSION)
  AC_SUBST(LSF_PATH)

])

