#!/bin/bash

cd /u/dwagner5/femera-mini-develop

CPUMODEL=`./cpumodel.sh`
CPUCOUNT=`./cpucount.sh`

module purge
module load gcc_8.3.0
makw clean
make -j$CPUCOUNT mini-all
module load intel
make -j$CPUCOUNT mini-all

XDIR=/u/dwagner5/femera-mini-develop
MDIR=/hpnobackup1/dwagner5/femera-test/cube

for YSTR in iso ort; do
for CSTR in icc gcc; do
#CSTR=icc
#YSTR=iso
for SIZE in medium large; do

case "$SIZE" in
  small)
    SSTR=3kdof
    MSTR=uhxt4p2
    PART=1
    ITER=100
    ;;
  medium)
    SSTR=1mdof
    MSTR=uhxt33p2
    PART=240
    ITER=100
    ;;
  large)
    SSTR=50mdof
    MSTR=uhxt123p2
    PART=1920
    ITER=100
    ;;
esac
advixe-cl --collect survey\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-"$SSTR"-tet10-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  XDIR/"femerq-"$CPUMODEL"-"$CSTR -v1 -c$CPUCOUNT -r0 -i$ITER -p $MDIR/$MSTR/$MSTR"n"$PART

advixe-cl --collect tripcounts --flop --stacks\
  --project-dir /u/dwagner5/intel/advixe/projects/"femera-"$SSTR"-tet10-"$YSTR"-"$CPUMODEL"-"$CSTR --\
  $XDIR/"femerq-"$CPUMODEL"-"$CSTR -v1 -c$CPUCOUNT -r0 -i$ITER -p $MDIR/$MSTR/$MSTR"n"$PART

done
done
done
# --flop --stacks
# -resume-after=30 -stop-after=40