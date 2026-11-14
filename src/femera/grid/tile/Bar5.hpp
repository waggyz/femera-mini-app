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
  Partition edges are numbered once, in row-major horizontal, vertical, then
  diagonal blocks. Horizontal and vertical edges point along positive axes;
  diagonals point 0->2 (canonical) or 1->3 (alternate). Vertex node IDs
  are row-major; interior IDs follow in edge-major order, from endpoint 0.
  Variant 0 is canonical, 1 alternate, 2 checkerboard with canonical at
  even global tile parity, and 3 its complement. TilePattern provides the
  global tile offset. Invalid inputs return false without writing outputs.
  Periodic vertex connectivity identifies both opposite boundary pairs, with
  row-major nx*ny vertex IDs, independent of p. It does not merge partition
  edges or provide periodic higher-order node IDs. Odd periodic extents can
  have a non-alternating checkerboard seam.
  Coordinate calls use partition dimensions (not only a vertex/element count).
  Tiles lie in the XY plane: tile_size[0:2] scales x and y, tile_size[2] is
  unused, and a null origin is (0,0,0). Caller supplies valid buffers.
  */
  /* Node numbering for P1 and P2 grids match finite difference (FD) conventions.
     The nodes are numbered first along the x-direction, then along the
     y-direction. P3 grids are *not* compatible with FD because they have "holes"
     in the node layout. Here are examples for P1 and P2 2x2 element grids:
     
      6 --------- 7 --------- 8        20 -- 21 -- 22 -- 23 -- 24
      |  \        |        /  |         |  \  |  /  |  \  |  /  |
      |     \     |     /     |        15 -- 16 -- 17 -- 18 -- 19
      |        \  |  /        |         |  /  |  \  |  /  |  \  |
      3 --------- 4 --------- 5        10 -- 11 -- 12 -- 13 -- 14
      |        /  |  \        |         |  \  |  /  |  \  |  /  |
      |     /     |     \     |         5 --- 6 --- 7 --- 8 --- 9
      |  /        |        \  |         |  /  |  \  |  /  |  \  |
      0 --------- 1 --------- 2         0 --- 1 --- 2 --- 3 --- 4
         2-node Bars P1 grid               3-node Bars P2 grid
         same as 3x3 FD grid               same as 5x5 FD grid
   */
  // Partition dimensions count tiles. Counts require positive order and
  // dimensions small enough that all intermediate results fit in Local_int.
  static constexpr fmr::Local_int calc_tile_elem_node_n (
    const fmr::Dim_int elem_p) noexcept {
    return elem_p + 1;
  }
  static constexpr fmr::Local_int calc_tile_part_elem_n (
    const fmr::Local_int* dims) noexcept {
    const fmr::Local_int nx = dims[0], ny = dims[1];
    return nx == 0 || ny == 0 ? 0
      : nx * (ny + 1) + (nx + 1) * ny + nx * ny;
  }
  static constexpr fmr::Local_int calc_tile_part_node_n (
    const fmr::Dim_int elem_p, const fmr::Local_int* dims) noexcept {
    const fmr::Local_int nx = dims[0], ny = dims[1];
    return nx == 0 || ny == 0 ? 0
      : (nx + 1) * (ny + 1)
        + (elem_p - 1) * calc_tile_part_elem_n (dims);
  }
  static constexpr bool calc_tile_vert_conn (fmr::Local_int conn[2],
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p, TilePattern pattern = {0, 0, 0})
  noexcept;
  static constexpr bool calc_tile_vert_conn_prdc (fmr::Local_int conn[2],
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p, TilePattern pattern = {0, 0, 0})
  noexcept;
  static constexpr bool calc_tile_node_conn (fmr::Local_int* conn,
    fmr::Local_int tile_i, const fmr::Local_int* part_size,
    fmr::Dim_int elem_p, TilePattern pattern = {0, 0, 0}) noexcept;
  //
  // Vertex coordinates -------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_vert_coor (F coor[3],
    fmr::Local_int vert_i, const fmr::Local_int* part_size,
    const F tile_size[3], const F part_orig[3]) noexcept;
  //
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_vert_coor (F coor[6],
    fmr::Local_int elem_i, const fmr::Local_int* part_size,
    const F tile_size[3], const F part_orig[3], TilePattern pattern) noexcept;
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_elem_coor_vert (F coor[6],
    fmr::Local_int elem_i, const fmr::Local_int* part_size,
    const F tile_size[3], const F part_orig[3], TilePattern pattern) noexcept;
  //
  // Node coordinates ---------------------------------------------------------
  template <typename F=fmr::Geom_float>
  static constexpr bool calc_tile_part_node_coor (F* coor,
    const fmr::Local_int node_i,
    const fmr::Dim_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F tile_size[3], const F part_orig[3],
    TilePattern pattern = {0, 0, 0}
  ) noexcept;
  //
};//end class Bar5

} } }//end femera::grid::tile:: namespace

#include "Bar5.ipp"

#endif
//end FEMERA_HAS_GRID_TILE_BAR5_HPP
