#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_face_n () const noexcept {
  return T::tris_n + T::quad_n;
}
template <class T, typename fmr::Local_int P, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,F>::get_node_n () const noexcept {
  // Handles solid and interface elements
  return T::vert_n// vertex nodes
    + (P -1) * T::edge_n// edge nodes
    + (P==3  ? T::vols_n * (T::tris_n + T::quad_n)// face nodes for P==3
    : 0);      // vols_n is 0 (1D,2D elems) or 1 (3D solid elems)
}
template <class T, typename fmr::Local_int P, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,F>::get_tile_node_n () const noexcept {
  // Handles solid and interface elements
  return T::tile_vert_n// vertex nodes
    + (P -1) * T::tile_edge_n// edge nodes
    + (P==3  ? T::tile_vols_n * (T::tile_tris_n + T::tile_quad_n)// face nodes for P==3
    : 0);      // vols_n is 0 (1D,2D elems) or 1 (3D solid elems)
}
#if 0
template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_elem_conn (fmr::Local_int i)
const noexcept {
  switch(i){
    case 0: return T::tris_n;
    case 1: return T::quad_n;
    default: return 0;
  }
}
#endif
template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::jacs_size
  (const fmr::Local_int intp_n, const fmr::Local_int this_d
  ) const noexcept {// default 1
  return intp_n * (this_d*this_d);
}
template <class T, typename fmr::Local_int P, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,F>::jacd_size
  (const fmr::Local_int intp_n, const fmr::Local_int this_d
  ) const noexcept {// default 1
  return intp_n * (this_d*this_d + 1);
}
template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::poly_func_size
  (const fmr::Local_int intp_n) const noexcept {// default 1
  return intp_n * get_node_n ();
}
template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::poly_grad_size
  (const fmr::Local_int intp_n, const fmr::Local_int this_d) const noexcept {// default 1
  return intp_n * get_node_n () * this_d;
}

  template <class T, typename fmr::Local_int P, typename F>//, typename Y>
  constexpr
  F elem::Elem<T,P,F>::calc_delt_qual (const F* vert_coor, const fmr::Dim_int dims)
  const noexcept {
    // Calculate the ratio of the minimum edge length
    // to the maximum coordinate absolute value.
    //NOTE only element vertices are used for this quality metric.
    const auto vert_n = get_vert_n ();
    F min_edge = std::numeric_limits<F>::max ();
    F max_coord = std::numeric_limits<F>::lowest ();
    for(fmr::Local_int i=0; i<(vert_n * dims); ++i){
      const F coor = abs(vert_coor[i]);
      if(coor > max_coord) max_coord = coor;
    }
    // Early return if max_coord is zero to avoid division by zero.
    if(!(max_coord < F(0) || max_coord > F(0))) return F(0);
    //
    //const auto conn = get_vert_conn ();// Use edge connectivity {0,1}.
    const auto edge_n = get_edge_n ();
    for(fmr::Local_int j=0; j<edge_n; ++j){// loop through edges
      F edge_len2 = F(0);// tracks square of edge length
      //const auto vert_i = conn[j*2 + 0];
      //const auto vert_j = conn[j*2 + 1];
      const auto vert_i = j*2 + 0;
      const auto vert_j = j*2 + 1;
      for(fmr::Dim_int d=0; d<dims; ++d){
        const F diff = vert_coor[vert_i*dims + d] - vert_coor[vert_j*dims + d];
        edge_len2 += diff * diff;
      }
      if(edge_len2 < min_edge) min_edge = edge_len2;
    }
    return sqrt(min_edge) / max_coord;
  }//end calc_delt_qual

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
