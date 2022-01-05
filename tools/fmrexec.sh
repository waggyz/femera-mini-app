#!/bin/bash
#
# fmrexec -n 2 --bind-to core -map-by node:pe=2 build/femera/mini -n2
#
if [ 1 -eq 1 ]; then
  mpiexec -n 2 --bind-to core -map-by node:pe=2 "$@" -n2
  exit $?
else
  # TODO Make this work
  if [ -z "$OMP_NUM_THREADS" ]; then
  echo "CL: $@"
    N=`echo "$@" | grep -ioP '(?<=PE\=)[0-9]+'`
    if [ ! -z "$N" ]; then
      export OMP_NUM_THREADS=$N
    fi
  fi
  if [ -z "$OMP_NUM_THREADS" ]; then
    export OMP_NUM_THREADS=1
  fi
  echo OMP_NUM_THREADS $OMP_NUM_THREADS
  export OMP_PROC_BIND=close
  export OMP_PLACES=cores
  mpiexec -x OMP_NUM_THREADS=$OMP_NUM_THREADS \
          -x OMP_PROC_BIND=close \
          -x OMP_PLACES=cores \
          "$@"
  exit $?
fi
#
