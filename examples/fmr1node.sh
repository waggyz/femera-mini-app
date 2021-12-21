#!/bin/bash
#
# fmr1node build/femera/mini
#
NMPI=2
NCPU=`tools/cpucount.sh`
if [ $NCPU -lt 4 ]; then
  NMPI=1
fi
if [ $NCPU -ge 8 ]; then
  NMPI=4
fi
NOMP=$(( NCPU / NMPI ))
if [ $NOMP -lt 1 ]; then
  NOMP=1
fi
#
EXEC=$1
shift
mpiexec -n $NMPI --bind-to core -map-by node:pe=$NOMP $EXEC -n$NOMP $@
exit $?