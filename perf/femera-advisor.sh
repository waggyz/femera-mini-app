#!/bin/bash

cd /u/dwagner5/femera-mini-develop

CPUMODEL=`./cpumodel.sh`
CPUCOUNT=`./cpucount.sh`

export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=$CPUCOUNT

module purge
module load gcc_8.3.0
make clean
make -j$CPUCOUNT all
module load intel
make -j$CPUCOUNT mini-all

XDIR=/u/dwagner5/femera-mini-develop
MDIR=/hpnobackup1/dwagner5/femera-test/cube

CSTR=icc
YSTR=iso

#for CSTR in icc; do # icc gcc
#for YSTR in iso; do # iso ort
#for SIZE in medium large largeminpart; do
for SIZE in largeminpart; do
for PORD in 1 2 3; do

case "$SIZE" in
medium)
  NMODEL=1
  ITER=10000
  ZSTR=500kdof
  PART=$(( 6 * $CPUCOUNT ))
  case "$PORD" in
  1)
  ESTR=tet4
    MSTR=uhxt52p1
    ;;
  2)
  ESTR=tet10
    MSTR=uhxt26p2
    ;;
  3)
  ESTR=tet20
    MSTR=uhxt17p3
    ;;
  esac
  GSTR=$MSTR"n"$PART
  SSTR=""
  ;;
large)
  NMODEL=1
  ITER=100
  ZSTR=50mdof
  PART=3360
  case "$PORD" in
  1)
    ESTR=tet4
    MSTR=uhxt246p1
    #NOTE May be too big for (meshing? partitioning?) tet4 on westmere k2 nodes
    #     but femera runs it.
    ;;
  2)
    ESTR=tet10
    MSTR=uhxt123p2
    ;;
  3)
    ESTR=tet20
    MSTR=uhxt80p3
    ;;
  esac
  GSTR=$MSTR"n"
  SSTR="-xS12 -yS14 -zS20"
  ;;
largeminpart)
  NMODEL=1
  ITER=100
  ZSTR=50mminpart
  PART=$CPUCOUNT
  case "$PORD" in
  1)
    ESTR=tet4
    MSTR=uhxt246p1
    ;;
  2)
    ESTR=tet10
    MSTR=uhxt123p2
    ;;
  3)
    ESTR=tet20
    MSTR=uhxt80p3
    ;;
  esac
  GSTR=$MSTR"n"$PART
  SSTR=""
  ;;
smallFIXME)
  #FIXME This will not work with ittnotify around the iter loop.
  NMODEL=$(( 12 * $CPUCOUNT ))
  ITER=4000
  ESTR=tet10
  ZSTR=5kdof
  MSTR=uhxtFIXMEp2
  PART=1
  GSTR=$MSTR"n"$PART
  SSTR=""
  ;;
esac

case "$YSTR" in
  iso)
    $XDIR/"gmsh2fmr-"$CPUMODEL"-gcc" -v1 \
    -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 $SSTR \
    -M0 -E100e9 -N0.3 \
    -a $MDIR/$MSTR/$GSTR
    ;;
  ort)
    $XDIR/"gmsh2fmr-"$CPUMODEL"-gcc" -v1 \
    -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 $SSTR \
    -M0 -E100e9 -N0.3 -R \
    -a $MDIR/$MSTR/$GSTR
    ;;
esac
if [ "$NMODEL" == "1" ]; then
  EXESTR=$XDIR/"femerq-"$CPUMODEL"-"$CSTR" -c"$CPUCOUNT" -r0 -i"$ITER\
" -p "$MDIR/$MSTR/$MSTR"n"$PART
else
  EXESTR=$XDIR/"femera-mmq-"$CPUMODEL"-"$CSTR" -c"$PART" -n"$CPUCOUNT" -m"$NMODEL\
" -r0 -i"$ITER" -p "$MDIR/$MSTR/$MSTR"n"$PART
fi
advixe-cl --collect survey\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-"$ZSTR"-"$ESTR"-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  $EXESTR

advixe-cl --collect tripcounts --flop --stacks\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-"$ZSTR"-"$ESTR"-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  $EXESTR
#advixe-cl --report survey --show-all-columns --no-show-all-rows --format=csv\
# --project-dir /u/dwagner5/intel/advixe/projects/"femera-"$ZSTR"-"$ESTR"-"$YSTR"-"$CPUMODEL"-"$CSTR

done
done
