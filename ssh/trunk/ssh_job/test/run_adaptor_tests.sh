#!/bin/sh

# Copyright (c) 2009 Chris Miceli (cmicel1@cct.lsu.edu), Ashley Zebrowski (azebrowski@cct.lsu.edu)
# Distributed under the Boost Software License, Version 1.0. (See accompanying 
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Header

# Comments:  Added saga-job submit and state to Chris' saga-job run adaptor,
#            and changed the filename/format to match what Ole's using.  It's
#            tested and works with both localhost and qb.loni.org set as REMOTEHOST.
#            Suspend and resume are not tested for, as they are not implemented for the SSH adaptor.
#            Please let me know if this gives you any problems.  (Ashley, 11/20/09)
while getopts h arg
do
   case $arg in
      h) echo "This program tests the functionality of the ssh job adaptor."
         exit 0;;
   esac
done

if test "x$REMOTEHOST" = "x"; then
  REMOTEHOST=localhost
fi

EXIT_FAILURE=0
SUBMIT_FAILURE=0

# Create a dummy temporary file location
FILE=/tmp/saga.adaptor.ssh.test.$$

# Test out saga-job run


STRING=testing_ssh_job_run
echo "Running: saga-job run ssh://$REMOTEHOST/ echo $STRING > $FILE"
$SAGA_LOCATION/bin/saga-job run ssh://$REMOTEHOST/ "echo $STRING > $FILE"
RESULT=`$SAGA_LOCATION/bin/saga-job run ssh://$REMOTEHOST/ cat $FILE`

if test "$RESULT" != "testing_ssh_job_run"
then
   echo "Failed: $RESULT"
   EXIT_FAILURE=1
fi

# Test saga-job submit
STRING=testing_ssh_job_submit
echo "Running: saga-job submit ssh://$REMOTEHOST echo $STRING > $FILE && sleep 300"
JOB_ID=`$SAGA_LOCATION/bin/saga-job submit ssh://$REMOTEHOST/ "echo $STRING > $FILE && sleep 300"`

sleep 1 # wait a second in case the submit is delayed at all

# Went ahead and used the SSH command as opposed to the saga-file run to keep their testing separate
RESULT=`ssh $REMOTEHOST cat $FILE`

if test "$RESULT" != "testing_ssh_job_submit"
then
   echo "Failed: $RESULT"
   SUBMIT_FAILURE=1
   EXIT_FAILURE=1
fi

# Test saga-job state (relies on saga-job submit succeeding, obviously)

# grab the job id from the previous submit command
JOB_ID=`echo $JOB_ID | sed s/Job\ ID:\ //`

if [ $SUBMIT_FAILURE = 1 ]
then
  echo "Omitting saga-job state testing -- submission failed, so we have no job to check the state of."
  EXIT_FAILURE=1
else
  echo "Running: saga-job state ssh://$REMOTEHOST $JOB_ID"
  RESULT=`$SAGA_LOCATION/bin/saga-job state ssh://$REMOTEHOST $JOB_ID`
  if test "$RESULT" != "$JOB_ID: Running"
  then
    echo "Failed: $RESULT"
    EXIT_FAILURE=1
  fi
fi

echo "Omitting saga-job suspend and saga-job resume as they are not currently supported by the SSH adaptor."

if [ $EXIT_FAILURE = 1 ]
then
   echo "Some tests have failed!"
   exit 1
else
   exit 0
fi
