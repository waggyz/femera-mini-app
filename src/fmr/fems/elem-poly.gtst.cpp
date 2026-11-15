#include "elem-poly.hpp"
#include "../../femera/grid/elem/Tets.hpp"
#include "../../femera/grid/elem/Quad.hpp"
#include "../../femera/grid/elem/Cube.hpp"
#include "../math.hpp"

#include <gtest/gtest.h>
#include <cmath>

int main (int argc, char** argv) {
  return femera::test:: early_main (&argc, argv);
}

TEST(FmrElemPoly, TrivialTest) {
  EXPECT_EQ( 1, 1 );
}

namespace fmr {
TEST(FmrElemPoly, SerendipityPolynomialTermCount) {
  // use to check node count
  //fmr::math::poly_term_n (Poly ptype, uint pord, uint nvars)
  // Bars
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 1), 2 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 1), 3 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 1), 4 );
  // Quad
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 2), 4 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 2), 8 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 2),12 );
  // Cube
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 1, 3), 8 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 2, 3),20 );
  EXPECT_EQ( math::poly_term_n(math::Poly::Serendipity, 3, 3),32 );
}
//
}//end namespace fmr::

namespace {

template <typename T>
constexpr bool constexpr_near(T actual, T expected, T tolerance) {
  const T difference = actual - expected;
  return difference >= -tolerance && difference <= tolerance;
}

constexpr bool triangle_constexpr_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  double f[10] = {};
  double g[20] = {};
  double gt[20] = {};
  return fmr::fems::elem::calc_poly_func_T2D10N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x) == g
    && fmr::fems::elem::calc_poly_node_grad_T2D10N(gt, x) == gt
    && constexpr_near(f[0], 1.0, 1.0e-14)
    && constexpr_near(f[9], 0.0, 1.0e-14)
    && constexpr_near(g[0], -5.5, 1.0e-14)
    && constexpr_near(gt[0], g[0], 1.0e-14);
}
static_assert(triangle_constexpr_test(), "cubic triangle must be constexpr");
constexpr bool triangle_mixed_precision_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  float f[10] = {};
  float g[20] = {};
  return fmr::fems::elem::calc_poly_func_T2D10N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x) == g
    && constexpr_near(f[0], 1.0f, 1.0e-6f)
    && constexpr_near(g[0], -5.5f, 1.0e-6f);
}
static_assert(triangle_mixed_precision_test(), "triangle supports mixed precision");

constexpr bool incomplete_triangle_constexpr_test() {
  const double x[3] = {0.0, 0.0, 0.0};
  float f[9] = {}, g[18] = {}, gt[18] = {};
  return fmr::fems::elem::calc_poly_func_T2D9N(f, x) == f
    && fmr::fems::elem::calc_poly_grad_node_T2D9N(g, x) == g
    && fmr::fems::elem::calc_poly_node_grad_T2D9N(gt, x) == gt
    && constexpr_near(f[0], 1.0f, 1.0e-6f)
    && constexpr_near(g[0], -5.5f, 1.0e-6f)
    && constexpr_near(gt[0], g[0], 1.0e-6f);
}
static_assert(incomplete_triangle_constexpr_test(), "Tri9 must be constexpr");

#define FMR_TEST_CUBE_CONSTEXPR(NAME, NODES, DIM, DERIVATIVE) \
constexpr bool constexpr_##NAME() { \
  const double x[3] = {0,0,0}; \
  float f[NODES] = {}, g[NODES*DIM] = {}, gt[NODES*DIM] = {}; \
  return fmr::fems::elem::calc_poly_func_##NAME(f,x) == f \
    && fmr::fems::elem::calc_poly_grad_node_##NAME(g,x) == g \
    && fmr::fems::elem::calc_poly_node_grad_##NAME(gt,x) == gt \
    && constexpr_near(f[0],1.0f,1e-6f) \
    && constexpr_near(g[0],DERIVATIVE,1e-6f) \
    && constexpr_near(g[0],gt[0],1e-6f); \
} \
static_assert(constexpr_##NAME(), "cube basis must support mixed-precision constexpr");

FMR_TEST_CUBE_CONSTEXPR(C2D4N, 4, 2, -1.0f)
FMR_TEST_CUBE_CONSTEXPR(C2D8N, 8, 2, -3.0f)
FMR_TEST_CUBE_CONSTEXPR(C2D12N, 12, 2, -5.5f)
FMR_TEST_CUBE_CONSTEXPR(C3D8N, 8, 3, -1.0f)
FMR_TEST_CUBE_CONSTEXPR(C3D20N, 20, 3, -3.0f)
FMR_TEST_CUBE_CONSTEXPR(C3D32N, 32, 3, -5.5f)
#undef FMR_TEST_CUBE_CONSTEXPR

template <class Topology, int Order, int Dim, int NodeN,
          typename Values, typename Gradient, typename Transposed>
void check_cube_basis(Values values, Gradient gradient, Transposed transposed) {
  using E = femera::grid::elem::Elem<Topology,Order>;
  double nodes[std::size_t(NodeN*3)] = {};
  ASSERT_EQ(E::get_node_n(), NodeN);
  ASSERT_TRUE(E::get_elem_node_coor(nodes));
  for (int n=0; n<NodeN; ++n) {
    double guarded[std::size_t(NodeN+2)] = {};
    guarded[0] = 123;
    guarded[NodeN+1] = 456;
    ASSERT_EQ(values(guarded+1, nodes+3*n), guarded+1);
    EXPECT_DOUBLE_EQ(guarded[0], 123);
    EXPECT_DOUBLE_EQ(guarded[NodeN+1], 456);
    for (int i=0; i<NodeN; ++i) EXPECT_NEAR(guarded[1+i], i==n ? 1.0 : 0.0, 1e-13);
  }
  const double points[3][3] = {{0.17,0.31,0.47}, {0.72,0.24,0.61}, {0,0.5,1}};
  for (const auto& x : points) {
    double f[std::size_t(NodeN)] = {}, guard[std::size_t(NodeN*Dim+2)] = {};
    double guardt[std::size_t(NodeN*Dim+2)] = {};
    guard[0] = guardt[0] = 123;
    guard[NodeN*Dim+1] = guardt[NodeN*Dim+1] = 456;
    double* g = guard+1;
    double* gt = guardt+1;
    ASSERT_EQ(values(f,x), f);
    ASSERT_EQ(gradient(g,x), g);
    ASSERT_EQ(transposed(gt,x), gt);
    EXPECT_DOUBLE_EQ(guard[0], 123);
    EXPECT_DOUBLE_EQ(guard[NodeN*Dim+1], 456);
    EXPECT_DOUBLE_EQ(guardt[0], 123);
    EXPECT_DOUBLE_EQ(guardt[NodeN*Dim+1], 456);
    for (int d=0; d<Dim; ++d) {
      double xp[3] = {x[0],x[1],x[2]}, xm[3] = {x[0],x[1],x[2]};
      xp[d] += 1e-6;
      xm[d] -= 1e-6;
      double fp[std::size_t(NodeN)] = {}, fm[std::size_t(NodeN)] = {}, sum = 0;
      values(fp,xp);
      values(fm,xm);
      for (int i=0; i<NodeN; ++i) {
        EXPECT_DOUBLE_EQ(g[Dim*i+d], gt[d*NodeN+i]);
        EXPECT_NEAR(g[Dim*i+d], (fp[i]-fm[i])/2e-6, 1e-8);
        sum += g[Dim*i+d];
      }
      EXPECT_NEAR(sum,0,1e-12);
    }
    for (int a=0; a<=Order; ++a) for (int b=0; b<=Order; ++b)
      for (int c=0; c<=(Dim==3 ? Order : 0); ++c) {
        const int superlinear = (a>1 ? a : 0)+(b>1 ? b : 0)+(c>1 ? c : 0);
        if (superlinear>Order) continue;
        const int powers[3] = {a,b,c};
        double actual = 0, deriv[3] = {};
        for (int i=0; i<NodeN; ++i) {
          double nodal = 1;
          for (int d=0; d<Dim; ++d) nodal *= std::pow(nodes[3*i+d], powers[d]);
          actual += nodal*f[i];
          for (int d=0; d<Dim; ++d) deriv[d] += nodal*g[Dim*i+d];
        }
        double expected = 1;
        for (int d=0; d<Dim; ++d) expected *= std::pow(x[d], powers[d]);
        EXPECT_NEAR(actual,expected,1e-12);
        for (int d=0; d<Dim; ++d) {
          double derivative = powers[d];
          if (powers[d] > 0) {
            for (int k=0; k<Dim; ++k)
              derivative *= std::pow(x[k],powers[k]-(k==d ? 1 : 0));
          }
          EXPECT_NEAR(deriv[d],derivative,1e-12);
        }
      }
  }
}

template <int Dim, int NodeN, typename FloatValues, typename FloatGrad,
          typename FloatTransposed, typename DoubleValues, typename DoubleGrad>
void check_cube_mixed_precision(FloatValues values, FloatGrad gradient,
    FloatTransposed transposed, DoubleValues double_values, DoubleGrad double_gradient) {
  const double x[3] = {0.17,0.41,0.72};
  float f[std::size_t(NodeN)] = {}, g[std::size_t(Dim*NodeN)] = {};
  float gt[std::size_t(Dim*NodeN)] = {};
  double expected[std::size_t(NodeN)] = {}, eg[std::size_t(Dim*NodeN)] = {};
  values(f,x); gradient(g,x); transposed(gt,x);
  double_values(expected,x); double_gradient(eg,x);
  for (int i=0; i<NodeN; ++i) {
    EXPECT_NEAR(f[i],expected[i],2e-6);
    for (int d=0; d<Dim; ++d) {
      EXPECT_FLOAT_EQ(g[i*Dim+d],gt[d*NodeN+i]);
      EXPECT_NEAR(g[i*Dim+d],eg[i*Dim+d],2e-6);
    }
  }
}

template <fmr::Local_int NodeN, typename Values, typename Gradient,
          typename TransposedGradient>
void expect_bar_gradient_matches_finite_difference(
  const double (&reference_nodes)[NodeN], Values values, Gradient gradient,
  TransposedGradient transposed_gradient) {
  const double points[] = {0.17, 0.43, 0.76};
  const double h = 1.0e-6;
  for(const double xi : points){
    SCOPED_TRACE(xi);
    const double x[3] = {xi, 0.0, 0.0};
    double dN[NodeN];
    double dN_transposed[NodeN];
    gradient(dN, x);
    transposed_gradient(dN_transposed, x);
    double derivative_sum = 0.0;
    for(fmr::Local_int node=0; node<NodeN; ++node){
      const double x_plus[3] = {xi+h, 0.0, 0.0};
      const double x_minus[3] = {xi-h, 0.0, 0.0};
      double N_plus[NodeN];
      double N_minus[NodeN];
      values(N_plus, x_plus);
      values(N_minus, x_minus);
      const double numerical = (N_plus[node]-N_minus[node])/(2.0*h);
      EXPECT_NEAR(dN[node], numerical, 1.0e-8);
      EXPECT_DOUBLE_EQ(dN_transposed[node], dN[node]);
      derivative_sum += dN[node];
    }
    EXPECT_NEAR(derivative_sum, 0.0, 1.0e-12);
  }
  for(fmr::Local_int node=0; node<NodeN; ++node){
    const double x[3] = {reference_nodes[node], 0.0, 0.0};
    double N[NodeN];
    values(N, x);
    for(fmr::Local_int basis=0; basis<NodeN; ++basis){
      EXPECT_NEAR(N[basis], basis == node ? 1.0 : 0.0, 1.0e-14);
    }
  }
}

}// end anonymous namespace

TEST(FmrElemPoly, SerendipityCubeBasisAndGradients) {
  using femera::grid::elem::Quad;
  using femera::grid::elem::Cube;
  using namespace fmr::fems::elem;
#define FMR_CHECK_CUBE(TOPO, ORDER, DIM, NODES, NAME) \
  check_cube_basis<TOPO,ORDER,DIM,NODES>( \
    calc_poly_func_##NAME<double,double>, calc_poly_grad_node_##NAME<double,double>, \
    calc_poly_node_grad_##NAME<double,double>);
  FMR_CHECK_CUBE(Quad,1,2,4,C2D4N)
  FMR_CHECK_CUBE(Quad,2,2,8,C2D8N)
  FMR_CHECK_CUBE(Quad,3,2,12,C2D12N)
  FMR_CHECK_CUBE(Cube,1,3,8,C3D8N)
  FMR_CHECK_CUBE(Cube,2,3,20,C3D20N)
  FMR_CHECK_CUBE(Cube,3,3,32,C3D32N)
#undef FMR_CHECK_CUBE
}

TEST(FmrElemPoly, SerendipityCubeMixedPrecision) {
  using namespace fmr::fems::elem;
#define FMR_CHECK_MIXED(DIM, NODES, NAME) \
  check_cube_mixed_precision<DIM,NODES>(calc_poly_func_##NAME<float,double>, \
    calc_poly_grad_node_##NAME<float,double>, calc_poly_node_grad_##NAME<float,double>, \
    calc_poly_func_##NAME<double,double>, calc_poly_grad_node_##NAME<double,double>);
  FMR_CHECK_MIXED(2,4,C2D4N)
  FMR_CHECK_MIXED(2,8,C2D8N)
  FMR_CHECK_MIXED(2,12,C2D12N)
  FMR_CHECK_MIXED(3,8,C3D8N)
  FMR_CHECK_MIXED(3,20,C3D20N)
  FMR_CHECK_MIXED(3,32,C3D32N)
#undef FMR_CHECK_MIXED
}

namespace {
template <int Order, int QuadN, int CubeN, typename Values2, typename Grad2,
          typename Values3, typename Grad3>
void check_cube_face_traces(Values2 values2, Grad2 grad2, Values3 values3, Grad3 grad3) {
  using femera::grid::elem::Cube;
  for (int face=0; face<6; ++face) {
    const auto* verts = Cube::quad_vert_conn+4*face;
    int mapping[std::size_t(QuadN)] = {};
    for (int i=0; i<4; ++i) mapping[i] = int(verts[i]);
    for (int e=0; e<4 && Order>1; ++e) {
      const auto a = verts[e], b = verts[(e+1)%4];
      int match = -1;
      bool reverse = false;
      for (int j=0; j<12; ++j) {
        const auto first = Cube::bars_vert_conn[2*j], last = Cube::bars_vert_conn[2*j+1];
        if ((first==a && last==b) || (first==b && last==a)) {
          match = j;
          reverse = first!=a;
        }
      }
      ASSERT_GE(match,0);
      for (int j=0; j<Order-1; ++j) {
        mapping[4+e*(Order-1)+j] = 8+match*(Order-1)
          +(reverse ? Order-2-j : j);
      }
    }
    const double points[2][3] = {{0.21,0.37,0}, {0.72,0.18,0}};
    for (const auto& uv : points) {
      double x[3] = {}, tangent[2][3] = {};
      for (int d=0; d<3; ++d) {
        const double a = Cube::vert_coor[3*int(verts[0])+d];
        tangent[0][d] = Cube::vert_coor[3*int(verts[1])+d]-a;
        tangent[1][d] = Cube::vert_coor[3*int(verts[3])+d]-a;
        x[d] = a+uv[0]*tangent[0][d]+uv[1]*tangent[1][d];
      }
      double f2[std::size_t(QuadN)] = {}, g2[std::size_t(QuadN*2)] = {};
      double f3[std::size_t(CubeN)] = {}, g3[std::size_t(CubeN*3)] = {};
      values2(f2,uv); grad2(g2,uv); values3(f3,x); grad3(g3,x);
      for (int i=0; i<QuadN; ++i) {
        EXPECT_NEAR(f2[i], f3[mapping[i]], 1e-13);
        for (int d=0; d<2; ++d) {
          double tangential = 0;
          for (int k=0; k<3; ++k) tangential += g3[3*mapping[i]+k]*tangent[d][k];
          EXPECT_NEAR(g2[2*i+d], tangential, 1e-13);
        }
      }
      for (int i=0; i<CubeN; ++i) {
        bool belongs = false;
        for (int j=0; j<QuadN; ++j) if (mapping[j]==i) belongs = true;
        if (!belongs) {
          EXPECT_NEAR(f3[i],0,1e-13);
          for (int d=0; d<2; ++d) {
            double tangential = 0;
            for (int k=0; k<3; ++k) tangential += g3[3*i+k]*tangent[d][k];
            EXPECT_NEAR(tangential,0,1e-13);
          }
        }
      }
    }
  }
}

template <int Order, int NodeN, typename Values, typename Gradient,
          typename BarValues, typename BarGradient>
void check_quad_edges(Values values, Gradient gradient, BarValues bar_values,
                      BarGradient bar_gradient) {
  using femera::grid::elem::Quad;
  for (int e=0; e<4; ++e) for (double t : {0.17,0.52,0.84}) {
    const auto a=Quad::bars_vert_conn[2*e], b=Quad::bars_vert_conn[2*e+1];
    double x[3] = {}, tangent[2] = {};
    for (int d=0; d<2; ++d) {
      tangent[d] = Quad::vert_coor[3*int(b)+d]-Quad::vert_coor[3*int(a)+d];
      x[d] = Quad::vert_coor[3*int(a)+d]+t*tangent[d];
    }
    const double axis[3] = {t,0,0};
    double f[std::size_t(NodeN)] = {}, g[std::size_t(NodeN*2)] = {};
    double bar[std::size_t(Order+1)] = {}, bg[std::size_t(Order+1)] = {};
    values(f,x); gradient(g,x); bar_values(bar,axis); bar_gradient(bg,axis);
    for (int i=0; i<NodeN; ++i) {
      int idx = i==int(a) ? 0 : (i==int(b) ? 1 : -1);
      for (int j=0; j<Order-1; ++j) {
        if (i==4+e*(Order-1)+j) idx = 2+j;
      }
      EXPECT_NEAR(f[i],idx<0 ? 0 : bar[idx],1e-13);
      EXPECT_NEAR(g[2*i]*tangent[0]+g[2*i+1]*tangent[1],
        idx<0 ? 0 : bg[idx],1e-13);
    }
  }
}
}

TEST(FmrElemPoly, SerendipityCubeFaceAndEdgeTraces) {
  using namespace fmr::fems::elem;
#define FMR_CHECK_TRACE(ORDER, QN, CN, Q, C, B) \
  check_cube_face_traces<ORDER,QN,CN>(calc_poly_func_##Q<double,double>, \
    calc_poly_grad_node_##Q<double,double>, calc_poly_func_##C<double,double>, \
    calc_poly_grad_node_##C<double,double>); \
  check_quad_edges<ORDER,QN>(calc_poly_func_##Q<double,double>, \
    calc_poly_grad_node_##Q<double,double>, calc_poly_func_##B<double,double>, \
    calc_poly_grad_node_##B<double,double>);
  FMR_CHECK_TRACE(1,4,8,C2D4N,C3D8N,C1D2N)
  FMR_CHECK_TRACE(2,8,20,C2D8N,C3D20N,C1D3N)
  FMR_CHECK_TRACE(3,12,32,C2D12N,C3D32N,C1D4N)
#undef FMR_CHECK_TRACE
}

TEST(FmrElemPoly, BarReferenceGradientsMatchFiniteDifferences) {
  const double p1_nodes[2] = {0.0, 1.0};
  const double p2_nodes[3] = {0.0, 1.0, 0.5};
  const double p3_nodes[4] = {0.0, 1.0, 1.0/3.0, 2.0/3.0};
  expect_bar_gradient_matches_finite_difference<2>(
    p1_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D2N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D2N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D2N(dN, x);
    });
  expect_bar_gradient_matches_finite_difference<3>(
    p2_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D3N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D3N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D3N(dN, x);
    });
  expect_bar_gradient_matches_finite_difference<4>(
    p3_nodes,
    [](double* N, const double* x) {
      return fmr::fems::elem::calc_poly_func_C1D4N(N, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_grad_node_C1D4N(dN, x);
    },
    [](double* dN, const double* x) {
      return fmr::fems::elem::calc_poly_node_grad_C1D4N(dN, x);
    });
}

TEST(FmrElemPoly, CubicTriangleBasisAndGradients) {
  const double nodes[10][3] = {
    {0,0,0}, {1,0,0}, {0,1,0},
    {1.0/3,0,0}, {2.0/3,0,0},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,0}, {0,1.0/3,0},
    {1.0/3,1.0/3,0}
  };
  for (int n=0; n<10; ++n) {
    double f[10] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, nodes[n]);
    for (int i=0; i<10; ++i) EXPECT_NEAR(f[i], i==n ? 1.0 : 0.0, 1e-14);
  }
  const double points[3][3] = {{0.17,0.21,0}, {0.43,0.13,0}, {0.0,0.5,0}};
  for (const auto& x : points) {
    double f[10] = {}, g[20] = {}, gt[20] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(g, x);
    fmr::fems::elem::calc_poly_node_grad_T2D10N(gt, x);
    for (int d=0; d<2; ++d) {
      double xp[3] = {x[0], x[1], 0}, xm[3] = {x[0], x[1], 0};
      xp[d] += 1e-6;
      xm[d] -= 1e-6;
      double fp[10] = {}, fm[10] = {};
      fmr::fems::elem::calc_poly_func_T2D10N(fp, xp);
      fmr::fems::elem::calc_poly_func_T2D10N(fm, xm);
      double sum = 0;
      for (int i=0; i<10; ++i) {
        EXPECT_DOUBLE_EQ(g[2*i+d], gt[d*10+i]);
        EXPECT_NEAR(g[2*i+d], (fp[i]-fm[i])/2e-6, 1e-8);
        sum += g[2*i+d];
      }
      EXPECT_NEAR(sum, 0, 1e-13);
    }
    for (int a=0; a<=3; ++a) for (int b=0; b<=3-a; ++b) {
      double value = 0, dx = 0, dy = 0;
      for (int i=0; i<10; ++i) {
        const double nodal = std::pow(nodes[i][0], a)*std::pow(nodes[i][1], b);
        value += f[i]*nodal;
        dx += g[2*i]*nodal;
        dy += g[2*i+1]*nodal;
      }
      EXPECT_NEAR(value, std::pow(x[0],a)*std::pow(x[1],b), 1e-13);
      EXPECT_NEAR(dx, a ? a*std::pow(x[0],a-1)*std::pow(x[1],b) : 0, 1e-13);
      EXPECT_NEAR(dy, b ? b*std::pow(x[0],a)*std::pow(x[1],b-1) : 0, 1e-13);
    }
  }
}

TEST(FmrElemPoly, CubicTriangleConstantLoadIntegration) {
  const double points[4][3] = {
    {1.0/3,1.0/3,0}, {0.2,0.2,0}, {0.6,0.2,0}, {0.2,0.6,0}
  };
  const double weights[4] = {-27.0/96,25.0/96,25.0/96,25.0/96};
  double loads[10] = {};
  for (int q=0; q<4; ++q) {
    double f[10] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(f, points[q]);
    for (int i=0; i<10; ++i) loads[i] += weights[q]*f[i];
  }

  for (int i=0; i<10; ++i) {
    EXPECT_NEAR(loads[i], i<3 ? 1.0/60 : (i<9 ? 3.0/80 : 9.0/40), 1e-14);
  }
}

TEST(FmrElemPoly, CubicTriangleMatchesTetrahedronFace) {
  const double points[3][3] = {{0.1,0.2,0}, {0.4,0.3,0}, {0.0,0.7,0}};
  const int face_nodes[10] = {0,1,2,4,5,6,7,8,9,16};
  for (const auto& x : points) {
    double triangle[10] = {}, tri_grad[20] = {};
    double tetrahedron[20] = {}, tet_grad[60] = {};
    fmr::fems::elem::calc_poly_func_T2D10N(triangle, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(tri_grad, x);
    femera::grid::elem::Tets::calc_poly_func_20(tetrahedron, x);
    femera::grid::elem::Tets::calc_poly_grad_node_20(tet_grad, x);
    for (int i=0; i<10; ++i) {
      EXPECT_NEAR(triangle[i], tetrahedron[face_nodes[i]], 1e-14);
      for (int d=0; d<2; ++d) {
        EXPECT_NEAR(tri_grad[2*i+d], tet_grad[d*20+face_nodes[i]], 1e-14);
      }
    }
  }
}

TEST(FmrElemPoly, IncompleteCubicTriangleBasisAndGradients) {
  const double nodes[9][3] = {
    {0,0,0}, {1,0,0}, {0,1,0},
    {1.0/3,0,0}, {2.0/3,0,0},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,0}, {0,1.0/3,0}
  };
  for (int n=0; n<9; ++n) {
    double guarded[11] = {};
    guarded[0] = 123;
    guarded[10] = 456;
    ASSERT_EQ(fmr::fems::elem::calc_poly_func_T2D9N(guarded+1, nodes[n]), guarded+1);
    EXPECT_DOUBLE_EQ(guarded[0], 123);
    EXPECT_DOUBLE_EQ(guarded[10], 456);
    for (int i=0; i<9; ++i) EXPECT_NEAR(guarded[i+1], i==n ? 1.0 : 0.0, 1e-14);
  }
  const double points[4][3] = {
    {0.17,0.21,0}, {0.43,0.13,0}, {0.0,0.5,0}, {1.0/3,1.0/3,0}
  };
  for (const auto& x : points) {
    double f[9] = {}, g_storage[20] = {}, gt_storage[20] = {};
    g_storage[0] = gt_storage[0] = 123;
    g_storage[19] = gt_storage[19] = 456;
    double* g = g_storage+1;
    double* gt = gt_storage+1;
    fmr::fems::elem::calc_poly_func_T2D9N(f, x);
    ASSERT_EQ(fmr::fems::elem::calc_poly_grad_node_T2D9N(g, x), g);
    ASSERT_EQ(fmr::fems::elem::calc_poly_node_grad_T2D9N(gt, x), gt);
    EXPECT_DOUBLE_EQ(g_storage[0], 123);
    EXPECT_DOUBLE_EQ(g_storage[19], 456);
    EXPECT_DOUBLE_EQ(gt_storage[0], 123);
    EXPECT_DOUBLE_EQ(gt_storage[19], 456);
    for (int d=0; d<2; ++d) {
      double xp[3] = {x[0], x[1], 0}, xm[3] = {x[0], x[1], 0};
      xp[d] += 1e-6;
      xm[d] -= 1e-6;
      double fp[9] = {}, fm[9] = {}, sum = 0;
      fmr::fems::elem::calc_poly_func_T2D9N(fp, xp);
      fmr::fems::elem::calc_poly_func_T2D9N(fm, xm);
      for (int i=0; i<9; ++i) {
        EXPECT_DOUBLE_EQ(g[2*i+d], gt[d*9+i]);
        EXPECT_NEAR(g[2*i+d], (fp[i]-fm[i])/2e-6, 1e-8);
        sum += g[2*i+d];
      }
      EXPECT_NEAR(sum, 0, 1e-13);
    }
    for (int a=0; a<=2; ++a) for (int b=0; b<=2-a; ++b) {
      double value = 0, dx = 0, dy = 0;
      for (int i=0; i<9; ++i) {
        const double nodal = std::pow(nodes[i][0], a)*std::pow(nodes[i][1], b);
        value += f[i]*nodal;
        dx += g[2*i]*nodal;
        dy += g[2*i+1]*nodal;
      }
      EXPECT_NEAR(value, std::pow(x[0],a)*std::pow(x[1],b), 1e-13);
      EXPECT_NEAR(dx, a ? a*std::pow(x[0],a-1)*std::pow(x[1],b) : 0, 1e-13);
      EXPECT_NEAR(dy, b ? b*std::pow(x[0],a)*std::pow(x[1],b-1) : 0, 1e-13);
    }
  }
}

TEST(FmrElemPoly, IncompleteTriangleMatchesCubicEdgesAndConstrainedTri10) {
  const double vertices[3][2] = {{0,0}, {1,0}, {0,1}};
  const double parameters[] = {0.17, 0.5, 0.83};
  for (int edge=0; edge<3; ++edge) for (const double t : parameters) {
    const int next = (edge+1)%3;
    const double x[3] = {
      (1-t)*vertices[edge][0]+t*vertices[next][0],
      (1-t)*vertices[edge][1]+t*vertices[next][1], 0
    };
    const double xi[3] = {t,0,0};
    double tri[9] = {}, grad[18] = {}, bar[4] = {}, dbar[4] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(tri, x);
    fmr::fems::elem::calc_poly_grad_node_T2D9N(grad, x);
    fmr::fems::elem::calc_poly_func_C1D4N(bar, xi);
    fmr::fems::elem::calc_poly_grad_node_C1D4N(dbar, xi);
    const int indices[4] = {edge,next,3+2*edge,4+2*edge};
    for (int i=0; i<9; ++i) {
      int mapped = -1;
      for (int j=0; j<4; ++j) if (indices[j] == i) mapped = j;
      const double tangent = grad[2*i]*(vertices[next][0]-vertices[edge][0])
        + grad[2*i+1]*(vertices[next][1]-vertices[edge][1]);
      EXPECT_NEAR(tri[i], mapped<0 ? 0 : bar[mapped], 1e-14);
      EXPECT_NEAR(tangent, mapped<0 ? 0 : dbar[mapped], 1e-13);
    }
  }
  const double points[3][3] = {{0.13,0.21,0}, {0.5,0.2,0}, {1.0/3,1.0/3,0}};
  const double nodal[9] = {0.7,-1.1,2.0,0.2,3.0,-0.5,1.7,-2.0,0.3};
  double center = 0;
  for (int i=0; i<9; ++i) center += (i<3 ? -1.0/6 : 1.0/4)*nodal[i];
  for (const auto& x : points) {
    double f9[9] = {}, f10[10] = {}, g9[18] = {}, g10[20] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(f9, x);
    fmr::fems::elem::calc_poly_func_T2D10N(f10, x);
    fmr::fems::elem::calc_poly_grad_node_T2D9N(g9, x);
    fmr::fems::elem::calc_poly_grad_node_T2D10N(g10, x);
    double v9 = 0, v10 = center*f10[9];
    double d9[2] = {}, d10[2] = {center*g10[18],center*g10[19]};
    for (int i=0; i<9; ++i) {
      v9 += nodal[i]*f9[i];
      v10 += nodal[i]*f10[i];
      for (int d=0; d<2; ++d) {
        d9[d] += nodal[i]*g9[2*i+d];
        d10[d] += nodal[i]*g10[2*i+d];
      }
    }
    EXPECT_NEAR(v9, v10, 1e-14);
    for (int d=0; d<2; ++d) EXPECT_NEAR(d9[d], d10[d], 1e-13);
  }
}

TEST(FmrElemPoly, IncompleteTriangleConstantLoadIntegration) {
  const double points[4][3] = {
    {1.0/3,1.0/3,0}, {0.2,0.2,0}, {0.6,0.2,0}, {0.2,0.6,0}
  };
  const double weights[4] = {-27.0/96,25.0/96,25.0/96,25.0/96};
  double loads[9] = {};
  for (int q=0; q<4; ++q) {
    double f[9] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(f, points[q]);
    for (int i=0; i<9; ++i) loads[i] += weights[q]*f[i];
  }
  for (int i=0; i<9; ++i) {
    EXPECT_NEAR(loads[i], i<3 ? -1.0/48 : 3.0/32, 1e-14);
  }
}

TEST(FmrElemPoly, IncompleteTriangleIsSymmetricUnderVertexPermutations) {
  const int permutations[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
  };
  const double bary[9][3] = {
    {1,0,0}, {0,1,0}, {0,0,1},
    {2.0/3,1.0/3,0}, {1.0/3,2.0/3,0},
    {0,2.0/3,1.0/3}, {0,1.0/3,2.0/3},
    {1.0/3,0,2.0/3}, {2.0/3,0,1.0/3}
  };
  const double lambda[3] = {0.26,0.31,0.43};
  const double x[3] = {lambda[1], lambda[2], 0};
  double original[9] = {};
  fmr::fems::elem::calc_poly_func_T2D9N(original, x);
  for (const auto& perm : permutations) {
    const double xp[3] = {lambda[perm[1]], lambda[perm[2]], 0};
    double transformed[9] = {};
    fmr::fems::elem::calc_poly_func_T2D9N(transformed, xp);
    for (int i=0; i<9; ++i) {
      int mapped = -1;
      for (int j=0; j<9; ++j) {
        bool match = true;
        for (int k=0; k<3; ++k) {
          if (std::abs(bary[j][k]-bary[i][perm[k]]) > 1e-14) match = false;
        }
        if (match) mapped = j;
      }
      ASSERT_GE(mapped, 0);
      EXPECT_NEAR(original[i], transformed[mapped], 1e-14);
    }
  }
}
