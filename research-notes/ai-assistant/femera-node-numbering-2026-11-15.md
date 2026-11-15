# Femera finite element node numbering conventions

- Each element contains a reference first-order element defining the vertices: `vert_coor` and its transpose `coor_vert`.
- Each element contains edge vertex connectivity in `bars_vert_conn`.
- 2D and 3D elements contain face vertext connectivity in `tris_vert_conn` and `quad_vert_conn`. (These are `nullptr` when there are no faces of the corresponding shape.)
- All 3D element faces define their local node winding in a counter-clockwise (CCW) order when viewed from outside the element for outward-pointing face normals.

**Node numbering**

- First, vertex nodes.
- Then, if present, one or more edge nodes, in order of the edges defined in `bars_vert_conn`.
- Then, if present, face nodes, using `tris_vert_conn` followed by `quad_vert_conn`.

---

Let's reconsider node numbering.

Consider each element's `vert_coor` as canonical and correct. Do *not* consider the current implementation's `bars_vert_conn`, `tris_vert_conn`, or `quad_vert_conn` as reference. These may be incorrect, as well as inconvenient or non-optimal. Only `Tets` has shape functions and gradients defined, and these are subject to change (reordering) as a result of this analysis. The `Bars` element will use the shape functions and gradients in #src/fmr/fems/elem-poly.hpp, and I think these are correct for these 1D elements. (2D and 3D n-cube functions are declared but not yet defined.)

Please think through the implications for composing higher-dimension shape functions from lower-dimension functions and make a plan to:

- Check and correct `bars_vert_conn`, `tris_vert_conn`, and `quad_vert_conn` for each element. Use `tri3_norm` in #src/fmr/math.hpp to check reference element face normals.
- Compose n-cube 2D and 3D shape functions and gradients from the implemented 1-cube (1D) functions.

Save this plan to a file in the #research-notes/ai-assistant/ folder.

---

## Analysis and implementation plan

### Decisions and invariants

- Treat each element's `vert_coor` as authoritative, including its vertex
  order. Check that `coor_vert` is its exact transpose; do not repair an
  inconvenient topology by moving or renumbering vertices.
- Number nodes as vertices, then edge nodes grouped by the directed edges in
  `bars_vert_conn`, then face nodes grouped by `tris_vert_conn` and
  `quad_vert_conn`. On a directed edge `(a,b)`, the cubic nodes are at
  `(2a+b)/3` and `(a+2b)/3`, in that order. Reversing an edge therefore swaps
  its two cubic nodes even though its quadratic midpoint is unchanged.
- Adopt one explicit order for edges and faces per element and document it;
  connectivity winding and node order are related but distinct. A cyclic
  rotation of a face preserves its normal, while reversal changes it. Shared
  edges and faces need orientation/permutation maps when two elements use
  different local directions, rather than assuming equal local indices.
- Target the standard serendipity/simplex families rather than preserving
  prototype behavior. The revised
  [node-count rule](../../src/femera/grid/Elem.ipp) agrees with the current
  [P1/P2/P3 test expectations](../../src/femera/grid/Elem.gtst.cpp):

  | Element | P1 | P2 | P3 |
  | --- | ---: | ---: | ---: |
  | Bars | 2 | 3 | 4 |
  | Tris | 3 | 6 | 10 |
  | Quad | 4 | 8 | 12 |
  | Tets | 4 | 10 | 20 |
  | Cube | 8 | 20 | 32 |
  | Prmd | 5 | 13 | 21 |
  | Wdge | 6 | 15 | 26 |

  These expectations are requirements under investigation, not proof that
  all of the resulting elements are standard or mutually compatible. The
  selected complete cubic triangle has three vertices, six edge nodes and
  one interior node, matching the full Tri10 traces of Tet20 and Wdge26.
  The 21-node
  pyramid also has nine boundary nodes on each triangular face, so check its
  intended   cubic space and compatibility with full Tri10 faces before
  certifying it. Counts alone do not define a finite-element space.
  Triangular cubic interior counts are now explicit per-topology metadata;
  higher polynomial orders remain outside the P1-P3 plan.
- This is a prototype: revise internal APIs, layouts, node counts and tests
  where required by the chosen mathematical requirements. No backward
  compatibility or migration of prototype data is required. Pymera is
  currently stubbed; preserve a clear path to its eventual user-facing
  element metadata without treating its current stubs as a fixed API.

### 1. Audit and establish reference topology

1. Inventory `Bars`, `Tris`, `Quad`, `Tets`, `Prmd`, `Wdge`, `Cube`, and
   interface `Ibar`, `Itri`, and `Iqud` in
   [elem/](../../src/femera/grid/elem/). Record the canonical coordinates,
   topological dimension, coordinate stride, vertex/edge/face counts, and
   expected node counts for P1-P3. Derive edges and faces from the intended
   element geometry, not from the existing connectivity arrays. Check index
   bounds, distinct vertices per nondegenerate edge/face, no duplicate
   undirected edges or faces, and that every face boundary consists of
   declared edges. For ordinary closed 3D solids also check two incident faces
   per edge, opposite directed traversal of each shared face edge, and
   `V - E + F = 2`.
2. Preserve useful existing edge order where correct, but publish explicit
   chosen edge directions and face orders alongside reference-node tables.
   For each nondegenerate 3D solid, use
   [`tri3_norm`](../../src/fmr/math.ipp) on each oriented triangle and both
   triangles `(0,1,2)` and `(0,2,3)` of each quadrilateral. Assert success,
   consistent quadrilateral normals, and a strictly positive dot product of
   each normal with `(face centroid - interior point)`. Use a known interior
   point such as the vertex centroid for these convex reference solids;
   never treat a failed normal calculation as a passing orientation test.
   For 2D solids, check the documented positive reference-plane winding and
   boundary coverage instead of an outward-volume normal.
3. Use the existing arrays only as candidates to compare, not as evidence.
   The initial checks against canonical coordinates identified the
   following corrections, now implemented:
   - `Tets`: all four former face windings pointed inward. The outward set in
     the present face order is `(0,2,1)`, `(0,1,3)`, `(0,3,2)`,
     `(1,2,3)`.
   - `Prmd`: the former base `(0,1,2,3)` pointed inward and was reversed. The
     apex is `(0,0,1)`, not above the square center; test actual coordinates,
     not the drawing.
   - `Wdge`: the former bottom triangle `(0,1,2)` pointed inward and was reversed.
   - `Cube`: all six former quadrilateral windings pointed inward and were
     reversed while retaining the selected face order.
   - `Bars`, `Tris`, and `Quad`: verify their edge/face sets and directed
     ordering under the same rules rather than assuming they require changes.
4. Handle cohesive/interface elements separately. `Itri` and `Iqud` have
   coincident but distinct vertex layers: their two face normals can be
   checked for nondegeneracy and opposite directions, but a centroid-to-face
   outward test is meaningless at zero thickness. Record which layer is
   positive and which is negative and test corresponding edge-node
   permutations. `Ibar` has two-coordinate vertex storage and coincident
   layers; its quadrilateral is geometrically degenerate, so
   `tri3_norm` cannot establish a normal. Check its two directed bars and
   the intended *topological* quad cycle and pairing instead. Do not invent
   a volume or reject a valid zero-thickness interface. Clarify its face-node
   semantics and coordinate stride before applying generic three-coordinate
   tests.
5. Add topology tests for all elements, including absent-face null pointers
   and declared lengths. Correct connectivity and update orientation tests
   that currently encode inward normals. Check the `vert_coor`/`coor_vert`
   access conventions for each class before using the generic `Elem` API:
   `Bars` uses templated, three-component arrays; several other element
   classes expose non-templated arrays, and `Ibar` stores two components.

### 2. Make node coordinates and consumers follow that topology

1. Repair and test [`get_elem_node_coor`](../../src/femera/grid/Elem.ipp)
   before using it as a node-order oracle. Its edge writes currently omit the
   edge index, so later edges overwrite earlier ones. Its P3 face loops repeat
   the same centroid into multiple slots and omit the face index; they also
   run for faces for which the current count rule allocates no nodes.
   Write exactly one slot per eligible face, honor the chosen count policy,
   and check buffer boundaries with sentinel/canary or sanitizer tests.
   Resolve the interface-element face policy and two-versus-three-coordinate
   storage explicitly rather than silently reading past `Ibar` arrays.
2. Check all P1-P3 node coordinates against independently constructed tables:
   vertices first, `(P-1)` positions for each directed edge, then one centroid
   per eligible triangular face in the chosen order. A Tri10 centroid is an
   element-interior node even though the triangle also appears as a face in
   the topology metadata. The targeted cubic Cube32 and Quad12 have no face
   centers. Test
   nontrivial edges and all faces, not just `Bars`. Check that generated
   counts, sizes and coordinates agree and that the caller-provided geometry
   path uses the same convention.
3. [Tets shape functions and gradients](../../src/femera/grid/elem/Tets.ipp),
   `tet10_coor`/`tet20_coor` and their transposes now agree with the
   finalized edge and face order. The chosen tetrahedral edge sequence is
   `(0,1)`, `(1,2)`, `(2,0)`, `(3,0)`, `(3,2)`, `(3,1)`, matching the
   existing basis. The legacy `Tets::calc_poly_grad_node_*` actually stores
   reference-major blocks: retain alternative layouts for later HPC
   performance evaluation.
4. Update consumers of these orderings, including node-coordinate generation,
   tetrahedron tests and assembly. For actual supported external mesh
   formats, specify conversions to and from the chosen internal numbering
   and test round trips; do not add migration machinery for prototype data.

### 3. Compose constexpr n-cube values and gradients

1. Retain the `[0,1]` reference interval and existing 1D endpoint-first
   functions in [elem-poly.ipp](../../src/fmr/fems/elem-poly.ipp). Define
   helpers that evaluate 1D values and derivatives independently for each
   axis, respecting mixed `Y`/`G` precision and C++14 `constexpr`
   restrictions. Derive axis/side and orientation maps from the chosen
   Quad/Cube reference topology; a negative-axis edge must reverse the two
   1D cubic edge basis indices. Do not rely on lexicographic tensor-product
   indexing coinciding with element vertex or edge order.
2. For P1 use products of linear endpoint functions. For P2 and P3 build
   vertex and edge functions in the appropriate serendipity space from the
   1D linear/quadratic/cubic functions, with the necessary vertex/edge
   corrections; a raw full tensor product has 9/27 or 16/64 nodes and is
   not the requested element. Derive and verify the interpolation formulas,
   including their edge restrictions, rather than merely deleting interior
   tensor-product functions.
3. Implement standard Cube32 for P3: eight vertices and two nodes on each
   of twelve directed edges, with no face-center nodes. Rename the undefined
   `C3D38N` declarations to `C3D32N` and correct their extents. Verify
   unisolvence and polynomial reproduction for the selected standard
   serendipity space; do not add the previously proposed Cube38 enrichment.
4. Require the restrictions of Cube8/20/32 to quadrilateral faces to agree
   with Quad4/8/12, after the appropriate face and edge permutations, for
   both values and tangential derivatives. Verify triangular-face compatibility
   for Tets, Wdge and Prmd.
5. Compute gradients analytically with the product rule from the same 1D
   values and derivatives and the same correction terms. Define and test the
   exact layouts: node-major `g[node * dim + axis]` for
   `calc_poly_grad_node_*`, and reference-major `g[axis * node_n + node]`
   for `calc_poly_node_grad_*`, as documented by the new 1D API. Correct the
   copied bar comments and array extents in the 2D/3D declarations: values
   have `node_n` entries and gradients have `node_n * dim`. Keep existing
   pointer-return, `noexcept`, `static constexpr` and input-coordinate
   conventions where useful, and ensure the two layouts are transposes.
6. Connect the implementations to the relevant element evaluation path,
   including the intended Bars integration and Quad/Cube dispatch for
   supported orders and integration-point strides. Inspect current callers
   before selecting an interface; do not confuse reference dimension with
   physical embedding dimension. Preserve layout alternatives, including
   the Tets reference-major layout, pending HPC performance testing.

### 4. Verification and acceptance

- Compile-time `static_assert` evaluations in C++14 for representative values
  and both gradients of each n-cube order, plus float/double and mixed
  precision instantiations. Test the returned output pointer and guard
  regions around arrays.
- Independently verify the full Kronecker-delta matrix at every reference
  node, including every cubic edge direction. At multiple interior and
  boundary points, check partition of
  unity, zero gradient sums, and analytical gradients against centered finite
  differences in every reference direction with precision-appropriate
  tolerances. Verify both layouts element by element.
- For the selected standard n-cube and complete simplex spaces, test
  reproduction of every monomial of total degree at most P and the
  corresponding derivatives, including affine coordinate reproduction and
  reference Jacobians. Check the full declared polynomial space as well;
  serendipity also includes suitable higher-total-degree terms. If an
  incomplete simplex or pyramid is intentionally retained, document and
  test its actual reproduction space instead of asserting full P3
  completeness.
- On every edge compare restrictions with the matching 1D basis after
  orientation permutation, including zero traces for non-edge nodes. On
  every solid face check restrictions and tangential derivatives, and test
  adjacent cells under face rotations/reflections and reversed shared edges.
  Verify Cube32/Quad12 trace equality and the selected triangular trace
  policy for Tets, Wdge and Prmd.
- Run the focused topology, node-coordinate, Tets and elem-poly tests, plus
  relevant mesh/assembly integration tests; use memory-safety instrumentation
  for node-buffer tests where available. Finally run the applicable wider
  test/build suite and record any pre-existing blockers separately.

---

### Implementation progress

- Implemented the complete cubic triangle values and both reference-gradient
  layouts. The current tests cover nodal interpolation, cubic reproduction,
  finite differences, constant face loads, and the Tet20 reference bottom-face
  trace. The triangle routines are lower-level evaluators; general element
  dispatch and load-assembly integration remain future work.
- Added an explicit incomplete Tri9 evaluator with both gradient layouts. It
  constrains the Tri10 interior value to `-sum(vertices)/6 + sum(edges)/4`;
  this reproduces P2 and cubic edge traces, but not all cubic polynomials.
  Its tests cover interpolation, derivatives, edge orientations, vertex
  permutations, constant-load integration and equivalence to the constrained
  Tri10 basis. Use is intended for pyramid faces, but the eventual pyramid
  volume basis must be shown to restrict to this exact space.
- Updated the cubic triangle and triangular-interface node counts and fixed
  edge and triangular-interior coordinate writes. Tests cover generated cubic
  nodes and buffer bounds for the solid topologies and triangular interface,
  plus the two-component reference-coordinate stride of `Ibar`.
- Corrected all 3D solid face windings using the reference coordinates. Tests
  check outward normals, face-edge incidence, directed traversal, edge
  uniqueness and coordinate transposes. The tetrahedron's directed edges
  now agree with its existing Tet10/Tet20 coordinates and basis; tests check
  nodal interpolation and all four Tri10 face restrictions, including
  orientation-dependent edge permutations.
- Implemented `static constexpr` Quad4/8/12 and Cube8/20/32 values and both
  reference-gradient layouts by composing the 1D basis and derivatives with
  vertex corrections. Tests cover C++14 constant evaluation, mixed precision,
  all reference nodes, gradients, the full serendipity polynomial space,
  directed quadrilateral edges and all six oriented cube-face traces.
- Added a shared barycentric Lagrange construction for complete Tri3/6/10
  and Tet4/10/20 values and both reference-gradient layouts. Tri10 now uses
  this construction; Tri9 retains its constraint on Tri10. Tests compare
  Tri10 and Tet4/10/20 with their previous formulas, and check nodes,
  complete polynomial reproduction, derivatives, directed triangle edges,
  and all tetrahedral face traces for each order. The existing specialized
  tetrahedron evaluators remain available for comparison and benchmarking.
- Preserve both existing gradient layouts and other storage choices until
  comprehensive performance testing on HPC hardware; do not remove or select
  layouts merely to make prototype APIs uniform.
- Still to implement: reusable face-orientation/permutation handling and the
  general element evaluation/dispatch path, including Bars and the new n-cube
  evaluators. Wedge and pyramid volume bases and verification of their
  triangular face restrictions remain to be implemented; in particular, the
  pyramid must be checked against the selected Tri9 space. No wedge-pyramid
  constraint or assembly path has been implemented yet.

---

I think I corrected the `get_node_n` calculation in `Elem`. The goal here is to implement the standard serendipity/simplex family of elements. Expectations in #Elem.gtst.cpp document my current understanding of the correct counts. This code is in very early development; we're determining the requirements while prototyping the implementation, and there is no such thing as backward compatibility at this point. I talk with intended users often and have a good understanding of what they will need exposed in the Python API (Pymera), which is only stubbed out at present until the internals are working well enough to demonstrate. This plan looks good so far, with these adjustments.

---

Implement the 10-node cubic triangle (and fix `Elem::get_node_n`). The userbase will be doing mostly 3D solid modelling, so it is more immediately important to be able to integrate over triangle faces (to apply loads) than to solve cubic triangle 2D models (which can be deferred).

And, one final note before implementation. This is prototype high-performance computing (HPC) code, which is why you see a variety of storage/layout options (including transposed options). Decisions on which layouts to retain/use will be deferred until after comprehensive performance testing has been completed on HPC hardware. Now, please begin implementing this plan.

---

On reviewing node placement, I see that both 9- and 10-node triangles will be needed. The `Wdge` (prism) element has 10-node triangular faces, while the `Prmd` element has 9-node triangular faces. Please verify this and recommend a solution that will support both triangle variants.

---
