#ifndef FEMERA_HAS_GRID_ELEM_BARS_IPP
#define FEMERA_HAS_GRID_ELEM_BARS_IPP

namespace femera { namespace grid { namespace elem {
#if 0
/*
C++11 requires redundant declaration/definition for constexpr arrays to 
reference them later.
*/
//constexpr fmr::Local_int Bars::tris_conn [];
//constexpr fmr::Local_int Bars::quad_conn [];

inline constexpr fmr::Local_int Bars::bars_conn [];
inline constexpr fmr::Geom_float Bars::vert_coor [];
inline constexpr fmr::Geom_float Bars::coor_vert [];

#endif

  inline constexpr
  bool get_tile_conn (fmr::Local_int* conn,//FIXME generalize & move to Elem.ipp
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* grid_size)
    noexcept{
    conn[0] = (tile_i + 0) % grid_size[0];
    conn[1] = (tile_i + 1) % grid_size[0];// periodic BC
    return tile_i < grid_size[0];// false if tile_i out of range
  }
  inline constexpr
  bool get_tile_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* grid_size,// handles grid_x not multiple of part_x
    const fmr::Local_int* part_size)
    noexcept{
    conn[0] = (tile_i + 0) % grid_size[0] % part_size[0];
    conn[1] = (tile_i + 1) % grid_size[0] % part_size[0];// periodic BC
    return tile_i < grid_size[0];// false if tile_i out of range
  }
#if 0
  inline
  bool Elem::get_tile_conn (fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int grid_x = 1,// handles grid_x not multiple of part_x
    const fmr::Local_int part_x = 1) noexcept{
    conn[0] = tile_i % grid_x % part_x;
    conn[1] = (tile_i + 1) % grid_x % part_x;// periodic BC
    return tile_i < grid_x;// false if tile_i out of range
  }
    // free surface BCs
  constexpr fmr::Local_int* Bars::get_tile_conn (fmr::Local_int tile_i,// local element index
    const fmr::Local_int grid_x = 1,
    const fmr::Local_int part_x = 1) noexcept{
    return new fmr::Local_int[2]{
      tile_i % grid_x % part_x,
      tile_i % grid_x % part_x + 1};// periodic BC
    }
#endif
} } }//end femera::grid::elem namespace

//end FEMERA_HAS_GRID_ELEM_BARS_IPP
#endif
