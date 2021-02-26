#!/bin/bash
#
MPIDIR="$(dirname `which mpicc`)/../share"
cd "$MPIDIR"; MPIDIR=`pwd`
echo -f $MPIDIR/*/*.supp | head -n1
#
