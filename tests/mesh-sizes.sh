#!/bin/bash
#
echo mesh-sizes.sh $1 $2 $3 $4 $5...
MESH_N="$1"
H_MIN="$2"
H_MAX="$3"
DIR="$4"
MESH_0="$5"
#
if [ "$MESH_N" -lt 1 ] ; then exit 1; fi
if [ "$H_MIN" -lt 1 ] ; then exit 1; fi
if [ "$H_MAX" -lt 1 ] ; then exit 1; fi
if [ -z "$DIR" ] ; then exit 1; fi

C=`tools/cpucount.sh`
if [ -z "$MESH_0" ] ; then # running serial...
  MESH_0=1;
  MESH_D=1;
  export OMP_NUM_THREADS=$C # running parallel...
  GMSH_OMP="-nt $C"
else
  MESH_D=$C;
  export OMP_NUM_THREADS=1
  GMSH_OMP="-nt 1"
fi
TIMEFILE="$DIR-time.err.$MESH_0"
#
mkdir -p $DIR
mkdir -p $DIR/stl
#rm -f "$DIR/*"
#rm -f "$DIR/stl/*"
#rm -f "$DIR-time.err"
#rm -f "$DIR-time.csv"

#H_MIN=2; # 7:10kdof
#H_MAX=38; # WAS 33

P=2
# TODO part_size in nodes, >0 partitions
PS=-5000;
#TODO ply2 wrl p3d?
#
OPTS_GEO=`cat "tests/geo/gmsh-opts.geo"`
BASE_GEO=`grep -v '^Include' "tests/geo/uhxt-cube.geo"`
STLTAIL="Surface Loop(1) = {1};\nVolume(1) = {1};\n"
STLTAIL=$STLTAIL"\np=$P;\n$OPTS_GEO\n"
for MESH_I in $(seq $MESH_0 $MESH_D $MESH_N) ; do
  case $(( ( $RANDOM * 10 ) / ( 32767 + 1 ) + 1 )) in
     1) FMT="geo"; BIN="-geo" ;;
     2) FMT="geo_unrolled"; BIN="-geo" ;;
     3) FMT="cgns"; BIN="-bin" ;;
     4) FMT="msh4"; BIN="-bin" ;;
     5) FMT="msh2"; BIN="-asc" ;;
     6) FMT="msh2"; BIN="-bin" ;;
     7) FMT="mesh"; BIN="-asc" ;;
#     7) FMT="unv" ; BIN="-asc" ;; # CAD only? No p2?
     8) FMT="vtk" ; BIN="-bin" ;;
     9) FMT="stl" ; BIN="-bin" ;;
    10) FMT="bdf" ; BIN="-asc" ;;
    *) echo oops ;;
  esac
  GEOFILE="tests/geo/uhxt-cube.geo"
  OUTFILE="$DIR/cube-$MESH_I""$BIN.$FMT"
  # Distribute evenly over log mesh size.
  H1=`echo "e (l ($H_MIN) + $RANDOM/32767 * l ($H_MAX/$H_MIN))" | bc -l`
  H2=`echo "$H1 + $RANDOM/32767 * 2 - 1" | bc -l`
  H1=`printf "%.5f" $H1`; # Round.
  H2=`printf "%.5f" $H2`;
  if [ "$FMT" = "geo" ] ; then
    echo $OUTFILE
    TS0=$(date +%s%N)
    printf "p=$P; h1=$H1; h2=$H2; part_size=$PS; mesh_d=-1;\n$OPTS_GEO\n$BASE_GEO" > "$OUTFILE"
    TS1=$(date +%s%N)
    ELAPSED=`echo "($TS1 - $TS0) * 10^-9" | bc -l`
    ELAPSED=`printf "%.9f" $ELAPSED`;
    echo "$MESH_I,$ELAPSED" >> "$TIMEFILE"
#    EXEC=printf "p=$P; h1=$H1; h2=$H2; part_size=$PS; mesh_d=-1;\n$OPTS_GEO\n$BASE_GEO" > "$OUTFILE"
#    /bin/time --format="$MESH_I,%e,%M,%t,%S,%U,%P" $EXEC 2>> "$TIMEFILE"
    # wall clock, max mem, avg mem use, kernel CPU-sec, user CPU-sec, %CPU
  else
    EXEC="gmsh $GMSH_OMP"
    EXEC=$EXEC" -setnumber p $P -setnumber h1 $H1 -setnumber h2 $H2"
    EXEC=$EXEC" -setnumber part_size $PS"
    MESHD=" -setnumber mesh_d 3"
    if [ "$FMT" = "geo_unrolled" ] ; then
      MESHD=" -setnumber mesh_d -1"
    fi
    if [ "$FMT" = "stl" ] ; then
      MESHD=" -setnumber mesh_d 2"
      RUNFILE="$DIR/cube-$MESH_I""-stl$BIN.geo"
      STLFILE="stl/cube-$MESH_I""$BIN.stl"
      printf "Merge '$STLFILE';\n$STLTAIL\n" >$RUNFILE
      OUTFILE="$DIR/$STLFILE"
    fi
    EXEC=$EXEC"$MESHD $GEOFILE"
    if [ "$BIN" = "-bin" ] ; then
      EXEC=$EXEC" -v 3 $BIN -save -o $OUTFILE"
    else
      EXEC=$EXEC" -v 3 -save -o $OUTFILE"
    fi
    echo $EXEC
    /bin/time --format="$MESH_I,%e,%M,%t,%S,%U,%P" $EXEC 2>> "$TIMEFILE"
    if [ "$FMT" = "geo_unrolled" ] ; then
      echo "p=$P;" >> $OUTFILE
      cat "tests/geo/gmsh-opts.geo" >> $OUTFILE
    fi
  fi
done
#
