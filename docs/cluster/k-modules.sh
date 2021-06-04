#!/bin/bash

module purge;
module load cmake_3.15.5 anaconda_3 Python_3.7.1 libGL-6.5 \
  petsc_3.11.3_gcc_openmpi paraview_5.6.1 boost_1.66.0 \
  gcc_9.2.0

#NOTE module unload anaconda_3 before building FLTK
#NOTE May need a more recent version of paraview.
