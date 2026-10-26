# Femera development history: from fast kernels to an integrated toolchain

Prepared: 2026-10-26

## Scope and evidence

This history combines the developer's account of approximately eight years of
sole development as a doctoral engineering researcher for NASA with a scan of
the archived v0.1 and v0.2 repositories. It also uses the current v0.3 README
to explain the intended synthesis.

The sources have different evidentiary roles:

- The developer's account supplies the research motivation, reported performance
  gains, and intended future direction.
- Archived source, examples, scripts, and documentation show what was implemented
  or explored in each development line.
- Git history supplies dated milestones, not independent validation of every
  commit message or a complete record of the research.

Neither archive was built or benchmarked for this history. Performance figures
below are attributed reports, not newly reproduced measurements. The presence
of an interface, enum, script, or TODO does not by itself establish a completed
or validated feature.

### Repository snapshots examined

| Development line | Local archive | Checked-out commit | Commit date |
|------------------|---------------|--------------------|-------------|
| v0.1 | [femera-v0.1](../femera-v0.1/) | `1a5fee2e9bab9d49f0df6c10a4d956dd9a80c9c3` | 2024-07-02 |
| v0.2 | [femera-v0.2](../femera-v0.2/) | `35f81183ce8ff73132429117e11f7ca04886cdc7` | 2021-10-18 |

Both clones contain history reaching an initial commit dated 2019-01-17,
`edcc5b6a`. This dates the available repository history, not necessarily the
beginning of the research. The v0.2 README carries a 2018 NASA copyright notice.

The version numbers describe successive research emphases, not strictly
non-overlapping calendar periods. In particular, the v0.1 numerical line
continued receiving changes after the main v0.2 data-handling experiments.

## 1. Research objective and the evolving performance question

Femera began by addressing a fundamental finite element computing problem:
large simulations can spend substantial memory and execution time storing,
moving, and applying matrix data.

The first development line concentrated on the numerical core: could a
genuinely matrix-free, element-by-element response formulation substantially
reduce those costs? Once that approach demonstrated its value, the next
question concerned the surrounding workflow. Fast solves alone do not ensure
fast analyses if geometry preparation, mesh conversion, file access, and
visualization dominate elapsed time.

This distinction is especially important for uncertainty quantification (UQ).
A workflow involving thousands of related analyses can repeatedly incur
preprocessing and file-handling costs, even when an individual solve is fast.
Large individual models create a related problem through the size of the
intermediate data.

The three versions therefore form a coherent progression:

1. **v0.1:** establish efficient physics and solver kernels.
2. **v0.2:** investigate flexible data handling and lower-overhead workflows.
3. **v0.3:** integrate those lessons into a broader scientific computing
   framework while recovering and verifying the numerical performance.

## 2. Femera v0.1: establishing the numerical approach

### 2.1 Element-by-element, genuinely matrix-free response

The [v0.1 README](../femera-v0.1/README.md) describes Femera as a finite
element-by-element (EBE), matrix-free implementation designed for a range of
problem sizes and HPC architectures.

The developer identifies the central contribution as an EBE matrix-free
tensor-train element-response approach. In this context, "matrix-free" is
important at the element level as well as the global level: avoiding assembly
of a global sparse matrix is not the same as retaining a dense stiffness matrix
for every element.

The [physics interface](../femera-v0.1/phys.h) makes that distinction visible.
It includes `EBE_TTE` and `EBE_LMS` evaluation choices and storage for element
stiffness data used for traditional EBE comparison. The
[vectorized isotropic elasticity kernel](../femera-v0.1/elas-iso3-vect.cc)
separately estimates tensor-response and stiffness-matrix work and data volume.
The source thus contains explicit machinery for exploring the costs of the
different response approaches.

The developer reports:

- More than a tenfold reduction in memory use relative to traditional sparse
  matrix implementations.
- Approximately three- to sixfold faster solve times, with still larger gains
  for some smaller models.

These figures motivate the later versions. A reproducible quantitative account
would additionally need the original comparison implementations, hardware,
meshes, tolerances, and measurement records; this scan does not reconstruct
that benchmark study.

### 2.2 Physics and discretization coverage

The archived mini-app supports first-order, second-order, and incomplete
20-node third-order tetrahedral elements, as documented in its README and
represented in the [element implementation](../femera-v0.1/elem-tet.cc).

Its numerical experiments extended beyond a single isotropic elastic kernel:

- Isotropic and orthotropic elasticity.
- Thermal conduction and thermoelastic response.
- Material orientations, including randomly assigned orientations and imported
  Bunge-angle data.
- Exploratory nonlinear/plasticity kernels.
- Comparisons among reference, baseline, optimized, and vectorized
  implementations.

For example, the archive contains
[orthotropic elasticity](../femera-v0.1/elas-ort3.cc),
[isotropic thermal](../femera-v0.1/ther-iso3.cc),
[thermoelastic](../femera-v0.1/elas-ther-iso3.cc), and
[plasticity-related](../femera-v0.1/elas-plkh-iso3.cc) source.
This is evidence of development breadth, not a claim that every variant reached
the same maturity. Several paths retain explicit experimental or incomplete
status.

### 2.3 Solvers, partitions, and hardware-conscious implementation

The practical solver path includes preconditioned conjugate gradient and an
experimental nonlinear conjugate-gradient option. The README documents Jacobi
and nonlinear tangent-Jacobi choices, load increments, iteration limits, and
absolute and relative stopping tolerances.

The [OpenMP PCG implementation](../femera-v0.1/halo-pcg-omp.cc) and
[nonlinear CG implementation](../femera-v0.1/halo-ncg-omp.cc) show a
partition-oriented organization with halo-related data and synchronization
concerns. The mini-app is organized around partitioned models rather than a
single monolithic assembled matrix.

The implementation uses a C++ framework with low-level, C-style numerical
kernels. The [shared kernel code](../femera-v0.1/phys-inline.cc) contains AVX
intrinsics for response calculations, rotations, and accumulation. Data
alignment, loop organization, compiler behavior, and data movement were active
optimization concerns.

The performance infrastructure reinforces that emphasis:

- [Strong-scaling experiments](../femera-v0.1/perf/scale-strong.sh).
- [Weak-scaling experiments](../femera-v0.1/perf/scale-weak.sh).
- [Partition-size experiments](../femera-v0.1/perf/test-part-size.sh).
- [Physics profiling](../femera-v0.1/perf/profile-physics.sh).
- [CPU identification and SIMD support checks](../femera-v0.1/unit-test/).

The scaling scripts vary element order, model size, partition counts, and core
counts, and explicitly configure OpenMP placement and binding. They document
the intended performance investigations, not proof that every scripted case
completed successfully.

### 2.4 A deliberately narrow model-preparation pipeline

The numerical mini-app relied on partitioned, bespoke `.fmr` model files.
The [Gmsh converter](../femera-v0.1/gmsh2fmr.cc) translates Gmsh version 2
ASCII mesh data and adds boundary conditions, loads, and material properties.

The README demonstrates cube and microstructure workflows using Gmsh and
Neper. Conversion can assign material properties by partition, select nodes by
physical tags or coordinate planes, and apply mechanical or thermal conditions.

This made the numerical experiments practical, but retained a file-oriented
pipeline:

```text
Geometry / mesh generation
    -> mesh files
        -> conversion and model annotation
            -> partitioned .fmr files
                -> Femera solve
```

The limitations were explicit. The README warns of race conditions in the
OpenMP converter and recommends the serial converter. Some command-line
features were untested or experimental.

The lesson was not that the numerical formulation had failed. Rather, the
surrounding preparation and data pipeline could consume an increasing fraction
of total runtime as the numerical core became faster.

## 3. Femera v0.2: moving the experiment to data and workflow

### 3.1 A separate research mini-app

The [v0.2 README](../femera-v0.2/README.md) explicitly targets modular HPC
architecture and flexible data handling, with the goal of minimizing
file/data-conversion bottlenecks while ultimately retaining v0.1 performance.

A dated transition is visible in commit `f2676376` on 2021-02-26:
"initializing data mini-app to optimize data handling..."

According to the developer, this version intentionally omitted the physics and
solver work to focus on the data problem. The archive supports that account:
the [physics source](../femera-v0.2/src/Phys.cpp) is effectively an empty
namespace, while substantive implementations reside in the data, processing,
and simulation-preparation code.

Consequently, v0.2 should be understood as a complementary architectural
experiment, not a demonstrated replacement for the complete v0.1 solver.

### 3.2 Modular work, processing, and data interfaces

The [Work interface](../femera-v0.2/src/Work.hpp) supplies shared metadata,
processing/data references, timing, lifecycle methods, preparation, and
checking. It is explicitly a polymorphic interface and includes a note
considering a future CRTP refactoring.

Specialized work is organized into processing, data, and simulation tasks.
The [processing modules](../femera-v0.2/src/Proc/) include MPI, OpenMP, and
GoogleTest integration.

The [data dispatcher](../femera-v0.2/src/Data.cpp) locates handlers for file
types, obtains file information, scans data, and delegates meshing and
partitioning operations. Simulation-side classes such as
[Mesh](../femera-v0.2/src/Sims/Mesh.cpp) request typed data and prepare
connectivity and Jacobian-related arrays.

The architecture shifts the central abstraction from "read the bespoke solve
file" toward "obtain the data or operation required by this simulation."

### 3.3 Gmsh and CGNS as complementary integrations

Two substantial data backends are present:

- [Dmsh](../femera-v0.2/src/Data/Dmsh.cpp), using the Gmsh API.
- [Dcgn](../femera-v0.2/src/Data/Dcgn.cpp), using CGNS and, when configured,
  parallel CGNS access.

The Gmsh path can generate meshes and partition them through library calls,
rather than requiring a separate meshing executable and intermediate mesh file
for every operation. The implementation also handles model selection,
configuration, data extraction, and visualization-related operations.

The CGNS path explores structured file metadata, geometry and connectivity
access, and serial/independent/collective access choices. The
[CGNS handler declaration](../femera-v0.2/src/Data/Dcgn.hpp) also records
application-specific mappings for material, boundary-condition, and solver
information.

The developer's motivation was the absence of a satisfactory open standard for
the complete finite element elasticity model workflow being targeted. CGNS,
with its CFD heritage, was a useful candidate rather than a complete answer
to all elasticity-specific semantics. This history does not independently
establish a universal absence of mesh standards.

### 3.4 In-memory preprocessing and visualization

The important workflow change is the ability to retain generated models in a
library-managed process and retrieve their data through an API:

```text
Parameterized geometry
    -> in-process meshing / partitioning
        -> data access for simulation preparation
            -> visualization or optional file output
```

This does not eliminate all I/O. Geometry definitions, imported meshes,
checkpoints, and exported images can still involve files. It removes the
requirement to serialize and reload every intermediate representation.

The developer reports that this substantially improved throughput and showed
that software not designed primarily as an HPC application, notably Gmsh, could
be useful in HPC workflows through its API.

The archive contains a concrete demonstration direction:
[fiber-polygon-anim.geo](../femera-v0.2/tests/geo/fiber-polygon-anim.geo)
parameterizes composite-fiber representative volume element geometry using a
frame number. Its history culminates in commit `35f81183`, dated 2021-10-18,
adding the composite fiber-shape RVE animation demonstration.

That example connects geometry variation, meshing/visualization infrastructure,
and repeated-workflow research. It is not evidence of a completed coupled
physics solve in v0.2.

### 3.5 Concurrency was an integration problem, not just a compiler option

The archive's MPI/OpenMP infrastructure and backend-specific coordination show
that useful HPC integration required accounting for library behavior.

The Gmsh implementation controls initialization, model selection, threading,
and the number of open models. CGNS exposes different concurrency modes and
maintains backend-specific state.

The commit record contains fixes for race conditions and model limits, and a
2021-06-22 visualization milestone explicitly notes a one-OpenMP-thread-per-MPI-
rank restriction at that stage. These are historical integration constraints,
not a claim that the restriction applies unchanged to all later code.

Likewise, the v0.2 README says this snapshot must run under MPI. That requirement
belongs to the archived mini-app, not necessarily to every subsequent Femera
configuration.

The [size-sweep driver](../femera-v0.2/tests/run-sizes.sh) records a
performance-testing direction involving many generated meshes, parallel
preparation, elapsed time, and memory measurements. Quantitative comparison
results were not reconstructed for this history.

### 3.6 Modularity and plugins: implemented structure versus ambition

The [Plug module](../femera-v0.2/src/Main/Plug.cpp) explicitly registers
processing and data modules and constructs selected task types.
Its [header](../femera-v0.2/src/Main/Plug.hpp) defines implementation identifiers
with a trailing user boundary.

The broader [type declarations](../femera-v0.2/src/type.hpp) include
plugin-oriented choices for scheduling and partitioning, while the
[developer guide](../femera-v0.2/docs/developer-guide.txt) discusses user plugin
locations and future integrations.

These show that extensibility was already part of the architecture. The
examined registration/construction path is nevertheless based on compiled-in
modules and explicit switches. The name `Plug` alone does not establish a
finished runtime shared-library loading system or stable third-party plugin ABI.

## 4. Selected chronology

| Date | Evidence | Historical significance |
|------|----------|-------------------------|
| 2019-01-17 | `edcc5b6a`, shared initial repository history | Earliest available repository commit in both clones |
| 2019-11-25 | v0.1 `d46ed107` | Plasticity-related development included a dummy driver for non-AVX configurations |
| 2020-01-23 | v0.1 `11c14b2a` | Halo-value storage optimization |
| 2020-02-19 | v0.1 `1ca07653` | Commit records completion of isotropic thermal physics at that stage |
| 2020-02-21 | v0.1 `438d207d` | AVX-intrinsics implementation for isotropic thermal physics |
| 2021-02-26 | v0.2 `f2676376` | Explicit initialization of the data-handling mini-app |
| 2021-04-10 | v0.2 `2ffb5de5` | Commit records Gmsh meshing and partitioning a geometry model when needed |
| 2021-04-20 | v0.2 `7a9c0d45` | Gmsh Jacobian/determinant data access milestone |
| 2021-04-24 | v0.2 `20fea1b9` | CGNS coordinate-reading milestone, cautiously described in the commit itself |
| 2021-06-22 | v0.2 `ed751792` | Gmsh visualization milestone with an explicit concurrency limitation |
| 2021-07-13 | v0.2 `5ef656d3` | Draw/save timing instrumentation |
| 2021-10-18 | v0.2 `35f81183` | Composite fiber-shape RVE animation demonstration |
| 2024-03-29 to 2024-03-30 | v0.1 `474ce953`, `428c032b` | Further orthotropic thermal kernel and preconditioner experimentation |
| 2024-07-02 | v0.1 `1a5fee2e` | Gmsh-to-Femera converter segfault fix; checked-out numerical-line snapshot |

Milestones above summarize commit descriptions alongside inspected source.
They are not a release certification or an assertion that all related work was
complete on those dates.

## 5. Femera v0.3: integrating the research lines

The [current README](../../README.md) states the central v0.3 objective:
verify that v0.1 numerical performance can be maintained while supplying the
flexible data handling demonstrated by v0.2.

The current design expresses a broader decomposition through `Work` and
families for processing, data, tasks, tests, physics, grids, and algorithms.
The [current physics adapter](../../src/femera/Phys.hpp), for example, reflects
the move toward CRTP-based implementation machinery considered in the v0.2
source.

This is an integration and reimplementation effort, not merely a renaming of
the old code. Its success needs to be evaluated at both levels:

- Numerical correctness, memory use, and solver/kernel performance.
- End-to-end throughput across geometry, meshing, data movement, simulation,
  visualization, and repeated analyses.

The developer also plans finite-difference and related methods, motivated by
structured-grid workloads where finite element analysis may not be the most
efficient choice. This is a planned direction, not a capability demonstrated
by the v0.1/v0.2 archive scan.

User extensibility remains important: researchers should be able to add their
own models, methods, and data capabilities. The recent
[runtime-interface proposal](./femera-runtime-interfaces-plan-2026-10-26.md)
and [plugin-capability notes](./femera-user-plugin-capabilities-notes-2026-10-26.md)
are prospective design discussions, not completed historical features.

## 6. Overall interpretation

Femera's development is best understood as progressively widening the boundary
of the performance problem.

**v0.1 addressed the cost of the numerical operator and solve.** It established
the practical value of the developer's matrix-free EBE response approach and
provided a hardware-conscious experimental solver.

**v0.2 addressed the cost of moving models through the analysis workflow.**
It explored modular data interfaces, direct library integration, in-memory
meshing, and visualization as ways to avoid repeatedly paying for intermediate
files and conversions.

**v0.3 aims to make those advantages coexist in an extensible framework.**
The objective is not simply a fast solver attached to more file readers, but
an efficient scientific toolchain whose numerical methods, data sources, and
user-supplied capabilities can evolve without sacrificing performance or
forcing unnecessary intermediate representations.
