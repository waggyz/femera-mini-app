#ifndef FEMERA_HAS_GRID_FREE_IPP
#define FEMERA_HAS_GRID_FREE_IPP

namespace femera { namespace grid { namespace free {

template <class T, fmr::Dim_int P, typename F>
constexpr void Free<T, P, F>::get_elem_conn (// caller ensures conn has enough space
  fmr::Local_int* conn, const fmr::Local_int* part_node_conn,
  const fmr::Local_int elem_i) noexcept {
  const auto conn_n = get_elem_node_n();
  const std::size_t offset = std::size_t(elem_i) * std::size_t(conn_n);
  for (fmr::Local_int i = 0; i < conn_n; ++i) {
    conn[i] = part_node_conn[offset + std::size_t(i)];
  }
}

template <class T, fmr::Dim_int P, typename F>
template <typename Y>
constexpr void Free<T, P, F>::get_elem_jacd (
  Y* jval, const Y* part_jacd, const fmr::Local_int jacd_size,
  const fmr::Local_int elem_i) noexcept {// caller ensures jacd_size is correct
  //if (jacd_size == 0) { return; }
  const std::size_t offset = std::size_t(elem_i) * std::size_t(jacd_size);
  for (fmr::Local_int i = 0; i < jacd_size; ++i) {
    jval[i] = part_jacd[offset + std::size_t(i)];
  }
}

template <class T, fmr::Dim_int P, typename F>
template <typename Y>
constexpr void Free<T, P, F>::get_elem_shpf (
  Y* shpf, const Y* part_shpf, const fmr::Local_int shpf_size,
  const fmr::Local_int record_i) noexcept {// caller ensures shpf_size is correct
  //if (shpf_size == 0) { return; }
  const std::size_t offset = std::size_t(record_i) * std::size_t(shpf_size);
  for (fmr::Local_int i = 0; i < shpf_size; ++i) {
    shpf[i] = part_shpf[offset + std::size_t(i)];
  }
}

template <class T, fmr::Dim_int P, typename F>
template <typename Y>
constexpr void Free<T, P, F>::get_elem_shpg (
  Y* shpg, const Y* part_shpg, const fmr::Local_int shpg_size,
  const fmr::Local_int record_i) noexcept {// caller ensures shpg_size is correct
  //if (shpg_size == 0) { return; }
  const std::size_t offset = std::size_t(record_i) * std::size_t(shpg_size);
  for (fmr::Local_int i = 0; i < shpg_size; ++i) {
    shpg[i] = part_shpg[offset + std::size_t(i)];
  }
}

} } }//end femera::grid::free:: namespace

//end FEMERA_HAS_GRID_FREE_IPP
#endif
