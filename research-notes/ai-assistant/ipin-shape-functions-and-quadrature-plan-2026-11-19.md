# Plan: Implement `Ipin` Shape Functions and Quadrature

## Goal

Give the two-node P1 `Ipin` element evaluable shape functions, reference gradients, and reference-domain quadrature. This module represents geometry only; constitutive response, traction laws, and physical stiffness belong in the Physics module.

## Current state

- `Ipin` has `elem_d == 1`, two vertices, `Poly::None`, and `edge_n == 0`. Its current tests expect zero shape/gradient output sizes and a `false` result from all three polynomial evaluation methods.
- `Elem::get_poly_func_size()` and `get_poly_grad_size()` return zero when the element has no edges, faces, or volumes. Marking `Ipin` polynomial without describing its segment would therefore still leave the evaluation API disabled.
- Polynomial evaluation dispatch is specialized by topology. `Bars.ipp` already dispatches P1 values and both gradient layouts to the `C1D2N` routines in `elem-poly.ipp`; `Ipin` currently has no dispatch specialization.
- The existing 1D reference interval and node order are `[0,1]`, with node 0 at `xi=0` and node 1 at `xi=1`. The P1 basis is therefore `N0 = 1 - xi`, `N1 = xi`, and `dN/dxi = [-1, +1]`.
- `Tets.hpp` is the current in-tree example of element-local integration point/weight arrays. No shared `Elem` quadrature accessor is visible in the current contract.
- `Ipin.hpp` contains a speculative stiffness example and open modeling questions. They are Physics-module concerns and do not define the geometry API implemented by this plan.

## Proposed behavior

- Expose the P1 polynomial basis through the existing `Elem<Ipin>` shape and gradient API. For `xi` in `[0,1]`, return `[1-xi, xi]`; return `[-1,1]` for either gradient layout. Since the reference dimension is one, both layouts contain two values in the same order.
- Declare one segment edge with endpoint connectivity `{0,1}` so `Elem` recognizes a one-dimensional integration entity and its shape/gradient size helpers become active. Keep support explicitly P1; do not imply that `Elem<Ipin,2>` or `Elem<Ipin,3>` is implemented merely because `Elem` has a general order parameter.
- Add one Gauss point at `xi=0.5` with reference-domain weight `1.0`. The point and weight describe the reference interval only; the mapped physical measure is determined from element geometry by the consuming module. The rule is geometry data and makes no claim about exactness for any future physics integrand.

## Implementation sequence

1. **Enable the one-dimensional reference segment.** In `Ipin.hpp`, select the full P1 polynomial, set the edge count to one, and provide endpoint connectivity `{0,1}`. Give the canonical reference vertices coordinates 0 and 1 and set the reference edge length to 1; physical coordinates are supplied by the containing mesh.
2. **Add P1 polynomial dispatch.** In `Ipin.ipp`, specialize `detail::poly_dispatch<Ipin,1>` and route values, node-major gradients, and reference-major gradients to `calc_poly_func_C1D2N`, `calc_poly_grad_node_C1D2N`, and `calc_poly_node_grad_C1D2N`. Do not add P2/P3 dispatch or advertise higher-order support.
3. **Declare the integration rule.** Add a one-point count and packed point/weight array following the existing `Tets.hpp` naming/layout convention: `[xi, weight] = [0.5, 1.0]`. Keep the data as reference-domain quadrature and note that it is not physical-length-scaled.
4. **Replace placeholder tests.** In `Ipin.gtst.cpp`, assert the P1 type and segment metadata, test values at both endpoints and the midpoint, test both gradient layouts and node ordering, verify canonical coordinate layouts, and check the reference integration point and weight. Do not add a stiffness or constitutive-response test in this geometry module.
5. **Validate compatibility.** Confirm the edge declaration does not unintentionally change geometry helpers or existing metadata assumptions for `Ipin`. Ensure invalid polynomial orders are not presented as supported, and retain tests for `Pins` and `Bars` as regressions for shared `Elem` behavior.

## Verification criteria

- For `xi = 0`, `0.5`, and `1`, values are respectively `[1,0]`, `[0.5,0.5]`, and `[0,1]`; values sum to one.
- Both gradient layouts return `[-1,1]`, independent of `xi`; gradients sum to zero. With one reference coordinate, each output size is `node_n * elem_d == 2`.
- The one-point rule is exactly `[0.5,1.0]` and its natural weight sum is one.
- The reference vertices and endpoint connectivity describe the canonical segment `[0,1]`; the quadrature entry is `[0.5,1.0]`.
- The existing `Pins` no-polynomial behavior remains unchanged. Run the focused `Ipin` and `Pins` tests and the relevant element build/test target.

## Geometry/physics boundary

This implementation provides topology, interpolation, reference gradients, and reference quadrature only. The physical interpretation of `Ipin` and other elements is not defined here. The Physics module is responsible for interpreting mapped geometry, selecting physical measures, defining constitutive/traction-separation laws, and assembling their contributions.

## Follow-on questions: higher-order edges and cohesive interfaces

### Higher-order `Ipin` used on structural edges

`Elem` derives three- and four-node counts from one edge for P2/P3, and `Bars` provides corresponding line basis kernels. A future higher-order `Ipin` geometry can reuse those kernels if its node ordering and reference map match. Add higher-order evaluation only with explicit support in dispatch and tests for interpolation, gradients, node placement, and geometry mapping. Quadrature rules may be stored as reference geometry data, but their adequacy for stiffness or another response cannot be decided until the Physics integrand is defined. Keep P2/P3 out of this initial P1 implementation.

### `Ibar`, `Itri`, and `Iqud` cohesive interfaces

`Ibar` has four vertices arranged as two paired line sides in 2D; `Itri` has six vertices arranged as two paired triangular sides in 3D; `Iqud` has eight vertices arranged as two paired quadrilateral sides in 3D. Their natural dimensions are respectively one, two, and two, and their connectivity encodes the paired sides with opposite orientation. Geometry can provide side correspondence, basis evaluation, orientation, mapping, and reference quadrature. Interpolated displacement jumps, local traction frames, constitutive response, and physical integration measures belong to Physics.

The interface families do not reduce automatically to `Ipin`: their node pairing and parent domains differ. They may reuse compatible basis kernels, and Physics may derive a lower-dimensional connector under an explicitly defined reduction and scaling, but that is not a geometry-only identity. Keep cohesive response and dimensional-reduction tests in the Physics module; geometry tests here should cover only topology, interpolation, mapping, orientation, and quadrature.
