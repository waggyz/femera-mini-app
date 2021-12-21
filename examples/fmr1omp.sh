#!/bin/bash
#
# fmr1node build/femera/mini
#
NMPI=`tools/cpucount.sh`
NOMP=1
#
EXEC=$1
shift
mpiexec -n $NMPI --bind-to core -map-by node:pe=$NOMP $EXEC -n$NOMP $@
exit $?