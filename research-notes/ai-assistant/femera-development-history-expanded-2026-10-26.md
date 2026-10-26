# Femera development history: numerical kernels, data workflows, and integration

Prepared: 2026-10-26

This expanded history incorporates the archived v0.1 and v0.2 source trees,
selected Git milestones, the unpublished v0.1 technical memorandum draft, and
the newly supplied figures and benchmark data. It supplements rather than
overwrites the [earlier source-based history](./femera-development-history-2026-10-26.md).

## 1. Research context and how to read the evidence

David Wagner describes Femera as approximately eight years of sole development
undertaken as a doctoral engineering researcher for NASA. Its progression is
best understood as a sequence of research questions:

1. Can finite element physics and iterative solution be made substantially
   faster and less memory-intensive?
2. Once the numerical core is fast, how much of the analysis workflow is still
   spent preparing, converting, writing, and rereading models?
3. Can the successful numerical and data-handling approaches coexist in an
   extensible scientific framework?

These questions correspond to v0.1, v0.2, and the current v0.3 effort. They do
not describe strictly disjoint calendar periods.

### Sources and provenance

| Evidence | Role in this history |
|----------|----------------------|
| [Developer's account](./femera-version-history-prompt.md) | Research motivation, overall reported results, future direction |
| [v0.1 archive](../femera-v0.1/) | Numerical implementation, converters, tests, performance scripts |
| [v0.2 archive](../femera-v0.2/) | Modular data handling, processing infrastructure, API integrations |
| [Unpublished draft technical memorandum](../femera-prior-work/femera-v0.1-tm-DRAFT-2026-01-25.tex) | Mathematical explanation, experimental setup, performance interpretation |
| [Figures](../femera-prior-work/figs/) | Recorded comparisons, profiles, rooflines, decomposition and scaling studies |
| [Source data](../femera-prior-work/data/) | Timing samples, profiler exports, optimization records, comparison logs |
| [Near-billion-DOF full log](../femera-prior-work/solve-1gdof-iso-i100000-6148b-gcc.log) and [excerpt with PBS completion record](../femera-prior-work/solve-1gdof-iso-i100000-6148b-gcc.snip.log) | Original large-model execution, accuracy diagnostics, and job-resource evidence |
| [Current README](../../README.md) | v0.3 integration objective |

The numerical measurements were obtained on cluster compute nodes, not on this
development notebook. This history documents those experiments; it does not
attempt to reproduce their performance on different hardware. Inspecting
archived measurements is distinct from rerunning the experiments.

Selected figures and data were inspected, not every file in the collection.
The manuscript remains a draft with incomplete sections, placeholder values,
and provisional notes. Its analytical estimates, plotted measurements, and
unfinished material are identified separately below.

The checked-out archive snapshots are:

- v0.1: `1a5fee2e9bab9d49f0df6c10a4d956dd9a80c9c3`, dated 2024-07-02.
- v0.2: `35f81183ce8ff73132429117e11f7ca04886cdc7`, dated 2021-10-18.

Both clones retain shared history reaching `edcc5b6a`, dated 2019-01-17.
That is the beginning of the available repository history, not necessarily the
beginning of the research. The v0.2 README carries a 2018 NASA copyright notice.

## 2. Femera v0.1: reducing the cost of finite element response

### 2.1 A genuinely matrix-free element-by-element formulation

The [v0.1 README](../femera-v0.1/README.md) presents Femera as an
element-by-element (EBE), matrix-free implementation tuned for different
problem sizes and HPC architectures.

The central distinction is not simply the absence of an assembled global
sparse matrix. An EBE implementation can avoid global assembly while still
storing a dense stiffness matrix for every element. Femera's tensor-train
element-response approach instead evaluates the response through a sequence
of small tensor operations.

In conceptual terms:

```text
Nodal trial values
    -> element interpolation/gradient operations
        -> constitutive response
            -> integrated element contributions
                -> accumulated nodal response
```

The draft develops this formulation in global and material coordinates. Here,
"tensor train" refers to the element-response sequence described in the
manuscript; this history does not assume an additional general-purpose
low-rank tensor decomposition algorithm.

The [physics interface](../femera-v0.1/phys.h) explicitly distinguishes
`EBE_TTE` and `EBE_LMS`, and retains element-stiffness storage for comparison.
The [vectorized isotropic kernel](../femera-v0.1/elas-iso3-vect.cc) separately
estimates tensor-response and stiffness-matrix work and data movement.

The research therefore compares different representations of the operator,
not merely different compilers applied to the same sparse-matrix code.

### 2.2 What is retained instead of a stiffness matrix

The draft's methods section explains two assumptions used to reduce memory
traffic:

- A model is divided into homogeneous material regions or partitions, allowing
  material properties to be reused across many elements.
- Higher-order tetrahedral side nodes occupy their natural, straight-sided
  locations, allowing an element's geometric mapping to be represented by a
  constant Jacobian.

Precomputed inverse-transposed Jacobians and determinants support regular
access during repeated response evaluation. These geometric and constitutive
data are much smaller than stored element or global stiffness matrices.

The method is matrix-free, not data-free: it retains connectivity, geometry,
material information, solver vectors, and partition/halo data. The documented
assumptions also matter when interpreting the scope of the performance results;
they should not be generalized automatically to arbitrarily curved elements or
arbitrary per-integration-point material data.

### 2.3 Physics, element orders, and reference implementations

The numerical mini-app supports linear tetrahedra, quadratic tetrahedra, and
incomplete 20-node cubic tetrahedra. It includes isotropic and orthotropic
elasticity, thermal and thermoelastic work, and exploratory nonlinear/plasticity
paths.

Representative sources include:

- [Tetrahedral elements](../femera-v0.1/elem-tet.cc).
- [Orthotropic elasticity](../femera-v0.1/elas-ort3.cc).
- [Isotropic thermal response](../femera-v0.1/ther-iso3.cc).
- [Thermoelastic response](../femera-v0.1/elas-ther-iso3.cc).
- [Plasticity-related implementation](../femera-v0.1/elas-plkh-iso3.cc).

Material orientations and constitutive symmetry are part of the investigation.
The draft contrasts general 21-parameter elasticity with orthotropic,
cubic, and isotropic specializations. Evaluating in material coordinates can
exploit constitutive sparsity, while isotropic response can avoid unnecessary
coordinate transformations.

The archive contains baseline, reference, and vectorized implementations.
The presence of a source file is not a claim that every physics variant reached
identical verification or production maturity. The draft's nonlinear and
multiphysics examples remain substantially less developed in prose than its
linear-elastic analysis.

### 2.4 Solver and partition design

The [PCG implementation](../femera-v0.1/halo-pcg-omp.cc) and
[nonlinear CG implementation](../femera-v0.1/halo-ncg-omp.cc) operate on
partitioned models. The README documents Jacobi preconditioning, an experimental
nonlinear CG option, tangent-Jacobi selection, load increments, and iteration
and residual controls.

The draft describes duplicated shared nodes at partition boundaries. One
primary node gathers contributions before accumulated values are scattered
back to the contributing partitions.

This incurs some extra storage, but supports locality and latency hiding.
The intended evaluation separates halo-related work from interior work so that
synchronization and computation can overlap. The performance strategy therefore
combines:

- A smaller operator representation.
- Locality-conscious partition data.
- Halo synchronization organization.
- Efficient small-tensor computation.

It is not solely an arithmetic optimization.

## 3. What the v0.1 experimental record establishes

### 3.1 Cluster-node experiments across hardware generations

The draft identifies compute-node configurations spanning Westmere, Sandy
Bridge, Ivy Bridge, Haswell, and Skylake, among others. Its Skylake A/B entries
describe dual-socket Xeon 6148 systems with 40 total cores and different memory
capacities, approximately 96 and 375 GB in the draft's table.

The figures and filenames retain these distinctions. The
[cross-node quadratic-tetrahedron profile](../femera-prior-work/figs/femera-profile-nodes-tet10-elas-iso-gcc.png)
compares six configurations, including both Skylake variants.

The draft documents GNU 8.3.0 and Intel 2019.4.243 in its vectorization
discussion, and describes representative small, medium, and large tests around
5 thousand, 500 thousand, and 50 million DOFs.

These are single-node HPC experiments, even though the nodes belong to
clusters. The manuscript lists multi-node work as a future direction. It would
be incorrect to reinterpret a 40-core result as a demonstrated multi-node
distributed-memory scaling result.

### 3.2 A concrete engineering model: approximately 26 million DOFs

The draft identifies a lack-of-fusion pore model as an engineering example.
The [comparison figure](../femera-prior-work/figs/femera-26MDOF-6148.png)
reports the following rounded values on Skylake A:

| Implementation | Plotted wall-clock total | Reported memory |
|----------------|--------------------------|-----------------|
| Femera | 1 min 48 s | 2.7 GB |
| EBE local matrix storage | 6 min 6 s | 26 GB |
| SciFEN | 6 min 41 s | 41 GB |

The time bars stack read, initialization, and 2,007-iteration phases. They should
not be described as physics-kernel-only timings. The chart annotates time
ratios of approximately 3.4 and 3.7 relative to Femera.

Using the rounded memory labels gives approximately 9.6 times the memory for
stored-element matrices and 15.2 times for SciFEN. This example supports a
specific, substantial memory advantage; it does not establish one invariant
ratio for every problem and comparison method.

The source-data collection includes
[optimization measurements for this model](../femera-prior-work/data/LoF_pores-opt-steps.csv).
Those rows identify 25,932,897 DOFs, 400 partitions, 40 compute threads, and
2,007 iterations.

The associated [optimization-history chart](../femera-prior-work/figs/femera-opt-steps.png)
shows that mathematical reformulation and implementation refinement both
matter. Its plotted totals range from 8:47 for the initial Intel-compiler case
and 7:21 for the initial GNU case to 1:51 and 1:48 for the respective final
cases. Intermediate stages are labeled Reference, Baseline, and Intrinsics.

The optimization CSVs contain staged runs rather than a single homogeneous
population. They should not be averaged wholesale or assumed to map to every
figure bar without the original plotting transformation.

### 3.3 Repeated timing records corroborate a narrower comparison

The archived
[isotropic tensor-response samples](../femera-prior-work/data/profile-physics-tet10-elas-iso-6148-gcc.csv)
and [local-matrix samples](../femera-prior-work/data/profile-physics-tet10-elas-lms-6148-icc.csv)
provide a directly readable example of repeated measurements.

Both contain ten rows for a 49,834,668-DOF model, 3,360 partitions, 40 compute
threads, and 100 iterations. The CSV output expression in
[the numerical mini-app](../femera-v0.1/femera-mini.cc) identifies column 12
as iteration-loop seconds and column 13 as DOF-iterations per second.

Summarizing column 12 gives:

| Recorded implementation | Samples | Mean 100-iteration loop | Observed range |
|-------------------------|---------|-------------------------|----------------|
| Tensor isotropic, GNU build | 10 | 10.710 s | 10.623-10.819 s |
| Local matrix storage, Intel build | 10 | 33.052 s | 32.971-33.242 s |

The ratio of mean recorded loop times is approximately 3.09. This is an
arithmetic summary of existing cluster measurements, not a new benchmark.
The different compiler builds are part of the comparison; this is not a
controlled isolation of the operator representation alone.

It also measures iteration-loop throughput, not time to reach a prescribed
engineering accuracy. Full solution comparisons additionally depend on
preconditioning, stopping criteria, and convergence.

### 3.4 Memory comparisons depend strongly on element order

The [physics memory chart](../femera-prior-work/figs/femera-physics-memory-6148.png)
shows three bars per evaluation category, interpreted in the manuscript's
linear/quadratic/cubic tetrahedral context.

For elastic sparse-matrix evaluation, the chart annotates memory ratios of
approximately 2.7, 14, and 17 relative to the tensor-response baseline. For
elastic local-matrix storage, the annotations are approximately 5.4, 8.9, and 12.

This gives a more informative account than a blanket "ten times less memory":
the benefit varies with polynomial order and stored representation.
The thermal sparse-matrix category is explicitly labeled `EST`, so those bars
must be treated as estimates rather than additional measured solver runs.

The companion
[physics time chart](../femera-prior-work/figs/femera-physics-time-6148.png)
uses normalized total CPU-time units, core-seconds per million DOFs. It is not
the same metric as the engineering example's elapsed wall-clock total. Its
elastic sparse-matrix annotations range from about 1.21 to 5.2, while the
local-matrix annotations are about 3.0-3.5.

The developer's summary of roughly three- to sixfold speedups and greater than
tenfold memory savings is consistent with important parts of this record, but
the figures also document cases outside those ranges. The historical conclusion
is a strong, workload-dependent advantage, not a universal multiplier.

### 3.5 Vectorization and rooflines explain the performance

The [shared tensor kernels](../femera-v0.1/phys-inline.cc) contain explicit AVX
operations. The draft reports that the tested compilers could generate useful
SSE code for several kernels, but did not automatically produce efficient
wider-vector implementations of the tensor physics kernels.

The chosen implementation pads some three-component data to four components.
Using three of four double-precision AVX lanes can still outperform a nominally
more compact layout when instruction selection, access patterns, and small
tensor operations improve.

The [tensor-response roofline](../femera-prior-work/figs/femera-roofline-6148-50mdof-tet10-iso.png)
distinguishes gradient-related, constitutive, force-accumulation, and PCG
kernels. The [local-matrix roofline](../femera-prior-work/figs/femera-roofline-6148-50mdof-tet10-lms.png)
shows the different response-evaluation balance.
An [Advisor export](../femera-prior-work/data/advixe-50mdof-tet10-iso-6148-icc.csv)
preserves profiler measurements behind this line of analysis.

These plots distinguish measured and effective arithmetic work. More executed
FLOPs do not necessarily mean a slower implementation: padding or vector
operations may execute extra arithmetic while reducing elapsed time.
Nor is raw GFLOP/s interchangeable with useful solved-model throughput.

The draft separately tabulates potential DRAM-limited speedups, including
estimated iteration advantages of 4.9 for P2 and 11.0 for P3 against SpMV.
Those are analytical estimates under a bandwidth model, not measured
application speedups to be merged into the timing results.

### 3.6 Model size, over-decomposition, and concurrent small solves

The [cross-node profile](../femera-prior-work/figs/femera-profile-nodes-tet10-elas-iso-gcc.png)
shows aggregate iteration throughput changing substantially with model size.
On the Skylake configurations, the plotted rate is around a billion
DOF-iterations per second for small cases and roughly half that at large sizes.
This is repeated response/iteration throughput, not a claim that a complete
billion-DOF engineering problem converges in one second.

The [quadratic decomposition study](../femera-prior-work/figs/femera-decomp-p2-iso-6148b.png)
compares fixed partition counts from 40 through 40,960 and concurrent-model
strategies. The curves cross: a useful configuration for a large model can
impose excessive overhead on a small one.

This supports two complementary strategies:

- Over-decompose large models to improve locality and working-set behavior.
- Solve multiple smaller models concurrently when one small model cannot use
  the whole node effectively.

The corresponding [partition sweep data](../femera-prior-work/data/part-uhxt265-338p2-elas-iso-6148b-gcc.csv)
contains repeated runs at model scales of hundreds of millions of DOFs.
The archive's [strong-scaling script](../femera-v0.1/perf/scale-strong.sh)
also controls OpenMP placement, binding, and partition/core combinations.

Over-decomposition is therefore an experimental tuning parameter, not a rule
that the largest possible number of partitions is always best.

### 3.7 Scaling records include hardware limitations

The [large-model scaling figure](../femera-prior-work/figs/femera-scale-50mdof-6148-iso.png)
plots per-core throughput for strong and weak scaling, rather than only total
node throughput. It explicitly marks a thermal-throttling region at higher
core counts.

That annotation is historically important: not every decrease in per-core
throughput can be assigned to a software synchronization defect. Frequency,
thermal behavior, memory bandwidth, and parallel overhead all affect scaling.
Conversely, the chart should not be summarized as ideal linear scaling.

A [SciFEN comparison log](../femera-prior-work/data/scifen-dat/scifen-tet10-6148o.log)
also preserves a dated July 2019 cluster job using 40 allocated CPUs and 40 MPI
processes, together with software/dependency details. This is direct documentary
evidence of the cluster comparison context, not a notebook experiment.

### 3.8 Near-billion-DOF validation: corroborated by execution records

The subsequently supplied
[full solve log](../femera-prior-work/solve-1gdof-iso-i100000-6148b-gcc.log)
corroborates the large-model calculation described in the manuscript's final
notes. The [concise excerpt](../femera-prior-work/solve-1gdof-iso-i100000-6148b-gcc.snip.log)
also preserves a PBS completion notification dated 2020-01-19, with exit status
zero. This dates the execution evidence much earlier than the 2026 draft.

The record identifies AVX2 execution, 40 OpenMP threads, Jacobi preconditioning,
a zero initial solution, and a unit-cube domain. The filename and manuscript
identify the 6148/Skylake B configuration; the solver text itself establishes
the thread count and execution characteristics, not the CPU model independently.

| Quantity | Recorded value |
|----------|----------------|
| Elements | 240,323,876 |
| Unique system nodes | 318,067,976 |
| Degrees of freedom | 954,203,928 |
| Partitions | 40,000 |
| OpenMP threads | 40 |
| Partition-local node entries, including duplication | 383,868,090 |
| Read/setup time | 112.441223 s |
| Solver initialization time | 53.229206 s |
| Iterations executed | 100,000 |
| Iteration-loop wall time | 187,811.734375 s, approximately 52.17 hours |
| PBS job wall time | 52:13:21 |
| PBS reported memory | 108,273,972 kb |
| PBS reported virtual memory | 111,594,056 kb |
| Reported iteration throughput | 508.06 million DOF-iterations/s |
| Reported physics rate | 454.65 GFLOP/s |
| Reported physics arithmetic intensity | 1.24 FLOP/byte |

The distinction between unique nodes and partition-local entries exposes the
storage cost of duplicated halo nodes. The PBS resource values are retained in
their original units; its `ncpus=1` accounting field should not be substituted
for the solver's explicit 40-thread execution record.

The run was configured with a zero relative-tolerance target and an explicit
100,000-iteration limit. It finished at that limit. Therefore, it should not
be described as requiring 100,000 iterations to reach a normal engineering
accuracy target. The recorded iterative residual diagnostic falls from
approximately `7.18e3` initially to `8.61e-5` at iteration 10,000 and `1.02e-82`
at iteration 100,000.

The final analytical-comparison diagnostics report:

- Average normalized displacement-error magnitude: `1.05e-13`.
- Maximum normalized displacement-error magnitude: `5.63e-13`.
- Response stress: `1.00e8` Pa.
- Extension strain: `1.00e-3`.
- Inferred modulus: `1.00e11` Pa.
- Reported modulus error: `1.33e-14`.

The error values above retain the log's normalized-error interpretation, rather
than being relabeled as dimensional displacement errors. The separate final
comparison block prints `R2 = 9.31e-18` and `||R|| = 3.05e-9`; it should not
be conflated with the iterative solver's `1.02e-82` diagnostic. In particular,
the latter alone is not evidence of an equally small true residual or forward
solution error in floating-point arithmetic.

The timing breakdown reports 95.10% combined work and 4.90% synchronization
and overhead, plus a load-balance percentage of 101.69%. These are the program's
reported accounting metrics, not a separately measured parallel efficiency.

Together, the logs provide direct evidence of successful execution and an
analytical solution check at nearly one billion DOFs on a single compute node.
They strengthen the historical claim beyond a draft-only account without
turning one test into proof of correctness for every formulation or workload.

## 4. Why a fast solver led to a data-handling research line

The v0.1 numerical experiments used a deliberately narrow preparation pipeline.
The [Gmsh converter](../femera-v0.1/gmsh2fmr.cc) reads Gmsh version 2 ASCII
meshes, adds loads, constraints, materials, and orientations, and writes
partitioned bespoke `.fmr` files.

```text
Geometry / meshing
    -> mesh files
        -> conversion and model annotation
            -> partitioned .fmr files
                -> solve
```

That pipeline supported the kernel experiments and microstructure examples,
but it made model preparation and intermediate serialization distinct costs.
The README even warns of races in the OpenMP converter and recommends its
serial counterpart.

For very large models, file size and data movement can become bottlenecks.
For UQ and parameter studies, the repeated cost across thousands of analyses
can be just as important. Faster numerical kernels make those non-solver costs
a larger fraction of the workflow.

This is the motivation for v0.2: widen the performance objective from the
iterative solve to the complete preparation-and-analysis pipeline.

## 5. Femera v0.2: a modular data-handling experiment

### 5.1 Scope and chronology

The [v0.2 README](../femera-v0.2/README.md) explicitly focuses on modular HPC
architecture and flexible data handling. Commit `f2676376`, dated 2021-02-26,
records initialization of the data-handling mini-app.

The developer explains that this version intentionally did not implement the
physics and solver. The nearly empty
[physics source](../femera-v0.2/src/Phys.cpp) supports that distinction.
The substantive work lies in processing, simulation preparation, and data
backends, not in reproducing v0.1 numerical solves.

The numerical branch nevertheless continued evolving afterward. Version
numbers describe research emphasis, not a strict chronological replacement.

### 5.2 Work hierarchy and typed data

The [Work interface](../femera-v0.2/src/Work.hpp) supplies lifecycle,
preparation/checking operations, task organization, shared processing/data
references, and performance meters. A source comment already considers a
future CRTP refactoring.

The [Data interface](../femera-v0.2/src/Data.hpp) manages metadata and typed
integer/geometry arrays, including caches intended to avoid repeated reads and
to marshal data for distribution. Its comments explicitly warn against
unnecessary copies and note thread-safety concerns.

The [dispatcher](../femera-v0.2/src/Data.cpp) selects data handlers, scans
metadata, and delegates preparation operations. Simulation-side
[Mesh code](../femera-v0.2/src/Sims/Mesh.cpp) requests connectivity and
Jacobian-related data through those abstractions.

This changes the application question from "which bespoke file must I read?"
to "which data does this operation need, and which backend can supply it?"

### 5.3 Gmsh as an in-process service

The [Gmsh backend](../femera-v0.2/src/Data/Dmsh.cpp) integrates model selection,
mesh generation, partition-related operations, data access, and visualization
support through library APIs.

The key workflow is:

```text
Parameterized geometry
    -> in-process model and mesh generation
        -> extraction of required simulation data
            -> optional visualization and file output
```

This is not a claim of zero I/O or zero-copy access. Geometry inputs and final
outputs can still be files, and data extraction can involve copies. The
architectural gain is removing mandatory serialization and reload of every
intermediate stage.

The developer reports major throughput improvements and practical HPC use of
Gmsh despite its not being primarily an HPC application. The inspected source
supports the mechanism; the newly supplied numerical dataset does not by
itself quantify a v0.2 end-to-end speedup.

### 5.4 CGNS and the finite element data-model problem

The [CGNS backend](../femera-v0.2/src/Data/Dcgn.cpp) explores metadata,
coordinate/connectivity access, and serial or parallel access modes.
Its [declarations](../femera-v0.2/src/Data/Dcgn.hpp) record mappings for
material, boundary-condition, and solver information.

The developer identifies a lack of a satisfactory open standard covering the
targeted finite element elasticity model workflow. CGNS's CFD heritage made
it a useful data-handling candidate, but not a ready-made solution to every
elasticity-specific semantic requirement.

This is more precise than asserting that no open mesh standards exist:
geometry/connectivity exchange is not the same as a complete, interoperable
elasticity simulation specification.

### 5.5 Processing, library coordination, and actual run evidence

The [processing modules](../femera-v0.2/src/Proc/) include MPI, OpenMP, and
GoogleTest. Backend code coordinates library state, initialization, model
selection, and concurrency rather than assuming every dependency is freely
thread-safe.

The [March 2021 example output](../femera-v0.2/examples/mini-detail-2021-03-04.out)
records a concrete combined configuration: two MPI processes with two OpenMP
threads each, GoogleTest integration, and both CGNS and Gmsh inputs.
It shows the modular workflow in operation, rather than just declared class
names. Its development-machine hardware should not be confused with the
cluster nodes used for v0.1 performance measurements.

Historical commits record race-condition work, limits on simultaneous Gmsh
models, and a June 2021 visualization implementation initially restricted to
one OpenMP thread per MPI rank. Such details qualify the integration claims:
useful library-based HPC workflows required explicit coordination and testing.

### 5.6 Repeated geometry and visualization

The [fiber-shape RVE example](../femera-v0.2/tests/geo/fiber-polygon-anim.geo)
uses a frame parameter to vary geometry. OpenCASCADE-based construction,
meshing infrastructure, and view configuration connect repeated model
generation to animation.

Commit `35f81183`, dated 2021-10-18, adds this composite-fiber RVE animation
demonstration. Earlier June/July commits record visualization and draw/save
timing work.

The example is a useful bridge to repeated-analysis research, but is not
evidence of a completed physics solve in the data-only mini-app.
The [size-sweep driver](../femera-v0.2/tests/run-sizes.sh) also targets
many-model preparation with elapsed-time and memory accounting.

### 5.7 Extensibility was already a design objective

The [Plug module](../femera-v0.2/src/Main/Plug.cpp) registers compiled-in
processing and data modules and constructs selected task types. Its
[header](../femera-v0.2/src/Main/Plug.hpp) includes a user identifier boundary.
The [developer guide](../femera-v0.2/docs/developer-guide.txt) discusses plugin
locations and possible future integrations.

This is evidence of modularity and extension intent, not proof of a completed
runtime shared-library loader or stable plugin ABI. The examined path still
uses explicitly compiled registrations and construction switches.

## 6. Selected development milestones

| Date | Commit or record | Significance |
|------|------------------|--------------|
| 2019-01-17 | `edcc5b6a` | Shared initial repository history |
| 2019-07-29 | Archived SciFEN cluster log | Dated 40-process comparison context |
| 2020-01-19 | Near-billion-DOF log and PBS completion record | Successful 954,203,928-DOF, 40-thread, 40,000-partition run; 100,000 iterations and approximately 52 hours |
| 2020-01-23 | v0.1 `11c14b2a` | Halo-value storage optimization |
| 2020-02-19 | v0.1 `1ca07653` | Isotropic thermal physics milestone |
| 2020-02-21 | v0.1 `438d207d` | Isotropic thermal AVX implementation |
| 2021-02-26 | v0.2 `f2676376` | Data-handling mini-app initialized |
| 2021-03-04 | Archived example output | Combined MPI/OpenMP and Gmsh/CGNS workflow |
| 2021-04-10 | v0.2 `2ffb5de5` | On-demand Gmsh meshing/partitioning milestone |
| 2021-04-20 | v0.2 `7a9c0d45` | Gmsh Jacobian/determinant data access |
| 2021-04-24 | v0.2 `20fea1b9` | CGNS coordinate-reading milestone |
| 2021-05-24 | v0.2 `db890129` | Race-condition and open-model-limit work |
| 2021-06-22 | v0.2 `ed751792` | Visualization milestone with a stated concurrency restriction |
| 2021-07-13 | v0.2 `5ef656d3` | Draw/save timing instrumentation |
| 2021-10-18 | v0.2 `35f81183` | Composite-fiber RVE animation demonstration |
| 2024-03-29/30 | v0.1 `474ce953`, `428c032b` | Further orthotropic thermal experimentation |
| 2024-07-02 | v0.1 `1a5fee2e` | Converter segfault fix; numerical archive snapshot |
| 2026-01-25 | Draft manuscript filename | Theory/performance write-up preserved as a draft, not a release date |

Commit descriptions identify development milestones, not certification that
every related feature was complete. The manuscript's filename also does not
date the underlying measurements.

## 7. Femera v0.3: synthesis and generalization

The [current README](../../README.md) states the central objective: maintain
the numerical performance demonstrated by v0.1 while providing the flexible
data handling demonstrated by v0.2.

The current organization separates processing, data, tasks, tests, physics,
grids, and algorithms under `Work`, with CRTP implementation machinery.
This is a reimplementation and integration effort rather than a simple
packaging of the old executables.

The two earlier lines establish complementary requirements:

- Preserve the compact representation and efficient repeated numerical work.
- Avoid making file conversion the compulsory boundary between every tool.
- Support large individual models and batches of smaller related models.
- Keep optional libraries behind stable application interfaces.
- Make new scientific implementations possible without rewriting the core.

The developer plans finite-difference and related methods for structured-grid
workloads where finite elements may be an unnecessarily expensive choice.
That is a future direction, not a completed capability established by these
archives.

Recent [runtime-interface](./femera-runtime-interfaces-plan-2026-10-26.md)
and [user-plugin](./femera-user-plugin-capabilities-notes-2026-10-26.md)
discussions address the extensibility side of the synthesis. They are proposals,
not historical implementations or retroactive descriptions of v0.1/v0.2.

## 8. Overall historical interpretation

Femera's development progressively widens the boundary of the performance
problem.

**v0.1 demonstrated a different cost structure for finite element response.**
The figures, repeated measurements, and profiler records document meaningful
time and memory advantages on cluster nodes, while also exposing dependence
on element order, compiler, model size, partitioning, and hardware behavior.

**v0.2 investigated how to keep those numerical gains from being overwhelmed
by the surrounding workflow.** Its contribution is modular, API-driven data
handling, including in-process meshing and visualization, rather than another
iteration of the numerical solver.

**v0.3 seeks to combine both in an extensible scientific toolchain.** The target
is neither only a fast kernel nor only a collection of file readers. It is a
framework in which numerical methods, data sources, execution strategies, and
researcher-supplied capabilities can cooperate without unnecessary storage,
conversion, or abstraction costs.

The available record supports that research progression without requiring an
attempt to reproduce historical compute-node performance on a notebook, and
without presenting draft estimates or planned features as completed results.
