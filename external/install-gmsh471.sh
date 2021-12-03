#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ -f "$INSTALL_DIR/lib64/libgmsh.a" ]; then
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

  CK=`grep "HAS_OMP_FOR_SIMD" "$SRC_DIR/contrib/hxt/core/include/hxt_mesh.h"`
  if [ -z "$CK" ]; then # Check if patch applied.
    $FMRDIR/external/gmsh471-gcc48-patch.sh # Apply patch for gcc48
    echo "patched gmsh 4.7.1 for gcc 4.8"
  fi
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  cmake $FLAGS "$SRC_DIR"
  make $JFLAG
  make install
fi
exit
