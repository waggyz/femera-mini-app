# Embedding Lower-Dimensional Elements

## Current issue

`Elem` exposes both `get_elem_d()` and `get_sims_d()`, but the Jacobian routines use `sims_d` for both matrix dimensions and for the shape-gradient stride. That assumes the element dimension equals the simulation dimension.

For an element with reference dimension $r = \text{elem\_d}$ embedded in a space of dimension $s = \text{sims\_d}$, the Jacobian is generally rectangular:

$$
J_{a i} = \sum_{k=0}^{n-1} X_{k a}\,\frac{\partial N_k}{\partial \xi_i},
\qquad J \in \mathbb{R}^{s \times r}
$$

Here, $X$ contains node coordinates, $n$ is the node count, $a$ indexes ambient coordinates, and $i$ indexes reference coordinates.

For `Bars`, $r=1$. With natural node positions `[0, 1]`, the Jacobian is `[1]` in 1D, `[1, 0]^T` in 2D, or `[1, 0, 0]^T` in 3D. Only the 1D Jacobian is an identity matrix; expecting a 3×3 identity for an embedded bar is incorrect.

## Recommended generic design

- Keep reference dimension and ambient dimension distinct throughout the API.
- Compute/store shape-function gradients with `elem_d` components per node, not `sims_d`. For a linear bar, the gradient buffer is `node_n × 1`, containing `[-1, 1]`.
- Compute Jacobians with `sims_d` rows and `elem_d` columns. The loops, output size, and indexing must use both dimensions independently.
- Define array layouts explicitly. For example, use node-major coordinates `node_n × sims_d`, node-major reference gradients `node_n × elem_d`, and row-major Jacobians `sims_d × elem_d`.
- Make the ambient dimension available to the element evaluation from the mesh or simulation context, or as an explicit `Elem` parameter. `Bars::sims_d` is currently fixed at 3, so that static value cannot represent the same bar used in both 2D and 3D without separating its topology/reference basis from its embedding coordinates.
- Validate `elem_d <= sims_d` and ensure coordinate and gradient buffer sizes match their documented layouts.

The current Jacobian size helper uses `this_d * this_d`; an embedded Jacobian needs `sims_d * elem_d` entries per integration point.

## Measures and inverse operations

For square Jacobians (`sims_d == elem_d`), the determinant is the usual signed volume/area/length scaling. For embedded elements, use the metric tensor:

$$
G = J^T J,\qquad \text{measure scale} = \sqrt{\det(G)}
$$

This yields length scaling for a bar and area scaling for a surface in 3D. Do not apply a square-matrix inverse or determinant directly to a rectangular Jacobian; use a suitable pseudo-inverse or metric-based formulation when needed.

## Mixed-dimensional assembly

Embedding geometry alone does not make a truss or spring compatible with a solid model. Its global degrees of freedom, local stiffness/residual formulation, and assembly mapping must use the same ambient displacement components and node numbering conventions as the solid. A truss contributes along its current axis; a spring may instead constrain relative displacement according to its specified direction or law.

## Suggested tests

- Check canonical Jacobians for a bar embedded in 1D, 2D, and 3D: respectively `1×1`, `2×1`, and `3×1`.
- Check a 2D surface embedded in 3D produces a `3×2` Jacobian.
- Retain a 3D solid test for a `3×3` identity Jacobian.
- Check Jacobian storage sizes and multi-integration-point strides for rectangular cases.
- Check metric-based measure scaling and a mixed truss/solid assembly case.

Relevant implementation surfaces: `Elem.hpp`, `Elem.ipp`, `Bars.hpp`, and `Bars.ipp`.