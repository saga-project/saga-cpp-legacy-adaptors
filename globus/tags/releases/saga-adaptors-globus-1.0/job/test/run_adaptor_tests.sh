#!/bin/sh
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying 
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Header
while getopts h arg
do
   case $arg in
      h) echo "This program performs some rudimentary adaptor tests."
         exit 0;;
   esac
done

if test "x$REMOTEHOST" = "x"; then
  REMOTEHOST=localhost
fi

STRING=saga_was_here
FILE=/tmp/saga.adaptor.gram.test

echo 'Run a remote echo test and put result in tmp file:'
echo $SAGA_LOCATION/bin/saga-job run gram://$REMOTEHOST/ /bin/sh -c "echo $STRING > $FILE"
$SAGA_LOCATION/bin/saga-job run gram://$REMOTEHOST/ /bin/sh -c "echo $STRING > $FILE"

echo 'Check what the remote temp file contains:'
echo $SAGA_LOCATION/bin/saga-job run gram://$REMOTEHOST/ /bin/cat $FILE
RESULT=`$SAGA_LOCATION/bin/saga-job run gram://$REMOTEHOST/ /bin/cat $FILE`

# Clean up temporaries
$SAGA_LOCATION/bin/saga-job run gram://$REMOTEHOST/ rm -f $FILE

# Verify the cat exited what was expected
if test "$RESULT" = "$STRING"; then
   echo "Success"
   exit 0;
else
   echo "Failed: $RESULT != $STRING"
   exit 1
fi
