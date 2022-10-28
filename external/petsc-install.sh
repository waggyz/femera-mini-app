#!/bin/bash
#
INSTALL_DIR="$1"
if [ -f "$2" ]; then FLAGS=`tr --delete '\n' < "$2"`; fi
JFLAG="$3"

if [ 0 -eq 1 ]; then # if [ -f "$INSTALL_DIR/include/petsc.h" ]; then
  echo "looks like PETSc is already installed"
  exit 0
else
  # PETSc cpmplains if these are set here.
  unset MAKEFLAGS
  #export CC=`which gcc`
  #export CXX=`which g++`
  # Instead, do like this:
  FLAGS="CC=`which gcc` CXX=`which g++` $FLAGS"
  #
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
  SRC_DIR="$FMRDIR/external/petsc"
  CPUMODEL=`fmrmodel`

  cd "$SRC_DIR"
  echo "./configure $FLAGS"
  eval ./configure $FLAGS
  make PETSC_ARCH=$CPUMODEL all
  # NOTE make test takes about 4 hr to run, may only show failed test output
  # make PETSC_ARCH=$CPUMODEL test
  MFLAGS="--no-print-directory -s -- PETSC_DIR=$SRC_DIR PETSC_ARCH=$CPUMODEL"
  make PETSC_ARCH=$CPUMODEL $MFLAGS install
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="" check
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="$CPUMODEL" check
fi
exit
