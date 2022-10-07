# Femera

Femera is an open-source finite element-by-element (EBE) matrix-free
application that can be tuned for performance across a wide range of problem
sizes and HPC architectures.

This version (0.3) of the Femera mini-app is for optimizing the basic
capabilities of a high-performance finite element analysis toolchain.
The primary goal is to verify that the high level of performance demonstrated
by version 0.1 can be maintained while providing the flexible data handling
demonstrated by version 0.2.

## Femera demo mini-app installation

### 1. Clone Femera with its required and optional external packages

**Important** Do **not** download or build Femera in a location that has spaces
in the path. At least one external package will not install correctly if there
are spaces in the build path.

```bash
git clone -b demo https://github.com/waggyz/femera-mini-app.git
```
This will download the Femera 0.3 repository into the folder
femera-mini-app/.

**Important** the command-line examples assume that the current working
directory is the root of the Femera repository (femera-mini-app/ by default).

### 2. Set up the build environment

The recommended Femera configuration requires several applications and libraries.

* BLAS and LAPACK must be installed for use on non-Intel CPUs.
* CMake 3.? is required for several external packages.
* Pybind11 requires boost 1.56.
* MPI: OpenMPI 2.1.1 and 3.1 have been tested.

### 3. Set up the Python environment

[**TODO**] Install pytest, pytest-check, and python-subunit in your environment.

* PETSc installation requires the python module configure.

### 4. Set build options in config.local.

Copy one of the configuration files (examples/config.*) to config.local.

```bash
cd femera-mini-app
cp examples/config.default config.local
```

Edit config.local.

### 5. Build and install Femera

**Important** Build Femera on the machine that it will be used on.

```bash
cd femera-mini-app
make external
make mini
make test tune
make install
```



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
