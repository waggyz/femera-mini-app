# Requirements: Element geometry data for finite-element partitions

## Purpose and scope

A future finite-element partition will own an unstructured collection of
homogeneous elements and initialize contiguous, partition-local geometry data
for matrix-free element-by-element kernels. This note defines what that caller
needs from the element/geometry API, with `Elem` as the element-level entry
point unless later responsibility work establishes a better owner.

The partition and `Elem` have different responsibilities:

- The partition owns mesh traversal, local-to-global connectivity, gathering
  physical coordinates, allocating/filling storage for all its elements,
  thread-local lifetime, and invalidating/rebuilding geometry data when the
  mesh geometry changes.
- `Elem` or a geometry kernel owns the calculation for one element: reference
  basis gradients at integration points, the physical Jacobian, and the
  dimension-appropriate transform and measure data.
- `Tile` may provide optimized structured-partition constants where geometry
  is shared. That optimization is separate from the per-element path required
  for unstructured meshes.

This is a geometry-data contract, not an element stiffness/residual API. It
does not make mixed-dimensional elements automatically compatible with a
solid's displacement degrees of freedom or assembly.

## Current API gap

[`Elem<T, P, F>`](../../src/femera/grid/Elem.hpp) exposes the reference
dimension and sizing helpers. It does not expose a mapping classification
method or a method to create precomputed Jacobian/transform-and-measure data.
The current Jacobian functions in
[`elem-jacs.hpp`](../../src/fmr/fems/elem-jacs.hpp) and
[`elem-jacs.ipp`](../../src/fmr/fems/elem-jacs.ipp) calculate the forward
Jacobian from caller-supplied coordinates and reference gradients. They do
not calculate its inverse, a determinant, an embedded measure, or a
transformed-gradient operator, and they do not gather coordinates from a
mesh.

The dimensions do let a caller determine the case itself:

- `sims_d == elem_d`: square mapping.
- `sims_d > elem_d`: lower-dimensional mapping embedded in the ambient space.
- `sims_d < elem_d`: invalid mapping and should be rejected.

This comparison is not yet a named `Elem` operation or a guarantee that the
appropriate precomputed quantities can be produced.

## Required element-level capabilities

### 1. Explicit mapping dimensions and classification

At initialization, the caller must provide or otherwise establish:

- `r = elem_d`, the reference/topological dimension;
- `s = sims_d`, the physical/ambient coordinate dimension;
- geometry node coordinates and their declared AoS or SoA layout;
- geometry-node count and geometry-basis gradients at each integration point;
- integration-point count and a documented integration-point ordering.

The element-level API must make the square/embedded/invalid distinction
explicit, either through a named query or a typed result from the
precomputation operation. Do not infer the meaning of an output scalar from
buffer length or assume `s == r` from the topology. `s` belongs to the
physical geometry context, not to `Bars` topology.

### 2. Forward Jacobian and reference-gradient contract

For each integration point, calculate

`J[a,i] = sum_k X[k,a] * dN[k,i]`,

where `J` has shape `s x r`, `a` is an ambient coordinate, and `i` is a
reference coordinate. Reference gradients have `r` components per geometry
node. Layout, strides, scalar type, and buffer capacities must be explicit.
The canonical compact Jacobian layout should be row-major
`[integration_point][ambient_row][reference_column]`; the partition should be
able to append each element's result without padding for absent dimensions.

### 3. Square-map outputs

For `s == r`, the per-integration-point geometry data needed by standard
matrix-free kernels includes:

- the signed determinant `det(J)`, retained when orientation matters;
- the inverse transpose `J^{-T}` for transforming reference gradients to
  physical gradients;
- if kernels require a nonnegative integration multiplier, the measure
  `abs(det(J))` must be available distinctly from the signed determinant.

The API and storage must not conflate determinant, absolute measure, and
reciprocal determinant. Singular or invalid square maps must have explicit
status/error behavior; they must not yield success-shaped data or silently
substitute zero, one, or an identity transform.

### 4. Embedded-map outputs

For `s > r`, a rectangular `J` has no ordinary determinant or two-sided
inverse. For full-column-rank maps, the geometry API must provide:

- the nonnegative measure scale
  `mu = sqrt(det(J^T J))`;
- a reference-to-tangent physical gradient transform
  `T = J (J^T J)^{-1}`, with shape `s x r`, so a reference gradient `g_ref`
  maps to the tangent physical gradient `T g_ref`;
- an explicit embedded mapping classification so callers never interpret
  `mu` as a signed determinant or `T` as an ordinary inverse transpose.

`T` is the minimum-norm/tangent transform; it does not define derivatives in
the normal complement of the embedded element. The API must state this
limitation. If a future physics kernel needs normal/tangential decomposition,
that is a separate operator requirement.

For rank-deficient or degenerate maps (`det(J^T J) == 0`, within the chosen
numerical policy), the operation must report failure or a typed degenerate
status. It must not attempt to invert the singular metric or present an
ordinary determinant/inverse. Tolerance policy and precision must be explicit
and tested rather than hidden in a generic fallback.

### 5. Output suitable for partition-owned `jacs_dets`

Provide a single-element operation that fills caller-owned output buffers (or
an equivalently type-safe result) from one element's geometry coordinates and
the selected geometry basis/integration points. It must:

- return/identify the mapping case and make outputs' meanings discoverable;
- define exact sizes, scalar types, layouts, and per-integration-point stride
  for each case;
- keep square and embedded payloads unambiguous. If a common record is used,
  it needs an explicit kind/tag and cannot make an embedded metric measure
  look like `det(J)`;
- report invalid dimensions, unsupported layouts, insufficient geometry
  nodes, and degenerate maps through explicit status/error behavior;
- write only the documented output span and leave broader mesh traversal to
  the caller.

The partition can then loop over its unstructured elements, gather the
coordinates using connectivity, call this operation, and append compact
records into its contiguous geometry storage. The inner solver kernels should
read those precomputed records contiguously and should not need to gather
coordinates or recompute geometry per degree of freedom.

## Geometry basis, solution basis, and reuse

The basis used to map physical geometry is not necessarily the basis used for
the solution field. Geometry precomputation must be keyed by the geometry
node ordering/basis, integration rule and point ordering, dimensions,
precision, and output layout—not implicitly by the solution polynomial order.
The API should allow a linear geometry map and a higher-order solution
element, as well as position-dependent higher-order geometry maps.

Reference gradients may be cached once per element type, geometry basis,
integration rule, precision, and layout. Physical Jacobians and their inverse
or embedded transforms/measures generally vary per unstructured element and
must be stored per element and integration point. Structured partitions may
reuse constant geometry data when proven valid, as `Tile` can support, without
changing the unstructured contract.

If physical coordinates change (for example, in updated-Lagrangian or
large-deformation analysis), the partition must know that its cached geometry
is stale and rebuild/update it. Initialization-time caching alone is not
sufficient for mutable geometry.

## Matrix-free and performance requirements

- Make a partition's precomputed geometry a contiguous, caller-owned array
  organized for the dominant kernel access. Document whether it is
  element-major or integration-point-major and benchmark that choice.
- Store only values needed by the selected kernel/physics path. For example,
  an embedded bar in 1D may need only one scalar measure per point, while an
  embedded bar in 3D needs a tangent transform as well as its measure if
  physical gradients are required.
- Permit geometry values that are constant across integration points or
  elements to be represented/reused compactly when correctness permits.
- Keep any common `jacs_dets` layout's type/metadata sufficient to distinguish
  signed determinant, unsigned measure, and transformation data. Avoid an
  opaque `+1` convention whose scalar meaning changes with dimensions.
- Benchmark precomputation cost, bytes per element/point, cache behavior, and
  end-to-end matrix-free kernel performance for representative orders,
  dimensions, and degrees of freedom per node. A precompute path is valuable
  when the solver's repeated reads amortize the initialization/storage cost.

## Validation requirements

Tests for the single-element operation and its partition-facing buffer
contract should cover:

- square 1D, 2D, and 3D maps, including signed orientation, absolute measure,
  inverse-transpose transform, and singular maps;
- embedded 1D-in-2D/3D bars and 2D-in-3D surfaces, checking rectangular
  Jacobians, `sqrt(det(J^T J))`, tangent transforms, rotations/translations,
  and degenerate/rank-deficient cases;
- exact sizes, strides, layouts, multiple elements and multiple integration
  points, and guard values around output buffers;
- nonconstant higher-order geometry where Jacobians vary by integration
  point, plus a case where geometry order differs from solution order;
- caller-side unstructured connectivity gathering and contiguous packing,
  without making `Elem` own or traverse the mesh;
- invalid `s < r` mappings and every documented error/status path.

Compare against hand-computed cases or independent analytic/finite-difference
oracles, not only agreement among storage variants. Preserve square solid
regressions and run representative performance comparisons before selecting a
final precomputed layout.
