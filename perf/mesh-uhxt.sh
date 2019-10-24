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
if [ ! -f $DIR"/uhxt"$H"p"$P"n1.msh" ]; then
  if [ -n "$LOGFILE" ]; then
    echo "Meshing uhxt"$H"p"$P"n1.msh..." >> "$LOGFILE"
  else
    echo "Meshing uhxt"$H"p"$P"n1.msh..."
  fi
  gmsh -nt $C -v $VERB -setnumber p $P -setnumber h $H -setnumber n 1 -3 \
    -format msh2 -o $DIR"/uhxt"$H"p"$P"n1.msh" -save geo/uhxt-cube.geo
fi
if [ $N -ge 1000 ]; then
  NXYZ=($(python perf/part_slice_xyz.py -n $N -c $C))
  N=${NXYZ[0]}
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n"$N".msh" ]; then
  if [ -n "$LOGFILE" ]; then
    echo "Partitioning to uhxt"$H"p"$P"n"$N".msh..." >> "$LOGFILE"
  else
    echo "Partitioning to uhxt"$H"p"$P"n"$N".msh..."
  fi
  if [ $N -lt 1000 ]; then
    gmsh -v $VERB -nt $C -part $N \
      -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" \
      $DIR"/uhxt"$H"p"$P"n1.msh"
  else
    gmsh -v $VERB -nt $C -setnumber n $N \
      -setnumber sx ${NXYZ[1]} -setnumber sy ${NXYZ[2]} -setnumber sz ${NXYZ[3]} \
      -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" \
      -merge $DIR"/uhxt"$H"p"$P"n1.msh" -save geo/simplepart.geo
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
      -M0 -E100e9 -N0.3 -a $DIR"/uhxt"$H"p"$P"n"$N
  ;;
  elas-ort)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$N
  ;;
  ther-iso)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
      -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a $DIR"/uhxt"$H"p"$P"n"$N
  ;;
  ther-ort)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
      -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a $DIR"/uhxt"$H"p"$P"n"$N
  ;;
  plas-iso)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a $DIR"/uhxt"$H"p"$P"n"$N
  ;;
  *)
    $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
      -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$N
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
        -M0 -E100e9 -N0.3 -a $DIR"/uhxt"$H"p"$P"n"$N
    ;;
    elas-ort)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$N
    ;;
    ther-iso)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -a $DIR"/uhxt"$H"p"$P"n"$N
    ;;
    ther-ort)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -x@1.0 -Tu10 \
        -M0 -E100e9 -N0.3 -A20e-6 -K100e-6 -R -a $DIR"/uhxt"$H"p"$P"n"$N
    ;;
    plas-iso)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        -M0 -E100e9 -N0.3 -J300e6 -J100e6 -a $DIR"/uhxt"$H"p"$P"n"$N
    ;;
    *)
      $GMSH2FMR -v$VERB -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 \
        -M0 -E100e9 -N0.3 -R -a $DIR"/uhxt"$H"p"$P"n"$N
    esac
  fi
fi
