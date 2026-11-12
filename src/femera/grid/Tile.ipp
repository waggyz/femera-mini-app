#ifndef FEMERA_HAS_GRID_TILE_IPP
#define FEMERA_HAS_GRID_TILE_IPP

namespace femera { namespace grid {

// Structured mesh tiling elements ============================================
template <class T, typename fmr::Local_int P, typename F>
 constexpr fmr::Local_int tile::Tile<T,P,F>::get_tile_elem_node_n () const noexcept {
  // Handles solid and interface elements
  return T::tile_vert_n// vertex nodes
    + (P -1) * T::tile_edge_n// edge nodes
    + (P==3  ? T::tile_vols_n * (T::tile_tris_n + T::tile_quad_n)// face nodes for P==3
    : 0);      // vols_n is 0 (1D,2D elems) or 1 (3D solid elems)
}

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_TILE_IPP
#endif
