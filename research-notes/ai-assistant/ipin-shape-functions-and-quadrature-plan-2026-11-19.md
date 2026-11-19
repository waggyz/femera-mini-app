# Plan: Implement `Ipin` Shape Functions and Quadrature

## Goal

Give the two-node P1 `Ipin` element evaluable shape functions, reference gradients, and a one-point integration rule that support the axis-wise spring stiffness described in [`Ipin.hpp`](../../src/femera/grid/elem/Ipin.hpp). Preserve the distinction between this discrete coordinate-wise spring model and a geometry-oriented truss/bar formulation.

## Current state

- `Ipin` has `elem_d == 1`, two vertices, `Poly::None`, and `edge_n == 0`. Its current tests expect zero shape/gradient output sizes and a `false` result from all three polynomial evaluation methods.
- `Elem::get_poly_func_size()` and `get_poly_grad_size()` return zero when the element has no edges, faces, or volumes. Marking `Ipin` polynomial without describing its segment would therefore still leave the evaluation API disabled.
- Polynomial evaluation dispatch is specialized by topology. `Bars.ipp` already dispatches P1 values and both gradient layouts to the `C1D2N` routines in `elem-poly.ipp`; `Ipin` currently has no dispatch specialization.
- The existing 1D reference interval and node order are `[0,1]`, with node 0 at `xi=0` and node 1 at `xi=1`. The P1 basis is therefore `N0 = 1 - xi`, `N1 = xi`, and `dN/dxi = [-1, +1]`.
- `Tets.hpp` is the current in-tree example of element-local integration point/weight arrays. No shared `Elem` quadrature accessor is visible in the current contract.
- The comment's stiffness matrix has three uncoupled coordinate blocks, with `kx`, `ky`, and `kz` on each block. It does not describe a rotation-dependent axial truss constitutive law.

## Proposed behavior

- Expose the P1 polynomial basis through the existing `Elem<Ipin>` shape and gradient API. For `xi` in `[0,1]`, return `[1-xi, xi]`; return `[-1,1]` for either gradient layout. Since the reference dimension is one, both layouts contain two values in the same order.
- Declare one segment edge with endpoint connectivity `{0,1}` so `Elem` recognizes a one-dimensional integration entity and its shape/gradient size helpers become active. Keep support explicitly P1; do not imply that `Elem<Ipin,2>` or `Elem<Ipin,3>` is implemented merely because `Elem` has a general order parameter.
- Add one Gauss point at `xi=0.5` with natural-domain weight `1.0`. The weight sums to the reference interval length, one, and is exact for the constant P1 stiffness integrand in the comment. Do not multiply this weight by physical segment length for the stated discrete spring relation. If a later API defines `k` per unit physical length, that scaling belongs in that formulation.
- Preserve the stated coordinate-wise matrix: applying `B = [-1,1]` independently to each displacement component and diagonal response `diag(kx,ky,kz)` yields the three uncoupled `[[k,-k],[-k,k]]` blocks. Shape evaluation and quadrature supply interpolation/integration; they do not themselves define assembly or the spring constants.

## Implementation sequence

1. **Enable the one-dimensional segment topology.** In `Ipin.hpp`, change the polynomial type to the supported full P1 line basis, set the segment edge count to one, and provide endpoint connectivity `{0,1}`. Keep the two vertices and reference dimension unchanged. Review `edge_l`, default coordinate arrays, and any edge-metadata consumers before assigning new values; `Ipin`'s physical node coordinates are supplied by the containing mesh and should not be inferred from its all-zero placeholder coordinates.
2. **Add P1 polynomial dispatch.** In `Ipin.ipp`, specialize `detail::poly_dispatch<Ipin,1>` and route values, node-major gradients, and reference-major gradients to `calc_poly_func_C1D2N`, `calc_poly_grad_node_C1D2N`, and `calc_poly_node_grad_C1D2N`. Do not add P2/P3 dispatch or advertise higher-order support.
3. **Declare the integration rule.** Add a one-point count and packed point/weight array following the existing `Tets.hpp` naming/layout convention: `[xi, weight] = [0.5, 1.0]`. Keep the data as reference-domain quadrature and note that it is not physical-length-scaled.
4. **Replace placeholder tests.** In `Ipin.gtst.cpp`, assert the active P1 type and nonzero sizes, test values at both endpoints and the midpoint, test both gradient layouts and their node ordering, and check the integration point and weight. Add a hand-computed check that `B^T diag(kx,ky,kz) B` has the three expected uncoupled node-pair blocks; keep this narrowly scoped to the behavior described by the header comment.
5. **Validate compatibility.** Confirm the edge declaration does not unintentionally change geometry helpers or existing metadata assumptions for `Ipin`. Ensure invalid polynomial orders are not presented as supported, and retain tests for `Pins` and `Bars` as regressions for shared `Elem` behavior.

## Verification criteria

- For `xi = 0`, `0.5`, and `1`, values are respectively `[1,0]`, `[0.5,0.5]`, and `[0,1]`; values sum to one.
- Both gradient layouts return `[-1,1]`, independent of `xi`; gradients sum to zero. With one reference coordinate, each output size is `node_n * elem_d == 2`.
- The one-point rule is exactly `[0.5,1.0]` and its natural weight sum is one.
- The stiffness calculation gives `kx`, `ky`, and `kz` diagonal 2-by-2 blocks with negative off-diagonal terms and no cross-component coupling, matching the comment.
- The existing `Pins` no-polynomial behavior remains unchanged. Run the focused `Ipin` and `Pins` tests and the relevant element build/test target.

## Modeling boundary

The comment's matrix represents independent spring response along global coordinate axes. A physical line/truss element whose stiffness follows the segment orientation would need a separate constitutive/coordinate transformation and a clear definition of whether its stiffness parameter is total spring stiffness or stiffness per unit length. Do not fold that modeling change into this shape-function and quadrature implementation.

## Follow-on questions: higher-order edges and cohesive interfaces

### Higher-order `Ipin` used on structural edges

The P1 matrix is a two-node spring matrix. Extending an edge element to P2 or P3 changes its nodal basis and stiffness matrix; it does not mean copying the same 2-by-2 block to additional nodes. `Elem` already derives three- and four-node counts from one edge for P2/P3, and the existing `Bars` dispatch provides the corresponding line basis functions and gradients. A future higher-order `Ipin` path could reuse those basis kernels, but must first settle what `Ipin` models:

- **Coordinate-wise spring field:** retain independent response in global x/y/z components. Assemble `K_ab = integral (dN_a/dxi) D (dN_b/dxi) w dxi`, with diagonal `D = diag(kx,ky,kz)` only if the material parameters and integration measure are defined consistently. This reproduces the P1 blocks under the appropriate normalization, but is not an orientation-independent axial truss law.
- **Axial structural edge/truss:** compute the physical tangent from the geometry mapping, project displacement gradients onto that tangent, and define an axial constitutive parameter such as `EA`. This gives stiffness coupled across spatial components for a non-axis-aligned edge and is a different formulation from the current displayed matrix. A curved higher-order edge additionally has a varying tangent and line measure.

For a straight, linearly mapped edge with constant material data, the P2 stiffness integrand is degree two in the reference coordinate and needs at least two Gauss points for exact integration; P3 gives degree four and needs at least three. These counts apply to the standard derivative-product term. Curved isoparametric geometry can make the mapped integrand non-polynomial, so quadrature order should be selected or convergence-tested for the actual formulation rather than inferred from the shape order alone. A later P2/P3 implementation should test polynomial reproduction, constant-strain behavior where applicable, rigid-body modes, symmetry, nonnegative energy, and convergence with quadrature order.

Keep P2/P3 out of the initial P1 implementation. When added, expose and test each supported order explicitly, add the order-appropriate quadrature data, and avoid treating the one-point P1 rule as sufficient for higher-order stiffness integration.

### `Ibar`, `Itri`, and `Iqud` cohesive interfaces

The higher-dimensional interface elements share a useful interpolation pattern with an edge element: evaluate a parent-face basis at each integration point and use it to interpolate fields. Their topologies are nevertheless distinct. `Ibar` has four vertices arranged as two paired line sides in 2D; `Itri` has six vertices arranged as two triangular sides in 3D; `Iqud` has eight vertices arranged as two quadrilateral sides in 3D. Their natural dimensions are respectively one, two, and two, and their existing connectivity encodes the two sides with opposite orientation.

A cohesive implementation should evaluate corresponding-side shape functions, interpolate the displacement jump (plus-side field minus minus-side field), transform that jump to local normal/tangential components as required by the traction-separation law, and integrate the resulting traction/tangent over the interface's physical line or area measure. The basis may be shared or reused from `Bars`, `Tris`, and `Quad`; pairing, side orientation, jump assembly, constitutive response, and physical integration measure remain interface-specific. Higher-order cohesive elements also need consistent matching node order on both sides and sufficient quadrature for both the basis and the traction law.

These elements do **not** reduce directly to the current `Ipin` topology or matrix. `Ipin` has two nodes and represents one pair of connected points with vector displacement DOFs; a cohesive line in 2D has two nodes on each side (four total), while cohesive surfaces have paired triangles or quadrilaterals. A reduction can share a scalar basis or arise as a deliberate limit after integrating out interface dimensions and specifying the constitutive scaling, but the result is a lumped connector only after that modeling operation. It is not an automatic consequence of taking the P1 shape functions of `Ibar`, `Itri`, or `Iqud`.

Before claiming one family generalizes or reduces to another, specify whether `Ipin` means an edge/truss member, a point connector, or a zero-thickness cohesive law; define its physical dimension, node pairing, local frame, and stiffness/traction units. The present P1 basis-and-quadrature task can proceed independently, but should not encode any of these unresolved constitutive choices.
