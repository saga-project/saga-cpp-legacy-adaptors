#! /bin/sh

NAME=org_naregi_rns

COMMANDS="rns-bulk-add rns-add rns-bench rns-getxml rns-mkdir rns-rmdir rns-callerinfo rns-gridftp-del rns-mv rns-setacl rns-chgrp rns-gridftp-get rns-ping rns-setxml rns-chmod rns-gridftp-ln rns-rm rns-stat rns-chown rns-gridftp-put rns-version rns-getacl rns-ls rns-getepr rns-ls-l rns-rmacl rns-xquery lfcj-ls lfcj-rns-migrate lfcj-stat lfcj-test rns-kv-get rns-kv-set rns-kv-rm rns-kv-ls"
OBSOLETE="rns-rm-r rns-rm-f"

if [ -f ./env-rns.sh ]; then
    . ./env-rns.sh
fi
if [ ! -d "$GLOBUS_LOCATION" ] ; then
    echo "Error: invalid GLOBUS_LOCATION: $GLOBUS_LOCATION" 1>&2
    exit 1
fi
if [ -z "${JAVA_HOME}" -o -z "${ANT_HOME}" ]; then
    echo Please set JAVA_HOME and ANT_HOME environment variable. 1>&2
    exit 1
fi

if [ ! -f ./lib/saxon9he.jar ]; then
    echo Please install Saxon-HE Java to lib/saxon9he.jar. 1>&2
    exit 1
fi

retv=0
if [ X$1 = Xuninstall ]; then
    globus-undeploy-gar ${NAME}
    retv=$?
    cd $GLOBUS_LOCATION/bin
    for n in $COMMANDS ; do
        rm -f $n
    done
    for n in $OBSOLETE ; do
        rm -f $n
    done
else
    SAVE_CLASSPATH=$CLASSPATH
    CLASSPATH=
    export CLASSPATH
    if [ -f rns_build.xml ]; then
        ant -f rns_build.xml clean
        ant -f rns_build.xml
    fi
    retv=$?
    if [ $retv -eq 0 ]; then
        globus-undeploy-gar ${NAME}
        globus-deploy-gar ${NAME}.gar
        retv=$?
    fi
    CLASSPATH=$SAVE_CLASSPATH
    export CLASSPATH
fi

exit $retv
