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
CSV1=$PERFDIR/"50mdof-tet10"
CSV2=$CPUMODEL"-"$CSTR".csv"
C=$CPUCOUNT
P=2; H=121;# 50 MDOF @ 3 DOF/node ---------------------------------------------
MESHNAME="uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
#
# plas-xxx Plasticity FIXME not vectorized yet
#
PHYS=elas-dmv;# DMAT in global coordinates
"$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
echo Warming up...
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
  $REPEAT_TEST_N" times..."
for I in $(seq 1 $REPEAT_TEST_N ); do
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
    >> $CSV1"-"$PHYS"-"$CSV2
done
#
PHYS=elas-ort;# Orthotropic
"$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
echo Warming up...
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
  $REPEAT_TEST_N" times..."
for I in $(seq 1 $REPEAT_TEST_N ); do
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
    >> $CSV1"-"$PHYS"-"$CSV2
done
#
PHYS=elas-iso;# Isotropic
"$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
echo Warming up...
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
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
  "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
echo "Running "$ITERS" iterations of "$MESHNAME" (elas-lms),"\
  $REPEAT_TEST_N" times..."
for I in $(seq 1 $REPEAT_TEST_N ); do
  "$LMSEXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
    >> $CSV1"-elas-lms-"$CSV2
done
#
CSTR=gcc
CSV2=$CPUMODEL"-"$CSTR".csv"
P=2; H=178;# 50 MDOF @ 1 DOF/node ---------------------------------------------
MESHNAME="uhxt"$H"p"$P/"uhxt"$H"p"$P"n"$N
#FIXME Check the node count
#
PHYS=ther-iso;# Thermal
"$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ $PHYS "$MESHDIR";
echo Warming up...
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS_MIN -p "$MESHDIR"/"$MESHNAME" > /dev/null
echo "Running "$ITERS" iterations of "$MESHNAME" ($PHYS),"\
  $REPEAT_TEST_N" times..."
for I in $(seq 1 $REPEAT_TEST_N ); do
  "$FMREXE" -v1 -c$C -r$RTOL -i$ITERS -p "$MESHDIR"/"$MESHNAME" \
    >> $CSV1"-"$PHYS"-"$CSV2
done
#
# Return the *.fmr to elas-iso
"$PERFDIR"/"mesh-part.sh" $H $P $SX $SY $SZ "elas-iso" "$MESHDIR";
#
#******************************************************************************
# thel-iso# thermo-elastic
#FIXME not vectorized yet
#
#P=2; H=112;# 50 MDOF @ 4 DOF/node
#
# Return the *.fmr to elas-iso
#
