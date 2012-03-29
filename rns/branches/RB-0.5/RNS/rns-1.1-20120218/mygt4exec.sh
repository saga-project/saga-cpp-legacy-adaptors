#! /bin/sh

RNS_HOME=`dirname $0`

####################################
CLASSPATH=""
. $RNS_HOME/env-rns.sh

$@
