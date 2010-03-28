
#
# SYNOPSIS
#
#   AX_SAGA_CHECK_AWS([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the Amazon Web Services (AWS) command line tools.
#
#   If no path to the installed aws tools is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/aws-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_AWS)
#     AC_SUBST(AWS_VERSION)
#     AC_SUBST(AWS_LOCATION)
#
#   This macro requires AX_SAGA_CHECK_JAVA.
#
# LAST MODIFICATION
#
#   2008-10-14
#
# COPYLEFT
#
#   Copyright (c) 2008 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty 
#   provided the copyright notice and this notice are preserved.

m4_include([config/saga_java.m4])

AC_DEFUN([AX_SAGA_CHECK_AWS],
[
  AC_ARG_VAR([AWS_LOCATION],[AWS installation root (EC2_HOME)])


  AX_SAGA_CHECK_JAVA()

  if test "x$HAVE_JAVA" = "xyes"; then

    HAVE_AWS=no
    tmp_location="$AWS_LOCATION"

    AC_ARG_WITH([aws],
                AS_HELP_STRING([--with-aws=DIR],
                [use aws (default is YES) at DIR (optional)]),
                [
                if test "$withval" = "no"; then
                  want_aws="no"
                elif test "$withval" = "yes"; then
                  want_aws="yes"
                  tmp_location="$AWS_LOCATION"
                else
                  want_aws="yes"
                  tmp_location="$withval"
                fi
                ],
                [want_aws="yes"])


    if test "x$want_aws" = "xyes"; then
      
      packages=`ls /usr/local/package/aws-* 2>>/dev/null`
      
      for tmp_path in $tmp_location $EC2_HOME /usr /usr/local /opt $packages; do
        
        AC_MSG_CHECKING(for aws in $tmp_path)

        if ! test -x "$tmp_path/bin/ec2ver"; then
          AC_MSG_RESULT(no)
          continue
        fi

        EC2_HOME="$tmp_path"
        export EC2_HOME

        AWS_VERSION=`$tmp_path/bin/ec2ver 2>&1 | head -1 | cut -f 1 -d '-'`

        if test "x$AWS_VERSION" = "x"; then
          AC_MSG_RESULT(no)
          continue
        fi

        AC_MSG_RESULT(yes)

        AWS_LOCATION=$tmp_path
        HAVE_AWS=yes

        export HAVE_AWS

        # if test "$tmp_location" != "" && \
        #    test "$tmp_location" != "$tmp_path" ; then
        #   AC_MSG_WARN([AWS found:
        #                not in $tmp_location
        #                but in $tmp_path])
        # fi

        break;

      done # foreach path


      # if test "$HAVE_AWS" == "no" ; then
      #   AWS_LOCATION=""
      #   AC_MSG_WARN(AWS not found)
      # fi

    fi # want_aws


    AC_SUBST(HAVE_AWS)
    AC_SUBST(HAVE_AWS_DETAIL)
    AC_SUBST(AWS_VERSION)
    AC_SUBST(AWS_LOCATION)
  
  fi # have java

])

