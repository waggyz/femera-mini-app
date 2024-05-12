# Femera

Femera is an open-source finite element-by-element (EBE) matrix-free
application that can be tuned for performance across a wide range of problem
sizes and high-performance computing (HPC) architectures.

This version (0.3) of the Femera mini-app is for optimizing the basic 
capabilities of a high-performance finite element analysis toolchain.
The primary goal is to verify that the high level of performance demonstrated
by version 0.1 can be maintained while providing the flexible data handling
demonstrated by version 0.2.

## Femera demo mini-app installation

Femera is distributed only as source code. It is intended to be built by the user and optimized for the specific hardware that it will be run on. Femera supports several optional external packages, which may be provided or built by the user. The Femera source includes instructions and scripts to help with this process.

### 1. Clone Femera and optional external packages

**Important**: do **not** download or build Femera in a location that has spaces in the path. Some external packages may not install correctly if there are spaces in the build path.

```bash
git clone -b demo https://github.com/waggyz/femera-mini-app.git
[**TODO**] download git submodules

cd femera-mini-app
```
This will download Femera 0.3 source code into the folder femera-mini-app/.

**Important**: command-line examples assume that the current working
directory is the root of the Femera repository (femera-mini-app/ by default).

### 2. Set up the build environment

The recommended Femera configuration uses several external applications and
libraries.

* BLAS and LAPACK must be installed to use several packages on non-Intel CPUs.
* CMake 3.? is required for several external packages.
* Pybind11 requires boost 1.56.
* MPI: OpenMPI 2.1.1 and 3.1 have been tested. [**FIXME** only Femera 0.2]
* X11

For Red Hat Enterprise Linux (RHEL) 8, install the following packages.

```bash
#TODO Add RHEL CodeReady repository.
#  https://stackoverflow.com/questions/36651091/how-to-install-packages-in-linux-centos-without-root-user-with-automatic-depen

sudo yum install tcl tk glut-devel numactl-devel [**TODO**] more?

# Developers 
sudo yum install bats graphiz gtest-devel
#TODO valgrind?
```

### 3. Set up the Python environment

Install Python modules.
```bash
python3 -m pip install --user pytest pytest-check python-subunit numpy configure
#TODO cython?
```

### 4. Set build options in config.local.

Copy one of the configuration files (examples/config.*) to config.local.

```bash
cd femera-mini-app                      # if you are not there already
cp examples/config.default config.local # for the default configuration
vi config.local                         # or use the text editor of your choice
```

Edit config.local.

### 5. Build and install Femera

**Important**: build Femera and its dependencies on the machine that they will be used on.

```bash
cd femera-mini-app
make scan
make external
make mini
make test tune
make install
```

## Manual build

1. Check system
   * Python3, MPI, OpenMP, BLAS, LAPACK, SuiteSparse, CUDA, OpenGL, libGL, X11, libnuma, libcairo, Tcl/Tk, (Bats, Valgrind)
   * Developers: Graphviz, cinclude2dot, (Lyx or LaTeX?)
2. If needed, build and install Bats, **Pybind11**, FreeType, FLTK, libXML2, libgif, and **googletest**.
   * PETSc downloads the external package source code to femera-mini-demo/external/petsc-packages/(CPUNAME)-debug/git.googletest/
   * [**TODO** Test new PETSc version > v3.18.0-25-gc2e5504b09e for bugs noted here.]
   * PETSc 3.20.2 does not compile with gcc 8.5.0 20210514 (Red Hat 8.5.0-18)
   * TRYING 3.19.6 ... works!
3. Build and install PETSc with debugging. 
   * Note that this may require considerable (>15 GB) available disk space during the build, depending on the optional external packages selected.
```bash
cd external/petsc

./configure PETSC_ARCH=i7-12800H-debug --prefix=/home/dwagner5/.local/i7-12800H \
  --with-packages-build-dir=/home/dwagner5/Code/femera-mini-demo/external/petsc-packages \
  --doCleanup=1 --with-shared-libraries --with-debugging \
  --with-64-bit-indices --with-memalign=32 \
  COPTFLAGS='-march=native -mtune=native \
  -Ofast -fno-builtin-sin -fno-builtin-cos -fvisibility=hidden -funroll-loops \
  -frename-registers -ftree-vectorize -fno-common -fearly-inlining' \
  CXXOPTFLAGS='-march=native -mtune=native -Ofast -fno-builtin-sin \
  -fno-builtin-cos -fvisibility=hidden -fvisibility-inlines-hidden \
  -funroll-loops -frename-registers -ftree-vectorize -fno-common \
  -fearly-inlining' FOPTFLAGS='-march=native -mtune=native -Ofast \
  -fvisibility=hidden -funroll-loops -frename-registers -ftree-vectorize \
  -fno-common -fearly-inlining' \
  --with-openmp --with-mpi --with-petsc4py \
  --download-fblaslapack --download-suitesparse --download-hwloc \
  --download-mpi4py --download-zlib --download-libjpeg --download-libpng \
  --download-boost --download-opencascade --download-hdf5 --download-cgns \
  --download-netcdf --download-metis --download-parmetis --download-fftw \
  --download-gsl --download-eigen

# --download-pnetcdf --download-exodusii --download-libmesh --download-mumps
# These need approval

# ADD --with-cuda            # if CUDA is installed

# TODO --with-64-bit-indices # For compatibility with default Femera build.
                             # Chaco, maybe others, not compatible with int64.
# TODO switch between 32-bit integers (default) and 64-bit.
# No int64 indices: --download-chaco --download-ml (**BUG** --download-ptscotch --download-moab)
# TODO check if --download-libmesh install fails because of int64

# --download-googletest   # removed because [**BUG**] PETSc configure fails
                          # PETSc downloads googletest v1.10.x
# --with-googletest       # Cannot use --download-mpich or --download-openmpi 
#                           when not using --download-googletest. Perhaps you 
#                           want --download-googletest.
# --download-mpich        # REMOVED to see if --download-googletest will work.
# NO: gtest doesn't configure.
# NO: --with-googletest-dir=/home/dwagner5/.local/lib64 : does not work
# NO: --with-googletest-dir=/home/dwagner5/Code/femera-mini-demo.huh/external/googletest : does not work
# NO: --with-googletest-include=/home/dwagner5/.local/include --with-googletest-lib=/home/dwagner5/.local/lib64

# --download-hwloc     # removed from here because of the BUG below
# --download-mpi4py    # removed from here because of the BUG below
# --with-petsc4py      #                      "
                       # NOTE: version 3.19.6 may have fixed these

# --with-fblaslapack   # FIXME using --download-fblaslapack for debugging

# --with-mpi           # NOTE: no longer using --download-mpich for debugging
# --with-clean=1       # REMOVES external packages
# --doCleanup=0        # keeps package build files; should save time on rebuild

# maybe --with-scalar-type=complex
# TEST --with-openmp
# maybe --download-mumps  # direct solver

# REMOVED --download-szlib request canceled (CITR00026869): unresponsive OEM
# REMOVED --download-giflib request canceled (CITR00026866): vendor did not supply info

# --with-shared-libraries # required for petsc4py
#TODO rerun adding --with-shared-libraries=0 # for static linking

make PETSC_DIR=/home/dwagner5/Code/femera-mini-demo/external/petsc PETSC_ARCH=i7-12800H-debug all

make PETSC_DIR=/home/dwagner5/Code/femera-mini-demo/external/petsc PETSC_ARCH=i7-12800H-debug install

make PETSC_DIR=/home/dwagner5/.local/i7-12800H PETSC_ARCH="" check

# TODO REPORT dependency BUGS for --download-hwloc needs libxml2

# Optionally, run several hours of tests.
# make PETSC_DIR=/home/dwagner5/Code/femera-mini-demo/external/petsc PETSC_ARCH="$CPUMODEL-debug" test
```

4. Build and install Gmsh with support for CGNS and HDF5.
```bash
JFLAG=-j4

# export CC=`which gcc`
# export CXX=`which g++`
# export MPICC_CC=`which mpicc`
# export MPICXX_CXX=`which mpic++`

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dwagner5/.local/i7-12800H/lib
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dwagner5/.local/lib64
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/dwagner5/.local/lib

# FMRDIR=$(cd "$(dirname "$0")/.."; pwd) # when called from script in external
# FMRDIR=`pwd` # command line in femera-mini-demo directory

FMRDIR=/home/dwagner5/Code/femera-mini-demo # hardcoded here

cd $FMRDIR

SRC_DIR="$FMRDIR/external/gmsh"
BUILD_DIR="$FMRDIR/build/external/gmsh"

FLAGS="-DCMAKE_INSTALL_PREFIX=/home/dwagner5/.local/i7-12800H -DCMAKE_PREFIX_PATH=/home/dwagner5/.local/i7-12800H -DENABLE_BUILD_LIB=1 -DENABLE_BUILD_SHARED=1 -DENABLE_BUILD_DYNAMIC=1 -DOpenGL_GL_PREFERENCE=GLVND -DENABLE_OPENMP=1 -DENABLE_MPI=1 -DENABLE_PETSC=1 -DENABLE_CGNS=1 -DENABLE_FLTK=1 -DENABLE_OSMESA=1 -DENABLE_GRAPHICS=1 -DENABLE_OCC=1 -DENABLE_OCC_CAF=1 -DENABLE_OCC_STATIC=1 -DENABLE_CAIRO=1"
# -DENABLE_CGNS_CPEX0045=0

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

eval cmake $FLAGS "$SRC_DIR"
make $JFLAG
make install
```
5. Rebuild and install PETSc with debugging after adding the flags `--with-gmsh --with-googletest --download-zlib-static=1 --with-shared-libraries=0`
   * **FIXME** Need these here?: --download-hwloc --download-mpi4py --with-petsc4py.
6. Return to the Femera root directory to build and install femera-debug.
7. Return to the PETSc directory to build and install PETSc without debugging by changing `PETSC_ARCH=i7-12800H` and changing the flag `--with-debugging=0`.
8. Test it
```bash
make PETSC_DIR=/home/dwagner5/Code/femera-mini-demo/external/petsc PETSC_ARCH=i7-12800H test
```
# Tests may take an hour or longer and may only show failed test output.

9. Return to the Femera root directory to build and install Femera without debugging.  

# Notices
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
