#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ -f "$INSTALL_DIR/lib/petsc.a" ]; then
  echo "looks like PETSc is already installed"
  exit 0
else
  echo " FIXME  Check lib in external/install-petsc.sh >&2"
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/freetype"
  # BUILD_DIR="$FMRDIR/build/external/freetype"

  # mkdir -p "$BUILD_DIR"
  cd "$SRC_DIR"

  ./configure $FLAGS
  make $JFLAG
  make install
fi
exit
