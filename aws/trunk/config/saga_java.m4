
#
# SYNOPSIS
#
#   AX_SAGA_CHECK_JAVA([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the JAVA interpreter
#
#   If no path to the installed java interpreter is given,
#   the macro searchs under /usr, /usr/java /usr/local, 
#   /usr/local/java, /opt, /opt/java and /usr/local/package/java-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_JAVA)
#     AC_SUBST(JAVA_VERSION)
#     AC_SUBST(JAVA_LOCATION)
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

AC_DEFUN([AX_SAGA_CHECK_JAVA],
[
  AC_ARG_VAR([JAVA_LOCATION],[JAVA installation root (JAVA_HOME)])

  HAVE_JAVA=no

  tmp_location=""
  AC_ARG_WITH([java],
              AS_HELP_STRING([--with-java=DIR],
              [use java (default is YES) at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                want_java="no"
              elif test "$withval" = "yes"; then
                want_java="yes"
                tmp_location=""
              else
                want_java="yes"
                tmp_location="$withval"
              fi
              ],
              [want_java="yes"])

  # use JAVA_LOCATION if avaialble, and if not 
  # overwritten by --with-awjava

  if test "x$want_java" = "xyes"; then
    
    packages=`ls /usr/local/package/java-* 2>>/dev/null`
    
    for tmp_path in $tmp_location $JAVA_HOME /usr /usr/java /usr/local /usr/local/java /opt /opt/java $packages; do
      
      AC_MSG_CHECKING(for java in $tmp_path)

      if ! test -x "$tmp_path/bin/java"; then
        AC_MSG_RESULT(no)
        continue
      fi

      JAVA_HOME="$tmp_path"
      export JAVA_HOME

      JAVA_VERSION=`$tmp_path/bin/java -version 2>&1 | head -1 | cut -f 2 -d '"'`

      if test "x$JAVA_VERSION" = "x"; then
        AC_MSG_RESULT(no)
        continue
      fi

      AC_MSG_RESULT(yes)

      JAVA_LOCATION=$tmp_path
      HAVE_JAVA=yes

      export HAVE_JAVA

      AC_SUBST(HAVE_JAVA)
      AC_SUBST(JAVA_VERSION)
      AC_SUBST(JAVA_LOCATION)
        
      # if test "$tmp_location" != "" && \
      #    test "$tmp_location" != "$tmp_path" ; then
      #   AC_MSG_WARN([JAVA found:
      #                not in $tmp_location
      #                but in $tmp_path])
      # fi

      break;

    done # foreach path


    # if test "$HAVE_JAVA" == "no" ; then
    #   JAVA_LOCATION=""
    #   AC_MSG_WARN(JAVA not found)
    # fi

  fi # want_java

])

