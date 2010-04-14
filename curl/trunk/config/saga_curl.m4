#
# SYNOPSIS
#
#   AX_SAGA_CHECK_CURL([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the CURL libraries of a particular version (or newer)
#
#   If no path to the installed curl library is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/curl-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_CURL)
#     AC_SUBST(CURL_LOCATION)
#     AC_SUBST(CURL_CPPFLAGS) 
#     AC_SUBST(CURL_LDFLAGS)
#     AC_SUBST(CURL_S_LIBS)
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

AC_DEFUN([AX_SAGA_CHECK_CURL],
[
  AC_ARG_VAR([CURL_LOCATION],[CURL installation directory])

  HAVE_CURL=no

  tmp_location=""
  AC_ARG_WITH([curl],
              AS_HELP_STRING([--with-curl=DIR],
              [use curl (default is YES) at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                want_curl="no"
              elif test "$withval" = "yes"; then
                want_curl="yes"
                tmp_location=""
              else
                want_curl="yes"
                tmp_location="$withval"
              fi
              ],
              [want_curl="yes"])

  # use CURL_LOCATION if avaialble, and if not 
  # overwritten by --with-curl=<dir>

  if test "x$want_curl" = "xyes"; then
    
    packages=`ls /usr/local/package/curl-* 2>>/dev/null`

    if test "$tmp_location-$CURL_LOCATION" = "-"; then
      paths="/usr /usr/local /opt /sw $packages"
    else
      paths="$tmp_location $CURL_LOCATION"
    fi
    
    for tmp_path in $paths; do
      
      AC_MSG_CHECKING(for curl in $tmp_path)

      have_something=`ls $tmp_path/lib/libcurl* 2>/dev/null`

      if test "x$have_something" = "x"; then
        AC_MSG_RESULT(no)
        continue
      fi

      saved_cppflags=$CPPFLAGS
      saved_ldflags=$LDFLAGS

      CURL_PATH=$tmp_path
      CURL_LDFLAGS="-L$tmp_path/lib/ -lcurl"
      CURL_CPPFLAGS="-I$tmp_path/include/"

      if test -e "$tmp_path/lib/libcurl.a"; then
        CURL_S_LIBS="$tmp_path/lib/libcurl.a"
      fi

      CPPFLAGS="$CPPFLAGS $CURL_CPPFLAGS"
      export CPPFLAGS

      LDFLAGS="$LDFLAGS $CURL_LDFLAGS"
      export LDFLAGS

      AC_LINK_IFELSE([AC_LANG_PROGRAM([[@%:@include <curl/curl.h>]],
                                      [[
                                        curl_easy_init ();
                                        return (0);
                                      ]])],
                                      link_curl="yes",
                                      link_curl="no")
            
      if test "x$link_curl" = "xno"; then
      
        AC_MSG_RESULT(no)

        LDFLAGS=$saved_ldflags
        CPPFLAGS=$saved_cppflags
        CURL_LOCATION=""
      
      else

        AC_MSG_RESULT(yes)

        CURL_LOCATION=$tmp_path
        HAVE_CURL=yes

        export HAVE_CURL

        AC_SUBST(HAVE_CURL)
        AC_SUBST(CURL_LOCATION)
        AC_SUBST(CURL_CPPFLAGS)
        AC_SUBST(CURL_LDFLAGS)
        AC_SUBST(CURL_S_LIBS)
        
        #if test "$tmp_location" != "" && \
        #   test "$tmp_location" != "$tmp_path" ; then
        #  AC_MSG_WARN([CURL found:
        #               not in $tmp_location
        #               but in $tmp_path])
        #fi

        break;

      fi # link ok

    done # foreach path


    #if test "$HAVE_CURL" == "no" ; then
    #  CURL_LOCATION=""
    #  AC_MSG_WARN(CURL not found)
    #fi

  fi # want_curl

])

