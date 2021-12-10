#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/include/petsc.h" ]; then
  echo "looks like PETSc is already installed"
  exit 0
else
  export CC=`which gcc`
  export CXX=`which g++`
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
  SRC_DIR="$FMRDIR/external/petsc"
  CPUMODEL=`fmrmodel`

  cd "$SRC_DIR"
  ./configure $FLAGS
  make PETSC_ARCH=$CPUMODEL all
  # NOTE sys ex31_1 test fails
  # NOTE make test takes a REALLY long time to run, only show failed test output
  # make PETSC_ARCH=$CPUMODEL test
  MFLAGS="--no-print-directory -s -- PETSC_DIR=$SRC_DIR PETSC_ARCH=$CPUMODEL"
  make PETSC_ARCH=$CPUMODEL $MFLAGS install
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="" check
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="$CPUMODEL" check
fi
exit
