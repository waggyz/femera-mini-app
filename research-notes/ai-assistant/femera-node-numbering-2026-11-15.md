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
- The current [node-count rule](../../src/femera/grid/Elem.ipp) adds one
  face node per face at P3 only when `vols_n` is nonzero. Thus the intended
  solid n-cube counts are Quad 4/8/12 and Cube 8/20/38 for P1/P2/P3.
  A 38-node cube is an enriched element, not the standard edge-only 32-node
  cubic serendipity cube. Preserve the 38-node contract unless a deliberate
  element-family change is made.

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
   Preliminary checks against the canonical coordinates identify the
   following work:
   - `Tets`: all four current face windings point inward. One outward set in
     the present face order is `(0,2,1)`, `(0,1,3)`, `(0,3,2)`,
     `(1,2,3)`.
   - `Prmd`: the current base `(0,1,2,3)` points inward; reverse it. The
     apex is `(0,0,1)`, not above the square center; test actual coordinates,
     not the drawing.
   - `Wdge`: the current bottom triangle `(0,1,2)` points inward; reverse it.
   - `Cube`: all six current quadrilateral windings point inward. Reverse
     each while retaining the selected face order, and check both triangles.
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
   run for 2D elements even though the count rule allocates no face node
   there. Write exactly one slot per eligible face, honor the count policy,
   and check buffer boundaries with sentinel/canary or sanitizer tests.
   Resolve the interface-element face policy and two-versus-three-coordinate
   storage explicitly rather than silently reading past `Ibar` arrays.
2. Check all P1-P3 node coordinates against independently constructed tables:
   vertices first, `(P-1)` positions for each directed edge, then one centroid
   per eligible triangular and quadrilateral face in the chosen order. Test
   nontrivial edges and all faces, not just `Bars`. Check that generated
   counts, sizes and coordinates agree and that the caller-provided geometry
   path uses the same convention.
3. Reconcile [Tets shape functions and gradients](../../src/femera/grid/elem/Tets.ipp),
   `tet10_coor`/`tet20_coor` and their transposes with the finalized edge and
   face order. In particular, current Tet10 functions put edges `(2,3)` and
   `(1,3)` in the opposite order from the active connectivity; Tet20 also
   contains reversed cubic edge directions. Apply the same permutation to
   values and every gradient component. The legacy `Tets::calc_poly_grad_node_*`
   actually stores reference-major blocks: preserve or deliberately migrate
   that API and its callers rather than inferring layout from its name.
4. Audit consumers of these orderings, including node-coordinate generation,
   tetrahedron tests, mesh import/export conventions, assembly and any stored
   connectivity. If external node orders differ, define explicit import/export
   permutations and test round trips. Do not silently reinterpret existing
   mesh data.

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
3. For Cube P3 explicitly implement the six face-center degrees of freedom
   implied by 38 nodes. A concrete construction to evaluate is: start with
   the 32-node cubic serendipity nodal basis `S_i`; on a face with varying
   axes `u,v` and fixed axis `w`, form
   `B_f = q_edge(u) * q_edge(v) * l_side(w)`, using the quadratic 1D
   midpoint function and linear 1D endpoint function. It is one at that face
   center and zero at every other selected node. Set each old basis to
   `S_i - sum_f S_i(center_f) * B_f`, and use the six `B_f` as face bases
   in `quad_vert_conn` order. Prove/test unisolvence, partition of unity and
   cubic polynomial reproduction for this enriched space before adopting it.
   Differentiate these corrections too.
4. Explicitly resolve trace compatibility: Cube38 has a 13-node enriched
   quadrilateral face trace (12 boundary nodes plus a center), whereas the
   planned standalone Quad12 has no face-center degree of freedom. Adjacent
   Cube38 faces must share the center and use the correct face permutation;
   a Cube38-to-Quad12 interface requires a defined constraint, an enriched
   Quad13 alternative, or another intentional policy. Do not claim identical
   trace spaces or drop the cube face nodes to hide this mismatch.
5. Compute gradients analytically with the product rule from the same 1D
   values and derivatives and the same correction terms. Define and test the
   exact layouts: node-major `g[node * dim + axis]` for
   `calc_poly_grad_node_*`, and reference-major `g[axis * node_n + node]`
   for `calc_poly_node_grad_*`, as documented by the new 1D API. Correct the
   copied bar comments and array extents in the 2D/3D declarations: values
   have `node_n` entries and gradients have `node_n * dim`. Keep existing
   pointer-return, `noexcept`, `static constexpr` and input-coordinate
   conventions, and ensure the two layouts are transposes.
6. Connect the implementations to the relevant element evaluation path,
   including the intended Bars integration and Quad/Cube dispatch for
   supported orders and integration-point strides. Inspect current callers
   before selecting an interface; do not confuse reference dimension with
   physical embedding dimension or silently change the legacy Tets layout.

### 4. Verification and acceptance

- Compile-time `static_assert` evaluations in C++14 for representative values
  and both gradients of each n-cube order, plus float/double and mixed
  precision instantiations. Test the returned output pointer and guard
  regions around arrays.
- Independently verify the full Kronecker-delta matrix at every reference
  node, including every cubic edge direction and all six Cube38 face
  centers. At multiple interior and boundary points, check partition of
  unity, zero gradient sums, and analytical gradients against centered finite
  differences in every reference direction with precision-appropriate
  tolerances. Verify both layouts element by element.
- Test reproduction of every monomial of total degree at most P and the
  corresponding derivatives, including affine coordinate reproduction and
  reference Jacobians. Check the actual declared polynomial space and
  enrichment separately; do not equate P3 completeness with a pure
  total-degree-three basis.
- On every edge compare restrictions with the matching 1D basis after
  orientation permutation, including zero traces for non-edge nodes. On
  every solid face check restrictions and tangential derivatives, and test
  adjacent cells under face rotations/reflections and reversed shared edges.
  For Cube38 test both the 13-node face trace and whichever Quad12 coupling
  policy is selected.
- Run the focused topology, node-coordinate, Tets and elem-poly tests, plus
  relevant mesh/assembly integration tests; use memory-safety instrumentation
  for node-buffer tests where available. Finally run the applicable wider
  test/build suite and record any pre-existing blockers separately.
