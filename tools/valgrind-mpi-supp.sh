#!/bin/bash
#
MPIDIR="$(dirname `which mpicc`)/../share"
cd "$MPIDIR"; MPIDIR=`pwd`
echo $MPIDIR/*/*.supp | head -n1
#
