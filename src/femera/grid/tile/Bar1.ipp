#ifndef FEMERA_HAS_GRID_TILE_BAR1_IPP
#define FEMERA_HAS_GRID_TILE_BAR1_IPP

namespace femera { namespace grid { namespace tile {

constexpr bool Bar1::calc_tile_vert_conn (
  fmr::Local_int conn[2],
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
    noexcept {
    conn[0] = elem_p * tile_i;
    conn[1] = elem_p * (tile_i + 1);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bar1::calc_tile_vert_conn_prdc (
  fmr::Local_int conn[2],
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
    noexcept{
    if (part_size[0] == 0) return false;//TODO remove this check?
    // part_size counts elements; each element spans p node indices.
    conn[0] = elem_p * (tile_i % part_size[0]);
    conn[1] = elem_p * ((tile_i + 1) % part_size[0]);
    return tile_i < part_size[0];// false if tile_i out of range
  }
constexpr bool Bar1::calc_tile_node_conn (
  fmr::Local_int* conn,
    const fmr::Local_int tile_i,// local element index
    const fmr::Local_int* part_size,
    const fmr::Dim_int elem_p)
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
constexpr bool Bar1::calc_tile_part_node_coor(F* coor,
    const fmr::Local_int node_i,
    const fmr::Dim_int elem_p,
    const fmr::Local_int* part_elem_dims,
    const F tile_size[3],
    const F part_orig[3]
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
  constexpr bool Bar1::calc_tile_vert_coor(F coor[3],
    const fmr::Local_int vert_i, const fmr::Local_int vert_n,
    const F tile_size[3], const F part_orig[3])
    noexcept{
    coor[0] = (part_orig ? part_orig[0] : F(0.0)) + vert_i * tile_size[0];
    coor[1] = (part_orig ? part_orig[1] : F(0.0)) + vert_i * tile_size[1];
    coor[2] = (part_orig ? part_orig[2] : F(0.0)) + vert_i * tile_size[2];
    return vert_i < vert_n;
  }
  template <typename F>
constexpr bool Bar1::calc_tile_elem_vert_coor(//x1,y1,z1, x2,y2,z2
    F vert_coor[6],// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3])// origin should default to {0.0,0.0,0.0}
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
  constexpr bool Bar1::calc_tile_elem_coor_vert(//x1,x2, y1,y2, z1, z2
    F vert_coor[6],// size: vert_n * sims_d
    const fmr::Local_int elem_i, const fmr::Local_int elem_n,
    const F tile_size[3], const F part_orig[3])// origin should default to {0.0,0.0,0.0}
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
  constexpr fmr::Local_int* Bar1::get_tile_vert_conn (fmr::Local_int tile_i,
    // local element index
    const fmr::Local_int grid_x = 1,
    const fmr::Local_int part_x = 1) noexcept{
    return new fmr::Local_int[2]{
      tile_i % grid_x % part_x,
      tile_i % grid_x % part_x + 1};// periodic BC
    }
#endif
  //
  // Halo operations ----------------------------------------------------------
  // Easy for this tile: only two nodes and elements might be at partition edges.
  //TODO check if get_elem_n() > 0?
  constexpr
  fmr::Local_int Bar1::calc_halo_node_n (
    const fmr::Local_int*, const fmr::Local_int) noexcept { return 2;
    }
  constexpr
  fmr::Local_int Bar1::calc_halo_elem_n (
    const fmr::Local_int*, const fmr::Local_int) noexcept { return 2;
    }
  constexpr
    fmr::Local_int Bar1::calc_halo_node (fmr::Local_int halo_node_i,
    const fmr::Local_int* part_elem_dims, const fmr::Dim_int elem_p) noexcept
    {return halo_node_i <= 0
      ? 0 : Bar1::calc_tile_part_node_n(elem_p, part_elem_dims) - 1;
    }
  constexpr
  fmr::Local_int Bar1::calc_sort_halo_elem (const fmr::Local_int halo_elem_i,
    const fmr::Local_int* part_elem_dims) noexcept {
      return  (halo_elem_i <= 0) ? 0
      : ((halo_elem_i == 1) ? Bar1::get_tile_part_elem_n(part_elem_dims) - 1
        : halo_elem_i - 1);
    }
/*
  std::valarray<fmr::Local_int> part_elem_dims(3);
  part_elem_dims[0] = do_elem_n; part_elem_dims[1] = 1; part_elem_dims[2] = 1;


  std::valarray<fmr::Geom_float> tile_size(3);
  tile_size[0] = bars_len / part_elem_dims[0];
  tile_size[1] = 0.0; tile_size[2] = 0.0;

  std::valarray<fmr::Local_int>
  part_node_conn (std::size_t(part_elem_n) * elem_node_n);

      std::valarray<fmr::Geom_float>
      part_node_coor(std::size_t(part_node_n) * part_d);

#if 0
          if(is_unstructured) {
            part_node_coor[n * part_d + d] += 0.99 * (bars_len / do_elem_n)
            * (fmr::Geom_float(rand()) / RAND_MAX - 0.5);// [-0.5,0.5]
          }
#endif


#ifdef DEBUG_PRINT
    const auto ok = Tile<Bar1, elem_p>::get_tile_node_conn
      (&part_node_conn[el * elem_node_n], h, &part_elem_dims[0]);
    if (el < 10) {
      const auto i = el * elem_node_n;
      mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "elem",
        "%4u conn %u %u (halo sort)",
        el, part_node_conn[i], part_node_conn[i+1]);
    }
#endif
#ifdef DEBUG_PRINT
        const auto ok = tile.get_tile_part_node_coor
          (buf, n, &part_elem_dims[0], &tile_size[0]);//TODO get rid of buf?
        for (fmr::Dim_int d = 0; d < part_d; ++d) {
          part_node_coor[n * part_d + d] = buf[d];
        }
        if (n < 10) {
          const auto i = n * part_d;
          mini->data->send (fmr::log, ok?" ok ":" :( ", "tile", "node",
            "%4u coor %f", n, double(part_node_coor[i]));
        }
#endif
*/
  template <typename F=fmr::Geom_float>// Get a whole mesh (for comparison)
  fmr::Local_int calc_tile_mesh (// returns halo_elem_n
    fmr::Local_int* node_conn, F* node_coor,//NOTE elems halo sorted, nodes not.
    const F tile_size[3],// only [0] used for this tile
    const fmr::Local_int* part_elem_dims,
    const fmr::Dim_int part_d,
    const fmr::Dim_int elem_p,
    const F* orig_coor
  ) noexcept{
  if (!node_conn || !node_coor || !tile_size || !part_elem_dims) return 0;
  F zero_coor[3] = {0.0, 0.0, 0.0};
  const auto part_orig = orig_coor ? orig_coor : zero_coor;
  //
  using femera::grid::tile::Tile;
  using femera::grid::tile::Bar1;
  //
  Tile<Bar1> tile;
  switch(elem_p) {
    case 1: tile = Tile<Bar1, 1>(); break;
    case 2: tile = Tile<Bar1, 2>(); break;
    case 3: tile = Tile<Bar1, 3>(); break;
    default: tile = Tile<Bar1, 1>(); break;
  }
  //
  const auto part_elem_n = tile.get_tile_part_elem_n (&part_elem_dims[0]);
  const auto elem_node_n = tile.get_tile_elem_node_n ();
  //
  // Connectivity: first halo elements, then the remaining elements.
  for (fmr::Local_int el = 0; el < part_elem_n; ++el) {
    const auto h = tile.get_halo_sort_elem (el, &part_elem_dims[0]);
  //
    tile.get_tile_node_conn
      (&node_conn[el * elem_node_n], h, &part_elem_dims[0]);
  }
  // Node coordinates
  //TODO also renumber local nodes, halo first.
  const auto part_node_n = tile.get_tile_part_node_n (&part_elem_dims[0]);
  fmr::Geom_float buf[4];
  for ( fmr::Local_int n=0; n < part_node_n; ++n) {
    tile.get_tile_part_node_coor
      (buf, n, &part_elem_dims[0], &tile_size[0]);//TODO get rid of buf?
    for (fmr::Dim_int d = 0; d < part_d; ++d) {
      node_coor[n * part_d + d] = buf[d] + part_orig[d];
    }
  }
  return tile.get_halo_elem_n(part_elem_dims);
  }


//
} } }//end femera::grid::tile namespace


//end FEMERA_HAS_GRID_TILE_BAR1_IPP
#endif
