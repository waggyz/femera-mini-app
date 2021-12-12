#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/lib/libfreetype.a" ]; then
  echo "looks like FreeType is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/freetype"
  # BUILD_DIR="$FMRDIR/build/external/freetype"

  # mkdir -p "$BUILD_DIR"
  cd "$SRC_DIR"
  # python3 -m pip install docwriter #FIXME requires elevated priveleges
  if [ -f ".gitignore" ]; then sh autogen.sh; fi

  eval ./configure $FLAGS
  make $JFLAG
  make install
fi
exit
