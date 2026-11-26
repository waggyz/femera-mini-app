#ifndef FEMERA_HAS_GRID_FREE_HPP
#define FEMERA_HAS_GRID_FREE_HPP

#include "Elem.hpp"

#include <cstddef>
#include <type_traits>

namespace femera { namespace grid { namespace elem {
class Bars;
} } }

namespace femera { namespace grid { namespace free {

// Stateless access to caller-owned arrays for a homogeneous element partition.
template <class T, fmr::Dim_int P=1, typename F=fmr::Geom_float>
class Free {
  static_assert(std::is_same<T, elem::Bars>::value,
    "Free currently supports Bars only");
  static_assert(P >= 1 && P <= 3,
    "Free supports polynomial orders 1 through 3");
public:
  using Element = elem::Elem<T, P, F>;

  static constexpr fmr::Dim_int get_elem_p () noexcept {
    return Element::get_elem_p();
  }
  static constexpr fmr::Dim_int get_elem_d () noexcept {
    return Element::get_elem_d();
  }
  static constexpr fmr::Local_int get_elem_node_n () noexcept {
    return Element::get_node_n();
  }
  static constexpr fmr::Local_int get_elem_vert_n () noexcept {
    return Element::get_vert_n();
  }
  // Ordinary packed matrix-plus-measure width; caller supplies
  // sims_d >= get_elem_d(), independently of the fixed reference dimension.
  static constexpr fmr::Local_int get_elem_jacd_size (
    const fmr::Dim_int sims_d) noexcept {
    return Element::get_jacs_dets_size(1, sims_d);
  }

  // Copy [element][local node], including all higher-order solution nodes.
  // Caller supplies valid, non-overlapping ranges and a representable offset.
  static constexpr void get_elem_conn (
    fmr::Local_int* conn, const fmr::Local_int* part_node_conn,
    const fmr::Local_int elem_i) noexcept;

  // Natural-node Bars store one opaque geometric record per element, reused
  // for every quadrature point. jacd_size counts scalars, including the measure.
  // Nonempty copies require valid, non-overlapping ranges and a representable
  // offset. Zero size is a no-op and permits null pointers.
  template <typename Y>
  static constexpr void get_elem_jacd (
    Y* jval, const Y* part_jacd, const fmr::Local_int jacd_size,
    const fmr::Local_int elem_i) noexcept;
};

} } }//end femera::grid::free:: namespace

#include "Free.ipp"

#endif
