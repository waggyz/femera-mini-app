#!/bin/bash
#
# fmrexec -n 2 --bind-to core -map-by node:pe=2 build/femera/mini -n2 -o2
#
FMREXE="femera"
MAP_BY="node"
NCPU=`fmrcores`
if [ -z "$NCPU" ]; then
  NCPU=2
fi
for ARG in "$@"
do
  if [[ "$ARG" == -n* ]]; then
    ARG="-fmr:n"${ARG#-n}
  fi
  if [[ "$ARG" == -o* ]]; then
    ARG="-fmr:o"${ARG#-o}
  fi
  case "$ARG" in
  exe:* | exec:* | -exe:* | -exec:* | --exe:* | --exec:*)
    FMREXE="${ARG#*exe*:}"
    ;;
  "auto" | "femera")
    PRE=fmr
    ;;
  "test")
    FMRARGS+=" -fmr:test"
    PRE=fmr
    ;;
  tdd*)
    #NOTE The Femera build directory must be in the PATH.
    #FIXME change -T to -fmr:test
    FMRARGS+=" -T"
    # MPIARGS+=" -n 2"
    NMPI=2
    NOMP=2
    PRE=fmr
    X="${ARG#tdd:}"
    if [ -n "$X" ]; then
      FMREXE=$X
    fi
    ;;
  "grind" | "valgrind" | "memcheck")
    VGEXE="valgrind"
    PRE=vg
    # --leak-check=full --track-origins=yes"
    #VGEXE+=" --suppressions=$BUILD_CPU/$VALGRIND_SUPP"
    #VGEXE+=" --log-file=$BUILD_CPU/mini.valgrind.log"
    ;;
  -fmr:n*)
    PRE=fmr
    NMPI=${ARG#-fmr:n}
    # FMRARGS+=" -n"${ARG##*-fmr:n}
    # MPIARGS+=" -n "${ARG##*-fmr:n}
    ;;
  -fmr:o*)
    PRE=fmr
    NOMP=${ARG#-fmr:o}
    # FMRARGS=${ARG##*-fmr:o}
    # MPIARGS+=" -map-by node:pe=${ARG##*-fmr:o}"
    ;;
  *:pe=*)
    NOMP=${ARG#*pe=}
    MAP_BY="${ARG%%pe=*}"
    ;;
  *mpi:*)
    PRE=mpi; MPIARGS+=" "${ARG%%mpi:*}""${ARG#*mpi:};;
  *vg:*)
    PRE=vg; VGARGS+=" ""${ARG%%vg:*}"${ARG#*mpi:};;
  *)
    case "$PRE" in
    mpi)
      MPIARGS+=" $ARG";;
    vg)
      VGARGS+=" $ARG";;
    *)
      FMRARGS+=" $ARG";;
    esac
    ;;
  esac
done
if [ -z "$MPIARGS" ]; then
  MPIARGS=" --bind-to core"
fi
if [ -z "$NMPI" ]; then
  if [ -z "$NOMP" ]; then
    NNUM=`fmrnumas 2>/dev/null`
    if [ -z "$NNUM" ]; then
      NNUM=2
    fi
    NMPI=$NNUM
  else
    let "NMPI = $NCPU / $NOMP"
  if [ "$NMPI" -lt "1" ]; then NMPI=1; fi
  fi
fi
MPIARGS+=" -n ""$NMPI"
if [ -z "$NOMP" ]; then
  let "NOMP = $NCPU / $NMPI"
  if [ "$NOMP" -lt "1" ]; then NOMP=1; fi
fi
MPIARGS+=" -map-by $MAP_BY:pe=""$NOMP"
if [ -n "$VGARGS" ]; then
  VGEXE="valgrind"
fi
if [ -n "$VGEXE" ]; then
  VGARGS=" "
fi
FMREXE+=" -fmr:n$NMPI -fmr:o$NOMP"
#echo $VGEXE "$VGARGS"mpiexec $MPIARGS $FMREXE "$FMRARGS"
$VGEXE "$VGARGS"mpiexec $MPIARGS $FMREXE "$FMRARGS"

exit $?

if [ 0 -eq 1 ]; then
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
