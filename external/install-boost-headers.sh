#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

echo " FIXME  externall/install-boost-headers.sh is not ready yet..."
exit 1


if xxx; then # if [ -f "$INSTALL_DIR/lib/libTKBool.a" ]; then
  echo "looks like Boost is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
  SRC_DIR="$FMRDIR/external/boost"
  cd "$SRC_DIR"
  eval ./bootstrap.sh $FLAGS
  ./b2 install
fi
exit


