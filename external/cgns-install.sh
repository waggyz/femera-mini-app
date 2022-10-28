#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/lib/libcgns_static.a" ]; then
  echo "looks like CGNS is already installed"
  exit 0
else
  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/cgns"
  BUILD_DIR="$FMRDIR/build/external/cgns"
  mkdir -p "$BUILD_DIR"


  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  export CFLAGS="-Doff64_t=__off64_t " #TODO Check if this is needed.
  export LDFLAGS=" -L$INSTALL_DIR/lib64 -L$INSTALL_DIR/lib"

  MPIDIR=$(cd "$(dirname `which mpicc`)/.."; pwd)
  export CMAKE_PREFIX_PATH="$INSTALL_DIR;$MPIDIR"
  export CMAKE_INCLUDE_PATH="$INSTALL_DIR/include;$MPIDIR/include"
  export CMAKE_LIBRARY_PATH="$INSTALL_DIR/lib64;$INSTALL_DIR/lib;$MPIDIR/lib"

  # in: $FMRDIR/external/cgns/CMakeLists.txt
  #Change: 217:    # add_executable (h5dump IMPORTED)
  if [ 1 -eq 1 ]; then
    cp -p "$FMRDIR/external/cgns/CMakeLists.txt" "$FMRDIR/external/cgns/CMakeLists.orig"
    sed -i '/h5dump IMPORTED/s/^/#/' "$FMRDIR/external/cgns/CMakeLists.txt"
  fi
  # Create CMake initial cache.
  CCONFIG="$BUILD_DIR/CMakeConfig.cmake"
  echo "" > $CCONFIG
  echo "set( HDF5_NEED_ZLIB ON CACHE BOOL \"\")" >> $CCONFIG
  echo "set( HDF5_NEED_SZIP ON CACHE BOOL \"\")" >> $CCONFIG
  echo "set( HDF5_NEED_MPI ON CACHE BOOL \"\")" >> $CCONFIG
  echo "set( CGNS_ENABLE_PARALLEL ON CACHE BOOL \"\")" >> $CCONFIG

  # Preset some CMake variables.
  echo "set( MPI_C_COMPILER \"`which mpicc`\" )" >> $CCONFIG
  echo "set( BUILD_SHARED_LIBS ON )" >> $CCONFIG
  echo "set( HDF5_IS_PARALLEL ON )" >> $CCONFIG
  echo "set( HDF5_ENABLE_PARALLEL ON )" >> $CCONFIG

  cd "$BUILD_DIR"
  cmake -C "$CCONFIG" $FLAGS "$SRC_DIR"
  make $JFLAG
  make install
  cd "$FMRDIR"

  if [ -f "$FMRDIR/external/cgns/CMakeLists.orig" ]; then
    rm "$FMRDIR/external/cgns/CMakeLists.txt"
    mv "$FMRDIR/external/cgns/CMakeLists.orig" \
      "$FMRDIR/external/cgns/CMakeLists.txt"
  fi

  if [ 1 -eq 0 ]; then
    # NOTE ptests expects to find libcgns_static.a.
    rm -f $INSTALL_DIR/lib/libcgns_static.a
    ln -s $INSTALL_DIR/lib/libcgns.a $INSTALL_DIR/lib/libcgns_static.a
    export CC=`which mpicc`
    export CFLAGS="-I$INSTALL_DIR/include -L$MPIDIR -lhdf5 -lmpi"
    export LD_LIBRARY_PATH
    mkdir -p "$BUILD_DIR/ptests"
    if [ "`which lfs 2>/dev/null`" ]; then # Parallel filesystem
    lfs setstripe -c 16 -S 1m ptests
    fi
    cd "$BUILD_DIR/ptests"
    cmake -Wno-dev                             \
      -DCMAKE_INSTALL_DIR="$INSTALL_DIR" \
      -DMPIEXEC=`which mpiexec`                \
      "$FMRDIR/external/cgns/src/ptests"

    make
    ./pcgns_ctest             # Run serial, then
    mpiexec -np 4 pcgns_ctest # compare to parallel.
    cd "$FMRDIR"
  fi
fi
exit

# Requirements for pyCGNS http://pycgns.sourceforge.net/install.html
#    Python (starting from v2.4)
#    numpy (v1.1 +)
#    cython (v0.16 +)
#    hdf5 (v1.8.5 +)
#    CHLone (v0.4 +)
#    VTK (v5.8 +)
#    Qt (v4.7 +)
#    pySide (v4.7 +)
#
# Parallel file access requires MPI.
# Fortran 2003
