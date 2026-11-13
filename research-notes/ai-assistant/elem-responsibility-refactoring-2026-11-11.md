# Separating element topology, tiling, basis functions, and Jacobians

## Recommendation

Yes, a **separation by responsibility** is worthwhile, especially if structured
mesh generation, higher-order bases, or embedded elements are going to grow.
This is an early-development internal API, and there is no need to preserve the
existing API. Design the new boundaries directly, update their internal callers
and tests, and remove obsolete entry points rather than adding compatibility
facades or wrappers.

Prefer a reference topology descriptor, a reference-basis policy, an optional
structured-tiling policy, and geometric mapping functions. Do not assume that
`Elem` needs to survive: keep a generic element type only if it represents a
useful composition or provides genuinely shared topology logic. Otherwise,
replace it with the smaller concepts. Do not start with a large class hierarchy
or mechanically move every constant. Some constants belong to the reference
element itself, while others depend on basis, tiling, geometry, or layout.

The strongest reason is not the length of `Elem.hpp`: it is that the concepts
vary independently. A bar can have the same topology but a different basis
order; the same basis can be evaluated on many physical bars; and a bar in an
imported, unstructured mesh does not need structured-tiling operations.
Nevertheless, this is a design improvement, not a prerequisite for all current
uses. If no other basis or meshing strategy is planned, a smaller extraction
of tiling alone may give most of the benefit with less churn.

This note is an analysis only. Names and interfaces below are illustrative.

## What the repository currently does

- [`Elem<T, P, F>`](../../src/femera/grid/Elem.hpp) is a CRTP-style,
  stateless facade. It exposes topology and reference coordinates, node
  counts, basis and Jacobian **size helpers**, and structured-tiling wrappers.
  Most tiling wrappers dispatch to static `T::calc_tile_*` methods. The generic
  count and size formulas are in [`Elem.ipp`](../../src/femera/grid/Elem.ipp).
- [`Bars`](../../src/femera/grid/elem/Bars.hpp) derives from
  `Elem<Bars>` (default order 1), and friends all `Elem` specializations.
  `Elem<Bars, 2>` and `Elem<Bars, 3>` supply the other orders. `Bars` holds
  private topology/reference data and tiling constants and implementations;
  its P1/P2/P3 shape-function and gradient functions are public static
  methods in [`Bars.ipp`](../../src/femera/grid/elem/Bars.ipp). They are not
  automatically selected by `Elem`'s `P`: callers choose the named method.
- The actual Jacobian multiplication is **already outside `Elem`**, as free
  functions in [`fmr/fems/elem.hpp`](../../src/fmr/fems/elem.hpp) and
  [`elem.ipp`](../../src/fmr/fems/elem.ipp). `Elem`'s active Jacobian-related
  methods are buffer-size calculations; its old Jacobian declarations are
  disabled. Moving only those size methods would not refactor the kernels.
- [`Tets`](../../src/femera/grid/elem/Tets.hpp) also contains shape formulas,
  named by node count rather than `Bars`' polynomial-order names. The shape
  headers currently searched have `calc_tile_*` implementations only for
  `Bars`, so tiling must remain an optional capability rather than a required
  operation for every element.
- A source search found the relevant `get_tile_*`, `calc_poly_*`, and
  `elem_jacs_*` uses primarily in the element/kernel tests and
  [`Bars.perf.cpp`](../../src/femera/grid/elem/Bars.perf.cpp).
  [`Bars.gtst.cpp`](../../src/femera/grid/elem/Bars.gtst.cpp) also checks
  empty types and constexpr tile dispatch. These are useful design and
  performance properties to evaluate, not requirements to preserve an API.

## Boundaries and ownership

| Responsibility | Proposed owner | What it depends on |
| --- | --- | --- |
| Reference topology | `Bars` or `BarTopology` | Reference dimension, vertices, edges, local connectivity, canonical reference vertex coordinates and their documented storage layout. Not physical bar coordinates. |
| Basis / reference interpolation | `BarBasis<P>` or `Basis<BarTopology, P>` | Polynomial family, order, node ordering, reference nodes, value and reference-gradient evaluation, basis buffer sizes. A basis is not necessarily uniquely determined by topology and order alone. |
| Structured mesh generation | `BarTiling<P>` or `StructuredTiling<BarTopology, P>` | Tile pattern, partition dimensions, numbering, order, orientation, periodic policy, tile counts and coordinates. |
| Geometric mapping | Free functions or `GeometryMap`/`Jacobian` service, preferably building on `fmr::fems::elem` | Geometry-basis gradients, physical node coordinates, reference and ambient dimensions, scalar precision, layouts, integration points; no ownership of mesh or material state required. |
| Optional composition | A lightweight element definition, if useful | Associates topology and basis without duplicating their APIs. It should not be required for independent use of tiling or geometry kernels. |

`get_elem_node_coor` and the quality calculation require separate ownership
decisions: reference-node construction belongs with the basis if it describes
its reference nodes; mesh-quality evaluation belongs with geometry/quality
algorithms operating on physical coordinates and topology connectivity.

Reference element vertex coordinates and topological connectivity remain
associated with the reference shape. Node count and polynomial family are
better associated with a **particular** basis; for example, complete and
serendipity spaces need not have the same nodes. Tiling counts are properties
of a tile pattern, not necessarily equal to element counts: `Bars` happens to
be self-tiling. Physical Jacobians depend on a particular set of coordinates
and evaluation points and should not be stored as global shape constants.
Quadrature rules and cached values, if needed, should be separately identified
by basis, rule, precision, and layout rather than hidden in topology.

The current `get_elem_node_coor` interpolates nodes from vertices according to
the current ordering and P1-P3 assumptions. It is not a general curved-element
geometry generator; putting it on a basis or reference-node helper might make
that assumption more explicit. A geometric map may use a geometry basis and a
solution basis of different orders, so avoid baking the solution `P` into
every Jacobian calculation.

## Concrete `Bars` example

For the existing reference interval `xi in [0, 1]`, the P1 basis has
`N = [1 - xi, xi]` and `dN/dxi = [-1, 1]`. The current node ordering for P2
is endpoints then midpoint, and P3 is endpoints then the nodes at `1/3` and
`2/3`. Preserve that convention consistently across the basis, reference
nodes, and tile connectivity.

A possible static API, shown as pseudocode rather than compilable C++:

```cpp
using Topology = BarTopology;            // reference dimension 1, two vertices
using Basis = BarBasis<2>;               // P2, three nodes, known local order
using Tiling = StructuredBarTiling<2>;   // structured numbering for that order

Basis::values(N, xi);
Basis::reference_gradients(dN_dxi, xi);
Tiling::node_connectivity(conn, tile_index, partition_elements);
geometry::jacobians(J, physical_nodes, dN_dxi,
                    integration_point_count, /*reference_d=*/1,
                    /*ambient_d=*/3, layouts);
```

For example, with four elements in a nonperiodic partition and tile index 2,
P2 node connectivity is `[4, 6, 5]`; P3 is `[6, 9, 7, 8]`. A P2
partition of three elements has seven nodes. A periodic **vertex**
connectivity operation wraps the last endpoint to zero; the existing API
does not provide periodic node connectivity. In a new API, choose and document
whether periodic node connectivity is supported separately. The existing
connectivity methods allow an explicit `p` to override template `P`; prefer
one source of truth in the redesigned API, such as `StructuredBarTiling<P>`,
unless a real runtime-order use case warrants a separate runtime interface.

For a P1 physical bar with endpoints `X0` and `X1`, the geometric Jacobian is
the column `J = X1 - X0`, evaluated in the chosen ambient dimension. In 3D,
`X0 = (2, 3, 5)` and `X1 = (7, 11, 16)` give `J = [5, 8, 11]^T`.
`J` is the same at every integration point for this linear geometry, and its
length scaling is `sqrt(210)`. A higher-order bar with independently placed
geometry nodes may have a position-dependent Jacobian; do not optimize it as
constant just because its topology is a bar.

## Implementation approaches

### A. Small static policies and free geometry kernels (recommended)

Introduce a topology descriptor, an opt-in tiling policy, and an
order/family-specific basis policy. Adapt the existing `fmr::fems::elem`
mapping kernels to the new dimension and layout contracts instead of keeping
duplicate Jacobian loops. Make topology information available through a narrow
internal interface; `Bars`' private members are currently accessible to `Elem`,
not to an arbitrary new policy. Replace the old internal entry points and
update callers directly.

**Pros:** independent, testable capabilities; compile-time dispatch and
inlining; stateless policy types. Policies can be class templates or specialized
traits, while geometry remains a set of free functions.

**Cons:** more names and headers, and potential template diagnostics and code
size. Avoid cyclic dependencies:
topology should not depend on basis/tiling, and low-level `fmr` kernels should
not acquire a reverse dependency on `femera` shape types.

### B. Narrow namespace/free-function extraction

Move bar tiling and basis algorithms into separate namespaces or headers with
explicit inputs. Metadata can use small traits or topology constants. Adapt
the existing Jacobian free functions to the chosen geometry contract.

**Pros:** smallest conceptual change, no objects or inheritance, convenient
for kernel-level testing and layouts. **Cons:** a loose collection of functions
may make valid shape/order/family combinations less discoverable, and explicit
metadata plumbing can become repetitive. This is a good first step if more
generic policies are not yet needed.

### C. Composed runtime services or polymorphic interfaces

Have a mesh builder consume a tiler object, and an evaluator consume a basis
and mapping interface selected at runtime. Topology remains a small descriptor.
This can be useful for plugins or inputs that choose shape/order at runtime.

**Pros:** runtime selection and isolated responsibilities. **Cons:** more
ownership/lifetime and error-handling decisions; virtual or indirect calls
inside point loops can inhibit inlining and vectorization. Prefer selecting
once per batch and calling statically dispatched kernels within hot loops;
benchmark rather than assuming a cost. No evidence here requires this
complexity now.

Inheritance mixins are another way to keep the current method surface, but
they risk replacing one large base class with several interdependent bases,
ambiguous names, and the same CRTP/private-access issues. Prefer explicit
policies or functions unless inheritance gives a demonstrated benefit.

## Critical correctness and API decisions

1. **Reference vs. ambient dimensions.** The current size helpers use
   `sims_d * sims_d`, and all three existing Jacobian kernels loop over
   `sims_d` for both axes. `Bars::sims_d` is fixed at 3. For an element of
   reference dimension `r` in ambient dimension `s`, a compact Jacobian is
   `s x r`, with `intp_n * s * r` entries. A compact reference gradient needs
   `intp_n * node_n * r` entries. Keep dimensions explicit and validate
   `r <= s`; the ambient dimension should come from the physical context,
   not the reference topology.
2. **Choose and document layouts.** Current `Bars::calc_poly_grad_node_C1D2N`
   writes dimension-major `[dimension][node]`, while `calc_poly_node_grad_C1D2N`
   writes node-major `[node][dimension]`, each padded to three components.
   The `fmr` reference kernels consume node-major gradients and the `_a`
   kernel consumes dimension-major gradients. The test-local variable names
   in [`fmr/fems/elem.gtst.cpp`](../../src/fmr/fems/elem.gtst.cpp) are
   opposite to the `Bars` names; document indexing, not just `grad`/`grat`.
   Prefer compact rectangular storage with explicit indexing and choose
   supported layouts based on real kernel needs. Update callers and tests
   together; do not retain padded layouts merely for compatibility.
3. **Higher-order gradient correctness.** P2/P3 bar value formulas depend
   only on `xi`, but their current gradient implementations set derivatives
   of `L1` in the unused directions to `-1`, unlike P1's zero padded
   components. Do not blindly bless or copy these as correct 3D derivatives:
   add independent derivative tests and correct the new implementation to
   match its reference-coordinate definition. Existing P1 tests do not
   establish P2/P3 correctness.
4. **Measures and inverses.** For square maps, distinguish signed determinant
   from its absolute integration measure. For full-column-rank embedded maps,
   use `sqrt(det(J^T J))` for measure and a metric/pseudoinverse-based
   transformation when appropriate; no ordinary determinant or inverse exists
   for rectangular `J`. Define behavior for zero-length/degenerate maps and
   orientation separately. Current `jacd_size` only computes a size; the
   inspected kernel file does not implement a determinant.
5. **Define error and shape contracts deliberately.** Decide on caller-owned
   buffers, lengths, error reporting, origin defaults, and coordinate dimension.
   Several current bar tiling functions write outputs before returning false
   for an out-of-range index; periodic vertex connectivity returns false
   immediately for zero partition size. Prefer documented validation before
   writing where practical, rather than inheriting these behaviors by accident.
   `tile_dirs` is accepted but ignored by the current bar coordinate function:
   either implement meaningful orientation semantics or remove it from this
   API. Retain constexpr dispatch if useful and make unsupported tiling fail
   clearly without requiring all shapes to implement it.
6. **Language and performance.** The root [`Makefile`](../../Makefile)
   selects C++14; avoid assuming C++20 concepts or inline variables. Keep
   scalar types and conversion rules explicit (`Geom_float`, `Phys_float`,
   `F`, `Y`, and `G` are not interchangeable by assumption). Compare
   [`Bars.perf.cpp`](../../src/femera/grid/elem/Bars.perf.cpp) before and
   after meaningful extraction, including build/code-size effects if relevant.

## Suggested implementation sequence and verification

1. **Define the internal contracts first.** Choose the supported topology,
   basis, tiling and geometry interfaces, ordering, layouts, error semantics,
   and which values are compile-time or runtime. Inventory internal callers
   so they can be changed together. Use existing code as evidence, not as a
   compatibility specification.
2. **Separate structured tiling for bars.** Move tile metadata and routines
   behind a stateless opt-in policy or free functions. Check P1/P2/P3 vertex
   and node numbering, partition counts, boundary/empty cases, orientation,
   and coordinate outputs. Other shapes must remain usable without a tiler.
3. **Separate basis evaluation and metadata.** Provide explicit order/family
   selection and use unambiguous layout names. Test P1-P3
   Kronecker values at every reference node, partition of unity, derivative
   sums, and independent finite-difference/analytic derivatives at multiple
   interior points; check each supported gradient layout and buffer guards.
   Correct the P2/P3 unused-direction behavior rather than carrying it forward.
4. **Implement dimension-aware geometry mapping.**
   Cross-check AoS/SoA and node-major/dimension-major indexing at multiple
   integration points, using actual basis gradients. Test 1D bars in 1D, 2D,
   and 3D, a 2D reference surface embedded in 3D, and square 3D solids;
   verify exact sizes, translated/rotated/scaled bars, multi-point strides,
   nonconstant higher-order geometry, singular cases, and measure. Compare
   applicable square results against the old kernels as a useful cross-check,
   not as the sole oracle. The synthetic multi-dimensional tests for bars
   currently exercise square multiplication, not a physically correct embedded
   bar map.
5. **Update internal callers and remove obsolete code.** Run focused `Bars`,
   `Elem`-related, and `fmr::fems::elem` tests and the relevant performance
   target, followed by wider build/tests as needed. Update documentation with
   dimensions, layouts, ordering, and boundary semantics. Keep `Elem` only if
   its remaining role justifies it.

The earlier [embedded-element note](embed-lowd-elems-2026-11-06.md) provides
additional context on rectangular Jacobians and mixed-dimensional assembly.
