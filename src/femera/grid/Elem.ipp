#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_face_n () noexcept {
  return T::tris_n + T::quad_n;
}
#if 1
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_node_n () noexcept {
  return T::vert_n
    + (P -1) * T::edge_n
    + (P==3  ? T::vols_n * (T::tris_n + T::quad_n) : 0);
}
#endif
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::jacd_size
  (const fmr::Local_int intp_n) noexcept {// default 1
  return intp_n * (D*D + 1);
}

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
