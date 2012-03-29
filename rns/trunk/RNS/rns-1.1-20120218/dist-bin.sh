#! /bin/sh

INCLUDE="org_naregi_rns.gar install.sh uninstall.sh env-rns.sh mygt4exec.sh README sample.rns-client.conf sample.rns-server.conf"

cd `dirname $0`
WORKDIR=`pwd`
NAME=`basename $WORKDIR`
TMPNAME=${NAME}-1.1-bin-`date '+%Y%m%d'`

mkdir $TMPNAME
if [ $? -ne 0 ]; then
    exit 1
fi
for n in $INCLUDE ; do
    cp -P -a $n $TMPNAME
done

FILENAME=${TMPNAME}-`date '+%H%M'`.tar.gz
tar -h --owner=root --group=root -cvzf $FILENAME $TMPNAME

rm -rf $TMPNAME

DIST=`dirname $WORKDIR`
mv $FILENAME $DIST
