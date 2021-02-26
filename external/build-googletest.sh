#!/bin/bash
# Argument is local installation directory
#
export CC=`which gcc`
export CXX=`which g++`
export MPICC_CC=`which mpicc`
export MPICXX_CXX=`which mpic++`

FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
if [ -n "$1" ]; then # Build directory
  BUILD_DIR="$1"
else
  BUILD_DIR="$FMRDIR/build/gmsh"
fi
if [ -n "$2" ]; then # Installation directory
  INSTALL_PREFIX="$2"
else
  INSTALL_PREFIX="$FMRDIR/build/external"
fi

export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib64:$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"

mkdir -p "$INSTALL_PREFIX"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
  "$FMRDIR/external/googletest"

make -j8
make install
