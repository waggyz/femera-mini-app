#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Dim_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_face_n () noexcept {
  return T::tris_n + T::quad_n;
}
template <class T, typename fmr::Dim_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_node_n () noexcept {
  // Handles solid and interface elements
  return T::vert_n// vertex nodes
    + (P -1) * T::edge_n// edge nodes
    + ((P<3) ? 0 : T::vols_n * (T::tris_n + T::quad_n));// face nodes for P==3
           // vols_n is 0 (1D,2D elems) or 1 (3D solid elems)
}
// Element shape function and Jacobian stuff ==================================
//
template <class T, typename fmr::Dim_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_poly_func_size
  (const fmr::Local_int intp_n) noexcept {// default 1
  return intp_n * get_node_n ();
}
template <class T, typename fmr::Dim_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_poly_grad_size
  (const fmr::Local_int intp_n, const fmr::Dim_int elem_d) noexcept {
  return intp_n * get_node_n () * elem_d;
}
template <class T, typename fmr::Dim_int P, typename F>
constexpr bool elem::Elem<T,P,F>::get_elem_node_coor (F* node_coor,
  const F* vert_coor) noexcept {// 3D coordinates
  //NOTE Caller checks array bounds:
  //     node_coor is size get_node_n() * 3
  //     vert_coor is size get_vert_n() * 3.
  //
  // Calculate this based on get_elem_coor_vert () and element constituents:
  // concatenate elem_conn {0,1,...,vert_n-1} coordinates, then,
  // for P>1, follow get_bars_vert_conn () calculate edge node coordinates, and 
  // for P>2, follow get_tris_vert_conn (), get_quad_vert_conn () for face node
  // coordinates.
  // Interior volume nodes not needed for serendipity elements.
  if (node_coor == nullptr || vert_coor == nullptr
    || get_vert_n() <= 0 || P<=0 || P>3) return false;
  for (fmr::Local_int i = 0; i < get_vert_n (); ++i) {
    for (fmr::Local_int j = 0; j < 3; ++j) {
      node_coor[i*3 + j] = vert_coor[i*3 + j];
    }
  }
  if (P == 1) return true;// First-order elements have only vertex nodes.
  if(get_edge_n() > 0){
    if(get_bars_vert_conn() == nullptr) return false;
    for (fmr::Local_int el = 0; el < get_edge_n(); ++el) {
      for (fmr::Local_int i = 0; i < P-1; ++i) {
        const F vert0_weight = (P == 2) ? F(0.5)
          : ((i == 0) ? F(2.0/3.0) : F(1.0/3.0));
        const F vert1_weight = (P == 2) ? F(0.5)
          : ((i == 0) ? F(1.0/3.0) : F(2.0/3.0));
        node_coor[get_vert_n() * 3 + i * 3 + 0]// x-coordinate
        = vert0_weight * vert_coor[get_bars_vert_conn()[el*2+0]*3 + 0]
        + vert1_weight * vert_coor[get_bars_vert_conn()[el*2+1]*3 + 0];
        node_coor[get_vert_n() * 3 + i * 3 + 1]// y-coordinate
        = vert0_weight * vert_coor[get_bars_vert_conn()[el*2+0]*3 + 1]
        + vert1_weight * vert_coor[get_bars_vert_conn()[el*2+1]*3 + 1];
        node_coor[get_vert_n() * 3 + i * 3 + 2]// z-coordinate
        = vert0_weight * vert_coor[get_bars_vert_conn()[el*2+0]*3 + 2]
        + vert1_weight * vert_coor[get_bars_vert_conn()[el*2+1]*3 + 2];
      }
    }
  }
  if (P == 2) return true;// Second-order elements have vertex and edge nodes only.
  // Calculate face node coordinates for P>2
  if (get_tris_n() > 0) {
    if (get_tris_vert_conn() == nullptr) return false;
    const F intrp_tri = 1.0/3.0;// only one interpolation weight needed.
    // three nodes for triangular face, one centroid calculation.
    // starting index for face nodes
    const fmr::Local_int start_tris = get_vert_n() * 3 + get_edge_n() * (P-1) * 3; 
    for (fmr::Local_int el = 0; el < get_tris_n(); ++el) {
      // Calculate face node coordinates for each triangular face
      for(fmr::Local_int i = 0; i < 3; ++i) {
        for(fmr::Local_int j = 0; j < 3; ++j) {// let the compilre unroll this
          node_coor[start_tris + i * 3 + j]
          = intrp_tri * (
            vert_coor[get_tris_vert_conn()[el*3+0]*3 + j]
          + vert_coor[get_tris_vert_conn()[el*3+1]*3 + j]
          + vert_coor[get_tris_vert_conn()[el*3+2]*3 + j]);
        }
      }
    }
  }
  if (get_quad_n() > 0) {
    if (get_quad_vert_conn() == nullptr) return false;
    const F intrp_quad = 0.25;
    const fmr::Local_int start_quad
    = get_vert_n() * 3 + get_edge_n() * (P-1) * 3 + get_tris_n() * 3;
    for (fmr::Local_int el = 0; el < get_quad_n(); ++el) {
      // Calculate face node coordinates for each quadrilateral face
      for (fmr::Local_int i = 0; i < 4; ++i) {
        for (fmr::Local_int j = 0; j < 3; ++j) {
          node_coor[start_quad + i * 3 + j]
          = intrp_quad * (
            vert_coor[get_quad_vert_conn()[el*4+0]*3 + j]
          + vert_coor[get_quad_vert_conn()[el*4+1]*3 + j]
          + vert_coor[get_quad_vert_conn()[el*4+2]*3 + j]
          + vert_coor[get_quad_vert_conn()[el*4+3]*3 + j]);
        }
      }
    }
  }
  return true;
}
#if 0
template <class T, typename fmr::Dim_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_elem_conn (fmr::Local_int i)
noexcept {
  switch(i){
    case 0: return T::tris_n;
    case 1: return T::quad_n;
    default: return 0;
  }
}
#endif
// Element quality metrics ====================================================
  template <class T, typename fmr::Dim_int P, typename F>//, typename Y>
  constexpr
  F elem::Elem<T,P,F>::get_delt_qual (
    const F* vert_coor, const fmr::Dim_int dims, const bool is_squared)
  noexcept {
    // Calculate the ratio of the minimum edge length
    // to the maximum coordinate absolute value.
    //NOTE only element vertices are used for this quality metric.
    //TODO genalize & move to Elem.ipp?
    const auto vert_n = get_vert_n ();
    F min_edge = std::numeric_limits<F>::max ();
    F max_coord = std::numeric_limits<F>::lowest ();
    for(fmr::Local_int i=0; i<(vert_n * dims); ++i){
      const F coor = abs(vert_coor[i]);
      //if(coor > max_coord) max_coord = coor;
      max_coord = (coor > max_coord) ? coor : max_coord;
    }
    // Early return if max_coord is zero to avoid division by zero.
    //if(!(max_coord < F(0) || max_coord > F(0))) return F(0);
    //
    const auto edge_n = get_edge_n ();
    for(fmr::Local_int j=0; j<edge_n; ++j){// loop through edges
      F edge_len2 = F(0);// tracks square of edge length
      //const auto vert_i = conn[j*2 + 0];
      //const auto vert_j = conn[j*2 + 1];
      const auto vert_i = j*2 + 0;
      const auto vert_j = j*2 + 1;
      for(fmr::Local_int d=0; d<dims; ++d){
        const F diff = vert_coor[vert_i*dims + d] - vert_coor[vert_j*dims + d];
        edge_len2 += diff * diff;
      }
      //if(edge_len2 < min_edge) min_edge = edge_len2;
      min_edge = (edge_len2 < min_edge) ? edge_len2 : min_edge;
    }
    //return sqrt(min_edge) / max_coord;
    return (!(max_coord < F(0) || max_coord > F(0))) ? F(0)
      : (is_squared ? min_edge / (max_coord * max_coord)
      : sqrt(min_edge) / max_coord);
  }//end get_delt_qual

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
