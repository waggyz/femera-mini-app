#!/bin/bash
#
DIR="build/test/sm-md"
#mkdir -p build/test
mkdir -p $DIR
rm -f "$DIR/*"
rm -f "$DIR-time.err"
rm -f "$DIR-time.csv"

C=`tools/cpucount.sh`
GMSH_OMP="-nt $C" #TODO Not working on K?
#GMSH_OMP=""

MESH_N=150

P=2
H_MIN=2; # 7:10kdof
H_MAX=37; # WAS 33
# part_size in nodes, >0 will partition
PS=5000
#TODO stl ply2 wrl p3d?
MESH_I=0
for MESH_I in $(seq 1 1 $MESH_N) ; do
  case $(( ( $RANDOM * 10 ) / ( 32767 + 1 ) + 1 )) in
     1) FMT="geo_unrolled"; BIN="-geo" ;;
     2) FMT="msh4"; BIN="-asc" ;;
     3) FMT="msh4"; BIN="-bin" ;;
     4) FMT="msh2"; BIN="-asc" ;;
     5) FMT="msh2"; BIN="-bin" ;;
     6) FMT="cgns"; BIN="-bin" ;;
     7) FMT="mesh"; BIN="-asc" ;;
#     7) FMT="unv" ; BIN="-asc" ;; # CAD only? No p2?
     8) FMT="vtk" ; BIN="-bin" ;;
     9) FMT="vtk" ; BIN="-asc" ;;
    10) FMT="bdf" ; BIN="-asc" ;;
    *) echo oops ;;
  esac
  MESHFILE="$DIR/cube-$MESH_I""$BIN.$FMT"
  # Distribute evenly over log mesh size.
  H=`echo "e (l ($H_MIN) + $RANDOM/32767 * l ($H_MAX/$H_MIN))" | bc -l`
  H=`printf "%.5f" $H`; # Round it.
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
  /bin/time --format="%e" $EXEC 2>> "$DIR-time.err"
  if [ "$FMT" = "geo_unrolled" ] ; then
    echo "p=$P;" >> $MESHFILE
    cat "tests/geo/gmsh-opts.geo" >> $MESHFILE
  fi
done
#
