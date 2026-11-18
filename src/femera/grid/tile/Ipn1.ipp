#ifndef FEMERA_HAS_GRID_TILE_IPN1_IPP
#define FEMERA_HAS_GRID_TILE_IPN1_IPP

namespace femera { namespace grid { namespace tile {

//NOTE The underlying Pins elements have no edges, but the Ipn1 tile is a bar
//     with one edge, so its tiling methods can delegate to Bar1.

constexpr bool Ipn1::calc_tile_vert_conn (
  fmr::Local_int conn[2],
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
    noexcept {
    return Bar1::calc_tile_vert_conn(conn, tile_i, part_size, elem_p);
  }
constexpr bool Ipn1::calc_tile_vert_conn_prdc (
  fmr::Local_int conn[2],
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
    noexcept{
    return Bar1::calc_tile_vert_conn_prdc(conn, tile_i, part_size, elem_p);
  }
constexpr bool Ipn1::calc_tile_node_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
    noexcept {
    return Bar1::calc_tile_node_conn(conn, tile_i, part_size, elem_p);
  }
template <typename F>
constexpr bool Ipn1::calc_tile_part_node_coor(F* coor,
    const fmr::Local_int node_i,
    const fmr::Dim_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F tile_size[3],
    const F part_orig[3]
  ) noexcept{
    return Bar1::calc_tile_part_node_coor(
      coor, node_i, elem_p, part_elem_dims, tile_size, part_orig);
  }
  template <typename F>
  constexpr bool Ipn1::calc_tile_vert_coor(F coor[3],
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F tile_size[3], const F part_orig[3])
    noexcept{
    return Bar1::calc_tile_vert_coor(coor, vert_i, vert_n, tile_size, part_orig);
  }
  template <typename F>
constexpr bool Ipn1::calc_tile_elem_vert_coor(//x1,y1,z1, x2,y2,z2
    F vert_coor[6],// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3])// origin should default to {0.0,0.0,0.0}
    noexcept{
      return Bar1::calc_tile_elem_vert_coor(
        vert_coor, elem_i, elem_n, tile_size, part_orig);
    }
  template <typename F>
  constexpr bool Ipn1::calc_tile_elem_coor_vert(//x1,x2, y1,y2, z1, z2
    F vert_coor[6],// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3])// origin should default to {0.0,0.0,0.0}
    noexcept{
      return Bar1::calc_tile_elem_coor_vert(
        vert_coor, elem_i, elem_n, tile_size, part_orig);
    }

#if 0
  inline
  bool Elem::get_tile_vert_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int grid_x = 1,// handles grid_x not multiple of part_x
    const fmr::Local_int part_x = 1) noexcept{
    conn[0] = tile_i % grid_x % part_x;
    conn[1] = (tile_i + 1) % grid_x % part_x;// periodic BC
    return tile_i < grid_x;// false if tile_i out of range
  }
  // call Bar1 method
#endif
  //
  // Halo operations ----------------------------------------------------------
  constexpr
  fmr::Local_int Ipn1::calc_halo_node_n (
    const fmr::Local_int* part_elem_dims, const fmr::Local_int elem_p) noexcept {
    return Bar1::calc_halo_node_n(part_elem_dims, elem_p);
    }
  constexpr
  fmr::Local_int Ipn1::calc_halo_elem_n (
    const fmr::Local_int* part_elem_dims, const fmr::Local_int elem_p) noexcept {
    return Bar1::calc_halo_elem_n(part_elem_dims, elem_p);
    }
  constexpr
    fmr::Local_int Ipn1::calc_halo_node (fmr::Local_int halo_node_i,
    const fmr::Local_int* part_elem_dims, const fmr::Dim_int elem_p) noexcept
    {
    return Bar1::calc_halo_node(halo_node_i, part_elem_dims, elem_p);
    }
  constexpr
  fmr::Local_int Ipn1::calc_sort_halo_elem (const fmr::Local_int halo_elem_i,
    const fmr::Local_int* part_elem_dims) noexcept {
      return Bar1::calc_sort_halo_elem(halo_elem_i, part_elem_dims);
    }
//
} } }//end femera::grid::tile namespace


//end FEMERA_HAS_GRID_TILE_IPN1_IPP
#endif
