#!/bin/bash
#
MESH_N=200
#
# P2: H = 2:1kdof, 7:10kdof, 35:1Mdof, 77:10Mdof, 164:100Mdof
H_MIN=2
H_MAX=35
#
C=`tools/cpucount.sh`
#
#CNCR="1" # Prep serial.
CNCR=$C   # Prep parallel.
#
TESTDIR="build/tests/sizes"
TIMEFILE="$TESTDIR-run.err"
rm -rf "$TESTDIR"
rm -f $TESTDIR.*.out
rm -f $TIMEFILE.*
rm -f $TESTDIR-time.err.*
rm -f "$TESTDIR-time.csv"
#
EXEC="tests/mesh-sizes.sh $MESH_N $H_MIN $H_MAX $TESTDIR"
echo $EXEC...
for RUN_I in $(seq 1 1 $CNCR) ; do
  /bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC $RUN_I 2>> "$TIMEFILE.$RUN_I" &
  # wall clock, max mem, avg mem use, kernel CPU-sec, user CPU-sec, %CPU
done
wait
cat $TIMEFILE.* > "$TIMEFILE"
cat $TESTDIR-time.err.* > "$TESTDIR-time.err"
#
EXEC="fmr1omp build/femera/mini -v5 -Dd0 -t7 $TESTDIR"
EXEC=$EXEC" -o'build/femera/cube-tets.cgn'"
echo $EXEC...
/bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC 2>> "$TIMEFILE"
#
tests/timing2octave.csv.sh "$TESTDIR"
#
