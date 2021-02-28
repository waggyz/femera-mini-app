# Femera

Femera is an open-source finite element-by-element (EBE) matrix-free
application that can be tuned for performance across a wide range of problem
sizes and HPC architectures.

This version (0.2) of the Femera mini-app is targeted at optimizing a modular
software architecture for high-performance computing. The primary goal is to
maintain the high level of performance demonstrated by version 0.1 while
providing flexible data handling. This mini-app is intended to minimize the
file/data conversion bottleneck that often limits the overall performance of
finite element applications by developing efficient interfaces to libraries that can
read and write several common file formats.

## Femera data handling mini-app installation

### 1. Clone Femera with its required and optional external packages

**Important** Do **not** build Femera in a location that has spaces in the path.

```bash
git clone --recursive -b data https://github.com/waggyz/femera-mini-app.git
```

This populates external/ with:
googletest (gtest), testy, HDF5, CGNS, gmsh, pybind11 (when available)

These have several dependencies.

* BLAS and LAPACK must be installed.
* CMake 3.? is required for several external packages.
* Pybind11 requires boost 1.56.

### 2. Set up your Python environment

[**NOTE** Skip this step for now]
[**TODO**] Install pytest, pytest-check, and python-subunit in your environment.

* PETSc installation requires the python module configure.

### 3. Install MPI

OpenMPI 2.1.1 and 3.1 have been tested.

### 4. Build external packages

```bash
cd femera-mini-app
make external
```

### 5. Set build options in config.local.

**TODO** See docs/install-femera.txt for details.

### 6. Build Femera

**Important** Build Femera on the machine that you will use it on.
```bash
make -j
```

### 7. Try it

This version of the mini-app must be run under MPI.
A bash script (fmr1node) is provided for single-node runs.

```bash
fmr1node build/femera/mini tests/mesh/cube-tet6*.* -o'build/femera/cube-tet6.cgn'
```

## Femera command line interface

For a list of command line options, run:

```bash
build/femera/mini -h
```
You may also see GoogleTest options, if you built Femera with it.

**TODO** Describe how to run some examples.


 Notices:
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
