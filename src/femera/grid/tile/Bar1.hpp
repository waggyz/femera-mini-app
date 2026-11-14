#ifndef FEMERA_HAS_GRID_TILE_BAR1_HPP
#define FEMERA_HAS_GRID_TILE_BAR1_HPP

#include "../Tile.hpp"

namespace femera { namespace grid { namespace tile {

class Bar1 final : public Tile<Bar1> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Tile;
private:
// Scalars ====================================================================
  static constexpr fmr::Dim_int        elem_d = 1;
  static constexpr fmr::Dim_int        tile_d = 1;
  static constexpr fmr::Local_int tile_elem_n = 1;// # this type element / tile
  //
  static constexpr fmr::Local_int tile_vert_n = 2;
  static constexpr fmr::Local_int tile_edge_n = 1;
  static constexpr fmr::Local_int tile_tris_n = 0;
  static constexpr fmr::Local_int tile_quad_n = 0;
  static constexpr fmr::Local_int tile_vols_n = 0;

  static constexpr fmr::Local_int tile_alts_n = 2;

  static constexpr fmr::Local_int
  bars_vert_conn [Bar1::tile_alts_n * Bar1::tile_edge_n * 2u] 
  = { 0,1, // Canonical tile
      1,0  // Alternate
    };
  static constexpr fmr::Local_int* tris_vert_conn = nullptr;
  static constexpr fmr::Local_int* quad_vert_conn = nullptr;
  //
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

  Alternate connectivity (reversed Bars, prototype for higher dimensions)
  - For P1, vert_conn sequence: {1,0}, {2,1}, {3,2},...
  - For P2, vert_conn sequence: {2,0}, {4,2}, {6,4},...
  - For P3, vert_conn sequence: {3,0}, {6,3}, {9,6},...

  - For P1, node conn sequence: {1,0}, {2,1}, {3,2},...
  - For P2, node conn sequence: {2,0,1}, {4,2,3}, {6,4,5},...
  - For P3, node conn sequence: {3,0,2,1}, {6,3,5,4}, {9,6,8,7},...

  Tiling variants:
  0 Standard tiling: consecutive elements along the bar (left to right).
  1 Reversed tiling: consecutive elements along the bar with reversed
    connectivity (right to left).
  2 Alternating tiling: consecutive elements along the bar with alternate
    (reversed) connectivity.
  3 Alternating like 2, but starting with the alternate, instead of canonical,
    tile.

  - These are not particularly useful for modelling.
  - They are useful for prototyping tiling variants in higher dimensions.
  */
  static constexpr fmr::Local_int calc_tile_part_node_n (
    const fmr::Dim_int elem_p, const fmr::Local_int* part_elem_dims)
  noexcept{ return part_elem_dims[0] * elem_p  + 1;
  }
  static constexpr bool calc_tile_vert_conn (fmr::Local_int conn[2],
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p)
  noexcept;
  static constexpr bool calc_tile_vert_conn_prdc (fmr::Local_int conn[2],
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p)
  noexcept;
  static constexpr bool calc_tile_node_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p) noexcept;
  //
  // Vertex coordinates -------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_vert_coor (F coor[3],
    fmr::Local_int vert_i, fmr::Local_int vert_n,
    const F tile_size[3], const F part_orig[3]) noexcept;
  //
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_vert_coor (F coor[6],
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3]) noexcept;
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_coor_vert (F coor[6],
    fmr::Local_int elem_i, fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3]) noexcept;
  //
  // Node coordinates ---------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_part_node_coor (F* coor,
    const fmr::Local_int node_i,
    const fmr::Dim_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F tile_size[3], const F part_orig[3]
  ) noexcept;
  //
};//end class Bar1

} } }//end femera::grid::tile:: namespace

#include "Bar1.ipp"

#endif
//end FEMERA_HAS_GRID_TILE_BAR1_HPP
