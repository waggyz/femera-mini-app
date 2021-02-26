# Femera

Femera is an open-source finite element-by-element (EBE) matrix-free
application that can be tuned for performance across a wide range of problem
sizes and HPC architectures.


## Femera full installation



### 1. Clone Femera with its required and optional external packages

**Important** Do **not** build Femera in a location that has spaces in the path.

```bash
git clone --recursive -b develop https://gitlab.larc.nasa.gov/dwagner5/Femera
```

This populates external/ with:
googletest (gtest), testy, HDF5, CGNS, gmsh, pybind11, PETSc,
zyclops (when available)

These have several dependencies.

* BLAS and LAPACK must be installed.
* CMake 3.? required for several external packages.
* Pybind11 requires boost 1.56.

### 2. Set up your Python environment

[**NOTE** Skip this step for now]
[**TODO**] Install pytest, pytest-check, and python-subunit in your environment.

 * PETSc installation requires the python module configure.

### 3. Install MPI

OpenMPI 2.1.1 and 3.1 have been tested.

### 4. Build external packages

```bash
cd Femera
make external
```

### 5. Set up your Python environment again

[**NOTE** Skip this step for now]
[**TODO**] Install mpi4py, pyhdf5, pycgns, pygmsh, pypetsc, etc.

### 6. Set build options in config.local.

**TODO** See docs/install-femera.txt for details.

### 7. Build Femera

**Important** Build Femera on the machine that you will use it on.
```bash
make -j
```
**TODO** make test
**TODO** make -j install

Tune Femera for your hardware.

**TODO** make tune

**WARNING** This takes a while to run.

## Femera command line interface

Use femerea -h for a list of command line options.


**TODO** Describe how to run some examples.



## Femera mini-app quick start

**WARNING** Use the Femera full installation instructions above during initial
Femera 0.1 development.

**TODO** Move this to the top when it works.

The basic mini-app only requires bash, gmake, Python 2.7,
and a compiler that supports C++11 and OpenMP.
CMake 3.15, Python 3.6 (with pytest, pytest-check, and python-subunit
modules), gtest, and gmsh are useful for development and performance
optimization, but not required.

```bash
git clone https://github.com/waggyz/femera-mini-app

#--or--

git clone --recursive --branch mini https://gitlab.larc.nasa.gov/dwagner5/Femera
```

**Important** Build Femera on the machine that you will use it on.

```bash
make -j
make test
make -j install
```

Tune Femera for your hardware.
```bash
make tune
```
**Warning** This takes a while to run.
# Notices:
Copyright 2018 United States Government as represented by the Administrator of
the National Aeronautics and Space Administration. No copyright is claimed in
the United States under Title 17, U.S. Code. All Other Rights Reserved.

# Disclaimers
No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR
FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR
FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE
SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF
PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE
UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY
PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY
LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE,
INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE
OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED
STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR
RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH
MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
