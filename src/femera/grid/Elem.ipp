#ifndef FEMERA_HAS_GRID_ELEM_IPP
#define FEMERA_HAS_GRID_ELEM_IPP

namespace femera { namespace grid {

template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_face_n () const noexcept {
  return T::tris_n + T::quad_n;
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::get_node_n () const noexcept {
  return T::vert_n
    + (P -1) * T::edge_n
    + (P==3  ? T::vols_n * (T::tris_n + T::quad_n) : 0);
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::jacs_size
  (const fmr::Local_int intp_n) const noexcept {// default 1
  return intp_n * (D*D);
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::jacd_size
  (const fmr::Local_int intp_n) const noexcept {// default 1
  return intp_n * (D*D + 1);
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::poly_func_size
  (const fmr::Local_int intp_n) const noexcept {// default 1
  return intp_n * get_node_n ();
}
template <class T, typename fmr::Local_int P, typename fmr::Local_int D, typename F>
 constexpr fmr::Local_int elem::Elem<T,P,D,F>::poly_grad_size
  (const fmr::Local_int intp_n) const noexcept {// default 1
  return intp_n * get_node_n () * D;
}
  // Element Jacobian and determinant calculations
  //template <typename Y>
  template <class T, typename fmr::Local_int P, typename fmr::Local_int D,
    typename F>//, typename Y>
  void elem::Elem<T,P,D,F>::jacs_nsoa (fmr::Phys_float* jacs,
    const F* node_coor,// x1,x2,..., y1,y2,..., z1,z2,...
    const fmr::Local_int intp_n,
    const fmr::Local_int node_n) noexcept {
      /* In general, Jacobians are D x D matrices at each integration point.
         The jacs array should be filled with the Jacobian matrices
         for each integration point.
         Each Jacobians is a matrix-matrix product of the shape function gradients
         and the node coordinates.
      */
     //const fmr::Phys_float shapg [D*node_n] = {};
     for(fmr::Local_int ip=0; ip<intp_n; ++ip){
       for(fmr::Local_int i=0; i<D; ++i){
         for(fmr::Local_int j=0; j<D; ++j){
           jacs[ip*D*D + i*D + j] = 0;
           for(fmr::Local_int k=0; k<node_n; ++k){
             jacs[ip*D*D + i*D + j] += node_coor[i*node_n + k]
               * /* poly_grad_NODE_N[] */ 0;
           }
         }
       }
     }
    }
} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
