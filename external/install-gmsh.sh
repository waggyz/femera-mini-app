#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/lib64/libgmsh.a" ]; then
  echo "looks like Gmsh is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/gmsh"
  BUILD_DIR="$FMRDIR/build/external/gmsh"

  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  cmake $FLAGS "$SRC_DIR"
  make $JFLAG
  make install
fi
exit
