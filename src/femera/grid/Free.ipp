#ifndef FEMERA_HAS_GRID_FREE_IPP
#define FEMERA_HAS_GRID_FREE_IPP

namespace femera { namespace grid { namespace free {

template <class T, fmr::Dim_int P, typename F>
constexpr void Free<T, P, F>::get_elem_conn (
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
  const fmr::Local_int elem_i) noexcept {
  if (jacd_size == 0) { return; }
  const std::size_t offset = std::size_t(elem_i) * std::size_t(jacd_size);
  for (fmr::Local_int i = 0; i < jacd_size; ++i) {
    jval[i] = part_jacd[offset + std::size_t(i)];
  }
}

} } }//end femera::grid::free:: namespace

//end FEMERA_HAS_GRID_FREE_IPP
#endif
