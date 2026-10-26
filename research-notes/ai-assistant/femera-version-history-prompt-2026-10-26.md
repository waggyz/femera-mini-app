# Femera development history in brief

As a doctoral engineering researcher for NASA, I have been developing Femera (sole developer) for about 8 years. This is Femera v0.3, which is re-implemnting features from v0.1 and v0.2, which are available as git clones in the folders #research-notes/femera-v0.1/ and #research-notes/femera-v0.2/, respectively.

**Femera v0.1** (physics and solve performance)
 * #research-notes/femera-v0.1/
 * Focused on demonstrating the performance of physics and solver kernels
 * Implements my element by element(EBE)(really) matrix free tensor train element response approach
 * Better than 10x less memory used, 3-6x faster solve time (and even more for smaller models), over traditional sparse matrix implementations
 * Used bespoke file format

It became clear that very large file operations, or the thousands of file operations needed by UQ application, would be a significant finite element anaylsis toolchain bottleneck.
 
**Femera v0.2** (flexible data handling)

* #research-notes/femera-v0.2/
* Focused on demonstrating the use of various file handling libraries (no physics or solver)
* Modular architecture
* There is no open standard for finite element elasticity meshes (CGNS is for CFD)
* Demonstrated that in-memory meshing (and visualization), rather than loading data saved from another application, improves throughput enormously
* Also demonstrated that non-HPC specific applications (such as Gmsh) can perform adequately in an HPC environment through their APIs.

**Femera v0.3** (this version)

* Combines the performance of v0.1 with the flexible data handling of v0.2, including in-memory operations that traditionally involve large amounts of file I/O
* Also plan to implement finite difference (FD) and other related methods, because colleagues tend to use finite element analysis on large structured grids and complain about performance, rather than using FD

Please scan #research-notes/femera-v0.1/ and #research-notes/femera-v0.2/ and write a more detailed development history to a new file in the #research-notes/ai-assistant/ folder.