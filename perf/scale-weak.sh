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
GMSH2FMR=$EXEDIR/"gmsh2fmr-"$CPUMODEL"-gcc"
#
module load gcc_8.3.0
#make clean
make -j$CPUCOUNT all
#
export OMP_NUM_THREADS=$CPUCOUNT
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NESTED=false;
#
if [ 1 == 1 ]; then # Small concurrent model tests
  for SIZE in sm; do # md lg; do
    for P in 1 2 3; do
      if [ $SIZE == "sm" ]; then
        I=1667; I0=40; N=1; NMODEL=240;# Number of models using all cores
        export OMP_PROC_BIND=spread,close
        export OMP_NESTED=true;
        export OMP_MAX_ACTIVE_LEVELS=2
        case $P in
          1)
            #H=10;# 5 kdof
            H=19;# 25 kdof
            ;;
          2)
            #H=5;# 5 kdof
            H=9;# 25 kdof
            ;;
          3)
            #H=3;# 5 kdof
            H=6;# 25 kdof
            ;;
        esac
      fi
      MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
      CSV=$PERFDIR/"weak-uhxt"$H"p"$P"-"$YSTR"-"$CPUMODELC".csv"
      echo "Warming up..."
      $EXE -c$CPUCOUNT -n$CPUCOUNT -m$NMODEL  -i$I0 -p $MESH > $CSV
      for C in $(seq 1 $CPUCOUNT); do
        NC=$(( $NMODEL / $CPUCOUNT * $C ))
        echo $REPEAT,$NC >> $CSV
        echo "Running "$I" iterations of $NC x uhxt"$H"p"$P"n"$N" on "$C" cores, "$REPEAT" times..."
        for X in $(seq 1 $REPEAT); do
          $EXE -c$C -n$C -m$NC -r0 -i$I -p $MESH >> $CSV
        done
      done
    done
  done
fi
if [ 1 == 1 ]; then # Medium and Large Weak Scaling Tests
  EXE=$EXEDIR/femerq-$CPUMODELC
  export OMP_PROC_BIND=spread
  export OMP_NESTED=false;
  unset OMP_MAX_ACTIVE_LEVELS
  for SZSTR in 500kdof; do # 500kdof 50mdof; do
    for P in 1; do
      for C in 1 2 3 4 6 8 10 16 20 30 40; do
        case $SZSTR in
          500kdof)
          N=$(( 6 * $C ))
          I=10000; I0=100;
          case $P in
            1) # P=1
              case $C in
                1)  H=15; ;; #  12.5 kdof
                2)  H=19; ;; #  25   kdof
                3)  H=21; ;; #  37.5 kdof
                4)  H=23; ;; #  50   kdof
                6)  H=27; ;; #  75   kdof
                8)  H=30; ;; # 100   kdof
                10) H=33; ;; # 125   kdof
                12) H=35; ;; # 150   kdof
                16) H=38; ;; # 200   kdof
                20) H=41; ;; # 250   kdof
                30) H=47; ;; # 375   kdof
                40) H=52; ;; # 500   kdof
              esac
            ;;
            2) # P=2
              case $C in
                1)  H=7;  ;; #  12.5 kdof
                2)  H=9;  ;; #  25   kdof
                3)  H=10; ;; #  37.5 kdof
                4)  H=11; ;; #  50   kdof
                6)  H=13; ;; #  75   kdof
                8)  H=15; ;; # 100   kdof
                10) H=16; ;; # 125   kdof
                12) H=17; ;; # 150   kdof
                16) H=19; ;; # 200   kdof
                20) H=21; ;; # 250   kdof
                30) H=23; ;; # 375   kdof
                40) H=26; ;; # 500   kdof
              esac
            ;;
            3) # P=3
              case $C in
                1)  H=4;  ;; #  12.5 kdof
                2)  H=6;  ;; #  25   kdof
                3)  H=7;  ;; #  37.5 kdof
                4)  H=8;  ;; #  50   kdof
                6)  H=9;  ;; #  75   kdof
                8)  H=10; ;; # 100   kdof
                10) H=11; ;; # 125   kdof
                12) H=12; ;; # 150   kdof
                16) H=13; ;; # 200   kdof
                20) H=14; ;; # 250   kdof
                30) H=15; ;; # 375   kdof
                40) H=17; ;; # 500   kdof
              esac
            ;;
          esac
          ;;
          50mdof)
          N=$(( 84 * $C ))
          I=100; I0=1;
          case $P in
            1) # P=1
              case $C in
                1)  H=72;  ;; #  1.25 Mdof
                2)  H=91;  ;; #  2.5  Mdof
                3)  H=104; ;; #  3.75 Mdof
                4)  H=113; ;; #  5    Mdof
                6)  H=133; ;; #  7.5  Mdof
                8)  H=141; ;; # 10    Mdof
                10) H=154; ;; # 12.5  Mdof
                12) H=167; ;; # 15    Mdof
                16) H=183; ;; # 20    Mdof
                20) H=195; ;; # 25    Mdof
                30) H=222; ;; # 37.5  Mdof
                40) H=246; ;; # 50    Mdof
              esac
            ;;
            2) # P=2
              case $C in
                1)  H=35;  ;; #  1.25 Mdof
                2)  H=45;  ;; #  2.5  Mdof
                3)  H=51;  ;; #  3.75 Mdof
                4)  H=57;  ;; #  5    Mdof
                6)  H=64;  ;; #  7.5  Mdof
                8)  H=71;  ;; # 10    Mdof
                10) H=77;  ;; # 12.5  Mdof
                12) H=82;  ;; # 15    Mdof
                16) H=90;  ;; # 20    Mdof
                20) H=96;  ;; # 25    Mdof
                30) H=112; ;; # 37.5  Mdof
                40) H=121; ;; # 50    Mdof
              esac
            ;;
            3) # P=3
              case $C in
                1)  H=22; ;; #  1.25 Mdof
                2)  H=29; ;; #  2.5  Mdof
                3)  H=33; ;; #  3.75 Mdof
                4)  H=37; ;; #  5    Mdof
                6)  H=42; ;; #  7.5  Mdof
                8)  H=46; ;; # 10    Mdof
                10) H=51; ;; # 12.5  Mdof
                12) H=54; ;; # 15    Mdof
                16) H=61; ;; # 20    Mdof
                20) H=63; ;; # 25    Mdof
                30) H=73; ;; # 37.5  Mdof
                40) H=80; ;; # 50    Mdof
              esac
            ;;
          esac
          case $C in
            16) SLICESTR="-xS 8 -yS 12 -zS 14" ;; # 1344 parts
            20) SLICESTR="-xS 10 -yS 12 -zS 14" ;; # 1680 parts
            30) SLICESTR="-xS 12 -yS 14 -zS 15" ;; # 2520 parts
            40) SLICESTR="-xS 14 -yS 15 -zS 16" ;; # 3360 parts
            *) unset SLICESTR
            ;;
          esac
          ;;
        esac
        MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
        CSV=$PERFDIR/"weak-"$SZSTR"-p"$P"-"$YSTR"-"$CPUMODELC".csv"
        if [ -n "$SLICESTR" ]; then
          if [ 0 -eq 1 ];then
            $GMSH2FMR -v1  -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001\
            $SLICESTR -M0 -E100e9 -N0.3 -a $MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"
          fi
        else
          $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$GMSH2FMR" "elas-"$YSTR $C
        fi
        echo "Warming up..."
        $EXE -c$C -i$I0 -p $MESH
        echo "Running "$I" iterations of uhxt"$H"p"$P"n"$N" on "$C" cores, "$REPEAT" times..."
        for X in $(seq 1 $REPEAT); do
          $EXE -c$C -r0 -i$I -p $MESH >> $CSV
        done
      done
    done
  done
fi
