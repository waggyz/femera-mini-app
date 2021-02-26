#!/bin/bash

module purge;
module load cmake_3.15.5 anaconda_3 Python_3.7.1       \
  petsc_3.11.3_gcc_openmpi paraview_5.6.1 boost_1.66.0 \
  gcc_9.2.0

#NOTE May need a more recent version of paraview.
