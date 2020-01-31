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
I=10
#
CSTR=gcc
#
MEM=`free -g  | grep Mem | awk '{print $7}'`
CPUMODEL=`$EXEDIR/cpumodel.sh`
CPUCOUNT=`$EXEDIR/cpucount.sh`
GMSH=`which gmsh`
#
CPUMODELC=$CPUMODEL"-"$CSTR
EXE=$EXEDIR/femerq-$CPUMODELC
if [ $MEM > 100 ]; then
  CPUNAMEC=$CPUMODEL"b""-"$CSTR
else
  CPUNAMEC=$CPUMODEL"-"$CSTR
fi
#
module load gcc_8.3.0
make clean
make -j$CPUCOUNT all

#
export OMP_NUM_THREADS=$CPUCOUNT
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
#
for P in 1 2 3; do
  case $P in
    1)
      H=531; N=3360; SLICE="-xS 12 -yS 14 -zS 20";
      DIR=$MESHDIR/"uhxt"$H"p"$P
      $GMSH -nt $CPUCOUNT -v1 -setnumber p $P -setnumber h $H -setnumber n 1 -3 \
        -format msh2 -o $DIR/"uhxt"$H"p"$P"n.msh" -save $EXEDIR/geo/uhxt-cube.geo
      #NOTE Too big to slice in 90GB RAM.
      ;;
    2)
      H=265; N=18480; SLICE="-xS 20 -yS 28 -zS 33";
      ;;
    3)
      H=174; N=9240; SLICE="-xS 20 -yS 21 -zS 22";
      ;;
  esac
  MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
  if [ 1 -eq 1 ]; then
o    $EXEDIR/"gmsh2fmr-"$CPUMODEL"-gcc" -v1 \
     -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      $SLICE -M0 -E100e9 -N0.3 -a $MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"
  fi
  CSV=$PERFDIR/"strong-uhxt"$H"p"$P"-"$CPUNAMEC".csv"
  # Warm up
  $EXE -c$CPUCOUNT -i1 -p $MESH > $CSV
  for C in $(seq 1 $CPUCOUNT); do
    if [ $(( $(($N / $C)) * $C)) -eq $N ]; then
      for X in $(seq 1 $REPEAT); do
        $EXE -c$C -i$I -p $MESH >> $CSV
      done
    fi
  done
done
#
