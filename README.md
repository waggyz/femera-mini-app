# Femera
Femera is an open-source finite element-by-element (EBE) matrix-free implementation that can be tuned for performance across a wide range of problem sizes and HPC architectures. It is implemented in a C++ framework, but computational kernels are coded as vanilla C with vectorfor better performance.

# Femera Mini-App
The Femera Mini-App is a minimal implentation with limited element and mesh format support.

## Quick Start
Run some system tests.

 * `make unit-test`

Build and check the executable `femera-<cpumodel>-gcc` using the make target `femera-mini`.  If you have Gmsh 2.x mesh files of your models, you'll need a mesh file format converter. *WARNING* The OpenMP converter does not work (race conditions). Instead, compile the single-core serial converter `gmsh2fmr-ser`. These, and a minimal, quiet version of femera, `femerq`,  can be built with:


```bash
# Set `C` to the total number of physical cores in your machine.
C=4;
`make -j$C all`
``` 

*Make a note of the name of the executables built.*

If you have [Gmsh 4](http://gmsh.info/ "Gmsh Website") installed, try making and solving a one million degree of freedom (MDOF) quadratic tetrahedron model.

 * Set `C` to the total number of physical cores in your machine.
 * Set `P` to 2 for qaudratic tets.
 * Set `H` to 33 elements (34 nodes) along each edge of the modeled cube.
 * Set `N` to the number of partitions desired (N>=C); try 4xC to 16xC.

```bash
C=4; P=2; H=33; N=16
CPUMODEL=`./cpumodel.sh`
gmsh -setnumber p $P -setnumber h $H -setnumber n $N -nt $C geo/unst-cube.geo -
./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu0.001 -M0 -E100e9 -N0.3 -R -v2 -ap "cube/uhxt"$H"p"$P"n"$N
./femera-$CPUMODEL -v2 -c $C -p "cube/uhxt"$H"p"$P"n"$N
```

Then, try a 1.3 MDOF quadratic tet thermoelastic model.

```bash
C=4 ; P=2 ; H=33 ; N=16
CPUMODEL=`./cpumodel.sh`
gmsh -setnumber p $P -setnumber h $H -setnumber n $N -nt $C geo/unst-cube.geo -
./gmsh2fmr -t111 -z0 -t666 -x0 -t333 -y0 -t444 -xu0.001 -t444 -Tu 10 -M0 -E100e9 -N0.3 -A20e-6 -K250 -R -v2 -ap "cube/uhxt"$H"p"$P"n"$N
./femera-$CPUMODEL -v2 -c $C -p "cube/uhxt"$H"p"$P"n"$N
```

If you have [Neper](http://neper.sourceforge.net/ "Neper Website") installed, try making and solving a microstructure model.

```bash
C=4 ; N=16
CPUMODEL=`./cpumodel.sh`
neper -T -reg 1 -n $N -o "neper/n"$N"-id1"
neper -M "neper/n"$N"-id1.tess"
./gmsh2fmr -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 -x@1.0 -xu0.001 -M0 -E100e9 -N0.3 -R -v1 -a "neper/n"$N"-id1"
./femera-$CPUMODEL -v2 -c $C -s1 -p "neper/n"$N"-id1"
```

To generate a PNG of the model:
```bash
neper -V "neper/n"$N"-id1.tess" -datacellcol id -print "neper/n"$N"-id1"
```

Here is a more detailed  microstructure model.

```bash
C=4 ; N=100 ; L=0.005
CPUMODEL=`./cpumodel.sh`
neper -T -reg 1 -morpho graingrowth -oricrysym cubic -ori uniform -domain "cube("$L,$L,$L")" -n $N -for tess,ori -o "neper/cubic"$N
neper -V "neper/cubic"$N".tess" -datacellcol id -print "neper/cubic"$N
neper -M "neper/cubic"$N".tess"
gmsh -refine -o "neper/cubic"$N"s1p1.msh2" "neper/cubic"$N".msh"
./gmsh2fmr -x@0.0 -x0 -y@0.0 -y0 -z@0.0 -z0 "-x@"$L -xu0.001 -M0 -E136.31e9 -N0.37 -G127.40e9 -R -v1 -a "neper/cubic"$N"s1p1"
./femera-$CPUMODEL -v2 -c $C -s1 -p "neper/cubic"$N"s1p1"
```

## Mini-App Files

`femera.h` `femera-mini.cc` Set compile-time constants in the header file (though most of these don't work). The femera-mini executable parses a partitioned set of `.fmr` files having the provided base name.  It supports a few command line options.

 * `-p <str>` Read partitioned `<str>_#.fmr` finite element model files in parallel. *Required.*
 * `-v <int>` Verbosity level [default: 1]; `-v2` is good.
 * `-c <uint>` Number of parallel compute threads.
 * `-d <int>` Choose a diagonal preconditioner:
   * `-d0` for none,
   * `-d1` for element row one-norm sum, or
   * `-d3` for Jacobi [default].
 * `-s <int>` Choose a solver:
   * `-s1` for conjugate gradient [default], or
   * `-s2` for nonlinear conjugate gradient [experimental].
 * `-r <float>` Solution tolerance.
 * `-i <int>` Maximum number of iterations.

`mesh.h` `mesh.cc` An instance of this class contains global model information.

`elem.h` `elem.cc` `elem-tet.cc` An Elem instance contains a single mesh partition. The mini-app only supports first- and second-order tetrahedral elements at present.

`phys.h` `phys.cc` `elas-iso3.cc` `elas-ort3.cc` `elas-ther-iso3.cc` `elas-ther-ort3.cc` Linear-elastic and thermoelastic models implement both tensor train and (eventually) the more traditional local matrix evaluation approaches. Two preconditioners are also provided. Reference, optimized, and vectorized (AVX, non-thermo only) versions are available. 

`solv.h` `solv.cc` `halo-pcg-omp.cc` `halo-ncg-omp.cc` Implement preconditioned conjugate gradient and an experimental nonlinear cojugate gradient solver.

`align.h` This implements a hacky way to align data structures for better vector performance. Valgrind doesn't like it.

## Mini-App Accesories
These are not necessary for compiling the Mini-App.

`test.h` `test.cc` These provide reference solutions for the test problems.

`gmsh.h` `gmsh2.cc` `gmsh2fmr.cc` The mesh file converter command line tool `gmsh2fmr` also encodes boundary conditions, loads, and elastic material properties in the `*_#.fmr` partitioned mesh files created. It only supports Gmsh `*.msh` version 2 ASCII mesh files. Note that [Gmsh 4](http://gmsh.info/ "Gmsh Website") is needed to create partitioned `*_#.msh` mesh format 2 files, though `gmsh2fmr` can also partition meshes by gmsh volume physical IDs.

 * `-a <str>` Read Gmsh file `<str>.msh` or `<str>.msh2`, partition the mesh by volume physical IDs, and save the partitions as ASCII files `<str>_#.fmr`.
 * `-ap <str>` Read partitioned Gmsh files `<str>_#.msh`, and save the partitions as ASCII files `<str>_?.fmr`.
 * `-v<int>` Verbosity level [default: 1]; set higher to see details of each partition.

 Boundary conditions may be applied to nodes specified by node number, Gmsh physical ID of linear or surface elements, or by location within a plane.
 
 * `-n<int>` Apply boundary condition or load to node with Gmsh ID `<int>`.  *IMPORTANT NOTE* not tested.
 * `-t<int>` Apply boundary condition or load to nodes that are in lines and surfaces tagged with Gmsh physical ID `<int>`.
 * `-x@<float>`  `-y@<float>`  `-z@<float>` Apply boundary condition or load to nodes located in a specified plane at, e.g., x=`<float>`.

Specify the condition to apply after a node selection argument above.

 * `-x0` `-y0` `-z0` Fix displacement to zero in the specified direction.
 * `-xu<float>` `-yu<float>` `-zu<float>` Set displacement in the direction specified to `<float>`.
 * `-xf<float>` `-yf<float>` `-zf<float>` Set RHS nodal force in the direction specified to `<float>`.
 - *Note* these direction specifiers (`x` `y` `z`) may be combined, e.g., `-xyz0` fixes all three displacement components to zero (encastre).

Material properties are set for each partition.

 * `-M0` Apply the properties in the arguments following as the default for all partitions.
 * `-M<int>` Apply material properties to partition number `<int>`. *Note* partitions are one-indexed.
 * `-E<float>` `-N<float>` `-G<float>` Young's modulus, Poisson's ratio, and (optional) shear modulus.
 * `-C<float>` Material tensor constant. *IMPORTANT NOTE* not tested.
 * `-X<float>` `-Y<float>` `-Z<float>` material orientation components in radians. *IMPORTANT NOTE* Only the Bunge convention is supported:
   `-Z<float> -X<float> -Z<float>`.
 * `-R` Apply random material orientations to each partition.
 * `-B <str>` Read Bunge orientations *in radians* for all partitions from the ASCII file `<str>`.
 * `-O <str>` Read Bunge orientations *in degrees* for all partitions from the ASCII file `<str>`.

`geo/unit-cube.geo` `geo/unst-cube.geo` `cube/*.fmr` `neper/*.fmr` Mini-App test meshes.

`makefile` This contains several targets for testing and timing the Femera mini-app an its components.

`README.md` `LICENSE` This file, and another copy of the information below.

# Notices:
Copyright **2018** United States Government as represented by the Administrator of the National Aeronautics and Space Administration. No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.

# Disclaimers
_No Warranty_: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS, ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS, RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE, IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."

_Waiver and Indemnity_:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM, RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.  RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE, UNILATERAL TERMINATION OF THIS AGREEMENT.
