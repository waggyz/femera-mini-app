#!/bin/bash
#
# fmrexec -n 2 --bind-to core -map-by node:pe=2 build/femera/mini -n2 -o2
#
#FIXME This is for OpenMPI. Need one for MPICH.
#
FMREXE="femera"
MAP_BY="node"
NCPU=`fmrcores`
DO_RUN="true"
if [ -z "$NCPU" ]; then
  NCPU=2
fi
for ARG in "$@"
do
  if [[ -f "$ARG" && -x "$ARG" ]]; then
    FMREXE="$ARG"
  else
    if [[ "$ARG" == -n* ]]; then
      TODO_ARG="-fmr:n"${ARG#-n}
    fi
    if [[ "$ARG" == -o* ]]; then
      TODO_ARG="-fmr:o"${ARG#-o}
    fi
    case "$ARG" in
    "auto" | "femera")
      PRE=fmr
      ;;
    "echo")
      DO_RUN=""
      ;;
    "tune")
      FMRARGS+=" -fmr:P"
      PRE=fmr
      ;;
    "test")
      #FMRARGS+=" -fmr:test"
      #TODO change -T to -fmr:test ?
      FMRARGS+=" -fmr:T"
      PRE=fmr
      ;;
    tdd*)
      #TODO change -T to -fmr:test ?
      FMRARGS+=" -fmr:T"
      NMPI=2
      NOMP=2
      PRE=fmr
      #TODO tdd:<executable> is deprecated
      #FMREXE="femera"
      if [ "$ARG" != "tdd" ]; then
        X="${ARG#tdd:}"
        if [ -n "$X" ]; then
          FMREXE=$X
        fi
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
      ;;
    -fmr:o*)
      PRE=fmr
      NOMP=${ARG#-fmr:o}
      ;;
    *:pe=*)
      NOMP=${ARG#*pe=}
      MAP_BY="${ARG%%pe=*}"
      ;;
    *mpi:*)
      PRE=mpi; MPIARGS+=" "${ARG%%mpi:*}""${ARG#*mpi:};;
    *vg:*)
      PRE=vg; VGARGS+=" ""${ARG%%vg:*}"${ARG#*vg:};;
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
  fi
done
if [ -z "$MPIARGS" ]; then
  MPIARGS=" --bind-to core"
  #MPIARGS=" --bind-to core:overload-allowed"
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

if [ 0 -eq 1 ]; then #FIXME disable for MPICH CLI debugging
if [ -z "$NOMP" ]; then
  let "NOMP = $NCPU / $NMPI"
  if [ "$NOMP" -lt "1" ]; then NOMP=1; fi
fi
MPIARGS+=" -map-by $MAP_BY:pe=""$NOMP"
fi

if [ -n "$VGARGS" ]; then
  VGEXE="valgrind"
fi
if [ -n "$VGEXE" ]; then
  VGARGS+=" "
fi
#FMREXE+=" -fmr:n$NMPI -fmr:o$NOMP"
FMREXE+=" -fmr:o$NOMP"
#FMREXE+=" -o$NOMP" #TODO pass -o or -fmr:o ?
RUN=$VGEXE"$VGARGS"mpiexec$MPIARGS" "$FMREXE$FMRARGS

LOCAL_INSTALL="/home/dwagner5/.local"
#FIXME argument?

TMP_LIBRARY_PATH=$LD_LIBRARY_PATH
if ! [[ "$TMP_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/lib64:" ]]
then
  TMP_LIBRARY_PATH="$LOCAL_INSTALL/lib64:$TMP_LIBRARY_PATH"
fi
if ! [[ "$TMP_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/lib:" ]]
then
  TMP_LIBRARY_PATH="$LOCAL_INSTALL/lib:$TMP_LIBRARY_PATH"
fi
CPUMODEL=`fmrmodel`
if ! [ -z "$CPUMODEL" ]; then
  if ! [[ "$TMP_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/$CPUMODEL/lib64:" ]]
  then
    TMP_LIBRARY_PATH="$LOCAL_INSTALL/$CPUMODEL/lib64:$TMP_LIBRARY_PATH"
  fi
  if ! [[ "$LD_LIBRARY_PATH:" =~ "$LOCAL_INSTALL/$CPUMODEL/lib:" ]]
  then
    TMP_LIBRARY_PATH="$LOCAL_INSTALL/$CPUMODEL/lib:$TMP_LIBRARY_PATH"
  fi
fi
OMP_PLACES=cores; export OMP_PROC_BIND=spread
#TODO does setting HWLOC_XMLFILE speed up MPICH MPI_init (..)?
#TODO ...not on development VM
#F="$LOCAL_INSTALL/$CPUMODEL/share/hwloc/hwloc.xml"
#if [ -f "$F" ]; then
#  echo "LD_LIBRARY_PATH=$TMP_LIBRARY_PATH HWLOC_XMLFILE=$F HWLOC_THISSYSTEM=1 $RUN"
#  if [ "$DO_RUN" == "true" ]; then
#    LD_LIBRARY_PATH=$TMP_LIBRARY_PATH HWLOC_XMLFILE=$F HWLOC_THISSYSTEM=1 $RUN
#  fi
#else
  echo "env LD_LIBRARY_PATH=$TMP_LIBRARY_PATH OMP_PROC_BIND=spread OMP_PLACES=cores OMP_NUM_THREADS=$NOMP $RUN"
  if [ "$DO_RUN" == "true" ]; then
    env LD_LIBRARY_PATH=$TMP_LIBRARY_PATH OMP_PROC_BIND=spread OMP_PLACES=cores OMP_NUM_THREADS=$NOMP $RUN
  fi
#fi
exit $?

if [ 0 -eq 1 ]; then
  # TODO Make this work?
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

