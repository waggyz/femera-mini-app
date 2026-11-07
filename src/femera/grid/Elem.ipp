#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Local_int P, typename F>
constexpr fmr::Local_int elem::Elem<T,P,F>::get_face_n () const noexcept {
  return T::tris_n + T::quad_n;
}
template <class T, typename fmr::Local_int P, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,F>::get_node_n () const noexcept {
  return T::vert_n
    + (P -1) * T::edge_n
    + (P==3  ? T::vols_n * (T::tris_n + T::quad_n) : 0);
}
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
  // Element Jacobian and determinant calculations
  //template <typename Y>
  template <class T, typename fmr::Local_int P, typename F>//, typename Y>
  inline void elem::Elem<T,P,F>::calc_jacs_nsoa_ref (fmr::Phys_float* jacs,
    const F* coor_node,// x1,x2,..., y1,y2,..., z1,z2,...
    const F* poly_grad,// shape function gradients at integration points
    const fmr::Local_int intp_n, const fmr::Local_int sims_d) const noexcept {
      /* In general, Jacobians are D x D matrices at each integration point.
         The jacs array should be filled with the Jacobian matrices
         for each integration point.
         Each Jacobians is a matrix-matrix product of the shape function gradients
         and the node coordinates.
      */
     const auto node_n = get_node_n ();
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<sims_d; ++i){
         for(fmr::Local_int j=0; j<sims_d; ++j){
           jacs[ip*sims_d*sims_d + i*sims_d + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){
             jacs[ip*sims_d*sims_d + i*sims_d + j] += coor_node[i*node_n + k]
               * poly_grad[ip*node_n*sims_d + k*sims_d + j];
  } } } } }
  template <class T, typename fmr::Local_int P, typename F>//, typename Y>
  inline void elem::Elem<T,P,F>::calc_jacs_naos_ref (fmr::Phys_float* jacs,
    const F* node_coor,// x1,y1,z1, x2,y2,z2,...
    const F* poly_grad,// shape function gradients at integration points
    const fmr::Local_int intp_n, const fmr::Local_int sims_d) const noexcept {
      /* In general, Jacobians are D x D matrices at each integration point.
         The jacs array should be filled with the Jacobian matrices
         for each integration point.
         Each Jacobians is a matrix-matrix product of the shape function gradients
         and the node coordinates.
      */
     const auto node_n = get_node_n ();
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<sims_d; ++i){
         for(fmr::Local_int j=0; j<sims_d; ++j){
           jacs[ip*sims_d*sims_d + i*sims_d + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){
             jacs[ip*sims_d*sims_d + i*sims_d + j] += node_coor[k*sims_d + i]
               * poly_grad[ip*node_n*sims_d + k*sims_d + j];
  } } } } }

  template <class T, typename fmr::Local_int P, typename F>//, typename Y>
  constexpr
  F elem::Elem<T,P,F>::calc_delt_qual (const F* node_coor, const fmr::Dim_int dims)
  const noexcept {
    // Calculate the ratio of the minimum edge length
    // to the maximum coordinate absolute value.
    const auto node_n = get_node_n ();
    F min_edge = std::numeric_limits<F>::max ();
    F max_coord = std::numeric_limits<F>::lowest ();
    for(fmr::Local_int i=0; i<(node_n * dims); ++i){
      const F coor = abs(node_coor[i]);
      if(coor > max_coord) max_coord = coor;
    }
    // Early return if max_coord is zero to avoid division by zero.
    if(!(max_coord < F(0) || max_coord > F(0))) return F(0);
    //
    const auto conn = get_bars_conn ();// Use edge connectivity.
    const auto edge_n = get_edge_n ();
    for(fmr::Local_int j=0; j<edge_n; ++j){// loop through edges
      F edge_len2 = F(0);// tracks square of edge length
      const auto vert_i = conn[j*2 + 0];
      const auto vert_j = conn[j*2 + 1];
      for(fmr::Dim_int d=0; d<dims; ++d){
        const F diff = node_coor[vert_i*dims + d] - node_coor[vert_j*dims + d];
        edge_len2 += diff * diff;
      }
      if(edge_len2 < min_edge) min_edge = edge_len2;
    }
    return std::sqrt(min_edge) / max_coord;
  }//end calc_delt_qual

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
