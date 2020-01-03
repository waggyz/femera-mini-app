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
#for SIZE in largeminpart; do
for SIZE in medium large largeminpart small; do
for PORD in 1 2 3; do

case "$SIZE" in
medium)
  ZSTR=500kdof
  NMODEL=1
  ITER=10000
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
  ZSTR=50mdof
  NMODEL=1
  ITER=100
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
  ZSTR=50mminpart
  NMODEL=1
  ITER=100
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
small)
  #FIXED This does not work with ittnotify around the iter loop.
  #
  ZSTR=5kdof
  #
  #NMODEL=$(( 12 * $CPUCOUNT ))
  #FIXME Should set total model count constant?
  # NMODEL = 250 = 100/4000 * 50e6/5e3 = 10000/4000 * 500e3/5e3
  # NMODEL = 240 = 100/4167 * 50e6/5e3 = 10000/4167 * 500e3/5e3
  if [ "$CPUMODEL" == "E7-4830" ]; then
    NMODEL=280
    ITER=3571
  else
    NMODEL=240
    ITER=4167
  fi
  case "$PORD" in
  1)
    ESTR=tet4
    MSTR=uhxt11p1
    ;;
  2)
    ESTR=tet10
    MSTR=uhxt5p2
    ;;
  3)
    ESTR=tet20
    MSTR=uhxt3p3
    ;;
  esac
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
if [ $NMODEL -eq 1 ]; then
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
