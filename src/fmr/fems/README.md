# Low-level finite element kernels

This folder contains both reference and optimized functions for finite element computational geometry operations, for real and hypercomplex algebras.

**Element operations**

- Element geometric Jacobian (**J**), inverse, transpose, patial rotation, determinate (element volume)
- Element response (forward eval)
- Small strain tensor (**H**)
- Shape function gradient tensor (**G**) and equivalent Voigt vector
- Cauchy stress tensor (**S**) and equivalent Voigt vector (see mtrl for material response kernels)
- Element nodal force tensor (**F**)
- Element stiffness matrix (**k**), for comparison (EBE-LMS)
- Element internal energy (integrate energy density, below)

**Integration point operations**

- Stresses, strains
- Energy density (stress * strain)

**Mesh operations**

- Jacobi preconditioner (diagonal of **K**)
- Volume (integrate element Jacobian dets): with & without displacements
- Mass (integrate density): constant, cubic spline
- External energy (integrate: **f** · **u**)
- Sparse stiffness matrix (**K**): use PETSc (for comparison)

**Node operations**

- Nodal forces from pressure (integrate over faces)

*TODO* do solver kernels go in here, too? Do FD, FV, etc. kernels differ?
