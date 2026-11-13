#ifndef FEMERA_HAS_GRID_TILE_HPP
#define FEMERA_HAS_GRID_TILE_HPP

#include "../../fmr/fmr.hpp"

namespace femera { namespace grid { namespace tile {//TODO grid -> geom
//namespace femera { namespace geom { //TODO change to here?
#if 0
// Forward-delare element shapes TODO is this needed?
//class Pins;//TODO point element for "soft" pins & arbitrary point forces?
class Bars;
//class Ipts;//TODO 1D interface element embedded in 1D, 2D, 3D space
class Ibar;// 1D interface element embedded in 2D or 3D space
class Itri; class Iqud;// 2D interface elements embedded in 3D space
class Tris; class Quad;
class Tets; class Prmd; class Wdge; class Cube;
//class User;//TODO user element
#endif
template <class T, typename fmr::Dim_int P=1, typename F=fmr::Geom_float>
class Tile {
  /*
  All of the elements defined by Elem can tile a cartesian grid of the
  same dimension as the element dimesion, even interface elements.
  First-order (P1) element tiled grids are compatible with other regular
  cartesian grids, e.g., for finite difference (FD analysis).
  Some higher-order element tiled grids have "holes" that make them incompatible
  with regular cartesian grids.
  */
  public:
  // Simple constexpr accessors -----------------------------------------------
  // constexpr functions are implicitly inline

  static constexpr fmr::Dim_int   get_elem_d () noexcept { return T::elem_d; }
  static constexpr fmr::Dim_int   get_tile_d () noexcept { return T::tile_d; }

  // Structured grid element tiling ===========================================
  //
  static constexpr fmr::Local_int get_tile_elem_n () noexcept {
    return T::tile_elem_n;}
  static constexpr fmr::Local_int get_tile_vert_n () noexcept {
    return T::tile_vert_n;}
  static constexpr fmr::Local_int get_tile_edge_n () noexcept {
    return T::tile_edge_n;}
  static constexpr fmr::Local_int get_tile_face_n () noexcept {
    return T::tile_tris_n;}
  static constexpr fmr::Local_int get_tile_quad_n () noexcept {
    return T::tile_quad_n;}
  static constexpr fmr::Local_int get_tile_vols_n () noexcept {
    return T::tile_vols_n;}// 3D elements of this type (assumes all same)
  //
  static constexpr fmr::Local_int get_tile_elem_node_n () noexcept;

  static constexpr fmr::Local_int get_tile_part_elem_n (
    const fmr::Local_int* part_elem_dims)// size 3 for 1D,2D,3D elements
  noexcept {
  //NOTE only [0] is used for 1D elements like Bars,
  //     for 2D elements only [0] and [1] are used,
  //     for 3D elements all three are used.
  //
    return T::tile_elem_n
    * ((part_elem_dims[0] > 0 && T::elem_d > 0) ? part_elem_dims[0] : 0)
    * ((part_elem_dims[1] > 0 && T::elem_d > 1) ? part_elem_dims[1] : 1)
    * ((part_elem_dims[2] > 0 && T::elem_d > 2) ? part_elem_dims[2] : 1);
  }
  static constexpr fmr::Local_int get_tile_part_node_n (
    const fmr::Local_int* part_elem_dims)
  noexcept {
    return T::calc_tile_part_node_n(P, part_elem_dims);
  }
  // Tiled (strucured mesh) node coordinates
  static constexpr bool get_tile_part_node_coor (F* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int* part_elem_dims,// partition dimensions nodes_x,node_y,node_z for 1D,2D,3D elems
    const F* tile_size,
    const F* part_orig = nullptr// default is {0.0,0.0,0.0}
  ) noexcept {
    return T::calc_tile_part_node_coor (
      coor, node_i, P, part_elem_dims, tile_size, part_orig);
  }
  //NOTE node_coor, part_node_dims, tile_size, part_orig bounds checked by caller.
  // Orientation of the tile is specified by tile_dirs,
  // which changes where mesh "holes" are when tiling by, e.g., Wdge elements.
  //
  // P1 FE meshes are the same as FD grids, regardless of element type.
#if 0
  constexpr bool get_tile_elem_coor_node (F* node_coor,
    const fmr::Local_int elem_i);
#endif
#if 0
  constexpr bool get_tile_elem_node_coor(fmr::Geom_float* node_coor,
    const fmr::Local_int node_i,
    const fmr::Local_int node_n,
    const fmr::Geom_float* tile_size,
    const fmr::Geom_float* part_orig = {0.0,0.0,0.0})// origin should default to {0.0,0.0,0.0}
  //NOTE node_coor, part_orig bounds should be checked by the caller.
  //TODO consistent and periodically alternating orientations .
    noexcept;
#endif
  // Tile operations dispatch to T's corresponding static calc_* method.
  // Shapes without an implementation fail at compile time when called.
  // Connectivity implementations receive p (defaulting to P), allowing
  // shape-specific behavior for each order without templating the shape.
  //TODO how to handle when grid_size not evenly divisible by part_size?
  static constexpr bool get_tile_vert_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Dim_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_vert_conn (conn, tile_i, part_size, p);
    }
  static constexpr bool get_tile_vert_conn_prdc (fmr::Local_int* conn,// Periodic BCs
    //NOTE This is for a periodic partition, not an overall periodic grid.
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Dim_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_vert_conn_prdc (conn, tile_i, part_size, p);
    }
  //
  static constexpr bool get_tile_node_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,
    const fmr::Local_int* part_size,
    const fmr::Dim_int p=P)// {x}, {x,y}, {x,y,z} for 1D,2D,3D elems
    //NOTE conn, part_size bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_node_conn (conn, tile_i, part_size, p);
    }
  //
  static constexpr bool get_tile_vert_coor(F* vert_coor,
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE node_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_vert_coor (vert_coor, vert_i, vert_n, tile_size, part_orig);
    }
  //
  static constexpr bool get_tile_elem_vert_coor(//TODO need: 
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, elem_size, part_orig bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_elem_vert_coor (vert_coor, elem_i, elem_n, tile_size, part_orig);
    }
  static constexpr bool get_tile_elem_coor_vert(
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    //NOTE vert_coor, tile_size, part_orig bounds should be checked by the caller.
    noexcept {
      return T::calc_tile_elem_coor_vert (vert_coor, elem_i, elem_n, tile_size, part_orig);
    }
};//end Elem

} } }//end femera::grid::tile:: namespace

#include "Tile.ipp"

#endif
//end FEMERA_HAS_GRID_TILE_HPP

