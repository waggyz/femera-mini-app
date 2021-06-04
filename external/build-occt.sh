#!/bin/bash
# First argument is build directory
# Second argument is local installation directory
# Third argument contains occt cmake flags
#
export CC=`which gcc`
export CXX=`which g++`
export MPICC_CC=`which mpicc`
export MPICXX_CXX=`which mpic++`

FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
if [ -n "$1" ] ; then # Build directory
  BUILD_DIR="$1"
else
  BUILD_DIR="$FMRDIR/build/occt"
fi
if [ -n "$2" ] ; then # Installation directory
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
  -DBUILD_LIBRARY_TYPE=Static                  \
  -DCMAKE_BUILD_TYPE=Release                   \
  -DBUILD_MODULE_Draw=0                        \
  -DBUILD_MODULE_Visualization=0               \
  -DBUILD_MODULE_ApplicationFramework=0        \
  $3 "$BUILD_DIR"

make -j8
make install
