#!/bin/sh

RNSCMD_PKG="org.naregi.rns.command."
RNSTEST_PKG="org.naregi.rns.test."
HEAPSIZE=256

if [ x"$RNS_HOME" = x ]; then
  RNS_HOME=.
fi
if [ x"$GLOBUS_LOCATION" != x ]; then
  . $GLOBUS_LOCATION/etc/globus-devel-env.sh
fi
if [ x"$GLOBUS_OPTIONS" = x ]; then
  GLOBUS_OPTIONS="-Xmx${HEAPSIZE}m"
fi

COMMAND_NAME=`basename $0`
CLASSPATH="${RNS_HOME}/bin:${RNS_HOME}/build/stubs/classes/:$CLASSPATH"
JAVAOPT="$GLOBUS_OPTIONS -Drns.command.name=$COMMAND_NAME -Djava.security.egd=file:///dev/urandom -classpath $CLASSPATH"

if [ x$RNS_CLIENT_CONFIG != x ]; then
  JAVAOPT="-Drns.config=$RNS_CLIENT_CONFIG $JAVAOPT"
fi
