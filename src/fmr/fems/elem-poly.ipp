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
  using Work = typename std::common_type<G, double>::type;
  f[ 0] = Y(Work(1)-Work(x[0]));
  f[ 1] = Y(x[0]);
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
  using Work = typename std::common_type<Y, double>::type;
  const Y L2=Y(x[0]);
  const Y L1=Y(Work(1)-Work(L2));
  f[ 0] = Y(Work(2)*Work(L1)*Work(L1) - Work(L1));// Vertex nodes
  f[ 1] = Y(Work(2)*Work(L2)*Work(L2) - Work(L2));
  f[ 2] = Y(Work(4)*Work(L1)*Work(L2));// Edge node
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_grad_node_C1D3N
(Y g[3u], const G x[3u]) noexcept {
  using Work = typename std::common_type<Y, double>::type;
  const Y xi=Y(x[0]);
  g[0]=Y(Work(4)*Work(xi)-Work(3));
  g[1]=Y(Work(4)*Work(xi)-Work(1));
  g[2]=Y(Work(4)-Work(8)*Work(xi));
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
  using Work = typename std::common_type<Y, double>::type;
  const Y L2=Y(x[0]);
  const Y L1=Y(Work(1)-Work(L2));
  const Work a=Work(L1), b=Work(L2);
  f[ 0]=Y(Work(0.5)*a*(Work(3)*a-Work(1))*(Work(3)*a-Work(2)));
  f[ 1]=Y(Work(0.5)*b*(Work(3)*b-Work(1))*(Work(3)*b-Work(2)));
  // Edge nodes
  const Work ab=Work(Y(L1*L2));
  f[ 2]=Y(Work(4.5)*(ab*(Work(3)*a-Work(1))));
  f[ 3]=Y(Work(4.5)*(ab*(Work(3)*b-Work(1))));
  return f;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_grad_node_C1D4N
(Y g[4u], const G x[3u]) noexcept {
  using Work = typename std::common_type<Y, double>::type;
  const Y xi=Y(x[0]);
  const Y L1=Y(Work(1)-Work(xi));
  const Work a=Work(L1), b=Work(xi);
  g[0]=Y(-Work(0.5)*(Work(27)*a*a-Work(18)*a+Work(2)));
  g[1]=Y( Work(0.5)*(Work(27)*b*b-Work(18)*b+Work(2)));
  g[2]=Y( Work(4.5)*(Work(2)-Work(10)*b+Work(9)*b*b));
  g[3]=Y( Work(4.5)*(-Work(1)+Work(8)*b-Work(9)*b*b));
  return g;
}
template <typename Y=fmr::Phys_float, typename G=fmr::Geom_float> constexpr
Y* elem::calc_poly_node_grad_C1D4N
(Y g[4u], const G x[3u]) noexcept {
  calc_poly_grad_node_C1D4N(g, x);
  return g;
}

namespace elem { namespace detail {

// Edge basis functions are 1D interior functions times linear functions on
// the other axes. Subtract their vertex interpolation to make them nodal.
template <int Dim, int Order, bool Grad, bool Transposed, typename Y, typename G>
constexpr Y* cube_basis (Y* out, const G x[3]) noexcept {
  using Work = typename std::common_type<Y, G, double>::type;
  constexpr int vertices = Dim == 2 ? 4 : 8;
  constexpr int edges = Dim == 2 ? 4 : 12;
  constexpr int nodes = vertices + (Order-1)*edges;
  const int sides[8][3] = {
    {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0},
    {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1}
  };
  const int ends[12][2] = {
    {0,1}, {1,2}, {2,3}, {3,0},
    {4,5}, {5,6}, {6,7}, {7,4},
    {0,4}, {1,5}, {2,6}, {3,7}
  };
  Work linear[3][2] = {}, dlinear[3][2] = {};
  Work edge[3][4] = {}, dedge[3][4] = {};
  for (int d=0; d<Dim; ++d) {
    const Work axis[3] = {Work(x[d]), Work(0), Work(0)};
    calc_poly_func_C1D2N(linear[d], axis);
    if (Grad) calc_poly_grad_node_C1D2N(dlinear[d], axis);
    if (Order == 2) {
      calc_poly_func_C1D3N(edge[d], axis);
      if (Grad) calc_poly_grad_node_C1D3N(dedge[d], axis);
    } else if (Order == 3) {
      calc_poly_func_C1D4N(edge[d], axis);
      if (Grad) calc_poly_grad_node_C1D4N(dedge[d], axis);
    }
  }
  for (int v=0; v<vertices; ++v) {
    if (!Grad) {
      Work value = Work(1);
      for (int d=0; d<Dim; ++d) value *= linear[d][sides[v][d]];
      out[v] = Y(value);
    } else {
      for (int k=0; k<Dim; ++k) {
        Work value = Work(1);
        for (int d=0; d<Dim; ++d) {
          value *= d==k ? dlinear[d][sides[v][d]] : linear[d][sides[v][d]];
        }
        out[Transposed ? k*nodes+v : v*Dim+k] = Y(value);
      }
    }
  }
  for (int e=0; e<edges && Order>1; ++e) {
    const int a = ends[e][0], b = ends[e][1];
    int axis = 0;
    while (sides[a][axis] == sides[b][axis]) ++axis;
    for (int j=0; j<Order-1; ++j) {
      const int one_d = 2 + (sides[a][axis] == 0 ? j : Order-2-j);
      const int node = vertices + e*(Order-1) + j;
      const Work t = Work(j+1)/Work(Order);
      const int components = Grad ? Dim : 1;
      for (int k=0; k<components; ++k) {
        Work value = Grad && k==axis ? dedge[axis][one_d] : edge[axis][one_d];
        for (int d=0; d<Dim; ++d) if (d != axis) {
          value *= Grad && k==d ? dlinear[d][sides[a][d]] : linear[d][sides[a][d]];
        }
        const int index = !Grad ? node : (Transposed ? k*nodes+node : node*Dim+k);
        const int ia = !Grad ? a : (Transposed ? k*nodes+a : a*Dim+k);
        const int ib = !Grad ? b : (Transposed ? k*nodes+b : b*Dim+k);
        out[index] = Y(value);
        out[ia] = Y(Work(out[ia]) - (Work(1)-t)*value);
        out[ib] = Y(Work(out[ib]) - t*value);
      }
    }
  }
  return out;
}

} }// end elem::detail

#define FMR_CUBE_BASIS(NAME, DIM, ORDER, NODES) \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_func_##NAME (Y f[NODES], const G x[3]) noexcept { \
  return elem::detail::cube_basis<DIM,ORDER,false,false>(f,x); \
} \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_grad_node_##NAME (Y g[NODES*DIM], const G x[3]) noexcept { \
  return elem::detail::cube_basis<DIM,ORDER,true,false>(g,x); \
} \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_node_grad_##NAME (Y g[NODES*DIM], const G x[3]) noexcept { \
  return elem::detail::cube_basis<DIM,ORDER,true,true>(g,x); \
}

FMR_CUBE_BASIS(C2D4N,  2, 1, 4)
FMR_CUBE_BASIS(C2D8N,  2, 2, 8)
FMR_CUBE_BASIS(C2D12N, 2, 3, 12)
FMR_CUBE_BASIS(C3D8N,  3, 1, 8)
FMR_CUBE_BASIS(C3D20N, 3, 2, 20)
FMR_CUBE_BASIS(C3D32N, 3, 3, 32)

#undef FMR_CUBE_BASIS

namespace elem { namespace detail {

template <int Dim, int Order, bool Grad, bool Transposed, typename Y, typename G>
constexpr Y* simplex_basis (Y* out, const G x[3]) noexcept {
  using Work = typename std::common_type<Y, G, double>::type;
  constexpr int vertices = Dim+1;
  constexpr int edges = Dim == 2 ? 3 : 6;
  constexpr int faces = Order == 3 ? (Dim == 2 ? 1 : 4) : 0;
  constexpr int nodes = vertices + edges*(Order-1) + faces;
  const int ends[6][2] = {{0,1},{1,2},{2,0},{3,0},{3,2},{3,1}};
  const int face_vertices[4][3] = {{0,2,1},{0,1,3},{0,3,2},{1,2,3}};
  Work bary[4] = {Work(1),Work(0),Work(0),Work(0)};
  for (int d=0; d<Dim; ++d) {
    bary[d+1] = Work(x[d]);
    bary[0] -= bary[d+1];
  }
  for (int n=0; n<nodes; ++n) {
    int alpha[4] = {};
    if (n<vertices) {
      alpha[n] = Order;
    } else if (n<vertices+edges*(Order-1)) {
      const int edge = (n-vertices)/(Order>1 ? Order-1 : 1);
      const int j = (n-vertices)%(Order>1 ? Order-1 : 1)+1;
      alpha[ends[edge][0]] = Order-j;
      alpha[ends[edge][1]] = j;
    } else if (Dim == 2) {
      alpha[0] = alpha[1] = alpha[2] = 1;
    } else {
      const int face = n-vertices-edges*(Order-1);
      for (int i=0; i<3; ++i) alpha[face_vertices[face][i]] = 1;
    }
    // Lagrange factor for barycentric index alpha: product (Order*L-k)/(k+1).
    // Differentiate the product directly, without dividing by a vanishing factor.
    Work factor[4] = {}, derivative[4] = {};
    for (int i=0; i<vertices; ++i) {
      factor[i] = Work(1);
      for (int k=0; k<alpha[i]; ++k) {
        const Work term = (Work(Order)*bary[i]-Work(k))/Work(k+1);
        derivative[i] = derivative[i]*term + factor[i]*Work(Order)/Work(k+1);
        factor[i] *= term;
      }
    }
    if (!Grad) {
      Work value = Work(1);
      for (int i=0; i<vertices; ++i) value *= factor[i];
      out[n] = Y(value);
    } else {
      for (int d=0; d<Dim; ++d) {
        Work value = Work(0);
        for (int i=0; i<vertices; ++i) {
          const int db = i==0 ? -1 : (i==d+1 ? 1 : 0);
          Work term = derivative[i]*Work(db);
          for (int j=0; j<vertices; ++j) if (j!=i) term *= factor[j];
          value += term;
        }
        out[Transposed ? d*nodes+n : n*Dim+d] = Y(value);
      }
    }
  }
  return out;
}

} }// end elem::detail

#define FMR_SIMPLEX_BASIS(NAME, DIM, ORDER, NODES) \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_func_##NAME (Y f[NODES], const G x[3]) noexcept { \
  return elem::detail::simplex_basis<DIM,ORDER,false,false>(f,x); \
} \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_grad_node_##NAME (Y g[NODES*DIM], const G x[3]) noexcept { \
  return elem::detail::simplex_basis<DIM,ORDER,true,false>(g,x); \
} \
template <typename Y, typename G> constexpr \
Y* elem::calc_poly_node_grad_##NAME (Y g[NODES*DIM], const G x[3]) noexcept { \
  return elem::detail::simplex_basis<DIM,ORDER,true,true>(g,x); \
}

FMR_SIMPLEX_BASIS(T2D3N,  2, 1, 3)
FMR_SIMPLEX_BASIS(T2D6N,  2, 2, 6)
FMR_SIMPLEX_BASIS(T2D10N, 2, 3, 10)
FMR_SIMPLEX_BASIS(T3D4N,  3, 1, 4)
FMR_SIMPLEX_BASIS(T3D10N, 3, 2, 10)
FMR_SIMPLEX_BASIS(T3D20N, 3, 3, 20)
#undef FMR_SIMPLEX_BASIS

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
