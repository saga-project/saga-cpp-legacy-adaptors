
###########################################################
# 
# @file: globus.m4
# 
# @description: 
#   Autoconf macros for detecting Globus settings
#
# Contributed by Andre Merzky   <merzky@cs.vu.nl>.
# 
###########################################################

### LICENSE ###

###########################################################
#
# AC Macro for determining the GLOBUS_LOCATION
#
AC_DEFUN([AX_SAGA_GLOBUS],
[

  FAILED=0;

  GLOBUS_SUBSYSTEM=$1

  AC_MSG_CHECKING([for globus-subsystem])

  if test "x$GLOBUS_SUBSYSTEM" != "x" ; then
    AC_MSG_RESULT([ using $GLOBUS_SUBSYSTEM])
  else
    AC_MSG_RESULT([ using none])
  fi
  

  SAGA_AC_GLOBUS_LOC

  if test "x$HAVE_GLOBUS_LOCATION" = "xno"; then
    FAILED=1
  fi


  if test "$FAILED" -eq "0"; then

    if test ! -x "$GLOBUS_LOCATION/bin/globus-makefile-header"; then
      AC_MSG_WARN([globus-makefile-header not in $GLOBUS_LOCATION])
      FAILED=1
    fi

  fi # ! FAILED


  if test "$FAILED" -eq "0"; then

    globus_query_flags="--flavor=$GLOBUS_FLAVOR globus_gram_client globus_gass_copy globus_gass_server_ez globus_rls_client globus_ftp_client"
    globus_query_cmd="$GLOBUS_LOCATION/bin/globus-makefile-header"

    if ! test -x "$globus_query_cmd" ; then
      AC_MSG_WARN([Cannot run globus_query_cmd: $globus_query_cmd])
      FAILED=1
    fi

  fi # ! FAILED


  if test "$FAILED" -eq "0"; then

    AC_MSG_NOTICE([running $globus_query_cmd $globus_query_flags])
    eval "$globus_query_cmd $globus_query_flags > globus_defines"

    if test $? != 0; then
      AC_MSG_WARN([error running globus-makefile-header ($globus_query_cmd $globus_query_flags > globus_defines), 
                    maybe invalid GLOBUS_FLAVOR $MY_GLOBUS_FLAVOR, 
                    use --with-globus-flavor=<flavor>])
    fi
	  
  fi # ! FAILED


  if test "$FAILED" -eq "0"; then

    GLOBUS_CFLAGS=`grep GLOBUS_CFLAGS   globus_defines | sed -e 's/^GLOBUS_CFLAGS *= *//'`
    GLOBUS_LDFLAGS=`grep GLOBUS_LDFLAGS  globus_defines | sed -e 's/^GLOBUS_LDFLAGS *= *//'`
    GLOBUS_INCLUDES=`grep GLOBUS_INCLUDES globus_defines | sed -e 's/^GLOBUS_INCLUDES *= *//'`
    GLOBUS_LIBS=`grep GLOBUS_PKG_LIBS globus_defines | sed -e 's/^GLOBUS_PKG_LIBS *= *//'`


    globus_version_cmd=$GLOBUS_LOCATION/bin/globus-version
    
    if ! test -x $globus_version_cmd ; then
      AC_MSG_WARN([Cannot run globus_version_cmd: $globus_version_cmd])
      GLOBUS_VERSION=unknown
    else
    
      # check for version
      AC_MSG_NOTICE([running $globus_version_cmd])
    
      GLOBUS_VERSION=`$globus_version_cmd`
    
      if test $? != 0; then
        AC_MSG_WARN([error running globus-version ($globus_version_cmd), 
                      maybe invalid GLOBUS_FLAVOR $MY_GLOBUS_FLAVOR, 
                      use --with-globus-flavor=<flavor>])
      fi
    
    fi  # have globus-version

  fi # ! FAILED


  if test "$FAILED" -eq "0"; then

    ##### Check for individual packages - GRAM
    #
    GLOBUS_HAVE_GRAM=`grep GLOBUS_PKG_LIBS globus_defines | grep globus_gram_client`
    if test -z "$GLOBUS_HAVE_GRAM" ; then
      GLOBUS_HAVE_GRAM="no"
    else
    	GLOBUS_HAVE_GRAM="yes"	
    	GLOBUS_GRAM_CLIENT_VERSION=`grep GLOBUS_GRAM_CLIENT_VERSION globus_defines | sed -e 's/^GLOBUS_GRAM_CLIENT_VERSION *= *//'`
      AC_DEFINE([SAGA_HAVE_GLOBUS_GRAM], [1])
    fi

    ##### Check for individual packages - GridFTP
    #
    GLOBUS_HAVE_GRIDFTP=`grep GLOBUS_PKG_LIBS globus_defines | grep globus_ftp_client`
    if test -z "$GLOBUS_HAVE_GRIDFTP" ; then
      GLOBUS_HAVE_GRIDFTP="no"
    else
    	GLOBUS_HAVE_GRIDFTP="yes"	
    	GLOBUS_GRIDFTP_CLIENT_VERSION=`grep GLOBUS_FTP_CLIENT_VERSION globus_defines | sed -e 's/^GLOBUS_FTP_CLIENT_VERSION *= *//'`
      AC_DEFINE([SAGA_HAVE_GLOBUS_GRIDFTP], [1])
    fi

    ##### Check for individual packages - RLS
    #
    GLOBUS_RLS_CLIENT_VERSION=`grep GLOBUS_RLS_CLIENT_VERSION globus_defines | sed -e 's/^GLOBUS_RLS_CLIENT_VERSION *= *//'`
    if test -z "$GLOBUS_RLS_CLIENT_VERSION" ; then
      GLOBUS_HAVE_RLS="no"
    else
    	GLOBUS_HAVE_RLS="yes"	
    	#GLOBUS_RLS_CLIENT_VERSION=`grep GLOBUS_RLS_CLIENT_VERSION globus_defines | sed -e 's/^GLOBUS_RLS_CLIENT_VERSION *= *//'`
      AC_DEFINE([SAGA_HAVE_GLOBUS_RLS], [1])
    fi

    ##### Check for individual packages - GSI
    #
    GLOBUS_GSI_CLIENT_VERSION=`grep GLOBUS_GSSAPI_GSI_VERSION globus_defines | sed -e 's/^GLOBUS_GSSAPI_GSI_VERSION *= *//'`
    if test -z "$GLOBUS_GSI_CLIENT_VERSION" ; then
      GLOBUS_HAVE_GSI="no"
    else
      GLOBUS_HAVE_GSI="yes"
      #GLOBUS_RLS_CLIENT_VERSION=`grep GLOBUS_RLS_CLIENT_VERSION globus_defines | sed -e 's/^GLOBUS_RLS_CLIENT_VERSION *= *//'`
      AC_DEFINE([SAGA_HAVE_GLOBUS_GSI], [1])
    fi


    ##### Check for individual packages - GASS Copy
    #
    GLOBUS_HAVE_GASS_COPY=`grep GLOBUS_PKG_LIBS globus_defines | grep globus_gass_copy`
    if test -z "$GLOBUS_HAVE_GASS_COPY" ; then
      GLOBUS_HAVE_GASS_COPY="no"
    else
    	GLOBUS_HAVE_GASS_COPY="yes"	
      GLOBUS_GASS_COPY_CLIENT_VERSION=`grep GLOBUS_GASS_COPY_VERSION globus_defines | sed -e 's/^GLOBUS_GASS_COPY_VERSION *= *//'`
      AC_DEFINE([SAGA_HAVE_GLOBUS_GASS], [1])
    fi

    eval "rm globus_defines"

  fi # ! FAILED


  if test "$FAILED" -eq "0"; then
    HAVE_GLOBUS=yes
  else
    HAVE_GLOBUS=no
  fi

#  AC_MSG_NOTICE([ ================================================ ])
#  AC_MSG_NOTICE([ HAVE_GLOBUS:           $HAVE_GLOBUS              ])
#  AC_MSG_NOTICE([ GLOBUS_LDFLAGS:        $GLOBUS_LDFLAGS           ])
#  AC_MSG_NOTICE([ GLOBUS_INCLUDES:       $GLOBUS_INCLUDES          ])
#  AC_MSG_NOTICE([ GLOBUS_LIBS:           $GLOBUS_LIBS              ])
#  AC_MSG_NOTICE([ GLOBUS_HAVE_GRAM:      $GLOBUS_HAVE_GRAM         ])
#  AC_MSG_NOTICE([ GLOBUS_HAVE_GRIDFTP:   $GLOBUS_HAVE_GRIDFTP      ])
#  AC_MSG_NOTICE([ GLOBUS_HAVE_GASS_COPY: $GLOBUS_HAVE_GASS_COPY    ])
#  AC_MSG_NOTICE([ GLOBUS_HAVE_RLS:       $GLOBUS_HAVE_RLS          ])
#  AC_MSG_NOTICE([ ================================================ ])
#
  AC_SUBST(HAVE_GLOBUS)
  AC_SUBST(GLOBUS_HAVE_GRAM)
  AC_SUBST(GLOBUS_HAVE_GRIDFTP)
  AC_SUBST(GLOBUS_HAVE_GASS_COPY)
  AC_SUBST(GLOBUS_HAVE_RLS)
  AC_SUBST(GLOBUS_CFLAGS)
  AC_SUBST(GLOBUS_LDFLAGS)
  AC_SUBST(GLOBUS_INCLUDES)
  AC_SUBST(GLOBUS_LIBS)
])
#
###########################################################

###########################################################
#
# AC Macro for determining the GLOBUS_LOCATION and 
# GlOBUS_FLAVOR
# 
# Contributed by Konrad Karczewski <xeno@icis.pcz.pl>
#
AC_DEFUN([SAGA_AC_GLOBUS_LOC],
[
  AC_MSG_CHECKING([for globus-location directory])
  AC_ARG_VAR([GLOBUS_LOCATION],[Globus installation directory])

  AC_ARG_WITH([globus-location], 
              [AC_HELP_STRING([--with-globus-location=<dir>], 
                              [use globus ......... at <dir> (default=auto)])],
                              [GLOBUS_LOCATION=$withval],
                              [GLOBUS_LOCATION=$GLOBUS_LOCATION])

  FAILED=0
  
  if test -d "$GLOBUS_LOCATION" ; then
    GLOBUS_LOCATION=$GLOBUS_LOCATION
    AC_MSG_RESULT([ found $GLOBUS_LOCATION])
  else
    AC_MSG_WARN([ no globus at $GLOBUS_LOCATION])
    FAILED=1
  fi

    
  # GPT_LOCATION 
  if test "$FAILED" -eq "0"; then
    
    AC_MSG_CHECKING([for gpt-location directory])
    AC_ARG_VAR([GPT_LOCATION],[GPT installation directory])
    AC_ARG_WITH([gpt-location], 
                [AC_HELP_STRING([--with-gpt-location=<dir>], 
                                [use gpt ......... at <dir> (default=auto)])],
                                [GPT_LOCATION=$withval],
                                [GPT_LOCATION=$GPT_LOCATION])
    
    if test "x$GPT_LOCATION" = "x" ; then
      GPT_LOCATION=${GLOBUS_LOCATION}
    fi
    
    if test -d "$GPT_LOCATION" ; then
      GPT_LOCATION=$GPT_LOCATION
      AC_MSG_RESULT([ found $GPT_LOCATION])
    else
      AC_MSG_WARN([ no such directory $GPT_LOCATION])
      FAILED=1
    fi
  
  fi # ! FAILED


  # Globus flavors
  if test "$FAILED" -eq "0"; then
    
    AC_ARG_VAR([GLOBUS_FLAVOR],[Globus installation flavor])
    
    AC_MSG_CHECKING([for globus flavor])
    
    AC_ARG_WITH([globus-flavor], 
                [AC_HELP_STRING([--with-globus-flavor=<flavor>], 
                                [use globus flavor .. <flavor> (default=auto)])],
                                [GLOBUS_FLAVOR=$withval],
                                [GLOBUS_FLAVOR=$GLOBUS_FLAVOR])
    
    
    if test "x$GLOBUS_FLAVOR" = "xyes" ; then
      GLOBUS_FLAVOR=""
    fi
    
    if test "x$GLOBUS_FLAVOR" = "xno" ; then
      GLOBUS_FLAVOR=""
    fi
    
    if test "x$GLOBUS_FLAVOR" != "x" ; then
      
      AC_MSG_RESULT([ $GLOBUS_FLAVOR])

    else

      if test -x ${GPT_LOCATION}/sbin/gpt-query ; then
        
        eval "${GPT_LOCATION}/sbin/gpt-query globus_core \
                   | grep globus_core \
                   | grep -v noflavor \
                   | sed -e 's/^.*globus_core-//;s/-.*//' \
                   | tr ' ' '\n' \
                   | sort \
                   | uniq \
                   | grep -v -e '^ *$' \
                   > globus_defines_2"

        TMP_GLOBUS_FLAVOR=`cat globus_defines_2 \
                   | grep -v -e "^ *$" \
                   | head -1`

        TMP_GLOBUS_FLAVOR_THREADS=`cat globus_defines_2 \
                   | grep -v -e "^ *$" \
                   | grep    pthr \
                   | head -1`

        # prefer threaded flavors
        if test "x$TMP_GLOBUS_FLAVOR_THREADS" != "x"; then
          GLOBUS_FLAVOR=$TMP_GLOBUS_FLAVOR_THREADS
        else
          GLOBUS_FLAVOR=$TMP_GLOBUS_FLAVOR
        fi

        eval "rm globus_defines_2"

        # AC_MSG_NOTICE([FLAVOR  : $GLOBUS_FLAVOR])

      fi


      if test "x${GLOBUS_FLAVOR}" != "x" ; then

        AC_MSG_RESULT([ $GLOBUS_FLAVOR])

      else

        AC_MSG_RESULT([- not found])
        AC_MSG_WARN([please provide proper globus flavor names])
        FAILED=1

      fi

    fi 

  fi # ! FAILED

      
  if test "$FAILED" -eq "0"; then


    # Globus includes
    AC_MSG_CHECKING([for globus-include])
    GLOBUS_INCLUDE="${GLOBUS_LOCATION}/include/${GLOBUS_FLAVOR}"

    if test ! -d "$GLOBUS_INCLUDE" ; then
      AC_MSG_WARN([ no such directory $GLOBUS_INCLUDE])
    else
      AC_MSG_RESULT([ $GLOBUS_INCLUDE])
    fi

    if test ! -d "GLOBUS_INCLUDE" ; then
      AC_MSG_CHECKING([for globus-include])
      GLOBUS_INCLUDE="${GLOBUS_LOCATION}/include/"

      if test ! -d "$GLOBUS_INCLUDE" ; then
        AC_MSG_WARN([ no such directory $GLOBUS_INCLUDE])
      else
        AC_MSG_RESULT([ $GLOBUS_INCLUDE])
      fi
    fi


  fi  # ! FAILED


  if test "$FAILED" -eq "0"; then
    HAVE_GLOBUS_LOCATION=yes
  else
    HAVE_GLOBUS_LOCATION=no
  fi

  AC_SUBST(HAVE_GLOBUS_LOCATION)

  AC_SUBST(GPT_LOCATION)
  AC_SUBST(GLOBUS_LOCATION)
  AC_SUBST(GLOBUS_INCLUDE)
  AC_SUBST(GLOBUS_FLAVOR)

#  AC_MSG_NOTICE([ ================================================ ])
#  AC_MSG_NOTICE([ GLOBUS_LOCATION:       $GLOBUS_LOCATION          ])
#  AC_MSG_NOTICE([ GLOBUS_FLAVOR:         $GLOBUS_FLAVOR            ])
#  AC_MSG_NOTICE([ =============================================== ])
])



