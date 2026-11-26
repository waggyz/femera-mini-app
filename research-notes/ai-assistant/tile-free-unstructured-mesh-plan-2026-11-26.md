# Free element-based mesh access: alternatives and implementation plan

Date: 2026-11-26  
Status: Bars-only prototype; focused tests and translation-unit checks pass.

## Current prototype

The copied placeholders now implement
[Free.hpp](../../src/femera/grid/Free.hpp) and
[Free.ipp](../../src/femera/grid/Free.ipp) in `femera::grid::free`.
Only `Free<elem::Bars, P, F>` is enabled, for P1-P3. Tris/Tets and other
elements are future work, not supported or certified by this implementation.

The public interface is:

```cpp
#include "src/femera/grid/Free.hpp"
#include "src/femera/grid/elem/Bars.hpp"

using femera::grid::elem::Bars;
using femera::grid::free::Free;

using Mesh = Free<Bars, 2>;
// Element aliases Elem<Bars, 2>; reference dimension remains fixed at 1.
const auto conn_n = Mesh::get_elem_node_n();
const auto jacd_size = Mesh::get_elem_jacd_size(1); // physical dimension
// Call with caller-owned buffers:
// Mesh::get_elem_conn(conn, master_conn, elem_i);
// Mesh::get_elem_jacd(jval, master_jacd, jacd_size, elem_i);
```

Metadata accessors are `get_elem_p`, `get_elem_d`, `get_elem_node_n`,
`get_elem_vert_n`, and `get_elem_jacd_size(sims_d)`. The latter describes the
ordinary packed matrix-plus-measure convention; fetches accept an explicit
record width for the caller's actual representation.

The two copies are header-visible `static constexpr void` functions marked
`noexcept`. They allocate nothing, copy full-order connectivity, and fetch one
opaque geometric record per bar element, independently of quadrature count.
Valid buffers, sufficient capacity, non-overlap, and representable offsets
are caller preconditions. A zero geometric width is a no-op.

The remaining copied translation units and test header have been stripped of
irrelevant scaffolding. Two focused tests in
[Free.gtst.cpp](../../src/femera/grid/Free.gtst.cpp) exercise P1-P3 connectivity,
float/double record copies, several record widths and elements, destination
guards, and zero-width no-ops. The standalone Google Test entry point avoids
requiring Femera job initialization for these stateless copies.

After the user reauthorized validation, both `FreeBars.*` Google Tests passed
when compiled as C++14 with `-Ofast`, native architecture tuning, loop unrolling,
and warning flags. Syntax-only compilation of `Free.cpp` and `Free.test.cpp`
also passed with warning flags and no diagnostics.
No full application build, optimized assembly comparison, or runtime-performance
claim is made.
Integration into the existing bar response caller and broader geometry tests
remain deferred.

This is early prototype work: no backward-compatibility requirement or broad
test-certification gate is imposed. Each underlying element type has a fixed
reference `elem_d`; physical embedding dimension is separate. Earlier unknown-
dimension concerns applied only to an untyped marker, not to `Free<Bars>`.
The historical alternatives and proposed future steps below are retained for
design context and are not implemented requirements.

## Scope and baseline proposal

The baseline proposal is `Tile<Free>` as a stateless, header-defined
specialization with two static operations:

1. Copy one element's connectivity from a master array into a supplied buffer.
2. Copy one integration point's Jacobian/determinant record from a master array
   into a supplied buffer.

The caller owns every array and supplies its sizes, indexing, ordering, and
geometric meaning. No allocation, mesh loading, Jacobian computation, pointer
registration, runtime topology dispatch, or coordinate gathering is required
for these two operations.

The user's prior-version testing identifies these copies as important
performance paths. The goal is **no abstraction overhead relative to the
existing copy loops**, not a predicted performance ratio between structured and
unstructured meshes.

This revision supersedes the original proposals to pass unused structured-grid
arguments, invent dimensions for `Free`, return false for invalid inputs, add
file readers, and predict 10-20% overhead or a fixed implementation timeline.
None of those is needed or established by this request.

The [element-based alternative](#preferred-alternative-freebars-based-on-elem)
below is now preferred: `Free<Bars, P, F>`, using `Elem<Bars, P, F>` for metadata,
rather than `Free<Bar1>` using tile metadata. It fits homogeneous partitions,
uniform quadrature counts, and vertex-defined geometry directly. Earlier
sections retain the raw-copy and tile-based alternatives for comparison; their
recommendations are superseded by the element-based section and the current
Bars-only prototype. No alternative has been benchmarked.

## Relevant current code

- [Tile.hpp](../../src/femera/grid/Tile.hpp): the primary template
  `Tile<T, P, F>` forwards structured operations to `T::calc_*` methods.
  `get_elem_d()` returns `T::elem_d`; counts and patterns also assume a topology.
- [Tile.ipp](../../src/femera/grid/Tile.ipp): currently an empty implementation
  shell in `femera::grid`, whereas `Tile` lives in `femera::grid::tile`.
- [Bars.perf.hpp](../../src/femera/grid/elem/Bars.perf.hpp):
  `MeshPhys::get_part_resp` copies connectivity into `conn`, then gathers
  displacement values using those indices. It also copies Jacobian/determinant
  scalars into `jval`, whose last active scalar is used as the determinant.
  The arrays are caller-owned `std::valarray` members.
- [Bars.perf.hpp](../../src/femera/grid/elem/Bars.perf.hpp):
  `init_mesh` currently sets one integration point and builds one geometric
  record per bar element. Affine geometry reuse is therefore relevant.
- [fmr.hpp](../../src/fmr/fmr.hpp): `Dim_int` is unsigned `uint_fast8_t`;
  `-1` must not be used as an "unknown dimension."
- [Makefile](../../Makefile): the GNU build uses C++14, optional LTO, and an
  existing `ENABLE_INLINE_REPORT=ON` path for optimization evidence.
- [Tile.gtst.cpp](../../src/femera/grid/Tile.gtst.cpp): existing Google Test
  file suitable for focused copy tests; it currently has a trivial test.

The performance files have work-in-progress edits. Implementation must read
their current contents again before changing them; this planning revision
does not modify those files.

## Proposed interface

Keep the user's parameter order. Use `void` for a copy with documented
preconditions: there is no meaningful success result or recoverable failure
that these signatures can detect, since they receive no master-array capacity.
Use `noexcept` for the numeric copies and header-visible definitions.

The following is an implementation sketch, not an implemented API:

```cpp
namespace femera { namespace grid { namespace tile {

struct Free {};

template <fmr::Dim_int P, typename F>
class Tile<Free, P, F> {
public:
  static constexpr void get_tile_elem_conn (
    fmr::Local_int* conn, const fmr::Local_int* part_node_conn,
    const fmr::Local_int conn_n, const fmr::Local_int elem_i) noexcept {
    if (conn_n == 0) { return; }
    const std::size_t offset = std::size_t(elem_i) * std::size_t(conn_n);
    for (fmr::Local_int i = 0; i < conn_n; ++i) {
      conn[i] = part_node_conn[offset + std::size_t(i)];
    }
  }

  template <typename Y>
  static constexpr void get_tile_elem_jacd (
    Y* jval, const Y* this_jacd,
    const fmr::Local_int jacd_size, const fmr::Local_int elem_i,
    const fmr::Local_int elem_intp_n, const fmr::Local_int ip) noexcept {
    if (jacd_size == 0) { return; }
    const std::size_t offset =
      (std::size_t(elem_i) * std::size_t(elem_intp_n) + std::size_t(ip))
      * std::size_t(jacd_size);
    for (fmr::Local_int i = 0; i < jacd_size; ++i) {
      jval[i] = this_jacd[offset + std::size_t(i)];
    }
  }
};

} } }
```

Add an explicit `<cstddef>` include where these definitions reside.
`constexpr` functions are implicitly inline and C++14 permits these loops.
They also work at runtime with ordinary pointers.

Make the Jacobian copy a function template so both float and double master
arrays work independently of the class's default geometry scalar. Source and
destination must have the same scalar type; this is not a conversion operation.
`P` and `F` preserve compatibility with the primary template but do not determine
record sizes or topology in this specialization.

Either spelling is valid:

```cpp
using femera::grid::tile::Free;
using femera::grid::tile::Tile;

Tile<Free>::get_tile_elem_conn(conn, master_conn, conn_n, elem_i);

Tile<Free> free;
free.get_tile_elem_jacd(jval, master_jacd, jacd_size, elem_i, elem_intp_n, ip);
```

Prefer qualified static calls in the performance loop to communicate that no
object or stored state is needed. Object syntax should generate the same code.

### Layout and preconditions

Connectivity is packed as `[element][local node]`:

`conn[i] = part_node_conn[elem_i * conn_n + i]`.

Geometric records are packed as `[element][integration point][record scalar]`:

`jval[i] = this_jacd[(elem_i * elem_intp_n + ip) * jacd_size + i]`.

- `conn_n` and `jacd_size` are scalar counts, not byte counts.
- Connectivity values are copied unchanged: no node renumbering or gathering.
- `jacd_size` includes every scalar in a complete record, including the final
  determinant or other caller-defined geometric measure.
- A nonempty copy requires valid pointers and sufficient source/destination
  capacity. For Jacobian copies, `elem_intp_n > 0` and `ip < elem_intp_n`.
- Counts and indices must be nonnegative if a signed `Local_int` configuration
  is used. Offsets and extents must fit `std::size_t` and the allocated arrays.
  Casting before multiplication avoids intermediate `Local_int` overflow but
  does not remove these caller obligations.
- Source and destination ranges must not overlap.
- Zero scalar count is a valid no-op, including with null pointers. The early
  return avoids forming an offset or dereferencing either pointer.
- Validate array extents and layout at the caller's setup boundary using
  existing diagnostics/exceptions. Do not silently report success for an invalid
  nonempty request or introduce per-fetch validation without measuring its cost.
  Optional debug assertions can be considered separately.

`get_tile_elem_jacd` copies an opaque record. It does not compute, invert,
transpose, normalize, or validate Jacobians. Existing tile methods describe
transposed inverse Jacobians plus a non-inverted determinant. Preserve that
contract at their consumers; never substitute an inverse determinant or assume
that a rectangular Jacobian has an ordinary determinant.

## Alternatives

| Approach | Assessment |
| --- | --- |
| Empty `Free` tag with a partial specialization and static copies | Baseline: explicit runtime pointers/counts, no owned state or topology assumptions. |
| `Free<elem::Bars, P, F>` using `Elem` | Preferred under the stated partition/geometry invariants; element metadata and simplex geometric reuse do not depend on a tiling descriptor. |
| `Free<T, P, F>` modeled after `Tile<T, P, F>` | Stronger candidate for a homogeneous-element kernel needing metadata and stored-data access through one type; see the detailed comparison below. |
| Add master-array copy overloads to the primary `Tile<T>` | Could expose identical fetch names on structured and free types, but broadens all tiles before a consumer requires it. Revisit only when defining a genuinely common kernel contract. |
| Standalone copy functions, optionally forwarded by `Tile<Free>` | Equally capable of inlining. Reuse an existing suitable helper if found; otherwise unnecessary forwarding for two short loops. |
| Non-owning view storing pointers and strides | Does not violate caller ownership and can optimize well. Useful for richer layouts, but unnecessary state and lifetime contracts for the requested explicit-pointer API. |
| Compile-time count overloads | May improve specialization when the compiler cannot propagate counts. Defer until code-generation evidence shows a need. Runtime arguments that become constants after inlining may already unroll. |
| `std::copy_n` or `std::memcpy` | Plausible copy implementations, not intrinsically faster or slower. Explicit loops match current code and support C++14 `constexpr`; measure alternatives only if necessary. |
| Runtime virtual mesh interface | Useful for other requirements, but not needed for two static numeric fetches. Do not replace the existing Tile architecture. |

Do not claim that storing runtime pointers prevents optimization, or that
virtual interfaces can never devirtualize. The recommendation is based on the
requested scope and simplicity, not unsupported performance generalizations.

## Dimensions and what "unified" means

`Free` describes a data-access policy, not a reference element. The two copies
do not require an element dimension, shape, polynomial order, or node topology.

Consequently, the initial specialization should **not** offer `get_elem_d()`,
`get_tile_d()`, structured counts, patterns, periodic connectivity generation,
halo enumeration, or mesh generation. Do not fabricate zero counts or sentinel
dimensions to make generic consumers compile.

Continue to obtain dimensions, node counts, shape gradients, and quadrature
from the caller's known element or mesh metadata. Copying data does not require
a runtime element registry, even when different calls use different record
sizes.

The initial interface unifies the `Tile<...>` family and stored-data fetch
operations; it does not make every existing structured method interchangeable.
If a future kernel must switch between generated and stored geometry through
one signature, design that shared fetch contract explicitly. Do not add unused
`tile_size`, coordinates, partition dimensions, or origin parameters just to
imitate a structured signature.

## Alternative: Free<T, P, F> modeled after Tile

### Meaning and ownership

Use a separate class template:

```cpp
template <class T, fmr::Dim_int P=1, typename F=fmr::Geom_float>
class Free;
```

`Tile<Bar1, P, F>` represents generated structured data and its metadata.
`Free<Bar1, P, F>` represents stored data with compatible element metadata.
Both remain stateless: the caller passes master pointers, output buffers, and
element/point indices on each call. Parameterizing `Free` does not mean storing
arrays, constructing a `Bar1`, or assuming consecutive mesh node IDs.

`Bar1` is a tile descriptor, not the `elem::Bars` reference topology. Accepting
it is convenient because it already supplies the bar kernel's metadata; this
distinction matters when generalizing beyond that descriptor.

### Reuse public metadata, not structured behavior

[Bar1.hpp](../../src/femera/grid/tile/Bar1.hpp) makes its constructor and
`calc_*` members private and grants friendship to `Tile`, not to a new `Free`.
It is also `final`. Therefore:

- Do not derive from `Bar1`, access `T::elem_d` directly, or add friendship merely
  to duplicate `Tile`'s metadata logic.
- Use selected **public static accessors** of `Tile<T, P, F>` instead.
- Do not publicly inherit `Tile<T, P, F>`: that would also expose procedural
  connectivity, partition count formulas, halo generation, and Cartesian
  coordinate operations as if they applied to the unstructured mesh.
- Keep forwarding explicit and limited; no broad Tile facade is needed.

For example, a metadata-only sketch is:

```cpp
template <class T, fmr::Dim_int P=1, typename F=fmr::Geom_float>
class Free {
  using Metadata = Tile<T, P, F>;
public:
  static constexpr fmr::Dim_int get_elem_p () noexcept {
    return P;
  }
  static constexpr fmr::Dim_int get_elem_d () noexcept {
    return Metadata::get_elem_d();
  }
  static constexpr fmr::Local_int get_tile_elem_node_n () noexcept {
    return Metadata::get_tile_elem_node_n();
  }
  static constexpr fmr::Local_int get_tile_elem_jacs_dets_size () noexcept {
    return Metadata::get_tile_elem_jacs_dets_size();
  }
};
```

This sketch assumes the selected descriptor supplies those accessors with the
right per-element meaning. It does not instantiate a metadata object or fetch
any generated mesh data. The `get_tile_*` names preserve current caller naming;
the records being fetched are still **single elements**, not whole tiles.

### Fetch signatures: explicit versus descriptor-derived sizes

There are two sensible typed API variants:

1. Keep the baseline's explicit `conn_n` and `jacd_size` signatures, adding only
   metadata accessors. This is the smallest caller change, but the template
   parameter does not constrain the record sizes.
2. Derive sizes in typed fetches, omitting redundant count arguments. This is
   preferable if `Free<Bar1>` is intended to promise a specific element layout:

```cpp
Free<Bar1, P>::get_tile_elem_conn(conn, master_conn, elem_i);
Free<Bar1, P>::get_tile_elem_jacd(
  jval, master_jacd, elem_i, stored_intp_n, ip);
```

The connectivity loop uses `get_tile_elem_node_n()`. The geometric loop uses
`get_tile_elem_jacs_dets_size()`. Their offset equations remain exactly the
baseline's equations. Keep `stored_intp_n` and `ip` runtime arguments: the
descriptor does not specify the number of stored geometric records.

Keep the Jacobian fetch itself templated on the source/destination scalar type,
as in the baseline, so `Free<Bar1>` can copy float or double without conversion.
Keep `void`, `noexcept`, valid-range/non-overlap preconditions, and caller-side
validation. A derived, nonzero record size does not make a null pointer valid
or let the helper detect master capacity.

For `Bar1`, P1 gives two connectivity entries, P2 three, and P3 four. The
descriptor's geometric record size remains two for its 1D inverse-Jacobian
and determinant convention. `P` is separate from the `1` in the name `Bar1`;
do not infer polynomial order from the tile descriptor's spelling.

If explicit-size overloads are retained, document them as raw-layout operations,
not as guarantees of descriptor-compatible data. Avoid checking a redundant
runtime count on every fetch; validate compatibility during setup or use the
size-derived interface.

### Where topology knowledge is insufficient

- `get_elem_d()` is a reference dimension. The physical embedding dimension,
  Jacobian convention, and packed record width must still match the actual
  caller's data. A 1D bar embedded in 2D/3D does not automatically use Bar1's
  two-scalar record.
- [Bar5.hpp](../../src/femera/grid/tile/Bar5.hpp) has five bars per tile,
  four tile vertices, `tile_d=2`, and `elem_d=1`. Its explicit per-element node
  accessor returns `P+1`; a free fetch must not copy all tile vertices or all
  five elements. It does not currently supply the same geometric-record-size
  method as Bar1, so blindly forwarding that accessor does not implement
  `Free<Bar5>` Jacobian fetching.
- An unstructured mesh has no implied Cartesian `get_tile_d()` or partition
  dimensions. Do not forward tile counts, partition formulas, halo enumerators,
  or pattern variants as free-mesh metadata.
- Neither `Free<Bar1>` nor `Tile<Free>` automatically provides shape functions.
  Continue to use `Elem<Bars, P>` for the current bar kernel. A future general
  shape mapping should be an explicit separate contract.

For descriptors whose geometric layout is unavailable or unsuitable, require
an explicit raw width or a future geometry-layout descriptor. Do not guess
`elem_d * elem_d + 1` for all embeddings. A future element-based
`Free<elem::Bars, ...>` could decouple element metadata from tiling, but would
need a deliberately different metadata adapter; it is not drop-in compatible
with `Tile<T>` forwarding.

### Comparison with the baseline

| Concern | `Tile<Free>` | `Free<Bar1, P, F>` |
| --- | --- | --- |
| Caller-owned buffers and stateless calls | Yes | Yes |
| `get_elem_d()` and node count | Supplied elsewhere | Selectively forwarded compile-time metadata |
| Copy widths | Explicit at every call | Explicit or descriptor-derived; derived widths express a stronger contract |
| Unknown/custom record layouts | Directly usable | Needs raw-width API or suitable metadata |
| Different element types per call | Same raw API, caller chooses widths | Separate typed specializations/batches; raw overload can still copy arbitrary widths |
| Known-size optimization | Depends on caller constant propagation | Derived widths are visible in the helper |
| Guaranteed speed advantage | None | None |
| Primary `Tile` changes | Add tag and specialization | None required; new template uses public metadata |
| Risk of leaking grid assumptions | Low with a narrow specialization | Low only with selective forwarding, not public inheritance |
| Full existing kernel substitutability | No | No: structured initialization/halo calls still differ |

Both should inline to the same memory operations for identical, known widths.
Typed widths can help when a runtime count would otherwise stay unknown, but
there is no inherent penalty for the raw policy when the caller already obtains
counts from compile-time metadata. Code size may grow with typed instantiations;
the existing Tile approach already makes a similar tradeoff.

### Caller migration and naming

The current `MeshPhys<T, P, F>` owns a `Tile<T, P>` member and uses it for both
metadata and structured initialization. A typed fetch policy could initially
be used as `Free<T, P, F>` beside that existing tile, replacing only the two
copies. It could also supply the response kernel's three needed metadata
queries: dimension, per-element node count, and geometric record size.

It is **not** safe to replace the member's type globally with `Free<T, P, F>`:
mesh generation, grid-based partition sizing, and halo operations still depend
on the structured tile. A later policy-based response kernel can accept either
provider only after both expose a genuine common fetch signature. Neither
proposal currently supplies that signature on all structured tiles.

The baseline empty `struct Free` and a new `template<...> class Free` cannot
share the same name in the same namespace. Because the baseline is only a
proposal, choose one public spelling before implementation:

- Adopt `Free<T, P, F>` and drop the unimplemented `Tile<Free>` spelling; or
- Keep `Tile<Free>` and give the typed policy a different name.

Do not introduce both overlapping public APIs just to preserve an unimplemented
draft. If raw copies are needed within the typed design, use small shared
implementation helpers or explicit raw overloads, not another public hierarchy.

### Earlier tile-based recommendation and implementation path

This recommendation predates the element-based alternative below and is
retained only to explain the comparison. Prefer the element-based path under
the clarified partition invariants.

For the broader original goal of a unified interface that includes element
metadata, **prefer `Free<T, P, F>` with selective metadata forwarding and
size-derived fetches**, initially certified for `Bar1`. It provides a legitimate
`get_elem_d()` without sentinel values and expresses the fixed-size homogeneous
element case directly. It preserves all caller-ownership and inlining goals.

For extraction of just the two raw loops, the baseline remains simpler and
fully sufficient. Choosing the typed design is an interface/metadata decision,
not a claim of measured speed improvement.

If the typed design is selected:

1. Add a header-visible `Free<T, P, F>` in a new
   `src/femera/grid/Free.hpp`, including `Tile.hpp`. Callers also include their
   descriptor header. Do not make `Tile.hpp` include `Bar1.hpp`, which already
   includes `Tile.hpp`, or alter descriptor friendship.
2. Forward only the supported element metadata and implement the two
   size-derived copies. Reuse existing suitable copy helpers if found.
3. Test P1-P3 Bar1 connectivity with arbitrary IDs and distinct elements, float
   and double geometric records with distinct points, and unchanged buffer
   guards. Verify the actual record widths and reference dimension through
   those fetch tests, not only trivial accessor tests.
4. Replace the two response-kernel copies, preserving affine reuse and
   `elem_mod`. Keep structured initialization and halos untouched.
5. Compare baseline loops, raw-policy calls, and typed-policy calls in actual
   optimized caller kernels. Use the existing performance validation criteria.
   Do not ship a second policy solely to perform this comparison; a temporary
   local experiment can compare candidates.

No benchmark or compilation of this alternative is claimed by this document.

## Preferred alternative: Free<Bars> based on Elem

### Partition and geometry contracts

The user's clarified performance model supplies the important invariants:

- Each partition has one element type and one common quadrature-point count.
  Select the kernel at the partition boundary, not by switching element types
  inside the element loop.
- Under the supported natural-node restriction, geometry is determined by
  vertex coordinates; edge/face node positions are reconstructed from them.
  This does not permit curved or independently displaced higher-order geometry
  nodes.
- Bars, Tris, and Tets therefore have affine geometric mappings: Jacobians and
  their determinants/measures are independent of reference point and solution
  order. Store one complete geometric record per element.

`P` still describes the solution/interpolation order. A `Free<T, P, F>` kernel
assumes a fixed order/layout within its invocation; homogeneous topology and
quadrature alone do not logically guarantee identical order. If differing
orders are introduced later, split into fixed-order batches or explicitly
revise the connectivity-stride contract.

These invariants allow packed arrays and partition-wide strides without CSR
offsets, per-element type tags, or per-element quadrature counts. They do not
imply equal Jacobians across different elements: arbitrary vertex coordinates
still produce one generally different record for each element.

### Why the element descriptor is the better dependency

Use `femera::grid::elem::Bars`, not `femera::grid::tile::Bar1`, as the type
argument. The former describes the reference element; the latter describes a
particular way of generating structured bar meshes.

[Elem.hpp](../../src/femera/grid/Elem.hpp) already supplies:

- `get_elem_p()`, `get_elem_d()`, `get_vert_n()`, and `get_node_n()`.
- Element edge/face connectivity and reference shape functions/gradients.
- `get_elem_node_coor` for reconstructing natural higher-order coordinates.
- `get_jacs_dets_size(1, sims_d)` for the ordinary packed matrix-plus-measure
  record width, with an explicit physical dimension.

Forward through `Elem<T, P, F>` rather than accessing private fields on `Bars`.
`Bars` is final, with private fields and friendship granted to `Elem`; there is
no reason to derive from it or add `Free` friendship.

Prefer an explicit alias and selective forwarding, rather than inheriting the
whole `Elem` interface. This keeps storage fetches distinct from reference
evaluation and avoids accidental signature/meaning collisions.

Metadata-only sketch, in a namespace alongside the existing mesh policies:

```cpp
template <class T, fmr::Dim_int P=1, typename F=fmr::Geom_float>
class Free {
public:
  using Element = femera::grid::elem::Elem<T, P, F>;

  static constexpr fmr::Dim_int get_elem_p () noexcept {
    return Element::get_elem_p();
  }
  static constexpr fmr::Dim_int get_elem_d () noexcept {
    return Element::get_elem_d();
  }
  static constexpr fmr::Local_int get_elem_node_n () noexcept {
    return Element::get_node_n();
  }
  static constexpr fmr::Local_int get_elem_vert_n () noexcept {
    return Element::get_vert_n();
  }
  static constexpr fmr::Local_int get_elem_jacd_size (
    const fmr::Dim_int sims_d) noexcept {
    return Element::get_jacs_dets_size(1, sims_d);
  }
};
```

Keep the physical dimension explicit: `Elem` defaults some sizes to three
dimensions, and topology reference coordinates may have three stored components.
Neither establishes the physical dimension of the partition.

The size helper above describes the existing ordinary packed convention, not
every optimized low-dimensional representation. The master-array producer and
fetch consumer must agree on inverse/pseudoinverse orientation, extra scalar
meaning, and any special layout. Fetching remains an opaque same-type copy.

### Free<Bars> versus Free<Bar1>

| Concern | `Free<Bar1, P, F>` | `Free<Bars, P, F>` |
| --- | --- | --- |
| Metadata source | `Tile<Bar1, P, F>` | `Elem<Bars, P, F>` |
| Meaning of type parameter | Structured tiling descriptor | Reference element topology |
| Reference dimension / full node count | Available through tile facade | Available directly through element facade |
| Higher-order basis | Separate mapping from Bar1 to Bars needed | `Element` alias already selects the matching basis |
| Geometric record width | Bar1 reports two for its 1D convention | Dimension-aware ordinary width via Elem; explicit raw width for other layouts |
| Simplex affine classification | Tied to a tile naming/mapping layer | Directly classify Bars, Tris, and Tets |
| Arbitrary physical placement | Possible, but structured descriptor is incidental | Natural model: caller supplies arbitrary vertex positions |
| Multi-element tiles | Must distinguish tile and element metadata | No tile counts or patterns involved |
| Single-type partition dispatch | Possible | Direct match to the partition invariant |
| Array ownership / inline copies | Caller-owned / header-visible | Identical |
| Guaranteed runtime advantage | None | None; cleaner contracts, not an automatic speedup |

One `Free<Bars>` can fetch stored bar records whether their source was a Bar1
grid, a Bar5 truss grid, or an imported unstructured mesh. This is especially
useful given that Bar5 has several elements per tile but the kernel operates on
one bar at a time. No separate `Free<Bar5>` metadata adapter is necessary.

Keep `Tile<Bar1>` and `Tile<Bar5>` where structured mesh generation and halo
logic actually need them. They need not determine the element type of a
stored-data response policy.

### Separate quadrature points from geometric records

Define two counts explicitly:

- `quad_intp_n`: the common number of quadrature points used by every element
  in this partition.
- `geom_intp_n`: the number of geometric records stored per element.

For the supported natural-node Bars/Tris/Tets contract, `geom_intp_n = 1` for
all supported orders, regardless of `quad_intp_n`.

For a non-simplex natural-node element, vertex-only geometry does **not**
generally mean a constant Jacobian. Distorted Quad/Cube mappings can vary by
reference point; use `geom_intp_n = quad_intp_n` for their point-dependent path.
Any additional constant-mapping optimization requires evidence about that
geometry, not just fewer geometry nodes.

Represent the proven affine-simplex property by a small compile-time topology
trait or specialization for Bars, Tris, and Tets. Keep it conditional on the
natural-node contract; do not label all higher-order simplices affine in a
future curved/isoparametric mode. For other supported topologies use the
point-dependent path; reject unsupported geometry contracts at setup or compile
time, not by returning false-shaped metadata.

Layout equations:

```text
conn_n = Elem<T, P, F>::get_node_n()
connectivity offset = elem_i * conn_n

simplex geometric offset = elem_i * jacd_size
point-dependent geometric offset =
  (elem_i * quad_intp_n + ip) * jacd_size
```

Use wide offset arithmetic and all the raw-copy capacity/non-overlap contracts
already described above. Retain `elem_mod` as caller-side record mapping.

For the ordinary matrix-plus-measure layout:

```text
master connectivity scalars = elem_n * conn_n
simplex geometric scalars = elem_n * jacd_size
point-dependent geometric scalars = elem_n * quad_intp_n * jacd_size
```

An order-independent geometric record does **not** imply an order-independent
connectivity width. P2/P3 degrees of freedom must still be gathered/scattered.
Nor does it imply one quadrature point is enough: higher-order reference
gradients, material variation, and physics still require the configured rule.

### Fetch API and hot-loop shape

Use element-oriented names rather than retaining `get_tile_*` on a type that
has no tile semantics. A narrow proposed API is:

```cpp
// Full interpolation connectivity; width comes from Elem<T, P, F>.
Free<Bars, P>::get_elem_conn(conn, master_conn, elem_i);

// One simplex record, reused over all quadrature points.
Free<Bars, P>::get_elem_jacd(jval, master_jacd, jacd_size, record_elem_i);

// Per-point variant for a supported non-simplex provider.
Free<Quad, P>::get_elem_jacd(
  jval, master_jacd, jacd_size, record_elem_i, quad_intp_n, ip);
```

Here `Bars` and `Quad` name underlying element types. The geometric scalar type
is deduced from master/output pointers. Retaining an explicit `jacd_size`
preserves the user's choice of geometry representation and embedding; the
ordinary size can be obtained once from `get_elem_jacd_size(sims_d)`. If the
representation and dimension are fixed in the compiled caller, this argument
can constant-propagate exactly as before.

The simplex overload deliberately needs no quadrature stride or point index.
For a later common kernel interface, a topology-specialized adapter can expose
both paths without runtime type selection. Do not read a nonexistent
`quad_intp_n`-sized simplex block merely to keep an identical surface signature.
A common API accepting quadrature indices must explicitly map them to geometric
point zero for affine simplices.

Simplex loop:

1. Fetch full element connectivity.
2. Fetch one geometric record and obtain its determinant/measure.
3. Gather the full solution data.
4. Iterate `quad_intp_n` times using the same record, but the correct
   point-specific reference gradient and weight.
5. Scatter the element response.

No geometry calculation or reconstruction is introduced into these fetches.

### Vertex-only geometry setup

Caller setup can gather just `Element::get_vert_n()` coordinates using the
vertex entries of the full connectivity. The existing natural-node ordering
puts vertices first, followed by edge and eligible face nodes; imported
connectivity must match that order before entering the hot kernel.

For simplices, form the geometric map from the P1 reference element and vertex
coordinates, then calculate its inverse/pseudoinverse and determinant/measure
once per element. Changing solution order must not change this geometric map.
The existing `calc_bars_invt_dets<P>` in
[Bars.perf.hpp](../../src/femera/grid/elem/Bars.perf.hpp) already gathers the
first `Elem<Bars, 1>` nodes from a full-order connectivity row and uses P1
gradients. Reuse that mechanism rather than computing high-order geometry in
the response loop.

`Elem::get_elem_node_coor` can reconstruct natural edge/face coordinates for
export, validation, or a caller that needs them. Vertex-only setup does not
require deleting a caller's full coordinate array, and the copy policy must
not impose that storage choice. General non-simplex setup may also reconstruct
coordinates or evaluate its vertex-defined map at the configured points; it
must not assume an affine map.

The present bar setup has a TODO for rectangular pseudoinverse geometry.
Element-based metadata supports dimension-aware sizing, but does not itself
implement that missing calculation. Keep embedded-geometry implementation
outside this copy extraction; document and validate the supported producer
layout.

### Revised preferred implementation sequence

1. Introduce `Free<T, P, F>` in a narrow new `src/femera/grid/Free.hpp` based on
   `Elem.hpp`, not `Tile.hpp`. Callers include the selected element header.
   Keep the existing mesh-policy namespace initially; avoid a namespace-wide
   reorganization. Do not change the primary Tile class or descriptor friendship.
2. Forward element metadata and add full-connectivity and simplex-record
   copies, beginning with Bars only. Extend to Tris and Tets as their Elem
   implementations develop. Defer general non-simplex point-fetch
   implementation unless a caller needs it.
3. Keep quadrature metadata at partition setup. Allocate one geometric record
   per simplex element, not per solution node or quadrature point. No runtime
   element tags, mixed-type dispatch, CSR storage, or per-element point counts.
4. In the bar response caller, use `Free<elem::Bars, P, F>` beside the existing
   structured tile. Keep generation, partition sizing, halos, and boundary
   conditions unchanged. Do not blindly use its existing `T` argument:
   `MeshPhys<T, P, F>` currently receives a tile type, not an element type.
5. Test full connectivity and single-record addressing for P1-P3 simplices
   using multiple elements and multiple quadrature points. Include nonzero
   element indices, distinct per-element records, and destination guards.
   The master geometry should contain only `elem_n * jacd_size` entries so an
   accidental quadrature stride is exposed.
6. Verify vertex-only geometry against reconstructed natural P2/P3 coordinates
   on nondegenerate, non-axis-aligned Bars/Tris/Tets, with multiple reference
   points. Check Jacobian/measure invariance across point and order, and that
   the P-dependent basis/response still uses the correct quadrature.
   Use existing geometry/evaluation helpers and numerical tolerances.
7. Verify existing bar responses, source-copy equivalence, actual optimized
   caller assembly, and repeated performance results. The typed architecture
   is cleaner; any claimed speed or memory improvement must name its baseline.
   In particular, the raw and tile-based plans already allowed affine reuse:
   choosing `Free<Bars>` alone does not improve their one-record-per-element
   storage further.

The initial Bars-only implementation is described at the top of this document.
The broader geometry tests, caller integration, and performance measurements
in this proposed sequence remain future work, not completion gates for the
early prototype.

## Baseline implementation sequence

### 1. Add the specialization and focused tests

- Define the empty `Free` tag in `femera::grid::tile` near the template, and put
  the partial specialization after the primary class in
  [Tile.hpp](../../src/femera/grid/Tile.hpp). This needs no new element header or
  change to `Elem.hpp`.
- Add the two public static functions with the exact layouts above.
- Keep the primary template unchanged. If definitions move to
  [Tile.ipp](../../src/femera/grid/Tile.ipp), use the correct `tile` namespace and
  preserve header visibility.
- Add behavioral tests in [Tile.gtst.cpp](../../src/femera/grid/Tile.gtst.cpp).
  Exercise the public API, not a duplicate implementation.

### 2. Replace only the two buffer copies

In [Bars.perf.hpp](../../src/femera/grid/elem/Bars.perf.hpp), keep the existing
structured `tile` for geometry metadata and initialization. Add `Tile<Free>`
only as the stored-data fetch policy:

```cpp
Tile<Free>::get_tile_elem_conn (
  &conn[0], &part_node_conn[0], conn_n, elem_i);
```

For `[element][point][scalar]` storage, inside the integration-point loop:

```cpp
Tile<Free>::get_tile_elem_jacd (
  jval, this_jacd, jacd_size, elem_i % elem_mod, elem_intp_n, ip);
```

Keep `elem_mod` outside the helper: it is caller-selected record mapping, not a
universal rule for unstructured meshes. The caller must ensure it is nonzero
and selects an allocated record.

The current work-in-progress source references `ip` before the later `ip`
loop. Resolve placement deliberately when implementing, rather than copying
that expression unchanged:

- **Affine geometry with one stored record per element:** fetch once before the
  integration loop with stored point count `1` and point index `0`; reuse
  `jval` and its determinant for all quadrature points.
- **Geometry stored per integration point:** fetch inside the `ip` loop with
  the actual stored point count; extract `this_det` after each fetch. Allocate
  and populate all records before selecting this path.

The stored geometric point count need not equal the quadrature point count.
The present bar initialization uses one point; preserve that behavior without
expanding the implementation into general curved-element support.

Do not change displacement gathering, arithmetic, boundary conditions, solver
behavior, mesh loading, or array ownership as part of the copy extraction.

### 3. Verify correctness, optimization, and performance

Correctness tests:

- Connectivity: select first, middle, and last elements with distinctive,
  nonconsecutive node IDs; verify exact copies and untouched destination guards.
- Jacobians: at least two elements and three points, all records distinctive;
  test nonzero `elem_i` and `ip`, the last point, and an affine one-point record.
  These cases catch an omitted point stride or wrong element offset.
- Exercise float and double records and more than one record width.
- Zero-count copies leave buffers unchanged and permit null pointers.
- Verify source arrays are unchanged. Do not invoke undefined invalid/overlap
  cases as normal unit tests.
- Compare the existing bar response/solver results before and after extraction,
  including the existing caller-selected `elem_mod` cases.

Optimization evidence:

- Capture a baseline from the working performance kernel immediately before
  extraction, then build the replacement with the same compiler and flags.
- Use the existing inline-report option and inspect optimized assembly for
  actual caller kernels, not only an isolated helper.
- Check that no call to the two fetch helpers remains, and compare address
  arithmetic, memory operations, alias checks, vectorization, and loop
  unrolling against the original loops.
- Test both compile-time-propagated sizes and genuinely runtime sizes. Neither
  `inline`, `constexpr`, nor `const` parameters guarantee unrolling.
- A compiler-generated copy-library call is not by itself a regression; compare
  the original loop's lowering and runtime results too.
- Do not add forced-inline attributes, restrict-like annotations, manual
  unrolling, or intrinsics without evidence. Preserve repository conventions
  if an inline attribute later proves necessary.

Performance evidence:

- Use the existing bar performance harness with identical mesh, physics,
  threading, warmup, flags, and hardware settings for baseline and replacement.
- Include cache-resident and larger master arrays. Keep outputs observable so
  the compiler cannot delete the copies.
- Interleave repeated baseline/replacement runs and record variation. Judge
  wrapper overhead against the original stored-array loops, not against
  analytically generated structured geometry.
- Acceptance: exact fetch results, unchanged response results, no additional
  helper-call overhead in the optimized hot path, and no reproducible slowdown
  beyond measured baseline variation. Record timings and dispersion rather than
  inventing a universal percentage threshold.

## Deferred work

Coordinate fetching, heterogeneous/CSR connectivity, per-element variable point
counts, non-packed strides, validated views, and a common generated/stored
kernel interface can follow demonstrated consumer needs. A caller can already
pass a pointer to a selected block to reuse the packed-copy operations.

Grid/Part initialization changes, mesh readers, external-library integration,
runtime shape registries, and hybrid-domain orchestration are separate tasks,
not prerequisites for the two hot-path fetches.

## Completion criteria

The implementation is complete when the two methods are available through the
selected policy, their packed-array behavior is tested, the two caller copy sites
use them without changing record mapping or affine reuse, and optimized code
plus performance results support the no-abstraction-overhead goal.

The passing focused tests and translation-unit checks are recorded at the top
of this document. No assembly comparison or benchmark result is claimed.
