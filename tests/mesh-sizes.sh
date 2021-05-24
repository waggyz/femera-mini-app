#!/bin/bash
#
echo mesh-sizes.sh $1 $2 $3 $4...
MESH_N="$1"
H_MIN="$2"
H_MAX="$3"
DIR="$4"
#
if [ "$MESH_N" -lt 1 ] ; then exit 1; fi
if [ "$H_MIN" -lt 1 ] ; then exit 1; fi
if [ "$H_MAX" -lt 1 ] ; then exit 1; fi
if [ -z "$DIR" ] ; then exit 1; fi
#
mkdir -p $DIR
rm -f "$DIR/*"
rm -f "$DIR-time.err"
rm -f "$DIR-time.csv"

C=`tools/cpucount.sh`
export OMP_NUM_THREADS=$C
GMSH_OMP="-nt $C" #TODO Not working on K?
#GMSH_OMP=""

#MESH_N=150
#H_MIN=2; # 7:10kdof
#H_MAX=38; # WAS 33

P=2
# TODO part_size in nodes, >0 partitions
PS=-5000;
#TODO stl ply2 wrl p3d?
#
OPTS_GEO=`cat "tests/geo/gmsh-opts.geo"`
BASE_GEO=`grep -v '^Include' "tests/geo/uhxt-cube.geo"`
MESH_I=0
for MESH_I in $(seq 1 1 $MESH_N) ; do
  case $(( ( $RANDOM * 10 ) / ( 32767 + 1 ) + 1 )) in
     1) FMT="geo"; BIN="-geo" ;;
     2) FMT="geo_unrolled"; BIN="-geo" ;;
     3) FMT="cgns"; BIN="-bin" ;;
     4) FMT="msh4"; BIN="-asc" ;;
     5) FMT="msh4"; BIN="-bin" ;;
#     4) FMT="msh2"; BIN="-asc" ;;
     6) FMT="msh2"; BIN="-bin" ;;
     7) FMT="mesh"; BIN="-asc" ;;
#     7) FMT="unv" ; BIN="-asc" ;; # CAD only? No p2?
     8) FMT="vtk" ; BIN="-bin" ;;
     9) FMT="vtk" ; BIN="-asc" ;;
    10) FMT="bdf" ; BIN="-asc" ;;
    *) echo oops ;;
  esac
  MESHFILE="$DIR/cube-$MESH_I""$BIN.$FMT"
  # Distribute evenly over log mesh size.
  H1=`echo "e (l ($H_MIN) + $RANDOM/32767 * l ($H_MAX/$H_MIN))" | bc -l`
  H2=`echo "$H1 + $RANDOM/32767 * 2 - 1" | bc -l`
  H1=`printf "%.5f" $H1`; # Round.
  H2=`printf "%.5f" $H2`;
  if [ "$FMT" = "geo" ] ; then
    echo $MESHFILE
    TS0=$(date +%s%N)
    printf "p=$P; h1=$H1; h2=$H2; part_size=$PS; mesh_d=-1;\n$OPTS_GEO\n$BASE_GEO" > "$MESHFILE"
    TS1=$(date +%s%N)
    ELAPSED=`echo "($TS1 - $TS0) * 10^-9" | bc -l`
    ELAPSED=`printf "%.9f" $ELAPSED`;
    echo "$ELAPSED" >> "$DIR-time.err"
#    EXEC=printf "p=$P; h1=$H1; h2=$H2; part_size=$PS; mesh_d=-1;\n$OPTS_GEO\n$BASE_GEO" > "$MESHFILE"
#    /bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC 2>> "$DIR-time.err"
    # wall clock, max mem, avg mem use, kernel CPU-sec, user CPU-sec, %CPU
  else
    EXEC="gmsh $GMSH_OMP"
    EXEC=$EXEC" -setnumber p $P -setnumber h1 $H1 -setnumber h2 $H2"
    EXEC=$EXEC" -setnumber part_size $PS"
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
    /bin/time --format="%e,%M,%t,%S,%U,%P" $EXEC 2>> "$DIR-time.err"
    if [ "$FMT" = "geo_unrolled" ] ; then
      echo "p=$P;" >> $MESHFILE
      cat "tests/geo/gmsh-opts.geo" >> $MESHFILE
    fi
  fi
done
#
