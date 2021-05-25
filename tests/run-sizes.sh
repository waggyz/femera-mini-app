#!/bin/bash
#
MESH_N=4000
#
# P2: H = 2:1kdof, 7:10kdof, 35:1Mdof, 77:10Mdof, 164:100Mdof
H_MIN=2
H_MAX=77
#
C=`tools/cpucount.sh`
#
TESTDIR="build/tests/xs-md-10fmt"
TIMEFILE="$TESTDIR-run.err"
rm -rf "$TESTDIR"
rm -f $TESTDIR.*.out
rm -f "$TIMEFILE"
#
EXEC="tests/mesh-sizes.sh $MESH_N $H_MIN $H_MAX $TESTDIR"
echo $EXEC...
/bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC 2>> "$TIMEFILE"
# wall clock, max mem, avg mem use, kernel CPU-sec, user CPU-sec, %CPU
#
EXEC="fmr1omp build/femera/mini -v5 -Dd0 -t7 $TESTDIR"
EXEC=$EXEC" -o'build/femera/cube-tets.cgn'"
echo $EXEC...
/bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC 2>> "$TIMEFILE"
#
tests/timing2octave.csv.sh "$TESTDIR"
#
