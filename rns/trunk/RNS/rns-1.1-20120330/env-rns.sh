# required
GLOBUS_LOCATION=/RNS/gt4
RNS_HOME=/RNS/rns

# optional (Please comment out followings if you set JAVA_HOME or ANT_HOME)
MY_JAVA_HOME=/RNS/java
MY_ANT_HOME=/RNS/ant


######################################################################
PATH=${RNS_HOME}/rnsfs:${GLOBUS_LOCATION}/bin:${GLOBUS_LOCATION}/sbin:${PATH}

if [ -n "${MY_JAVA_HOME}" ]; then
    PATH=${MY_JAVA_HOME}/bin:${PATH}
    JAVA_HOME=${MY_JAVA_HOME}
    export JAVA_HOME
fi
if [ -n "${MY_ANT_HOME}" ]; then
    PATH=${MY_ANT_HOME}/bin:${PATH}
    ANT_HOME=${MY_ANT_HOME}
    export ANT_HOME
fi

export GLOBUS_LOCATION PATH
