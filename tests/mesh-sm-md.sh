#!/bin/bash
#
DIR="build/test/sm-md"
#mkdir -p build/test
mkdir -p $DIR

C=`tools/cpucount.sh`
GMSH_OMP="-nt $C" #TODO Not working on K?
#GMSH_OMP=""

MESH_N=50

P=2
H_MIN=7
H_MAX=33
# part_size in nodes, >0 will partition
PS=5000

MESH_I=0
for MESH_I in $(seq 1 1 $MESH_N) ; do
  case $(( ( $RANDOM * 6 ) / ( 32767 + 1 ) + 1)) in
    1) FMT="cgns"; BIN="-bin" ;;
    2) FMT="msh2"; BIN="-bin" ;;
    3) FMT="msh4"; BIN="-bin" ;;
    4) FMT="msh2"; BIN="-asc" ;;
    5) FMT="msh4"; BIN="-asc" ;;
    6) FMT="geo_unrolled"; BIN="-geo" ;;
    *) echo oops ;;
  esac
  MESHFILE="$DIR/cube-$MESH_I""$BIN.$FMT"
  # Distribute evenly over log mesh size.
  H=`echo "e (l ($H_MIN) + $RANDOM/32767 * l ($H_MAX/$H_MIN))" | bc -l`
  EXEC="gmsh $GMSH_OMP"
  EXEC=$EXEC" -setnumber p $P -setnumber h $H -setnumber part_size $PS"
  if [ "$FMT" = "geo_unrolled" ] ; then
    EXEC=$EXEC" -setnumber mesh_d -1"
  else
    EXEC=$EXEC" -setnumber mesh_d 3"
  fi
  EXEC=$EXEC" tests/geo/uhxt-cube.geo"
  if [ "$BIN" = "-bin" ] ; then
    EXEC=$EXEC" -v 3 $BIN -save -o $MESHFILE"
  else
    EXEC=$EXEC" -v 3 -save -o $MESHFILE"
  fi
  echo $EXEC
  $EXEC
  #/usr/bin/time $EXEC
done
#
