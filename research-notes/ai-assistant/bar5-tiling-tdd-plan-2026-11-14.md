# Bar5 tiling: test-first implementation plan

## Goal and starting point

Implement every `calc_*` method declared in
[`Bar5.hpp`](../../src/femera/grid/tile/Bar5.hpp), including the four tiling
variants, through the public `Tile` interface. Work in short red-green-refactor
cycles: write an independently checkable test, demonstrate the intended
failure, implement only enough to pass it, then refactor and rerun regressions.
Do not treat a compile/link failure from an undefined method as the final
behavioral test.

Initially [`Bar5.ipp`](../../src/femera/grid/tile/Bar5.ipp) was empty,
[`Bar5.gtst.cpp`](../../src/femera/grid/tile/Bar5.gtst.cpp) had only a trivial
test, and the header's `calc_tile_part_node_n` was the Bar1 formula rather
than a 2D truss node count. See implementation notes below for progress.
[`Bar1.ipp`](../../src/femera/grid/tile/Bar1.ipp) and
[`Bar1.gtst.cpp`](../../src/femera/grid/tile/Bar1.gtst.cpp) illustrate dispatch,
endpoint-first higher-order bar connectivity, coordinate layouts, and bounds
results, but do not implement variants. Do not copy their 1D indexing into
Bar5.

Before this work, [`Tile.hpp`](../../src/femera/grid/Tile.hpp) multiplied
partition dimensions according to `tile_d` but assumed every tile contributed
the same number of partition elements; its connectivity and coordinate
methods lacked the context required for Bar5 variants and 2D layouts. Its
per-element node count used tile vertices and edges, correct for one Bar1
element but not for one Bar5 bar.

## Contract to settle before coding

The owner confirmed the following nonperiodic choices and the periodic seam
policy. Encode them in tests and API documentation before implementation.

1. **Tile and element indexing.** Partition dimensions `(nx, ny)` count
   rectangular tiles, with x varying fastest. Within a tile the five local
   bars remain bottom, right, top, left, diagonal; the alternate diagonal is
   `1 -> 3`. Partition element indices enumerate *unique* bars, so `e / 5`
   is not a valid general mapping. Use a documented deterministic ordering:
   horizontal edges first, then vertical edges, then diagonals, each row-major
   with x fastest. Orient horizontal and vertical bars in increasing x and y,
   respectively; diagonals follow the selected tile's local directed table.
2. **Shared edges versus element instances.** Deduplicate perimeter bars
   inside each partition: return and count every geometric edge once. Each
   partition independently contains a complete mesh; any reconciliation
   across partition boundaries is handled at a higher level. For positive
   dimensions the element count is `H + R + D` as defined below.
3. **Variants are tile patterns, not just alternate connectivity tables.**
   Expose a named variant with canonical default: 0 = all `0 -> 2`,
   1 = all `1 -> 3`, 2 = checkerboard with `(x + y) % 2 == 0` canonical,
   3 = its complement. This defines alternation in two dimensions rather
   than relying on flattened-index parity, which differs on even-width grids.
   Rows start with alternating tiles, and parity uses an explicit global
   tile offset `(ox, oy)`: use `(x + ox + y + oy) % 2`. The offset defaults
   to zero. Test horizontal and vertical adjacency, row boundaries, and
   partition seams.
4. **Numbering and order.** Vertex connectivity uses row-major coarse-grid
   vertex IDs, x fastest. P2 node connectivity uses row-major IDs on the
   refined `(2*nx+1) x (2*ny+1)` grid, matching the FD grid; all points in
   that grid are used. Other orders use coarse vertices followed by interior
   nodes per unique horizontal, vertical, and diagonal edge, without unused
   grid points. Per-bar node connectivity is `[endpoint 0, endpoint 1,
   interior nodes from endpoint 0 to endpoint 1]`.
5. **Geometry.** Define the initial rectangular tile in the XY plane:
   `origin + (x * size_x, y * size_y, 0)`, with three output components and
   a translated z origin. Define how `tile_size[2]` is interpreted rather
   than copying Bar1's 3D direction-vector meaning; general tilted/affine
   planes require a separate explicit two-vector API if needed. Resolve null
   origin consistently with Bar1. Element-coordinate methods must agree with
   connectivity and the selected diagonal, in both vertex-major and
   coordinate-major layouts.
6. **Bounds and periodicity.** Specify supported polynomial orders (at least
   P1, P2, P3), pointer/buffer preconditions, zero dimensions, invalid indices
   and variants, arithmetic overflow, and whether outputs are unchanged on
   failure. Avoid division or modulo by zero. Bar1 often writes output before
   returning false, so do not silently impose a different contract on Bar1.
   Periodic vertex connectivity wraps both x and y. Odd periodic extents may
   have a non-alternating checkerboard seam; document and test this. Define
   the compact periodic node-ID space and its size, corner wrapping, high-order
   IDs, and treatment of one-tile dimensions. Current Tile
   exposes periodic **vertex** connectivity only; do not imply that
   nonperiodic node connectivity or coordinates are automatically periodic.
   If periodic node connectivity/counts are needed, add and test a coherent
   API; otherwise document the limitation.

For the recommended nonperiodic model, let
`V = (nx + 1)(ny + 1)`, `H = nx(ny + 1)`,
`R = (nx + 1)ny`, and `D = nx ny`. For positive dimensions the unique node
count is `V + (p - 1)(H + R + D)`: every unique edge has `p - 1`
interior nodes, including one diagonal per tile. Define empty-partition
semantics separately; do not extrapolate this formula to a zero dimension
without an explicit decision. Count arithmetic needs a checked representation
or a documented valid range because existing count APIs return a bare integer.

## TDD sequence

### 1. Establish baseline and executable tests

- Inspect the configured Makefile, `CPUMODEL`/build directory, GoogleTest
  setup, and test harness. Build and run the existing focused `Bar1`, `Tile`,
  and `Bar5` tests; record baseline failures separately. The repository has
  Makefile `.gtst`/`.gtst.out` targets, for example
  `make build/<configured-CPUMODEL>/femera/grid/tile/Bar5.gtst` followed by
  the resulting executable; verify the actual configuration rather than
  assuming a CPU name or CMake target.
- Replace the trivial Bar5 test with public-interface tests. First test
  metadata: element dimension 1, tile dimension 2, five bar instances, four
  tile vertices, five edges, two local alternate tables and their exact
  directed pairs. Include stateless/`constexpr` checks where applicable. These
  two alternates are distinct from the **four** partition variants.
- Keep each subsequent red test small. Where an unimplemented constexpr
  definition prevents linking, temporarily use the smallest test-visible
  skeleton needed to run and observe a behavioral failure before green.

### 2. Fix shared count and dispatch contracts

- First add failing `Tile<Bar5>` tests for a `2 x 3` partition: 23 unique
  bars; compare `2 x 1` (9), `1 x 2` (9), and empty-dimension behavior. Add
  Bar1 regression tests for ignored dimensions and existing counts.
- Delegate partition counting to the shape where necessary, while preserving
  the existing generic `tile_d` behavior for other shapes. Inventory
  other `Tile` specializations and callers before changing the generic path.
- Add P1/P2/P3 tests distinguishing per-bar node count (`p + 1`) from the
  number of unique nodes in a whole tile. Correct `get_tile_elem_node_n` for
  Bar5 via shape-specific dispatch while retaining the existing generic
  behavior for other shapes; document the distinction.
- Introduce a variant-aware interface and any required dimension/offset
  context through explicit overloads or a small typed options object. Keep
  existing Bar1 call signatures, default behavior, `constexpr`, and
  `noexcept` where supported. Add dispatch tests for the default and explicit
  variants, including a test shape or Bar1 regression, before using it in Bar5.
  Avoid forcing unsupported signatures on unrelated shapes.

### 3. P1 nonperiodic connectivity and pattern selection

- Write exact pair tests for all five unique bars of a `1 x 1` tile in
  variants 0 and 1. In edge-class order these are `(0,1), (2,3), (0,2),
  (1,3), (0,3)` for canonical and `(0,1), (2,3), (0,2), (1,3), (1,2)`
  for alternate. The perimeter orientations are global positive-axis
  orientations, not the top and left directions in the local tile table.
- For a `2 x 1` canonical partition, assert nine directed pairs in the
  agreed edge-class order: `(0,1), (1,2), (3,4), (4,5),`
  `(0,3), (1,4), (2,5), (0,4), (1,5)`.
  This catches edge order, duplicate shared edges, and flattening mistakes.
  Assert first invalid element and empty dimensions return the agreed failure.
- Test variants 2 and 3 on `2 x 2` and `3 x 2` partitions with explicit
  expected diagonal endpoints; check that each is the complement of the other,
  and that variants 0 and 1 remain uniform. Test nonzero tile offsets and two
  adjacent partitions if global parity is supported.
- Implement `calc_tile_vert_conn` and P1 `calc_tile_node_conn` using one
  shared, variant-aware tile/edge mapping. Test that P1 node connectivity
  equals vertex connectivity for every bar, not just the diagonals.

### 4. Unique higher-order nodes and connectivity

- First test counts and fixed node IDs for a single tile and a `2 x 1`
  partition. Under the proposed node-count contract, P1/P2/P3 counts are
  respectively `4/9/14` and `6/15/24`. Add a `2 x 2` case and all variants:
  changing the diagonal changes its locations, not the number of nodes.
- Before coding, write the precise edge-ID/offset scheme into the tests.
  Assert endpoint-first P2 and P3 node lists for horizontal, vertical, and
  both diagonal directions. Verify a shared edge is returned exactly once
  with one set of endpoint and interior node IDs. Check that
  every valid node ID is in range, every allocated node is used, and no
  distinct edges accidentally share interior IDs.
- Implement `calc_tile_part_node_n` and P2/P3
  `calc_tile_node_conn`, keeping the runtime order override and template
  order behavior consistent with the existing Tile interface. Add guards and
  boundary tests before enabling any additional orders.

### 5. Partition-node and vertex coordinates

- Add independently calculated coordinate tests with non-square tile sizes,
  nonzero `(x,y,z)` origin, default null origin, multiple rows, and first,
  last, and out-of-range IDs. Test all three components, both diagonal
  variants, and P1/P2/P3 interior edge interpolation.
- For each bar, check that its node coordinates equal linear interpolation
  between its own oriented endpoints at fractions `j/p`. A variant-dependent
  diagonal interior position means that
  `get_tile_part_node_coor` also needs the pattern and partition context.
- Add explicit dimension-aware vertex-coordinate overloads: `vert_n` alone
  cannot determine a 2D row stride. Implement `calc_tile_vert_coor` and
  `calc_tile_part_node_coor` against the agreed numbering. Retain the old
  Bar1 overloads; never infer a unique 2D shape from a total count.

### 6. Element-coordinate layouts

- For perimeter and both diagonal bars in multiple rows, assert exact six
  component values for `calc_tile_elem_vert_coor` (AoS: `x0,y0,z0,x1,y1,z1`)
  and `calc_tile_elem_coor_vert` (SoA: `x0,x1,y0,y1,z0,z1`).
  Cross-check each endpoint with its connectivity ID and independently
  calculated vertex coordinates. Test variants 0-3, boundaries, and offset.
- Extend the Tile coordinate dispatch with dimensions and variant/offset:
  `elem_n` alone cannot distinguish different partition shapes or determine
  the selected diagonal. Implement both Bar5 coordinate methods with a common
  mapping and layout-specific output; preserve Bar1's original overloads.

### 7. Periodic vertex connectivity

- Before implementation, pin the periodic contract from the decisions above
  with hand-computed cases: a `2 x 2` partition with separate x/y seam and
  corner crossings; both diagonal orientations; P1 and P2/P3 endpoint IDs;
  and zero/one-tile extents. Check valid-ID ranges against the explicitly
  defined periodic numbering, and invalid index/variant handling.
- Test variants 2/3 on even and odd periodic extents, including seam parity.
  Permit and document the non-alternating seam for odd extents; do not
  silently claim strict alternation.
- Implement `calc_tile_vert_conn_prdc` without zero modulo. If supporting
  periodic higher-order node connectivity is required, add matching count,
  coordinate/identification, and connectivity tests and API in the same
  cycle; otherwise state that only endpoints are periodic.

### 8. Refactor and integrated verification

- Share indexing, local connectivity, node-ID and orientation helpers between
  paths so variant choice cannot diverge. Retain independent expected-value
  tests rather than computing the oracle through those same helpers.
- Exercise all declared Bar5 methods through `Tile<Bar5, P, F>`, including
  default and explicit variants and representative `constexpr` calls. Add
  small-grid exhaustive checks: valid IDs, all expected bars, shared nodes,
  interpolation, translations/scaling, endpoint agreement and AoS/SoA
  equivalence. Use sentinel buffers to check the agreed failure and write
  bounds contracts.
- Rerun focused Bar5, Bar1 and Tile GoogleTests and the relevant configured
  build/test targets. Run other affected tile tests after shared API changes;
  report missing dependencies, pre-existing failures, and unsupported cases
  rather than treating a build alone as a passing test.
- Update the Bar5/Tile API comments with dimensions, numbering, direction,
  variant parity, periodic and error semantics, and edge deduplication.
  Keep truss mechanics, mixed-element assembly, and general 3D affine tiling
  out of scope unless the contract decision makes them necessary.

## Completion criteria

Every declared method is defined and exercised; P1-P3, all four agreed
patterns, coordinate layouts, shared-edge direction, explicit invalid cases,
and documented periodic behavior have independent passing oracles. Existing
Bar1 behavior remains covered. The count APIs and all variant-aware calls
agree on indexing and geometry, and any deliberate API or modeling changes
are documented.

## Implementation notes, 2026-11-14

- Updated the plan for the owner's decisions: unique edges within each
  complete partition, global-offset checkerboard parity, wrapping both axes,
  and permitting odd-extent non-alternating periodic seams. The existing
  `tile_d` fix is retained; unique-edge counts require shape-specific
  dispatch in addition to that fix.
- First red cycle: added metadata and P1-P3 partition-count tests. The
  existing implementation returned 10 instead of 9 for a `2 x 1` partition,
  30 instead of 23 for `2 x 3`, and incorrect node counts. Added optional
  shape-specific partition-count dispatch in `Tile` with the original
  generic calculation as fallback, and implemented Bar5's unique-edge and
  unique-node formulas. Empty dimensions return zero. Positive order and
  representable intermediate arithmetic are currently preconditions.
- Second red cycle: per-element node-count tests returned 4, 9 and 14 for
  P1, P2 and P3 instead of 2, 3 and 4. Added optional shape-specific
  element-node-count dispatch and Bar5's `p + 1` implementation; generic
  behavior remains the fallback.
- Both cycles included behavioral red runs before the implementation and a
  green run afterward. The final focused Bar5 executable reported 6/6 tests
  passing, including the harness's own test; the separately rebuilt Bar1
  executable reported 18/18 passing, and the separately rebuilt Tile executable
  reported 2/2 passing. Tests used the configured MPI/OpenMP harness and
  existing libraries. The test harness can report GoogleTest
  failures while the process still exits successfully, so inspect its
  `test gtst FAIL` output, not just the shell status.
- Correction: an experimental direct link command failed after adding a
  runtime connectivity-table test. This was not evidence that the project
  fails to build from a clean state; the owner reports that the clean project
  build works. The normal configured Makefile build was used in the next
  implementation cycle. The table test remains constexpr.

## Implementation notes, continuation

- Added `TilePattern` with variant and global tile offsets, and compatible
  connectivity overloads. Partition edge order is horizontal, vertical,
  diagonal; each block is x-fastest. Initially P1-P3 node connectivity used
  row-major vertices followed by edge-major interior IDs; see the P2
  numbering change below.
  Nonperiodic connectivity rejects empty partitions, zero order, invalid
  variants and out-of-range indices without modifying the output. Existing
  Bar1 entry points remain available.
- Wrote tests and ran the configured Makefile build with behavioral stubs
  first: canonical unique-edge, checkerboard-offset and higher-order tests
  failed as intended. Implemented connectivity and made them pass. Added
  `1 x 1`, odd-width, partition-offset, constexpr and exhaustive small-grid
  uniqueness/node-coverage checks.
- Wrote periodic endpoint tests first and observed failures, then implemented
  wrapping in both axes. Periodic IDs use row-major `nx * ny` vertices at
  every polynomial order; the complete partition edge list is retained.
  Odd checkerboard extents permit a non-alternating periodic seam. This
  endpoint API does not provide periodic interior-node IDs or periodic
  node-coordinate/count APIs.
- Wrote dimension-aware coordinate tests first and observed failures, then
  added coordinate overloads and implementations for vertex, partition node,
  and element AoS/SoA coordinates. Both variants and offsets reach diagonal
  node geometry. For Bar5 the older count-only vertex/element-coordinate
  signatures cannot identify a 2D row stride and are not implemented; use
  the overloads accepting partition dimensions. Bar1's old calls are
  unchanged. `tile_size[2]` is ignored, the origin supplies z, and null
  origin is zero. Added P1-P3/all-variant geometry consistency tests.
- Counts still require positive polynomial order and dimensions small enough
  to avoid overflow in `Local_int`, as documented in the header; the count
  APIs have no error channel. Nonperiodic and periodic connectivity and
  coordinates assume valid pointer/buffer sizes. Consider checked count APIs
  as a follow-up before accepting untrusted or very large dimensions.
- Final configured Makefile/MPI/OpenMP test runs reported Bar5 19/19,
  Bar1 18/18 and Tile 2/2 passing, including the harness's own tests. The
  focused executables were rebuilt against the changed headers; the complete
  project-wide suite was not run.

## P2 FD numbering update

- The P2 coordinate set already matched the refined FD grid, but its
  vertices-first numbering did not. The user's FD comparison and new
  hand-calculated connectivity tests failed before the change.
- P2 node coordinates and node connectivity now use refined-grid row-major
  IDs. Coarse vertex connectivity, including periodic vertex connectivity,
  remains unchanged; P1 and P3 node numbering is unchanged. P2 diagonal
  midpoints have the same ID in both orientations. Runtime order overrides
  follow the requested order's numbering.
- Added exact connectivity cases, rectangular-grid and variant/offset
  coverage, endpoint/midpoint checks, and constexpr and periodic regressions.
  Configured test runs reported Bar5 22/22 and Bar1 18/18 passing.
