#ifndef FMR_HAS_FEMS_ELEM_POLY_IPP
#define FMR_HAS_FEMS_ELEM_POLY_IPP

namespace fmr { namespace fems {

// These functions evaluate shape functions at x[0] in the one-dimensional
// reference interval. Physical coordinate dimension is independent.
// When edge nodes are at their natural locations, the Jacobian is constant,
// and only calc_poly_func_C1D2N and calc_poly_grad_node_C1D2N are needed for
// constant Jacobian evaluation.
//
template <typename Y, typename G> constexpr
// Mixed precision support for single or double precision
Y* elem::calc_poly_func_C1D2N// C1 domain: [0.0,1.0]
(Y f[2u], const G x[3u]) noexcept {// x[1], x[2] not used for 1D bar element.
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

template <typename Y, typename G> constexpr
Y* elem::calc_poly_func_T2D10N (Y f[10u], const G x[3u]) noexcept {
  const Y L[3] = {Y(1)-Y(x[0])-Y(x[1]), Y(x[0]), Y(x[1])};
  for (int i=0; i<3; ++i) {
    f[i] = Y(0.5)*L[i]*(Y(3)*L[i]-Y(1))*(Y(3)*L[i]-Y(2));
    const int j = (i+1)%3;
    f[3+2*i] = Y(4.5)*L[i]*L[j]*(Y(3)*L[i]-Y(1));
    f[4+2*i] = Y(4.5)*L[i]*L[j]*(Y(3)*L[j]-Y(1));
  }
  f[9] = Y(27)*L[0]*L[1]*L[2];
  return f;
}

template <typename Y, typename G> constexpr
Y* elem::calc_poly_grad_node_T2D10N (Y g[20u], const G x[3u]) noexcept {
  const Y L[3] = {Y(1)-Y(x[0])-Y(x[1]), Y(x[0]), Y(x[1])};
  const Y dL[3][2] = {{Y(-1),Y(-1)}, {Y(1),Y(0)}, {Y(0),Y(1)}};
  for (int i=0; i<3; ++i) {
    const int j = (i+1)%3;
    for (int d=0; d<2; ++d) {
      g[2*i+d] = Y(0.5)*(Y(27)*L[i]*L[i]-Y(18)*L[i]+Y(2))*dL[i][d];
      g[2*(3+2*i)+d] = Y(4.5) * (
        L[j]*(Y(6)*L[i]-Y(1))*dL[i][d]
        + L[i]*(Y(3)*L[i]-Y(1))*dL[j][d]);
      g[2*(4+2*i)+d] = Y(4.5) * (
        L[j]*(Y(3)*L[j]-Y(1))*dL[i][d]
        + L[i]*(Y(6)*L[j]-Y(1))*dL[j][d]);
    }
  }
  for (int d=0; d<2; ++d) {
    g[18+d] = Y(27)*(dL[0][d]*L[1]*L[2]
      + L[0]*dL[1][d]*L[2] + L[0]*L[1]*dL[2][d]);
  }
  return g;
}

template <typename Y, typename G> constexpr
Y* elem::calc_poly_node_grad_T2D10N (Y g[20u], const G x[3u]) noexcept {
  Y node_grad[20] = {};
  calc_poly_grad_node_T2D10N(node_grad, x);
  for (int i=0; i<10; ++i) {
    for (int d=0; d<2; ++d) {
      g[d*10+i] = node_grad[i*2+d];
    }
  }
  return g;
}

// Constrain the Tri10 interior degree of freedom using its quadratic interpolant.
template <typename Y, typename G> constexpr
Y* elem::calc_poly_func_T2D9N (Y f[9u], const G x[3u]) noexcept {
  Y full[10] = {};
  calc_poly_func_T2D10N(full, x);
  for (int i=0; i<9; ++i) {
    const Y weight = i<3 ? -Y(1)/Y(6) : Y(1)/Y(4);
    f[i] = full[i] + weight*full[9];
  }
  return f;
}

template <typename Y, typename G> constexpr
Y* elem::calc_poly_grad_node_T2D9N (Y g[18u], const G x[3u]) noexcept {
  Y full[20] = {};
  calc_poly_grad_node_T2D10N(full, x);
  for (int i=0; i<9; ++i) {
    const Y weight = i<3 ? -Y(1)/Y(6) : Y(1)/Y(4);
    for (int d=0; d<2; ++d) {
      g[2*i+d] = full[2*i+d] + weight*full[18+d];
    }
  }
  return g;
}

template <typename Y, typename G> constexpr
Y* elem::calc_poly_node_grad_T2D9N (Y g[18u], const G x[3u]) noexcept {
  Y full[20] = {};
  calc_poly_node_grad_T2D10N(full, x);
  for (int i=0; i<9; ++i) {
    const Y weight = i<3 ? -Y(1)/Y(6) : Y(1)/Y(4);
    for (int d=0; d<2; ++d) {
      g[d*9+i] = full[d*10+i] + weight*full[d*10+9];
    }
  }
  return g;
}

} }// end fmr::fems namespace

#endif
//end FMR_HAS_FEMS_ELEM_POLY_IPP
