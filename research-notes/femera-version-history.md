# Femera prototype versions

## Femera 0.1: Matrix-free finite element geometry, physics, and solver performance testing

### Element types

* Linear, quadratic, cubic tetrahedra

### Material models

Linear isotropic and various anisotropic variants:

* Elastic
* Thermal
* Thermal-elastic (one-way coupling)
* Thermal-elastic (two-way coupling)

Nonlinear isotropic:

 * Linear hardening kinematic plasticity
 
### Performance

Compared to traditional sparse matrix finite element applications:

* Memory required reduced by 10x
* Solve time reduced by 4-6x for large models

## Femera 0.2: Modular architecure with flexiblle data handling

Tested several data file formats, CAD, and visualization libraries

### Performance

* Many libraries, even those not intended for high performance computing, perform adequately.
* File I/O is bottleneck unless full stack (meshing, partitioning, solving, post-processing, visualizing) is done entirely in memory.
* When meshing is done outside of Femera, saving and loading only the surface mesh (e.g., STL), then making the 3D mesh internally, is a reasonable compromise.
 
 
## Femera 0.3 = 0.1 + 0.2: Full stack demo (this version)

Combine physics and solver computational kernels from version 0.1 with modular architecture and flexible data handling of version 0.2.


