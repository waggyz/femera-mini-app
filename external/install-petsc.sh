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
  if [ 0 -eq 1 ]; then #TODO Move to config.petsc.
  #
  EXTFLAGS=$EXTFLAGS --with-memalign=32 #TODO match Femera: <4,8,16,32,64>
  #
#  EXTFLAGS=$EXTFLAGS --download-hwloc=yes #FIXME use instead of libnuma?
  #
  # needed by other external packages
  #EXTFLAGS=$EXTFLAGS --download-openmpi=yes
  #EXTFLAGS=$EXTFLAGS --download-mpich=yes
  EXTFLAGS=$EXTFLAGS --download-zlib=yes
  EXTFLAGS=$EXTFLAGS --download-szlib=yes
  EXTFLAGS=$EXTFLAGS --download-boost=yes
  EXTFLAGS=$EXTFLAGS --download-opencascade=yes
  EXTFLAGS=$EXTFLAGS --download-googletest=yes
  EXTFLAGS=$EXTFLAGS --download-metis=yes
  EXTFLAGS=$EXTFLAGS --download-hdf5=yes
  EXTFLAGS=$EXTFLAGS --download-libjpeg=yes
  EXTFLAGS=$EXTFLAGS --download-libpng=yes
  EXTFLAGS=$EXTFLAGS --download-giflib=yes
  #EXTFLAGS=$EXTFLAGS --download-openblas=yes
  #
  # more file formats
  EXTFLAGS=$EXTFLAGS --download-moab=yes
  EXTFLAGS=$EXTFLAGS --download-exodusii=yes
  #
  # partitioners
  EXTFLAGS=$EXTFLAGS --download-chaco=yes
  #EXTFLAGS=$EXTFLAGS --download-parmetis=yes
  EXTFLAGS=$EXTFLAGS --download-ptscotch=yes
  #EXTFLAGS=$EXTFLAGS --download-party=yes
  #
  # future expansion
  EXTFLAGS=$EXTFLAGS --download-ml=yes
  EXTFLAGS=$EXTFLAGS --download-fftw=yes
  EXTFLAGS=$EXTFLAGS --download-mpi4py=yes
  #EXTFLAGS=$EXTFLAGS --download-moose=yes
  #EXTFLAGS=$EXTFLAGS --download-mumps=yes
  #EXTFLAGS=$EXTFLAGS --download-pnetcdf=yes
  #EXTFLAGS=$EXTFLAGS --download-libmesh=yes
  #
  fi
  export MPICC_CC=`which mpicc`
  export MPICXX_CXX=`which mpic++`

  FMRDIR=$(cd "$(dirname "$0")/.."; pwd)
  SRC_DIR="$FMRDIR/external/petsc"
  CPUMODEL=`fmrmodel`

  cd "$SRC_DIR"
  echo "./configure $FLAGS"
  eval ./configure $FLAGS
  make PETSC_ARCH=$CPUMODEL all
  # NOTE make test takes about 4 hr to run, may only show failed test output here
  # make PETSC_ARCH=$CPUMODEL test
  MFLAGS="--no-print-directory -s -- PETSC_DIR=$SRC_DIR PETSC_ARCH=$CPUMODEL"
  make PETSC_ARCH=$CPUMODEL $MFLAGS install
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="" check
  make PETSC_DIR=$INSTALL_DIR PETSC_ARCH="$CPUMODEL" check
fi
exit
