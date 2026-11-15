# Femera finite element node numbering conventions

- Each element contains a reference first-order element defining the vertices: `vert_coor` and its transpose `coor_vert`.
- Each element contains edge vertex connectivity in `bars_vert_conn`.
- 2D and 3D elements contain face vertext connectivity in `tris_vert_conn` and `quad_vert_conn`. (These are `nullptr` when there are no faces of the corresponding shape.)
- All 3D element faces define their local node winding in a counter-clockwise (CCW) order when viewed from outside the element for outward-pointing face normals.

**Node numbering**

- First, vertex nodes.
- Then, if present, one or more edge nodes, in order of the edges defined in `bars_vert_conn`.
- Then, if present, face nodes, using `tris_vert_conn` followed by `quad_vert_conn`.

