#
# SYNOPSIS
#
#   AX_SAGA_CHECK_SOCI([MINIMUM-VERSION])
#
# DESCRIPTION
#
#   Test for the SOCI libraries of a particular version (or newer)
#
#   If no path to the installed soci library is given,
#   the macro searchs under /usr, /usr/local, /opt and
#   /usr/local/package/soci-*
#
#   This macro calls:
#
#     AC_SUBST(HAVE_SOCI)
#     AC_SUBST(SOCI_LOCATION)
#     AC_SUBST(SOCI_CPPFLAGS) 
#     AC_SUBST(SOCI_LDFLAGS)
#     AC_SUBST(SOCI_S_LIBS)
#
# LAST MODIFICATION
#
#   2007-03-04
#   2007-11-06  always use internal soci!
#
# COPYLEFT
#
#   Copyright (c) 2007 Andre Merzky      <andre@merzky.net>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.

AC_DEFUN([AX_SAGA_CHECK_SOCI],
[
  #AC_MSG_NOTICE(SOCI will be build internally)
  HAVE_SOCI=no
  export HAVE_SOCI
  HAVE_INTERNAL_SOCI=yes
  export HAVE_INTERNAL_SOCI

  # internal backends
  AC_MSG_CHECKING(for backend sqlite3)

  if test "x$HAVE_SQLITE3" = "xyes"; then
    AC_MSG_RESULT(found)
    soci_backends_ok="$soci_backends_ok-sqlite3-"
    SOCI_BACKENDS="$SOCI_BACKENDS sqlite3"
    HAVE_SOCI_SQLITE3=yes
  else
    AC_MSG_RESULT(not found)
  fi
  
  AC_MSG_CHECKING(for backend postgresql)

  if test "x$HAVE_POSTGRESQL" = "xyes"; then
    AC_MSG_RESULT(found)
    soci_backends_ok="$soci_backends_ok-postgresql-"
    SOCI_BACKENDS="$SOCI_BACKENDS postgresql"
    HAVE_SOCI_POSTGRESQL=yes
  else
    AC_MSG_RESULT(not found)
  fi

  if test "x$soci_backends_ok" == "x"; then
    AC_MSG_WARN(Cannot build any of the internal backends!)
    HAVE_INTERNAL_SOCI=no
    HAVE_SOCI=no
  fi

  AC_SUBST(HAVE_SOCI)
  AC_SUBST(HAVE_INTERNAL_SOCI)
  AC_SUBST(HAVE_SOCI_SQLITE3)
  AC_SUBST(HAVE_SOCI_POSTGRESQL)
  AC_SUBST(HAVE_SOCI_MYSQL)
  AC_SUBST(HAVE_SOCI_ODBC)
  AC_SUBST(HAVE_SOCI_ORACLE)
  AC_SUBST(HAVE_SOCI_FIREBIRD)
  AC_SUBST(SOCI_BACKENDS)
  AC_SUBST(SOCI_CPPFLAGS)
  AC_SUBST(SOCI_LDFLAGS)
  AC_SUBST(SOCI_S_LIBS)
  AC_SUBST(SOCI_LOCATION)
  
])


