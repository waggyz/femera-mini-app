# Free mesh-access design summary

Date: 2026-11-26  
Status: Bars-only prototype; focused tests and translation-unit checks pass.

## Current implementation

[Free.hpp](../../src/femera/grid/Free.hpp) and
[Free.ipp](../../src/femera/grid/Free.ipp) implement
`femera::grid::free::Free<elem::Bars, P, F>` for P1-P3. Only Bars is supported
at this stage; other simplex elements will follow their Elem implementations.

The class exposes an `Element` alias to `Elem<Bars, P, F>`, fixed reference
dimension/order and node/vertex counts, dimension-aware ordinary geometric
record sizing, and the two stateless caller-buffer copies shown below.
`elem_d` belongs to the element type; physical embedding dimension is separate.
No storage ownership, geometry calculation, Tile changes, or performance-kernel
integration is introduced.

Both focused copy tests passed after the user reauthorized validation.
C++14 syntax checks of the cleaned translation units also passed without
diagnostics. Broader geometry and performance testing is deferred; no
backward compatibility or certification requirement is implied by the older
planning sections.

## Preferred design: Free<Bars>, using Elem directly

Prefer `Free<T, P, F>` parameterized by an underlying element type such as
`elem::Bars`, `elem::Tris`, or `elem::Tets`. Forward metadata selectively through
`Elem<T, P, F>`, not through `Tile<Bar1>`.

This directly matches the clarified partition invariants:

- One element type and a uniform quadrature-point count per partition.
- Natural edge/face nodes make geometry vertex-defined.
- Bars, Tris, and Tets have one constant geometric record per element,
  independent of solution order and quadrature-point count.

Keep `P` and the packed connectivity layout fixed for each compiled kernel
invocation. Homogeneous topology/quadrature does not alone enforce uniform order.

| Concern | `Free<Bar1, P>` | `Free<Bars, P>` |
| --- | --- | --- |
| Dependency | Structured tile descriptor | Reference element descriptor |
| Metadata | `Tile` facade | `Elem` facade |
| Basis and vertex ordering | Requires mapping to Bars | Same element descriptor already provides them |
| Geometry width | Two-scalar Bar1 convention | Explicit physical dimension/layout |
| Arbitrary stored bar meshes | Supported, but tiling dependency incidental | Natural fit |
| Bar1 versus Bar5 mesh sources | Different tile metadata | Same underlying Bars element |
| Ownership and inline-copy capability | Caller-owned, stateless | Unchanged |

Proposed hot-path calls:

```cpp
Free<Bars, P>::get_elem_conn(conn, master_conn, elem_i);
Free<Bars, P>::get_elem_jacd(jval, master_jacd, jacd_size, record_elem_i);
```

Connectivity width comes from `Elem<Bars, P>::get_node_n()`. Keep geometric
width explicit, or obtain the ordinary packed width once from
`Elem<Bars, P>::get_jacs_dets_size(1, sims_d)`. Do not accidentally use its
three-dimensional default for a one-dimensional physical partition.

Fetch the simplex record once outside the quadrature loop. The master offset
is `elem_i * jacd_size`, not `elem_i * quad_intp_n * jacd_size`. Retain full
higher-order connectivity and point-specific basis gradients/weights:
constant geometry does not imply one quadrature point or only vertex DOFs.

Vertex-defined geometry is not generally affine for non-simplex elements.
Distorted Quad/Cube geometry still needs the point-dependent record path.
Physical embedding and inverse/pseudoinverse layout remain explicit producer/
consumer contracts; the copy architecture does not implement missing geometry
calculations.

**Current recommendation:** adopt element-based `Free<T, P, F>` for the stored
mesh policy. Keep `Tile<Bar1>` or `Tile<Bar5>` only for actual structured mesh
generation/halo work. No guaranteed speed advantage over equally specialized
copies is claimed; the benefit is the correct architectural dependency and
explicit partition/geometry invariants.

The alternatives below are retained as historical comparisons, not the current
preferred dependency.

## Baseline API: Tile<Free>

`Tile<Free>` is a stateless data-access policy with two header-defined static
copies, using caller-owned source and destination buffers:

```cpp
get_tile_elem_conn(conn, part_node_conn, conn_n, elem_i);
get_tile_elem_jacd(jval, this_jacd, jacd_size, elem_i, elem_intp_n, ip);
```

Layouts:

- Connectivity: `[element][local node]`.
- Jacobian/determinant records: `[element][stored point][record scalar]`.

The Jacobian fetch copies exactly `jacd_size` scalars, including the determinant
or caller-defined measure, without transforming their meaning. It should
deduce the source/destination scalar type.

Prefer `Tile<Free>::...` calls; an empty object can call the same static methods.
The proposed return type is `void`, with `noexcept` and documented valid-buffer
preconditions. Zero count is a valid no-op, not an error fallback.

## Baseline design decisions

- Use an empty `Free` tag and a partial specialization in
  [Tile.hpp](../../src/femera/grid/Tile.hpp).
- No dimensions, topology, array registration, allocation, or shape registry.
  `get_elem_d()` remains the responsibility of known element/mesh metadata.
- Keep the existing structured tile for metadata and initialization in
  [Bars.perf.hpp](../../src/femera/grid/elem/Bars.perf.hpp).
- Keep `elem_mod` mapping at the caller.
- Reuse one fetched record for affine geometry. Fetch inside the integration
  loop only when records are actually stored per point.
- Validate layout/capacity during setup, not silently inside unchecked copies.

## Alternatives and performance

### Typed alternative: Free<Bar1>

Model a separate `Free<T, P=1, F=fmr::Geom_float>` template after `Tile`.
It remains stateless and uses the same caller-owned arrays, but selectively
forwards public metadata from `Tile<T, P, F>`:

- `get_elem_p()` and `get_elem_d()`.
- `get_tile_elem_node_n()`.
- `get_tile_elem_jacs_dets_size()` when the descriptor's layout applies.

A size-derived fetch API can omit redundant width arguments:

```cpp
Free<Bar1, P>::get_tile_elem_conn(conn, master_conn, elem_i);
Free<Bar1, P>::get_tile_elem_jacd(
  jval, master_jacd, elem_i, stored_intp_n, ip);
```

| Concern | `Tile<Free>` | `Free<Bar1, P>` |
| --- | --- | --- |
| Data ownership | Caller | Caller |
| Element metadata | External to policy | Known through descriptor |
| Copy widths | Runtime arguments | Can be compile-time derived |
| Arbitrary layouts | Natural raw interface | Requires matching metadata or raw-width overload |
| Inlining | Expected, verify | Expected, verify |
| Full structured kernel replacement | No | No |

Do not inherit `Tile<T>` publicly: free meshes must not expose Cartesian
partition counts or generated halos accidentally.
[Bar1.hpp](../../src/femera/grid/tile/Bar1.hpp) has private descriptor members
and is final; use public `Tile` accessors rather than deriving from Bar1 or
adding friendship.

`Bar1` supplies a two-scalar geometric record for its 1D convention, not every
embedded bar geometry. Also, tile counts are not element counts:
[Bar5.hpp](../../src/femera/grid/tile/Bar5.hpp) has five bars and four vertices
per tile, but `P+1` nodes per individual bar, and lacks Bar1's geometric size
accessor. Generalization needs an explicit per-element metadata contract.

**Earlier recommendation:** prefer typed `Free<T, P, F>` if the unified interface needs
element metadata as well as fetching. Keep the raw baseline if the scope is
only extracting the two copy loops. Typed sizes may help constant propagation,
but do not guarantee a speed advantage over raw calls with known sizes.

An empty `struct Free` and a class template `Free` cannot coexist under the
same name in the same namespace. Choose the public design before implementation;
neither draft is an existing compatibility obligation.

### Other alternatives

Non-owning views and standalone helpers can also inline efficiently; reject
them for unnecessary scope here, not an assumed optimization penalty.
Compile-time size overloads and alternative copy primitives are options only
if measurements demonstrate a need.

The target is no abstraction overhead compared with the original copy loops.
Inlining and unrolling are plausible with the existing aggressive flags, not
guaranteed. Verify actual caller assembly, inline reports, and repeated
baseline/replacement performance runs.

The earlier 10-20% overhead estimate, fixed timeline, broad integration phases,
and invented `Free` dimensions are superseded.

## Implementation order

The detailed plan now prefers an element-based path: add a narrow `Free.hpp`
using `Elem`, test P1-P3 simplex connectivity and one-record geometric fetches
with multiple quadrature points, then replace only stored-array copies.
Verify vertex-defined geometry and optimized callers. Keep structured
initialization and halo handling unchanged. Earlier tile-based and raw-copy
paths remain documented for comparison.

Baseline path:

1. Add the two methods and focused tests in
   [Tile.gtst.cpp](../../src/femera/grid/Tile.gtst.cpp).
2. Replace only the two copies in the bar response kernel, preserving storage
   layout, record selection, and numerical behavior.
3. Compare correctness, optimized caller code, and measured runtime against
   the original loops.

See the [implementation plan](tile-free-unstructured-mesh-plan-2026-11-26.md)
for all three designs, partition invariants, geometry layouts, and current
implementation status. No full application build or performance result is
claimed yet.
