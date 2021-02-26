#!/bin/bash
# NOTE Requires MPI and HDF5 to be installed.
# MAYBE requires CMake 3.12 (3.15 recommended)

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


# Build parallel for Femera.

cd "$BUILD_DIR"
cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
  -DHDF5_BUILD_CPP_LIB:BOOL=OFF       \
  -DHDF5_ENABLE_PARALLEL:BOOL=ON      \
  -DMPIEXEC_MAX_NUMPROCS:STRING=4     \
  -DBUILD_STATIC_LIBS:BOOL=ON         \
  -DBUILD_SHARED_LIBS:BOOL=ON         \
  -DCTEST_BUILD_CONFIGURATION=Release \
  -DHDF5_NO_PACKAGES:BOOL=ON          \
  "$FMRDIR/external/hdf5"

make -j8 # NOTE -j is unable to fork enough on a k-cluster login node.
make install

cd examples
make test

#==============================================================================
#wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.bz2 -P "build/"
#wget https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.10.5.tar.bz2 -P "build/"
