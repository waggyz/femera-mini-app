# Plan: Correct embedded lower-dimensional elements

## Goal

Make a lower-dimensional element map correctly into a higher-dimensional
physical space, using `Bars` as the first implementation and test case. Keep
reference dimension (`elem_d`) distinct from ambient simulation dimension
(`sims_d`) throughout gradients, Jacobian storage, and geometric measures.
Extend the same contract to other embedded shapes only after it is proven on
bars.

This is an implementation plan, not a broad topology/basis/tiling
reorganization. The responsibility boundaries discussed in
[`elem-responsibility-refactoring-2026-11-11.md`](./elem-responsibility-refactoring-2026-11-11.md)
can be revisited independently.

## Current state and what is already in place

- [`Elem<T, P, F>`](../../src/femera/grid/Elem.hpp) exposes `get_elem_d()`,
  while its gradient-size helper accepts a dimension argument defaulting to
  three. Its Jacobian size helpers currently assume square matrices
  (`sims_d * sims_d`) and do not account for the element's reference
  dimension. `get_elem_node_coor` intentionally emits three coordinates per
  node.
- `Bars` already describes a one-dimensional reference element
  (`elem_d == 1`) with canonical vertices at `(0,0,0)` and `(1,0,0)`. Its
  geometry-node construction remains three-coordinate storage.
- Shape evaluation and Jacobian multiplication are already outside the
  `Bars`/`Elem` implementation: bar basis functions are in
  [`elem-poly.hpp`](../../src/fmr/fems/elem-poly.hpp) and
  [`elem-poly.ipp`](../../src/fmr/fems/elem-poly.ipp); three Jacobian kernels
  are in [`elem-jacs.hpp`](../../src/fmr/fems/elem-jacs.hpp) and
  [`elem-jacs.ipp`](../../src/fmr/fems/elem-jacs.ipp). The kernels currently
  use `sims_d` for both Jacobian axes and gradient stride, so they calculate
  only square matrices.
- [`Bars.gtst.cpp`](../../src/femera/grid/elem/Bars.gtst.cpp) documents the
  unresolved 3-component gradient sizes. The generic Jacobian test in
  [`elem-jacs.gtst.cpp`](../../src/fmr/fems/elem-jacs.gtst.cpp) checks
  agreement among storage variants, but uses synthetic square gradients and
  does not verify a physical embedded-bar Jacobian.
- The two earlier research notes already identify rectangular Jacobians,
  explicit layouts, metric-based measures, and focused embedded-element
  tests. Retain those requirements, but do not assume their proposed
  architectural extraction is needed for this correction.

## Proposed contract

For an element with `r = elem_d` reference coordinates and `s = sims_d`
physical coordinates:

- Store/evaluate reference gradients with `r` components per node and
  integration point: `[ip][node][reference-coordinate]` for the canonical
  node-major layout.
- Calculate a row-major rectangular Jacobian with shape `s x r`:
  `J[a,r_i] = sum_k X[k,a] * dN[k,r_i]`. Use `s` only for physical
  coordinate indexing and `r` only for reference-gradient indexing.
- Store `intp_n * s * r` Jacobian entries. If `jacd_size` continues to mean
  Jacobian entries plus one scalar per point, size it as
  `intp_n * (s * r + 1)`; document that scalar's meaning and do not imply an
  ordinary determinant exists for rectangular maps.
- Obtain ambient dimension from the physical geometry/caller, not from
  `Bars` topology. `Elem` may supply reference topology and size metadata,
  but must not pin a bar's ambient dimension.
- For square maps, preserve the signed determinant separately from the
  nonnegative integration measure. For full-column-rank embedded maps,
  compute the measure as `sqrt(det(J^T J))`. Do not add a rectangular
  inverse/determinant shortcut. Decide and test explicit behavior for
  degenerate maps before exposing measure results.
- Keep coordinate and gradient storage layouts explicit. The existing
  AoS/SoA coordinate kernels and the dimension-major gradient variant must
  all use the same `(s,r)` contract, or the unused variant should be removed
  only after confirming it has no needed callers. Avoid silently preserving
  three-component padding in the new reference-gradient contract.

## Implementation sequence

1. **Settle the API and indexing contract.**
   Inventory all callers of `Elem` size helpers and the three
   `elem_jacs_*` variants. Choose whether reference dimension comes from
   `Elem::get_elem_d()` or an explicit kernel parameter, and make ambient
   dimension explicit at the mapping call. Document argument sizes and
   `[ip][row][column]` / coordinate layouts. Validate supported dimensions
   and buffer-size arithmetic consistently with existing error conventions.
2. **Make bar gradients reference-dimensional.**
   Update P1, P2, and P3 bar gradient evaluators to represent derivatives
   with respect to the single bar coordinate, using their established
   endpoint-then-interior node order. Correct unused-direction derivatives
   rather than retaining padded values. Update `Elem` gradient sizing only
   where it is genuinely shared; for a bar, the expected gradient count is
   `intp_n * node_n * 1`, not `* 3`.
3. **Generalize the geometry kernels to rectangular maps.**
   Update each used `elem_jacs_*` kernel to accept or derive both dimensions
   and compute exactly `intp_n * s * r` entries. Preserve AoS/SoA and
   gradient-layout distinctions where they are useful, and explicitly
   document each indexing formula. Adjust `Elem::jacs_size` and
   `Elem::jacd_size` (or replace them with a clearer shared helper) to use
   reference and ambient dimensions.
4. **Add correct embedded measures only where required by callers.**
   First locate current determinant/measure consumers; the current Jacobian
   kernel implementation only multiplies coordinates and gradients. If
   measure conversion is in scope for those consumers, implement square
   determinant handling and embedded metric scaling as separate operations,
   with defined singular/degenerate behavior. Do not expand this step into
   mixed-dimensional stiffness or global degree-of-freedom assembly.
5. **Extend beyond bars after the contract is stable.**
   Verify that the API can represent a two-dimensional reference surface in
   three dimensions (`3 x 2`) without changing bar-specific assumptions.
   Then update applicable surface-element callers/tests. Avoid changing
   unrelated tiling or element ownership code.

## Verification

- **Basis gradients:** for P1-P3, check analytic derivatives at multiple
  interior points against finite differences; check derivative sums are zero,
  only the one reference component is present, and node ordering is preserved.
- **Canonical bar Jacobians:** using reference endpoints `[0,1]`, verify
  `1 x 1` `[1]` in 1D, `2 x 1` `[1,0]^T` in 2D, and `3 x 1`
  `[1,0,0]^T` in 3D.
- **Geometry variations:** verify translation leaves `J` unchanged and
  rotation/scaling produce the expected column and measure.
- **Storage and strides:** verify exact compact gradient/Jacobian sizes,
  multiple integration-point offsets, and equivalent results for every
  supported coordinate/gradient layout. Use guard values to detect writes
  beyond compact buffers.
- **Regression:** retain a square 3D solid Jacobian test; add a `3 x 2`
  surface-in-3D test when the generic kernel is used for that case. Test
  measure separately for square and embedded maps if implemented.
- Run the focused `Bars`, `Elem`, and `fmr::fems::elem` tests, then the
  relevant build/test targets. Do not treat agreement between kernel variants
  as the only oracle; include hand-computed expected matrices.

## Out of scope

- Replacing `Elem` wholesale with topology, basis, and tiling policies.
- Redesigning structured bar tiling, high-order node placement, or
  `get_elem_node_coor`'s three-coordinate convention unless required to pass
  the dimension-aware mapping contract.
- Mixed truss/solid assembly, displacement-degree mapping, or constitutive
  behavior. Correct embedded geometry is necessary but does not by itself
  make a bar mechanically compatible with a solid model.
