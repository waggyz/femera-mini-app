#!/bin/bash
# module unload anaconda_3 #NOTE Needed for K
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr '\n' ' ' < "$2"`; fi
JFLAG="$3"

#if which fltk-config 2>/dev/null ; then
if [ 0 -eq 1 ]; then # [ -f "$INSTALL_DIR/lib/libfltk.a" ]; then
  echo "looks like FLTK is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/fltk"
  # BUILD_DIR="$FMRDIR/build/external/fltk"
  # mkdir -p "$BUILD_DIR"
  cd "$SRC_DIR"
  if [ ! -f "configure" ]; then make makeinclude; fi
  eval ./configure  $FLAGS
  make $JFLAG
  make install
fi
exit
