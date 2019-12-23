#!/bin/bash
#
#
ADVDIR="/u/dwagner5/intel/advixe/projects"
PERFDIR="/u/dwagner5/femera-mini-develop/perf"
#
#
module load intel Python_3.7.1

#PYTHONPATH=$PREFIX_PATH
#pip install --target="$PREFIX_PATH" csvfilter
#
for SIZE in 500kdof 50mdof; do
for ESTR in tet4 tet10 tet20; do
for YSTR in iso; do
#for NSTR in "X5675-6148" X5675 6148 E5-2640 E5-2650 E5-2670 7210; do
for NSTR in "X5675-6148"; do
for CSTR in icc; do
  if [ $NSTR=="" ]; then
    NAME=$SIZE"-"$ESTR"-"$YSTR"-"$NSTR
  else
    NAME=$SIZE"-"$ESTR"-"$YSTR"-"$NSTR"-"$CSTR
  fi
  ADIR=$ADVDIR"/femera-"$NAME
  ACSV=$PERFDIR"/survey-"$NAME".csv"
  if [ -d "$ADIR" ]; then
    EXE="advixe-cl --report survey --show-all-columns --no-show-all-rows --format=csv"
    EXE=$EXE" --csv-delimiter=tab --project-dir "$ADIR" --report-output="$ACSV
    #
    #echo $EXE
    PCSV=$PERFDIR"/advixe-"$NAME".csv"
    echo "Extracting "$ADIR" to "$PCSV"..."
    $EXE
    # ID,Total Time,Self Time,Total Elaped Time,Self Elaped Time,
    # Line,Self GFLOPS,Total GFLOPS,Self AI,Total Arithmetic Intenity,
    # Self Overall AI,Total Overall AI,Self GFLOP,Self GINTOP,Self Giga OP,
    # Self Memory GB,Self GB/s
    cut -f 1,7,9,10,11,28,49,50,51,52,64,65,70,71 --output-delimiter=,\
    $ACSV |  sed  '/[apq]/d' | sed 's/[s<"]//g' | sed '/^$/d' > $PCSV
    #
  fi
done
done
done
done
done
#
#
#