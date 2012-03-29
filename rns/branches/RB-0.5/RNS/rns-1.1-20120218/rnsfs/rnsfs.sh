#!/bin/sh

HEAPSIZE=512
#LOG_LEVEL=WARN

######
if [ $# -lt 1 ]; then
    echo "Usage: $0 /mount/point [ FUSE options ([ -f | -d ] [ -s ]) ]"
    exit 1
fi

RNSFS_HOME=`dirname $0`
if [ -f $RNSFS_HOME/build.conf ]; then
    . $RNSFS_HOME/build.conf
fi
if [ -z $FUSE4J_HOME ]; then
    echo "need FUSE4J_HOME environment variable"
    exit 2
fi

if [ -z "$JAVA_HOME" -o -z "$GLOBUS_LOCATION" -o -z "$CLASSPATH" ]; then
    cd $RNSFS_HOME
    RNSFS_HOME=`pwd`
    RNS_HOME=`dirname $RNSFS_HOME`
    . $RNS_HOME/env-rns.sh
    . ${GLOBUS_LOCATION}/etc/globus-devel-env.sh
fi

JRESERVERDIR=${JAVA_HOME}/jre/lib/amd64/server
if [ ! -d $JRESERVERDIR ]; then
    JRESERVERDIR=${JAVA_HOME}/jre/lib/i386/server
fi

RNSFS_JAR=$RNSFS_HOME/rnsfs.jar
JAVAFS=$FUSE4J_HOME/native/javafs
FS_CLASS=org/naregi/rns/client/fuse/RNSFS_FUSE

CLASSPATH="$CLASSPATH:$RNSFS_JAR:$FUSE4J_JAR"

#LD_LIBRARY_PATH=$FUSE_HOME/lib:$JRESERVERDIR
LD_LIBRARY_PATH=$JRESERVERDIR
export LD_LIBRARY_PATH

### old fuse4j
#JAVA_OPTS="-J-Xmx${HEAPSIZE}m -J-Djava.class.path=$CLASSPATH -J-Drns.command.name=rnsfs -J-Djava.security.egd=file:///dev/urandom"
#$JAVAFS -C${FS_CLASS} $JAVA_OPTS $* -o direct_io,use_ino -s

$JAVAFS $* -o class=${FS_CLASS} -o "jvm=-Xmx${HEAPSIZE}m" -o "jvm=-Djava.class.path=$CLASSPATH" -o "jvm=-Drns.command.name=rnsfs" -o "jvm=-Djava.security.egd=file:///dev/urandom" -o direct_io,use_ino -s

