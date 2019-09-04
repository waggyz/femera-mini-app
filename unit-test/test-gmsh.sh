#!/bin/bash
PASS_COLOR="\e[92m"
INFO_COLOR="\e[94m"
NOTE_COLOR="\e[96m"
WARN_COLOR="\e[93m\e[1m"
FAIL_COLOR="\e[91m\e[1m\e[7m"
NORM_COLOR="\e[0m"
#
PASS="$PASS_COLOR Pass $NORM_COLOR"
INFO="$INFO_COLOR Info $NORM_COLOR"
NOTE="$NOTE_COLOR Note $NORM_COLOR"
WARN="$WARN_COLOR WARN $NORM_COLOR"
FAIL="$FAIL_COLOR FAIL $NORM_COLOR"
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
for N in 1 4; do
  P=2 ; H=9 ; C=`./cpucount.sh`
  FILE="cube/uhxt"$H"p"$P"n"$N
  rm -f $FILE".msh2"
  rm -f $FILE"_*.msh"
  LOGFILE="unit-test/gmsh.log"
  ERRFILE="unit-test/gmsh.err"
  gmsh -setnumber p $P -setnumber h $H -setnumber n $N -nt $C geo/unst-cube.geo -\
   > $LOGFILE 2>$ERRFILE
  #
  ERR=`grep -i error $ERRFILE`
  if [ -z "$ERR" ]; then
    echo 0 $ME:${LINENO} Gmsh executed without errors.
  else
    echo 1 $ME:${LINENO} $ERRFILE contains errors.
  fi
  #
  if [ $N -eq 1 ]; then
    MSHFILE=$FILE".msh2"
    if [ -f "$MSHFILE" ]; then
      echo 0 $ME:${LINENO} Gmsh created $MSHFILE.
    else
      echo 1 $ME:${LINENO} Mesh file $MSHFILE does not exist.
      exit
    fi
  else
    for I in `seq 1 $N`; do
      MSHFILE=$FILE"_"$I".msh"
      if [ -f "$MSHFILE" ]; then
        echo 0 $ME:${LINENO} Gmsh created $MSHFILE.
      else
        echo 1 $ME:${LINENO} Mesh file $MSHFILE does not exist.
        exit
      fi
    done
  fi
  #
  ELEMS=`grep -A1 -m1 Elements $MSHFILE | tail -n1`
  NODES=`grep -A1 -m1 Nodes $MSHFILE | tail -n1`
  if [ $(($ELEMS * $NODES)) -gt 0 ]; then
    echo 0 $ME:${LINENO} $MSHFILE contains $ELEMS elements and $NODES nodes.
  else
    echo 1 $ME:${LINENO} $MSHFILE contains $ELEMS elements and $NODES nodes.
  fi
done
#
STR=`grep -ioE -m1 '[0-9]+ thread' $LOGFILE`
NT=`echo $STR | sed 's@^[^0-9]*\([0-9]\+\).*@\1@'`
if [ $NT -le 1 ]; then
  printf "0 $ME:${LINENO}$NOTE""Gmsh is using only $STR.\n"
  printf "0 $ME:${LINENO}$INFO""Compile gmsh with OpenMP for parallel meshing.\n"
else
  echo "0 Gmsh can use up to "$STR"s."
fi
#

