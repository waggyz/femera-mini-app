#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # [ -f "$INSTALL_DIR/lib/libpybind11.a" ]; then
  echo "looks like pybind11 is already installed"
  exit 0
else
  echo " FIXME  Check lib test in external/install-pybind11.sh." >&2
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  #if [ -z "$JFLAG" ]; then # Make job limit
  #  J FLAG="-j`$FMRDIR/tools/fmrcores.sh`"
  #fi
  SRC_DIR="$FMRDIR/external/pybind11"
  BUILD_DIR="$FMRDIR/build/external/pybind11"

  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  eval cmake $FLAGS "$SRC_DIR"
  make $JFLAG
  make install
fi
exit
