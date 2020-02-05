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
export OMP_PROC_BIND=spread
export OMP_NESTED=false;
#
if [ 1 == 0 ]; then # Small concurrent model tests
  for SIZE in sm; do # md lg; do
    for P in 1 2 3; do
      if [ $SIZE == "sm" ]; then
        I=4000; I0=40; N=1; NMODEL=240;# Number of models using all cores
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
#
if [ 1 == 1 ]; then # Medium and Large Weak Scaling Tests
  EXE=$EXEDIR/femerq-$CPUMODELC
  I=100; I0=1;
  export OMP_PROC_BIND=spread
  export OMP_NESTED=false;
  unset OMP_MAX_ACTIVE_LEVELS
  for SZSTR in 50mdof; do # 500kdof 50mdof; do
    for P in 2 3 1; do
      for C in 1 2 3 4 6 8 10 16 20 30 40; do
        case $SZSTR in
          500kdof)
          N=$(( 6 * $C ))
          case $P in
            1) # P=1
              case $C in
                1) H=11; ;; # 12.5 kdof
                2) H=11; ;; # XXX kdof
              esac
            ;;
            2) # P=2
              case $C in
                1) H=11; ;; # 12.5 kdof
                2) H=11; ;; # XXX kdof
              esac
            ;;
            3) # P=3
              case $C in
                1) H=11; ;; # 12.5 kdof
                2) H=11; ;; # XXX kdof
              esac
            ;;
          esac
          ;;
          50mdof)
          N=$(( 84 * $C ))
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
          ;;
        esac
        MESH=$MESHDIR/"uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
        CSV=$PERFDIR/"weak-"$SZSTR"-p"$P"-"$YSTR"-"$CPUMODELC".csv"
        $PERFDIR/mesh-uhxt.sh $H $P $N "$MESHDIR" "$EXEDIR/$GMSH2FMR"
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
