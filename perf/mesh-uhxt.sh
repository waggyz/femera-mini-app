#!/bin/bash
H=$1; P=$2; N=$3; MESHDIR=$4; GMSH2FMR=$5;
C=`./cpucount.sh`
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
if [ ! -f $DIR"/uhxt"$H"p"$P"n1.msh2" ]; then
  gmsh -v 3 -setnumber p $P -setnumber h $H -setnumber n 1 -nt $C geo/unst-cube.geo -
  mv "cube/uhxt"$H"p"$P"n1.msh2" $DIR"/uhxt"$H"p"$P"n1.msh2"
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n"$N".msh" ]; then
  gmsh -v 3 -part $N -nt $C -format msh2 -o $DIR"/uhxt"$H"p"$P"n"$N".msh" $DIR"/uhxt"$H"p"$P"n1.msh2" -
fi
if [ ! -f $DIR"/uhxt"$H"p"$P"n"$N"_1.fmr" ]; then
  $GMSH2FMR -v1 -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -M0 -E100e9 -N0.3 -R -v3 -a $DIR"/uhxt"$H"p"$P"n"$N
fi
