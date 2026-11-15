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
    + ((P<3) ? 0 : T::cubic_tris_node_n);
}
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
// Element shape function and Jacobian stuff ==================================
//
namespace elem { namespace detail {

template <class T, fmr::Dim_int P>
struct poly_dispatch {
  static_assert(P >= 1 && P <= 3, "polynomial dispatch supports orders 1 through 3");
  static_assert(P < 1 || P > 3 || !std::is_same<T, T>::value,
    "polynomial dispatch is not implemented for this topology");
};

enum class poly_output { values, node_grad, grad_node };

template <class T, fmr::Dim_int P, typename F>
constexpr void eval_poly(F* out, const F* intp, fmr::Local_int node_n,
                         fmr::Local_int intp_n, fmr::Dim_int elem_d,
                         poly_output output) {
  using E = Elem<T,P,F>;
  using Dispatch = poly_dispatch<T,P>;
  if (node_n != E::get_node_n() || elem_d != E::get_elem_d())
    throw std::invalid_argument("element polynomial node count or reference dimension mismatch");
  if (intp_n != 0 && (out == nullptr || intp == nullptr))
    throw std::invalid_argument("element polynomial buffers must not be null");
  const std::size_t out_stride = std::size_t(node_n)
    * (output == poly_output::values ? 1u : std::size_t(elem_d));
  for (fmr::Local_int ip=0; ip<intp_n; ++ip) {
    F x[3] = {};
    for (fmr::Dim_int d=0; d<elem_d; ++d)
      x[d] = intp[std::size_t(ip)*elem_d+d];
    F* result = out+std::size_t(ip)*out_stride;
    switch (output) {
      case poly_output::values: Dispatch::values(result,x); break;
      case poly_output::node_grad: Dispatch::node_grad(result,x); break;
      case poly_output::grad_node: Dispatch::grad_node(result,x); break;
    }
  }
}

} }// end elem::detail

template <class T, typename fmr::Dim_int P, typename F>
constexpr void elem::Elem<T,P,F>::get_poly_func_vals(F* out, const F* intp,
    fmr::Local_int node_n, fmr::Local_int intp_n, fmr::Dim_int elem_d) {
  detail::eval_poly<T,P>(out,intp,node_n,intp_n,elem_d,detail::poly_output::values);
}
template <class T, typename fmr::Dim_int P, typename F>
constexpr void elem::Elem<T,P,F>::get_poly_node_grad(F* out, const F* intp,
    fmr::Local_int node_n, fmr::Local_int intp_n, fmr::Dim_int elem_d) {
  detail::eval_poly<T,P>(out,intp,node_n,intp_n,elem_d,detail::poly_output::node_grad);
}
template <class T, typename fmr::Dim_int P, typename F>
constexpr void elem::Elem<T,P,F>::get_poly_grad_node(F* out, const F* intp,
    fmr::Local_int node_n, fmr::Local_int intp_n, fmr::Dim_int elem_d) {
  detail::eval_poly<T,P>(out,intp,node_n,intp_n,elem_d,detail::poly_output::grad_node);
}

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
constexpr bool elem::Elem<T,P,F>::get_elem_node_coor_impl (F* node_coor,
  const F* vert_coor, fmr::Dim_int stride) noexcept {
  //NOTE Caller checks array bounds:
  //     node_coor is size get_node_n() * 3
  //     vert_coor has get_vert_n() vertices with the supplied stride.
  //
  // Calculate this based on get_elem_coor_vert () and element constituents:
  // concatenate elem_conn {0,1,...,vert_n-1} coordinates, then,
  // for P>1, follow get_bars_vert_conn () calculate edge node coordinates, and 
  // for P==3, follow get_tris_vert_conn () for eligible triangular interiors.
  // Cubic triangle interiors may be element-interior or solid face nodes.
  if (node_coor == nullptr || vert_coor == nullptr
    || get_vert_n() <= 0 || P<=0 || P>3) return false;
  for (fmr::Local_int i = 0; i < get_vert_n (); ++i) {
    for (fmr::Local_int j = 0; j < 3; ++j) {
      node_coor[i*3 + j] = j < stride ? vert_coor[i*stride + j] : F(0);
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
        for (fmr::Local_int j = 0; j < 3; ++j) {
          node_coor[(get_vert_n() + el * (P-1) + i) * 3 + j]
            = vert0_weight * node_coor[get_bars_vert_conn()[el*2+0]*3 + j]
            + vert1_weight * node_coor[get_bars_vert_conn()[el*2+1]*3 + j];
        }
      }
    }
  }
  if (P == 2) return true;// Second-order elements have vertex and edge nodes only.
  // Calculate face node coordinates for P>2
  const fmr::Local_int face_node_n = get_node_n() - get_vert_n()
    - get_edge_n() * (P-1);
  if (face_node_n > 0) {
    if (get_tris_vert_conn() == nullptr) return false;
    const F intrp_tri = 1.0/3.0;// only one interpolation weight needed.
    const fmr::Local_int start_tris = get_vert_n() + get_edge_n() * (P-1);
    for (fmr::Local_int el = 0; el < face_node_n; ++el) {
      for (fmr::Local_int j = 0; j < 3; ++j) {
        node_coor[(start_tris + el) * 3 + j] = intrp_tri * (
            node_coor[get_tris_vert_conn()[el*3+0]*3 + j]
          + node_coor[get_tris_vert_conn()[el*3+1]*3 + j]
          + node_coor[get_tris_vert_conn()[el*3+2]*3 + j]);
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

} }//end femera::grid namespace

//end FEMERA_HAS_GRID_ELEM_IPP
#endif
