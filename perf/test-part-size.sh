#!/bin/bash
#
PERFDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )";
if [ -d "/hpnobackup1/dwagner5/femera-test/cube" ]; then
  MESHDIR=/hpnobackup1/dwagner5/femera-test/cube
else
  MESHDIR=cube
fi
EXEDIR=$( cd "$PERFDIR"; cd ../; pwd )
#
REPEAT=6
PHYS="elas-iso"
CSTR=gcc
#
CPUMODEL=`"$EXEDIR"/cpumodel.sh`
CPUCOUNT=`"$EXEDIR"/cpucount.sh`
#
CPUMODELC=$CPUMODEL"-"$CSTR
C=$CPUCOUNT
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=$CPUCOUNT
#
for P in 2 3 2 1; do
  case $P in
    1) H=246; I=100; I0=1; ;;
    2) H=121; I=100; I0=1; ;;
    3) H=80;  I=100; I0=1; ;;
  esac
  CSVFILE=$PERFDIR/"part-uhxt"$H"p"$P"-"$CPUMODELC".csv"
  while IFS="," read -r N SX SY SZ REMAINDER; do
    # echo $N $SX $SY $SZ
    $PERFDIR/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS $MESHDIR
    FMRNAME=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
    $EXEDIR/"femerq-"$CPUMODELC -v1 -c$C -i$I0 -p $FMRNAME
    for X in $(seq 1 $REPEAT); do
      $EXEDIR/"femerq-"$CPUMODELC -v1 -c$C -i$I -p $FMRNAME >> $CSVFILE
    done;
  done < "$PERFDIR/part-slice.csv"
done
#
#