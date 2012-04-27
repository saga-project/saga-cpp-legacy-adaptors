#!/bin/sh

LOG_LEVEL=WARN

######
if [ $# -lt 1 ]; then
    echo "Usage: $0 /mount/point [ FUSE options ([ -f | -d ] [ -s ]) ]"
    exit 1
fi

RNSFS_HOME=`dirname $0`
if [ -f $RNSFS_HOME/build.conf ]; then
    . $RNSFS_HOME/build.conf
fi
if [ -z $FUSE_J_HOME ]; then
    echo "need FUSE_J_HOME environment variable"
    exit 2
fi

. $FUSE_J_HOME/build.conf

if [ -z $RNS_HOME ]; then
    cd $RNSFS_HOME
    RNSFS_HOME=`pwd`
    RNS_HOME=`dirname $RNSFS_HOME`
    . $RNS_HOME/env-rns.sh
fi

RNSFSJAR=$RNSFS_HOME/rnsfs.jar

LD_LIBRARY_PATH=$FUSE_J_HOME/jni:$FUSE_HOME/lib

export LD_LIBRARY_PATH

JAVA_OPTS="-XX:MaxPermSize=512m -Drns.command.name=rnsfs -Djava.security.egd=file:///dev/urandom"

$JDK_HOME/bin/java $JAVA_OPTS \
    -classpath $CLASSPATH:$RNSFSJAR:$FUSE_J_HOME/build:$FUSE_J_HOME/lib/commons-logging-1.0.4.jar \
    -Dfuse.logging.level=$LOG_LEVEL \
    -Dorg.apache.commons.logging.Log=fuse.logging.FuseLog \
    org.naregi.rns.client.fuse.RNSFilesystem \
    $* -o direct_io
