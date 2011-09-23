#/bin/bash

# Example usage of the saga command line tools to copy a 
# file from and to GridFTP resources

# remote -> local
$SAGA_LOCATION/bin/saga-file copy gridftp://qb1.loni.org/etc/passwd file://localhost//tmp/

# remote -> remote (3rd party)
$SAGA_LOCATION/bin/saga-file copy gridftp://qb1.loni.org/etc/passed gridft://eric1.loni.org/tmp/

# local -> remote 
$SAGA_LOCATION/bin/saga-file copy file://localhost//tmp/ gridftp://qb1.loni.org/etc/passwd


