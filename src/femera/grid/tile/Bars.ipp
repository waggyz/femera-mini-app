#ifndef FEMERA_HAS_GRID_TILE_BARS_IPP
#define FEMERA_HAS_GRID_TILE_BARS_IPP

namespace femera { namespace grid { namespace tile {

constexpr bool Bars::calc_tile_vert_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int p)
    noexcept {
    conn[0] = p * tile_i;
    conn[1] = p * (tile_i + 1);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bars::calc_tile_vert_conn_prdc (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int p)
    noexcept{
    if (part_size[0] == 0) return false;//TODO remove this check?
    // part_size counts elements; each element spans p node indices.
    conn[0] = p * (tile_i % part_size[0]);
    conn[1] = p * ((tile_i + 1) % part_size[0]);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bars::calc_tile_node_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Local_int elem_p)
    noexcept {
    conn[0] = elem_p * tile_i;
    conn[1] = elem_p * (tile_i + 1);
    if (elem_p > 1) {
      for (fmr::Local_int j = 1; j < elem_p; ++j) {
        conn[j + 1] = elem_p * tile_i + j;
      }
    }
    return tile_i < part_size[0];// false if tile_i out of range
  }
template <typename F>
constexpr bool Bars::calc_tile_part_node_coor(F* coor,
    const fmr::Local_int node_i,
    const fmr::Local_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F* tile_size,
    const F* part_orig
  ) noexcept{
    const F node_delt = F(1.0) / F(elem_p);
    // Guard against null part_orig pointer.
    coor[0] = (part_orig ? part_orig[0] : F(0.0))
      + node_i * tile_size[0] * node_delt;
    coor[1] = (part_orig ? part_orig[1] : F(0.0))
      + node_i * tile_size[1] * node_delt;
    coor[2] = (part_orig ? part_orig[2] : F(0.0))
      + node_i * tile_size[2] * node_delt;
    //TODO FIXME this only works for P=1
    return node_i < elem_p * part_elem_dims[0] + 1;
  }
  template <typename F>
  constexpr bool Bars::calc_tile_vert_coor(F* coor,
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F* tile_size, const F* part_orig)
    noexcept{
    coor[0] = (part_orig ? part_orig[0] : F(0.0)) + vert_i * tile_size[0];
    coor[1] = (part_orig ? part_orig[1] : F(0.0)) + vert_i * tile_size[1];
    coor[2] = (part_orig ? part_orig[2] : F(0.0)) + vert_i * tile_size[2];
    return vert_i < vert_n;
  }
  template <typename F>
constexpr bool Bars::calc_tile_elem_vert_coor(//x1,y1,z1, x2,y2,z2
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v * 3 + 0] = (part_orig ? part_orig[0] : F(0.0))
          + (elem_i + v) * tile_size[0];
        vert_coor[v * 3 + 1] = (part_orig ? part_orig[1] : F(0.0))
          + (elem_i + v) * tile_size[1];
        vert_coor[v * 3 + 2] = (part_orig ? part_orig[2] : F(0.0))
          + (elem_i + v) * tile_size[2];
      }
      return elem_i < elem_n;
    }
  template <typename F>
  constexpr bool Bars::calc_tile_elem_coor_vert(//x1,x2, y1,y2, z1, z2
    F* vert_coor,// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F* tile_size, const F* part_orig)// origin should default to {0.0,0.0,0.0}
    noexcept{
      for (fmr::Local_int v = 0; v < 2; ++v) {// 2 vertices for 1D bar element
        vert_coor[v + 0] = (part_orig ? part_orig[0] : F(0.0))
          + (elem_i + v) * tile_size[0];
        vert_coor[v + 2] = (part_orig ? part_orig[1] : F(0.0))
          + (elem_i + v) * tile_size[1];
        vert_coor[v + 4] = (part_orig ? part_orig[2] : F(0.0))
         + (elem_i + v) * tile_size[2];
      }
      return elem_i < elem_n;
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
    // free surface BCs
  constexpr fmr::Local_int* Bars::get_tile_vert_conn (fmr::Local_int tile_i,
    // local element index
    const fmr::Local_int grid_x = 1,
    const fmr::Local_int part_x = 1) noexcept{
    return new fmr::Local_int[2]{
      tile_i % grid_x % part_x,
      tile_i % grid_x % part_x + 1};// periodic BC
    }
#endif

} } }//end femera::grid::tile namespace


//end FEMERA_HAS_GRID_TILE_BARS_IPP
#endif
