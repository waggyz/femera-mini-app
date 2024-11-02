#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
inline constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_face_n () noexcept {
  return this->get_tris_n () + this->get_quad_n ();
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
inline constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_node_n () noexcept {
  return this->get_vert_n ()
    + (P -1) * this->get_edge_n ()
    + (P==3  ? this->get_vols_n () * this->get_face_n () : 0);
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
inline fmr::Local_int elem::Elem<T,P,D,F>::jacd_size
  (const fmr::Local_int intp_n) noexcept {// default 1
  return intp_n * (D*D + 1);
}

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
