#ifndef FEMERA_HAS_GRID_TILE_BAR5_HPP
#define FEMERA_HAS_GRID_TILE_BAR5_HPP

#include "../Tile.hpp"

namespace femera { namespace grid { namespace tile {

class Bar5 final : public Tile<Bar5> {
  template <class T, fmr::Dim_int P, typename F>
  friend class Tile;
  /*
  This class tiles a 2D space with 1D bar elements with rectangular
  5-bar truss structures. The natural tile is square [0.0, 1.0] in the 2D plane.
  Vertex nodes match the 2D Quad element:
  
       3 --- 2       3 --- 2
       |   / |       | \   |
       |  /  |       |  \  |
       | /   |       |   \ |
       0 --- 1       0 --- 1
      Canonical     Alternate
  */
private:
// Scalars ====================================================================
  static constexpr fmr::Dim_int        elem_d = 1;
  static constexpr fmr::Dim_int        tile_d = 2;
  static constexpr fmr::Local_int tile_elem_n = 5;// # this type element / tile
  //
  static constexpr fmr::Local_int tile_vert_n = 4;
  static constexpr fmr::Local_int tile_edge_n = 5;
  static constexpr fmr::Local_int tile_tris_n = 0;
  static constexpr fmr::Local_int tile_quad_n = 0;
  static constexpr fmr::Local_int tile_vols_n = 0;
  //
  static constexpr fmr::Local_int tile_alts_n = 2;
  //
  // Tiling =====================================================================
  static constexpr fmr::Local_int
  bars_vert_conn [Bar5::tile_alts_n * Bar5::tile_edge_n * 2u]
   = { 0,1, 1,2, 2,3, 3,0, 0,2,// Canonical tile
       0,1, 1,2, 2,3, 3,0, 1,3 // Alternate //TODO last can be: 3,1
  };
  static constexpr fmr::Local_int* tris_vert_conn = nullptr;
  static constexpr fmr::Local_int* quad_vert_conn = nullptr;
 /*
  Tiling variants:
  0 Standard (canonical) tiling: consecutive elements along the bar (left to right) with
    the diagonal element from bottom left to top right.
  1 Reversed tiling: consecutive elements along the bar (left to right) with
    the diagonal element from bottom right to top left.
  2 Alternating tiling: consecutive elements along the bar, alternating between
    standard and reversed connectivity.
  3 Alternating like 2, but starting with the alternate, instead of canaonical,
    tile.

  - These are not particularly useful for modelling.
  - They are useful for prototyping and testing tiling variants in higher dimensions.
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
};//end class Bar5

} } }//end femera::grid::tile:: namespace

#include "Bar5.ipp"

#endif
//end FEMERA_HAS_GRID_TILE_BAR5_HPP
