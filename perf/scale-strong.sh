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
EXE=$EXEDIR/femerq-$CPUMODELC
#
module load gcc_8.3.0
#make clean
make -j$CPUCOUNT all
#
export OMP_NUM_THREADS=$CPUCOUNT
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
#
for SIZE in md lg; do
for P in 1 2 3; do
  if [ $SIZE == "md" ]; then
    I=10000; I0=100;
    case $P in
      1)
        H=52; N=240;# 500 kdof
        ;;
      2)
        H=26; N=240;# 500 kdof
        ;;
      3)
        H=17; N=240;# 500 kdof
        ;;
    esac
  else
    I=100; I0=1;
    #I=10; I0=1;# 500 Mdof
    case $P in
      1)
        H=246; N=3360; # SLICE="-xS 14 -yS 15 -zS 16" # 50 Mdof 
        #H=531; N=3360; SLICE="-xS 12 -yS 14 -zS 20";
        if [ 1 -eq 0 ]; then
          O=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n.msh"
          #NOTE Too big to slice in 90GB RAM.
          echo "Meshing uhxt"$H"p"$P"n.msh..."
          $GMSH -nt $CPUCOUNT -v1 -setnumber p $P -setnumber h $H -setnumber n 1 -3 \
            -format msh2 -o $O -save $EXEDIR/geo/uhxt-cube.geo
        fi
        ;;
      2)
        H=121; N=3360;# 50 Mdof
        #H=265; N=18480; SLICE="-xS 20 -yS 28 -zS 33";
        ;;
      3)
        H=80; N=3360;# 50 Mdof
        #H=174; N=9240; SLICE="-xS 20 -yS 21 -zS 22";
        ;;
    esac
    if [ 1 -eq 0 ]; then
      echo "Slicing and converting uhxt"$H"p"$P"n"$N"..."
      $EXEDIR/"gmsh2fmr-"$CPUMODEL"-gcc" -v1 \
        -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICE -M0 -E100e9 -N0.3 -a $MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"
    fi
  fi
  MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
  CSV=$PERFDIR/"strong-uhxt"$H"p"$P"-"$YSTR"-"$CPUMODELC".csv"
  echo "Warming up..."
  $EXE -c$CPUCOUNT -i$I0 -p $MESH > $CSV
  for C in $(seq 1 $CPUCOUNT); do
    if [ $(( $(($N / $C)) * $C)) -eq $N ]; then
      IC=$(( 3 * $C * $I / $CPUCOUNT / 2 ));# Round up
      if [ $IC -lt 10 ]; then IC=10; fi
      echo "Running "$IC" iterations of uhxt"$H"p"$P"n"$N" on "$C" cores, "$REPEAT" times..."
      for X in $(seq 1 $REPEAT); do
        $EXE -c$C -r0 -i$IC -p $MESH >> $CSV
      done
    fi
  done
done
done
#
