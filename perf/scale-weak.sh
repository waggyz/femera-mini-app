#!/bin/bash
#
# 500 MDOF
# tet4 : uhxt531p1n3360  (3520) parts  3*4 * 7*2 * 5*4
# tet10: uhxt365p2n18480 (15360-20480) parts 4*5 * 4*7 * 3*11
# tet20: uhxt174p3n9240  (8960-14040) 5*4 * 3*7 * 2*11
#
MESHDIR=/hpnobackup1/dwagner5/femera-test/cube
EXEDIR=/u/dwagner5/femera-mini-develop
PERFDIR=$EXEDIR/perf
#
REPEAT=6
#
YSTR=iso
CSTR=gcc
#
CPUMODEL=`$EXEDIR/cpumodel.sh`
CPUCOUNT=`$EXEDIR/cpucount.sh`
GMSH=`which gmsh`
#
CPUMODELC=$CPUMODEL"-"$CSTR
EXE=$EXEDIR/femera-mmq-$CPUMODELC
#
module load gcc_8.3.0
#make clean
make -j$CPUCOUNT all
#
export OMP_NUM_THREADS=$CPUCOUNT
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread,close
export OMP_NESTED=false;
#
for SIZE in sm; do # md lg; do
for P in 1 2 3; do
  if [ $SIZE == "sm" ]; then
    I=4000; I0=40; N=1; NMODEL=240;
    export OMP_PROC_BIND=spread,close
    export OMP_NESTED=true;
    export OMP_MAX_ACTIVE_LEVELS=2
    case $P in
      1)
        H=11;# 5 kdof
        ;;
      2)
        H=5;# 5 kdof
        ;;
      3)
        H=3;# 5 kdof
        ;;
    esac
  fi
  MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
  CSV=$PERFDIR/"weak-uhxt"$H"p"$P"-"$YSTR"-"$CPUMODELC".csv"
  echo "Warming up..."
  $EXE -c$CPUCOUNT -n$CPUCOUNT -m$NMODEL  -i$I0 -p $MESH > $CSV
  for C in $(seq 1 $CPUCOUNT); do
    if [ $(( $(($NMODEL / $C)) * $C)) -eq $NMODEL ]; then
      NC=$(( $NMODEL / $C ))
      echo "Running "$I" iterations of uhxt"$H"p"$P"n"$N" on "$C" cores, "$REPEAT" times..."
      for X in $(seq 1 $REPEAT); do
        $EXE -c$C -n$C -m$NC -r0 -i$I -p $MESH >> $CSV
      done
    fi
  done
done
done
#
