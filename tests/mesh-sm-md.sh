#!/bin/bash
#
DIR="build/test/sm-md"
#mkdir -p build/test
mkdir -p $DIR

C=`tools/cpucount.sh`
GMSH_OMP="-nt $C" #TODO Not working on K?
#GMSH_OMP=""

SIM_N=50

P=2
H_MIN=7
H_MAX=33
# part_size in nodes, >0 will partition
PS=5000

SIM_I=0
for I in $(seq 1 1 $SIM_N) ; do
  case $(( ( $RANDOM * ( 5 + 1 ) ) / ( 32767 + 1 ) )) in
    0) FMT="cgns"; BIN="-bin" ;;
    1) FMT="msh2"; BIN="-bin" ;;
    2) FMT="msh4"; BIN="-bin" ;;
    3) FMT="msh2"; BIN="-asc" ;;
    4) FMT="msh4"; BIN="-asc" ;;
    5) FMT="geo_unrolled"; BIN="-geo" ;;
    *) echo oops ;;
  esac
    ((SIM_I+=1))
    MESHFILE="$DIR/cube-$SIM_I""$BIN.$FMT"
    # distribute evenly over log sim size
    H=`echo "e (l ($H_MIN) + $RANDOM/32767 * l ($H_MAX/$H_MIN))" | bc -l`
    EXEC="gmsh $GMSH_OMP"
    EXEC=$EXEC" -setnumber p $P -setnumber h $H -setnumber part_size $PS"
    if [ "$FMT" = "geo_unrolled" ] ; then
      EXEC=$EXEC" -setnumber mesh_d 0"
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
