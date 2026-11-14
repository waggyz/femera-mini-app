#ifndef FMR_HAS_FEMS_ELEM_POLY_IPP
#define FMR_HAS_FEMS_ELEM_POLY_IPP

namespace fmr { namespace fems {

// These functions evaluate shape functions at x[0] in the one-dimensional
// reference interval. Physical coordinate dimension is independent.
// When edge nodes are at their natural locations, the Jacobian is constant,
// and only calc_poly_func_C1D2N and calc_poly_grad_node_C1D2N are needed for constant Jacobian evaluation.
//
template <typename Y, typename G> constexpr
// Mixed precision support for single or double precision
Y* elem::calc_poly_func_C1D2N// C1 domain: [0.0,1.0]
(Y f[2u], const G x[3u]) noexcept {// x[1], x[2] not used for 1D bar element?
  f[ 0] = 1.0-x[0];
  f[ 1] = x[0];
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_grad_node_C1D2N
(Y g[2u], const G[]) noexcept {
  g[0]=-1.0;
  g[1]= 1.0;
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_node_grad_C1D2N
(Y g[2u], const G[]) noexcept {
  g[0]=-1.0;
  g[1]= 1.0;
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_func_C1D3N
(Y f[3u], const G x[3u]) noexcept {
  const Y L2=x[0];
  const Y L1=1.0-L2;
  f[ 0] = 2.0*L1*L1 - L1;// Vertex nodes
  f[ 1] = 2.0*L2*L2 - L2;
  f[ 2] = 4.0*L1*L2;// Edge node
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_grad_node_C1D3N
(Y g[3u], const G x[3u]) noexcept {
  const Y xi=x[0];
  g[0]=4.0*xi-3.0;
  g[1]=4.0*xi-1.0;
  g[2]=4.0-8.0*xi;
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_node_grad_C1D3N
(Y g[3u], const G x[3u]) noexcept {
  calc_poly_grad_node_C1D3N(g, x);
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_func_C1D4N
(Y f[4u], const G x[3u]) noexcept {
  const Y L2=x[0];
  const Y L1=(1.0-L2);
  f[ 0]= 0.5* L1 *(3.0* L1 -1.0)*(3.0* L1 -2.0);// Vertex nodes;
  f[ 1]= 0.5* L2 *(3.0* L2 -1.0)*(3.0* L2 -2.0);
  // Edge nodes
  f[ 2]=4.5*( L1*L2 *(3.0*L1 -1.0) );
  f[ 3]=4.5*( L1*L2 *(3.0*L2 -1.0) );
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_grad_node_C1D4N
(Y g[4u], const G x[3u]) noexcept {
  const Y xi=x[0];
  const Y L1=1.0-xi;
  g[0]=-0.5*(27.0*L1*L1-18.0*L1+2.0);
  g[1]= 0.5*(27.0*xi*xi-18.0*xi+2.0);
  g[2]= 4.5*(2.0-10.0*xi+9.0*xi*xi);
  g[3]= 4.5*(-1.0+8.0*xi-9.0*xi*xi);
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_node_grad_C1D4N
(Y g[4u], const G x[3u]) noexcept {
  calc_poly_grad_node_C1D4N(g, x);
  return g;
}

} }// end fmr::fems namespace

#endif
//end FMR_HAS_FEMS_ELEM_POLY_IPP
