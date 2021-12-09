#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

#if which h5clear 2>/dev/null ; then
if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/lib/libhdf5.a" ]; then
  echo "looks like HDF5 is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/hdf5"
  BUILD_DIR="$FMRDIR/build/external/hdf5"

  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  cmake $FLAGS "$SRC_DIR"
  make $JFLAG
  make install

  cd examples
  make test
fi
exit
#==============================================================================
#wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.0/src/hdf5-1.12.0.tar.bz2 -P "build/"
#wget https://support.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.10.5.tar.bz2 -P "build/"

