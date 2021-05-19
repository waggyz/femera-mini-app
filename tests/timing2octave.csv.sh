#!/bin/bash
#
TESTDIR="$1"
echo "timing2octave.csv.sh $1"...
#
if [[ ! -f "$TESTDIR.0.out" ]] ; then
  echo "$TESTDIR.0.out does not exist."
  exit 1
fi
#
CSV="$TESTDIR.csv"
cat $TESTDIR.*.out > $CSV
#
sed -i 's/"Proc"/0/g' $CSV
sed -i 's/"Femera"/0,0,0/g' $CSV
#
# mesh prep (iter,zzzz,...) post
sed -i 's/"chck"/1/g' $CSV
sed -i 's/"scan"/1.1/g' $CSV
sed -i 's/"info"/1.2/g' $CSV
sed -i 's/"mesh"/2/g' $CSV
sed -i 's/"prep"/3/g' $CSV
#
sed -i 's/"iter"/4/g' $CSV
sed -i 's/"zzzz"/5/g' $CSV
#
sed -i 's/"post"/10/g' $CSV
#
# bin:1, asc:2,
# geo:1, gmsh2:2, gmsh4:4, cgns:5
#
# -geo.geo_unrolled -> 2,1
# -bin.cgns -> ,1,5
# -asc.msh2 -> ,2,2
# -asc.msh4 -> ,2,4
# -bin.msh2 -> ,1,2
# -bin.msh4 -> ,1,4
#
sed -i 's/-geo.geo_unrolled"/,2,1/g' $CSV
sed -i 's/-asc.geo_unrolled"/,2,1/g' $CSV
sed -i 's/-geo.geo"/,3,1/g' $CSV
sed -i 's/-asc.geo"/,3,1/g' $CSV
sed -i 's/-bin.cgns"/,1,5/g' $CSV
sed -i 's/-asc.msh2"/,2,2/g' $CSV
sed -i 's/-asc.msh4"/,2,4/g' $CSV
sed -i 's/-bin.msh2"/,1,2/g' $CSV
sed -i 's/-bin.msh4"/,1,4/g' $CSV
sed -i 's/-asc.mesh"/,2,7/g' $CSV
sed -i 's/-asc.vtk"/,2,6/g' $CSV
sed -i 's/-bin.vtk"/,1,6/g' $CSV
sed -i 's/-asc.bdf"/,2,8/g' $CSV
sed -i 's/-asc.unv"/,2,9/g' $CSV
#
sed -i 's/-bin"/,1,0/g' $CSV
sed -i 's/-asc"/,2,0/g' $CSV
sed -i 's/-geo"/,0,1/g' $CSV
#
sed -i 's/"build\/tests\/sm-md\/cube-//g' $CSV
sed -i 's/"cube-//g' $CSV
#
PREPROC="build/tests/sm-md-time.err"
grep '^[0-9]' "build/tests/sm-md-time.err" > "build/tests/sm-md-time.csv"
#
#
#