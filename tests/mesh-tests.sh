#!/bin/bash
#
if [ -z "$1" ]; then
  MESHDIR=tests/mesh
else
  MESHDIR=$1
fi
# Looks like gmsh CGNS does not work under MPI.
MPIEXEC=""
if [ 0 -eq 1 ];then
  MPIEXEC=`which mpiexec 2>/dev/null`
  if [ -n "$MPIEXEC" ]; then
  MPIEXEC="mpiexec -np 4"
  else
  MPIEXEC=""
  fi
fi
for P in 1 2 3; do
for N in 1 2; do
if [ $N -gt 1 ]; then
  PART="-part $N"
else
  PART=""
fi
C=`tools/cpucount.sh`
GMSH_OMP="-nt $C" #TODO Not working on K?
GMSH_OMP=""

cp -f "tests/geo/cube-tet6p1n2-geo.geo" "$MESHDIR"
if [ "$N" -eq 1 ]; then
rm -f "$MESHDIR/cube-tet6p"$P"n"$N".msh2"
# rm -f "$MESHDIR/cube-tet6p"$P"n"$N".inp"
$MPIEXEC gmsh $GMSH_OMP -setnumber p $P -setnumber h 1 tests/geo/unit-cube.geo \
 -v 3 $PART -save -o "$MESHDIR/cube-tet6p"$P"n"$N".msh2" -3 2>/dev/null
# else
# Hm, looks like gmsh can write, but not read .inp files...
# $MPIEXEC gmsh $GMSH_OMP -setnumber p $P -setnumber h 1 tests/geo/unit-cube.geo \
#  -v 3 $PART -save -o "$MESHDIR/cube-tet6p"$P"n"$N".inp" -3 2>/dev/null
fi
rm -f "$MESHDIR/cube-tet6p"$P"n"$N".cgns"
gmsh $GMSH_OMP -setnumber p $P -setnumber h 1 tests/geo/unit-cube.geo \
 -v 3 $PART -save -o "$MESHDIR/cube-tet6p"$P"n"$N".cgns" -3
chmod -w "$MESHDIR/cube-tet6p"$P"n"$N"."*
done
done
#
#gmsh -nt $C -setnumber p $P -setnumber h 1 tests/geo/unit-cube.geo \
# -v 3 $PART -save -o "$MESHDIR/cube-tet6p"$P"n"$N".inp" -3
#$MPIEXEC gmsh -nt $C -setnumber p $P -setnumber h 1 -setnumber n $N \
# tests/geo/unit-cube.geo \
# -v 3 -save -o "$MESHDIR/cube-tet6p"$P"n"$N".mesh" 2>/dev/null

# H5=`which h5import`
# if [ -n "$h5import" ]; then
#   rm -f "$MESHDIR/cube-tet6.cgn"
# fi
# if [ -n "$h5import" ]; then
#   h5copy --enable-error-stack -i "$MESHDIR/cube-tet6p"$P"n"$N".cgns" \
#  -o "$MESHDIR/cube-tet6.cgn"
# fi
