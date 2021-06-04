#!/bin/bash
# First argument is build directory
# Second argument is local installation directory
# Third argument contains gmsh cmake flags
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
  -DCMAKE_PREFIX_PATH="$INSTALL_PREFIX"        \
  -DENABLE_BUILD_DYNAMIC=1                     \
  -DENABLE_BUILD_LIB=1                         \
  $3 "$FMRDIR/external/gmsh"

make -j8
make install
