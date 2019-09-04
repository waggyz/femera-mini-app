#!/bin/bash
#
echo 0 $0 exists.
ME=$(basename -- "$0")
#
GMSH_FULLPATH=`which gmsh`
if [ -z "$GMSH_FULLPATH" ]; then
  echo 1 $ME:${LINENO} gmsh not found in your path.
  exit
else
  echo 0 $ME:${LINENO} Found "$GMSH_FULLPATH".
fi
#
GMSH_VERSION=`gmsh --version 2> >(cat)`;
if [ -z "$GMSH_VERSION" ]; then
  echo 1 $ME:${LINENO} gmsh --version does not work as expected.
else
  echo 0 $ME:${LINENO} Using gmsh version "$GMSH_VERSION".
fi
#
FILE="geo/unst-cube.geo"
if [ -f "$FILE" ]; then
  echo 0 $ME:${LINENO} Geometry file $FILE exists.
else
  echo 1 $ME:${LINENO} Geometry file $FILE does not exist.
fi
#
for N in 1 16; do
  P=2 ; H=9 ; C=`./cpucount.sh`
  FILE="cube/uhxt"$H"p"$P"n"$N".msh2"
  ERRFILE="unit-test/gmsh.err"
  rm -f $FILE
  gmsh -setnumber p $P -setnumber h $H -setnumber n $N -nt $C geo/unst-cube.geo -\
  > unit-test/gmsh.log 2>$ERRFILE
  #
  ERR=`grep -i error $ERRFILE`
  if [ -z "$ERR" ]; then
    echo 0 $ME:${LINENO} Gmsh executed without errors.
  else
    echo 1 $ME:${LINENO} $ERRFILE contains errors.
  fi
  #
  if [ -f "$FILE" ]; then
    echo 0 $ME:${LINENO} Gmsh created $FILE.
  else
    echo 1 $ME:${LINENO} Mesh file $FILE does not exist.
    exit
  fi
  #
  ELEMS=`grep -A1 -m1 Elements $FILE | tail -n1`
  NODES=`grep -A1 -m1 Nodes $FILE | tail -n1`
  if [ $(($ELEMS * $NODES)) -gt 0 ]; then
    echo 0 $ME:${LINENO} $FILE contains $ELEMS elements and $NODES nodes.
  else
    echo 1 $ME:${LINENO} $FILE contains $ELEMS elements and $NODES nodes.
  fi
done
#

