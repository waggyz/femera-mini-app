#!/bin/bash
#
#
ADVDIR=/u/dwagner5/intel/advixe/projects/
#
#
module load intel
#
for SIZE in 500kdof 50mdof; do
for ESTR in tet4 tet10 tet20; do
for YSTR in iso; do
#for NSTR in "X5675-6148" X5675 6148 E5-2640 E5-2650 E5-2670 7210; do
for NSTR in "X5675-6148"; do
for CSTR in icc; do
  echo $NSTR
  if [ $NSTR=="" ]; then
    NAME=$SIZE"-"$ESTR"-"$YSTR"-"$NSTR
  else
    NAME=$SIZE"-"$ESTR"-"$YSTR"-"$NSTR"-"$CSTR
  fi
  ADIR=$ADVDIR"/femera-"$NAME
  if [ -d "$DIR" ]; then
    EXE="advixe-cl --report survey --show-all-columns --no-show-all-rows --format=csv"
    EXE=$EXE" --project-dir" $ADIR "--report-output=survey-"$NAME".csv"
    #
    echo $EXE
    $EXE
  fi
done
done
done
done
done
#
#
#