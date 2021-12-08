#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ -f "$INSTALL_DIR/lib/petsc.so" ]; then
  echo "looks like PETSc is already installed"
  exit 0
else
  echo " FIXME  Check lib in external/install-petsc.sh >&2"
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)

  SRC_DIR="$FMRDIR/external/petsc"

  cd "$SRC_DIR"

  ./configure $FLAGS
  make PETSC_ARCH=`fmrmodel` all test < echo " "
  MFLAGS="--no-print-directory -s --  PETSC_DIR=$SRC_DIR PETSC_ARCH=`fmrmodel`"
  make PETSC_ARCH=`fmrmodel` $MFLAGS install
  # make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="" check
fi
exit
