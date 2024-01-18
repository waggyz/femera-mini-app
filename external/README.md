# External Sub-modules


* external/README.md
## Build Scripts
* external/build-googletest.sh
* external/build-hdf5.sh
* external/build-cgns.sh
* external/build-pybind11.sh
* external/build-gmsh.sh

## Recommended
googletest/ --branch v1.10.x
gmsh/ --branch gmsh_4_5_6
pytest
pytest-check
python-subunit

## Optional
openmpi/
hdf5/ --branch hdf5_1_12
CGNS/
petsc/ --branch maint
pybind11/ --branch v2.5.0
paraview/

# Installation

1. Install MPI.
2. Install HDF5 (requires MPI).
3. Install CGNS (requires HDF5).

Optional

BLAS/LAPAK?

* Configure Python (requires pip or equivalent).
  * pip install pytest pytest-check

* Install googletest.
  * pip install python-subunit
* Install pybind11.
* Install PETSc (requires MPI).
* Install Gmsh (options: OpenMP CGNS, PETSc, BLAS, LAPAK, OpenCASCADE).
* Install ParaView.

* Configure python some more.
  * pip install numpy mpi4py
  * pip install CGNS

===============================================================================
'''bash
# TODO Put this somewhere.
# Installation directory for external packages
INSTALL_PREFIX=xxx
export LD_LIBRARY_PATH=$INSTALL_PREFIX/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=$INSTALL_PREFIX/include:$C_INCLUDE_PATH

export PETSC_DIR
export PETSC_ARCH
pip install petsc4py

#  * pip install petsc petsc4py (requires numpy mpi4py) 
#  (requires petsc if not already installe)
'''
