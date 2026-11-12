#ifndef FEMERA_HAS_GRID_TILE_BARS_HPP
#define FEMERA_HAS_GRID_TILE_BARS_HPP

#include "../Tile.hpp"

namespace femera { namespace grid { namespace tile {

class Bars final : public Tile<Bars> {
  template <class T, fmr::Local_int P, typename F>
  friend class Tile;
private:
// Scalars ====================================================================
#if 1
  static constexpr fmr::Local_int sims_d = 3;//TODO get from Elem<Bars>?
  static constexpr fmr::Local_int elem_d = 1;
#endif
  static constexpr fmr::Local_int tile_elem_n = 1;// # this type element / tile
  
  static constexpr fmr::Local_int tile_vert_n = 2;
  static constexpr fmr::Local_int tile_edge_n = 1;
  static constexpr fmr::Local_int tile_tris_n = 0;
  static constexpr fmr::Local_int tile_quad_n = 0;
  static constexpr fmr::Local_int tile_vols_n = 0;

  // Tiling =====================================================================
  // Bars are self-tiling (no other element types involved).
  /*
  Vertex connectivity:
  - For P1, vert_conn sequence: {0,1}, {1,2}, {2,3},...
  - For P2, vert_conn sequence: {0,2}, {2,4}, {4,6},...
  - For P3, vert_conn sequence: {0,3}, {3,6}, {6,9},...

  Node connectivity:
  - For P1, node conn sequence: {0,1}, {1,2}, {2,3},...
  - For P2, node conn sequence: {0,2,1}, {2,4,3}, {4,6,5},...
  - For P3, node conn sequence: {0,3,1,2}, {3,6,4,5}, {6,9,7,8},...
  */
  static constexpr fmr::Local_int calc_tile_part_node_n (
    const fmr::Local_int elem_p, const fmr::Local_int* part_elem_dims)
  noexcept{ return part_elem_dims[0] * elem_p  + 1;
  }
  static constexpr bool calc_tile_vert_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p)
    noexcept;
  static constexpr bool calc_tile_vert_conn_prdc (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p)
    noexcept;
  static constexpr bool calc_tile_node_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Local_int p) noexcept;
  //
  // Vertex coordinates -------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_vert_coor (F* coor,
    fmr::Local_int vert_i, fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig) noexcept;
  //
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_vert_coor (F* coor,
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig) noexcept;
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_coor_vert (F* coor,
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig) noexcept;
  //
  // Node coordinates ---------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_part_node_coor (F* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F* tile_size, const F* part_orig
  ) noexcept;
  //
};//end class Bars

} } }//end femera::grid::tile:: namespace

#include "Bars.ipp"

#endif
//end FEMERA_HAS_GRID_TILE_BARS_HPP
