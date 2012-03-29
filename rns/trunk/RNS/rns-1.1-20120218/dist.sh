#! /bin/sh

EXCLUDE="*~ */*~ *.log org_naregi_rns.gar .svn build bin var tmp lib/saxon9he.jar"
EXEC="install.sh uninstall.sh dist.sh dist-bin.sh mygt4exec.sh command/rns-test-limit command/rns-test-stress command/rns-n-mkdir command/tcpmonitor.sh command/rns-test command/rns-test-loop command/rns-bench rnsfs/rnsfs.sh rnsfs/install.sh"

cd `dirname $0`
WORKDIR=`pwd`
NAME=`basename $WORKDIR`
TMPNAME=${NAME}-1.1-`date '+%Y%m%d'`

ant -f javadoc.xml

cd rnsfs
make clean
cd $WORKDIR

EX=
for e in $EXCLUDE ; do
    EX="--exclude=${e} ${EX}"
done

find . -type f -exec chmod -x {} \;
for e in $EXEC ; do
    chmod +x $e
done

cd `dirname $WORKDIR`
ln -s $NAME $TMPNAME
tar -h --owner=root --group=root $EX -cvzf ${TMPNAME}-`date '+%H%M'`.tar.gz $TMPNAME
rm $TMPNAME
