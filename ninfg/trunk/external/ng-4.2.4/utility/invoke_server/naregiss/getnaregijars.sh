#! /bin/sh

NAREGI_DIR=/usr/naregi
FIND=/usr/bin/find
CLASSPATH=

append_classpath () {
    if test -z "${CLASSPATH}";then
        CLASSPATH=$1
    else
        CLASSPATH=${CLASSPATH}:$1
    fi
}

NAREGI_JARS_DIR="${NAREGI_DIR}/lib/jars"
for i in `${FIND} "${NAREGI_JARS_DIR}" -name '*.jar'`
do
    append_classpath "$i"
done
echo $CLASSPATH
