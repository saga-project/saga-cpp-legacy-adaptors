#!/bin/sh

. $GLOBUS_LOCATION/etc/globus-devel-env.sh
LANG=C java org.apache.axis.utils.tcpmon 8081 localhost 8080 
