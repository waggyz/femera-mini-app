#ifndef FEMERA_HAS_GRID_TILE_IPP
#define FEMERA_HAS_GRID_TILE_IPP

namespace femera { namespace grid {

namespace tile {

template <class T, typename fmr::Dim_int P, typename F>
template <typename Y>
constexpr void Tile<T, P, F>::get_elem_shpf (
  Y* shpf, const Y* part_shpf, const fmr::Local_int shpf_size,
  const fmr::Local_int record_i) noexcept {
  //if (shpf_size == 0) { return; }// caller ensures shpf_size is correct
  const std::size_t offset = std::size_t(record_i) * std::size_t(shpf_size);
  for (fmr::Local_int i = 0; i < shpf_size; ++i) {
    shpf[i] = part_shpf[offset + std::size_t(i)];
  }
}

template <class T, typename fmr::Dim_int P, typename F>
template <typename Y>
constexpr void Tile<T, P, F>::get_elem_shpg (
  Y* shpg, const Y* part_shpg, const fmr::Local_int shpg_size,
  const fmr::Local_int record_i) noexcept {
  //if (shpg_size == 0) { return; }// caller ensures shpg_size is correct
  const std::size_t offset = std::size_t(record_i) * std::size_t(shpg_size);
  for (fmr::Local_int i = 0; i < shpg_size; ++i) {
    shpg[i] = part_shpg[offset + std::size_t(i)];
  }
}

} } }//end femera::grid::tile namespace

//end FEMERA_HAS_GRID_TILE_IPP
#endif
