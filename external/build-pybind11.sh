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
  BUILD_DIR="$FMRDIR/build/pybind11"
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
  -DBOOST_ROOT:PATHNAME=/usr/local/pkgs-modules/boost_1.66.0 \
  -DDOWNLOAD_CATCH=0 \
  "$FMRDIR/external/pybind11"

make -j8
make install
