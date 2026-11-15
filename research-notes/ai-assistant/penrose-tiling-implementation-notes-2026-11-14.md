# Penrose tiling implementation notes

## Aim and working choice

Explore a Penrose mesh in three useful representations:

1. A graph made only of 1D `Bars`.
2. A mesh of 2D `Quad` elements.
3. The same quadrilateral mesh subdivided into two `Tris` per quad.

Use the **P2 rhombus tiling** as the initial target. It consists of thick
rhombs with angles 72/108 degrees and thin rhombs with angles 36/144 degrees.
Both are quadrilaterals, so one geometric patch can be represented first by
its edges, then by `Quad`, and finally by `Tris`. A substitution or matching
rule must distinguish thick from thin rhombs and preserve their edge-matching
decorations; geometry alone does not enforce a Penrose tiling.

This note is a design sketch, not an implementation of a Penrose substitution
table. Before coding a generator, select and record a verified rhomb
substitution rule, including its orientation/reflection conventions and edge
labels. Do not reconstruct those rules from memory.

## What the repository already provides

- [`Bars.hpp`](../../src/femera/grid/elem/Bars.hpp) is a 1D finite element
  shape (`elem_d == 1`). It can represent each edge of a Penrose patch as an
  element, but is not itself a tiling rule.
- [`Bar5.hpp`](../../src/femera/grid/tile/Bar5.hpp) and
  [`Bar5.ipp`](../../src/femera/grid/tile/Bar5.ipp) define a 2D structured
  tile whose elements are 1D bars: four square-boundary edges and one
  diagonal, with canonical/alternate/checkerboard diagonal patterns. Its
  connectivity, coordinate, partition, and higher-order tests in
  [`Bar5.gtst.cpp`](../../src/femera/grid/tile/Bar5.gtst.cpp) are useful
  examples of contracts to test. A square with a selectable diagonal is not a
  Penrose rhombus or a Penrose patch generator.
- [`Quad.hpp`](../../src/femera/grid/elem/Quad.hpp) describes a 2D, four-vertex
  element with one quadrilateral connectivity record. Its reference
  coordinates are a unit square; physical mesh connectivity can refer to
  arbitrary patch vertices.
- [`Tris.hpp`](../../src/femera/grid/elem/Tris.hpp) describes a 2D, three-
  vertex element with one triangle connectivity record.
- [`Tile.hpp`](../../src/femera/grid/Tile.hpp) is oriented around a tile
  shape's static metadata and structured partition dimensions, plus
  shape-provided connectivity/coordinate methods. The current source has
  `Bar1` and `Bar5` tile implementations, but no corresponding Penrose,
  quadrilateral-tile, or triangle-tile generator. A Penrose patch is
  unstructured and non-periodic; forcing its generation into `(nx, ny)`
  structured-grid indexing would conflate two different concerns.

## Recommended separation of responsibilities

Keep Penrose generation independent of finite-element shape evaluation:

- **Patch generator:** creates tile records (type, orientation, matching-edge
  labels) and stable vertex identities.
- **Mesh conversion:** emits a shared vertex-coordinate table and element
  connectivity for the requested representation (bars, quads, or triangles).
- **FE shapes:** use `Bars`, `Quad`, and `Tris` for element metadata, basis
  functions, and downstream element operations. Avoid placing substitution
  logic in those element classes.

An initial implementation can expose a small Penrose patch value type and
plain arrays/vectors for vertices and connectivity. Consider a `Tile<T>`
specialization only if the project later establishes a generic interface for
unstructured patch generators; the existing `Tile<T>` contract is not a
natural fit for an inflation-generated patch.

## Generation and geometry

Represent a tile as a type (thick or thin), an orientation/reflection, and an
ordered cycle of four vertex IDs. Store matching labels on edges or derive
them from the chosen substitution data. Each substitution record should
identify child types, child orientations, and vertex/edge relationships in a
parent-local coordinate frame. Applying it should:

1. Select the child records for the parent's type and orientation.
2. Transform the child geometry consistently (inflation scale, rotation, and
   any reflection).
3. Reuse shared vertices and edges rather than making independent copies for
   adjacent child tiles.
4. Check matching labels on every newly internal edge and retain valid labels
   on the patch boundary.

Generate a finite patch from a documented seed or seed patch and a requested
inflation depth. Keep the patch boundary explicit: a finite patch cut from an
infinite tiling need not close into a periodic domain.

Use the golden ratio `phi = (1 + sqrt(5)) / 2` in the selected inflation
construction and in any length relationships required by that rule. Keep
topological identity separate from floating-point coordinates. Prefer stable
symbolic vertex keys carried through subdivision. If coordinate-based
welding is needed, use a documented scale-aware tolerance and verify the
resulting edge lengths and incidence; raw floating-point equality is
insufficient, and an overly large tolerance can incorrectly merge distinct
vertices.

## Stage 1: edge-only `Bars` representation

Build the first converter from the generated patch to a Penrose edge graph:

- Emit the four boundary sides of each rhombus as undirected candidate edges.
- Deduplicate by the canonical pair of stable endpoint IDs, not by
  coordinates. Shared rhombus sides must become one mesh edge.
- Keep any per-edge matching decoration as metadata on the unique edge. It is
  important to the Penrose rule even though `Bars` connectivity only needs the
  endpoints.
- Emit one `Bars` element per unique edge, with the two vertex IDs in a
  documented endpoint order. `Bar5`'s deterministic unique-edge ordering is a
  useful precedent, but Penrose edges should not be enumerated as a regular
  horizontal/vertical/diagonal partition.
- Compute each vertex coordinate once and use that same table for every edge.

This phase tests vertex sharing, unique-edge extraction, edge lengths, and the
matching-rule boundary before adding 2D element assembly. A bar-only mesh is
the wireframe/skeleton; it does not yet encode each rhombus as a filled
finite-element cell.

## Stage 2: `Quad` representation

Convert each rhombus record to one quadrilateral connectivity record:

- Retain the same shared vertex table and emit the four IDs in a consistent
  cyclic order (recommended: counter-clockwise in the XY plane).
- Use the same `Quad` shape for both rhombus types; their physical
  coordinates, not two different element classes, distinguish their geometry.
- Check that each quad has four distinct vertices, positive signed area under
  the chosen orientation, and four sides of the expected Penrose edge length.
- For adjacent quads, confirm common sides use the same endpoint IDs in
  opposite traversal directions.
- Keep the Penrose type/orientation/matching labels alongside the element
  record if later refinement or validation needs them; ordinary `Quad`
  connectivity alone does not retain matching-rule state.

At this point the patch topology is a 2D finite-element mesh. Its lack of a
regular Cartesian partition is expected and should not be "fixed" by adding
dummy structured-grid cells.

## Stage 3: two `Tris` per `Quad`

Triangulate each rhombus without changing its four original vertices:

- Pick one diagonal by a deterministic, documented rule based on the
  quadrilateral's local vertex order or tile metadata. Use the same rule for
  all instances of a tile type/orientation.
- Emit two triangles using that diagonal. For a CCW quad cycle `(v0,v1,v2,v3)`,
  the `v0-v2` choice yields `(v0,v1,v2)` and `(v0,v2,v3)`; the `v1-v3` choice
  yields `(v0,v1,v3)` and `(v1,v2,v3)`. Verify both children retain the
  desired orientation.
- Reuse the quad vertices; the diagonal is an internal edge, not a new vertex.
  The resulting mesh has twice as many area elements as quads, with the same
  external boundary.
- Check that every source quad is covered exactly once by its two triangles,
  their areas sum to the quad area, and no triangle is degenerate.

Do not silently treat the chosen diagonal as a Penrose matching edge: it is a
triangulation detail and should be tagged separately if the mesh API stores
edge classifications.

## Shared mesh data and counts

For a patch with `V` unique vertices, `Q` rhombi, and `E` unique boundary
edges:

- Bars mesh: `V` vertices and `E` bar elements.
- Quad mesh: `V` vertices and `Q` quad elements.
- Tri mesh: `V` vertices and `2Q` triangle elements, with up to one additional
  unique mesh edge per quad for the internal diagonals (less if a later
  representation merges or subdivides topology).

The coordinate table is common to all three forms. Edge and face arrays are
conversions of a common patch topology, not separate geometry-generation
paths. This helps guarantee the bars trace the quad boundaries and the
triangles exactly cover the quads.

## Focused validation plan

1. **Rule data:** unit-test every thick/thin orientation/reflection entry;
   verify child types/orientations, matching labels, edge lengths, and the
   expected inflation/area relationship.
2. **Patch topology:** for a seed and at least one nontrivial inflation depth,
   verify all edge matches, no unmatched interior edges, valid boundary edges,
   no duplicate faces, and no unintended coordinate welds.
3. **Bars conversion:** every rhombus contributes four candidate sides;
   shared sides appear once in the unique bar table; every bar has distinct
   endpoints and the expected length.
4. **Quad conversion:** each rhombus produces one CCW quad referencing four
   existing vertices; neighboring quads share the same side IDs.
5. **Tri conversion:** each quad produces exactly two CCW triangles; summed
   triangle area matches the source quad; no additional vertices are created.
6. **Cross-representation consistency:** compare edge sets from bars against
   quad boundaries, and compare the union of triangle boundaries/internal
   diagonals against each source quad's selected triangulation.
7. **Regression and integration:** build the smallest relevant mesh/element
   tests for `Bars`, `Quad`, and `Tris`. Keep the existing `Bar5` tests as
   regressions; they test structured bar tiling and should remain independent
   of Penrose-specific assertions.

## Scope decisions to settle before implementation

- Which exact Penrose rhomb substitution/matching convention is the source of
  truth, and how are orientations/reflections numbered?
- What seed and finite-patch boundary are required (single seed, star, or
  another initial patch)?
- Are matching labels preserved as mesh metadata, or is generating a valid
  patch the only requirement?
- Which diagonal convention should the triangle export use?
- Does the first deliverable need only P1 connectivity, or also higher-order
  `Quad`/`Tris` node placement and interpolation?
- Should bars be exported as a distinct mesh or exposed as the edge view of
  the same quad/triangle mesh?

## Out of scope for the first pass

- Periodic Penrose meshes; ordinary finite Penrose patches are not periodic.
- Replacing or broadening `Tile<T>` solely to accommodate one unstructured
  generator.
- Editing `Bar5` to implement Penrose rules.
- Higher-order interpolation, refinement of already assembled FE meshes, or
  solver-specific boundary conditions before P1 topology is validated.
