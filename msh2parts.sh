#!/bin/bash

BASE=$1
N=$2

gmsh -part_topo -part_ghosts -part $N -part_split -format msh2 -o $1"-n"$N -save $BASE".msh"

./gmsh2fmr -v3 -x@0.000155748 -x0 -y@-1.27054482e-05 -y0 -z@0.000879453 -z0 \
  -y@0.00026010193 -yu0.001 -M0 -E70e9 -N0.3 -X0 -Z0 -X0 -ap $BASE

# x: 0.000155748 - 0.000725461
# y: -1.27054482e-05 - 0.000260010193
# z: 0.000879453 - 0.00142136

