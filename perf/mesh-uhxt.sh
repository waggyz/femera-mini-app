#!/bin/bash
H=$1; P=$2; N=$3; MESHDIR=$4; GMSH2FMR=$5;
if [ -n "$6" ]; then PHYS=$6; else PHYS=elas-ort; fi
if [ -n "$7" ]; then LOGFILE=$7; else LOGFILE=""; fi
C=`./cpucount.sh`
VERB=1
#
DIR=$MESHDIR"/uhxt"$H"p"$P
if [ ! -d $DIR ]; then
    mkdir $DIR
fi
#if [ -f $MESHDIR"/uhxt"$H"p"$P"n1.msh2" ]; then
#  #mv $MESHDIR"/uhxt"$H"p"$P"n"* $DIR
#  #echo $MESHDIR"/uhxt"$H"p"$P"n"* | xargs mv -t $DIR
#  find $MESHDIR -mindepth 1 -maxdepth 1 -name "uhxt$Hp$Pn*" -exec mv -t $DIR {} +
#fi
#if [ -f $DIR"/uhxt"$H"p"$P"n1.msh2" ]; then
#  mv $DIR"/uhxt"$H"p"$P"n1.msh2" $DIR"/uhxt"$H"p"$P"n1.msh"
#fi
#if [ -f $DIR"/uhxt"$H"p"$P"n"$N".msh2" ]; then
#  mv $DIR"/uhxt"$H"p"$P"n"$N".msh2" $DIR"/uhxt"$H"p"$P"n"$N".msh"
#fi
if [ -f $DIR"/uhxt"$H"p"$P"n1.msh" ]; then
  mv $DIR"/uhxt"$H"p"$P"n1.msh" $DIR"/uhxt"$H"p"$P"n.msh"
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n.msh" ]; then
  if [ -n "$LOGFILE" ]; then
    echo "Meshing uhxt"$H"p"$P"n.msh..." >> "$LOGFILE"
  else
    echo "Meshing uhxt"$H"p"$P"n.msh..."
  fi
  gmsh -nt $C -v $VERB -setnumber p $P -setnumber h $H -setnumber n 1 -3 \
    -format msh2 -o $DIR"/uhxt"$H"p"$P"n.msh" -save geo/uhxt-cube.geo
fi
PARTSTR="METIS"
if [ $N -lt 1000 ]; then
  N=$(( $N / $C * $C ))
  NN=$N
  SLICEARG=""
else
  NXYZ=($(python perf/part_slice_xyz.py -n $N -c $C))
  N=${NXYZ[0]}
  SX=${NXYZ[1]}
  SY=${NXYZ[2]}
  SZ=${NXYZ[3]}
  PARTSTR="slicing "$SX"x"$SY"x"$SZ""
  NN=""
  SLICEARG="-xS"$SX" -yS"$SY" -zS"$SZ
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n"$N".msh" ]; then
  if [ $N -lt 1000 ]; then
    if [ -n "$LOGFILE" ]; then
      echo "Partitioning to uhxt"$H"p"$P"n"$N".msh ("$PARTSTR")..." >> "$LOGFILE"
    else
      echo "Partitioning to uhxt"$H"p"$P"n"$N".msh ("$PARTSTR")..."
    fi
    gmsh -v $VERB -nt $C -part $N \
      -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" \
      $DIR"/uhxt"$H"p"$P"n.msh"
  #else
  #  gmsh -v $VERB -nt $C -setnumber n $N \
  #    -setnumber sx $SX -setnumber sy $SY -setnumber sz $SZ \
  #    -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" \
  #    -merge $DIR"/uhxt"$H"p"$P"n.msh" -save geo/simplepart.geo
  fi
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n"$N"_1.fmr" ]; then
  if [ -n "$LOGFILE" ]; then
    echo "Converting to uhxt"$H"p"$P"n"$N"_*.fmr..." >> "$LOGFILE"
  else
    echo "Converting to uhxt"$H"p"$P"n"$N"_*.fmr..."
  fi
  case $PHYS in
  elas-iso)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      $SLICEARG -M0 -E100e9 -N0.3 -a $DIR"/uhxt"$H"p"$P"n"$NN
  ;;
  elas-ort)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      $SLICEARG -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
  ;;
  ther-iso)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
      $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a $DIR"/uhxt"$H"p"$P"n"$NN
  ;;
  ther-ort)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
      $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
  ;;
  plas-iso)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      $SLICEARG -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a $DIR"/uhxt"$H"p"$P"n"$NN
  ;;
  *)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      $SLICEARG -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
  esac
else
  # Convert existing .fmr files, if necessary.
  IS_ORTHO=`grep -i Orient $DIR"/uhxt"$H"p"$P"n"$N"_1.fmr"`
  IS_THERM=`grep -i Therm  $DIR"/uhxt"$H"p"$P"n"$N"_1.fmr"`
  IS_PLAST=`grep -i Plast  $DIR"/uhxt"$H"p"$P"n"$N"_1.fmr"`
  if [ -n "$IS_ORTHO" ]; then
    if [ -n "$IS_THERM" ]; then
      IS_PHYS=ther-ort
    elif [ -n "$IS_PLAST" ]; then
      IS_PHYS=plas-ort
    else
      IS_PHYS=elas-ort
    fi
  else
    if [ -n "$IS_THERM" ]; then
      IS_PHYS=ther-iso
    elif [ -n "$IS_PLAST" ]; then
      IS_PHYS=plas-iso
    else
      IS_PHYS=elas-iso
    fi
  fi
  if [ -n "$LOGFILE" ]; then
    echo "uhxt"$H"p"$P"n"$N"_1.fmr physics is "$IS_PHYS"." >> "$LOGFILE"
  else
    echo "uhxt"$H"p"$P"n"$N"_1.fmr physics is "$IS_PHYS"."
  fi
  if [ $IS_PHYS != $PHYS ]; then
    if [ -n "$LOGFILE" ]; then
      echo "Converting to "$PHYS" physics..." >> "$LOGFILE"
    else
      echo "Converting to "$PHYS" physics..."
    fi
    #FIXME replace this with shell scripts to modify physics blocks.
    case $PHYS in
    elas-iso)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -a $DIR"/uhxt"$H"p"$P"n"$NN
    ;;
    elas-ort)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
    ;;
    ther-iso)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a $DIR"/uhxt"$H"p"$P"n"$NN
    ;;
    ther-ort)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        $SLICEARG -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
    ;;
    plas-iso)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a $DIR"/uhxt"$H"p"$P"n"$NN
    ;;
    *)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        $SLICEARG -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$NN
    esac
  fi
fi
