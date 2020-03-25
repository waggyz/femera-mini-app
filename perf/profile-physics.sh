#!/bin/bash
# Large model physics evaluation profiling
#
EXEDIR="/u/dwagner5/femera-mini-develop"
PERFDIR=$EXEDIR/"perf"
#
CPUMODEL=`"$EXEDIR"/"cpumodel.sh"`
CPUCOUNT=`"$EXEDIR"/"cpucount.sh"`
#
CSTR=gcc
#
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=$CPUCOUNT
#
module purge
module load gcc_8.3.0
make -j$CPUCOUNT all
module load intel_2019.4.243
make -j$CPUCOUNT all
#
FMREXE=$EXEDIR/"femerq-"$CPUMODEL"-"$CSTR
LMSEXE=$EXEDIR/"femeqk-"$CPUMODEL"-icc"
#
if [ -d "/hpnobackup1/dwagner5/femera-test/cube" ]; then
  MESHDIR="/hpnobackup1/dwagner5/femera-test/cube"
else
  MESHDIR=$EXEDIR/"cube"
fi
echo "Mesh Directory: "$MESHDIR"/"
#
REPEAT_TEST_N=10;# Repeat each test N times
#
RTOL=0; ITERS_MIN=10; ITERS=100;# 5 GDOF total @ 50 MDOF system size
#
SX=14; SY=15; SZ=16; N=$(( $SX * $SY * $SZ ));# 3360 parts
#
CSV2=$CPUMODEL"-"$CSTR".csv"
C=$CPUCOUNT
for P in 2 3 1; do
  for PHYS in elas-dmv elas-ort elas-iso ther-iso; do
    case $P in
      1) H=246; ELSTR=tet4; ;; # 50 MDOF @ 3 DOF/node -------------------------
      2) H=121; ELSTR=tet10;  ;;
      3) H=80;  ELSTR=tet20;  ;;
    esac
    CSV1=$PERFDIR/"profile-physics-"$ELSTR
    MESHNAME="uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
    case $PHYS in
    # plas-xxx Plasticity FIXME not vectorized yet
      #
      #
    elas-dmv) # DMAT in global coordinates
      "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
      echo Warming up...
        /bin/time "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-"$PHYS"-"$CSV2
      done
    ;;
    elas-ort) # Orthotropic
      "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
      echo Warming up...
        /bin/time "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-"$PHYS"-"$CSV2
      done
    ;;
    elas-iso) # Isotropic
      "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
      echo Warming up...
        /bin/time "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-"$PHYS"-"$CSV2
      done
      #
      # elas-lms Local matrix storage
      CSTR=icc
      CSV2=$CPUMODEL"-"$CSTR".csv"
      echo Warming up...
        /bin/time "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" (elas-lms),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-elas-lms-"$CSV2
      done
      CSTR=gcc
      CSV2=$CPUMODEL"-"$CSTR".csv"
      ;;
    ther-iso) # Thermal
      case $P in
        1) H=343; ELSTR=tet4; ;; # 50 MDOF @ 1 DOF/node -----------------------
        2) H=178; ELSTR=tet10; ;;
        3) H=117; ELSTR=tet20; ;;
      esac
      #FIXME Check the node count
      CSV1=$PERFDIR/"profile-physics-"$ELSTR
      MESHNAME="uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
      #
      "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
      echo Warming up...
        /bin/time "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-"$PHYS"-"$CSV2
      done
      # ther-lms Local matrix storage
      CSTR=icc
      CSV2=$CPUMODEL"-"$CSTR".csv"
      echo Warming up...
        /bin/time "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
      echo "Running "$ITERS" iterations of "$MESHNAME" (ther-lms),"\
        $REPEAT_TEST_N" times..."
      for I in $(seq 1 $REPEAT_TEST_N ); do
        "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
          >> $CSV1"-ther-lms-"$CSV2
      done
      CSTR=gcc
      CSV2=$CPUMODEL"-"$CSTR".csv"
      #
      # Return the *.fmr to elas-iso
      "$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ "elas-iso" "$MESHDIR";
    ;;
    #**************************************************************************
    # thel-iso# thermo-elastic
      #FIXME not vectorized yet
      #
      #P=2; H=112;# 50 MDOF @ 4 DOF/node
      #
      # Return the *.fmr to elas-iso
      #
    esac
  done
done
#
