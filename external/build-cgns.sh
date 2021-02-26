#!/bin/bash
# Arguments are build and installation directories
#
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

# in: $FMRDIR/external/CGNS/CMakeLists.txt
#Change: 217:    # add_executable (h5dump IMPORTED)
if [ 1 -eq 1 ]; then
  cp -p "$FMRDIR/external/CGNS/CMakeLists.txt" "$FMRDIR/external/CGNS/CMakeLists.orig"
  sed -i '/h5dump IMPORTED/s/^/#/' "$FMRDIR/external/CGNS/CMakeLists.txt"
fi

mkdir -p "$INSTALL_PREFIX"
mkdir -p "$BUILD_DIR"

export CC=`which gcc`
export CFLAGS="-Doff64_t=__off64_t " #TODO Check if this is needed.
export LDFLAGS=" -L$INSTALL_PREFIX/lib64 -L$INSTALL_PREFIX/lib"
export LD_LIBRARY_PATH="$INSTALL_PREFIX/lib64:$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH"

MPIDIR=$(cd "$(dirname `which mpicc`)/.."; pwd)
export CMAKE_PREFIX_PATH="$INSTALL_PREFIX;$MPIDIR"
export CMAKE_INCLUDE_PATH="$INSTALL_PREFIX/include;$MPIDIR/include"
export CMAKE_LIBRARY_PATH="$INSTALL_PREFIX/lib64;$INSTALL_PREFIX/lib;$MPIDIR/lib"

# Create CMake initial cache.
CCONFIG="$BUILD_DIR/CMakeConfig.txt"
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

# Set some CMake options.
DCONFIG=" \
-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
-DCGNS_ENABLE_LFS:BOOL=ON              \
-DCGNS_ENABLE_64BIT:BOOL=ON            \
-DCGNS_ENABLE_LEGACY:BOOL=OFF          \
-DCGNS_ENABLE_SCOPING:BOOL=OFF         \
-DCGNS_ENABLE_BASE_SCOPE:BOOL=OFF      \
-DCGNS_ENABLE_TESTS:BOOL=OFF           \
-DCGNS_ENABLE_FORTRAN:BOOL=OFF         \
-DCGNS_ENABLE_HDF5:BOOL=ON             \
"
# -DCMAKE_PREFIX_PATH=$INSTALL_PREFIX;$MPIDIR    \
# -DCMAKE_INCLUDE_PATH=$INSTALL_PREFIX/include:$MPIDIR/include    \
# -DCMAKE_LIBRARY_PATH=$INSTALL_PREFIX/lib64:$INSTALL_PREFIX/lib:$MPIDIR/lib    \



cd "$BUILD_DIR"
cmake -C "$CCONFIG" $DCONFIG $FMRDIR/external/CGNS

make -j8
make install

if [ -f "$FMRDIR/external/CGNS/CMakeLists.orig" ]; then
  rm "$FMRDIR/external/CGNS/CMakeLists.txt"
  mv "$FMRDIR/external/CGNS/CMakeLists.orig" \
    "$FMRDIR/external/CGNS/CMakeLists.txt"
fi
cd "$FMRDIR"

if [ 1 -eq 1 ]; then
  # NOTE ptests expects to find libcgns_static.a.
  rm -f $INSTALL_PREFIX/lib/libcgns_static.a
  ln -s $INSTALL_PREFIX/lib/libcgns.a $INSTALL_PREFIX/lib/libcgns_static.a
  export CC=`which mpicc`
  export CFLAGS="-I$INSTALL_PREFIX/include -L$MPIDIR -lhdf5 -lmpi"
  export LD_LIBRARY_PATH
  mkdir -p "$BUILD_DIR/ptests"
  if [ "`which lfs 2>/dev/null`" ]; then # Parallel filesystem
  lfs setstripe -c 16 -S 1m ptests
  fi
  cd "$BUILD_DIR/ptests"
  cmake -Wno-dev                             \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DMPIEXEC=`which mpiexec`                \
    "$FMRDIR/external/CGNS/src/ptests"

  make
  ./pcgns_ctest             # Run serial, then
  mpiexec -np 4 pcgns_ctest # compare to parallel.
  cd "$FMRDIR"
fi







