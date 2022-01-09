#!/bin/bash
#
# fmrexec -n 2 --bind-to core -map-by node:pe=2 build/femera/mini -n2 -o2
#
FMREXE="femera"
for ARG in "$@"
do
  case "$ARG" in
  "mini")
    FMREXE="mini"
    ;;
  "test")
    FMRARGS+=" -fmr:test"
    ;;
  "tdd")
    #NOTE The Femera build directory must be in the PATH.
    #FIXME change -T to -fmr:test
    FMRARGS+=" -T"
    NMPI=2
    NOMP=2
    ;;
  "grind" | "valgrind" | "memcheck")
    VGEXE="valgrind"
    # --leak-check=full --track-origins=yes"
    #VGEXE+=" --suppressions=$BUILD_CPU/$VALGRIND_SUPP"
    #VGEXE+=" --log-file=$BUILD_CPU/mini.valgrind.log"
    ;;
  "run" | "femera")
    ;;
  -fmr:n*)
    NMPI=${ARG##*-fmr:n}
    # FMRARGS+=" -n"${ARG##*-fmr:n}
    MPIARGS+=" -n "${ARG##*-fmr:n}
    ;;
  -fmr:o*)
    NOMP=${ARG##*-fmr:o}
    # FMRARGS=${ARG##*-fmr:o}
    MPIARGS+=" -map-by node:pe=${ARG##*-fmr:o}"
    ;;
  mpi:*)
    MPIARGS+=" "${ARG##*:}
    ;;
  -mpi:*)
    MPIARGS+=" -"${ARG##*:}
    ;;
  --mpi:*)
    MPIARGS+=" --"${ARG##*:}
    ;;
  vg:*)
    VGARGS+=" "${ARG##*:}
    ;;
  -vg:*)
    VGARGS+=" -"${ARG##*:}
    ;;
  --vg:*)
    VGARGS+=" --"${ARG##*:}
    ;;
  *)
    FMRARGS+=" $ARG"
    ;;
  esac
done
if [ -e "$MPIARGS" ]; then
  MPIARGS=" -n "$NMPI" --bind-to core -map-by node:pe="$NOMP
fi

if [ 1 -eq 1 ]; then
  echo "$VGEXE""$VGARGS" mpiexec"$MPIARGS" $FMREXE -o$NOMP $FMRARGS
#      "$VGEXE""$VGARGS" mpiexec"$MPIARGS" $FMREXE -o$NOMP $FMRARGS
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
