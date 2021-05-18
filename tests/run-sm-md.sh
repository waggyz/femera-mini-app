#!/bin/bash
#
MESH_N=150
#
# P2: H = 2:1kdof, 7:10kdof, 39:1Mdof, ?72:10Mdof, ?157:100Mdof
H_MIN=2
H_MAX=39
#
C=`tools/cpucount.sh`
#
TESTDIR="build/tests/sm-md"
TIMEFILE="build/tests/run-sm-md.err"
rm -rf "$TESTDIR"
rm -f $TESTDIR.*.out
rm -f "$TIMEFILE"
#
EXEC="tests/mesh-sm-md.sh $MESH_N $H_MIN $H_MAX $TESTDIR"
echo $EXEC...
/bin/time --format="%e" $EXEC 2>> "$TIMEFILE"
#
EXEC="fmr1omp build/femera/mini -v5 -Dd0 -t7 $TESTDIR"
EXEC=$EXEC" -o'build/femera/cube-tets.cgn'"
echo $EXEC...
/bin/time --format="%e" $EXEC 2>> "$TIMEFILE"
#
tests/timing2octave.csv.sh "$TESTDIR"
#
