# Femera prototype versions

## Femera 0.1: Matrix-free finite element geometry, physics, and solver performance testing

### 3D element types

* Linear, quadratic, cubic tetrahedra

### 3D material models

Linear isotropic and various anisotropic variants:

* Elastic
* Thermal
* Thermal-elastic (both one-way and two-way coupling)

Nonlinear isotropic:

* Linear hardening kinematic plasticity

### Physics & solver performance

Linear elastic quadratic tet performance compared to traditional sparse matrix finite element applications:

* Memory required reduced by 10x
* Large models: solve time reduced by 4-6x
* Even faster for small to medium models

## Femera 0.2: Modular architecure with flexiblle data handling

Tested several data file format, CAD, and visualization libraries

### Performance

* Many libraries, even those not intended for high performance computing, perform adequately under MPI.
* File I/O is bottleneck unless full stack (meshing, partitioning, solving, post-processing, visualizing) is done entirely in memory.
* When meshing is done outside of Femera, saving and loading only the surface mesh (e.g., STL file), then making the 3D mesh internally, is a reasonable compromise.

## Femera 0.3 = 0.1 + 0.2: Full stack demo (this version)

Combine physics and solver computational kernels from version 0.1, with modular architecture and flexible data handling of version 0.2, and expose it all for use in Python.

