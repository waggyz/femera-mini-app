#ifndef FEMERA_HAS_GRID_TILE_BAR5_IPP
#define FEMERA_HAS_GRID_TILE_BAR5_IPP

namespace femera { namespace grid { namespace tile {

constexpr bool Bar5::calc_tile_vert_conn (fmr::Local_int* conn,
  const fmr::Local_int edge_i, const fmr::Local_int* dims,
  const fmr::Dim_int p, const TilePattern pattern) noexcept {
  const fmr::Local_int nx = dims[0], ny = dims[1];
  if (nx == 0 || ny == 0 || p == 0 || pattern.variant > 3
      || edge_i >= calc_tile_part_elem_n (dims)) return false;

  const fmr::Local_int horizontal_n = nx * (ny + 1);
  const fmr::Local_int vertical_n = (nx + 1) * ny;
  fmr::Local_int x = 0, y = 0;
  if (edge_i < horizontal_n) {
    x = edge_i % nx;
    y = edge_i / nx;
    conn[0] = y * (nx + 1) + x;
    conn[1] = conn[0] + 1;
  } else if (edge_i < horizontal_n + vertical_n) {
    const fmr::Local_int i = edge_i - horizontal_n;
    x = i % (nx + 1);
    y = i / (nx + 1);
    conn[0] = y * (nx + 1) + x;
    conn[1] = conn[0] + nx + 1;
  } else {
    const fmr::Local_int i = edge_i - horizontal_n - vertical_n;
    x = i % nx;
    y = i / nx;
    const bool alternate = pattern.variant == 1
      || (pattern.variant >= 2 && ((x ^ y ^ pattern.offset_x
        ^ pattern.offset_y ^ (pattern.variant - 2)) & 1u));
    const fmr::Local_int bottom_left = y * (nx + 1) + x;
    conn[0] = bottom_left + (alternate ? 1 : 0);
    conn[1] = bottom_left + nx + (alternate ? 1 : 2);
  }
  return true;
}

constexpr bool Bar5::calc_tile_node_conn (fmr::Local_int* conn,
  const fmr::Local_int edge_i, const fmr::Local_int* dims,
  const fmr::Dim_int p, const TilePattern pattern) noexcept {
  if (!calc_tile_vert_conn (conn, edge_i, dims, p, pattern)) return false;
  const fmr::Local_int vertex_n = (dims[0] + 1) * (dims[1] + 1);
  for (fmr::Local_int j = 0; j + 1 < p; ++j) {
    conn[2 + j] = vertex_n + edge_i * (p - 1) + j;
  }
  return true;
}
constexpr bool Bar5::calc_tile_vert_conn_prdc (fmr::Local_int* conn,
  const fmr::Local_int edge_i, const fmr::Local_int* dims,
  const fmr::Dim_int p, const TilePattern pattern) noexcept {
  if (!calc_tile_vert_conn (conn, edge_i, dims, p, pattern)) return false;
  for (unsigned j = 0; j < 2; ++j) {
    const fmr::Local_int x = conn[j] % (dims[0] + 1);
    const fmr::Local_int y = conn[j] / (dims[0] + 1);
    conn[j] = (y % dims[1]) * dims[0] + x % dims[0];
  }
  return true;
}
template <typename F>
constexpr bool Bar5::calc_tile_vert_coor (F* coor, const fmr::Local_int vert_i,
  const fmr::Local_int* dims, const F* size, const F* origin) noexcept {
  const fmr::Local_int nx = dims[0], ny = dims[1];
  if (nx == 0 || ny == 0 || vert_i >= (nx + 1) * (ny + 1)) return false;
  coor[0] = (origin ? origin[0] : F(0)) + F(vert_i % (nx + 1)) * size[0];
  coor[1] = (origin ? origin[1] : F(0)) + F(vert_i / (nx + 1)) * size[1];
  coor[2] = origin ? origin[2] : F(0);
  return true;
}
template <typename F>
constexpr bool Bar5::calc_tile_elem_vert_coor (F* coor, const fmr::Local_int edge_i,
  const fmr::Local_int* dims, const F* size, const F* origin,
  const TilePattern pattern) noexcept {
  fmr::Local_int vertices[2] = {};
  if (!calc_tile_vert_conn (vertices, edge_i, dims, 1, pattern)) return false;
  return calc_tile_vert_coor (coor, vertices[0], dims, size, origin)
    && calc_tile_vert_coor (coor + 3, vertices[1], dims, size, origin);
}
template <typename F>
constexpr bool Bar5::calc_tile_elem_coor_vert (F* coor, const fmr::Local_int edge_i,
  const fmr::Local_int* dims, const F* size, const F* origin,
  const TilePattern pattern) noexcept {
  F aos[6] = {};
  if (!calc_tile_elem_vert_coor (aos, edge_i, dims, size, origin, pattern)) return false;
  for (unsigned v = 0; v < 2; ++v)
    for (unsigned d = 0; d < 3; ++d)
      coor[2*d+v] = aos[3*v+d];
  return true;
}
template <typename F>
constexpr bool Bar5::calc_tile_part_node_coor (F* coor, const fmr::Local_int node_i,
  const fmr::Dim_int p, const fmr::Local_int* dims, const F* size,
  const F* origin, const TilePattern pattern) noexcept {
  if (p == 0 || dims[0] == 0 || dims[1] == 0 || pattern.variant > 3
      || node_i >= calc_tile_part_node_n (p, dims)) return false;
  const fmr::Local_int vertex_n = (dims[0] + 1) * (dims[1] + 1);
  if (node_i < vertex_n) return calc_tile_vert_coor (coor, node_i, dims, size, origin);
  const fmr::Local_int local = node_i - vertex_n;
  const fmr::Local_int edge_i = local / (p - 1);
  const fmr::Local_int interior = local % (p - 1);
  F endpoints[6] = {};
  if (!calc_tile_elem_vert_coor (endpoints, edge_i, dims, size, origin, pattern))
    return false;
  const F fraction = F(interior + 1) / F(p);
  for (unsigned d = 0; d < 3; ++d)
    coor[d] = endpoints[d] + fraction * (endpoints[3+d] - endpoints[d]);
  return true;
}

} } }//end femera::grid::tile namespace


//end FEMERA_HAS_GRID_TILE_BAR5_IPP
#endif
