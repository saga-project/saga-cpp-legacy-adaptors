#! /bin/sh

if [ -f ./env-rns.sh ]; then
    . ./env-rns.sh
fi
if [ ! -d "$GLOBUS_LOCATION" ] ; then
    echo "Error: GLOBUS_LOCATION invalid or not set: $GLOBUS_LOCATION" 1>&2
    exit 1
fi

if [ -z "${JAVA_HOME}" -o -z "${ANT_HOME}" ]; then
    echo Please set JAVA_HOME and ANT_HOME environment variable.
    exit 1
else
    export PATH GLOBUS_LOCATION RNS_HOME JAVA_HOME ANT_HOME
    . ${GLOBUS_LOCATION}/etc/globus-devel-env.sh
fi

if [ X$1 = Xuninstall ]; then
    make clean
else
    make
fi
exit $?

