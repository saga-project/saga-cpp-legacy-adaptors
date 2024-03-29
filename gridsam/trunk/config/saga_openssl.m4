#
# SYNOPSIS
#
#   AX_SAGA_CHECK_OPENSSL([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the OPENSSL libraries of a particular version (or newer)
#
#   If no path to the installed openssl library is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/openssl-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_OPENSSL)
#     AC_SUBST(OPENSSL_LOCATION)
#     AC_SUBST(OPENSSL_CPPFLAGS) 
#     AC_SUBST(OPENSSL_LDFLAGS)
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

AC_DEFUN([AX_SAGA_CHECK_OPENSSL],
[
  AC_ARG_VAR([OPENSSL_LOCATION],[OPENSSL installation directory])

  HAVE_OPENSSL=no

  tmp_location=""
  AC_ARG_WITH([openssl],
              AS_HELP_STRING([--with-openssl=DIR],
              [use openssl (default is YES) at DIR (optional)]),
              [
              if test "$withval" = "no"; then
                want_openssl="no"
              elif test "$withval" = "yes"; then
                want_openssl="yes"
                tmp_location=""
              else
                want_openssl="yes"
                tmp_location="$withval"
              fi
              ],
              [want_openssl="yes"])

  # use OPENSSL_LOCATION if avaialble, and if not 
  # overwritten by --with-openssl=<dir>

  if test "x$want_openssl" = "xyes"; then
    
    packages=`ls /usr/local/package/openssl-* 2>>/dev/null`
    
    if test "$tmp_location-$OPENSSL_LOCATION" = "-"; then
      paths="/usr /usr/local /opt /sw $packages"
    else
      paths="$tmp_location $OPENSSL_LOCATION"
    fi

    for tmp_path in $paths; do
      
      AC_MSG_CHECKING(for openssl in $tmp_path)

      saved_cppflags=$CPPFLAGS
      saved_ldflags=$LDFLAGS

      OPENSSL_PATH=$tmp_path
      OPENSSL_LDFLAGS="-L$tmp_path/lib/ -lssl -lcrypto"
      OPENSSL_CPPFLAGS="-I$tmp_path/include/"

      CPPFLAGS="$CPPFLAGS $OPENSSL_CPPFLAGS"
      export CPPFLAGS

      LDFLAGS="$LDFLAGS $OPENSSL_LDFLAGS"
      export LDFLAGS

      AC_LINK_IFELSE([AC_LANG_PROGRAM([[@%:@include <openssl/ssl.h>]],
                                      [[
                                        SSL_library_init ();
                                        return (0);
                                      ]])],
                                      link_openssl="yes",
                                      link_openssl="no")
            
      if test "x$link_openssl" = "xno"; then
      
        AC_MSG_RESULT(no)

        LDFLAGS=$saved_ldflags
        CPPFLAGS=$saved_cppflags
        OPENSSL_LOCATION=""
      
      else

        AC_MSG_RESULT(yes)

        OPENSSL_LOCATION=$tmp_path
        HAVE_OPENSSL=yes

        export HAVE_OPENSSL

        AC_SUBST(HAVE_OPENSSL)
        AC_SUBST(OPENSSL_LOCATION)
        AC_SUBST(OPENSSL_CPPFLAGS)
        AC_SUBST(OPENSSL_LDFLAGS)
        
        #if test "$tmp_location" != "" && \
        #   test "$tmp_location" != "$tmp_path" ; then
        #  AC_MSG_WARN([OPENSSL found:
        #               not in $tmp_location
        #               but in $tmp_path])
        #fi

        break;

      fi # link ok

    done # foreach path


    #if test "$HAVE_OPENSSL" == "no" ; then
    #  OPENSSL_LOCATION=""
    #  AC_MSG_WARN(OPENSSL not found)
    #fi

  fi # want_openssl

])

