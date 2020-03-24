#!/bin/bash
VERB=1
case "$#" in
  4) H=$1; P=$2; N=$3; MESHDIR=$4; PHYS="elas-iso"; ;;
  5) H=$1; P=$2; N=$3; C=$4; MESHDIR=$5; PHYS="elas-iso"; ;;
  6) H=$1; P=$2; N=$3; C=$4; PHYS=$5; MESHDIR=$6; ;;
  7) H=$1; P=$2; SX=$3; SY=$4; SZ=$5; PHYS=$6; MESHDIR=$7;
     N=$(( $SX * $SY * $SZ )); ;;
  *) echo "Please supply 4-7 arguments."; exit 2; ;;
esac
#if [ -n "$7" ]; then LOGFILE=">>"$7; else LOGFILE=""; fi
#
PERFDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )";
EXEDIR=$( cd "$PERFDIR"; cd ../; pwd )
#
CPUMODEL=`"$EXEDIR"/cpumodel.sh`
CPUCOUNT=`"$EXEDIR"/cpucount.sh`
GMSH2FMR="$EXEDIR"/"gmsh2fmr-"$CPUMODEL"-gcc"
if [ -e "$C" ]; then C=$CPUCOUNT; fi
#
DIR="$MESHDIR"/"uhxt"$H"p"$P
if [ ! -d "$DIR" ]; then
  mkdir "$DIR"
fi
if [ $H -lt 75 ]; then
  if [ ! -f "$DIR""/uhxt"$H"p"$P"n1.msh" ]; then
    if [ -f "$DIR""/uhxt"$H"p"$P"n.msh" ]; then
      cp "$DIR""/uhxt"$H"p"$P"n.msh" "$DIR""/uhxt"$H"p"$P"n1.msh"
    fi
  fi
fi
export OMP_SCHEDULE=static
export OMP_PLACES=cores
export OMP_PROC_BIND=spread
export OMP_NUM_THREADS=$CPUCOUNT
if [ ! -f "$DIR""/uhxt"$H"p"$P"n.msh" ]; then
  echo "Meshing uhxt"$H"p"$P"n.msh..." "$LOGFILE"
  gmsh -nt $CPUCOUNT -v $VERB -setnumber p $P -setnumber h $H -setnumber n 1 -3 \
    -format msh2 -o "$DIR""/uhxt"$H"p"$P"n.msh" geo/uhxt-cube.geo -save
fi
if [ $N -gt 0 ]; then
  if [ -n "$SX" ]; then
    PARTSTR="slicing "$SX"x"$SY"x"$SZ""
    NN=""
    SLICEARG="-xS"$SX" -yS"$SY" -zS"$SZ
  else
    if [ $N -lt 2000 ]; then
      PARTSTR="METIS"
      if [ $N -gt $C ]; then
        N=$(( $N / $C * $C ))
      fi
      NN=$N
      SLICEARG=""
    else
      PARTSTR="slicing "$SX"x"$SY"x"$SZ""
      NXYZ=($(python perf/part_slice_xyz.py -n $N -c $C))
      N=${NXYZ[0]}
      SX=${NXYZ[1]}
      SY=${NXYZ[2]}
      SZ=${NXYZ[3]}
      NN=""
      SLICEARG="-xS"$SX" -yS"$SY" -zS"$SZ
    fi
  fi
  if [ ! -f "$DIR""/uhxt"$H"p"$P"n"$N".msh" ]; then
    if [ "$PARTSTR" = "METIS" ]; then
      echo "Partitioning to uhxt"$H"p"$P"n"$N".msh ("$PARTSTR")..." "$LOGFILE"
      gmsh -v $VERB -nt $CPUCOUNT -part $N \
        -format msh2 -o "$DIR""/uhxt"$H"p"$P"n"$N".msh" \
        "$DIR""/uhxt"$H"p"$P"n.msh" -save
    #else
    #  gmsh -v $VERB -nt $CPUCOUNT -setnumber n $N \
    #    -setnumber sx $SX -setnumber sy $SY -setnumber sz $SZ \
    #    -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" \
    #    -merge $DIR"/uhxt"$H"p"$P"n.msh" geo/simplepart.geo -save
    fi
  fi
  if [ ! -f "$DIR""/uhxt"$H"p"$P"n"$N"_1.fmr" ]; then
    echo "Converting to uhxt"$H"p"$P"n"$N"_*.fmr..." "$LOGFILE"
    case $PHYS in
    elas-iso)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    elas-ort)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    elas-dmv)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -D -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    thel-iso)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    thel-ort)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    plas-iso)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    ther-iso)
      export OMP_NUM_THREADS=1
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -x@1.0 -xu10 \
        $SLICEARG -M0 -K100e-6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    ther-ort)
      export OMP_NUM_THREADS=1
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -x@1.0 -xu10 \
        $SLICEARG -M0 -K100e-6 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
    ;;
    *)
      "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
    esac
  else
    # Convert existing .fmr files, if necessary.
    IS_ELAST=`grep -i Elast  "$DIR""/uhxt"$H"p"$P"n"$N"_1.fmr"`
    IS_ORTHO=`grep -i Orient "$DIR""/uhxt"$H"p"$P"n"$N"_1.fmr"`
    IS_THERM=`grep -i Therm  "$DIR""/uhxt"$H"p"$P"n"$N"_1.fmr"`
    IS_PLAST=`grep -i Plast  "$DIR""/uhxt"$H"p"$P"n"$N"_1.fmr"`
    if [ -n "$IS_ELAST" ]; then
      if [ -n "$IS_ORTHO" ]; then
        if [ -n "$IS_THERM" ]; then
          IS_PHYS=thel-ort
        elif [ -n "$IS_PLAST" ]; then
          IS_PHYS=plas-ort
        else
          IS_PHYS=elas-ort
        fi
      else
        if [ -n "$IS_THERM" ]; then
          IS_PHYS=thel-iso
        elif [ -n "$IS_PLAST" ]; then
          IS_PHYS=plas-iso
        else
          IS_PHYS=elas-iso
        fi
      fi
    else
      if [ -n "$IS_ORTHO" ]; then
        if [ -n "$IS_THERM" ]; then
          IS_PHYS=ther-ort
        fi
      else
        if [ -n "$IS_THERM" ]; then
          IS_PHYS=ther-iso
        fi
      fi
    fi
    echo "uhxt"$H"p"$P"n"$N"_1.fmr physics is "$IS_PHYS"." "$LOGFILE"
    if [ "$IS_PHYS" != "$PHYS" ]; then
      echo "Converting to "$PHYS" physics..." "$LOGFILE"
      #FIXME replace this with shell scripts to modify physics blocks.
      case $PHYS in
      elas-iso)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
          $SLICEARG -M0 -E100e9 -N0.3 -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      elas-ort)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
          $SLICEARG -M0 -E100e9 -N0.3 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      elas-dmv)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
          $SLICEARG -M0 -E100e9 -N0.3 -D -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      thel-iso)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
          $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      thel-ort)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
          $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      ther-iso)
      export OMP_NUM_THREADS=1
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -x@1.0 -xu10 \
          $SLICEARG -M0 -K100e-6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      ther-ort)
      export OMP_NUM_THREADS=1
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -x@1.0 -xu10 \
          $SLICEARG -M0 -K100e-6 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      plas-iso)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
          $SLICEARG -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a "$DIR""/uhxt"$H"p"$P"n"$NN
      ;;
      *)
        "$GMSH2FMR" -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
          $SLICEARG -M0 -E100e9 -N0.3 -R -a "$DIR""/uhxt"$H"p"$P"n"$NN
      esac
    fi
  fi
fi