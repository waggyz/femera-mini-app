#!/bin/bash
H=$1; N=$2;
# First, mesh and partition with linear tets
gmsh -setnumber p 1 -setnumber h $H -setnumber n $N geo/unst-cube-p3.geo -

# Then, convert to cubic tets
gmsh "cube/unst"$H"p1n"$N".msh2" -order 3 -3 -format msh2\
 -o "cube/unst"$H"p3n"$N".msh2"

# Change tet physical IDs to partition numbers
awk '{ if (($2 == 29) && ($4 == 123)) $4=$7; print $0 }'\
 "cube/unst"$H"p3n"$N".msh2" > "cube/unst"$H"p3n"$N".msh"

# Toss intermediate files
rm "cube/unst"$H"p1n"$N".msh2" "cube/unst"$H"p3n"$N".msh2"

# Make .fmr files
./gmsh2fmr -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001\
 -M0 -E100e9 -N0.3 -R -v2 -a "cube/unst"$H"p3n"$N
