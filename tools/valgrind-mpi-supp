#!/bin/bash
#
WMPICC=`which mpicc`
MPIDIR=`dirname $WMPICC`
if test -d "$MPIDIR../share"; then
  SHAREDIR="/../share"
  cd "$MPIDIR"; SHAREDIR=`pwd`
  if test -f "$SHAREDIR/*/*.supp"; then
    echo $SHAREDIR/*/*.supp | head -n1
  fi
fi
#
